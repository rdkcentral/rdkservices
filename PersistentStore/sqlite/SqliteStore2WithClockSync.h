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

enum class IARM_EventId_t;

namespace WPEFramework {
namespace Plugin {

    template <typename ACTUALSTORE2>
    class SqliteStore2WithClockSync : public ACTUALSTORE2 {
    private:
        SqliteStore2WithClockSync(const SqliteStore2WithClockSync<ACTUALSTORE2>&) = delete;
        SqliteStore2WithClockSync<ACTUALSTORE2>& operator=(const SqliteStore2WithClockSync<ACTUALSTORE2>&) = delete;

        class IARMHandler {
        private:
            IARMHandler(const IARMHandler&) = delete;
            IARMHandler& operator=(const IARMHandler&) = delete;

            void onEvent(const char* owner, IARM_EventId_t eventId, void* data, size_t len);

        public:
            IARMHandler();
            ~IARMHandler();

        private:
            bool clockSynced;
        };

    public:
        template <typename... Args>
        SqliteStore2WithClockSync(Args&&... args)
            : ACTUALSTORE2(std::forward<Args>(args)...)
        {
        }
        virtual ~SqliteStore2WithClockSync() override = default;

    public:
        uint32_t SetValue(const string& ns, const string& key, const string& value, const Exchange::IStore2::ScopeType scope, const uint32_t ttl) override
        {
            return ((ttl != 0) && !handler.clockSynced)
                ? Core::ERROR_PENDING_CONDITIONS
                : ACTUALSTORE2::SetValue(ns, key, value, scope, ttl);
        }
        uint32_t GetValue(const string& ns, const string& key, const Exchange::IStore2::ScopeType scope, string& value, uint32_t& ttl) override
        {
            string v;
            uint32_t t;
            auto result = ACTUALSTORE2::GetValue(ns, key, scope, v, t);
            if (result == Core::ERROR_NONE) {
                if ((t != 0) && !handler.clockSynced) {
                    result = Core::ERROR_PENDING_CONDITIONS;
                } else {
                    value = v;
                    ttl = t;
                }
            } else if ((result == Core::ERROR_UNKNOWN_KEY) && !handler.clockSynced) {
                result = Core::ERROR_PENDING_CONDITIONS;
            }
            return result;
        }

    private:
        IARMHandler handler;
    };

} // namespace Plugin
} // namespace WPEFramework
