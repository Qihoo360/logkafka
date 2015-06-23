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
#include "base/json.h"

#include "easylogging/easylogging++.h"

using namespace std;

namespace base {

static const char* kJsonTypeNames[]
    = { "Null", "False", "True", "Object", "Array", "String", "Number" };
const char** Json::TypeNames = kJsonTypeNames;

void Json::getValue(const rapidjson::Value &obj, const char *name, int32_t &value)
{/*{{{*/
    if (NULL == name) {
        throw JsonErr("name is NULL");
    }

    if (!obj.IsObject()) {
        throw JsonErr("obj is not a valid json object");
    }

    Value::ConstMemberIterator itr = obj.FindMember(name);
    if (itr != obj.MemberEnd()) {
        const Value &v = itr->value;
        if (v.IsInt()) {
            value = v.GetInt();
        } else {
            throw JsonErr("the value of " + string(name) + " is not int32_t");
        }
    } else {
        throw JsonErr("json object do not have name (" + string(name) + ")");
    }
}/*}}}*/

void Json::getValue(const rapidjson::Value &obj, const char *name, uint32_t &value)
{/*{{{*/
    if (NULL == name) {
        throw JsonErr("name is NULL");
    }

    if (!obj.IsObject()) {
        throw JsonErr("obj is not a valid json object");
    }

    Value::ConstMemberIterator itr = obj.FindMember(name);
    if (itr != obj.MemberEnd()) {
        const Value &v = itr->value;
        if (v.IsUint()) {
            value = v.GetUint();
        } else {
            throw JsonErr("the value of " + string(name) + " is not uint32_t");
        }
    } else {
        throw JsonErr("json object do not have name (" + string(name) + ")");
    }
}/*}}}*/

void Json::getValue(const rapidjson::Value &obj, const char *name, int64_t &value)
{/*{{{*/
    if (NULL == name) {
        throw JsonErr("name is NULL");
    }

    if (!obj.IsObject()) {
        throw JsonErr("obj is not a valid json object");
    }

    Value::ConstMemberIterator itr = obj.FindMember(name);
    if (itr != obj.MemberEnd()) {
        const Value &v = itr->value;
        if (v.IsInt64()) {
            value = v.GetInt64();
        } else {
            throw JsonErr("the value of " + string(name) + " is not int64_t");
        }
    } else {
        throw JsonErr("json object do not have name (" + string(name) + ")");
    }
}/*}}}*/

void Json::getValue(const rapidjson::Value &obj, const char *name, uint64_t &value)
{/*{{{*/
    if (NULL == name) {
        throw JsonErr("name is NULL");
    }

    if (!obj.IsObject()) {
        throw JsonErr("obj is not a valid json object");
    }

    Value::ConstMemberIterator itr = obj.FindMember(name);
    if (itr != obj.MemberEnd()) {
        const Value &v = itr->value;
        if (v.IsUint64()) {
            value = v.GetUint64();
        } else {
            throw JsonErr("the value of " + string(name) + " is not uint64_t");
        }
    } else {
        throw JsonErr("json object do not have name (" + string(name) + ")");
    }
}/*}}}*/

void Json::getValue(const rapidjson::Value &obj, const char *name, string &value)
{/*{{{*/
    if (NULL == name) {
        throw JsonErr("name is NULL");
    }

    if (!obj.IsObject()) {
        throw JsonErr("obj is not a valid json object");
    }

    Value::ConstMemberIterator itr = obj.FindMember(name);
    if (itr != obj.MemberEnd()) {
        const Value &v = itr->value;
        if (v.IsString()) {
            value = v.GetString();
        } else {
            throw JsonErr("the value of " + string(name) + " is not string");
        }
    } else {
        throw JsonErr("json object do not have name (" + string(name) + ")");
    }
}/*}}}*/

void Json::getValue(const rapidjson::Value &obj, const char *name, bool &value)
{/*{{{*/
    if (NULL == name) {
        throw JsonErr("name is NULL");
    }

    if (!obj.IsObject()) {
        throw JsonErr("obj is not a valid json object");
    }

    Value::ConstMemberIterator itr = obj.FindMember(name);
    if (itr != obj.MemberEnd()) {
        const Value &v = itr->value;
        if (v.IsBool()) {
            value = v.GetBool();
        } else {
            throw JsonErr("the value of " + string(name) + " is not bool");
        }
    } else {
        throw JsonErr("json object do not have name (" + string(name) + ")");
    }
}/*}}}*/

string Json::serialize(const Value &v)
{/*{{{*/
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    v.Accept(writer);
    const char* json = buffer.GetString();
    return string(json);
}/*}}}*/

} // namespace base 
