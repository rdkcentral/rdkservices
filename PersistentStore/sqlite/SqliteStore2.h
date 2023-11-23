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

#include "ISqliteDb.h"

#include <interfaces/IStore2.h>

namespace WPEFramework {
namespace Plugin {

    class SqliteStore2 : public Exchange::IStore2 {
    private:
        SqliteStore2(const SqliteStore2&) = delete;
        SqliteStore2& operator=(const SqliteStore2&) = delete;

    public:
        SqliteStore2(ISqliteDb* db)
            : _data(db)
            , _clients()
            , _clientLock()
        {
            ASSERT(_data != nullptr);
            _data->AddRef();
        }
        virtual ~SqliteStore2() override
        {
            _data->Release();
        }

    public:
        // IStore2 methods

        virtual uint32_t Register(Exchange::IStore2::INotification* notification) override
        {
            Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

            ASSERT(std::find(_clients.begin(), _clients.end(), notification) == _clients.end());

            notification->AddRef();
            _clients.push_back(notification);

            return Core::ERROR_NONE;
        }
        virtual uint32_t Unregister(Exchange::IStore2::INotification* notification) override
        {
            Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

            std::list<Exchange::IStore2::INotification*>::iterator
                index(std::find(_clients.begin(), _clients.end(), notification));

            ASSERT(index != _clients.end());

            if (index != _clients.end()) {
                notification->Release();
                _clients.erase(index);
            }

            return Core::ERROR_NONE;
        }

        virtual uint32_t SetValue(const string& ns, const string& key, const string& value, const ScopeType scope, const uint32_t ttl) override;
        virtual uint32_t GetValue(const string& ns, const string& key, const ScopeType scope, string& value, uint32_t& ttl) override;
        virtual uint32_t DeleteKey(const string& ns, const string& key, const ScopeType scope) override;
        virtual uint32_t DeleteNamespace(const string& ns, const ScopeType scope) override;

        BEGIN_INTERFACE_MAP(SqliteStore2)
        INTERFACE_ENTRY(Exchange::IStore2)
        END_INTERFACE_MAP

    protected:
        void OnValueChanged(const string& ns, const string& key, const string& value, const ScopeType scope)
        {
            Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

            std::list<Exchange::IStore2::INotification*>::iterator
                index(_clients.begin());

            while (index != _clients.end()) {
                (*index)->ValueChanged(ns, key, value, scope);
                index++;
            }
        }

    protected:
        ISqliteDb* _data;
        std::list<Exchange::IStore2::INotification*> _clients;
        Core::CriticalSection _clientLock;
    };

} // namespace Plugin
} // namespace WPEFramework
