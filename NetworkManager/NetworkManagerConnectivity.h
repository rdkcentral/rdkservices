/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#pragma once

#include <cstring>
#include <atomic>
#include <vector>
#include <thread>
#include <chrono>
#include <map>
#include <curl/curl.h>
#include <condition_variable>
#include <mutex>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <ctime>

enum nsm_ipversion
{
    NSM_IPRESOLVE_WHATEVER  = 0, /* default, resolves addresses to all IP*/
    NSM_IPRESOLVE_V4        = 1, /* resolve to IPv4 addresses */
    NSM_IPRESOLVE_V6        = 2  /* resolve to IPv6 addresses */
};

enum nsm_internetState {
    NO_INTERNET,
    LIMITED_INTERNET,
    CAPTIVE_PORTAL,
    FULLY_CONNECTED,
    UNKNOWN,
};

enum nsm_connectivity_httpcode {
    HttpStatus_response_error               = 99,
    HttpStatus_200_OK                      = 200,
    HttpStatus_204_No_Content              = 204,
    HttpStatus_301_Moved_Permanentl        = 301,
    HttpStatus_302_Found                   = 302,     // captive portal
    HttpStatus_307_Temporary_Redirect      = 307,
    HttpStatus_308_Permanent_Redirect      = 308,
    HttpStatus_403_Forbidden               = 403,
    HttpStatus_404_Not_Found               = 404,
    HttpStatus_511_Authentication_Required = 511      // captive portal RFC 6585
};

#define NMCONNECTIVITY_CURL_HEAD_REQUEST          true
#define NMCONNECTIVITY_CURL_GET_REQUEST           false

#define NMCONNECTIVITY_MONITOR_DEFAULT_INTERVAL   60     // sec
#define NMCONNECTIVITY_MONITOR_MIN_INTERVAL       5      // sec
#define NMCONNECTIVITY_CURL_REQUEST_TIMEOUT_MS    5000   // ms
#define NMCONNECTIVITY_NO_INTERNET_RETRY_COUNT    4      // 4 retry
#define NMCONNECTIVITY_CONN_MONITOR_RETRY_COUNT   3      // 3 retry
#define NMCONNECTIVITY_CAPTIVE_MONITOR_INTERVAL   30     //  sec
#define NMCONNECTIVITY_CONN_MONITOR_RETRY_INTERVAL   30     //  sec

namespace WPEFramework
{
    namespace Plugin
    {
        /* save user specific endponint in to a chache file and load form the file if monitorEndpoints are empty case wpeframework restared */
        class EndpointCache {
            public:
                bool isEndpointCashFileExist();
                void writeEnpointsToFile(const std::vector<std::string>& endpoints);
                std::vector<std::string> readEnpointsFromFile();

                EndpointCache() : CachefilePath("/tmp/nm.plugin.endpoints") {}
                ~EndpointCache(){}
            private:
                std::string CachefilePath;
        };

        class TestConnectivity
        {
            TestConnectivity(const TestConnectivity&) = delete;
            const TestConnectivity& operator=(const TestConnectivity&) = delete;

        public:
            TestConnectivity(const std::vector<std::string>& endpoints, long timeout_ms = 2000, bool  = true, nsm_ipversion ipversion = NSM_IPRESOLVE_WHATEVER);
            ~TestConnectivity(){}
            std::string getCaptivePortal() {return captivePortalURI;}
            nsm_internetState getInternetState(){return internetSate;}
        private:
            nsm_internetState checkCurlResponse(const std::vector<std::string>& endpoints, long timeout_ms,  bool headReq, nsm_ipversion ipversion);
            nsm_internetState checkInternetStateFromResponseCode(const std::vector<int>& responses);
            std::string captivePortalURI;
            nsm_internetState internetSate;
        };

        class ConnectivityMonitor
        {
        public:
            ConnectivityMonitor();
            ~ConnectivityMonitor();
            bool startContinuousConnectivityMonitor(int timeoutInSeconds);
            bool stopContinuousConnectivityMonitor();
            bool startConnectivityMonitor(bool interfaceStatus);
            void setConnectivityMonitorEndpoints(const std::vector<std::string> &endpoints);
            std::vector<std::string> getConnectivityMonitorEndpoints();
            bool isConnectedToInternet(nsm_ipversion ipversion);
            nsm_internetState getInternetState(nsm_ipversion ipversion);
            std::string getCaptivePortalURI();

        private:
            ConnectivityMonitor(const ConnectivityMonitor&) = delete;
            ConnectivityMonitor& operator=(const ConnectivityMonitor&) = delete;
            void connectivityMonitorFunction();
            void notifyInternetStatusChangedEvent(nsm_internetState newState);
            /* connectivity monitor */
            std::thread connectivityMonitorThrd;
            std::condition_variable cvConnectivityMonitor;
            std::atomic<int> continuousMonitorTimeout;
            std::atomic<bool> doConnectivityMonitor;
            std::vector<std::string> connectivityMonitorEndpt;
            /*continuous connectivity monitor */
            std::atomic<bool> doContinuousMonitor;
            std::thread continuousMonitorThrd;
            std::condition_variable cvContinuousMonitor;
            void continuousMonitorFunction();

            EndpointCache endpointCache;
            std::mutex endpointMutex;
            std::atomic<bool> ginterfaceStatus;
            std::atomic<nsm_internetState> gInternetState;
            std::atomic<nsm_internetState> gIpv4InternetState;
            std::atomic<nsm_internetState> gIpv6InternetState;
        };
    } // namespace Plugin
} // namespace WPEFramework
