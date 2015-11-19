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
#include "logkafka/io_handler.h"

namespace logkafka {

IOHandler::IOHandler()
{/*{{{*/
    m_file = NULL;
    m_line = NULL;
    m_last_io_time = (struct timeval){0};
    m_filter = NULL;
    m_output = NULL;
}/*}}}*/

IOHandler::~IOHandler()
{/*{{{*/
    free(m_line);
    m_line = NULL;
}/*}}}*/

bool IOHandler::init(FILE *file,
                     PositionEntry *position_entry,
                     unsigned int max_line_at_once,
                     unsigned int line_max_bytes,
                     void *filter,
                     void *output,
                     ReceiveFunc receiveLines)
{/*{{{*/
    m_file = file;
    m_position_entry = position_entry;
    m_max_line_at_once = max_line_at_once;
    m_line_max_bytes = line_max_bytes;
    m_filter = filter;
    m_output = output;
    m_receive_func = receiveLines;

    if (NULL == (m_line = reinterpret_cast<char *>(malloc(m_line_max_bytes + 1)))) {
        LERROR << "Fail to malloc " << (m_line_max_bytes + 1) << " bytes"
               << ", " << strerror(errno);
        return false;
    }
    
    bzero(m_line, m_line_max_bytes + 1);

    if (0 != gettimeofday(&m_last_io_time, NULL)) {
        LERROR << "Fail to get time";
        return false;
    }

    return true;
}/*}}}*/

void IOHandler::onNotify(void *arg)
{/*{{{*/
    IOHandler *ioh = reinterpret_cast<IOHandler *>(arg);

    if (NULL == ioh) {
        LERROR << "io handler is NULL";
        return;
    }

    ScopedLock l(ioh->m_io_handler_mutex);

    if (NULL == ioh->m_receive_func)
        return;

    if (NULL == ioh->m_file)
        return;
    
    /* handle last unreceived lines */ 
    if (!ioh->m_lines.empty()) {
        ioh->updateLastIOTime();
        if ((*ioh->m_receive_func)(ioh->m_filter, ioh->m_output, ioh->m_lines)) {
            ioh->m_position_entry->updatePos(ioh->getFilePos());
            ioh->m_lines.clear();
        } else {
            return;
        }
    }

    bool read_more = false;

    do {
        ioh->m_lines.clear();
        read_more = false;

        while (true) {
            char *line = NULL;
            pthread_mutex_lock(&(ioh->m_file_mutex).mutex());
            if (NULL != ioh->m_file) {
                line = fgets(ioh->m_line, ioh->m_line_max_bytes + 1, ioh->m_file);
            }
            pthread_mutex_unlock(&(ioh->m_file_mutex).mutex());

            if (NULL != line) {
                size_t len = strlen(ioh->m_line);
                if (ioh->m_line[len-1] == '\n') ioh->m_line[len-1] = '\0';
                ioh->m_lines.push_back(string(ioh->m_line));
            } else {
                if (int err = ferror(ioh->m_file)) {
                    LERROR << "Fail to read from fd " << fileno(ioh->m_file)
                           << ", ferror: " << err;
                }

                /* fgets of BSD libc will not clear EOF flag bit (glibc will), 
                 * so we have to call clearerr to clear the flag bit of EOF & ERR
                 * */
                clearerr(ioh->m_file);

                break;
            }

            if (ioh->m_lines.size() >= ioh->m_max_line_at_once) {
                 read_more = true;
                 break;
            }
        }

        if (!ioh->m_lines.empty()) {
            /* XXX: restart one timer here, when timeout, 
             * delete path from corresponding tail watcher.  
             * NOTE: if using just one loop for all watchers,
             * the callbacks are executed one after another,
             * if one callback cost too much time, the timer
             * will timeout early than expected, when choosing
             * timeout value, you should take this into consideration.
             * */
            ioh->updateLastIOTime();
            if ((*ioh->m_receive_func)(ioh->m_filter, ioh->m_output, ioh->m_lines)) {
                ioh->m_position_entry->updatePos(ioh->getFilePos());
                ioh->m_lines.clear();
            } else {
                read_more = false;
            }
        }
    } while (read_more);
}/*}}}*/

void IOHandler::updateLastIOTime()
{/*{{{*/
    if (0 == pthread_mutex_trylock(&m_last_io_time_mutex.mutex())) {
        if (0 != gettimeofday(&m_last_io_time, NULL)) {
            LERROR << "Fail to get time";
        }
        pthread_mutex_unlock(&m_last_io_time_mutex.mutex());
    }
}/*}}}*/

bool IOHandler::getLastIOTime(struct timeval &tv)
{/*{{{*/
    bool res = false;
    if (0 == pthread_mutex_trylock(&m_last_io_time_mutex.mutex())) {
        tv = m_last_io_time;
        pthread_mutex_unlock(&m_last_io_time_mutex.mutex());
        res = true;
    }
    return res;
}/*}}}*/

void IOHandler::close()
{/*{{{*/
    if (0 == pthread_mutex_lock(&m_file_mutex.mutex())) {
        if (NULL != m_file) {
            LINFO << "Closing file"
                   << ", fd: " << fileno(m_file)
                   << ", inode: " << getInode(m_file);
            fclose(m_file); m_file = NULL;
        }
        pthread_mutex_unlock(&m_file_mutex.mutex());
    }
}/*}}}*/

long IOHandler::getFileInode()
{/*{{{*/
    long inode = INO_NONE;
    if (0 == pthread_mutex_lock(&m_file_mutex.mutex())) {
        inode = (NULL != m_file)? getInode(m_file): inode;
        pthread_mutex_unlock(&m_file_mutex.mutex());
    }
    return inode;
}/*}}}*/

long IOHandler::getFileSize()
{/*{{{*/
    long fsize = 0;
    if (0 == pthread_mutex_lock(&m_file_mutex.mutex())) {
        fsize = (NULL != m_file)? getFsize(m_file): fsize;
        pthread_mutex_unlock(&m_file_mutex.mutex());
    }
    return fsize;
}/*}}}*/

long IOHandler::getFilePos()
{/*{{{*/
    long fpos = 0;
    if (0 == pthread_mutex_lock(&m_file_mutex.mutex())) {
        fpos = (NULL != m_file)? ftell(m_file): fpos;
        pthread_mutex_unlock(&m_file_mutex.mutex());
    }
    return fpos;
}/*}}}*/

} // namespace logkafka
