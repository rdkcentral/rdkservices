/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2022 RDK Management
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
#include "libmediaplayer.h"
#include "MediaEngineRMF.h"

#include "UtilsCStr.h"
#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

const string WPEFramework::Plugin::MediaEngineRMF::SERVICE_NAME = "org.rdk.MediaEngineRMF";
const string WPEFramework::Plugin::MediaEngineRMF::METHOD_PLAY = "play";
const string WPEFramework::Plugin::MediaEngineRMF::METHOD_STOP = "stop";
const string WPEFramework::Plugin::MediaEngineRMF::METHOD_MUTE = "mute";
const string WPEFramework::Plugin::MediaEngineRMF::METHOD_SET_VIDEO_RECTANGLE = "setVideoRectangle";
const string WPEFramework::Plugin::MediaEngineRMF::METHOD_SET_VOLUME = "setVolume";
const string WPEFramework::Plugin::MediaEngineRMF::METHOD_INITIALIZE = "initialize";
const string WPEFramework::Plugin::MediaEngineRMF::METHOD_DEINITIALIZE = "deinitialize";
const string WPEFramework::Plugin::MediaEngineRMF::EVT_ON_STATUS = "onStatusChanged";
const string WPEFramework::Plugin::MediaEngineRMF::EVT_ON_WARNING = "onWarning";
const string WPEFramework::Plugin::MediaEngineRMF::EVT_ON_ERROR = "onError";

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

using namespace std;
using namespace libmediaplayer;

struct kv_pair
{
    const char * key;
    const char * value;
};

static kv_pair environment_variables[] = { //TODO: AXG1v4-specific settings. Move this to somewhere platform-specific.
    {"RMF_OSAL_THREAD_INFO_CALL_PORT", "54128"},
    {"DtcpSrmFilePath", "/opt/persistent/dtcp.srm"},
    {"GST_REGISTRY_FORK", "no"},
    {"GST_REGISTRY", "/opt/.gstreamer/registry.bin"},
    {"PFC_ROOT", "/"},
    {"VL_ECM_RPC_IF_NAME", "eth2:0"},
    {"VL_DOCSIS_DHCP_IF_NAME", "eth2"},
    {"VL_DOCSIS_WAN_IF_NAME", "eth2"},
    {"VL_PLATFORM_NAME", "BCM_NEXUS_LINUX_74XX"},
    {"PERM_NVRAM_DIR", "/opt/persistent"},
    {"DYN_NVRAM_DIR", "/opt/persistent"},
    {"brcm_directfb_mode", "n"},
    {"brcm_multiprocess_server", "refsw_server"},
    {"brcm_multiprocess_mode", "y"},
    {"dtcp_create_session_wait", "y"}
};

static void set_env_variables()
{
    int list_size = sizeof(environment_variables) / sizeof(kv_pair);
    for (int i = 0; i < list_size; i++)
    {
        setenv(environment_variables[i].key, environment_variables[i].value, 1);
    }
}

namespace WPEFramework {

    namespace {

        static Plugin::Metadata<Plugin::MediaEngineRMF> metadata(
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

        SERVICE_REGISTRATION(MediaEngineRMF, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        MediaEngineRMF* MediaEngineRMF::_instance = nullptr;

        static void event_callback(notification_payload * payload, void * data)
        {
            (void) data;
            if(nullptr != MediaEngineRMF::_instance)
            {
                MediaEngineRMF::_instance->onEvent(payload);
            }
        }

        static void error_callback(notification_payload * payload, void * data)
        {
            (void) data;
            if(nullptr != MediaEngineRMF::_instance)
            {
                MediaEngineRMF::_instance->onError(payload);
            }   
        }


        MediaEngineRMF::MediaEngineRMF()
            : PluginHost::JSONRPC(), m_player_state(player_state::UNINITIALIZED)
        {
            LOGINFO("ctor");

            MediaEngineRMF::_instance = this;
            Register(METHOD_PLAY, &MediaEngineRMF::play, this);
            Register(METHOD_STOP, &MediaEngineRMF::stop, this);
            Register(METHOD_INITIALIZE, &MediaEngineRMF::initialize_player, this);
            Register(METHOD_DEINITIALIZE, &MediaEngineRMF::deinitialize_player, this);
            Register(METHOD_SET_VOLUME, &MediaEngineRMF::setVolume, this);
            Register(METHOD_SET_VIDEO_RECTANGLE, &MediaEngineRMF::setVideoRectangle, this);
            Register(METHOD_MUTE, &MediaEngineRMF::mute, this);


        }

        MediaEngineRMF::~MediaEngineRMF()
        {
            //LOGINFO("dtor");
            Unregister(METHOD_PLAY);
            Unregister(METHOD_STOP);
            Unregister(METHOD_INITIALIZE);
            Unregister(METHOD_DEINITIALIZE);
            Unregister(METHOD_SET_VOLUME);
            Unregister(METHOD_SET_VIDEO_RECTANGLE);
            Unregister(METHOD_MUTE);
        }

        const string MediaEngineRMF::Initialize(PluginHost::IShell* /* service */)
        {
            return "";
        }

        void MediaEngineRMF::Deinitialize(PluginHost::IShell* /* service */)
        {
            MediaEngineRMF::_instance = nullptr;
        }

        string MediaEngineRMF::Information() const
        {
            return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
        }

        // Registered methods begin
        //parameters: {"source_type" : "qam"}
        uint32_t MediaEngineRMF::initialize_player(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success  = false;
            returnIfStringParamNotFound(parameters, "source_type");
            const std::string & type = parameters["source_type"].String();
            if("qam" == type)
            {
                if(player_state::PLATFORM_INITIALIZED <= m_player_state)
                {
                    LOGINFO("QAM is already initialized.");
                    success = true;
                }
                else
                {
                    LOGINFO("Initializing QAM support.");
                    set_env_variables();
                    if(0 != mediaplayer::initialize(QAM, true, true))
                    {
                        LOGERR("Could not initialize QAM support");
                    }
                    else
                    {
                        m_player_state = player_state::PLATFORM_INITIALIZED;
                        success = true;
                    }
                }
            }
            else
            {
                LOGERR("Unsupported source type: %s", type.c_str());
            }
            returnResponse(success);
        }

        //parameters: {"source_type" : "qam"}
        uint32_t MediaEngineRMF::deinitialize_player(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success  = false;
            if(player_state::PLATFORM_INITIALIZED != m_player_state)
            {
                LOGERR("Wrong state. Cannot deinitialize.");
            }
            else
            {
                returnIfStringParamNotFound(parameters, "source_type");
                const std::string & type = parameters["source_type"].String();
                if("qam" == type)
                {
                    LOGINFO("De-initializing QAM support.");
                    if(0 != mediaplayer::deinitialize(QAM))
                    {
                        LOGERR("Could not deinitialize QAM support");
                    }
                    else
                    {
                        m_player_state = player_state::UNINITIALIZED;
                        success = true;
                    }
                }
            }
            returnResponse(success);
        }

        //parameters: {"source_type" : "qam", "identifier" : "ocap://0x123a"} 
        uint32_t MediaEngineRMF::play(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success  = false;
            returnIfStringParamNotFound(parameters, "source_type");
            returnIfStringParamNotFound(parameters, "identifier");

            if(player_state::PLATFORM_INITIALIZED != m_player_state)
            {
                LOGERR("Platform not initialized");
            }
            else
            {
                const std::string source_type = parameters["source_type"].String();
                const std::string identifier = parameters["identifier"].String();
                if("qam" == source_type)
                {
                    //Process for qam mediaplayer.
                    if((std::string::npos != identifier.find("ocap://0x")|| (std::string::npos != identifier.find("tune://")) )
                    {
                        m_player = std::unique_ptr <mediaplayer> (mediaplayer::createMediaPlayer(QAM, identifier));
                        if(nullptr == m_player)
                        {
                            LOGERR("Media player creation failed.");
                        }
                        else
                        {
                            m_player_state = player_state::INSTANTIATED;
                            LOGINFO("Successfully created media player.");
                            m_player->registerEventCallbacks(event_callback, error_callback, nullptr);
                            m_player->setVideoRectangle(m_video_rectangle.x, m_video_rectangle.y, m_video_rectangle.width, m_video_rectangle.height);

                            if(0 != m_player->play())
                            {
                                LOGERR("Play failed.");
                            }
                            else
                            {
                                success = true;
                                m_player_state = player_state::PLAYING;
                                LOGINFO("Play call returned successfully");
                            }
                        }
                    }
                    else
                    {
                        LOGERR("Identifier is not an OCAP locator.");
                    }
                }
                else
                {
                    LOGERR("Unsupported source type.");
                }
            }
            returnResponse(success);
        }

        //parameters: {}
        uint32_t MediaEngineRMF::stop(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success  = false;
            if(player_state::PLAYING != m_player_state)
            {
                LOGERR("Player is not currently playing.");
            }
            else
            {
                m_player->registerEventCallbacks(nullptr, nullptr, nullptr);
                if(0 != m_player->stop())
                {
                    LOGERR("Stop failed.");
                }
                m_player_state = player_state::PLATFORM_INITIALIZED;
                m_player.reset();
                success  = true;
            }
            returnResponse(success);
        }

        //parameters: {"mute" : true}
        uint32_t MediaEngineRMF::mute(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success  = false;
            returnIfBooleanParamNotFound(parameters, "mute");
            bool mute = parameters["mute"].Boolean();

            if(player_state::INSTANTIATED > m_player_state)
            {
                LOGERR("Cannot mute before creating player.");
            }
            else
            {
                m_player->mute(mute);
                success  = true;
            }
            returnResponse(success);
        }
        //parameters: {"volume" : 0.5}
        uint32_t MediaEngineRMF::setVolume(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool success  = false;
            returnIfNumberParamNotFound(parameters, "volume");
            
            float volume = static_cast <float> (parameters["volume"].Float());

            if(player_state::INSTANTIATED > m_player_state)
            {
                LOGERR("Cannot set volume before creating player.");
            }
            else
            {
                m_player->setVolume(volume);
                success  = true;
            }
            returnResponse(success);
        }

        //parameters: {"video_rectangle" : {"x" : 0, "y": 0, "width" : 1080, "height" : 720}}
        uint32_t MediaEngineRMF::setVideoRectangle(const JsonObject& parameters, JsonObject& response)
        {
            const char * object_label = "video_rectangle";
            LOGINFOMETHOD();
            bool success  = false;

            if(!parameters.HasLabel(object_label) || parameters[object_label].Content() != WPEFramework::Core::JSON::Variant::type::OBJECT)
            {
                LOGERR("No argument '%s' or it has incorrect type", object_label);
                returnResponse(false);
            }
            JsonObject rect = parameters[object_label].Object();

            returnIfNumberParamNotFound(rect, "x");
            returnIfNumberParamNotFound(rect, "y");
            returnIfNumberParamNotFound(rect, "width");
            returnIfNumberParamNotFound(rect, "height");
            
            m_video_rectangle.x = rect["x"].Number();
            m_video_rectangle.y = rect["x"].Number();
            m_video_rectangle.width = rect["width"].Number();
            m_video_rectangle.height = rect["height"].Number();

            if(player_state::INSTANTIATED > m_player_state)
            {
                LOGERR("Player not created. Caching video rectangle parameters.");
            }
            else
            {
                m_player->setVideoRectangle(m_video_rectangle.x, m_video_rectangle.y, m_video_rectangle.width, m_video_rectangle.height);
            }

            success  = true;
            returnResponse(success);
        }

        void MediaEngineRMF::onEvent(notification_payload * payload)
        {
            JsonObject params;
            params["title"] = payload->m_title;
            params["source"] = payload->m_source;
            params["code"] = payload->m_code;
            params["message"] = payload->m_message;
            sendNotify(EVT_ON_STATUS.c_str(), params);
        }

        void MediaEngineRMF::onError(notification_payload * payload)
        {
            JsonObject params;
            params["title"] = payload->m_title;
            params["source"] = payload->m_source;
            params["code"] = payload->m_code;
            params["message"] = payload->m_message;
            sendNotify(EVT_ON_ERROR.c_str(), params);
        }

        // Internal methods end
    } // namespace Plugin
} // namespace WPEFramework
