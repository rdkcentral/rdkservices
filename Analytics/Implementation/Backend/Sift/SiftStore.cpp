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

#include "SiftStore.h"
#include "UtilsLogging.h"

namespace WPEFramework
{
    namespace Plugin
    {
        const std::string TABLE{"queue"};


            SiftStore::SiftStore(const std::string &path, uint32_t eventsLimit):
                mEventsLimit(eventsLimit),
                mLocalStore(),
                mMutex()
            {
                if(mLocalStore.Open(path))
                {
                    if(mLocalStore.CreateTable(TABLE))
                    {
                        LOGINFO("Table created");
                        if(mLocalStore.SetLimit(TABLE, eventsLimit))
                        {
                            LOGINFO("Limit set");
                        }
                        else
                        {
                            LOGERR("Failed to set limit");
                        }
                    }
                    else
                    {
                        LOGERR("Failed to create table");
                    }
                }
                else
                {
                    LOGERR("Failed to open store");
                }
            }

            SiftStore::~SiftStore()
            {
            }

            std::pair<uint32_t, uint32_t> SiftStore::GetEventCount() const
            {
                std::lock_guard<std::mutex> lock(mMutex);

                return mLocalStore.GetEntriesCount(TABLE, 0, mEventsLimit);
            }

            std::vector<std::string> SiftStore::GetEvents(uint32_t start, uint32_t count) const
            {
                std::lock_guard<std::mutex> lock(mMutex);
                if (!count)
                {
                    LOGWARN("Count is zero which is invalid");
                    return std::vector<std::string>();
                }

                return mLocalStore.GetEntries(TABLE, start, count);
            }

            bool SiftStore::RemoveEvents(uint32_t start, uint32_t end)
            {
                std::lock_guard<std::mutex> lock(mMutex);
                return mLocalStore.RemoveEntries(TABLE, start, end);
            }

            bool SiftStore::PostEvent(const std::string &entry)
            {
                std::lock_guard<std::mutex> lock(mMutex);
                return mLocalStore.AddEntry(TABLE, entry);
            }
    }
}