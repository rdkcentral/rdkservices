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
#include <chrono>
#include <iomanip>
#include <sstream>
#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <type_traits>
#include <core/core.h>
#include <websocket/websocket.h>

#define SYSPLUGIN_CALLSIGN      "org.rdk.System"
#define SYSPLUGIN_SERVER_PORT   "127.0.0.1:9998"

using namespace std;
using namespace WPEFramework;

typedef enum SME_t {
    SME_NoneExit = 0,
    SME_cacheContains,
    SME_clearLastDeepSleepReason,
    SME_enableMoca,
    SME_disableMoca,
    SME_enableXREConnectionRetention,
    SME_getAvailableStandbyModes,
    SME_getCachedValue,
    SME_getCoreTemperature,
    SME_getDeviceInfo,
    SME_getDownloadedFirmwareInfo,
    SME_getFirmwareDownloadPercent,
    SME_getFirmwareUpdateInfo,
    SME_getFirmwareUpdateState,
    SME_getLastDeepSleepReason,
    SME_getMacAddresses,
    SME_getMilestones,
    SME_getMode,
    SME_getPowerState,
    SME_getPreferredStandbyMode,
    SME_getPreviousRebootInfo,
    SME_getPreviousRebootInfo2,
    SME_getPreviousRebootReason,
    SME_getRFCConfig,
    SME_getSerialNumber,
    SME_getStateInfo,
    SME_getSystemVersions,
    SME_getTemperatureThresholds,
    SME_getTimeZoneDST,
    SME_getXconfParams,
    SME_hasRebootBeenRequested,
    SME_isGzEnabled,
    SME_queryMocaStatus,
    SME_reboot,
    SME_removeCacheKey,
    SME_requestSystemUptime,
    SME_setCachedValue,
    SME_setDeepSleepTimer,
    SME_setGzEnabled,
    SME_setMode,
    SME_setPowerState,
    SME_setPreferredStandbyMode,
    SME_setTemperatureThresholds,
    SME_setTimeZoneDST,
    SME_updateFirmware,
    SME_MAX,
};

std::map<SME_t, std::string> SMName = {
    {SME_cacheContains, "cacheContains"},
    {SME_clearLastDeepSleepReason, "clearLastDeepSleepReason"},
    {SME_enableMoca, "enableMoca"},
    {SME_disableMoca, "disableMoca"},
    {SME_enableXREConnectionRetention, "enableXREConnectionRetention"},
    {SME_getAvailableStandbyModes, "getAvailableStandbyModes"},
    {SME_getCachedValue, "getCachedValue"},
    {SME_getCoreTemperature, "getCoreTemperature"},
    {SME_getDeviceInfo, "getDeviceInfo"},
    {SME_getDownloadedFirmwareInfo, "getDownloadedFirmwareInfo"},
    {SME_getFirmwareDownloadPercent, "getFirmwareDownloadPercent"},
    {SME_getFirmwareUpdateInfo, "getFirmwareUpdateInfo"},
    {SME_getFirmwareUpdateState, "getFirmwareUpdateState"},
    {SME_getLastDeepSleepReason, "getLastDeepSleepReason"},
    {SME_getMacAddresses, "getMacAddresses"},
    {SME_getMilestones, "getMilestones"},
    {SME_getMode, "getMode"},
    {SME_getPowerState, "getPowerState"},
    {SME_getPreferredStandbyMode, "getPreferredStandbyMode"},
    {SME_getPreviousRebootInfo, "getPreviousRebootInfo"},
    {SME_getPreviousRebootInfo2, "getPreviousRebootInfo2"},
    {SME_getPreviousRebootReason, "getPreviousRebootReason"},
    {SME_getRFCConfig, "getRFCConfig"},
    {SME_getSerialNumber, "getSerialNumber"},
    {SME_getStateInfo, "getStateInfo"},
    {SME_getSystemVersions, "getSystemVersions"},
    {SME_getTemperatureThresholds, "getTemperatureThresholds"},
    {SME_getTimeZoneDST, "getTimeZoneDST"},
    {SME_getXconfParams, "getXconfParams"},
    {SME_hasRebootBeenRequested, "hasRebootBeenRequested"},
    {SME_isGzEnabled, "isGzEnabled"},
    {SME_queryMocaStatus, "queryMocaStatus"},
    {SME_reboot, "reboot"},
    {SME_removeCacheKey, "removeCacheKey"},
    {SME_requestSystemUptime, "requestSystemUptime"},
    {SME_setCachedValue, "setCachedValue"},
    {SME_setDeepSleepTimer, "setDeepSleepTimer"},
    {SME_setGzEnabled, "setGzEnabled"},
    {SME_setMode, "setMode"},
    {SME_setPowerState, "setPowerState"},
    {SME_setPreferredStandbyMode, "setPreferredStandbyMode"},
    {SME_setTemperatureThresholds, "setTemperatureThresholds"},
    {SME_setTimeZoneDST, "setTimeZoneDST"},
    {SME_updateFirmware, "updateFirmware"},
    {SME_MAX, "MAX"},
};

/******************************* Begin: Handle Selection *******************************/

void cacheContains(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	if (!remoteObject)
		return;

	JsonObject parameters, response;
	std::string result;
	std::string key;

	std::cout << "Key name to check :";
	std::cin >> key;
	parameters["key"] = key;
	
	parameters.ToString(result);
	/* Hack ? - remove the escape characters from the result for aesthetics. */
    result.erase(std::remove(result.begin(), result.end(), '\\'), result.end());
	std::cout << "Request : '" << result << "'" << std::endl;
	int status = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T(methodName), parameters, response);
	if (Core::ERROR_NONE != status) {
		std::cout << "Invoke failed with return status :" << std::hex << status << std::endl;
	} else {
		response.ToString(result);
		/* Hack ? - remove the escape characters from the result for aesthetics. */
   		result.erase(std::remove(result.begin(), result.end(), '\\'), result.end());
		std::cout << "Response: '" << result << "'" << std::endl;
	}
}

void clearLastDeepSleepReason(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	if (!remoteObject)
		return;

	JsonObject parameters, response;
	std::string result;
	int status = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T(methodName), parameters, response);
	if (Core::ERROR_NONE != status) {
		std::cout << "Invoke failed with return status :" << std::hex << status << std::endl;
	} else {
		response.ToString(result);
		/* Hack ? - remove the escape characters from the result for aesthetics. */
		result.erase(std::remove(result.begin(), result.end(), '\\'), result.end());
		std::cout << "Response: '" << result << "'" << std::endl;
	}
}

void enableMoca(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	if (!remoteObject)
		return;

	JsonObject parameters, response;
	std::string result;
	bool enable;

	std::cout << "Enable ? (0/1) :";
	std::cin >> enable;
	parameters["enable"] = enable;
	
	int status = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T(methodName), parameters, response);
	if (Core::ERROR_NONE != status) {
		std::cout << "Invoke failed with return status :" << std::hex << status << std::endl;
	} else {
		response.ToString(result);
		/* Hack ? - remove the escape characters from the result for aesthetics. */
		result.erase(std::remove(result.begin(), result.end(), '\\'), result.end());
		std::cout << "Response: '" << result << "'" << std::endl;
	}
}

/******************************** End : Handle Selection *******************************/


void showUsage(char *pName)
{
    std::cout << pName << " <Thunder Access Environment> <ip:port> <callSign>" << std::endl;
    std::cout << pName << " THUNDER_ACCESS " << SYSPLUGIN_SERVER_PORT << SYSPLUGIN_CALLSIGN << std::endl;
    exit(0);
}

/* System plugin Events */
const std::string SystemEventNames[] = {
    "onSampleEvent",
    "onSystemPowerStateChanged",
    "onSystemModeChanged",
    "onFirmwareUpdateInfoReceived",
    "onFirmwareUpdateStateChange",
    "onTemperatureThresholdChanged",
    "onMacAddressesRetreived",
    "onRebootRequest",
};

/* This section is related to the event handler implementation for Thunder Plugin Events. */

inline uint64_t TimeStamp() 
{
    return std::chrono::duration_cast<std::chrono::microseconds>
        (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

namespace Handlers {
    /* Common Event Handler */
    static void onEventHandler(const Core::JSON::String& parameters) {
        std::string message;
        parameters.ToString(message);
        /* Hack ? - remove the escape characters from the result for aesthetics. */
        message.erase(std::remove(message.begin(), message.end(), '\\'), message.end());
        std::cout << "[" << TimeStamp() << "][System-JSONRPCEvt] : " << message << std::endl;
    }
}

std::string getMethodName(SME_t SME)
{
    std::string methodName = "None";

    auto it = SMName.find(SME);
    if (SMName.end() != it) {
        methodName = it->second;
    }
    return methodName;
}


SME_t getChoice(void)
{
    int SMEOption;

    std::cout << "============================= Menu =========================" << std::endl;
    for (int i; i < SME_MAX; i++) {
        std::cout << " [" << i << "] " << setw(30) << getMethodName((SME_t)i) << "  " << std::endl;
        if (i%2 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << "==========> Enter Option(Number) :";
    std::cin >> SMEOption;
    if (SMEOption < 0)
        SMEOption = 0;
    return (SME_t)(((SME_t)SMEOption < SME_MAX)? SMEOption : 0);
}

/* System plugin - Invoke System plugin with supplied Method test payloads. */
int EvaluateMethods(JSONRPC::LinkType<Core::JSON::IElement>* remoteObject)
{
    int retStatus = 0;

    do {
        switch((retStatus = getChoice())) {
			case SME_cacheContains:
				cacheContains(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_clearLastDeepSleepReason: clearLastDeepSleepReason(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_enableMoca: enableMoca(getMethodName((SME_t)retStatus), remoteObject); break;
			/* TODO: implement Method options. */
            case SME_MAX:
            default:
                std::cout << "Selected method is ' " << getMethodName((SME_t)retStatus) << "'" << std::endl;
				std::cout << "Select SME_MAX to exit" << std::endl;
                break;
        }
    } while (retStatus != SME_MAX);

    return retStatus;
}

int main(int argc, char** argv)
{
    int retStatus = -1;
    std::string env, server, callsign, event;

    if (argc != 4) {
        showUsage(argv[0]);
    }

    for (int i = 0; i < argc; i++) {
        switch (i) {
            case 0: break; /* Self-name */
            case 1: env = argv[i]; break;
            case 2: server = argv[i]; break;
            case 3: callsign = argv[i]; break;
            default: showUsage(argv[0]); break;
        }
    }

    Core::SystemInfo::SetEnvironment(_T(env), (_T(server)));
    JSONRPC::LinkType<Core::JSON::IElement> *remoteObject = new JSONRPC::LinkType<Core::JSON::IElement>(_T(callsign), _T(""));

    if (remoteObject) {
        std::cout << "[" << TimeStamp() << "][System-MainFunctn] : Registering Event Handlers..." << std::endl;
        /* Experimental: Register a common Event Handler for all Events */
        for (int i = 0; i < std::extent<decltype(SystemEventNames), 0>::value; i++) {
            if (remoteObject->Subscribe<Core::JSON::String>(1000, _T(SystemEventNames[i]),
                        &Handlers::onEventHandler) == Core::ERROR_NONE) {
                std::cout << "[" << TimeStamp() << "][System-MainFunctn] : Subscribed to '"
                    << SystemEventNames[i] << "'" << std::endl;
            } else {
                std::cout << "[" << TimeStamp() << "][System-MainFunctn] : Failed to subscribe '"
                    << SystemEventNames[i] << "'" << std::endl;
            }
        }

        /* Busy loop. */
        retStatus = EvaluateMethods(remoteObject);

        /* Clean-Up */
        std::cout << "[" << TimeStamp() << "][System-MainFunctn] : Clean-Up triggered..." << std::endl;
        for (int i = 0; i < std::extent<decltype(SystemEventNames), 0>::value; i++) {
            remoteObject->Unsubscribe(1000, _T(SystemEventNames[i]));
            std::cout << "[" << TimeStamp() << "][System-MainFunctn] : Unsubscribed from '"
                << SystemEventNames[i] << "'"<< std::endl;
        }
        delete remoteObject;
    } else {
        std::cout << "[" << TimeStamp() << "][System-MainFunctn] : remoteObject creation failed" << std::endl;
    }

    return retStatus;
}

