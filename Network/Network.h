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

#include <string>
#include <atomic>

#include "Module.h"
#include "NetUtils.h"
#include "libIARM.h"


// Define this to use netlink calls (where there may be an alternative method but netlink could provide
// the information or perform the action required)
//#define USE_NETLINK
#define MAX_IP_ADDRESS_LEN 46
#define NETSRVMGR_INTERFACES_MAX 16
#define MAX_ENDPOINTS 5
#define MAX_ENDPOINT_SIZE 512
#define MAX_URI_LEN 512
#define MAX_HOST_NAME_LEN 128

typedef enum _NetworkManager_EventId_t {
    IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_ENABLED=50,
    IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_CONTROL_PERSISTENCE,
    IARM_BUS_NETWORK_MANAGER_EVENT_WIFI_INTERFACE_STATE,
    IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_ENABLED_STATUS,
    IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_CONNECTION_STATUS,
    IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_IPADDRESS,
    IARM_BUS_NETWORK_MANAGER_EVENT_DEFAULT_INTERFACE,
    IARM_BUS_NETWORK_MANAGER_EVENT_INTERNET_CONNECTION_CHANGED,
    IARM_BUS_NETWORK_MANAGER_MAX,
} IARM_Bus_NetworkManager_EventId_t;


typedef struct {
    char name[16];
    char mac[20];
    unsigned int flags;
} NetSrvMgr_Interface_t;

typedef struct {
    unsigned char         size;
    NetSrvMgr_Interface_t interfaces[NETSRVMGR_INTERFACES_MAX];
} IARM_BUS_NetSrvMgr_InterfaceList_t;

typedef enum _NetworkManager_GetIPSettings_ErrorCode_t
{
  NETWORK_IPADDRESS_ACQUIRED,
  NETWORK_IPADDRESS_NOTFOUND,
  NETWORK_NO_ROUTE_INTERFACE,
  NETWORK_NO_DEFAULT_ROUTE,
  NETWORK_DNS_NOT_CONFIGURED,
  NETWORK_INVALID_IPADDRESS,
} NetworkManager_GetIPSettings_ErrorCode_t;

typedef struct {
    char interface[16];
    char ipversion[16];
    bool autoconfig;
    char ipaddress[MAX_IP_ADDRESS_LEN];
    char netmask[MAX_IP_ADDRESS_LEN];
    char gateway[MAX_IP_ADDRESS_LEN];
    char dhcpserver[MAX_IP_ADDRESS_LEN];
    char primarydns[MAX_IP_ADDRESS_LEN];
    char secondarydns[MAX_IP_ADDRESS_LEN];
    bool isSupported;
    NetworkManager_GetIPSettings_ErrorCode_t errCode;
} IARM_BUS_NetSrvMgr_Iface_Settings_t;

typedef struct
{
    unsigned char size;
    char          endpoints[MAX_ENDPOINTS][MAX_ENDPOINT_SIZE];
} IARM_BUS_NetSrvMgr_Iface_TestEndpoints_t;

typedef enum _InternetConnectionState_t {
    NO_INTERNET,
    LIMITED_INTERNET,
    CAPTIVE_PORTAL,
    FULLY_CONNECTED
}InternetConnectionState_t;

typedef struct
{
    int connectivityState;
    int monitorInterval;
    bool monitorConnectivity;
    char captivePortalURI[MAX_URI_LEN];
} IARM_BUS_NetSrvMgr_Iface_InternetConnectivityStatus_t;

typedef struct {
    char interface[16];
    char gateway[MAX_IP_ADDRESS_LEN];
} IARM_BUS_NetSrvMgr_DefaultRoute_t;

typedef struct {
    char interface[16];
    bool status;
} IARM_BUS_NetSrvMgr_Iface_EventInterfaceStatus_t;

typedef IARM_BUS_NetSrvMgr_Iface_EventInterfaceStatus_t IARM_BUS_NetSrvMgr_Iface_EventInterfaceEnabledStatus_t;
typedef IARM_BUS_NetSrvMgr_Iface_EventInterfaceStatus_t IARM_BUS_NetSrvMgr_Iface_EventInterfaceConnectionStatus_t;

typedef struct {
    char interface[16];
    char ip_address[MAX_IP_ADDRESS_LEN];
    bool is_ipv6;
    bool acquired;
} IARM_BUS_NetSrvMgr_Iface_EventInterfaceIPAddress_t;

typedef struct {
    char oldInterface[16];
    char newInterface[16];
} IARM_BUS_NetSrvMgr_Iface_EventDefaultInterface_t;

typedef struct
{
    char server[MAX_HOST_NAME_LEN];
    uint16_t port;
    bool ipv6;
    char interface[16];
    uint16_t bind_timeout;
    uint16_t cache_timeout;
    bool sync;
    char public_ip[MAX_IP_ADDRESS_LEN];
} IARM_BUS_NetSrvMgr_Iface_StunRequest_t;

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
        class Network : public PluginHost::IPlugin
            , public PluginHost::JSONRPC
            , public PluginHost::ISubSystem::IInternet
        {
        private:

            // We do not allow this plugin to be copied !!
            Network(const Network&) = delete;
            Network& operator=(const Network&) = delete;

            //Private variables
            std::atomic_bool m_isPluginInited{false};
            std::thread m_registrationThread;

            //Begin methods
            uint32_t getQuirks(const JsonObject& parameters, JsonObject& response);

            // Network_API_Version_1
            uint32_t getInterfaces(const JsonObject& parameters, JsonObject& response);
            uint32_t isInterfaceEnabled(const JsonObject& parameters, JsonObject& response);
            uint32_t setInterfaceEnabled(const JsonObject& parameters, JsonObject& response);
            uint32_t getDefaultInterface(const JsonObject& parameters, JsonObject& response);
            uint32_t setDefaultInterface(const JsonObject& parameters, JsonObject& response);
            uint32_t getStbIp(const JsonObject& parameters, JsonObject& response);
            uint32_t trace(const JsonObject& parameters, JsonObject& response);
            uint32_t traceNamedEndpoint(const JsonObject& parameters, JsonObject& response);
            uint32_t getNamedEndpoints(const JsonObject& parameters, JsonObject& response);
            uint32_t ping(const JsonObject& parameters, JsonObject& response);
            uint32_t pingNamedEndpoint(const JsonObject& parameters, JsonObject& response);
            uint32_t setIPSettings(const JsonObject& parameters, JsonObject& response);
            uint32_t setIPSettings2(const JsonObject& parameters, JsonObject& response);
            uint32_t getIPSettings(const JsonObject& parameters, JsonObject& response);
            uint32_t getIPSettings2(const JsonObject& parameters, JsonObject& response);
            uint32_t getSTBIPFamily(const JsonObject& parameters, JsonObject& response);
            uint32_t isConnectedToInternet(const JsonObject& parameters, JsonObject& response);
            uint32_t setConnectivityTestEndpoints(const JsonObject& parameters, JsonObject& response);
            uint32_t getInternetConnectionState(const JsonObject& parameters, JsonObject& response);
            uint32_t startConnectivityMonitoring(const JsonObject& parameters, JsonObject& response);
            uint32_t getCaptivePortalURI(const JsonObject& parameters, JsonObject& response);
            uint32_t stopConnectivityMonitoring(const JsonObject& parameters, JsonObject& response);
            uint32_t getPublicIP(const JsonObject& parameters, JsonObject& response);
            uint32_t setStunEndPoint(const JsonObject& parameters, JsonObject& response);
            bool getIPIARMWrapper(IARM_BUS_NetSrvMgr_Iface_Settings_t& iarmData, const string interface, const string ipversion);

            void onInterfaceEnabledStatusChanged(std::string interface, bool enabled);
            void onInterfaceConnectionStatusChanged(std::string interface, bool connected);
            void onInternetStatusChange(InternetConnectionState_t InternetConnectionState);
            void onInterfaceIPAddressChanged(std::string interface, std::string ipv6Addr, std::string ipv4Addr, bool acquired);
            void onDefaultInterfaceChanged(std::string oldInterface, std::string newInterface);

            static void eventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

            // Netmask Validation
            bool isValidCIDRv4(std::string interface);
            // Internal methods
            bool _getDefaultInterface(std::string& interface, std::string& gateway);

            void retryIarmEventRegistration();
            void threadEventRegistration();

            bool _doTrace(std::string &endpoint, int packets, JsonObject& response);
            bool _doTraceNamedEndpoint(std::string &endpointName, int packets, JsonObject& response);

            JsonObject _doPing(const std::string& guid, const std::string& endPoint, int packets);
            JsonObject _doPingNamedEndpoint(const std::string& guid, const std::string& endpointName, int packets);
            bool getIPSettingsInternal(const JsonObject& parameters, JsonObject& response,int& errCode);
            uint32_t setIPSettingsInternal(const JsonObject& parameters, JsonObject& response);

        public:
            Network();
            virtual ~Network();

            //Build QueryInterface implementation, specifying all possible interfaces to be returned.
            BEGIN_INTERFACE_MAP(Network)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_ENTRY(PluginHost::ISubSystem::IInternet)
            END_INTERFACE_MAP
            
            /*
            * ------------------------------------------------------------------------------------------------------------
            * ISubSystem::INetwork methods
            * ------------------------------------------------------------------------------------------------------------
            */
            string PublicIPAddress() const override
            {
                return m_publicIPAddress;
            }
            network_type NetworkType() const override
            {
                return (m_publicIPAddress.empty() == true ? PluginHost::ISubSystem::IInternet::UNKNOWN : (m_ipversion == "IPV6" ? PluginHost::ISubSystem::IInternet::IPV6 : PluginHost::ISubSystem::IInternet::IPV4));
            }

            //IPlugin methods
            virtual const std::string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual std::string Information() const override;
            uint32_t getPublicIPInternal(const JsonObject& parameters, JsonObject& response);

        public:
            static Network *_instance;
            static Network *getInstance() {return _instance;}

        private:
            PluginHost::IShell* m_service;
            NetUtils m_netUtils;
            string m_stunEndPoint;
            string m_isHybridDevice;
            string m_defaultInterface;
            string m_gatewayInterface;
            string m_publicIPAddress;
            string m_ipversion;
            uint16_t m_stunPort;
            uint16_t m_stunBindTimeout;
            uint16_t m_stunCacheTimeout;
            bool m_stunSync;
            uint32_t m_apiVersionNumber;
            std::atomic<bool> m_useIpv4WifiCache;
            std::atomic<bool> m_useIpv6WifiCache;
            std::atomic<bool> m_useIpv4EthCache;
            std::atomic<bool> m_useIpv6EthCache;
            std::atomic<bool> m_useStbIPCache;
            string m_stbIpCache;
            std::atomic<bool> m_useDefInterfaceCache;
            string m_defInterfaceCache;
            string m_defIpversionCache;
            std::atomic<bool> m_useInterfacesCache;
            IARM_BUS_NetSrvMgr_InterfaceList_t m_interfacesCache;

            IARM_BUS_NetSrvMgr_Iface_Settings_t m_ipv4WifiCache;
            IARM_BUS_NetSrvMgr_Iface_Settings_t m_ipv6WifiCache;
            IARM_BUS_NetSrvMgr_Iface_Settings_t m_ipv4EthCache;
            IARM_BUS_NetSrvMgr_Iface_Settings_t m_ipv6EthCache;
        };
    } // namespace Plugin
} // namespace WPEFramework
