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

#include <map>
#include <string>
#include "../../Module.h"
#include "IAnalyticsBackend.h"
#include "UtilsLibraryLoader.h"

// Interface for Analytics Backends
namespace WPEFramework {
namespace Plugin {

    class AnalyticsBackendLoader
    {
    public:
        AnalyticsBackendLoader() = default;
        ~AnalyticsBackendLoader() = default;

        uint32_t Load(std::string path);
        IAnalyticsBackendPtr GetBackend() const
        {
            return mAnalyticsBackend;
        }
    private:
            Utils::LibraryLoader mLibrariesLoader;
            IAnalyticsBackendPtr mAnalyticsBackend;
    };

}
}
