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
#include <thread>
#include <functional>
#include <boost/optional.hpp>

#include "../utils.h"

#include "ConnectionInitializationResult.h"
#include "websocketpp/server.hpp"

namespace WebSockets   {

template<
    template <typename> typename Role,
    template <typename> typename MessagingInterface,
    template <typename> typename PingPong,
    template <typename, typename> typename Encryption
>
class WSEndpoint : public Role<WSEndpoint<Role, MessagingInterface, PingPong, Encryption> >,
                   public MessagingInterface<WSEndpoint<Role, MessagingInterface, PingPong, Encryption> >,
                   private PingPong<WSEndpoint<Role, MessagingInterface, PingPong, Encryption> >,
                   public Encryption<WSEndpoint<Role, MessagingInterface, PingPong, Encryption>, Role<WSEndpoint<Role, MessagingInterface, PingPong, Encryption> > >
{
public:
    WSEndpoint();
    ~WSEndpoint();

private:
    WSEndpoint(const WSEndpoint&) = delete;
    WSEndpoint& operator=(const WSEndpoint&) = delete;

    friend Role<WSEndpoint>;
    friend MessagingInterface<WSEndpoint>;
    friend PingPong<WSEndpoint>;
    friend Encryption<WSEndpoint, Role<WSEndpoint> >;

    using WebsocketppEndpoint = typename Encryption<WSEndpoint, Role<WSEndpoint> >::EndpointType;
    using ConnectionPtr = typename WebsocketppEndpoint::connection_ptr;
    using ConnectionHandler = websocketpp::connection_hdl;

    bool send(const std::string& message);
    void closeConnection();
    void startEventLoop();
    void stopEventLoop();
    WSEndpoint::ConnectionPtr getConnection(ConnectionHandler handler);

    void registerHandlers();
    void onMessage(ConnectionHandler, typename WebsocketppEndpoint::message_ptr msg);
    void onOpen(ConnectionHandler);
    void onFail(ConnectionHandler);
    void onClose(ConnectionHandler);

    WebsocketppEndpoint endpointImpl_;
    ConnectionHandler connectionHandler_;
    boost::optional<std::thread> eventLoopThread_;
    std::function<void(ConnectionInitializationResult)> connectionInitializationCallback_;
    std::function<void(void)> connectionClosedCallback_;
};

}   // namespace WebSockets
