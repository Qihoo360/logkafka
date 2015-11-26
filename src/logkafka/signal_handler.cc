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
#include "logkafka/signal_handler.h"

namespace logkafka {

bool SignalHandler::init(uv_loop_t *loop, 
        int signum,
        void *signal_func_arg,
        SignalFunc on_signal)
{
    m_signal_func_arg = signal_func_arg;
    m_signal_func = on_signal;

    if (uv_signal_init(loop, &m_handle) < 0) {
        LERROR << "Fail to init uv signal for signal " << sig2str(signum);
        return false;
    }

    m_handle.data = this;

    if (uv_signal_start(&m_handle, default_cb, signum) < 0) {
        LERROR << "Fail to start uv signal for signal " << sig2str(signum);
        return false;
    }

    return true;
}

void SignalHandler::default_cb(uv_signal_t* handle, int signum)
{
    SignalHandler *sh = reinterpret_cast<SignalHandler *>(handle->data);
    if (NULL == sh->m_signal_func) {
        LERROR << "Signal handler callback function is NULL";
        return;
    }

    LINFO << "Receiving signal " << sig2str(signum);
    (*sh->m_signal_func)(sh->m_signal_func_arg);

    uv_signal_stop(&sh->m_handle);
}

} // namespace logkafka
