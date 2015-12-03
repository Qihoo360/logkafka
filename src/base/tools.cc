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
#include "base/tools.h"

#include <arpa/inet.h>
#include <glob.h>
#include <libgen.h>
#include <sys/time.h>

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>

#include <logkafka/common.h>

#include "easylogging/easylogging++.h"

using namespace std;

vector<string> explode(const string& str, char separator)
{/*{{{*/
    std::stringstream ss(str);
    std::string segment;
    std::vector<std::string> seglist;

    while(std::getline(ss, segment, separator)) {
       seglist.push_back(segment);
    }
    
    return seglist;
}/*}}}*/

vector<string> searchFiles(const string& pattern)
{/*{{{*/
    vector<string> files;
    glob_t pglob;

    if (glob(pattern.c_str(), 0, NULL, &pglob) == 0) {
        for (size_t i = 0; i < pglob.gl_pathc; ++i) {
            string file = pglob.gl_pathv[i];
            files.push_back(file);
        }
    }

    globfree(&pglob);
    return files;
}/*}}}*/

void ltrim(char *str)
{/*{{{*/
    int i = 0, j, len = strlen(str);
    while (str[i] != '\0') {
        if (!isspace(str[i]))break;
        ++i;
    }
    if (i != 0) {
        for (j = 0; j <= len-i; ++j) {
            str[j] = str[j+i];
        }
    }
}/*}}}*/

void rtrim(char *str)
{/*{{{*/
    int i = strlen(str)-1;
    while (i >= 0) {
        if (!isspace(str[i]))break;
        i--;
    }
    str[++i]='\0';
}/*}}}*/

void trim(char *str)
{/*{{{*/
    ltrim(str);
    rtrim(str);
}/*}}}*/

string int2Str(long long val)
{/*{{{*/
    stringstream ss;
    ss << val;
    return ss.str();
}/*}}}*/

bool str2Bool(std::string str)
{/*{{{*/
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    std::istringstream is(str);
    bool b;
    is >> std::boolalpha >> b;
    return b;
}/*}}}*/

const char* realDir(const char *filepath, char *realdir)
{/*{{{*/
    char buf[PATH_MAX + 1] = {'\0'};

    if (NULL == realpath(filepath, buf)) {
        fprintf(stderr, "get realpath error: %s\n", strerror(errno));
        return NULL;
    }

    strcpy(realdir, dirname(buf));
    
    return realdir;
}/*}}}*/

bool isAbsPath(const char* filepath)
{/*{{{*/
    size_t len = strlen(filepath);
    if (len < 1) {
        fprintf(stderr, "filepath length is %ld\n", len);
        return false;
    }

    return filepath[0] == '/';
}/*}}}*/

set<std::string> diff_set(set<std::string> s1, set<std::string> s2)
{/*{{{*/
    set<std::string> s;
    set_difference(s1.begin(), s1.end(), 
            s2.begin(), s2.end(), 
            inserter(s, s.end()));

    return s;
};/*}}}*/

set<std::string> intersect_set(set<std::string> s1, set<std::string> s2)
{/*{{{*/
    set<std::string> s;
    set_intersection(s1.begin(), s1.end(), 
            s2.begin(), s2.end(), 
            inserter(s, s.end()));

    return s;
};/*}}}*/

ino_t getInode(const char *path)
{/*{{{*/
    struct stat buf;
    off_t fsize;
    ino_t inode;
    if (0 == stat(path, &buf)) {
        fsize = buf.st_size;
        inode = buf.st_ino;
    } else {
        fsize = 0;
        inode = INO_NONE;
    }

    return inode;
};/*}}}*/

ino_t getInode(int fd)
{/*{{{*/
    struct stat buf;
    ino_t inode;
    if (0 == fstat(fd, &buf)) {
        inode = buf.st_ino;
    } else {
        inode = INO_NONE;
    }

    return inode;
}/*}}}*/

ino_t getInode(FILE *fp)
{/*{{{*/
    if (NULL == fp) return INO_NONE;
    return getInode(fileno(fp));
};/*}}}*/

off_t getFsize(const char *path)
{/*{{{*/
    struct stat buf;
    off_t fsize;
    ino_t inode;
    if (0 == stat(path, &buf)) {
        fsize = buf.st_size;
        inode = buf.st_ino;
    } else {
        fsize = 0;
        inode = INO_NONE;
    }

    return fsize;
};/*}}}*/

off_t getFsize(int fd)
{/*{{{*/
    struct stat buf;
    off_t fsize;
    if (0 == fstat(fd, &buf)) {
        fsize = buf.st_size;
    } else {
        fsize = 0;
    }

    return fsize;
};/*}}}*/

off_t getFsize(FILE *fp)
{/*{{{*/
    if (NULL == fp) return 0;
    return getFsize(fileno(fp));
};/*}}}*/

string sig2str(int signum)
{/*{{{*/
    return string(strsignal(signum));
}/*}}}*/

long long hexstr2num(const char *buf, long long default_num)
{/*{{{*/
    if (NULL == buf) return default_num;

    char *pEnd;
    long long num = strtoll(buf, &pEnd, 16);
    return (LONG_MIN != num && LONG_MAX != num) ? num: default_num;
}/*}}}*/

void strReplace(std::string& str, 
        const std::string& oldStr, 
        const std::string& newStr) 
{/*{{{*/
    size_t pos = 0;
    while ((pos = str.find(oldStr, pos)) != std::string::npos) {
       str.replace(pos, oldStr.length(), newStr);
       pos += newStr.length();
    }
}  /*}}}*/

/* globerr --- print error message for glob() */

int globerr(const char *path, int eerrno)
{/*{{{*/
    LERROR << "Glob error, path " << path << ", " << strerror(eerrno);
    return 0;   /* let glob() keep going */
}/*}}}*/

bool globPath(const string &path_pattern, vector<string> &paths)
{/*{{{*/
    int flags = 0;
    glob_t results;
    int ret;

    ret = glob(path_pattern.c_str(), flags, globerr, &results);
    if (ret != 0) {
        LERROR << "Fail to glob path_pattern " << path_pattern
            << ", " << 
    /* ugly: */ (ret == GLOB_ABORTED ? "filesystem problem" :
             ret == GLOB_NOMATCH ? "no match of pattern" :
             ret == GLOB_NOSPACE ? "no dynamic memory" :
             "unknown problem");
        return false;
    }

    for (unsigned i = 0; i < results.gl_pathc; ++i)
        paths.push_back(results.gl_pathv[i]);

    globfree(&results);

    return true;
}/*}}}*/

#ifndef _GNU_SOURCE
int fdprintf(int fd, size_t bufmax, const char *fmt, ...)
{/*{{{*/
    char *buffer;
    int n;
    va_list ap;

    buffer = reinterpret_cast<char *>(malloc (bufmax));
    if (!buffer)
        return 0;

    va_start(ap, fmt);
    n = vsnprintf(buffer, bufmax, fmt, ap);
    va_end(ap);

    write(fd, buffer, n);
    free(buffer);
    return n;
}/*}}}*/
#endif

string getHostname()
{/*{{{*/
    char hostname[_POSIX_HOST_NAME_MAX + 1] = {'\0'};

    if (0 != gethostname(hostname, _POSIX_HOST_NAME_MAX + 1)) {
        LERROR << "Fail to get hostname";
        return "";
    }

    return string(hostname);
}/*}}}*/
