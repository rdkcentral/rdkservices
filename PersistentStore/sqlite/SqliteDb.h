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

#include "ISqliteDb.h"

namespace WPEFramework {
namespace Plugin {

    class SqliteDb : public ISqliteDb {
    private:
        SqliteDb(const SqliteDb&) = delete;
        SqliteDb& operator=(const SqliteDb&) = delete;

    public:
        SqliteDb(const string& path, uint32_t maxSize, uint32_t maxValue)
            : _data(nullptr)
            , _path(path)
            , _maxSize(maxSize)
            , _maxValue(maxValue)
        {
        }
        virtual ~SqliteDb() override { Close(); }

    public:
        // ISqliteDb methods

        virtual uint32_t Open() override;
        inline operator sqlite3*() override
        {
            return (_data);
        }

        BEGIN_INTERFACE_MAP(SqliteDb)
        INTERFACE_ENTRY(ISqliteDb)
        END_INTERFACE_MAP

    protected:
        uint32_t Close();
        bool IsValid() const
        {
            return (Core::File(_path).Exists());
        }
        uint32_t Vacuum();
        uint32_t CreateSchema();

    protected:
        sqlite3* _data;
        const string _path;
        const uint32_t _maxSize;
        const uint32_t _maxValue;
        Core::CriticalSection _dataLock;
    };

} // namespace Plugin
} // namespace WPEFramework
