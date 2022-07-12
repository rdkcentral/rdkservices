/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/


#include <algorithm>
#include <regex>
#undef LOG // we don't need LOG from audiocapturemgr_iarm as we are defining our own LOG
#include "DataCapture.h"
#include <curl/curl.h>
#include "socket_adaptor.h"

#include "UtilsCStr.h"
#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

const string WPEFramework::Plugin::DataCapture::SERVICE_NAME = "org.rdk.DataCapture";
const string WPEFramework::Plugin::DataCapture::METHOD_ENABLE_AUDIO_CAPTURE = "enableAudioCapture";
const string WPEFramework::Plugin::DataCapture::METHOD_GET_AUDIO_CLIP = "getAudioClip";
const string WPEFramework::Plugin::DataCapture::EVT_ON_AUDIO_CLIP_READY = "onAudioClipReady";
pthread_mutex_t WPEFramework::Plugin::DataCapture::_mutex = PTHREAD_MUTEX_INITIALIZER;

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

using namespace std;
using namespace audiocapturemgr;

namespace WPEFramework {

    namespace {

        static Plugin::Metadata<Plugin::DataCapture> metadata(
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
    
    namespace Plugin {

        static bool verify_result(IARM_Result_t ret, iarmbus_acm_arg_t &param)
        {
            if(IARM_RESULT_SUCCESS != ret)
            {
                LOGERR("Bus call failed.");
                return false;
            }
            if(0 != param.result)
            {
                LOGERR("ACM implementation of the bus call returned failure.");
                return false;
            }
            return true;
        }

        static void cleanup_samples()
        {
            string path(AUDIOCAPTUREMGR_FILE_PATH AUDIOCAPTUREMGR_FILENAME_PREFIX "*");
            LOGINFO("by path %s", C_STR(path));

            Core::Directory dir(C_STR(path));
            while (dir.Next()) Core::File(AUDIOCAPTUREMGR_FILE_PATH + dir.Name()).Destroy();
        }

        SERVICE_REGISTRATION(DataCapture, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        DataCapture* DataCapture::_instance = nullptr;

        DataCapture::DataCapture()
            : PluginHost::JSONRPC()
            , _session_id(-1)
            , _max_supported_duration(0)
            , _is_precapture(false)
            , _duration(0)
        {
            LOGINFO("ctor");

            cleanup_samples();

            DataCapture::_instance = this;
            Register(METHOD_ENABLE_AUDIO_CAPTURE, &DataCapture::enableAudioCaptureWrapper, this);
            Register(METHOD_GET_AUDIO_CLIP, &DataCapture::getAudioClipWrapper, this);

            _sock_adaptor.reset(new socket_adaptor());
        }

        DataCapture::~DataCapture()
        {
            //LOGINFO("dtor");
            Unregister(METHOD_ENABLE_AUDIO_CAPTURE);
            Unregister(METHOD_GET_AUDIO_CLIP);
        }

        const string DataCapture::Initialize(PluginHost::IShell* /* service */)
        {
            InitializeIARM();
            return "";
        }

        void DataCapture::Deinitialize(PluginHost::IShell* /* service */)
        {
            DeinitializeIARM();
            DataCapture::_instance = nullptr;
        }

        string DataCapture::Information() const
        {
            return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
        }

        void DataCapture::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARMBUS_AUDIOCAPTUREMGR_NAME, DATA_CAPTURE_IARM_EVENT_AUDIO_CLIP_READY, iarmEventHandler));
            }
        }

        void DataCapture::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARMBUS_AUDIOCAPTUREMGR_NAME, DATA_CAPTURE_IARM_EVENT_AUDIO_CLIP_READY));
            }
        }

        // Registered methods begin
        uint32_t DataCapture::enableAudioCaptureWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            returnIfNumberParamNotFound(parameters, "bufferMaxDuration");

            auto bufferMaxDuration = (unsigned int)parameters["bufferMaxDuration"].Number();

            int ret = enableAudioCapture(bufferMaxDuration);
            response["error"] = ret;
            returnResponse(0 == ret);
        }

        uint32_t DataCapture::getAudioClipWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "clipRequest");

            Core::JSON::VariantContainer clipRequest;
            if (parameters["clipRequest"].Content() == Core::JSON::Variant::type::STRING)
            {
                string clipRequestJSONString = parameters["clipRequest"].String();
                replace(clipRequestJSONString.begin(), clipRequestJSONString.end(), '\'', '\"');

                Core::OptionalType<Core::JSON::Error> error;
                LOGINFO("Try parse '%s' as JSON string", clipRequestJSONString.c_str());
                if (!clipRequest.FromString(clipRequestJSONString, error))
                {
                    LOGERR("Failed parse JSON string from 'clipRequest': %s", error.Value().Message().c_str());
                    returnResponse(false);
                }
                LOGWARN("Arguments received as string JSON object: %s", clipRequestJSONString.c_str());
            }
            else if (parameters["clipRequest"].Content() != Core::JSON::Variant::type::OBJECT)
            {
                LOGERR("clipRequest' should have type STRING or OBJECT");
                returnResponse(false);
            }
            else
            {
                clipRequest = parameters["clipRequest"].Object();
                LOGINFO("Arguments received as parsed hash.");
            }

            returnIfStringParamNotFound(clipRequest, "stream");
            returnIfStringParamNotFound(clipRequest, "url");
            returnIfNumberParamNotFound(clipRequest, "duration");
            returnIfStringParamNotFound(clipRequest, "captureMode");

            int ret = getAudioClip(clipRequest);
            response["error"] = ret;
            returnResponse(0 == ret);
        }
        // Registered methods end

        // Internal methods begin
        int DataCapture::enableAudioCapture(unsigned int bufferMaxDuration)
        {

            LOGINFO("DataCaptureService calling enableAudioCapture: bufferMaxDuration = %d", (int)bufferMaxDuration);

            IARM_Result_t ret;
            auto incoming_duration = static_cast <unsigned int> (bufferMaxDuration);

            /*Are we being asked to turn this off?*/
            if(0 == incoming_duration)
            {
                /* Turn off whatever session you have going on.*/
                if(0 < _session_id)
                {
                    iarmbus_acm_arg_t param;
                    param.session_id = _session_id;

                    ret = IARM_Bus_Call(IARMBUS_AUDIOCAPTUREMGR_NAME, IARMBUS_AUDIOCAPTUREMGR_STOP, (void *) &param, sizeof(param));
                    if(IARM_RESULT_SUCCESS != ret)
                    {
                        LOGERR("Failed to stop audiocapturemgr session.");
                    }
                    ret = IARM_Bus_Call(IARMBUS_AUDIOCAPTUREMGR_NAME, IARMBUS_AUDIOCAPTUREMGR_CLOSE, (void *) &param, sizeof(param));
                    if(IARM_RESULT_SUCCESS != ret)
                    {
                        LOGERR("Failed to close audiocapturemgr session.");
                    }
                    _session_id = -1;
                    return ret;
                }
            }

            /*Open session*/
            if(0 > _session_id)
            {
                iarmbus_acm_arg_t param;
                param.details.arg_open.source = 0; //primary
                param.details.arg_open.output_type = BUFFERED_FILE_OUTPUT;
                ret = IARM_Bus_Call(IARMBUS_AUDIOCAPTUREMGR_NAME, IARMBUS_AUDIOCAPTUREMGR_OPEN, (void *) &param, sizeof(param));
                if(!verify_result(ret, param))
                {
                    return ACM_RESULT_PRECAPTURE_NOT_SUPPORTED;
                }
                _session_id = param.session_id;
            }

            /*Gather max duration information.*/
            {
                iarmbus_acm_arg_t param;
                param.session_id = _session_id;
                ret = IARM_Bus_Call(IARMBUS_AUDIOCAPTUREMGR_NAME, IARMBUS_AUDIOCAPTUREMGR_GET_OUTPUT_PROPS, (void *) &param, sizeof(param));
                if(!verify_result(ret, param))
                {
                    LOGWARN("Unable to read max duration. Setting safe limit of 10");
                    _max_supported_duration = 10;
                }
                else
                {
                    _max_supported_duration = param.details.arg_output_props.output.max_buffer_duration;
                }
            }

            if(_max_supported_duration < incoming_duration)
            {
                LOGERR("Incoming precapture duration is too big.");
                return (int)_max_supported_duration; // return max supported duration.
            }

            /*Set precapture length*/
            {
                iarmbus_acm_arg_t param;
                param.session_id = _session_id;
                param.details.arg_output_props.output.buffer_duration = incoming_duration;
                ret = IARM_Bus_Call(IARMBUS_AUDIOCAPTUREMGR_NAME, IARMBUS_AUDIOCAPTUREMGR_SET_OUTPUT_PROPERTIES, (void *) &param, sizeof(param));
                if(!verify_result(ret, param))
                {
                    LOGERR("Failed to set precature duration.");
                    return ACM_RESULT_GENERAL_FAILURE;
                }
            }

            /*Get format of incoming sample */
            {
                iarmbus_acm_arg_t param;
                param.session_id = _session_id;
                ret = IARM_Bus_Call(IARMBUS_AUDIOCAPTUREMGR_NAME, IARMBUS_AUDIOCAPTUREMGR_GET_AUDIO_PROPS, (void *) &param, sizeof(param));
                if(!verify_result(ret, param))
                {
                    LOGERR("Failed to get output properties.");
                    return ACM_RESULT_GENERAL_FAILURE;
                }
                else
                {
                    _audio_properties = param.details.arg_audio_properties;
                    constructFormatString();
                }
            }

            /*Start buffering*/
            {
                iarmbus_acm_arg_t param;
                param.session_id = _session_id ;
                ret = IARM_Bus_Call(IARMBUS_AUDIOCAPTUREMGR_NAME, IARMBUS_AUDIOCAPTUREMGR_START, (void *) &param, sizeof(param));
                if(!verify_result(ret, param))
                {
                    LOGERR("Failed to start audiocapture session");
                    return ACM_RESULT_GENERAL_FAILURE;
                }
                return param.result;
            }
        }

        int DataCapture::getAudioClip(const JsonObject& clipRequest)
        {
            const string& stream = clipRequest["stream"].String();
            _destination_url = clipRequest["url"].String();
            _duration = (unsigned int)clipRequest["duration"].Number();
            const string& captureMode = clipRequest["captureMode"].String();
            _is_precapture = (captureMode == "preCapture");

            LOGINFO("DataCaptureService calling getAudioClip: stream = %s, url = %s, duration = %d, captureMode = %s, session id = %d",
                         stream.c_str(), _destination_url.c_str(), _duration, captureMode.c_str(), _session_id);

            if(0 > _session_id)
            {
                LOGERR("Audio capture not enabled.");
                return ACM_RESULT_GENERAL_FAILURE;
            }

            if(stream != "primary")
            {
                LOGERR("Error! Audiocapture supports only primary audio.");
                return ACM_RESULT_STREAM_UNAVAILABLE;
            }

            iarmbus_acm_arg_t param;
            param.session_id = _session_id;
            param.details.arg_sample_request.duration = (float)_duration;
            param.details.arg_sample_request.is_precapture = _is_precapture;

            /*For post-capture, verify duration is within bounds. For precapture, duration was fixed earlier*/
            if((!param.details.arg_sample_request.is_precapture) && (_max_supported_duration < _duration))
            {
                LOGERR("Capture duration is too big.");
                return ACM_RESULT_DURATION_OUT_OF_BOUNDS; // return max supported duration.
            }

            IARM_Result_t ret = IARM_Bus_Call(IARMBUS_AUDIOCAPTUREMGR_NAME,  IARMBUS_AUDIOCAPTUREMGR_REQUEST_SAMPLE, (void *) &param, sizeof(param));
            if(IARM_RESULT_SUCCESS != ret)
            {
                return ACM_RESULT_GENERAL_FAILURE;
            }
            return param.result;
        }

        void DataCapture::constructFormatString()
        {
            _audio_format_string = "";
            switch(_audio_properties.format)
            {
                case acmFormate16BitStereo:
                    _audio_format_string += "codec=PCM_16_"; break;
                case acmFormate16BitMonoLeft: //fall-through
                case acmFormate16BitMonoRight: //fall-through
                case acmFormate16BitMono:
                    _audio_format_string += "codec=PCM_1_16_"; break;
                case acmFormate24BitStereo:
                    _audio_format_string += "codec=PCM_24_"; break;
                case acmFormate24Bit5_1:
                    _audio_format_string += "codec=PCM_6_24_"; break;
                default:
                    LOGERR("Unsupported audio format!");
            }

            switch(_audio_properties.sampling_frequency)
            {
                case acmFreqe48000:
                    _audio_format_string += "48000&"; break;
                case acmFreqe44100:
                    _audio_format_string += "44100&"; break;
                case acmFreqe32000:
                    _audio_format_string += "32000&"; break;
                case acmFreqe24000:
                    _audio_format_string += "24000&"; break;
                case acmFreqe16000:
                    _audio_format_string += "16000&"; break;
                default:
                    LOGERR("Unsupported audio sampling rate!");
            }
            LOGINFO("New format string is %s", _audio_format_string.c_str());
        }

        void DataCapture::iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            pthread_mutex_lock(&_mutex);
            LOGINFO("entry with owner: %s", owner);

            if( _instance != nullptr)
            {
                _instance->eventHandler(owner, eventId, data, len);
            }
            else
            {
                LOGERR("Failed handle IARM event: DataCapture instance is null");
            }
            pthread_mutex_unlock(&_mutex);
        }

        void DataCapture::eventHandler(const char *owner, IARM_EventId_t eventId, void *eventData, size_t len)
        {
            if(DATA_CAPTURE_IARM_EVENT_AUDIO_CLIP_READY == eventId)
            {
                iarmbus_notification_payload_t * payload = static_cast <iarmbus_notification_payload_t *> (eventData);
                string dataLocator(payload->dataLocator);
                string delimiter = "/";
                size_t pos = 0;
                string fileName;
                pos = dataLocator.rfind(delimiter);
                fileName = dataLocator.substr(pos + delimiter.length(), dataLocator.length());
                int attemptsLeft = 2;
                vector<unsigned char> data;
                int time_wait_sec = 1;

                JsonObject params;
                params["fileName"] = fileName;

                while (attemptsLeft) {
                    if(0 == _sock_adaptor->connect_socket(payload->dataLocator))
                    {
                        _sock_adaptor->get_data(data); // closes the socket
                        if (data.size() > 0) {
                            LOGINFO("Got a clip: %lu bytes", data.size());
                            break;
                        } else {
                            LOGWARN("No data in the socket. One more attempt in %d sec", time_wait_sec);
                            usleep(1000 * 1000 * time_wait_sec);
                            --attemptsLeft;
                            continue;
                        }
                    }
                }

                if(data.size() > 0)
                {
                    std::string error_str;
                    if (uploadDataToUrl(data, _destination_url.c_str(), error_str))
                    {
                        params["status"] = true;
                        params["message"] = "Success";

                    } else {
                        LOGERR("Upload failed: %s (cURL error)", C_STR(error_str));
                        params["status"] = false;
                        params["message"] = std::string("Upload Failed: ") + error_str;
                    }

                    // Optionally, we can save a file
//                    FILE * pFile;
//                    const char* path = strcat(payload->dataLocator, ".received.pcm");
//                    pFile = fopen (path, "wb");
//                    fwrite (&data[0] , sizeof(unsigned char), data.size(), pFile);
//                    fclose (pFile);
                    // now, upload it, then remove:
//                    if (remove(path) != 0)
//                    {
//                        LOGERR("Unable to delete %s", path);
//                    }
                } else {
                    LOGERR("Unable to read data from %s (connection error)", payload->dataLocator);
                    params["status"] = false;
                    params["message"] = std::string("Unable to read data from  ") + string(payload->dataLocator);
                }

                string message;
                params.ToString(message);
                LOGINFO("Sending notification %s: %s", C_STR(EVT_ON_AUDIO_CLIP_READY), C_STR(message));
                sendNotify(C_STR(EVT_ON_AUDIO_CLIP_READY), params);
            }
        }

        bool DataCapture::uploadDataToUrl(std::vector<unsigned char> &data, const char *url, std::string &error_str)
        {
            CURL *curl;
            CURLcode res;
            bool call_succeeded = true;

            if(!url || !strlen(url))
            {
                LOGERR("no url given");
                return false;
            }

            LOGWARN("uploading pcm data of size %lu to '%s'", data.size(), url);

            //init curl
            curl_global_init(CURL_GLOBAL_ALL);
            curl = curl_easy_init();

            if(!curl)
            {
                LOGERR("could not init curl\n");
                return false;
            }

            //create header
            struct curl_slist *chunk = NULL;
            chunk = curl_slist_append(chunk, "Content-Type: audio/x-wav");

            //set url and data
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, &data[0]);

            //perform blocking upload call
            res = curl_easy_perform(curl);

            //output success / failure log
            if(CURLE_OK == res)
            {
                long response_code;

                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

                if(600 > response_code && response_code >= 400)
                {
                    LOGERR("uploading failed with response code %ld\n", response_code);
                    error_str = std::string("response code:") + std::to_string(response_code);
                    call_succeeded = false;
                }
                else
                    LOGWARN("upload done");
            }
            else
            {
                LOGERR("upload failed with error %d:'%s'", res, curl_easy_strerror(res));
                error_str = std::to_string(res) + std::string(":'") + std::string(curl_easy_strerror(res)) + std::string("'");
                call_succeeded = false;
            }
            //clean up curl object
            curl_easy_cleanup(curl);
            curl_slist_free_all(chunk);

            return call_succeeded;
        }
        // Internal methods end
    } // namespace Plugin
} // namespace WPEFramework
