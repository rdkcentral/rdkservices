#pragma once

#include <map>
#include <string>
#include "../../Module.h"

// Interface for Analytics Backedn
namespace WPEFramework {
namespace Plugin {

    struct IAnalyticsBackend {
        virtual ~IAnalyticsBackend() = default;

        struct Event
        {
            std::string eventName;
            std::string eventVersion;
            std::string eventSource;
            std::string eventSourceVersion;
            std::list<std::string> cetList;
            uint64_t epochTimestamp;
            std::string eventPayload;
        };

        const static std::string SIFT;

        virtual uint32_t Configure(PluginHost::IShell* shell) = 0;
        virtual uint32_t SendEvent(const Event& event) = 0;
    };

    typedef std::map<std::string, IAnalyticsBackend&> IAnalyticsBackends;

    struct IAnalyticsBackendAdministrator {
        static IAnalyticsBackends& Instances();

        virtual ~IAnalyticsBackendAdministrator() = default;
    };

}
}
