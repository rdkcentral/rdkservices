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
        class StoreLimitType : public Exchange::IStoreLimit, protected ACTUALHANDLE {
        private:
            StoreLimitType(const StoreLimitType<ACTUALHANDLE>&) = delete;
            StoreLimitType<ACTUALHANDLE>& operator=(const StoreLimitType<ACTUALHANDLE>&) = delete;

        public:
            template <typename... Args>
            StoreLimitType(Args&&... args)
                : Exchange::IStoreLimit()
                , ACTUALHANDLE(std::forward<Args>(args)...)
            {
            }
            ~StoreLimitType() override = default;

        public:
            // IStoreLimit methods

            uint32_t SetNamespaceStorageLimit(const ScopeType scope, const string& ns, const uint32_t size) override
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
                    sqlite3_prepare_v2(*this, "INSERT INTO limits (n,size)"
                                              " SELECT id, ?"
                                              " FROM namespace"
                                              " WHERE name = ?"
                                              ";",
                        -1, &stmt, nullptr);

                    sqlite3_bind_int(stmt, 1, size);
                    sqlite3_bind_text(stmt, 2, ns.c_str(), -1, SQLITE_TRANSIENT);

                    rc = sqlite3_step(stmt);
                    sqlite3_finalize(stmt);

                    if (rc == SQLITE_DONE) {
                        result = Core::ERROR_NONE;
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
            uint32_t GetNamespaceStorageLimit(const ScopeType scope, const string& ns, uint32_t& size) override
            {
                ASSERT(scope == ScopeType::DEVICE);

                uint32_t result;

                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(*this, "SELECT size"
                                          " FROM limits"
                                          " INNER JOIN namespace ON namespace.id = limits.n"
                                          " where name = ?"
                                          ";",
                    -1, &stmt, nullptr);

                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

                auto rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW) {
                    size = (uint32_t)sqlite3_column_int(stmt, 0);
                    result = Core::ERROR_NONE;
                } else if (rc == SQLITE_DONE) {
                    result = Core::ERROR_NOT_EXIST;
                } else {
                    TRACE(Trace::Error, (_T("Sqlite select limit error %d"), rc));
                    result = Core::ERROR_GENERAL;
                }

                sqlite3_finalize(stmt);

                return result;
            }

            BEGIN_INTERFACE_MAP(StoreLimitType)
            INTERFACE_ENTRY(Exchange::IStoreLimit)
            END_INTERFACE_MAP
        };

    } // namespace Sqlite
} // namespace Plugin
} // namespace WPEFramework
