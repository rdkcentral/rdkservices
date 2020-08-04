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

#include "PlayerInfo.h"

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::PlayerInfo;

    // Registration
    //

    void PlayerInfo::RegisterAll()
    {
        Property<CodecsData>(_T("playerinfo"), &PlayerInfo::get_playerinfo, nullptr, this);
        Property<SinkAtmosCapabilityData>(_T("getSinkAtmosCapability"), &PlayerInfo::getSinkAtmosCapabilityJrpc, nullptr, this);
        Property<SoundModeData>(_T("getSoundMode"), &PlayerInfo::getSoundModeJrpc, nullptr, this);
        Property<AudioAtmosOutputModeData>(_T("setAudioAtmosOutputMode"), nullptr, &PlayerInfo::setAudioAtmosOutputModeJrpc, this);
    }

    void PlayerInfo::UnregisterAll()
    {
        Unregister(_T("playerinfo"));
        Unregister(_T("getSinkAtmosCapability"));
        Unregister(_T("getSoundMode"));
        Unregister(_T("setAudioAtmosOutputMode"));
    }

    // API implementation
    //

    // Property: playerinfo - Player general information
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t PlayerInfo::get_playerinfo(CodecsData& response) const
    {
        Info(response);

        return Core::ERROR_NONE;
    }

    uint32_t PlayerInfo::getSinkAtmosCapabilityJrpc(SinkAtmosCapabilityData& response) const
    {
       getSinkAtmosCapability(response);
        return Core::ERROR_NONE;
    }

    uint32_t PlayerInfo::getSoundModeJrpc(SoundModeData& response) const
    {
        getSoundMode(response);
        return Core::ERROR_NONE;
    }

    uint32_t PlayerInfo::setAudioAtmosOutputModeJrpc(const AudioAtmosOutputModeData& param)
    {
        setAudioAtmosOutputMode(param);
        return Core::ERROR_NONE;
    }

    void PlayerInfo::AudioModeChangedJrpc(const string& audioPortMode, const string&  audioPortType)
   {
       AudioModeChangedParamsData params;
       params.AudioPortMode = audioPortMode;
       params.AudioPortType = audioPortType;
       Notify(_T("AudioPortModeChanged"), params);
   }

} // namespace Plugin

}

