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
#include <fstream>

#include "UtilsJsonRpc.h"

#define DEVICE_DIAGNOSTICS_METHOD_NAME_GET_CONFIGURATION  "getConfiguration"
#define DEVICE_DIAGNOSTICS_METHOD_GET_AV_DECODER_STATUS "getAVDecoderStatus"
#define DEVICE_DIAGNOSTICS_METHOD_GET_MILE_STONES "getMilestones"
#define DEVICE_DIAGNOSTICS_METHOD_LOG_MILESTONE "logMilestone"

#define DEVICE_DIAGNOSTICS_EVT_ON_AV_DECODER_STATUS_CHANGED "onAVDecoderStatusChanged"

#define MILESTONES_LOG_FILE                     "/opt/logs/rdk_milestones.log"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

enum SysSrv_ErrorCode {
    SysSrv_FileNotPresent,
    SysSrv_FileAccessFailed
};

/**
  * @brief : To map the error code with matching error message.
  * @param1[in] : error code of type SysSrv_ErrorCode.
  * @return : string; error message.
  */
std::string getErrorDescription(int errCode);

/***
 * @brief  : Used to read file contents into a vector
 * @param1[in] : Complete file name with path
 * @param2[in] : Destination vector buffer to be filled with file contents
 * @return : <bool>; TRUE if operation success; else FALSE.
 */
bool getFileContent(std::string fileName, std::vector<std::string> & vecOfStrs);

/***
 * @brief  : Used to construct JSON response from Vector.
 * @param1[in] : Destination JSON response buffer
 * @param2[in] : JSON "Key"
 * @param3[in] : Source Vector.
 * @return : <bool>; TRUE if operation success; else FALSE.
 */
void setJSONResponseArray(JsonObject& response, const char* key,
        const std::vector<string>& items);

/***
 * @brief   : Used to construct response with module error status.
 * @param1[in]  : Error Code
 * @param2[out]: "response" JSON Object which is returned by the API
   with updated module error status.
 */
void populateResponseWithError(int errorCode, JsonObject& response);

namespace WPEFramework
{
    namespace {

        static Plugin::Metadata<Plugin::DeviceDiagnostics> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin
    {
        SERVICE_REGISTRATION(DeviceDiagnostics, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

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
        : PluginHost::JSONRPC()
        {
            DeviceDiagnostics::_instance = this;

            Register(DEVICE_DIAGNOSTICS_METHOD_NAME_GET_CONFIGURATION, &DeviceDiagnostics::getConfigurationWrapper, this);
            Register(DEVICE_DIAGNOSTICS_METHOD_GET_AV_DECODER_STATUS, &DeviceDiagnostics::getAVDecoderStatus, this);
	    Register(DEVICE_DIAGNOSTICS_METHOD_GET_MILE_STONES, &DeviceDiagnostics::getMilestones, this);
            Register(DEVICE_DIAGNOSTICS_METHOD_LOG_MILESTONE, &DeviceDiagnostics::logMilestones, this);
        }

        DeviceDiagnostics::~DeviceDiagnostics()
        {
            Unregister(DEVICE_DIAGNOSTICS_METHOD_NAME_GET_CONFIGURATION);
            Unregister(DEVICE_DIAGNOSTICS_METHOD_GET_AV_DECODER_STATUS);
	    Unregister(DEVICE_DIAGNOSTICS_METHOD_GET_MILE_STONES);
            Unregister(DEVICE_DIAGNOSTICS_METHOD_LOG_MILESTONE);
        }

        /* virtual */ const string DeviceDiagnostics::Initialize(PluginHost::IShell* service)
        {
#ifdef ENABLE_ERM
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

        string DeviceDiagnostics::Information() const
        {
            return (string());
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

	 /***
         * @brief : To fetch the list of milestones.
         * @param1[in]  : {params":{}}
         * @param2[out] : "result":{"milestones":["<string>","<string>","<string>"],
         *      "success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t DeviceDiagnostics::getMilestones(const JsonObject& parameters,
                JsonObject& response)
        {
            bool retAPIStatus = false;
	    std::vector<string> milestones;

            if (Core::File(string(MILESTONES_LOG_FILE)).Exists()) {
                retAPIStatus = getFileContent(MILESTONES_LOG_FILE, milestones);
                if (retAPIStatus) {
                    setJSONResponseArray(response, "milestones", milestones);
                } else {
                    populateResponseWithError(SysSrv_FileAccessFailed, response);
                }
            } else {
                populateResponseWithError(SysSrv_FileNotPresent, response);
            }
            returnResponse(retAPIStatus);
        }

      /**
       * @brief Logs marker to rdk milestones log file
       *
       * @param[in]  parameters   - Must include 'marker'.
       * @param[out] response     - Success.
       *
       * @return                  A code indicating success.
       */
       uint32_t DeviceDiagnostics::logMilestones(const JsonObject& parameters, JsonObject& response)
       {
           LOGINFOMETHOD();
           returnIfStringParamNotFound(parameters, "marker");
           std::string marker = parameters["marker"].String();
 #ifdef RDK_LOG_MILESTONE
           logMilestone(marker.c_str());
 #endif
           returnResponse(true);
       }

    } // namespace Plugin
} // namespace WPEFramework

std::map<int, std::string> ErrCodeMap = {
    {SysSrv_FileNotPresent, "Expected file not found"},
    {SysSrv_FileAccessFailed, "File access failed"}
};

std::string getErrorDescription(int errCode)
{
    std::string errMsg = "Unexpected Error";

    auto it = ErrCodeMap.find(errCode);
    if (ErrCodeMap.end() != it) {
        errMsg = it->second;
    }
    return errMsg;
}

/***
 * @brief	: Used to read file contents into a vector
 * @param1[in]	: Complete file name with path
 * @param2[in]	: Destination vector buffer to be filled with file contents
 * @return	: <bool>; TRUE if operation success; else FALSE.
 */
bool getFileContent(std::string fileName, std::vector<std::string> & vecOfStrs)
{
    bool retStatus = false;
    std::ifstream inFile(fileName.c_str(), std::ios::in);

    if (!inFile.is_open())
        return retStatus;

    std::string line;
    retStatus = true;
    while (std::getline(inFile, line)) {
        if (line.size() > 0) {
            vecOfStrs.push_back(line);
        }
    }
    inFile.close();
    return retStatus;
}

/***
 * @brief	: Used to construct JSON response from Vector.
 * @param1[in]	: Destination JSON response buffer
 * @param2[in]	: JSON "Key"
 * @param3[in]	: Source Vector.
 * @return	: <bool>; TRUE if operation success; else FALSE.
 */
void setJSONResponseArray(JsonObject& response, const char* key,
        const std::vector<string>& items)
{
    JsonArray arr;

    for (auto& i : items) {
        arr.Add(JsonValue(i));
    }
    response[key] = arr;
}

/***
 * @brief	: Used to construct response with module error status.
 * @param1[in]	: Error Code
 * @param2[out]: "response" JSON Object which is returned by the API
   with updated module error status.
 */
void populateResponseWithError(int errorCode, JsonObject& response)
{
     if (errorCode) {
        LOGWARN("Method %s failed; reason : %s\n", __FUNCTION__,
        getErrorDescription(errorCode).c_str());
        response["SysSrv_Status"] = static_cast<uint32_t>(errorCode);
        response["errorMessage"] = getErrorDescription(errorCode);
     }
}
