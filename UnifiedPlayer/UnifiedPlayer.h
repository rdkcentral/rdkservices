/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright Synamedia, All rights reserved
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

#ifndef __UNIFIED_PLAYER_H__
#define __UNIFIED_PLAYER_H__

#include <mutex>
#include "Module.h"
#include "utils.h"
#include "AbstractPlugin.h"
#include <websocket/websocket.h>
#include <interfaces/json/JsonData_UnifiedPlayer.h>

#include "RTPlayer.h"
#include "UnifiedPlayerNotify.h"

// Annoying defect in auto-generation means this class is mistaken for Floatarray
namespace WPEFramework {
namespace JsonData {
    namespace UnifiedPlayer {
            class StringarrayresultInfo : public Core::JSON::Container {
            public:
                StringarrayresultInfo()
                    : Core::JSON::Container()
                {
                    Add(_T("success"), &Success);
                    Add(_T("value"), &Value);
                    Add(_T("failurereason"), &Failurereason);
                }

                StringarrayresultInfo(const StringarrayresultInfo&) = delete;
                StringarrayresultInfo& operator=(const StringarrayresultInfo&) = delete;

            public:
                Core::JSON::Boolean Success; // Returning whether this method failed or succeed
                Core::JSON::ArrayType<Core::JSON::String> Value; // The returned string array value
                Core::JSON::DecUInt32 Failurereason; // Reason why it's failed
            }; // class StringarrayresultInfo
    }
}
}


namespace WPEFramework {

    namespace Plugin {
        using namespace JsonData::UnifiedPlayer;

        class UnifiedPlayer :  public AbstractPlugin, public WPEFramework::UnifiedPlayerNotify {
        public:
            UnifiedPlayer();
            virtual ~UnifiedPlayer();

        public/*members*/:
            static UnifiedPlayer* _instance;

        public /*constants*/:
            static const short API_VERSION_NUMBER_MAJOR;
            static const short API_VERSION_NUMBER_MINOR;
            static const string SERVICE_NAME;
            //methods
            static const string METHOD_GET_API_VERSION_NUMBER;
            static const string METHOD_OPEN;
            static const string METHOD_PRETUNE;
            static const string METHOD_PARK;
            static const string METHOD_DESTROY;
            static const string METHOD_SET_POSITION;
            static const string METHOD_SET_POSITION_RELATIVE;
            static const string METHOD_GET_POSITION;
            static const string METHOD_SET_SPEED;
            static const string METHOD_GET_SPEED;
            static const string METHOD_SET_MUTE;
            static const string METHOD_GET_MUTE;
            static const string METHOD_GET_DURATION;
            static const string METHOD_GET_AVAILABLE_SPEEDS;
            static const string METHOD_SET_DEFAULT_AUDIO_LANGUAGE;
            static const string METHOD_GET_DEFAULT_AUDIO_LANGUAGE;
            static const string METHOD_SET_PRIMARY_AUDIO_LANGUAGE;
            static const string METHOD_GET_PRIMARY_AUDIO_LANGUAGE;
            static const string METHOD_SET_SECONDARY_AUDIO_LANGUAGE;
            static const string METHOD_GET_SECONDARY_AUDIO_LANGUAGE;
            static const string METHOD_GET_CURRENT_AUDIO_LANGUAGE;
            static const string METHOD_GET_AVAILABLE_AUDIO_LANGUAGES;
            static const string METHOD_SET_DEFAULT_SUBTITLES_LANGUAGE;
            static const string METHOD_GET_DEFAULT_SUBTITLES_LANGUAGE;
            static const string METHOD_SET_PRIMARY_SUBTITLES_LANGUAGE;
            static const string METHOD_GET_PRIMARY_SUBTITLES_LANGUAGE;
            static const string METHOD_GET_CURRENT_SUBTITLES_LANGUAGE;
            static const string METHOD_GET_AVAILABLE_SUBTITLES_LANGUAGES;
            static const string METHOD_SET_SUBTITLES_ENABLED;
            static const string METHOD_GET_SUBTITLES_ENABLED;
            static const string METHOD_SET_SUBTITLES_DISPLAYED;
            static const string METHOD_GET_SUBTITLES_DISPLAYED;
            static const string METHOD_SET_VIDEO_RECTANGLE;
            static const string METHOD_PLAY;
            static const string METHOD_PAUSE;
            static const string METHOD_SEEK_TO_LIVE;
            static const string METHOD_STOP;
            static const string METHOD_REQUEST_STATUS;

        private/*registered methods*/:
            void RegisterAll();
            void UnregisterAll();
            // Note: `JsonObject& parameters` corresponds to `params` in JSON RPC call
            //methods
            uint32_t getApiVersionNumber(const JsonObject& parameters, JsonObject& response);
            uint32_t endpoint_open(const JsonData::UnifiedPlayer::MediarequestInfo& params, JsonData::UnifiedPlayer::SessionidresultInfo& response);

            uint32_t endpoint_pretune(const JsonData::UnifiedPlayer::MediarequestInfo& params, JsonData::UnifiedPlayer::SessionidresultInfo& response);

            // Method: park - Park an active non-HTTP source for later use
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_park(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: destroy - Destroy a media playback session
            // Return codes:
            //  - ERROR_NONE: Success`
            uint32_t endpoint_destroy(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: setposition - Sets playback position
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_setposition(const JsonData::UnifiedPlayer::FloatsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: setpositionrelative - Sets playback position relative to current position, aka skip
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_setpositionrelative(const JsonData::UnifiedPlayer::FloatsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: getposition - Gets playback position
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getposition(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::FloatresultInfo& response);

            // Method: setspeed - Sets playback speed
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_setspeed(const JsonData::UnifiedPlayer::FloatsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: getspeed - Gets playback speed
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getspeed(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::FloatresultInfo& response);

            // Method: getduration - Gets duration of Closed Assests
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getduration(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::FloatresultInfo& response);

            // Method: getavailablespeeds - Gets supported playback speeds
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getavailablespeeds(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::FloatarrayresultInfo& response);

            // Method: setdefaultaudiolanguage - Sets default audio language
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_setdefaultaudiolanguage(const JsonData::UnifiedPlayer::StringsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: getdefaultaudiolanguage - Gets default audio language
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getdefaultaudiolanguage(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::StringresultInfo& response);

            // Method: setprimaryaudiolanguage - Sets primary, presenting, audio language
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_setprimaryaudiolanguage(const JsonData::UnifiedPlayer::StringsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: getprimaryaudiolanguage - Gets primary audio language
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getprimaryaudiolanguage(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::StringresultInfo& response);

            // Method: setsecondaryaudiolanguage - Sets secondary, audio-description, audio language
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_setsecondaryaudiolanguage(const JsonData::UnifiedPlayer::StringsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: getsecondaryaudiolanguage - Gets secondary audio language
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getsecondaryaudiolanguage(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::StringresultInfo& response);

            // Method: getcurrentaudiolanguage - Gets the audio language currently presented to the user
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getcurrentaudiolanguage(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::StringresultInfo& response);

            // Method: getavailableaudiolanguages - Gets all the audio languages currently in the PMT or Manifest
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getavailableaudiolanguages(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::StringarrayresultInfo& response);

            // Method: setdefaultsubtitleslanguage - Sets default subtitles language
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_setdefaultsubtitleslanguage(const JsonData::UnifiedPlayer::StringsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: getdefaultsubtitleslanguage - Gets default subtitles language
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getdefaultsubtitleslanguage(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::StringresultInfo& response);

            // Method: setprimarysubtitleslanguage - Sets primary, presenting, subtitles language
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_setprimarysubtitleslanguage(const JsonData::UnifiedPlayer::StringsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: getprimarysubtitleslanguage - Gets primary subtitles language
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getprimarysubtitleslanguage(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::StringresultInfo& response);

            // Method: getcurrentsubtitleslanguage - Gets the subtitles language currently presented to the user
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getcurrentsubtitleslanguage(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::StringresultInfo& response);

            // Method: getavailablesubtitleslanguages - Gets all the audio languages currently in the PMT or Manifest
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getavailablesubtitleslanguages(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::StringarrayresultInfo& response);

            // Method: setmute - Sets the mute status of the presentation
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_setmute(const JsonData::UnifiedPlayer::MutecontrolsessionvalueData& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: getmute - Gets the mute status of the presentation
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getmute(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::GetmuteResultData& response);

        // Method: setsubtitlesenabled - Sets if subtitles are decoded and presented
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_setsubtitlesenabled(const JsonData::UnifiedPlayer::BooleansessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: getsubtitlesenabled - Gets if subtitles are enabled
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getsubtitlesenabled(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::BooleanresultInfo& response);

            // Method: setsubtitlesdisplayed - Sets if subtitles are presented
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_setsubtitlesdisplayed(const JsonData::UnifiedPlayer::BooleansessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: getsubtitlesdisplayed - Gets if subtitles are presented
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_getsubtitlesdisplayed(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::BooleanresultInfo& response);

            // Method: setvideorectangle - Sets the size and position of the video
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_setvideorectangle(const JsonData::UnifiedPlayer::RectanglesessionvalueData& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: play - Play the Video at x1 speed, HTTP sources only
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_play(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: pause - Pauses the video, HTTP sources only
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_pause(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: seektolive - seeks to the live point, HTTP sources only, non-closed manifests only
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_seektolive(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: stop - Stops playback and closes the source
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_stop(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response);

            // Method: requeststatus - Causes a status event to be triggered
            // Return codes:
            //  - ERROR_NONE: Success
            uint32_t endpoint_requeststatus(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response);
        public:
            // All events are defined in UnifiedPlayerNotify.h
            // Event: onplayerinitialized - Sent when the player has accepted a media URL
            void event_onplayerinitialized(const uint32_t& sessionid) override;

            // Event: onmediaopened - Sent when media is successfully opened
            void event_onmediaopened(const uint32_t& sessionid, const string& mediatype, const uint32_t& duration, const uint32_t& width, const uint32_t& height, const std::string& availablespeeds, const std::string& availableaudiolanguages, const std::string& availablesubtitleslanguages) override;

            // Event: onplaying - Sent when the player transitions to x1 play speed
            void event_onplaying(const uint32_t& sessionid) override;

            // Event: onpaused - Sent when the player enters a paused state
            void event_onpaused(const uint32_t& sessionid) override;

            // Event: onclosed - Sent when media is closed
            void event_onclosed(const uint32_t& sessionid) override;

            // Event: oncomplete - Sent when the stream complete
            void event_oncomplete(const uint32_t& sessionid) override;

            // Event: onstatus - Sent when status changes
            void event_onstatus(const uint32_t& sessionid, const float& position, const float& duration) override;

            // Event: onprogress - Sent when progress or duration changes
            void event_onprogress(const uint32_t& sessionid, const float& position, const float& duration, const float& speed, const float& start, const float& end) override;

            // Event: onwarning - Sent when a warning is issued
            void event_onwarning(const uint32_t& sessionid, const uint32_t& code, const string& description) override;

            // Event: onerror - Sent when an error is issued
            void event_onerror(const uint32_t& sessionid, const uint32_t& code, const string& description) override;

            // Event: onbitratechange - Sent when current ABR bitrate changes
            void event_onbitratechange(const uint32_t& sessionid, const uint32_t& bitrate, const string& reason) override;

            // Event: oncasdata - CAS public data is sent asynchronously from CAS System
            void event_oncasdata(const uint32_t& sessionid, const string& casData) override;

            //Events from rtRemote Interface
            void onPlayerStatusChanged(string playerState);
            void onPlaybackRateChanged(float speed);
            

        private/*internal methods*/:
            UnifiedPlayer(const UnifiedPlayer&) = delete;
            UnifiedPlayer& operator=(const UnifiedPlayer&) = delete;
            //Convert comma separated value to array and store it in response
            void convertPlainStringToFloatVector(const std::string& str, std::vector<float>& vec);
            void convertPlainStringToStrVector(const std::string& plainStr, std::vector<std::string>& vec);
            template<class ArrayType, class VectorType>
            void addToArrayParamter(Core::JSON::ArrayType<ArrayType>& param, std::vector<VectorType>& vec);

        private/*members*/:
            uint32_t m_apiVersionNumber;
            std::shared_ptr<WPEFramework::RTPlayer>  m_RTPlayer;
        };
    } // namespace Plugin

} // namespace WPEFramework

#endif