/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */
 
#pragma once

#include "Module.h"

#ifndef __WINDOWS__
#include <syslog.h>
#endif

namespace WPEFramework {
namespace Plugin {

    class TraceOutput : public Trace::ITraceMedia {
    public:
        TraceOutput(const TraceOutput&) = delete;
        TraceOutput& operator=(const TraceOutput&) = delete;

        TraceOutput(const bool syslogging, const bool abbreviated)
            : _syslogging(syslogging)
            , _abbreviated(abbreviated)
        {
        }
        virtual ~TraceOutput()
        {
        }

    public:
        virtual void Output(const char fileName[], const uint32_t lineNumber, const char className[], const Trace::ITrace* information)
        {
#ifndef __WINDOWS__
            if (_syslogging == true) {
                if( _abbreviated == true ) {
                    string time(Core::Time::Now().ToTimeOnly(true));
                    syslog(LOG_NOTICE, "[%s]: %s\n", time.c_str(), information->Data());
                } else {
                    string time(Core::Time::Now().ToRFC1123(true));
                    syslog(LOG_NOTICE, "[%s]:[%s]:[%s:%d]: %s: %s\n", time.c_str(), information->Module(), Core::FileNameOnly(fileName), lineNumber, information->Category(), information->Data());
                }
            } else
#endif
            {
                if( _abbreviated == true ) {
                    string time(Core::Time::Now().ToTimeOnly(true));
                    printf("[%s]: %s\n", time.c_str(), information->Data());
                } else {
                    string time(Core::Time::Now().ToRFC1123(true));
                    printf("[%s]:[%s]:[%s:%d] %s: %s\n", time.c_str(), information->Module(), Core::FileNameOnly(fileName), lineNumber, information->Category(), information->Data());
                }
            }
        }

    private:
        bool _syslogging;
        bool _abbreviated;
    };
}
}
