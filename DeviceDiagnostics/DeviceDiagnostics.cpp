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
#include <libIBus.h>

#include "utils.h"

#define DEVICE_DIAGNOSTICS_METHOD_NAME_GET_CONFIGURATION  "getConfiguration"
#define DEVICE_DIAGNOSTICS_METHOD_GET_VIDEO_DECODER_STATUS "getVideoDecoderStatus"
#define DEVICE_DIAGNOSTICS_METHOD_GET_AUDIO_DECODER_STATUS "getAudioDecoderStatus"

#define DEVICE_DIAGNOSTICS_EVT_ON_VIDEO_DECODER_STATUS_CHANGED "onVideoDecoderStatusChanged"
#define DEVICE_DIAGNOSTICS_EVT_ON_AUDIO_DECODER_STATUS_CHANGED "onAudioDecoderStatusChanged"

#define EVT_ON_VIDEO_DECODER_STATUS_CHANGED "onVideoDecoderStatusChanged"
#define EVT_ON_AUDIO_DECODER_STATUS_CHANGED "onAudioDecoderStatusChanged"

#define IARM_BUS_PLAYBACK_DIAG_STATUS_CHANGE_EVENT 200
#define IARM_BUS_PLAYBACK_DIAG_BUS_NAME "PlaybackDiag"

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(DeviceDiagnostics, 1, 0);

        DeviceDiagnostics* DeviceDiagnostics::_instance = nullptr;

        const int curlTimeoutInSeconds = 30;
        static const char *decoderStatusStr[] = {
            "ACTIVE",
            "PAUSED",
            "IDLE",
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
            registerMethod(DEVICE_DIAGNOSTICS_METHOD_GET_VIDEO_DECODER_STATUS, &DeviceDiagnostics::getVideoDecoderStatus, this);
            registerMethod(DEVICE_DIAGNOSTICS_METHOD_GET_AUDIO_DECODER_STATUS, &DeviceDiagnostics::getAudioDecoderStatus, this);
        }

        DeviceDiagnostics::~DeviceDiagnostics()
        {
        }

        /* virtual */ const string DeviceDiagnostics::Initialize(PluginHost::IShell* service)
        {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_PLAYBACK_DIAG_BUS_NAME,
                            IARM_BUS_PLAYBACK_DIAG_STATUS_CHANGE_EVENT,
                            DeviceDiagnostics::decoderStatusHandler));
            }

            return "";
        }

        void DeviceDiagnostics::Deinitialize(PluginHost::IShell* /* service */)
        {
            DeviceDiagnostics::_instance = nullptr;

            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_PLAYBACK_DIAG_BUS_NAME,
                            IARM_BUS_PLAYBACK_DIAG_STATUS_CHANGE_EVENT));
            }

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
        /* Searches m_{video,audio}DecoderStatus for most active decoder.
         * Most active status is "Active" followed by "Paused" and then
         * "Idle".
         *
         * decoderName can be "video" or "audio"
         *
         * When there are no decoders in map, IDLE state will be returned.
         */
        DeviceDiagnostics::DecoderStatus DeviceDiagnostics::getMostActiveDecoderStatus(const std::string &decoderName)
        {
            std::unordered_map<std::string, DecoderStatusInfo> *decoderStatus;
            DecoderStatus mostActiveStatus = DECODER_STATUS_IDLE;

            if (decoderName == "video")
                decoderStatus = &m_videoDecoderStatus;
            else
                decoderStatus = &m_audioDecoderStatus;

            for (auto const &status: *decoderStatus)
                if (status.second.status < mostActiveStatus)
                    mostActiveStatus = status.second.status;

            return mostActiveStatus;
        }

        /* Called each time IARM event with decoder status is received from
         * gstreamer.
         *
         * This function will create new, update or delete entry in
         * m_{video,audio}DecoderStatus, depending on received state
         * of decoder. If we update decoder that was most recently
         * read by user via "getDecoderStatus" api, event is emitted
         * to notify user about decoder status change.
         */
        void DeviceDiagnostics::decoderStatusHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            typedef struct _IARM_BUS_Diag_EventData_t {
                char pipeline_id[64];
                char pipeline_name[64];
                char decoder[16];
                char status[16];
                char action[16];
            } IARM_Bus_Diag_EventData_t;

            IARM_Bus_Diag_EventData_t *eventData = (IARM_Bus_Diag_EventData_t *)data;
            struct DecoderStatusInfo decoderInfo;
            std::unordered_map<std::string, DecoderStatusInfo> *decoderStatus;
            DecoderStatus mostActiveStatus;
            DecoderStatus prevMostActiveStatus;

            LOGINFO("new decoder status received");
            LOGINFO("eventData->pipeline_id: %s", eventData->pipeline_id);
            LOGINFO("eventData->pipeline_name: %s", eventData->pipeline_name);
            LOGINFO("eventData->decoder: %s", eventData->decoder);
            LOGINFO("eventData->status: %s", eventData->status);
            LOGINFO("eventData->action: %s", eventData->action);

            DeviceDiagnostics* t = DeviceDiagnostics::_instance;
            if (t == nullptr)
                return;

            if (strcmp(eventData->decoder, "video") == 0) {
                decoderStatus = &t->m_videoDecoderStatus;
            } else if (strcmp(eventData->decoder, "audio") == 0) {
                decoderStatus = &t->m_audioDecoderStatus;
            } else {
                LOGERR("invalid decoder '%s' received on IARM, ignoring event",
                        eventData->decoder);
                return;
            }

            if (strcmp(eventData->action, "DELETE") == 0)
            {
                /* remove this decoder from map */
                decoderStatus->erase(std::string(eventData->pipeline_id));
                LOGINFO("decoder removed");
                return;
            }

            if (strcmp(eventData->action, "UPDATE") == 0) {
                if (decoderStatus->find(eventData->pipeline_id) == decoderStatus->end()) {
                    /* trying to update nonexisting pipeline,
                     * according to RDK-31097 that's an error
                     */
                    LOGERR("decoder id '%s' does not exist and action is UPDATE; "
                            "decoder status ignored", eventData->pipeline_id);
                    return;
                }
            }

            decoderInfo.pipeName = eventData->pipeline_name;

            if (strcmp(eventData->status, "IDLE") == 0)
                decoderInfo.status = DECODER_STATUS_IDLE;
            else if (strcmp(eventData->status, "PAUSED") == 0)
                decoderInfo.status = DECODER_STATUS_PAUSED;
            else if (strcmp(eventData->status, "ACTIVE") == 0)
                decoderInfo.status = DECODER_STATUS_ACTIVE;
            else
            {
                LOGERR("invalid decoder status '%s' for pipeline '%s'",
                        eventData->status, eventData->pipeline_name);
                return;
            }

            /* add/update map with new state */
            prevMostActiveStatus = t->getMostActiveDecoderStatus(eventData->decoder);
            (*decoderStatus)[std::string(eventData->pipeline_id)] = decoderInfo;
            mostActiveStatus = t->getMostActiveDecoderStatus(eventData->decoder);
            /* send notify only when most active state has changed,
             * ie, when both decoders are ACTIVE and one of them goes
             * to IDLE, notification will not be send since most
             * active decoder is still ACTIVE */
            if (prevMostActiveStatus != mostActiveStatus)
                t->onDecoderStatusChange(eventData->decoder, eventData->status);
            LOGINFO("decoder updated successfully");
        }

        void DeviceDiagnostics::onDecoderStatusChange(const std::string &decoder, const std::string &status)
        {
            JsonObject params;
            if (decoder == "video") {
                params["videoDecoderStatusChange"] = status;
                sendNotify(EVT_ON_VIDEO_DECODER_STATUS_CHANGED, params);
            } else {
                params["audioDecoderStatusChange"] = status;
                sendNotify(EVT_ON_AUDIO_DECODER_STATUS_CHANGED, params);
            }
        }

        uint32_t DeviceDiagnostics::getVideoDecoderStatus(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            DecoderStatus status = getMostActiveDecoderStatus("video");
            response["videoDecoderStatus"] = decoderStatusStr[status];
            returnResponse(true);
        }

        uint32_t DeviceDiagnostics::getAudioDecoderStatus(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            DecoderStatus status = getMostActiveDecoderStatus("audio");
            response["audioDecoderStatus"] = decoderStatusStr[status];
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


