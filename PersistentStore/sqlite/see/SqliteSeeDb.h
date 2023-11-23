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

#pragma once

#include "../SqliteDb.h"

namespace WPEFramework {
namespace Plugin {

    class SqliteSeeDb : public SqliteDb {
    private:
        SqliteSeeDb(const SqliteSeeDb&) = delete;
        SqliteSeeDb& operator=(const SqliteSeeDb&) = delete;

    public:
        SqliteSeeDb(const string& path, uint32_t maxSize, uint32_t maxValue, const string& key)
            : SqliteDb(path, maxSize, maxValue)
            , _key(key)
        {
        }
        virtual ~SqliteSeeDb() override = default;

        uint32_t Open() override;

    protected:
        uint32_t Key(bool shouldReKey);
        bool IsEncrypted() const;
        virtual uint32_t GenerateKey(std::vector<uint8_t>& key) const
        {
            key = std::vector<uint8_t>(_key.begin(), _key.end());

            return Core::ERROR_NONE;
        }

    protected:
        string _key;
    };

} // namespace Plugin
} // namespace WPEFramework
