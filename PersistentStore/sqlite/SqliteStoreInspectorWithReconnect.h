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

#include <interfaces/IStoreInspector.h>

namespace WPEFramework {
namespace Plugin {

    template <typename ACTUALSTOREINSPECTOR>
    class SqliteStoreInspectorWithReconnect : public ACTUALSTOREINSPECTOR {
    private:
        SqliteStoreInspectorWithReconnect(const SqliteStoreInspectorWithReconnect<ACTUALSTOREINSPECTOR>&) = delete;
        SqliteStoreInspectorWithReconnect<ACTUALSTOREINSPECTOR>& operator=(const SqliteStoreInspectorWithReconnect<ACTUALSTOREINSPECTOR>&) = delete;

    public:
        template <typename... Args>
        SqliteStoreInspectorWithReconnect(Args&&... args)
            : ACTUALSTOREINSPECTOR(std::forward<Args>(args)...)
        {
        }
        virtual ~SqliteStoreInspectorWithReconnect() override = default;

    public:
        uint32_t SetNamespaceLimit(const string& ns, const uint32_t size, const Exchange::IStoreInspector::ScopeType scope) override
        {
            auto result = ACTUALSTOREINSPECTOR::SetNamespaceLimit(ns, size, scope);
            if (result == Core::ERROR_UNAVAILABLE) {
                if (ACTUALSTOREINSPECTOR::_data->Open() == Core::ERROR_NONE) {
                    result = ACTUALSTOREINSPECTOR::SetNamespaceLimit(ns, size, scope);
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
