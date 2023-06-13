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

#include "SystemAudioPlayer.h"

namespace WPEFramework {
namespace Plugin {

    void SystemAudioPlayer::RegisterAll()
    {
        Register("open", &SystemAudioPlayer::Open, this);        
        Register("play", &SystemAudioPlayer::Play, this);
        Register("playbuffer", &SystemAudioPlayer::PlayBuffer, this);
        Register("pause", &SystemAudioPlayer::Pause, this);
        Register("resume", &SystemAudioPlayer::Resume, this);
        Register("stop", &SystemAudioPlayer::Stop, this);
        Register("close", &SystemAudioPlayer::Close, this);
        Register("setMixerLevels", &SystemAudioPlayer::SetMixerLevels, this);
        Register("setSmartVolControl", &SystemAudioPlayer::SetSmartVolControl, this);
        Register("isspeaking", &SystemAudioPlayer::IsPlaying, this);
	Register("config", &SystemAudioPlayer::Config, this);
        Register("getPlayerSessionId", &SystemAudioPlayer::GetPlayerSessionId, this);
    }
    
   
    uint32_t SystemAudioPlayer::Open(const JsonObject& parameters, JsonObject& response)
    {
        if(_sap) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret= _sap->Open(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

     uint32_t SystemAudioPlayer::Config(const JsonObject& parameters, JsonObject& response)
    {
        if(_sap) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret= _sap->Config(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t SystemAudioPlayer::GetPlayerSessionId(const JsonObject& parameters, JsonObject& response)
    {
        if(_sap) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret= _sap->GetPlayerSessionId(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t SystemAudioPlayer::Play(const JsonObject& parameters, JsonObject& response)
    {
        if(_sap) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret = _sap->Play(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t SystemAudioPlayer::PlayBuffer(const JsonObject& parameters, JsonObject& response)
    {
        if(_sap) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret= _sap->PlayBuffer(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t SystemAudioPlayer::Stop(const JsonObject& parameters, JsonObject& response)
    {
        if(_sap) {
            string params,result;
            parameters.ToString(params);
            uint32_t ret = _sap->Stop(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t SystemAudioPlayer::Close(const JsonObject& parameters ,JsonObject& response)
    {
        if(_sap) {
            string params,result;
            parameters.ToString(params);
            uint32_t ret = _sap->Close(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }


    uint32_t SystemAudioPlayer::SetMixerLevels(const JsonObject& parameters, JsonObject& response)
    {
        if(_sap) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret= _sap->SetMixerLevels(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t SystemAudioPlayer::SetSmartVolControl(const JsonObject& parameters, JsonObject& response)
    {
        if(_sap) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret= _sap->SetSmartVolControl(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t SystemAudioPlayer::Pause(const JsonObject& parameters, JsonObject& response)
    {
        if(_sap) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret= _sap->Pause(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t SystemAudioPlayer::Resume(const JsonObject& parameters, JsonObject& response)
    {
        if(_sap) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret= _sap->Resume(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }

    uint32_t SystemAudioPlayer::IsPlaying(const JsonObject& parameters, JsonObject& response)
    {
        if(_sap) {
            string params, result;
            parameters.ToString(params);
            uint32_t ret = _sap->IsPlaying(params, result);
            response.FromString(result);
            return ret;
        }
        return Core::ERROR_NONE;
    }


    void SystemAudioPlayer::dispatchJsonEvent(const char *event, const string &data)
    {
        SAPLOG_WARNING("Notify %s %s", event, data.c_str());
        JsonObject params;
        params.FromString(data);
        Notify(event, params);
    }

} // namespace Plugin
} // namespace WPEFramework
