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
#include "logkafka/producer.h"

#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <sys/time.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>

#include "base/tools.h"

#include "easylogging/easylogging++.h"

using namespace std;
using namespace base;

namespace logkafka {

const map<string, int> Producer::cc_map = Producer::createCompressionCodecMap();

Producer::Producer()
{/*{{{*/
    m_compression_codec = "";
    m_conf = NULL;
    m_rk = NULL;
}/*}}}*/

Producer::~Producer()
{/*{{{*/
}/*}}}*/

/**
 * Kafka logger callback (optional)
 */
void Producer::rdkafkaLogger(const rd_kafka_t *rk, 
        int level, const char *fac, const char *buf) 
{/*{{{*/
    LWARNING << "RDKAFKA: " << fac << ": " << rd_kafka_name(rk) << ": " << buf;
}/*}}}*/

bool Producer::init(Zookeeper& zookeeper, 
    const string &compression_codec,
    const KafkaConf &kafka_conf)
{/*{{{*/
    char errstr[512];

    /* Get brokers */
    m_brokers = zookeeper.getBrokerUrls();

    /* Kafka configuration */
    m_conf = rd_kafka_conf_new();

    if (rd_kafka_conf_set(m_conf, "compression.codec",
                compression_codec.c_str(),
                errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        LERROR << "Fail to set kafka conf, " << errstr;
        return false;
    }

    if (rd_kafka_conf_set(m_conf, "message.max.bytes",
                int2Str(kafka_conf.message_max_bytes).c_str(),
                errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        LERROR << "Fail to set kafka conf, " << errstr;
        return false;
    }

    if (rd_kafka_conf_set(m_conf, "message.send.max.retries",
                int2Str(kafka_conf.message_send_max_retries).c_str(),
                errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        LERROR << "Fail to set kafka conf, " << errstr;
        return false;
    }

    if (rd_kafka_conf_set(m_conf, "queue.buffering.max.messages",
                int2Str(kafka_conf.queue_buffering_max_messages).c_str(),
                errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        LERROR << "Fail to set kafka conf, " << errstr;
        return false;
    }

    /* If offset reporting (-o report) is enabled, use the
     * richer dr_msg_cb instead. */
    bool report_offsets = false;
    if (report_offsets) {
        rd_kafka_conf_set_dr_msg_cb(m_conf, msgDelivered2);
    } else {
        rd_kafka_conf_set_dr_cb(m_conf, msgDelivered);
    }

    /* Create Kafka handle */
    if (!(m_rk = rd_kafka_new(RD_KAFKA_PRODUCER, m_conf,
                    errstr, sizeof(errstr)))) {
        LERROR << "Failed to create new producer, " << errstr;
        return false;
    }

    /* Set logger */
    rd_kafka_set_logger(m_rk, rdkafkaLogger);
    rd_kafka_set_log_level(m_rk, LOG_WARNING);

    /* Add brokers */
    if (rd_kafka_brokers_add(m_rk, m_brokers.c_str()) == 0) {
        LERROR << "No valid brokers specified";
        return false;
    }

    return true;
}/*}}}*/

void Producer::close()
{/*{{{*/
    /* Wait for messages to be delivered */
    while (rd_kafka_outq_len(m_rk) > 0)
        rd_kafka_poll(m_rk, 100);

    if (NULL != m_rk) {
        LINFO << "Destroying kafka instance: " << rd_kafka_name(m_rk);
        rd_kafka_destroy(m_rk);
        m_rk = NULL;
    }
}/*}}}*/

bool Producer::send(const vector<string> &messages,
        const string &brokers, 
        const string &topic, 
        const string &key, 
        int required_acks,
        int partition,
        int message_timeout_ms) 
{/*{{{*/
    bool ret = true;
    long r;
    rd_kafka_topic_t *rkt;
    rd_kafka_topic_conf_t *topic_conf;
    long msgcnt = messages.size();
    long failcnt = 0;
    long i;
    rd_kafka_message_t *rkmessages;

    char errstr[512];

    /* Topic configuration */
    topic_conf = rd_kafka_topic_conf_new();
    rd_kafka_topic_conf_set(topic_conf,
        "produce.offset.report",
        "true", errstr, sizeof(errstr));
    rd_kafka_topic_conf_set(topic_conf,
        "message.timeout.ms",
        int2Str(message_timeout_ms).c_str(), errstr, sizeof(errstr));

    /* Create topic */
    rkt = rd_kafka_topic_new(m_rk, topic.c_str(), topic_conf);
    if (!rkt) {
        LERROR << "Failed to create topic: " << strerror(errno);
    }
    
    /* Create messages */
    rkmessages = (rd_kafka_message_t*)calloc(sizeof(*rkmessages), msgcnt);
    for (i = 0 ; i < msgcnt ; ++i) {
        long *msgidp = (long *)malloc(sizeof(*msgidp));
        *msgidp = i;

        rkmessages[i].len     = messages[i].length();
        rkmessages[i].payload = strndup(messages[i].c_str(), rkmessages[i].len);
        rkmessages[i].key_len = key.length();
        rkmessages[i].key     = strndup(key.c_str(), rkmessages[i].key_len);
        rkmessages[i]._private = msgidp;
    }

    r = rd_kafka_produce_batch(rkt, partition, RD_KAFKA_MSG_F_FREE,
            rkmessages, msgcnt);

    /* Scan through messages to check for errors. */
    for (i = 0 ; i < msgcnt ; ++i) {
        if (rkmessages[i].err) {
            ++failcnt;
            if (failcnt < 100) {
                LERROR << "Message #" << i 
                       << " failed: " << rd_kafka_err2str(rkmessages[i].err);
                ret = false;
            }
        }
    }

    /* All messages should've been produced. */
    if (r < msgcnt) {
        LERROR << "Not all messages were accepted "
                  "by produce_batch(): " << r << " < " << msgcnt;
        if (msgcnt - r != failcnt)
            LERROR << "Discrepency between failed messages"
                   << " (" << failcnt << ")"
                   << " and return value " << (msgcnt - r) 
                   << " (" << msgcnt<< " - " << r << ")";

        LERROR << (msgcnt -r) << "/" << msgcnt << " messages failed";
        ret = false;
    }

    rd_kafka_poll(m_rk, 0);

    /* Note: librdkafka will duplicate the key once more, 
     * so we can free the original one after producing*/
    for (i = 0 ; i < msgcnt ; ++i) {
        free(rkmessages[i].key);
    }

    free(rkmessages);
    LINFO << "Partitioner: Produced "<< r << " messages, waiting for deliveries";

    /* Destroy topic */
    rd_kafka_topic_destroy(rkt);

    return ret;
}/*}}}*/

/**
 * Message delivery report callback using the richer rd_kafka_message_t object.
 */
void Producer::msgDelivered2(rd_kafka_t *rk,
        const rd_kafka_message_t *rkmessage, 
        void *opaque) 
{/*{{{*/
    bool quiet = true;
    if (rkmessage->err) {
        LERROR << "Message delivery failed: "
            << rd_kafka_message_errstr(rkmessage);
    } else if (!quiet) {
        LINFO << "Message delivered (" 
            << rkmessage->len << " bytes"
            << ", offset " << rkmessage->offset
            << ", partition " << rkmessage->partition;
    }
}/*}}}*/

/**
 * Delivery reported callback.
 * Called for each message once to signal its delivery status.
 */
void Producer::msgDelivered(rd_kafka_t *rk,
        void *payload, 
        size_t len,
        rd_kafka_resp_err_t err,
        void *opaque, 
        void *msg_opaque) 
{/*{{{*/
    free(msg_opaque);

    if (err != RD_KAFKA_RESP_ERR_NO_ERROR)
        LERROR << "Message delivery failed: "<< rd_kafka_err2str(err);
}/*}}}*/

map<string, int> Producer::createCompressionCodecMap()
{/*{{{*/
    map<string, int> cc_map;
    cc_map["none"] = 0;
    cc_map["gzip"] = 1;
    cc_map["snappy"] = 2;

    return cc_map;
}/*}}}*/

} // namespace logkafka
