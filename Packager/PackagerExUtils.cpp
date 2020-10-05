/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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

#include <sqlite3.h>
#include <glib.h>

#include <archive.h>
#include <archive_entry.h>

#include <curl/curl.h>
#include <curl/easy.h>

#include <uuid/uuid.h>

#include <limits.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <fstream>
#include <streambuf>
#include <regex>

#include <inttypes.h>
#include <dirent.h>
#include <sys/stat.h>

#include <iostream>
#include <cstring>
#include <cstdlib>

#include "PackagerExUtils.h"

#include "PackagerExImplementation.h"


#ifndef SQLITE_FILE_HEADER
#define SQLITE_FILE_HEADER "SQLite format 3"
#endif

#define SQLITE *(sqlite3**) &mData

namespace WPEFramework {
namespace Plugin {

// Helper
//
static PackageInfoEx* row2pkg(sqlite3_stmt *stmt); // private fwd

// Storage
//
const int64_t                        PackagerExUtils::MAX_SIZE_BYTES       = 1000000;
const int64_t                        PackagerExUtils::MAX_VALUE_SIZE_BYTES = 1000;
JsonObject                           PackagerExUtils::mPackageCfg;

// std::list<PackageInfoEx *>           PackagerExUtils::mPackages;
//std::list<PackageInfoEx *>::iterator PackagerExUtils::mPkgIter;

std::vector<std::thread>             PackagerExUtils::mThreadPool; // thread pool
WPEFramework::Plugin::JobPool        PackagerExUtils::mJobPool;
Core::CriticalSection                PackagerExUtils::mThreadLock;

void*                                PackagerExUtils::mData = nullptr;

#if defined(SQLITE_HAS_CODEC)

    bool PackagerExUtils::fileEncrypted(const char* f)
    {
        FILE* fd = fopen(f, "rb");
        if (!fd)
        {
            return false;
        }

        int    magicSize = strlen(SQLITE_FILE_HEADER);
        char* fileHeader = (char*)malloc(magicSize + 1);
        int     readSize = (int)fread(fileHeader, 1, magicSize, fd);

        fclose(fd);

        bool eq = magicSize == readSize && ::memcmp(fileHeader, SQLITE_FILE_HEADER, magicSize) == 0;
        free(fileHeader);

        return !eq;
    }
#endif

    bool PackagerExUtils::fileRemove(const char* f)
    {
        // LOGINFO(" ... Removing >>>  '%s' ", f);

        return ( remove (f) == 0);
    }

    bool PackagerExUtils::fileExists(const char* f)
    {
        return g_file_test(f, G_FILE_TEST_EXISTS);
    }

    uint64_t PackagerExUtils::folderSize(const char *d)
    {
        int64_t total_size = 0;
#if 1
        string path(d);

        // command to be executed
        std::string cmd("du -sb ");
        cmd.append(path);
        cmd.append(" | cut -f1 2>&1");

        // execute above command and get the output
        FILE *stream = popen(cmd.c_str(), "r");
        if (stream)
        {
            const int max_size = 2048;
            char readbuf[max_size];
            if (fgets(readbuf, max_size, stream) != NULL)
            {
                pclose(stream);
                return atoll(readbuf);
            }

            pclose(stream);

            LOGERR("PackagerExUtils::folderSize( %s ) - parse FAILED", d);
            return -1; // return error val
        }
        else
        {
            LOGERR("PackagerExUtils::folderSize( %s ) - command FAILED", d);
            return -1; // return error val
        }
#else
        DIR *dd;
        struct dirent *de;
        struct stat buf;

        int exists;

        dd = opendir(d);
        if (dd == NULL)
        {
            LOGERR("PackagerExUtils::folderSize( %s ) - FAILED", d);
            return -1;
        }

        total_size = 0;

        // Iterate folders and files
        for (de = readdir(dd); de != NULL; de = readdir(dd))
        {
            exists = stat(de->d_name, &buf);
            if (exists < 0)
            {
                LOGERR("Couldn't stat %s\n", de->d_name);
            }
            else
            {
                total_size += buf.st_size;
            }
        }//FOR

        closedir(dd);

        // LOGINFO("PackagerExUtils::folderSize( %s )  = %jd", d, total_size);
#endif

        return total_size;
    }


    bool iequals(const string& a, const string& b)
    {
        unsigned int sz = a.size();
        if (b.size() != sz)
            return false;
        for (unsigned int i = 0; i < sz; ++i)
            if (tolower(a[i]) != tolower(b[i]))
                return false;
        return true;
    }

    string PackagerExUtils::fileExtension(const std::string& f)
    {
        std::string::size_type idx = f.rfind('.');

        if(idx != std::string::npos)
        {
            std::string extension = f.substr(idx+1);

            return extension;
        }

        return "";
    }

    bool PackagerExUtils::fileEndsWith(const std::string& f, const std::string& ext)
    {
        std::string::size_type idx = f.rfind('.');

        if(idx != std::string::npos)
        {
            std::string extension = f.substr(idx+1);

            if(iequals(extension, ext) ) //extension.compare(ext))
            {
                return true;
            }
        }

        return false;
    }

    bool PackagerExUtils::removeFolder(const string& dirname)
    {
        return PackagerExUtils::removeFolder( (const char *) dirname.c_str() );
    }

    bool PackagerExUtils::removeFolder(const char *dirname)
    {
    if (dirname == nullptr)
    {
        LOGERR(" removeFolder() - bad args - NULL");
        return 0;
    }
#if 1
    string dd(dirname);

    string cmd = "rm -rf " + dd;

    LOGERR(" calling >>>  system( %s )", cmd.c_str() );

    system(cmd.c_str());
#else
        const char *topdir = dirname;

        DIR *dir;
        struct dirent *entry;
        char path[PATH_MAX];

        if (path == nullptr)
        {
            LOGERR("Out of memory error");
            return 0;
        }

        dir = opendir(dirname);
        if (dir == nullptr)
        {
            perror("Error opendir()");
            return 0;
        }

        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
            {
                snprintf(path, (size_t) PATH_MAX, "%s/%s", dirname, entry->d_name);
                if (entry->d_type == DT_DIR)
                {
                    removeFolder(path);
                }

                PackagerExUtils::fileRemove(path);
            }
        }//WHILE

        closedir(dir);

        // finally... remove the top folder
        PackagerExUtils::fileRemove(topdir);
#endif

        return 1;
    }

    std::string PackagerExUtils::getGUID()
    {
        uuid_t uuid;
        char uuid_str[37];

        // Generate GUID
        uuid_generate_time_safe(uuid);
        uuid_unparse_lower(uuid, uuid_str);

        return std::string( uuid_str );
    }

    bool PackagerExUtils::initDB(const char* filename, const char* key)
    {
        LOGINFO(" ... SQLite >>  filename: %s    key: %s", filename, key);

        sqlite3* &db = SQLITE;

        term(); // ensure closed

        bool shouldEncrypt = key && *key;
    #if defined(SQLITE_HAS_CODEC)
        bool shouldReKey = shouldEncrypt &&
                           PackagerExUtils::fileExists(filename) &&
                          !PackagerExUtils::fileEncrypted(filename);
    #endif
        int rc = sqlite3_open(filename, &db);
        if (rc)
        {
            LOGERR(" ... SQLite >>  %d : %s", rc, sqlite3_errmsg(db));
            term();
            return false;
        }

        if (shouldEncrypt)
        {
    #if defined(SQLITE_HAS_CODEC)
            std::vector<uint8_t> pKey;
    #if defined(USE_PLABELS)

            // NOTE: pbnj_utils stores the nonce under XDG_DATA_HOME/data.
            // If the dir doesn't exist it will fail
            auto path = g_build_filename(g_get_user_data_dir(), "data", nullptr);

            if (!PackagerExUtils::fileExists(path))
            {
                g_mkdir_with_parents(path, 0755);
            }
            g_free(path);

            bool result = pbnj_utils::prepareBufferForOrigin(key, [&pKey](const std::vector<uint8_t>& buffer)
            {
                pKey = buffer;
            });

            if (!result)
            {
                // LOGERR("pbnj_utils fail");
                PackagerExUtils::term();
                return false;
            }
    #else
            // LOGWARN("SQLite encryption key is not secure, path=%s", filename);
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
              LOGERR(" ... Failed to attach encryption key to SQLite");
                // LOGERR("Failed to attach encryption key to SQLite database %s\nCause - %s", filename, sqlite3_errmsg(db));
                PackagerExUtils::term();
                return false;
            }

            if (shouldReKey && !fileEncrypted(filename))
            {
                LOGERR(" ... SQLite database file is clear after re-key");

                // LOGERR("SQLite database file is clear after re-key, path=%s", filename);
            }
    #endif
        }

        char *errmsg = nullptr;

        // rc = sqlite3_exec(db, "CREATE TABLE if not exists namespace ("
        //                       "id INTEGER PRIMARY KEY,"
        //                       "name TEXT UNIQUE"
        //                       ");", 0, 0, &errmsg);

        // if (rc != SQLITE_OK || errmsg)
        // {
        //     if (errmsg)
        //     {
        //         fprintf(stderr, " %s() ... %d : %s", __PRETTY_FUNCTION__, rc, errmsg);

        //         // LOGERR("%d : %s", rc, errmsg);
        //         sqlite3_free(errmsg);
        //     }
        //     else
        //     {
        //         fprintf(stderr, " %s() ... %d : %s", __PRETTY_FUNCTION__, rc, "(none)");

        //         // LOGERR("%d", rc);
        //     }
        // }

    //     if (rc == SQLITE_NOTADB
    //         && shouldEncrypt
    // #if defined(SQLITE_HAS_CODEC)
    //         && !shouldReKey // re-key should never fail
    // #endif
    //             )
    //     {
    //         fprintf(stderr, " %s() ... SQLite database is encrypted, but the key doesn't work", __PRETTY_FUNCTION__);

    //         // LOGWARN("SQLite database is encrypted, but the key doesn't work");
    //         PackagerExUtils::term();

    //         if (!fileRemove(filename) || fileExists(filename))
    //         {
    //             // LOGERR("Can't remove file");
    //             return false;
    //         }

    //         rc = sqlite3_open(filename, &db);
    //         PackagerExUtils::term();

    //         if (rc || !PackagerExUtils::fileExists(filename))
    //         {
    //           fprintf(stderr, " %s() ... SQLite >> Can't create file", __PRETTY_FUNCTION__);
    //             // LOGERR("Can't create file");
    //             return false;
    //         }
    //         // LOGWARN("SQLite database has been reset, trying re-key");
    //         return PackagerExUtils::initDB(filename, key);
    //     }

        if ( createTable() == false)
        {
            LOGERR(" ... SQLite >> createTable() .... FAILED ! \n");
            return false;
        }

        rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, &errmsg);

        if (rc != SQLITE_OK || errmsg)
        {
            if (errmsg)
            {
              LOGERR(" ... SQLite >> %d : %s \n", rc, errmsg);
              // LOGERR("%d : %s", rc, errmsg);
              sqlite3_free(errmsg);
            }
            else
            {
              LOGINFO(" ... SQLite >> %d : %s \n", rc, "(none1)");
              // LOGERR("%d", rc);
            }
        }

        return true;
    }

    bool PackagerExUtils::createTable()
    {
        char   *errmsg;
        sqlite3* &db = SQLITE;

        int rc = sqlite3_exec(db, "CREATE TABLE if not exists dacTable ("
                                    "name TEXT NOT NULL,"
                                    "bundlePath TEXT NOT NULL,"
                                    "version TEXT NOT NULL,"
                                    "id TEXT PRIMARY KEY NOT NULL,"
                                    "installed TEXT NOT NULL,"
                                    "size INT NOT NULL,"
                                    "type TEXT NOT NULL"
                                    // "FOREIGN KEY(ns) REFERENCES namespace(id) ON DELETE CASCADE ON UPDATE NO ACTION,"
                                    // "UNIQUE(ns,key) ON CONFLICT REPLACE"
                                    ");", 0, 0, &errmsg);

        if (rc != SQLITE_OK || errmsg)
        {
            if (errmsg)
            {
                LOGERR(" ... SQLite >> %d : %s", rc, errmsg);

                // LOGERR("%d : %s", rc, errmsg);
                sqlite3_free(errmsg);
                return false;
            }
        }

        return true;
    }


    void PackagerExUtils::term()
    {
        sqlite3* &db = SQLITE;

        if (db)
        {
          sqlite3_close(db);
        }

        db = NULL;
    }

    void PackagerExUtils::vacuum()
    {
        LOGINFO(" ... ENTER");

        sqlite3* &db = SQLITE;

        if (db)
        {
            char *errmsg;
            int rc = sqlite3_exec(db, "VACUUM", 0, 0, &errmsg);
            if (rc != SQLITE_OK || errmsg)
            {
                if (errmsg)
                {
                    LOGERR(" ... SQLite >> %d : %s", rc, errmsg);

                    // LOGERR("%s", errmsg);
                    sqlite3_free(errmsg);
                }
                else
                {
                    LOGINFO(" ... SQLite >> %d : %s", rc, "(none2)");
                    // LOGERR("%d", rc);
                }
            }
        }
    }

    bool PackagerExUtils::hasPkgRow(const string& pkgId)
    {
        bool success = false;

        sqlite3* &db = SQLITE;

        if (db)
        {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "SELECT id"
                                    " FROM dacTable"
                                    " where id = ?"
                                    ";", -1, &stmt, nullptr);

            // SELECT this 'pkgId'
            //
            int rc = sqlite3_bind_text(stmt, 1, pkgId.c_str(), -1, SQLITE_TRANSIENT);

            // excute SQL
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW)
            {
                success = true; // FOUND !!
            }
            else
            {
                LOGINFO("... '%s' not found ... ", pkgId.c_str());
            }

            sqlite3_finalize(stmt);
        }

        return success;
    }

    bool PackagerExUtils::hasPkgRow(const char* pkgId)
    {
        return PackagerExUtils::hasPkgRow(string(pkgId));
    }

    bool PackagerExUtils::addPkgRow(const PackageInfoEx* pkg)
    {
        if(pkg == nullptr)
        {
            LOGERR(" ...  Bad Args...  NULL");
            return false;
        }

LOGINFO(" ... Adding row for '%s'... ", pkg->Name().c_str());

        bool success = false;

        sqlite3* &db = SQLITE;

        // Check size of addition to SQL
        //
        // if (db)
        // {
        //     sqlite3_stmt *stmt;
        //     sqlite3_prepare_v2(db, "SELECT sum(s) FROM ("
        //                            " SELECT sum(length(key)+length(value)) s FROM item"
        //                            " UNION ALL"
        //                            " SELECT sum(length(name)) s FROM namespace"
        //                            ");", -1, &stmt, nullptr);

        //     if (sqlite3_step(stmt) == SQLITE_ROW)
        //     {
        //         int64_t size = sqlite3_column_int64(stmt, 0);
        //         if (size > MAX_SIZE_BYTES)
        //         {
        //             // LOGWARN("max size exceeded: %lld", size);
        //         }
        //         else
        //         {
        //             success = true;
        //         }
        //     }
        //     else
        //     {
        //     // LOGERR("ERROR getting size: %s", sqlite3_errmsg(db));
        //     }

        //     sqlite3_finalize(stmt);
        // }

        // if (success)
        // {
        //     success = false;

        //     sqlite3_stmt *stmt;
        //     sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO dacTable (id) VALUES (?);", -1, &stmt, nullptr);

        //     sqlite3_bind_text(stmt, 1, ns.c_str(), -1, SQLITE_TRANSIENT);


        //     int rc = sqlite3_step(stmt);
        //     if (rc != SQLITE_DONE)
        //     {
        //      // LOGERR("ERROR inserting data: %s", sqlite3_errmsg(db));
        //     }
        //     else
        //     {
        //         success = true;
        //     }

        //     sqlite3_finalize(stmt);
        // }

success = true;

        if (success)
        {
            LOGINFO("...  Inserting data ... %ld bytes", pkg->SizeInBytes());

            success = false;

            sqlite3_stmt *stmt;
            int result = sqlite3_prepare_v2(db, "INSERT INTO dacTable (name, bundlePath, version, id, installed, size, type)  VALUES (?,?,?,?,?,?,?)", -1, &stmt, nullptr);

            if (result != SQLITE_OK)
            {
                LOGERR("ERROR  >>> sqlite3_prepare_v2() ... %s", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }

            if ((result = sqlite3_bind_text(stmt, 1, pkg->Name().c_str(), -1, SQLITE_TRANSIENT) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... Name: '%s' ... #1 ... %s", pkg->Name().c_str(), sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }

            if ((result = sqlite3_bind_text(stmt, 2, pkg->BundlePath().c_str(), -1, SQLITE_TRANSIENT) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... BundlePath ... #2 ... %s", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }

            if ((result = sqlite3_bind_text(stmt, 3, pkg->Version().c_str(), -1, SQLITE_TRANSIENT) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... Version ... #3 ... %s", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }

            if ((result = sqlite3_bind_text(stmt, 4, pkg->PkgId().c_str(), -1, SQLITE_TRANSIENT) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... PkgId ... #4 ... %s", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }

            if ((result = sqlite3_bind_text(stmt, 5, pkg->Installed().c_str(), -1, SQLITE_TRANSIENT) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... Installed ... #5 ... %s", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }

            if ((result = sqlite3_bind_int( stmt, 6, pkg->SizeInBytes()) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... SizeInBytes ... #6 ... %s", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return result;
            }

            if ((result = sqlite3_bind_text(stmt, 7, pkg->Type().c_str(), -1, SQLITE_TRANSIENT) ) != SQLITE_OK)
            {
                LOGERR("ERROR inserting data ... Type ... #7 ... %s  val: %s", sqlite3_errmsg(db), pkg->Type().c_str() );
                sqlite3_finalize(stmt);
                return result;
            }

            int rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                LOGERR("ERROR inserting data: %s", sqlite3_errmsg(db));
            }
            else
            {
                success = true;
            }

            sqlite3_finalize(stmt);
        }

//         if (success)
//         {
//             success = false;

//             sqlite3_stmt *stmt;
//             sqlite3_prepare_v2(db, "SELECT sum(s) FROM ("
//                                     " SELECT sum(length(key)+length(value)) s FROM item"
//                                     " UNION ALL"
//                                     " SELECT sum(length(name)) s FROM namespace"
//                                     ");", -1, &stmt, nullptr);

//             if (sqlite3_step(stmt) == SQLITE_ROW)
//             {
//                 int64_t size = sqlite3_column_int64(stmt, 0);
//                 if (size > MAX_SIZE_BYTES)
//                 {
//                     // LOGWARN("max size exceeded: %lld", size);

// // TODO: Fixme
// // TODO: Fixme

//                     // JsonObject params;
//                     // sendNotify(C_STR(EVT_ON_STORAGE_EXCEEDED), params);
//                 }
//                 else
//                 {
//                     success = true;
//                 }
//             }
//             else
//             {
//                 // LOGERR("ERROR getting size: %s", sqlite3_errmsg(db));
//             }

//             sqlite3_finalize(stmt);
//         }

        return success;
    }

    PackageInfoEx* row2pkg(sqlite3_stmt *stmt)
    {
        PackageInfoEx* pkg = nullptr;

        if(stmt)
        {
            pkg = Core::Service<PackageInfoEx>::Create<PackageInfoEx>();

            if(pkg)
            {
                pkg->setName(        sqlite3_column_text(stmt, 0) ); // name
                pkg->setBundlePath(  sqlite3_column_text(stmt, 1) ); // path
                pkg->setVersion(     sqlite3_column_text(stmt, 2) ); // version
                pkg->setPkgId(       sqlite3_column_text(stmt, 3) ); // id
                pkg->setInstalled(   sqlite3_column_text(stmt, 4) ); // installed
                pkg->setSizeInBytes( sqlite3_column_int( stmt, 5) ); // size (bytes)
                pkg->setType(        sqlite3_column_text(stmt, 6) ); // type
            }
            else
            {
                LOGERR(" row2pkg() - pkg create failed.");
            }
        }
        else
        {
            LOGERR(" row2pkg() - bad args.");
        }

        return pkg;
    }

    PackageInfoEx* PackagerExUtils::getPkgRow(const string& pkgId)
    {
        PackageInfoEx* pkg = nullptr;

        sqlite3* &db = SQLITE;

        if (db)
        {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "SELECT name, bundlePath, version, id, installed, size, type"
                                    " FROM dacTable"
                                    " where id = ?"
                                    ";", -1, &stmt, nullptr);

            // SELECT this 'pkgId'
            //
            int rc = sqlite3_bind_text(stmt, 1, pkgId.c_str(), -1, SQLITE_TRANSIENT);

            // excute SQL
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) // FOUND !!
            {
                pkg = row2pkg(stmt);
            }
            else
            {
                LOGWARN("ERROR:  getPkgRow() ...  %s not found: %d", pkgId.c_str(), rc);
            }

            sqlite3_finalize(stmt);
        }

        return pkg;
    }

    bool PackagerExUtils::delPkgRow(const string& pkgId)
    {
        if(pkgId.empty()) // passed empty string ?
        {
            LOGERR("... delPkgRow() - passed empty ID ");
            return false;
        }

        bool success = false;

        sqlite3* &db = SQLITE;

        if (db)
        {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "DELETE FROM dacTable"
                                    " where id = (?)"
                                    ";", -1, &stmt, NULL);

            sqlite3_bind_text(stmt, 1, pkgId.c_str(), -1, SQLITE_TRANSIENT);

            int rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                LOGERR("ERROR removing data: %s", sqlite3_errmsg(db));
            }
            else
            {
                LOGINFO("INFO removed data: %s", pkgId.c_str());
                success = true;
            }

            sqlite3_finalize(stmt);
        }
        return success;
    }

    static int showCallback(void *NotUsed, int argc, char **argv, char **azColName)
    {
        // fprintf(stderr, "\n - - - - - - - - - - - - - - - - - - - - - - - - - - - - <showCallback> \n");
        // int argc: holds the number of results
        // (array) azColName: holds each column returned
        // (array) argv: holds each value
        for(int i = 0; i < argc; i++)
        {
            // Show column name, value, and newline
            cout << azColName[i] << ": " << argv[i] << endl;
        }

        // Insert a newline
        cout << endl;
        // fprintf(stderr, "\n - - - - - - - - - - - - - - - - - - - - - - - - - - - - </showCallback> \n");

        // Return successful
        return 0;
    }

    void PackagerExUtils::showTable()
    {
        // LOGINFO(" - ENTER \n");

        char *zErrMsg;
        sqlite3* &db = SQLITE;

        // Save SQL insert data
        char sql[] = "SELECT * FROM 'dacTable';";

        // Run the SQL
        int rc = sqlite3_exec(db, &sql[0], showCallback, 0, &zErrMsg);

        if (rc != SQLITE_OK)
        {
            LOGERR("ERROR showing table ... %s", sqlite3_errmsg(db));
        }

        // LOGINFO(" - DONE \n");
    }

    void PackagerExUtils::updatePkgList(PackageList_t& list)
    {
        list.clear();

        sqlite3* &db = SQLITE;

        if (db)
        {
            sqlite3_stmt *stmt;
            sqlite3_prepare( db, "SELECT * from dacTable;", -1, &stmt, NULL );//preparing the statement
            sqlite3_step( stmt );//executing the statement

            while( sqlite3_column_text( stmt, 0 ) )
            {
                PackageInfoEx* pkg = row2pkg(stmt);
                sqlite3_step( stmt );

                if(pkg)
                {
                    list.push_back(pkg);

                    // LOGINFO("ADDING pkg to list ... %s", pkg->PkgId().c_str() );
                }
            }//WHILE
        }
        else
        {
            LOGERR(" updatePkgList() ...  FAILED");
        }
    }

    int64_t PackagerExUtils::sumSizeInBytes()
    {
        sqlite3* &db = SQLITE;
        int64_t total = 0;

        if (db)
        {
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "SELECT SUM(size) FROM dacTable;", -1, &stmt, nullptr);

            // excute SQL
            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) // FOUND !!
            {
                total = sqlite3_column_int(stmt, 0);
//                LOGWARN("ERROR:  sumSizeInBytes() ...  %ld bytes   rc: %d", total, rc);
            }
            else
            {
                LOGWARN("ERROR:  sumSizeInBytes() ...   sqlite3_step   bad: %d", rc);
            }

            sqlite3_finalize(stmt);
        }

        return total;
    }


    // ARCHIVE CODE
    static int
    copy_data(struct archive *ar, struct archive *aw)
    {
      int r;
      const void *buff;
      size_t size;
    #if ARCHIVE_VERSION_NUMBER >= 3000000
      int64_t offset;
    #else
      off_t offset;
    #endif

      for (;;)
      {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
        {
          return (ARCHIVE_OK);
        }
        if (r != ARCHIVE_OK)
        {
          return (r);
        }
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK)
        {
          // warn("archive_write_data_block()",
          //     archive_error_string(aw));
          return (r);
        }
      }
    }

    PackagerExUtils::DACrc_t PackagerExUtils::extractPKG(const char *filename, const char *to_path /* = nullptr */)
    {
      struct archive *a;
      struct archive *ext;
      struct archive_entry *entry;
      int flags;
      int r;

LOGINFO(" ... Extracting >>>  '%s' ", filename);

      // Select which attributes we want to restore.
      flags =  ARCHIVE_EXTRACT_TIME;
      flags |= ARCHIVE_EXTRACT_PERM;
      flags |= ARCHIVE_EXTRACT_ACL;
      flags |= ARCHIVE_EXTRACT_FFLAGS;

      a = archive_read_new();
      archive_read_support_format_all(a);
    //   archive_read_support_compression_all(a); // DEPRECATED ?
      archive_read_support_filter_all(a);

      ext = archive_write_disk_new();

      archive_write_disk_set_options(ext, flags);
      archive_write_disk_set_standard_lookup(ext);

      if ((r = archive_read_open_filename(a, filename, 10240)))
      {
        LOGERR("  >>>  FATAL - '%s' NOT found.", filename);
        return DACrc_t::dac_FAIL;
      }

      int read_count = 0;
      for (;;)
      {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
        {
          LOGERR(" %s r: %d ", archive_error_string(a), r);

          if(read_count == 0)
          {
            LOGERR(" .. Next Header ... Empty / Bad file > ARCHIVE_EOF\n");
            return DACrc_t::dac_FAIL;
          }

          break; // complete
        }
        if (r < ARCHIVE_OK)
        {
            LOGERR("r < ARCHIVE_OK ... %s   r: %d", archive_error_string(a), r);
        }

        if (r < ARCHIVE_WARN)
        {
            LOGERR(" .. Next Header ... Unexpected > ARCHIVE_WARN\n");
            return DACrc_t::dac_FAIL;
        }

        if(to_path != nullptr)
        {
            std::string targetFilepath(to_path);// = "/opt/";
            targetFilepath += archive_entry_pathname(entry);

            archive_entry_set_pathname(entry, targetFilepath.c_str());

//          LOGINFO(" EXTRACT >>>  entry: %s", targetFilepath.c_str());
        }

        read_count++;

        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK)
        {
          LOGERR("%s", archive_error_string(ext));
        }
        else if (archive_entry_size(entry) > 0)
        {
          r = copy_data(a, ext);
          if (r < ARCHIVE_OK)
          {
            LOGERR("%s", archive_error_string(ext));
          }

          if (r < ARCHIVE_WARN)
          {
            LOGERR(" ... Entry Size ... Unexpected > ARCHIVE_WARN");
            return DACrc_t::dac_FAIL;
          }
        }

        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK)
        {
          LOGERR("%s", archive_error_string(ext));
        }
        if (r < ARCHIVE_WARN)
        {
          LOGERR("  ...  Write Finish ... Unexpected > ARCHIVE_WARN");
          return DACrc_t::dac_FAIL;
        }
      }

      archive_read_close(a);
      archive_read_free(a);
      archive_write_close(ext);
      archive_write_free(ext);

      return DACrc_t::dac_OK; // success
    }

    void example_function()
    {
        std::cout << "bla" << std::endl;
    }

#ifdef USE_THREAD_POOL
    void PackagerExUtils::setupThreadQ(PackagerImplementation *ptr)
    {
        int num_threads = max<int>(1, std::thread::hardware_concurrency() / 2); // be nice

        LOGINFO(" ... hardware_concurrency()  tt: %d \n", num_threads);

        for (int i = 0; i < num_threads; i++)
        {
            LOGINFO(" ... Starting Worker()  i: %d \n", i);
            mThreadPool.push_back(std::thread(&JobPool::worker_func, &mJobPool));
        }
    }

    void PackagerExUtils::killThreadQ()
    {
        PackagerExUtils::mJobPool.done();

        // Kill workers
        for (unsigned int i = 0; i < PackagerExUtils::mThreadPool.size(); i++)
        {
            LOGINFO(" ... Killing WORKER  i: %d\n", i);
            PackagerExUtils::mThreadPool.at(i).join();
        }
    }
#endif // USE_THREAD_POOL

    void PackagerExUtils::addJob(JobMeta_t &job)
    // void PackagerExUtils::addJob( FOO_T &job )
    {
        mJobPool.push( job );

        // for (int i = 0; i < 10; i++)
        // {
        //     LOGINFO(" ... Adding JOB  i: %d \n", i);
        // }
    }

    size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
    {
        size_t written;
        written = fwrite(ptr, size, nmemb, stream);
        return written;
    }

    PackagerExUtils::DACrc_t PackagerExUtils::downloadJSON(const char *url, const char *tempName)
    {
        // Download JSON manifest...
        //
        if(downloadURL(url, tempName) == DACrc_t::dac_OK)
        {
            // Read entire JSON text file...
            std::ifstream t(tempName);
            std::string txt((std::istreambuf_iterator<char>(t)),
                             std::istreambuf_iterator<char>());

            // Parse the text to JSON ... and get install URL.
            mPackageCfg = JsonObject(txt);

            return DACrc_t::dac_OK;
        }

         return DACrc_t::dac_FAIL;
    }

    // Inspired by -
    //
    // https://stackoverflow.com/questions/5620235/cpp-regular-expression-to-validate-url/31613265
    //
    PackagerExUtils::DACrc_t PackagerExUtils::validateURL(const char *url)
    {
        std::regex url_regex (
            R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
            std::regex::extended
        );

        std::smatch url_match_result;

        LOGINFO(" ... Checking URL: %s ", url);

        std::string urlS(url);

        if (std::regex_match( urlS, url_match_result, url_regex))
        {
            LOGINFO(" ... Valid URL: %s \n", url);

            return DACrc_t::dac_OK;  /// ok
        }
        else
        {
            LOGERR(" ... Invalid URL: %s \n", url);

            return DACrc_t::dac_FAIL;
        }
    }

    PackagerExUtils::DACrc_t PackagerExUtils::downloadURL(const char *url, const char *tempName)
    {
      CURL *curl;
      FILE *fp;
      CURLcode res;

      PackagerExUtils::DACrc_t rc = DACrc_t::dac_FAIL;

      if(!url || !tempName)
      {
          LOGERR("... ERROR: BAD args ... nullptr");
          return rc;
      }

      // Always cleanup
      PackagerExUtils::fileRemove(tempName);

      LOGINFO(" ... Downloading >>> '%s' ... as '%s' ", url, tempName);

      curl = curl_easy_init();
      if (curl)
      {
          fp = fopen(tempName,"wb");

          if(!fp)
          {
            LOGERR("... download: '%s' ... as '%s'  - FAILED  (disk full ?) \n", url, tempName);
            return rc;
          }

          curl_easy_setopt(curl, CURLOPT_URL, url);
          curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
          curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
          curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
          curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
          curl_easy_setopt(curl, CURLOPT_USERAGENT, "Packager/1.0");

          res = curl_easy_perform(curl);

          if(res == CURLE_OK)
          {
              LOGERR("... download: '%s' ... as '%s'  - OK \n", url, tempName);

              rc = DACrc_t::dac_OK;  // SUCCESS

              // queue download for install
          }
          else
          {
            LOGERR("... download: '%s' ... as '%s'  - FAILED  (Not found ?) \n", url, tempName);
            return rc;
          }

          curl_easy_cleanup(curl);

          fclose(fp);
      }

      return rc;
    }

    //================================================================================================

    JobPool::JobPool() :
        mJobQ(), mLock(), mDataCondition(), mAcceptJobs(true)
    {
    }

    JobPool::~JobPool()
    {
    }

//    void JobPool::push(const FOO_T &job)
    void JobPool::push(JobMeta_t& job)
    {
        std::unique_lock<std::mutex> lock(mLock);
        mJobQ.push(job);

        // when we send the notification immediately,
        // the consumer will try to get the lock , so unlock asap
        lock.unlock();
        mDataCondition.notify_one();
    }

    void JobPool::done()
    {
        std::unique_lock<std::mutex> lock(mLock);
        mAcceptJobs = false;
        lock.unlock();

        // when we send the notification immediately,
        // the consumer will try to get the lock , so unlock asap
        mDataCondition.notify_all();

        //notify all waiting threads.
    }

    void JobPool::worker_func()
    {
        //std::function<void()> func;
        JobMeta_t job;

        // FOO_T func;

        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(mLock);
                mDataCondition.wait(lock, [this]()
                {
                  return !mJobQ.empty() || !mAcceptJobs;
                });

                if (!mAcceptJobs && mJobQ.empty())
                {
                    //lock will be release automatically.
                    //finish the thread loop and let it join in the main thread.
                    return;
                }

                job = mJobQ.front();
                mJobQ.pop();

            }//release the lock - scope !

            LOGINFO("CALLING JOB");
            LOGINFO("CALLING JOB");
            LOGINFO("CALLING JOB");
//            func();

            // doInstall( job )
        }
    }
  }  // namespace Plugin
}  // namespace WPEFramework
