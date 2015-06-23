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
#ifndef BASE_STAT_WATCHER_H_
#define BASE_STAT_WATCHER_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string>

#include "base/common.h"
#include "base/mutex.h"
#include "base/scoped_lock.h"

#include "easylogging/easylogging++.h"
#include <uv.h>

using namespace std;
using namespace base;

namespace base {

typedef void (*StatFunc)(void *);

class StatWatcher
{
    public:
        bool init(uv_loop_t *loop, 
                string path,
                long interval,
                void *event_cb_func_arg,
                StatFunc event_cb_func);
        void start();
        void stop();
        void close();
    private:
        string m_path;
        uv_loop_t *m_loop;
        uv_fs_poll_t *m_handle;
        StatFunc m_event_cb_func;
        void *m_event_cb_func_arg;

        static void cb_func(uv_fs_poll_t* handle, 
                int status, 
                const uv_stat_t* prev, 
                const uv_stat_t* curr);
        static void on_fs_poll_close_complete(uv_handle_t* handle);
};

} // namespace base

#endif // BASE_STAT_WATCHER_H_
