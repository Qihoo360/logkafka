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
#include "logkafka/tail_watcher.h"

namespace logkafka {

const unsigned long TailWatcher::TIMER_WATCHER_DEFAULT_REPEAT = 3000UL;
const unsigned long TailWatcher::STAT_WATCHER_DEFAULT_INTERVAL = 1000UL;

TailWatcher::TailWatcher()
{/*{{{*/
    m_receive_func = NULL;
    m_timer_trigger = NULL;
    m_stat_trigger = NULL;
    m_rotate_handler = NULL;
    m_output = NULL;
    m_manager = NULL;
    m_filter = NULL;
}/*}}}*/

TailWatcher::~TailWatcher()
{/*{{{*/
    m_timer_trigger->close();
    delete m_timer_trigger; m_timer_trigger = NULL;
    m_stat_trigger->close();
    delete m_stat_trigger; m_stat_trigger = NULL;
    {
        ScopedLock l(m_rotate_handler_mutex);
        delete m_rotate_handler; m_rotate_handler = NULL;
    }
    {
        ScopedLock l(m_io_handler_mutex);
        delete m_io_handler; m_io_handler = NULL;
    }
    delete m_output; m_output = NULL;
    delete m_filter; m_filter = NULL;
}/*}}}*/

bool TailWatcher::init(uv_loop_t *loop, 
        string path_pattern, 
        string path, 
        PositionEntry *position_entry,
        unsigned long stat_silent_max_ms,
        bool read_from_head,
        unsigned long max_line_at_once,
        unsigned long line_max_bytes, 
        unsigned long read_max_bytes,
        char line_delimiter,
        bool remove_delimiter,
        UpdateFunc updateWatcher,
        ReceiveFunc receiveLines,
        TaskConf conf,
        Manager *manager,
        Output *output)
{/*{{{*/
    /* We will not close watch until stat change time expired m_stat_silent_max_ms, 
     * remove or change state_wait to infinite */
    m_stat_silent_max_ms = stat_silent_max_ms;
    m_unwatched = false;
    m_path_pattern = path_pattern;
    m_path = path;
    m_position_entry = position_entry; 
    m_read_from_head = read_from_head;
    m_max_line_at_once = max_line_at_once;
    m_line_max_bytes = line_max_bytes;
    m_read_max_bytes = read_max_bytes;
    m_line_delimiter = line_delimiter;
    m_remove_delimiter = remove_delimiter;
    m_updateWatcher = updateWatcher;
    m_receive_func = receiveLines;
    m_conf = conf;
    m_manager = manager;
    m_output = output; 

    m_loop = loop;

    m_filter = new FilterRegex(conf.filter_conf);
    if (!m_filter->init(NULL)) {
        LWARNING << "Fail to init filter";
        delete m_filter; m_filter = NULL;
    }

    m_timer_trigger = new TimerWatcher();
    if (!m_timer_trigger->init(m_loop, 0, TIMER_WATCHER_DEFAULT_REPEAT,
                this, &onNotify)) {
        LERROR << "Fail to init timer watcher";
        delete m_timer_trigger; m_timer_trigger = NULL;
        return false;
    }

    m_stat_trigger = new StatWatcher();
    if (!m_stat_trigger->init(m_loop, path, STAT_WATCHER_DEFAULT_INTERVAL,
                this, &onNotify)) {
        LERROR << "Fail to init stat watcher";
        delete m_stat_trigger; m_stat_trigger = NULL;
        return false;
    }

    m_io_handler = NULL;
    
    {
        ScopedLock l(m_rotate_handler_mutex);
        m_rotate_handler = new RotateHandler(); 
        if (!m_rotate_handler->init(path, this, onRotate)) {
            LERROR << "Fail to init rotate handler";
            delete m_rotate_handler; m_rotate_handler = NULL;
            return false;
        }
    }

    return true;
}/*}}}*/

void TailWatcher::onNotify(void *arg)
{/*{{{*/
    TailWatcher *tw = (TailWatcher *)arg;

    {
        /* handle rotating */
        ScopedLock l(tw->m_rotate_handler_mutex);
        if (NULL != tw->m_rotate_handler)
            tw->m_rotate_handler->onNotify((void *)tw->m_rotate_handler);
    }

    {
        /* handle io */
        ScopedLock l(tw->m_io_handler_mutex);
        if (NULL != tw->m_io_handler)
            tw->m_io_handler->onNotify((void *)tw->m_io_handler);
    }
}/*}}}*/

bool TailWatcher::onRotate(void *arg, FILE *file)
{/*{{{*/
    TailWatcher *tw = (TailWatcher *)arg;
    PositionEntry *pe = tw->m_position_entry;
    unsigned int max_line_at_once = tw->m_max_line_at_once;
    unsigned int line_max_bytes = tw->m_line_max_bytes;
    unsigned int read_max_bytes = tw->m_read_max_bytes;
    char line_delimiter = tw->m_line_delimiter;
    bool remove_delimiter = tw->m_remove_delimiter;
    ReceiveFunc receiveLines = tw->m_receive_func;
    UpdateFunc updateWatcher = tw->m_updateWatcher;

    ScopedLock l(tw->m_io_handler_mutex);

    if (NULL == (tw->m_io_handler)) {
        int pos = 0;
        if (NULL != file) {
            struct stat buf;
            fstat(fileno(file), &buf);
            off_t fsize = buf.st_size;
            ino_t inode = buf.st_ino;

            ino_t last_inode = pe->readInode();
            if (inode == last_inode) {
                pos = pe->readPos();
            } else if (last_inode != 0) {
                pos = 0;
                LINFO << "Updating position entry, inode: " << inode << ", pos: " << pos;
                pe->update(inode, pos);
            } else {
                pos = tw->m_read_from_head? 0: fsize;
                LINFO << "Updating position entry, inode: " << inode << ", pos: " << pos;
                pe->update(inode, pos);
            }

            fseek(file, pos, SEEK_SET);

            tw->m_io_handler = new IOHandler();
            bool res = tw->m_io_handler->init(file, pe, max_line_at_once, 
                    line_max_bytes, read_max_bytes,
                    line_delimiter, remove_delimiter,
                    tw->m_filter, tw->m_output, receiveLines);
            if (!res) {
                LERROR << "Fail to init io handler, inode: " << inode;
                delete tw->m_io_handler; tw->m_io_handler = NULL;
                return false;
            }
        }
    } else {
        if (NULL != file) {
            struct stat buf;
            fstat(fileno(file), &buf);
            off_t fsize = buf.st_size;
            ino_t inode = buf.st_ino;

            ino_t last_inode = pe->readInode();
            if (inode == last_inode) { // truncated
                pe->updatePos(fsize);

                IOHandler *io_handler = new IOHandler();
                bool res = io_handler->init(file, pe, max_line_at_once, 
                        line_max_bytes, read_max_bytes,
                        line_delimiter, remove_delimiter,
                        tw->m_filter, tw->m_output, receiveLines);
                if (!res) {
                    LERROR << "Fail to init io handler, inode: " << inode;
                    delete io_handler;
                    return false;
                }

                tw->m_io_handler->close();

                delete tw->m_io_handler;
                tw->m_io_handler = io_handler;
            } else if (NULL == tw->m_io_handler->m_file) {
                off_t curpos = ftell(file);
                pe->update(inode, curpos);

                IOHandler *io_handler = new IOHandler();
                bool res = io_handler->init(file, pe, max_line_at_once, 
                        line_max_bytes, read_max_bytes,
                        line_delimiter, remove_delimiter,
                        tw->m_filter, tw->m_output, receiveLines);
                if (!res) {
                    LERROR << "Fail to init io handler, inode: " << inode;
                    delete io_handler;
                    return false;
                }

                delete tw->m_io_handler;
                tw->m_io_handler = io_handler;
            } else {
                //(*updateWatcher)(tw->m_manager, tw->m_path_pattern, tw->m_path, 
                //        swapState(&tw->m_position_entry, tw->m_io_handler));
                l.unlock();
                if (!(*updateWatcher)(tw->m_manager, tw->m_path_pattern, tw->m_path, tw->m_position_entry)) {
                    LWARNING << "Fail to rotate " << tw->m_path;
                    return false;
                } else {
                    LDEBUG << "Closing file"
                           << ", fd: " << fileno(file)
                           << ", inode: " << getInode(file);
                    fclose(file); 
                }
            }
        }
    }

    return true;
}/*}}}*/

PositionEntry *TailWatcher::swapState(PositionEntry **pep, IOHandler *io_handler)
{/*{{{*/
    PositionEntry *pe = *pep;

    MemoryPositionEntry *mpe = new MemoryPositionEntry();
    mpe->update(pe->readInode(), pe->readPos());

    *pep = mpe;
    io_handler->m_position_entry = mpe;

    return pe;
}/*}}}*/

void TailWatcher::stop(bool close_io)
{/*{{{*/
    if (NULL != m_timer_trigger) m_timer_trigger->stop();
    if (NULL != m_stat_trigger) m_stat_trigger->stop();

    ScopedLock l(m_io_handler_mutex);
    if (close_io && NULL != m_io_handler) {
        m_io_handler->onNotify(this->m_io_handler);
        m_io_handler->close();
    }
}/*}}}*/

void TailWatcher::start()
{/*{{{*/
    if (m_timer_trigger) m_timer_trigger->start();
    if (m_stat_trigger) m_stat_trigger->start();
    onNotify(this);
}/*}}}*/

bool TailWatcher::isActive() 
{/*{{{*/
    bool is_active = true;

    struct timeval cur_tv = (struct timeval){0};
    if (0 != gettimeofday(&cur_tv, NULL)) {
        LERROR << "Fail to get time";
        return is_active;
    }

    struct timeval last_stat_time = (struct timeval){0};
    {
        ScopedLock l(m_io_handler_mutex);
        if (NULL == m_io_handler)
            return false;

        if (!m_io_handler->getLastIOTime(last_stat_time)) {
            LERROR << "Fail to get last io time";
            return true;
        }
    }

    LDEBUG << "m_stat_silent_max_ms: " << m_stat_silent_max_ms
           << ", cur_tv.tv_sec: " << cur_tv.tv_sec
           << ", m_last_stat_time : " << last_stat_time.tv_sec;
    if ((cur_tv.tv_sec - last_stat_time.tv_sec) * 1000UL > m_stat_silent_max_ms) {
        LINFO << "Set tail watcher to inactive"
              << ", path_pattern " << m_path_pattern 
              << ", path " << m_path;
        is_active = false;
    }

    return is_active;
}/*}}}*/

string TailWatcher::getPath()
{/*{{{*/
    return m_path;
}/*}}}*/

bool TailWatcher::isStateSilentMaxMsValid(unsigned long stat_silent_max_ms)
{/*{{{*/
     unsigned long lower_limit = 
             max(STAT_WATCHER_DEFAULT_INTERVAL,
             TIMER_WATCHER_DEFAULT_REPEAT);

     if (stat_silent_max_ms <= lower_limit) {
         LERROR << "stat_silent_max_ms should > " << lower_limit; 
         return false;
     }

     return true;
}/*}}}*/

} // namespace logkafka
