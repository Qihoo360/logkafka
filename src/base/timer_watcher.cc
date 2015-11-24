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
#include "base/timer_watcher.h"

namespace base {

bool TimerWatcher::init(uv_loop_t *loop, 
        long timeout,
        long repeat,
        void *event_cb_func_arg,
        TimerFunc event_cb_func)
{/*{{{*/
    m_loop = loop;
    m_event_cb_func_arg = event_cb_func_arg;
    m_event_cb_func = event_cb_func;
    m_timeout = timeout;
    m_repeat = repeat;

    m_handle = new uv_timer_t();
    int res = uv_timer_init(m_loop, m_handle);
    if (res < 0) {
        LERROR << "Fail to init timer, " << uv_strerror(res);
        delete m_handle; m_handle = NULL;
        return false;
    }

    m_handle->data = this;

    res = uv_timer_start(m_handle, (uv_timer_cb) &cb_func, m_timeout, m_repeat);
    if (res < 0) {
        LERROR << "Fail to start fs event, " << uv_strerror(res);
        close();
        return false;
    }

    return true;
}/*}}}*/

void TimerWatcher::cb_func(uv_timer_t *w)
{/*{{{*/
    TimerWatcher *tw = reinterpret_cast<TimerWatcher *>(w->data);
    if (NULL == tw->m_event_cb_func)
        return;

    (*tw->m_event_cb_func)(tw->m_event_cb_func_arg);
}/*}}}*/

void TimerWatcher::stop()
{/*{{{*/
    uv_timer_stop(m_handle);
}/*}}}*/

void TimerWatcher::start()
{/*{{{*/
    uv_timer_start(m_handle, cb_func, m_timeout, m_repeat);
}/*}}}*/

void TimerWatcher::on_timer_close_complete(uv_handle_t* handle)
{/*{{{*/
    delete (uv_timer_t *)handle;
}/*}}}*/

void TimerWatcher::close()
{/*{{{*/
    if (NULL == m_handle) return;
    uv_close((uv_handle_t *)m_handle, on_timer_close_complete);
    while (uv_is_active((uv_handle_t *)m_handle)) { }
}/*}}}*/

} // namespace base
