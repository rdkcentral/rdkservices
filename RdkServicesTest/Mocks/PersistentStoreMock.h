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

#ifndef PERSISTENTSTOREMOCK_H
#define PERSISTENTSTOREMOCK_H

#include <gmock/gmock.h>

#include "PersistentStore.h"

class PersistentStoreMock : public WPEFramework::Plugin::PersistentStore {
public:
    virtual ~PersistentStoreMock() = default;

    MOCK_METHOD(void, event_onValueChanged, (const string&, const string&, const string&), (override));
    MOCK_METHOD(void, event_onStorageExceeded, (), (override));
    MOCK_METHOD(std::vector<string>, LegacyLocations, (), (const, override));
};

#endif //PERSISTENTSTOREMOCK_H
