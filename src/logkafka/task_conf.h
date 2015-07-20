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

#include "easylogging/easylogging++.h"

using namespace std;

namespace logkafka {

struct LogConf{
    /* log file path, example: /usr/local/apache2/logs/access_log.%Y%m%d */
    string log_path;

    /* if false, we will discard previous messages 
     * when logkafka goes up after it crash or stopped. 
     * Otherwise it will send all the messages */
    bool follow_last;

     /* max lines of messages to be sent */
    int batchsize;

    bool read_from_head;

    LogConf()
    {/*{{{*/
        log_path = "";
        follow_last = false;
        batchsize = 200;
        read_from_head = true;
    }/*}}}*/

    bool operator==(const LogConf& hs) const
    {/*{{{*/
        return (log_path == hs.log_path) &&
            (follow_last == hs.follow_last) &&
            (batchsize == hs.batchsize);
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
           << "read from head" << lc.read_from_head;

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

    bool operator==(const TaskConf& hs) const
    {/*{{{*/
        return (valid == hs.valid) &&
            (log_conf == hs.log_conf) &&
            (kafka_topic_conf == hs.kafka_topic_conf);
    };/*}}}*/

    friend ostream& operator << (ostream& os, const TaskConf& tc)
    {/*{{{*/
        os << "valid: " << tc.valid
           << "log conf" << tc.log_conf 
           << "kafka topic conf" << tc.kafka_topic_conf;

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
