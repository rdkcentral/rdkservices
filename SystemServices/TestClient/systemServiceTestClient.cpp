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

/**
 * @file systemServicesTestClient.cpp
 * @brief Thunder Plugin based Implementation of CPP Test Client for System service API's.
 * @reference RDK-25849.
 */

#include <iostream>
#include <bits/stdc++.h>
#include <ctime>
#include <string>
#include<stdio.h>
#include<stdlib.h>

#include "Module.h"

#define SYSSRV_CALLSIGN "org.rdk.SystemServices"
#define SERVER_DETAILS  "127.0.0.1:9998"

using namespace std;
using namespace WPEFramework;

/* This section can be used for API validation logic. */
void showMenu()
{
    std::cout<<"Enter your choice\n";
    std::cout<<"1.setTimeZoneDST\n";
    std::cout<<"2.getTimeZoneDST\n";
    std::cout<<"3.getCoreTemperature\n";
    std::cout<<"4.getPreviousRebootInfo\n";
    std::cout<<"5.getCachedValue\n";
    std::cout<<"6.isGzEnabled\n";
    std::cout<<"7.updateFirmware\n";
    std::cout<<"8.getMode\n";
    std::cout<<"9.getPreferredStandbyMode\n";
    std::cout<<"10.getAvailableStandbyModes\n";
    std::cout<<"11.setCachedValue\n";
    std::cout<<"12.setMode\n";
    std::cout<<"13.setpreferredStandbyModes\n";
    std::cout<<"14.cacheContains api\n";
    std::cout<<"15.removeCacheKey api\n";
    std::cout<<"16.setGzEnabled api\n";
    std::cout<<"17.getFirmwareUpdateInfo\n";
    std::cout<<"18.getSystemVersions\n";
    std::cout<<"19.getDownloadPercent\n";
    std::cout<<"20.getTemperatureThreshold\n";
    std::cout<<"21.setTemperatureThreshold\n";
    std::cout<<"22.getXconfParams\n";
    std::cout<<"23.setDeepSleepTimer\n";
}

/* This section is related to the event handler implementation for SystemService Plugin Events. */

namespace Handlers {
    /* Event Handlers */
    static void onSystemPowerStateChangedHandler(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        std::cout << "[SysSrvEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
    static void onSystemModeChangedHandler(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        std::cout << "[SysSrvEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
    static void onFirmwareUpdateInfoReceivedHandler(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        std::cout << "[SysSrvEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
    static void onFirmwareUpdateStateChangeHandler(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        std::cout << "[SysSrvEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
    static void onTemperatureThresholdChangedHandler(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        std::cout << "[SysSrvEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
    static void onMacAddressesRetreivedHandler(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        std::cout << "[SysSrvEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
    static void onRebootRequestHandler(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        std::cout << "[SysSrvEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
    static void onSampleEventHandler(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        std::cout << "[SysSrvEvt] " << __FUNCTION__ << ": " << message << std::endl;
    }
}

int main(int argc, char** argv)
{
    int retStatus = -1;
    JSONRPC::Client* remoteObject = NULL;
    std::string reqPayload;

    int choice;
    JsonObject result,response,value;
    JsonObject parameters,params;
    uint32_t ret;
    int size;
    int i;
    string req, appName, json, jsonObj_str, resp, str;

    Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));

    if (NULL == remoteObject) {
        remoteObject = new JSONRPC::Client(_T(SYSSRV_CALLSIGN), _T(""));
        if (NULL == remoteObject) {
            std::cout << "JSONRPC::Client initialization failed" << std::endl;
        } else {
            JsonObject request, response;

            /* Register handlers for Event reception. */
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onSystemPowerStateChanged"),
                        &Handlers::onSystemPowerStateChangedHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onSystemPowerStateChangedHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onSystemPowerStateChangedHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onSystemModeChanged"),
                        &Handlers::onSystemModeChangedHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onSystemModeChangedHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onSystemModeChangedHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onFirmwareUpdateInfoReceived"),
                        &Handlers::onFirmwareUpdateInfoReceivedHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onFirmwareUpdateInfoReceivedHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onFirmwareUpdateInfoReceivedHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onFirmwareUpdateStateChange"),
                        &Handlers::onFirmwareUpdateStateChangeHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onFirmwareUpdateStateChangeHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onFirmwareUpdateStateChangeHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onTemperatureThresholdChanged"),
                        &Handlers::onTemperatureThresholdChangedHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onTemperatureThresholdChangedHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onTemperatureThresholdChangedHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onMacAddressesRetreived"),
                        &Handlers::onMacAddressesRetreivedHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onMacAddressesRetreivedHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onMacAddressesRetreivedHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onRebootRequest"),
                        &Handlers::onRebootRequestHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onRebootRequestHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onRebootRequestHandler" << std::endl;
            }
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("onSampleEvent"),
                        &Handlers::onSampleEventHandler) == Core::ERROR_NONE) {
                std::cout << "Subscribed to : onSampleEventHandler" << std::endl;
            } else {
                std::cout << "Failed to Subscribe notification handler : onSampleEventHandler" << std::endl;
            }

            /* API Validation Logic. */
            while (true) {
                showMenu();
                std::cin >> choice;
                switch (choice) {
                    case 1:
                        {
                            JsonObject params,param;
                            std::cout<<"set DST Time zone\n";
                            std::cout<<"enter the app name\n";
                            std::cin>> appName;
                            std::cout<<"enter time zone\n";
                            string timeZone;
                            std::cin>> timeZone;
                            params["appName"] = appName;
                            param["timezone"] = timeZone;
                            string param_json;
                            param.ToString(param_json);
                            std::cout<<"param json object is "<<param_json.c_str();
                            params["param"].FromString(param_json);
                            params.ToString(json);
                            std::cout<<"setTimeZoneDST :the request json  is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("setTimeZoneDST"), params, result);
                            if (result["success"].Boolean()) {
                                std::cout<<"SystemServices setTimeZoneDST call Success\n";
                            } else {
                                std::cout<<"SystemServices setTimeZoneDST call failed \n";
                            }
                        }
                        break;

                    case 2:
                        {
                            JsonObject params;
                            std::cout<<"getTimeZoneDST\n";
                            std::cout<<"enter the app name\n";
                            std::cin>> appName;
                            params["appName"] = appName;
                            params.ToString(json);
                            std::cout<<"getTimeZoneDST the request json  is "<<
                                json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("getTimeZoneDST"), params, result);
                            if (result["success"].Boolean()) {
                                string tzone = result["timeZone"].String();
                                std::cout<<"SystemServices: getTimeZoneDST time zone value is "<<tzone<<"\n";
                                std::cout<<"SystemServices: getTimeZoneDST call Success\n";
                            } else {
                                std::cout<<"SystemServices: getTimeZoneDST  call failed \n";
                            }
                        }
                        break;

                    case 3:
                        {
                            JsonObject params;
                            std::cout<<"getCoreTemperature\n";
                            std::cout<<"enter the app name\n";

                            std::cin>> appName;
                            params["appName"] = appName;
                            params.ToString(json);
                            std::cout<<"getCoreTemperature the request json  is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("getCoreTemperature"), params, result);
                            if (result["success"].Boolean()) {
                                string temp = result["temperature"].String();
                                std::cout<<"SystemServices: getCoreTemperature temperature  value is "<<temp<<"\n";
                                std::cout<<"SystemServices: getCoreTemperature call Success\n";
                            } else {
                                std::cout<<"SystemServices: getCoreTemperature  call failed \n";
                            }
                        }
                        break;

                    case 4:
                        {
                            JsonObject params;
                            std::cout<<"getPreviousRebootInfo\n";
                            std::cout<<"enter the app name\n";

                            std::cin>> appName;
                            params["appName"] = appName;
                            params.ToString(json);
                            std::cout<<"getPreviousRebootInfo the request json  is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("getPreviousRebootInfo"), params, result);
                            if (result["success"].Boolean()) {
                                string timeStamp=result["timeStamp"].String();
                                string reason=result["reason"].String();
                                std::cout<<"SystemServices:  getPreviousRebootInfo time stamp  value is "
                                    <<timeStamp<<"reason is"<<reason<<"\n";
                                std::cout<<"SystemServices: getPreviousRebootInfo  call Success\n";
                            } else {
                                std::cout<<"SystemServices: getPreviousRebootInfo  call failed \n";
                            }
                        }
                        break;

                    case 5:
                        {
                            JsonObject params,param;
                            std::cout<<"getCachedValue\n";
                            std::cout<<"enter the app name\n";
                            std::cin>> appName;
                            params["appName"] = appName;
                            std::cout<<"enter cache key string\n";
                            string cachekey;
                            std::cin>> cachekey;
                            param["cacheKey"] = cachekey;
                            string param_json;
                            param.ToString(param_json);
                            std::cout<<"param json object is "<<param_json.c_str()<<"\n";
                            params["param"].FromString(param_json);
                            string json;
                            params.ToString(json);
                            std::cout<<"getCachedValue the json request string is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("getCachedValue"), params, result);
                            if (result["success"].Boolean()) {
                                result.ToString(resp);
                                std::cout<<"json output getCached value "<<resp.c_str()<<"\n";
                                std::cout<<"SystemServices: getCachedValue  call Success\n";
                            } else {
                                std::cout<<"SystemServices: getCachedValue  call failed \n";
                            }
                        }
                        break;

                    case 6:
                        {
                            JsonObject params;
                            std::cout<<"isGzEnabled\n";
                            std::cout<<"enter the app name\n";
                            std::cin>> appName;
                            params["appName"] = appName;
                            params.ToString(json);
                            std::cout<<"isGzEnabled the json request string is "<<json.c_str()<<"\n";

                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("isGzEnabled"), params, result);
                            if (result["success"].Boolean()) {
                                bool enable = result["enabled"].Boolean();
                                std::cout<<"SystemServices: isGzEnabled enabled value is "<<enable<<"\n";
                                std::cout<<"SystemServices: isGzEnabled  call Success\n";
                            } else {
                                std::cout<<"SystemServices: isGzEnabled  call failed \n";
                            }
                        }
                        break;

                    case 7:
                        {
                            JsonObject params;
                            std::cout<<"updateFirmware\n";
                            std::cout<<"enter the app name\n";
                            std::cin>> appName;
                            params["appName"] = appName;
                            params.ToString(json);
                            std::cout<<"updateFirmware the json request string is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("updateFirmware"), params, result);
                            if (result["success"].Boolean()) {
                                std::cout<<"SystemServices: updateFirmware  call Success\n";
                            } else {
                                std::cout<<"SystemServices: updateFirmware  call failed \n";
                            }
                        }
                        break;

                    case 8:
                        {
                            JsonObject params;
                            std::cout<<"getMode\n";
                            std::cout<<"enter the app name\n";

                            std::cin>> appName;
                            params["appName"] = appName;
                            params.ToString(json);
                            std::cout<<"getMode the json request string is "<<json.c_str()<<"\n";

                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("getMode"), params, result);
                            if (result["success"].Boolean()) {
                                JsonObject param;
                                string resp_str;
                                resp_str = result["modeInfo"].String();
                                std::cout<<"result of getMode "<<resp_str.c_str()<<"\n";
                                param.FromString(resp_str);
                                std::cout<<"getmode the result json object is "<<resp_str.c_str()<<"\n";
                                string duration = param["duration"].String();
                                string mode = param["mode"].String();
                                std::cout<<"SystemServices : getMode  mode is "
                                    <<mode<<"duration is "<<duration<<"\n";
                                std::cout<<"SystemServices: getMode  call Success\n";
                            } else {
                                std::cout<<"SystemService: getMode  call failed \n";
                            }
                        }
                        break;

                    case 9:
                        {
                            JsonObject params;
                            std::cout<<"getPreferredStandbyMode\n";
                            std::cout<<"enter the app name\n";
                            std::cin>> appName;
                            params["appName"] = appName;
                            params.ToString(json);
                            std::cout<<"getPreferredStandbyMode the json request string is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("getPreferredStandbyMode"), params, result);
                            if (result["success"].Boolean()) {
                                string standBy=result["preferredStandbyMode"].String();
                                std::cout<<"SystemServices: preferredStandbyMode value is  "<<standBy<<"\n";
                                std::cout<<"SystemServices: preferredStandbyMode  call Success\n";
                            } else {
                                std::cout<<"SystemServices: preferredStandbyMode call failed \n";
                            }
                        }
                        break;

                    case 10:
                        {
                            JsonObject params;
                            std::cout<<"getAvailableStandbyModes\n";
                            std::cout<<"enter the app name\n";
                            std::cin>> appName;
                            params["appName"] = appName;
                            params.ToString(json);
                            std::cout<<"getAvailableStandbyModes the json request string is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("getAvailableStandbyModes"), params, result);
                            if (result["success"].Boolean()) {
                                string standBy=result["supportedStandbyModes"].String();
                                std::cout<<"SystemServices: getAvailableStandbyModes value is  "<<standBy<<"\n";
                                std::cout<<"SystemServices : getAvailableStandbyModes   call Success\n";
                            } else {
                                std::cout<<"SystemServices: getAvailableStandbyModes  call failed \n";
                            }
                        }
                        break;

                    case 11:
                        {
                            JsonObject params,param;
                            std::cout<<"setCachedValue\n";
                            std::cout<<"enter the app name\n";
                            std::cin>> appName;
                            params["appName"] = appName;
                            std::cout<<"enter cache key value\n";
                            string cachekey;
                            std::cin>> cachekey;
                            std::cout<<"enter cache value\n";
                            string cachevalue;
                            std::cin>> cachevalue;
                            param["cacheKey"] = cachekey;
                            param["cacheValue"] = cachevalue;
                            param.ToString(jsonObj_str);
                            std::cout<<"setCachedValue param json object is "<<jsonObj_str.c_str()<<"\n";
                            params["param"].FromString(jsonObj_str);
                            params.ToString(json);
                            std::cout<<"the json request string is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("setCachedValue"), params, result);
                            if (result["success"].Boolean()) {
                                std::cout<<"Systemservice :set cached value api success\n";
                            } else {
                                std::cout<<"Systemservice :set cached value api failure\n";
                            }
                        }
                        break;

                    case 12:
                        {
                            JsonObject params,param;
                            std::cout<<"setMode api\n";
                            std::cout<<"enter the app name\n";
                            std::cin>> appName;
                            params["appName"] = appName;
                            std::cout<<"enter mode\n";
                            string mode;
                            std::cin>> mode;
                            std::cout<<"enter duration\n";
                            string duration;
                            std::cin>> duration;
                            param["mode"] = mode;
                            param["duration"] = duration;
                            param.ToString(jsonObj_str);
                            std::cout<<"param json object is "<<jsonObj_str.c_str()<<"\n";
                            params["param"].FromString(jsonObj_str);
                            params.ToString(json);
                            std::cout<<"setMode the json request string is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("setMode"), params, result);
                            if (result["success"].Boolean()) {
                                std::cout<<"Systemservice :setmode api success\n";
                            } else {
                                std::cout<<"Systemservice :setmode  api failure\n";
                            }
                        }
                        break;

                    case 13:
                        {
                            JsonObject params,param;
                            std::cout<<"setpreferredStandbyModes\n";
                            std::cout<<"enter app name\n";
                            std::cin>> appName;
                            params["appName"] = appName;
                            string mode;
                            std::cout<<"enter mode\n";
                            std::cin>> mode;
                            param["mode"] = mode;
                            param.ToString(jsonObj_str);
                            std::cout<<"param json object is "<<jsonObj_str.c_str()<<"\n";
                            params["param"].FromString(jsonObj_str);
                            params.ToString(json);
                            std::cout<<"setPreferredStandbyModes the json request string is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("setPreferredStandbyMode"), params, result);
                            if (result["success"].Boolean()) {
                                std::cout<<"Systemservice :setPreferredStandbyMode api success\n";
                            } else {
                                std::cout<<"Systemservice :setPreferredStandbyMode api failure\n";
                            }
                        }
                        break;

                    case 14:
                        {
                            JsonObject params,param;
                            std::cout<<"cacheContains api\n";
                            std::cout<<"enter the app name\n";
                            std::cin>> appName;
                            params["appName"] = appName;
                            std::cout<<"enter cache key\n";
                            string cacheKey;
                            std::cin>> cacheKey;
                            param["cacheKey"] = cacheKey;
                            param.ToString(jsonObj_str);
                            std::cout<<"param json object is "<<jsonObj_str.c_str()<<"\n";
                            params["param"].FromString(jsonObj_str);
                            params.ToString(json);
                            std::cout<<"cacheContains the json request string is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("cacheContains"), params, result);
                            if (result["success"].Boolean()) {
                                std::cout<<"Systemservice :cacheContains api success\n";
                            } else {
                                std::cout<<"Systemservice :cacheContains  api failure\n";
                            }
                        }
                        break;

                    case 15:
                        {
                            JsonObject params,param;
                            std::cout<<"removeCacheKey api\n";
                            std::cout<<"enter the app name\n";
                            std::cin>> appName;
                            params["appName"] = appName;
                            std::cout<<"enter cache key\n";
                            string cacheKey;
                            std::cin>> cacheKey;
                            param["cacheKey"]=cacheKey;
                            param.ToString(jsonObj_str);
                            std::cout<<"param json object is "<<jsonObj_str.c_str()<<"\n";
                            params["param"].FromString(jsonObj_str);
                            params.ToString(json);
                            std::cout<<"removeCacheKey the json request string is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("removeCacheKey"), params, result);
                            if (result["success"].Boolean()) {
                                std::cout<<"Systemservice :removeCacheKey api success\n";
                            } else {
                                std::cout<<"Systemservice :removeCacheKey api failure\n";
                            }
                        }
                        break;

                    case 16:
                        {
                            JsonObject params;
                            std::cout<<"setGzEnabled api\n";
                            std::cout<<"enter the app name\n";
                            std::cin>> appName;
                            params["appName"] = appName;
                            bool param;
                            std::cout<<"enter if GzEnabled is true(enter 1) or false(enter 0)\n";
                            std::cin>> param;
                            params["param"] = param;
                            params.ToString(json);
                            std::cout<<"setGzEnabled the json request string is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("setGzEnabled"), params, result);
                            if (result["success"].Boolean()) {
                                std::cout<<"Systemservice :setGzEnabled api success\n";
                            } else {
                                std::cout<<"Systemservice :setGzEnabled api failure\n";
                            }
                        }
                        break;

                    case 17:
                        {
                            JsonObject params,param;
                            std::cout<<"getFirmwareUpdateInfo\n";
                            std::cout<<"enter the app name\n";
                            std::cin>> appName;
                            params["appName"] = appName;
                            std::cout<<"enter the GUID value\n";
                            string guid;
                            std::cin>> guid;
                            param["GUID"] = guid;
                            param.ToString(jsonObj_str);
                            std::cout<<"param json object is "<<jsonObj_str.c_str()<<"\n";
                            params["param"].FromString(jsonObj_str);
                            params.ToString(json);
                            std::cout<<"getFirmwareUpdateInfo  the json request string is "<<json.c_str()<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("getFirmwareUpdateInfo"), params, result);
                            if (result["success"].Boolean()) {
                                bool asyncResponse;
                                asyncResponse = result["asyncResponse"].Boolean();
                                std::cout<<"firmwareupdateinfo asyncresponse value is "<<asyncResponse<<"\n";
                                std::cout<<"Systemservice :getFirmwareUpdateInfo  api success\n";
                            } else {
                                std::cout<<"Systemservice :getFirmwareUpdateInfo  api failure\n";
                            }
                        }
                        break;

                    case 18:
                        {
                            std::cout<<"----------getSystemVersions----------\n";
                            std::cout<<"Enter the number of properties\n";
                            std::cin>> size;
                            string pName[size];
                            std::cout<<"Enter the properties which needs to be registered\n1.Enter the name pf API\n";
                            for (i = 0; i < size; i++) {
                                std::cin>> pName[i];
                            }
                            req="\"" + pName[0] +"\"";
                            parameters["appName"].FromString(req);
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("getSystemVersions"), parameters, result);
                            std::cout<<ret;
                            result.ToString(str);
                            std::cout<<str<<"\n";
                            if (result["success"].Boolean()) {
                                string stbVersion =  result["stbVersion"].String();
                                string receiverVersion = result["receiverVersion"].String();
                                string stbTimestamp = result["stbTimestamp"].String();
                                std::cout<<"stbVersionNumber  = "<<stbVersion<<"\n";
                                std::cout<<"receiverVersion = "<<receiverVersion<<"\n";
                                std::cout<<"stbTimestamp = "<<stbTimestamp<<"\n";
                                std::cout<<"Successfully get requested the API getSystemVersions \n";
                            } else {
                                std::cout<<"call failed \n";
                            }
                        }
                        break;

                    case 19:
                        {
                            std::cout<<"----getFirmwareDownloadPercent------\n";
                            std::cout<<"Enter the number of properties\n1.Enter the name pf API\n";
                            std::cin>> size;
                            string pName[size];
                            std::cout<<"Enter the properties which needs to be registered\n";
                            for(i=0;i<size;i++){
                                std::cin>> pName[i];
                            }
                            //To create the json format request string
                            req="\"" + pName[0] +"\"";
                            std::cout<<"Request string is"<<req<<"\n";
                            parameters["appName"].FromString(req);
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("getFirmwareDownloadPercent"), parameters, result);
                            result.ToString(str);
                            std::cout<<str<<"\n";
                            if (result["success"].Boolean()) {
                                int m_downloadPercent = result["downloadPercent"].Number();
                                std::cout<<"downloadPercent = "<<m_downloadPercent<<"\n";
                                std::cout<<"Successfully get requested the API getFirmwareDownloadPercent \n";
                            } else {
                                std::cout<<" call failed \n";
                            }
                        }
                        break;

                    case 20:
                        {
                            std::cout<<"-----getTemperatureThresholds-------\n";
                            std::cout<<"Enter the number of properties\n";
                            std::cin>> size;
                            string pName[size];
                            std::cout<<"Enter the properties which needs to be registered\n1.Enter the name of API\n";
                            for(i=0;i<size;i++){
                                std::cin>> pName[i];
                            }
                            req="\"" + pName[0] +"\"";
                            std::cout<<"\nRequested string is"<<req<<"\n";
                            parameters["appName"].FromString(req);
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("getTemperatureThresholds"), parameters, response);

                            value.FromString(response["temperatureThresholds"].String());
                            response.ToString(str);
                            std::cout<<str<<"\n";
                            if (response["success"].Boolean()) {
                                string warnData = value["WARN"].String();
                                string maxData = value["MAX"].String();
                                string temperatureData = value["temperature"].String();
                                std::cout<<"Temperature WARN = "<<warnData<<"\n";
                                std::cout<<"Temperature MAX = "<<maxData<<"\n";
                                std::cout<<"Temperature = "<<temperatureData<<"\n";
                                std::cout<<"Successfully get requested the API getTemperatureThresholds \n";
                            } else {
                                std::cout<<"call failed \n";
                            }
                        }
                        break;
                    case 21:
                        {
                            std::cout<<"-------setTemperatureThresholds-----\n";
                            std::cout<<"Enter the number of properties\n";
                            std::cin>> size;
                            string pName[size];
                            std::cout<<"Enter the properties which needs to be registered\n1.Enter the WARN value of temperature\n2.Enter the MAX value of temperature\n";
                            for(i=0;i<size;i++){
                                std::cin>> pName[i];
                            }
                            parameters["appName"].FromString(req);
                            value["WARN"] = pName[1];
                            value["MAX"] = pName[2];
                            parameters["param"] = value;
                            std::cout<<"Request string is"<<req<<"\n";
                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("setTemperatureThresholds"), parameters, response);
                            response.ToString(str);
                            std::cout<<str<<"\n";
                            if (response["success"].Boolean()) {
                                std::cout<<"set sucessfully\n";
                            } else {
                                std::cout<<" call failed \n";
                            }
                        }
                        break;

                    case 22:
                        {
                            std::cout<<"-----getXconfParams----------- \n";
                            std::cout<<"Enter the number of properties\n";
                            std::cin>> size;
                            string pName[size];
                            std::cout<<"Enter the properties which needs to be registered\n1.Enter the name of API\n";
                            for(i=0;i<size;i++){
                                std::cin>> pName[i];
                            }
                            req="\"" + pName[0] +"\"";
                            std::cout<<"Request string is"<<req<<"\n";
                            parameters["appName"].FromString(req);

                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("getXconfParams"), parameters, result);
                            result.ToString(str);
                            std::cout<<str<<"\n";
                            if (result["success"].Boolean()) {
                                value.FromString(result["xconfParams"].String());
                                string eStbMac = value["eStbMac"].String();
                                string model = value["model"].String();
                                string firmwareVersion = value["firmwareVersion"].String();
                                std::cout<<"eStbMac = "<<eStbMac<<"\n";
                                std::cout<<"STB Model = "<<model<<"\n";
                                std::cout<<"firmwareVersion = "<<firmwareVersion<<"\n";
                                std::cout<<"Successfully get requested XconfParams \n";
                            } else {
                                std::cout<<" call failed \n";
                            }
                        }
                        break;

                    case 23:
                        {
                            std::cout<<"---------setDeepSleepTimer---------\n";
                            std::cout<<"Enter the number of properties\n";
                            std::cin>> size;
                            string pName[size];
                            std::cout<<"Enter the properties which needs to be registered\n";
                            for(i=0;i<size;i++){
                                std::cin>> pName[i];
                            }
                            parameters["appName"].FromString(req);
                            value["deepSleepInterval"]=pName[1];
                            parameters["param"]=value;

                            ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
                                    _T("setDeepSleepTimer"), parameters, response);
                            response.ToString(str);
                            std::cout<<str<<"\n";

                            if (response["success"].Boolean()) {
                                std::cout<<"set successfully"<<"\n";
                            } else {
                                std::cout<<"call failed"<<"\n";
                            }
                        }
                        break;
                    default:
                        break;
                }

                std::cout<<"to continue press y to quit press any other key \n";
                char c;
                std::cin >> c;
                if ((c == 'y') || (c == 'Y')) {
                    continue;
                } else {
                    break;
                }
            }
        }

    }
    return retStatus;
}
