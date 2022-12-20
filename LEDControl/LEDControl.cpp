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

#include "LEDControl.h"
#include <algorithm>

#include "rdk/iarmmgrs-hal/pwrMgr.h"

#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"
#include "dsFPD.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

#define FPD_LED_DEVICE_NONE "NONE"
#define FPD_LED_DEVICE_ACTIVE "ACTIVE"
#define FPD_LED_DEVICE_STANDBY "STANDBY"
#define FPD_LED_DEVICE_WPS_CONNECTING "WPS_CONNECTING"
#define FPD_LED_DEVICE_WPS_CONNECTED "WPS_CONNECTED"
#define FPD_LED_DEVICE_WPS_ERROR "WPS_ERROR"
#define FPD_LED_DEVICE_FACTORY_RESET "FACTORY_RESET"
#define FPD_LED_DEVICE_USB_UPGRADE "USB_UPGRADE"
#define FPD_LED_DEVICE_SOFTWARE_DOWNLOAD_ERROR "DOWNLOAD_ERROR"



namespace WPEFramework
{
    namespace {

        static Plugin::Metadata<Plugin::LEDControl> metadata(
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
        SERVICE_REGISTRATION(LEDControl, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        LEDControl* LEDControl::_instance = nullptr;



        LEDControl::LEDControl()
        : PluginHost::JSONRPC(),
        m_isPlatInitialized (false)
        {
           LOGWARN("ctor");
           LEDControl::_instance = this;
           Register("getSupportedLEDStates", &LEDControl::getSupportedLEDStates, this);
           Register("getLEDState", &LEDControl::getLEDState, this);
           Register("setLEDState", &LEDControl::setLEDState, this);

        }

        LEDControl::~LEDControl()
        {

        }

        const string LEDControl::Initialize(PluginHost::IShell* /* service */)
        {
            string msg;
            if (!m_isPlatInitialized){
                LOGINFO("Doing plat init");
                if (dsERR_NONE != dsFPInit()){
                    msg = "dsFPInit failed";
		    LOGERR("dsFPInit failed");
		    return msg;
		}
                m_isPlatInitialized = true;
            }

            // On success return empty, to indicate there is no error text.
           return msg;
        }

        void LEDControl::Deinitialize(PluginHost::IShell* /* service */)
        {
            LEDControl::_instance = nullptr;

            if (m_isPlatInitialized){
                LOGINFO("Doing plat uninit");
                dsFPTerm();
                m_isPlatInitialized = false;
            }
        }

        void LEDControl::setResponseArray(JsonObject& response, const char* key, const vector<string>& items)
        {
            JsonArray arr;
            for(auto& i : items) arr.Add(JsonValue(i));

            response[key] = arr;

            string json;
            response.ToString(json);
        }

        /**
         * @brief This method returns all the led states supported by the platform.
         *
         * @param: None.
         * @return Returns the success code of underlying method.
         */
        uint32_t LEDControl::getSupportedLEDStates(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;
            vector<string> supportedLEDStates;

            try {
                unsigned int states = dsFPD_LED_DEVICE_NONE;
                dsError_t err = dsFPGetSupportedLEDStates (&states);
                if (!err) {
                    if(!states)supportedLEDStates.emplace_back(FPD_LED_DEVICE_NONE);
                    if(states & (1<<dsFPD_LED_DEVICE_ACTIVE))supportedLEDStates.emplace_back(FPD_LED_DEVICE_ACTIVE);
                    if(states & (1<<dsFPD_LED_DEVICE_STANDBY))supportedLEDStates.emplace_back(FPD_LED_DEVICE_STANDBY);
                    if(states & (1<<dsFPD_LED_DEVICE_WPS_CONNECTING))supportedLEDStates.emplace_back(FPD_LED_DEVICE_WPS_CONNECTING);
                    if(states & (1<<dsFPD_LED_DEVICE_WPS_CONNECTED))supportedLEDStates.emplace_back(FPD_LED_DEVICE_WPS_CONNECTED);
                    if(states & (1<<dsFPD_LED_DEVICE_WPS_ERROR))supportedLEDStates.emplace_back(FPD_LED_DEVICE_WPS_ERROR);
                    if(states & (1<<dsFPD_LED_DEVICE_FACTORY_RESET))supportedLEDStates.emplace_back(FPD_LED_DEVICE_FACTORY_RESET);
                    if(states & (1<<dsFPD_LED_DEVICE_USB_UPGRADE))supportedLEDStates.emplace_back(FPD_LED_DEVICE_USB_UPGRADE);
                    if(states & (1<<dsFPD_LED_DEVICE_SOFTWARE_DOWNLOAD_ERROR))supportedLEDStates.emplace_back(FPD_LED_DEVICE_SOFTWARE_DOWNLOAD_ERROR);
                    success = true;
                } else {
                        LOGERR("dsFPGetSupportedLEDStates returned error %d", err);
                }

            } catch (...){
                LOGERR("Exception in supportedLEDStates");
            }
            setResponseArray(response, "supportedLEDStates", supportedLEDStates);
            returnResponse(success);
        }

        /**
         * @brief This method returns current led state of the platform.
         *
         * @param: None.
         * @return Returns the success code of underlying method.
         */
        uint32_t LEDControl::getLEDState(const JsonObject& parameters, JsonObject& response)
        {
            bool success = false;

            try
            {
                dsFPDLedState_t state;
               dsError_t err = dsFPGetLEDState (&state);
               if (!err) {
                    success = true;
                    switch (state) {
                    case dsFPD_LED_DEVICE_NONE:
                        response["state"] = FPD_LED_DEVICE_NONE;
                        break;
                    case dsFPD_LED_DEVICE_ACTIVE:
                        response["state"] = FPD_LED_DEVICE_ACTIVE;
                        break;
                    case dsFPD_LED_DEVICE_STANDBY:
                        response["state"] = FPD_LED_DEVICE_STANDBY;
                        break;
                    case dsFPD_LED_DEVICE_WPS_CONNECTING:
                        response["state"] = FPD_LED_DEVICE_WPS_CONNECTING;
                        break;
                    case dsFPD_LED_DEVICE_WPS_CONNECTED:
                        response["state"] = FPD_LED_DEVICE_WPS_CONNECTED;
                        break;
                    case dsFPD_LED_DEVICE_WPS_ERROR:
                        response["state"] = FPD_LED_DEVICE_WPS_ERROR;
                        break;
                    case dsFPD_LED_DEVICE_FACTORY_RESET:
                        response["state"] = FPD_LED_DEVICE_FACTORY_RESET;
                        break;
                    case dsFPD_LED_DEVICE_USB_UPGRADE:
                        response["state"] = FPD_LED_DEVICE_USB_UPGRADE;
                        break;
                    case dsFPD_LED_DEVICE_SOFTWARE_DOWNLOAD_ERROR:
                        response["state"] = FPD_LED_DEVICE_SOFTWARE_DOWNLOAD_ERROR;
                        break;

                    default :
                        LOGERR("Unsupported LEDState %d", state);
                        LOGTRACEMETHODFIN();
                        return WPEFramework::Core::ERROR_BAD_REQUEST;
                    }
                } else {
                    LOGERR("dsFPGetLEDState returned error %d", err);
                    LOGTRACEMETHODFIN();
                    return WPEFramework::Core::ERROR_ILLEGAL_STATE;
                }
            }
            catch(...)
            {
                LOGERR("Exception in dsFPGetLEDState");
                LOGTRACEMETHODFIN();
                return WPEFramework::Core::ERROR_ILLEGAL_STATE;
            }

            returnResponse(success);
        }

        /**
         * @brief This method changes the current led state to the one, mentioned by the user.
         *
         * @param[in] led state to apply.
         * @return Returns the success code of underlying method.
         * @ingroup SERVMGR_FRONTPANEL_API
         */
        uint32_t LEDControl::setLEDState(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "state");
            string strLedState = parameters["state"].String();
            bool success = false;
            try
            {
                dsFPDLedState_t state = dsFPD_LED_DEVICE_NONE;
                if (0==strncmp(strLedState.c_str(), FPD_LED_DEVICE_ACTIVE, strlen(FPD_LED_DEVICE_ACTIVE)) &&
				(strlen(strLedState.c_str()) == strlen(FPD_LED_DEVICE_ACTIVE)) ){
                    state = dsFPD_LED_DEVICE_ACTIVE;
                } else if (0==strncmp(strLedState.c_str(), FPD_LED_DEVICE_STANDBY, strlen(FPD_LED_DEVICE_STANDBY)) &&
				(strlen(strLedState.c_str()) == strlen(FPD_LED_DEVICE_STANDBY)) ){
                    state = dsFPD_LED_DEVICE_STANDBY;
                } else if (0==strncmp(strLedState.c_str(), FPD_LED_DEVICE_WPS_CONNECTING, strlen(FPD_LED_DEVICE_WPS_CONNECTING)) && 
				(strlen(strLedState.c_str()) == strlen(FPD_LED_DEVICE_WPS_CONNECTING))){
                    state = dsFPD_LED_DEVICE_WPS_CONNECTING;
                } else if (0==strncmp(strLedState.c_str(), FPD_LED_DEVICE_WPS_CONNECTED, strlen(FPD_LED_DEVICE_WPS_CONNECTED)) &&
				(strlen(strLedState.c_str()) == strlen(FPD_LED_DEVICE_WPS_CONNECTED)) ){
                    state = dsFPD_LED_DEVICE_WPS_CONNECTED;
                } else if (0==strncmp(strLedState.c_str(), FPD_LED_DEVICE_WPS_ERROR, strlen(FPD_LED_DEVICE_WPS_ERROR)) &&
				(strlen(strLedState.c_str()) == strlen(FPD_LED_DEVICE_WPS_ERROR)) ){
                    state = dsFPD_LED_DEVICE_WPS_ERROR;
                } else if (0==strncmp(strLedState.c_str(), FPD_LED_DEVICE_FACTORY_RESET, strlen(FPD_LED_DEVICE_FACTORY_RESET)) &&
				(strlen(strLedState.c_str()) == strlen(FPD_LED_DEVICE_FACTORY_RESET)) ){
                    state = dsFPD_LED_DEVICE_FACTORY_RESET;
                } else if (0==strncmp(strLedState.c_str(), FPD_LED_DEVICE_USB_UPGRADE, strlen(FPD_LED_DEVICE_USB_UPGRADE)) &&
				(strlen(strLedState.c_str()) == strlen(FPD_LED_DEVICE_USB_UPGRADE)) ){
                    state = dsFPD_LED_DEVICE_USB_UPGRADE;
                } else if (0==strncmp(strLedState.c_str(), FPD_LED_DEVICE_SOFTWARE_DOWNLOAD_ERROR, strlen(FPD_LED_DEVICE_SOFTWARE_DOWNLOAD_ERROR)) && 
				(strlen(strLedState.c_str()) == strlen(FPD_LED_DEVICE_SOFTWARE_DOWNLOAD_ERROR)) ){
                    state = dsFPD_LED_DEVICE_SOFTWARE_DOWNLOAD_ERROR;
                } else {
                    //Invalid parameter
                    LOGERR("UNKNOWN state : %s", strLedState.c_str());
                    LOGTRACEMETHODFIN();
                    return WPEFramework::Core::ERROR_BAD_REQUEST;
                }
                if (dsFPD_LED_DEVICE_NONE!=state) {
		    LOGINFO("dsFPSetLEDState state:%s state:%d", strLedState.c_str(), state);
                    dsError_t err = dsFPSetLEDState (state);
                    if (!err) {
                        success = true;
                    } else {
                        LOGERR("dsFPSetLEDState returned error %d", err);
                        LOGTRACEMETHODFIN();
                        return WPEFramework::Core::ERROR_ILLEGAL_STATE;
                    }
                }
            }
            catch (...)
            {
                LOGERR("Exception in dsFPSetLEDState");
                LOGTRACEMETHODFIN();
                return WPEFramework::Core::ERROR_ILLEGAL_STATE;
            }

            returnResponse(success);
        }


    } // namespace Plugin
} // namespace WPEFramework
