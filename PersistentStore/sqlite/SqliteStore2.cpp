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

#include "SqliteStore2.h"

#include <sqlite3.h>

// Error if someone deleted the file, occurs for write operations
#define SQLITE_IS_ERROR_DBWRITE(rc) (rc == SQLITE_READONLY || rc == SQLITE_CORRUPT)

namespace WPEFramework {
namespace Plugin {

    uint32_t SqliteStore2::SetValue(const string& ns, const string& key, const string& value, const ScopeType scope, const uint32_t ttl)
    {
        ASSERT(scope == ScopeType::DEVICE);

        uint32_t result;

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(*_data, "INSERT OR IGNORE INTO namespace (name) values (?);",
            -1, &stmt, nullptr);

        sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

        auto rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc == SQLITE_DONE) {
            sqlite3_prepare_v2(*_data, "INSERT INTO item (ns,key,value,ttl)"
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
                OnValueChanged(ns, key, value, scope);
                result = Core::ERROR_NONE;
            } else {
                TRACE(Trace::Error, (_T("sqlite3_step returned %d"), rc));
                if (SQLITE_IS_ERROR_DBWRITE(rc)) {
                    result = Core::ERROR_UNAVAILABLE;
                } else if (rc == SQLITE_CONSTRAINT) {
                    result = Core::ERROR_INVALID_INPUT_LENGTH;
                } else {
                    result = Core::ERROR_GENERAL;
                }
            }
        } else {
            TRACE(Trace::Error, (_T("sqlite3_step returned %d"), rc));
            if (SQLITE_IS_ERROR_DBWRITE(rc)) {
                result = Core::ERROR_UNAVAILABLE;
            } else if (rc == SQLITE_CONSTRAINT) {
                result = Core::ERROR_INVALID_INPUT_LENGTH;
            } else {
                result = Core::ERROR_GENERAL;
            }
        }

        return result;
    }

    uint32_t SqliteStore2::GetValue(const string& ns, const string& key, const ScopeType scope, string& value, uint32_t& ttl)
    {
        ASSERT(scope == ScopeType::DEVICE);

        uint32_t result;

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(*_data, "SELECT key, value, ttl"
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
                    auto seconds = sqlite3_column_int64(stmt, 2) - Core::Time::Now().Ticks() / Core::Time::TicksPerMillisecond / 1000;
                    if (seconds > 0) {
                        value = (const char*)sqlite3_column_text(stmt, 1);
                        ttl = seconds;
                        result = Core::ERROR_NONE;
                    } else {
                        TRACE(Trace::Error, (_T("ttl expired")));
                        result = Core::ERROR_UNKNOWN_KEY;
                    }
                } else {
                    value = (const char*)sqlite3_column_text(stmt, 1);
                    ttl = 0;
                    result = Core::ERROR_NONE;
                }
            } else {
                TRACE(Trace::Error, (_T("sqlite3_step returned SQLITE_NULL")));
                result = Core::ERROR_UNKNOWN_KEY;
            }
        } else {
            TRACE(Trace::Error, (_T("sqlite3_step returned %d"), rc));
            if (rc == SQLITE_DONE) {
                result = Core::ERROR_NOT_EXIST;
            } else {
                result = Core::ERROR_GENERAL;
            }
        }

        sqlite3_finalize(stmt);

        return result;
    }

    uint32_t SqliteStore2::DeleteKey(const string& ns, const string& key, const ScopeType scope)
    {
        ASSERT(scope == ScopeType::DEVICE);

        uint32_t result;

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(*_data, "DELETE FROM item"
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
            TRACE(Trace::Error, (_T("sqlite3_step returned %d"), rc));
            result = Core::ERROR_GENERAL;
        }

        return result;
    }

    uint32_t SqliteStore2::DeleteNamespace(const string& ns, const ScopeType scope)
    {
        ASSERT(scope == ScopeType::DEVICE);

        uint32_t result;

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(*_data, "DELETE FROM namespace where name = ?;", -1, &stmt, NULL);

        sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

        auto rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc == SQLITE_DONE) {
            result = Core::ERROR_NONE;
        } else {
            TRACE(Trace::Error, (_T("sqlite3_step returned %d"), rc));
            result = Core::ERROR_GENERAL;
        }

        return result;
    }

} // namespace Plugin
} // namespace WPEFramework
