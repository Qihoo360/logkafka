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
#ifndef LOGKAFKA_IO_HANDLER_H_
#define LOGKAFKA_IO_HANDLER_H_

#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <string>
#include <vector>

#include "base/common.h"
#include "base/mutex.h"
#include "base/scoped_lock.h"
#include "base/tools.h"
#include "logkafka/position_entry.h"

#include "easylogging/easylogging++.h"

using namespace std;
using namespace base;

namespace logkafka {

typedef bool (*ReceiveFunc)(void *, void *, const vector<string> &);

class IOHandler
{
    public:
        IOHandler();
        ~IOHandler();
        bool init(FILE *file,
                  PositionEntry *position_entry,
                  unsigned int max_line_at_once,
                  unsigned int line_max_bytes,
                  unsigned int read_max_bytes,
                  char line_delimiter,
                  bool remove_delimiter,
                  void *filter,
                  void *output,
                  ReceiveFunc receiveLines);
        void close();
        static void onNotify(void *arg);
        bool getLastIOTime(struct timeval &tv);
        long getFileInode();
        long getFileSize();
        long getFilePos();

    public:
        FILE *m_file;
        PositionEntry *m_position_entry;

    private:
        void updateLastIOTime();
        bool getLastBufferStuckTime(struct timeval &tv);
        void updateLastBufferStuckTime();
        bool isBufferStuck();

    private:
        unsigned int m_max_line_at_once;
        unsigned int m_line_max_bytes;
        unsigned int m_buffer_max_bytes;
        unsigned long m_buffer_stuck_max_ms;
        bool m_buffer_last_segment;
        ReceiveFunc m_receive_func;
        void *m_filter;
        void *m_output;

        char *m_buffer;
        size_t m_buffer_len;
        char m_line_delimiter;
        bool m_remove_delimiter;
        vector<string> m_lines;

        struct timeval m_last_io_time;
        struct timeval m_last_buffer_stuck_time;

        Mutex m_last_io_time_mutex;
        Mutex m_last_buffer_stuck_time_mutex;
        Mutex m_file_mutex;
};

} // namespace logkafka

#endif // LOGKAFKA_IO_HANDLER_H_
