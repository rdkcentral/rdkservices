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

#include "../Module.h"
#include <interfaces/IStoreCache.h>
#include <sqlite3.h>

namespace WPEFramework {
namespace Plugin {
    namespace Sqlite {

        class StoreCache : public Exchange::IStoreCache {
        private:
            StoreCache(const StoreCache&) = delete;
            StoreCache& operator=(const StoreCache&) = delete;

        public:
            StoreCache()
                : StoreCache(getenv(PATH_ENV))
            {
            }
            StoreCache(const string& path)
                : IStoreCache()
                , _path(path)
            {
                Open();
            }
            ~StoreCache() override
            {
                Close();
            }

        private:
            void Open()
            {
                Core::File file(_path);
                Core::Directory(file.PathName().c_str()).CreatePath();
                auto rc = sqlite3_open(_path.c_str(), &_data);
                if (rc != SQLITE_OK) {
                    OnError(__FUNCTION__, rc);
                }
                const std::vector<string> statements = {
                    "pragma busy_timeout = 1000000;"
                };
                for (auto& sql : statements) {
                    auto rc = sqlite3_exec(_data, sql.c_str(), nullptr, nullptr, nullptr);
                    if (rc != SQLITE_OK) {
                        OnError(__FUNCTION__, rc);
                    }
                }
            }
            void Close()
            {
                auto rc = sqlite3_close_v2(_data);
                if (rc != SQLITE_OK) {
                    OnError(__FUNCTION__, rc);
                }
            }

        public:
            uint32_t FlushCache() override
            {
                uint32_t result;

                auto rc = sqlite3_db_cacheflush(_data);

                if (rc == SQLITE_OK) {
                    result = Core::ERROR_NONE;
                } else {
                    OnError(__FUNCTION__, rc);
                    result = Core::ERROR_GENERAL;
                }

                sync();

                return result;
            }

            BEGIN_INTERFACE_MAP(StoreCache)
            INTERFACE_ENTRY(IStoreCache)
            END_INTERFACE_MAP

        private:
            void OnError(const char* fn, const int status) const
            {
                TRACE(Trace::Error, (_T("%s sqlite error %d"), fn, status));
            }

        private:
            const string _path;
            sqlite3* _data;
        };

    } // namespace Sqlite
} // namespace Plugin
} // namespace WPEFramework
