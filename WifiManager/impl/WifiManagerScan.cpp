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
 * WifiManager implementation related to performing scans of wireless networks.
 *
 */

#include "WifiManagerScan.h"
#include "../WifiManager.h" // Need access to WifiManager::getInstance so can't use 'WifiManagerInterface.h'
#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

// RDK
#include "rdk/iarmbus/libIBus.h"
#include "wifiSrvMgrIarmIf.h"

// std
#include <sstream>
#include <regex>

namespace Utils {
std::string formatIARMResult(IARM_Result_t result)
{
    switch (result) {
    case IARM_RESULT_SUCCESS:       return std::string("IARM_RESULT_SUCCESS [success]");
    case IARM_RESULT_INVALID_PARAM: return std::string("IARM_RESULT_INVALID_PARAM [invalid input parameter]");
    case IARM_RESULT_INVALID_STATE: return std::string("IARM_RESULT_INVALID_STATE [invalid state encountered]");
    case IARM_RESULT_IPCCORE_FAIL:  return std::string("IARM_RESULT_IPCORE_FAIL [underlying IPC failure]");
    case IARM_RESULT_OOM:           return std::string("IARM_RESULT_OOM [out of memory]");
    default:
        std::ostringstream tmp;
        tmp << result << " [unknown IARM_Result_t]";
        return tmp.str();
    }
}
}

using namespace WPEFramework;
using namespace WPEFramework::Plugin;

namespace WPEC = WPEFramework::Core;
namespace WPEJ = WPEFramework::Core::JSON;

namespace
{
    // Commonly used strings to avoid typos
    char const* const g_error = "error";
    char const* const g_ssid = "ssid";
    char const* const g_incremental = "incremental";
    char const* const g_frequency = "frequency";
    char const* const g_getAvailableSSIDs = "getAvailableSSIDs";
    char const* const g_getAvailableSSIDsWithName = "getAvailableSSIDsWithName";
    char const* const g_moreData = "moreData";
    char const* const g_ssids = "ssids";
    char const* const g_SSID_name = "SSID_name";
    char const* const g_timeout = "timeout";
}

WifiManagerScan::Filter WifiManagerScan::filter = {};

/**
 * \brief Register event handlers.
 *
 */
std::string WifiManagerScan::Initialize(PluginHost::IShell*)
{
    LOGINFO("initializing");

    // Register event handlers for wireless scan related events
    IARM_Result_t res;
    IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDs, WifiManagerScan::iarmEventHandler));
    IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDsIncr, WifiManagerScan::iarmEventHandler));

    // Successful
    return string();
}

/**
 * \brief Unregister event handlers.
 *
 */
void WifiManagerScan::Deinitialize(PluginHost::IShell* service)
{
    LOGINFO("deinitializing");

    IARM_Result_t res;
    IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDs));
    IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDsIncr));
}

/**
 * \brief Get the available access points asynchronously.
 *
 * The results are published on via the "onAvailableSSIDs" event.
 *
 * \param parameters        Must include 'incremental'. Optionally includes 'ssid' and/or 'frequency'.
 * \param[out] response     Always includes 'success' if successful.
 * \return                  A code indicating success.
 *
 */

uint32_t WifiManagerScan::startScan(const JsonObject &parameters, JsonObject &response) const
{
    LOGINFOMETHOD();
    returnIfBooleanParamNotFound(parameters, g_incremental);
    const bool incremental = parameters[g_incremental].Boolean();

    filter = {};

    if (parameters.HasLabel(g_ssid)) {
        std::string ssid;
        getStringParameter(g_ssid, ssid);
        if (ssid.length()) {
            filter.onSsid = true;
            filter.ssid = ssid;
        }
    }
    if (parameters.HasLabel(g_frequency)) {
        std::string frequency;
        getStringParameter(g_frequency, frequency);
        if (frequency.length()) {
            filter.onFrequency = true;
            filter.frequency = frequency;
        }
    }

    if (incremental)
    {
        return getAvailableSSIDsAsyncIncr(parameters, response);
    }
    else
    {
        return getAvailableSSIDsAsync(parameters, response);
    }
}

/**
 * \brief Get the available access points asynchronously.
 *
 * The results are published on via the "onAvailableSSIDs" event.
 *
 * \param parameters    There are no parameters to this method.
 * \param[out] response Whether the call succeeded, not the results.
 * \return              A status code.
 *
 */
uint32_t WifiManagerScan::getAvailableSSIDsAsync(const JsonObject& parameters, JsonObject& response) const
{
    LOGINFOMETHOD();

    // There are no parameters
    IARM_Bus_WiFiSrvMgr_SsidList_Param_t param;
    memset(&param, 0, sizeof(param));

    // Issue the query via IARM bus, the response will be sent as an event
    IARM_Result_t res;
    IARM_CHECK( IARM_Bus_Call(
                    IARM_BUS_NM_SRV_MGR_NAME,
                    IARM_BUS_WIFI_MGR_API_getAvailableSSIDsAsync,
                    reinterpret_cast<void *>(&param),
                    sizeof(IARM_Bus_WiFiSrvMgr_SsidList_Param_t)) );
    if(res == IARM_RESULT_SUCCESS) {
        returnResponse(true);
    } else {
        std::ostringstream reason;
        reason << "Invoking 'IARM_BUS_WIFI_MGR_API_getAvailableSSIDsAsync' failed with error '" << Utils::formatIARMResult(res) << "'";
        response[g_error] = reason.str();
        LOGERR("%s", reason.str().c_str());
        returnResponse(false);
    }
}

/**
 * \brief Get the available access points asynchronously and in increments.
 *
 * The results are published via the "onAvailableSSIDsIncr" event. Currently this is done in three stages:
 * high priority 5GHz; 2.4GHz; low priority 5GHz.
 *
 * \param parameters    There are no parameters to this method.
 * \param[out] response Whether the call succeeded, not the results.
 * \return              A status code.
 *
 */
uint32_t WifiManagerScan::getAvailableSSIDsAsyncIncr(const JsonObject &parameters, JsonObject &response) const
{
    LOGINFOMETHOD()

    // There are no parameters
    IARM_Bus_WiFiSrvMgr_SsidList_Param_t param;
    memset(&param, 0, sizeof(param));

    // Issue the query via IARM bus, the response will be sent as an event
    IARM_Result_t res;
    IARM_CHECK( IARM_Bus_Call(
                    IARM_BUS_NM_SRV_MGR_NAME,
                    IARM_BUS_WIFI_MGR_API_getAvailableSSIDsAsyncIncr,
                    reinterpret_cast<void *>(&param),
                    sizeof(IARM_Bus_WiFiSrvMgr_SsidList_Param_t)) );
    if(res == IARM_RESULT_SUCCESS) {
        returnResponse(true);
    } else {
        std::ostringstream reason;
        reason << "Invoking 'IARM_BUS_WIFI_MGR_API_getAvailableSSIDsAsyncIncr' failed with error '" << Utils::formatIARMResult(res) << "'";
        response[g_error] = reason.str();
        LOGERR("%s", reason.str().c_str());
        returnResponse(false);
    }
}

/**
 * \brief Cancel a current incremental asynchronous scan started by 'getAvailableSSIDsAsyncIncr'.
 *
 * \param parameters    Ignored.
 * \param[out] response Will contain 'success' key.
 * \return Error code.
 *
 */
uint32_t WifiManagerScan::stopScan(const JsonObject& parameters, JsonObject& response)
{
    LOGINFOMETHOD()

    // No parameters
    IARM_Bus_WiFiSrvMgr_Param_t param;
    memset(&param, 0, sizeof(param));

    IARM_Result_t res;
    IARM_CHECK( IARM_Bus_Call(
                    IARM_BUS_NM_SRV_MGR_NAME,
                    IARM_BUS_WIFI_MGR_API_stopProgressiveWifiScanning,
                    reinterpret_cast<void*>(&param),
                    sizeof(IARM_Bus_WiFiSrvMgr_Param_t)) );

    returnResponse(res == IARM_RESULT_SUCCESS);
}

/**
 * \brief Handle events from the IARM bus relating to wireless scanning.
 *
 * \param owner   Where the event originated.
 * \param eventId The unique identifier of the oevent.
 * \param data    Data associated with the event.
 * \param len     Length of 'data' in bytes.
 *
 */
void WifiManagerScan::iarmEventHandler(char const* owner, IARM_EventId_t eventId, void* data, size_t len)
{
    // Only care about events originating from the network server manager
    if (strcmp(owner, IARM_BUS_NM_SRV_MGR_NAME) != 0)
        return;

    if ((eventId == IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDs) || (eventId == IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDsIncr)) {
        IARM_BUS_WiFiSrvMgr_EventData_t const* eventData = reinterpret_cast<IARM_BUS_WiFiSrvMgr_EventData_t*>(data);

        LOGINFO("Event IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDs[Incr] received. '%s'", eventData->data.wifiSSIDList.ssid_list);

        // The returned SSIDs are in a JSON document
        std::string const serialized(eventData->data.wifiSSIDList.ssid_list);
        JsonObject eventDocument;
        WPEC::OptionalType<WPEJ::Error> error;
        if (!WPEJ::IElement::FromString(serialized, eventDocument, error)) {
            LOGERR("Failed to parse JSON document containing SSIDs. Due to: %s", WPEJ::ErrorDisplayMessage(error).c_str());
            return;
        }
        if ((!eventDocument.HasLabel(g_getAvailableSSIDs)) || (eventDocument[g_getAvailableSSIDs].Content() != WPEJ::Variant::type::ARRAY)) {
            LOGERR("JSON document does not have key 'getAvailableSSIDs' as array");
            return;
        }

        JsonArray ssids = eventDocument[g_getAvailableSSIDs].Array();
        applyFilter(ssids, filter);

        JsonObject params;
        params[g_ssids] = ssids;
        params[g_moreData] = eventData->data.wifiSSIDList.more_data;
        WifiManager::getInstance().onAvailableSSIDs(params);
    }
}

void WifiManagerScan::applyFilter(JsonArray &ssids, const WifiManagerScan::Filter &filter) {
    JsonArray result;
    std::regex re;
    std::smatch m;
    if(filter.onSsid)
    {
        try
        {
            re = std::regex(filter.ssid);
        }
        catch(const std::regex_error &e)
        {
           LOGERR("Incorrect regex: %s", e.what());
        }
    }

    for(int i=0; i<ssids.Length(); i++) {
        JsonObject object = ssids[i].Object();
        if(filter.onSsid) {
            std::string str = object[g_ssid].String();

            if(!std::regex_match(str, m, re)){
                LOGINFO("SSID filter out %s ~= %s", str.c_str(), filter.ssid.c_str());
                continue;
            }
        }
        if(filter.onFrequency && object[g_frequency].String() != filter.frequency) {
            LOGINFO("Frequency filter out %s != %s", object[g_frequency].String().c_str(), filter.frequency.c_str());
            continue;
        }

        result.Add(object);
    }
    ssids = result;
}
