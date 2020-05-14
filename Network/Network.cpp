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

// Event types
#define EVT_NETWORK_CONNECTION_STATUS_UPDATE     "onConnectionStatusChanged"
#define EVT_NETWORK_CONNECTION_IP_ADDRESS_UPDATE "onIPAddressStatusChanged"


/* Netsrvmgr Based Macros & Structures */
#define IARM_BUS_NM_SRV_MGR_NAME "NET_SRV_MGR"
#define IARM_BUS_NETSRVMGR_API_getActiveInterface "getActiveInterface"
#define IARM_BUS_NETSRVMGR_API_getNetworkInterfaces "getNetworkInterfaces"
#define IARM_BUS_NETSRVMGR_API_isInterfaceEnabled "isInterfaceEnabled"
#define IARM_BUS_NETSRVMGR_API_getInterfaceControlPersistence  "getInterfaceControlPersistence"
#define IARM_BUS_NETSRVMGR_API_getSTBip "getSTBip"
#define INTERFACE_SIZE 10
#define INTERFACE_LIST 50
#define MAX_IP_ADDRESS_LEN 46
#define DEFAULT_PING_PACKETS 15

typedef struct _IARM_BUS_NetSrvMgr_Iface_EventData_t {
    union {
        char activeIface[INTERFACE_SIZE];
        char allNetworkInterfaces[INTERFACE_LIST];
        char enableInterface[INTERFACE_SIZE];
        char activeIfaceIpaddr[MAX_IP_ADDRESS_LEN];
    };
    char interfaceCount;
    bool isInterfaceEnabled;
} IARM_BUS_NetSrvMgr_Iface_EventData_t;

typedef enum _NetworkManager_EventId_t {
    IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_ENABLED=50,
    IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_CONTROL_PERSISTENCE,
    IARM_BUS_NETWORK_MANAGER_MAX,
} IARM_Bus_NetworkManager_EventId_t;

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

            m_apiVersionNumber = 0;
            Network::_instance = NULL;
        }

        const string Network::Initialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();

            Utils::IARM::init();

            return string();
        }

        void Network::Deinitialize(PluginHost::IShell* /* service */)
        {
            LOGINFO();
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
                IARM_Result_t ret = IARM_RESULT_SUCCESS;
                IARM_BUS_NetSrvMgr_Iface_EventData_t param;
                ret = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME,IARM_BUS_NETSRVMGR_API_getNetworkInterfaces, (void*)&param, sizeof(param));
                if (ret == IARM_RESULT_SUCCESS)
                {
                    if(param.interfaceCount)
                    {
                        std::stringstream strm(param.allNetworkInterfaces);
                        std::string interfaceName = "";
                        JsonArray networkInterfaces;

                        /* The list of interfaces from netsrvmgr just returns a comma separated list of WIFI, ETHERNET etc.
                         * We need extra information for this api so we will get it from other calls to netsrvmgr and from
                         * the network monitoring information we have acquired.
                         */
                        while (std::getline(strm, interfaceName, ','))
                        {
                            std::string macAddr = "";
                            bool enabled = false;
                            bool connected = false;
                            JsonObject interface;

                            if (!_getInterfaceEnabled(interfaceName, enabled))
                            {
                                LOGERR("Failed to get enabled state for %s", interfaceName.c_str());
                            }
                            if (!_getInterfaceMACAddress(interfaceName, macAddr))
                            {
                                LOGERR("Failed to get MAC address for %s", interfaceName.c_str());
                            }
                            if (!_getInterfaceConnected(interfaceName, connected))
                            {
                                LOGERR("Failed to get connected state for %s", interfaceName.c_str());
                            }

                            interface["interface"] = interfaceName;
                            interface["macAddress"] = macAddr;
                            interface["enabled"] = enabled;
                            interface["connected"] = connected;

                            networkInterfaces.Add(interface);
                        }

                        response["interfaces"] = networkInterfaces;
                    }
                    else
                    {
                        response["interfaces"] = std::string();
                    }

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
                std::string interface = "";

#ifdef USE_NETLINK
                //TBD - check netlink method gets the correct interface on IPv6
                if (m_netUtils.GetDefaultInterfaceDescription(interface))
                {
                    response["interface"] = interface;
                    returnResponse(true);
                }
#else
                if (_getActiveInterface(interface))
                {
                    response["interface"] = interface;
                    returnResponse(true);
                }
#endif
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
                //TBD - for now, we will only support this for IPv4, how to do this if configured for ipv6?
                if (!NetUtils::isConfiguredIPV6())
                {
                    std::string interface = "";
                    bool persist = false;

                    if (parameters.HasLabel("interface"))
                    {
                        getStringParameter("interface", interface);

                        if (m_netUtils.SetDefaultInterface(interface))
                        {
                            if (parameters.HasLabel("persist"))
                            {
                                getBoolParameter("persist", persist);
                                if (!NetUtils::setDefaultGatewayPersistent(persist ? interface.c_str(): NULL))
                                {
                                    LOGERR("Failed to persist default interface (%s)", interface.c_str());
                                }
                            }

                            returnResponse(true);
                        }
                    }
                    else
                    {
                        LOGWARN ("No interface specified\n");
                    }
                }
                else
                {
                    LOGWARN ("Device is not configured for IPv4\n");
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

        uint32_t Network::isInterfaceEnabled (const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN ("Entering %s \n", __FUNCTION__);
            if (m_apiVersionNumber >= 1)
            {
                std::string interface = "";
                bool enabled = false;

                if (parameters.HasLabel("interface"))
                {
                    getStringParameter("interface", interface);

                    if (_getInterfaceEnabled(interface, enabled))
                    {
                        response["enabled"] = enabled;
                        returnResponse(true);
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
                std::string interface = "";
                bool enabled = false;
                bool persist = false;
                if ((parameters.HasLabel("interface")) && (parameters.HasLabel("enabled")) && (parameters.HasLabel("persist")))
                {
                    getStringParameter("interface", interface);
                    getBoolParameter("enabled", enabled);
                    getBoolParameter("persist", persist);

                    IARM_BUS_NetSrvMgr_Iface_EventData_t iarmData = { 0 };
                    strncpy(iarmData.enableInterface, interface.c_str(), INTERFACE_SIZE);

                    // First set enabled state
                    iarmData.isInterfaceEnabled = enabled;
                    if (IARM_RESULT_SUCCESS == IARM_Bus_BroadcastEvent (IARM_BUS_NM_SRV_MGR_NAME, (IARM_EventId_t) IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_ENABLED, (void *)&iarmData, sizeof(iarmData)))
                    {
                        LOGINFO ("Broadcasted IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_ENABLED = %d for interface '%s'\n", iarmData.isInterfaceEnabled, iarmData.enableInterface);

                        // Then set persist state
                        iarmData.isInterfaceEnabled = persist;
                        if (IARM_RESULT_SUCCESS == IARM_Bus_BroadcastEvent (IARM_BUS_NM_SRV_MGR_NAME, (IARM_EventId_t) IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_CONTROL_PERSISTENCE, (void *)&iarmData, sizeof(iarmData)))
                        {
                            LOGINFO("Broadcasted IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_CONTROL_PERSISTENCE = %d for interface '%s'\n", iarmData.isInterfaceEnabled, iarmData.enableInterface);

                            response["success"] = true;
                            returnResponse(true);
                        }
                        else
                        {
                            LOGWARN ("Failed to broadcast IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_CONTROL_PERSISTENCE = %d for interface '%s'\n", iarmData.isInterfaceEnabled, iarmData.enableInterface);
                        }
                    }
                    else
                    {
                        LOGWARN ("Failed to broadcast IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_ENABLED = %d for interface '%s'\n", iarmData.isInterfaceEnabled, iarmData.enableInterface);
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


        /*
         * Notifications
         */
        void Network::_asyncNotifyConnection(JsonObject &params)
        {
            sendNotify(EVT_NETWORK_CONNECTION_STATUS_UPDATE, params);
        }

        void Network::_asyncNotifyIPAddr(JsonObject &params)
        {
            sendNotify(EVT_NETWORK_CONNECTION_IP_ADDRESS_UPDATE, params);
        }


        /*
         * Internal functions
         */

        bool Network::_getInterfaceEnabled(std::string &interface, bool &enabled)
        {
            IARM_BUS_NetSrvMgr_Iface_EventData_t param = {0};
            strncpy(param.enableInterface, interface.c_str(), INTERFACE_SIZE);
            if (IARM_RESULT_SUCCESS == IARM_Bus_Call (IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_isInterfaceEnabled, (void*)&param, sizeof(param)))
            {
                LOGINFO("%s :: Enabled = %d \n",__FUNCTION__,param.isInterfaceEnabled);
                enabled = param.isInterfaceEnabled;
                return true;
            }
            else
            {
                LOGWARN ("Call to %s for %s failed\n", IARM_BUS_NM_SRV_MGR_NAME, __FUNCTION__);
            }

            return false;
        }

        bool Network::_getActiveInterface(std::string &interface)
        {
            IARM_BUS_NetSrvMgr_Iface_EventData_t param;
            if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getActiveInterface, (void*)&param, sizeof(param)))
            {
                LOGINFO("%s :: Interface = %s \n",__FUNCTION__,param.activeIface);
                interface = param.activeIface;
                return true;
            }
            else
            {
                LOGWARN ("Call to %s for %s failed\n", IARM_BUS_NM_SRV_MGR_NAME, __FUNCTION__);
            }

            return false;
        }

        /* Internal methods */
        bool Network::_getInterfaceMACAddress(std::string &interface, std::string &macAddr)
        {
            if (m_netUtils.GetInterfaceMACAddress(interface, macAddr))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        bool Network::_getInterfaceConnected(std::string &interface, bool &connected)
        {
            if (m_netUtils.GetInterfaceConnected(interface, connected))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    } // namespace Plugin
} // namespace WPEFramework
