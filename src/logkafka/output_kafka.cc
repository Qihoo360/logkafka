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
#include "logkafka/output_kafka.h"

namespace logkafka {

map< string, Producer *> OutputKafka::m_producer_map;
KafkaConf OutputKafka::m_kafka_conf;

bool OutputKafka::output(void *arg, const vector<string> &lines)
{/*{{{*/
    OutputKafka *ok = reinterpret_cast<OutputKafka *>(arg);
    KafkaTopicConf kafka_topic_conf = ok->m_kafka_topic_conf;
    Producer *producer = ok->m_producer_map[kafka_topic_conf.compression_codec];

    return producer->send(lines,
                "", 
                kafka_topic_conf.topic, 
                kafka_topic_conf.key, 
                kafka_topic_conf.required_acks,
                kafka_topic_conf.partition,
                kafka_topic_conf.message_timeout_ms);
}/*}}}*/

bool OutputKafka::init(void *arg, string compression_codec)
{/*{{{*/
    return OutputKafka::initProducer(arg, compression_codec);
}/*}}}*/

bool OutputKafka::initProducer(void *arg, string compression_codec)
{/*{{{*/
    Zookeeper *zookeeper = reinterpret_cast<Zookeeper *>(arg);

    map<string, int>::const_iterator iter 
        = Producer::cc_map.find(compression_codec);
    if (iter == Producer::cc_map.end())
        return false;

    if (NULL == m_producer_map[iter->first]) {
        LINFO << "Try to init producer, compression_codec is "
              << iter->first;
        Producer *producer = new Producer();
        if (!producer->init(*zookeeper, iter->first, m_kafka_conf))
        {
            LERROR << "Fail to init producer, compression_codec is "
                   << iter->first;
            delete producer;
            return false;
        }
        m_producer_map[iter->first] = producer;
    }

    return true;
}/*}}}*/

bool OutputKafka::setKafkaTopicConf(KafkaTopicConf kafka_topic_conf)
{/*{{{*/
    m_kafka_topic_conf = kafka_topic_conf;
    return true;
}/*}}}*/

bool OutputKafka::stopProducers()
{/*{{{*/
    map<string, int>::const_iterator iter;
    for (iter = Producer::cc_map.begin(); iter != Producer::cc_map.end(); ++iter) {
        if (NULL != m_producer_map[iter->first]) {
            m_producer_map[iter->first]->close();
            delete m_producer_map[iter->first];
            m_producer_map[iter->first] = NULL;
        }
    }

    return true;
}/*}}}*/

} // namespace logkafka
