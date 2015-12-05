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
#include "logkafka/manager.h"

#include <unistd.h>

#include "base/json.h"
#include "base/tools.h"
#include "base/scoped_lock.h"

#include "easylogging/easylogging++.h"

using namespace std;
using namespace base;
using namespace rapidjson;

namespace logkafka {

Manager::Manager(const Config *config)
    : m_config(config)
{/*{{{*/
    m_pos_path = config->pos_path;
    m_refresh_interval = config->refresh_interval;
    m_line_max_bytes = config->line_max_bytes;
    m_read_max_bytes = config->read_max_bytes;
    m_stat_silent_max_ms = config->stat_silent_max_ms;

    m_refresh_trigger = NULL;
    m_loop = NULL;
    m_pos_file = NULL;
    m_position_file = NULL;
    m_zookeeper = NULL;
}/*}}}*/

Manager::~Manager()
{/*{{{*/
    delete m_zookeeper; m_zookeeper = NULL;
    delete m_refresh_trigger; m_refresh_trigger = NULL;
    delete m_position_file; m_position_file = NULL;

    {
        ScopedLock l(m_tail_watchers_mutex);
        TailVec::iterator iter = m_tails_deleted.begin();
        while (iter != m_tails_deleted.end()) {
            delete(*iter); *iter = NULL;
            m_tails_deleted.erase(iter);
        }
    }

    {
        ScopedLock l(m_tail_watchers_mutex);
        for (TailMap::iterator iter = m_tails.begin();
                iter != m_tails.end(); ++iter) {
            delete iter->second; iter->second = NULL;
        }

        for (TaskMap::iterator iter = m_tasks.begin();
                iter != m_tasks.end(); ++iter) {
            delete iter->second; iter->second = NULL;
        }
    }

    OutputKafka::stopProducers();
}/*}}}*/

bool Manager::init(uv_loop_t *loop)
{/*{{{*/
    m_loop = loop;

    initKafkaConf();
    initZookeeper();

    return true;
}/*}}}*/

bool Manager::initKafkaConf()
{/*{{{*/
    m_kafka_conf.message_max_bytes = m_config->line_max_bytes + m_config->key_max_bytes;
    m_kafka_conf.message_send_max_retries = m_config->message_send_max_retries;
    m_kafka_conf.queue_buffering_max_messages = m_config->queue_buffering_max_messages;

    return true;
}/*}}}*/

bool Manager::initZookeeper()
{/*{{{*/
    assert (NULL == m_zookeeper);
    m_zookeeper = new Zookeeper();

    if (!m_zookeeper->init(m_config->zookeeper_urls, m_config->kafka_chroot_path, m_config->logkafka_id)) {
        LERROR << "Fail to init zookeeper" 
               << ", zookeeper urls " << m_config->zookeeper_urls
               << ", kafka chroot path " << m_config->kafka_chroot_path;
        delete m_zookeeper; m_zookeeper = NULL;
        return false;
    }

    return true;
}/*}}}*/

bool Manager::refreshTaskConfs()
{/*{{{*/
    string config = m_zookeeper->getLogConfig();

    /* minimal config length should be greater than '{}' */
    if (config.length() <= 2) config = "{}";

    /* 1. Parse a JSON text string to a document. */
    Document document;
    if (document.Parse<0>(config.c_str()).HasParseError()) {
        LERROR << "Json parsing failed, json: " << config;
        return false;
    }

    m_task_confs.clear();

    /* 2. Access values in document. */
    if (!document.IsObject()) {
        LERROR << "Document is not object, type: "
            << Json::TypeNames[document.GetType()];
        return false;
    }

    /* Iterating object members */
    for (Value::ConstMemberIterator itr = document.MemberBegin();
            itr != document.MemberEnd(); ++itr) {
        if (!itr->name.IsString()) {
            LERROR << "Json error: name is not string";
            continue;
        }

        string path_pattern = (itr->name).GetString();

        TaskConf item;

        const Value &log_item = itr->value;
        /* topic must be acquired from json */
        try {
            string topic;
            Json::getValue(log_item, "topic", topic);
            item.kafka_topic_conf.topic = topic;
        } catch(const JsonErr &err) {
            LERROR << "Json error: " << err
                   << "Json string: " << Json::serialize(log_item);
            continue;
        } catch(...) { 
            LERROR << "Unknown exception";
            continue;
        }

        try {
            string valid;
            Json::getValue(log_item, "valid", valid);
            item.valid = str2Bool(valid);
        } catch(...) { /* default value */ }

        item.log_conf.log_path = itr->name.GetString();

        try {
            string follow_last;
            Json::getValue(log_item, "follow_last", follow_last);
            item.log_conf.follow_last = str2Bool(follow_last);
        } catch(...) { /* default value */ }

        try {
            string batchsize;
            Json::getValue(log_item, "batchsize", batchsize);
            item.log_conf.batchsize = atoi(batchsize.c_str());
            if (item.log_conf.batchsize > (long)m_config->queue_buffering_max_messages) {
                LWARNING << "The batch size " << item.log_conf.batchsize 
                         << " is larger than queue size "
                         << m_config->queue_buffering_max_messages
                         << ", path pattern " << path_pattern;
                item.log_conf.batchsize = m_config->queue_buffering_max_messages;
            }
        } catch(...) { /* default value */ }

        try {
            string line_delimiter;
            Json::getValue(log_item, "line_delimiter", line_delimiter);
            item.log_conf.line_delimiter = atoi(line_delimiter.c_str());
        } catch(...) { /* default value */ }

        try {
            string remove_delimiter;
            Json::getValue(log_item, "remove_delimiter", remove_delimiter);
            item.log_conf.remove_delimiter = str2Bool(remove_delimiter.c_str());
        } catch(...) { /* default value */ }

        try {
            string read_from_head;
            Json::getValue(log_item, "read_from_head", read_from_head);
            item.log_conf.read_from_head = str2Bool(read_from_head.c_str());
        } catch(...) { /* default value */ }

        try {
            string key;
            Json::getValue(log_item, "key", key);
            item.kafka_topic_conf.key = key;
        } catch(...) { /* default value */ }

        try {
            string partition;
            Json::getValue(log_item, "partition", partition);
            item.kafka_topic_conf.partition = atoi(partition.c_str());
        } catch(...) { /* default value */ }

        try {
            string compression_codec;
            Json::getValue(log_item, "compression_codec", compression_codec);
            item.kafka_topic_conf.compression_codec = compression_codec;
        } catch(...) { /* default value */ }

        try {
            string required_acks;
            Json::getValue(log_item, "required_acks", required_acks);
            item.kafka_topic_conf.required_acks = atoi(required_acks.c_str());
        } catch(...) { /* default value */ }

        try {
            string message_timeout_ms;
            Json::getValue(log_item, "message_timeout_ms", message_timeout_ms);
            item.kafka_topic_conf.message_timeout_ms = atoi(message_timeout_ms.c_str());
        } catch(...) { /* default value */ }

        try {
            string regex_filter_pattern;
            Json::getValue(log_item, "regex_filter_pattern", regex_filter_pattern);
            item.filter_conf.regex_filter_pattern = regex_filter_pattern;
        } catch(...) { /* default value */ }

        if (item.isLegal()) {
            m_task_confs[path_pattern] = item;
        }
    }

    return true;
}/*}}}*/

bool Manager::start()
{/*{{{*/
    if (NULL == (m_pos_file = fopen(m_pos_path.c_str(), "r+"))) {
        LWARNING << "Position file " << m_pos_path << " does not exists";
        if (NULL == (m_pos_file = fopen(m_pos_path.c_str(), "w+"))) {
            LERROR << "Fail to create position file " << m_pos_path;
            return false;
        }
        LINFO << "Create position file " << m_pos_path;
    };
    setvbuf(m_pos_file, reinterpret_cast<char *>(NULL), _IONBF, 0);
    m_position_file = PositionFile::parse(m_pos_file);

    refreshWatchers(this);

    m_refresh_trigger = new TimerWatcher();
    bool res = m_refresh_trigger->init(m_loop,
                        0,
                        m_refresh_interval,
                        this,
                        refreshWatchers);

    if (!res) { 
        LERROR << "Fail to init refresh watcher";
        delete m_refresh_trigger; m_refresh_trigger = NULL;
        return false;
    }

    return true;
}/*}}}*/

bool Manager::stop()
{/*{{{*/
    if (NULL != m_refresh_trigger) {
        m_refresh_trigger->stop();
    }

    ScopedLock l(m_tail_watchers_mutex);
    stopWatchers(getTailsKeys(m_tails), true, false);

    if (NULL != m_pos_file) {
        fclose(m_pos_file); m_pos_file = NULL;
    }

    if (NULL != m_zookeeper) {
        m_zookeeper->close();
    }

    return true;
}/*}}}*/

void Manager::refreshWatchers(void *arg)
{/*{{{*/
    Manager *manager = reinterpret_cast<Manager *>(arg);
    ScopedLock l(manager->m_tail_watchers_mutex);

    TaskMap &tasks = manager->m_tasks;
    TailMap &tails = manager->m_tails;

    if (!manager->refreshTaskConfs()) {
        LERROR << "Fail to get task config";
        return;
    }

    if (!manager->refreshTasks()) {
        LERROR << "Fail to refresh tasks";
        return;
    }
    
    /* get added tasks */
    set<string> added = diff_set(
            manager->getTasksKeys(tasks), 
            manager->getTailsKeys(tails));

    /* get deleted tasks */
    set<string> deleted = diff_set(
            manager->getTailsKeys(tails), 
            manager->getTasksKeys(tasks));

    /* get same tasks (path_pattern same, but conf may differ) */
    set<string> keeped = intersect_set(
            manager->getTailsKeys(tails), 
            manager->getTasksKeys(tasks));

    manager->stopWatchers(deleted, true, true);
    manager->startWatchers(added);
    manager->updateWatchers(keeped);
}/*}}}*/

bool Manager::refreshTasks()
{/*{{{*/
    set<string> task_confs_keys = 
        getTaskConfsKeys(m_task_confs);

    set<string> tasks_keys = 
        getTasksKeys(m_tasks);

    /* get added task confs */
    set<string> added = diff_set(task_confs_keys, tasks_keys);

    /* get deleted task confs */
    set<string> deleted = diff_set(tasks_keys, task_confs_keys);

    /* get same task confs */
    set<string> keeped = intersect_set(tasks_keys, task_confs_keys);

    deleteTasks(deleted);
    addTasks(added);
    updateTasks(keeped);

    /* delete old tail watchers */
    {
        ScopedLock l(m_tail_watchers_deleted_mutex); 
        TailVec::iterator iter = m_tails_deleted.begin();
        while (iter != m_tails_deleted.end()) {
            delete(*iter); *iter = NULL;
            m_tails_deleted.erase(iter);
        }
    }

    return true;
}/*}}}*/

void Manager::addTasks(set<string> path_patterns)
{/*{{{*/
    set<string>::const_iterator iter;
    for (iter = path_patterns.begin(); iter != path_patterns.end(); ++iter) {
        string path_pattern = *iter;
        Task *task = new Task(m_config->path_queue_max_size);
        task->conf = m_task_confs[path_pattern];

        if (m_tasks.find(path_pattern) != m_tasks.end()) {
            delete m_tasks[path_pattern];
        }
        m_tasks[path_pattern] = task;

        updateTaskPaths(path_pattern);

        LINFO << "Add task with path_pattern: " << path_pattern;
    }
}/*}}}*/

void Manager::deleteTasks(set<string> path_patterns)
{/*{{{*/
    set<string>::const_iterator iter;
    for (iter = path_patterns.begin(); iter != path_patterns.end(); ++iter) {
        string path_pattern = *iter;
        delete getTask(path_pattern);
        m_tasks.erase(path_pattern);

        LINFO << "Delete task with path_pattern: " << path_pattern;
    }
}/*}}}*/

void Manager::updateTasks(set<string> path_patterns)
{/*{{{*/
    set<string>::const_iterator iter; 
    for (iter = path_patterns.begin(); iter != path_patterns.end(); ++iter) {
        string path_pattern = *iter;
        m_tasks[path_pattern]->conf = m_task_confs[path_pattern];
        updateTaskPaths(path_pattern);

        LINFO << "Update task with path_pattern: " << path_pattern;
    }
}/*}}}*/

void Manager::updateTaskPaths(const string &path_pattern)
{/*{{{*/
    assert(m_tasks.find(path_pattern) != m_tasks.end());
    Task &task = *m_tasks[path_pattern];

    string expanded_path = expandPath(path_pattern);

    /* add paths in position file if this is first updating */
    if (task.stat.first_update_paths
            && expanded_path != path_pattern
            && task.conf.log_conf.follow_last)
    {
        string last_path;
        if (m_position_file->getPath(path_pattern, last_path)) {
            /* add last path in position file */
            LDEBUG << "Add last path " << last_path << " from position file";
            task.addPath(last_path);

            /* replace the time format substring in path_pattern with '*',
             * glob all files with this pattern, add the files which are 
             * newer than last_path and older than expanded_path */
            vector<string> path_pattern_globed;
            if (getPathPatternGlobed(path_pattern, path_pattern_globed)) {
                for (vector<string>::const_iterator iter = path_pattern_globed.begin();
                        iter != path_pattern_globed.end(); ++iter) 
                {
                    if (*iter > last_path && *iter < expanded_path) {
                        LDEBUG << "Add path " << *iter << " with glob";
                        task.addPath(*iter);
                    }
                }
            }
        }

        task.stat.first_update_paths = false;
    }

    /* remove expired paths */
    if (m_tails.find(path_pattern) != m_tails.end()) {
        TailWatcher &tail = *m_tails[path_pattern];
        while (task.hasPath()) {
            string first_path = task.getFirstPath();

            LDEBUG << "Expaned path" << expanded_path;
            LDEBUG << "Tail path " << tail.getPath();
            LDEBUG << "Task first path " << first_path;

            if (tail.getPath() == first_path) {
                if (!tail.isActive()) {
                    task.delFirstPath();
                    LINFO << "Delete inactive path " << first_path 
                          << " from task with path_pattern " << path_pattern;
                } else {
                    LINFO << "Path " << first_path  << " is active";
                    break;
                }
            } else {
                if (-1 == access(first_path.c_str(), R_OK)
                        && first_path < expanded_path) {
                    task.delFirstPath();
                    LINFO << "Delete expired path " << first_path 
                          << " from task with path_pattern " << path_pattern;
                } else {
                    LINFO << "Path " << first_path  << " exists, or not expired";
                    break;
                }
            }
        }
    } else {
        while (task.hasPath()) {
            string first_path = task.getFirstPath();
            if (-1 == access(first_path.c_str(), R_OK)
                    && first_path < expanded_path) {
                task.delFirstPath();
                LINFO << "Delete expired path " << first_path 
                      << " from task with path_pattern " << path_pattern;
            } else {
                break;
            }
        }
    }

    task.addPath(expanded_path);
}/*}}}*/

string Manager::expandPath(const string &path_pattern)
{/*{{{*/
    char outstr[PATH_MAX + 1] = {'\0'};

    time_t t = time(NULL);
    struct tm lt;
    struct tm *ltp = localtime_r(&t, &lt);
    if (ltp == NULL) {
        LERROR << "Fail to get localtime";
        return path_pattern;
    }

    if (0 == strftime(outstr, sizeof(outstr), path_pattern.c_str(), ltp)) {
        LERROR << "Fail to strftime with path_pattern: " << path_pattern;
        return path_pattern;
    }

    string p(outstr);

    return p;
}/*}}}*/

bool Manager::getPathPatternGlobed(const string &path_pattern, 
        vector<string> &path_pattern_globed)
{/*{{{*/
    string path_pattern_to_glob = path_pattern;
    while (true) {
        std::size_t start = path_pattern_to_glob.find_first_of('%', 0);
        if (start == string::npos) {
            break;
        }

        std::size_t i = start;
        while (i < path_pattern_to_glob.length() - 1) {
            char cp = path_pattern_to_glob[i];
            char cn = path_pattern_to_glob[i+1];
            if (cp == '%' && isTimeFormatConversionSpecifier(cn)) {
                i += 2;
            } else {
                break;
            }
        }

        if (i != start) {
            path_pattern_to_glob.erase(path_pattern_to_glob.begin() + start, 
                    path_pattern_to_glob.begin() + i);
            path_pattern_to_glob.insert(start, "*");
        }
    }

    return globPath(path_pattern_to_glob, path_pattern_globed);
}/*}}}*/

bool Manager::isTimeFormatConversionSpecifier(char c)
{/*{{{*/
    /* NOTE: we treat all character in [a-zA-Z+] as time format convertion 
     * specifier just for simplicity and efficiency. If accuracy is required
     * some day, for accuracy and efficiency, just use hash table to store
     * all real time format convertion specifiers (see 'man strftime').
     */
    if ((c >= 'a' && c <= 'z') 
            || (c >= 'A' && c <= 'Z') 
            || c == '+') {
        return true;
    }

    return false;
}/*}}}*/

TailWatcher* Manager::setupWatcher(
        TaskConf conf,
        string path_pattern, 
        string path,
        PositionEntry *position_entry,
        bool enabled)
{/*{{{*/
    LDEBUG << "Task conf" << conf.log_conf;

    OutputKafka *output = new OutputKafka();
    output->setKafkaConf(m_kafka_conf);
    if (!output->init(m_zookeeper, conf.kafka_topic_conf.compression_codec)) {
        LERROR << "Fail to init kafka output";
        delete output;
        return NULL;
    };

    output->setKafkaTopicConf(conf.kafka_topic_conf);

    // init tail watcher
    TailWatcher *tail_watcher = new TailWatcher();
    bool res = tail_watcher->init(m_loop, 
            path_pattern, 
            path, 
            position_entry,
            m_stat_silent_max_ms, 
            conf.log_conf.read_from_head,
            conf.log_conf.batchsize,
            m_line_max_bytes,
            m_read_max_bytes,
            conf.log_conf.line_delimiter,
            conf.log_conf.remove_delimiter,
            updateWatcherRotate, 
            receiveLines,
            conf,
            this,
            output);

    if (!res) {
        LERROR << "Fail to init tail watcher";
        delete tail_watcher; tail_watcher = NULL;
    }

    return tail_watcher;
}/*}}}*/

void Manager::startWatchers(set<string> paths)
{/*{{{*/
    set<string>::iterator iter;

    for (iter = paths.begin(); iter != paths.end(); ++iter) {
        string path_pattern = *iter;
        Task *task = m_tasks[path_pattern];

        if (!task->conf.valid) return;

        if (m_tails.find(path_pattern) != m_tails.end()) return;

        FilePositionEntry *pe = NULL;

        if (NULL != m_position_file) {
            PositionEntryKey pek = {path_pattern, task->getPath()};
            pe = (*m_position_file)[pek];
            if (task->conf.log_conf.read_from_head && pe->readInode() == INO_NONE) {
                pe->update(task->getInode(), 0);
            }
        }

        string path = task->getPath();
        bool enabled = task->getEnabled();
        TailWatcher *tw = setupWatcher(
                task->conf,
                path_pattern, 
                path,
                pe,
                enabled);
        if (NULL != tw) {
            m_tails[path_pattern] = tw;
        }
    }
}/*}}}*/

void Manager::stopWatchers(set<string> path_patterns, 
        bool immediate,
        bool unwatched)
{/*{{{*/
    for (set<string>::iterator iter = path_patterns.begin(); 
            iter != path_patterns.end(); ++iter) {
        string path_pattern = *iter;

        TailMap::iterator it_st;
        it_st = m_tails.find(path_pattern);

        if (it_st != m_tails.end()) {
            TailWatcher *tw = it_st->second;
            tw->m_unwatched = unwatched;

            if (immediate) {
                closeWatcher(tw, true, true);
                delete tw; it_st->second = NULL;
                m_tails.erase(it_st);
            }
        }
    }
}/*}}}*/

void Manager::closeWatcher(TailWatcher *tw, 
        bool close_io, 
        bool remove_pos_entry)
{/*{{{*/
    tw->stop(close_io);
    flushBuffer(tw);

    PositionEntryKey pek = {tw->m_path_pattern, tw->getPath()};
    if (tw->m_unwatched && NULL != m_position_file) {
        (*m_position_file)[pek]->updatePos(PositionFile::UNWATCHED_POSITION);
    }

    if (remove_pos_entry)
        m_position_file->remove(pek);
}/*}}}*/

void Manager::flushBuffer(TailWatcher *tw)
{/*{{{*/
}/*}}}*/

void Manager::updateWatchers(set<string> path_patterns)
{/*{{{*/
    set<string>::iterator it_s;

    for (it_s = path_patterns.begin(); it_s != path_patterns.end(); ++it_s) {
        string path_pattern = *it_s;
        Task *task = m_tasks[path_pattern];
        TailWatcher *tail = m_tails[path_pattern];

        if (task->conf == tail->m_conf) {
            if (task->getPath() != tail->getPath()) {
                LINFO << "Update tail watcher with path_pattern " << path_pattern
                      << ", change path from " << tail->getPath()
                      << " to " << task->getPath();
                set<string> paths;
                paths.insert(path_pattern);
                stopWatchers(paths, true, true);
                startWatchers(paths);
            }

            continue;
        }

        if (task->conf.log_conf != tail->m_conf.log_conf 
                || task->conf.kafka_topic_conf != tail->m_conf.kafka_topic_conf
                || task->conf.filter_conf != tail->m_conf.filter_conf)
        {
            closeWatcher(tail, true, false);
            PositionEntryKey pek = {path_pattern, tail->getPath()};
            m_tails[path_pattern] = setupWatcher(
                    task->conf, 
                    path_pattern, 
                    task->getPath(), 
                    (*m_position_file)[pek],
                    task->getEnabled());
        }

        if (task->getEnabled() && !tail->getEnabled()) {
            tail->start();
            tail->setEnabled(true);
        }

        if (!task->getEnabled() && tail->getEnabled()) {
            tail->stop(false);
            tail->setEnabled(false);
        }
    }
}/*}}}*/

bool Manager::updateWatcherRotate(Manager *manager,
        string path_pattern,
        string path,
        PositionEntry *position_entry)
{/*{{{*/
    LINFO << "Update watcher rotate"
        << ", path_pattern " << path_pattern
        << ", path " << path;
    ScopedLock l(manager->m_tail_watchers_mutex);

    TailWatcher *tw = NULL;
    TailMap::iterator iter 
        = (manager->m_tails).find(path);
    if (iter != manager->m_tails.end())
        tw = iter->second;

    if (tw->isActive()) {
        LINFO << "Original file is still active, give up rotating this time";
        return false;
    }

    manager->closeWatcher(tw, true, false); 
    position_entry->updatePos(0); // read from head

    TailWatcher *tw_new = manager->setupWatcher(
            tw->m_conf,
            path_pattern, 
            path, 
            position_entry, 
            manager->m_tasks[path_pattern]->getEnabled());
    if (NULL == tw_new) {
        LERROR << "Fail to set up new tail watcher, "
              << ", path_pattern: " << path_pattern
              << ", path " << path;
        return false;
    } else {
        manager->m_tails[path_pattern] = tw_new;
        LINFO << "Finish setting up new tail watcher";

        {
            /* Save tail watcher to be deleted */
            ScopedLock l(manager->m_tail_watchers_deleted_mutex); 
            manager->m_tails_deleted.push_back(tw);
        }
    }

    return true;
}/*}}}*/

bool Manager::receiveLines(void *filter, void *output, const vector<string> &lines)
{/*{{{*/
    if (NULL == output) {
        LERROR << "output function is NULL";
        return false;
    }

    Filter *flt = reinterpret_cast<Filter *>(filter);
    vector<string> valid_lines = lines;
    if (NULL != flt) {
        flt->filter(flt, valid_lines);
    }

    if (valid_lines.empty()) {
        LINFO << "lines is empty";
        return true;
    }

    Output *out = reinterpret_cast<Output *>(output);
    return out->output(out, valid_lines);
}/*}}}*/

void Manager::uploadCollectingState(void *arg)
{/*{{{*/
    if (NULL == arg) {
        LERROR << "Update collecting state arg is NULL";
        return;
    }

    Manager *manager = reinterpret_cast<Manager*>(arg);
    
    Zookeeper *zookeeper = manager->m_zookeeper;
    if (NULL == manager->m_zookeeper) {
        LERROR << "Zookeeper is NULL";
        return;
    }

    string info  = manager->getCollectingState();
    LDEBUG << "Get tail watcher info json string: " << info;

    if (!zookeeper->setLogState(
                       info.c_str(), 
                       info.length(), 
                       &onZookeeperSetComplete))
    {
        LERROR << "Fail to set collecting state " << info;
    }
}/*}}}*/

string Manager::getCollectingState()
{/*{{{*/
    string filename;
    string info;

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    writer.StartObject();

    ScopedLock l(m_tail_watchers_mutex);
    for (TailMap::iterator iter = m_tails.begin(); 
            iter != m_tails.end(); ++iter) {
        string path_pattern = iter->first;
        TailWatcher *tw = iter->second;

        if (NULL == tw) continue;

        if ("" != path_pattern) {
            writer.String(path_pattern.c_str());
            tw->Serialize(writer);
        }
    }

    writer.EndObject();

    info = sb.GetString();

    return info;
}/*}}}*/

void Manager::onZookeeperSetComplete(int rc, const struct Stat *stat, const void *data)
{/*{{{*/
    if (NULL == data) {
        LERROR << "Zookeeper set complete data is NULL";
        return;
    }

    const char *path = reinterpret_cast<const char *>(data);

    if (0 != rc) {
        LERROR << "Fail to set zk path " << path << ", " << zerror(rc);
    }

    free((char*)path);
}/*}}}*/

set<string> Manager::getTasksKeys(const TaskMap &tasks)
{/*{{{*/
    std::set<std::string> s;
    std::transform(tasks.begin(), tasks.end(), 
            std::inserter(s, s.begin()), GetKey<TaskMap::value_type>());

    return s;
}/*}}}*/

set<string> Manager::getTailsKeys(const TailMap &tails)
{/*{{{*/
    std::set<std::string> s;
    std::transform(tails.begin(), tails.end(), 
            std::inserter(s, s.begin()), GetKey<TailMap::value_type>());

    return s;
}/*}}}*/

set<string> Manager::getTaskConfsKeys(const TaskConfMap &task_confs)
{/*{{{*/
    std::set<std::string> s;
    std::transform(task_confs.begin(), task_confs.end(),
            std::inserter(s, s.begin()), GetKey<TaskConfMap::value_type>());

    return s;
}/*}}}*/

TailWatcher *Manager::getTailWatcher(string path_pattern)
{/*{{{*/
    TailWatcher *p = NULL;

    TailMap::const_iterator iter = m_tails.find(path_pattern);

    if (iter != m_tails.end())
        p = iter->second;

    return p;
}/*}}}*/

Task *Manager::getTask(string path_pattern)
{/*{{{*/
    Task *p = NULL;

    TaskMap::const_iterator iter = m_tasks.find(path_pattern);

    if (iter != m_tasks.end())
        p = iter->second;

    return p;
}/*}}}*/

} // namespace logkafka
