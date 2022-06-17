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

#include "UnifiedPlayer.h"
#include "UnifiedError.h"

const short WPEFramework::Plugin::UnifiedPlayer::API_VERSION_NUMBER_MAJOR = 1;
const short WPEFramework::Plugin::UnifiedPlayer::API_VERSION_NUMBER_MINOR = 0;
const string WPEFramework::Plugin::UnifiedPlayer::SERVICE_NAME = "org.rdk.UnifiedPlayer";
//methods
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_API_VERSION_NUMBER = "getApiVersionNumber";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_OPEN ="open";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_PRETUNE ="pretune";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_PARK ="park";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_DESTROY ="destroy";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_SET_POSITION ="setposition";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_SET_POSITION_RELATIVE ="setpositionrelative";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_POSITION ="getposition";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_SET_SPEED ="setspeed";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_SPEED ="getspeed";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_SET_MUTE ="setmute";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_MUTE ="getmute";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_DURATION ="getduration";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_AVAILABLE_SPEEDS ="getavailablespeeds";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_SET_DEFAULT_AUDIO_LANGUAGE ="setdefaultaudiolanguage";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_DEFAULT_AUDIO_LANGUAGE ="getdefaultaudiolanguage";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_SET_PRIMARY_AUDIO_LANGUAGE ="setprimaryaudiolanguage";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_PRIMARY_AUDIO_LANGUAGE ="getprimaryaudiolanguage";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_SET_SECONDARY_AUDIO_LANGUAGE ="setsecondaryaudiolanguage";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_SECONDARY_AUDIO_LANGUAGE ="getsecondaryaudiolanguage";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_CURRENT_AUDIO_LANGUAGE ="getcurrentaudiolanguage";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_AVAILABLE_AUDIO_LANGUAGES ="getavailableaudiolanguages";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_SET_DEFAULT_SUBTITLES_LANGUAGE ="setdefaultsubtitleslanguage";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_DEFAULT_SUBTITLES_LANGUAGE ="getdefaultsubtitleslanguage";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_SET_PRIMARY_SUBTITLES_LANGUAGE ="setprimarysubtitleslanguage";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_PRIMARY_SUBTITLES_LANGUAGE ="getprimarysubtitleslanguage";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_CURRENT_SUBTITLES_LANGUAGE ="getcurrentsubtitleslanguage";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_AVAILABLE_SUBTITLES_LANGUAGES ="getavailablesubtitleslanguages";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_SET_SUBTITLES_ENABLED ="setsubtitlesenabled";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_SUBTITLES_ENABLED ="getsubtitlesenabled";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_SET_SUBTITLES_DISPLAYED ="setsubtitlesdisplayed";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_GET_SUBTITLES_DISPLAYED ="getsubtitlesdisplayed";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_SET_VIDEO_RECTANGLE ="setvideorectangle";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_PLAY ="play";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_PAUSE ="pause";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_SEEK_TO_LIVE ="seektolive";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_STOP ="stop";
const string WPEFramework::Plugin::UnifiedPlayer::METHOD_REQUEST_STATUS ="requeststatus";
//events

using namespace std;

namespace WPEFramework {

    namespace Plugin {

        using namespace JsonData::UnifiedPlayer;

        SERVICE_REGISTRATION(UnifiedPlayer, 1, 0);

        UnifiedPlayer* UnifiedPlayer::_instance = nullptr;

        UnifiedPlayer::UnifiedPlayer()
            : AbstractPlugin()
            , m_apiVersionNumber(API_VERSION_NUMBER_MAJOR)
        {
            UnifiedPlayer::_instance = this;
            m_RTPlayer = std::make_shared<RTPlayer>(this);

            RegisterAll();
        }
        
        void UnifiedPlayer::RegisterAll()
        {
            Register<JsonObject, JsonObject>(METHOD_GET_API_VERSION_NUMBER, &UnifiedPlayer::getApiVersionNumber, this);
            Register<MediarequestInfo,SessionidresultInfo>(_T(METHOD_OPEN), &UnifiedPlayer::endpoint_open, this);
            Register<MediarequestInfo,SessionidresultInfo>(_T(METHOD_PRETUNE), &UnifiedPlayer::endpoint_pretune, this);
            Register<WPEFramework::Core::JSON::DecUInt32,ResultInfo>(_T(METHOD_PARK), &UnifiedPlayer::endpoint_park, this);
            Register<WPEFramework::Core::JSON::DecUInt32,ResultInfo>(_T(METHOD_DESTROY), &UnifiedPlayer::endpoint_destroy, this);
            Register<FloatsessionvalueInfo,ResultInfo>(_T(METHOD_SET_POSITION), &UnifiedPlayer::endpoint_setposition, this);
            Register<FloatsessionvalueInfo,ResultInfo>(_T(METHOD_SET_POSITION_RELATIVE), &UnifiedPlayer::endpoint_setpositionrelative, this);
            Register<WPEFramework::Core::JSON::DecUInt32,FloatresultInfo>(_T(METHOD_GET_POSITION), &UnifiedPlayer::endpoint_getposition, this);
            Register<FloatsessionvalueInfo,JsonData::UnifiedPlayer::ResultInfo>(_T(METHOD_SET_SPEED), &UnifiedPlayer::endpoint_setspeed, this);
            Register<WPEFramework::Core::JSON::DecUInt32,FloatresultInfo>(_T(METHOD_GET_SPEED), &UnifiedPlayer::endpoint_getspeed, this);
            Register<WPEFramework::Core::JSON::DecUInt32,FloatresultInfo>(_T(METHOD_GET_DURATION), &UnifiedPlayer::endpoint_getduration, this);
            Register<WPEFramework::Core::JSON::DecUInt32,FloatarrayresultInfo>(_T(METHOD_GET_AVAILABLE_SPEEDS), &UnifiedPlayer::endpoint_getavailablespeeds, this);
            Register<StringsessionvalueInfo,ResultInfo>(_T(METHOD_SET_DEFAULT_AUDIO_LANGUAGE), &UnifiedPlayer::endpoint_setdefaultaudiolanguage, this);
            Register<WPEFramework::Core::JSON::DecUInt32,StringresultInfo>(_T(METHOD_GET_DEFAULT_AUDIO_LANGUAGE), &UnifiedPlayer::endpoint_getdefaultaudiolanguage, this);
            Register<StringsessionvalueInfo,ResultInfo>(_T(METHOD_SET_PRIMARY_AUDIO_LANGUAGE), &UnifiedPlayer::endpoint_setprimaryaudiolanguage, this);
            Register<WPEFramework::Core::JSON::DecUInt32,StringresultInfo>(_T(METHOD_GET_PRIMARY_AUDIO_LANGUAGE), &UnifiedPlayer::endpoint_getprimaryaudiolanguage, this);
            Register<StringsessionvalueInfo,ResultInfo>(_T(METHOD_SET_SECONDARY_AUDIO_LANGUAGE), &UnifiedPlayer::endpoint_setsecondaryaudiolanguage, this);
            Register<WPEFramework::Core::JSON::DecUInt32,StringresultInfo>(_T(METHOD_GET_SECONDARY_AUDIO_LANGUAGE), &UnifiedPlayer::endpoint_getsecondaryaudiolanguage, this);
            Register<WPEFramework::Core::JSON::DecUInt32,StringresultInfo>(_T(METHOD_GET_CURRENT_AUDIO_LANGUAGE), &UnifiedPlayer::endpoint_getcurrentaudiolanguage, this);
            Register<WPEFramework::Core::JSON::DecUInt32,StringarrayresultInfo>(_T(METHOD_GET_AVAILABLE_AUDIO_LANGUAGES), &UnifiedPlayer::endpoint_getavailableaudiolanguages, this);
            Register<StringsessionvalueInfo,ResultInfo>(_T(METHOD_SET_DEFAULT_SUBTITLES_LANGUAGE), &UnifiedPlayer::endpoint_setdefaultsubtitleslanguage, this);
            Register<WPEFramework::Core::JSON::DecUInt32,StringresultInfo>(_T(METHOD_GET_DEFAULT_SUBTITLES_LANGUAGE), &UnifiedPlayer::endpoint_getdefaultsubtitleslanguage, this);
            Register<StringsessionvalueInfo,ResultInfo>(_T(METHOD_SET_PRIMARY_SUBTITLES_LANGUAGE), &UnifiedPlayer::endpoint_setprimarysubtitleslanguage, this);
            Register<WPEFramework::Core::JSON::DecUInt32,StringresultInfo>(_T(METHOD_GET_PRIMARY_SUBTITLES_LANGUAGE), &UnifiedPlayer::endpoint_getprimarysubtitleslanguage, this);
            Register<WPEFramework::Core::JSON::DecUInt32,StringresultInfo>(_T(METHOD_GET_CURRENT_SUBTITLES_LANGUAGE), &UnifiedPlayer::endpoint_getcurrentsubtitleslanguage, this);
            Register<WPEFramework::Core::JSON::DecUInt32,StringarrayresultInfo>(_T(METHOD_GET_AVAILABLE_SUBTITLES_LANGUAGES), &UnifiedPlayer::endpoint_getavailablesubtitleslanguages, this);
            Register<MutecontrolsessionvalueData,ResultInfo>(_T(METHOD_SET_MUTE), &UnifiedPlayer::endpoint_setmute, this);
            Register<WPEFramework::Core::JSON::DecUInt32,GetmuteResultData>(_T(METHOD_GET_MUTE), &UnifiedPlayer::endpoint_getmute, this);
            Register<BooleansessionvalueInfo,ResultInfo>(_T(METHOD_SET_SUBTITLES_ENABLED), &UnifiedPlayer::endpoint_setsubtitlesenabled, this);
            Register<WPEFramework::Core::JSON::DecUInt32,BooleanresultInfo>(_T(METHOD_GET_SUBTITLES_ENABLED), &UnifiedPlayer::endpoint_getsubtitlesenabled, this);
            Register<BooleansessionvalueInfo,ResultInfo>(_T(METHOD_SET_SUBTITLES_DISPLAYED), &UnifiedPlayer::endpoint_setsubtitlesdisplayed, this);
            Register<WPEFramework::Core::JSON::DecUInt32,BooleanresultInfo>(_T(METHOD_GET_SUBTITLES_DISPLAYED), &UnifiedPlayer::endpoint_getsubtitlesdisplayed, this);
            Register<RectanglesessionvalueData,ResultInfo>(_T(METHOD_SET_VIDEO_RECTANGLE), &UnifiedPlayer::endpoint_setvideorectangle, this);
            Register<WPEFramework::Core::JSON::DecUInt32,ResultInfo>(_T(METHOD_PLAY), &UnifiedPlayer::endpoint_play, this);
            Register<WPEFramework::Core::JSON::DecUInt32,ResultInfo>(_T(METHOD_PAUSE), &UnifiedPlayer::endpoint_pause, this);
            Register<WPEFramework::Core::JSON::DecUInt32,ResultInfo>(_T(METHOD_SEEK_TO_LIVE), &UnifiedPlayer::endpoint_seektolive, this);
            Register<WPEFramework::Core::JSON::DecUInt32,ResultInfo>(_T(METHOD_STOP), &UnifiedPlayer::endpoint_stop, this);
            Register<WPEFramework::Core::JSON::DecUInt32,ResultInfo>(_T(METHOD_REQUEST_STATUS), &UnifiedPlayer::endpoint_requeststatus, this);
        }

        UnifiedPlayer::~UnifiedPlayer()
        {
            UnregisterAll();
            UnifiedPlayer::_instance = nullptr;
        }
        
        void UnifiedPlayer::UnregisterAll()
        {
            // LOGINFO("dtor");
            Unregister(_T(METHOD_REQUEST_STATUS));
            Unregister(_T(METHOD_STOP));
            Unregister(_T(METHOD_SEEK_TO_LIVE));
            Unregister(_T(METHOD_PAUSE));
            Unregister(_T(METHOD_PLAY));
            Unregister(_T(METHOD_SET_VIDEO_RECTANGLE));
            Unregister(_T(METHOD_GET_SUBTITLES_DISPLAYED));
            Unregister(_T(METHOD_SET_SUBTITLES_DISPLAYED));
            Unregister(_T(METHOD_GET_SUBTITLES_ENABLED));
            Unregister(_T(METHOD_SET_SUBTITLES_ENABLED));
            Unregister(_T(METHOD_GET_MUTE));
            Unregister(_T(METHOD_SET_MUTE));
            Unregister(_T(METHOD_GET_AVAILABLE_SUBTITLES_LANGUAGES));
            Unregister(_T(METHOD_GET_CURRENT_SUBTITLES_LANGUAGE));
            Unregister(_T(METHOD_GET_PRIMARY_SUBTITLES_LANGUAGE));
            Unregister(_T(METHOD_SET_PRIMARY_SUBTITLES_LANGUAGE));
            Unregister(_T(METHOD_GET_DEFAULT_SUBTITLES_LANGUAGE));
            Unregister(_T(METHOD_SET_DEFAULT_SUBTITLES_LANGUAGE));
            Unregister(_T(METHOD_GET_AVAILABLE_AUDIO_LANGUAGES));
            Unregister(_T(METHOD_GET_CURRENT_AUDIO_LANGUAGE));
            Unregister(_T(METHOD_GET_SECONDARY_AUDIO_LANGUAGE));
            Unregister(_T(METHOD_SET_SECONDARY_AUDIO_LANGUAGE));
            Unregister(_T(METHOD_GET_PRIMARY_AUDIO_LANGUAGE));
            Unregister(_T(METHOD_SET_PRIMARY_AUDIO_LANGUAGE));
            Unregister(_T(METHOD_GET_DEFAULT_AUDIO_LANGUAGE));
            Unregister(_T(METHOD_SET_DEFAULT_AUDIO_LANGUAGE));
            Unregister(_T(METHOD_GET_AVAILABLE_SPEEDS));
            Unregister(_T(METHOD_GET_DURATION));
            Unregister(_T(METHOD_GET_SPEED));
            Unregister(_T(METHOD_SET_SPEED));
            Unregister(_T(METHOD_GET_POSITION));
            Unregister(_T(METHOD_SET_POSITION_RELATIVE));
            Unregister(_T(METHOD_SET_POSITION));
            Unregister(_T(METHOD_DESTROY));
            Unregister(_T(METHOD_PARK));
            Unregister(_T(METHOD_PRETUNE));
            Unregister(_T(METHOD_OPEN));
        }

        // Registered methods begin
        uint32_t UnifiedPlayer::getApiVersionNumber(const JsonObject& parameters, JsonObject& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_NONE;
            UNUSED(parameters);

            response["version"] = m_apiVersionNumber;
            return result;
        }

        // Method: open - opens a Live/ OTT source
        // Return codes:
        //  - ERROR_NONE: Success
        //  - ERROR_BAD_REQUEST: Unsupported model
        uint32_t UnifiedPlayer::endpoint_open(const MediarequestInfo& params, SessionidresultInfo& response)
        {
            LOGDBG("UnifiedPlayer open called");
            rtError _error = RT_OK;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            const string& mediaurl = params.Mediaurl.Value();
            const ModeType& mode = params.Mode.Value();
            const ManageType& manage = params.Manage.Value();
            const string& casinitdata = params.Casinitdata.Value();
            const string& casocdmid = params.Casocdmid.Value();
            bool muteAudio = params.Mute.Audio.Value();
            bool muteVideo = params.Mute.Video.Value();
            LOGDBG("media URL:%s, ocdmid = %s", mediaurl.c_str(), casocdmid.c_str());

            if((mode == ModeType::MODE_NONE) &&
                (manage == ManageType::MANAGE_FULL || manage == ManageType::MANAGE_NO_PSI || manage == ManageType::MANAGE_NO_TUNER))
            {
                if(casocdmid.empty() || (manage == ManageType::MANAGE_FULL && mediaurl.empty())) {
                    LOGINFO("ocdmcasid is mandatory for CAS management session");
                    _error = RT_ERROR;
                }
            }
            else if ((mode == ModeType::MODE_LIVE || mode == ModeType::MODE_PLAYBACK) && manage == ManageType::MANAGE_NONE) {
                if (mediaurl.empty())
                {
                    LOGINFO("MediaUrl is mandatory when ModeType is LIVE, PLAYBACK");
                    _error = RT_ERROR;
                }
            }
            else {
                LOGINFO("Invalid Option to Open media or cas management session ...");
                _error = RT_ERROR;
            }

            if(_error != RT_OK)
            {
                LOGDBG("UnifiedPlayer Open Session Failed");
                result = RTERROR_TO_WPEERROR(_error);
                RTERROR_TO_RESPONSE(_error);
                return result;
            }

            JsonObject jsonParams;
            jsonParams["mediaurl"] = mediaurl;
            switch(mode){
                case ModeType::MODE_NONE:
                    jsonParams["mode"] = "MODE_NONE";
                    break;
                case ModeType::MODE_LIVE:
                    jsonParams["mode"] = "MODE_LIVE";
                    break;
                case ModeType::MODE_RECORD:
                    jsonParams["mode"] = "MODE_RECORD";
                    break;
                case ModeType::MODE_PLAYBACK:
                    jsonParams["mode"] = "MODE_PLAYBACK";
                    break;
            }
            switch(manage){
                case ManageType::MANAGE_NONE:
                    jsonParams["manage"] = "MANAGE_NONE";
                    break;
                case ManageType::MANAGE_FULL:
                    jsonParams["manage"] = "MANAGE_FULL";
                    break;
                case ManageType::MANAGE_NO_PSI:
                    jsonParams["manage"] = "MANAGE_NO_PSI";
                    break;
                case ManageType::MANAGE_NO_TUNER:
                    jsonParams["manage"] = "MANAGE_NO_TUNER";
                    break;
            }
            jsonParams["casinitdata"] = casinitdata;
            jsonParams["casocdmid"] = casocdmid;
            jsonParams["muteaudio"] = muteAudio;
            jsonParams["mutevideo"] = muteVideo;
            
            std::string openParams;
            uint32_t sessionId;
            response.Sessionid = UNDEFINED_SESSION_ID;
            jsonParams.ToString(openParams);
            LOGDBG("OpenData = %s\n", openParams.c_str());
            _error = m_RTPlayer->open(openParams, &sessionId);

            if (_error == RT_OK ) {
                response.Sessionid = sessionId;
                LOGDBG("UnifiedPlayer set open succeeded with session Id %d", sessionId);
            }
            else {
                LOGWARN("Error in Opening media session...");
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);
            return result;
        }

        // Method: pretune - Pretune a non-HTTP source
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_pretune(const MediarequestInfo& params, JsonData::UnifiedPlayer::SessionidresultInfo& response)
        {
	    LOGDBG("UnifiedPlayer pretune called");

            rtError _error = RT_OK;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            const string& mediaurl = params.Mediaurl.Value();
            const ModeType& mode = params.Mode.Value();
            const ManageType& manage = params.Manage.Value();
            const string& casinitdata = params.Casinitdata.Value();
            const string& casocdmid = params.Casocdmid.Value();
            bool muteAudio = params.Mute.Audio.Value();
            bool muteVideo = params.Mute.Video.Value();
            LOGDBG("media URL:%s, ocdmid = %s", mediaurl.c_str(), casocdmid.c_str());

            JsonObject jsonParams;
            jsonParams["mediaurl"] = mediaurl;
            switch(mode){
                case ModeType::MODE_NONE:
                    jsonParams["mode"] = "MODE_NONE";
                    break;
                case ModeType::MODE_LIVE:
                    jsonParams["mode"] = "MODE_LIVE";
                    break;
                case ModeType::MODE_RECORD:
                    jsonParams["mode"] = "MODE_RECORD";
                    break;
                case ModeType::MODE_PLAYBACK:
                    jsonParams["mode"] = "MODE_PLAYBACK";
                    break;
            }
	    switch(manage){
                case ManageType::MANAGE_NONE:
                    jsonParams["manage"] = "MANAGE_NONE";
                    break;
                case ManageType::MANAGE_FULL:
                    jsonParams["manage"] = "MANAGE_FULL";
                    break;
                case ManageType::MANAGE_NO_PSI:
                    jsonParams["manage"] = "MANAGE_NO_PSI";
                    break;
                case ManageType::MANAGE_NO_TUNER:
                    jsonParams["manage"] = "MANAGE_NO_TUNER";
                    break;
            }
            jsonParams["casinitdata"] = casinitdata;
            jsonParams["casocdmid"] = casocdmid;
            jsonParams["muteaudio"] = muteAudio;
            jsonParams["mutevideo"] = muteVideo;

            std::string pretuneParams;
            uint32_t sessionId;
            response.Sessionid = UNDEFINED_SESSION_ID;
            jsonParams.ToString(pretuneParams);
            LOGDBG("OpenData = %s\n", pretuneParams.c_str());
            _error = m_RTPlayer->pretune(pretuneParams, &sessionId);

            if (_error == RT_OK ) {
                LOGDBG("UnifiedPlayer set  succeeded..");
		response.Sessionid = sessionId;
            }
            else {
                LOGWARN("Error in Opening media session...");
            }
	    result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);
            return result;
        }

        // Method: park - Park an active non-HTTP source for later use
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_park(const WPEFramework::Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response)
        {
	    rtError _error = RT_OK;
            uint32_t result;

            _error = m_RTPlayer->park(params);
            if (_error == RT_OK) {
                LOGDBG("UnifiedPlayer park CAS Management session succeeded..\n");
            }
            else
            {
                LOGWARN("Error in park functionality...\n");
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: destroy - Destroy a media playback session
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_destroy(const WPEFramework::Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response)
        {
            rtError _error = RT_OK;
            uint32_t result;

            _error = m_RTPlayer->destroy(params);
            if (_error == RT_OK) {
                LOGDBG("UnifiedPlayer destroy CAS Management Session succeeded..\n");
            }
            else
            {
                LOGWARN("Error in destroying CAS Management Session...\n");
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: setposition - Sets playback position
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_setposition(const FloatsessionvalueInfo& parameters, JsonData::UnifiedPlayer::ResultInfo& response)
        {
            rtError _error = RT_ERROR;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(parameters.Value.IsSet() )
            {
                if(parameters.Session.IsSet())
                {
                    sessionId = parameters.Session.Value();
                }
                _error = m_RTPlayer->setPosition(parameters.Value.Value(), sessionId);
            }

            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: setpositionrelative - Sets playback position relative to current position, aka skip
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_setpositionrelative(const FloatsessionvalueInfo& parameters, JsonData::UnifiedPlayer::ResultInfo& response)
        {
            rtError _error = RT_ERROR;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(parameters.Value.IsSet() )
            {
                if(parameters.Session.IsSet())
                {
                    sessionId = parameters.Session.Value();
                }
                _error = m_RTPlayer->setPositionRelative(parameters.Value.Value(), sessionId);
            }

             result = RTERROR_TO_WPEERROR(_error);
             RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: getposition - Gets playback position
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getposition(const Core::JSON::DecUInt32& params, FloatresultInfo& response)
        {
            rtError _error = RT_ERROR;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            uint32_t sessionId = UNDEFINED_SESSION_ID;
            float pos = 0.0;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }
            _error = m_RTPlayer->getPosition(&pos, sessionId);
            if(_error == RT_OK)
            {
                response.Value = pos;
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: setspeed - Sets playback speed
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_setspeed(const FloatsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response)
	{
            rtError _error = RT_ERROR;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.Value.IsSet() )
            {
                if(params.Session.IsSet())
                {
                    sessionId = params.Session.Value();
                }
                _error = m_RTPlayer->setSpeed(params.Value.Value(), sessionId);
            }

             result = RTERROR_TO_WPEERROR(_error);
             RTERROR_TO_RESPONSE(_error);

            return result; 
	}

        // Method: getspeed - Gets playback speed
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getspeed(const Core::JSON::DecUInt32& params, FloatresultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            float speed = 0.0;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }
            _error = m_RTPlayer->getSpeed(&speed, sessionId);
            if(_error == RT_OK)
            {
                response.Value = speed;
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: getduration - Gets duration of Closed Assests
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getduration(const Core::JSON::DecUInt32& params, FloatresultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            float duration = 0.0;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }
            _error = m_RTPlayer->getDuration(&duration, sessionId);
            if(_error == RT_OK)
            {
                response.Value = duration;
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: getavailablespeeds - Gets supported playback speeds
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getavailablespeeds(const Core::JSON::DecUInt32& params, FloatarrayresultInfo& response)
	{
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            Core::JSON::Float availSpeeds;
            std::string  availableSpeeds = "";
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }
            _error = m_RTPlayer->getAvailableSpeeds(availableSpeeds, sessionId);
            if( RT_OK == _error)
            {
                std::stringstream ss(availableSpeeds);
                std::string item;
                while (std::getline(ss, item, ','))
                {
                   availSpeeds=std::stof(item);
                   response.Value.Add(availSpeeds);
                }
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
	}
        // Method: setdefaultaudiolanguage - Sets default audio language
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_setdefaultaudiolanguage(const StringsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if (params.IsSet() && !params.Value.Value().empty())
            {
                if (params.Session.IsSet())
                {
                  sessionId = params.Session.Value();
                }
                _error = m_RTPlayer->setDefaultAudioLanguage(params.Value.Value(), sessionId);
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: getdefaultaudiolanguage - Gets default audio language
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getdefaultaudiolanguage(const Core::JSON::DecUInt32& params, StringresultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            std::string  defaultAudioLanguage = "";
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }
            _error = m_RTPlayer->getDefaultAudioLanguage(defaultAudioLanguage, sessionId);
            if( RT_OK == _error)
            {
               response.Value = defaultAudioLanguage;
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: setprimaryaudiolanguage - Sets primary, presenting, audio language
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_setprimaryaudiolanguage(const StringsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if (params.IsSet() && !params.Value.Value().empty())
            {
                std::string audioLanguage = params.Value.Value();
                if (params.Session.IsSet())
                {
                  sessionId = params.Session.Value();
                }
                _error = m_RTPlayer->setPrimaryAudioLanguage(audioLanguage, sessionId);
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: getprimaryaudiolanguage - Gets primary audio language
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getprimaryaudiolanguage(const Core::JSON::DecUInt32& params, StringresultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            std::string audioLanguage = "";
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }
            _error = m_RTPlayer->getPrimaryAudioLanguage(audioLanguage, sessionId);
            if( RT_OK == _error)
            {
               response.Value = audioLanguage;
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: setsecondaryaudiolanguage - Sets secondary, audio-description, audio language
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_setsecondaryaudiolanguage(const StringsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if (params.IsSet() && !params.Value.Value().empty())
            {
                if (params.Session.IsSet())
                {
                  sessionId = params.Session.Value();
                }
                _error = m_RTPlayer->setSecondaryAudioLanguage(params.Value.Value(), sessionId);
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: getsecondaryaudiolanguage - Gets secondary audio language
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getsecondaryaudiolanguage(const Core::JSON::DecUInt32& params, StringresultInfo& response)
	{
	    uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            std::string  secondaryAudioLanguage = "";
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }

            _error = m_RTPlayer->getSecondaryAudioLanguage(secondaryAudioLanguage, sessionId);
            if( RT_OK == _error)
            {
               response.Value = secondaryAudioLanguage;
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
	}
        // Method: getcurrentaudiolanguage - Gets the audio language currently presented to the user
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getcurrentaudiolanguage(const Core::JSON::DecUInt32& params, StringresultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            std::string currentAudioLanguage = "";
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }

            _error = m_RTPlayer->getCurrentAudioLanguage(currentAudioLanguage, sessionId);
            if( RT_OK == _error)
            {
               response.Value = currentAudioLanguage;
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: getavailableaudiolanguages - Gets all the audio languages currently in the PMT or Manifest
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getavailableaudiolanguages(const Core::JSON::DecUInt32& params, StringarrayresultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            Core::JSON::String availableLang;
            std::string  availableAudioLanguages = "";
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }

            _error = m_RTPlayer->getAvailableAudioLanguages(availableAudioLanguages, sessionId);
            if( RT_OK == _error)
            {
                std::stringstream ss(availableAudioLanguages);
                std::string item;
                while (std::getline(ss, item, ','))
                {
                   availableLang=item;
                   response.Value.Add(availableLang);
                }
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: setdefaultsubtitleslanguage - Sets default subtitles language
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_setdefaultsubtitleslanguage(const StringsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response)
	{
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if (params.IsSet() && !params.Value.Value().empty())
            {
                if (params.Session.IsSet())
                {
                  sessionId = params.Session.Value();
                }
                _error = m_RTPlayer->setDefaultSubtitlesLanguage(params.Value.Value(), sessionId);
            }

            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
	}
        // Method: getdefaultsubtitleslanguage - Gets default subtitles language
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getdefaultsubtitleslanguage(const Core::JSON::DecUInt32& params, StringresultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            std::string  subtitleLanguage = "";
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }

            _error = m_RTPlayer->getDefaultSubtitlesLanguage(subtitleLanguage, sessionId);
            if( RT_OK == _error)
            {
                response.Value = subtitleLanguage;
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: setprimarysubtitleslanguage - Sets primary, presenting, subtitles language
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_setprimarysubtitleslanguage(const StringsessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if (params.IsSet() && !params.Value.Value().empty())
            {
                if (params.Session.IsSet())
                {
                  sessionId = params.Session.Value();
                }
                _error = m_RTPlayer->setPrimarySubtitlesLanguage(params.Value.Value(), sessionId);
            }

            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: getprimarysubtitleslanguage - Gets primary subtitles language
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getprimarysubtitleslanguage(const Core::JSON::DecUInt32& params, StringresultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            std::string  subtitleLanguage = "";
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }

            _error = m_RTPlayer->getPrimarySubtitlesLanguage(subtitleLanguage, sessionId);
            if( RT_OK == _error)
            {
                response.Value = subtitleLanguage;
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: getcurrentsubtitleslanguage - Gets the subtitles language currently presented to the user
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getcurrentsubtitleslanguage(const Core::JSON::DecUInt32& params, StringresultInfo& response)
	{
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            std::string currentSubtitlesLanguage = "";
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }

            _error = m_RTPlayer->getCurrentSubtitlesLanguage(currentSubtitlesLanguage, sessionId);
            if( RT_OK == _error)
            {
               response.Value = currentSubtitlesLanguage;
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
	}
        // Method: getavailablesubtitleslanguages - Gets all the audio languages currently in the PMT or Manifest
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getavailablesubtitleslanguages(const Core::JSON::DecUInt32& params, StringarrayresultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            Core::JSON::String availableSubtLang;
            std::string  availableSubtitleLanguages = "";
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }

            _error = m_RTPlayer->getAvailableSubtitleLanguages(availableSubtitleLanguages, sessionId);
            if( RT_OK == _error)
            {
                std::stringstream ss(availableSubtitleLanguages);
                std::string item;
                while (std::getline(ss, item, ','))
                {
                   availableSubtLang=item;
                   response.Value.Add(availableSubtLang);
                }
            }

            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }
        
    // Method: setmute - Sets if media components should be muted or not
    // Return codes:
    //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_setmute(const MutecontrolsessionvalueData& params, ResultInfo& response)
        {
						uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            uint32_t sessionId = UNDEFINED_SESSION_ID;
						if (params.Session.IsSet())
						{
							sessionId = params.Session.Value();
						}
            if (params.Video.IsSet())
            {
              const bool& video = params.Video.Value();
              _error = m_RTPlayer->setVideoMute(video, sessionId);
            }
            if (params.Audio.IsSet())
            {
              const bool& audio = params.Audio.Value();
              _error = m_RTPlayer->setAudioMute(audio, sessionId);
            }

            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: getmute - Gets status of media components muting
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getmute(const Core::JSON::DecUInt32& params, GetmuteResultData& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _vError = RT_ERROR;
            rtError _aError = RT_ERROR;
            bool videoMute = false;
            bool audioMute = false;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }
            _vError = m_RTPlayer->getVideoMute(&videoMute, sessionId);
            _aError = m_RTPlayer->getAudioMute(&audioMute, sessionId);
            LOGDBG("GETMUTE: video:%d audio: %d", (int)videoMute, (int)audioMute);
            if((_vError == RT_OK) && (_aError == RT_OK))
            {
                response.Video = videoMute;
                response.Audio = audioMute;
                result = RTERROR_TO_WPEERROR(_vError);
                RTERROR_TO_RESPONSE(_vError);
            }
            else if (_vError != RT_OK)
            {
                result = RTERROR_TO_WPEERROR(_vError);
                RTERROR_TO_RESPONSE(_vError);
            }
            else
            {
                result = RTERROR_TO_WPEERROR(_aError);
                RTERROR_TO_RESPONSE(_aError);
            }
            return result;
	} 
        // Method: setsubtitlesenabled - Sets if subtitles are decoded and presented
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_setsubtitlesenabled(const BooleansessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response)
        {
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            uint32_t sessionId = UNDEFINED_SESSION_ID;
            if (params.IsSet() )
            {
                if (params.Session.IsSet())
                {
                  sessionId = params.Session.Value();
                }
                _error = m_RTPlayer->setsubtitlesEnabled(params.Value.Value(), sessionId);
            }

            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: getsubtitlesenabled - Gets if subtitles are enabled
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getsubtitlesenabled(const Core::JSON::DecUInt32& params, BooleanresultInfo& response)
        {
            rtError _error = RT_ERROR;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            bool subt = 0;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }

            _error = m_RTPlayer->getsubtitlesEnabled(&subt, sessionId);
            if(_error == RT_OK)
            {
                response.Value = subt;
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: setsubtitlesdisplayed - Sets if subtitles are presented
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_setsubtitlesdisplayed(const BooleansessionvalueInfo& params, JsonData::UnifiedPlayer::ResultInfo& response)
	{
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            rtError _error = RT_ERROR;
            uint32_t sessionId = UNDEFINED_SESSION_ID;
            if (params.IsSet() )
            {
                if (params.Session.IsSet())
                {
                  sessionId = params.Session.Value();
                }
                _error = m_RTPlayer->setsubtitlesDisplayed(params.Value.Value(), sessionId);
            }

            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
	}
        // Method: getsubtitlesdisplayed - Gets if subtitles are presented
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_getsubtitlesdisplayed(const Core::JSON::DecUInt32& params, BooleanresultInfo& response)
	{
            rtError _error = RT_ERROR;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            bool subt = 0;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if(params.IsSet())
            {
                sessionId = params.Value();
            }

            _error = m_RTPlayer->getsubtitlesDisplayed(&subt, sessionId);
            if(_error == RT_OK)
            {
                response.Value = subt;
            }
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
	}
        // Method: setvideorectangle - Sets the size and position of the video
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_setvideorectangle(const RectanglesessionvalueData& params, JsonData::UnifiedPlayer::ResultInfo& response)
	{
	    rtError _error = RT_ERROR;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            const uint32_t& x = params.X.Value();
            const uint32_t& y = params.Y.Value();
            const uint32_t& w = params.W.Value();
            const uint32_t& h = params.H.Value();
            if (params.Session.IsSet() == true)
            {
              sessionId  = params.Session.Value();
            }

            _error = m_RTPlayer->setVideoRectangle(x, y, w, h, sessionId);
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
	}
        // Method: play - Play the Video at x1 speed, HTTP sources only
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_play(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response)
        {
            rtError _error = RT_ERROR;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if (params.IsSet() == true)
            {
              sessionId  = params.Value();
            }
            _error = m_RTPlayer->play(sessionId);
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: pause - Pauses the video, HTTP sources only
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_pause(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response)
        {
            rtError _error = RT_ERROR;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if (params.IsSet() == true)
            {
              sessionId  = params.Value();
            }
            _error = m_RTPlayer->pause(sessionId);
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: seektolive - seeks to the live point, HTTP sources only, non-closed manifests only
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_seektolive(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response)
        {
	    rtError _error = RT_ERROR;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if (params.IsSet() == true)
            {
              sessionId  = params.Value();
            }
            _error = m_RTPlayer->seekToLive(sessionId);
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: stop - Stops playback and closes the source
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_stop(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response)
        {
            rtError _error = RT_ERROR;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if (params.IsSet() == true)
            {
              sessionId  = params.Value();
            }
            _error = m_RTPlayer->stop(sessionId);
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }

        // Method: requeststatus - Causes a status event to be triggered
        // Return codes:
        //  - ERROR_NONE: Success
        uint32_t UnifiedPlayer::endpoint_requeststatus(const Core::JSON::DecUInt32& params, JsonData::UnifiedPlayer::ResultInfo& response)
        {
	    rtError _error = RT_ERROR;
            uint32_t result = WPEFramework::Core::ERROR_BAD_REQUEST;
            uint32_t sessionId = UNDEFINED_SESSION_ID;

            if (params.IsSet() == true)
            {
              sessionId  = params.Value();
            }
            _error = m_RTPlayer->requestStatus(sessionId);
            result = RTERROR_TO_WPEERROR(_error);
            RTERROR_TO_RESPONSE(_error);

            return result;
        }



        // Internal methods begin
        // Thunder plugins communication end

        // Event management
        // 1.
        // 2.
        // 3.
         // Event management end
         // Internal methods end
    } // namespace Plugin

} // namespace WPEFramework
