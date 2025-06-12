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

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "../../Module.h"
#include "ILocalStore.h"
#include "DatabaseConnection.h"

namespace WPEFramework
{
    namespace Plugin
    {
        class LocalStore: public ILocalStore
        {
        public:
            LocalStore();
            ~LocalStore();

            bool Open(const std::string &path) override;
            bool CreateTable(const std::string &table) override;
            bool SetLimit(const std::string &table, uint32_t limit) override;
            std::pair<uint32_t, uint32_t> GetEntriesCount(const std::string &table, uint32_t start, uint32_t maxCount) const override;
            std::vector<std::string> GetEntries(const std::string &table, uint32_t start, uint32_t count) const override;
            bool RemoveEntries(const std::string &table, uint32_t start, uint32_t end) override;
            bool AddEntry(const std::string &table, const std::string &entry) override;
        private:

            std::string buildGetEventsQuery(const std::string &table, uint32_t start, uint32_t count) const;

            DatabaseConnectionPtr mDatabaseConnection;
            std::string mPath;
        };
    }
}
