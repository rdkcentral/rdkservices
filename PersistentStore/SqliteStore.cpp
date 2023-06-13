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

#include "SqliteStore.h"

#include "UtilsLogging.h"

#include <sqlite3.h>

#if defined(USE_PLABELS)
#include "pbnj_utils.hpp"
#include <glib.h>
#endif

#ifndef SQLITE_FILE_HEADER
#define SQLITE_FILE_HEADER "SQLite format 3"
#endif

#define SQLITE *(sqlite3**)&_data
#define SQLITE_IS_ERROR_DBWRITE(rc) (rc == SQLITE_READONLY || rc == SQLITE_CORRUPT)

namespace {
#if defined(USE_PLABELS)
bool GenerateKey(const char *key, std::vector<uint8_t> &pKey)
{
    // NOTE: pbnj_utils stores the nonce under XDG_DATA_HOME/data.
    // If the dir doesn't exist it will fail

    auto path = g_build_filename(g_get_user_data_dir(), "data", nullptr);
    if (!Core::File(string(path)).Exists())
        g_mkdir_with_parents(path, 0755);
    g_free(path);

    return pbnj_utils::prepareBufferForOrigin(key, [&pKey](const std::vector<uint8_t> &buffer)
    {
        pKey = buffer;
    });
}
#endif
}

namespace WPEFramework {
namespace Plugin {

SqliteStore::SqliteStore()
    : _data(nullptr),
      _path(),
      _key(),
      _maxSize(0),
      _maxValue(0),
      _clients(),
      _clientLock(),
      _lock()
{
}

uint32_t SqliteStore::Register(Exchange::IStore::INotification *notification)
{
    Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

    ASSERT(std::find(_clients.begin(), _clients.end(), notification) == _clients.end());

    notification->AddRef();
    _clients.push_back(notification);

    return Core::ERROR_NONE;
}

uint32_t SqliteStore::Unregister(Exchange::IStore::INotification *notification)
{
    Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

    std::list<Exchange::IStore::INotification *>::iterator
        index(std::find(_clients.begin(), _clients.end(), notification));

    ASSERT(index != _clients.end());

    if (index != _clients.end()) {
        notification->Release();
        _clients.erase(index);
    }

    return Core::ERROR_NONE;
}

uint32_t SqliteStore::Open(const string &path, const string &key, uint32_t maxSize, uint32_t maxValue)
{
    CountingLockSync lock(_lock, 0);

    if (IsOpen()) {
        // Seems open!

        return Core::ERROR_ILLEGAL_STATE;
    }

    Close();

    _path = path;
    _key = key;
    _maxSize = maxSize;
    _maxValue = maxValue;

#if defined(SQLITE_HAS_CODEC)
    bool shouldEncrypt = !key.empty();
    bool shouldReKey = shouldEncrypt && IsValid() && !IsEncrypted();

    std::vector<uint8_t> pKey;
    if (shouldEncrypt) {
#if defined(USE_PLABELS)
        if (!GenerateKey(key.c_str(), pKey)) {
            LOGERR("pbnj_utils fail");

            Close();

            return Core::ERROR_GENERAL;
        }
#else
        LOGWARN("Key is not secure");
        pKey = std::vector<uint8_t>(key.begin(), key.end());
#endif
    }
#endif

    sqlite3* &db = SQLITE;

    int rc = sqlite3_open(path.c_str(), &db);
    if (rc != SQLITE_OK) {
        LOGERR("%d : %s", rc, sqlite3_errstr(rc));

        Close();

        return Core::ERROR_OPENING_FAILED;
    }

#if defined(SQLITE_HAS_CODEC)
    if (shouldEncrypt) {
        rc = Encrypt(pKey);
        if (rc != SQLITE_OK) {
            LOGERR("Failed to attach key - %s", sqlite3_errstr(rc));

            Close();

            return Core::ERROR_GENERAL;
        }
    }
#endif

    rc = CreateTables();

#if defined(SQLITE_HAS_CODEC)
    if (rc == SQLITE_NOTADB
        && shouldEncrypt
        && !shouldReKey // re-key should never fail
        ) {
        LOGWARN("The key doesn't work");

        Close();

        if (!Core::File(path).Destroy() || IsValid()) {
            LOGERR("Can't remove file");

            return Core::ERROR_DESTRUCTION_FAILED;
        }

        sqlite3* &db = SQLITE;

        rc = sqlite3_open(path.c_str(), &db);
        if (rc != SQLITE_OK) {
            LOGERR("Can't create file");

            return Core::ERROR_OPENING_FAILED;
        }

        LOGWARN("SQLite database has been reset, trying re-key");

        rc = Encrypt(pKey);
        if (rc != SQLITE_OK) {
            LOGERR("Failed to attach key - %s", sqlite3_errstr(rc));

            Close();

            return Core::ERROR_GENERAL;
        }

        rc = CreateTables();
    }
#endif

    return Core::ERROR_NONE;
}

uint32_t SqliteStore::SetValue(const string &ns, const string &key, const string &value)
{
    if (ns.size() > _maxValue ||
        key.size() > _maxValue ||
        value.size() > _maxValue) {
        return Core::ERROR_INVALID_INPUT_LENGTH;
    }

    uint32_t result = Core::ERROR_GENERAL;

    int retry = 0;
    int rc;
    do {
        CountingLockSync lock(_lock);

        sqlite3* &db = SQLITE;

        if (!db)
            break;

        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "SELECT sum(s) FROM ("
                               " SELECT sum(length(key)+length(value)) s FROM item"
                               " UNION ALL"
                               " SELECT sum(length(name)) s FROM namespace"
                               ");", -1, &stmt, nullptr);

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            int64_t size = sqlite3_column_int64(stmt, 0);
            if (size > _maxSize) {
                LOGWARN("max size exceeded: %" PRId64 "\n", size);

                result = Core::ERROR_WRITE_ERROR;
            } else {
                result = Core::ERROR_NONE;
            }
        }
        else
            LOGERR("ERROR getting size: %s", sqlite3_errstr(rc));

        sqlite3_finalize(stmt);

        if (result == Core::ERROR_NONE) {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO namespace (name) values (?);", -1, &stmt, nullptr);

            sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                LOGERR("ERROR inserting data: %s", sqlite3_errstr(rc));
                result = Core::ERROR_GENERAL;
            }

            sqlite3_finalize(stmt);
        }

        if (result == Core::ERROR_NONE) {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "INSERT INTO item (ns,key,value)"
                                   " SELECT id, ?, ?"
                                   " FROM namespace"
                                   " WHERE name = ?"
                                   ";", -1, &stmt, nullptr);

            sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, ns.c_str(), -1, SQLITE_TRANSIENT);

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                LOGERR("ERROR inserting data: %s", sqlite3_errstr(rc));
                result = Core::ERROR_GENERAL;
            }
            else {
                ValueChanged(ns, key, value);
            }

            sqlite3_finalize(stmt);
        }
    }
    while ((result != Core::ERROR_NONE) &&
        SQLITE_IS_ERROR_DBWRITE(rc) &&
        (++retry < 2) &&
        (Open(_path, _key, _maxSize, _maxValue) == Core::ERROR_NONE));

    if (result == Core::ERROR_NONE) {
        CountingLockSync lock(_lock);

        sqlite3* &db = SQLITE;

        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "SELECT sum(s) FROM ("
                               " SELECT sum(length(key)+length(value)) s FROM item"
                               " UNION ALL"
                               " SELECT sum(length(name)) s FROM namespace"
                               ");", -1, &stmt, nullptr);

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            int64_t size = sqlite3_column_int64(stmt, 0);
            if (size > _maxSize) {
                LOGWARN("max size exceeded: %" PRId64 "\n", size);

                StorageExceeded();
            }
        }
        else
            LOGERR("ERROR getting size: %s", sqlite3_errstr(rc));

        sqlite3_finalize(stmt);
    }

    return result;
}

uint32_t SqliteStore::GetValue(const string &ns, const string &key, string &value)
{
    uint32_t result = Core::ERROR_GENERAL;

    CountingLockSync lock(_lock);

    sqlite3* &db = SQLITE;

    if (db) {
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "SELECT value"
                               " FROM item"
                               " INNER JOIN namespace ON namespace.id = item.ns"
                               " where name = ? and key = ?"
                               ";", -1, &stmt, nullptr);

        sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            value = (const char *) sqlite3_column_text(stmt, 0);
            result = Core::ERROR_NONE;
        }
        else
            LOGWARN("not found: %d", rc);

        sqlite3_finalize(stmt);
    }

    return result;
}

uint32_t SqliteStore::DeleteKey(const string &ns, const string &key)
{
    uint32_t result = Core::ERROR_GENERAL;

    int retry = 0;
    int rc;
    do {
        CountingLockSync lock(_lock);

        sqlite3* &db = SQLITE;

        if (!db)
            break;

        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "DELETE FROM item"
                               " where ns in (select id from namespace where name = ?)"
                               " and key = ?"
                               ";", -1, &stmt, NULL);

        sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_TRANSIENT);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
            LOGERR("ERROR removing data: %s", sqlite3_errstr(rc));
        else
            result = Core::ERROR_NONE;

        sqlite3_finalize(stmt);
    }
    while ((result != Core::ERROR_NONE) &&
        SQLITE_IS_ERROR_DBWRITE(rc) &&
        (++retry < 2) &&
        (Open(_path, _key, _maxSize, _maxValue) == Core::ERROR_NONE));

    return result;
}

uint32_t SqliteStore::DeleteNamespace(const string &ns)
{
    uint32_t result = Core::ERROR_GENERAL;

    int retry = 0;
    int rc;
    do {
        CountingLockSync lock(_lock);

        sqlite3* &db = SQLITE;

        if (!db)
            break;

        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "DELETE FROM namespace where name = ?;", -1, &stmt, NULL);

        sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
            LOGERR("ERROR removing data: %s", sqlite3_errstr(rc));
        else
            result = Core::ERROR_NONE;

        sqlite3_finalize(stmt);
    }
    while ((result != Core::ERROR_NONE) &&
        SQLITE_IS_ERROR_DBWRITE(rc) &&
        (++retry < 2) &&
        (Open(_path, _key, _maxSize, _maxValue) == Core::ERROR_NONE));

    return result;
}

uint32_t SqliteStore::GetKeys(const string &ns, std::vector<string> &keys)
{
    uint32_t result = Core::ERROR_GENERAL;

    CountingLockSync lock(_lock);

    sqlite3* &db = SQLITE;

    keys.clear();

    if (db) {
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "SELECT key"
                               " FROM item"
                               " where ns in (select id from namespace where name = ?)"
                               ";", -1, &stmt, NULL);

        sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

        while (sqlite3_step(stmt) == SQLITE_ROW)
            keys.push_back((const char *) sqlite3_column_text(stmt, 0));

        sqlite3_finalize(stmt);

        result = Core::ERROR_NONE;
    }

    return result;
}

uint32_t SqliteStore::GetNamespaces(std::vector<string> &namespaces)
{
    uint32_t result = Core::ERROR_GENERAL;

    CountingLockSync lock(_lock);

    sqlite3* &db = SQLITE;

    namespaces.clear();

    if (db) {
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "SELECT name FROM namespace;", -1, &stmt, NULL);

        while (sqlite3_step(stmt) == SQLITE_ROW)
            namespaces.push_back((const char *) sqlite3_column_text(stmt, 0));

        sqlite3_finalize(stmt);

        result = Core::ERROR_NONE;
    }

    return result;
}

uint32_t SqliteStore::GetStorageSize(std::map<string, uint64_t> &namespaceSizes)
{
    uint32_t result = Core::ERROR_GENERAL;

    CountingLockSync lock(_lock);

    sqlite3* &db = SQLITE;

    namespaceSizes.clear();

    if (db) {
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "SELECT name, sum(length(key)+length(value))"
                               " FROM item"
                               " INNER JOIN namespace ON namespace.id = item.ns"
                               " GROUP BY name"
                               ";", -1, &stmt, NULL);

        while (sqlite3_step(stmt) == SQLITE_ROW)
            namespaceSizes[(const char *) sqlite3_column_text(stmt, 0)] = sqlite3_column_int(stmt, 1);

        sqlite3_finalize(stmt);

        result = Core::ERROR_NONE;
    }

    return result;
}

uint32_t SqliteStore::FlushCache()
{
    uint32_t result = Core::ERROR_GENERAL;

    CountingLockSync lock(_lock);

    sqlite3* &db = SQLITE;

    if (db) {
        int rc = sqlite3_db_cacheflush(db);
        if (rc != SQLITE_OK) {
            LOGERR("Error while flushing sqlite database cache: %d", rc);
        }
        else {
            result = Core::ERROR_NONE;
        }
    }

    sync();

    return result;
}

uint32_t SqliteStore::Term()
{
    CountingLockSync lock(_lock, 0);

    Close();

    return Core::ERROR_NONE;
}

void SqliteStore::ValueChanged(const string &ns, const string &key, const string &value)
{
    Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

    std::list<Exchange::IStore::INotification *>::iterator
        index(_clients.begin());

    while (index != _clients.end()) {
        (*index)->ValueChanged(ns, key, value);
        index++;
    }
}

void SqliteStore::StorageExceeded()
{
    Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

    std::list<Exchange::IStore::INotification *>::iterator
        index(_clients.begin());

    while (index != _clients.end()) {
        (*index)->StorageExceeded();
        index++;
    }
}

int SqliteStore::Encrypt(const std::vector<uint8_t> &key)
{
    int rc = SQLITE_OK;

#if defined(SQLITE_HAS_CODEC)
    sqlite3* &db = SQLITE;

    bool shouldReKey = !IsEncrypted();

    if (!shouldReKey) {
        rc = sqlite3_key_v2(db, nullptr, key.data(), key.size());
    } else {
        rc = sqlite3_rekey_v2(db, nullptr, key.data(), key.size());
        if (rc == SQLITE_OK)
            Vacuum();
    }

    if (shouldReKey && !IsEncrypted()) {
        LOGERR("SQLite database file is clear after re-key");
    }
#endif

    return rc;
}

int SqliteStore::CreateTables()
{
    sqlite3* &db = SQLITE;

    char *errmsg;

    int rc = sqlite3_exec(db, "CREATE TABLE if not exists namespace ("
                              "id INTEGER PRIMARY KEY,"
                              "name TEXT UNIQUE"
                              ");", 0, 0, &errmsg);
    if (rc != SQLITE_OK || errmsg) {
        if (errmsg) {
            LOGERR("%d : %s", rc, errmsg);
            sqlite3_free(errmsg);
        }
        else
            LOGERR("%d", rc);

        return rc;
    }

    rc = sqlite3_exec(db, "CREATE TABLE if not exists item ("
                          "ns INTEGER,"
                          "key TEXT,"
                          "value TEXT,"
                          "FOREIGN KEY(ns) REFERENCES namespace(id) ON DELETE CASCADE ON UPDATE NO ACTION,"
                          "UNIQUE(ns,key) ON CONFLICT REPLACE"
                          ");", 0, 0, &errmsg);
    if (rc != SQLITE_OK || errmsg) {
        if (errmsg) {
            LOGERR("%d : %s", rc, errmsg);
            sqlite3_free(errmsg);
        }
        else
            LOGERR("%d", rc);

        return rc;
    }

    rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, &errmsg);
    if (rc != SQLITE_OK || errmsg) {
        if (errmsg) {
            LOGERR("%d : %s", rc, errmsg);
            sqlite3_free(errmsg);
        }
        else
            LOGERR("%d", rc);

        return rc;
    }

    return SQLITE_OK;
}

int SqliteStore::Vacuum()
{
    sqlite3* &db = SQLITE;

    char *errmsg;

    int rc = sqlite3_exec(db, "VACUUM", 0, 0, &errmsg);

    if (rc != SQLITE_OK || errmsg) {
        if (errmsg) {
            LOGERR("%s", errmsg);
            sqlite3_free(errmsg);
        }
        else {
            LOGERR("%d", rc);
        }

        return rc;
    }

    return SQLITE_OK;
}

int SqliteStore::Close()
{
    sqlite3* &db = SQLITE;

    if (db) {
        int rc = sqlite3_db_cacheflush(db);
        if (rc != SQLITE_OK) {
            LOGERR("Error while flushing sqlite database cache: %d", rc);
        }

        sqlite3_close_v2(db);
    }

    db = nullptr;

    return SQLITE_OK;
}

bool SqliteStore::IsOpen() const
{
    sqlite3* &db = SQLITE;

    return (db && IsValid());
}

bool SqliteStore::IsValid() const
{
    return (Core::File(_path).Exists());
}

bool SqliteStore::IsEncrypted() const
{
    bool result = false;

    Core::File file(_path);

    if (file.Exists() && file.Open(true)) {
        const uint32_t bufLen = strlen(SQLITE_FILE_HEADER);
        char buffer[bufLen];

        result =
            (file.Read(reinterpret_cast<uint8_t *>(buffer), bufLen) != bufLen) ||
                (::memcmp(buffer, SQLITE_FILE_HEADER, bufLen) != 0);
    }

    return result;
}

} // namespace Plugin
} // namespace WPEFramework
