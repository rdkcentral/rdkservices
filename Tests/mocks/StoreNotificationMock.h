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

#ifndef STORENOTIFICATIONMOCK_H
#define STORENOTIFICATIONMOCK_H

#include <gmock/gmock.h>

#include <interfaces/IStore.h>

class StoreNotificationMock : public WPEFramework::Exchange::IStore::INotification {
public:
    virtual ~StoreNotificationMock() = default;

    MOCK_METHOD(void, ValueChanged, (const string&, const string&, const string&), (override));
    MOCK_METHOD(void, StorageExceeded, (), (override));

    BEGIN_INTERFACE_MAP(StoreNotificationMock)
    INTERFACE_ENTRY(WPEFramework::Exchange::IStore::INotification)
    END_INTERFACE_MAP
};

#endif //STORENOTIFICATIONMOCK_H
