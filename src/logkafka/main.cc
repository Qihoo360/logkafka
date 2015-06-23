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
/**
 * Logkafka collect log file and send to kafka 0.8 
 * 
 * Manager get log file configs from connected zookeeper cluster,
 * then start tail watchers for collecting and sending accordingly.
 *
 * TailWatcher read log file, send lines to kafka, then record log
 * file position in position file.
 */

#include <iostream>

#include "logkafka/config.h"
#include "logkafka/logkafka.h"
#include "logkafka/option.h"

#include "easylogging/easylogging++.h"
_INITIALIZE_EASYLOGGINGPP
#define ELPP_THREAD_SAFE

using namespace std;
using namespace logkafka;

int main(int argc, char** argv)
{
    /* init option with args */
    Option opt(argc, argv);

    /* init easylogging */
    easyloggingpp::Configurations confFromFile(opt.easylogging_config_path);
    easyloggingpp::Loggers::reconfigureAllLoggers(confFromFile);
    easyloggingpp::Configurations defaultConf;
    defaultConf.setToDefault();
    easyloggingpp::Loggers::reconfigureLogger("business", defaultConf);

    /* init logkafka config */
    Config *lk_cfg = new Config();
    if (!lk_cfg->init(opt.logkafka_config_path.c_str())) {
        cout << "Fail to init logkafka config, please check log" << endl;
        delete lk_cfg;
        return EXIT_FAILURE;
    }

    /* init and start logkafka */
    LogKafka *lk = new LogKafka(lk_cfg);

    if (!lk->init()) {
        cout << "Fail to init logkafka, please check log" << endl;
        delete lk;
        return EXIT_FAILURE;
    }

    if (!lk->start()) {
        cout << "Fail to start logkafka, please check log" << endl;
        return EXIT_FAILURE;
    }

    delete lk;
    delete lk_cfg;

    return EXIT_SUCCESS;
}
