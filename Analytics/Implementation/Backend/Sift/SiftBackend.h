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

#include "../AnalyticsBackend.h"
#include "SiftConfig.h"
#include "SiftStore.h"
#include "SiftUploader.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace WPEFramework {
namespace Plugin {

    class SiftBackend: public IAnalyticsBackend {
    private:
        SiftBackend(const SiftBackend&) = delete;
        SiftBackend& operator=(const SiftBackend&) = delete;

    public:
        SiftBackend();
        ~SiftBackend();
        uint32_t SendEvent(const Event& event) override;
        uint32_t Configure(PluginHost::IShell* shell) override;
        uint32_t SetSessionId(const std::string& sessionId) override;

    private:

        struct Config
        {
            std::string url;
            std::string apiKey;
        };

        enum ActionType
        {
            ACTION_TYPE_UNDEF,
            ACTION_TYPE_DEVICE_INFO_SET,
            ACTION_TYPE_POPULATE_CONFIG,
            ACTION_TYPE_SEND_EVENT,
            ACTION_TYPE_SHUTDOWN
        };

        struct Action
        {
            ActionType type;
            std::shared_ptr<IAnalyticsBackend::Event> payload;
        };

        void ActionLoop();
        bool SendEventInternal(const Event& event, const SiftConfig::Attributes &attributes);

        static uint8_t GenerateRandomCharacter();
        static std::string GenerateRandomOctetString( uint32_t numOctets );
        static std::string GenerateRandomUUID();

        std::mutex mQueueMutex;
        std::condition_variable mQueueCondition;
        std::thread mThread;
        std::queue<Action> mActionQueue;
        std::queue<IAnalyticsBackend::Event> mEventQueue;

        PluginHost::IShell* mShell;
        SiftConfigPtr mConfigPtr;
        SiftStorePtr mStorePtr;
        SiftUploaderPtr mUploaderPtr;
    };

}
}
                            