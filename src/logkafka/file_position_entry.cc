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
#include "logkafka/file_position_entry.h"

namespace logkafka {

const size_t FilePositionEntry::POS_SIZE = 16;
const size_t FilePositionEntry::INO_OFFSET = 17;
const size_t FilePositionEntry::INO_SIZE = 8;
const int FilePositionEntry::LN_OFFSET = 25;
const int FilePositionEntry::SIZE = 26;

FilePositionEntry::FilePositionEntry(FILE *file, off_t seek)
{/*{{{*/
    m_file = file;
    m_seek = seek;
}/*}}}*/

bool FilePositionEntry::update(ino_t inode, off_t pos)
{/*{{{*/
    fseek(m_file, m_seek, SEEK_SET);
    fprintf(m_file, "%016llx\t%08x", 
            (unsigned long long)pos, (unsigned int)inode);

    return true;
}/*}}}*/

bool FilePositionEntry::updatePos(off_t pos) 
{/*{{{*/
    fseek(m_file, m_seek, SEEK_SET);
    fprintf(m_file, "%016llx", (unsigned long long)pos);

    return true;
}/*}}}*/

off_t FilePositionEntry::readPos() 
{/*{{{*/
    fseek(m_file, m_seek, SEEK_SET);
    char buf[POS_SIZE + 1] = {'\0'};
    return (1 == fread(buf, POS_SIZE, 1, m_file)) ? 
        hexstr2num(buf, -1): -1;
}/*}}}*/

ino_t FilePositionEntry::readInode() 
{/*{{{*/
    fseek(m_file, m_seek + INO_OFFSET, SEEK_SET);
    char buf[INO_SIZE + 1] = {'\0'};
    return (1 == fread(buf, INO_SIZE, 1, m_file)) ? 
        hexstr2num(buf, INO_NONE): INO_NONE;
}/*}}}*/

} // namespace logkafka
