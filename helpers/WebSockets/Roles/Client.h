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
#include <functional>

#include "websocketpp/config/asio_no_tls_client.hpp"
#include "websocketpp/config/asio_client.hpp"
#include "websocketpp/client.hpp"

#include "Module.h"
#include "UtilsLogging.h"

namespace WebSockets   {

template<typename Derived>
class Client
{
public:
    using NotEncryptedEndpointType = websocketpp::client<websocketpp::config::asio_client>;
    using EncryptedEndpointType = websocketpp::client<websocketpp::config::asio_tls_client>;

    Client() = default;

    bool connect(std::string address, std::function<void(ConnectionInitializationResult)> connectionInitializationCallback,
        std::function<void(void)> connectionClosedCallback);
    void disconnect();

protected:
    ~Client() = default;

private:
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
};

template<typename Derived>
bool Client<Derived>::connect(std::string address, std::function<void(ConnectionInitializationResult)> connectionInitializationCallback,
    std::function<void(void)> connectionClosedCallback)
{
    Derived& derived = static_cast<Derived&>(*this);
    const std::string uri = derived.addProtocolToAddress(address);

    LOGINFO("Connecting, uri: %s", uri.c_str());

    derived.connectionInitializationCallback_ = connectionInitializationCallback;
    derived.connectionClosedCallback_ = connectionClosedCallback;

    websocketpp::lib::error_code ec;
    auto connection = derived.endpointImpl_.get_connection(uri, ec);
    if (ec) {
        LOGERR("Can't prepare connection to connect with: %s, reason: %s", uri.c_str(), ec.message().c_str());
        return false;
    }

    derived.startEventLoop();
    LOGINFO("Calling connect on: %s\n", uri.c_str());
    derived.endpointImpl_.connect(connection);
    return true;
}

template<typename Derived>
void Client<Derived>::disconnect()
{
    LOGINFO();
    Derived& derived = static_cast<Derived&>(*this);
    derived.closeConnection();
    LOGINFO("Disconnection successfull");
}

}   // namespace WebSockets
