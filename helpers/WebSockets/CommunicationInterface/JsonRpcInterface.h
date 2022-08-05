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
#include <future>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <websocketpp/frame.hpp>
#include <boost/optional.hpp>

#include "../JsonRpc/Request.h"
#include "../JsonRpc/Response.h"
#include "../JsonRpc/Notification.h"

#include "Module.h"
#include "UtilsJsonRpc.h"
#include "UtilsLogging.h"

namespace WebSockets   {

template<typename Derived>
class JsonRpcInterface
{
public:
    JsonRpcInterface() = default;

    // Note: return value: sending result && receiving result && json-rpc's response field "success" value
    bool sendRequest(const JsonRpc::Request& request, JsonRpc::Response& response);
    void setNotificationHandler(std::function<void(const JsonRpc::Notification&)> notificationHandler);

protected:
    ~JsonRpcInterface() = default;
    void onMessage(const std::string& message);

    websocketpp::frame::opcode::value opcode_{websocketpp::frame::opcode::text};

private:
    JsonRpcInterface(const JsonRpcInterface&) = delete;
    JsonRpcInterface& operator=(const JsonRpcInterface&) = delete;

    bool getAsyncResponse(uint32_t id, std::future<string>& futureReponse, JsonRpc::Response& response);
    boost::optional<std::future<string> > createFutureResponse(uint32_t id);
    void removePromise(uint32_t id);
    bool isRequestProcessingOngoing(uint32_t id);
    void handleNotification(const std::string& message);
    void handleResponse(uint32_t id, const std::string& message);
    bool processResponse(uint32_t id, const std::string &responseString, JsonRpc::Response &deviceResponse) const;

    std::mutex responseMutex_;
    std::map<uint32_t, std::promise<std::string> > responseToPromise_;
    std::function<void(const JsonRpc::Notification&)> notificationHandler_;
};

template<typename Derived>
bool JsonRpcInterface<Derived>::sendRequest(const JsonRpc::Request& request, JsonRpc::Response& response)
{
    if (isRequestProcessingOngoing(request.getId()))
    {
        LOGERR("Processing of request with ID:%d is already ongoing. Dropping new one:%s",
            request.getId(), request.toString().c_str());
        return false;
    }

    auto futureReponse = createFutureResponse(request.getId());
    if (!futureReponse)
    {
        LOGERR("Can't create future response.");
        return false;
    }

    LOGINFO("Sending json-rpc request: %s", request.toString().c_str());
    Derived& derived = static_cast<Derived&>(*this);
    if (derived.send(request.toString()))
    {
        // Using futureResponse in addition to request.getId() to not lock mutex and search again.
        return getAsyncResponse(request.getId(), *futureReponse, response);
    }
    else
    {
        LOGERR("Sending request with ID:%d failed. Cleaning internal state.", request.getId());
        removePromise(request.getId());
        return false;
    }
    return false;
}

template<typename Derived>
void JsonRpcInterface<Derived>::setNotificationHandler(std::function<void(const JsonRpc::Notification&)> notificationHandler)
{
    notificationHandler_ = notificationHandler;
}

template<typename Derived>
void JsonRpcInterface<Derived>::onMessage(const std::string& message)
{
    LOGINFO("On message: %s", message.c_str());

    JsonObject json;
    if (!json.FromString(message))
    {
        LOGERR("Discarding message. Message contains malformed JSON: %s", message.c_str());
        return;
    }
    if (json.HasLabel("id"))
    {
        LOGINFO("Message contains id field");
        if (json.HasLabel("method"))
        {
            // TO DO: Implement when there will be use case for client receiving request
            // or when JsonRpcInterace will be used for SingleClientServer
        }
        else
        {
            if (json["id"].Content() != WPEFramework::Core::JSON::Variant::type::NUMBER)
            {
                LOGERR("Received message contains ID field which is not a number. Dropping.");
                return;
            }
            handleResponse(json["id"].Number(), message);
            return;
        }
    }
    else
    {
        LOGINFO("Message doesn't contains id field");
        handleNotification(message);
    }
}

template<typename Derived>
bool JsonRpcInterface<Derived>::getAsyncResponse(uint32_t id, std::future<string>& futureReponse, JsonRpc::Response& response)
{
    switch (futureReponse.wait_for(std::chrono::seconds(5)))
    {
        case std::future_status::ready:
            removePromise(id);
            return processResponse(id, futureReponse.get(), response);
        case std::future_status::timeout:
            LOGERR("Timeout for request/response ID:%d", id);
            removePromise(id);
            return false;
        case std::future_status::deferred:
            LOGERR("Execution for request/response ID:%d deferred. Internal error.", id);  // "should not happen"
            removePromise(id);
            return false;
    }
    return false;
}

template<typename Derived>
boost::optional<std::future<string> > JsonRpcInterface<Derived>::createFutureResponse(uint32_t id)
{
    std::lock_guard<std::mutex> lock(responseMutex_);
    auto insertionResult = responseToPromise_.emplace(std::make_pair(id, std::promise<std::string>{} ));
    if (!insertionResult.second)
    {
        LOGERR("Request with the same ID:%d already saved.", id);
        return boost::none;
    }
    return insertionResult.first->second.get_future();
}

template<typename Derived>
void JsonRpcInterface<Derived>::removePromise(uint32_t id)
{
    std::lock_guard<std::mutex> lock(responseMutex_);
    responseToPromise_.erase(id);
}

template<typename Derived>
bool JsonRpcInterface<Derived>::isRequestProcessingOngoing(uint32_t id)
{
    std::lock_guard<std::mutex> lock(responseMutex_);
    return responseToPromise_.count(id) != 0;
}

template<typename Derived>
void JsonRpcInterface<Derived>::handleNotification(const std::string& message)
{
    LOGINFO();
    JsonRpc::Notification notif;
    notif.FromString(message);
    if (!notif.isValid())
    {
        LOGERR("Malformed jsonrpc notification. Dropping.");
        return;
    }
    if (!notificationHandler_)
    {
        LOGINFO("No handler for notifications set. Dropping notification.");
        return;
    }
    notificationHandler_(notif);
}

template<typename Derived>
void JsonRpcInterface<Derived>::handleResponse(uint32_t id, const std::string& message)
{
    std::lock_guard<std::mutex> lock(responseMutex_);
    const auto& idToPromise = responseToPromise_.find(id);
    if (idToPromise == responseToPromise_.end())
    {
        LOGERR("Can't find request with id:%d. Dropping response.", id);
        return;
    };
    idToPromise->second.set_value(message);
}

template<typename Derived>
bool JsonRpcInterface<Derived>::processResponse(uint32_t id, const std::string &responseString, JsonRpc::Response &deviceResponse) const
{
    bool success = false;

    deviceResponse.FromString(responseString);

    if (deviceResponse.isValid() && (deviceResponse.getId() == id))
    {
        JsonObject parameters;

        if (deviceResponse.isResult())
        {
            deviceResponse.getResult(parameters);
            if (parameters.HasLabel("success"))
            {
                getBoolParameter("success", success);
                LOGINFO("Result success: %u", success);
            }
        }

        if (deviceResponse.isError())
        {
            deviceResponse.getError(parameters);
            int error;
            std::string message;
            getNumberParameter("code", error);
            getStringParameter("message", message);
            LOGERR("Error code:message: %i:%s", error, message.c_str());
        }
    }
    else
    {
        LOGERR("Invalid response request_id:response_id %u:%u", id, deviceResponse.getId());
    }

    return success;
}

}   // namespace WebSockets
