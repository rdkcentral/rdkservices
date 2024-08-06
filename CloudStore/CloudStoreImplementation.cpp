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

#include "CloudStoreImplementation.h"
#include "grpc/Store2.h"

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(CloudStoreImplementation, 1, 0);

    CloudStoreImplementation::CloudStoreImplementation()
        : _accountStore2(Core::Service<Grpc::Store2>::Create<Exchange::IStore2>())
    {
        ASSERT(_accountStore2 != nullptr);
    }

    CloudStoreImplementation::~CloudStoreImplementation()
    {
        if (_accountStore2 != nullptr) {
            _accountStore2->Release();
            _accountStore2 = nullptr;
        }
    }

} // namespace Plugin
} // namespace WPEFramework
