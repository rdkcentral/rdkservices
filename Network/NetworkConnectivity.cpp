#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <algorithm>
#include <time.h>

#include <vector>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <atomic>
#include <mutex>
#include <curl/curl.h>

#include "UtilsLogging.h"
#include "Module.h"
#include "Network.h"
#include "NetworkConnectivity.h"

namespace WPEFramework {
    namespace Plugin {

    bool EndpointCache::isEndpointCashFileExist()
    {
        std::ifstream fileStream(CachefilePath);
        return fileStream.is_open();
    }

    void EndpointCache::writeEnpointsToFile(const std::vector<std::string>& endpoints)
    {
        std::ofstream outputFile(CachefilePath);
        if (outputFile.is_open())
        {
            for (const std::string& str : endpoints)
            {
                outputFile << str << '\n';
            }
            outputFile.close();
            LOGINFO("Connectivity endpoints successfully written to a file");
        }
        else
        {
            LOGERR("Connectivity endpoints file write error");
        }
    }

    std::vector<std::string> EndpointCache::readEnpointsFromFile()
    {
        std::vector<std::string> readStrings;
        std::ifstream inputFile(CachefilePath);
        if (inputFile.is_open())
        {
            std::string line;
            while (std::getline(inputFile, line))
            {
                readStrings.push_back(line);
            }
            inputFile.close();
        }
        else
        {
            LOGERR("Failed to open connectivity endpoint file");
        }
        return readStrings;
    }

    void Connectivity::setConnectivityDefaultEndpoints(const std::vector<std::string> &defaultEndpoints)
    {
        _defaultEndpoints.clear();
        for (auto endpoint : defaultEndpoints)
        {
            if(!endpoint.empty() && endpoint.size() > 3)
                _defaultEndpoints.push_back(endpoint.c_str());
            else
                LOGERR("endpoint are not valid = %s", endpoint.c_str());
        }

        std::string endpoints_str;
        for (const auto& endpoint : _defaultEndpoints)
            endpoints_str.append(endpoint).append(" ");
        LOGINFO("default endpoints count %d and endpoints:- %s", static_cast<int>(_defaultEndpoints.size()), endpoints_str.c_str());
    }

    bool ConnectivityMonitor::isConnectivityMonitorEndpointSet()
    {
        if(monitorEndpoints.size() > 0)
        {
            return true;
        }
        else if(endpointCache.isEndpointCashFileExist())
        {
            monitorEndpoints = endpointCache.readEnpointsFromFile();
            std::string endpoints_str;
            for (const auto& endpoint : monitorEndpoints)
                endpoints_str.append(endpoint).append(" ");
            LOGINFO("updated endpoints count %d and endpoints:- %s", static_cast<int>(monitorEndpoints.size()), endpoints_str.c_str());
            if(monitorEndpoints.size() > 0)
                return true;
        }
        return false;
    }

    bool ConnectivityMonitor::isConnectedToInternet(nsm_ipversion ipversion)
    {
        bool connectivityStatus = false;

        if(isConnectivityMonitorEndpointSet())
            connectivityStatus = testConnectivity(getConnectivityMonitorEndpoints(), 2000, ipversion);
        else
            connectivityStatus = testConnectivity(_defaultEndpoints, 2000, ipversion);

        if (connectivityStatus)
        {
            LOGINFO("isConnectedToInternet = true");
        }
        else
        {
            LOGWARN("isConnectedToInternet = false");
        }

        return connectivityStatus;
    }

    nsm_internetState ConnectivityMonitor::getInternetConnectionState(nsm_ipversion ipversion)
    {
        bool isconnected = isConnectedToInternet(ipversion);
        return ((isconnected == true) ? FULLY_CONNECTED : NO_INTERNET);
    }

    std::string ConnectivityMonitor::getCaptivePortalURI()
    {
        return std::string("");
    }

    void ConnectivityMonitor::setConnectivityMonitorEndpoints(const std::vector<std::string> &endpoints)
    {
        const std::lock_guard<std::mutex> lock(endpointMutex);
        monitorEndpoints.clear();
        for (auto endpoint : endpoints) {
            if(!endpoint.empty() && endpoint.size() > 3)
                monitorEndpoints.push_back(endpoint.c_str());
            else
                LOGINFO("endpoint not vallied = %s", endpoint.c_str());
        }

        // write the endpoints to a file
        endpointCache.writeEnpointsToFile(monitorEndpoints);

        std::string endpoints_str;
        for (const auto& endpoint : monitorEndpoints)
            endpoints_str.append(endpoint).append(" ");
        LOGINFO("Connectivity monitor endpoints -: %d :- %s", static_cast<int>(monitorEndpoints.size()), endpoints_str.c_str());
    }

    std::vector<std::string> ConnectivityMonitor::getConnectivityMonitorEndpoints()
    {
        const std::lock_guard<std::mutex> lock(endpointMutex);
        std::vector<std::string> endpoints;
        for (auto endpoint : monitorEndpoints) {
            endpoints.push_back(endpoint);
        }
        return endpoints;
    }

    static bool curlVerboseEnabled() {
        std::ifstream fileStream("/tmp/network.plugin.debug");
        return fileStream.is_open();
    }

    static long current_time ()
    {
        struct timespec ts;
        clock_gettime (CLOCK_MONOTONIC, &ts);
        return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    }

    bool Connectivity::testConnectivity(const std::vector<std::string>& endpoints, long timeout_ms, nsm_ipversion ipversion)
    {
        int endpoints_count = endpoints.size();

        if(endpoints_count < 1) {
            LOGERR("endpoints size error ! ");
            return false;
        }

        long deadline = current_time() + timeout_ms, time_now = 0;
        CURLM *curl_multi_handle = curl_multi_init();
        if (!curl_multi_handle)
        {
            LOGERR("curl_multi_init returned NULL");
            return -1;
        }
        CURLMcode mc;
        std::vector<CURL*> curl_easy_handles;
        for (const auto& endpoint : endpoints)
        {
            CURL *curl_easy_handle = curl_easy_init();
            if (!curl_easy_handle)
            {
                LOGERR("endpoint = <%s> curl_easy_init returned NULL", endpoint.c_str());
                continue;
            }
            curl_easy_setopt(curl_easy_handle, CURLOPT_URL, endpoint.c_str());
            curl_easy_setopt(curl_easy_handle, CURLOPT_PRIVATE, endpoint.c_str());
            curl_easy_setopt(curl_easy_handle, CURLOPT_CONNECT_ONLY, 1L);
            curl_easy_setopt(curl_easy_handle, CURLOPT_TIMEOUT_MS, deadline - current_time());
            if ((ipversion == CURL_IPRESOLVE_V4) || (ipversion == CURL_IPRESOLVE_V6))
                curl_easy_setopt(curl_easy_handle, CURLOPT_IPRESOLVE, ipversion);
            if(curlVerboseEnabled())
                curl_easy_setopt(curl_easy_handle, CURLOPT_VERBOSE, 1L);
            if (CURLM_OK != (mc = curl_multi_add_handle(curl_multi_handle, curl_easy_handle)))
            {
                LOGERR("endpoint = <%s> curl_multi_add_handle returned %d (%s)", endpoint.c_str(), mc, curl_multi_strerror(mc));
                curl_easy_cleanup(curl_easy_handle);
                continue;
            }
            curl_easy_handles.push_back(curl_easy_handle);
        }
        int connectivity = 0, handles, msgs_left;
    #if LIBCURL_VERSION_NUM < 0x074200
        int numfds, repeats = 0;
    #endif
        char *endpoint;
        while (1)
        {
            if (CURLM_OK != (mc = curl_multi_perform(curl_multi_handle, &handles)))
            {
                LOGERR("curl_multi_perform returned %d (%s)", mc, curl_multi_strerror(mc));
                break;
            }
            for (CURLMsg *msg; NULL != (msg = curl_multi_info_read(curl_multi_handle, &msgs_left)); )
            {
                if (msg->msg != CURLMSG_DONE)
                    continue;
                if (CURLE_OK == msg->data.result)
                    connectivity++;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &endpoint);
                //LOGINFO("endpoint = <%s> error = %d (%s)", endpoint, msg->data.result, curl_easy_strerror(msg->data.result));
            }
            time_now = current_time();
            if (handles == 0 || time_now >= deadline)
                break;
    #if LIBCURL_VERSION_NUM < 0x074200
            if (CURLM_OK != (mc = curl_multi_wait(curl_multi_handle, NULL, 0, deadline - time_now, &numfds)))
            {
                LOGERR("curl_multi_wait returned %d (%s)", mc, curl_multi_strerror(mc));
                break;
            }
            if (numfds == 0)
            {
                repeats++;
                if (repeats > 1)
                    usleep(10*1000); /* sleep 10 ms */
            }
            else
                repeats = 0;
    #else
            if (CURLM_OK != (mc = curl_multi_poll(curl_multi_handle, NULL, 0, deadline - time_now, NULL)))
            {
                LOGERR("curl_multi_poll returned %d (%s)", mc, curl_multi_strerror(mc));
                break;
            }
    #endif
        }

        if(curlVerboseEnabled())
            LOGINFO("connectivity = %d, endpoints count = %d, handles = %d, deadline = %ld, time_now = %ld", connectivity, endpoints_count, handles, deadline, time_now);

        for (const auto& curl_easy_handle : curl_easy_handles)
        {
            curl_easy_getinfo(curl_easy_handle, CURLINFO_PRIVATE, &endpoint);
            curl_multi_remove_handle(curl_multi_handle, curl_easy_handle);
            curl_easy_cleanup(curl_easy_handle);
        }
        curl_multi_cleanup(curl_multi_handle);

        if(endpoints_count == 1) // for one endpoint
        {
            if(connectivity == 1)
                return true;
        }
        else if (endpoints_count > 1)
        {
            int successRate;
            if(endpoints_count % 2)
            {
                successRate = (endpoints_count/2) + 1;
            }
            else
            {
                successRate = endpoints_count/2;
            }
            //LOGINFO("50 percentage endpoints count %d", successRate);
            if(connectivity >= successRate) // checking ~50% endpoints connectivity
                return true;
        }

        return false;
    }

    bool ConnectivityMonitor::startConnectivityMonitor(int timeoutInSeconds)
    {

        if(!isConnectivityMonitorEndpointSet())
        {
            LOGINFO("Connectivity monitor endpoints are not set !");
            return false;
        }

        timeout.store(timeoutInSeconds >= defaultTimeoutInSec ? timeoutInSeconds:defaultTimeoutInSec);

        if (!isMonitorThreadRunning())
        {
            stopFlag.store(false);
            notifyNow.store(false);
            thread_ = std::thread(&ConnectivityMonitor::connectivityMonitorFunction, this);
            threadRunning = true;
            LOGINFO("Connectivity monitor started with %d", timeout.load());
        }
        else
        {
            cv_.notify_all();
            LOGINFO("Connectivity monitor Restarted with %d", timeout.load());
        }
        return true;
    }

    bool ConnectivityMonitor::isMonitorThreadRunning()
    {
        return threadRunning.load();
    }

    bool ConnectivityMonitor::stopConnectivityMonitor()
    {
        if (isMonitorThreadRunning()) {
            stopFlag.store(true);
            cv_.notify_all();
            if (thread_.joinable()) {
                thread_.join();
                threadRunning = false;
                LOGINFO("Connectivity monitor stopped");
            }
            return true;
        }
        else
            LOGWARN("Connectivity monitor not running");
        return false;
    }

    void ConnectivityMonitor::signalConnectivityMonitor()
    {
        if (isMonitorThreadRunning())
        {
            notifyNow.store(true);
            cv_.notify_all();
            LOGINFO("Connectivity monitor triggered ");
        }
    }

    void ConnectivityMonitor::connectivityMonitorFunction()
    {
        while (!stopFlag)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (cv_.wait_for(lock, std::chrono::seconds(timeout.load())) == std::cv_status::timeout)
            {
                //LOGINFO("Connectivity monitor thread timeout");
                notifyNow.store(false);
            }
            else
            {
                if(!stopFlag)
                {
                    LOGINFO("Connectivity monitor received a trigger");
                }
                else
                    break;
            }
            bool isconnected = isConnectedToInternet(NSM_IPRESOLVE_WHATEVER);
            nsm_internetState InternetConnectionState = (isconnected == true) ? FULLY_CONNECTED : NO_INTERNET;
            Network::notifyInternetStatusChange(InternetConnectionState, notifyNow.load());
        }
        LOGWARN("Connectivity monitor exiting");
    }

    } // namespace Plugin
} // namespace WPEFramework
