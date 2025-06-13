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
#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace WPEFramework
{
    namespace Plugin
    {
        struct ILocalStore
        {
            virtual ~ILocalStore() = default;
            virtual bool Open(const std::string &path) = 0;
            virtual bool CreateTable(const std::string &table) = 0;
            virtual bool SetLimit(const std::string &table, uint32_t limit) = 0;
            virtual std::pair<uint32_t, uint32_t> GetEntriesCount(const std::string &table, uint32_t start, uint32_t maxCount) const = 0;
            virtual std::vector<std::string> GetEntries(const std::string &table, uint32_t start, uint32_t count) const = 0;
            virtual bool RemoveEntries(const std::string &table, uint32_t start, uint32_t end) = 0;
            virtual bool AddEntry(const std::string &table, const std::string &entry) = 0;
        };

        using ILocalStorePtr = std::shared_ptr<ILocalStore>;

    }
}