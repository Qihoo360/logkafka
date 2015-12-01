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
#ifndef LOGKAFKA_MANAGER_H_
#define LOGKAFKA_MANAGER_H_

#include <map>

#include "base/common.h"
#include "logkafka/config.h"
#include "logkafka/output_kafka.h"
#include "logkafka/position_file.h"
#include "logkafka/producer.h"
#include "logkafka/signal_handler.h"
#include "logkafka/tail_watcher.h"
#include "logkafka/task_conf.h"
#include "logkafka/zookeeper.h"

#include <uv.h>

using namespace std;
using namespace base;

namespace logkafka {

class TailWatcher;

typedef std::map<std::string, Task* > TaskMap;
typedef std::map<std::string, TailWatcher*> TailMap;
typedef std::map<std::string, TaskConf> TaskConfMap;
typedef std::vector<TailWatcher*> TailVec;

class Manager
{
    public:
        Manager() {};
        Manager(const Config *config);
        ~Manager();

        bool init(uv_loop_t *loop);
        bool start();
        bool stop(); 

        static void uploadCollectingState(void *arg);

    public:
        Zookeeper *m_zookeeper;

    private:
        bool initZookeeper();

        /* kafka global conf relevant functions */
        bool initKafkaConf();

        /* task confs relevant functions */
        bool refreshTaskConfs();

        /* tasks relevant functions */
        bool refreshTasks();
        void updateTaskPaths(const string &path_pattern);
        void updateTasks(set<string> path_patterns);
        void deleteTasks(set<string> path_patterns);
        void addTasks(set<string> path_patterns);

        string expandPath(const string &path_pattern);
        bool getPathPatternGlobed(const string &path_pattern, 
                vector<string> &path_pattern_globed);
        inline bool isTimeFormatConversionSpecifier(char c);

        /* tail watchers relevant functions */
        static void refreshWatchers(void *arg);
        void startWatchers(set<string> added);
        TailWatcher* setupWatcher(
                TaskConf conf,
                string path_pattern, 
                string path,
                PositionEntry *position_entry,
                bool enabled = true);
        void updateWatchers(set<string> path_patterns);
        void updateWatcher(Manager *manager,
                string path_pattern,
                string path, 
                PositionEntry *position_entry);
        void stopWatchers(set<string> removed, 
                bool immediate = false,
                bool unwatched = false);
        void closeWatcher(TailWatcher *tw, 
                bool close_io = true, 
                bool remove_pos_entry = false);
        static bool updateWatcherRotate(Manager *manager,
                string path_pattern,
                string path,
                PositionEntry *position_entry);
        void flushBuffer(TailWatcher *tw);
        static bool receiveLines(void *filter, void *output, const vector<string> &lines);

        set<string> getTasksKeys(const TaskMap &tasks);
        set<string> getTailsKeys(const TailMap &tails);
        set<string> getTaskConfsKeys(const TaskConfMap &task_confs);
        TailWatcher *getTailWatcher(string path_pattern);
        Task *getTask(string path_pattern);

        string getCollectingState();
        static void onZookeeperSetComplete(int rc, const struct Stat *stat, const void *data);

    private:
        unsigned long m_refresh_interval;
        unsigned long m_line_max_bytes;
        unsigned long m_read_max_bytes;
        unsigned long m_stat_silent_max_ms;
        string m_pos_path;
        uv_loop_t *m_loop;
        const Config *m_config;

        KafkaConf m_kafka_conf;
        TaskConfMap m_task_confs;
        TaskMap m_tasks;
        TailMap m_tails;
        TailVec m_tails_deleted;

        TimerWatcher *m_refresh_trigger;

        FILE* m_pos_file;
        PositionFile *m_position_file;

        Mutex m_tail_watchers_mutex;
        Mutex m_tail_watchers_deleted_mutex;
};

} // namespace logkafka

#endif // LOGKAFKA_MANAGER_H_
