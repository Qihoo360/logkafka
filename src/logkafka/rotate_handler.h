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
#ifndef LOGKAFKA_ROTATE_HANDLER_H_
#define LOGKAFKA_ROTATE_HANDLER_H_

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <string>

#include "base/common.h"
#include "base/mutex.h"
#include "base/scoped_lock.h"

#include "easylogging/easylogging++.h"

using namespace std;
using namespace base;

namespace logkafka {

typedef bool (*RotateFunc)(void *, FILE *);

class RotateHandler
{
    public:
        RotateHandler() {m_last_rotate_time = (struct timeval){0};};
        bool init(string path,
                  void *rotate_func_arg,
                  RotateFunc on_rotate);
        static void onNotify(void *arg);
        void updateLastRotateTime();
        bool getLastRotateTime(struct timeval &tv);

    public:
        string m_path;
        RotateFunc m_rotate_func;
        void *m_rotate_func_arg;
        ino_t m_inode;
        off_t m_fsize;

    private:
        struct timeval m_last_rotate_time;
        Mutex m_last_rotate_time_mutex;
};

} // namespace logkafka

#endif // LOGKAFKA_ROTATE_HANDLER_H_
