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
#include <interfaces/IStoreCache.h>
#include <sqlite3.h>
#ifdef WITH_SYSMGR
#include <libIBus.h>
#include <sysMgr.h>
#endif

namespace WPEFramework {
namespace Plugin {
    namespace Sqlite {

        class Store2 : public Exchange::IStore2,
                       public Exchange::IStoreCache,
                       public Exchange::IStoreInspector,
                       public Exchange::IStoreLimit {
        private:
            class Job : public Core::IDispatch {
            public:
                Job(Store2* parent, const IStore2::ScopeType scope, const string& ns, const string& key, const string& value)
                    : _parent(parent)
                    , _scope(scope)
                    , _ns(ns)
                    , _key(key)
                    , _value(value)
                {
                    _parent->AddRef();
                }
                ~Job() override
                {
                    _parent->Release();
                }
                void Dispatch() override
                {
                    _parent->OnValueChanged(_scope, _ns, _key, _value);
                }

            private:
                Store2* _parent;
                const IStore2::ScopeType _scope;
                const string _ns;
                const string _key;
                const string _value;
            };

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
                , IStoreCache()
                , IStoreInspector()
                , IStoreLimit()
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
                rc = sqlite3_busy_timeout(_data, SQLITE_TIMEOUT); // Timeout
                if (rc != SQLITE_OK) {
                    OnError(__FUNCTION__, rc);
                }
                const std::vector<string> statements = {
                    "pragma foreign_keys = on;",
                    "create table if not exists namespace"
                    " (id integer primary key,name text unique);",
                    "create table if not exists item"
                    " (ns integer,key text,value text,"
                    "foreign key(ns) references namespace(id) on delete cascade on update no action,"
                    "unique(ns,key) on conflict replace);",
                    "create table if not exists limits"
                    " (n integer,size integer,"
                    "foreign key(n) references namespace(id) on delete cascade on update no action,"
                    "unique(n) on conflict replace);",
                    "alter table item add column ttl integer;",
                    "create temporary trigger if not exists ns_empty insert on namespace"
                    " begin select case when length(new.name) = 0"
                    " then raise (fail, 'empty') end; end;",
                    "create temporary trigger if not exists key_empty insert on item"
                    " begin select case when length(new.key) = 0"
                    " then raise (fail, 'empty') end; end;",
                    "create temporary trigger if not exists ns_maxvalue insert on namespace"
                    " begin select case when length(new.name) > "
                        + std::to_string(_maxValue) + " then raise (fail, 'max value') end; end;",
                    "create temporary trigger if not exists key_maxvalue insert on item"
                    " begin select case when length(new.key) > "
                        + std::to_string(_maxValue) + " then raise (fail, 'max value') end; end;",
                    "create temporary trigger if not exists value_maxvalue insert on item"
                    " begin select case when length(new.value) > "
                        + std::to_string(_maxValue) + " then raise (fail, 'max value') end; end;",
                    "create temporary trigger if not exists ns_maxsize insert on namespace"
                    " begin select case when"
                    " (select sum(s) from (select sum(length(key)+length(value)) s from item"
                    " union all select sum(length(name)) s from namespace"
                    " union all select length(new.name) s)) > "
                        + std::to_string(_maxSize) + " then raise (fail, 'max size') end; end;",
                    "create temporary trigger if not exists item_maxsize insert on item"
                    " begin select case when"
                    " (select sum(s) from (select sum(length(key)+length(value)) s from item"
                    " union all select sum(length(name)) s from namespace"
                    " union all select length(new.key)+length(new.value) s)) > "
                        + std::to_string(_maxSize) + " then raise (fail, 'max size') end; end;",
                    "create temporary trigger if not exists item_limit_default insert on item"
                    " begin select case when"
                    " (select length(new.key)+length(new.value)+sum(length(key)+length(value)) from item where ns = new.ns) > "
                        + std::to_string(_limit) + " then raise (fail, 'limit') end; end;",
                    "create temporary trigger if not exists item_limit insert on item"
                    " begin select case when"
                    " (select size-length(new.key)-length(new.value)-sum(length(key)+length(value)) from limits"
                    " inner join item on limits.n = item.ns where n = new.ns) < 0"
                    " then raise (fail, 'limit') end; end;"
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
            bool IsTimeSynced() const
            {
#ifdef WITH_SYSMGR
                // Get actual state, as it may change at any time...
                IARM_Bus_Init(IARM_INIT_NAME);
                IARM_Bus_Connect();
                IARM_Bus_SYSMgr_GetSystemStates_Param_t param;
                if ((IARM_Bus_Call_with_IPCTimeout(
                         IARM_BUS_SYSMGR_NAME,
                         IARM_BUS_SYSMGR_API_GetSystemStates,
                         &param,
                         sizeof(param),
                         IARM_TIMEOUT) // Timeout
                        != IARM_RESULT_SUCCESS)
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
            uint32_t SetValue(const IStore2::ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl) override
            {
                ASSERT(scope == IStore2::ScopeType::DEVICE);

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
                    Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(
                        Core::ProxyType<Job>::Create(this, scope, ns, key, value))); // Decouple notification

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
            uint32_t GetValue(const IStore2::ScopeType scope, const string& ns, const string& key, string& value, uint32_t& ttl) override
            {
                ASSERT(scope == IStore2::ScopeType::DEVICE);

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
            uint32_t DeleteKey(const IStore2::ScopeType scope, const string& ns, const string& key) override
            {
                ASSERT(scope == IStore2::ScopeType::DEVICE);

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
            uint32_t DeleteNamespace(const IStore2::ScopeType scope, const string& ns) override
            {
                ASSERT(scope == IStore2::ScopeType::DEVICE);

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
            uint32_t GetKeys(const IStoreInspector::ScopeType scope, const string& ns, RPC::IStringIterator*& keys) override
            {
                ASSERT(scope == IStoreInspector::ScopeType::DEVICE);

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
            uint32_t GetNamespaces(const IStoreInspector::ScopeType scope, RPC::IStringIterator*& namespaces) override
            {
                ASSERT(scope == IStoreInspector::ScopeType::DEVICE);

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
            uint32_t GetStorageSizes(const IStoreInspector::ScopeType scope, INamespaceSizeIterator*& storageList) override
            {
                ASSERT(scope == IStoreInspector::ScopeType::DEVICE);

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
                    storageList = (Core::Service<RPC::IteratorType<INamespaceSizeIterator>>::Create<INamespaceSizeIterator>(list));
                    result = Core::ERROR_NONE;
                } else {
                    OnError(__FUNCTION__, rc);
                    result = Core::ERROR_GENERAL;
                }

                return result;
            }
            uint32_t SetNamespaceStorageLimit(const IStoreLimit::ScopeType scope, const string& ns, const uint32_t size) override
            {
                ASSERT(scope == IStoreLimit::ScopeType::DEVICE);

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
            uint32_t GetNamespaceStorageLimit(const IStoreLimit::ScopeType scope, const string& ns, uint32_t& size) override
            {
                ASSERT(scope == IStoreLimit::ScopeType::DEVICE);

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

            BEGIN_INTERFACE_MAP(Store2)
            INTERFACE_ENTRY(IStore2)
            INTERFACE_ENTRY(IStoreCache)
            INTERFACE_ENTRY(IStoreInspector)
            INTERFACE_ENTRY(IStoreLimit)
            END_INTERFACE_MAP

        private:
            void OnValueChanged(const IStore2::ScopeType scope, const string& ns, const string& key, const string& value)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

                std::list<INotification*>::iterator
                    index(_clients.begin());

                while (index != _clients.end()) {
                    // If main process is out of threads, this can time out, and IPC will mess up...
                    (*index)->ValueChanged(scope, ns, key, value);
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
