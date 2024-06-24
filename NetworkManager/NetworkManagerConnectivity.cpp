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

#include <curl/curl.h>
#include <resolv.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <fstream>

#include "NetworkManagerImplementation.h"
#include "NetworkManagerConnectivity.h"
#include "NetworkManagerLogger.h"

namespace WPEFramework
{
    namespace Plugin
    {

    extern NetworkManagerImplementation* _instance;

    static const char* getInternetStateString(nsm_internetState state)
    {
        switch(state)
        {
            case NO_INTERNET: return "NO_INTERNET";
            case LIMITED_INTERNET: return "LIMITED_INTERNET";
            case CAPTIVE_PORTAL: return "CAPTIVE_PORTAL";
            case FULLY_CONNECTED: return "FULLY_CONNECTED";
            default: return "UNKNOWN";
        }
    }

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
        }
        else
        {
            NMLOG_ERROR("Connectivity endpoints file write error");
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
            NMLOG_ERROR("Failed to open connectivity endpoint cache file");
        }
        return readStrings;
    }

    TestConnectivity::TestConnectivity(const std::vector<std::string>& endpoints, long timeout_ms, bool headReq, nsm_ipversion ipversion)
    {
        internetSate = UNKNOWN;
        if(endpoints.size() < 1) {
            NMLOG_ERROR("Endpoints size error ! curl check not possible");
            return;
        }

        internetSate = checkCurlResponse(endpoints, timeout_ms, headReq, ipversion);
    }

    static bool curlVerboseEnabled() {
        std::ifstream fileStream("/tmp/nm.plugin.debug");
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
        LOG_DBG("%s",(char*)ptr);
    #endif
        return size * nmemb;
    }

    nsm_internetState TestConnectivity::checkCurlResponse(const std::vector<std::string>& endpoints, long timeout_ms,  bool headReq, nsm_ipversion ipversion)
    {
        long deadline = current_time() + timeout_ms, time_now = 0, time_earlier = 0;

        CURLM *curl_multi_handle = curl_multi_init();
        if (!curl_multi_handle)
        {
            NMLOG_ERROR("curl_multi_init returned NULL");
            return NO_INTERNET;
        }

        CURLMcode mc;
        std::vector<CURL*> curl_easy_handles;
        std::vector<int> http_responses;
        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "Cache-Control: no-cache, no-store");
        chunk = curl_slist_append(chunk, "Connection: close");
        for (const auto& endpoint : endpoints)
        {
            CURL *curl_easy_handle = curl_easy_init();
            if (!curl_easy_handle)
            {
                NMLOG_ERROR("endpoint = <%s> curl_easy_init returned NULL", endpoint.c_str());
                continue;
            }
            curl_easy_setopt(curl_easy_handle, CURLOPT_URL, endpoint.c_str());
            curl_easy_setopt(curl_easy_handle, CURLOPT_PRIVATE, endpoint.c_str());
            /* set our custom set of headers */
            curl_easy_setopt(curl_easy_handle, CURLOPT_HTTPHEADER, chunk);
            curl_easy_setopt(curl_easy_handle, CURLOPT_USERAGENT, "RDKCaptiveCheck/1.0");
            if(!headReq)
            {
               // NMLOG_TRACE("curlopt get request");
                /* HTTPGET request added insted of HTTPHEAD request fix for DELIA-61526 */
                curl_easy_setopt(curl_easy_handle, CURLOPT_HTTPGET, 1L);
            }
            curl_easy_setopt(curl_easy_handle, CURLOPT_WRITEFUNCTION, writeFunction);
            curl_easy_setopt(curl_easy_handle, CURLOPT_TIMEOUT_MS, deadline - current_time());
            if ((ipversion == CURL_IPRESOLVE_V4) || (ipversion == CURL_IPRESOLVE_V6))
            {
                NMLOG_INFO("curlopt ipversion = %s reqtyp = %s", ipversion == CURL_IPRESOLVE_V4?"ipv4 only":"ipv6 only", headReq? "HEAD":"GET");
                curl_easy_setopt(curl_easy_handle, CURLOPT_IPRESOLVE, ipversion);
            }
            else
            {
                NMLOG_INFO("curlopt ipversion = whatever reqtyp = %s", headReq? "HEAD":"GET");
            }
            if(curlVerboseEnabled())
                curl_easy_setopt(curl_easy_handle, CURLOPT_VERBOSE, 1L);
            if (CURLM_OK != (mc = curl_multi_add_handle(curl_multi_handle, curl_easy_handle)))
            {
                NMLOG_ERROR("endpoint = <%s> curl_multi_add_handle returned %d (%s)", endpoint.c_str(), mc, curl_multi_strerror(mc));
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
        char *endpoint = nullptr;
        while (1)
        {
            if (CURLM_OK != (mc = curl_multi_perform(curl_multi_handle, &handles)))
            {
                NMLOG_ERROR("curl_multi_perform returned %d (%s)", mc, curl_multi_strerror(mc));
                break;
            }
            for (CURLMsg *msg; NULL != (msg = curl_multi_info_read(curl_multi_handle, &msgs_left)); )
            {
                long response_code = -1;
                if (msg->msg != CURLMSG_DONE)
                    continue;
                if (CURLE_OK == msg->data.result) {
                    curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &endpoint);
                    if (curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &response_code) == CURLE_OK)
                    {
                        //NMLOG_TRACE("endpoint = <%s> http response code <%d>", endpoint, static_cast<int>(response_code));
                        if (HttpStatus_302_Found == response_code) {
                            if ( (curl_easy_getinfo(msg->easy_handle, CURLINFO_REDIRECT_URL, &url) == CURLE_OK) && url != nullptr) {
                                NMLOG_TRACE("captive portal found !!!");
                                captivePortalURI = url;
                            }
                        }
                    }
                }
                else
                    NMLOG_ERROR("endpoint = <%s> curl error = %d (%s)", endpoint, msg->data.result, curl_easy_strerror(msg->data.result));
                http_responses.push_back(response_code);
            }
            time_earlier = time_now;
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
                NMLOG_ERROR("curl_multi_poll returned %d (%s)", mc, curl_multi_strerror(mc));
                break;
            }
    #endif
        }

        if(curlVerboseEnabled()) {
            NMLOG_TRACE("endpoints count = %d response count %d, handles = %d, deadline = %ld, time_now = %ld, time_earlier = %ld",
                static_cast<int>(endpoints.size()), static_cast<int>(http_responses.size()), handles, deadline, time_now, time_earlier);
        }

        for (const auto& curl_easy_handle : curl_easy_handles)
        {
            curl_easy_getinfo(curl_easy_handle, CURLINFO_PRIVATE, &endpoint);
            //LOG_DBG("endpoint = <%s> terminating attempt", endpoint);
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

    nsm_internetState TestConnectivity::checkInternetStateFromResponseCode(const std::vector<int>& responses)
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
                    NMLOG_INFO("Internet State: FULLY_CONNECTED - %.1f%%", (percentage*100));
                break;
                case HttpStatus_200_OK:
                    InternetConnectionState = LIMITED_INTERNET;
                    NMLOG_INFO("Internet State: LIMITED_INTERNET - %.1f%%", (percentage*100));
                break;
                case HttpStatus_511_Authentication_Required:
                case HttpStatus_302_Found:
                    InternetConnectionState = CAPTIVE_PORTAL;
                    NMLOG_INFO("Internet State: CAPTIVE_PORTAL - %.1f%%", (percentage*100));
                break;
                default:
                    InternetConnectionState = NO_INTERNET;
                    if(http_response_code == -1)
                        NMLOG_ERROR("Internet State: NO_INTERNET (curl error)");
                    else
                        NMLOG_WARNING("Internet State: NO_INTERNET (http code: %d - %.1f%%)", static_cast<int>(http_response_code), percentage * 100);
                    break;
            }
        }
        return InternetConnectionState;
    }

    ConnectivityMonitor::ConnectivityMonitor()
    {
        if(endpointCache.isEndpointCashFileExist())
        {
            std::vector<std::string> cachedEndPnt = endpointCache.readEnpointsFromFile();
            setConnectivityMonitorEndpoints(cachedEndPnt);
            NMLOG_WARNING("cached connectivity endpoints loaded ..");
        }
        else
            connectivityMonitorEndpt.push_back("http://clients3.google.com/generate_204");

        doContinuousMonitor = false;
        doConnectivityMonitor = false;
        gInternetState = nsm_internetState::UNKNOWN;
        gIpv4InternetState = nsm_internetState::UNKNOWN;
        gIpv6InternetState = nsm_internetState::UNKNOWN;
        ginterfaceStatus = false;
    }

    ConnectivityMonitor::~ConnectivityMonitor()
    {
        NMLOG_WARNING("~ConnectivityMonitor");
        doContinuousMonitor = false;
        doConnectivityMonitor = false;
        cvConnectivityMonitor.notify_one();
        cvContinuousMonitor.notify_one();
        if (continuousMonitorThrd.joinable())
            continuousMonitorThrd.join();
        if (connectivityMonitorThrd.joinable())
            connectivityMonitorThrd.join();
    }

    std::vector<std::string> ConnectivityMonitor::getConnectivityMonitorEndpoints()
    {
        const std::lock_guard<std::mutex> lock(endpointMutex);
        std::vector<std::string> endpoints;
        for (auto endpoint : connectivityMonitorEndpt) {
            endpoints.push_back(endpoint);
        }
        return endpoints;
    }

    void ConnectivityMonitor::setConnectivityMonitorEndpoints(const std::vector<std::string> &endpoints)
    {
        const std::lock_guard<std::mutex> lock(endpointMutex);
        connectivityMonitorEndpt.clear();
        for (auto endpoint : endpoints) {
            if(!endpoint.empty() && endpoint.size() > 3)
                connectivityMonitorEndpt.push_back(endpoint.c_str());
            else
                NMLOG_ERROR("endpoint not vallied = %s", endpoint.c_str());
        }

        // write the endpoints to a file
        endpointCache.writeEnpointsToFile(connectivityMonitorEndpt);

        std::string endpointsStr;
        for (const auto& endpoint : connectivityMonitorEndpt)
            endpointsStr.append(endpoint).append(" ");
        NMLOG_INFO("Connectivity monitor endpoints -: %d :- %s", static_cast<int>(connectivityMonitorEndpt.size()), endpointsStr.c_str());
    }

    bool ConnectivityMonitor::isConnectedToInternet(nsm_ipversion ipversion)
    {
        if (nsm_internetState::FULLY_CONNECTED == getInternetState(ipversion))
        {
            NMLOG_INFO("isConnectedToInternet %s = true", (ipversion == nsm_ipversion::NSM_IPRESOLVE_WHATEVER)?"":(ipversion == nsm_ipversion::NSM_IPRESOLVE_V4? "IPv4":"IPv6"));
            return true;
        }
        NMLOG_WARNING("isConnectedToInternet %s = false",(ipversion == nsm_ipversion::NSM_IPRESOLVE_WHATEVER)?"":(ipversion == nsm_ipversion::NSM_IPRESOLVE_V4? "IPv4":"IPv6") );
        return false;
    }

    nsm_internetState ConnectivityMonitor::getInternetState(nsm_ipversion ipversion)
    {
        nsm_internetState internetState = nsm_internetState::UNKNOWN;
        // If monitor connectivity is running take the cache value

        if ( doContinuousMonitor && (nsm_ipversion::NSM_IPRESOLVE_V4 == ipversion || nsm_ipversion::NSM_IPRESOLVE_WHATEVER == ipversion)
                                           && gIpv4InternetState != nsm_internetState::UNKNOWN ) {
            NMLOG_WARNING("Reading Ipv4 internet state cached value %s", getInternetStateString(gIpv4InternetState));
            internetState = gIpv4InternetState;
        }
        else if ( doContinuousMonitor && (nsm_ipversion::NSM_IPRESOLVE_V6 == ipversion || nsm_ipversion::NSM_IPRESOLVE_WHATEVER == ipversion)
                                           && gIpv6InternetState != nsm_internetState::UNKNOWN ) {
            NMLOG_WARNING("Reading Ipv6 internet state cached value %s", getInternetStateString(gIpv6InternetState));
            internetState = gIpv6InternetState;
        }
        else
        {
            TestConnectivity testInternet(getConnectivityMonitorEndpoints(), NMCONNECTIVITY_CURL_REQUEST_TIMEOUT_MS, NMCONNECTIVITY_CURL_GET_REQUEST, ipversion);
            internetState = testInternet.getInternetState();
        }
        return internetState;
    }

    std::string ConnectivityMonitor::getCaptivePortalURI()
    {
        TestConnectivity testInternet(getConnectivityMonitorEndpoints(), NMCONNECTIVITY_CURL_REQUEST_TIMEOUT_MS, NMCONNECTIVITY_CURL_GET_REQUEST, NSM_IPRESOLVE_WHATEVER);
        if(nsm_internetState::CAPTIVE_PORTAL == testInternet.getInternetState())
        {
            NMLOG_WARNING("captive portal URI = %s", testInternet.getCaptivePortal().c_str());
            return testInternet.getCaptivePortal();
        }
        NMLOG_WARNING("No captive portal found !");
        return std::string("");
    }

    bool ConnectivityMonitor::startContinuousConnectivityMonitor(int timeoutInSeconds)
    {
        continuousMonitorTimeout.store(timeoutInSeconds >= NMCONNECTIVITY_MONITOR_MIN_INTERVAL ? timeoutInSeconds : NMCONNECTIVITY_MONITOR_DEFAULT_INTERVAL);
        if (doContinuousMonitor)
        {
            if(doConnectivityMonitor)
            {
                NMLOG_INFO("continuous monitor new timeout updated %d Sec", continuousMonitorTimeout.load());
            }
            else
            {
                NMLOG_INFO("continuous monitor restarted with %d Sec", continuousMonitorTimeout.load());
                cvContinuousMonitor.notify_one();
            }
            return true;
        }

        if (continuousMonitorThrd.joinable()) {
            NMLOG_WARNING("continuous monitor joinable thread running");
            doContinuousMonitor = false;
            continuousMonitorThrd.join();
        }

        doContinuousMonitor = true;
        continuousMonitorThrd = std::thread(&ConnectivityMonitor::continuousMonitorFunction, this);
        NMLOG_INFO("continuous connectivity monitor started with %d Sec", continuousMonitorTimeout.load());
        if(!continuousMonitorThrd.joinable()) {
            NMLOG_ERROR("continuous connectivity monitor start Failed");
            return false;
        }
        return true;
    }

    bool ConnectivityMonitor::stopContinuousConnectivityMonitor()
    {
        doContinuousMonitor = false;
        cvContinuousMonitor.notify_one();
        if (continuousMonitorThrd.joinable())
            continuousMonitorThrd.join();
        NMLOG_INFO("continuous connectivity monitor stopped");
        return true;
    }

    /*
     *
     * call startConnectivityMonitor function
     *  -->  when IP address accuired
     *  -->  when etherenet/wifi disconnected
     */
    bool ConnectivityMonitor::startConnectivityMonitor(bool interfaceStatus)
    {
        ginterfaceStatus = interfaceStatus; /* this will give interface status connected/ disconnected */
        gInternetState = UNKNOWN;
        gIpv4InternetState = nsm_internetState::UNKNOWN;
        gIpv6InternetState = nsm_internetState::UNKNOWN;
        if (doConnectivityMonitor)
        {
            cvConnectivityMonitor.notify_one();
            NMLOG_TRACE("trigger connectivity monitor thread");
            return true;
        }

        if (connectivityMonitorThrd.joinable()) { // cleanup of previous thread
            doConnectivityMonitor = false;
            connectivityMonitorThrd.join();
        }

        doConnectivityMonitor = true;
        connectivityMonitorThrd = std::thread(&ConnectivityMonitor::connectivityMonitorFunction, this);
        if(!connectivityMonitorThrd.joinable()) {
            NMLOG_ERROR("connectivity monitor start failed");
            return false;
        }

        NMLOG_ERROR("connectivity monitor started %d", NMCONNECTIVITY_MONITOR_MIN_INTERVAL);
        return true;
    }

    void ConnectivityMonitor::notifyInternetStatusChangedEvent(nsm_internetState newInternetState)
    {
        if(_instance != nullptr)
        {
            Exchange::INetworkManager::InternetStatus oldState = static_cast<Exchange::INetworkManager::InternetStatus>(gInternetState.load());
            Exchange::INetworkManager::InternetStatus newState = static_cast<Exchange::INetworkManager::InternetStatus>(newInternetState);
            _instance->ReportInternetStatusChangedEvent(oldState , newState);
            gInternetState = newInternetState;
        }
        else
            NMLOG_WARNING("NetworkManagerImplementation Instance NULL notifyInternetStatusChange failed.");
    }

    void ConnectivityMonitor::continuousMonitorFunction()
    {
        int TempInterval = continuousMonitorTimeout.load();
        std::mutex connMutex;
        nsm_ipversion ipResolveTyp = NSM_IPRESOLVE_WHATEVER;
        int notifyPreRetry = 1;
        nsm_internetState currentInternetState = nsm_internetState::UNKNOWN;

        do
        {
            if(doConnectivityMonitor)
            {
                NMLOG_TRACE("connectivity monitor running so skiping ccm check");
                gIpv4InternetState = UNKNOWN;
                gIpv6InternetState = UNKNOWN;
                std::unique_lock<std::mutex> lock(connMutex);
                cvContinuousMonitor.wait_for(lock, std::chrono::seconds(continuousMonitorTimeout.load()));
                ipResolveTyp = NSM_IPRESOLVE_WHATEVER; /* some interface change happense*/
                continue;
            }
            else if (ipResolveTyp == NSM_IPRESOLVE_WHATEVER)
            {
                nsm_internetState ipV4InternetState = UNKNOWN;
                nsm_internetState ipV6InternetState = UNKNOWN;
                auto curlCheckThrdIpv4 = [&]() {
                    TestConnectivity testInternet(getConnectivityMonitorEndpoints(), NMCONNECTIVITY_CURL_REQUEST_TIMEOUT_MS, 
                                                                                    NMCONNECTIVITY_CURL_GET_REQUEST, NSM_IPRESOLVE_V4);
                    ipV4InternetState = testInternet.getInternetState();
                };

                auto curlCheckThrdIpv6 = [&]() {
                    TestConnectivity testInternet(getConnectivityMonitorEndpoints(), NMCONNECTIVITY_CURL_REQUEST_TIMEOUT_MS, 
                                                                                        NMCONNECTIVITY_CURL_GET_REQUEST, NSM_IPRESOLVE_V6);
                    ipV6InternetState = testInternet.getInternetState();
                };
                std::thread ipv4thread(curlCheckThrdIpv4);
                std::thread ipv6thread(curlCheckThrdIpv6);
                // Wait for both threads to finish
                ipv4thread.join();
                ipv6thread.join();
                if(ipV4InternetState == FULLY_CONNECTED) {
                    ipResolveTyp = NSM_IPRESOLVE_V4;
                    currentInternetState = ipV4InternetState;
                    NMLOG_INFO("connectivity monitor default ip resolve IPV4");
                }
                else if(ipV6InternetState == FULLY_CONNECTED) {
                    ipResolveTyp = NSM_IPRESOLVE_V6;
                    currentInternetState = ipV6InternetState;
                    NMLOG_INFO("connectivity monitor default ip resolve IPV6");
                }
                else /* not changing ip resolve type */
                    currentInternetState = ipV4InternetState;
            }
            else /* IPV4 or IPV6 based on default values */
            {
                TestConnectivity testInternet(getConnectivityMonitorEndpoints(), NMCONNECTIVITY_CURL_REQUEST_TIMEOUT_MS,
                                                                                    NMCONNECTIVITY_CURL_HEAD_REQUEST, ipResolveTyp);
                currentInternetState = testInternet.getInternetState();
                if(ipResolveTyp == NSM_IPRESOLVE_V4)
                    gIpv4InternetState = currentInternetState;
                else if(ipResolveTyp == NSM_IPRESOLVE_V6)
                    gIpv6InternetState = currentInternetState;
            }

            if (currentInternetState == NO_INTERNET)
            {
                if(gInternetState == FULLY_CONNECTED && notifyPreRetry < NMCONNECTIVITY_NO_INTERNET_RETRY_COUNT)
                {
                    /* it will prevent posting notification */
                    currentInternetState = gInternetState;
                    TempInterval = 5;
                    NMLOG_INFO("No internet retrying connection check %d ...", notifyPreRetry);
                    notifyPreRetry++;
                    /* no internet state retry do it in ipv4 and ipv6 sepratly in two thread */
                    ipResolveTyp = NSM_IPRESOLVE_WHATEVER;
                }
                else
                {
                    notifyPreRetry = 1;
                    TempInterval = continuousMonitorTimeout.load();
                }
            }
            else {
                notifyPreRetry = 1;
                TempInterval = continuousMonitorTimeout.load();
            }

            if(gInternetState != currentInternetState)
            {
                NMLOG_INFO("Internet state changed to %s", getInternetStateString(currentInternetState));
                /* Notify Internet state change */
                notifyInternetStatusChangedEvent(currentInternetState);
            }

            //NMLOG_INFO("icm %d, ccm %d", doConnectivityMonitor.load(), doContinuousMonitor.load());
            if(!doContinuousMonitor)
                break;

            /* wait for next interval */
            std::unique_lock<std::mutex> lock(connMutex);
            if (cvContinuousMonitor.wait_for(lock, std::chrono::seconds(TempInterval)) != std::cv_status::timeout)
                NMLOG_INFO("continous connectivity monitor recieved signal. skping %d sec interval", TempInterval);

        } while(doContinuousMonitor);

        gIpv4InternetState = UNKNOWN;
        gIpv6InternetState = UNKNOWN;
        NMLOG_TRACE("continous connectivity monitor exit");
    }

    void ConnectivityMonitor::connectivityMonitorFunction()
    {
        int TempInterval = NMCONNECTIVITY_MONITOR_MIN_INTERVAL;
        std::mutex connMutex;
        bool notifyNow = true;
        int notifyPreRetry = 1;
        nsm_internetState currentInternetState = nsm_internetState::UNKNOWN;
        nsm_internetState tempInternetState = nsm_internetState::UNKNOWN;

        do
        {
            TestConnectivity testInternet(getConnectivityMonitorEndpoints(), NMCONNECTIVITY_CURL_REQUEST_TIMEOUT_MS, 
                                                                            NMCONNECTIVITY_CURL_GET_REQUEST, NSM_IPRESOLVE_WHATEVER);
            currentInternetState = testInternet.getInternetState();
            if(currentInternetState == CAPTIVE_PORTAL)
            {
                /* set to every 30 sec interval */
                TempInterval = NMCONNECTIVITY_CONN_MONITOR_RETRY_INTERVAL;
            }
            else if(currentInternetState == LIMITED_INTERNET)
            {
                TempInterval = NMCONNECTIVITY_CONN_MONITOR_RETRY_INTERVAL;
            }
            else // fullyconnect / noInternet
            {
                if(notifyPreRetry <= NMCONNECTIVITY_CONN_MONITOR_RETRY_COUNT - 1)
                {
                    if(tempInternetState != currentInternetState ) // check for continous same state
                    {
                        tempInternetState = currentInternetState;
                        notifyPreRetry = 1;
                        NMLOG_INFO("Connectivity check retrying %d ...", notifyPreRetry);
                    }
                    else
                    {
                        notifyPreRetry++;
                        NMLOG_INFO("Connectivity check retrying %d ...", notifyPreRetry);
                    }

                    if(gInternetState != UNKNOWN)
                        currentInternetState = gInternetState;
                    TempInterval = 5;
                }
                else if(tempInternetState != currentInternetState) // last state have change
                {
                    tempInternetState = currentInternetState;
                    notifyPreRetry = 1;
                    TempInterval = 5;
                    NMLOG_INFO("Connectivity check retrying %d ...", notifyPreRetry);
                }
                else
                {
                    if(currentInternetState == FULLY_CONNECTED)
                    {
                        doConnectivityMonitor = false;  // self exit
                        notifyNow = true; // post current state when retry complete
                    }
                    else if(ginterfaceStatus) // interface is active and still no internet, continue check every 30 sec
                    {
                        TempInterval = NMCONNECTIVITY_CONN_MONITOR_RETRY_INTERVAL;
                        /* notify if retry completed and state stil no internet state */
                        if(notifyPreRetry == NMCONNECTIVITY_CONN_MONITOR_RETRY_COUNT)
                        {
                            notifyPreRetry++;
                            notifyNow = true;
                        }
                    }
                    else // no interface connected
                    {
                        doConnectivityMonitor = false;
                        notifyNow = true;
                    }
                }
            }

            if(gInternetState != currentInternetState || notifyNow)
            {
                notifyNow = false;
                NMLOG_INFO("notify internet state %s", getInternetStateString(currentInternetState));
                notifyInternetStatusChangedEvent(currentInternetState);
            }

            if(!doConnectivityMonitor)
                break;
            /* wait for next interval */
            std::unique_lock<std::mutex> lock(connMutex);
            if (cvConnectivityMonitor.wait_for(lock, std::chrono::seconds(TempInterval)) != std::cv_status::timeout) {
                NMLOG_INFO("connectivity monitor recieved signal. skping %d sec interval", TempInterval);
                notifyPreRetry = 1;
                notifyNow = true;  // new signal came should notify in next check
            }
            
        } while(doConnectivityMonitor);

        if(!doContinuousMonitor)
            gInternetState = nsm_internetState::UNKNOWN; // no continous monitor running reset to unknow
        NMLOG_TRACE("initial connectivity monitor exit");
    }

    } // namespace Plugin
} // namespace WPEFramework
