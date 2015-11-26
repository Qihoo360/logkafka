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
#include "logkafka/zookeeper.h"

#include <cstdlib>
#include <sstream>

#include "base/tools.h"
#include "logkafka/producer.h"

#include "easylogging/easylogging++.h"

using namespace base;

namespace logkafka {

const unsigned long Zookeeper::REFRESH_INTERVAL_MS = 30000UL;
const unsigned long Zookeeper::SESSION_TIMEOUT_MS = 30000UL;

Zookeeper::Zookeeper()
{/*{{{*/
    m_zhandle = NULL;
    m_loop = NULL;
    m_zk_log_fp = NULL;
    m_refresh_timer_trigger = NULL;
    m_thread = NULL;

    m_log_config = "{}";
    m_broker_urls = "";
    m_session_timeout_ms = SESSION_TIMEOUT_MS;
    m_clientid = NULL;

    m_registered = false;
}/*}}}*/

Zookeeper::~Zookeeper()
{/*{{{*/
    delete m_loop; m_loop = NULL;
    delete m_zk_log_fp; m_zk_log_fp = NULL;
    delete m_refresh_timer_trigger; m_refresh_timer_trigger = NULL;
    delete m_thread; m_thread = NULL;
    delete m_clientid; m_clientid = NULL;
}/*}}}*/

bool Zookeeper::init(const string &zookeeper_urls, 
        const string &kafka_chroot_path,
        const string &logkafka_id,
        long refresh_interval)
{/*{{{*/
    m_zookeeper_urls = zookeeper_urls;
    m_kafka_chroot_path = kafka_chroot_path;
    m_logkafka_id = logkafka_id;

    m_zk_log_fp = fopen("/dev/null", "w");
    zoo_set_log_stream(m_zk_log_fp);

    m_broker_ids_path = m_kafka_chroot_path + "/brokers/ids";
    m_logkafka_config_path = m_kafka_chroot_path + "/logkafka/config";
    m_logkafka_client_path = m_kafka_chroot_path + "/logkafka/client";

    m_client_logkafka_id_path = m_logkafka_client_path + "/" + m_logkafka_id;
    m_config_logkafka_id_path = m_logkafka_config_path + "/" + m_logkafka_id;

    refresh((void *)this);

    /* Use one zookeeper loop for all watchers */
    m_loop = new uv_loop_t();
    int res = uv_loop_init(m_loop);
    if (res < 0) {
        LERROR << "Fail to init uv loop, " << uv_strerror(res);
        return false;
    }

    /* The existence of the async handle will keep the loop alive. */  
    m_exit_handle.data = this;
    uv_async_init(m_loop, &m_exit_handle, exitAsyncCb);

    m_refresh_timer_trigger = new TimerWatcher();
    if (!m_refresh_timer_trigger->init(m_loop, 0,
            refresh_interval,
            this, &refresh)) {
        LERROR << "Fail to init refresh timer";
        delete m_refresh_timer_trigger; m_refresh_timer_trigger = NULL;
        return false;
    }
 
    /* Run zookeeper loop in another thread */
    m_thread = new uv_thread_t();
    uv_thread_create(m_thread, &threadFunc, m_loop);

    return true;
}/*}}}*/

void Zookeeper::threadFunc(void *arg)
{/*{{{*/
    uv_loop_t *loop = reinterpret_cast<uv_loop_t *>(arg);
    int res = uv_run(loop, UV_RUN_DEFAULT);
    if (res < 0) {
        LERROR << "Fail to run loop, " << uv_strerror(res);
    }
}/*}}}*/

bool Zookeeper::connect()
{/*{{{*/
    if (NULL != m_zhandle) {
        LINFO << "Close invaild zookeeper connection...";
        zookeeper_close(m_zhandle);
        m_zhandle = NULL;
    }

    int flags = 0;
    LDEBUG << "Initiating client connection"
           << ", zookeeper urls = " << m_zookeeper_urls
           << ", watcher = " << globalWatcher
           << ", sessionTimeout = " << m_session_timeout_ms
           << ", sessionId = " << (m_clientid == 0 ? 0 : m_clientid->client_id)
           << ", sessionPasswd = " << ((m_clientid == 0) || (strncmp(m_clientid->passwd, "", 2) == 0) ? "<null>" : "<hidden>")
           << ", context = " << (void*)this
           << ", flags = " << flags;
    m_zhandle = zookeeper_init(m_zookeeper_urls.c_str(), 
            globalWatcher, m_session_timeout_ms, m_clientid, (void*)this, flags);
    if (NULL == m_zhandle) {
        LERROR << "Fail to init zhandle, zookeeper urls " << m_zookeeper_urls;
        return false;
    }

    return true;
}/*}}}*/

void Zookeeper::close()
{/*{{{*/
    m_refresh_timer_trigger->close();

    uv_stop(m_loop);
    uv_async_send(&m_exit_handle);

    ScopedLock lk(m_zhandle_mutex);
    if (NULL != m_zhandle) {
        zookeeper_close(m_zhandle);
        m_zhandle = NULL;
    }

    if (NULL != m_zk_log_fp) {
        fclose(m_zk_log_fp); 
        m_zk_log_fp = NULL;
    }
}/*}}}*/

void Zookeeper::exitAsyncCb(uv_async_t* handle) 
{/*{{{*/
    /* After closing the async handle, it will no longer keep the loop alive. */
    Zookeeper *zookeeper = reinterpret_cast<Zookeeper *>(handle->data);
    uv_close((uv_handle_t*) &zookeeper->m_exit_handle, NULL);
} /*}}}*/

void Zookeeper::refresh(void *arg)
{/*{{{*/
    if (NULL == arg) {
        LERROR << "Zookeeper refresh function arg is NULL";
        return;
    }

    Zookeeper *zookeeper = reinterpret_cast<Zookeeper *>(arg);

    if (!zookeeper->refreshConnection()) {
        LERROR << "Fail to refresh zookeeper connection";
        return;
    }

    if (!zookeeper->refreshBrokerUrls()) {
        LERROR << "Fail to refresh broker urls";
    }

    if (!zookeeper->refreshWatchers()) {
        LERROR << "Fail to refresh zookeeper watchers";
    }

    if (!zookeeper->refreshLogConfig()) {
        LERROR << "Fail to refresh log config";
    }
}/*}}}*/

bool Zookeeper::refreshConnection()
{/*{{{*/
    ScopedLock l(m_zhandle_mutex);

    int res = zoo_state(m_zhandle);
    if (NULL == m_zhandle || ZOO_EXPIRED_SESSION_STATE == res) {
        LDEBUG << "Zookeeper error, " << zerror(res);
        if (!connect()) {
            LERROR << "Fail to reset zookeeper connection";
            return false;
        }
    }

    return true;
}/*}}}*/

bool Zookeeper::refreshWatchers()
{/*{{{*/
    ScopedLock l(m_zhandle_mutex);

    /* set config change watcher */
    if (!setWatcher(m_config_logkafka_id_path, configChangeWatcher, (void*)this)) {
        LERROR << "Fail to set config change watcher";
        return false;
    }

    /* set broker change watcher */
    if (!setChildrenWatcher(m_broker_ids_path, brokerChangeWatcher, (void*)this)) {
        LERROR << "Fail to set broker change watcher";
        return false;
    }

    if (!ensurePathExist(m_logkafka_client_path)) {
        LERROR << "Fail to create zookeeper path, " << m_logkafka_client_path;
        return false;
    }

    /* create EPHEMERAL node for checking whether logkafka is alive */
    struct Stat stat;
    int status = ZOK;
    int len = 0;
    char *buf = NULL;
    buf = (char *)malloc(len + 1);
    bzero(buf, len + 1);
    status = zoo_get(m_zhandle, m_client_logkafka_id_path.c_str(), 0, buf, &len, &stat);
    if (ZOK == status) {
        if (0 == stat.ephemeralOwner) { // persistent node
            LINFO << "Deleting persistent zookeeper path, " << m_client_logkafka_id_path;
            if (ZOK != zoo_delete(m_zhandle, m_client_logkafka_id_path.c_str(), -1)) {
                LERROR << "Fail to delete persistent zookeeper path, "
                       << m_client_logkafka_id_path;
                return false;
            }
        } else { // ephemeral node
            /* if this process have never created client/logkafka_id node, 
             * it must be created by another process, we must quit */
            if (!m_registered) {
                LERROR << "This error may emerge in two situations: \n"
                       << "    1. Another logkafka process with the same id " 
                       << m_logkafka_id << " is running.\n"
                       << "    2. Last running process with the same id just stopped"
                       << " a moment ago, wait for " << (m_session_timeout_ms/1000) 
                       << " seconds and retry";
                exit(EXIT_FAILURE);
            }
        }
    }
    free(buf);

    if (ZOK != status) {
        LDEBUG << "Zookeeper get error, " << zerror(status);
        LDEBUG << "Creating zookeeper path, " << m_client_logkafka_id_path;
        if (zoo_create(m_zhandle, m_client_logkafka_id_path.c_str(), NULL, 0, &ZOO_OPEN_ACL_UNSAFE, 
                    ZOO_EPHEMERAL, NULL, 0) != ZOK)
        {
            LERROR << "Fail to create zookeeper path, " << m_client_logkafka_id_path;
            return false;
        }

        m_registered = true;
    }

    return true;
}/*}}}*/

bool Zookeeper::refreshLogConfig()
{/*{{{*/
    ScopedLock l(m_log_config_mutex);

    string log_config;
    if (!getZnodeData(m_config_logkafka_id_path, log_config)) {
        LERROR << "Fail to get log config";
        return false;
    }

    m_log_config = log_config;

    return true;
}/*}}}*/

bool Zookeeper::refreshBrokerUrls()
{/*{{{*/
    ScopedLock l(m_broker_urls_mutex);

    vector<string> ids;
    if (!getBrokerIds(ids)) {
        return false;
    }

    m_broker_urls = "";
    vector<string>::const_iterator iter;
    for (iter = ids.begin(); iter != ids.end(); ++iter) {
        string host, port;
        string bid = *iter; 
        if (!getBrokerIpAndPort(bid, host, port)) {
            return false;
        }

        if (iter != ids.begin()) {
            m_broker_urls.append(",");
        }

        m_broker_urls.append(host);
        m_broker_urls.append(":");
        m_broker_urls.append(port);
    }

    LDEBUG << "Broker urls: " << m_broker_urls.c_str();

    return true;
}/*}}}*/

string Zookeeper::getLogConfig()
{/*{{{*/
    ScopedLock l(m_log_config_mutex);
    return m_log_config;
}/*}}}*/

bool Zookeeper::ensurePathExist(const string& path)
{/*{{{*/
    if (NULL == m_zhandle) {
        LWARNING << "Zookeeper handle is NULL";
        return false;
    }

    if (ZOK == zoo_exists(m_zhandle, path.c_str(), 0, NULL)) {
        LINFO << "Zookeeper node " << path << " already exists";
        return true;
    }

    size_t start_pos = 1;
    size_t pos = 0;
    while ((pos = path.find('/', start_pos)) != string::npos) {
        string parent_path = path.substr(0, pos);
        start_pos = pos+1;
        zoo_create(m_zhandle, parent_path.c_str(), NULL, 0, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
    }

    int ret = zoo_create(m_zhandle, path.c_str(), NULL, 0, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
    if (ret != ZOK && ret != ZNODEEXISTS) {
        LERROR << "Create znode failed: " << path.c_str()
               << ", error: " << zerror(ret);
        return false;
    }

    return true;
}/*}}}*/

bool Zookeeper::getZnodeData(const string& path, string &data)
{/*{{{*/
    if (NULL == m_zhandle) {
        LWARNING << "Zookeeper handle is NULL";
        return false;
    }

    struct Stat stat;
    int status = ZOK;
    int len = 0;
    bool ret = false; 
    char *buf = NULL;

    status = zoo_exists(m_zhandle, path.c_str(), 0, &stat);
    len = (status == ZOK) ? stat.dataLength : ZNODE_BUF_MAX_LEN;

    buf = (char *)malloc(len + 1);
    bzero(buf, len + 1);
    status = zoo_get(m_zhandle, path.c_str(), 0, buf, &len, NULL);
    if (status == ZOK) {
        data = string(buf);
        ret = true;
    } else {
        LERROR << "Get znode error"
               << ", path: " << path 
               << ", error:%s" << zerror(status);
    }
    free(buf);

    return ret;
}/*}}}*/

bool Zookeeper::setWatcher(const string& path, 
        watcher_fn watcher, void *wctx)
{/*{{{*/
    LINFO << "Try to set watcher " << path.c_str();

    if (NULL == m_zhandle) {
        LWARNING << "Zookeeper handle is NULL";
        return false; 
    }

    if (!ensurePathExist(path)) {
        LERROR << "Create znode " << path;
        return false;
    }

    int len = 0;
    int ret;
    if (ZOK != (ret = zoo_wget(m_zhandle, path.c_str(), watcher, wctx, NULL, &len, NULL))) {
        LWARNING << "Set watcher failed: " << path;
        return false;
    }
    LINFO << "Set watcher success: " << path.c_str();

    return true;
}/*}}}*/

bool Zookeeper::setChildrenWatcher(const string& path, 
        watcher_fn watcher, void *wctx)
{/*{{{*/
    LINFO << "Try to set children watcher " << path.c_str();

    if (NULL == m_zhandle) {
        LWARNING << "Zookeeper handle is NULL";
        return false; 
    }

    if (!ensurePathExist(path)) {
        LERROR << "Create znode " << path.c_str();
        return false;
    }

    int ret = ZOK;
    if (ZOK != (ret = zoo_wget_children(m_zhandle, path.c_str(), watcher, wctx, NULL))) {
        LWARNING << "Set children watcher failed: " << path.c_str();
        return false;
    }
    LINFO << "Set children watcher success: " << path.c_str();
    return true;
}/*}}}*/

const char* Zookeeper::state2String(int state)
{/*{{{*/
    if (state == 0)
        return "CLOSED_STATE";
    if (state == ZOO_CONNECTING_STATE)
        return "CONNECTING_STATE";
    if (state == ZOO_ASSOCIATING_STATE)
        return "ASSOCIATING_STATE";
    if (state == ZOO_CONNECTED_STATE)
        return "CONNECTED_STATE";
    if (state == ZOO_EXPIRED_SESSION_STATE)
        return "EXPIRED_SESSION_STATE";
    if (state == ZOO_AUTH_FAILED_STATE)
        return "AUTH_FAILED_STATE";

    return "INVALID_STATE";
}/*}}}*/

const char* Zookeeper::event2String(int ev)
{/*{{{*/
    switch (ev) {
        case 0:
            return "ZOO_ERROR_EVENT";
        case CREATED_EVENT_DEF:
            return "ZOO_CREATED_EVENT";
        case DELETED_EVENT_DEF:
            return "ZOO_DELETED_EVENT";
        case CHANGED_EVENT_DEF:
            return "ZOO_CHANGED_EVENT";
        case CHILD_EVENT_DEF:
            return "ZOO_CHILD_EVENT";
        case SESSION_EVENT_DEF:
            return "ZOO_SESSION_EVENT";
        case NOTWATCHING_EVENT_DEF:
            return "ZOO_NOTWATCHING_EVENT";
    }
    return "INVALID_EVENT";
}/*}}}*/

void Zookeeper::globalWatcher(zhandle_t* zhandle, int type, 
        int state, const char* path, void* context)
{/*{{{*/
    LINFO << "Watcher event: " << event2String(type)
          << ", state: " << state2String(state)
          << ", path: " << path;

    if (NULL == context) {
        LWARNING << "Broker change watcher context is NULL";
        return;
    }

    Zookeeper *zookeeper = reinterpret_cast<Zookeeper *>(context);

    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE) {
            LINFO << "Connect to zookeeper successfully.";
        } 

        if (state == ZOO_AUTH_FAILED_STATE) {
            LERROR << "Authentication failure, shutting down...";
        } 

        if (state == ZOO_EXPIRED_SESSION_STATE) {
            LINFO << "Session expired, try to reconnect...";
            refresh((void*)zookeeper);
        }
    }
}/*}}}*/

void Zookeeper::brokerChangeWatcher(zhandle_t* zhandle, int type, 
        int state, const char* path, void* context)
{/*{{{*/
    LINFO << "Watcher event: " << event2String(type)
          << ", state: " << state2String(state)
          << ", path: " << path;

    if (NULL == context) {
        LWARNING << "Broker change watcher context is NULL";
        return;
    }

    Zookeeper *zookeeper = reinterpret_cast<Zookeeper *>(context);

    zookeeper->refreshWatchers();

    if (type != ZOO_SESSION_EVENT) { 
        zookeeper->refreshBrokerUrls();
    }
}/*}}}*/

void Zookeeper::configChangeWatcher(zhandle_t* zhandle, int type, 
        int state, const char* path, void* context)
{/*{{{*/
    LINFO << "Watcher event: " << event2String(type)
          << ", state: " << state2String(state)
          << ", path: " << path;

    if (NULL == context) {
        LWARNING << "Config change watcher context is NULL";
        return;
    }

    Zookeeper *zookeeper = reinterpret_cast<Zookeeper *>(context);

    zookeeper->refreshWatchers();

    if (type != ZOO_SESSION_EVENT) { 
        zookeeper->refreshLogConfig();
    }
}/*}}}*/

bool Zookeeper::getBrokerIds(vector<string>& ids)
{/*{{{*/
    if (NULL == m_zhandle) {
        LWARNING << "Zookeeper handle is NULL";
        return false; 
    }

    struct String_vector brokerids;

    int ret = zoo_get_children(m_zhandle, m_broker_ids_path.c_str(), 0, &brokerids);
    if (ret != ZOK) {
        LERROR << "Get children error"
               << ", path: " << m_broker_ids_path
               << ", error: " << zerror(ret);

        return false;
    } else {
        ids.clear();
        for (int i = 0; i < brokerids.count; ++i) {
            ids.push_back(brokerids.data[i]);
        }
        deallocate_String_vector(&brokerids);
        return true;
    }
}/*}}}*/

bool Zookeeper::getBrokerIpAndPort(const string& brokerid, 
        string& host, string& port)
{/*{{{*/
    string brokerid_info_path = m_broker_ids_path + "/" + brokerid;
    string brokerinfo;
    if (!getZnodeData(brokerid_info_path, brokerinfo)) {
        LERROR << "Fail to get broker info";
        return false;
    }

    /* 1. Parse a JSON text string to a document. */
    Document document;
    if (document.Parse<0>(brokerinfo.c_str()).HasParseError()) {
        LERROR << "Json parsing failed, json: " << brokerinfo;
        return false;
    }

    /* 2. Access values in document. */
    if (!document.IsObject()) {
        LERROR << "Document is not object, type: " << Json::TypeNames[document.GetType()];
        return false;
    }

    try {
        Json::getValue(document, "host", host);

        int port_int;
        Json::getValue(document, "port", port_int);
        port = int2Str(port_int);
    } catch (const JsonErr &err) {
        LERROR << "Json error: " << err;
        return false;
    }

    return true;
}/*}}}*/

string Zookeeper::getBrokerUrls()
{/*{{{*/
    ScopedLock l(m_broker_urls_mutex);
    return m_broker_urls;
}/*}}}*/

bool Zookeeper::setLogState( const char *buf, int buflen,
        stat_completion_t completion)
{/*{{{*/
    ScopedLock l(m_zhandle_mutex);

    if (NULL == m_zhandle) {
        LWARNING << "Zookeeper handle is NULL";
        return false;
    }

    int ret = ZOK;
    const char *client_path = m_client_logkafka_id_path.c_str();
    char *path = strndup(client_path, strlen(client_path));
    if (NULL == path) {
        LERROR << "Fail to strndup path " << client_path;
        return false;
    }

    /* NOTE: use zookeeper async set for not blocking the main loop */
    if ((ret = zoo_aset(m_zhandle, client_path, buf, buflen,
                    -1, completion, path)) != ZOK)
    {
        LERROR << "Fail to set znode, " << zerror(ret)
               << ", path: " << path
               << ", buf: " << buf
               << ", buflen: " << buflen;
        free(path);
        return false;
    }

    return true;
}/*}}}*/

} // namespace logkafka
