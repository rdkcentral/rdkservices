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

#include "LoggingPreferences.h"
#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"
#include <sysMgr.h>

using namespace std;

namespace WPEFramework {

    namespace Plugin {

        SERVICE_REGISTRATION(LoggingPreferences, 1, 0);

        LoggingPreferences* LoggingPreferences::_instance = nullptr;

        LoggingPreferences::LoggingPreferences()
            : PluginHost::JSONRPC()
        {
            LOGINFO("ctor");
            LoggingPreferences::_instance = this;
            Register("isKeystrokeMaskEnabled", &LoggingPreferences::isKeystrokeMaskEnabled, this);
            Register("setKeystrokeMaskEnabled", &LoggingPreferences::setKeystrokeMaskEnabled, this);
        }

        LoggingPreferences::~LoggingPreferences()
        {
            //LOGINFO("dtor");
        }

        const string LoggingPreferences::Initialize(PluginHost::IShell* /* service */)
        {
            InitializeIARM();
            return "";
        }

        void LoggingPreferences::Deinitialize(PluginHost::IShell* /* service */)
        {
            DeinitializeIARM();
            LoggingPreferences::_instance = nullptr;
        }

        void LoggingPreferences::InitializeIARM()
        {
            Utils::IARM::init();
        }

        void LoggingPreferences::DeinitializeIARM()
        {
        }

        //Begin methods
        uint32_t LoggingPreferences::isKeystrokeMaskEnabled(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t param;
            IARM_Result_t res = IARM_Bus_Call(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref, (void *)&param, sizeof(param));
            if(res != IARM_RESULT_SUCCESS)
            {
                LOGERR("IARM call failed with status %d while reading preferences", res);
                returnResponse(false);
            }

            response["keystrokeMaskEnabled"] = !param.logStatus;
            returnResponse(true);
        }
        uint32_t LoggingPreferences::setKeystrokeMaskEnabled(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            returnIfBooleanParamNotFound(parameters, "keystrokeMaskEnabled");

            IARM_BUS_SYSMGR_KEYCodeLoggingInfo_Param_t params;
            IARM_Result_t res = IARM_Bus_Call(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_API_GetKeyCodeLoggingPref, (void *)&params, sizeof(params));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("IARM call failed with status %d while reading preferences", res);
                returnResponse(false);
            }

            bool enabled = parameters["keystrokeMaskEnabled"].Boolean();

            if (enabled == params.logStatus)
            {
                params = { enabled ? 0 : 1 };
                IARM_Result_t res = IARM_Bus_Call(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_API_SetKeyCodeLoggingPref, (void *)&params, sizeof(params));
                if (res != IARM_RESULT_SUCCESS)
                {
                    LOGERR("IARM call failed with status %d while setting preferences", res);
                    returnResponse(false);
                }

                onKeystrokeMaskEnabledChange(enabled);
            }
            else
            {
                LOGWARN("Keystroke mask already %s", enabled ? "enabled" : "disabled");
            }

            returnResponse(true);
        }
        //End methods

        //Begin events
        void LoggingPreferences::onKeystrokeMaskEnabledChange(bool enabled)
        {
            JsonObject params;
            params["keystrokeMaskEnabled"] = enabled;
            sendNotify("onKeystrokeMaskEnabledChange", params);
        }
        //End events

    } // namespace Plugin
} // namespace WPEFramework
