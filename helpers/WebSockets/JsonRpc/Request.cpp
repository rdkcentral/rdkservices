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

#include "Request.h"
#include "utils.h"

namespace WebSockets   {
namespace JsonRpc      {

std::atomic<uint32_t> Request::idSequence(0);

Request::Request() : WPEFramework::Core::JSONRPC::Message()
{
}

uint32_t Request::getId() const
{
    return Id.Value();
}

uint32_t Request::generateId()
{
    return (++idSequence);
}

bool Request::create(std::string method, const JsonObject &parameters)
{
    if (!JSONRPC.IsSet() || JSONRPC.Value().compare(WPEFramework::Core::JSONRPC::Message::DefaultVersion))
    {
        LOGERR("Failed JSONRPC2 version check");
        return false;
    }

    if (method.empty())
    {
        LOGERR("Provided method is empty");
        return false;
    }
    Designator = method;

    std::string paramsString;
    parameters.ToString(paramsString);
    if (!paramsString.empty()) {
        Parameters = paramsString;
    }

    Id = generateId();
    return true;
}

std::string Request::toString() const
{
    std::string request;
    ToString(request);
    return request;
}

}   // namespace JsonRpc
}   // namespace WebSockets
