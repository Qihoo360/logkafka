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
#include "logkafka/config.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>

#include "base/tools.h"
#include "logkafka/tail_watcher.h"

using namespace std;

namespace logkafka {

Config::Config()
{/*{{{*/
    cfg_opt_t opts[] =
    {
        CFG_STR("zookeeper.connect", DEFAULT_ZOOKEEPER_CONNECT, CFGF_NONE),
        CFG_STR("pos.path", DEFAULT_POS_PATH, CFGF_NONE),
        CFG_STR("logkafka.id", DEFAULT_LOGKAFKA_ID, CFGF_NONE),
        CFG_INT("line.max.bytes", DEFAULT_LINE_MAX_BYTES, CFGF_NONE),
        CFG_INT("read.max.bytes", DEFAULT_READ_MAX_BYTES, CFGF_NONE),
        CFG_INT("key.max.bytes", DEFAULT_KEY_MAX_BYTES, CFGF_NONE),
        CFG_INT("stat.silent.max.ms", DEFAULT_STAT_SILENT_MAX_MS, CFGF_NONE),
        CFG_INT("zookeeper.upload.interval", DEFAULT_ZOOKEEPER_UPLOAD_INTERVAL,
                CFGF_NONE),
        CFG_INT("refresh.interval", DEFAULT_REFRESH_INTERVAL, CFGF_NONE),
        CFG_INT("path.queue.max.size", DEFAULT_PATH_QUEUE_MAX_SIZE, CFGF_NONE),
        CFG_INT("message.send.max.retries", DEFAULT_MESSAGE_SEND_MAX_RETRIES,
                CFGF_NONE),
        CFG_INT("queue.buffering.max.messages", DEFAULT_QUEUE_BUFFERING_MAX_MESSAGES,
                CFGF_NONE),
        CFG_END()
    };

    m_cfg = cfg_init(opts, CFGF_NONE);
}/*}}}*/

Config::~Config()
{/*{{{*/
    cfg_free(m_cfg);
}/*}}}*/

bool Config::init(const char* filepath)
{/*{{{*/
    int ret = cfg_parse(m_cfg, filepath);
    switch (ret) {
        case CFG_PARSE_ERROR:
            fprintf(stderr, "Config file parsing error!\n");
            return false;
        case CFG_FILE_ERROR:
            fprintf(stderr, "Config file error!\n");
            return false;
        default:
            break;
    };

    char realdir[PATH_MAX + 1] = {'\0'};
    if (NULL == realDir(filepath, realdir)) {
        fprintf(stderr, "Get realdir of filepath(%s) error!\n", filepath);
        return false;
    }
    string realdir_s(realdir);
        
    LINFO << "Get logkafka configs: ";

    zookeeper_connect = cfg_getstr(m_cfg, "zookeeper.connect"); 
    PRINT_VAR(zookeeper_connect);
    pos_path = cfg_getstr(m_cfg, "pos.path"); 
    PRINT_VAR(pos_path);
    logkafka_id = cfg_getstr(m_cfg, "logkafka.id"); 
    if (logkafka_id == "") logkafka_id = getHostname();
    PRINT_VAR(logkafka_id);
    line_max_bytes = cfg_getint(m_cfg, "line.max.bytes"); 
    PRINT_VAR(line_max_bytes);
    read_max_bytes = cfg_getint(m_cfg, "read.max.bytes");
    PRINT_VAR(read_max_bytes);
    key_max_bytes = cfg_getint(m_cfg, "key.max.bytes");
    PRINT_VAR(key_max_bytes);
    stat_silent_max_ms = cfg_getint(m_cfg, "stat.silent.max.ms"); 
    PRINT_VAR(stat_silent_max_ms);
    zookeeper_upload_interval = cfg_getint(m_cfg, "zookeeper.upload.interval"); 
    PRINT_VAR(zookeeper_upload_interval);
    refresh_interval = cfg_getint(m_cfg, "refresh.interval");
    PRINT_VAR(refresh_interval);
    path_queue_max_size = cfg_getint(m_cfg, "path.queue.max.size");
    PRINT_VAR(path_queue_max_size);
    message_send_max_retries = cfg_getint(m_cfg, "message.send.max.retries"); 
    PRINT_VAR(message_send_max_retries);
    queue_buffering_max_messages = cfg_getint(m_cfg, "queue.buffering.max.messages"); 
    PRINT_VAR(queue_buffering_max_messages);

    size_t first_slash = zookeeper_connect.find_first_of("/", 0);
    zookeeper_urls = zookeeper_connect.substr(0, first_slash);
    if (first_slash != string::npos) {
        kafka_chroot_path = zookeeper_connect.substr(first_slash);
    } else {
        kafka_chroot_path = "";
    }

    if (!isAbsPath(pos_path.c_str())) {
        pos_path = realdir_s + '/' + pos_path;
    }

    if (logkafka_id == "") {
        fprintf(stderr, "The logkafka_id %s is not valid!\n",
                logkafka_id.c_str());
        return false;
    }

    if (line_max_bytes > HARD_LIMIT_LINE_MAX_BYTES) {
        fprintf(stderr, "The line_max_bytes %lu exceeds hard limit %lu!\n",
                line_max_bytes, HARD_LIMIT_LINE_MAX_BYTES);
        return false;
    }

    if (read_max_bytes > HARD_LIMIT_READ_MAX_BYTES) {
        fprintf(stderr, "The read_max_bytes %lu exceeds hard limit %lu!\n",
                read_max_bytes, HARD_LIMIT_READ_MAX_BYTES);
        return false;
    }

    if (key_max_bytes > HARD_LIMIT_KEY_MAX_BYTES) {
        fprintf(stderr, "The key_max_bytes %lu exceeds hard limit %lu!\n",
                key_max_bytes, HARD_LIMIT_KEY_MAX_BYTES);
        return false;
    }

    if (read_max_bytes < line_max_bytes) {
        fprintf(stderr, "The read_max_bytes %lu is less than line_max_bytes %lu!\n",
                read_max_bytes, line_max_bytes);
        return false;
    }

    if (!TailWatcher::isStateSilentMaxMsValid(stat_silent_max_ms)) {
        fprintf(stderr, "The stat_silent_max_ms %lu is not valid!\n",
                stat_silent_max_ms);
        return false;
    }

    if (zookeeper_upload_interval > HARD_LIMIT_ZOOKEEPER_UPLOAD_INTERVAL) {
        fprintf(stderr, "The zookeeper_upload_interval %lu exceeds hard limit %lu!\n",
                zookeeper_upload_interval, HARD_LIMIT_ZOOKEEPER_UPLOAD_INTERVAL);
        return false;
    }

    return true;
}/*}}}*/

} // namespace logkafka
