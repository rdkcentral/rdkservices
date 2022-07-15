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

#pragma once

#include "Module.h"
#include "WifiManagerDefines.h"
#include "WifiManagerInterface.h"
#include "impl/WifiManagerWPS.h"
#include "impl/WifiManagerState.h"
#include "impl/WifiManagerConnect.h"
#include "impl/WifiManagerSignalThreshold.h"
#include "impl/WifiManagerScan.h"
#include "impl/WifiManagerEvents.h"

namespace WPEFramework {

    namespace Plugin {
        // This is a server for a JSONRPC communication channel.
        // For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
        // By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
        // This realization of this interface implements, by default, the following methods on this plugin
        // - exists
        // - register
        // - unregister
        // Any other methood to be handled by this plugin  can be added can be added by using the
        // templated methods Register on the PluginHost::JSONRPC class.
        // As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
        // this class exposes a public method called, Notify(), using this methods, all subscribed clients
        // will receive a JSONRPC message as a notification, in case this method is called.
        class WifiManager : public WifiManagerInterface, public PluginHost::IPlugin, public PluginHost::JSONRPC {
        public:
            WifiManager();
            virtual ~WifiManager();
            WifiManager(const WifiManager&) = delete;
            WifiManager& operator=(const WifiManager&) = delete;

            static const short API_VERSION_NUMBER_MAJOR;
            static const short API_VERSION_NUMBER_MINOR;

            //Begin methods
            virtual uint32_t getQuirks(const JsonObject& parameters, JsonObject& response) const override;
            virtual uint32_t getCurrentState(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t startScan(const JsonObject& parameters, JsonObject& response) const override;
            virtual uint32_t stopScan(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t getConnectedSSID(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t setEnabled(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t connect(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t disconnect(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t initiateWPSPairing(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t initiateWPSPairing2(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t cancelWPSPairing(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t saveSSID(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t clearSSID(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t getPairedSSID(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t getPairedSSIDInfo(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t isPaired(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t setSignalThresholdChangeEnabled(const JsonObject& parameters, JsonObject& response) override;
            virtual uint32_t isSignalThresholdChangeEnabled(const JsonObject& parameters, JsonObject& response) const;
            virtual uint32_t getSupportedSecurityModes(const JsonObject& parameters, JsonObject& response) override;
            //End methods

            //Begin events
            virtual void onWIFIStateChanged(WifiState state, bool isLNF) override;
            virtual void onError(ErrorCode code) override;
            virtual void onSSIDsChanged() override;
            virtual void onWifiSignalThresholdChanged(float signalStrength, const std::string &strength) override;
            virtual void onAvailableSSIDs(JsonObject const& ssids) override;
            //End events

            //Build QueryInterface implementation, specifying all possible interfaces to be returned.
            BEGIN_INTERFACE_MAP(WifiManager)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

            //IPlugin methods
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override;

            uint32_t getApiVersionNumber() const {return apiVersionNumber;};
            void setApiVersionNumber(uint32_t apiVersion) {apiVersionNumber = apiVersion;};
            bool getConnectedSSID2(const JsonObject &parameters, JsonObject &response);

            //Internal methods
            static WifiManager& getInstance();

        private:
            uint32_t apiVersionNumber;
            static WifiManager* instance;
            WifiManagerWPS wifiWPS;
            WifiManagerScan wifiScan;
            WifiManagerState wifiState;
            WifiManagerEvents wifiEvents;
            WifiManagerConnect wifiConnect;
            WifiManagerSignalThreshold wifiSignalThreshold;
        };
    } // namespace Plugin
} // namespace WPEFramework
