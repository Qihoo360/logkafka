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
#ifndef LOGKAFKA_POSITION_ENTRY_H_
#define LOGKAFKA_POSITION_ENTRY_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <string>
#include <vector>

#include "base/common.h"

using namespace std;

namespace logkafka {

class PositionEntry
{
    public:
        PositionEntry() {};
        virtual ~PositionEntry() {};
        virtual bool update(ino_t inode, off_t pos) = 0;
        virtual bool updatePos(off_t pos) = 0;
        virtual ino_t readInode() = 0;
        virtual off_t readPos() = 0;
};

} // namespace logkafka

#endif // LOGKAFKA_POSITION_ENTRY_H_
