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

#include "SiftStore.h"

#include <mutex>
#include <string>
#include <vector>
#include <thread>
#include <condition_variable>
#include <memory>


namespace WPEFramework
{
    namespace Plugin
    {
        class SiftUploader
        {
        public:

            SiftUploader(SiftStorePtr storePtr,
                            const std::string &url,
                            const uint32_t &maxRandomisationWindowTime,
                            const uint32_t &maxEventsInPost,
                            const uint32_t &maxRetries,
                            const uint32_t &minRetryPeriod,
                            const uint32_t &maxRetryPeriod,
                            const uint32_t &exponentialPeriodicFactor);
            ~SiftUploader();

            void setDeviceInfoRequiredFields( const std::string &accountId, const std::string &deviceId, const std::string &partnerId);

        private:

            enum UploaderState
            {
                RANDOMISATION_WINDOW_WAIT_STATE,
                COLLECT_ANALYTICS,
                POST_ANALYTICS
            };

            SiftUploader(const SiftUploader&) = delete;
            SiftUploader& operator=(const SiftUploader&) = delete;

            void Run();
            bool PerformWaitIfRetryNeeded();
            uint32_t RandomisationWindowTimeGenerator() const;
            bool CollectEventsFromAnalyticsStore(uint32_t count);
            std::string ComposeJSONEventArrayToBeUploaded(const std::vector<std::string> &events) const;
            void updateEventDeviceInfoIfRequired(JsonObject &event) const;
            void validateResponse(const std::string &response, const std::vector<std::string> &events) const;

            static long PostJson(const std::string& url, const std::string& json, std::string &response);

            SiftStorePtr mStorePtr;
            std::string mUrl;
            uint32_t mMaxRandomisationWindowTime;
            uint32_t mMaxEventsInPost;
            uint32_t mMaxRetries;
            uint32_t mMinRetryPeriod;
            uint32_t mMaxRetryPeriod;
            uint32_t mExponentialPeriodicFactor;

            std::string mAccountId;
            std::string mDeviceId;
            std::string mPartnerId;

            mutable std::mutex mMutex;
            UploaderState mUploaderState;
            std::condition_variable mCondition;
            std::thread mThread;
            bool mStop;
            uint32_t mCurrentRetryCount;
            uint32_t mEventStartIndex;
            std::vector<std::string> mEvents;
        };

        typedef std::unique_ptr<SiftUploader> SiftUploaderPtr;
    }
}
