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

#include <interfaces/IStoreCache.h>

namespace WPEFramework {
namespace Plugin {

    class SqliteStoreCache : public Exchange::IStoreCache {
    private:
        SqliteStoreCache(const SqliteStoreCache&) = delete;
        SqliteStoreCache& operator=(const SqliteStoreCache&) = delete;

    public:
        SqliteStoreCache(ISqliteDb* db)
            : _data(db)
        {
            ASSERT(_data != nullptr);
            _data->AddRef();
        }
        virtual ~SqliteStoreCache() override
        {
            _data->Release();
        }

    public:
        // IStoreCache methods

        virtual uint32_t FlushCache() override;

        BEGIN_INTERFACE_MAP(SqliteStoreCache)
        INTERFACE_ENTRY(Exchange::IStoreCache)
        END_INTERFACE_MAP

    private:
        ISqliteDb* _data;
    };

} // namespace Plugin
} // namespace WPEFramework
