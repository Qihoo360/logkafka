///////////////////////////////////////////////////////////////////////////
//
// logkafka - Collect logs and send lines to Apache Kafka v0.8+
//
///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Qihoo 360 Technology Co., Ltd. All rights reserved.
//
// Licensed under the MIT License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////
#include "logkafka/logkafka.h"

#include "easylogging/easylogging++.h"

namespace logkafka {

LogKafka::LogKafka(const Config *config)
{/*{{{*/
    m_config = config;
    m_manager = NULL;
    m_loop = NULL;
    m_signal_handler = NULL;
    m_upload_timer_trigger = NULL;
}/*}}}*/

LogKafka::~LogKafka()
{/*{{{*/
    delete m_upload_timer_trigger; m_upload_timer_trigger = NULL;
    delete m_signal_handler; m_signal_handler = NULL;
    delete m_manager; m_manager = NULL;
    delete m_loop; m_loop = NULL;
}/*}}}*/

bool LogKafka::init()
{/*{{{*/
    /* Use just one loop for all watchers */
    m_loop = new uv_loop_t();
    int res = uv_loop_init(m_loop);
    if (res < 0) {
        LERROR << "Fail to init uv loop, " << uv_strerror(res);
        return false;
    }

    m_manager = new Manager(m_config);
    if (!m_manager->init(m_loop)) {
        LERROR << "Fail to init manager";
        return false;
    }

    return true;
}/*}}}*/

bool LogKafka::start()
{/*{{{*/
    if (!m_manager->start()) {
        LERROR << "Fail to start manager";
        return false;
    }

    m_signal_handler = new SignalHandler();
    if (!m_signal_handler->init(m_loop, SIGINT, this, onSignal)) {
        LERROR << "Fail to init signal handler";
        delete m_signal_handler; m_signal_handler= NULL;
        return false;
    }

    m_upload_timer_trigger = new TimerWatcher();
    if (!m_upload_timer_trigger->init(m_loop, 0,
            m_config->zookeeper_upload_interval,
            m_manager, &Manager::uploadCollectingState)) {
        LERROR << "Fail to init update timer";
        delete m_upload_timer_trigger; m_upload_timer_trigger = NULL;
        return false;
    }

    /* The existence of the async handle will keep the loop alive. */  
    m_exit_handle.data = this;
    uv_async_init(m_loop, &m_exit_handle, exitAsyncCb);

    int res = uv_run(m_loop, UV_RUN_DEFAULT);
    if (res < 0) {
        LERROR << "Fail to run loop, " << uv_strerror(res);
        return false;
    }

    return true;
}/*}}}*/

void LogKafka::onSignal(void *arg)
{/*{{{*/
    if (NULL == arg) return;
    LogKafka *lk = reinterpret_cast<LogKafka *>(arg);

    lk->close();
}/*}}}*/

void LogKafka::exitAsyncCb(uv_async_t* handle) 
{/*{{{*/
    /* After closing the async handle, it will no longer keep the loop alive. */
    LogKafka *lk = reinterpret_cast<LogKafka *>(handle->data);
    uv_close((uv_handle_t*) &lk->m_exit_handle, NULL);
} /*}}}*/

void LogKafka::stop()
{/*{{{*/
    assert(NULL != m_loop);
    m_upload_timer_trigger->stop();
    m_manager->stop();
    uv_stop(m_loop);
}/*}}}*/

void LogKafka::close()
{/*{{{*/
    assert(NULL != m_loop);
    stop();
    m_upload_timer_trigger->close();
    uv_async_send(&m_exit_handle);
}/*}}}*/

} // namespace logkafka
