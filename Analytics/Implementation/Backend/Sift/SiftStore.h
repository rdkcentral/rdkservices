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

#include "../../../Module.h"
#include "../../LocalStore/LocalStore.h"

#include <mutex>
#include <string>
#include <vector>
#include <memory>

namespace WPEFramework
{
    namespace Plugin
    {

        class SiftStore
        {
        public:
            SiftStore(const std::string &path, uint32_t eventsLimit);
            ~SiftStore();

            std::pair<uint32_t, uint32_t> GetEventCount() const;
            std::vector<std::string> GetEvents(uint32_t start, uint32_t count) const;
            bool RemoveEvents(uint32_t start, uint32_t end);
            bool PostEvent(const std::string &entry);

        private:
            uint32_t mEventsLimit;
            LocalStore mLocalStore;
            mutable std::mutex mMutex;
        };

        typedef std::shared_ptr<SiftStore> SiftStorePtr;

    }
}
