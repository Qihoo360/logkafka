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
#ifndef BASE_TIMER_WATCHER_H_
#define BASE_TIMER_WATCHER_H_

#include "base/common.h"

#include <uv.h>
#include "easylogging/easylogging++.h"

namespace base {

typedef void (*TimerFunc)(void *);

class TimerWatcher
{
    public:
        bool init(uv_loop_t *loop, 
                long timeout, 
                long repeat,
                void *event_cb_func_arg,
                TimerFunc event_cb_func);
        void stop();
        void start();
        void close();

    private:
        uv_loop_t *m_loop;
        uv_timer_t *m_handle;
        long m_timeout;
        long m_repeat;
        TimerFunc m_event_cb_func;
        void *m_event_cb_func_arg;

        static void cb_func(uv_timer_t *w);
        static void on_timer_close_complete(uv_handle_t* handle);
};

} // namespace base

#endif // BASE_TIMER_WATCHER_H_
