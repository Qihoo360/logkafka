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
#ifndef LOGKAFKA_FILE_POSITION_ENTRY_H_
#define LOGKAFKA_FILE_POSITION_ENTRY_H_

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <string>
#include <vector>

#include "base/common.h"
#include "base/tools.h"
#include "logkafka/position_entry.h"

#include "easylogging/easylogging++.h"

using namespace std;

namespace logkafka {

class FilePositionEntry: public virtual PositionEntry 
{
    public:
        FilePositionEntry(): PositionEntry() {};
        FilePositionEntry(FILE *file, off_t seek);
        ~FilePositionEntry() {};
        bool init(FILE *file, off_t seek);
        bool update(ino_t inode, off_t pos);
        bool updatePos(off_t pos);
        ino_t readInode();
        off_t readPos();

    private:
        FILE *m_file;
        off_t m_seek;

        static const size_t POS_SIZE;
        static const size_t INO_OFFSET;
        static const size_t INO_SIZE;
        static const int LN_OFFSET;
        static const int SIZE;
};

} // namespace logkafka

#endif // LOGKAFKA_FILE_POSITION_ENTRY_H_
