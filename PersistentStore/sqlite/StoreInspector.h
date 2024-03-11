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
#include <interfaces/IStore2.h>
#include <sqlite3.h>

namespace WPEFramework {
namespace Plugin {
    namespace Sqlite {

        class StoreInspector : public Exchange::IStoreInspector {
        private:
            StoreInspector(const StoreInspector&) = delete;
            StoreInspector& operator=(const StoreInspector&) = delete;

            typedef RPC::IteratorType<INamespaceSizeIterator> NamespaceSizeIterator;

        public:
            StoreInspector()
                : StoreInspector(getenv(PATH_ENV))
            {
            }
            StoreInspector(const string& path)
                : IStoreInspector()
                , _path(path)
            {
                Open();
            }
            ~StoreInspector() override
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
                    "pragma foreign_keys = on;",
                    "pragma busy_timeout = 1000000;",
                    "create table if not exists namespace (id integer primary key,name text unique);",
                    "create table if not exists item (ns integer,key text,value text,foreign key(ns) references namespace(id) on delete cascade on update no action,unique(ns,key) on conflict replace);"
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
            uint32_t GetKeys(const ScopeType scope, const string& ns, RPC::IStringIterator*& keys) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(_data, "select key"
                                          " from item"
                                          " where ns in (select id from namespace where name = ?)"
                                          ";",
                    -1, &stmt, nullptr);
                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);
                std::list<string> list;
                int rc;
                while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                    list.emplace_back((const char*)sqlite3_column_text(stmt, 0));
                }
                sqlite3_finalize(stmt);

                if (rc == SQLITE_DONE) {
                    keys = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(list));
                    result = Core::ERROR_NONE;
                } else {
                    OnError(__FUNCTION__, rc);
                    result = Core::ERROR_GENERAL;
                }

                return result;
            }
            uint32_t GetNamespaces(const ScopeType scope, RPC::IStringIterator*& namespaces) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(_data, "select name from namespace;", -1, &stmt, nullptr);
                std::list<string> list;
                int rc;
                while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                    list.emplace_back((const char*)sqlite3_column_text(stmt, 0));
                }
                sqlite3_finalize(stmt);

                if (rc == SQLITE_DONE) {
                    namespaces = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(list));
                    result = Core::ERROR_NONE;
                } else {
                    OnError(__FUNCTION__, rc);
                    result = Core::ERROR_GENERAL;
                }

                return result;
            }
            uint32_t GetStorageSizes(const ScopeType scope, INamespaceSizeIterator*& storageList) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(_data, "select name, sum(length(key)+length(value))"
                                          " from item"
                                          " inner join namespace on namespace.id = item.ns"
                                          " group by name"
                                          ";",
                    -1, &stmt, nullptr);
                std::list<NamespaceSize> list;
                int rc;
                while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                    NamespaceSize namespaceSize;
                    namespaceSize.ns = (const char*)sqlite3_column_text(stmt, 0);
                    namespaceSize.size = sqlite3_column_int(stmt, 1);
                    list.emplace_back(namespaceSize);
                }
                sqlite3_finalize(stmt);

                if (rc == SQLITE_DONE) {
                    storageList = (Core::Service<NamespaceSizeIterator>::Create<INamespaceSizeIterator>(list));
                    result = Core::ERROR_NONE;
                } else {
                    OnError(__FUNCTION__, rc);
                    result = Core::ERROR_GENERAL;
                }

                return result;
            }

            BEGIN_INTERFACE_MAP(StoreInspector)
            INTERFACE_ENTRY(IStoreInspector)
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
