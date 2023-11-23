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

#include "../Module.h"

#include <sqlite3.h>

namespace WPEFramework {
namespace Plugin {
    namespace Sqlite {

        class Handle {
        private:
            Handle(const Handle&) = delete;
            Handle& operator=(const Handle&) = delete;

        public:
            Handle()
                : _data(nullptr)
            {
                Open();
            }
            virtual ~Handle() { Close(); }

            virtual uint32_t Open();
            inline operator sqlite3*()
            {
                return (_data);
            }

        private:
            uint32_t Close();
            uint32_t CreateSchema();

        private:
            sqlite3* _data;
            Core::CriticalSection _dataLock;
        };

    } // namespace Sqlite
} // namespace Plugin
} // namespace WPEFramework
