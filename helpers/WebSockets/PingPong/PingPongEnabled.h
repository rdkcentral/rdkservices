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

#include "websocketpp/common/connection_hdl.hpp"
#include "websocketpp/common/functional.hpp"
#include "websocketpp/error.hpp"

#include "Module.h"
#include "UtilsLogging.h"

namespace WebSockets   {

template<typename Derived>
class PingPongEnabled
{
public:
    typedef typename websocketpp::connection_hdl ConnectionHandler;
    PingPongEnabled() = default;

protected:
    ~PingPongEnabled() = default;
    void startPing(ConnectionHandler handler);

private:
    void schedule(uint interval, const std::function<void(websocketpp::lib::error_code const &)>& cb);
    void ping(websocketpp::lib::error_code ecc);
    void onPong(ConnectionHandler hdl, std::string);
    void onPongTimeout(ConnectionHandler hdl, std::string);
    void printConnectionState(const websocketpp::session::state::value& state) const;

    const uint pingInterval_{5000};
    PingPongEnabled(const PingPongEnabled&) = delete;
    PingPongEnabled& operator=(const PingPongEnabled&) = delete;
};

template<typename Derived>
void PingPongEnabled<Derived>::startPing(ConnectionHandler handler)
{
    LOGINFO();
    Derived& derived = static_cast<Derived&>(*this);
    auto connection = derived.getConnection(handler);
    if (!connection)
    {
        LOGERR("Cant get connection.");
        return;
    }
    printConnectionState(connection->get_state());
    connection->set_pong_handler(std::bind(&PingPongEnabled::onPong, this,
        websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
    connection->set_pong_timeout_handler(std::bind(&PingPongEnabled::onPongTimeout, this,
        websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));

    schedule(pingInterval_, std::bind(&PingPongEnabled::ping, this, websocketpp::lib::placeholders::_1));
}

template<typename Derived>
void PingPongEnabled<Derived>::schedule(uint interval, const std::function<void(websocketpp::lib::error_code const &)>& cb)
{
    Derived& derived = static_cast<Derived&>(*this);
    derived.endpointImpl_.set_timer(
        interval,
        websocketpp::lib::bind(
            cb,
            websocketpp::lib::placeholders::_1
        )
    );
}

template<typename Derived>
void PingPongEnabled<Derived>::ping(websocketpp::lib::error_code ecc)
{
    LOGINFO();
    Derived& derived = static_cast<Derived&>(*this);
    auto connection = derived.getConnection(derived.connectionHandler_);
    if (!connection)
    {
        // Probably connection closed and cleaned up.
        LOGINFO("Cant get connection. Not sending ping.");
        return;
    }
    if (connection->get_state() != websocketpp::session::state::open)
    {
        // Connection will be/was closed internaly and onClose will be/has been launched
        printConnectionState(connection->get_state());
        LOGINFO("Connection state is not open/working. Not sending ping.");
        return;
    }
    websocketpp::lib::error_code ec;
    connection->ping("",ec);
    if (ec)
    {
        LOGERR("Sending ping failed, reason: %s", ec.message().c_str());
        return;
    }
}

template<typename Derived>
void PingPongEnabled<Derived>::onPong(ConnectionHandler hdl, std::string)
{
    LOGINFO("Pong received");
    schedule(pingInterval_, std::bind(&PingPongEnabled::ping, this, websocketpp::lib::placeholders::_1));
}

template<typename Derived>
void PingPongEnabled<Derived>::onPongTimeout(ConnectionHandler hdl, std::string)
{
    LOGINFO("Pong timeout. Closing connection.");
    Derived& derived = static_cast<Derived&>(*this);
    derived.closeConnection();
}

template<typename Derived>
void PingPongEnabled<Derived>::printConnectionState(const websocketpp::session::state::value& state) const
{
    static std::map<int, std::string> m = {
        {websocketpp::session::state::connecting, "connecting"},
        {websocketpp::session::state::open, "open"},
        {websocketpp::session::state::closing, "closing"},
        {websocketpp::session::state::closed, "closed"}};
    LOGINFO("Connection state is: %s", m[state].c_str());
}

}   // namespace WebSockets
