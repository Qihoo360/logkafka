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
#ifndef LOGKAFKA_POSITION_FILE_H_
#define LOGKAFKA_POSITION_FILE_H_

#include <regex.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "base/tools.h"
#include "base/common.h"
#include "logkafka/common.h"
#include "logkafka/file_position_entry.h"

#include "easylogging/easylogging++.h"

#ifdef LOGKAFKA_PCRE2
#include "pcre2.h"
#endif

using namespace std;

namespace logkafka {

struct PositionEntryKey;
typedef FilePositionEntry *value_t;
typedef map<PositionEntryKey, FilePositionEntry *> FilePositionEntryMap;

struct PositionEntryKey 
{
    string path_pattern;
    string path;

    bool operator==(const PositionEntryKey& hs) const
    {
        return (path_pattern == hs.path_pattern) &&
            (path == hs.path);
    };

    bool operator!=(const PositionEntryKey& hs) const
    {
        return !operator==(hs);
    };

    bool operator<(const PositionEntryKey& hs) const
    {
        return (path_pattern < hs.path_pattern) ||
            (path_pattern == hs.path_pattern && path < hs.path);
    };
};

class PositionFile
{
    public:
        PositionFile() {};
        PositionFile(FILE *file, 
                FilePositionEntryMap pe_map, 
                off_t last_pos);
        ~PositionFile();
        bool init(FILE *file,
                FilePositionEntryMap pe_map,
                off_t last_pos);
        value_t& operator[](const PositionEntryKey &key);
        void remove(const PositionEntryKey &pek);
        static PositionFile *parse(FILE *file);
        static bool parseLine(string line, 
                PositionEntryKey &pek,
                off_t &pos,
                ino_t &inode);
        static bool compact(FILE *file);
        bool getPath(const string &path_pattern, string &path);

    public:
        FILE *m_file;
        off_t m_last_pos;
        FilePositionEntryMap m_pe_map;

        static PositionFile *m_pf;
        static const int64_t UNWATCHED_POSITION;
};

} // namespace logkafka

#endif // LOGKAFKA_POSITION_FILE_H_
