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

#include <interfaces/IStoreCache.h>
#include <sqlite3.h>

namespace WPEFramework {
namespace Plugin {
    namespace Sqlite {

        template <typename ACTUALHANDLE>
        class StoreCacheType : public Exchange::IStoreCache, protected ACTUALHANDLE {
        private:
            StoreCacheType(const StoreCacheType<ACTUALHANDLE>&) = delete;
            StoreCacheType<ACTUALHANDLE>& operator=(const StoreCacheType<ACTUALHANDLE>&) = delete;

        public:
            template <typename... Args>
            StoreCacheType(Args&&... args)
                : Exchange::IStoreCache()
                , ACTUALHANDLE(std::forward<Args>(args)...)
            {
            }
            ~StoreCacheType() override = default;

        public:
            // IStoreCache methods

            uint32_t FlushCache() override
            {
                uint32_t result;

                auto rc = sqlite3_db_cacheflush(*this);

                if (rc == SQLITE_OK) {
                    result = Core::ERROR_NONE;
                } else {
                    TRACE(Trace::Error, (_T("Sqlite cache flush error %d"), rc));
                    result = Core::ERROR_GENERAL;
                }

                sync();

                return result;
            }

            BEGIN_INTERFACE_MAP(StoreCacheType)
            INTERFACE_ENTRY(Exchange::IStoreCache)
            END_INTERFACE_MAP
        };

    } // namespace Sqlite
} // namespace Plugin
} // namespace WPEFramework
