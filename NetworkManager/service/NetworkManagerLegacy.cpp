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
        void NetworkManager::RegisterLegacyMethods()
        {
            Register("getInterfaces",                     &NetworkManager::getInterfaces, this);
            Register("isInterfaceEnabled",                &NetworkManager::isInterfaceEnabled, this);
            Register("getStbIp",                          &NetworkManager::getStbIp, this);
            Register("getSTBIPFamily",                    &NetworkManager::getSTBIPFamily, this);
            Register("getPublicIP",                       &NetworkManager::getPublicIP, this);
            Register("setInterfaceEnabled",               &NetworkManager::setInterfaceEnabled, this);
            Register("getDefaultInterface",               &NetworkManager::getDefaultInterface, this);
            Register("setDefaultInterface",               &NetworkManager::setDefaultInterface, this);
            Register("setIPSettings",                     &NetworkManager::setIPSettings, this);
            Register("getIPSettings",                     &NetworkManager::getIPSettings, this);
            Register("getInternetConnectionState",        &NetworkManager::getInternetConnectionState, this);
            Register("ping",                              &NetworkManager::ping, this);
            Register("isConnectedToInternet",             &NetworkManager::isConnectedToInternet, this);
            Register("setConnectivityTestEndpoints",      &NetworkManager::SetConnectivityTestEndpoints, this);
            Register("startConnectivityMonitoring",       &NetworkManager::StartConnectivityMonitoring, this);
            Register("getCaptivePortalURI",               &NetworkManager::getCaptivePortalURI, this);
            Register("stopConnectivityMonitoring",        &NetworkManager::StopConnectivityMonitoring, this);
            Register("cancelWPSPairing",                  &NetworkManager::cancelWPSPairing, this);
            Register("clearSSID",                         &NetworkManager::clearSSID, this);
            Register("connect",                           &NetworkManager::WiFiConnect, this);
            Register("disconnect",                        &NetworkManager::disconnect, this);
            Register("getConnectedSSID",                  &NetworkManager::getConnectedSSID, this);
            Register("getConnectedSSID",                  &NetworkManager::getConnectedSSID, this);
            Register("startScan",                         &NetworkManager::StartWiFiScan, this);
            Register("stopScan",                          &NetworkManager::StopWiFiScan, this);
            Register("getPairedSSID",                     &NetworkManager::GetKnownSSIDs, this);
            Register("getPairedSSIDInfo",                 &NetworkManager::getPairedSSIDInfo, this);
            Register("initiateWPSPairing",                &NetworkManager::StartWPS, this);
            Register("isPaired",                          &NetworkManager::isPaired, this);
            Register("saveSSID",                          &NetworkManager::AddToKnownSSIDs, this);
            Register("setEnabled",                        &NetworkManager::setEnabled, this);
            Register("getSupportedSecurityModes",         &NetworkManager::GetSupportedSecurityModes, this);
        }

        /**
         * Unregister all our JSON-RPC methods
         */
        void NetworkManager::UnregisterLegacyMethods()
        {
            Unregister("cancelWPSPairing");
            Unregister("clearSSID");
            Unregister("connect");
            Unregister("disconnect");
            Unregister("getConnectedSSID");
            Unregister("getSupportedSecurityModes");
            Unregister("startScan");
            Unregister("stopScan");
            Unregister("getInterfaces");
            Unregister("isInterfaceEnabled");
            Unregister("getStbIp");
            Unregister("getSTBIPFamily");
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
            Unregister("getPairedSSID");
            Unregister("getPairedSSIDInfo");
            Unregister("initiateWPSPairing");
            Unregister("isPaired");
            Unregister("saveSSID");
            Unregister("setEnabled");
        }

        uint32_t NetworkManager::getInterfaces (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;
            JsonArray array;

            rc = GetAvailableInterfaces(parameters, tmpResponse);
           
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

            response["Interfaces"] = array;
            response["success"] = tmpResponse["success"];
            return rc;
        }
        uint32_t NetworkManager::setInterfaceEnabled (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            string interface;
            JsonObject tmpResponse;
            JsonObject tmpParameters;
            
            if("WIFI" == parameters["interface"].String())
                interface = "wlan0";
            else if("ETHERNET" == parameters["interface"].String())
                interface = "eth0";
            
            tmpParameters["interface"] = interface;
            tmpParameters["enabled"] = parameters["enabled"];
            rc = SetInterfaceEnabled(tmpParameters, tmpResponse);
            if (Core::ERROR_NONE == rc)
            {
                response["success"] = tmpResponse["success"];
            }

            return rc;
        }
        uint32_t NetworkManager::getDefaultInterface (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;

            rc = GetPrimaryInterface(parameters, tmpResponse);

            if (Core::ERROR_NONE == rc)
            {
                if ("wlan0" == tmpResponse["interface"].String())
                    response["interface"] = "WIFI";
                else if("eth0" == tmpResponse["interface"].String())     
                    response["interface"] = "ETHERNET";
                response["success"] = tmpResponse["success"];
            }
            return rc;
        }
        uint32_t NetworkManager::setDefaultInterface(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;
            JsonObject tmpParameters;
            string interface;
            if("WIFI" == parameters["interface"].String())
                tmpParameters["interface"] = "wlan0";
            else if("ETHERNET" == parameters["interface"].String())
                tmpParameters["interface"] = "eth0";
            
            rc = SetPrimaryInterface(tmpParameters, tmpResponse);

            if (Core::ERROR_NONE == rc)
            { 
                response["success"] = tmpResponse["success"];
            }
            return rc;
        }
        uint32_t NetworkManager::setIPSettings(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;
            JsonObject tmpParameters;
            Exchange::INetworkManager::IPAddressInfo result{};
            
            if("WIFI" == parameters["interface"].String())
                tmpParameters["interface"] = "wlan0";
            else if("ETHERNET" == parameters["interface"].String())
                tmpParameters["interface"] = "eth0";
            
            tmpParameters["ipversion"] = parameters["ipversion"];
            tmpParameters["autoconfig"] = parameters["autoconfig"];

            rc = SetIPSettings(tmpParameters, tmpResponse);

            if (Core::ERROR_NONE == rc)
            {
                response["supported"] = true;
                response["success"] = tmpResponse["success"];
            }
            return rc;
        }
        uint32_t NetworkManager::getIPSettings (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            string interface;
            JsonObject tmpResponse;
            JsonObject tmpParameters;

            tmpParameters["ipversion"] = parameters["ipversion"];
            if ("WIFI" == parameters["interface"].String())
                tmpParameters["interface"] = "wlan0";
            else if("ETHERNET" == parameters["interface"].String())
                tmpParameters["interface"] = "eth0";

            rc = GetIPSettings(tmpParameters, tmpResponse); 

            if (Core::ERROR_NONE == rc)
            {
                response["interface"]    = tmpParameters["interface"];
                response["ipversion"]    = tmpResponse["ipversion"];
                response["autoconfig"]   = tmpResponse["autoconfig"];
                response["dhcpserver"]   = tmpResponse["dhcpserver"];
                response["ipaddr"]       = tmpResponse["ipaddress"];
                response["netmask"]      = tmpResponse["prefix"];
                response["gateway"]      = tmpResponse["gateway"];
                response["primarydns"]   = tmpResponse["primarydns"];
                response["secondarydns"] = tmpResponse["secondarydns"];
                response["success"]      = tmpResponse["success"];
            }
            return rc;
        }
        uint32_t NetworkManager::isConnectedToInternet(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;

            string ipversion = parameters["ipversion"].String();
            rc = IsConnectedToInternet(parameters, tmpResponse);

            if (Core::ERROR_NONE == rc)
            {
                response["connectedToInternet"] = tmpResponse["isConnectedToInternet"];
                response["ipversion"] = ipversion.c_str();
                response["success"] = true;
            }
            return rc;
        }
        uint32_t NetworkManager::getInternetConnectionState(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            uint32_t rc1 = Core::ERROR_GENERAL;
            string endPoint;
            JsonObject tmpResponse;
            JsonObject tmpCaptivePortalResponse;
            JsonObject tmpParameters;
            string ipversion = parameters["ipversion"].String();
            rc = IsConnectedToInternet(parameters, tmpResponse);
            if (Core::ERROR_NONE == rc)
            {
                if (tmpResponse["status"].String() == "LIMITED_INTERNET")
                    response["state"] = 1;
                else if (tmpResponse["status"].String() == "CAPTIVE_PORTAL")
                {
                    response["state"] = 2;
                    rc1 = getCaptivePortalURI(tmpParameters, tmpCaptivePortalResponse);

                    if (Core::ERROR_NONE == rc1)
                        response["URI"] = tmpCaptivePortalResponse["uri"];
                }
                else if (tmpResponse["status"].String() == "FULLY_CONNECTED")
                    response["state"] = 3;
                else
                    response["state"] = 0;
                response["ipversion"] = ipversion.c_str();
                response["success"] = true;
            }
            return rc;
        }
        uint32_t NetworkManager::ping(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            struct in_addr ipv4address;
            struct in6_addr ipv6address;
            JsonObject tmpParameters;
            JsonObject tmpResponse;
            string endpoint{};

            endpoint = parameters["endpoint"].String();

            if (inet_pton(AF_INET, endpoint.c_str(), &ipv4address) > 0)
                tmpParameters["ipversion"] = "IPv4";
            else if (inet_pton(AF_INET6, endpoint.c_str(), &ipv6address) > 0)
                tmpParameters["ipversion"] = "IPv6";
            
            tmpParameters["noOfRequest"] = parameters["packets"];
            tmpParameters["endpoint"] = parameters["endpoint"];
            tmpParameters["timeout"] = 5;
            tmpParameters["guid"] = parameters["guid"];
            
            rc = Ping(tmpParameters, response);

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            return rc;
        }
        uint32_t NetworkManager::getCaptivePortalURI(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;
            
            rc = GetCaptivePortalURI(parameters, tmpResponse);

            if (Core::ERROR_NONE == rc)
            {
                response["URI"] = tmpResponse["uri"];
                response["success"] = tmpResponse["success"];
            }
            return rc;
        }
        uint32_t NetworkManager::getStbIp(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            string ipAddress{};
            JsonObject tmpParameters;
            JsonObject tmpResponse;

            tmpParameters["ipversion"] = "IPv4";
            rc = GetPublicIP(tmpParameters, tmpResponse);
            
            if (Core::ERROR_NONE == rc)
            {
                response["ip"] = tmpResponse["publicIP"];
                response["success"] = tmpResponse["success"];
            }
            return rc;
        }
        uint32_t NetworkManager::getSTBIPFamily(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            string ipversion{"IPv4"};
            string ipAddress{};
            JsonObject tmpParameters;
            JsonObject tmpResponse;

            if("AF_INET" == parameters["family"].String())
                ipversion = "IPv4";
            else if("AF_INET6" == parameters["family"].String())
                ipversion = "IPv6";
            
            tmpParameters["ipversion"] = ipversion;            
            rc = GetPublicIP(tmpParameters, tmpResponse);

            if (Core::ERROR_NONE == rc)
            {
                response["ip"] = tmpResponse["publicIP"];;
                response["success"] = tmpResponse["success"];
            }
            return rc;
        }
        uint32_t NetworkManager::getPublicIP(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            string interface;
            string ipversion{"IPv4"};
            JsonObject tmpParameters;
            JsonObject tmpResponse;

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
            rc = GetIPSettings(tmpParameters, tmpResponse);

            if (Core::ERROR_NONE == rc)
            {
                response["public_ip"]    = tmpResponse["ipaddress"];
                response["success"]      = tmpResponse["success"];
            }
            return rc;
        }

        uint32_t NetworkManager::isInterfaceEnabled (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;

            rc = GetAvailableInterfaces(parameters, tmpResponse);
          
            const JsonArray& tmpArray = tmpResponse["interfaces"].Array();
            for (int i=0; i<tmpArray.Length(); i++)
            {
                JsonObject each;
                const JsonObject& arrayEntry = tmpArray[i].Object();
                if(parameters["interface"].String() == arrayEntry["type"].String())
                {
                    response["enabled"] = arrayEntry["isEnabled"];
                    break;
                }
            }
            response["success"] = tmpResponse["success"];

            return rc;
        }
        uint32_t NetworkManager::cancelWPSPairing (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            uint32_t result = 1;

            rc = StopWPS(parameters, response);

            if (Core::ERROR_NONE == rc)
            {
                result = 0;
            }
            response["result"] = result;
            return rc;
        }
        uint32_t NetworkManager::clearSSID (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            uint32_t result = 1;
            JsonObject tmpParameters;

            tmpParameters["ssid"] = "";
            rc = RemoveKnownSSID(tmpParameters, response);

            if (Core::ERROR_NONE == rc)
            {
                result = 0;
            }
            response["result"] = result;
            return rc;
        }
        uint32_t NetworkManager::saveSSID (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            uint32_t result = 1;
            JsonObject tmpParameters;

            rc = AddToKnownSSIDs(tmpParameters, response);

            if (Core::ERROR_NONE == rc)
            {
                result = 0;
            }
            response["result"] = result;
            return rc;
        }
 
        uint32_t NetworkManager::disconnect (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            uint32_t result = 1;

            rc = WiFiDisconnect(parameters, response);

            if (Core::ERROR_NONE == rc)
            {
                result = 0;
            }
            response["result"] = result;
            return rc;
        }
        uint32_t NetworkManager::getConnectedSSID (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;

            rc = GetConnectedSSID(parameters, tmpResponse);

            if (Core::ERROR_NONE == rc)
            {
                response["ssid"] = tmpResponse["ssid"];
                response["bssid"] = tmpResponse["bssid"];
                response["rate"] = tmpResponse["rate"];
                response["noise"] = tmpResponse["noise"];
                response["security"] = tmpResponse["securityMode"];
                response["signalStrength"] = tmpResponse["signalStrength"];
                response["frequency"] = tmpResponse["frequency"];
                response["success"] = tmpResponse["success"];
            }
            return rc;
        }
        
        uint32_t NetworkManager::getPairedSSIDInfo (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            JsonObject tmpResponse;

            rc = GetConnectedSSID(parameters, tmpResponse);

            if (Core::ERROR_NONE == rc)
            {
                response["ssid"] = tmpResponse["ssid"];
                response["bssid"] = tmpResponse["bssid"];
                response["success"] = tmpResponse["success"];
            }
            return rc;
        }
        uint32_t NetworkManager::isPaired (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;

            rc = GetKnownSSIDs(parameters, response);

            if (Core::ERROR_NONE == rc)
            {
                if (response["ssids"].String().empty())
                {
                    response["result"] = 1;
                }
                else
                {
                    response["result"] = 0;
                }
                response["success"] = true;
            }
            return rc;
        }
        uint32_t NetworkManager::setEnabled (const JsonObject& parameters, JsonObject& response)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            string interface = "wlan0";
            bool isEnabled = parameters["enable"].Boolean();

            if (_NetworkManager)
                rc = _NetworkManager->SetInterfaceEnabled(interface, isEnabled);
            else
                rc = Core::ERROR_UNAVAILABLE;

            if (Core::ERROR_NONE == rc)
            {
                response["success"] = true;
            }
            return rc;
        }
    }
}
