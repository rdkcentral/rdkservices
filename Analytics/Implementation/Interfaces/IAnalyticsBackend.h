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
#pragma once
#include <string>
#include <list>
#include <plugins/IShell.h>

#include "ISystemTime.h"
#include "ILocalStore.h"


// Interface for Analytics Backends
namespace WPEFramework {
namespace Plugin {

    struct IAnalyticsBackend {
        virtual ~IAnalyticsBackend() = default;

        struct Event
        {
            std::string eventName;
            std::string eventVersion;
            std::string eventSource;
            std::string eventSourceVersion;
            std::list<std::string> cetList;
            uint64_t epochTimestamp;
            std::string appId;
            std::string eventPayload;
        };

        virtual const std::string& Name() const = 0;

        virtual uint32_t Configure(PluginHost::IShell* shell, ISystemTimePtr sysTime, ILocalStorePtr store) = 0;
        virtual uint32_t SendEvent(const Event& event) = 0;
    };

    using IAnalyticsBackendPtr = std::shared_ptr<IAnalyticsBackend>;
}
}