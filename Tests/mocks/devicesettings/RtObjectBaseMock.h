/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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

#include <gmock/gmock.h>

class rtObjectBaseMock : public rtObjectBaseImpl {
public:
    virtual ~rtObjectBaseMock() = default;

    MOCK_METHOD(rtError, set, (const char* name, const char* value), (const, override));
    MOCK_METHOD(rtError, set, (const char* name, const rtValue& value), (const, override));
    MOCK_METHOD(rtError, set, (const char* name, bool value), (const, override));
    MOCK_METHOD(rtString, get, (const char* name), (const, override));
    MOCK_METHOD(rtError, sendReturns, (const char* messageName, rtString& result), (const, override));


};
