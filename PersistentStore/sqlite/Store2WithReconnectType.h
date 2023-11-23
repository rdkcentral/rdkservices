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

#include <interfaces/IStore2.h>

namespace WPEFramework {
namespace Plugin {
    namespace Sqlite {

        // Note, this class expects sqlite error on write if the file deletes unexpectedly. As it's not documented it may not work

        template <typename ACTUALSTORE2>
        class Store2WithReconnectType : public ACTUALSTORE2 {
        private:
            Store2WithReconnectType(const Store2WithReconnectType<ACTUALSTORE2>&) = delete;
            Store2WithReconnectType<ACTUALSTORE2>& operator=(const Store2WithReconnectType<ACTUALSTORE2>&) = delete;

        public:
            template <typename... Args>
            Store2WithReconnectType(Args&&... args)
                : ACTUALSTORE2(std::forward<Args>(args)...)
            {
            }
            ~Store2WithReconnectType() override = default;

        public:
            uint32_t SetValue(const Exchange::IStore2::ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl) override
            {
                auto result = ACTUALSTORE2::SetValue(scope, ns, key, value, ttl);
                if (result == Core::ERROR_UNAVAILABLE) {
                    // SetValue is so important to us that we want to create (presumably) a new file and write anyway

                    if (ACTUALSTORE2::Open() == Core::ERROR_NONE) {
                        result = ACTUALSTORE2::SetValue(scope, ns, key, value, ttl);
                    }
                }
                if (result == Core::ERROR_UNAVAILABLE) {
                    result = Core::ERROR_GENERAL;
                }
                return result;
            }
        };

    } // namespace Sqlite
} // namespace Plugin
} // namespace WPEFramework
