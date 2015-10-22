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
#include "logkafka/option.h"

#include <cstdlib>

#include <libgen.h>
#include <tclap/CmdLine.h>

using namespace std;

namespace logkafka {

Option::Option()
    : logkafka_config_path(""),
      easylogging_config_path(""),
      daemon(false)
{/*{{{*/
}/*}}}*/

Option::Option(int argc, char *argv[])
    : logkafka_config_path(""),
      easylogging_config_path(""),
      daemon(false)
{/*{{{*/
    parseArgs(argc, argv, *this); 
}/*}}}*/

void Option::parseArgs(int argc, char *argv[], Option &option)
{/*{{{*/
    using namespace TCLAP;
    const std::string prog_name = basename(argv[0]);
    std::vector<const char *> arg_vec(&argv[0], &argv[0] + argc);
    arg_vec[0] = prog_name.c_str();
    try {
        CmdLine cmd("Producer daemon for Apache Kafka 0.8", ' ', " ");

        ValueArg<std::string> arg_config_path("f", "logkafka_config_path", 
                "Pathname of logkafka config file.", true, 
                option.logkafka_config_path, "LOGKAFKA_CONFIG_PATH");
        cmd.add(arg_config_path);

        ValueArg<std::string> arg_easylogging_config_path("e", "easylogging_config_path", 
                "Pathname of easylogging config file.", true, 
                option.easylogging_config_path, "EASYLOGGING_CONFIG_PATH");
        cmd.add(arg_easylogging_config_path);

        SwitchArg arg_daemon("d", "daemon", "Run as a daemon.", cmd, false, NULL);

        cmd.parse(argc, &arg_vec[0]);

        option.logkafka_config_path = arg_config_path.getValue();
        option.easylogging_config_path = arg_easylogging_config_path.getValue();
        option.daemon = arg_daemon.getValue();
    } catch (const ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(EXIT_FAILURE);
    }
}/*}}}*/

} // namespace logkafka
