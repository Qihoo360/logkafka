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
#include "logkafka/memory_position_entry.h"

namespace logkafka {

bool MemoryPositionEntry::update(ino_t inode, off_t pos)
{/*{{{*/
    m_inode = inode;
    m_pos = pos;

    return true;
}/*}}}*/

bool MemoryPositionEntry::updatePos(off_t pos)
{/*{{{*/
    m_pos = pos;

    return true;
}/*}}}*/

ino_t MemoryPositionEntry::readInode()
{/*{{{*/
    return m_inode;
}/*}}}*/

off_t MemoryPositionEntry::readPos()
{/*{{{*/
    return m_pos;
}/*}}}*/

} // namespace logkafka
