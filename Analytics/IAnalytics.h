/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#pragma once

#include "Module.h"

// @stubgen:include <com/IIteratorType.h>

#define ID_ANALYTICS_OFFSET 0x1000

namespace WPEFramework {
namespace Exchange {
    // @json 1.0.0
    struct EXTERNAL IAnalytics : virtual public Core::IUnknown {
        enum { ID = RPC::IDS::ID_EXTERNAL_INTERFACE_OFFSET + ID_ANALYTICS_OFFSET };

        virtual ~IAnalytics() override = default;

        using IStringIterator = RPC::IIteratorType<string, RPC::ID_STRINGITERATOR>;

        /**
         * @alt sendEvent
         * @brief Send an event to the analytics server
         * @param eventName Name of the event
         * @param eventVersion Version of the event
         * @param eventSource Source of the event
         * @param eventSourceVersion Version of the event source
         * @param cetList List of CETs
         * @param epochTimestamp Epoch timestamp of the event
         * @param uptimeTimestamp Uptime timestamp of the event
         * @param eventPayload Payload of the event
         */
        virtual Core::hresult SendEvent(const string& eventName /* @in */,
                                   const string& eventVersion /* @in */,
                                   const string& eventSource /* @in */,
                                   const string& eventSourceVersion /* @in */,
                                   IStringIterator* const& cetList /* @in */,
                                   const uint64_t epochTimestamp /* @in */,
                                   const uint64_t uptimeTimestamp /* @in */,
                                   const string& eventPayload /* @in */ ) = 0;
    };
}
}