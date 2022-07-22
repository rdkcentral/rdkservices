/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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

#include "Telemetry.h"

#include "UtilsJsonRpc.h"
#include "UtilsTelemetry.h"

#include "rfcapi.h"

// Methods
#define TELEMETRY_METHOD_SET_REPORT_PROFILE_STATUS "setReportProfileStatus"
#define TELEMETRY_METHOD_LOG_APPLICATION_EVENT "logApplicationEvent"

#define RFC_CALLERID "Telemetry"
#define RFC_REPORT_PROFILES "Device.X_RDKCENTRAL-COM_T2.ReportProfiles"
#define RFC_REPORT_DEFAULT_PROFILE_ENABLE "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.FTUEReport.Enable"
#define T2_PERSISTENT_FOLDER "/opt/.t2reportprofiles/"
#define DEFAULT_PROFILES_FILE "/etc/t2profiles/default.json"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework
{

    namespace {

        static Plugin::Metadata<Plugin::Telemetry> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin
    {
        SERVICE_REGISTRATION(Telemetry, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        Telemetry* Telemetry::_instance = nullptr;

        Telemetry::Telemetry()
        : PluginHost::JSONRPC()
        {
            Telemetry::_instance = this;

            Register(TELEMETRY_METHOD_SET_REPORT_PROFILE_STATUS, &Telemetry::setReportProfileStatus, this);
            Register(TELEMETRY_METHOD_LOG_APPLICATION_EVENT, &Telemetry::logApplicationEvent, this);

            Utils::Telemetry::init();
        }

        Telemetry::~Telemetry()
        {

        }

        const string Telemetry::Initialize(PluginHost::IShell* service )
        {
            JsonObject config;
            config.FromString(service->ConfigLine());
            std::string t2PersistentFolder = config.HasLabel("t2PersistentFolder") ? config["t2PersistentFolder"].String() : T2_PERSISTENT_FOLDER;
            bool isEMpty = true;
            DIR *d = opendir(t2PersistentFolder.c_str());
            if (NULL != d)
            {
                struct dirent *de;

                while ((de = readdir(d)))
                {
                    if (0 == de->d_name[0] || 0 == strcmp(de->d_name, ".") || 0 == strcmp(de->d_name, ".."))
                        continue;

                    isEMpty = false;
                    break;
                }

                closedir(d);
            }

            if (isEMpty)
            {
                Core::File file;
                std::string defaultProfilesFile = config.HasLabel("defaultProfilesFile") ? config["defaultProfilesFile"].String() : DEFAULT_PROFILES_FILE;
                file = defaultProfilesFile.c_str();
                file.Open();
                if (file.IsOpen())
                {
                    if (file.Size() > 0)
                    {
                        std::vector <char> defaultProfile;
                        defaultProfile.resize(file.Size() + 1);
                        uint32_t rs = file.Read((uint8_t *)defaultProfile.data(), file.Size());
                        defaultProfile.data()[rs] = 0;
                        if (file.Size() == rs)
                        {

                            std::stringstream ss;
                            // Escaping quotes
                            for (uint32_t n = 0; n < rs; n++)
                            {
                                char ch = defaultProfile.data()[n];
                                if ('\"' == ch)
                                    ss << "\\";
                                ss << ch;
                            }

                            WDMP_STATUS wdmpStatus = setRFCParameter((char *)RFC_CALLERID, RFC_REPORT_PROFILES, ss.str().c_str(), WDMP_STRING);
                            if (WDMP_SUCCESS != wdmpStatus)
                            {
                                LOGERR("Failed to set Device.X_RDKCENTRAL-COM_T2.ReportProfiles: %d", wdmpStatus);
                            }
                        }
                        else
                        {
                            LOGERR("Got wrong number of bytes, %d instead of %d", rs, (int)file.Size());
                        }
                    }
                    else
                    {
                        LOGERR("%s is 0 size", defaultProfilesFile.c_str());
                    }
                }
                else
                {
                    LOGERR("Failed to open %s", defaultProfilesFile.c_str());
                }
            }
            return "";
        }

        void Telemetry::Deinitialize(PluginHost::IShell* /* service */)
        {
            Telemetry::_instance = nullptr;
        }

        uint32_t Telemetry::setReportProfileStatus(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            if (parameters.HasLabel("status"))
            {
                string status;
                getStringParameter("status", status);

                if (status != "STARTED" && status != "COMPLETE")
                {
                    LOGERR("Only the 'STARTED' or 'COMPLETE' status is allowed");
                    returnResponse(false);
                }

                WDMP_STATUS wdmpStatus = setRFCParameter((char *)RFC_CALLERID, RFC_REPORT_DEFAULT_PROFILE_ENABLE, status == "COMPLETE" ? "true" : "false", WDMP_BOOLEAN);
                if (WDMP_SUCCESS != wdmpStatus)
                {
                    LOGERR("Failed to set %s: %d", RFC_REPORT_DEFAULT_PROFILE_ENABLE, wdmpStatus);
                    returnResponse(false);
                }

                returnResponse(true);

            }
            else
            {
                LOGERR("No status' parameter");
                returnResponse(false);
            }

            returnResponse(false);
        }

        uint32_t Telemetry::logApplicationEvent(const JsonObject& parameters, JsonObject& response)
        {
            // Temporary disabled since current DCA format doesn't allow grepping log statements with json formatted strings
            //LOGINFOMETHOD();
            LOGINFO();

            if (parameters.HasLabel("eventName") && parameters.HasLabel("eventValue"))
            {
                string eventName;
                getStringParameter("eventName", eventName);

                string eventValue;
                getStringParameter("eventValue", eventValue);

                LOGINFO("eventName:%s, eventValue:%s", eventName.c_str(), eventValue.c_str());
                Utils::Telemetry::sendMessage((char *)eventName.c_str(), (char *)eventValue.c_str());
            }
            else
            {
                LOGERR("No 'eventName' or 'eventValue' parameter");
                returnResponse(false);
            }

            returnResponse(true);
        }

    } // namespace Plugin
} // namespace WPEFramework

