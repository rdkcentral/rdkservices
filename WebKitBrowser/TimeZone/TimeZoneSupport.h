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
 
#ifndef __TIMEZONESUPPORTH
#define __TIMEZONESUPPORTH

#include "Module.h"
#include <gio/gio.h>

namespace WPEFramework {
namespace TZ {
    class TimeZoneSupport {
    public:
        TimeZoneSupport();
        void Initialize();
        void Deinitialize();
    
    private:
        static void HandleTimeZoneFileUpdate(GFileMonitor *monitor, GFile *file, GFile *other, GFileMonitorEvent evtype, gpointer user_data);

    private:
        GFileMonitor *_timeZoneFileMonitor;
        gulong _timeZoneFileMonitorId;
        std::string _previousTimeZone;
        std::string _tzFile;
    };
}
}

#endif // __TIMEZONESUPPORTH
