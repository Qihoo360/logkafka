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
#include "logkafka/filter_regex.h"

namespace logkafka {

bool FilterRegex::init(void *arg)
{/*{{{*/
    PCRE2_SIZE erroffset;
    int errorcode;

    if (m_filter_conf.regex_filter_pattern == "") {
        LINFO << "Regex filter pattern is not set";
        return false;
    }

    PCRE2_SPTR pattern = (PCRE2_SPTR)(m_filter_conf.regex_filter_pattern.c_str());

    m_re = pcre2_compile(pattern, -1, 0, &errorcode, &erroffset, NULL);
    if (m_re == NULL) {
        PCRE2_UCHAR8 buffer[120];
        (void)pcre2_get_error_message(errorcode, buffer, 120);
        /* Handle error */
        LERROR << "Fail to compile pattern, " << buffer;
        return false;
    }

    return true;
}/*}}}*/

bool FilterRegex::filter(void *arg, vector<string> &lines)
{/*{{{*/
    FilterRegex *fr = reinterpret_cast<FilterRegex *>(arg);

    if (NULL == fr) {
        LERROR << "Regex filter is NULL";
        return false;
    }

    pcre2_code *re = fr->m_re;
    pcre2_match_data *match_data;
    int rc;

    match_data = pcre2_match_data_create(20, NULL);

    vector<string>::iterator iter;

    for (iter = lines.begin(); iter != lines.end(); ) {
        string line = *iter;
        PCRE2_SPTR value = (PCRE2_SPTR)line.c_str(); 
        rc = pcre2_match(re, value, -1, 0, 0, match_data, NULL);
        if (rc > 0) {
            LDEBUG << "Regex filter drop line: " << line;
            lines.erase(iter);
        } else {
            ++iter;
        }
    }

    pcre2_match_data_free(match_data);

    return true;
}/*}}}*/

} // namespace logkafka
