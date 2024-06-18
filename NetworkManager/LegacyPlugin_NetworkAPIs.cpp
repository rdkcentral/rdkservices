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
#include "LegacyPlugin_NetworkAPIs.h"
#include "NetworkManagerLogger.h"


using namespace std;
using namespace WPEFramework::Plugin;
#define API_VERSION_NUMBER_MAJOR 2
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0
#define NETWORK_MANAGER_CALLSIGN    "org.rdk.NetworkManager.1"
#define SUBSCRIPTION_TIMEOUT_IN_MILLISECONDS 5000

#define LOGINFOMETHOD() { string json; parameters.ToString(json); NMLOG_TRACE("Legacy params=%s", json.c_str() ); }
#define LOGTRACEMETHODFIN() { string json; response.ToString(json); NMLOG_TRACE("Legacy response=%s", json.c_str() ); }

namespace WPEFramework
{
    class Job : public Core::IDispatch {
    public:
        Job(function<void()> work)
        : _work(work)
        {
        }
        void Dispatch() override
        {
            _work();
        }

    private:
        function<void()> _work;
    };

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

    Network* _gNWInstance = nullptr;
    namespace Plugin
    {
        SERVICE_REGISTRATION(Network, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);
        Network::Network()
        : PluginHost::JSONRPC()
        , m_service(nullptr)
        , m_subsIfaceStateChange(false)
        , m_subsActIfaceChange(false)
        , m_subsIPAddrChange(false)
        , m_subsInternetChange(false)
       {
           _gNWInstance = this;
           m_defaultInterface = "wlan0";
           m_timer.connect(std::bind(&Network::subscribeToEvents, this));
           RegisterLegacyMethods();
       }

        Network::~Network()
        {
            _gNWInstance = nullptr;
        }

        void Network::activatePrimaryPlugin()
        {
            uint32_t result = Core::ERROR_ASYNC_FAILED;
            string callsign(NETWORK_MANAGER_CALLSIGN);
            Core::Event event(false, true);
            Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create([&]() {
                auto interface = m_service->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                if (interface == nullptr) {
                    result = Core::ERROR_UNAVAILABLE;
                    NMLOG_WARNING("no IShell for %s", callsign.c_str());
                } else {
                    NMLOG_INFO("Activating %s", callsign.c_str());
                    result = interface->Activate(PluginHost::IShell::reason::REQUESTED);
                    interface->Release();
                }
                event.SetEvent();
            })));
            event.Lock();

            return;
        }

        const string Network::Initialize(PluginHost::IShell*  service )
        {
            m_service = service;
            m_service->AddRef();

            string callsign(NETWORK_MANAGER_CALLSIGN);

            string token = "";

            // TODO: use interfaces and remove token
            auto security = m_service->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
            if (security != nullptr) {
                string payload = "http://localhost";
                if (security->CreateToken(
                            static_cast<uint16_t>(payload.length()),
                            reinterpret_cast<const uint8_t*>(payload.c_str()),
                            token)
                        == Core::ERROR_NONE) {
                    NMLOG_TRACE("Network plugin got security token");
                } else {
                    NMLOG_WARNING("Network plugin failed to get security token");
                }
                security->Release();
            } else {
                NMLOG_INFO("Network plugin: No security agent");
            }

            string query = "token=" + token;

            auto interface = m_service->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
            if (interface != nullptr)
            {
                PluginHost::IShell::state state = interface->State(); 
                NMLOG_INFO("Current status of the %s plugin is %d", callsign.c_str(), state);

                if((PluginHost::IShell::state::ACTIVATED  == state) || (PluginHost::IShell::state::ACTIVATION == state))
                {
                    NMLOG_INFO("Dependency Plugin '%s' Ready", callsign.c_str());
                }
                else
                {
                    activatePrimaryPlugin();
                }
                interface->Release();
            }
        
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
            m_networkmanager = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >(_T(NETWORK_MANAGER_CALLSIGN), _T(NETWORK_MANAGER_CALLSIGN), false, query);

            m_timer.start(SUBSCRIPTION_TIMEOUT_IN_MILLISECONDS);
            return string();
        }

        void Network::Deinitialize(PluginHost::IShell* /* service */)
        {
            UnregisterLegacyMethods();
            m_service->Release();
            m_service = nullptr;
            _gNWInstance = nullptr;
        }

        string Network::Information() const
        {
             return(string());
        }

        /**
         * Hook up all our JSON RPC methods
         *
         * Each method definition comprises of:
         *  * Input parameters
         *  * Output parameters
         *  * Method name
         *  * Function that implements that method
         */
        void Network::RegisterLegacyMethods()
        {
            CreateHandler({2});

            Register("getStbIp",                          &Network::getStbIp, this);
            Register("getSTBIPFamily",                    &Network::getSTBIPFamily, this);
            Register("getInterfaces",                     &Network::getInterfaces, this);
            Register("isInterfaceEnabled",                &Network::isInterfaceEnabled, this);
            Register("getPublicIP",                       &Network::getPublicIP, this);
            Register("setInterfaceEnabled",               &Network::setInterfaceEnabled, this);
            Register("getDefaultInterface",               &Network::getDefaultInterface, this);
            Register("setDefaultInterface",               &Network::setDefaultInterface, this);
            Register("setIPSettings",                     &Network::setIPSettings, this);
            Register("getIPSettings",                     &Network::getIPSettings, this);
            Register("getInternetConnectionState",        &Network::getInternetConnectionState, this);
            Register("ping",                              &Network::doPing, this);
            Register("isConnectedToInternet",             &Network::isConnectedToInternet, this);
            Register("setStunEndPoint",                   &Network::setStunEndPoint, this);
            Register("trace",                             &Network::doTrace, this);
            Register("setConnectivityTestEndpoints",      &Network::setConnectivityTestEndpoints, this);
            Register("startConnectivityMonitoring",       &Network::startConnectivityMonitoring, this);
            Register("getCaptivePortalURI",               &Network::getCaptivePortalURI, this);
            Register("stopConnectivityMonitoring",        &Network::stopConnectivityMonitoring, this);
            GetHandler(2)->Register<JsonObject, JsonObject>("setIPSettings", &Network::setIPSettings, this);
            GetHandler(2)->Register<JsonObject, JsonObject>("getIPSettings", &Network::getIPSettings, this);
        }

        /**
         * Unregister all our JSON-RPC methods
         */
        void Network::UnregisterLegacyMethods()
        {
            Unregister("getInterfaces");
            Unregister("isInterfaceEnabled");
            Unregister("getPublicIP");
            Unregister("setInterfaceEnabled");
            Unregister("getDefaultInterface");
            Unregister("setDefaultInterface");
            Unregister("setIPSettings");
            Unregister("getIPSettings");
            Unregister("getInternetConnectionState");
            Unregister("ping");
            Unregister("isConnectedToInternet");
            Unregister("setConnectivityTestEndpoints");
            Unregister("startConnectivityMonitoring");
            Unregister("getCaptivePortalURI");
            Unregister("stopConnectivityMonitoring");
        }

#define CIDR_NETMASK_IP_LEN 32
const string CIDR_PREFIXES[CIDR_NETMASK_IP_LEN] = {
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

        uint32_t Network::getInterfaces (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;
            JsonArray array;

            LOGINFOMETHOD();

            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("GetAvailableInterfaces"), parameters, tmpResponse);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if ((rc == Core::ERROR_NONE) && (tmpResponse["success"].Boolean()))
            {
                const JsonArray& tmpArray = tmpResponse["interfaces"].Array();
                for (int i=0; i<tmpArray.Length(); i++)
                {
                    JsonObject each;
                    const JsonObject& arrayEntry = tmpArray[i].Object();
                    each[_T("interface")] = arrayEntry["type"];
                    each[_T("macAddress")] = arrayEntry["mac"];
                    each[_T("enabled")] = arrayEntry["isEnabled"];
                    each[_T("connected")] = arrayEntry["isConnected"];

                    array.Add(JsonValue(each));
                }

                response["interfaces"] = array;
                response["success"] = tmpResponse["success"];
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t Network::setStunEndPoint(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpParameters;
            tmpParameters["endPoint"] = parameters["server"];
            tmpParameters["port"] = parameters["port"];
            tmpParameters["bindTimeout"] = parameters["timeout"];
            tmpParameters["cacheTimeout"] = parameters["cache_timeout"];

            if (m_networkmanager)
                rc =  m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("SetStunEndpoint"), tmpParameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t Network::setInterfaceEnabled (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            string interface;
            JsonObject tmpParameters;
            
            LOGINFOMETHOD();
            if("WIFI" == parameters["interface"].String())
                interface = "wlan0";
            else if("ETHERNET" == parameters["interface"].String())
                interface = "eth0";
            
            tmpParameters["interface"] = interface;
            tmpParameters["enabled"]  = parameters["enabled"];

            if (m_networkmanager)
            {
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("SetInterfaceState"), tmpParameters, response);
            }
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t Network::getDefaultInterface (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;

            LOGINFOMETHOD();

            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("GetPrimaryInterface"), parameters, tmpResponse);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                if ("wlan0" == tmpResponse["interface"].String())
                    response["interface"] = "WIFI";
                else if("eth0" == tmpResponse["interface"].String())     
                    response["interface"] = "ETHERNET";
                response["success"] = tmpResponse["success"];
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t Network::setDefaultInterface(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpParameters;
            string interface;
            LOGINFOMETHOD();
            if("WIFI" == parameters["interface"].String())
                tmpParameters["interface"] = "wlan0";
            else if("ETHERNET" == parameters["interface"].String())
                tmpParameters["interface"] = "eth0";
            
            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("SetPrimaryInterface"), tmpParameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();

            return rc;
        }

        uint32_t Network::setIPSettings(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;
            JsonObject tmpParameters;
            LOGINFOMETHOD();
            
            if("WIFI" == parameters["interface"].String())
                tmpParameters["interface"] = "wlan0";
            else if("ETHERNET" == parameters["interface"].String())
                tmpParameters["interface"] = "eth0";
            
            tmpParameters["ipversion"] = parameters["ipversion"];
            tmpParameters["autoconfig"] = parameters["autoconfig"];
            tmpParameters["ipaddress"] = parameters["ipaddr"];
            auto it = find(begin(CIDR_PREFIXES), end(CIDR_PREFIXES), parameters["netmask"].String());
            if (it == end(CIDR_PREFIXES))
                return rc;
            else
                tmpParameters["prefix"] = distance(begin(CIDR_PREFIXES), it);
            tmpParameters["gateway"] = parameters["gateway"];
            tmpParameters["primarydns"] = parameters["primarydns"];
            tmpParameters["secondarydns"] = parameters["secondarydns"];

            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("SetIPSettings"), tmpParameters, tmpResponse);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["supported"] = true;
                response["success"] = tmpResponse["success"];
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t Network::getIPSettings (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;
            JsonObject tmpParameters;
            size_t index;

            LOGINFOMETHOD();

            if (parameters.HasLabel("ipversion"))
                tmpParameters["ipversion"] = parameters["ipversion"];
            if (parameters.HasLabel("interface"))
            {
                if ("WIFI" == parameters["interface"].String())
                    tmpParameters["interface"] = "wlan0";
                else if("ETHERNET" == parameters["interface"].String())
                    tmpParameters["interface"] = "eth0";
            }

            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("GetIPSettings"), tmpParameters, tmpResponse);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                string ipversion = tmpResponse["ipversion"].String();
                std::transform(ipversion.begin(), ipversion.end(), ipversion.begin(), ::toupper);

                if (parameters.HasLabel("interface"))
                {
                    response["interface"] = parameters["interface"];
                }
                else
                {
                    if ("wlan0" == m_defaultInterface)
                        response["interface"] = "WIFI";
                    else if("eth0" == m_defaultInterface)
                        response["interface"] = "ETHERNET";
                }

                response["ipversion"]    = tmpResponse["ipversion"];
                response["autoconfig"]   = tmpResponse["autoconfig"];
                response["ipaddr"]       = tmpResponse["ipaddress"];
                if(tmpResponse["ipaddress"].String().empty())
                    response["netmask"]  = "";
                else if ("IPV4" == ipversion)
                {
                    index = tmpResponse["prefix"].Number();
                    if(CIDR_NETMASK_IP_LEN <= index)
                        return Core::ERROR_GENERAL;
                    response["netmask"]  = CIDR_PREFIXES[index];
                }
                else if ("IPV6" == ipversion)
                {
                    response["netmask"]  = tmpResponse["prefix"];
                }
                response["gateway"]      = tmpResponse["gateway"];
                response["dhcpserver"]   = tmpResponse["dhcpserver"];
                response["primarydns"]   = tmpResponse["primarydns"];
                response["secondarydns"] = tmpResponse["secondarydns"];
                response["success"]      = tmpResponse["success"];
            }
            LOGTRACEMETHODFIN();
            return rc;
        }
        uint32_t Network::isConnectedToInternet(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;

            LOGINFOMETHOD();
            string ipversion = parameters["ipversion"].String();

            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("IsConnectedToInternet"), parameters, tmpResponse);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["connectedToInternet"] = tmpResponse["isConnectedToInternet"];
                if(ipversion == "IPV4" || ipversion == "IPV6")
                    response["ipversion"] = ipversion.c_str();
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }
        uint32_t Network::getInternetConnectionState(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            uint32_t rc1 = Core::ERROR_GENERAL;
            string endPoint;
            JsonObject tmpResponse;
            JsonObject captivePortalResponse;
            JsonObject tmpParameters;
            string status;

            LOGINFOMETHOD();
            string ipversion = parameters["ipversion"].String();

            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("IsConnectedToInternet"), parameters, tmpResponse);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                status = tmpResponse["status"].String();
                NMLOG_TRACE("status = %s\n", status.c_str() );
                NMLOG_TRACE("tmpResponse[status].String() = %s\n", tmpResponse["status"].String().c_str() );
                if(status == "LIMITED_INTERNET")
                    response["state"] = static_cast<int>(2);
                else if(status == "CAPTIVE_PORTAL")
                {
                    response["state"] = static_cast<int>(1);
                    rc1 = getCaptivePortalURI(tmpParameters, captivePortalResponse);
                    if (Core::ERROR_NONE == rc1)
                        response["uri"] = captivePortalResponse["uri"];
                }
                else if(status == "FULLY_CONNECTED")
                    response["state"] = static_cast<int>(3);
                else
                    response["state"] = static_cast<int>(0);

                if(ipversion == "IPV4" || ipversion == "IPV6")
                    response["ipversion"] = ipversion.c_str();
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }
        uint32_t Network::doPing(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            struct in_addr ipv4address;
            struct in6_addr ipv6address;
            JsonObject tmpParameters;
            JsonObject tmpResponse;
            string endpoint{};

            LOGINFOMETHOD();

            endpoint = parameters["endpoint"].String();

            if (inet_pton(AF_INET, endpoint.c_str(), &ipv4address) > 0)
                tmpParameters["ipversion"] = "IPv4";
            else if (inet_pton(AF_INET6, endpoint.c_str(), &ipv6address) > 0)
                tmpParameters["ipversion"] = "IPv6";
            
            tmpParameters["noOfRequest"] = parameters["packets"];
            tmpParameters["endpoint"] = parameters["endpoint"];
            tmpParameters["timeout"] = 5;
            tmpParameters["guid"] = parameters["guid"];
            
            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(15000, _T("Ping"), tmpParameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["target"] = parameters["endpoint"];
                response["guid"] = parameters["guid"];
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t Network::doTrace(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpParameters;
            tmpParameters["endpoint"]  = parameters["endpoint"].String();
            tmpParameters["packets"]   = parameters["packets"].Number();
            tmpParameters["guid"]      = "";

            if(m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(20000, _T("Trace"), tmpParameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t Network::getPublicIP(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            string interface;
            string ipversion{"IPv4"};
            JsonObject tmpParameters;
            JsonObject tmpResponse;

            LOGINFOMETHOD();
            if("WIFI" == parameters["iface"].String())
                interface = "wlan0";
            else if("ETHERNET" == parameters["iface"].String())
                interface = "eth0";

             if(parameters["ipv6"].Boolean())
                ipversion = "IPv6";
            else
                ipversion = "IPv4";

            tmpParameters["interface"] = interface;
            tmpParameters["ipversion"] = ipversion;

            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("GetPublicIP"), tmpParameters, tmpResponse);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["public_ip"]    = tmpResponse["publicIP"];
                response["success"]      = tmpResponse["success"];
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t Network::isInterfaceEnabled (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpParameters;

            LOGINFOMETHOD();
            string interface = parameters["interface"].String();
            string newInterface = "";

            if("WIFI" == interface)
                newInterface = "wlan0";
            else if("ETHERNET" == interface)
                newInterface = "eth0";

            tmpParameters["interface"] = newInterface;

            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("GetInterfaceState"), tmpParameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;
          
            LOGTRACEMETHODFIN();

            return rc;
        }

        uint32_t Network::setConnectivityTestEndpoints(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("SetConnectivityTestEndpoints"), parameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t Network::startConnectivityMonitoring(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            uint32_t interval = parameters["interval"].Number();

            NMLOG_TRACE("connectivity interval = %d", interval);

            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("StartConnectivityMonitoring"), parameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t Network::getCaptivePortalURI(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string endPoint;
             if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("GetCaptivePortalURI"), parameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t Network::stopConnectivityMonitoring(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;

            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("StopConnectivityMonitoring"), parameters, response);
            else
                rc = Core::ERROR_UNAVAILABLE;

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t Network::getStbIp(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            LOGTRACEMETHODFIN();
            JsonObject tmpResponse;

            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("GetIPSettings"), parameters, tmpResponse);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["ip"]         = tmpResponse["ipaddress"];
                response["success"]    = true;
            }
            return rc;
        }

        uint32_t Network::getSTBIPFamily(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            LOGTRACEMETHODFIN();
            JsonObject tmpResponse;
            JsonObject tmpParameters;

            tmpParameters["ipversion"] = parameters["family"];

            if (m_networkmanager)
                rc = m_networkmanager->Invoke<JsonObject, JsonObject>(5000, _T("GetIPSettings"), tmpParameters, tmpResponse);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["ip"]         = tmpResponse["ipaddress"];
                response["success"]    = true;
            }
            return rc;
        }

        /** Private */
        void Network::subscribeToEvents(void)
        {
            uint32_t errCode = Core::ERROR_GENERAL;
            if (m_networkmanager)
            {
                if (!m_subsIfaceStateChange)
                {
                    errCode = m_networkmanager->Subscribe<JsonObject>(5000, _T("onInterfaceStateChange"), &Network::onInterfaceStateChange);
                    if (Core::ERROR_NONE == errCode)
                        m_subsIfaceStateChange = true;
                    else
                        NMLOG_ERROR ("Subscribe to onInterfaceStateChange failed, errCode: %u", errCode);
                }

                if (!m_subsActIfaceChange)
                {
                    errCode = m_networkmanager->Subscribe<JsonObject>(5000, _T("onActiveInterfaceChange"), &Network::onActiveInterfaceChange);
                    if (Core::ERROR_NONE == errCode)
                        m_subsActIfaceChange = true;
                    else
                    {
                        NMLOG_ERROR("Subscribe to onActiveInterfaceChange failed, errCode: %u", errCode);
                    }
                }

                if (!m_subsIPAddrChange)
                {
                    errCode = m_networkmanager->Subscribe<JsonObject>(5000, _T("onIPAddressChange"), &Network::onIPAddressChange);
                    if (Core::ERROR_NONE == errCode)
                        m_subsIPAddrChange = true;
                    else
                    {
                        NMLOG_ERROR("Subscribe to onIPAddressChange failed, errCode: %u", errCode);
                    }
                }

                if (!m_subsInternetChange)
                {
                    errCode = m_networkmanager->Subscribe<JsonObject>(5000, _T("onInternetStatusChange"), &Network::onInternetStatusChange);
                    if (Core::ERROR_NONE == errCode)
                        m_subsInternetChange = true;
                    else
                    {
                        NMLOG_ERROR("Subscribe to onInternetStatusChange failed, errCode: %u", errCode);
                    }
                }
            }

            if (m_subsIfaceStateChange && m_subsActIfaceChange && m_subsIPAddrChange && m_subsInternetChange)
                m_timer.stop();
        }

        string Network::getInterfaceMapping(const string & interface)
        {
            if(interface == "wlan0")
                return string("WIFI");
            else if(interface == "eth0")
                return string("ETHERNET");
            return string(" ");
        }

        /** Event Handling and Publishing */
        void Network::ReportonInterfaceStateChange(const JsonObject& parameters)
        {
            LOGINFOMETHOD();
            JsonObject legacyParams;
            string json;
            string state = parameters["state"].String();

            legacyParams["interface"] = getInterfaceMapping(parameters["interface"].String()); 

            /* State check */
            if(state == "INTERFACE_ADDED")
                legacyParams["enabled"] = true;
            else if(state == "INTERFACE_REMOVED")
                legacyParams["enabled"] = false;
            else if(state == "INTERFACE_LINK_UP")
                legacyParams["status"] = "CONNECTED";
            else if(state == "INTERFACE_LINK_DOWN")
                legacyParams["status"] = "DISCONNECTED";

            if((state == "INTERFACE_ADDED") || (state == "INTERFACE_REMOVED"))
            {
                Notify("onInterfaceStatusChanged", legacyParams);
            }
            else if((state == "INTERFACE_LINK_UP") || (state == "INTERFACE_LINK_DOWN"))
            {
                Notify("onConnectionStatusChanged", legacyParams);
            }

            return;
        }

        void Network::ReportonActiveInterfaceChange(const JsonObject& parameters)
        {
            LOGINFOMETHOD();
            JsonObject legacyParams;
            
            legacyParams["oldInterfaceName"] = getInterfaceMapping(parameters["oldInterfaceName"].String());
            legacyParams["newInterfaceName"] = getInterfaceMapping(parameters["newInterfaceName"].String());

            m_defaultInterface = parameters["newInterfaceName"].String();
            Notify("onDefaultInterfaceChanged", legacyParams);
            return;
        }

        void Network::ReportonIPAddressChange(const JsonObject& parameters)
        {
            LOGINFOMETHOD();
            JsonObject legacyParams;
            legacyParams["interface"] = getInterfaceMapping(parameters["interface"].String()); 

            if (parameters["isIPv6"].Boolean())
            {
                legacyParams["ip6Address"] = parameters["ipAddress"];
                legacyParams["ip4Address"] = "";
            }
            else
            {
                legacyParams["ip4Address"] = parameters["ipAddress"];
                legacyParams["ip6Address"] = "";
            }

            legacyParams["status"] = parameters["status"];

            Notify("onIPAddressStatusChanged", legacyParams);

            if ("ACQUIRED" == parameters["status"].String())
                m_defaultInterface = parameters["interface"].String();

            return;
        }

        void Network::ReportonInternetStatusChange(const JsonObject& parameters)
        {
            Notify("onInternetStatusChange", parameters);
            return;
        }

        void Network::onInterfaceStateChange(const JsonObject& parameters)
        {
            LOGINFOMETHOD();
            if(_gNWInstance)
                _gNWInstance->ReportonInterfaceStateChange(parameters);

            return;
        }

        void Network::onActiveInterfaceChange(const JsonObject& parameters)
        {
            LOGINFOMETHOD();
            if(_gNWInstance)
                _gNWInstance->ReportonActiveInterfaceChange(parameters);
            return;
        }

        void Network::onIPAddressChange(const JsonObject& parameters)
        {
            LOGINFOMETHOD();
            if(_gNWInstance)
                _gNWInstance->ReportonIPAddressChange(parameters);
            return;
        }

        void Network::onInternetStatusChange(const JsonObject& parameters)
        {
            LOGINFOMETHOD();
            if(_gNWInstance)
                _gNWInstance->ReportonInternetStatusChange(parameters);
        }
    }
}
