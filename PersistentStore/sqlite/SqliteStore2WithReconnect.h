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

    template <typename ACTUALSTORE2>
    class SqliteStore2WithReconnect : public ACTUALSTORE2 {
    private:
        SqliteStore2WithReconnect(const SqliteStore2WithReconnect<ACTUALSTORE2>&) = delete;
        SqliteStore2WithReconnect<ACTUALSTORE2>& operator=(const SqliteStore2WithReconnect<ACTUALSTORE2>&) = delete;

    public:
        template <typename... Args>
        SqliteStore2WithReconnect(Args&&... args)
            : ACTUALSTORE2(std::forward<Args>(args)...)
        {
        }
        virtual ~SqliteStore2WithReconnect() override = default;

    public:
        virtual uint32_t SetValue(const string& ns, const string& key, const string& value, const Exchange::IStore2::ScopeType scope, const uint32_t ttl) override
        {
            auto result = ACTUALSTORE2::SetValue(ns, key, value, scope, ttl);
            if (result == Core::ERROR_UNAVAILABLE) {
                if (ACTUALSTORE2::_data->Open() == Core::ERROR_NONE) {
                    result = ACTUALSTORE2::SetValue(ns, key, value, scope, ttl);
                }
            }
            if (result == Core::ERROR_UNAVAILABLE) {
                result = Core::ERROR_GENERAL;
            }
            return result;
        }
    };

} // namespace Plugin
} // namespace WPEFramework
