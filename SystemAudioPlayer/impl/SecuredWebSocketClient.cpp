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

#include "SecuredWebSocketClient.h"
#include "AudioPlayer.h"

namespace impl {

SecuredWebSocketClient::SecuredWebSocketClient(
    AudioPlayer* audioPlayer,
    const SecurityParameters& secParams)
    : wsClient_(audioPlayer)
{
    SAPLOG_INFO("Creating secured websocket client.");
    wsClient_.wsClient_.setCAFileNames(secParams.CAFileNames);
    wsClient_.wsClient_.setCertFileName(secParams.certFileName);
    wsClient_.wsClient_.setKeyFileName(secParams.keyFileName);
}

SecuredWebSocketClient::~SecuredWebSocketClient()
{
    SAPLOG_INFO("Destroying secured websocket client.");
}

void SecuredWebSocketClient::connect(const std::string& uri)
{
    wsClient_.connect(uri);
}

void SecuredWebSocketClient::disconnect()
{
    wsClient_.disconnect();
}

ConnectionType SecuredWebSocketClient::getConnectionType() const
{
    return ConnectionType::Secured;
}

} // namespace impl
