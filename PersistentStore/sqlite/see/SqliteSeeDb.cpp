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

#include "SqliteSeeDb.h"

#ifndef SQLITE_FILE_HEADER
#define SQLITE_FILE_HEADER "SQLite format 3"
#endif

namespace WPEFramework {
namespace Plugin {

    uint32_t SqliteSeeDb::Open()
    {
        Core::SafeSyncType<Core::CriticalSection> lock(_dataLock);

        uint32_t result;

        if (_data && IsValid()) {
            // Seems open!
            result = Core::ERROR_ALREADY_CONNECTED;
        } else {
            Close();

            Core::File file(_path);
            Core::Directory(file.PathName().c_str()).CreatePath();

            auto shouldEncrypt = !_key.empty();
            auto shouldReKey = shouldEncrypt && IsValid() && !IsEncrypted();

            auto rc = sqlite3_open(_path.c_str(), &_data);

            if (rc == SQLITE_OK) {
                // Database is open, supply the key, if needed

                if (shouldEncrypt) {
                    result = Key(shouldReKey);
                } else {
                    result = Core::ERROR_NONE;
                }

                if (result == Core::ERROR_NONE) {
                    result = CreateSchema();

                    if ((result == Core::ERROR_UNAVAILABLE)
                        && shouldEncrypt
                        && !shouldReKey // re-key should never fail
                    ) {
                        // Key doesn't work, delete database and create new

                        Close();
                        if (Core::File(_path).Destroy() && !IsValid()) {
                            rc = sqlite3_open(_path.c_str(), &_data);
                            if (rc == SQLITE_OK) {
                                // Database is open, supply the key

                                result = Key(false);
                                if (result == Core::ERROR_NONE) {
                                    result = CreateSchema();
                                }
                            } else {
                                TRACE(Trace::Error, (_T("sqlite3_open returned %d"), rc));
                                result = Core::ERROR_GENERAL;
                            }
                        } else {
                            TRACE(Trace::Error, (_T("could not delete file %s"), _path.c_str()));
                            result = Core::ERROR_DESTRUCTION_FAILED;
                        }
                    }
                }
            } else {
                TRACE(Trace::Error, (_T("sqlite3_open returned %d"), rc));
                result = Core::ERROR_GENERAL;
            }
        }

        return result;
    }

    uint32_t SqliteSeeDb::Key(bool shouldReKey)
    {
        uint32_t result;

        std::vector<uint8_t> key;
        result = GenerateKey(key);

        if (result == Core::ERROR_NONE) {
            int rc;
            if (!shouldReKey) {
                rc = sqlite3_key_v2(*_data, nullptr, key.data(), key.size());
            } else {
                rc = sqlite3_rekey_v2(*_data, nullptr, key.data(), key.size());
                if (rc == SQLITE_OK)
                    Vacuum();
            }
            if (rc != SQLITE_OK) {
                TRACE(Trace::Error, (_T("sqlite3_key_v2/sqlite3_rekey_v2 returned %d"), rc));
                result = Core::ERROR_GENERAL;
            }
        }

        return SQLITE_OK;
    }

    bool SqliteSeeDb::IsEncrypted() const
    {
        bool result;

        Core::File file(_path);

        if (file.Exists()) {
            if (file.Open(true)) {
                auto bufLen = strlen(SQLITE_FILE_HEADER);
                char buffer[bufLen];

                result = (file.Read(reinterpret_cast<uint8_t*>(buffer), bufLen) != bufLen)
                    || (::memcmp(buffer, SQLITE_FILE_HEADER, bufLen) != 0);
            } else {
                TRACE(Trace::Error, (_T("could not open file %s"), _path.c_str()));
                result = false;
            }
        } else {
            result = false;
        }

        return result;
    }

} // namespace Plugin
} // namespace WPEFramework
