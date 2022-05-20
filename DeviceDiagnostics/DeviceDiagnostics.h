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

#pragma once

#include <thread>
#include <mutex>
#ifdef ENABLE_ERM
#include <essos-resmgr.h>
#endif

#include "Module.h"

#define MILESTONES_LOG_FILE                     "/opt/logs/rdk_milestones.log"

enum SysSrv_ErrorCode {
    SysSrv_FileNotPresent,
    SysSrv_FileAccessFailed
};

namespace WPEFramework {

    namespace Plugin {
        class DeviceDiagnostics : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:
            DeviceDiagnostics(const DeviceDiagnostics&) = delete;
            DeviceDiagnostics& operator=(const DeviceDiagnostics&) = delete;

            //Begin methods
            uint32_t getConfigurationWrapper(const JsonObject& parameters, JsonObject& response);
	    uint32_t getMilestones(const JsonObject& parameters, JsonObject& response);
            //End methods

            int getConfiguration(const std::string& postData, JsonObject& response);
            uint32_t getAVDecoderStatus(const JsonObject& parameters, JsonObject& response);
            int getMostActiveDecoderStatus();
            void onDecoderStatusChange(int status);
#ifdef ENABLE_ERM
            static void *AVPollThread(void *arg);
#endif

        private:
#ifdef ENABLE_ERM
            std::thread m_AVPollThread;
            std::mutex m_AVDecoderStatusLock;
            EssRMgr* m_EssRMgr;
            int m_pollThreadRun;
#endif

        public:
            DeviceDiagnostics();
            virtual ~DeviceDiagnostics();
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual string Information() const override;

            BEGIN_INTERFACE_MAP(MODULE_NAME)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        public:
            static DeviceDiagnostics* _instance;

        };
	} // namespace Plugin
} // namespace WPEFramework

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
        const vector<string>& items);

/***
 * @brief   : Used to construct response with module error status.
 * @param1[in]  : Error Code
 * @param2[out]: "response" JSON Object which is returned by the API
   with updated module error status.
 */
void populateResponseWithError(int errorCode, JsonObject& response);

