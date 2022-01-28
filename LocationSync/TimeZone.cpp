/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
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
**/

#include "TimeZone.h"

namespace WPEFramework {
namespace Plugin {

    void TimeZone::Synchronize(const string &file) {
        Core::SafeSyncType <Core::CriticalSection> lock(_adminLock);

        _file = file;

        auto timeZone = GetTimeZone();

        if (timeZone.empty() == false) {
            TimeZoneChanged(timeZone);
        }
    }

    void TimeZone::Register(Exchange::ITimeZone::INotification *notification) {
        Core::SafeSyncType <Core::CriticalSection> lock(_adminLock);

        ASSERT(std::find(_clients.begin(), _clients.end(), notification) == _clients.end());

        notification->AddRef();
        _clients.push_back(notification);
    }

    void TimeZone::Unregister(Exchange::ITimeZone::INotification *notification) {
        Core::SafeSyncType <Core::CriticalSection> lock(_adminLock);

        std::list<Exchange::ITimeZone::INotification *>::iterator
            index(std::find(_clients.begin(), _clients.end(), notification));

        ASSERT(index != _clients.end());

        if (index != _clients.end()) {
            notification->Release();
            _clients.erase(index);
        }
    }

    string TimeZone::GetTimeZone() {
        string result;

        Core::SafeSyncType <Core::CriticalSection> lock(_adminLock);

        if (_file.empty() == false) {
            Core::File file(_file);

            if (file.Exists() && file.Open(true)) {
                const uint32_t bufLen = 16;

                uint32_t len;
                char buffer[bufLen];

                do {
                    len = file.Read(reinterpret_cast<uint8_t *>(buffer), bufLen);

                    if (len > 0) {
                        result.append(buffer, len);
                    }
                } while (len == bufLen);
            }
        }

        return result;
    }

    bool TimeZone::SetTimeZone(const string &timeZone) {
        bool result = false;

        Core::SafeSyncType <Core::CriticalSection> lock(_adminLock);

        if (_file.empty() == false) {
            Core::File file(_file);

            if (file.Create()) {
                auto bufLen = timeZone.size();

                auto len = file.Write(
                    reinterpret_cast<const uint8_t *>(timeZone.data()), bufLen);

                if (len == bufLen) {
                    TimeZoneChanged(timeZone);
                    result = true;
                }
            }
        }

        return result;
    }

    void TimeZone::TimeZoneChanged(const string &timeZone) {
        // runs always in the context of the adminLock

        std::list<Exchange::ITimeZone::INotification *>::iterator
            index(_clients.begin());

        while (index != _clients.end()) {
            (*index)->TimeZoneChanged(timeZone);
            index++;
        }
    }

} // namespace Plugin
} // namespace WPEFramework
