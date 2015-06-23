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
#include <cassert>
#include "logkafka/position_file.h"

namespace logkafka {

PositionFile *PositionFile::m_pf = NULL;
const int64_t PositionFile::UNWATCHED_POSITION = 0xffffffffffffffff;

PositionFile::PositionFile(FILE *file,
        FilePositionEntryMap pe_map,
        off_t last_pos)
{/*{{{*/
    m_file = file;
    m_pe_map = pe_map;
    m_last_pos = last_pos;
}/*}}}*/

PositionFile::~PositionFile()
{/*{{{*/
    for (FilePositionEntryMap::iterator iter = m_pe_map.begin();
            iter != m_pe_map.end(); ++iter) {
        delete iter->second; iter->second = NULL;
    }
}/*}}}*/

bool PositionFile::init(FILE *file,
        FilePositionEntryMap pe_map,
        off_t last_pos)
{/*{{{*/
    assert(NULL != file);

    m_file = file;
    m_pe_map = pe_map;
    m_last_pos = last_pos;

    return true;
}/*}}}*/

value_t& PositionFile::operator[](const PositionEntryKey &pek)
{/*{{{*/
    string path_pattern = pek.path_pattern;
    string path = pek.path;

    FilePositionEntryMap::iterator iter = m_pe_map.find(pek);

    if (iter != m_pe_map.end()) {
        return iter->second;
    }

    fseek(m_file, m_last_pos, SEEK_SET);
    fwrite(path_pattern.c_str(), path_pattern.length(), 1, m_file);
    fwrite("\t", 1, 1, m_file);

    fwrite(path.c_str(), path.length(), 1, m_file);
    fwrite("\t", 1, 1, m_file);

    off_t seek = ftell(m_file);

    string zero = "0000000000000000\t00000000\n";
    fwrite(zero.c_str(), zero.length(), 1, m_file);

    m_last_pos = ftell(m_file);

    return m_pe_map[pek] = new FilePositionEntry(m_file, seek);
}/*}}}*/

PositionFile* PositionFile::parse(FILE *file)
{/*{{{*/
    compact(file);

    FilePositionEntryMap pe_map;

    fseek(file, 0, SEEK_SET);
    long fsize = getFsize(file);

    char *buf = (char *)malloc(fsize);
    if (NULL == buf) {
        LERROR << "Fail to malloc " << fsize << " bytes";
        return NULL;
    }

    char *line;
    while (NULL != (line = fgets(buf, fsize, file))) {
        off_t pos;
        ino_t inode;
        PositionEntryKey pek;

        string line_str(line);
        if (!PositionFile::parseLine(line_str, pek, pos, inode))
            continue;

        long seek = ftell(file) - strlen(line) + pek.path_pattern.length() + 1
            + pek.path.length() + 1;
        pe_map[pek] = new FilePositionEntry(file, seek);
    }

    PositionFile::m_pf = new PositionFile(file, pe_map, ftell(file));

    free(buf);

    return PositionFile::m_pf;
}/*}}}*/

bool PositionFile::parseLine(string line, 
        PositionEntryKey &pek,
        off_t &pos,
        ino_t &inode)
{/*{{{*/
    string &path_pattern = pek.path_pattern;
    string &path = pek.path;

    bool is_match = true;
    string pattern = "^([^\t]+)\t([^\t]+)\t([0-9a-fA-F]+)\t([0-9a-fA-F]+)$";

    int cflags = REG_EXTENDED | REG_NEWLINE;
    regex_t reg;
    int res = regcomp(&reg, pattern.c_str(), cflags);

    if (res == 0) {
        regmatch_t pmatch[4];
        const size_t nmatch = 5;
        char *line_cstr = new char[line.length() + 1];
        strcpy(line_cstr, line.c_str());
        // do stuff
        int status = regexec(&reg, line_cstr, nmatch, pmatch, 0);
        if (status == REG_NOMATCH) {
            is_match = false;
            path_pattern = "";
            path = "";
            pos = -1;
            inode = INO_NONE;
        } else {
            int i;
            i = 1; path_pattern = line.substr(pmatch[i].rm_so, pmatch[i].rm_eo - pmatch[i].rm_so);

            i = 2; path = line.substr(pmatch[i].rm_so, pmatch[i].rm_eo - pmatch[i].rm_so);

            i = 3; string pos_str = line.substr(pmatch[i].rm_so, pmatch[i].rm_eo - pmatch[i].rm_so);
            pos = hexstr2num(pos_str.c_str(), -1);

            i = 4; string inode_str = line.substr(pmatch[i].rm_so, pmatch[i].rm_eo - pmatch[i].rm_so);
            inode = hexstr2num(inode_str.c_str(), 0);
        }

        delete[] line_cstr; line_cstr = NULL;
        regfree(&reg);
    } else {
        is_match = false;
        path_pattern = "";
        path = "";
        pos = -1;
        inode = INO_NONE;
    }

    return is_match;
}/*}}}*/

bool PositionFile::compact(FILE *file)
{/*{{{*/
    FilePositionEntryMap pe_map;

    fseek(file, 0, SEEK_SET);
    long fsize = getFsize(file);

    char *buf = reinterpret_cast<char *>(malloc(fsize));
    if (NULL == buf) {
        LERROR << "Fail to malloc " << fsize << " bytes";
        return false;
    }

    char *line;
    map<string, string> existent_entries;
    while (NULL != (line = fgets(buf, fsize, file))) {
        off_t pos;
        ino_t inode;
        PositionEntryKey pek;

        string line_str(line);
        if (!PositionFile::parseLine(line_str, pek, pos, inode)) {
            continue;
        }

        if (UNWATCHED_POSITION == pos) {
            continue;
        }

        existent_entries[pek.path_pattern] 
            = line_str.substr(pek.path_pattern.length());
    }

    free(buf);

    if (ftruncate(fileno(file), 0) != 0) {
        LERROR << "Fail to truncate file, fileno " << fileno(file);
        return false;
    }

    fseek(file, 0, SEEK_SET);
    for (map<string, string>::const_iterator iter = existent_entries.begin();
            iter != existent_entries.end(); ++iter) {
        string existent_entry = iter->first + iter->second;
        fwrite(existent_entry.c_str(), existent_entry.length(), 1, file);
    }

    return true;
}/*}}}*/

void PositionFile::remove(const PositionEntryKey &pek)
{/*{{{*/
    if (m_pe_map.find(pek) != m_pe_map.end()) {
        delete m_pe_map[pek]; m_pe_map[pek] = NULL;
    }

    m_pe_map.erase(pek);
}/*}}}*/

bool PositionFile::getPath(const string &path_pattern, string &path)
{/*{{{*/
    for (FilePositionEntryMap::const_reverse_iterator iter = m_pe_map.rbegin();
            iter != m_pe_map.rend(); ++iter) {
        if (iter->first.path_pattern == path_pattern) {
            path = iter->first.path;
            return true;
        }
    }

    return false;
}/*}}}*/

} // namespace logkafka
