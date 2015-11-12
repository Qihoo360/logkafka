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
#ifndef LOGKAFKA_FILTER_H_
#define LOGKAFKA_FILTER_H_

#include <string>
#include <vector>

using namespace std;

namespace logkafka {

class Filter
{
    public:
        Filter() {};
        virtual ~Filter() {};
        virtual bool init(void *arg) = 0;
        virtual bool filter(void *arg, vector<string> &lines) = 0;
};

} // namespace logkafka

#endif // LOGKAFKA_FILTER_H_
