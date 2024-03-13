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

#include "NetworkManagerImplementation.h"
#include "NetworkConnectivity.h"
#include "WifiSignalStrengthMonitor.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace NetworkManagerLogger;

#define CIDR_NETMASK_IP_LEN 32

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(NetworkManagerImplementation, NETWORKMANAGER_MAJOR_VERSION, NETWORKMANAGER_MINOR_VERSION, NETWORKMANAGER_PATCH_VERSION);

        NetworkManagerImplementation::NetworkManagerImplementation()
            : _notificationCallbacks({})
        {
            /* Initialize Network Manager */
            NetworkManagerLogger::Init();

            LOG_ENTRY_FUNCTION();
            /* Name says it all */
            platform_init();

            /* Initialize STUN Endpoints */
            m_stunEndPoint = "stun.l.google.com";
            m_stunPort = 19302;
            m_stunBindTimeout = 30;
            m_stunCacheTimeout = 0;
            m_defaultInterface = "";
            m_publicIP = "";
        }

        NetworkManagerImplementation::~NetworkManagerImplementation()
        {
            connectivityMonitor.stopContinuousConnectivityMonitoring();
            LOG_ENTRY_FUNCTION();
        }

        /**
         * Register a notification callback
         */
        uint32_t NetworkManagerImplementation::Register(INetworkManager::INotification *notification)
        {
            LOG_ENTRY_FUNCTION();
            _notificationLock.Lock();

            // Make sure we can't register the same notification callback multiple times
            if (std::find(_notificationCallbacks.begin(), _notificationCallbacks.end(), notification) == _notificationCallbacks.end()) {
                _notificationCallbacks.push_back(notification);
                notification->AddRef();
            }

            _notificationLock.Unlock();

            return Core::ERROR_NONE;
        }

        /**
         * Unregister a notification callback
         */
        uint32_t NetworkManagerImplementation::Unregister(INetworkManager::INotification *notification)
        {
            LOG_ENTRY_FUNCTION();
            _notificationLock.Lock();

            // Make sure we can't register the same notification callback multiple times
            auto itr = std::find(_notificationCallbacks.begin(), _notificationCallbacks.end(), notification);
            if (itr != _notificationCallbacks.end()) {
                (*itr)->Release();
                _notificationCallbacks.erase(itr);
            }

            _notificationLock.Unlock();

            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::Configure(const string& configLine /* @in */, NMLogging& logLevel /* @out */)
        {
            if(configLine.empty())
            {
                NMLOG_FATAL("config line : is empty !");
                return(Core::ERROR_GENERAL);
            }

            NMLOG_TRACE("config line : %s", configLine.c_str());

            Config config;
            if(config.FromString(configLine))
            {
                /* stun configuration copy */
                m_stunEndPoint = config.stun.stunEndpoint.Value();
                m_stunPort = config.stun.port.Value();
                m_stunBindTimeout = config.stun.interval.Value();

                NMLOG_TRACE("config : stun endpoint %s", m_stunEndPoint.c_str());
                NMLOG_TRACE("config : stun port %d", m_stunPort);
                NMLOG_TRACE("config : stun interval %d", m_stunBindTimeout);

                NMLOG_TRACE("config : endpoint 1 %s", config.connectivity.endpoint_1.Value().c_str());
                NMLOG_TRACE("config : endpoint 2 %s", config.connectivity.endpoint_2.Value().c_str());
                NMLOG_TRACE("config : endpoint 3 %s", config.connectivity.endpoint_3.Value().c_str());
                NMLOG_TRACE("config : endpoint 4 %s", config.connectivity.endpoint_4.Value().c_str());
                NMLOG_TRACE("config : endpoint 5 %s", config.connectivity.endpoint_5.Value().c_str());
                NMLOG_TRACE("config : interval %d", config.connectivity.ConnectivityCheckInterval.Value());

                NMLOG_TRACE("config : loglevel %d", config.loglevel.Value());
                logLevel = static_cast <NMLogging>(config.loglevel.Value());
                // configure loglevel in libWPEFrameworkNetworkManagerImplementation.so
                NetworkManagerLogger::SetLevel(static_cast <NetworkManagerLogger::LogLevel>(logLevel));

                std::vector<std::string> endpoints;
                endpoints.push_back(config.connectivity.endpoint_1.Value().c_str());
                endpoints.push_back(config.connectivity.endpoint_2.Value().c_str());
                endpoints.push_back(config.connectivity.endpoint_3.Value().c_str());
                endpoints.push_back(config.connectivity.endpoint_4.Value().c_str());
                endpoints.push_back(config.connectivity.endpoint_5.Value().c_str());

                //set connectivity endpoint
                connectivityMonitor.setConnectivityMonitorEndpoints(endpoints);
            }
            else
                NMLOG_ERROR("Plugin configuration read error !");

            return(Core::ERROR_NONE);
        }

        /* @brief Get STUN Endpoint to be used for identifying Public IP */
        uint32_t NetworkManagerImplementation::GetStunEndpoint (string &endPoint /* @out */, uint32_t& port /* @out */, uint32_t& bindTimeout /* @out */, uint32_t& cacheTimeout /* @out */) const
        {
            LOG_ENTRY_FUNCTION();
            endPoint = m_stunEndPoint;
            port = m_stunPort;
            bindTimeout = m_stunBindTimeout;
            cacheTimeout = m_stunCacheTimeout;
            return Core::ERROR_NONE;
        }

        /* @brief Set STUN Endpoint to be used to identify Public IP */
        uint32_t NetworkManagerImplementation::SetStunEndpoint (string const endPoint /* @in */, const uint32_t port /* @in */, const uint32_t bindTimeout /* @in */, const uint32_t cacheTimeout /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            if (!endPoint.empty())
                m_stunEndPoint = endPoint;
            if (port != 0)
                m_stunPort = port;

            m_stunBindTimeout = bindTimeout;
            m_stunCacheTimeout = cacheTimeout;
            return Core::ERROR_NONE;
        }

        /* @brief Get ConnectivityTest Endpoints */
        uint32_t NetworkManagerImplementation::GetConnectivityTestEndpoints(IStringIterator*& endPoints/* @out */) const
        {
            LOG_ENTRY_FUNCTION();
            std::vector<std::string> tmpEndPoints = connectivityMonitor.getConnectivityMonitorEndpoints();
            endPoints = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(tmpEndPoints));

            return Core::ERROR_NONE;
        }

        /* @brief Set ConnectivityTest Endpoints */
        uint32_t NetworkManagerImplementation::SetConnectivityTestEndpoints(IStringIterator* const endPoints /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            std::vector<std::string> tmpEndPoints;
            if(endPoints)
            {
                string endPoint{};
                while (endPoints->Next(endPoint) == true)
                {
                    tmpEndPoints.push_back(endPoint);
                }
                connectivityMonitor.setConnectivityMonitorEndpoints(tmpEndPoints);
            }
            return Core::ERROR_NONE;
        }

        /* @brief Get Internet Connectivty Status */ 
        uint32_t NetworkManagerImplementation::IsConnectedToInternet(const string &ipversion /* @in */, InternetStatus &result /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            nsm_internetState isconnected;
            nsm_ipversion tmpVersion = NSM_IPRESOLVE_WHATEVER;
            if(0 == strcasecmp("IPv4", ipversion.c_str()))
                tmpVersion = NSM_IPRESOLVE_V4;
            else if(0 == strcasecmp("IPv6", ipversion.c_str()))
                tmpVersion = NSM_IPRESOLVE_V6;

            isconnected = connectivityMonitor.getInternetConnectionState(tmpVersion);
            if (FULLY_CONNECTED == isconnected)
                result = INTERNET_FULLY_CONNECTED;
            else if (CAPTIVE_PORTAL == isconnected)
                result = INTERNET_CAPTIVE_PORTAL;
            else if (LIMITED_INTERNET == isconnected)
                result = INTERNET_LIMITED;
            else
                result = INTERNET_NOT_AVAILABLE;

            return Core::ERROR_NONE;
        }

        /* @brief Get Authentication URL if the device is behind Captive Portal */ 
        uint32_t NetworkManagerImplementation::GetCaptivePortalURI(string &endPoints/* @out */) const
        {
            LOG_ENTRY_FUNCTION();
            endPoints = connectivityMonitor.getCaptivePortalURI();
            return Core::ERROR_NONE;
        }

        /* @brief Start The Internet Connectivity Monitoring */ 
        uint32_t NetworkManagerImplementation::StartConnectivityMonitoring(const uint32_t interval/* @in */)
        {
            LOG_ENTRY_FUNCTION();
            if (connectivityMonitor.doContinuousConnectivityMonitoring(interval))
                return Core::ERROR_NONE;
            else
                return Core::ERROR_GENERAL;
        }

        /* @brief Stop The Internet Connectivity Monitoring */ 
        uint32_t NetworkManagerImplementation::StopConnectivityMonitoring(void) const
        {
            LOG_ENTRY_FUNCTION();
            if (connectivityMonitor.stopContinuousConnectivityMonitoring())
                return Core::ERROR_NONE;
            else
                return Core::ERROR_GENERAL;
        }

        /* @brief Get the Public IP used for external world communication */
        uint32_t NetworkManagerImplementation::GetPublicIP (const string &ipversion /* @in */,  string& ipAddress /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            stun::bind_result result;
            bool isIPv6 = (0 == strcasecmp("IPv6", ipversion.c_str()));
            stun::protocol  proto (isIPv6 ? stun::protocol::af_inet6  : stun::protocol::af_inet);
            if(stunClient.bind(m_stunEndPoint, m_stunPort, m_defaultInterface, proto, m_stunBindTimeout, m_stunCacheTimeout, result))
            {
                ipAddress = result.public_ip;
                return Core::ERROR_NONE;
            }
            else
            {
                return Core::ERROR_GENERAL;
            }
        }

        /* @brief Set the network manager plugin log level */
        uint32_t NetworkManagerImplementation::SetLogLevel(const NMLogging& logLevel /* @in */)
        {
            NetworkManagerLogger::SetLevel((LogLevel)logLevel);
            return Core::ERROR_NONE;
        }

        /* @brief Request for ping and get the response in as event. The GUID used in the request will be returned in the event. */
        uint32_t NetworkManagerImplementation::Ping (const string ipversion /* @in */,  const string endpoint /* @in */, const uint32_t noOfRequest /* @in */, const uint16_t timeOutInSeconds /* @in */, const string guid /* @in */, string& response /* @out */)
        {   
            char cmd[100] = "";
            if(0 == strcasecmp("IPv6", ipversion.c_str()))
            {   
                snprintf(cmd, sizeof(cmd), "ping6 -c %d -W %d '%s' 2>&1", noOfRequest, timeOutInSeconds, endpoint.c_str());
            }
            else
            {   
                snprintf(cmd, sizeof(cmd), "ping -c %d -W %d '%s' 2>&1", noOfRequest, timeOutInSeconds, endpoint.c_str());
            }
            
            NMLOG_INFO ("The Command is %s", cmd);
            string commandToExecute(cmd);
            executeExternally(NETMGR_PING, commandToExecute, response);

            return Core::ERROR_NONE;
        }

        /* @brief Request for trace get the response in as event. The GUID used in the request will be returned in the event. */
        uint32_t NetworkManagerImplementation::Trace (const string ipversion /* @in */,  const string endpoint /* @in */, const uint32_t noOfRequest /* @in */, const string guid /* @in */, string& response /* @out */)
        {
            char cmd[256] = "";
            if(0 == strcasecmp("IPv6", ipversion.c_str()))
            {
                snprintf(cmd, 256, "traceroute6 -w 3 -m 6 -q %d %s 64 2>&1", noOfRequest, endpoint.c_str());
            }
            else
            {
                snprintf(cmd, 256, "traceroute -w 3 -m 6 -q %d %s 52 2>&1", noOfRequest, endpoint.c_str());
            }

            NMLOG_INFO ("The Command is %s", cmd);
            string commandToExecute(cmd);
            executeExternally(NETMGR_TRACE, commandToExecute, response);

            return Core::ERROR_NONE;
        }

        void NetworkManagerImplementation::executeExternally(NetworkEvents event, const string commandToExecute, string& response)
        {
            FILE *pipe = NULL;
            string output{};
            char buffer[1024];
            JsonObject pingResult;

            pipe = popen(commandToExecute.c_str(), "r");
            if (pipe == NULL)
            {   
                NMLOG_INFO ("%s: failed to open file '%s' for read mode with result: %s", __FUNCTION__, commandToExecute.c_str(), strerror(errno));
                return;
            }

            if (NETMGR_PING == event)
            {
                while (!feof(pipe) && fgets(buffer, 1024, pipe) != NULL)
                {
                    // remove newline from buffer
                    buffer[strcspn(buffer, "\n")] = '\0';
                    string line(buffer);

                    if( line.find( "packet" ) != string::npos )
                    {
                        //Example: 10 packets transmitted, 10 packets received, 0% packet loss
                        stringstream ss( line );
                        int transCount;
                        ss >> transCount;
                        pingResult["packetsTransmitted"] = transCount;

                        string token;
                        getline( ss, token, ',' );
                        getline( ss, token, ',' );
                        stringstream ss2( token );
                        int rxCount;
                        ss2 >> rxCount;
                        pingResult["packetsReceived"] = rxCount;

                        getline( ss, token, ',' );
                        string prefix = token.substr(0, token.find("%"));
                        pingResult["packetLoss"] = prefix.c_str();

                    }
                    else if( line.find( "min/avg/max" ) != string::npos )
                    {
                        //Example: round-trip min/avg/max = 17.038/18.310/20.197 ms
                        stringstream ss( line );
                        string fullpath;
                        getline( ss, fullpath, '=' );
                        getline( ss, fullpath, '=' );

                        string prefix;
                        int index = fullpath.find("/");
                        if (index >= 0)
                        {
                            prefix = fullpath.substr(0, fullpath.find("/"));
                            pingResult["tripMin"] = prefix.c_str();
                        }

                        index = fullpath.find("/");
                        if (index >= 0)
                        {
                            fullpath = fullpath.substr(index + 1, fullpath.length());
                            prefix = fullpath.substr(0, fullpath.find("/"));
                            pingResult["tripAvg"] = prefix.c_str();
                        }

                        index = fullpath.find("/");
                        if (index >= 0)
                        {
                            fullpath = fullpath.substr(index + 1, fullpath.length());
                            prefix = fullpath.substr(0, fullpath.find("/"));
                            pingResult["tripMax"] = prefix.c_str();
                        }

                        index = fullpath.find("/");
                        if (index >= 0)
                        {
                            fullpath = fullpath.substr(index + 1, fullpath.length());
                            pingResult["tripStdDev"] = fullpath.c_str();
                        }
                    }
                    else if( line.find( "bad" ) != string::npos )
                    {
                        pingResult["success"] = false;
                        pingResult["error"] = "Bad Address";
                    }
                }

                pingResult.ToString(response);
                NMLOG_INFO("Response is, %s", response.c_str());
            }
            else if (NETMGR_TRACE == event)
            {

                // We return the entire output of the trace command but since this contains newlines it is not valid as
                // a json value so we will parse the output into an array of strings, one element for each line.
                JsonArray list;
                while (!feof(pipe) && fgets(buffer, 1024, pipe) != NULL)
                {
                    // remove newline from buffer
                    buffer[strcspn(buffer, "\n")] = '\0';
                    string line(buffer);
                    list.Add(line);
                }

                list.ToString(response);
                NMLOG_INFO("Response is, %s", response.c_str());
            }
            fclose(pipe);
            return;
        }

        // WiFi Specific Methods
        /* @brief Initiate a WIFI Scan; This is Async method and returns the scan results as Event */
        uint32_t NetworkManagerImplementation::GetSupportedSecurityModes(ISecurityModeIterator*& securityModes /* @out */) const
        {
            LOG_ENTRY_FUNCTION();
            std::vector<WIFISecurityModeInfo> modeInfo {
                                                            {WIFI_SECURITY_NONE,                  "WIFI_SECURITY_NONE"},
                                                            {WIFI_SECURITY_WEP_64,                "WIFI_SECURITY_WEP_64"},
                                                            {WIFI_SECURITY_WEP_128,               "WIFI_SECURITY_WEP_128"},
                                                            {WIFI_SECURITY_WPA_PSK_TKIP,          "WIFI_SECURITY_WPA_PSK_TKIP"},
                                                            {WIFI_SECURITY_WPA_PSK_AES,           "WIFI_SECURITY_WPA_PSK_AES"},
                                                            {WIFI_SECURITY_WPA2_PSK_TKIP,         "WIFI_SECURITY_WPA2_PSK_TKIP"},
                                                            {WIFI_SECURITY_WPA2_PSK_AES,          "WIFI_SECURITY_WPA2_PSK_AES"},
                                                            {WIFI_SECURITY_WPA_ENTERPRISE_TKIP,   "WIFI_SECURITY_WPA_ENTERPRISE_TKIP"},
                                                            {WIFI_SECURITY_WPA_ENTERPRISE_AES,    "WIFI_SECURITY_WPA_ENTERPRISE_AES"},
                                                            {WIFI_SECURITY_WPA2_ENTERPRISE_TKIP,  "WIFI_SECURITY_WPA2_ENTERPRISE_TKIP"},
                                                            {WIFI_SECURITY_WPA2_ENTERPRISE_AES,   "WIFI_SECURITY_WPA2_ENTERPRISE_AES"},
                                                            {WIFI_SECURITY_WPA_WPA2_PSK,          "WIFI_SECURITY_WPA_WPA2_PSK"},
                                                            {WIFI_SECURITY_WPA_WPA2_ENTERPRISE,   "WIFI_SECURITY_WPA_WPA2_ENTERPRISE"},
                                                            {WIFI_SECURITY_WPA3_PSK_AES,          "WIFI_SECURITY_WPA3_PSK_AES"},
                                                            {WIFI_SECURITY_WPA3_SAE,              "WIFI_SECURITY_WPA3_SAE"}
                                                        };

            using Implementation = RPC::IteratorType<Exchange::INetworkManager::ISecurityModeIterator>;
            securityModes = Core::Service<Implementation>::Create<Exchange::INetworkManager::ISecurityModeIterator>(modeInfo);

            return Core::ERROR_NONE;
        }

        void NetworkManagerImplementation::ReportInterfaceStateChangedEvent(INetworkManager::InterfaceState state, string interface)
        {
            LOG_ENTRY_FUNCTION();
            _notificationLock.Lock();
            for (const auto callback : _notificationCallbacks) {
                callback->onInterfaceStateChange(state, interface);
            }
            _notificationLock.Unlock();
        }

        void NetworkManagerImplementation::ReportIPAddressChangedEvent(const string& interface, bool isAcquired, bool isIPv6, const string& ipAddress)
        {
            LOG_ENTRY_FUNCTION();
            _notificationLock.Lock();
            for (const auto callback : _notificationCallbacks) {
                callback->onIPAddressChange(interface, isAcquired, isIPv6, ipAddress);
            }
            _notificationLock.Unlock();
        }

        void NetworkManagerImplementation::ReportActiveInterfaceChangedEvent(const string prevActiveInterface, const string currentActiveinterface)
        {
            LOG_ENTRY_FUNCTION();
            _notificationLock.Lock();
            for (const auto callback : _notificationCallbacks) {
                callback->onActiveInterfaceChange(prevActiveInterface, currentActiveinterface);
            }
            _notificationLock.Unlock();
        }

        void NetworkManagerImplementation::ReportInternetStatusChangedEvent(const InternetStatus oldState, const InternetStatus newstate)
        {
            LOG_ENTRY_FUNCTION();
            _notificationLock.Lock();
            for (const auto callback : _notificationCallbacks) {
                callback->onInternetStatusChange(oldState, newstate);
            }
            _notificationLock.Unlock();
        }

        void NetworkManagerImplementation::ReportAvailableSSIDsEvent(const string jsonOfWiFiScanResults)
        {
            LOG_ENTRY_FUNCTION();
            _notificationLock.Lock();
            NMLOG_INFO("scan result is, %s", jsonOfWiFiScanResults.c_str());
            for (const auto callback : _notificationCallbacks) {
                callback->onAvailableSSIDs(jsonOfWiFiScanResults);
            }
            _notificationLock.Unlock();
        }

        void NetworkManagerImplementation::ReportWiFiStateChangedEvent(const INetworkManager::WiFiState state)
        {
            LOG_ENTRY_FUNCTION();
            _notificationLock.Lock();
            for (const auto callback : _notificationCallbacks) {
                callback->onWiFiStateChange(state);
            }
            _notificationLock.Unlock();
        }

        void NetworkManagerImplementation::ReportWiFiSignalStrengthChangedEvent(const string ssid, const string signalLevel, const WiFiSignalQuality signalQuality)
        {
            LOG_ENTRY_FUNCTION();
            _notificationLock.Lock();
            for (const auto callback : _notificationCallbacks) {
                callback->onWiFiSignalStrengthChange(ssid, signalLevel, signalQuality);
                
            }
            _notificationLock.Unlock();
        }
    }
}
