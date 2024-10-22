/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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
 */

#include "Analytics.h"
#include "UtilsJsonRpc.h"

const string WPEFramework::Plugin::Analytics::ANALYTICS_METHOD_SEND_EVENT = "sendEvent";

namespace WPEFramework {

namespace Plugin {
   // Registration
    //

    void Analytics::RegisterAll()
    {
        Register(_T(ANALYTICS_METHOD_SEND_EVENT), &Analytics::SendEventWrapper, this);
    }

    void Analytics::UnregisterAll()
    {
        Unregister(_T(ANALYTICS_METHOD_SEND_EVENT));
    }

    // API implementation
    //

    // Method: sendEvent - Send an event to the analytics server
    // Return codes:
    //  - ERROR_NONE: Success
    //  - ERROR_GENERAL: Failed to send the event
    uint32_t Analytics::SendEventWrapper(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();

        uint32_t result = Core::ERROR_NONE;

        returnIfStringParamNotFound(parameters, "eventName");
        returnIfStringParamNotFound(parameters, "eventSource");
        returnIfStringParamNotFound(parameters, "eventSourceVersion");
        returnIfParamNotFound(parameters, "cetList");
        returnIfParamNotFound(parameters, "eventPayload");

        string eventName = parameters["eventName"].String();
        string eventVersion = (parameters.HasLabel("eventVersion") ? parameters["eventVersion"].String() : "");
        string eventSource = parameters["eventSource"].String();
        string eventSourceVersion = parameters["eventSourceVersion"].String();
        JsonArray cetListJson = parameters["cetList"].Array(); 
        std::list<string> cetList;
        for (int i=0; i<cetListJson.Length(); i++) {
            cetList.emplace_back(cetListJson[i].String());
        }
        RPC::IStringIterator *cetListIterator = Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(cetList);
        uint64_t epochTimestamp = (parameters.HasLabel("epochTimestamp"))? parameters["epochTimestamp"].Number() : 0;
        uint64_t uptimeTimestamp = (parameters.HasLabel("uptimeTimestamp"))? parameters["uptimeTimestamp"].Number() : 0;
        string eventPayload = parameters["eventPayload"].String();

        if (mAnalytics != nullptr) {
            result = mAnalytics->SendEvent(eventName,
                                            eventVersion,
                                            eventSource,
                                            eventSourceVersion,
                                            cetListIterator,
                                            epochTimestamp,
                                            uptimeTimestamp,
                                            eventPayload);
        }

        cetListIterator->Release();
        returnResponse(result == Core::ERROR_NONE);
    }

}

}