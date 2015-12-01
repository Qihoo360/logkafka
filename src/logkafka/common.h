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
#ifndef LOGKAFKA_COMMON_H_
#define LOGKAFKA_COMMON_H_

/* zookeeper event type constants */
#define CREATED_EVENT_DEF 1
#define DELETED_EVENT_DEF 2
#define CHANGED_EVENT_DEF 3
#define CHILD_EVENT_DEF 4
#define SESSION_EVENT_DEF -1
#define NOTWATCHING_EVENT_DEF -2

#define VERSION 1.0

#define ZNODE_BUF_MAX_LEN 10240U

#define DEFAULT_LINE_MAX_BYTES 1048576UL /* 1MB */
#define DEFAULT_READ_MAX_BYTES 1048576UL /* 1MB */
#define DEFAULT_KEY_MAX_BYTES 1024UL /* 1KB */
#define DEFAULT_STAT_SILENT_MAX_MS 10000UL /* milliseconds */
#define DEFAULT_BATCHSIZE 100U
#define DEFAULT_ZOOKEEPER_CONNECT "127.0.0.1:2181"
#define DEFAULT_POS_PATH "logkafka.pos"
#define DEFAULT_LOGKAFKA_ID ""
#define DEFAULT_ZOOKEEPER_UPLOAD_INTERVAL 10000UL /* milliseconds */
#define DEFAULT_REFRESH_INTERVAL 60000UL /* milliseconds */
#define DEFAULT_MESSAGE_SEND_MAX_RETRIES 10000UL
#define DEFAULT_QUEUE_BUFFERING_MAX_MESSAGES 10000UL
#define DEFAULT_PATH_QUEUE_MAX_SIZE 100
#define DEFAULT_RDKAFKA_POLL_TIMEOUT 100 /* milliseconds */

#define HARD_LIMIT_LINE_MAX_BYTES 1073741824UL /* 1GB */
#define HARD_LIMIT_READ_MAX_BYTES 1073741824UL /* 1GB */
#define HARD_LIMIT_KEY_MAX_BYTES 10240UL /* 10KB */
#define HARD_LIMIT_ZOOKEEPER_UPLOAD_INTERVAL 60000UL /* milliseconds */

#define FILEPOS_END -1       /* read from file end*/

#define LOGKAFKA_PCRE2
#define PCRE2_CODE_UNIT_WIDTH 8

#ifndef _POSIX_HOST_NAME_MAX
#define _POSIX_HOST_NAME_MAX 255
#endif

#endif // LOGKAFKA_COMMON_H_
