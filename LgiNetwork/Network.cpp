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
#include <arpa/inet.h>
#include "dbus/lginetwork_client.hpp"

#include "UtilsJsonRpc.h"
#include "UtilsLogging.h"

using namespace std;

#define DEFAULT_PING_PACKETS 15
#define CIDR_NETMASK_IP_LEN 32

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 2

/* Netsrvmgr Based Macros & Structures */
#define INTERFACE_SIZE 10
#define INTERFACE_LIST 50
#define MAX_IP_ADDRESS_LEN 46
#define MAX_IP_FAMILY_SIZE 10
#define MAX_HOST_NAME_LEN 128
#define MAX_ENDPOINTS 5
#define MAX_ENDPOINT_SIZE 260 // 253 + 1 + 5 + 1 (domain name max length + ':' + port number max chars + '\0')

// TODO: remove this
#define registerMethod(...) for (uint8_t i = 1; GetHandler(i); i++) GetHandler(i)->Register<JsonObject, JsonObject>(__VA_ARGS__)

namespace WPEFramework
{

    namespace {

        static Plugin::Metadata<Plugin::Network> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin
    {
        SERVICE_REGISTRATION(Network, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);
        Network* Network::_instance = nullptr;

        Network::Network()
        : PluginHost::JSONRPC()
        , m_apiVersionNumber(API_VERSION_NUMBER_MAJOR)
        {
            Network::_instance = this;
            m_isPluginInited = false;

            CreateHandler({2});

            // Quirk
            registerMethod("getQuirks", &Network::getQuirks, this);

            // Network_API_Version_1
            registerMethod("getInterfaces", &Network::getInterfaces, this);
            registerMethod("isInterfaceEnabled", &Network::isInterfaceEnabled, this);
            registerMethod("setInterfaceEnabled", &Network::setInterfaceEnabled, this);
            registerMethod("getDefaultInterface", &Network::getDefaultInterface, this);
            registerMethod("setDefaultInterface", &Network::setDefaultInterface, this);

            registerMethod("getStbIp", &Network::getStbIp, this);

            registerMethod("trace", &Network::trace, this);
            registerMethod("traceNamedEndpoint", &Network::traceNamedEndpoint, this);

            registerMethod("getNamedEndpoints", &Network::getNamedEndpoints, this);

            registerMethod("ping",              &Network::ping, this);
            registerMethod("pingNamedEndpoint", &Network::pingNamedEndpoint, this);

            Register("setIPSettings", &Network::setIPSettings, this);
            GetHandler(2)->Register<JsonObject, JsonObject>("setIPSettings", &Network::setIPSettings2, this);
            Register("getIPSettings", &Network::getIPSettings, this);
            GetHandler(2)->Register<JsonObject, JsonObject>("getIPSettings", &Network::getIPSettings2, this);

            registerMethod("getSTBIPFamily", &Network::getSTBIPFamily, this);
            registerMethod("isConnectedToInternet", &Network::isConnectedToInternet, this);
            registerMethod("setConnectivityTestEndpoints", &Network::setConnectivityTestEndpoints, this);

            registerMethod("getPublicIP", &Network::getPublicIP, this);
            registerMethod("setStunEndPoint", &Network::setStunEndPoint, this);

            m_defaultInterface = "";
            m_gatewayInterface = "";

            m_netUtils.InitialiseNetUtils();
            m_stunEndPoint = "stun.l.google.com";
            m_stunPort = 19302;
            m_stunBindTimeout = 30;
            m_stunCacheTimeout = 0;
            m_stunSync = true;
        }

        Network::~Network()
        {
            Network::_instance = nullptr;
        }

        const string Network::Initialize(PluginHost::IShell* /* service */)
        {
            string msg;
            int rcode = m_NetworkClient.Run();
            if (rcode)
            {
                LOGERR("Failed to initialize DBus Network Client (%d)", rcode);
                msg = "Failed to initialize DBus Network Client";
                m_isPluginInited = false;
            }
            else
            {
                m_isPluginInited = true;
                m_NetworkClient.onStatusChangeEvent = StatusChangeEvent;
                m_NetworkClient.onNetworkingEvent = NetworkingEvent;
                LOGINFO("Succeeded initializing LGI DBus Network Client");
            }

            return msg;
        }

        void Network::Deinitialize(PluginHost::IShell* /* service */)
        {
            m_isPluginInited = false;
            m_NetworkClient.Stop();
            Unregister("getQuirks");
            Unregister("getInterfaces");
            Unregister("isInterfaceEnabled");
            Unregister("setInterfaceEnabled");
            Unregister("getDefaultInterface");
            Unregister("setDefaultInterface");
            Unregister("getStbIp");
            Unregister("trace");
            Unregister("traceNamedEndpoint");
            Unregister("getNamedEndpoints");
            Unregister("ping");
            Unregister("pingNamedEndpoint");
            Unregister("setIPSettings");
            Unregister("getIPSettings");
            Unregister("isConnectedToInternet");
            Unregister("setConnectivityTestEndpoints");
            Unregister("getPublicIP");
            Unregister("setStunEndPoint");

            Network::_instance = nullptr;
        }

        string Network::Information() const
        {
             return(string());
        }

        bool Network::isValidCIDRv4(string buf)
        {
            string CIDR_PREFIXES[CIDR_NETMASK_IP_LEN] = {
                                                     "128.0.0.0",
                                                     "192.0.0.0",
                                                     "224.0.0.0",
                                                     "240.0.0.0",
                                                     "248.0.0.0",
                                                     "252.0.0.0",
                                                     "254.0.0.0",
                                                     "255.0.0.0",
                                                     "255.128.0.0",
                                                     "255.192.0.0",
                                                     "255.224.0.0",
                                                     "255.240.0.0",
                                                     "255.248.0.0",
                                                     "255.252.0.0",
                                                     "255.254.0.0",
                                                     "255.255.0.0",
                                                     "255.255.128.0",
                                                     "255.255.192.0",
                                                     "255.255.224.0",
                                                     "255.255.240.0",
                                                     "255.255.248.0",
                                                     "255.255.252.0",
                                                     "255.255.254.0",
                                                     "255.255.255.0",
                                                     "255.255.255.128",
                                                     "255.255.255.192",
                                                     "255.255.255.224",
                                                     "255.255.255.240",
                                                     "255.255.255.248",
                                                     "255.255.255.252",
                                                     "255.255.255.254",
                                                     "255.255.255.255",
                                                   };
            int i = 0;
            bool retval = false;
            while(i < CIDR_NETMASK_IP_LEN)
            {
                if((buf.compare(CIDR_PREFIXES[i])) == 0)
	        {
                    retval = true;
                    break;
                }
                i++;
            }
            return retval;
	    }

        // Wrapper methods
        uint32_t Network::getQuirks(const JsonObject& parameters, JsonObject& response)
        {
            JsonArray array;
            array.Add("RDK-20093");
            response["quirks"] = array;
            returnResponse(true)
        }

        uint32_t Network::getInterfaces (const JsonObject& parameters, JsonObject& response)
        {
            bool result = false;

            if (m_isPluginInited)
            {
                JsonArray networkInterfaces;
                std::vector<std::string>* interfaces = m_NetworkClient.getInterfaces();
                if (interfaces)
                {
                    for (auto& iface : *interfaces)
                    {
                        JsonObject interface;
                        std::map<std::string, std::string> parameters;
                        parameters[lginet::ParamMac] = "";
                        parameters[lginet::ParamConnectivity] = "";

                        if (iface == "")
                            continue;
                        if (m_NetworkClient.getSpecificParamsForInterface(iface, parameters))
                        {
#ifdef LGINET_STRICT_IFACE_NAMES
                            interface["interface"] = getTypeOfInterface(iface);
#else
                            interface["interface"] = iface;
#endif
                            interface["macAddress"] = parameters[lginet::ParamMac];
                            interface["connected"] = (parameters[lginet::ParamConnectivity] == "yes");
                            interface["enabled"] = m_NetworkClient.isInterfaceEnabled(iface);
                            networkInterfaces.Add(interface);
                        }
                    }
                    delete interfaces;
                    response["interfaces"] = networkInterfaces;
                    result = true;
                }
                else
                {
                    LOGWARN ("Call for %s failed", __FUNCTION__);
                }
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            returnResponse(result)
        }

        uint32_t Network::getDefaultInterface (const JsonObject& parameters, JsonObject& response)
        {
            string interface;
            string gateway;
            string type;

            bool result = false;
            if (m_isPluginInited)
            {
                if (_getDefaultInterface(interface, gateway, type))
                {
#ifdef LGINET_STRICT_IFACE_NAMES
                    response["interface"] = type;
#else
                    response["interface"] = interface;
#endif
                    result = true;
                }
                else
                {
                    LOGERR("Failed to get default interface");
                }
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            returnResponse(result);
        }

        uint32_t Network::setDefaultInterface (const JsonObject& parameters, JsonObject& response)
        {
            bool result = false;

            if(m_isPluginInited)
            {
                if ((parameters.HasLabel("interface")) && (parameters.HasLabel("persist")))
                {
                    // TODO:
                    LOGERR("Call %s not implemented", __FUNCTION__);
                }
                else
                {
                    LOGWARN ("Required attributes are not provided.");
                }
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            returnResponse(result)
        }

        uint32_t Network::getStbIp(const JsonObject &parameters, JsonObject &response)
        {
            bool result = false;

            if (m_isPluginInited)
            {
                // TODO:
                LOGERR("Call %s not implemented\n", __FUNCTION__);
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            returnResponse(result)
        }

        uint32_t Network::getSTBIPFamily(const JsonObject &parameters, JsonObject &response)
        {
            bool result = false;

            if (m_isPluginInited)
            {
                if (parameters.HasLabel("family"))
                {
                    // TODO:
                    LOGERR("Call %s not implemented\n", __FUNCTION__);
                }
                else
                {
                    LOGWARN ("Required Family Attribute is not provided.");
                }
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            returnResponse(result)
        }

        uint32_t Network::isInterfaceEnabled (const JsonObject& parameters, JsonObject& response)
        {
            bool result = false;

            if (m_isPluginInited)
            {
                if (parameters.HasLabel("interface"))
                {
                    string interface = "";
                    getStringParameter("interface", interface);
                    interface = convertIfaceName(interface);

                    response["enabled"] = m_NetworkClient.isInterfaceEnabled(interface);
                    result = true;
                }
                else
                {
                    LOGWARN("Required Interface attribute is not provided.");
                }
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            returnResponse(result)
        }

        // API expects either ETHERNET or WIFI as iface name, not actual, physical iface name
        // (like eth2 or wifi0). Convert API name to expected interface name.
        std::string Network::convertIfaceName(const std::string iface)
        {
            std::string result = "";
            bool WantWifi = false;
            if (iface == "WIFI")
                WantWifi = true;
            else if (iface != "ETHERNET")
            {
                // neither ethernet nor wifi, play dumb and assume someone passed physical name
                LOGINFO("unexpected interface name (%s), passing as-is in good faith", iface.c_str());
                return iface;
            }

            std::vector<std::string>* names = m_NetworkClient.getInterfaces();
            if (names)
            {
                for (auto& name : *names)
                {
                        std::map<std::string, std::string> parameters;
                        parameters[lginet::ParamType] = "";
                        if (m_NetworkClient.getSpecificParamsForInterface(name, parameters))
                        {
                            if ((parameters[lginet::ParamType] == "eth" && !WantWifi) ||
                                (parameters[lginet::ParamType] == "wifi" && WantWifi))
                            {
                                result = name;
                                break;
                            }
                        }
                }
                delete(names);
            }

            return result;
        }

#ifdef LGINET_STRICT_IFACE_NAMES
        // convertIfaceName is outside of this ifdef as we want to be
        // liberal in what we get and handle gracefully bad api usage
        string Network::getTypeOfInterface(const string interface)
        {
            if (interface.size() >= 4)
            {
                if (strncmp(interface.c_str(), "eth", 3) == 0)
                    return "ETHERNET";
                if (strncmp(interface.c_str(), "wlan", 4) == 0)
                    return "WIFI";
            }

            LOGERR("Cannot determine interface type for %s", interface.c_str());
            return "N/A";
        }
#endif

        uint32_t Network::setInterfaceEnabled (const JsonObject& parameters, JsonObject& response)
        {
            bool result = false;

            if (m_isPluginInited)
            {
                if ((parameters.HasLabel("interface")) && (parameters.HasLabel("enabled")) /*&& (parameters.HasLabel("persist"))*/)
                {
                    std::string interface;
                    bool enabled = false;
                    // TODO:
                    // bool persist = false;

                    getStringParameter("interface", interface);
                    getBoolParameter("enabled", enabled);
                    interface = convertIfaceName(interface);

                    if (!m_NetworkClient.setInterfaceEnabled(interface, enabled))
                    {
                        LOGERR("call to setInterfaceEnabled failed");
                    }
                    else
                    {
                        result = true;
                    }
                }
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            returnResponse(result)
        }

        uint32_t Network::getNamedEndpoints(const JsonObject& parameters, JsonObject& response)
        {
            JsonArray namedEndpoints;
            namedEndpoints.Add("CMTS");

            response["endpoints"] = namedEndpoints;
            returnResponse(true)
        }

        uint32_t Network::trace(const JsonObject& parameters, JsonObject& response)
        {
            bool result = false;

            if(m_isPluginInited)
            {
                if (!parameters.HasLabel("endpoint"))
                    LOGERR("No endpoint specified");
                else
                {
                    string endpoint = "";
                    int packets = 0;

                    getStringParameter("endpoint", endpoint);
                    if (parameters.HasLabel("packets")) // packets is optional?
                        getNumberParameter("packets", packets);

                    if (_doTrace(endpoint, packets, response))
                        result = true;
                    else
                        LOGERR("Failed to perform network trace");
                }
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            returnResponse(result)
        }

        uint32_t Network::traceNamedEndpoint(const JsonObject& parameters, JsonObject& response)
        {
            bool result = false;

            if (m_isPluginInited)
            {
                if (!parameters.HasLabel("endpointName"))
                    LOGERR("No endpointName specified");
                else
                {
                    string endpointName = "";
                    int packets = 0;

                    getStringParameter("endpointName", endpointName);
                    if (parameters.HasLabel("packets")) // packets is optional?
                        getNumberParameter("packets", packets);

                    if (_doTraceNamedEndpoint(endpointName, packets, response))
                        result = true;
                    else
                        LOGERR("Failed to perform network trace names endpoint");
                }
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            returnResponse(result)
        }

        uint32_t Network::ping (const JsonObject& parameters, JsonObject& response)
        {
            string guid;
            getStringParameter("guid", guid)

            uint32_t packets;
            getDefaultNumberParameter("packets", packets, DEFAULT_PING_PACKETS);

            bool result = false;

            if (m_isPluginInited)
            {
                if (parameters.HasLabel("endpoint"))
                {
                    string endpoint;
                    getStringParameter("endpoint", endpoint);
                    response = _doPing(guid, endpoint, packets);
                    result = response["success"].Boolean();
                }
                else
                {
                    LOGERR("No endpoint argument");
                }
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }
            returnResponse(result)
        }

        uint32_t Network::pingNamedEndpoint (const JsonObject& parameters, JsonObject& response)
        {
            string guid;
            getStringParameter("guid", guid)

            uint32_t packets;
            getDefaultNumberParameter("packets", packets, DEFAULT_PING_PACKETS);

            bool result = false;

            if (m_isPluginInited)
            {
                if (parameters.HasLabel("endpointName"))
                {
                    string endpointName;
                    getDefaultStringParameter("endpointName", endpointName, "");

                    response = _doPingNamedEndpoint(guid, endpointName, packets);
                    result = response["success"].Boolean();
                }
                else
                {
                    LOGERR("No endpointName argument");
                }
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            returnResponse(result)
        }

        uint32_t Network::setIPSettings(const JsonObject& parameters, JsonObject& response)
        {
            bool result = false;

            if (m_isPluginInited)
                return  setIPSettingsInternal(parameters, response);
            else
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);

            returnResponse(result)
        }

        uint32_t Network::setIPSettings2(const JsonObject& parameters, JsonObject& response)
        {
            JsonObject internal;
            string interface = "";
            string ipversion = "";
            string netmask = "";
            string gateway = "";
            string ipaddr = "";
            string primarydns = "";
            string secondarydns = "";
            bool autoconfig = true;
            bool result = false;

            if(m_isPluginInited)
            {
                getDefaultStringParameter("interface", interface, "");
                internal ["interface"] = interface;
                getDefaultStringParameter("ipversion", ipversion, "");
                internal ["ipversion"] = ipversion;
                getDefaultBoolParameter("autoconfig", autoconfig, true);
                internal ["autoconfig"] = autoconfig;
                getDefaultStringParameter("ipaddr", ipaddr, "");
                internal ["ipaddr"] = ipaddr;
                getDefaultStringParameter("netmask", netmask, "");
                internal ["netmask"] = netmask;
                getDefaultStringParameter("gateway", gateway, "");
                internal ["gateway"] = gateway;
                getDefaultStringParameter("primarydns", primarydns, "0.0.0.0");
                internal ["primarydns"] = primarydns;
                getDefaultStringParameter("secondarydns", secondarydns, "");
                internal ["secondarydns"] = secondarydns;

                return  setIPSettingsInternal(internal, response);
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }
            returnResponse(result)
        }

        uint32_t Network::setIPSettingsInternal(const JsonObject& parameters, JsonObject& response)
        {
            bool result = false;
            struct in_addr ip_address, gateway_address, mask;
            struct in_addr broadcast_addr1, broadcast_addr2;

            if ((parameters.HasLabel("interface")) && (parameters.HasLabel("ipversion")) && (parameters.HasLabel("autoconfig")) &&
                (parameters.HasLabel("ipaddr")) && (parameters.HasLabel("netmask")) && (parameters.HasLabel("gateway")) &&
                (parameters.HasLabel("primarydns")) && (parameters.HasLabel("secondarydns")))
            {
                response["supported"] = false;
                result = false;
                LOGERR("Call to %s not implemented.", __FUNCTION__);
            }

            returnResponse(result)
        }

        uint32_t Network::getIPSettings(const JsonObject& parameters, JsonObject& response)
        {
            bool result = false;
            if (m_isPluginInited)
            {
                string interface = "";
                string ipversion = "";

                if ((parameters.HasLabel("interface")) || (parameters.HasLabel("ipversion")))
                {
                    getStringParameter("interface", interface);
                    getStringParameter("ipversion", ipversion);

                    bool isIpv6 = ipversion == "IPv6";

                    string iface = convertIfaceName(interface);

                    std::map<std::string, std::string> parameters;
                    parameters[lginet::ParamDns] = "";

                    if (isIpv6)
                    {
                        LOGWARN("Reading ipv6 stats");
                        parameters[lginet::ParamIpv6Ip] = "";
                        parameters[lginet::ParamIpv6PrefixLength] = "";
                        parameters[lginet::ParamIpv6Gateway] = "";
                        parameters[lginet::ParamIpv6DhcpServer] = "";
                        parameters[lginet::ParamIpv6DhcpOptionPrefix] = "";
                    }
                    else
                    {
                        LOGWARN("Reading ipv4 stats");
                        parameters[lginet::ParamIpv4Ip] = "";
                        parameters[lginet::ParamIpv4Mask] = "";
                        parameters[lginet::ParamIpv4PrefixLength] = "";
                        parameters[lginet::ParamIpv4Gateway] = "";
                        parameters[lginet::ParamIpv4DhcpServer] = "";
                        parameters[lginet::ParamIpv4DhcpOptionPrefix] = "";
                    }

                    if (m_NetworkClient.getSpecificParamsForInterface(iface, parameters))
                    {
                        // retutn original interface name in case it was converted
                        response["interface"] = interface;
                        response["ipversion"] = ipversion;
                        response["primarydns"] = parameters[lginet::ParamDns];
                        response["autoconfig"] = (parameters[lginet::ParamIpv4DhcpServer] != "") ||
                                                 (parameters[lginet::ParamIpv6DhcpServer] != "");
                        if (isIpv6)
                        {
                            response["ipaddr"] = parameters[lginet::ParamIpv6Ip];
                            response["gateway"] = parameters[lginet::ParamIpv6Gateway];
                        }
                        else
                        {
                            response["ipaddr"] = parameters[lginet::ParamIpv4Ip];
                            response["netmask"] = parameters[lginet::ParamIpv4Mask];
                            response["gateway"] = parameters[lginet::ParamIpv4Gateway];
                        }
                        result = true;
                    }
                }
                else
                {
                    LOGERR("Required attributes (interface and/or ipversion) are missing");
                }
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            returnResponse(result)
        }

        uint32_t Network::getIPSettings2(const JsonObject& parameters, JsonObject& response)
        {
            // stub method.
            // turns out the getIpSettings in original method doesn't return _anything_ when IP isn't acquired yet
            // due to dhcp server absence/failure and their solution was to add new one that returns just
            // interface and autoconfig fields in that case. We're already doing it so just pass it to other method.
            return getIPSettings(parameters, response);
        }

        uint32_t Network::isConnectedToInternet (const JsonObject &parameters, JsonObject &response)
        {
            bool result = false;
            bool connected = false;

            if (m_isPluginInited)
            {
                std::vector<std::string>* interfaces = m_NetworkClient.getInterfaces();
                if (interfaces)
                {
                    for (auto& iface : *interfaces)
                    {
                        JsonObject interface;
                        std::map<std::string, std::string> parameters;
                        parameters[lginet::ParamConnectivity] = "";

                        if (iface == "")
                            continue;

                        if (m_NetworkClient.getSpecificParamsForInterface(iface, parameters))
                        {
                            connected = connected | (parameters[lginet::ParamConnectivity] == "yes");
                        }
                    }
                    delete interfaces;
                    result = true;
                }
                else
                {
                    LOGWARN ("Call for %s failed", __FUNCTION__);
                }
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }
            response["connectedToInternet"] = connected;
            returnResponse(result);
        }

        uint32_t Network::setConnectivityTestEndpoints (const JsonObject &parameters, JsonObject &response)
        {
            bool result = true;
            LOGWARN("Call ignored, not applicable here");
            returnResponse(result);
        }

        uint32_t Network::getPublicIP(const JsonObject& parameters, JsonObject& response)
        {
            JsonObject internal;
            internal ["server"] = m_stunEndPoint;
            internal ["port"] = m_stunPort;
            internal ["timeout"] = m_stunBindTimeout;
            internal ["cache_timeout"] = m_stunCacheTimeout;
            internal ["sync"] = m_stunSync;

            if (parameters.HasLabel("iface"))
                internal ["iface"] = parameters["iface"];

            if (parameters.HasLabel("ipv6"))
                internal ["ipv6"] = parameters["ipv6"];

            return  getPublicIPInternal(internal, response);
        }

        uint32_t Network::setStunEndPoint(const JsonObject& parameters, JsonObject& response)
        {
            getDefaultStringParameter("server", m_stunEndPoint, "stun.l.google.com");
            getDefaultNumberParameter("port", m_stunPort, 19302);
            getDefaultBoolParameter("sync", m_stunSync, true);
            getDefaultNumberParameter("timeout", m_stunBindTimeout, 30);
            getDefaultNumberParameter("cache_timeout", m_stunCacheTimeout, 0);

            returnResponse(true);
	    }

        uint32_t Network::getPublicIPInternal(const JsonObject& parameters, JsonObject& response)
        {
            bool result = false;

            string server, iface;
            int port;

            if (m_isPluginInited)
            {
                getStringParameter("server", server);
                if (server.length() > MAX_HOST_NAME_LEN - 1)
                {
                    LOGWARN("invalid args: server exceeds max length of %u", MAX_HOST_NAME_LEN);
                    returnResponse(result)
                }

                getNumberParameter("port", port);

                /*only makes sense to get both server and port or neither*/
                if (!server.empty() && !port)
                {
                    LOGWARN("invalid args: port missing");
                    returnResponse(result)
                }
                if (port && server.empty())
                {
                    LOGWARN("invalid args: server missing");
                    returnResponse(result)
                }

                getDefaultStringParameter("iface", iface, "");
                if (iface.length() > 16 - 1)
                {
                    LOGWARN("invalid args: interface exceeds max length of 16");
                    returnResponse(result)
                }

                // TODO:
                LOGERR("Call %s not implememted\n", __FUNCTION__);
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            response["success"] = result;
            return (Core::ERROR_NONE);
        }

        /*
         * Notifications
         * Note that some LGI NCS signals might be translated to different
         * RDKNetwork notification depending solely on signal parameters.
         * It's intentional.
         */

        void Network::onInterfaceEnabledStatusChanged(string interface, bool enabled)
        {
            JsonObject params;
            params["interface"] = interface;
            params["enabled"] = enabled;
            sendNotify("onInterfaceStatusChanged", params);
        }

        void Network::StatusChangeEvent(const std::string id, const std::string status)
        {
            if (Network::_instance)
            {
                LOGINFO("StatusChangeEvent entry id=%s status=%s", id.c_str(), status.c_str());
                if (status == "Disconnected")
                    Network::_instance->onInterfaceConnectionStatusChanged(id, false);
                else if (status == "Assigned")
                    Network::_instance->onInterfaceConnectionStatusChanged(id, true);
                else if (status == "Disabled")
                    Network::_instance->onInterfaceEnabledStatusChanged(id, false);
                else if (status == "Dormant")
                    Network::_instance->onInterfaceIPAddressChanged(id, "", "", false);
                LOGINFO("StatusChangeEvent exit");
            }
        }

        void Network::NetworkingEvent(const string id, const string event, const std::map<string, string> params)
        {
            if (Network::_instance)
                Network::_instance->onNetworkingEvent(id, event, params);
        }

        void Network::onNetworkingEvent(const string id, const string event, const std::map<string, string> params)
        {
            LOGINFO("onNetworkingEvent entry id=%s event=%s", id.c_str(), event.c_str());
            if (event == "dhcp4.options")
            {
                auto iter = params.find("ip");
                if (iter != params.end())
                {
                    m_dhcpEventSeen = true;
                    // this might trigger two IPAddressChanged events but that's fine
                    // - first one might not be actually triggered as ip isn't acquired
                    // yet at that time
                    onInterfaceIPAddressChanged(id, "", iter->second, true);
                }
            }
            else if (event == "dhcp6.options")
            {
                auto iter = params.find("ip");
                if (iter != params.end())
                {
                    m_dhcpEventSeen = true;
                    onInterfaceIPAddressChanged(id, iter->second, "", true);
                }
            }
            else if (event == "configuration.changed")
            {
                auto iter = params.find(lginet::ParamConnectivity);
                if (iter != params.end())
                    onInterfaceConnectionStatusChanged(id, iter->second == "yes");
                iter = params.find(lginet::ParamNetworkChanged);
                if (iter != params.end())
                {
                    // trigger ip change either if we got ack from dhcp or when
                    // network.changed is triggered but not but both in quick succession. dhcp
                    // event preceeds network.changed so reset it here - that way new (later)
                    // network.changed events are seen.
                    if (m_dhcpEventSeen)
                    {
                        m_dhcpEventSeen = false;
                    }
                    else
                    {
                        std::map<std::string, std::string> parameters;
                        parameters[lginet::ParamIpv4Ip] = "";
                        parameters[lginet::ParamIpv6Ip] = "";

                        if (m_NetworkClient.getSpecificParamsForInterface(id, parameters))
                        {
                            if (parameters[lginet::ParamIpv4Ip] != "" || parameters[lginet::ParamIpv6Ip] != "")
                                onInterfaceIPAddressChanged(id, parameters[lginet::ParamIpv6Ip], parameters[lginet::ParamIpv4Ip], true);
                        }
                    }
                }
            }
            LOGINFO("onNetworkingEvent exit");
        }

        void Network::onInterfaceConnectionStatusChanged(string interface, bool connected)
        {
            JsonObject params;
#ifdef LGINET_STRICT_IFACE_NAMES
            params["interface"] = getTypeOfInterface(interface);
#else
            params["interface"] = interface;
#endif

            params["status"] = string (connected ? "CONNECTED" : "DISCONNECTED");
            sendNotify("onConnectionStatusChanged", params);
        }

        void Network::onInterfaceIPAddressChanged(string interface, string ipv6Addr, string ipv4Addr, bool acquired)
        {
            JsonObject params;
#ifdef LGINET_STRICT_IFACE_NAMES
            params["interface"] = getTypeOfInterface(interface);
#else
            params["interface"] = interface;
#endif
            // prevent re-triggering event for the same IP address
            if (acquired)
            {
                if (m_oldIpv4 != ipv4Addr)
                {
                    if (ipv4Addr != "")
                    {
                        params["ip4Address"] = ipv4Addr;
                    }
                    m_oldIpv4 = ipv4Addr;
                }

                if (m_oldIpv6 != ipv6Addr)
                {
                    if (ipv6Addr != "")
                    {
                        params["ip6Address"] = ipv6Addr;
                    }
                    m_oldIpv6 = ipv6Addr;
                }
            }
            else
            {
                params["ip4Address"] = m_oldIpv4;
                params["ip6Address"] = m_oldIpv6;
            }
            params["status"] = string (acquired ? "ACQUIRED" : "LOST");
            sendNotify("onIPAddressStatusChanged", params);
        }

        void Network::onDefaultInterfaceChanged(string oldInterface, string newInterface)
        {
            JsonObject params;
#ifdef LGINET_STRICT_IFACE_NAMES
            params["oldInterfaceName"] = getTypeOfInterface(oldInterface);
            params["newInterfaceName"] = getTypeOfInterface(newInterface);
#else
            params["oldInterfaceName"] = oldInterface;
            params["newInterfaceName"] = newInterface;
#endif
            sendNotify("onDefaultInterfaceChanged", params);
        }

        /*
         * Internal functions
         */

        bool Network::_getDefaultInterface(string& interface, string& gateway, string& type)
        {
            bool result = false;

            if (m_isPluginInited)
            {
                interface = m_NetworkClient.getDefaultInterface();
                if (interface != "")
                {
                    std::map<std::string, std::string> parameters;
                    parameters[lginet::ParamIpv4Gateway] = "";
                    parameters[lginet::ParamIpv6Gateway] = "";
                    parameters[lginet::ParamType] = "";
                    if (m_NetworkClient.getSpecificParamsForInterface(interface, parameters))
                    {
                        type = parameters[lginet::ParamType];
                        if (type == "eth")
                            type = "ETHERNET";
                        else if (type == "wifi")
                            type = "WIFI";

                        if (parameters[lginet::ParamIpv4Gateway] != "")
                        {
                            gateway = parameters[lginet::ParamIpv4Gateway];
                            result = true;
                        }
                        else if (parameters[lginet::ParamIpv6Gateway] != "")
                        {
                            gateway = parameters[lginet::ParamIpv6Gateway];
                            result = true;
                        }
                    }
                }

                if (result)
                    LOGINFO("Evaluated default network interface: '%s' and gateway: '%s'", interface.c_str(), gateway.c_str());
                else
                    LOGWARN("Unable to detect default network interface");
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            return result;
        }

    } // namespace Plugin
} // namespace WPEFramework
