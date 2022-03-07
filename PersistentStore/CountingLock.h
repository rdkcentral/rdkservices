/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COUNTINGLOCK_H
#define COUNTINGLOCK_H

#include "Module.h"

namespace WPEFramework {
namespace Plugin {

class CountingLock
{
private:
    CountingLock(const CountingLock &) = delete;
    CountingLock &operator=(const CountingLock &) = delete;

public:
    CountingLock()
        : _lock(), _count(0)
    {
    }

    void Lock(unsigned int count)
    {
        _lock.Lock();

        while (_count > count);

        if (count != 0) {
            _count++;

            _lock.Unlock();
        }
    }

    void Unlock()
    {
        if (_count == 0) {
            _lock.Unlock();
        }
        else {
            _count--;
        }
    }

private:
    Core::CriticalSection _lock;
    std::atomic_uint _count;
};

class CountingLockSync
{
private:
    CountingLockSync() = delete;
    CountingLockSync(const CountingLockSync &) = delete;
    CountingLockSync &operator=(const CountingLockSync &) = delete;

public:
    explicit CountingLockSync(CountingLock &lock, unsigned int count = -1)
        : _lock(lock)
    {
        _lock.Lock(count);
    }

    ~CountingLockSync()
    {
        _lock.Unlock();
    }

private:
    CountingLock &_lock;
};

} // namespace Plugin
} // namespace WPEFramework

#endif //COUNTINGLOCK_H
