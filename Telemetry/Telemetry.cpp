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

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
#include "pwrMgr.h"
#include "UtilsIarm.h"
#endif /* USE_IARMBUS || USE_IARM_BUS */


#ifdef HAS_RBUS
#include "rbus.h"

#define RBUS_COMPONENT_NAME "TelemetryThunderPlugin"
#define T2_ON_DEMAND_REPORT "Device.X_RDKCENTRAL-COM_T2.UploadDCMReport"
#define T2_ABORT_ON_DEMAND_REPORT "Device.X_RDKCENTRAL-COM_T2.AbortDCMReport"
#endif

// Methods
#define TELEMETRY_METHOD_SET_REPORT_PROFILE_STATUS "setReportProfileStatus"
#define TELEMETRY_METHOD_LOG_APPLICATION_EVENT "logApplicationEvent"
#define TELEMETRY_METHOD_UPLOAD_REPORT "uploadReport"
#define TELEMETRY_METHOD_ABORT_REPORT "abortReport"

#define TELEMETRY_METHOD_EVT_ON_REPORT_UPLOAD "onReportUpload"


#define RFC_CALLERID "Telemetry"
#define RFC_REPORT_PROFILES "Device.X_RDKCENTRAL-COM_T2.ReportProfiles"
#define RFC_REPORT_DEFAULT_PROFILE_ENABLE "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.FTUEReport.Enable"
#define T2_PERSISTENT_FOLDER "/opt/.t2reportprofiles/"
#define DEFAULT_PROFILES_FILE "/etc/t2profiles/default.json"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 2
#define API_VERSION_NUMBER_PATCH 2

#ifdef HAS_RBUS
static rbusError_t rbusHandleStatus = RBUS_ERROR_NOT_INITIALIZED;
static rbusHandle_t rbusHandle;
#endif

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
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
        static void _powerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

        Telemetry::Telemetry()
        : PluginHost::JSONRPC()
        {
            Telemetry::_instance = this;

            Register(TELEMETRY_METHOD_SET_REPORT_PROFILE_STATUS, &Telemetry::setReportProfileStatus, this);
            Register(TELEMETRY_METHOD_LOG_APPLICATION_EVENT, &Telemetry::logApplicationEvent, this);
            Register(TELEMETRY_METHOD_UPLOAD_REPORT, &Telemetry::uploadReport, this);
            Register(TELEMETRY_METHOD_ABORT_REPORT, &Telemetry::abortReport, this);

            Utils::Telemetry::init();
        }

        Telemetry::~Telemetry()
        {

        }

        const string Telemetry::Initialize(PluginHost::IShell* service )
        {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            InitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

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
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            DeinitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

            Telemetry::_instance = nullptr;
#ifdef HAS_RBUS
            if (RBUS_ERROR_SUCCESS == rbusHandleStatus)
            {
                rbus_close(rbusHandle);
                rbusHandleStatus = RBUS_ERROR_NOT_INITIALIZED;
            }
#endif
        }

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
        void Telemetry::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, _powerEventHandler));
            }
        }

        void Telemetry::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED,_powerEventHandler) );
            }
        }

        void _powerEventHandler(const char *owner, IARM_EventId_t eventId,
                void *data, size_t len)
        {
            if (IARM_BUS_PWRMGR_EVENT_MODECHANGED == eventId)
            {
                if (nullptr == Telemetry::_instance)
                {
                    LOGERR("Telemetry::_instance is NULL.\n");
                    return;
                }

                IARM_Bus_PWRMgr_EventData_t *eventData = (IARM_Bus_PWRMgr_EventData_t *)data;

                if (IARM_BUS_PWRMGR_POWERSTATE_STANDBY == eventData->data.state.newState ||
                      IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP == eventData->data.state.newState)
                {
                    if (IARM_BUS_PWRMGR_POWERSTATE_ON == eventData->data.state.curState)
                    {
                        Telemetry::_instance->UploadReport();
                    }
                }
                else if(IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP == eventData->data.state.newState)
                {
                    Telemetry::_instance->AbortReport();
                }
            }
        }
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

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

#ifdef HAS_RBUS
        static void t2EventHandler(rbusHandle_t handle, char const* methodName, rbusError_t error, rbusObject_t param)
        {
            LOGINFO("Got %s rbus callback", methodName);

            if (RBUS_ERROR_SUCCESS == error)
            {
                rbusValue_t uploadStatus = rbusObject_GetValue(param, "UPLOAD_STATUS");

                if(uploadStatus)
                {
                    if (Telemetry::_instance)
                        Telemetry::_instance->onReportUploadStatus(rbusValue_GetString(uploadStatus, NULL));
                }
                else
                {
                    LOGERR("No 'UPLOAD_STATUS' value");
                    if (Telemetry::_instance)
                        Telemetry::_instance->onReportUploadStatus("No 'UPLOAD_STATUS' value");
                }
            }
            else
            {
                std::stringstream str;
                str << "Call failed with " << error << " error"; 
                LOGERR("%s", str.str().c_str());
                if (Telemetry::_instance)
                    Telemetry::_instance->onReportUploadStatus(str.str().c_str());
            }
        }

        void Telemetry::onReportUploadStatus(const char* status)
        {
            JsonObject eventData;
            std::string s(status);
            eventData["telemetryUploadStatus"] = s == "SUCCESS" ? "UPLOAD_SUCCESS" : "UPLOAD_FAILURE";
            sendNotify(TELEMETRY_METHOD_EVT_ON_REPORT_UPLOAD, eventData);
        }

        static void t2OnAbortEventHandler(rbusHandle_t handle, char const* methodName, rbusError_t error, rbusObject_t param)
        {
            LOGINFO("Got %s rbus callback", methodName);
        }

#endif
        uint32_t Telemetry::UploadReport()
        {
            LOGINFO("");
#ifdef HAS_RBUS
            if (RBUS_ERROR_SUCCESS != rbusHandleStatus)
            {
                rbusHandleStatus = rbus_open(&rbusHandle, RBUS_COMPONENT_NAME);
            }

            if (RBUS_ERROR_SUCCESS == rbusHandleStatus)
            {
                int rc = rbusMethod_InvokeAsync(rbusHandle, T2_ON_DEMAND_REPORT, NULL, t2EventHandler, 0);
                if (RBUS_ERROR_SUCCESS != rc)
                {
                    std::stringstream str;
                    str << "Failed to call " << T2_ON_DEMAND_REPORT << ": " << rc;
                    LOGERR("%s", str.str().c_str());

                    return Core::ERROR_RPC_CALL_FAILED;
                }
            }
            else
            {
                std::stringstream str;
                str << "rbus_open failed with error code " << rbusHandleStatus;
                LOGERR("%s", str.str().c_str());
                return Core::ERROR_OPENING_FAILED;
            }
#else
            LOGERR("No RBus support");
            return Core::ERROR_NOT_EXIST;
#endif
            return Core::ERROR_NONE;
        }

        uint32_t Telemetry::AbortReport()
        {
            LOGINFO("");
#ifdef HAS_RBUS
            if (RBUS_ERROR_SUCCESS != rbusHandleStatus)
            {
                rbusHandleStatus = rbus_open(&rbusHandle, RBUS_COMPONENT_NAME);
            }

            if (RBUS_ERROR_SUCCESS == rbusHandleStatus)
            {
                int rc = rbusMethod_InvokeAsync(rbusHandle, T2_ABORT_ON_DEMAND_REPORT, NULL, t2OnAbortEventHandler, 0);
                if (RBUS_ERROR_SUCCESS != rc)
                {
                    std::stringstream str;
                    str << "Failed to call " << T2_ABORT_ON_DEMAND_REPORT << ": " << rc;
                    LOGERR("%s", str.str().c_str());

                    return Core::ERROR_RPC_CALL_FAILED;
                }
            }
            else
            {
                std::stringstream str;
                str << "rbus_open failed with error code " << rbusHandleStatus;
                LOGERR("%s", str.str().c_str());
                return Core::ERROR_OPENING_FAILED;
            }
#else
            LOGERR("No RBus support");
            return Core::ERROR_NOT_EXIST;
#endif
            return Core::ERROR_NONE;
        }

        uint32_t Telemetry::uploadReport(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            return UploadReport();
        }

        uint32_t Telemetry::abortReport(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            return AbortReport();
        }

    } // namespace Plugin
} // namespace WPEFramework

