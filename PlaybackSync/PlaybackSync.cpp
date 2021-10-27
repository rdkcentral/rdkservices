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

#include <sstream>

#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wformat"
#include "utils.h"
#include "PlaybackSync.h"

namespace WPEFramework
{
    namespace Plugin
    {
        // Methods
        enum
        {
            PLAYBACKSYNC_METHOD_CONNECT_ID = 0,
            PLAYBACKSYNC_METHOD_SET_ROOM_ID,
            PLAYBACKSYNC_METHOD_GET_ROOM_ID,
            PLAYBACKSYNC_METHOD_SET_DEVICE_ID,
            PLAYBACKSYNC_METHOD_GET_DEVICE_ID,
            PLAYBACKSYNC_METHOD_LEADER_REQUEST_ID,
            PLAYBACKSYNC_METHOD_PLAYBACK_POS_ID,
            PLAYBACKSYNC_METHOD_PLAYBACK_MESSAGE_ID,
            PLAYBACKSYNC_METHOD_PLAYBACK_STARTED_ID,
            PLAYBACKSYNC_METHOD_PLAYBACK_ERROR_ID,
            PLAYBACKSYNC_METHOD_PLAYBACK_PAUSE_ID,
            PLAYBACKSYNC_METHOD_PLAYBACK_STOP_ID,
            PLAYBACKSYNC_METHOD_PLAYBACK_SPEED_ID,
            PLAYBACKSYNC_METHOD_PLAYBACK_SET_URL_ID,
            PLAYBACKSYNC_METHOD_PLAYBACK_GET_URL_ID,
            PLAYBACKSYNC_METHOD_PLAYBACK_SET_SVCKEY_ID,
            PLAYBACKSYNC_METHOD_PLAYBACK_GET_SVCKEY_ID
        };

        const char* g_methodNames[] =
        {
            [PLAYBACKSYNC_METHOD_CONNECT_ID]            = "connect",
            [PLAYBACKSYNC_METHOD_SET_ROOM_ID]           = "setRoom",
            [PLAYBACKSYNC_METHOD_GET_ROOM_ID]           = "getRoom",
            [PLAYBACKSYNC_METHOD_SET_DEVICE_ID]         = "setDevice",
            [PLAYBACKSYNC_METHOD_GET_DEVICE_ID]         = "getDevice",
            [PLAYBACKSYNC_METHOD_LEADER_REQUEST_ID]     = "requestLeadership",
            [PLAYBACKSYNC_METHOD_PLAYBACK_POS_ID]       = "publishPlaybackPosition",
            [PLAYBACKSYNC_METHOD_PLAYBACK_MESSAGE_ID]   = "publishMessage",
            [PLAYBACKSYNC_METHOD_PLAYBACK_STARTED_ID]   = "publishPlaybackStarted",
            [PLAYBACKSYNC_METHOD_PLAYBACK_ERROR_ID]     = "publishPlaybackError",
            [PLAYBACKSYNC_METHOD_PLAYBACK_PAUSE_ID]     = "publishPause",
            [PLAYBACKSYNC_METHOD_PLAYBACK_STOP_ID]      = "publishStop",
            [PLAYBACKSYNC_METHOD_PLAYBACK_SPEED_ID]     = "publishSpeed",
            [PLAYBACKSYNC_METHOD_PLAYBACK_SET_URL_ID]   = "setPlaybackURL",
            [PLAYBACKSYNC_METHOD_PLAYBACK_GET_URL_ID]   = "getPlaybackURL",
            [PLAYBACKSYNC_METHOD_PLAYBACK_SET_SVCKEY_ID]= "setPlaybackServiceKey",
            [PLAYBACKSYNC_METHOD_PLAYBACK_GET_SVCKEY_ID]= "getPlaybackServiceKey"
        };
        // End Methods

        // Parameters
        enum
        { 
            PLAYBACKSYNC_PARAM_ENDPOINT_ID = 0,
            PLAYBACKSYNC_PARAM_CLIENT_NAME_ID,
            PLAYBACKSYNC_PARAM_CLIENT_TYPE_ID,
            PLAYBACKSYNC_PARAM_TOKEN_ID,
            PLAYBACKSYNC_PARAM_ROOM_CODE_ID,
            PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID,
            PLAYBACKSYNC_PARAM_LEADER_REQUEST_ID,
            PLAYBACKSYNC_PARAM_PLAYBACK_URL_ID,
            PLAYBACKSYNC_PARAM_PLAYBACK_SVCKEY_ID,
            PLAYBACKSYNC_PARAM_PLAYBACK_MESSAGE_ID,
            PLAYBACKSYNC_PARAM_PLAYBACK_PAUSE_ID,
            PLAYBACKSYNC_PARAM_PLAYBACK_STOP_ID,
            PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID,
            PLAYBACKSYNC_PARAM_PLAYBACK_POSITION_ID,
            PLAYBACKSYNC_PARAM_PLAYBACK_STARTED_OVR_ID,
            PLAYBACKSYNC_PARAM_PLAYBACK_SPEED_ID,
            PLAYBACKSYNC_PARAM_PLAYBACK_ERROR_CODE_ID
        };

        const struct paramInfo_t g_paramInfo[] =
        {
            [PLAYBACKSYNC_PARAM_ENDPOINT_ID]                = {"stringEndpoint", WPEFramework::Core::JSON::Variant::type::STRING},
            [PLAYBACKSYNC_PARAM_CLIENT_NAME_ID]             = {"stringClientName", WPEFramework::Core::JSON::Variant::type::STRING},
            [PLAYBACKSYNC_PARAM_CLIENT_TYPE_ID]             = {"numberClientType", WPEFramework::Core::JSON::Variant::type::NUMBER},
            [PLAYBACKSYNC_PARAM_TOKEN_ID]                   = {"stringToken", WPEFramework::Core::JSON::Variant::type::STRING},
            [PLAYBACKSYNC_PARAM_ROOM_CODE_ID]               = {"stringRoomCode", WPEFramework::Core::JSON::Variant::type::STRING},
            [PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID]          = {"jsonDeviceDetails", WPEFramework::Core::JSON::Variant::type::OBJECT},
            [PLAYBACKSYNC_PARAM_LEADER_REQUEST_ID]          = {"booleanRoomLeader", WPEFramework::Core::JSON::Variant::type::BOOLEAN},
            [PLAYBACKSYNC_PARAM_PLAYBACK_URL_ID]            = {"stringURL", WPEFramework::Core::JSON::Variant::type::STRING},
            [PLAYBACKSYNC_PARAM_PLAYBACK_SVCKEY_ID]         = {"stringServiceKey", WPEFramework::Core::JSON::Variant::type::STRING},
            [PLAYBACKSYNC_PARAM_PLAYBACK_MESSAGE_ID]        = {"stringMessage", WPEFramework::Core::JSON::Variant::type::STRING},
            [PLAYBACKSYNC_PARAM_PLAYBACK_PAUSE_ID]          = {"booleanPause", WPEFramework::Core::JSON::Variant::type::BOOLEAN},
            [PLAYBACKSYNC_PARAM_PLAYBACK_STOP_ID]           = {"booleanStop", WPEFramework::Core::JSON::Variant::type::BOOLEAN},
            [PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID]      = {"numberTimestamp", WPEFramework::Core::JSON::Variant::type::NUMBER},
            [PLAYBACKSYNC_PARAM_PLAYBACK_POSITION_ID]       = {"numberPosition", WPEFramework::Core::JSON::Variant::type::NUMBER},
            [PLAYBACKSYNC_PARAM_PLAYBACK_STARTED_OVR_ID]    = {"numberSetupOverhead", WPEFramework::Core::JSON::Variant::type::NUMBER},
            [PLAYBACKSYNC_PARAM_PLAYBACK_SPEED_ID]          = {"numberSpeed", WPEFramework::Core::JSON::Variant::type::NUMBER},
            [PLAYBACKSYNC_PARAM_PLAYBACK_ERROR_CODE_ID]     = {"numberErrorCode", WPEFramework::Core::JSON::Variant::type::NUMBER}
        };
        // End Parameters

        // Events
        enum
        { 
            PLAYBACKSYNC_EVENT_MESSAGE_ID = 0,
            PLAYBACKSYNC_EVENT_LEADER_ID,
            PLAYBACKSYNC_EVENT_POSITION_ID,
            PLAYBACKSYNC_EVENT_PAUSE_ID,
            PLAYBACKSYNC_EVENT_SPEED_ID,
            PLAYBACKSYNC_EVENT_STARTED_ID,
            PLAYBACKSYNC_EVENT_ERROR_ID,
            PLAYBACKSYNC_EVENT_STOPPED_ID,
            PLAYBACKSYNC_EVENT_DEVICE_DETAILS_ID,
            PLAYBACKSYNC_EVENT_JOINED_ID,
            PLAYBACKSYNC_EVENT_LEFT_ID,
            PLAYBACKSYNC_EVENT_URL_ID,
            PLAYBACKSYNC_EVENT_SVCKEY_ID
        };

        const string g_eventNames[] = 
        {
            [PLAYBACKSYNC_EVENT_MESSAGE_ID]         = "onMessage",
            [PLAYBACKSYNC_EVENT_LEADER_ID]          = "onLeaderRequest",
            [PLAYBACKSYNC_EVENT_POSITION_ID]        = "onPosition",
            [PLAYBACKSYNC_EVENT_PAUSE_ID]           = "onPause",
            [PLAYBACKSYNC_EVENT_SPEED_ID]           = "onSpeed",
            [PLAYBACKSYNC_EVENT_STARTED_ID]         = "onPlay",
            [PLAYBACKSYNC_EVENT_ERROR_ID]           = "onError",
            [PLAYBACKSYNC_EVENT_STOPPED_ID]         = "onStopped",
            [PLAYBACKSYNC_EVENT_DEVICE_DETAILS_ID]  = "onDeviceDetails",
            [PLAYBACKSYNC_EVENT_JOINED_ID]          = "onDeviceJoined",
            [PLAYBACKSYNC_EVENT_LEFT_ID]            = "onDeviceLeft",
            [PLAYBACKSYNC_EVENT_URL_ID]             = "onPlaybackURL",
            [PLAYBACKSYNC_EVENT_SVCKEY_ID]          = "onPlaybackServiceKey"
        };

        const std::unordered_map<std::reference_wrapper<const std::string>, 
                                paramInfoList_t, 
                                std::hash<std::string>, std::equal_to<std::string>> g_eventInfo =
        {
            {g_eventNames[PLAYBACKSYNC_EVENT_MESSAGE_ID],   {
                                g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID],
                                g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID],
                                g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_MESSAGE_ID]
                            }
            },
            {g_eventNames[PLAYBACKSYNC_EVENT_LEADER_ID], {
                                    g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID],
                                    g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID],
                                    g_paramInfo[PLAYBACKSYNC_PARAM_LEADER_REQUEST_ID]
                                }
            },
            {g_eventNames[PLAYBACKSYNC_EVENT_POSITION_ID],  {
                                g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID],
                                g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID],
                                g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_POSITION_ID]
                            }
            },
            {g_eventNames[PLAYBACKSYNC_EVENT_PAUSE_ID], {
                            g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID],
                            g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID],
                            g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_POSITION_ID]
                        }
            },
            {g_eventNames[PLAYBACKSYNC_EVENT_SPEED_ID], {
                            g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID],
                            g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID],
                            g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_SPEED_ID]
                        }
            },
            {g_eventNames[PLAYBACKSYNC_EVENT_STARTED_ID],   {
                            g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID],
                            g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID],
                            g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_POSITION_ID]
                            }
            },
            {g_eventNames[PLAYBACKSYNC_EVENT_ERROR_ID], {
                            g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID],
                            g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID],
                            g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_ERROR_CODE_ID]
                        }
            },
            {g_eventNames[PLAYBACKSYNC_EVENT_STOPPED_ID],   {
                                g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID],
                                g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID]
                            }
            },
            {g_eventNames[PLAYBACKSYNC_EVENT_DEVICE_DETAILS_ID], {
                                    g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID],
                                    g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID]
                                }
            },
            {g_eventNames[PLAYBACKSYNC_EVENT_JOINED_ID],  {
                                    g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID],
                                    g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID]
                                }
            },
            {g_eventNames[PLAYBACKSYNC_EVENT_LEFT_ID],    {
                                    g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID],
                                    g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID]
                                }
            },
            {g_eventNames[PLAYBACKSYNC_EVENT_URL_ID],   {
                                    g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID],
                                    g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID],
                                    g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_URL_ID]
                                }
            },
            {g_eventNames[PLAYBACKSYNC_EVENT_SVCKEY_ID],    {
                                            g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID],
                                            g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID],
                                            g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_SVCKEY_ID]
                                        }
            }
        };
        // End Events
        SERVICE_REGISTRATION(PlaybackSync, 1, 0);

        PlaybackSync* PlaybackSync::_instance = nullptr;

        PlaybackSync::PlaybackSync()
        : AbstractPlugin(), m_linchpinRPC(this)
        {
            LOGDBG("Plugin constructed");
            PlaybackSync::_instance = this;

            Utils::IARM::init();

            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_CONNECT_ID], &PlaybackSync::ConnectToServer, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_SET_ROOM_ID], &PlaybackSync::setRoomID, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_GET_ROOM_ID], &PlaybackSync::getRoomID, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_SET_DEVICE_ID], &PlaybackSync::setDeviceDetails, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_GET_DEVICE_ID], &PlaybackSync::getDeviceDetails, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_LEADER_REQUEST_ID], &PlaybackSync::requestLeadership, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_PLAYBACK_SET_URL_ID], &PlaybackSync::setPlaybackURL, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_PLAYBACK_GET_URL_ID], &PlaybackSync::getPlaybackURL, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_PLAYBACK_SET_SVCKEY_ID], &PlaybackSync::setPlaybackServiceKey, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_PLAYBACK_GET_SVCKEY_ID], &PlaybackSync::getPlaybackServiceKey, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_PLAYBACK_MESSAGE_ID], &PlaybackSync::publishPlaybackMessage, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_PLAYBACK_PAUSE_ID], &PlaybackSync::publishPlaybackPause, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_PLAYBACK_SPEED_ID], &PlaybackSync::publishPlaybackSpeed, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_PLAYBACK_STOP_ID], &PlaybackSync::publishPlaybackStop, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_PLAYBACK_POS_ID], &PlaybackSync::publishPlaybackPosition, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_PLAYBACK_STARTED_ID], &PlaybackSync::publishPlaybackStarted, this);
            registerMethod(g_methodNames[PLAYBACKSYNC_METHOD_PLAYBACK_ERROR_ID], &PlaybackSync::publishPlaybackError, this);
        }

        PlaybackSync::~PlaybackSync()
        {
            LOGDBG("Plugin distructed");
        }

        void PlaybackSync::Deinitialize(PluginHost::IShell* /* service */)
        {
            LOGDBG("Plugin deinitialized");
            _instance = nullptr;
        }

        const string PlaybackSync::Initialize(PluginHost::IShell* service)
        {
            if (!m_linchpinRPC.Attach(service))
                return string("Not able to open communication with linchpin plugin/server");

            LOGDBG("Plugin initialized");
            return string();
        }

        void PlaybackSync::PopulateCachedParameters(const paramInfoList_t& paramInfoList, JsonObject& packParams)
        {
            for(const paramInfo_t& p : paramInfoList)
            {
                if (&p == &g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID])
                    packParams[p.paramName] = m_deviceDetails;
                else if (&p == &g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID])
                    packParams[p.paramName] = 0;
                else if (&p == &g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_URL_ID])
                    packParams[p.paramName] = m_playbackURL;
                else if (&p == &g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_SVCKEY_ID])
                    packParams[p.paramName] = m_playbackServiceKey;
                else
                    LOGINFO("Un-recognized parameter %s not packed", p.paramName);
            }

            std::string stringParams;
            packParams.ToString(stringParams);
            LOGINFO("Parameters packed %s", stringParams);
        }

        uint32_t PlaybackSync::ConnectToServer(const JsonObject& parameters, JsonObject& response)
        {
            returnIfStringParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_ENDPOINT_ID].paramName);
            returnIfStringParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_CLIENT_NAME_ID].paramName);
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_CLIENT_TYPE_ID].paramName);
            returnIfStringParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_TOKEN_ID].paramName);

            string endpoint = parameters[g_paramInfo[PLAYBACKSYNC_PARAM_ENDPOINT_ID].paramName].String();
            string client = parameters[g_paramInfo[PLAYBACKSYNC_PARAM_CLIENT_NAME_ID].paramName].String();
            int64_t type = parameters[g_paramInfo[PLAYBACKSYNC_PARAM_CLIENT_TYPE_ID].paramName].Number();
            string token = parameters[g_paramInfo[PLAYBACKSYNC_PARAM_TOKEN_ID].paramName].String();
            if(m_linchpinRPC.Connect(endpoint, client, type, token))
                returnResponse(true);

            LOGINFO("Connected to server endpoint %s, using client name %s", endpoint.c_str(), client.c_str());
            returnResponse(false);
        }

        uint32_t PlaybackSync::setRoomID(const JsonObject& parameters, JsonObject& response)
        {
            const char* paramLabel = g_paramInfo[PLAYBACKSYNC_PARAM_ROOM_CODE_ID].paramName;

            returnIfStringParamNotFound(parameters, paramLabel);
            string roomID = parameters[paramLabel].String();
            if (m_deviceDetailsString.empty())
            {
                LOGINFO("Missing device details room name %s not set", roomID.c_str());
                returnResponse(false);
            }

            if (m_roomID != roomID)
            {
                // Un-subscribe from the existing topic
                if (!m_roomID.empty())
                {
                    // Publish device leaving the room
                    JsonObject publishParams;
                    PopulateCachedParameters(g_eventInfo.at(g_eventNames[PLAYBACKSYNC_EVENT_LEFT_ID]), publishParams);
                    m_linchpinRPC.PublishMessage(m_roomID, g_eventNames[PLAYBACKSYNC_EVENT_LEFT_ID], publishParams);
                    m_linchpinRPC.Unsubscribe(m_roomID);
                    m_roomID.clear();
                }

                // Subscribe to the topic using linchpin with the roomID
                if (!roomID.empty())
                {
                    if(m_linchpinRPC.Subscribe(roomID))
                    {
                        m_roomID = roomID;
                        // Publish device joining the room
                        JsonObject publishParams;
                        PopulateCachedParameters(g_eventInfo.at(g_eventNames[PLAYBACKSYNC_EVENT_JOINED_ID]), publishParams);
                        m_linchpinRPC.PublishMessage(m_roomID, g_eventNames[PLAYBACKSYNC_EVENT_JOINED_ID], publishParams);
                    }
                    else
                    {
                        LOGINFO("Room name %s not set", roomID.c_str());
                        returnResponse(false);
                    }
                }
            }

            LOGINFO("Room name set to %s", m_roomID.c_str());
            returnResponse(true);
        }

        uint32_t PlaybackSync::getRoomID(const JsonObject& parameters, JsonObject& response)
        {
            const char* paramLabel = g_paramInfo[PLAYBACKSYNC_PARAM_ROOM_CODE_ID].paramName;

            response[paramLabel] = m_roomID.c_str();

            LOGINFO("Room name was set to %s", m_roomID.c_str());
            returnResponse(true);
        }

        uint32_t PlaybackSync::setDeviceDetails(const JsonObject& parameters, JsonObject& response)
        {
            const char* paramLabel = g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID].paramName;

            returnIfParamNotFound(parameters, paramLabel);

            JsonObject temp;
            std::string dd;
            if (parameters[paramLabel].Content() == Core::JSON::Variant::type::OBJECT &&
                parameters[paramLabel].Object().IsSet())
            {
                temp = parameters[paramLabel].Object();
                temp.ToString(dd);
            }
            else if (parameters[paramLabel].Content() == Core::JSON::Variant::type::STRING)
            {
                // Handle JSON passed in as strings
                dd = parameters[paramLabel].String();
                Core::OptionalType<Core::JSON::Error> error;
                // JSON integrity validation
                if (!temp.FromString(dd, error))
                {
                    LOGERR("%s at %ld - %s", error.Value().Message().c_str(), error.Value().Position(), error.Value().Context().c_str());
                    returnResponse(false);
                }
            }
            else
            {
                LOGERR("Invalid parameter type! JSON object expected got '%s'", parameters[paramLabel].String().c_str());
                returnResponse(false);
            }

            if (dd != m_deviceDetailsString)
            {
                // Device Details changed
                m_deviceDetails = temp;
                m_deviceDetailsString = dd;
                // Publish updates to room participants!!!!
                if (!m_roomID.empty())
                {
                    JsonObject publishParams;
                    PopulateCachedParameters(g_eventInfo.at(g_eventNames[PLAYBACKSYNC_EVENT_DEVICE_DETAILS_ID]), publishParams);
                    m_linchpinRPC.PublishMessage(m_roomID, g_eventNames[PLAYBACKSYNC_EVENT_DEVICE_DETAILS_ID], publishParams);
                }
            }

            LOGINFO("Device details set to '%s'", m_deviceDetailsString.c_str());
            returnResponse(true);
        }

        uint32_t PlaybackSync::getDeviceDetails(const JsonObject& parameters, JsonObject& response)
        {
            const char* paramLabel = g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID].paramName;

            response[paramLabel] = m_deviceDetails;

            LOGINFO("Device details were set to '%s'", m_deviceDetailsString.c_str());
            returnResponse(true);
        }

        uint32_t PlaybackSync::requestLeadership(const JsonObject& parameters, JsonObject& response)
        {
            const char* paramLabel = g_paramInfo[PLAYBACKSYNC_PARAM_LEADER_REQUEST_ID].paramName;

            returnIfBooleanParamNotFound(parameters, paramLabel);

            LOGINFO("Leadership request type '%s'", parameters[paramLabel].String().c_str());
            // Send leadership update to room participants
            JsonObject publishParams = parameters;
            PopulateCachedParameters(g_eventInfo.at(g_eventNames[PLAYBACKSYNC_EVENT_LEADER_ID]), publishParams);
            bool rc = m_linchpinRPC.PublishMessage(m_roomID, g_eventNames[PLAYBACKSYNC_EVENT_LEADER_ID], publishParams);
            returnResponse(rc);
        }

        uint32_t PlaybackSync::setPlaybackURL(const JsonObject& parameters, JsonObject& response)
        {
            return setPlaybackString(m_playbackURL, PLAYBACKSYNC_PARAM_PLAYBACK_URL_ID, parameters, response);
        }
        uint32_t PlaybackSync::getPlaybackURL(const JsonObject& parameters, JsonObject& response)
        {
            return getPlaybackString(m_playbackURL, PLAYBACKSYNC_PARAM_PLAYBACK_URL_ID, parameters, response);
        }
        uint32_t PlaybackSync::setPlaybackServiceKey(const JsonObject& parameters, JsonObject& response)
        {
            return setPlaybackString(m_playbackServiceKey, PLAYBACKSYNC_PARAM_PLAYBACK_SVCKEY_ID, parameters, response);
        }
        uint32_t PlaybackSync::getPlaybackServiceKey(const JsonObject& parameters, JsonObject& response)
        {
            return getPlaybackString(m_playbackServiceKey, PLAYBACKSYNC_PARAM_PLAYBACK_SVCKEY_ID, parameters, response);
        }

        uint32_t PlaybackSync::setPlaybackString(string& storage, uint8_t labelIndex, const JsonObject& parameters, JsonObject& response)
        {
            const char* paramLabel = g_paramInfo[labelIndex].paramName;
            returnIfStringParamNotFound(parameters, paramLabel);

            std::string pbs = parameters[paramLabel].String();
            if (storage != pbs)
                storage = pbs;
            // Update room participants with the new playback string
            JsonObject publishParams;
            uint8_t eventNameIndex = (labelIndex == PLAYBACKSYNC_PARAM_PLAYBACK_URL_ID) ? PLAYBACKSYNC_EVENT_URL_ID : PLAYBACKSYNC_EVENT_SVCKEY_ID;
            PopulateCachedParameters(g_eventInfo.at(g_eventNames[eventNameIndex]), publishParams);
            m_linchpinRPC.PublishMessage(m_roomID, g_eventNames[eventNameIndex], publishParams);

            LOGINFO("Playback set to '%s'", storage.c_str());
            returnResponse(true);
        }

        uint32_t PlaybackSync::getPlaybackString(string& storage, uint8_t labelIndex, const JsonObject& parameters, JsonObject& response)
        {
            const char* paramLabel = g_paramInfo[labelIndex].paramName;
            response[paramLabel] = storage;

            LOGINFO("Playback was set to '%s'", storage.c_str());
            returnResponse(true);
        }

        uint32_t PlaybackSync::publishPlaybackMessage(const JsonObject& parameters, JsonObject& response)
        {
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID].paramName);
            returnIfStringParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_MESSAGE_ID].paramName);

            JsonObject publishParams = parameters;
            publishParams[g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID].paramName] = m_deviceDetails;

            // Call the linchpin plugin's publish API with parameters and onMessage event
            bool rc = m_linchpinRPC.PublishMessage(m_roomID, g_eventNames[PLAYBACKSYNC_EVENT_MESSAGE_ID], publishParams);
            returnResponse(rc);
        }

        uint32_t PlaybackSync::publishPlaybackPause(const JsonObject& parameters, JsonObject& response)
        {
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID].paramName);
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_POSITION_ID].paramName);

            JsonObject publishParams = parameters;
            publishParams[g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID].paramName] = m_deviceDetails;

            // Call the linchpin plugin's publish API with parameters and onPausePlayback event
            bool rc = m_linchpinRPC.PublishMessage(m_roomID, g_eventNames[PLAYBACKSYNC_EVENT_PAUSE_ID], publishParams);
            returnResponse(rc);
        }

        uint32_t PlaybackSync::publishPlaybackStop(const JsonObject& parameters, JsonObject& response)
        {
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID].paramName);

            JsonObject publishParams = parameters;
            publishParams[g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID].paramName] = m_deviceDetails;

            // Call the linchpin plugin's publish API with parameters and onPlaybackStopped event
            bool rc = m_linchpinRPC.PublishMessage(m_roomID, g_eventNames[PLAYBACKSYNC_EVENT_STOPPED_ID], publishParams);
            returnResponse(rc);
        }

        uint32_t PlaybackSync::publishPlaybackPosition(const JsonObject& parameters, JsonObject& response)
        {
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID].paramName);
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_POSITION_ID].paramName);

            JsonObject publishParams = parameters;
            publishParams[g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID].paramName] = m_deviceDetails;

            // Call the linchpin plugin's publish API with parameters and onPlaybackPoistion event
            bool rc = m_linchpinRPC.PublishMessage(m_roomID, g_eventNames[PLAYBACKSYNC_EVENT_POSITION_ID], publishParams);
            returnResponse(rc);
        }

        uint32_t PlaybackSync::publishPlaybackStarted(const JsonObject& parameters, JsonObject& response)
        {
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID].paramName);
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_STARTED_OVR_ID].paramName);
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_POSITION_ID].paramName);

            JsonObject publishParams = parameters;
            publishParams[g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID].paramName] = m_deviceDetails;

            // Call the linchpin plugin's publish API with parameters and onPlaybackStarted event
            bool rc = m_linchpinRPC.PublishMessage(m_roomID, g_eventNames[PLAYBACKSYNC_EVENT_STARTED_ID], publishParams);
            returnResponse(rc);
        }

        uint32_t PlaybackSync::publishPlaybackSpeed(const JsonObject& parameters, JsonObject& response)
        {
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID].paramName);
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_SPEED_ID].paramName);

            JsonObject publishParams = parameters;
            publishParams[g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID].paramName] = m_deviceDetails;

            // Call the linchpin plugin's publish API with parameters and onPlaybackSpeed event
            bool rc = m_linchpinRPC.PublishMessage(m_roomID, g_eventNames[PLAYBACKSYNC_EVENT_SPEED_ID], publishParams);
            returnResponse(rc);
        }

        uint32_t PlaybackSync::publishPlaybackError(const JsonObject& parameters, JsonObject& response)
        {
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_TIMESTAMP_ID].paramName);
            returnIfNumberParamNotFound(parameters, g_paramInfo[PLAYBACKSYNC_PARAM_PLAYBACK_ERROR_CODE_ID].paramName);

            JsonObject publishParams = parameters;
            publishParams[g_paramInfo[PLAYBACKSYNC_PARAM_DEVICE_DETAILS_ID].paramName] = m_deviceDetails;

            // Call the linchpin plugin's publish API with parameters and onPlaybackSpeed event
            bool rc = m_linchpinRPC.PublishMessage(m_roomID, g_eventNames[PLAYBACKSYNC_EVENT_ERROR_ID], publishParams);
            returnResponse(rc);
        }

        void PlaybackSync::onNotifyMessageReceived(const string& room)
        {
            LOGINFO("Event publish notification received for room '%s'", room.c_str());
            if (m_roomID != room)
            {
                LOGWARN("Ignoring! notification received for room '%s', while subscribed to '%s'", room.c_str(), m_roomID.c_str());
                return;
            }

            string pbevent;
            JsonObject params;
            if (!m_linchpinRPC.FetchMessage(m_roomID, pbevent, params))
                return;

            string paramResponse;
            params.ToString(paramResponse);
            if (!ValidPlaybackSyncEvent(pbevent, params))
                LOGWARN("Event validation failure in message received for room '%s', event '%s' parameters '%s'",
                        m_roomID.c_str(), pbevent.c_str(), paramResponse.c_str());
            else
            {
                LOGINFO("Message received for room '%s', event '%s' parameters '%s'", m_roomID.c_str(), pbevent.c_str(), paramResponse.c_str());
                sendNotify(pbevent.c_str(), params);
            }
        }

        bool PlaybackSync::ValidPlaybackSyncEvent(string& eventName, JsonObject& params)
        {
            auto iterator = g_eventInfo.find(eventName);
            if (iterator != g_eventInfo.end())
            {
                // Found event, validate expected parameters list
                for(const paramInfo_t& p : iterator->second)
                {
                    if(!params.HasLabel(p.paramName) || params[p.paramName].Content() != p.paramType)
                    {
                        LOGWARN("No argument '%s' or it has incorrect type", p.paramName);
                        return false;
                    }
                }
                return true;
            }
            return false;
        }

        void PlaybackSync::onConnectionClosed(void)
        {
            LOGINFO("Event connection closed received");
        }
     } // namespace Plugin
} // namespace WPEFramework
