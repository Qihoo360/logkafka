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
#ifndef LOGKAFKA_H_
#define LOGKAFKA_H_

#include "base/timer_watcher.h"
#include "logkafka/config.h"
#include "logkafka/manager.h"
#include "logkafka/signal_handler.h"

using namespace std;
using namespace base;

namespace logkafka {

class LogKafka
{
    public:
        LogKafka() {};
        explicit LogKafka(const Config *config);
        ~LogKafka();

        bool init();
        void close();
        bool start();
        void stop();

        static void onSignal(void *arg);
        static void exitAsyncCb(uv_async_t* handle);

    private:
        Manager *m_manager;
        uv_loop_t *m_loop;
        SignalHandler *m_signal_handler;
        TimerWatcher *m_upload_timer_trigger;

        const Config *m_config;
        uv_async_t m_exit_handle;
};

} // namespace logkafka

#endif // LOGKAFKA_H_
