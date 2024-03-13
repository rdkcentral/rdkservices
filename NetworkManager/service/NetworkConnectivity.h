#pragma once

#include <string>
#include <atomic>
#include <vector>
#include <map>
#include <curl/curl.h>
#include <condition_variable>
#include <mutex>
#include "Module.h"
#include "NetworkManagerLogger.h"

#define CAPTIVEPORTAL_MAX_LEN 512
#define DEFAULT_MONITOR_TIMEOUT 60 // in seconds
#define MONITOR_TIMEOUT_INTERVAL_MIN 5
#define TEST_CONNECTIVITY_DEFAULT_TIMEOUT_MS    4000

enum nsm_ipversion {
    NSM_IPRESOLVE_WHATEVER  = 0, /* default, resolves addresses to all IP*/
    NSM_IPRESOLVE_V4        = 1, /* resolve to IPv4 addresses */
    NSM_IPRESOLVE_V6        = 2  /* resolve to IPv6 addresses */
};

enum nsm_internetState {
    UNKNOWN = -1,
    NO_INTERNET,
    LIMITED_INTERNET,
    CAPTIVE_PORTAL,
    FULLY_CONNECTED
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

namespace WPEFramework {
    namespace Plugin {

        /* save user specific endponint in to a chache file and load form the file if monitorEndpoints are empty case wpeframework restared */
        class EndpointCache {
            public:
                static EndpointCache& getInstance() {
                    static EndpointCache instance;
                    return instance;
                }

                bool isEndpointCashFileExist();
                void writeEnpointsToFile(const std::vector<std::string>& endpoints);
                std::vector<std::string> readEnpointsFromFile();

            private:
                EndpointCache() : CachefilePath("/tmp/network.plugin.endpoints") {}
                ~EndpointCache(){}

                std::string CachefilePath;
        };

        class Connectivity {

            Connectivity(const Connectivity&) = delete;
            const Connectivity& operator=(const Connectivity&) = delete;

        public:
            Connectivity(const std::string& configFilePath = "/etc/netsrvmgr.conf")
            {
                loadConnectivityConfig(configFilePath);
                if(m_defaultEndpoints.empty())
                {
                    NMLOG_ERROR("NETSRVMGR CONFIGURATION ERROR: CONNECTIVITY ENDPOINT EMPTY");
                    m_defaultEndpoints.clear();
                    m_defaultEndpoints.push_back("http://clients3.google.com/generate_204");
                }
            }
            ~Connectivity(){}

            nsm_internetState testConnectivity(const std::vector<std::string>& endpoints, long timeout_ms, nsm_ipversion ipversion);
            std::vector<std::string> getConnectivityDefaultEndpoints() { return m_defaultEndpoints; };
            std::string getCaptivePortal() { const std::lock_guard<std::mutex> lock(capitiveMutex); return g_captivePortal; }
            void setCaptivePortal(const char* captivePortal) {const std::lock_guard<std::mutex> lock(capitiveMutex); g_captivePortal = captivePortal; }

        private:
            void loadConnectivityConfig(const std::string& configFilePath);
            nsm_internetState checkInternetStateFromResponseCode(const std::vector<int>& responses);

            std::vector<std::string> m_defaultEndpoints;
            std::map<std::string, std::string> configMap;
            std::mutex capitiveMutex;
            std::string g_captivePortal;
            bool configMonitorConnectivityEnabled = false;
            int configMonitorInterval = DEFAULT_MONITOR_TIMEOUT;
        };

        class ConnectivityMonitor : public Connectivity {
            public:
                bool isConnectedToInternet(nsm_ipversion ipversion);
                nsm_internetState getInternetConnectionState(nsm_ipversion ipversion);
                std::string getCaptivePortalURI();
                void setConnectivityMonitorEndpoints(const std::vector<std::string> &endpoints);
                std::vector<std::string> getConnectivityMonitorEndpoints();
                bool doContinuousConnectivityMonitoring(int timeoutInSeconds);
                bool doInitialConnectivityMonitoring(int timeoutInSeconds);
                bool stopContinuousConnectivityMonitoring();
                bool stopInitialConnectivityMonitoring();
                bool isConnectivityMonitorEndpointSet();
                bool isMonitorThreadRunning();
                void signalConnectivityMonitor();
                void resetConnectivityCache() { g_internetState = nsm_internetState::UNKNOWN;}

                ConnectivityMonitor() : stopFlag(false), threadRunning(false), isContinuesMonitoringNeeded(false)
                {
                    setConnectivityMonitorEndpoints(getConnectivityDefaultEndpoints());
                }

                ~ConnectivityMonitor() {
                    NMLOG_INFO("~ConnectivityMonitor");
                    stopContinuousConnectivityMonitoring();
                }

            private:
                ConnectivityMonitor(const ConnectivityMonitor&) = delete;
                ConnectivityMonitor& operator=(const ConnectivityMonitor&) = delete;
                void connectivityMonitorFunction();

                EndpointCache& endpointCache = EndpointCache::getInstance();

                std::thread thread_;
                std::atomic<bool> stopFlag;
                std::atomic<bool> threadRunning;
                std::atomic<bool> isContinuesMonitoringNeeded;
                std::condition_variable cv_;
                std::atomic<int> timeout;
                std::vector<std::string> monitorEndpoints;
                const int defaultTimeoutInSec = DEFAULT_MONITOR_TIMEOUT;
                std::mutex mutex_;
                std::mutex endpointMutex;
                std::atomic<nsm_internetState> g_internetState = {nsm_internetState::UNKNOWN};
        };
    } // namespace Plugin
} // namespace WPEFramework
