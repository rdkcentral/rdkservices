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

#include "SqliteStoreInspector.h"

#include <sqlite3.h>

// Error if someone deleted the file, occurs for write operations
#define SQLITE_IS_ERROR_DBWRITE(rc) (rc == SQLITE_READONLY || rc == SQLITE_CORRUPT)

namespace WPEFramework {
namespace Plugin {

    uint32_t SqliteStoreInspector::SetNamespaceLimit(const string& ns, const uint32_t size, const ScopeType scope)
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
            sqlite3_prepare_v2(*_data, "INSERT INTO limits (n,size)"
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
                TRACE(Trace::Error, (_T("sqlite3_step returned %d"), rc));
                if (SQLITE_IS_ERROR_DBWRITE(rc)) {
                    result = Core::ERROR_UNAVAILABLE;
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

    uint32_t SqliteStoreInspector::GetNamespaceLimit(const string& ns, const ScopeType scope, uint32_t& size)
    {
        ASSERT(scope == ScopeType::DEVICE);

        uint32_t result;

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(*_data, "SELECT size"
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

    uint32_t SqliteStoreInspector::GetKeys(const string& ns, const ScopeType scope, RPC::IStringIterator*& keys)
    {
        ASSERT(scope == ScopeType::DEVICE);

        uint32_t result;

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(*_data, "SELECT key"
                                   " FROM item"
                                   " where ns in (select id from namespace where name = ?)"
                                   ";",
            -1, &stmt, NULL);

        sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

        std::list<string> list;
        do {
            auto rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                list.emplace_back((const char*)sqlite3_column_text(stmt, 0));
            } else {
                if (rc == SQLITE_DONE) {
                    keys = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(list));
                    result = Core::ERROR_NONE;
                } else {
                    TRACE(Trace::Error, (_T("sqlite3_step returned %d"), rc));
                    result = Core::ERROR_GENERAL;
                }
                break;
            }
        } while (true);

        sqlite3_finalize(stmt);

        return result;
    }

    uint32_t SqliteStoreInspector::GetNamespaces(const ScopeType scope, RPC::IStringIterator*& namespaces)
    {
        ASSERT(scope == ScopeType::DEVICE);

        uint32_t result;

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(*_data, "SELECT name FROM namespace;", -1, &stmt, NULL);

        std::list<string> list;
        do {
            auto rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                list.emplace_back((const char*)sqlite3_column_text(stmt, 0));
            } else {
                if (rc == SQLITE_DONE) {
                    namespaces = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(list));
                    result = Core::ERROR_NONE;
                } else {
                    TRACE(Trace::Error, (_T("sqlite3_step returned %d"), rc));
                    result = Core::ERROR_GENERAL;
                }
                break;
            }
        } while (true);

        sqlite3_finalize(stmt);

        return result;
    }

    uint32_t SqliteStoreInspector::GetNamespaceSizes(const ScopeType scope, INamespaceSizeIterator*& namespaceSizes)
    {
        ASSERT(scope == ScopeType::DEVICE);

        uint32_t result;

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(*_data, "SELECT name, sum(length(key)+length(value))"
                                   " FROM item"
                                   " INNER JOIN namespace ON namespace.id = item.ns"
                                   " GROUP BY name"
                                   ";",
            -1, &stmt, NULL);

        std::list<NamespaceSize> list;
        do {
            auto rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                NamespaceSize namespaceSize;
                namespaceSize.ns = (const char*)sqlite3_column_text(stmt, 0);
                namespaceSize.size = sqlite3_column_int(stmt, 1);
                list.emplace_back(namespaceSize);
            } else {
                if (rc == SQLITE_DONE) {
                    namespaceSizes = (Core::Service<NamespaceSizeIterator>::Create<INamespaceSizeIterator>(list));
                    result = Core::ERROR_NONE;
                } else {
                    TRACE(Trace::Error, (_T("sqlite3_step returned %d"), rc));
                    result = Core::ERROR_GENERAL;
                }
                break;
            }
        } while (true);

        sqlite3_finalize(stmt);

        return result;
    }

} // namespace Plugin
} // namespace WPEFramework
