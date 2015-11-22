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
#ifndef BASE_TOOLS_H_
#define BASE_TOOLS_H_

#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <map>
#include <set>
#include <iterator>

#include "base/common.h"

using namespace std;

extern vector<string> explode(const string&, char c);
extern vector<string> searchFiles(const string& pattern);
extern void rtrim(char *str);
extern void ltrim(char *str);
extern void trim(char *str);
extern string int2Str(long long val);
extern bool str2Bool(string str);
extern const char* realDir(const char *filepath, char *realdir);
extern bool isAbsPath(const char* filepath);

// also known as select1st in SGI STL implementation
template<typename T_PAIR>
struct GetKey: public std::unary_function<T_PAIR, typename T_PAIR::first_type>
{/*{{{*/
    const typename T_PAIR::first_type& operator()(const T_PAIR& item) const
    {
        return item.first;
    }
};/*}}}*/

extern set<std::string> diff_set(set<std::string> s1, set<std::string> s2);
extern set<std::string> intersect_set(set<std::string> s1, set<std::string> s2);

extern ino_t getInode(const char *path);
extern ino_t getInode(int fd);
extern ino_t getInode(FILE *fp);
extern off_t getFsize(const char *path);
extern off_t getFsize(int fd);
extern off_t getFsize(FILE *fp);
extern string sig2str(int signum);
extern long long hexstr2num(const char *buf, long long default_num);
void strReplace(std::string& str, 
        const std::string& oldStr, 
        const std::string& newStr);

int globerr(const char *path, int eerrno);
bool globPath(const string &path_pattern, vector<string> &paths);

#ifndef _GNU_SOURCE
int fdprintf(int fd, size_t bufmax, const char * fmt, ...);
#endif

extern string getHostname();

#endif // BASE_TOOLS_H_
