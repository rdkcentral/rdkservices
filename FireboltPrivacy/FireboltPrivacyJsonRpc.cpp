
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

#include "Module.h"
#include "FireboltPrivacy.h"
#include <interfaces/json/JsonData_FireboltPrivacy.h>

namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::FireboltPrivacy;

    void FireboltPrivacy::RegisterAll()
    {
        Property<Core::JSON::Boolean>(_T("allowresumepoint"), &FireboltPrivacy::endpoint_allowresumepoint, nullptr, this);
        Register<SetAllowResumePointsParamsData, void>(_T("setallowresumepoint"), &FireboltPrivacy::endpoint_setallowresumepoint, this);
    }
    void FireboltPrivacy::UnregisterAll()
    {
        Unregister(_T("allowresumepoint"));
        Unregister(_T("setallowresumepoint"));
    }
        uint32_t FireboltPrivacy::endpoint_setallowresumepoint(const SetAllowResumePointsParamsData& params){
          return 0;
        }
        uint32_t FireboltPrivacy::endpoint_allowresumepoint(Core::JSON::Boolean& response) const {
          response = true;
          return 0;
        }
}
}
