#include <signal.h>
#include <stdio.h>

#include <iostream>
#include <string>

#include "gtest/gtest.h"

#define private public  // hack complier
#define protected public

#include "logkafka/manager.h"
#include "logkafka/common.h"
#include "logkafka/config.h"
#include "logkafka/producer.h"
#include "logkafka/option.h"
#include "logkafka/zookeeper.h"

#include "easylogging/easylogging++.h"
_INITIALIZE_EASYLOGGINGPP
#define ELPP_THREAD_SAFE

#undef private
#undef protected

using namespace std;
using namespace logkafka;

// The fixture for testing class Project1. From google test primer.
class ManagerTest : public ::testing::Test {
protected:
        // You can remove any or all of the following functions if its body
        // is empty.

        ManagerTest() {
                // You can do set-up work for each test here.
            Config *lk_cfg = new Config();
            lk_cfg->init("");
            lk_cfg->pos_path = "/tmp/pos.logkafka";
            unlink(lk_cfg->pos_path.c_str());
            lk_cfg->zookeeper_connect = "127.0.0.1:2181";
            lk_cfg->line_max_bytes = 1048576;

            Option opt;
            opt.easylogging_config_path = "../conf/easylogging.conf";

            /* init easylogging */
            easyloggingpp::Configurations confFromFile(opt.easylogging_config_path);  // Load configuration from file
            easyloggingpp::Loggers::reconfigureAllLoggers(confFromFile); // Re-configures all the loggers to current configuration file
            easyloggingpp::Configurations defaultConf;
            defaultConf.setToDefault();
            easyloggingpp::Loggers::reconfigureLogger("business", defaultConf); // Only business logger uses default configurations
            
            g_manager= new Manager(lk_cfg);
            EXPECT_TRUE(g_manager->init(NULL));

            EXPECT_NE((void*)NULL, g_manager);
            EXPECT_NE((void*)NULL, g_manager->m_zookeeper);
            EXPECT_NE((void*)NULL, g_manager->m_pos_file);
        }

        virtual ~ManagerTest() {
                // You can do clean-up work that doesn't throw exceptions here.
            g_manager->stop();
            sleep(1);
            unlink(g_manager->m_config->pos_path.c_str());
            delete g_manager;
        }

        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        virtual void SetUp() {
                // Code here will be called immediately after the constructor (right
                // before each test).
        }

        virtual void TearDown() {
                // Code here will be called immediately after each test (right
                // before the destructor).
        }

public:
        int generateFile(int lines, string& filename)
        {
            ofstream of(filename.c_str(), ios::app);
            if (!of) return -1;
            int c = lines;
            
             while (c > 0) {
                 of << "2015-04-15 23:59:57 807 INFO  CommandController - api response to 112.97.36.66, cmd:router.ustartspeed, output:{u\"errcode\":0,\"d\":2,\"ack\":\"router.ustartspeed\"}" << endl;
                 c--;
             }
            of.close();
        }

        // Objects declared here can be used by all tests in the test case for Project1.
        Manager* g_manager;
};

// Test case must be called the class above
// Also note: use TEST_F instead of TEST to access the test fixture (from google test primer)
////TEST_F(ManagerTest, ManagerGetTaskConfig) {
////        EXPECT_TRUE(g_manager->getTaskConfig());
////}
////
////TEST_F(ManagerTest, ManagerReconn) {
////        EXPECT_TRUE(g_manager->resetZookeeper());
////}
////
////TEST_F(ManagerTest, ManagerStartSyncTask) {
////        bool is_task_started = g_manager->start();
////        sleep(1);
////        g_manager->doRoutineWork();
////        EXPECT_TRUE(is_task_started);
////}
////
////TEST_F(ManagerTest, RunNoneCompression) {
////        LogConf log_conf;
////        log_conf.log_path = "/tmp/logkafka_unittest_noneCompression.txt";
////        log_conf.follow_last = 1;
////        log_conf.batchsize= 1000;
////
////        KafkaConf kafka_conf;
////        kafka_conf.brokers = "";
////        kafka_conf.topic = "test";
////        kafka_conf.compression_codec = "none";
////        kafka_conf.required_acks = 1;
////        kafka_conf.key = "";
////        kafka_conf.partition = -1;
////
////        TaskConf task;
////        task.valid = true;
////        task.log_conf = log_conf;
////        task.kafka_conf = kafka_conf;
////        generateFile(100, log_conf.log_path);
////
////        Worker *collect_thread = new Worker(task, *g_manager->m_gdbm, 
////                *g_manager->getProduerInstance(task.kafka_conf.compression_codec),
////                g_manager->m_config);
////        EXPECT_NE((void*)NULL, collect_thread);
////        EXPECT_TRUE(collect_thread->run());
////        sleep(1);
////        EXPECT_TRUE(collect_thread->stop());
////        // unlink(log_conf.log_path.c_str());
////}
////
////
/////* will collect from 100 */
////TEST_F(ManagerTest, RundoUnfinished) {
////        LogConf log_conf;
////        log_conf.log_path = "/tmp/logkafka_unittest_noneCompression.txt";
////        log_conf.follow_last = 1;
////        log_conf.batchsize= 1000;
////
////        KafkaConf kafka_conf;
////        kafka_conf.brokers = "";
////        kafka_conf.topic = "test";
////        kafka_conf.compression_codec = "none";
////        kafka_conf.required_acks = 1;
////        kafka_conf.key = "";
////        kafka_conf.partition = -1;
////
////        TaskConf task;
////        task.valid = true;
////        task.log_conf = log_conf;
////        task.kafka_conf = kafka_conf;
////        generateFile(200, log_conf.log_path);
////        
////        Worker *collect_thread = new Worker(task, *g_manager->m_gdbm, 
////                *g_manager->getProduerInstance(task.kafka_conf.compression_codec),
////                g_manager->m_config);
////        EXPECT_NE((void*)NULL, collect_thread);
////
////        EXPECT_TRUE(collect_thread->run());
////        sleep(1);
////        EXPECT_TRUE(collect_thread->stop());
////        unlink(log_conf.log_path.c_str());
////}
////
////TEST_F(ManagerTest, RunFileRotate) {
////        LogConf log_conf;
////        log_conf.log_path = "/tmp/logkafka_unittest_noneCompression.txt";
////        string bak_filename = "/tmp/logkafka_unittest_noneCompression.txt.bak";
////        log_conf.follow_last = 1;
////        log_conf.batchsize= 1000;
////
////        KafkaConf kafka_conf;
////        kafka_conf.brokers = "";
////        kafka_conf.topic = "test";
////        kafka_conf.compression_codec = "none";
////        kafka_conf.required_acks = 1;
////        kafka_conf.key = "";
////        kafka_conf.partition = -1;
////
////        TaskConf task;
////        task.valid = true;
////        task.log_conf = log_conf;
////        task.kafka_conf = kafka_conf;
////        generateFile(100, log_conf.log_path);
////
////        Worker *collect_thread = new Worker(task, *g_manager->m_gdbm, 
////                *g_manager->getProduerInstance(task.kafka_conf.compression_codec),
////                g_manager->m_config);
////        EXPECT_NE((void*)NULL, collect_thread);
////        EXPECT_TRUE(collect_thread->run());
////        sleep(1);
////
////
////        // mv old file
////        char cmd[256] = {'\0'};
////        sprintf(cmd, "mv %s %s", log_conf.log_path.c_str(), bak_filename.c_str());
////        cout << cmd << endl;
////        system(cmd);
////
////        // create new file
////        generateFile(100, log_conf.log_path);
////        sleep(10);
////
////        EXPECT_TRUE(collect_thread->stop());
////        unlink(log_conf.log_path.c_str());
////        unlink(bak_filename.c_str());
////}
////
////
////TEST_F(ManagerTest, RunGzipCompression) {
////        LogConf log_conf;
////        log_conf.log_path = "/tmp/logkafka_unittest_gzipCompression.txt";
////        log_conf.follow_last = 1;
////        log_conf.batchsize= 1000;
////
////        KafkaConf kafka_conf;
////        kafka_conf.brokers = "";
////        kafka_conf.topic = "test";
////        kafka_conf.compression_codec = "gzip";
////        kafka_conf.required_acks = 1;
////        kafka_conf.key = "";
////        kafka_conf.partition = -1;
////
////        TaskConf task;
////        task.valid = true;
////        task.log_conf = log_conf;
////        task.kafka_conf = kafka_conf;
////        generateFile(1000, log_conf.log_path);
////        cout << "logkafka_unittest_gzipCompression: " << g_manager->m_gdbm->fetch(log_conf.log_path) << endl;
////
////        Worker *collect_thread = new Worker(task, *g_manager->m_gdbm, 
////                *g_manager->getProduerInstance(task.kafka_conf.compression_codec),
////                g_manager->m_config);
////        EXPECT_NE((void*)NULL, collect_thread);
////        EXPECT_TRUE(collect_thread->run());
////        sleep(10);
////        EXPECT_TRUE(collect_thread->stop());
////        unlink(log_conf.log_path.c_str());
////        cout << "logkafka_unittest_gzipCompression: " << g_manager->m_gdbm->fetch(log_conf.log_path) << endl;
////}
////
////TEST_F(ManagerTest, RunSnappyCompression) {
////        LogConf log_conf;
////        log_conf.log_path = "/tmp/logkafka_unittest_snappyCompression.txt";
////        log_conf.follow_last = 1;
////        log_conf.batchsize= 1000;
////
////        KafkaConf kafka_conf;
////        kafka_conf.brokers = "";
////        kafka_conf.topic = "test";
////        kafka_conf.compression_codec = "snappy";
////        kafka_conf.required_acks = 1;
////        kafka_conf.key = "";
////        kafka_conf.partition = -1;
////
////        TaskConf task;
////        task.valid = true;
////        task.log_conf = log_conf;
////        task.kafka_conf = kafka_conf;
////        generateFile(1000, log_conf.log_path);
////
////        Worker *collect_thread = new Worker(task, *g_manager->m_gdbm,
////                *g_manager->getProduerInstance(task.kafka_conf.compression_codec),
////                g_manager->m_config);
////        EXPECT_NE((void*)NULL, collect_thread);
////        EXPECT_TRUE(collect_thread->run());
////        sleep(10);
////        EXPECT_TRUE(collect_thread->stop());
////        unlink(log_conf.log_path.c_str());
////}
