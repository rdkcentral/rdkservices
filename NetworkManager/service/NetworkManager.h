/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2022 RDK Management
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

// Include the interface we created
//#include <interfaces/INetworkManager.h>
#include "INetworkManager.h"
#include "NetworkManagerLogger.h"

#include <string>
#include <atomic>

namespace WPEFramework
{
    namespace Plugin
    {
        /**
         * NetworkManager plugin that exposes an API over both COM-RPC and JSON-RPC
         *
         */
        class NetworkManager : public PluginHost::IPlugin, public PluginHost::JSONRPC, public PluginHost::ISubSystem::IInternet
        {
            /**
             * Our notification handling code
             *
             * Handle both the Activate/Deactivate notifications and provide a handler
             * for notifications raised by the COM-RPC API
             */
            class Notification : public RPC::IRemoteConnection::INotification,
                                 public Exchange::INetworkManager::INotification
            {
            private:
                Notification() = delete;
                Notification(const Notification &) = delete;
                Notification &operator=(const Notification &) = delete;
                string InterfaceStateToString(Exchange::INetworkManager::InterfaceState event)
                {
                    switch (event)
                    {
                        case Exchange::INetworkManager::INTERFACE_ADDED:
                            return "INTERFACE_ADDED";
                        case Exchange::INetworkManager::INTERFACE_LINK_UP:
                            return "INTERFACE_LINK_UP";
                        case Exchange::INetworkManager::INTERFACE_LINK_DOWN:
                            return "INTERFACE_LINK_DOWN";
                        case Exchange::INetworkManager::INTERFACE_ACQUIRING_IP:
                            return "INTERFACE_ACQUIRING_IP";
                        case Exchange::INetworkManager::INTERFACE_REMOVED:
                            return "INTERFACE_REMOVED";
                        case Exchange::INetworkManager::INTERFACE_DISABLED:
                            return "INTERFACE_DISABLED";
                    }
                    return "";
                }

                string WiFiSignalQualityToString(Exchange::INetworkManager::WiFiSignalQuality quality)
                {
                    switch (quality)
                    {
                        case Exchange::INetworkManager::WiFiSignalQuality::WIFI_SIGNAL_DISCONNECTED:
                            return "Disconnected";
                        case Exchange::INetworkManager::WiFiSignalQuality::WIFI_SIGNAL_WEAK:
                            return "Weak";
                        case Exchange::INetworkManager::WiFiSignalQuality::WIFI_SIGNAL_FAIR:
                            return "Fair";
                        case Exchange::INetworkManager::WiFiSignalQuality::WIFI_SIGNAL_GOOD:
                            return "Good";
                        case Exchange::INetworkManager::WiFiSignalQuality::WIFI_SIGNAL_EXCELLENT:
                            return "Excellent";
                    }
                    return "";
                }

                string InternetStatusToString(const Exchange::INetworkManager::InternetStatus internetStatus)
                {
                    switch (internetStatus)
                    {
                        case Exchange::INetworkManager::InternetStatus::INTERNET_LIMITED:
                            return "LIMITED_INTERNET";
                        case Exchange::INetworkManager::InternetStatus::INTERNET_CAPTIVE_PORTAL:
                            return "CAPTIVE_PORTAL";
                        case Exchange::INetworkManager::InternetStatus::INTERNET_FULLY_CONNECTED:
                            return "FULLY_CONNECTED";
                        default:
                            return "NO_INTERNET";
                    }
                    return "";
                }

            public:
                explicit Notification(NetworkManager *parent)
                    : _parent(*parent)
                {
                    ASSERT(parent != nullptr);
                }
                virtual ~Notification() override
                {
                }

            public:
                void onInterfaceStateChange(const Exchange::INetworkManager::InterfaceState event, const string interface) override
                {
                    NMLOG_TRACE("%s", __FUNCTION__);
                    JsonObject params;
#ifdef ENABLE_LEGACY_NSM_SUPPORT
                    JsonObject legacyParams;
                    JsonObject onConnParams;
                    std::string json;
                    if(interface == "wlan0")
                        legacyParams["interface"] = "WIFI";
                    else if(interface == "eth0")
                        legacyParams["interface"] = "ETHERNET";
                    if(event == Exchange::INetworkManager::INTERFACE_ADDED)
                        legacyParams["enabled"] = true;
                    else if(event == Exchange::INetworkManager::INTERFACE_REMOVED)
                        legacyParams["enabled"] = false;
                    _parent.Notify("onInterfaceStatusChanged", legacyParams);
                    if(event == Exchange::INetworkManager::INTERFACE_LINK_UP)
                        onConnParams["status"] = "CONNECTED";
                    else if(event == Exchange::INetworkManager::INTERFACE_LINK_DOWN)
                        onConnParams["status"] = "DISCONNECTED";
                    onConnParams["interface"] = legacyParams["interface"];
                    onConnParams.ToString(json);
                    NMLOG_TRACE("onConnectionStatusChanged onConnParams=%s", json.c_str() );
                    _parent.Notify("onConnectionStatusChanged", onConnParams);
#endif
                    params["interface"] = interface;
                    params["state"] = InterfaceStateToString(event);
                    _parent.Notify("onInterfaceStateChange", params);
                }

                void onIPAddressChange(const string interface, const bool isAcquired, const bool isIPv6, const string ipAddress) override
                {
                    NMLOG_TRACE("%s", __FUNCTION__);
                    JsonObject params;
#ifdef ENABLE_LEGACY_NSM_SUPPORT
                    JsonObject legacyParams;
                    if(isIPv6)
                    {
                        legacyParams["ip6Address"] = ipAddress;
                        legacyParams["ip4Address"] = "";
                    }
                    else
                    {
                        legacyParams["ip4Address"] = ipAddress;
                        legacyParams["ip6Address"] = "";
                    }
                    if(interface == "wlan0")
                        legacyParams["interface"] = "WIFI";
                    else if(interface == "eth0")
                        legacyParams["interface"] = "ETHERNET";
                    legacyParams["status"] = string (isAcquired ? "ACQUIRED" : "LOST");
                    _parent.Notify("onIPAddressStatusChanged", legacyParams);
#endif
                    params["status"] = string (isAcquired ? "ACQUIRED" : "LOST");
                    params["interface"] = interface;
                    params["ipAddress"] = ipAddress;
                    params["isIPv6"] = isIPv6;
                    _parent.Notify("onIPAddressChange", params);
                }

                void onActiveInterfaceChange(const string prevActiveInterface, const string currentActiveinterface) override
                {
                    NMLOG_TRACE("%s", __FUNCTION__);
                    JsonObject params;
#ifdef ENABLE_LEGACY_NSM_SUPPORT
                    JsonObject legacyParams;
                    string oldInterface;
                    string newInterface;
                    if(prevActiveInterface == "wlan0")
                        oldInterface = "WIFI";
                    else if(prevActiveInterface == "eth0")
                        oldInterface = "ETHERNET";
                    if(currentActiveinterface == "wlan0")
                        newInterface = "WIFI";
                    else if(currentActiveinterface == "eth0")
                        newInterface = "ETHERNET";
                    legacyParams["oldInterfaceName"] = oldInterface;
                    legacyParams["newInterfaceName"] = newInterface;
                    _parent.Notify("onDefaultInterfaceChanged", legacyParams);
#endif
                    params["oldInterfaceName"] = prevActiveInterface;
                    params["newInterfaceName"] = currentActiveinterface;
                    _parent.Notify("onActiveInterfaceChange", params);

                }

                void onInternetStatusChange(const Exchange::INetworkManager::InternetStatus oldState, const Exchange::INetworkManager::InternetStatus newstate) override
                {
                    NMLOG_TRACE("%s", __FUNCTION__);
                    JsonObject params;

                    params["state"] = static_cast <int> (newstate);;
                    params["status"] = InternetStatusToString(newstate);
                    params["prevState"] = static_cast <int> (oldState);
                    params["prevStatus"] = InternetStatusToString(oldState);

                    _parent.Notify("onInternetStatusChange", params);

                    if (Exchange::INetworkManager::InternetStatus::INTERNET_FULLY_CONNECTED == newstate)
                    {
                        NMLOG_INFO("Notify Thunder ISubsystem internet");
                        _parent.PublishToThunderAboutInternet();
                    }
                }

                // WiFi Notifications that other processes can subscribe to
                void onAvailableSSIDs(const string jsonOfWiFiScanResults) override
                {
                    NMLOG_TRACE("%s", __FUNCTION__);
                    JsonArray scanResults;
                    JsonObject result;
                    scanResults.FromString(jsonOfWiFiScanResults);
                    result["ssids"] = scanResults;
                    _parent.Notify("onAvailableSSIDs", result);

                }

                void onWiFiStateChange(const Exchange::INetworkManager::WiFiState state) override
                {
                    NMLOG_TRACE("%s", __FUNCTION__);
                    JsonObject result;
#ifdef ENABLE_LEGACY_NSM_SUPPORT
                    JsonObject legacyResult;
                    legacyResult["state"] = static_cast <int> (state);
                    legacyResult["isLNF"] = false;
                    _parent.Notify("onWIFIStateChanged", legacyResult);
#endif
                    result["state"] = static_cast <int> (state);
                    _parent.Notify("onWiFiStateChange", result);
                }

                void onWiFiSignalStrengthChange(const string ssid, const string signalLevel, const Exchange::INetworkManager::WiFiSignalQuality signalQuality) override
                {
                    NMLOG_TRACE("%s", __FUNCTION__);
                    JsonObject result;
#ifdef ENABLE_LEGACY_NSM_SUPPORT
                    JsonObject legacyResult;
                    legacyResult["signalStrength"] = WiFiSignalQualityToString(signalQuality);
                    legacyResult["strength"] = signalLevel;
                    _parent.Notify("onWifiSignalThresholdChanged", legacyResult);
#endif
                    result["ssid"] = ssid;
                    result["signalQuality"] = WiFiSignalQualityToString(signalQuality);
                    result["signalLevel"] = signalLevel;
                    _parent.Notify("onWiFiSignalStrengthChange", result);
                }

                // The activated/deactived methods are part of the RPC::IRemoteConnection::INotification
                // interface. These are triggered when Thunder detects a connection/disconnection over the
                // COM-RPC link.
                void Activated(RPC::IRemoteConnection * /* connection */) override
                {
                }

                void Deactivated(RPC::IRemoteConnection *connection) override
                {
                    // Something's caused the remote connection to be lost - this could be a crash
                    // on the remote side so deactivate ourselves
                    _parent.Deactivated(connection);
                }

                // Build QueryInterface implementation, specifying all possible interfaces we implement
                BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(Exchange::INetworkManager::INotification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                END_INTERFACE_MAP

            private:
                NetworkManager &_parent;
            };

        public:
            NetworkManager();
            ~NetworkManager() override;

            // Implement the basic IPlugin interface that all plugins must implement
            const string Initialize(PluginHost::IShell *service) override;
            void Deinitialize(PluginHost::IShell *service) override;
            string Information() const override;

            // Do not allow copy/move constructors
            NetworkManager(const NetworkManager &) = delete;
            NetworkManager &operator=(const NetworkManager &) = delete;

            // Build QueryInterface implementation, specifying all possible interfaces we implement
            // This is necessary so that consumers can discover which plugin implements what interface
            BEGIN_INTERFACE_MAP(NetworkManager)

            // Which interfaces do we implement?
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_ENTRY(PluginHost::ISubSystem::IInternet)

            // We need to tell Thunder that this plugin provides the INetworkManager interface, but
            // since it's not actually implemented here we tell Thunder where it can
            // find the real implementation
            // This allows other components to call QueryInterface<INetworkManager>() and
            // receive the actual implementation (which could be in-process or out-of-process)
            INTERFACE_AGGREGATE(Exchange::INetworkManager, _NetworkManager)
            END_INTERFACE_MAP

            /*
            * ------------------------------------------------------------------------------------------------------------
            * ISubSystem::IInternet methods
            * ------------------------------------------------------------------------------------------------------------
            */
            string PublicIPAddress() const override
            {
                return m_publicIPAddress;
            }
            network_type NetworkType() const override
            {
                return (m_publicIPAddress.empty() == true ? PluginHost::ISubSystem::IInternet::UNKNOWN : (m_publicIPAddressType == "IPV6" ? PluginHost::ISubSystem::IInternet::IPV6 : PluginHost::ISubSystem::IInternet::IPV4));
            }
            void PublishToThunderAboutInternet();

        private:
            // Notification/event handlers
            // Clean up when we're told to deactivate
            void Deactivated(RPC::IRemoteConnection *connection);

            // JSON-RPC setup
            void RegisterAllMethods();
            void UnregisterAllMethods();
#ifdef ENABLE_LEGACY_NSM_SUPPORT
            void RegisterLegacyMethods();
            void UnregisterLegacyMethods();
#endif

            // JSON-RPC methods (take JSON in, spit JSON back out)
            uint32_t SetLogLevel (const JsonObject& parameters, JsonObject& response);
            uint32_t GetAvailableInterfaces (const JsonObject& parameters, JsonObject& response);
            uint32_t GetPrimaryInterface (const JsonObject& parameters, JsonObject& response);
            uint32_t SetPrimaryInterface (const JsonObject& parameters, JsonObject& response);
            uint32_t EnableInterface (const JsonObject& parameters, JsonObject& response);
            uint32_t DisableInterface (const JsonObject& parameters, JsonObject& response);
            uint32_t GetIPSettings(const JsonObject& parameters, JsonObject& response);
            uint32_t SetIPSettings(const JsonObject& parameters, JsonObject& response);
            uint32_t GetStunEndpoint(const JsonObject& parameters, JsonObject& response);
            uint32_t SetStunEndpoint(const JsonObject& parameters, JsonObject& response);
            uint32_t GetConnectivityTestEndpoints(const JsonObject& parameters, JsonObject& response);
            uint32_t SetConnectivityTestEndpoints(const JsonObject& parameters, JsonObject& response);
            uint32_t IsConnectedToInternet(const JsonObject& parameters, JsonObject& response);
            uint32_t GetCaptivePortalURI(const JsonObject& parameters, JsonObject& response);
            uint32_t StartConnectivityMonitoring(const JsonObject& parameters, JsonObject& response);
            uint32_t StopConnectivityMonitoring(const JsonObject& parameters, JsonObject& response);
            uint32_t GetPublicIP(const JsonObject& parameters, JsonObject& response);
            uint32_t Ping(const JsonObject& parameters, JsonObject& response);
            uint32_t Trace(const JsonObject& parameters, JsonObject& response);
            uint32_t StartWiFiScan(const JsonObject& parameters, JsonObject& response);
            uint32_t StopWiFiScan(const JsonObject& parameters, JsonObject& response);
            uint32_t GetKnownSSIDs(const JsonObject& parameters, JsonObject& response);
            uint32_t AddToKnownSSIDs(const JsonObject& parameters, JsonObject& response);
            uint32_t RemoveKnownSSID(const JsonObject& parameters, JsonObject& response);
            uint32_t WiFiConnect(const JsonObject& parameters, JsonObject& response);
            uint32_t WiFiDisconnect(const JsonObject& parameters, JsonObject& response);
            uint32_t GetConnectedSSID(const JsonObject& parameters, JsonObject& response);
            uint32_t StartWPS(const JsonObject& parameters, JsonObject& response);
            uint32_t StopWPS(const JsonObject& parameters, JsonObject& response);
            uint32_t GetWifiState(const JsonObject& parameters, JsonObject& response);
            uint32_t GetWiFiSignalStrength(const JsonObject& parameters, JsonObject& response);
            uint32_t GetSupportedSecurityModes(const JsonObject& parameters, JsonObject& response);
#ifdef ENABLE_LEGACY_NSM_SUPPORT
            uint32_t getInterfaces (const JsonObject& parameters, JsonObject& response);
            uint32_t isInterfaceEnabled (const JsonObject& parameters, JsonObject& response);
            uint32_t setInterfaceEnabled (const JsonObject& parameters, JsonObject& response);
            uint32_t getPublicIP (const JsonObject& parameters, JsonObject& response);
            uint32_t getDefaultInterface(const JsonObject& parameters, JsonObject& response);
            uint32_t setDefaultInterface(const JsonObject& parameters, JsonObject& response);
            uint32_t setIPSettings(const JsonObject& parameters, JsonObject& response);
            uint32_t getIPSettings(const JsonObject& parameters, JsonObject& response);
            uint32_t getInternetConnectionState(const JsonObject& parameters, JsonObject& response);
            uint32_t ping(const JsonObject& parameters, JsonObject& response);
            uint32_t isConnectedToInternet(const JsonObject& parameters, JsonObject& response);
            uint32_t setConnectivityTestEndpoints(const JsonObject& parameters, JsonObject& response);
            uint32_t startConnectivityMonitoring(const JsonObject& parameters, JsonObject& response);
            uint32_t getCaptivePortalURI(const JsonObject& parameters, JsonObject& response);
            uint32_t stopConnectivityMonitoring(const JsonObject& parameters, JsonObject& response);
            uint32_t getPairedSSID(const JsonObject& parameters, JsonObject& response);
            uint32_t getPairedSSIDInfo(const JsonObject& parameters, JsonObject& response);
            uint32_t initiateWPSPairing(const JsonObject& parameters, JsonObject& response);
            uint32_t isPaired(const JsonObject& parameters, JsonObject& response);
            uint32_t saveSSID(const JsonObject& parameters, JsonObject& response);
            uint32_t cancelWPSPairing(const JsonObject& parameters, JsonObject& response);
            uint32_t clearSSID(const JsonObject& parameters, JsonObject& response);
            uint32_t connect(const JsonObject& parameters, JsonObject& response);
            uint32_t disconnect(const JsonObject& parameters, JsonObject& response);
            uint32_t getConnectedSSID(const JsonObject& parameters, JsonObject& response);
            uint32_t getSupportedSecurityModes(const JsonObject& parameters, JsonObject& response);
            uint32_t startScan(const JsonObject& parameters, JsonObject& response);
            uint32_t stopScan(const JsonObject& parameters, JsonObject& response);
            uint32_t getCurrentState(const JsonObject& parameters, JsonObject& response);
#endif
        private:
            uint32_t _connectionId;
            PluginHost::IShell *_service;
            Exchange::INetworkManager *_NetworkManager;
            Core::Sink<Notification> _notification;
            string m_publicIPAddress;
            string m_defaultInterface;
            string m_publicIPAddressType;
        };
    }
}
