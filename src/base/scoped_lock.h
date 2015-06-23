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
#ifndef BASE_SCOPEDLOCK_H_
#define BASE_SCOPEDLOCK_H_

#include "base/mutex.h"

namespace base {

class ScopedLock
{
    public:
        explicit ScopedLock(Mutex& m):m_mutex(m)
        {
            m_mutex.lock();
            m_locked= true;
        }

        ~ScopedLock()
        {
            if (m_locked) {
                m_mutex.unlock();
            }
        }

        void lock()
        {
            if (!m_locked) {
                m_mutex.lock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if (m_locked) {
                m_mutex.unlock();
                m_locked = false;
            }
        }

        bool locked() const
        {
            return m_locked;
        }

        Mutex& getMutex()
        {
            return m_mutex;
        }

    private:
        Mutex& m_mutex;
        bool m_locked;
};

} // namespace base

#endif // BASE_SCOPEDLOCK_H_
