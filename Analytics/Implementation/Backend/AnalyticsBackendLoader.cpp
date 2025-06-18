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
#include "AnalyticsBackendLoader.h"
#include "UtilsLogging.h"

namespace WPEFramework {
namespace Plugin {

uint32_t AnalyticsBackendLoader::Load(std::string path)
{
    if (path.empty()) {
        LOGERR("Analytics backend path is empty");
        return Core::ERROR_GENERAL;
    }

    std::string error;
    uint32_t ret = mLibrariesLoader.Load(path, error);
    if (ret != Utils::LibraryLoader::ErrorCode::NO_ERROR)
    {
        LOGERR("Failed to load analytics backend library (%s): %s", path.c_str(), error.c_str());
        return Core::ERROR_GENERAL;
    }

    mAnalyticsBackend = mLibrariesLoader.CreateShared<IAnalyticsBackend>(error);
    if (mAnalyticsBackend == nullptr)
    {
        LOGERR("Failed to create analytics backend object for library (%s): %s", path.c_str(), error.c_str());
        return Core::ERROR_GENERAL;
    }
    return Core::ERROR_NONE;

}

}
}