#pragma once

#include <string>
#include <atomic>
#include <vector>
#include <curl/curl.h>
#include <condition_variable>
#include <mutex>

#include "Module.h"

#define CAPTIVEPORTAL_MAX_LEN 512
#define DEFAULT_MONITOR_TIMEOUT 5

enum nsm_ipversion {
    ipresolve_whatever  = 0, /* default, resolves addresses to all IP*/
    ipresolve_v4        = 1, /* resolve to IPv4 addresses */
    ipresolve_v6        = 2  /* resolve to IPv6 addresses */
};

enum nsm_internetState {
    NO_INTERNET,
    LIMITED_INTERNET,
    CAPTIVE_PORTAL,
    FULLY_CONNECTED
};

namespace WPEFramework {
    namespace Plugin {

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
            Connectivity(){}
            ~Connectivity(){}

            bool testConnectivity(const std::vector<std::string>& endpoints, long timeout_ms, nsm_ipversion ipversion);
            void setConnectivityDefaultEndpoints(const std::vector<std::string> &endpoints);
            std::vector<std::string> _defaultEndpoints;
        };

        class ConnectivityMonitor : public Connectivity {
            public:
                static ConnectivityMonitor& getInstance() {
                    static ConnectivityMonitor instance;
                    return instance;
                }

                bool isConnectedToInternet(nsm_ipversion ipversion);
                void setConnectivityMonitorEndpoints(const std::vector<std::string> &endpoints);
                bool startConnectivityMonitor(int timeoutInSeconds);
                bool stopConnectivityMonitor();
                bool isConnectivityMonitorEndpointSet();
                bool isMonitorThreadRunning();
                void signalConnectivityMonitor();

            private:
                ConnectivityMonitor() : stopFlag(false), threadRunning(false) {}
                ~ConnectivityMonitor() {
                    stopConnectivityMonitor();
                }

                std::vector<std::string> getConnectivityMonitorEndpoints();
                ConnectivityMonitor(const ConnectivityMonitor&) = delete;
                ConnectivityMonitor& operator=(const ConnectivityMonitor&) = delete;
                void connectivityMonitorFunction();

                EndpointCache& endpointCache = EndpointCache::getInstance();

                std::thread thread_;
                std::atomic<bool> stopFlag;
                std::atomic<bool> threadRunning;
                std::condition_variable cv_;
                std::atomic<int> timeout;
                std::vector<std::string> monitorEndpoints;
                const int defaultTimeoutInSec = DEFAULT_MONITOR_TIMEOUT;
                std::mutex mutex_;
                std::mutex endpointMutex;
        };
    } // namespace Plugin
} // namespace WPEFramework
