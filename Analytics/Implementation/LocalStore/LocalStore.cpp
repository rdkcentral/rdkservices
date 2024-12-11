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

#include "LocalStore.h"
#include "UtilsLogging.h"
#include "DatabaseConnection.h"

#include <unistd.h>

namespace WPEFramework
{
    namespace Plugin
    {
        const std::string DB_EXT = "db";

        LocalStore::LocalStore():
            mDatabaseConnection(nullptr),
            mPath()
        {
        }

        LocalStore::~LocalStore()
        {
        }

        bool LocalStore::Open(const std::string &path)
        {
            bool status = false;
            const std::string dbPath = path + "." + DB_EXT;

            if (mPath == dbPath && mDatabaseConnection != nullptr && mDatabaseConnection->IsConnected())
            {
                status = true;
                LOGINFO("Database %s already opened", dbPath.c_str());
                return status;
            }

            // Creates a database connection object
            DatabaseConnectionPtr conn = std::make_shared<DatabaseConnection>();

            // Connects to the database, which creates the database file if needed
            if (conn->Connect(dbPath))
            {
                status = true;
                mDatabaseConnection = conn;
                mPath = dbPath;
            }
            else
            {
                LOGERR("AddDatabase failed to create a new database %s ", dbPath.c_str());
            }

            return status;
        }

        bool LocalStore::CreateTable(const std::string &table)
        {
            bool status = false;
            const std::string query = "CREATE TABLE IF NOT EXISTS " + table 
                + " (id INTEGER PRIMARY KEY AUTOINCREMENT, data TEXT)";

            if (mDatabaseConnection != nullptr && mDatabaseConnection->IsConnected())
            {
                if (mDatabaseConnection->Exec(query))
                {
                    status = true;
                }
                else
                {
                    LOGERR("Failed to create table %s", table.c_str());
                }
            }
            else
            {
                LOGERR("Failed to create table %s, no connection", table.c_str());
            }

            return status;
        }

        bool LocalStore::SetLimit(const std::string &table, uint32_t limit)
        {
            bool status = false;
            const std::string query = "PRAGMA max_page_count = " + std::to_string(limit);

            if (mDatabaseConnection != nullptr && mDatabaseConnection->IsConnected())
            {
                if (mDatabaseConnection->Exec(query))
                {
                    status = true;
                }
                else
                {
                    LOGERR("Failed to set limit %u", limit);
                }
            }
            else
            {
                LOGERR("Failed to set limit %u, no connection", limit);
            }

            return status;
        }

        std::pair<uint32_t, uint32_t> LocalStore::GetEntriesCount(const std::string &table, uint32_t start, uint32_t maxCount) const
        {
            std::pair<uint32_t, uint32_t> count = std::make_pair(0, 0);

            if (mDatabaseConnection != nullptr && mDatabaseConnection->IsConnected())
            {
                std::string query = buildGetEventsQuery(table, start, maxCount);
                if (!query.empty())
                {
                    DatabaseTable table;
                    if (mDatabaseConnection->ExecAndGetResults(query, table)
                        && table.NumRows() > 0)
                    {
                        // get start from first row's id value
                        count.first = std::stoi(table[0][0].GetValue());
                        // get count from number of rows
                        count.second = table.NumRows();
                    }
                }
                else
                {
                    LOGERR("Failed to build query with {%u, %u}", start, maxCount);
                }
            }
            else
            {
                LOGERR("Failed to get entries count, no connection");
            }

            return count;
        }

        std::vector<std::string> LocalStore::GetEntries(const std::string &table, uint32_t start, uint32_t count) const
        {
            std::vector<std::string> entries{};

            if (mDatabaseConnection != nullptr && mDatabaseConnection->IsConnected())
            {
                std::string query = buildGetEventsQuery(table, start, count);
                if (!query.empty())
                {
                    DatabaseTable table;
                    if (mDatabaseConnection->ExecAndGetResults(query, table))
                    {
                        for (uint32_t rowIdx = 0; rowIdx < table.NumRows(); rowIdx++)
                        {
                            if (table[rowIdx].NumCols() < 2)
                            {
                                LOGERR("Failed to get entries, invalid row");
                                continue;
                            }

                            std::string entry = table[rowIdx][1].GetValue();
                            entries.push_back(entry);
                        }
                    }
                    else
                    {
                        LOGERR("Failed to get entries, query %s", query.c_str());
                    }
                }
                else
                {
                    LOGERR("Failed to build query with {%u, %u}", start, count);
                }
            }
            else
            {
                LOGERR("Failed to get entries, no connection");
            }

            return entries;
        }

        bool LocalStore::RemoveEntries(const std::string &table, uint32_t start, uint32_t end)
        {
            bool status = false;

            if (mDatabaseConnection != nullptr && mDatabaseConnection->IsConnected())
            {
                std::string query = "DELETE FROM " + table + " WHERE id BETWEEN " + std::to_string(start) + " AND " + std::to_string(end);
                uint32_t modifiedRows = 0;
                if (mDatabaseConnection->ExecAndGetModified(query, modifiedRows))
                {
                    status = true;
                }
                else
                {
                    LOGERR("Failed to remove entries, query %s", query.c_str());
                }
            }
            else
            {
                LOGERR("Failed to remove entries, no connection");
            }

            return status;
        }

        bool LocalStore::AddEntry(const std::string &table, const std::string &entry)
        {
            bool status = false;

            if (mDatabaseConnection != nullptr && mDatabaseConnection->IsConnected())
            {
                std::string query = "INSERT INTO " + table + " (data) VALUES ('" + entry + "')";
                if (mDatabaseConnection->Exec(query))
                {
                    status = true;
                }
                else
                {
                    LOGERR("Failed to add entry, query %s", query.c_str());
                }
            }
            else
            {
                LOGERR("Failed to add entry, no connection");
            }

            return status;
        }

        std::string LocalStore::buildGetEventsQuery(const std::string &table, uint32_t start, uint32_t count) const
        {
            std::string query{};

            if (0 == start)
            {
                query.append("SELECT * FROM " + table);
                query.append(" WHERE");
                query.append(" id>((SELECT MAX(id) FROM " + table + ")-" +
                             std::to_string(count) + ")");
            }
            else if (start && count)
            {
                query.append("SELECT * FROM " + table);
                query.append(" WHERE");
                query.append(" id>=" + std::to_string(start) + " LIMIT " +
                             std::to_string(count));
            }
            else
            {
                LOGERR("Failed to build query with {%u, %u}", start, count);
            }

            return query;
        }

    }
}