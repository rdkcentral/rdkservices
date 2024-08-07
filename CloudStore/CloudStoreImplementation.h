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

#include "Module.h"
#include <interfaces/IStore2.h>

namespace WPEFramework {
namespace Plugin {

    class CloudStoreImplementation : public Exchange::IStore2 {
    private:
        CloudStoreImplementation(const CloudStoreImplementation&) = delete;
        CloudStoreImplementation& operator=(const CloudStoreImplementation&) = delete;

    public:
        CloudStoreImplementation();
        ~CloudStoreImplementation() override;

        BEGIN_INTERFACE_MAP(CloudStoreImplementation)
        INTERFACE_ENTRY(IStore2)
        END_INTERFACE_MAP

    private:
        uint32_t Register(IStore2::INotification* notification) override
        {
            if (_accountStore2 != nullptr) {
                _accountStore2->Register(notification);
            }
            return Core::ERROR_NONE;
        }
        uint32_t Unregister(IStore2::INotification* notification) override
        {
            if (_accountStore2 != nullptr) {
                _accountStore2->Unregister(notification);
            }
            return Core::ERROR_NONE;
        }
        uint32_t SetValue(const IStore2::ScopeType, const string& ns, const string& key, const string& value, const uint32_t ttl) override
        {
            if (_accountStore2 != nullptr) {
                return _accountStore2->SetValue(IStore2::ScopeType::ACCOUNT, ns, key, value, ttl);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t GetValue(const IStore2::ScopeType, const string& ns, const string& key, string& value, uint32_t& ttl) override
        {
            if (_accountStore2 != nullptr) {
                return _accountStore2->GetValue(IStore2::ScopeType::ACCOUNT, ns, key, value, ttl);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t DeleteKey(const IStore2::ScopeType, const string& ns, const string& key) override
        {
            if (_accountStore2 != nullptr) {
                return _accountStore2->DeleteKey(IStore2::ScopeType::ACCOUNT, ns, key);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t DeleteNamespace(const IStore2::ScopeType, const string& ns) override
        {
            if (_accountStore2 != nullptr) {
                return _accountStore2->DeleteNamespace(IStore2::ScopeType::ACCOUNT, ns);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }

    private:
        IStore2* _accountStore2;
    };

} // namespace Plugin
} // namespace WPEFramework
