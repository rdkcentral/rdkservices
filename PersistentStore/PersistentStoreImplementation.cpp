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

#include "PersistentStoreImplementation.h"
#include "grpc/Store2.h"
#include "sqlite/Store2.h"

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(PersistentStoreImplementation, 1, 0);

    PersistentStoreImplementation::PersistentStoreImplementation()
        : _deviceStore2(Core::Service<Sqlite::Store2>::Create<Exchange::IStore2>())
        , _deviceStoreCache(nullptr)
        , _deviceStoreInspector(nullptr)
        , _deviceStoreLimit(nullptr)
        , _accountStore2(Core::Service<Grpc::Store2>::Create<Exchange::IStore2>())
        , _store2Sink(*this)
    {
        if (_deviceStore2 != nullptr) {
            _deviceStore2->Register(&_store2Sink);
            _deviceStoreCache = _deviceStore2->QueryInterface<Exchange::IStoreCache>();
            _deviceStoreInspector = _deviceStore2->QueryInterface<Exchange::IStoreInspector>();
            _deviceStoreLimit = _deviceStore2->QueryInterface<Exchange::IStoreLimit>();
        }

        ASSERT(_deviceStore2 != nullptr);
        ASSERT(_deviceStoreCache != nullptr);
        ASSERT(_deviceStoreInspector != nullptr);
        ASSERT(_deviceStoreLimit != nullptr);
        ASSERT(_accountStore2 != nullptr);
    }

    PersistentStoreImplementation::~PersistentStoreImplementation()
    {
        if (_deviceStore2 != nullptr) {
            _deviceStore2->Unregister(&_store2Sink);
            _deviceStore2->Release();
            _deviceStore2 = nullptr;
        }
        if (_deviceStoreCache != nullptr) {
            _deviceStoreCache->Release();
            _deviceStoreCache = nullptr;
        }
        if (_deviceStoreInspector != nullptr) {
            _deviceStoreInspector->Release();
            _deviceStoreInspector = nullptr;
        }
        if (_deviceStoreLimit != nullptr) {
            _deviceStoreLimit->Release();
            _deviceStoreLimit = nullptr;
        }
        if (_accountStore2 != nullptr) {
            _accountStore2->Release();
            _accountStore2 = nullptr;
        }
    }

} // namespace Plugin
} // namespace WPEFramework
