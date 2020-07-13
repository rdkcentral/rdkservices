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
#include "utils.h"

#include "audioOutputPort.hpp"
#include "audioOutputPortType.hpp"
#include "videoOutputPort.hpp"
#include "manager.hpp"
#include "host.hpp"
#include "exception.hpp"

#if defined(USE_IARM)
#include "libIBus.h"
#include "libIBusDaemon.h"
#include "dsMgr.h"
#endif

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(PlayerInfo, 1, 0);

    static Core::ProxyPoolType<Web::Response> responseFactory(4);
    static Core::ProxyPoolType<Web::JSONBodyType<JsonData::PlayerInfo::CodecsData>> jsonResponseFactory(4);
    PlayerInfo* PlayerInfo::_instance = nullptr;

    /* virtual */ const string PlayerInfo::Initialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);
        ASSERT(_player == nullptr);

        string message;
        Config config;
        config.FromString(service->ConfigLine());
        _skipURL = static_cast<uint8_t>(service->WebPrefix().length());

        _player = service->Root<Exchange::IPlayerProperties>(_connectionId, 2000, _T("PlayerInfoImplementation"));

        if (_player != nullptr) {

            _audioCodecs = _player->AudioCodec();
            if (_audioCodecs != nullptr) {

                _videoCodecs = _player->VideoCodec();
                if (_videoCodecs == nullptr) {

                    _audioCodecs->Release();
                    _audioCodecs = nullptr;

                    _player->Release();
                    _player = nullptr;
                }
            } else {
                _player->Release();
                _player = nullptr;
            }
        }

        if (_player == nullptr) {
            message = _T("PlayerInfo could not be instantiated.");
        }

        try
        {
#if defined(USE_IARM)
            Utils::IARM::init();
            IARM_Result_t res;
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_AUDIO_MODE, AudioModeHandler) );
#endif
            //TODO: this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
            device::Manager::Initialize();
            LOGINFO("device::Manager::Initialize success");
        }
        catch(...)
        {
            LOGINFO("device::Manager::Initialize failed");
        }

        return message;
    }

    /* virtual */ void PlayerInfo::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_player != nullptr);
        if (_player != nullptr) {
            _player->Release();
        }
        _connectionId = 0;

        try
        {
            //TODO(MROLLINS) this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
            device::Manager::DeInitialize();
            LOGINFO("device::Manager::DeInitialize success");
        }
        catch(...)
        {
            LOGINFO("device::Manager::DeInitialize failed");
        }
    }

    /* virtual */ string PlayerInfo::Information() const
    {
        // No additional info to report.
        return (string());
    }

    /* virtual */ void PlayerInfo::Inbound(Web::Request& /* request */)
    {
    }

    /* virtual */ Core::ProxyType<Web::Response> PlayerInfo::Process(const Web::Request& request)
    {
        ASSERT(_skipURL <= request.Path.length());

        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());

        // By default, we assume everything works..
        result->ErrorCode = Web::STATUS_OK;
        result->Message = "OK";

        // <GET> - currently, only the GET command is supported, returning system info
        if (request.Verb == Web::Request::HTTP_GET) {

            Core::ProxyType<Web::JSONBodyType<JsonData::PlayerInfo::CodecsData>> response(jsonResponseFactory.Element());

            Core::TextSegmentIterator index(Core::TextFragment(request.Path, _skipURL, static_cast<uint32_t>(request.Path.length()) - _skipURL), false, '/');

            // Always skip the first one, it is an empty part because we start with a '/' if there are more parameters.
            index.Next();

            Info(*response);
            result->ContentType = Web::MIMETypes::MIME_JSON;
            result->Body(Core::proxy_cast<Web::IBody>(response));
        } else {
            result->ErrorCode = Web::STATUS_BAD_REQUEST;
            result->Message = _T("Unsupported request for the [PlayerInfo] service.");
        }

        return result;
    }

/* Begin Events */
    void PlayerInfo::Info(JsonData::PlayerInfo::CodecsData& playerInfo) const
    {
        Core::JSON::EnumType<JsonData::PlayerInfo::CodecsData::AudiocodecsType> audioCodec;
        _audioCodecs->Reset();
        while(_audioCodecs->Next()) {
            playerInfo.Audio.Add(audioCodec = static_cast<JsonData::PlayerInfo::CodecsData::AudiocodecsType>(_audioCodecs->Codec()));
        }

        Core::JSON::EnumType<JsonData::PlayerInfo::CodecsData::VideocodecsType> videoCodec;
        _videoCodecs->Reset();
        while(_videoCodecs->Next()) {
            playerInfo.Video.Add(videoCodec = static_cast<JsonData::PlayerInfo::CodecsData::VideocodecsType>(_videoCodecs->Codec()));
        }
    }

    uint32_t PlayerInfo::getSinkAtmosCapability (const JsonObject& parameters, JsonObject& response)
    {   //sample servicemanager response:
        LOGINFOMETHOD();
        bool success = true;
        dsATMOSCapability_t atmosCapability;
        try
        {
            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI0");
            if (aPort.isConnected()) {
                aPort.getSinkDeviceAtmosCapability (atmosCapability);
                response["atmos_capability"] = (int)atmosCapability;
            }
            else {
                LOGERR("getSinkAtmosCapability failure: HDMI0 not connected!\n");
                success = false;
            }
        }
        catch(const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION1(string("HDMI0"));
            success = false;
        }
        returnResponse(success);
    }

    uint32_t PlayerInfo::getSoundMode(const JsonObject& parameters, JsonObject& response)
    {   //sample servicemanager response:{"success":true,"soundMode":"AUTO (Dolby Digital 5.1)"}
        LOGINFOMETHOD();
        string audioPort = parameters["audioPort"].String();//empty value will browse all ports

        if (!checkPortName(audioPort))
            audioPort = "HDMI0";

        string modeString("");
        device::AudioStereoMode mode = device::AudioStereoMode::kStereo;  //default to stereo

        try
        {
            /* Return the sound mode of the audio ouput connected to the specified videoDisplay */
            /* Check if HDMI is connected - Return (default) Stereo Mode if not connected */
            if (audioPort.empty())
            {
                if (device::Host::getInstance().getVideoOutputPort("HDMI0").isDisplayConnected())
                {
                    audioPort = "HDMI0";
                }
                else
                {
                    /*  * If HDMI is not connected
                        * Get the SPDIF if it is supported by platform
                        * If Platform does not have connected ports. Default to HDMI.
                    */
                    audioPort = "HDMI0";
                    device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
                    for (size_t i = 0; i < vPorts.size(); i++)
                    {
                        device::VideoOutputPort &vPort = vPorts.at(i);
                        if (vPort.isDisplayConnected())
                        {
                            audioPort = "SPDIF0";
                            break;
                        }
                    }
                }
            }

            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);

            if (aPort.isConnected())
            {
                mode = aPort.getStereoMode();

                if (aPort.getType().getId() == device::AudioOutputPortType::kHDMI)
                {
                    /* In DS5, "Surround" implies "Auto" */
                    if (aPort.getStereoAuto() || mode == device::AudioStereoMode::kSurround)
                    {
                        LOGINFO("HDMI0 is in Auto Mode");
                        int surroundMode = device::Host::getInstance().getVideoOutputPort("HDMI0").getDisplay().getSurroundMode();
                        if ( surroundMode & dsSURROUNDMODE_DDPLUS)
                        {
                            LOGINFO("HDMI0 has surround DDPlus");
                            modeString.append("AUTO (Dolby Digital Plus)");
                        }
                        else if (surroundMode & dsSURROUNDMODE_DD)
                        {
                            LOGINFO("HDMI0 has surround DD 5.1");
                            modeString.append("AUTO (Dolby Digital 5.1)");
                        }
                        else
                        {
                            LOGINFO("HDMI0 does not surround");
                            modeString.append("AUTO (Stereo)");
                        }
                    }
                    else
                        modeString.append(mode.toString());
                }
                else
                {
                    if (mode == device::AudioStereoMode::kSurround)
                        modeString.append("Surround");
                    else
                        modeString.append(mode.toString());
                }
            }
            else
            {
                /*
                * VideoDisplay is not connected. Its audio mode is unknown. Return
                * "Stereo" as safe default;
                */
                mode = device::AudioStereoMode::kStereo;
                modeString.append("AUTO (Stereo)");
            }
        }
        catch (const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION0();
            //
            // Exception
            // "Stereo" as safe default;
            //
            mode = device::AudioStereoMode::kStereo;
            modeString += "AUTO (Stereo)";
        }

        LOGWARN("audioPort = %s, mode = %s!", audioPort.c_str(), modeString.c_str());

        response["soundMode"] = modeString;
        returnResponse(true);
    }

    uint32_t PlayerInfo::setAudioAtmosOutputMode (const JsonObject& parameters, JsonObject& response)
    {   //sample servicemanager response:
        LOGINFOMETHOD();
        returnIfParamNotFound(parameters, "enable");

        string sEnable = parameters["enable"].String();
        int enable = parameters["enable"].Boolean();

        bool success = true;
        try
        {
            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI0");
            if (aPort.isConnected()) {
                aPort.setAudioAtmosOutputMode (enable);
            }
            else {
                LOGERR("setAudioAtmosOutputMode failure: HDMI0 not connected!\n");
                success = false;
            }

        }
        catch (const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION2(string("HDMI0"), sEnable);
            success = false;
        }
        returnResponse(success);
    }

/* Begin methods */
    void PlayerInfo::audiomodeChanged(int AudioPortMode, int AudioPortType)
    {
        LOGINFO();
        JsonObject params;
        params["AudioPortMode"] = AudioPortMode;
        params["AudioPortType"] = AudioPortType;
        sendNotify("AudioPortModeChanged", params);
    }
/* End methods */

#if defined(USE_IARM)
    void PlayerInfo::AudioModeHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        LOGINFO();
        switch (eventId)
        {
            case IARM_BUS_DSMGR_EVENT_AUDIO_MODE:
                {
                    LOGINFO("Received Audio mode event IARM_BUS_DSMGR_EVENT_AUDIO_MODE");
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    int AudioPortMode = eventData->data.Audioport.mode;
                    int AudioPortType = eventData->data.Audioport.type;
                    if(PlayerInfo::_instance)
                    {
                        PlayerInfo::_instance->audiomodeChanged(AudioPortMode, AudioPortType);
                    }
                }
                break;
            default:
                break;
        }
    }
#endif

    bool PlayerInfo::checkPortName(std::string& name) const
    {
        if (Utils::String::stringContains(name,"HDMI"))
            name = "HDMI0";
        else if (Utils::String::stringContains(name,"SPDIF"))
            name = "SPDIF0";
        else if (Utils::String::stringContains(name,"IDLR"))
            name = "IDLR0";
        else if (!name.empty()) // Empty is allowed
            return false;

        return true;
    }


} // namespace Plugin
} // namespace WPEFramework
