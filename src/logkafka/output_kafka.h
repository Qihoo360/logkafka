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
#ifndef LOGKAFKA_OUTPUT_KAFKA_H_
#define LOGKAFKA_OUTPUT_KAFKA_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <string>
#include <vector>

#include "base/common.h"
#include "logkafka/output.h"
#include "logkafka/producer.h"
#include "logkafka/task_conf.h"

using namespace std;
using namespace base;

namespace logkafka {

class OutputKafka: public virtual Output
{
    public:
        OutputKafka(): Output() {};
        virtual ~OutputKafka() {};
        bool init(void *arg) { return true; };

        /* NOTE: not thread-safe */
        bool init(void *arg, string compression_codec);
        bool output(void *arg, const vector<string> &lines);
        bool setKafkaTopicConf(KafkaTopicConf kafka_topic_conf);

        /* NOTE: not thread-safe */
        static bool initProducer(void *arg, string compression_codec);

        static bool stopProducers();
        static bool setKafkaConf(KafkaConf kafka_conf) { 
            m_kafka_conf = kafka_conf;
            return true;
        };

    private:
        static map< string, Producer *> m_producer_map;
        KafkaTopicConf m_kafka_topic_conf;
        static KafkaConf m_kafka_conf;
};

} // namespace logkafka

#endif // LOGKAFKA_OUTPUT_KAFKA_H_
