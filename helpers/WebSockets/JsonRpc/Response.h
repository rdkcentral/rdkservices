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

#include <core/JSONRPC.h>

namespace WebSockets   {
namespace JsonRpc      {

class Response : public WPEFramework::Core::JSONRPC::Message
{
public:
    Response();
    virtual ~Response() = default;

    uint32_t getId() const;
    bool isResult() const;
    bool getResult(JsonObject& jsonObject) const;
    bool isError() const;
    bool getError(JsonObject& jsonObject) const;
    bool isValid() const;

private:
    Response(const Response&) = delete;
    Response& operator=(const Response&) = delete;
};

}   // namespace JsonRpc
}   // namespace WebSockets
