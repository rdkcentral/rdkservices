#pragma once

#include "../AnalyticsBackend.h"
#include "SiftConfig.h"

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
        bool SendEventInternal(const Event& event, const SiftConfig::Config &config);

        static uint8_t GenerateRandomCharacter();
        static std::string GenerateRandomOctetString( uint32_t numOctets );
        static std::string GenerateRandomUUID();
        static uint32_t PostJson(const std::string& url, const std::string& apiKey, const std::string& json);

        std::mutex mQueueMutex;
        std::condition_variable mQueueCondition;
        std::thread mThread;
        std::queue<Action> mActionQueue;
        std::queue<IAnalyticsBackend::Event> mEventQueue;

        PluginHost::IShell* mShell;
        SiftConfigPtr mConfigPtr;
    };

}
}
                            