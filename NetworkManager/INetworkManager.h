/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2023 RDK Management
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

// @stubgen:include <com/IIteratorType.h>

namespace WPEFramework
{
    namespace Exchange
    {
        enum myNetwork_ids {
            ID_NETWORKMANAGER                               = 0x800004E0,
            ID_NETWORKMANAGER_NOTIFICATION                  = ID_NETWORKMANAGER + 1,
            ID_NETWORKMANAGER_INTERFACE_DETAILS_ITERATOR    = ID_NETWORKMANAGER + 2,
            ID_NETWORKMANAGER_WIFI_SECURITY_MODE_ITERATOR   = ID_NETWORKMANAGER + 3
        };

        /* @json */
        struct EXTERNAL INetworkManager: virtual public Core::IUnknown
        {
            // All interfaces require a unique ID, defined in Ids.h
            enum { ID = ID_NETWORKMANAGER };

            // Define the RPC methods
            enum InterfaceType : uint8_t
            {
                INTERFACE_TYPE_ETHERNET,
                INTERFACE_TYPE_WIFI,
                INTERFACE_TYPE_P2P,
            };

            struct EXTERNAL InterfaceDetails {
                string m_type;
                string m_name;
                string m_mac;
                bool   m_isEnabled;
                bool   m_isConnected;
            };

            enum IPAddressType : uint8_t
            {
                IP_ADDRESS_V4,
                IP_ADDRESS_V6,
                IP_ADDRESS_BOTH
            };

            struct EXTERNAL IPAddressInfo {
                string m_ipAddrType;
                bool m_autoConfig;
                string m_dhcpServer;
                string m_v6LinkLocal;
                string m_ipAddress;
                uint32_t m_prefix;
                string m_gateway;
                string m_primaryDns;
                string m_secondaryDns;
            };

            // Define the RPC methods
            enum InternetStatus : uint8_t
            {
                INTERNET_NOT_AVAILABLE,
                INTERNET_LIMITED,
                INTERNET_CAPTIVE_PORTAL,
                INTERNET_FULLY_CONNECTED,
                INTERNET_UNKNOWN,
            };

            enum WiFiFrequency : uint8_t
            {
                WIFI_FREQUENCY_WHATEVER,
                WIFI_FREQUENCY_2_4_GHZ,
                WIFI_FREQUENCY_5_GHZ,
                WIFI_FREQUENCY_6_GHZ
            };

            enum WiFiWPS : uint8_t
            {
                WIFI_WPS_PBC,
                WIFI_WPS_PIN,
                WIFI_WPS_SERIALIZED_PIN
            };

            enum WIFISecurityMode : uint8_t
            {
                WIFI_SECURITY_NONE,
                WIFI_SECURITY_WEP_64,
                WIFI_SECURITY_WEP_128,
                WIFI_SECURITY_WPA_PSK_TKIP,
                WIFI_SECURITY_WPA_PSK_AES,
                WIFI_SECURITY_WPA2_PSK_TKIP,
                WIFI_SECURITY_WPA2_PSK_AES,
                WIFI_SECURITY_WPA_ENTERPRISE_TKIP,
                WIFI_SECURITY_WPA_ENTERPRISE_AES,
                WIFI_SECURITY_WPA2_ENTERPRISE_TKIP,
                WIFI_SECURITY_WPA2_ENTERPRISE_AES,
                WIFI_SECURITY_WPA_WPA2_PSK,
                WIFI_SECURITY_WPA_WPA2_ENTERPRISE,
                WIFI_SECURITY_WPA3_PSK_AES,
                WIFI_SECURITY_WPA3_SAE
            };

            struct EXTERNAL WiFiScanResults {
                    string           m_ssid;
                    WIFISecurityMode m_securityMode;
                    string           m_signalStrength;
                    WiFiFrequency    m_frequency;
            };

            struct EXTERNAL WiFiConnectTo {
                    string           m_ssid;
                    string           m_passphrase;
                    WIFISecurityMode m_securityMode;
                    string           m_identity;
                    string           m_caCert;
                    string           m_clientCert;
                    string           m_privateKey;
                    string           m_privateKeyPasswd;
            };

            struct EXTERNAL WiFiSSIDInfo {
                    string             m_ssid;
                    string             m_bssid;
                    WIFISecurityMode   m_securityMode;
                    string             m_signalStrength;
                    WiFiFrequency      m_frequency;
                    string             m_rate;
                    string             m_noise;
            };

            struct EXTERNAL WIFISecurityModeInfo {
                WIFISecurityMode m_securityMode;
                string           m_securityModeText;
            };

            enum WiFiSignalQuality : uint8_t
            {
                WIFI_SIGNAL_DISCONNECTED,
                WIFI_SIGNAL_WEAK,
                WIFI_SIGNAL_FAIR,
                WIFI_SIGNAL_GOOD,
                WIFI_SIGNAL_EXCELLENT
            };

            enum NMLogging : uint8_t
            {
                LOG_LEVEL_FATAL,
                LOG_LEVEL_ERROR,
                LOG_LEVEL_WARNING,
                LOG_LEVEL_INFO,
                LOG_LEVEL_VERBOSE,
                LOG_LEVEL_TRACE
            };

           // The state of the interface 
            enum InterfaceState : uint8_t
            {
                INTERFACE_ADDED,
                INTERFACE_LINK_UP,
                INTERFACE_LINK_DOWN,
                INTERFACE_ACQUIRING_IP,
                INTERFACE_REMOVED,
                INTERFACE_DISABLED
            };

            enum WiFiState : uint8_t
            {
                WIFI_STATE_UNINSTALLED,
                WIFI_STATE_DISABLED,
                WIFI_STATE_DISCONNECTED,
                WIFI_STATE_PAIRING,
                WIFI_STATE_CONNECTING,
                WIFI_STATE_CONNECTED,
                WIFI_STATE_SSID_NOT_FOUND,
                WIFI_STATE_SSID_CHANGED,
                WIFI_STATE_CONNECTION_LOST,
                WIFI_STATE_CONNECTION_FAILED,
                WIFI_STATE_CONNECTION_INTERRUPTED,
                WIFI_STATE_INVALID_CREDENTIALS,
                WIFI_STATE_AUTHENTICATION_FAILED,
                WIFI_STATE_ERROR
            };

            using IInterfaceDetailsIterator = RPC::IIteratorType<InterfaceDetails,     ID_NETWORKMANAGER_INTERFACE_DETAILS_ITERATOR>;
            using ISecurityModeIterator     = RPC::IIteratorType<WIFISecurityModeInfo, ID_NETWORKMANAGER_WIFI_SECURITY_MODE_ITERATOR>;
            using IStringIterator           = RPC::IIteratorType<string,               RPC::ID_STRINGITERATOR>;

            /* @brief Get all the Available Interfaces */
            virtual uint32_t GetAvailableInterfaces (IInterfaceDetailsIterator*& interfaces/* @out */) = 0;

            /* @brief Get the Primary Interface used for external world communication */
            virtual uint32_t GetPrimaryInterface (string& interface /* @out */) = 0;
            /* @brief Set the Primary Interface used for external world communication */
            virtual uint32_t SetPrimaryInterface (const string& interface/* @in */) = 0;

            /* @brief Enable/Disable the given interface */
            virtual uint32_t SetInterfaceState (const string& interface /* @in */, const bool& enabled /* @in */) = 0;
            /* @brief Get the state of given interface */
            virtual uint32_t GetInterfaceState (const string& interface /* @in */, bool& isEnabled /* @out */) = 0;

            /* @brief Get IP Address Of the Interface */
            virtual uint32_t GetIPSettings(const string& interface /* @in */, const string &ipversion /* @in */, IPAddressInfo& result /* @out */) = 0;
            /* @brief Set IP Address Of the Interface */
            virtual uint32_t SetIPSettings(const string& interface /* @in */, const string &ipversion /* @in */, const IPAddressInfo& address /* @in */) = 0;

            /* @brief Get STUN Endpoint to be used for identifying Public IP */
            virtual uint32_t GetStunEndpoint (string &endPoint /* @out */, uint32_t &port /* @out */, uint32_t &bindTimeout /* @out */, uint32_t &cacheTimeout /* @out */) const = 0;
            /* @brief Set STUN Endpoint to be used to identify Public IP */
            virtual uint32_t SetStunEndpoint (string const endPoint /* @in */, const uint32_t port /* @in */, const uint32_t bindTimeout /* @in */, const uint32_t cacheTimeout /* @in */)  = 0;

            /* @brief Get ConnectivityTest Endpoints */
            virtual uint32_t GetConnectivityTestEndpoints(IStringIterator*& endPoints/* @out */) const = 0;
            /* @brief Set ConnectivityTest Endpoints */
            virtual uint32_t SetConnectivityTestEndpoints(IStringIterator* const endPoints /* @in */) = 0;

            /* @brief Get Internet Connectivty Status */ 
            virtual uint32_t IsConnectedToInternet(const string &ipversion /* @in */, InternetStatus &result /* @out */) = 0;
            /* @brief Get Authentication URL if the device is behind Captive Portal */ 
            virtual uint32_t GetCaptivePortalURI(string &endPoints/* @out */) const = 0;

            /* @brief Start The Internet Connectivity Monitoring */ 
            virtual uint32_t StartConnectivityMonitoring(const uint32_t interval /* @in */) = 0;
            /* @brief Stop The Internet Connectivity Monitoring */ 
            virtual uint32_t StopConnectivityMonitoring(void) const = 0;

            /* @brief Get the Public IP used for external world communication */
            virtual uint32_t GetPublicIP (const string &ipversion /* @in */,  string& ipAddress /* @out */) = 0;

            /* @brief Request for ping and get the response in as event. The GUID used in the request will be returned in the event. */
            virtual uint32_t Ping (const string ipversion /* @in */,  const string endpoint /* @in */, const uint32_t noOfRequest /* @in */, const uint16_t timeOutInSeconds /* @in */, const string guid /* @in */, string& response /* @out */) = 0;

            /* @brief Request for trace get the response in as event. The GUID used in the request will be returned in the event. */
            virtual uint32_t Trace (const string ipversion /* @in */,  const string endpoint /* @in */, const uint32_t noOfRequest /* @in */, const string guid /* @in */, string& response /* @out */) = 0;


            // WiFi Specific Methods
            /* @brief Initiate a WIFI Scan; This is Async method and returns the scan results as Event */
            virtual uint32_t StartWiFiScan(const WiFiFrequency frequency /* @in */) = 0;
            virtual uint32_t StopWiFiScan(void) = 0;

            virtual uint32_t GetKnownSSIDs(IStringIterator*& ssids /* @out */) = 0;
            virtual uint32_t AddToKnownSSIDs(const WiFiConnectTo& ssid /* @in */) = 0;
            virtual uint32_t RemoveKnownSSID(const string& ssid /* @in */) = 0;

            virtual uint32_t WiFiConnect(const WiFiConnectTo& ssid /* @in */) = 0;
            virtual uint32_t WiFiDisconnect(void) = 0;
            virtual uint32_t GetConnectedSSID(WiFiSSIDInfo&  ssidInfo /* @out */) = 0;

            virtual uint32_t StartWPS(const WiFiWPS& method /* @in */, const string& wps_pin /* @in */) = 0;
            virtual uint32_t StopWPS(void) = 0;
            virtual uint32_t GetWifiState(WiFiState &state /* @out */) = 0;
            virtual uint32_t GetWiFiSignalStrength(string& ssid /* @out */, string& signalStrength /* @out */, WiFiSignalQuality& quality /* @out */) = 0;
            virtual uint32_t GetSupportedSecurityModes(ISecurityModeIterator*& securityModes /* @out */) const = 0;

            /* @brief Set the network manager plugin log level */
            virtual uint32_t SetLogLevel(const NMLogging& logLevel /* @in */) = 0;

            /* @brief configure network manager plugin */
            virtual uint32_t Configure(const string& configLine /* @in */, NMLogging& logLevel /* @out */) = 0;

            /* @event */
            struct EXTERNAL INotification : virtual public Core::IUnknown
            {
                enum { ID = ID_NETWORKMANAGER_NOTIFICATION };

                // Network Notifications that other processes can subscribe to
                virtual void onInterfaceStateChange(const InterfaceState event /* @in */, const string interface /* @in */) = 0;
                virtual void onActiveInterfaceChange(const string prevActiveInterface /* @in */, const string currentActiveinterface /* @in */) = 0;
                virtual void onIPAddressChange(const string interface /* @in */, const bool isAcquired /* @in */, const bool isIPv6 /* @in */, const string ipAddress /* @in */) = 0;
                virtual void onInternetStatusChange(const InternetStatus oldState /* @in */, const InternetStatus newstate /* @in */) = 0;

                // WiFi Notifications that other processes can subscribe to
                virtual void onAvailableSSIDs(const string jsonOfWiFiScanResults /* @in */) = 0;
                virtual void onWiFiStateChange(const WiFiState state /* @in */) = 0;
                virtual void onWiFiSignalStrengthChange(const string ssid /* @in */, const string signalLevel /* @in */, const WiFiSignalQuality signalQuality /* @in */) = 0;
            };

            // Allow other processes to register/unregister from our notifications
            virtual uint32_t Register(INetworkManager::INotification* notification) = 0;
            virtual uint32_t Unregister(INetworkManager::INotification* notification) = 0;
        };
    }
}
