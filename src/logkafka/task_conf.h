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
#ifndef LOGKAFKA_TASK_CONF_H_
#define LOGKAFKA_TASK_CONF_H_

#include <ostream>
#include <queue>
#include <string>

#include "base/tools.h"
#include "logkafka/config.h"

#include "easylogging/easylogging++.h"

using namespace std;

namespace logkafka {

struct LogConf {
    /* log file path, example: /usr/local/apache2/logs/access_log.%Y%m%d */
    string log_path;

    /* if false, we will discard previous messages 
     * when logkafka goes up after it crash or stopped. 
     * Otherwise it will send all the messages */
    bool follow_last;

     /* max lines of messages to be sent */
    int batchsize;

    bool read_from_head;

    char line_delimiter;

    bool remove_delimiter;

    LogConf()
    {/*{{{*/
        log_path = "";
        follow_last = true;
        batchsize = 200;
        read_from_head = true;
        line_delimiter = '\n';
        remove_delimiter = true;
    }/*}}}*/

    bool operator==(const LogConf& hs) const
    {/*{{{*/
        return (log_path == hs.log_path) &&
            (follow_last == hs.follow_last) &&
            (batchsize == hs.batchsize) &&
            (line_delimiter == hs.line_delimiter) &&
            (remove_delimiter == hs.remove_delimiter);
    };/*}}}*/

    bool operator!=(const LogConf& hs) const
    {/*{{{*/
        return !operator==(hs);
    };/*}}}*/

    friend ostream& operator << (ostream& os, const LogConf& lc)
    {/*{{{*/
        os << "log path: " << lc.log_path
           << "follow last" << lc.follow_last
           << "batchsize" << lc.batchsize
           << "read from head" << lc.read_from_head
           << "line delimiter" << lc.line_delimiter
           << "remove delimiter" << lc.remove_delimiter;

        return os;
    }/*}}}*/

    bool isLegal()
    {/*{{{*/
        return isPathPatternLegal();
    }/*}}}*/
    
    bool isPathPatternLegal()
    {/*{{{*/
        std::size_t found = log_path.find("*");
        if (found != std::string::npos)
            return false;
            
        return true;
    }/*}}}*/
};

struct FilterConf {
    string regex_filter_pattern;

    FilterConf()
    {/*{{{*/
        regex_filter_pattern = "";
    }/*}}}*/

    bool operator==(const FilterConf& hs) const
    {/*{{{*/
        return (regex_filter_pattern == hs.regex_filter_pattern);
    };/*}}}*/

    bool operator!=(const FilterConf& hs) const
    {/*{{{*/
        return !operator==(hs);
    };/*}}}*/

    friend ostream& operator << (ostream& os, const FilterConf& fc)
    {/*{{{*/
        os << "regex filter pattern: " << fc.regex_filter_pattern;

        return os;
    }/*}}}*/
};

struct KafkaTopicConf {
    string brokers;
    string topic;
    string compression_codec;
    int required_acks;
    string key;
    int partition;
    int message_timeout_ms;
    
    KafkaTopicConf()
    {/*{{{*/
        brokers = "";
        topic = "";
        compression_codec = "none";
        required_acks = 1;
        key = "";
        partition = -1;
        message_timeout_ms = 0;
    }/*}}}*/

    bool operator==(const KafkaTopicConf& hs) const
    {/*{{{*/
        return (brokers == hs.brokers) &&
            (topic == hs.topic) &&
            (compression_codec == hs.compression_codec) &&
            (required_acks == hs.required_acks) &&
            (key == hs.key) &&
            (partition == hs.partition) && 
            (message_timeout_ms == hs.message_timeout_ms);
    };/*}}}*/

    bool operator!=(const KafkaTopicConf& hs) const
    {/*{{{*/
        return !operator==(hs);
    };/*}}}*/

    friend ostream& operator << (ostream& os, const KafkaTopicConf& ktc)
    {/*{{{*/
        return os;
    }/*}}}*/

    bool isLegal()
    {/*{{{*/
        return true;
    }/*}}}*/
};

struct TaskConf
{
    /* if false, the log file will not be collected */
    bool valid;

    LogConf log_conf;
    KafkaTopicConf kafka_topic_conf;
    FilterConf filter_conf;

    bool operator==(const TaskConf& hs) const
    {/*{{{*/
        return (valid == hs.valid) &&
            (log_conf == hs.log_conf) &&
            (kafka_topic_conf == hs.kafka_topic_conf) &&
            (filter_conf == hs.filter_conf);
    };/*}}}*/

    friend ostream& operator << (ostream& os, const TaskConf& tc)
    {/*{{{*/
        os << "valid: " << tc.valid
           << "log conf" << tc.log_conf 
           << "kafka topic conf" << tc.kafka_topic_conf
           << "filter conf" << tc.filter_conf;

        return os;
    }/*}}}*/

    bool isLegal()
    {/*{{{*/
        return log_conf.isLegal() && kafka_topic_conf.isLegal();
    }/*}}}*/
};

struct TaskStat
{
    bool first_update_paths;
    queue<string> paths;

    TaskStat()
    {/*{{{*/
        first_update_paths = true;
    }/*}}}*/

    string getPath()
    { /*{{{*/
        return !paths.empty()? paths.front(): "";
    };/*}}}*/
};

struct Task
{
    Task(unsigned long path_queue_max_size): path_queue_max_size(path_queue_max_size) {};

    unsigned long path_queue_max_size;
    TaskConf conf;
    TaskStat stat;

    string getPath() { return getFirstPath(); };
    string getFirstPath() { return stat.getPath(); };
    void delFirstPath() { stat.paths.pop(); };
    bool addPath(string path) 
    { /*{{{*/
        if (stat.paths.empty()) {
            stat.paths.push(path);
            LINFO << "Add path " << path;
            return true;
        } else {
            string last_path = stat.paths.back();
            if (path != last_path) {
                if (stat.paths.size() >= path_queue_max_size) {
                    LERROR << "Fail to add path " << path 
                           << ", path queue size >= " << path_queue_max_size;
                    return false;
                }
                stat.paths.push(path);
                LINFO << "Add path " << path;
                return true;
            }
        }

        return false;
    };/*}}}*/
    bool hasPath() { return !stat.paths.empty(); };
    bool getEnabled() { return conf.valid; };
    ino_t getInode() { return ::getInode(getPath().c_str()); };
};

} // namespace logkafka

#endif // LOGKAFKA_TASK_CONF_H_
