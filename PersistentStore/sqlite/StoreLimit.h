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

        class StoreLimit : public Exchange::IStoreLimit {
        private:
            StoreLimit(const StoreLimit&) = delete;
            StoreLimit& operator=(const StoreLimit&) = delete;

        public:
            StoreLimit()
                : StoreLimit(
                      getenv(PATH_ENV),
                      std::stoul(getenv(MAXSIZE_ENV)),
                      std::stoul(getenv(MAXVALUE_ENV)))
            {
            }
            StoreLimit(const string& path, const uint64_t maxSize, const uint64_t maxValue)
                : IStoreLimit()
                , _path(path)
                , _maxSize(maxSize)
                , _maxValue(maxValue)
            {
                Open();
            }
            ~StoreLimit() override
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
                    "create table if not exists item (ns integer,key text,value text,foreign key(ns) references namespace(id) on delete cascade on update no action,unique(ns,key) on conflict replace);",
                    "create table if not exists limits (n integer,size integer,foreign key(n) references namespace(id) on delete cascade on update no action,unique(n) on conflict replace);",
                    "create temporary trigger if not exists ns_empty insert on namespace begin select case when length(new.name) = 0 then raise (fail, 'empty') end; end;",
                    "create temporary trigger if not exists ns_maxvalue insert on namespace begin select case when length(new.name) > " + std::to_string(_maxValue) + " then raise (fail, 'max value') end; end;",
                    "create temporary trigger if not exists ns_maxsize insert on namespace begin select case when (select sum(s) from (select sum(length(key)+length(value)) s from item union all select sum(length(name)) s from namespace union all select length(new.name) s)) > " + std::to_string(_maxSize) + " then raise (fail, 'max size') end; end;"
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
            uint32_t SetNamespaceStorageLimit(const ScopeType scope, const string& ns, const uint32_t size) override
            {
                ASSERT(scope == ScopeType::DEVICE);
                if (scope != ScopeType::DEVICE) {
                    return Core::ERROR_GENERAL;
                }

                uint32_t result;

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(_data, "insert or ignore into namespace (name) values (?);",
                    -1, &stmt, nullptr);
                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);
                auto rc = sqlite3_step(stmt);
                sqlite3_finalize(stmt);
                if (rc == SQLITE_DONE) {
                    sqlite3_prepare_v2(_data, "insert into limits (n,size)"
                                              " select id, ?"
                                              " from namespace"
                                              " where name = ?"
                                              ";",
                        -1, &stmt, nullptr);
                    sqlite3_bind_int(stmt, 1, size);
                    sqlite3_bind_text(stmt, 2, ns.c_str(), -1, SQLITE_TRANSIENT);
                    rc = sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                }

                if (rc == SQLITE_DONE) {
                    result = Core::ERROR_NONE;
                } else {
                    OnError(__FUNCTION__, rc);
                    if (rc == SQLITE_CONSTRAINT) {
                        result = Core::ERROR_INVALID_INPUT_LENGTH;
                    } else {
                        result = Core::ERROR_GENERAL;
                    }
                }

                return result;
            }
            uint32_t GetNamespaceStorageLimit(const ScopeType scope, const string& ns, uint32_t& size) override
            {
                ASSERT(scope == ScopeType::DEVICE);
                if (scope != ScopeType::DEVICE) {
                    return Core::ERROR_GENERAL;
                }

                uint32_t result;

                uint32_t s;
                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(_data, "select size"
                                          " from limits"
                                          " inner join namespace on namespace.id = limits.n"
                                          " where name = ?"
                                          ";",
                    -1, &stmt, nullptr);
                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);
                auto rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW) {
                    s = (uint32_t)sqlite3_column_int(stmt, 0);
                    result = Core::ERROR_NONE;
                }
                sqlite3_finalize(stmt);

                if (rc == SQLITE_ROW) {
                    size = s;
                    result = Core::ERROR_NONE;
                } else if (rc == SQLITE_DONE) {
                    result = Core::ERROR_NOT_EXIST;
                } else {
                    OnError(__FUNCTION__, rc);
                    result = Core::ERROR_GENERAL;
                }

                return result;
            }

            BEGIN_INTERFACE_MAP(StoreLimit)
            INTERFACE_ENTRY(IStoreLimit)
            END_INTERFACE_MAP

        private:
            void OnError(const char* fn, const int status) const
            {
                TRACE(Trace::Error, (_T("%s sqlite error %d"), fn, status));
            }

        private:
            const string _path;
            const uint64_t _maxSize;
            const uint64_t _maxValue;
            sqlite3* _data;
        };

    } // namespace Sqlite
} // namespace Plugin
} // namespace WPEFramework
