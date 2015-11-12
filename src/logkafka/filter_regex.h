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
#ifndef LOGKAFKA_FILTER_REGEX_H_
#define LOGKAFKA_FILTER_REGEX_H_

#include <string>
#include <vector>

#include "logkafka/filter.h"
#include "logkafka/task_conf.h"

#include "pcre2.h"

using namespace std;

namespace logkafka {

class FilterRegex: public virtual Filter
{
    public:
        FilterRegex(): Filter() {};
        FilterRegex(FilterConf filter_conf): 
            m_filter_conf(filter_conf) { m_re = NULL; };
        virtual ~FilterRegex() {
            pcre2_code_free(m_re); m_re = NULL;
        };
        bool init(void *arg);
        bool filter(void *arg, vector<string> &lines);

    private:
        FilterConf m_filter_conf;
        pcre2_code *m_re;
};

} // namespace logkafka

#endif // LOGKAFKA_FILTER_REGEX_H_
