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
#ifndef LOGKAFKA_ZOOKEEPER_H_ 
#define LOGKAFKA_ZOOKEEPER_H_

#include <iostream>
#include <string>
#include <vector>

#include "base/json.h"
#include "base/mutex.h"
#include "base/scoped_lock.h"
#include "base/timer_watcher.h"
#include "logkafka/common.h"

#include <zookeeper/zookeeper.h>

#include <uv.h>

using namespace std;
using namespace base;

namespace logkafka {

class Zookeeper
{
    public:
        Zookeeper();
        ~Zookeeper();

        bool init(const string &zookeeper_urls, 
                const string &kafka_chroot_path,
                const string &logkafka_id,
                long refresh_interval = REFRESH_INTERVAL_MS);

        string getBrokerUrls();
        string getLogConfig();

        bool setLogState(const char *buf, int buflen,
                stat_completion_t completion);
        void close();

    private:
        bool connect();
        void closeLoop();

        bool ensurePathExist(const string& path);
        bool getZnodeData(const string& path, string &data);
        bool getBrokerIds(vector<string>& ids);
        bool getBrokerIpAndPort(const string& brokerid, 
                string& host, string& port);

        static void refresh(void *arg);
        bool refreshConnection();
        bool refreshWatchers();
        bool refreshLogConfig();
        bool refreshBrokerUrls();

        bool setWatcher(const string& path, 
                watcher_fn watcher, void *wctx);
        bool setChildrenWatcher(const string& path, 
                watcher_fn watcher, void *wctx);

        static void globalWatcher(zhandle_t* zhandle, int type, 
                int state, const char* path, void* context);
        static void configChangeWatcher(zhandle_t* zhandle, int type, 
                int state, const char* path, void* context);
        static void brokerChangeWatcher(zhandle_t* zhandle, int type, 
                int state, const char* path, void* context);

        static const char* state2String(int state);
        static const char* event2String(int event);

        static void threadFunc(void *arg);
        static void exitAsyncCb(uv_async_t* handle);

    private:
        string m_zookeeper_urls;
        string m_kafka_chroot_path;
        string m_logkafka_id;
        string m_log_config;
        string m_broker_urls;
        string m_config_logkafka_id_path;
        string m_client_logkafka_id_path;
        string m_broker_ids_path;
        string m_logkafka_config_path;
        string m_logkafka_client_path;
        int m_session_timeout_ms;
        bool m_registered;

        zhandle_t *m_zhandle;
        clientid_t *m_clientid;
        FILE* m_zk_log_fp;
        uv_thread_t *m_thread;
        uv_loop_t *m_loop;
        TimerWatcher *m_refresh_timer_trigger;
        uv_async_t m_exit_handle;

        Mutex m_zhandle_mutex;
        Mutex m_log_config_mutex;
        Mutex m_broker_urls_mutex;

        static const unsigned long REFRESH_INTERVAL_MS;
        static const unsigned long SESSION_TIMEOUT_MS;
};

} // namespace Logkafka

#endif // LOGKAFKA_ZOOKEEPER_H_ 
