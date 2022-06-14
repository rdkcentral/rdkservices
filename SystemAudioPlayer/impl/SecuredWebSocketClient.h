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

#pragma once

#include <string>
#include <vector>

#include "IWebSocketClient.h"
#include "WebSocketClientImpl.h"
#include "WebSockets/Encryption/TlsEnabled.h"
#include "SecurityParameters.h"

class AudioPlayer;

namespace impl {

class SecuredWebSocketClient : public IWebSocketClient
{
public:
    SecuredWebSocketClient(
        AudioPlayer* audioPlayer,
        const SecurityParameters& secParams);
    ~SecuredWebSocketClient();
    void connect(const std::string& uri) override;
    void disconnect() override;
    ConnectionType getConnectionType() const override;

private:
    WebSocketClientImpl<WebSockets::TlsEnabled> wsClient_;
};

} // namespace impl
