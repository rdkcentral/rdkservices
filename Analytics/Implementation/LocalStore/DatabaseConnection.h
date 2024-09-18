/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
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
 **/
#pragma once

#include "../../Module.h"
#include "DatabaseInterface.h"

#include <mutex>
#include <stdint.h>
#include <string>
#include <vector>
#include <memory>

namespace WPEFramework {
    namespace Plugin {

        class DatabaseTable {
            public: class DatabaseRow {
                public: class DatabaseColumnEntry {
                    private: std::string mName;
                    std::string mValue;

                    public: DatabaseColumnEntry(std::string & name, std::string & value): mName(name),
                    mValue(value) {}

                    const std::string & GetName(void) const {
                        return mName;
                    }

                    const std::string & GetValue(void) const {
                        return mValue;
                    }
                };

                DatabaseRow() {
                    mRow.clear();
                }
                DatabaseColumnEntry & operator[](uint32_t idx) {
                    return mRow[idx];
                }
                void AddColEntry(const DatabaseColumnEntry & entry) {
                    mRow.push_back(entry);
                }

                uint32_t NumCols(void) const {
                    return mRow.size();
                }

                private: std::vector < DatabaseColumnEntry > mRow;
            };

            DatabaseTable() {
                mTable.clear();
            }
            uint32_t NumRows(void) const {
                return mTable.size();
            }
            DatabaseRow & operator[](uint32_t idx) {
                return mTable[idx];
            }
            void AddRow(const DatabaseRow & row) {
                mTable.push_back(row);
            }

            private: std::vector < DatabaseRow > mTable;
        };

        struct DatabaseQuery {
            std::string mQuery;
            std::string mDatabaseName;

            DatabaseQuery(std::string query, std::string dbName): mQuery(query), mDatabaseName(dbName) {}
        };

        class DatabaseConnection {
            public: DatabaseConnection();
            ~DatabaseConnection();

            bool Connect(const std::string & databaseName);
            bool DisConnect();
            bool Exec(const std::string & query);
            bool ExecAndGetModified(const std::string & query, uint32_t & modifiedRows);
            bool ExecAndGetResults(const std::string & query, DatabaseTable & table);
            const std::string & GetDatabaseName(void) const {
                return mDatabaseName;
            }
            bool IsConnected(void) {
                return (mDataBaseHandle != NULL);
            }

            private: static int32_t DbCallbackOnly(void * arg, int argc, char ** argv, char ** colName);
            static int32_t DbCallbackGetResults(void * arg, int argc, char ** argv, char ** colName);

            std::string mDatabaseName;
            DB_HANDLE * mDataBaseHandle;
            std::mutex mMutex;
        };

        typedef std::shared_ptr < DatabaseConnection > DatabaseConnectionPtr;

    }
}