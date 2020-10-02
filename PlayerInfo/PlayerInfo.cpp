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

    SERVICE_REGISTRATION(PlayerInfo, 1, 0);

    static Core::ProxyPoolType<Web::Response> responseFactory(4);
    static Core::ProxyPoolType<Web::JSONBodyType<JsonData::PlayerInfo::CodecsData>> jsonResponseFactory(4);

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
            Exchange::JPlayerProperties::Register(*this, _player);
            if ( (_player->AudioCodecs(_audioCodecs) != Core::ERROR_NONE)  || (_audioCodecs == nullptr) ) {
                if (_audioCodecs != nullptr) {
                    _audioCodecs->Release();
                    _audioCodecs = nullptr;
                }
                _player->Release();
                _player = nullptr;
            }
            else if ((_player->VideoCodecs(_videoCodecs) != Core::ERROR_NONE) || (_videoCodecs == nullptr) ) {
                if (_videoCodecs != nullptr) {

                    _videoCodecs->Release();
                    _videoCodecs = nullptr;
                }
                _audioCodecs->Release();
                _audioCodecs = nullptr;
                _player->Release();
                _player = nullptr;
            } else {

                // The code execution should proceed regardless of the _dolbyOut
                // value, as it is not a essential.
                // The relevant JSONRPC endpoints will return ERROR_UNAVAILABLE,
                // if it hasn't been initialized.
                _dolbyOut = _player->QueryInterface<Exchange::Dolby::IOutput>();
                if(_dolbyOut == nullptr){
                    SYSLOG(Logging::Startup, (_T("Dolby output switching service is unavailable.")));
                }
                else
                {
                    _notification.Initialize(_dolbyOut);
                    Exchange::Dolby::JOutput::Register(*this, _dolbyOut);
                }

            }
        }

        if (_player == nullptr) {
            message = _T("PlayerInfo could not be instantiated.");
        }
        return message;
    }

    /* virtual */ void PlayerInfo::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_player != nullptr);
        if (_player != nullptr) {
            Exchange::JPlayerProperties::Unregister(*this);
            _player->Release();
        }
        if (_dolbyOut != nullptr)
        {
             _notification.Deinitialize();
            Exchange::Dolby::JOutput::Unregister(*this);
        }
        _connectionId = 0;
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

    void PlayerInfo::Info(JsonData::PlayerInfo::CodecsData& playerInfo) const
    {
        Core::JSON::EnumType<JsonData::PlayerInfo::CodecsData::AudiocodecsType> audioCodec;
        _audioCodecs->Reset(0);
        Exchange::IPlayerProperties::AudioCodec audio;
        while(_audioCodecs->Next(audio)) {
            playerInfo.Audio.Add(audioCodec = static_cast<JsonData::PlayerInfo::CodecsData::AudiocodecsType>(audio));
        }

        Core::JSON::EnumType<JsonData::PlayerInfo::CodecsData::VideocodecsType> videoCodec;
        Exchange::IPlayerProperties::VideoCodec video;
        _videoCodecs->Reset(0);
        while(_videoCodecs->Next(video)) {
            playerInfo.Video.Add(videoCodec = static_cast<JsonData::PlayerInfo::CodecsData::VideocodecsType>(video));
        }
    }

} // namespace Plugin
} // namespace WPEFramework
