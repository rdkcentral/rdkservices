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

#include "NetworkManager.h"

#define LOGINFOMETHOD() { std::string json; parameters.ToString(json); NMLOG_TRACE("params=%s", json.c_str() ); }
#define LOGTRACEMETHODFIN() { std::string json; response.ToString(json); NMLOG_TRACE("response=%s", json.c_str() ); }

using namespace NetworkManagerLogger;

namespace WPEFramework
{
    namespace Plugin
    {
        /**
         * Hook up all our JSON RPC methods
         *
         * Each method definition comprises of:
         *  * Input parameters
         *  * Output parameters
         *  * Method name
         *  * Function that implements that method
         */
        void NetworkManager::RegisterAllMethods()
        {
            Register("SetLogLevel",                       &NetworkManager::SetLogLevel, this);
            Register("GetAvailableInterfaces",            &NetworkManager::GetAvailableInterfaces, this);
            Register("GetPrimaryInterface",               &NetworkManager::GetPrimaryInterface, this);
            Register("SetPrimaryInterface",               &NetworkManager::SetPrimaryInterface, this);
            Register("SetInterfaceState",                 &NetworkManager::SetInterfaceState, this);
            Register("GetInterfaceState",                 &NetworkManager::GetInterfaceState, this);
            Register("GetIPSettings",                     &NetworkManager::GetIPSettings, this);
            Register("SetIPSettings",                     &NetworkManager::SetIPSettings, this);
            Register("GetStunEndpoint",                   &NetworkManager::GetStunEndpoint, this);
            Register("SetStunEndpoint",                   &NetworkManager::SetStunEndpoint, this);
            Register("GetConnectivityTestEndpoints",      &NetworkManager::GetConnectivityTestEndpoints, this);
            Register("SetConnectivityTestEndpoints",      &NetworkManager::SetConnectivityTestEndpoints, this);
            Register("IsConnectedToInternet",             &NetworkManager::IsConnectedToInternet, this);
            Register("GetCaptivePortalURI",               &NetworkManager::GetCaptivePortalURI, this);
            Register("StartConnectivityMonitoring",       &NetworkManager::StartConnectivityMonitoring, this);
            Register("StopConnectivityMonitoring",        &NetworkManager::StopConnectivityMonitoring, this);
            Register("GetPublicIP",                       &NetworkManager::GetPublicIP, this);
            Register("Ping",                              &NetworkManager::Ping, this);
            Register("Trace",                             &NetworkManager::Trace, this);
            Register("StartWiFiScan",                     &NetworkManager::StartWiFiScan, this);
            Register("StopWiFiScan",                      &NetworkManager::StopWiFiScan, this);
            Register("GetKnownSSIDs",                     &NetworkManager::GetKnownSSIDs, this);
            Register("AddToKnownSSIDs",                   &NetworkManager::AddToKnownSSIDs, this);
            Register("RemoveKnownSSID",                   &NetworkManager::RemoveKnownSSID, this);
            Register("WiFiConnect",                       &NetworkManager::WiFiConnect, this);
            Register("WiFiDisconnect",                    &NetworkManager::WiFiDisconnect, this);
            Register("GetConnectedSSID",                  &NetworkManager::GetConnectedSSID, this);
            Register("StartWPS",                          &NetworkManager::StartWPS, this);
            Register("StopWPS",                           &NetworkManager::StopWPS, this);
            Register("GetWifiState",                      &NetworkManager::GetWifiState, this);
            Register("GetWiFiSignalStrength",             &NetworkManager::GetWiFiSignalStrength, this);
            Register("GetSupportedSecurityModes",         &NetworkManager::GetSupportedSecurityModes, this);
        }

        /**
         * Unregister all our JSON-RPC methods
         */
        void NetworkManager::UnregisterAllMethods()
        {
            Unregister("SetLogLevel");
            Unregister("GetAvailableInterfaces");
            Unregister("GetPrimaryInterface");
            Unregister("SetPrimaryInterface");
            Unregister("SetInterfaceState");
            Unregister("GetInterfaceState");
            Unregister("GetIPSettings");
            Unregister("SetIPSettings");
            Unregister("GetStunEndpoint");
            Unregister("SetStunEndpoint");
            Unregister("GetConnectivityTestEndpoints");
            Unregister("SetConnectivityTestEndpoints");
            Unregister("IsConnectedToInternet");
            Unregister("GetCaptivePortalURI");
            Unregister("StartConnectivityMonitoring");
            Unregister("StopConnectivityMonitoring");
            Unregister("GetPublicIP");
            Unregister("Ping");
            Unregister("Trace");
            Unregister("StartWiFiScan");
            Unregister("StopWiFiScan");
            Unregister("GetKnownSSIDs");
            Unregister("AddToKnownSSIDs");
            Unregister("RemoveKnownSSID");
            Unregister("WiFiConnect");
            Unregister("WiFiDisconnect");
            Unregister("GetConnectedSSID");
            Unregister("StartWPS");
            Unregister("StopWPS");
            Unregister("GetWifiState");
            Unregister("GetWiFiSignalStrength");
            Unregister("GetSupportedSecurityModes");
        }

        uint32_t NetworkManager::SetLogLevel (const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            uint32_t rc = Core::ERROR_GENERAL;
            LogLevel level = INFO_LEVEL;
            if (parameters.HasLabel("logLevel"))
            {
                level = static_cast <LogLevel> (parameters["logLevel"].Number());

                NetworkManagerLogger::SetLevel(level);

                const Exchange::INetworkManager::NMLogging log = static_cast <Exchange::INetworkManager::NMLogging> (level);
                if (_NetworkManager)
                    rc = _NetworkManager->SetLogLevel(log);
                else
                    rc = Core::ERROR_UNAVAILABLE;
            }
            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::GetAvailableInterfaces (const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            uint32_t rc = Core::ERROR_GENERAL;
            Exchange::INetworkManager::IInterfaceDetailsIterator* interfaces = NULL;
            if (_NetworkManager)
                rc = _NetworkManager->GetAvailableInterfaces(interfaces);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (interfaces)
            {
                NMLOG_TRACE("received response");
                JsonArray array;
                Exchange::INetworkManager::InterfaceDetails entry{};
                while (interfaces->Next(entry) == true) {
                    JsonObject each;
                    each[_T("type")] = entry.m_type;
                    each[_T("name")] = entry.m_name;
                    each[_T("mac")] = entry.m_mac;
                    each[_T("isEnabled")] = entry.m_isEnabled;
                    each[_T("isConnected")] = entry.m_isConnected;

                    array.Add(JsonValue(each));
                }

                interfaces->Release();
                NMLOG_TRACE("Sending Success");
                response["interfaces"] = array;
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::GetPrimaryInterface (const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string interface;
            if (_NetworkManager)
                rc = _NetworkManager->GetPrimaryInterface(interface);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["interface"] = interface;      
                m_defaultInterface = interface;
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::SetPrimaryInterface (const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string interface = parameters["interface"].String();
            if (_NetworkManager)
                rc = _NetworkManager->SetPrimaryInterface(interface);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            { 
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::SetInterfaceState(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string interface = parameters["interface"].String();
            bool enabled = parameters["enabled"].Boolean();
            if (_NetworkManager)
                rc = _NetworkManager->SetInterfaceState(interface, enabled);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::GetInterfaceState(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            bool isEnabled = false;
            string interface = parameters["interface"].String();

            if (_NetworkManager)
                rc = _NetworkManager->GetInterfaceState(interface, isEnabled);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["enabled"] = isEnabled;
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::GetIPSettings (const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string interface = "";
            string ipversion = "";
            Exchange::INetworkManager::IPAddressInfo result{};

            if (parameters.HasLabel("interface"))
                interface = parameters["interface"].String();
            if (parameters.HasLabel("ipversion"))
                ipversion = parameters["ipversion"].String();
            if (_NetworkManager)
                rc = _NetworkManager->GetIPSettings(interface, ipversion, result);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["interface"] = interface;
                if(result.m_ipAddrType == "IPV6" || result.m_ipAddrType == "IPV4")
                    result.m_ipAddrType[2] = tolower(result.m_ipAddrType[2]);
                response["ipversion"] = result.m_ipAddrType;
                response["autoconfig"]   = result.m_autoConfig;
                response["ipaddress"]    = result.m_ipAddress;
                response["prefix"]       = result.m_prefix;    
                response["gateway"]      = result.m_gateway;
                response["dhcpserver"]   = result.m_dhcpServer;
                if(!result.m_v6LinkLocal.empty())
                    response["v6LinkLocal"] = result.m_v6LinkLocal;
                response["primarydns"]   = result.m_primaryDns; 
                response["secondarydns"] = result.m_secondaryDns;
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::SetIPSettings(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            Exchange::INetworkManager::IPAddressInfo result{};
            string interface = "";
            string ipversion = "";

            if (parameters.HasLabel("interface"))
                interface = parameters["interface"].String();
            if (parameters.HasLabel("ipversion"))
                ipversion = parameters["ipversion"].String();
            result.m_autoConfig = parameters["autoconfig"].Boolean();
            if (!result.m_autoConfig)
            {
                result.m_ipAddress      = parameters["ipaddress"];
                result.m_prefix         = parameters["prefix"].Number();
                result.m_gateway        = parameters["gateway"];
                result.m_primaryDns     = parameters["primarydns"];
                result.m_secondaryDns   = parameters["secondarydns"];
            }

            if (_NetworkManager)
                rc = _NetworkManager->SetIPSettings(interface, ipversion, result);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::GetStunEndpoint(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string endPoint;
            uint32_t port;
            uint32_t bindTimeout;
            uint32_t cacheTimeout;

            if (_NetworkManager)
                rc = _NetworkManager->GetStunEndpoint(endPoint, port, bindTimeout, cacheTimeout);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["endPoint"] = endPoint;
                response["port"] = port;
                response["bindTimeout"] = bindTimeout;
                response["cacheTimeout"] = cacheTimeout;
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::SetStunEndpoint(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string endPoint = parameters["endPoint"].String();
            uint32_t port = parameters["port"].Number();
            uint32_t bindTimeout = parameters["bindTimeout"].Number();
            uint32_t cacheTimeout = parameters["cacheTimeout"].Number();

            if (_NetworkManager)
                rc = _NetworkManager->SetStunEndpoint(endPoint, port, bindTimeout, cacheTimeout);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::GetConnectivityTestEndpoints(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            Exchange::INetworkManager::IStringIterator* endpoints = NULL;
            

            if (_NetworkManager)
                rc = _NetworkManager->GetConnectivityTestEndpoints(endpoints);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                if (endpoints)
                {
                    JsonArray array;
                    string endPoint{};
                    while (endpoints->Next(endPoint) == true)
                    {
                        array.Add(endPoint);
                    }
                    endpoints->Release();
                    response["endpoints"] = array;
                    response["success"] = true;
                }
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::SetConnectivityTestEndpoints(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            ::WPEFramework::RPC::IIteratorType<string, RPC::ID_STRINGITERATOR>* endpointsIter{};
            JsonArray array = parameters["endpoints"].Array();

            if (0 == array.Length() || 5 < array.Length())
            {
                NMLOG_TRACE("minimum of 1 to maximum of 5 Urls are allowed");
                return rc;
            }

            std::vector<std::string> endpoints;
            JsonArray::Iterator index(array.Elements());
            while (index.Next() == true)
            {
                if (Core::JSON::Variant::type::STRING == index.Current().Content())
                {
                    endpoints.push_back(index.Current().String().c_str());
                }
                else
                {
                    NMLOG_TRACE("Unexpected variant type");
                    return rc;
                }
            }
            endpointsIter = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(endpoints));

            if (_NetworkManager)
                rc = _NetworkManager->SetConnectivityTestEndpoints(endpointsIter);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }

            if (endpointsIter)
                endpointsIter->Release();

            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::IsConnectedToInternet(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string ipversion = parameters["ipversion"].String();
            Exchange::INetworkManager::InternetStatus result;
            

            if (_NetworkManager)
                rc = _NetworkManager->IsConnectedToInternet(ipversion, result);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["isConnectedToInternet"] = (Exchange::INetworkManager::InternetStatus::INTERNET_FULLY_CONNECTED == result);
                response["internetState"] = static_cast <int> (result);
                switch (result)
                {
                case Exchange::INetworkManager::InternetStatus::INTERNET_LIMITED:
                    response["status"] = string("LIMITED_INTERNET");
                    break;
                case Exchange::INetworkManager::InternetStatus::INTERNET_CAPTIVE_PORTAL:
                    response["status"] = string("CAPTIVE_PORTAL");
                    break;
                case Exchange::INetworkManager::InternetStatus::INTERNET_FULLY_CONNECTED:
                    response["status"] = string("FULLY_CONNECTED");
                    // when fully connected to internet set Network subsystem
                    //setInternetSubsystem();
                    break;
                default:
                    response["status"] = string("NO_INTERNET");
                    break;
                }

                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::GetCaptivePortalURI(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string endPoint;
            if (_NetworkManager)
                rc = _NetworkManager->GetCaptivePortalURI(endPoint);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["uri"] = endPoint;
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::StartConnectivityMonitoring(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            uint32_t interval = parameters["interval"].Number();

            NMLOG_TRACE("connectivity interval = %d", interval);
            if (_NetworkManager)
                rc = _NetworkManager->StartConnectivityMonitoring(interval);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::StopConnectivityMonitoring(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;

            if (_NetworkManager)
                rc = _NetworkManager->StopConnectivityMonitoring();
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::GetPublicIP(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string ipAddress{};
            string ipversion = "IPv4";
            if (parameters.HasLabel("ipversion"))
                ipversion = parameters["ipversion"].String();

            if ((!m_publicIPAddress.empty()) && (m_publicIPAddressType == ipversion))
            {
                rc = Core::ERROR_NONE;
                ipAddress = m_publicIPAddress;
            }
            else
            {
                if (_NetworkManager)
                    rc = _NetworkManager->GetPublicIP(ipversion, ipAddress);
                else
                    rc = Core::ERROR_UNAVAILABLE;
            }

            if (Core::ERROR_NONE == rc)
            {
                response["publicIP"] = ipAddress;
                response["ipversion"] = ipversion;
                response["success"] = true;

                m_publicIPAddress = ipAddress;
                m_publicIPAddressType = ipversion;
                PublishToThunderAboutInternet();
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        void NetworkManager::PublishToThunderAboutInternet()
        {
            NMLOG_TRACE("No public IP persisted yet; Update the data");
            if (m_publicIPAddress.empty())
            {
                JsonObject input, output;
                GetPublicIP(input, output);
            }

            if (!m_publicIPAddress.empty())
            {
                PluginHost::ISubSystem* subSystem = _service->SubSystems();

                if (subSystem != nullptr)
                {
                    const PluginHost::ISubSystem::IInternet* internet(subSystem->Get<PluginHost::ISubSystem::IInternet>());
                    if (nullptr == internet)
                    {
                        subSystem->Set(PluginHost::ISubSystem::INTERNET, this);
                        NMLOG_INFO("Set INTERNET ISubSystem");
                    }

                    subSystem->Release();
                }
            }
        }

        uint32_t NetworkManager::Ping(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string result{};
            uint32_t rc = Core::ERROR_GENERAL;
            if (parameters.HasLabel("endpoint"))
            {
                string endpoint{};
                string guid{};
                string ipversion{"IPv4"};
                uint32_t noOfRequest = 3;
                uint16_t timeOutInSeconds = 5;

                endpoint = parameters["endpoint"].String();

                if (parameters.HasLabel("ipversion"))
                    ipversion = parameters["ipversion"].String();

                if (parameters.HasLabel("noOfRequest"))
                    noOfRequest  = parameters["noOfRequest"].Number();

                if (parameters.HasLabel("timeout"))
                    timeOutInSeconds  = parameters["timeout"].Number();

                if (parameters.HasLabel("guid"))
                    guid = parameters["guid"].String();

                if (_NetworkManager)
                    rc = _NetworkManager->Ping(ipversion, endpoint, noOfRequest, timeOutInSeconds, guid, result);
                else
                    rc = Core::ERROR_UNAVAILABLE;
            }

            if (Core::ERROR_NONE == rc)
            {
                JsonObject reply;
                reply.FromString(result);
                response = reply;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::Trace(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string result{};
            const string ipversion      = parameters["ipversion"].String();
            const string endpoint       = parameters["endpoint"].String();
            const uint32_t noOfRequest  = parameters["packets"].Number();
            const string guid           = parameters["guid"].String();

            if (_NetworkManager)
                rc = _NetworkManager->Trace(ipversion, endpoint, noOfRequest, guid, result);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                JsonObject reply;
                reply.FromString(result);
                reply["success"] = true;
                response = reply;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::StartWiFiScan(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            const Exchange::INetworkManager::WiFiFrequency frequency = static_cast <Exchange::INetworkManager::WiFiFrequency> (parameters["frequency"].Number());

            if (_NetworkManager)
                rc = _NetworkManager->StartWiFiScan(frequency);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::StopWiFiScan(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;

            if (_NetworkManager)
                rc = _NetworkManager->StopWiFiScan();
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::GetKnownSSIDs(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;

            JsonArray ssids;
            ::WPEFramework::RPC::IIteratorType<string, RPC::ID_STRINGITERATOR>* _ssids{};

            if (_NetworkManager)
                rc = _NetworkManager->GetKnownSSIDs(_ssids);

            if (Core::ERROR_NONE == rc)
            {
                ASSERT(_ssids != nullptr);

                if (_ssids != nullptr)
                {
                    string _resultItem_{};
                    while (_ssids->Next(_resultItem_) == true)
                    {
                        ssids.Add() = _resultItem_;
                    }
                    _ssids->Release();
                }
                response["ssids"] = ssids;
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::AddToKnownSSIDs(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            Exchange::INetworkManager::WiFiConnectTo ssid{};

            if (parameters.HasLabel("ssid") && parameters.HasLabel("passphrase"))
            {
                ssid.m_ssid            = parameters["ssid"].String();
                ssid.m_passphrase      = parameters["passphrase"].String();
                ssid.m_securityMode    = static_cast <Exchange::INetworkManager::WIFISecurityMode> (parameters["securityMode"].Number());

                if (_NetworkManager)
                    rc = _NetworkManager->AddToKnownSSIDs(ssid);
                else
                    rc = Core::ERROR_UNAVAILABLE;
            }

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::RemoveKnownSSID(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string ssid{};

            if (parameters.HasLabel("ssid"))
            {
                ssid = parameters["ssid"].String();
                if (_NetworkManager)
                    rc = _NetworkManager->RemoveKnownSSID(ssid);
                else
                    rc = Core::ERROR_UNAVAILABLE;
            }

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::WiFiConnect(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            Exchange::INetworkManager::WiFiConnectTo ssid{};

            if (parameters.HasLabel("ssid"))
                ssid.m_ssid            = parameters["ssid"].String();
            if (parameters.HasLabel("passphrase"))
                ssid.m_passphrase      = parameters["passphrase"].String();
            if (parameters.HasLabel("securityMode"))
                ssid.m_securityMode    = static_cast <Exchange::INetworkManager::WIFISecurityMode> (parameters["securityMode"].Number());
            //TODO Check Security modes
            if (parameters.HasLabel("identity"))
            ssid.m_identity              = parameters["identity"].String();
            if (parameters.HasLabel("caCert"))
            ssid.m_caCert                = parameters["caCert"].String();
            if (parameters.HasLabel("clientCert"))
            ssid.m_clientCert            = parameters["clientCert"].String();
            if (parameters.HasLabel("privateKey"))
            ssid.m_privateKey            = parameters["privateKey"].String();
            if (parameters.HasLabel("privateKeyPasswd"))
            ssid.m_privateKeyPasswd      = parameters["privateKeyPasswd"].String();

            if (_NetworkManager)
                rc = _NetworkManager->WiFiConnect(ssid);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::WiFiDisconnect(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;

            if (_NetworkManager)
                rc = _NetworkManager->WiFiDisconnect();
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::GetConnectedSSID(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            Exchange::INetworkManager::WiFiSSIDInfo ssidInfo{};

            if (_NetworkManager)
                rc = _NetworkManager->GetConnectedSSID(ssidInfo);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["ssid"] = ssidInfo.m_ssid;
                response["bssid"] = ssidInfo.m_bssid;
                response["securityMode"] = static_cast <int> (ssidInfo.m_securityMode);
                response["signalStrength"] = ssidInfo.m_signalStrength;
                response["frequency"] = static_cast <int> (ssidInfo.m_frequency);
                response["rate"] = ssidInfo.m_rate;
                response["noise"] = ssidInfo.m_noise;
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::StartWPS(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string wps_pin{};
            Exchange::INetworkManager::WiFiWPS method;

            if (parameters.HasLabel("method"))
            {
                method = static_cast <Exchange::INetworkManager::WiFiWPS> (parameters["method"].Number());
            }
            else
                method = Exchange::INetworkManager::WIFI_WPS_PBC;

            if ((Exchange::INetworkManager::WIFI_WPS_PIN == method) && parameters.HasLabel("wps_pin"))
            {
                wps_pin = parameters["wps_pin"].String();
            }

            if (_NetworkManager)
                rc = _NetworkManager->StartWPS(method, wps_pin);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::StopWPS(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;

            if (_NetworkManager)
                rc = _NetworkManager->StopWPS();
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::GetWifiState(const JsonObject& parameters, JsonObject& response)
        {
            Exchange::INetworkManager::WiFiState state;
            uint32_t rc = Core::ERROR_GENERAL;

            LOGINFOMETHOD();
            if (_NetworkManager)
                rc = _NetworkManager->GetWifiState(state);
            else
                rc = Core::ERROR_UNAVAILABLE;
            if (Core::ERROR_NONE == rc)
            {
                response["state"] = static_cast <int> (state);
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::GetWiFiSignalStrength(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            string ssid{};
            string signalStrength{};
            Exchange::INetworkManager::WiFiSignalQuality quality;

            if (_NetworkManager)
                rc = _NetworkManager->GetWiFiSignalStrength(ssid, signalStrength, quality);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["ssid"] = ssid;
                response["signalStrength"] = signalStrength;
                response["quality"] = static_cast <int> (quality);
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }

        uint32_t NetworkManager::GetSupportedSecurityModes(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            uint32_t rc = Core::ERROR_GENERAL;
            Exchange::INetworkManager::ISecurityModeIterator* securityModes{};

            if (_NetworkManager)
                rc = _NetworkManager->GetSupportedSecurityModes(securityModes);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                if (securityModes != nullptr)
                {
                    JsonObject modes{};
                    Exchange::INetworkManager::WIFISecurityModeInfo _resultItem_{};
                    while (securityModes->Next(_resultItem_) == true)
                    {
                        response.Set(_resultItem_.m_securityModeText.c_str(), JsonValue(_resultItem_.m_securityMode));
                    }
                    securityModes->Release();
                }
                response["success"] = true;
            }
            LOGTRACEMETHODFIN();
            return rc;
        }
    }
}
