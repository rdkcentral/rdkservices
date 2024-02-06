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

#include <interfaces/IStore2.h>
#include <sqlite3.h>

namespace WPEFramework {
namespace Plugin {
    namespace Sqlite {

        template <typename ACTUALHANDLE>
        class Store2Type : public Exchange::IStore2, protected ACTUALHANDLE {
        private:
            Store2Type(const Store2Type<ACTUALHANDLE>&) = delete;
            Store2Type<ACTUALHANDLE>& operator=(const Store2Type<ACTUALHANDLE>&) = delete;

        public:
            template <typename... Args>
            Store2Type(Args&&... args)
                : Exchange::IStore2()
                , ACTUALHANDLE(std::forward<Args>(args)...)
                , _clients()
                , _clientLock()
            {
            }
            ~Store2Type() override = default;

        public:
            // IStore2 methods

            uint32_t Register(Exchange::IStore2::INotification* notification) override
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

                ASSERT(std::find(_clients.begin(), _clients.end(), notification) == _clients.end());

                notification->AddRef();
                _clients.push_back(notification);

                return Core::ERROR_NONE;
            }
            uint32_t Unregister(Exchange::IStore2::INotification* notification) override
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

            uint32_t SetValue(const ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(*this, "INSERT OR IGNORE INTO namespace (name) values (?);",
                    -1, &stmt, nullptr);

                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

                auto rc = sqlite3_step(stmt);
                sqlite3_finalize(stmt);

                if (rc == SQLITE_DONE) {
                    sqlite3_prepare_v2(*this, "INSERT OR IGNORE INTO limits (n,size)"
                                              " SELECT id, ?"
                                              " FROM namespace"
                                              " WHERE name = ?"
                                              ";",
                        -1, &stmt, nullptr);

                    string limit;
                    Core::SystemInfo::GetEnvironment(_T("PERSISTENTSTORE_LIMIT"), limit);

                    sqlite3_bind_int(stmt, 1, std::stoul(limit));
                    sqlite3_bind_text(stmt, 2, ns.c_str(), -1, SQLITE_TRANSIENT);

                    rc = sqlite3_step(stmt);
                    sqlite3_finalize(stmt);

                    if (rc == SQLITE_DONE) {
                        sqlite3_prepare_v2(*this, "INSERT INTO item (ns,key,value,ttl)"
                                                  " SELECT id, ?, ?, ?"
                                                  " FROM namespace"
                                                  " WHERE name = ?"
                                                  ";",
                            -1, &stmt, nullptr);

                        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
                        sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
                        if (ttl != 0) {
                            sqlite3_bind_int64(stmt, 3, Core::Time::Now().Ticks() / Core::Time::TicksPerMillisecond / 1000 + ttl);
                        } else {
                            sqlite3_bind_null(stmt, 3);
                        }
                        sqlite3_bind_text(stmt, 4, ns.c_str(), -1, SQLITE_TRANSIENT);

                        rc = sqlite3_step(stmt);
                        sqlite3_finalize(stmt);

                        if (rc == SQLITE_DONE) {
                            OnValueChanged(ns, key, value);
                            result = Core::ERROR_NONE;
                        } else if (rc == SQLITE_CONSTRAINT) {
                            result = Core::ERROR_INVALID_INPUT_LENGTH;
                        } else {
                            TRACE(Trace::Error, (_T("Sqlite insert item error %d"), rc));
                            if (rc == SQLITE_READONLY || rc == SQLITE_CORRUPT) {
                                result = Core::ERROR_UNAVAILABLE;
                            } else {
                                result = Core::ERROR_GENERAL;
                            }
                        }
                    } else {
                        TRACE(Trace::Error, (_T("Sqlite insert limit error %d"), rc));
                        if (rc == SQLITE_READONLY || rc == SQLITE_CORRUPT) {
                            result = Core::ERROR_UNAVAILABLE;
                        } else {
                            result = Core::ERROR_GENERAL;
                        }
                    }
                } else if (rc == SQLITE_CONSTRAINT) {
                    result = Core::ERROR_INVALID_INPUT_LENGTH;
                } else {
                    TRACE(Trace::Error, (_T("Sqlite insert namespace error %d"), rc));
                    if (rc == SQLITE_READONLY || rc == SQLITE_CORRUPT) {
                        result = Core::ERROR_UNAVAILABLE;
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

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(*this, "SELECT key, value, ttl"
                                          " FROM namespace"
                                          " LEFT JOIN item ON (namespace.id = item.ns AND key = ?)"
                                          " where name = ?"
                                          ";",
                    -1, &stmt, nullptr);

                sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, ns.c_str(), -1, SQLITE_TRANSIENT);

                auto rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW) {
                    if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
                        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
                            auto seconds = (int64_t)sqlite3_column_int64(stmt, 2) - (int64_t)(Core::Time::Now().Ticks() / Core::Time::TicksPerMillisecond / 1000);
                            if (seconds > 0) {
                                value = (const char*)sqlite3_column_text(stmt, 1);
                                ttl = seconds;
                                result = Core::ERROR_NONE;
                            } else {
                                result = Core::ERROR_UNKNOWN_KEY;
                            }
                        } else {
                            value = (const char*)sqlite3_column_text(stmt, 1);
                            ttl = 0;
                            result = Core::ERROR_NONE;
                        }
                    } else {
                        result = Core::ERROR_UNKNOWN_KEY;
                    }
                } else if (rc == SQLITE_DONE) {
                    result = Core::ERROR_NOT_EXIST;
                } else {
                    TRACE(Trace::Error, (_T("Sqlite select item error %d"), rc));
                    result = Core::ERROR_GENERAL;
                }

                sqlite3_finalize(stmt);

                return result;
            }
            uint32_t DeleteKey(const ScopeType scope, const string& ns, const string& key) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(*this, "DELETE FROM item"
                                          " where ns in (select id from namespace where name = ?)"
                                          " and key = ?"
                                          ";",
                    -1, &stmt, NULL);

                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_TRANSIENT);

                auto rc = sqlite3_step(stmt);
                sqlite3_finalize(stmt);

                if (rc == SQLITE_DONE) {
                    result = Core::ERROR_NONE;
                } else {
                    TRACE(Trace::Error, (_T("Sqlite delete item error %d"), rc));
                    result = Core::ERROR_GENERAL;
                }

                return result;
            }
            uint32_t DeleteNamespace(const ScopeType scope, const string& ns) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(*this, "DELETE FROM namespace where name = ?;", -1, &stmt, NULL);

                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

                auto rc = sqlite3_step(stmt);
                sqlite3_finalize(stmt);

                if (rc == SQLITE_DONE) {
                    result = Core::ERROR_NONE;
                } else {
                    TRACE(Trace::Error, (_T("Sqlite delete namespace error %d"), rc));
                    result = Core::ERROR_GENERAL;
                }

                return result;
            }

            BEGIN_INTERFACE_MAP(Store2Type)
            INTERFACE_ENTRY(Exchange::IStore2)
            END_INTERFACE_MAP

        private:
            void OnValueChanged(const string& ns, const string& key, const string& value)
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

                std::list<Exchange::IStore2::INotification*>::iterator
                    index(_clients.begin());

                while (index != _clients.end()) {
                    (*index)->ValueChanged(ScopeType::DEVICE, ns, key, value);
                    index++;
                }
            }

        private:
            std::list<Exchange::IStore2::INotification*> _clients;
            Core::CriticalSection _clientLock;
        };

    } // namespace Sqlite
} // namespace Plugin
} // namespace WPEFramework
