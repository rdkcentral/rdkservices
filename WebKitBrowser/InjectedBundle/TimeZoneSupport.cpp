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
 
#include <syslog.h>

#include "TimeZoneSupport.h"

#ifdef TZ_FILE
const char *kTimeZoneFile = TZ_FILE;
#else
const char *kTimeZoneFile = "/opt/persistent/timeZoneDST";
#endif

namespace WPEFramework {

namespace TZ {

    void trim(std::string &str) {
        if (str.empty())
            return;

        gchar *tmp = g_strdup(str.c_str());
        str = std::string(g_strstrip(tmp));
        g_free(tmp);
    }

    void TimeZoneSupport::HandleTimeZoneFileUpdate(GFileMonitor *monitor, GFile *file, GFile *other, GFileMonitorEvent evtype, gpointer user_data) {
        if (evtype == G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT) {
            TimeZoneSupport *tzSupport = (TimeZoneSupport*)user_data;
            gchar *content = nullptr;
            gsize length = 0;

            auto result = g_file_load_contents(file, nullptr, &content, &length, nullptr, nullptr);
            if(result && content && length > 0) {
                std::string timeZone(content);
                trim(timeZone);

                if (timeZone != tzSupport->_previousTimeZone) {
                    tzSupport->_previousTimeZone = timeZone;
                    timeZone = ":" + timeZone;
                    SYSLOG(Trace::Information, (_T("TimeZone is updated to \"%s\" from \"%s\" file"), timeZone.c_str(), tzSupport->_tzFile.c_str()));
                    Core::SystemInfo::SetEnvironment(_T("TZ"), _T(timeZone), true);
                    tzset();
                }

                g_free(content);
            }
        }
    }

    TimeZoneSupport::TimeZoneSupport()
        : _timeZoneFileMonitor(nullptr)
        , _timeZoneFileMonitorId(0)
        , _previousTimeZone()
        , _tzFile(kTimeZoneFile)
    {
        trim(_tzFile);
    }

    void TimeZoneSupport::Initialize()
    {
        if(_tzFile.empty()) {
            SYSLOG(Trace::Warning, (_T("Invalid file input for TZ update")));
            return;
        }

        Core::SystemInfo::GetEnvironment(std::string(_T("TZ")), _previousTimeZone);
        trim(_previousTimeZone);

        GFile *file = g_file_new_for_path(_tzFile.c_str());
        if(g_file_query_exists(file, nullptr))
            HandleTimeZoneFileUpdate(nullptr, file, nullptr, G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT, this);

        _timeZoneFileMonitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE, nullptr, nullptr);
        _timeZoneFileMonitorId = g_signal_connect(_timeZoneFileMonitor, "changed", reinterpret_cast<GCallback>(HandleTimeZoneFileUpdate), this);
        SYSLOG(Trace::Information, (_T("Installed file monitor for \"%s\""), _tzFile.c_str()));

        g_object_unref(file);
    }

    void TimeZoneSupport::Deinitialize()
    {
        if (_timeZoneFileMonitor) {
            if (_timeZoneFileMonitorId > 0)
                g_signal_handler_disconnect(_timeZoneFileMonitor, _timeZoneFileMonitorId);

            g_object_unref(_timeZoneFileMonitor);
            _timeZoneFileMonitor = nullptr;
            _timeZoneFileMonitorId = 0;
        }
    }
}
}
