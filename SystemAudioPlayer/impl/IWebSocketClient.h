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

#include <memory>
#include <string>

namespace impl {

enum class ConnectionType
{
    Secured,
    Unsecured
};

inline ConnectionType getConnectionType(const std::string& uri)
{
    size_t found = uri.find("://");
    if(found != std::string::npos)
    {
        const auto& callsign = uri.substr(0,found);
        if ("wss" == callsign)
        {
            return ConnectionType::Secured;
        }
    }
    return ConnectionType::Unsecured;
}

class IWebSocketClient
{
public:
    virtual ~IWebSocketClient() = default;
    virtual void connect(const std::string& uri) = 0;
    virtual void disconnect() = 0;
    virtual ConnectionType getConnectionType() const = 0;
};

using WebSocketClientPtr = std::unique_ptr<IWebSocketClient>;

} // namespace impl
