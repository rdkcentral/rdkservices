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

#include "Handle.h"

namespace WPEFramework {
namespace Plugin {
    namespace Sqlite {

        uint32_t Handle::Open()
        {
            Core::SafeSyncType<Core::CriticalSection> lock(_dataLock);

            Close();

            uint32_t result;

            string path;
            Core::SystemInfo::GetEnvironment(_T("PERSISTENTSTORE_PATH"), path);

            Core::File file(path);
            Core::Directory(file.PathName().c_str()).CreatePath();

            auto rc = sqlite3_open(path.c_str(), &_data);
            if (rc == SQLITE_OK) {
                result = CreateSchema();
            } else {
                TRACE(Trace::Error, (_T("Sqlite open error %d for path %s"), rc, path.c_str()));
                result = Core::ERROR_GENERAL;
            }

            return result;
        }

        uint32_t Handle::Close()
        {
            Core::SafeSyncType<Core::CriticalSection> lock(_dataLock);

            uint32_t result;

            if (!_data) {
                // Seems closed!
                result = Core::ERROR_NONE;
            } else {
                auto rc = sqlite3_db_cacheflush(_data);
                if (rc != SQLITE_OK) {
                    TRACE(Trace::Error, (_T("Sqlite cache flush error %d"), rc));
                }

                rc = sqlite3_close_v2(_data);
                if (rc == SQLITE_OK) {
                    result = Core::ERROR_NONE;
                } else {
                    TRACE(Trace::Error, (_T("Sqlite close error %d"), rc));
                    result = Core::ERROR_GENERAL;
                }
            }

            return result;
        }

        uint32_t Handle::CreateSchema()
        {
            Core::SafeSyncType<Core::CriticalSection> lock(_dataLock);

            uint32_t result;

            string maxValue, maxSize;
            Core::SystemInfo::GetEnvironment(_T("PERSISTENTSTORE_MAXVALUE"), maxValue);
            Core::SystemInfo::GetEnvironment(_T("PERSISTENTSTORE_MAXSIZE"), maxSize);

            // This may be called for an existing database too, note the "if not exists"

            const std::vector<string> statements = {
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
                "PRAGMA busy_timeout = 1000000;",
                "CREATE TEMPORARY TRIGGER if not exists namespacetrigger INSERT ON namespace"
                " BEGIN"
                " SELECT CASE"
                " WHEN length(new.name) = 0"
                " THEN RAISE (FAIL, 'name empty')"
                " WHEN length(new.name) > "
                    + maxValue + " THEN RAISE (FAIL, 'name too large')"
                                 " WHEN (SELECT sum(s) FROM (SELECT sum(length(key)+length(value)) s FROM item"
                                 " UNION ALL"
                                 " SELECT sum(length(name)) s FROM namespace"
                                 " UNION ALL"
                                 " SELECT length(new.name) s)) > "
                    + maxSize + " THEN RAISE (FAIL, 'db full')"
                                " END;"
                                "END;",
                "CREATE TEMPORARY TRIGGER if not exists itemtrigger INSERT ON item"
                " BEGIN"
                " SELECT CASE"
                " WHEN length(new.key) = 0"
                " THEN RAISE (FAIL, 'key empty')"
                " WHEN length(new.key) > "
                    + maxValue + " THEN RAISE (FAIL, 'key too large')"
                                 " WHEN length(new.value) > "
                    + maxValue + " THEN RAISE (FAIL, 'value too large')"
                                 " WHEN (SELECT sum(s) FROM (SELECT sum(length(key)+length(value)) s FROM item"
                                 " UNION ALL"
                                 " SELECT sum(length(name)) s FROM namespace"
                                 " UNION ALL"
                                 " SELECT length(new.key)+length(new.value) s)) > "
                    + maxSize + " THEN RAISE (FAIL, 'db full')"
                                " WHEN (SELECT size-length(new.key)-length(new.value)-sum(length(key)+length(value)) FROM limits"
                                " INNER JOIN item ON limits.n = item.ns"
                                " where n = new.ns) < 0"
                                " THEN RAISE (FAIL, 'limit')"
                                " END;"
                                "END;"
            };

            for (auto& sql : statements) {
                char* errmsg = nullptr;
                auto rc = sqlite3_exec(_data, sql.c_str(), 0, 0, &errmsg);

                if ((rc == SQLITE_OK) && (errmsg == nullptr)) {
                    result = Core::ERROR_NONE;
                } else {
                    TRACE(Trace::Error, (_T("Sqlite '%s' error %d"), sql.c_str(), rc));
                    if (errmsg) {
                        TRACE(Trace::Error, (_T("Sqlite error '%s'"), errmsg));
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
                    TRACE(Trace::Error, (_T("Sqlite alter table error %d"), rc));
                    result = Core::ERROR_UNAVAILABLE;
                } else {
                    // If the column exists, we are not interested in this error
                }
            }

            return result;
        }

    } // namespace Sqlite
} // namespace Plugin
} // namespace WPEFramework
