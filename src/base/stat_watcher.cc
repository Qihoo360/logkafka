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
#include "base/stat_watcher.h"

namespace base {

bool StatWatcher::init(uv_loop_t *loop,
        string path, 
        long interval,
        void *event_cb_func_arg, 
        StatFunc event_cb_func)
{/*{{{*/
    m_loop = loop;
    m_path = path;
    m_event_cb_func_arg = event_cb_func_arg;
    m_event_cb_func = event_cb_func;

    m_handle = new uv_fs_poll_t();
    int res = uv_fs_poll_init(m_loop, m_handle);
    if (res < 0) {
        LERROR << "Fail to init fs event, " << uv_strerror(res);
        return false;
    }

    m_handle->data = this;
    res = uv_fs_poll_start(m_handle, cb_func, m_path.c_str(), interval);
    if (res < 0) {
        LERROR << "Fail to start fs event, " << uv_strerror(res);
        close();
        return false;
    }

    return true;
}/*}}}*/

void StatWatcher::cb_func(uv_fs_poll_t* handle, 
        int status, 
        const uv_stat_t* prev, 
        const uv_stat_t* curr)
{/*{{{*/
    StatWatcher *sw = reinterpret_cast<StatWatcher *>(handle->data);

    if (NULL == sw->m_event_cb_func) {
        LERROR << "stat watcher callback function is NULL";
        return;
    }

    (*sw->m_event_cb_func)(sw->m_event_cb_func_arg);
}/*}}}*/

void StatWatcher::stop()
{/*{{{*/
    uv_fs_poll_stop(m_handle);
}/*}}}*/

void StatWatcher::start()
{/*{{{*/
    uv_fs_poll_start(m_handle, cb_func, m_path.c_str(), 3000);
}/*}}}*/

void StatWatcher::on_fs_poll_close_complete(uv_handle_t* handle)
{/*{{{*/
    delete (uv_fs_poll_t *)handle;
}/*}}}*/

void StatWatcher::close()
{/*{{{*/
    if (NULL == m_handle) return;
    uv_close((uv_handle_t *)m_handle, on_fs_poll_close_complete);
    while (uv_is_active((uv_handle_t *)m_handle)) { }
}/*}}}*/

} // namespace base
