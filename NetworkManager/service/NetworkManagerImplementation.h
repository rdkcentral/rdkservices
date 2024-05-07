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
#include <iostream>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/rtnetlink.h>

using namespace std;

//#include <interfaces/INetworkManager.h>
#include "INetworkManager.h"
#include "NetworkConnectivity.h"
#include "StunClient.h"

#define LOG_ENTRY_FUNCTION() { printf( "Entering=%s\n", __FUNCTION__ ); }

namespace WPEFramework
{
    namespace Plugin
    {
        class NetworkManagerImplementation : public Exchange::INetworkManager
        {
        enum NetworkEvents {
                NETMGR_PING,
                NETMGR_TRACE,
            };

        public:
            NetworkManagerImplementation();
            ~NetworkManagerImplementation() override;

            // Do not allow copy/move constructors
            NetworkManagerImplementation(const NetworkManagerImplementation &) = delete;
            NetworkManagerImplementation &operator=(const NetworkManagerImplementation &) = delete;

            BEGIN_INTERFACE_MAP(NetworkManagerImplementation)
            INTERFACE_ENTRY(Exchange::INetworkManager)
            END_INTERFACE_MAP

            // Handle Notification registration/removal
            uint32_t Register(INetworkManager::INotification *notification) override;
            uint32_t Unregister(INetworkManager::INotification *notification) override;

        public:
            // Below Control APIs will work with RDK or GNome NW.
            /* @brief Get all the Available Interfaces */
            uint32_t GetAvailableInterfaces (IInterfaceDetailsIterator*& interfaces/* @out */) override;

            /* @brief Get the active Interface used for external world communication */
            uint32_t GetPrimaryInterface (string& interface /* @out */) override;
            /* @brief Set the active Interface used for external world communication */
            uint32_t SetPrimaryInterface (const string& interface/* @in */) override;

            uint32_t SetInterfaceEnabled (const string& interface/* @in */, const bool& isEnabled /* @in */) override;
            /* @brief Get IP Address Of the Interface */
            uint32_t GetIPSettings(const string& interface /* @in */, const string &ipversion /* @in */, IPAddressInfo& result /* @out */) override;
            /* @brief Set IP Address Of the Interface */
            uint32_t SetIPSettings(const string& interface /* @in */, const string &ipversion /* @in */, const IPAddressInfo& address /* @in */) override;

            // WiFi Specific Methods
            /* @brief Initiate a WIFI Scan; This is Async method and returns the scan results as Event */
            uint32_t StartWiFiScan(const WiFiFrequency frequency /* @in */);
            uint32_t StopWiFiScan(void) override;

            uint32_t GetKnownSSIDs(IStringIterator*& ssids /* @out */) override;
            uint32_t AddToKnownSSIDs(const WiFiConnectTo& ssid /* @in */) override;
            uint32_t RemoveKnownSSID(const string& ssid /* @in */) override;

            uint32_t WiFiConnect(const WiFiConnectTo& ssid /* @in */) override;
            uint32_t WiFiDisconnect(void) override;
            uint32_t GetConnectedSSID(WiFiSSIDInfo&  ssidInfo /* @out */) override;

            uint32_t StartWPS(const WiFiWPS& method /* @in */, const string& wps_pin /* @in */) override;
            uint32_t StopWPS(void) override;
            uint32_t GetWiFiSignalStrength(string& ssid /* @out */, string& signalStrength /* @out */, WiFiSignalQuality& quality /* @out */) override;

            uint32_t SetStunEndpoint (string const endPoint /* @in */, const uint32_t port /* @in */, const uint32_t bindTimeout /* @in */, const uint32_t cacheTimeout /* @in */) override;
            uint32_t GetStunEndpoint (string &endPoint /* @out */, uint32_t& port /* @out */, uint32_t& bindTimeout /* @out */, uint32_t& cacheTimeout /* @out */) const override;

            /* @brief Get ConnectivityTest Endpoints */
            uint32_t GetConnectivityTestEndpoints(IStringIterator*& endPoints/* @out */) const override;
            /* @brief Set ConnectivityTest Endpoints */
            uint32_t SetConnectivityTestEndpoints(IStringIterator* const endPoints /* @in */) override;

            /* @brief Get Internet Connectivty Status */ 
            uint32_t IsConnectedToInternet(const string &ipversion /* @in */, InternetStatus &result /* @out */) override;
            /* @brief Get Authentication URL if the device is behind Captive Portal */ 
            uint32_t GetCaptivePortalURI(string &endPoints/* @out */) const override;

            /* @brief Start The Internet Connectivity Monitoring */ 
            uint32_t StartConnectivityMonitoring(const uint32_t interval/* @in */) override;
            /* @brief Stop The Internet Connectivity Monitoring */ 
            uint32_t StopConnectivityMonitoring(void) const override;

            /* @brief Get the Public IP used for external world communication */
            uint32_t GetPublicIP (const string &ipversion /* @in */,  string& ipAddress /* @out */) override;

            /* @brief Request for ping and get the response in as event. The GUID used in the request will be returned in the event. */
            uint32_t Ping (const string ipversion /* @in */,  const string endpoint /* @in */, const uint32_t noOfRequest /* @in */, const uint16_t timeOutInSeconds /* @in */, const string guid /* @in */) override;

            /* @brief Request for trace get the response in as event. The GUID used in the request will be returned in the event. */
            uint32_t Trace (const string ipversion /* @in */,  const string endpoint /* @in */, const uint32_t noOfRequest /* @in */, const string guid /* @in */) override;

            uint32_t GetSupportedSecurityModes(ISecurityModeIterator*& securityModes /* @out */) const override;

            /* Events */
            void ReportInterfaceStateChangedEvent(INetworkManager::INotification::InterfaceState state, string interface);
            void ReportIPAddressChangedEvent(const string& interface, bool isAcquired, bool isIPv6, const string& ipAddress);
            void ReportActiveInterfaceChangedEvent(const string prevActiveInterface, const string currentActiveinterface);
            void ReportInternetStatusChangedEvent(const InternetStatus oldState, const InternetStatus newstate);
            void ReportAvailableSSIDsEvent(const string jsonOfWiFiScanResults);
            void ReportWiFiStateChangedEvent(const INetworkManager::INotification::WiFiState state);

        private:
            void platform_init();
            void Dispatch(NetworkEvents event, const string commandToExecute, const string guid);

        private:
            std::list<Exchange::INetworkManager::INotification *> _notificationCallbacks;
            Core::CriticalSection _notificationLock;
            string m_defaultInterface;
            string m_publicIP;
            stun::client stunClient;
            string m_stunEndPoint;
            uint16_t m_stunPort;
            uint16_t m_stunBindTimeout;
            uint16_t m_stunCacheTimeout;
            ConnectivityMonitor& connectivityMonitor = ConnectivityMonitor::getInstance();

        public:
            class EXTERNAL Job : public Core::IDispatch {
            protected:
                 Job(NetworkManagerImplementation *netMgr, NetworkEvents event, string &params, string &guid)
                    : _netMgr(netMgr)
                    , _event(event)
                    , _params(params)
                    , _guid(guid) {
                    if (_netMgr != nullptr) {
                        _netMgr->AddRef();
                    }
                }

           public:
                Job() = delete;
                Job(const Job&) = delete;
                Job& operator=(const Job&) = delete;
                ~Job() {
                    if (_netMgr != nullptr) {
                        _netMgr->Release();
                    }
                }

           public:
                static Core::ProxyType<Core::IDispatch> Create(NetworkManagerImplementation *netMgr, NetworkEvents event, string params, string guid) {
                    return (Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create(netMgr, event, params, guid)));
                }

                virtual void Dispatch() {
                    _netMgr->Dispatch(_event, _params, _guid);
                }

            private:
                NetworkManagerImplementation *_netMgr;
                const NetworkEvents _event;
                const string _params;
                const string _guid;
            };
        };
    }
}
