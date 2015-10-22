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
#ifndef LOGKAFKA_OPTION_H_
#define LOGKAFKA_OPTION_H_

#include <string>

using namespace std;

namespace logkafka {

class Option 
{
    public:
        Option();
        Option(int argc, char *argv[]);
        void parseArgs(int argc, char *argv[], Option &option);

    public:
        string logkafka_config_path;
        string easylogging_config_path;
        bool daemon;
};

} // namespace logkafka

#endif // LOGKAFKA_OPTION_H_ 
