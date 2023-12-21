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
#include "Module.h"
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
            printf("Connectivity endpoints successfully written to a file");
        }
        else
        {
            printf("Connectivity endpoints file write error");
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
            printf("Failed to open connectivity endpoint cache file");
        }
        return readStrings;
    }

    void Connectivity::loadConnectivityConfig(const std::string& configFilePath)
    {
        std::ifstream configFile(configFilePath);
        if (!configFile.is_open()) 
        {
            printf("Unable to open the configuration file: %s", configFilePath.c_str());
            return;
        }

        bool ConnectivityConfigFound = false;

        // load connectivity endpoint configuration
        std::string line;
        while (std::getline(configFile, line))
        {
            if (line == "[Connectivity_Config]")
            {
                ConnectivityConfigFound = true;
                continue;
            }

            if (ConnectivityConfigFound)
            {
                size_t equalsPos = line.find('=');
                if (equalsPos != std::string::npos)
                {
                    std::string key = line.substr(0, equalsPos);
                    std::string value = line.substr(equalsPos + 1);
                    configMap[key] = value;
                }
            }
        }

        configFile.close();

        /* Parse the connectivity monitor interval and enable values */
        configMonitorConnectivityEnabled = ((configMap["CONNECTIVITY_MONITOR_ENABLE"] == "1")? true:false);
        std::string monitorIntervalStr = configMap["CONNECTIVITY_MONITOR_INTERVAL"];
        if (!monitorIntervalStr.empty())
        {
            configMonitorInterval = std::stoi(monitorIntervalStr);
        }

        m_defaultEndpoints.clear();
        for (int i = 1; i <= 5; ++i)
        {
            std::string endpointName = "CONNECTIVITY_ENDPOINT_" + std::to_string(i);
            auto endpoint = configMap.find(endpointName);
            if (endpoint != configMap.end() && endpoint->second.length() > 3)
            {
                m_defaultEndpoints.push_back(endpoint->second);
            }
        }

        if(m_defaultEndpoints.empty())
        {
            printf("Default endpoints are empty !!");
        }
        else
        {
            std::string endpoints_str;
            for (const auto& endpoint : m_defaultEndpoints)
                endpoints_str.append(endpoint).append(" ");
            printf("default endpoints count %d and endpoints:- %s", static_cast<int>(m_defaultEndpoints.size()), endpoints_str.c_str());
            printf("default monitor connectivity interval: %d and monitor connectivity auto start : %s", configMonitorInterval, configMonitorConnectivityEnabled?"true":"false");
        }
    }

    bool ConnectivityMonitor::isConnectivityMonitorEndpointSet()
    {
        const std::lock_guard<std::mutex> lock(endpointMutex);
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
            printf("updated endpoints count %d and endpoints:- %s", static_cast<int>(monitorEndpoints.size()), endpoints_str.c_str());
            if(monitorEndpoints.size() > 0)
                return true;
        }
        return false;
    }

    bool ConnectivityMonitor::isConnectedToInternet(nsm_ipversion ipversion)
    {
        if (nsm_internetState::FULLY_CONNECTED == getInternetConnectionState(ipversion))
        {
            printf("isConnectedToInternet = true");
            return true;
        }

        printf("isConnectedToInternet = false");
        return false;
    }

    nsm_internetState ConnectivityMonitor::getInternetConnectionState(nsm_ipversion ipversion)
    {

        nsm_internetState internetState = nsm_internetState::UNKNOWN;

        // If monitor connectivity is running take the cashe value
        if ( isMonitorThreadRunning() && (ipversion == NSM_IPRESOLVE_WHATEVER) && (g_internetState != nsm_internetState::UNKNOWN) )
        {
            internetState = g_internetState;
        }
        else
        {
            if(isConnectivityMonitorEndpointSet())
                internetState = testConnectivity(getConnectivityMonitorEndpoints(), 2000, ipversion);
            else
                internetState = testConnectivity(getConnectivityDefaultEndpoints(), 2000, ipversion);
        }

        return internetState;
    }

    std::string ConnectivityMonitor::getCaptivePortalURI()
    {
        if (g_internetState == nsm_internetState::CAPTIVE_PORTAL)
        {
            return getCaptivePortal();
        }
        else
        {
            std::string captivePortal;
            if(getInternetConnectionState(NSM_IPRESOLVE_WHATEVER) == nsm_internetState::CAPTIVE_PORTAL)
            {
                return getCaptivePortal();
            }
            else
                printf("No captive portal found !");
        }
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
                printf("endpoint not vallied = %s", endpoint.c_str());
        }

        // write the endpoints to a file
        endpointCache.writeEnpointsToFile(monitorEndpoints);

        std::string endpoints_str;
        for (const auto& endpoint : monitorEndpoints)
            endpoints_str.append(endpoint).append(" ");
        printf("Connectivity monitor endpoints -: %d :- %s", static_cast<int>(monitorEndpoints.size()), endpoints_str.c_str());
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
    static size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
    #ifdef DBG_CURL_GET_RESPONSE
        printf("%s",(char*)ptr);
    #endif
        return size * nmemb;
    }

    nsm_internetState Connectivity::testConnectivity(const std::vector<std::string>& endpoints, long timeout_ms, nsm_ipversion ipversion)
    {
        long deadline = current_time() + timeout_ms, time_now = 0, time_earlier = 0;
        if(endpoints.size() < 1) {
            printf("endpoints size error ! ");
            return NO_INTERNET;
        }

        CURLM *curl_multi_handle = curl_multi_init();
        if (!curl_multi_handle)
        {
            printf("curl_multi_init returned NULL");
            return NO_INTERNET;
        }

        CURLMcode mc;
        std::vector<CURL*> curl_easy_handles;
        std::vector<int> http_responses;
        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "Cache-Control: no-cache, no-store");
        for (const auto& endpoint : endpoints)
        {
            CURL *curl_easy_handle = curl_easy_init();
            if (!curl_easy_handle)
            {
                printf("endpoint = <%s> curl_easy_init returned NULL", endpoint.c_str());
                continue;
            }
            curl_easy_setopt(curl_easy_handle, CURLOPT_URL, endpoint.c_str());
            curl_easy_setopt(curl_easy_handle, CURLOPT_PRIVATE, endpoint.c_str());
            /* set our custom set of headers */
            curl_easy_setopt(curl_easy_handle, CURLOPT_HTTPHEADER, chunk);
            curl_easy_setopt(curl_easy_handle, CURLOPT_USERAGENT, "RDKCaptiveCheck/1.0");
            /* set CURLOPT_HTTPGET option insted of CURLOPT_CONNECT_ONLY bcause we need to get the captiveportal URI not just connection only */
            curl_easy_setopt(curl_easy_handle, CURLOPT_HTTPGET, 1L);
            curl_easy_setopt(curl_easy_handle, CURLOPT_WRITEFUNCTION, writeFunction);
            curl_easy_setopt(curl_easy_handle, CURLOPT_TIMEOUT_MS, deadline - current_time());
            if ((ipversion == CURL_IPRESOLVE_V4) || (ipversion == CURL_IPRESOLVE_V6))
                curl_easy_setopt(curl_easy_handle, CURLOPT_IPRESOLVE, ipversion);
            if(curlVerboseEnabled())
                curl_easy_setopt(curl_easy_handle, CURLOPT_VERBOSE, 1L);
            if (CURLM_OK != (mc = curl_multi_add_handle(curl_multi_handle, curl_easy_handle)))
            {
                printf("endpoint = <%s> curl_multi_add_handle returned %d (%s)", endpoint.c_str(), mc, curl_multi_strerror(mc));
                curl_easy_cleanup(curl_easy_handle);
                continue;
            }
            curl_easy_handles.push_back(curl_easy_handle);
        }
        int handles, msgs_left;
        char *url = nullptr;
    #if LIBCURL_VERSION_NUM < 0x074200
        int numfds, repeats = 0;
    #endif
        char *endpoint;
        while (1)
        {
            if (CURLM_OK != (mc = curl_multi_perform(curl_multi_handle, &handles)))
            {
                printf("curl_multi_perform returned %d (%s)", mc, curl_multi_strerror(mc));
                break;
            }
            for (CURLMsg *msg; NULL != (msg = curl_multi_info_read(curl_multi_handle, &msgs_left)); )
            {
                long response_code = -1;
                if (msg->msg != CURLMSG_DONE)
                    continue;
                if (CURLE_OK == msg->data.result) {
                    curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &endpoint);
                    if (curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &response_code) == CURLE_OK)  {
                        if(curlVerboseEnabled())
                            printf("endpoint = <%s> response code <%d>",endpoint, static_cast<int>(response_code));
                        if (HttpStatus_302_Found == response_code) {
                            if ( (curl_easy_getinfo(msg->easy_handle, CURLINFO_REDIRECT_URL, &url) == CURLE_OK) && url != nullptr) {
                                //printf("captive portal found !!!");
                                setCaptivePortal(url);
                            }
                        }
                    }
                }
                //else
                //    printf("endpoint = <%s> error = %d (%s)", endpoint, msg->data.result, curl_easy_strerror(msg->data.result));
                http_responses.push_back(response_code);
            }
            time_earlier = time_now;
            time_now = current_time();
            if (handles == 0 || time_now >= deadline)
                break;
    #if LIBCURL_VERSION_NUM < 0x074200
            if (CURLM_OK != (mc = curl_multi_wait(curl_multi_handle, NULL, 0, deadline - time_now, &numfds)))
            {
                printf("curl_multi_wait returned %d (%s)", mc, curl_multi_strerror(mc));
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
                printf("curl_multi_poll returned %d (%s)", mc, curl_multi_strerror(mc));
                break;
            }
    #endif
        }
        if(curlVerboseEnabled()) {
            printf("endpoints count = %d response count %d, handles = %d, deadline = %ld, time_now = %ld, time_earlier = %ld",
                static_cast<int>(endpoints.size()), static_cast<int>(http_responses.size()), handles, deadline, time_now, time_earlier);
        }
        for (const auto& curl_easy_handle : curl_easy_handles)
        {
            curl_easy_getinfo(curl_easy_handle, CURLINFO_PRIVATE, &endpoint);
            //printf("endpoint = <%s> terminating attempt", endpoint);
            curl_multi_remove_handle(curl_multi_handle, curl_easy_handle);
            curl_easy_cleanup(curl_easy_handle);
        }
        curl_multi_cleanup(curl_multi_handle);
        /* free the custom headers */
        curl_slist_free_all(chunk);
        return checkInternetStateFromResponseCode(http_responses);
    }

    /*
     * verifying Most occurred response code is 50 % or more
     * Example 1 :
     *      if we have 5 endpoints so response also 5 ( 204 302 204 204 200 ) . Here count is 204 :- 3, 302 :- 1, 200 :- 1
     *      Return Internet State: FULLY_CONNECTED - 60 %
     * Example 2 :
     *      if we have 4 endpoints so response also 4 ( 204 204 200 200 ) . Here count is 204 :- 2, 200 :- 2
     *      Return Internet State: FULLY_CONNECTED - 50 %
     */

    nsm_internetState Connectivity::checkInternetStateFromResponseCode(const std::vector<int>& responses)
    {
        nsm_internetState InternetConnectionState = NO_INTERNET;
        nsm_connectivity_httpcode http_response_code = HttpStatus_response_error;

        int max_count = 0;
        for (int element : responses)
        {
            int element_count = count(responses.begin(), responses.end(), element);
            if (element_count > max_count)
            {
                http_response_code = static_cast<nsm_connectivity_httpcode>(element);
                max_count = element_count;
            }
        }

        /* Calculate the percentage of the most frequent code occurrences */
        float percentage = (static_cast<float>(max_count) / responses.size());

        /* 50 % connectivity check */
        if (percentage >= 0.5)
        {
            switch (http_response_code)
            {
                case HttpStatus_204_No_Content:
                    InternetConnectionState = FULLY_CONNECTED;
                    printf("Internet State: FULLY_CONNECTED - %.1f%%", (percentage*100));
                break;
                case HttpStatus_200_OK:
                    InternetConnectionState = LIMITED_INTERNET;
                    printf("Internet State: LIMITED_INTERNET - %.1f%%", (percentage*100));
                break;
                case HttpStatus_511_Authentication_Required:
                case HttpStatus_302_Found:
                    InternetConnectionState = CAPTIVE_PORTAL;
                    printf("Internet State: CAPTIVE_PORTAL -%.1f%%", (percentage*100));
                break;
                default:
                    InternetConnectionState = NO_INTERNET;
                    printf("Internet State: NO_INTERNET Response code: <%d> %.1f%%", static_cast<int>(http_response_code), (percentage*100));
            }
        }

        return InternetConnectionState;
    }

    bool ConnectivityMonitor::startConnectivityMonitor(int timeoutInSeconds)
    {

        if(!isConnectivityMonitorEndpointSet())
        {
            printf("Connectivity monitor endpoints are not set !");
            return false;
        }

        timeout.store(timeoutInSeconds >= MONITOR_TIMEOUT_INTERVAL_MIN ? timeoutInSeconds:defaultTimeoutInSec);

        if (!isMonitorThreadRunning())
        {
            stopFlag.store(false);
            thread_ = std::thread(&ConnectivityMonitor::connectivityMonitorFunction, this);
            threadRunning = true;
            printf("Connectivity monitor started with %d", timeout.load());
        }
        else
        {
            cv_.notify_all();
            printf("Connectivity monitor Restarted with %d", timeout.load());
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
                printf("Connectivity monitor stopped");
                g_internetState.store(nsm_internetState::UNKNOWN);
            }
            return true;
        }
        else
            printf("Connectivity monitor not running");
        return false;
    }

    void ConnectivityMonitor::signalConnectivityMonitor()
    {
        if (isMonitorThreadRunning())
        {
            /* Reset the global value to UNKNOWN state*/
            resetConnectivityCache();
            cv_.notify_all();
        }
    }

    void ConnectivityMonitor::connectivityMonitorFunction()
    {
        nsm_internetState InternetConnectionState = nsm_internetState::UNKNOWN;
        while (!stopFlag)
        {
            InternetConnectionState = testConnectivity(getConnectivityMonitorEndpoints(), 2000, NSM_IPRESOLVE_WHATEVER);
            if(g_internetState.load() != InternetConnectionState)
            {
                g_internetState.store(InternetConnectionState);
                //Network::notifyInternetStatusChange(g_internetState.load());
            }

            //wait for next timout or conditon signal
            std::unique_lock<std::mutex> lock(mutex_);
            if (cv_.wait_for(lock, std::chrono::seconds(timeout.load())) == std::cv_status::timeout)
            {
                printf("Connectivity monitor thread timeout");
            }
            else
            {
                if(!stopFlag)
                {
                    printf("Connectivity monitor received a trigger");
                }
            }
        }
        printf("Connectivity monitor exiting");
    }

    } // namespace Plugin
} // namespace WPEFramework
