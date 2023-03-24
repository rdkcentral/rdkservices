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

#include "WifiManagerConnect.h"
// #include "UtilsLogging.h"
#include "UtilsJsonRpc.h"

#include <cstring>

using namespace WPEFramework::Plugin;

uint32_t WifiManagerConnect::connect(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();
    returnResponse(false);
}

uint32_t WifiManagerConnect::disconnect(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();
    returnResponse(false);
}
