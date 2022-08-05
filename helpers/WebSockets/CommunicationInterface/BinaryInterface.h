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
#include <functional>
#include <string>
#include <websocketpp/frame.hpp>

#include "Module.h"
#include "UtilsLogging.h"

namespace WebSockets   {

template<typename Derived>
class BinaryInterface
{
public:
    BinaryInterface() = default;

    bool sendMessage(const std::string& message)
    {
        Derived& derived = static_cast<Derived&>(*this);
        return derived.send(message);
    }

    void setOnMessageHandler(const std::function<void(const std::string&)>& handler)
    {
        LOGINFO("Setting onMessage handler.");
        onMessage = handler;
    }

protected:
    ~BinaryInterface() = default;

    std::function<void(const std::string&)> onMessage{[](const std::string& message) { LOGWARN("Default onMessage."); }};
    websocketpp::frame::opcode::value opcode_{websocketpp::frame::opcode::binary};

private:
    BinaryInterface(const BinaryInterface&) = delete;
    BinaryInterface& operator=(const BinaryInterface&) = delete;
};

}   // namespace WebSockets
