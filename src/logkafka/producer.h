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
#ifndef LOGKAFKA_PRODUCER_H_
#define LOGKAFKA_PRODUCER_H_

#include <inttypes.h>
#include <sys/types.h>

#include <map>
#include <string>
#include <vector>

#include "logkafka/zookeeper.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <librdkafka/rdkafka.h>  /* for Kafka driver */
#ifdef __cplusplus
}
#endif

using namespace std;

namespace logkafka {

struct KafkaConf {
    long long message_max_bytes;
    long long message_send_max_retries; 
    long long queue_buffering_max_messages;
};

class Producer 
{
    public:
        Producer();
        ~Producer();

        bool init(Zookeeper& zookeeper, 
                const string &compression_codec,
                const KafkaConf &kafka_conf);
        void close();

        bool send(const vector<string> &messages,
                const string &brokers, 
                const string &topic, 
                const string &key, 
                int required_acks,
                int partition,
                int message_timeout_ms);

    public:
        static const map<string, int> cc_map;

    private:
        static map<string, int> createCompressionCodecMap();
        static void rdkafkaLogger(const rd_kafka_t *rk,
                int level, const char *fac, const char *buf);
        static void msgDelivered2(rd_kafka_t *rk,
                const rd_kafka_message_t *rkmessage, 
                void *opaque);
        static void msgDelivered(rd_kafka_t *rk, 
                void *payload, 
                size_t len,
                rd_kafka_resp_err_t err,
                void *opaque,
                void *msg_opaque);

    private:
        rd_kafka_conf_t *m_conf;
        rd_kafka_t *m_rk;
        string m_brokers;
        string m_compression_codec;
};

} // namespace logkafka

#endif // LOGKAFKA_PRODUCER_H_
