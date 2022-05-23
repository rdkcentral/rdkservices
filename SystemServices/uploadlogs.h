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

#ifndef RDKSERVICES_UPLOADLOGS_H
#define RDKSERVICES_UPLOADLOGS_H

#include <string>

namespace WPEFramework
{
namespace Plugin
{
namespace UploadLogs
{
    enum err_t { OK = 0, BadUrl, FilenameFail, SsrFail, TarFail, UploadFail, };
    err_t upload(const std::string& ssrUrl = std::string());
    int32_t LogUploadBeforeDeepSleep(void);
    std::string errToText(err_t err);
} // namespace UploadLogs
} // namespace Plugin
} // namespace WPEFramework

#endif //RDKSERVICES_UPLOADLOGS_H
