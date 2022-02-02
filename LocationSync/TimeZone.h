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

#ifndef RDKSERVICES_TIMEZONE_H
#define RDKSERVICES_TIMEZONE_H

#include "Module.h"
#include <interfaces/ITimeZone.h>

namespace WPEFramework {
namespace Plugin {

    class TimeZone : public Exchange::ITimeZone {
    private:
        TimeZone(const TimeZone &) = delete;
        TimeZone &operator=(const TimeZone &) = delete;

    public:
        TimeZone() = default;
        virtual ~TimeZone() = default;

        void Synchronize(const string &file);

    public:
        // ITimeZone methods

        virtual uint32_t Register(Exchange::ITimeZone::INotification *notification) override;
        virtual uint32_t Unregister(Exchange::ITimeZone::INotification *notification) override;
        virtual uint32_t GetTimeZone(string &timeZone) const override;
        virtual uint32_t SetTimeZone(const string &timeZone) override;

        BEGIN_INTERFACE_MAP(TimeZone)
        INTERFACE_ENTRY(Exchange::ITimeZone)
        END_INTERFACE_MAP

    private:
        void TimeZoneChanged(const string &timeZone);

    private:
        string _file;
        mutable Core::CriticalSection _adminLock;
        std::list<Exchange::ITimeZone::INotification *> _clients;
    };

} // namespace Plugin
} // namespace WPEFramework

#endif //RDKSERVICES_TIMEZONE_H
