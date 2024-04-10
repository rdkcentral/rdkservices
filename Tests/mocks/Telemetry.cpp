/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2023 Synamedia
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

#include "Telemetry.h"
#include <gmock/gmock.h>

TelemetryApiImpl* TelemetryApi::impl = nullptr;

TelemetryApi::TelemetryApi() {}

void TelemetryApi::setImpl(TelemetryApiImpl* newImpl)
{
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

void TelemetryApi::t2_init(char* component)
{
    EXPECT_NE(impl, nullptr);
    impl->t2_init(component);
}

T2ERROR TelemetryApi::t2_event_s(char* marker, char* value)
{
    EXPECT_NE(impl, nullptr);
    return impl->t2_event_s(marker, value);
}
void (*t2_init)(char*) = &TelemetryApi::t2_init;
T2ERROR (*t2_event_s)(char*,char*) = &TelemetryApi::t2_event_s;
