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

#include "SqliteDb.h"

#include <sqlite3.h>

namespace WPEFramework {
namespace Plugin {

    uint32_t SqliteDb::Open()
    {
        Core::SafeSyncType<Core::CriticalSection> lock(_dataLock);

        uint32_t result;

        if (_data && IsValid()) {
            // Seems open!
            result = Core::ERROR_ALREADY_CONNECTED;
        } else {
            Close();

            Core::File file(_path);
            Core::Directory(file.PathName().c_str()).CreatePath();

            auto rc = sqlite3_open(_path.c_str(), &_data);
            if (rc == SQLITE_OK) {
                result = CreateSchema();
            } else {
                TRACE(Trace::Error, (_T("sqlite3_open returned %d"), rc));
                result = Core::ERROR_GENERAL;
            }
        }

        return result;
    }

    uint32_t SqliteDb::Close()
    {
        uint32_t result;

        if (!_data) {
            // Seems closed!
            result = Core::ERROR_ALREADY_RELEASED;
        } else {
            auto rc = sqlite3_db_cacheflush(_data);
            if (rc != SQLITE_OK) {
                TRACE(Trace::Error, (_T("sqlite3_db_cacheflush returned %d"), rc));
            }

            rc = sqlite3_close_v2(_data);
            if (rc == SQLITE_OK) {
                result = Core::ERROR_NONE;
            } else {
                TRACE(Trace::Error, (_T("sqlite3_close_v2 returned %d"), rc));
                result = Core::ERROR_GENERAL;
            }
        }

        return result;
    }

    uint32_t SqliteDb::CreateSchema()
    {
        uint32_t result;

        char namespacetrigger[1000];
        snprintf(namespacetrigger, 1000,
            "CREATE TEMPORARY TRIGGER namespacetrigger INSERT ON namespace"
            " BEGIN"
            " SELECT CASE"
            " WHEN length(new.name) = 0"
            " THEN RAISE (FAIL, 'name empty')"
            " WHEN length(new.name) > %" PRIu32 ""
            " THEN RAISE (FAIL, 'name too large')"
            " WHEN (SELECT sum(s) FROM (SELECT sum(length(key)+length(value)) s FROM item"
            " UNION ALL"
            " SELECT sum(length(name)) s FROM namespace"
            " UNION ALL"
            " SELECT length(new.name) s)) > %" PRIu32 ""
            " THEN RAISE (FAIL, 'db full')"
            " END;"
            "END;",
            _maxValue, _maxSize);

        char itemtrigger[1000];
        snprintf(itemtrigger, 1000,
            "CREATE TEMPORARY TRIGGER itemtrigger INSERT ON item"
            " BEGIN"
            " SELECT CASE"
            " WHEN length(new.key) = 0"
            " THEN RAISE (FAIL, 'key empty')"
            " WHEN length(new.key) > %" PRIu32 ""
            " THEN RAISE (FAIL, 'key too large')"
            " WHEN length(new.value) > %" PRIu32 ""
            " THEN RAISE (FAIL, 'value too large')"
            " WHEN (SELECT sum(s) FROM (SELECT sum(length(key)+length(value)) s FROM item"
            " UNION ALL"
            " SELECT sum(length(name)) s FROM namespace"
            " UNION ALL"
            " SELECT length(new.key)+length(new.value) s)) > %" PRIu32 ""
            " THEN RAISE (FAIL, 'db full')"
            " WHEN (SELECT size-length(new.key)-length(new.value)-sum(length(key)+length(value)) FROM limits"
            " INNER JOIN item ON limits.n = item.ns"
            " where n = new.ns) < 0"
            " THEN RAISE (FAIL, 'limit')"
            " END;"
            "END;",
            _maxValue, _maxValue, _maxSize);

        std::vector<string> statements = {
            "CREATE TABLE if not exists namespace ("
            "id INTEGER PRIMARY KEY,"
            "name TEXT UNIQUE"
            ");",
            "CREATE TABLE if not exists item ("
            "ns INTEGER,"
            "key TEXT,"
            "value TEXT,"
            "FOREIGN KEY(ns) REFERENCES namespace(id) ON DELETE CASCADE ON UPDATE NO ACTION,"
            "UNIQUE(ns,key) ON CONFLICT REPLACE"
            ");",
            "CREATE TABLE if not exists limits ("
            "n INTEGER,"
            "size INTEGER,"
            "FOREIGN KEY(n) REFERENCES namespace(id) ON DELETE CASCADE ON UPDATE NO ACTION,"
            "UNIQUE(n) ON CONFLICT REPLACE"
            ");",
            "PRAGMA foreign_keys = ON;",
            namespacetrigger,
            itemtrigger
        };

        for (auto& sql : statements) {
            char* errmsg = nullptr;
            auto rc = sqlite3_exec(_data, sql.c_str(), 0, 0, &errmsg);

            if ((rc == SQLITE_OK) && (errmsg == nullptr)) {
                result = Core::ERROR_NONE;
            } else {
                TRACE(Trace::Error, (_T("sqlite3_exec returned %d"), rc));
                if (errmsg) {
                    TRACE(Trace::Error, (_T("sqlite3_exec returned %s"), errmsg));
                    sqlite3_free(errmsg);
                }
                if (rc == SQLITE_NOTADB) {
                    result = Core::ERROR_UNAVAILABLE;
                } else {
                    result = Core::ERROR_GENERAL;
                }
                break;
            }
        }

        if (result == Core::ERROR_NONE) {
            auto rc = sqlite3_exec(_data, "ALTER TABLE item ADD COLUMN ttl INTEGER;", 0, 0, 0);
            if (rc == SQLITE_NOTADB) {
                result = Core::ERROR_UNAVAILABLE;
            }
        }

        return result;
    }

    uint32_t SqliteDb::Vacuum()
    {
        uint32_t result;

        char* errmsg = nullptr;
        auto rc = sqlite3_exec(_data, "VACUUM", 0, 0, &errmsg);

        if ((rc == SQLITE_OK) && (errmsg == nullptr)) {
            result = Core::ERROR_NONE;
        } else {
            TRACE(Trace::Error, (_T("sqlite3_exec returned %d"), rc));
            if (errmsg) {
                TRACE(Trace::Error, (_T("sqlite3_exec returned %s"), errmsg));
                sqlite3_free(errmsg);
            }
            result = Core::ERROR_GENERAL;
        }

        return result;
    }

} // namespace Plugin
} // namespace WPEFramework
