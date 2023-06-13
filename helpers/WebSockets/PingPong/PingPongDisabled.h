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
#include "websocketpp/common/connection_hdl.hpp"

namespace WebSockets   {

template<typename Derived>
class PingPongDisabled
{
public:
    PingPongDisabled() = default;

protected:
    ~PingPongDisabled() = default;

    typedef typename websocketpp::connection_hdl ConnectionHandler;

    void startPing(ConnectionHandler handler)
    {
    }

private:
    PingPongDisabled(const PingPongDisabled&) = delete;
    PingPongDisabled& operator=(const PingPongDisabled&) = delete;


};

}   // namespace WebSockets
