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
#ifndef BASE_JSON_H_
#define BASE_JSON_H_

#include <cstdio>
#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/writer.h" // for stringify JSON
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/stringbuffer.h" 

using namespace std;
using namespace rapidjson;

namespace base {

typedef std::string JsonErr;

class Json {

    public:
    static void getValue(const rapidjson::Value &obj, const char *name, int32_t &value);
    static void getValue(const rapidjson::Value &obj, const char *name, uint32_t &value);
    static void getValue(const rapidjson::Value &obj, const char *name, int64_t &value);
    static void getValue(const rapidjson::Value &obj, const char *name, uint64_t &value);
    static void getValue(const rapidjson::Value &obj, const char *name, string &value);
    static void getValue(const rapidjson::Value &obj, const char *name, bool &value);
    static string serialize(const Value &v);

    static const char** TypeNames;
};

} // namespace base 

#endif // BASE_JSON_H_
