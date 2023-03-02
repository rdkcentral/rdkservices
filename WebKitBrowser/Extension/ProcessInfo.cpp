/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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

#include "Module.h"

#include <sys/prctl.h>

namespace WPEFramework {
namespace ProcessInfo {

void SetProcessName()
{
    std::string processName;

    Core::SystemInfo::GetEnvironment(std::string(_T("PROCESS_NAME")), processName);

    if (processName.empty()) {
        return;
    }

#ifdef __LINUX__
    int ret = prctl(PR_SET_NAME, processName.c_str());
#else
    #error("No implementation available to set process name on this platform")
#endif
    if (0 != ret ) {
        TRACE_GLOBAL(Trace::Error, (_T("Setting process name to '%s' FAILED with code %d\n"), processName.c_str(), ret));
    } else {
        TRACE_GLOBAL(Trace::Error, (_T("Setting process name to '%s' SUCCESS\n"), processName.c_str()));
    }
}

}  // namespace ProcessInfo
}  // namespace WPEFramework
