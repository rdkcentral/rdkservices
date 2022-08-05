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

#include "Response.h"

#include "UtilsLogging.h"

namespace WebSockets   {
namespace JsonRpc      {

Response::Response() : WPEFramework::Core::JSONRPC::Message()
{
}

uint32_t Response::getId() const
{
    return Id.Value();
}

bool Response::isResult() const
{
    return Result.IsSet();
}

bool Response::getResult(JsonObject& jsonObject) const
{
    bool result = false;

    if (isResult())
    {
        jsonObject.FromString(Result.Value());
        result = true;
    }

    return result;
}

bool Response::isError() const
{
    return Error.IsSet();
}

bool Response::getError(JsonObject& jsonObject) const
{
    bool result = false;

    if (isError())
    {
        std::string errorString;
        Error.ToString(errorString);
        jsonObject.FromString(errorString);
        result = true;
    }

    return result;
}

bool Response::isValid() const
{
    if (!JSONRPC.IsSet() || JSONRPC.Value().compare(WPEFramework::Core::JSONRPC::Message::DefaultVersion))
    {
        LOGERR("Failed jsonrpc version check");
        return false;
    }

    if (!Id.IsSet())
    {
        LOGERR("Failed jsonrpc id check");
        return false;
    }

    if (!Result.IsSet() && !Error.IsSet())
    {
        LOGERR("Failed jsonrpc result/error check - both missing");
        return false;
    }

    if (Result.IsSet() && Error.IsSet())
    {
        LOGERR("Failed jsonrpc result/error check - both set");
        return false;
    }

    if (Error.IsSet())
    {
        if (!Error.Code.IsSet() || !Error.Text.IsSet())
        {
            LOGERR("Failed jsonrpc error code/message check");
            return false;
        }
    }

    return true;
}

}   // namespace JsonRpc
}   // namespace WebSockets
