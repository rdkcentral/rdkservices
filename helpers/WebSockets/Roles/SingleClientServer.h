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
#include <boost/optional.hpp>

#include "../../utils.h"

#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/config/asio.hpp"
#include "websocketpp/server.hpp"

namespace WebSockets   {

template<typename Derived>
class SingleClientServer
{
public:
    using NotEncryptedEndpointType = websocketpp::server<websocketpp::config::asio>;
    using EncryptedEndpointType = websocketpp::server<websocketpp::config::asio_tls>;

    SingleClientServer() = default;

    bool start(int port, std::function<void(ConnectionInitializationResult)> connectionInitializationCallback,
        std::function<void(void)> connectionClosedCallback);
    void stop();

protected:
    ~SingleClientServer() = default;

private:
    SingleClientServer(const SingleClientServer&) = delete;
    SingleClientServer& operator=(const SingleClientServer&) = delete;
};

template<typename Derived>
bool SingleClientServer<Derived>::start(int port, std::function<void(ConnectionInitializationResult)> connectionInitializationCallback,
    std::function<void(void)> connectionClosedCallback)
{
    LOGINFO("Starting websocket server on port: %d", port);

    Derived& derived = static_cast<Derived&>(*this);
    derived.connectionInitializationCallback_ = connectionInitializationCallback;
    derived.connectionClosedCallback_ = connectionClosedCallback;

    websocketpp::lib::error_code ec;
    derived.endpointImpl_.listen(port, ec);
    if (ec) {
        LOGERR("Failed to start listening, reason: %s", ec.message().c_str());
        return false;
    }

    derived.endpointImpl_.start_accept(ec);
    if (ec) {
        LOGERR("Failed to start server, reason: %s", ec.message().c_str());
        return false;
    }

    derived.startEventLoop();
    return true;
}

template<typename Derived>
void SingleClientServer<Derived>::stop()
{
    LOGINFO();
    Derived& derived = static_cast<Derived&>(*this);
    websocketpp::lib::error_code ec;
    derived.endpointImpl_.stop_listening(ec);
    if (ec)
    {
        LOGERR("Ordering server to stop listening failed, reason: %s", ec.message().c_str());
        return;
    }
    derived.closeConnection();
    LOGINFO("Connection ordered to stop and server ordered to stop listening. Server will fully close when 'run' method ends.");
}

}   // namespace WebSockets
