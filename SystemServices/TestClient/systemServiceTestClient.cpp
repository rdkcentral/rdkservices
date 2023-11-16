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
#include <cassert>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <type_traits>
#include <core/core.h>
#include <websocket/websocket.h>
#include <securityagent/SecurityTokenUtil.h>

#define SYSPLUGIN_CALLSIGN		"org.rdk.System"
#define SYSPLUGIN_SERVER_PORT	"127.0.0.1:9998"
#define MAX_LENGTH 1024

using namespace std;
using namespace WPEFramework;

/* Thunder-Security: Security Token */
unsigned char g_ucSecToken[MAX_LENGTH] = {0};
std::string g_strSecToken = "";

uint64_t TimeStamp(void)
{
	return std::chrono::duration_cast<std::chrono::microseconds>
		(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

std::string makePretty(std::string result)
{
	/* Hack ? - remove the escape characters from the result for aesthetics. */
	result.erase(std::remove(result.begin(), result.end(), '\\'), result.end());
	return result;
}

typedef enum SME_t {
#ifdef DEBUG
	SME_sampleSystemServiceAPI,
#endif /* DEBUG */
	SME_cacheContains = 1,
	SME_clearLastDeepSleepReason,
	SME_enableMoca,
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
	SME_setNetworkStandbyMode,
	SME_getNetworkStandbyMode,
	SME_getPowerStateIsManagedByDevice,
	SME_getTimeZones,
	SME_uploadLogs,
	SME_MAX,
};

std::map<SME_t, std::string> SMName = {
#ifdef DEBUG
	{SME_sampleSystemServiceAPI, "sampleSystemServiceAPI"},
#endif /* DEBUG */
	{SME_cacheContains, "cacheContains"},
	{SME_clearLastDeepSleepReason, "clearLastDeepSleepReason"},
	{SME_enableMoca, "enableMoca"},
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
	{SME_setNetworkStandbyMode, "setNetworkStandbyMode"},
	{SME_getNetworkStandbyMode, "getNetworkStandbyMode"},
	{SME_getPowerStateIsManagedByDevice, "getPowerStateIsManagedByDevice"},
	{SME_getTimeZones, "getTimeZones"},
	{SME_uploadLogs, "uploadLogs"},
	{SME_MAX, "MAX"},
};

bool invokeJSONRPC(JSONRPC::LinkType<Core::JSON::IElement> *remoteObject, std::string method, JsonObject &param, JsonObject &result)
{
	bool ret = false;
	uint32_t retStatus = Core::ERROR_GENERAL;
	std::string response;
	assert(remoteObject != NULL);

	param.ToString(response);
	printf("\ninvokeJSONRPC '%s' with param: '%s'\n", method.c_str(), response.c_str());

	retStatus = remoteObject->Invoke<JsonObject, JsonObject>(5000, _T(method), param, result);
	if (Core::ERROR_NONE != retStatus) {
		printf("\nremoteObject->Invoke '%s' failed [retStatus: 0x%x]\n", method.c_str(), retStatus);
	} else {
		ret = true;
	}
	return ret;
}

/******************************* Begin: Handle Selection *******************************/

#ifdef DEBUG
void sampleSystemServiceAPI(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);
	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}
#endif /* DEBUG */

void cacheContains(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);
	JsonObject parameters, response;
	std::string result;
	std::string key;

	printf("\nKey name to check :");
	std::cin >> key;
	parameters["key"] = key;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void clearLastDeepSleepReason(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);
	JsonObject parameters, response;
	std::string result;
	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void enableMoca(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);
	JsonObject parameters, response;
	std::string result;
	bool enable = false;

	printf("\nEnable ? {value: 0/1} :");
	std::cin >> enable;
	parameters["value"] = enable;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getAvailableStandbyModes(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getCachedValue(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;
	std::string key;

	printf("\nKey name to check :");
	std::cin >> key;
	parameters["key"] = key;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getCoreTemperature(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getDeviceInfo(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;
	JsonArray paramsArray;
	std::string input;

	printf("\nInput 'key(s)' [Type 'end' to finish] :");
	do {
		input.empty();
		std::cin >> input;

		if (!input.length() || (input == "end"))
			break;

		paramsArray.Add(input);
	} while (true);

	/* TODO: "params" or "param" ? */
	parameters["params"] = paramsArray;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getDownloadedFirmwareInfo(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getFirmwareDownloadPercent(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getFirmwareUpdateInfo(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result, guid;

	/* GUID is optional. */
	printf("\nGUID (Optional):");
	std::cin >> guid;

	if (guid.length()) {
		parameters["GUID"] = guid;
	}

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getFirmwareUpdateState(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getLastDeepSleepReason(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getMacAddresses(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getMilestones(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getMode(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getPowerState(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getPreferredStandbyMode(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getPreviousRebootInfo(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getPreviousRebootInfo2(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getPreviousRebootReason(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getRFCConfig(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result, rfcListStr;
	JsonArray rfcListArray;

	printf("\nInput required RFC List [type 'end' to finish]: ");
	do {
		std::cin >> rfcListStr;
		if (!rfcListStr.length() || (rfcListStr == "end"))
			break;
		rfcListArray.Add(rfcListStr);
	} while (true);

	/* TODO: Documentation updation required. */
	parameters["rfcList"] = rfcListArray;
	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getSerialNumber(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getStateInfo(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result, queryState;

	printf("\nInput property state to be queried :");
	std::cin >> queryState;

	parameters["param"] = queryState;
	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getSystemVersions(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getTemperatureThresholds(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getTimeZoneDST(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void getXconfParams(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void isGzEnabled(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void queryMocaStatus(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void reboot(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result, reason;

	/* Optional: reboot reason. */
	printf("\nInput reboot reason (Optional) :");
	std::cin >> reason;
	if (reason.length()) {
		/* TODO: Update code & Doc to match SM Doc - "reason" with "rebootReason". */
		parameters["rebootReason"] = reason;
	}

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void removeCacheKey(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result, key;

	printf("\nInput 'key' to be removed :");
	std::cin >> key;

	parameters["key"] = key;
	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void requestSystemUptime(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void setCachedValue(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	JsonObject parameters, response;
	std::string result, key, value;

	printf("\nInput cache 'key' :");
	std::cin >> key;
	printf("\nInput cache 'value' :");
	std::cin >> value;

	parameters["key"] = key;
	parameters["value"] = value;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void setDeepSleepTimer(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;
	unsigned int seconds = 0;

	printf("\nInput Seconds :");
	std::cin >> seconds;

	parameters["seconds"] = seconds;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void setGzEnabled(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;
	bool enabled = false;

	/* TODO: correct doc with "enabled" */
	printf("\nEnable ? (0/1) :");
	std::cin >> enabled;
	parameters["enabled"] = enabled;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void setMode(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response, modeInfo;
	std::string result, mode;
	int duration = 0;

	printf("\nInput 'mode' :");
	std::cin >> mode;
	printf("\nInput 'duration(in seconds)' :");
	std::cin >> duration;

	modeInfo["mode"] = mode;
	modeInfo["duration"] = duration;
	parameters["modeInfo"] = modeInfo;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void setPowerState(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result, powerState, standbyReason;

	printf("\nInput 'powerState' :");
	std::cin >> powerState;
	printf("\nInput 'standbyReason' :");
	std::cin >> standbyReason;

	parameters["powerState"] = powerState;
	parameters["standbyReason"] = standbyReason;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void setPreferredStandbyMode(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result, standbyMode;

	printf("\nInput 'standbyMode' :");
	std::cin >> standbyMode;

	parameters["standbyMode"] = standbyMode;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void setTemperatureThresholds(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response, thresholds;
	std::string result, WARN, MAX;

	printf("\nInput 'WARN' level :");
	std::cin >> WARN;
	printf("\nInput 'MAX' level :");
	std::cin >> MAX;

	thresholds["WARN"] = WARN;
	thresholds["MAX"] = MAX;
	parameters["thresholds"] = thresholds;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void setTimeZoneDST(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result, timeZone;

	printf("\nInput 'timeZone' :");
	std::cin >> timeZone;

	parameters["timeZone"] = timeZone;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void updateFirmware(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
	printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

	JsonObject parameters, response;
	std::string result;

	if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
		response.ToString(result);
		printf("\nResponse: '%s'\n", result.c_str());
	}
}

void setNetworkStandbyMode(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
    printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

    JsonObject parameters, response;
    std::string result;
    bool nwStandby;

    printf("\nInput 'nwStandby' :");
    std::cin >> nwStandby;

    parameters["nwStandby"] = nwStandby;

    if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
        response.ToString(result);
        printf("\nResponse: '%s'\n", result.c_str());
    }
}

void getNetworkStandbyMode(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
    printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

    JsonObject parameters, response;
    std::string result;

    if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
        response.ToString(result);
        printf("\nResponse: '%s'\n", result.c_str());
    }
}

void getPowerStateIsManagedByDevice(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
    printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

    JsonObject parameters, response;
    std::string result;

    if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
        response.ToString(result);
        printf("\nResponse: '%s'\n", result.c_str());
    }
}

void getTimeZones(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
    printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

    JsonObject parameters, response;
    std::string result;

    if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
        response.ToString(result);
        printf("\nResponse: '%s'\n", result.c_str());
    }
}

void uploadLogs(std::string methodName, JSONRPC::LinkType<Core::JSON::IElement> *remoteObject)
{
    printf("[%llu] Inside (%s)\n", TimeStamp(), __FUNCTION__);

    JsonObject parameters, response;
    std::string result, url;

    printf("\nInput 'url' :");
    std::cin >> url;

    parameters["url"] = url;

    if (invokeJSONRPC(remoteObject, methodName, parameters, response)) {
        response.ToString(result);
        printf("\nResponse: '%s'\n", result.c_str());
    }
}

/******************************** End : Handle Selection *******************************/

void showUsage(char *pName)
{
	printf("%s <Thunder Access Environment> <ip:port> <callSign> <token>\n", pName);
	printf("%s THUNDER_ACCESS %s %s <token>\n", pName, SYSPLUGIN_SERVER_PORT, SYSPLUGIN_CALLSIGN);
	exit(0);
}

/* This section is related to the event handler implementation for Thunder Plugin Events. */
namespace Handlers {
	/* Common Event Handler */
	static void onEventHandler(const JsonObject& parameters) {
		std::string message;
		parameters.ToString(message);
		printf("\n[%llu][System-JSONRPCEvt]: '%s'\n", TimeStamp(), makePretty(message).c_str());
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

int getChoice(void)
{
	int SMEOption;

	printf("\n================================= Menu =================================\n");
	for (int i = 0 ; i < SME_MAX; i++) {
		printf("(%2d) %-30s	 ", i, getMethodName((SME_t)i).c_str());
		if (i%2) {
			printf("\n");
		}
	}
	printf("\n==========> Enter Option(Number)[MAX+ to Quit] : ");
	scanf("%d", &SMEOption);
	printf("\nReceived SMEOption = %d\n", SMEOption);
	printf("\n============================== Processing ==============================\n");
	if (SMEOption < 0)
		SMEOption = SME_MAX;
	return ((SMEOption < SME_MAX)? SMEOption : SME_MAX);
}

/* System plugin - Invoke System plugin with supplied Method test payloads. */
int EvaluateMethods(JSONRPC::LinkType<Core::JSON::IElement>* remoteObject)
{
	int retStatus = 0;

	do {
		switch((retStatus = getChoice())) {
			case SME_cacheContains: cacheContains(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_clearLastDeepSleepReason: clearLastDeepSleepReason(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_enableMoca: enableMoca(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getAvailableStandbyModes: getAvailableStandbyModes(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getCachedValue: getCachedValue(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getCoreTemperature: getCoreTemperature(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getDeviceInfo: getDeviceInfo(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getDownloadedFirmwareInfo: getDownloadedFirmwareInfo(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getFirmwareDownloadPercent: getFirmwareDownloadPercent(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getFirmwareUpdateInfo: getFirmwareUpdateInfo(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getFirmwareUpdateState: getFirmwareUpdateState(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getLastDeepSleepReason: getLastDeepSleepReason(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getMacAddresses: getMacAddresses(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getMilestones: getMilestones(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getMode: getMode(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getPowerState: getPowerState(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getPreferredStandbyMode: getPreferredStandbyMode(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getPreviousRebootInfo: getPreviousRebootInfo(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getPreviousRebootInfo2: getPreviousRebootInfo2(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getPreviousRebootReason: getPreviousRebootReason(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getRFCConfig: getRFCConfig(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getSerialNumber: getSerialNumber(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getStateInfo: getStateInfo(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getSystemVersions: getSystemVersions(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getTemperatureThresholds: getTemperatureThresholds(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getTimeZoneDST: getTimeZoneDST(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getXconfParams: getXconfParams(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_isGzEnabled: isGzEnabled(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_queryMocaStatus: queryMocaStatus(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_reboot: reboot(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_removeCacheKey: removeCacheKey(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_requestSystemUptime: requestSystemUptime(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_setCachedValue: setCachedValue(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_setDeepSleepTimer: setDeepSleepTimer(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_setGzEnabled: setGzEnabled(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_setMode: setMode(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_setPowerState: setPowerState(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_setPreferredStandbyMode: setPreferredStandbyMode(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_setTemperatureThresholds: setTemperatureThresholds(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_setTimeZoneDST: setTimeZoneDST(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_updateFirmware: updateFirmware(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_setNetworkStandbyMode: setNetworkStandbyMode(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getNetworkStandbyMode: getNetworkStandbyMode(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getPowerStateIsManagedByDevice: getPowerStateIsManagedByDevice(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_getTimeZones: getTimeZones(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_uploadLogs: uploadLogs(getMethodName((SME_t)retStatus), remoteObject); break;
			case SME_MAX:
			default:
				break;
		}
	} while (retStatus != SME_MAX);

	return retStatus;
}

int main(int argc, char** argv)
{
	int retStatus = -1;
	std::string env, server, callsign, sToken;

	if (argc != 5) {
		showUsage(argv[0]);
	}

	for (int i = 0; i < argc; i++) {
		switch (i) {
			case 0: break; /* Self-name */
			case 1: env = argv[i]; break;
			case 2: server = argv[i]; break;
			case 3: callsign = argv[i]; break;
			case 4: sToken = argv[i]; break;
			default: showUsage(argv[0]); break;
		}
	}

	Core::SystemInfo::SetEnvironment(_T(env), (_T(server)));

	/* Thunder-Security: Get Security Token */
	if (sToken.length()) {
		g_strSecToken = "token=" + sToken;
		printf("\nObtained token: '%s'\n", g_strSecToken.c_str());
	}

	JSONRPC::LinkType<Core::JSON::IElement> *remoteObject = new JSONRPC::LinkType<Core::JSON::IElement>(_T(callsign), _T(""), false, g_strSecToken);

	if (remoteObject) {
		/* System plugin Events */
		std::vector<std::string> SystemEventNames;
#ifdef DEBUG
		SystemEventNames.push_back("onSampleEvent");
#endif /* DEBUG */
		SystemEventNames.push_back("onSystemPowerStateChanged");
		SystemEventNames.push_back("onSystemModeChanged");
		SystemEventNames.push_back("onFirmwareUpdateInfoReceived");
		SystemEventNames.push_back("onFirmwareUpdateStateChange");
		SystemEventNames.push_back("onTemperatureThresholdChanged");
		SystemEventNames.push_back("onMacAddressesRetreived");
		SystemEventNames.push_back("onRebootRequest");

		printf("\n[%llu][System-MainFunctn] : Register a common Event Handler for all Events...\n", TimeStamp());
		/* Experimental: Register a common Event Handler for all Events */
		for (std::string eventName : SystemEventNames) {
			if (remoteObject->Subscribe<JsonObject>(1000, _T(eventName),
						&Handlers::onEventHandler) == Core::ERROR_NONE) {
				printf("\n[%llu][System-MainFunctn] : Subscribed to '%s'...\n",
					TimeStamp(), eventName.c_str());
			} else {
				printf("\n[%llu][System-MainFunctn] : Failed to subscribed to '%s'...\n",
					TimeStamp(), eventName.c_str());
			}
		}

		/* Busy loop. */
		printf("\n[%llu][System-MainFunctn] : Method validation loop...\n", TimeStamp());
		retStatus = EvaluateMethods(remoteObject);

		/* Clean-Up */
		printf("\n[%llu][System-MainFunctn] : Clean-Up triggered...\n", TimeStamp());

		for (std::string eventName : SystemEventNames) {
			remoteObject->Unsubscribe(1000, _T(eventName));
			printf("\n[%llu][System-MainFunctn] : Unsubscribed from '%s'...\n",
					TimeStamp(), eventName.c_str());
		}
		delete remoteObject;
	} else {
		printf("\n[%llu][System-MainFunctn] : remoteObject creation failed...\n", TimeStamp());
	}

	return retStatus;
}

