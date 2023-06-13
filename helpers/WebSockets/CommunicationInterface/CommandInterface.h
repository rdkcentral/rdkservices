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
#include <condition_variable>
#include <mutex>
#include <string>
#include <mutex>
#include <condition_variable>
#include <websocketpp/frame.hpp>

#include "Module.h"
#include "UtilsLogging.h"

namespace WebSockets   {

template<typename Derived>
class CommandInterface
{
public:
    CommandInterface() = default;

    bool sendCommand(std::string command, std::string& response)
    {
        LOGINFO("Send command: %s", command.c_str());
        Derived& derived = static_cast<Derived&>(*this);
        if (derived.send(command))
        {
            std::unique_lock<std::mutex> lock(responseMutex_);
            responseCondition_.wait_for(lock, std::chrono::seconds(5));
            response = lastResponse_;
            return true;
        }

        return false;
    }

protected:
    ~CommandInterface() = default;

    void onMessage(const std::string& message)
    {
        LOGINFO("On message: %s", message.c_str());
        std::lock_guard<std::mutex> lock(responseMutex_);
        lastResponse_ = message;
        responseCondition_.notify_one();
    }

    websocketpp::frame::opcode::value opcode_{websocketpp::frame::opcode::text};

private:
    CommandInterface(const CommandInterface&) = delete;
    CommandInterface& operator=(const CommandInterface&) = delete;

    std::string lastResponse_;
    std::mutex responseMutex_;
    std::condition_variable responseCondition_;
};

}   // namespace WebSockets
