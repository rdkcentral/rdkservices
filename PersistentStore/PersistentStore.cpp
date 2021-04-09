#include "PersistentStore.h"

#include <sqlite3.h>
#include <glib.h>
#include <unistd.h>

#if defined(USE_PLABELS)
#include "pbnj_utils.hpp"
#endif

#ifndef SQLITE_FILE_HEADER
#define SQLITE_FILE_HEADER "SQLite format 3"
#endif

#define SQLITE *(sqlite3**)&mData
#define SQLITE_IS_ERROR_DBWRITE (rc == SQLITE_READONLY || rc == SQLITE_CORRUPT)

const short WPEFramework::Plugin::PersistentStore::API_VERSION_NUMBER_MAJOR = 1;
const short WPEFramework::Plugin::PersistentStore::API_VERSION_NUMBER_MINOR = 0;
const string WPEFramework::Plugin::PersistentStore::SERVICE_NAME = "org.rdk.PersistentStore";
const string WPEFramework::Plugin::PersistentStore::METHOD_SET_VALUE = "setValue";
const string WPEFramework::Plugin::PersistentStore::METHOD_GET_VALUE = "getValue";
const string WPEFramework::Plugin::PersistentStore::METHOD_DELETE_KEY = "deleteKey";
const string WPEFramework::Plugin::PersistentStore::METHOD_DELETE_NAMESPACE = "deleteNamespace";
const string WPEFramework::Plugin::PersistentStore::METHOD_GET_KEYS = "getKeys";
const string WPEFramework::Plugin::PersistentStore::METHOD_GET_NAMESPACES = "getNamespaces";
const string WPEFramework::Plugin::PersistentStore::METHOD_GET_STORAGE_SIZE = "getStorageSize";
const string WPEFramework::Plugin::PersistentStore::METHOD_FLUSH_CACHE = "flushCache";
const string WPEFramework::Plugin::PersistentStore::EVT_ON_STORAGE_EXCEEDED = "onStorageExceeded";
const char* WPEFramework::Plugin::PersistentStore::STORE_NAME = "rdkservicestore";
const char* WPEFramework::Plugin::PersistentStore::STORE_KEY = "xyzzy123";
const int64_t WPEFramework::Plugin::PersistentStore::MAX_SIZE_BYTES = 1000000;
const int64_t WPEFramework::Plugin::PersistentStore::MAX_VALUE_SIZE_BYTES = 1000;

using namespace std;

namespace {
    bool fileEncrypted(const char* f)
    {
        FILE* fd = fopen(f, "rb");
        if (!fd)
            return false;

        int magicSize = strlen(SQLITE_FILE_HEADER);
        char* fileHeader = (char*)malloc(magicSize + 1);
        int readSize = (int)fread(fileHeader, 1, magicSize, fd);
        fclose(fd);

        bool eq = magicSize == readSize && ::memcmp(fileHeader, SQLITE_FILE_HEADER, magicSize) == 0;
        free(fileHeader);

        return !eq;
    }

    bool fileRemove(const char* f)
    {
        return (remove (f) == 0);
    }

    bool fileExists(const char* f)
    {
        return g_file_test(f, G_FILE_TEST_EXISTS);
    }
}

namespace WPEFramework {
    namespace Plugin {

        SERVICE_REGISTRATION(PersistentStore, 1, 0);

        PersistentStore::PersistentStore()
            : AbstractPlugin()
            , mData(nullptr)
            , mReading(0)
        {
            registerMethod(METHOD_SET_VALUE, &PersistentStore::setValueWrapper, this);
            registerMethod(METHOD_GET_VALUE, &PersistentStore::getValueWrapper, this);
            registerMethod(METHOD_DELETE_KEY, &PersistentStore::deleteKeyWrapper, this);
            registerMethod(METHOD_DELETE_NAMESPACE, &PersistentStore::deleteNamespaceWrapper, this);
            registerMethod(METHOD_GET_KEYS, &PersistentStore::getKeysWrapper, this);
            registerMethod(METHOD_GET_NAMESPACES, &PersistentStore::getNamespacesWrapper, this);
            registerMethod(METHOD_GET_STORAGE_SIZE, &PersistentStore::getStorageSizeWrapper, this);
            registerMethod(METHOD_FLUSH_CACHE, &PersistentStore::flushCacheWrapper, this);
        }

        PersistentStore::~PersistentStore()
        {
        }

        const string PersistentStore::Initialize(PluginHost::IShell* /* service */)
        {
            return open() ? "" : "init failed";
        }

        void PersistentStore::Deinitialize(PluginHost::IShell* /* service */)
        {
            term();
        }

        string PersistentStore::Information() const
        {
            return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
        }

        // Registered methods (wrappers) begin
        uint32_t PersistentStore::setValueWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = false;
            if (!parameters.HasLabel("namespace") ||
                !parameters.HasLabel("key") ||
                !parameters.HasLabel("value"))
            {
                response["error"] = "params missing";
            }
            else
            {
                string ns = parameters["namespace"].String();
                string key = parameters["key"].String();
                string value = parameters["value"].String();
                if (ns.empty() || key.empty())
                    response["error"] = "params empty";
                else if (ns.size() > 1000 || key.size() > 1000 || value.size() > 1000)
                    response["error"] = "params too long";
                else
                    success = setValue(ns, key, value);
            }

            returnResponse(success);
        }

        uint32_t PersistentStore::getValueWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = false;
            if (!parameters.HasLabel("namespace") ||
                !parameters.HasLabel("key"))
            {
                response["error"] = "params missing";
            }
            else
            {
                string ns = parameters["namespace"].String();
                string key = parameters["key"].String();
                if (ns.empty() || key.empty())
                {
                    response["error"] = "params empty";
                }
                else
                {
                    string value;
                    success = getValue(ns, key, value);
                    if (success)
                        response["value"] = value;
                }
            }

            returnResponse(success);
        }

        uint32_t PersistentStore::deleteKeyWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = false;
            if (!parameters.HasLabel("namespace") ||
                !parameters.HasLabel("key"))
            {
                response["error"] = "params missing";
            }
            else
            {
                string ns = parameters["namespace"].String();
                string key = parameters["key"].String();
                if (ns.empty() || key.empty())
                    response["error"] = "params empty";
                else
                    success = deleteKey(ns, key);
            }

            returnResponse(success);
        }

        uint32_t PersistentStore::deleteNamespaceWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = false;
            if (!parameters.HasLabel("namespace"))
            {
                response["error"] = "params missing";
            }
            else
            {
                string ns = parameters["namespace"].String();
                if (ns.empty())
                    response["error"] = "params empty";
                else
                    success = deleteNamespace(ns);
            }

            returnResponse(success);
        }

        uint32_t PersistentStore::getKeysWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = false;
            if (!parameters.HasLabel("namespace"))
            {
                response["error"] = "params missing";
            }
            else
            {
                string ns = parameters["namespace"].String();
                if (ns.empty())
                    response["error"] = "params empty";
                else
                {
                    vector<string> keys;
                    success = getKeys(ns, keys);
                    if (success) {
                        JsonArray jsonKeys;
                        for (auto it = keys.begin(); it != keys.end(); ++it)
                            jsonKeys.Add(*it);
                        response["keys"] = jsonKeys;
                    }
                }
            }

            returnResponse(success);
        }

        uint32_t PersistentStore::getNamespacesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = false;
            vector<string> namespaces;
            success = getNamespaces(namespaces);
            if (success)
            {
                JsonArray jsonNamespaces;
                for (auto it = namespaces.begin(); it != namespaces.end(); ++it)
                    jsonNamespaces.Add(*it);
                response["namespaces"] = jsonNamespaces;
            }

            returnResponse(success);
        }

        uint32_t PersistentStore::getStorageSizeWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = false;
            map<string, uint64_t> namespaceSizes;
            success = getStorageSize(namespaceSizes);
            if (success)
            {
                JsonObject jsonNamespaceSizes;
                for (auto it = namespaceSizes.begin(); it != namespaceSizes.end(); ++it)
                    jsonNamespaceSizes[it->first.c_str()] = it->second;
                response["namespaceSizes"] = jsonNamespaceSizes;
            }

            returnResponse(success);
        }

        uint32_t PersistentStore::flushCacheWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = flushCache();

            returnResponse(success);
        }

        bool PersistentStore::setValue(const string& ns, const string& key, const string& value)
        {
            LOGINFO("%s %s %s", ns.c_str(), key.c_str(), value.c_str());

            bool success = false;

            lock_guard<mutex> lck(mLock);
            while (mReading > 0);

            sqlite3* &db = SQLITE;

            int retry = 0;
            int rc;
            do
            {
                if (!db)
                    break;

                sqlite3_stmt *stmt;
                sqlite3_prepare_v2(db, "SELECT sum(s) FROM ("
                                       " SELECT sum(length(key)+length(value)) s FROM item"
                                       " UNION ALL"
                                       " SELECT sum(length(name)) s FROM namespace"
                                       ");", -1, &stmt, nullptr);

                rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW)
                {
                    int64_t size = sqlite3_column_int64(stmt, 0);
                    if (size > MAX_SIZE_BYTES)
                        LOGWARN("max size exceeded: %lld", size);
                    else
                        success = true;
                }
                else
                    LOGERR("ERROR getting size: %s", sqlite3_errstr(rc));

                sqlite3_finalize(stmt);

                if (success)
                {
                    success = false;

                    sqlite3_stmt *stmt;
                    sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO namespace (name) values (?);", -1, &stmt, nullptr);

                    sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

                    rc = sqlite3_step(stmt);
                    if (rc != SQLITE_DONE)
                        LOGERR("ERROR inserting data: %s", sqlite3_errstr(rc));
                    else
                        success = true;

                    sqlite3_finalize(stmt);
                }

                if (success)
                {
                    success = false;

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
                    if (rc != SQLITE_DONE)
                        LOGERR("ERROR inserting data: %s", sqlite3_errstr(rc));
                    else
                        success = true;

                    sqlite3_finalize(stmt);
                }
            } while (!success && SQLITE_IS_ERROR_DBWRITE(rc) && (++retry < 2) && open());

            if (success)
            {
                success = false;

                sqlite3_stmt *stmt;
                sqlite3_prepare_v2(db, "SELECT sum(s) FROM ("
                                       " SELECT sum(length(key)+length(value)) s FROM item"
                                       " UNION ALL"
                                       " SELECT sum(length(name)) s FROM namespace"
                                       ");", -1, &stmt, nullptr);

                rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW)
                {
                    int64_t size = sqlite3_column_int64(stmt, 0);
                    if (size > MAX_SIZE_BYTES)
                    {
                        LOGWARN("max size exceeded: %lld", size);

                        JsonObject params;
                        sendNotify(C_STR(EVT_ON_STORAGE_EXCEEDED), params);
                    }
                    else
                        success = true;
                }
                else
                    LOGERR("ERROR getting size: %s", sqlite3_errstr(rc));

                sqlite3_finalize(stmt);
            }

            return success;
        }

        bool PersistentStore::getValue(const string& ns, const string& key, string& value)
        {
            LOGINFO("%s %s", ns.c_str(), key.c_str());

            bool success = false;

            {
                lock_guard<mutex> lck(mLock);
                mReading++;
            }

            sqlite3* &db = SQLITE;

            if (db)
            {
                sqlite3_stmt *stmt;
                sqlite3_prepare_v2(db, "SELECT value"
                                       " FROM item"
                                       " INNER JOIN namespace ON namespace.id = item.ns"
                                       " where name = ? and key = ?"
                                       ";", -1, &stmt, nullptr);

                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_TRANSIENT);

                int rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW)
                {
                    value = (const char*)sqlite3_column_text(stmt, 0);
                    success = true;
                }
                else
                    LOGWARN("not found: %d", rc);
                sqlite3_finalize(stmt);
            }

            mReading--;

            return success;
        }

        bool PersistentStore::deleteKey(const string& ns, const string& key)
        {
            LOGINFO("%s %s", ns.c_str(), key.c_str());

            bool success = false;

            lock_guard<mutex> lck(mLock);
            while (mReading > 0);

            sqlite3* &db = SQLITE;

            int retry = 0;
            int rc;
            do
            {
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
                    success = true;

                sqlite3_finalize(stmt);
            } while (!success && SQLITE_IS_ERROR_DBWRITE(rc) && (++retry < 2) && open());

            return success;
        }

        bool PersistentStore::deleteNamespace(const string& ns)
        {
            LOGINFO("%s", ns.c_str());

            bool success = false;

            lock_guard<mutex> lck(mLock);
            while (mReading > 0);

            sqlite3* &db = SQLITE;

            int retry = 0;
            int rc;
            do
            {
                if (!db)
                    break;

                sqlite3_stmt *stmt;
                sqlite3_prepare_v2(db, "DELETE FROM namespace where name = ?;", -1, &stmt, NULL);

                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

                rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE)
                    LOGERR("ERROR removing data: %s", sqlite3_errstr(rc));
                else
                    success = true;

                sqlite3_finalize(stmt);
            } while (!success && SQLITE_IS_ERROR_DBWRITE(rc) && (++retry < 2) && open());

            return success;
        }

        bool PersistentStore::getKeys(const string& ns, std::vector<string>& keys)
        {
            LOGINFO("%s", ns.c_str());

            bool success = false;

            {
                lock_guard<mutex> lck(mLock);
                mReading++;
            }

            sqlite3* &db = SQLITE;

            keys.clear();

            if (db)
            {
                sqlite3_stmt *stmt;
                sqlite3_prepare_v2(db, "SELECT key"
                                       " FROM item"
                                       " where ns in (select id from namespace where name = ?)"
                                       ";", -1, &stmt, NULL);

                sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);

                while (sqlite3_step(stmt) == SQLITE_ROW)
                    keys.push_back((const char*)sqlite3_column_text(stmt, 0));

                sqlite3_finalize(stmt);
                success = true;
            }

            mReading--;

            return success;
        }

        bool PersistentStore::getNamespaces(std::vector<string>& namespaces)
        {
            bool success = false;

            {
                lock_guard<mutex> lck(mLock);
                mReading++;
            }

            sqlite3* &db = SQLITE;

            namespaces.clear();

            if (db)
            {
                sqlite3_stmt *stmt;
                sqlite3_prepare_v2(db, "SELECT name FROM namespace;", -1, &stmt, NULL);

                while (sqlite3_step(stmt) == SQLITE_ROW)
                    namespaces.push_back((const char*)sqlite3_column_text(stmt, 0));

                sqlite3_finalize(stmt);
                success = true;
            }

            mReading--;

            return success;
        }

        bool PersistentStore::getStorageSize(std::map<string, uint64_t>& namespaceSizes)
        {
            bool success = false;

            {
                lock_guard<mutex> lck(mLock);
                mReading++;
            }

            sqlite3* &db = SQLITE;

            namespaceSizes.clear();

            if (db)
            {
                sqlite3_stmt *stmt;
                sqlite3_prepare_v2(db, "SELECT name, sum(length(key)+length(value))"
                                       " FROM item"
                                       " INNER JOIN namespace ON namespace.id = item.ns"
                                       " GROUP BY name"
                                       ";", -1, &stmt, NULL);

                while (sqlite3_step(stmt) == SQLITE_ROW)
                    namespaceSizes[(const char*)sqlite3_column_text(stmt, 0)] = sqlite3_column_int(stmt, 1);

                sqlite3_finalize(stmt);
                success = true;
            }

            mReading--;

            return success;
        }

        bool PersistentStore::flushCache()
        {
            lock_guard<mutex> lck(mLock);
            while (mReading > 0);

            sqlite3* &db = SQLITE;
            bool success = false;

            if (db)
            {
                int rc = sqlite3_db_cacheflush(db);
                success = (rc == SQLITE_OK);
                if (rc != SQLITE_OK)
                {
                    LOGERR("Error while flushing sqlite database cache: %d", rc);
                }
            }
            sync();
            return success;
        }

        bool PersistentStore::open()
        {
            bool result;

            auto path = g_build_filename("opt", "persistent", nullptr);
            auto file = g_build_filename(path, STORE_NAME, nullptr);

            sqlite3* &db = SQLITE;

            if (db && fileExists(file))
                result = false; // Seems open!
            else
            {
                if (!fileExists(path))
                    g_mkdir_with_parents(path, 0745);

                result = init(file, STORE_KEY);
            }

            g_free(path);
            g_free(file);

            return result;
        }

        void PersistentStore::term()
        {
            sqlite3* &db = SQLITE;

            if (db)
            {
                int rc = sqlite3_db_cacheflush(db);
                if (rc != SQLITE_OK)
                {
                    LOGERR("Error while flushing sqlite database cache: %d", rc);
                }
                sqlite3_close_v2(db);
            }

            db = nullptr;
        }

        void PersistentStore::vacuum()
        {
            sqlite3* &db = SQLITE;

            if (db)
            {
                char *errmsg;
                int rc = sqlite3_exec(db, "VACUUM", 0, 0, &errmsg);
                if (rc != SQLITE_OK || errmsg)
                {
                    if (errmsg)
                    {
                        LOGERR("%s", errmsg);
                        sqlite3_free(errmsg);
                    }
                    else
                        LOGERR("%d", rc);
                }
            }
        }

        bool PersistentStore::init(const char* filename, const char* key)
        {
            sqlite3* &db = SQLITE;

            term();

            bool shouldEncrypt = key && *key;
#if defined(SQLITE_HAS_CODEC)
            bool shouldReKey = shouldEncrypt && fileExists(filename) && !fileEncrypted(filename);
#endif
            int rc = sqlite3_open(filename, &db);
            if (rc)
            {
                LOGERR("%d : %s", rc, sqlite3_errstr(rc));
                term();
                return false;
            }

            /* Based on pxCore, Copyright 2015-2018 John Robinson */
            /* Licensed under the Apache License, Version 2.0 */
            if (shouldEncrypt)
            {
#if defined(SQLITE_HAS_CODEC)
                std::vector<uint8_t> pKey;
#if defined(USE_PLABELS)

                // NOTE: pbnj_utils stores the nonce under XDG_DATA_HOME/data.
                // If the dir doesn't exist it will fail
                auto path = g_build_filename(g_get_user_data_dir(), "data", nullptr);
                if (!fileExists(path))
                    g_mkdir_with_parents(path, 0755);
                g_free(path);

                bool result = pbnj_utils::prepareBufferForOrigin(key, [&pKey](const std::vector<uint8_t>& buffer) {
                    pKey = buffer;
                });
                if (!result)
                {
                    LOGERR("pbnj_utils fail");
                    term();
                    return false;
                }
#else
                LOGWARN("SQLite encryption key is not secure, path=%s", filename);
                pKey = std::vector<uint8_t>(key, key + strlen(key));
#endif
                if (!shouldReKey)
                    rc = sqlite3_key_v2(db, nullptr, pKey.data(), pKey.size());
                else
                {
                    rc = sqlite3_rekey_v2(db, nullptr, pKey.data(), pKey.size());
                    if (rc == SQLITE_OK)
                        vacuum();
                }

                if (rc != SQLITE_OK)
                {
                    LOGERR("Failed to attach encryption key to SQLite database %s\nCause - %s", filename, sqlite3_errstr(rc));
                    term();
                    return false;
                }

                if (shouldReKey && !fileEncrypted(filename))
                    LOGERR("SQLite database file is clear after re-key, path=%s", filename);
#endif
            }

            char *errmsg;
            rc = sqlite3_exec(db, "CREATE TABLE if not exists namespace ("
                                  "id INTEGER PRIMARY KEY,"
                                  "name TEXT UNIQUE"
                                  ");", 0, 0, &errmsg);
            if (rc != SQLITE_OK || errmsg)
            {
                if (errmsg)
                {
                    LOGERR("%d : %s", rc, errmsg);
                    sqlite3_free(errmsg);
                }
                else
                    LOGERR("%d", rc);
            }

            if (rc == SQLITE_NOTADB
                && shouldEncrypt
#if defined(SQLITE_HAS_CODEC)
                && !shouldReKey // re-key should never fail
#endif
                    )
            {
                LOGWARN("SQLite database is encrypted, but the key doesn't work");
                term();
                if (!fileRemove(filename) || fileExists(filename))
                {
                    LOGERR("Can't remove file");
                    return false;
                }
                rc = sqlite3_open(filename, &db);
                term();
                if (rc || !fileExists(filename))
                {
                    LOGERR("Can't create file");
                    return false;
                }
                LOGWARN("SQLite database has been reset, trying re-key");
                return init(filename, key);
            }

            rc = sqlite3_exec(db, "CREATE TABLE if not exists item ("
                                  "ns INTEGER,"
                                  "key TEXT,"
                                  "value TEXT,"
                                  "FOREIGN KEY(ns) REFERENCES namespace(id) ON DELETE CASCADE ON UPDATE NO ACTION,"
                                  "UNIQUE(ns,key) ON CONFLICT REPLACE"
                                  ");", 0, 0, &errmsg);
            if (rc != SQLITE_OK || errmsg)
            {
                if (errmsg)
                {
                    LOGERR("%d : %s", rc, errmsg);
                    sqlite3_free(errmsg);
                }
                else
                    LOGERR("%d", rc);
            }

            rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, &errmsg);
            if (rc != SQLITE_OK || errmsg)
            {
                if (errmsg)
                {
                    LOGERR("%d : %s", rc, errmsg);
                    sqlite3_free(errmsg);
                }
                else
                    LOGERR("%d", rc);
            }

            return true;
        }
    } // namespace Plugin
} // namespace WPEFramework
