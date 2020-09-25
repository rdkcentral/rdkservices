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

#include "Network.h"
#include <net/if.h>

#define DEFAULT_PING_PACKETS 15

/* Netsrvmgr Based Macros & Structures */
#define IARM_BUS_NM_SRV_MGR_NAME "NET_SRV_MGR"
#define INTERFACE_SIZE 10
#define INTERFACE_LIST 50
#define MAX_IP_ADDRESS_LEN 46
#define MAX_IP_FAMILY_SIZE 10
#define IARM_BUS_NETSRVMGR_API_getActiveInterface "getActiveInterface"
#define IARM_BUS_NETSRVMGR_API_getNetworkInterfaces "getNetworkInterfaces"
#define IARM_BUS_NETSRVMGR_API_getInterfaceList "getInterfaceList"
#define IARM_BUS_NETSRVMGR_API_getDefaultInterface "getDefaultInterface"
#define IARM_BUS_NETSRVMGR_API_setDefaultInterface "setDefaultInterface"
#define IARM_BUS_NETSRVMGR_API_isInterfaceEnabled "isInterfaceEnabled"
#define IARM_BUS_NETSRVMGR_API_setInterfaceEnabled "setInterfaceEnabled"
#define IARM_BUS_NETSRVMGR_API_getSTBip "getSTBip"
#define IARM_BUS_NETSRVMGR_API_setIPSettings "setIPSettings"
#define IARM_BUS_NETSRVMGR_API_getIPSettings "getIPSettings"
#define IARM_BUS_NETSRVMGR_API_getSTBip_family "getSTBip_family"

typedef enum _NetworkManager_EventId_t {
    IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_ENABLED=50,
    IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_CONTROL_PERSISTENCE,
    IARM_BUS_NETWORK_MANAGER_EVENT_WIFI_INTERFACE_STATE,
    IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_ENABLED_STATUS,
    IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_CONNECTION_STATUS,
    IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_IPADDRESS,
    IARM_BUS_NETWORK_MANAGER_EVENT_DEFAULT_INTERFACE,
    IARM_BUS_NETWORK_MANAGER_MAX,
} IARM_Bus_NetworkManager_EventId_t;

typedef struct _IARM_BUS_NetSrvMgr_Iface_EventData_t {
    union {
        char activeIface[INTERFACE_SIZE];
        char allNetworkInterfaces[INTERFACE_LIST];
        char enableInterface[INTERFACE_SIZE];
        char activeIfaceIpaddr[MAX_IP_ADDRESS_LEN];
    };
    char interfaceCount;
    bool isInterfaceEnabled;
    bool persist;
    char ipfamily[MAX_IP_FAMILY_SIZE];
} IARM_BUS_NetSrvMgr_Iface_EventData_t;

typedef struct {
    char interface[16];
    char ipversion[16];
    bool autoconfig;
    char ipaddress[16];
    char netmask[16];
    char gateway[16];
    char primarydns[16];
    char secondarydns[16];
    bool isSupported;
} IARM_BUS_NetSrvMgr_Iface_Settings_t;

typedef struct {
    char name[16];
    char mac[20];
    unsigned int flags;
} NetSrvMgr_Interface_t;

typedef struct {
    unsigned char         size;
    NetSrvMgr_Interface_t interfaces[8];
} IARM_BUS_NetSrvMgr_InterfaceList_t;

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


namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(Network, 1, 0);
        Network* Network::_instance = nullptr;

        Network::Network() : PluginHost::JSONRPC()
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            Network::_instance = this;

            /* HardCode it for now; wait for Set API Version call to update this further */
            m_apiVersionNumber = 1;

            // Quirk
            Register("getQuirks", &Network::getQuirks, this);

            // Network_API_Version_1
            Register("getInterfaces", &Network::getInterfaces, this);
            Register("isInterfaceEnabled", &Network::isInterfaceEnabled, this);
            Register("setInterfaceEnabled", &Network::setInterfaceEnabled, this);
            Register("getDefaultInterface", &Network::getDefaultInterface, this);
            Register("setDefaultInterface", &Network::setDefaultInterface, this);

            Register("getStbIp", &Network::getStbIp, this);

            Register("setApiVersionNumber", &Network::setApiVersionNumberWrapper, this);
            Register("getApiVersionNumber", &Network::getApiVersionNumberWrapper, this);

            Register("trace", &Network::trace, this);
            Register("traceNamedEndpoint", &Network::traceNamedEndpoint, this);

            Register("getNamedEndpoints", &Network::getNamedEndpoints, this);

            Register("ping",              &Network::ping, this);
            Register("pingNamedEndpoint", &Network::pingNamedEndpoint, this);

            Register("setIPSettings", &Network::setIPSettings, this);
            Register("getIPSettings", &Network::getIPSettings, this);

            Register("getSTBIPFamily", &Network::getSTBIPFamily, this);

            m_netUtils.InitialiseNetUtils();
        }

        Network::~Network()
        {
            LOGWARN("Destructor of Network_%d", m_apiVersionNumber);

            Unregister("getQuirks");
            Unregister("getInterfaces");
            Unregister("isInterfaceEnabled");
            Unregister("setInterfaceEnabled");
            Unregister("getDefaultInterface");
            Unregister("setDefaultInterface");
            Unregister("getStbIp");
            Unregister("setApiVersionNumber");
            Unregister("getApiVersionNumber");
            Unregister("trace");
            Unregister("traceNamedEndpoint");
            Unregister("getNamedEndpoints");
            Unregister("ping");
            Unregister("pingNamedEndpoint");
            Unregister("setIPSettings");
            Unregister("getIPSettings");

            m_apiVersionNumber = 0;
            Network::_instance = NULL;
        }

        const string Network::Initialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();

            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_ENABLED_STATUS, eventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_CONNECTION_STATUS, eventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_IPADDRESS, eventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_DEFAULT_INTERFACE, eventHandler) );
            }

            return string();
        }

        void Network::Deinitialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_ENABLED_STATUS) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_CONNECTION_STATUS) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_IPADDRESS) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_DEFAULT_INTERFACE) );
            }
        }

        string Network::Information() const
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            // No additional info to report.
            return(string());
        }

        // Wrapper methods
        uint32_t Network::getQuirks(const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            JsonArray array;
            array.Add("RDK-20093");
            response["quirks"] = array;
            returnResponse(true);
        }

        uint32_t Network::setApiVersionNumberWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            if (parameters.HasLabel("version"))
            {
                getNumberParameter("version", m_apiVersionNumber);
                returnResponse(true);
            }
            returnResponse(false);
        }

        uint32_t Network::getApiVersionNumberWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            response["version"] = m_apiVersionNumber;
            returnResponse(true);
        }

        uint32_t Network::getInterfaces (const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);

            if (m_apiVersionNumber >= 1)
            {
                IARM_BUS_NetSrvMgr_InterfaceList_t list;
                if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getInterfaceList, (void*)&list, sizeof(list)))
                {
                    JsonArray networkInterfaces;

                    for (int i = 0; i < list.size; i++)
                    {
                        JsonObject interface;
                        std::string iface = m_netUtils.getInterfaceDescription(list.interfaces[i].name);
#ifdef NET_DEFINED_INTERFACES_ONLY
                        if (iface == "")
                            continue;					// Skip unrecognised interfaces...
#endif
                        interface["interface"] = iface;
                        interface["macAddress"] = string(list.interfaces[i].mac);
                        interface["enabled"] = ((list.interfaces[i].flags & IFF_UP) != 0);
                        interface["connected"] = ((list.interfaces[i].flags & IFF_RUNNING) != 0);

                        networkInterfaces.Add(interface);
                    }

                    response["interfaces"] = networkInterfaces;
                    returnResponse(true);
                }
                else
                {
                    LOGWARN ("Call to %s for %s failed\n", IARM_BUS_NM_SRV_MGR_NAME, __FUNCTION__);
                }
            }
            else
                LOGWARN ("This version of Network Software is not supporting this API..\n");

            returnResponse(false);
        }

        uint32_t Network::getDefaultInterface (const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            if (m_apiVersionNumber >= 1)
            {
                IARM_BUS_NetSrvMgr_Iface_EventData_t param;
                if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getActiveInterface, (void*)&param, sizeof(param)))
                {
                    LOGINFO("%s :: Interface = %s \n",__FUNCTION__,param.activeIface);
                    std::string interface = param.activeIface;
                    response["interface"] = interface;
                    returnResponse(true);
                }
                else
                {
                    LOGWARN ("Call to %s for %s failed\n", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getActiveInterface);
                }
            }
            else
            {
                LOGWARN ("This version of Network Software is not supporting this API..\n");
            }

            returnResponse(false);
        }

        uint32_t Network::setDefaultInterface (const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            if (m_apiVersionNumber >= 1)
            {
                if ((parameters.HasLabel("interface")) && (parameters.HasLabel("persist")))
                {
                    std::string interface = "";
                    bool persist = false;

                    getStringParameter("interface", interface);
                    getBoolParameter("persist", persist);

                    IARM_BUS_NetSrvMgr_Iface_EventData_t iarmData = { 0 };
                    strncpy(iarmData.enableInterface, interface.c_str(), INTERFACE_SIZE);
                    iarmData.persist = persist;

                    if (IARM_RESULT_SUCCESS == IARM_Bus_Call (IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setDefaultInterface, (void *)&iarmData, sizeof(iarmData)))
                    {
                        response["success"] = true;
                        returnResponse(true);
                    }
                    else
                    {
                        LOGWARN ("Call to %s for %s failed\n", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setDefaultInterface);
                    }
                }
            }
            else
            {
                LOGWARN ("This version of Network Software is not supporting this API..\n");
            }

            returnResponse(false);
        }

        uint32_t Network::getStbIp(const JsonObject &parameters, JsonObject &response)
        {
            IARM_Result_t ret = IARM_RESULT_SUCCESS;
            IARM_BUS_NetSrvMgr_Iface_EventData_t param;
            memset(&param, 0, sizeof(param));
            ret = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getSTBip, (void*)&param, sizeof(param));

            if (ret != IARM_RESULT_SUCCESS )
            {
                response["ip"] = "";
                returnResponse(false);
            }
            response["ip"] = std::string(param.activeIfaceIpaddr, MAX_IP_ADDRESS_LEN-1);
            returnResponse(true);
        }

        uint32_t Network::getSTBIPFamily(const JsonObject &parameters, JsonObject &response)
        {
            if (parameters.HasLabel("family"))
            {
                IARM_Result_t ret = IARM_RESULT_SUCCESS;
                IARM_BUS_NetSrvMgr_Iface_EventData_t param;
                memset(&param, 0, sizeof(param));
           
                std::string ipfamily("");
                getStringParameter("family", ipfamily);
                strncpy(param.ipfamily,ipfamily.c_str(),MAX_IP_FAMILY_SIZE);
                ret = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getSTBip_family, (void*)&param, sizeof(param));
                if (ret != IARM_RESULT_SUCCESS )
                {
                    LOGWARN ("Query to get IPaddress by Family Failed..\n");
                    response["ip"] = "";
                    returnResponse(false);
                }
            response["ip"] = std::string(param.activeIfaceIpaddr, MAX_IP_ADDRESS_LEN-1);
            returnResponse(true);
          }
          else
          {
                LOGWARN ("Required Family Attribute is not provided.\n");
          }
          returnResponse(false);
       }
        uint32_t Network::isInterfaceEnabled (const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            if (m_apiVersionNumber >= 1)
            {
                if (parameters.HasLabel("interface"))
                {
                    std::string interface = "";

                    getStringParameter("interface", interface);

                    IARM_BUS_NetSrvMgr_Iface_EventData_t param = {0};
                    strncpy(param.enableInterface, interface.c_str(), INTERFACE_SIZE);
                    if (IARM_RESULT_SUCCESS == IARM_Bus_Call (IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_isInterfaceEnabled, (void*)&param, sizeof(param)))
                    {
                        LOGINFO("%s :: Enabled = %d \n",__FUNCTION__,param.isInterfaceEnabled);
                        response["enabled"] = param.isInterfaceEnabled;
                        returnResponse(true);
                    }
                    else
                    {
                        LOGWARN ("Call to %s for %s failed\n", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_isInterfaceEnabled);
                    }
                }
            }
            else
            {
                LOGWARN ("This version of Network Software is not supporting this API..\n");
            }

            returnResponse(false);
        }

        uint32_t Network::setInterfaceEnabled (const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            if (m_apiVersionNumber >= 1)
            {
                if ((parameters.HasLabel("interface")) && (parameters.HasLabel("enabled")) && (parameters.HasLabel("persist")))
                {
                    std::string interface = "";
                    bool enabled = false;
                    bool persist = false;

                    getStringParameter("interface", interface);
                    getBoolParameter("enabled", enabled);
                    getBoolParameter("persist", persist);

                    IARM_BUS_NetSrvMgr_Iface_EventData_t iarmData = { 0 };
                    strncpy(iarmData.enableInterface, interface.c_str(), INTERFACE_SIZE);
                    iarmData.isInterfaceEnabled = enabled;
                    iarmData.persist = persist;

                    if (IARM_RESULT_SUCCESS == IARM_Bus_Call (IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setInterfaceEnabled, (void *)&iarmData, sizeof(iarmData)))
                    {
                        response["success"] = true;
                        returnResponse(true);
                    }
                    else
                    {
                        LOGWARN ("Call to %s for %s failed\n", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setInterfaceEnabled);
                    }
                }
            }
            else
            {
                LOGWARN ("This version of Network Software is not supporting this API..\n");
            }

            returnResponse(false);
        }

        uint32_t Network::getNamedEndpoints(const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            if (m_apiVersionNumber >= 1)
            {
                JsonArray namedEndpoints;
                JsonObject endpoint;

                endpoint["endpoint"] = "CMTS";
                namedEndpoints.Add(endpoint);

                response["endpoints"] = namedEndpoints;
                returnResponse(true);
            }
            else
            {
                LOGWARN ("This version of Network Software is not supporting this API..\n");
            }

            returnResponse(false);
        }

        uint32_t Network::trace(const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            if (m_apiVersionNumber >= 1)
            {
                if (!parameters.HasLabel("endpoint"))
                {
                    LOGERR("No endpoint specified");
                }
                else
                {
                    std::string endpoint = "";
                    int packets = 0;

                    getStringParameter("endpoint", endpoint);
                    if (parameters.HasLabel("packets")) // packets is optional?
                    {
                        getNumberParameter("packets", packets);
                    }

                    if (_doTrace(endpoint, packets, response))
                    {
                        returnResponse(true);
                    }
                    else
                    {
                        LOGERR("Failed to perform network trace");
                    }
                }
            }
            else
            {
                LOGWARN ("This version of Network Software is not supporting this API..\n");
            }

            returnResponse(false);
        }

        uint32_t Network::traceNamedEndpoint(const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            if (m_apiVersionNumber >= 1)
            {
                if (!parameters.HasLabel("endpointName"))
                {
                    LOGERR("No endpointName specified");
                }
                else
                {
                    std::string endpointName = "";
                    int packets = 0;

                    getStringParameter("endpointName", endpointName);
                    if (parameters.HasLabel("packets")) // packets is optional?
                    {
                        getNumberParameter("packets", packets);
                    }

                    if (_doTraceNamedEndpoint(endpointName, packets, response))
                    {
                        returnResponse(true);
                    }
                    else
                    {
                        LOGERR("Failed to perform network trace names endpoint");
                    }
                }
            }
            else
            {
                LOGWARN ("This version of Network Software is not supporting this API..\n");
            }

            returnResponse(false);
        }

        uint32_t Network::ping (const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            if (m_apiVersionNumber >= 1)
            {
                std::string endpoint = "";
                uint32_t packets = DEFAULT_PING_PACKETS;

                if (parameters.HasLabel("packets"))
                {
                    getNumberParameter("packets", packets);
                }

                if (parameters.HasLabel("endpoint"))
                {
                    getStringParameter("endpoint", endpoint);
                    response = _doPing(endpoint, packets);

                    returnResponse(response["success"]);
                }
                else
                {
                    // endpoint is required
                    returnResponse(false);
                }
            }
            else
                LOGWARN ("This version of Network Software is not supporting this API..\n");

            returnResponse(false);
        }

        uint32_t Network::pingNamedEndpoint (const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            if (m_apiVersionNumber >= 1)
            {
                std::string endpoint = "";
                uint32_t packets = DEFAULT_PING_PACKETS;

                if (parameters.HasLabel("packets"))
                {
                    getNumberParameter("packets", packets);
                }

                if (parameters.HasLabel("endpointName"))
                {
                    getStringParameter("endpointName", endpoint);
                    response = _doPingNamedEndpoint(endpoint, packets);

                    returnResponse(response["success"]);
                }
                else
                {
                    // endpoint is required
                    returnResponse(false);
                }
            }
            else
                LOGWARN ("This version of Network Software is not supporting this API..\n");

            returnResponse(false);
        }

        uint32_t Network::setIPSettings(const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            if (m_apiVersionNumber >= 1)
            {
                if ((parameters.HasLabel("interface")) && (parameters.HasLabel("ipversion")) && (parameters.HasLabel("autoconfig")) &&
                    (parameters.HasLabel("ipaddr")) && (parameters.HasLabel("netmask")) && (parameters.HasLabel("gateway")) &&
                    (parameters.HasLabel("primarydns")) && (parameters.HasLabel("secondarydns")))
                {
                    std::string interface = "";
                    std::string ipversion = "";
                    bool autoconfig = false;
                    std::string ipaddr  = "";
                    std::string netmask = "";
                    std::string gateway = "";
                    std::string primarydns   = "";
                    std::string secondarydns = "";

                    getStringParameter("interface", interface);
                    getStringParameter("ipversion", ipversion);
                    getBoolParameter("autoconfig", autoconfig);
                    getStringParameter("ipaddr", ipaddr);
                    getStringParameter("netmask", netmask);
                    getStringParameter("gateway", gateway);
                    getStringParameter("primarydns", primarydns);
                    getStringParameter("secondarydns", secondarydns);

                    IARM_BUS_NetSrvMgr_Iface_Settings_t iarmData = { 0 };
                    strncpy(iarmData.interface, interface.c_str(), 16);
                    strncpy(iarmData.ipversion, ipversion.c_str(), 16);
                    iarmData.autoconfig = autoconfig;
                    strncpy(iarmData.ipaddress, ipaddr.c_str(), 16);
                    strncpy(iarmData.netmask, netmask.c_str(), 16);
                    strncpy(iarmData.gateway, gateway.c_str(), 16);
                    strncpy(iarmData.primarydns, primarydns.c_str(), 16);
                    strncpy(iarmData.secondarydns, secondarydns.c_str(), 16);
                    iarmData.isSupported = true;

                    if (IARM_RESULT_SUCCESS == IARM_Bus_Call (IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setIPSettings, (void *)&iarmData, sizeof(iarmData)))
                    {
                        response["supported"] = iarmData.isSupported;
                        returnResponse(true);
                    }
                    else
                    {
                        response["supported"] = iarmData.isSupported;
                        returnResponse(false);
                    }
                }
            }
            else
            {
                LOGWARN ("This version of Network Software is not supporting this API..\n");
            }

            returnResponse(false);
        }

        uint32_t Network::getIPSettings(const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            if (m_apiVersionNumber >= 1)
            {
                if (parameters.HasLabel("interface"))
                {
                    std::string interface = "";
                    getStringParameter("interface", interface);

                    IARM_BUS_NetSrvMgr_Iface_Settings_t iarmData = { 0 };
                    strncpy(iarmData.interface, interface.c_str(), 16);
                    iarmData.isSupported = true;

                    if (IARM_RESULT_SUCCESS == IARM_Bus_Call (IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getIPSettings, (void *)&iarmData, sizeof(iarmData)))
                    {
                        response["interface"] = string(iarmData.interface);
                        response["ipversion"] = string(iarmData.ipversion);
                        response["autoconfig"] = iarmData.autoconfig;
                        response["ipaddr"] = string(iarmData.ipaddress);
                        response["netmask"] = string(iarmData.netmask);
                        response["gateway"] = string(iarmData.gateway);
                        response["primarydns"] = string(iarmData.primarydns);
                        response["secondarydns"] = string(iarmData.secondarydns);
                        returnResponse(true);
                    }
                    else
                    {
                        returnResponse(false);
                    }
                }
            }
            else
            {
                LOGWARN ("This version of Network Software is not supporting this API..\n");
            }

            returnResponse(false);
        }

        /*
         * Notifications
         */

        void Network::onInterfaceEnabledStatusChanged(string interface, bool enabled)
        {
            JsonObject params;
            params["interface"] = m_netUtils.getInterfaceDescription(interface);
            params["enabled"] = enabled;
            sendNotify("onInterfaceStatusChanged", params);
        }

        void Network::onInterfaceConnectionStatusChanged(string interface, bool connected)
        {
            JsonObject params;
            params["interface"] = m_netUtils.getInterfaceDescription(interface);
            params["status"] = string (connected ? "CONNECTED" : "DISCONNECTED");
            sendNotify("onConnectionStatusChanged", params);
        }

        void Network::onInterfaceIPAddressChanged(string interface, string ipv6Addr, string ipv4Addr, bool acquired)
        {
            JsonObject params;
            params["interface"] = m_netUtils.getInterfaceDescription(interface);
            if (ipv6Addr != "")
            {
                params["ip6Address"] = ipv6Addr;
            }
            if (ipv4Addr != "")
            {
                params["ip4Address"] = ipv4Addr;
            }
            params["status"] = string (acquired ? "ACQUIRED" : "LOST");
            sendNotify("onIPAddressStatusChanged", params);
        }

        void Network::onDefaultInterfaceChanged(string oldInterface, string newInterface)
        {
            JsonObject params;
            params["oldInterfaceName"] = m_netUtils.getInterfaceDescription(oldInterface);
            params["newInterfaceName"] = m_netUtils.getInterfaceDescription(newInterface);
            sendNotify("onDefaultInterfaceChanged", params);
        }

        void Network::eventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (Network::_instance)
                Network::_instance->iarmEventHandler(owner, eventId, data, len);
            else
                LOGWARN("WARNING - cannot handle IARM events without a Network plugin instance!");
        }

        void Network::iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (strcmp(owner, IARM_BUS_NM_SRV_MGR_NAME) != 0)
            {
                LOGERR("ERROR - unexpected event: owner %s, eventId: %d, data: %p, size: %d.", owner, (int)eventId, data, len);
                return;
            }
            if (data == NULL || len == 0)
            {
                LOGERR("ERROR - event with NO DATA: eventId: %d, data: %p, size: %d.", (int)eventId, data, len);
                return;
            }

            switch (eventId)
            {
            case IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_ENABLED_STATUS:
            {
                IARM_BUS_NetSrvMgr_Iface_EventInterfaceEnabledStatus_t *e = (IARM_BUS_NetSrvMgr_Iface_EventInterfaceEnabledStatus_t*) data;
#ifdef NET_DEFINED_INTERFACES_ONLY
                if (m_netUtils.getInterfaceDescription(e->interface) == "")
                    break;
#endif
                onInterfaceEnabledStatusChanged(e->interface, e->status);
                break;
            }
            case IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_CONNECTION_STATUS:
            {
                IARM_BUS_NetSrvMgr_Iface_EventInterfaceConnectionStatus_t *e = (IARM_BUS_NetSrvMgr_Iface_EventInterfaceConnectionStatus_t*) data;
#ifdef NET_DEFINED_INTERFACES_ONLY
                if (m_netUtils.getInterfaceDescription(e->interface) == "")
                    break;
#endif
                onInterfaceConnectionStatusChanged(e->interface, e->status);
                break;
            }
            case IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_IPADDRESS:
            {
                IARM_BUS_NetSrvMgr_Iface_EventInterfaceIPAddress_t *e = (IARM_BUS_NetSrvMgr_Iface_EventInterfaceIPAddress_t*) data;
#ifdef NET_DEFINED_INTERFACES_ONLY
                if (m_netUtils.getInterfaceDescription(e->interface) == "")
                    break;
#endif
                if (e->is_ipv6)
                {
#ifdef NET_NO_LINK_LOCAL_ANNOUNCE
                    if (!m_netUtils.isIPV6LinkLocal(e->ip_address))
#endif
                        onInterfaceIPAddressChanged(e->interface, e->ip_address, "", e->acquired);
                }
                else
                {
#ifdef NET_NO_LINK_LOCAL_ANNOUNCE
                    if (!m_netUtils.isIPV4LinkLocal(e->ip_address))
#endif
                        onInterfaceIPAddressChanged(e->interface, "", e->ip_address, e->acquired);
                }
                break;
            }
            case IARM_BUS_NETWORK_MANAGER_EVENT_DEFAULT_INTERFACE:
            {
                IARM_BUS_NetSrvMgr_Iface_EventDefaultInterface_t *e = (IARM_BUS_NetSrvMgr_Iface_EventDefaultInterface_t*) data;
                onDefaultInterfaceChanged(e->oldInterface, e->newInterface);
                break;
            }
            }
        }

        /*
         * Internal functions
         */

        bool Network::_getDefaultInterface(string& interface, string& gateway)
        {
            IARM_BUS_NetSrvMgr_DefaultRoute_t defaultRoute = {0};
            if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getDefaultInterface, (void*)&defaultRoute, sizeof(defaultRoute)))
            {
                LOGWARN ("Call to %s for %s returned interface = %s, gateway = %s\n", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getDefaultInterface, defaultRoute.interface, defaultRoute.gateway);
                interface = defaultRoute.interface;
                gateway = defaultRoute.gateway;
                return !interface.empty();
            }
            else
            {
                LOGWARN ("Call to %s for %s failed\n", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getDefaultInterface);
                return false;
            }
        }

    } // namespace Plugin
} // namespace WPEFramework
