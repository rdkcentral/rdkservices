/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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

#include "DeviceDiagnostics.h"

#include <curl/curl.h>
#include <time.h>

#include "utils.h"

#define DEVICE_DIAGNOSTICS_METHOD_NAME_GET_CONFIGURATION  "getConfiguration"
#define DEVICE_DIAGNOSTICS_METHOD_GET_AV_DECODER_STATUS "getAVDecoderStatus"

#define DEVICE_DIAGNOSTICS_EVT_ON_AV_DECODER_STATUS_CHANGED "onAVDecoderStatusChanged"

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(DeviceDiagnostics, 1, 0);

        DeviceDiagnostics* DeviceDiagnostics::_instance = nullptr;

        const int curlTimeoutInSeconds = 30;
        static const char *decoderStatusStr[] = {
            "IDLE",
            "PAUSED",
            "ACTIVE",
            NULL
        };

        static size_t writeCurlResponse(void *ptr, size_t size, size_t nmemb, std::string stream)
        {
          size_t realsize = size * nmemb;
          std::string temp(static_cast<const char*>(ptr), realsize);
          stream.append(temp);
          return realsize;
        }

        DeviceDiagnostics::DeviceDiagnostics()
        : AbstractPlugin()
        {
            DeviceDiagnostics::_instance = this;

            registerMethod(DEVICE_DIAGNOSTICS_METHOD_NAME_GET_CONFIGURATION, &DeviceDiagnostics::getConfigurationWrapper, this);
            registerMethod(DEVICE_DIAGNOSTICS_METHOD_GET_AV_DECODER_STATUS, &DeviceDiagnostics::getAVDecoderStatus, this);
        }

        DeviceDiagnostics::~DeviceDiagnostics()
        {
        }

        /* virtual */ const string DeviceDiagnostics::Initialize(PluginHost::IShell* service)
        {
#ifdef ENABLE_ERM
            int ret;

            if ((m_EssRMgr = EssRMgrCreate()) == NULL)
            {
                LOGERR("EssRMgrCreate() failed");
                return "EssRMgrCreate() failed";
            }

            m_pollThreadRun = 1;
            m_AVPollThread = std::thread(AVPollThread, this);
#else
            LOGWARN("ENABLE_ERM is not defined, decoder status will "
                    "always be reported as IDLE");
#endif

            return "";
        }

        void DeviceDiagnostics::Deinitialize(PluginHost::IShell* /* service */)
        {
#ifdef ENABLE_ERM
            m_AVDecoderStatusLock.lock();
            m_pollThreadRun = 0;
            m_AVDecoderStatusLock.unlock();
            m_AVPollThread.join();
            EssRMgrDestroy(m_EssRMgr);
#endif
            DeviceDiagnostics::_instance = nullptr;
        }

        uint32_t DeviceDiagnostics::getConfigurationWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            JsonArray names = parameters["names"].Array();

            JsonObject requestParams;
            JsonArray namePairs;

            JsonArray::Iterator index(names.Elements());

            while (index.Next() == true)
            {
                if (Core::JSON::Variant::type::STRING == index.Current().Content())
                {
                    //JSON::String s = index.Current().String();
                    JsonObject o;
                    o["name"] = index.Current().String();
                    namePairs.Add(o);
                }
                else
                    LOGWARN("Unexpected variant type");
            }

            requestParams["paramList"] = namePairs;

            string json;
            requestParams.ToString(json);

            if (0 == getConfiguration(json, response))
                returnResponse(true);

            returnResponse(false);
        }

        /* retrieves most active decoder status from ERM library,
         * this library keeps state of all decoders and will give
         * us only the most active status of any decoder */
        int DeviceDiagnostics::getMostActiveDecoderStatus()
        {
            int status = 0;

#ifdef ENABLE_ERM
            EssRMgrGetAVState(m_EssRMgr, &status);
#endif
            return status;
        }

        /* periodically polls ERM library for changes in most
         * active decoder and send thunder event when decoder
         * status changes. Needs to be done via poll and separate
         * thread because ERM doesn't support events. */
#ifdef ENABLE_ERM
        void *DeviceDiagnostics::AVPollThread(void *arg)
        {
            struct timespec poll_wait = { .tv_sec = 30, .tv_nsec = 0  };
            int lastStatus = EssRMgrRes_idle;
            int status;
            DeviceDiagnostics* t = DeviceDiagnostics::_instance;

            LOGINFO("AVPollThread started");
            for (;;)
            {
                nanosleep(&poll_wait, NULL);
                std::unique_lock<std::mutex> lock(t->m_AVDecoderStatusLock);
                if (t->m_pollThreadRun == 0)
                    break;

                status = t->getMostActiveDecoderStatus();
                lock.unlock();

                if (status == lastStatus)
                    continue;

                lastStatus = status;
                t->onDecoderStatusChange(status);
            }

            return NULL;
        }
#endif

        void DeviceDiagnostics::onDecoderStatusChange(int status)
        {
            JsonObject params;
            params["avDecoderStatusChange"] = decoderStatusStr[status];
            sendNotify(DEVICE_DIAGNOSTICS_EVT_ON_AV_DECODER_STATUS_CHANGED, params);
        }

        uint32_t DeviceDiagnostics::getAVDecoderStatus(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
#ifdef ENABLE_ERM
            m_AVDecoderStatusLock.lock();
            int status = getMostActiveDecoderStatus();
            m_AVDecoderStatusLock.unlock();
            response["avDecoderStatus"] = decoderStatusStr[status];
#else
            response["avDecoderStatus"] = decoderStatusStr[0];
#endif
            returnResponse(true);
        }

        int DeviceDiagnostics::getConfiguration(const std::string& postData, JsonObject& out)
        {
            LOGINFO("%s",__FUNCTION__);

            JsonObject DeviceDiagnosticsResult;
            int result = -1;

            long http_code = 0;
            std::string response;
            CURL *curl_handle = NULL;
            CURLcode res = CURLE_OK;
            curl_handle = curl_easy_init();

            LOGINFO("data: %s", postData.c_str());

            if (curl_handle) {

                curl_easy_setopt(curl_handle, CURLOPT_URL, "http://127.0.0.1:10999");
                curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postData.c_str());
                curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, postData.size());
                curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1); //when redirected, follow the redirections
                curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeCurlResponse);
                curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response);
                curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, curlTimeoutInSeconds);

                res = curl_easy_perform(curl_handle);
                curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);

                LOGWARN("Perfomed curl call : %d http response code: %ld", res, http_code);
                curl_easy_cleanup(curl_handle);
            }
            else {
                LOGWARN("Could not perform curl ");
            }

            if (res == CURLE_OK && (http_code == 0 || http_code == 200))
             {
                 LOGWARN("curl Response: %s", response.c_str());

                 JsonObject jsonHash;
                 jsonHash.FromString(response);

                 if (jsonHash.HasLabel("paramList"))
                 {
                    LOGWARN("key paramList present");
                    out["paramList"] = jsonHash["paramList"];
                    result = 0;
                 }
             }
            return result;
        }


    } // namespace Plugin
} // namespace WPEFramework


