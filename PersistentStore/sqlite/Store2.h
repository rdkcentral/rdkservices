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
#ifdef WITH_SYSMGR
#include <libIBus.h>
#include <sysMgr.h>
#endif

namespace WPEFramework {
namespace Plugin {
    namespace Sqlite {

        class Store2 : public Exchange::IStore2 {
        private:
            Store2(const Store2&) = delete;
            Store2& operator=(const Store2&) = delete;

        public:
            Store2()
                : Store2(
                      getenv(PATH_ENV),
                      std::stoul(getenv(MAXSIZE_ENV)),
                      std::stoul(getenv(MAXVALUE_ENV)),
                      std::stoul(getenv(LIMIT_ENV)))
            {
            }
            Store2(const string& path, const uint64_t maxSize, const uint64_t maxValue, const uint64_t limit)
                : IStore2()
                , _path(path)
                , _maxSize(maxSize)
                , _maxValue(maxValue)
                , _limit(limit)
            {
                Open();
            }
            ~Store2() override
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
                    "alter table item add column ttl integer;",
                    "create temporary trigger if not exists ns_empty insert on namespace begin select case when length(new.name) = 0 then raise (fail, 'empty') end; end;",
                    "create temporary trigger if not exists key_empty insert on item begin select case when length(new.key) = 0 then raise (fail, 'empty') end; end;",
                    "create temporary trigger if not exists ns_maxvalue insert on namespace begin select case when length(new.name) > " + std::to_string(_maxValue) + " then raise (fail, 'max value') end; end;",
                    "create temporary trigger if not exists key_maxvalue insert on item begin select case when length(new.key) > " + std::to_string(_maxValue) + " then raise (fail, 'max value') end; end;",
                    "create temporary trigger if not exists value_maxvalue insert on item begin select case when length(new.value) > " + std::to_string(_maxValue) + " then raise (fail, 'max value') end; end;",
                    "create temporary trigger if not exists ns_maxsize insert on namespace begin select case when (select sum(s) from (select sum(length(key)+length(value)) s from item union all select sum(length(name)) s from namespace union all select length(new.name) s)) > " + std::to_string(_maxSize) + " then raise (fail, 'max size') end; end;",
                    "create temporary trigger if not exists item_maxsize insert on item begin select case when (select sum(s) from (select sum(length(key)+length(value)) s from item union all select sum(length(name)) s from namespace union all select length(new.key)+length(new.value) s)) > " + std::to_string(_maxSize) + " then raise (fail, 'max size') end; end;",
                    "create temporary trigger if not exists item_limit_default insert on item begin select case when (select length(new.key)+length(new.value)+sum(length(key)+length(value)) from item where ns = new.ns) > " + std::to_string(_limit) + " then raise (fail, 'limit') end; end;",
                    "create temporary trigger if not exists item_limit insert on item begin select case when (select size-length(new.key)-length(new.value)-sum(length(key)+length(value)) from limits inner join item on limits.n = item.ns where n = new.ns) < 0 then raise (fail, 'limit') end; end;"
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
            static bool IsTimeSynced()
            {
#ifdef WITH_SYSMGR
                IARM_Bus_Init(IARM_INIT_NAME);
                IARM_Bus_Connect();
                IARM_Bus_SYSMgr_GetSystemStates_Param_t param;
                if ((IARM_Bus_Call(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_API_GetSystemStates, &param, sizeof(param)) != IARM_RESULT_SUCCESS)
                    || !param.time_source.state) {
                    return false;
                }
#endif
                return true;
            }

        public:
            uint32_t Register(INotification* notification) override
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

                ASSERT(std::find(_clients.begin(), _clients.end(), notification) == _clients.end());

                notification->AddRef();
                _clients.push_back(notification);

                return Core::ERROR_NONE;
            }
            uint32_t Unregister(INotification* notification) override
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

                std::list<INotification*>::iterator
                    index(std::find(_clients.begin(), _clients.end(), notification));

                ASSERT(index != _clients.end());

                if (index != _clients.end()) {
                    notification->Release();
                    _clients.erase(index);
                }

                return Core::ERROR_NONE;
            }

            uint32_t SetValue(const ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                if (ttl != 0) {
                    if (!IsTimeSynced()) {
                        return Core::ERROR_PENDING_CONDITIONS;
                    }
                }
                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(_data, "insert or ignore into namespace (name) values (?);",
                    -1, &stmt, nullptr);
                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);
                auto rc = sqlite3_step(stmt);
                sqlite3_finalize(stmt);
                if (rc == SQLITE_DONE) {
                    sqlite3_prepare_v2(_data, "insert into item (ns,key,value,ttl)"
                                              " select id, ?, ?, ?"
                                              " from namespace"
                                              " where name = ?"
                                              ";",
                        -1, &stmt, nullptr);
                    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
                    if (ttl != 0) {
                        sqlite3_bind_int64(stmt, 3, (int64_t)ttl + time(nullptr));
                    } else {
                        sqlite3_bind_null(stmt, 3);
                    }
                    sqlite3_bind_text(stmt, 4, ns.c_str(), -1, SQLITE_TRANSIENT);
                    rc = sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                }

                if (rc == SQLITE_DONE) {
                    OnValueChanged(ns, key, value);
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
            uint32_t GetValue(const ScopeType scope, const string& ns, const string& key, string& value, uint32_t& ttl) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                string k, v;
                int64_t t = 0;
                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(_data, "select key, value, ttl"
                                          " from namespace"
                                          " left join item on (namespace.id = item.ns and key = ?)"
                                          " where name = ?"
                                          ";",
                    -1, &stmt, nullptr);
                sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, ns.c_str(), -1, SQLITE_TRANSIENT);
                auto rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW) {
                    if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
                        k = (const char*)sqlite3_column_text(stmt, 0);
                        v = (const char*)sqlite3_column_text(stmt, 1);
                        t = sqlite3_column_int64(stmt, 2);
                    }
                }
                sqlite3_finalize(stmt);

                if (rc == SQLITE_ROW) {
                    if (!k.empty()) {
                        if (t == 0) {
                            value = v;
                            ttl = 0;
                            result = Core::ERROR_NONE;
                        } else if (IsTimeSynced()) {
                            t -= time(nullptr);
                            if (t > 0) {
                                value = v;
                                ttl = t;
                                result = Core::ERROR_NONE;
                            } else {
                                result = Core::ERROR_UNKNOWN_KEY;
                            }
                        } else {
                            result = Core::ERROR_PENDING_CONDITIONS;
                        }

                    } else {
                        result = Core::ERROR_UNKNOWN_KEY;
                    }
                } else if (rc == SQLITE_DONE) {
                    result = Core::ERROR_NOT_EXIST;
                } else {
                    OnError(__FUNCTION__, rc);
                    result = Core::ERROR_GENERAL;
                }

                return result;
            }
            uint32_t DeleteKey(const ScopeType scope, const string& ns, const string& key) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(_data, "delete from item"
                                          " where ns in (select id from namespace where name = ?)"
                                          " and key = ?"
                                          ";",
                    -1, &stmt, nullptr);
                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_TRANSIENT);
                auto rc = sqlite3_step(stmt);
                sqlite3_finalize(stmt);

                if (rc == SQLITE_DONE) {
                    result = Core::ERROR_NONE;
                } else {
                    OnError(__FUNCTION__, rc);
                    result = Core::ERROR_GENERAL;
                }

                return result;
            }
            uint32_t DeleteNamespace(const ScopeType scope, const string& ns) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(_data, "delete from namespace where name = ?;", -1, &stmt, nullptr);
                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);
                auto rc = sqlite3_step(stmt);
                sqlite3_finalize(stmt);

                if (rc == SQLITE_DONE) {
                    result = Core::ERROR_NONE;
                } else {
                    OnError(__FUNCTION__, rc);
                    result = Core::ERROR_GENERAL;
                }

                return result;
            }

            BEGIN_INTERFACE_MAP(Store2)
            INTERFACE_ENTRY(IStore2)
            END_INTERFACE_MAP

        private:
            void OnValueChanged(const string& ns, const string& key, const string& value)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

                std::list<INotification*>::iterator
                    index(_clients.begin());

                while (index != _clients.end()) {
                    (*index)->ValueChanged(ScopeType::DEVICE, ns, key, value);
                    index++;
                }
            }
            void OnError(const char* fn, const int status) const
            {
                TRACE(Trace::Error, (_T("%s sqlite error %d"), fn, status));
            }

        private:
            const string _path;
            const uint64_t _maxSize;
            const uint64_t _maxValue;
            const uint64_t _limit;
            sqlite3* _data;
            std::list<INotification*> _clients;
            Core::CriticalSection _clientLock;
        };

    } // namespace Sqlite
} // namespace Plugin
} // namespace WPEFramework
