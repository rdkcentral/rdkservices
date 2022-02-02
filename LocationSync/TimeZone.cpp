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

        string timeZone;

        if ((GetTimeZone(timeZone) == Core::ERROR_NONE) &&
            (timeZone.empty() == false)) {
            TimeZoneChanged(timeZone);
        }
    }

    uint32_t TimeZone::Register(Exchange::ITimeZone::INotification *notification) {
        Core::SafeSyncType <Core::CriticalSection> lock(_adminLock);

        ASSERT(std::find(_clients.begin(), _clients.end(), notification) == _clients.end());

        notification->AddRef();
        _clients.push_back(notification);

        return Core::ERROR_NONE;
    }

    uint32_t TimeZone::Unregister(Exchange::ITimeZone::INotification *notification) {
        Core::SafeSyncType <Core::CriticalSection> lock(_adminLock);

        std::list<Exchange::ITimeZone::INotification *>::iterator
            index(std::find(_clients.begin(), _clients.end(), notification));

        ASSERT(index != _clients.end());

        if (index != _clients.end()) {
            notification->Release();
            _clients.erase(index);
        }

        return Core::ERROR_NONE;
    }

    uint32_t TimeZone::GetTimeZone(string &timeZone) const {
        uint32_t result;

        Core::SafeSyncType <Core::CriticalSection> lock(_adminLock);

        if (_file.empty() == true) {
            result = Core::ERROR_ILLEGAL_STATE;
        } else {
            Core::File file(_file);

            if ((file.Exists() == false) || (file.Open(true) == false)) {
                result = Core::ERROR_NOT_EXIST;
            } else {
                timeZone.clear();

                const uint32_t bufLen = 16;

                uint32_t len;
                char buffer[bufLen];

                do {
                    len = file.Read(reinterpret_cast<uint8_t *>(buffer), bufLen);

                    if (len > 0) {
                        timeZone.append(buffer, len);
                    }
                } while (len == bufLen);

                result = Core::ERROR_NONE;
            }
        }

        return result;
    }

    uint32_t TimeZone::SetTimeZone(const string &timeZone) {
        uint32_t result;

        Core::SafeSyncType <Core::CriticalSection> lock(_adminLock);

        if (_file.empty() == true) {
            result = Core::ERROR_ILLEGAL_STATE;
        } else {
            Core::File file(_file);

            if (file.Create() == false) {
                result = Core::ERROR_NOT_EXIST;
            } else {
                auto bufLen = timeZone.size();

                auto len = file.Write(
                    reinterpret_cast<const uint8_t *>(timeZone.data()), bufLen);

                if (len != bufLen) {
                    result = Core::ERROR_WRITE_ERROR;
                } else {
                    TimeZoneChanged(timeZone);

                    result = Core::ERROR_NONE;
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
