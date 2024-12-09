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
#include "DatabaseConnection.h"
#include "UtilsLogging.h"

namespace WPEFramework {
    namespace Plugin {

        DatabaseConnection::DatabaseConnection(): mDatabaseName(), mDataBaseHandle(NULL), mMutex() {}

        DatabaseConnection::~DatabaseConnection() {
            DisConnect();
        }

        bool DatabaseConnection::Connect(const std::string & databaseName) {
            bool ret = false;

            // Ensure a database name is given
            if (!databaseName.empty()) {
                // If a connection is already open close it first
                if (mDataBaseHandle != NULL && DisConnect() != DB_OK) {
                    LOGERR("Database %s open failed: could not close existing "
                        "connection",
                        databaseName.c_str());
                } else {
                    // Assign database name
                    mDatabaseName = databaseName;

                    // Opens a database or creates one with create and rw privileges
                    int32_t queryRet = DB_OPEN((const char * ) databaseName.c_str(), & mDataBaseHandle);

                    // Handle result from DB query
                    if (DB_OK == queryRet) {
                        ret = true;
                        LOGINFO("Database %s open succeeded", databaseName.c_str());
                    } else {
                        LOGERR("Database %s open failed: %s db err code %d",
                            databaseName.c_str(),
                            DB_ERRMSG(mDataBaseHandle),
                            queryRet);
                    }
                }
            } else {
                LOGERR("Database open failed: invalid db name %s", databaseName.c_str());
            }

            return ret;
        }

        bool DatabaseConnection::DisConnect() {
            bool ret = false;

            // Closes a database reference by the database handle
            if (mDataBaseHandle != NULL) {
                // Closes a database based on the associated handle
                int32_t queryRet = DB_CLOSE(mDataBaseHandle);

                // Handle result from DB query
                if (DB_OK == queryRet) {
                    ret = true;
                    LOGINFO("Database %s close succeeded", mDatabaseName.c_str());
                    mDatabaseName.clear();
                    mDataBaseHandle = NULL;
                } else {
                    LOGERR("Database %s close failed: %s db err code %d",
                        mDatabaseName.c_str(),
                        DB_ERRMSG(mDataBaseHandle),
                        queryRet);
                }
            }

            return ret;
        }

        bool DatabaseConnection::Exec(const std::string & query) {
            bool ret = false;
            char * errmsg = NULL;

            std::lock_guard < std::mutex > lock(mMutex);

            // Verify the database handle was created
            if (mDataBaseHandle != NULL) {
                DatabaseQuery queryCbData(query, mDatabaseName);

                // Execute a generic query to the database based on the associated handle
                int32_t queryRet = DB_QUERY(mDataBaseHandle,
                    query.c_str(),
                    DbCallbackOnly,
                    (void * ) & queryCbData, &
                    errmsg);

                // Handle result from DB query
                if (DB_OK == queryRet) {
                    // Note that row data could be large and therefore cannot log query
                    ret = true;
                } else {
                    LOGERR("Database %s query failed errmsg: %s db err code %d",
                        mDatabaseName.c_str(),
                        errmsg,
                        queryRet);

                    // If error, database malloc's the message so it needs to be free'd
                    DB_FREE(errmsg);
                }
            } else {
                LOGERR("Database connection not established for %s. "
                    "Query failed.",
                    mDatabaseName.c_str());
            }

            return ret;
        }

        bool DatabaseConnection::ExecAndGetModified(const std::string & query,
            uint32_t & modifiedRows) {

            bool ret = false;
            char * errmsg = NULL;

            std::lock_guard < std::mutex > lock(mMutex);

            // Verify the database handle was created
            if (mDataBaseHandle != NULL) {
                DatabaseQuery queryCbData(query, mDatabaseName);

                // Execute a generic query to the database based on the associated handle
                int32_t queryRet = DB_QUERY(mDataBaseHandle,
                    query.c_str(),
                    DbCallbackOnly,
                    (void * ) & queryCbData, &
                    errmsg);

                // Handle result from DB query
                if (DB_OK == queryRet) {
                    // Executes a query to get how many rows in the table were affected
                    modifiedRows = DB_CHANGES(mDataBaseHandle);
                    ret = true;
                } else {
                    LOGERR("Database %s query failed errmsg: %s db err code %d",
                        mDatabaseName.c_str(),
                        errmsg,
                        queryRet);

                    // If error, database malloc's the message so it needs to be free'd
                    DB_FREE(errmsg);
                }
            } else {
                LOGERR("Database connection not established for %s. "
                    "Query failed.",
                    mDatabaseName.c_str());
            }

            return ret;
        }

        bool DatabaseConnection::ExecAndGetResults(const std::string & query,
            DatabaseTable & table) {
            bool ret = false;
            char * errmsg = NULL;

            std::lock_guard < std::mutex > lock(mMutex);

            if (mDataBaseHandle != NULL) {
                DatabaseTable result;

                // Execute a generic query to the database based on the associated handle
                // which also returns a result to client i.e SELECT * FROM blah
                int32_t queryRet = DB_QUERY(mDataBaseHandle,
                    query.c_str(),
                    DbCallbackGetResults,
                    (void * ) & result, &
                    errmsg);

                // Handle result from DB query
                if (DB_OK == queryRet) {
                    ret = true;
                    table = result;
                } else {
                    LOGERR("Database %s query failed with error: %s db err code %d",
                        mDatabaseName.c_str(),
                        errmsg,
                        queryRet);
                    DB_FREE(errmsg);
                }
            } else {
                LOGERR("Database connection not established for %s. Query failed.",
                    mDatabaseName.c_str());
            }

            return ret;
        }

        int32_t DatabaseConnection::DbCallbackOnly(void * arg,
            int argc,
            char ** argv,
            char ** colName) {
            int32_t ret = DB_ERROR;
            DatabaseQuery * query = static_cast < DatabaseQuery * > (arg);

            if (query) {
                ret = DB_OK;
            } else {
                LOGERR("Database query executed with no data");
            }

            return ret;
        }

        int32_t DatabaseConnection::DbCallbackGetResults(void * arg,
            int argc,
            char ** argv,
            char ** colName) {
            int32_t ret = DB_ERROR;
            DatabaseTable * table = static_cast < DatabaseTable * > (arg);

            if (table != NULL) {
                DatabaseTable::DatabaseRow row;

                for (int index = 0; index < argc; index++) {
                    std::string name = colName[index];
                    std::string value;
                    if (argv[index]) {
                        value = argv[index];
                    }

                    row.AddColEntry(
                        DatabaseTable::DatabaseRow::DatabaseColumnEntry(name, value));
                }

                table -> AddRow(row);

                ret = DB_OK;

            } else {
                LOGERR("Database invalid query, cannot get results");
            }

            return ret;
        }

    } // namespace Plugin
} // namespace WPEFramework