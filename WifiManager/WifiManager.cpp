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

#include "WifiManager.h"
#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

#include <vector>
#include <utility>

#include "libIBus.h"

const short WPEFramework::Plugin::WifiManager::API_VERSION_NUMBER_MAJOR = 2;
const short WPEFramework::Plugin::WifiManager::API_VERSION_NUMBER_MINOR = 0;

namespace {
    using WPEFramework::Plugin::WifiManager;
    using WifiManagerConstMethod = uint32_t (WifiManager::*)(const JsonObject &parameters, JsonObject &response) const;
    using WifiManagerMethod = uint32_t (WifiManager::*)(const JsonObject &parameters, JsonObject &response);

    std::vector<std::pair<const char*, WifiManagerMethod>> mutableMethods = {
        {"stopScan", &WifiManager::stopScan},
        {"setEnabled", &WifiManager::setEnabled},
        {"connect", &WifiManager::connect},
        {"disconnect", &WifiManager::disconnect},
        {"cancelWPSPairing", &WifiManager::cancelWPSPairing},
        {"saveSSID", &WifiManager::saveSSID},
        {"clearSSID", &WifiManager::clearSSID},
        {"setSignalThresholdChangeEnabled", &WifiManager::setSignalThresholdChangeEnabled},
        {"getPairedSSID", &WifiManager::getPairedSSID},
        {"getPairedSSIDInfo", &WifiManager::getPairedSSIDInfo},
        {"isPaired", &WifiManager::isPaired},
        {"getCurrentState", &WifiManager::getCurrentState},
        {"getConnectedSSID", &WifiManager::getConnectedSSID},
    };

    std::vector<std::pair<const char*, WifiManagerConstMethod>> constMethods = {
        {"getQuirks", &WifiManager::getQuirks},
        {"startScan", &WifiManager::startScan},
        {"isSignalThresholdChangeEnabled", &WifiManager::isSignalThresholdChangeEnabled}
    };
}

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(WifiManager, WifiManager::API_VERSION_NUMBER_MAJOR, WifiManager::API_VERSION_NUMBER_MINOR);

        WifiManager::WifiManager()
        : PluginHost::JSONRPC(),
          apiVersionNumber(API_VERSION_NUMBER_MAJOR)
        {
            CreateHandler({ 2 });

            for (const auto& mapping : constMethods) {
                Register(mapping.first, mapping.second, this);
                GetHandler(2)->Register<JsonObject, JsonObject>(mapping.first, mapping.second, this);
            }

            for (const auto& mapping : mutableMethods) {
                Register(mapping.first, mapping.second, this);
                GetHandler(2)->Register<JsonObject, JsonObject>(mapping.first, mapping.second, this);
            }

            /* Version 1 only API */
            Register("initiateWPSPairing", &WifiManager::initiateWPSPairing, this);

            /* Version 2 API */
            GetHandler(2)->Register<JsonObject, JsonObject>("getSupportedSecurityModes", &WifiManager::getSupportedSecurityModes, this);
            GetHandler(2)->Register<JsonObject, JsonObject>("initiateWPSPairing", &WifiManager::initiateWPSPairing2, this);
        }

        WifiManager::~WifiManager()
        {
        }

        const string WifiManager::Initialize(PluginHost::IShell* service)
        {
            Utils::IARM::init();

            if (instance != nullptr) {
                LOGERR("Expecting 'instance' to be initially unset; two instances of the plugin?");
                return string("Expecting m_instance to be initially unset");
            }
            instance = this;

            // Initialize other parts of the implementation
            string const scanMessage = wifiScan.Initialize(service);
            string const eventsMessage = wifiEvents.Initialize(service);

            // Combine their error messages (if any)
            return scanMessage + eventsMessage;
        }

        void WifiManager::Deinitialize(PluginHost::IShell* service)
        {
            wifiScan.Deinitialize(service);

            instance = nullptr;
        }

        string WifiManager::Information() const
        {
            // No additional info to report.
            return string();
        }

        uint32_t WifiManager::getQuirks(const JsonObject& parameters, JsonObject& response) const
        {
            LOGINFOMETHOD();

            JsonArray array;
            response["quirks"] = array;

            returnResponse(true);
        }

        uint32_t WifiManager::getCurrentState(const JsonObject &parameters, JsonObject &response)
        {
            uint32_t result = wifiState.getCurrentState(parameters, response);

            return result;
        }

        uint32_t WifiManager::startScan(const JsonObject &parameters, JsonObject &response) const
        {
            LOGINFOMETHOD();

            uint32_t const result = wifiScan.startScan(parameters, response);

            LOGTRACEMETHODFIN();
            return result;
        }

        uint32_t WifiManager::stopScan(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();

            uint32_t const result = wifiScan.stopScan(parameters, response);

            LOGTRACEMETHODFIN();
            return result;
        }

        uint32_t WifiManager::getConnectedSSID(const JsonObject &parameters, JsonObject &response)
        {
            bool result = false;

            if(getConnectedSSID2(parameters, response))
            {
                result = true;
            }
            else
            {
                LOGWARN("getConnectedSSID2 Call get failed,ret value:%d\n",result);
            }

            returnResponse(result);
        }

        bool WifiManager::getConnectedSSID2(const JsonObject &parameters, JsonObject &response)
        {
            bool result = false;

            if(wifiState.getConnectedSSID(parameters, response))
            {
                result = true;
            }
            else
            {
                LOGWARN("getConnectedSSID Call get failed,ret value:%d\n",result);
            }

            return result;
        }

        uint32_t WifiManager::setEnabled(const JsonObject &parameters, JsonObject &response)
        {
            uint32_t result = wifiState.setEnabled(parameters, response);

            return result;
        }

        uint32_t WifiManager::connect(const JsonObject &parameters, JsonObject &response)
        {
            uint32_t result = wifiConnect.connect(parameters, response);

            LOGTRACEMETHODFIN();
            return result;
        }

        uint32_t WifiManager::disconnect(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();

            uint32_t result = wifiConnect.disconnect(parameters, response);

            LOGTRACEMETHODFIN();
            return result;
        }

        uint32_t WifiManager::initiateWPSPairing(const JsonObject &parameters, JsonObject &response)
        {
            uint32_t result = wifiWPS.initiateWPSPairing(parameters, response);

            return result;
        }

        uint32_t WifiManager::initiateWPSPairing2(const JsonObject &parameters, JsonObject &response)
        {
            uint32_t result = wifiWPS.initiateWPSPairing2(parameters, response);

            return result;
        }

        uint32_t WifiManager::cancelWPSPairing(const JsonObject &parameters, JsonObject &response)
        {
            uint32_t result = wifiWPS.cancelWPSPairing(parameters, response);

            return result;
        }

        uint32_t WifiManager::saveSSID(const JsonObject &parameters, JsonObject &response)
        {
            uint32_t result = wifiWPS.saveSSID(parameters, response);

            return result;
        }

        uint32_t WifiManager::clearSSID(const JsonObject &parameters, JsonObject &response)
        {
            uint32_t result = wifiWPS.clearSSID(parameters, response);

            return result;
        }

        uint32_t WifiManager::getPairedSSID(const JsonObject &parameters, JsonObject &response)
        {
            uint32_t result = wifiWPS.getPairedSSID(parameters, response);

            return result;
        }

        uint32_t WifiManager::getPairedSSIDInfo(const JsonObject &parameters, JsonObject &response)
        {
            uint32_t result = wifiWPS.getPairedSSIDInfo(parameters, response);

            return result;
        }

        uint32_t WifiManager::isPaired(const JsonObject &parameters, JsonObject &response)
        {
            uint32_t result = wifiWPS.isPaired(parameters, response);

            return result;
        }

        uint32_t WifiManager::setSignalThresholdChangeEnabled(const JsonObject &parameters, JsonObject &response)
        {
            uint32_t result = wifiSignalThreshold.setSignalThresholdChangeEnabled(parameters, response);

            return result;
        }

        uint32_t WifiManager::isSignalThresholdChangeEnabled(const JsonObject &parameters, JsonObject &response) const
        {
            uint32_t result = wifiSignalThreshold.isSignalThresholdChangeEnabled(parameters, response);

            return result;
        }

        uint32_t WifiManager::getSupportedSecurityModes(const JsonObject &parameters, JsonObject &response)
        {
            uint32_t result = wifiState.getSupportedSecurityModes(parameters, response);

            return result;
        }

        /**
         * \brief Send an event that the WIFI state changed.
         *
         * \param state The current state of the wifi.
         * \param isLNF Whether this is a lost-and-found access point.
         */
        void WifiManager::onWIFIStateChanged(WifiState state, bool isLNF)
        {
            JsonObject params;
            params["state"] = static_cast<int>(state);
            params["isLNF"] = isLNF;
            if (!isLNF)
            {
                wifiState.setWifiStateCache(true, state);
                wifiState.resetWifiStateConnectedCache(false);
                wifiWPS.updateWifiWPSCache(false);
            }
            sendNotify("onWIFIStateChanged", params);
            GetHandler(2)->Notify("onWIFIStateChanged", params);
            if (state == WifiState::CONNECTED)
            {
                wifiSignalThreshold.setSignalThresholdChangeEnabled(true);
            }
            else
            {
                wifiSignalThreshold.setSignalThresholdChangeEnabled(false);
            }
        }

        /**
         * \brief Send an event that an WIFI related error has occurred.
         *
         * \param code The code of the error.
         */
        void WifiManager::onError(ErrorCode code)
        {
            JsonObject params;
            params["code"] = static_cast<int>(code);
            sendNotify("onError", params);
            GetHandler(2)->Notify("onError", params);
        }

        void WifiManager::onSSIDsChanged()
        {
            wifiWPS.updateWifiWPSCache(false);
            wifiState.resetWifiStateConnectedCache(false);
            sendNotify("onSSIDsChanged", JsonObject());
            GetHandler(2)->Notify("onSSIDsChanged", JsonObject());
        }

        void WifiManager::onWifiSignalThresholdChanged(float signalStrength, const std::string &strength)
        {
            JsonObject params;
            params["signalStrength"] = std::to_string(signalStrength);
            params["strength"] = strength;
            Notify("onWifiSignalThresholdChanged", params);
        }

        /**
         * \brief Send an event that the available SSIDs have updated.
         *
         * Service manager published a JSON object with one entry 'ssids' with an associated array of access points. For backwards compatibility the plugin does the same.
         *
         * \param ssids The ssid list as an object with a single entry.
         *
         */
        void WifiManager::onAvailableSSIDs(JsonObject const& ssids)
        {
            sendNotify("onAvailableSSIDs", ssids);
            GetHandler(2)->Notify("onAvailableSSIDs", ssids);
        }

        /**
        * \brief Get the current WifiManager instance
        *
        * WPEFramework will only have one WifiManger plugin at any one time and so only one instance of this class.
        *
        * \return The single instance.
        */
        WifiManager& WifiManager::getInstance() {
            if (!instance) {
                LOGERR("No instances of 'WifiManager' have been created");
                throw std::logic_error("No instances of 'WifiManager' have been created");
            }

            return *instance;
        }

        // In the WPEFramework there should only be a single instance of this class. We need to be able to find this instance when routing events the IARM bus.
        // This instance is generally accessed via WifiManager::getInstance.
        WifiManager* WifiManager::instance = nullptr;
    } // namespace Plugin
} // namespace WPEFramework
