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

#include <interfaces/IStoreInspector.h>

namespace WPEFramework {
namespace Plugin {

    class SqliteStoreInspector : public Exchange::IStoreInspector {
    private:
        SqliteStoreInspector(const SqliteStoreInspector&) = delete;
        SqliteStoreInspector& operator=(const SqliteStoreInspector&) = delete;

        typedef RPC::IteratorType<INamespaceSizeIterator> NamespaceSizeIterator;

    public:
        SqliteStoreInspector(ISqliteDb* db)
            : _data(db)
        {
            ASSERT(_data != nullptr);
            _data->AddRef();
        }
        virtual ~SqliteStoreInspector() override
        {
            _data->Release();
        }

    public:
        // IStoreInspector methods

        virtual uint32_t SetNamespaceLimit(const string& ns, const uint32_t size, const ScopeType scope) override;
        virtual uint32_t GetNamespaceLimit(const string& ns, const ScopeType scope, uint32_t& size) override;
        virtual uint32_t GetKeys(const string& ns, const ScopeType scope, RPC::IStringIterator*& keys) override;
        virtual uint32_t GetNamespaces(const ScopeType scope, RPC::IStringIterator*& namespaces) override;
        virtual uint32_t GetNamespaceSizes(const ScopeType scope, INamespaceSizeIterator*& namespaceSizes) override;

        BEGIN_INTERFACE_MAP(SqliteStoreInspector)
        INTERFACE_ENTRY(Exchange::IStoreInspector)
        END_INTERFACE_MAP

    protected:
        ISqliteDb* _data;
    };

} // namespace Plugin
} // namespace WPEFramework
