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

#include "websocketpp/config/asio_no_tls_client.hpp"
#include "websocketpp/client.hpp"

#include "../../utils.h"

namespace WebSockets   {

template <typename Derived, typename Role>
class NoEncryption
{
public:
    NoEncryption()
    {
        LOGINFO("Creating not encrypted websocket.\n");
    }

protected:
    using EndpointType = typename Role::NotEncryptedEndpointType;

    ~NoEncryption()
    {
        LOGINFO("Destroying not encrypted websocket.\n");
    }
    void setup()
    {
        LOGINFO("Encryption will not be set for this connection.\n");
    }
    std::string addProtocolToAddress(const std::string& address)
    {
        return std::string("ws://") + address;
    }

    void setAuthenticationState(ConnectionInitializationResult& result, websocketpp::connection_hdl handler)
    {
        result.setAuthenticationSuccess(true);
    }
};

}   // namespace WebSockets
