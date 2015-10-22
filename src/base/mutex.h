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
#ifndef BASE_MUTEX_H_
#define BASE_MUTEX_H_

#include <errno.h> 
#include <pthread.h>

namespace base {

class Mutex
{
    public:
        Mutex()
        {
            if ((pthread_mutex_init(&m_lock, NULL) != 0)) {
                throw "init spin lock failed!!!";
            }
        }

        ~Mutex()
        {
            pthread_mutex_destroy(&m_lock);
        }

        void lock()
        {
            pthread_mutex_lock(&m_lock);
        }

        void unlock()
        {
            pthread_mutex_unlock(&m_lock);
        }

        pthread_mutex_t& mutex()
        {
            return m_lock;
        }

    private:
        pthread_mutex_t m_lock;
};

} // namespace base

#endif // BASE_MUTEX_H_
