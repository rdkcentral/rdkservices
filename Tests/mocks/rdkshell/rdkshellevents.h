#pragma once

#include <typeindex>
#include <string>
#include <map>
#include <vector>
#include "rdkshelldata.h"

namespace RdkShell
{
    class RdkShellEventListener
    {
        public:
          virtual void onApplicationLaunched(const std::string& client) {}
          virtual void onApplicationConnected(const std::string& client) {}
          virtual void onApplicationDisconnected(const std::string& client)  {}
          virtual void onApplicationTerminated(const std::string& client) {}
          virtual void onApplicationFirstFrame(const std::string& client) {}
          virtual void onApplicationSuspended(const std::string& client) {}
          virtual void onApplicationResumed(const std::string& client) {}
          virtual void onApplicationActivated(const std::string& client) {}
          virtual void onUserInactive(const double minutes) {}
          virtual void onDeviceLowRamWarning(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb) {}
          virtual void onDeviceCriticallyLowRamWarning(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb) {}
          virtual void onDeviceLowRamWarningCleared(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb) {}
          virtual void onDeviceCriticallyLowRamWarningCleared(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb) {}
          virtual void onAnimation(std::vector<std::map<std::string, RdkShellData>>& animationData) {}
          virtual void onEasterEgg(const std::string& name, const std::string& actionJson) {}
          virtual void onPowerKey() {}
          virtual void onKeyEvent(const uint32_t keyCode, const uint32_t flags, const bool keyDown) {}
          virtual void onSizeChangeComplete(const std::string& client) {}
    };

    const std::string RDKSHELL_EVENT_APPLICATION_LAUNCHED = "onApplicationLaunched";
    const std::string RDKSHELL_EVENT_APPLICATION_CONNECTED = "onApplicationConnected";
    const std::string RDKSHELL_EVENT_APPLICATION_DISCONNECTED = "onApplicationDisconnected";
    const std::string RDKSHELL_EVENT_APPLICATION_TERMINATED = "onApplicationTerminated";
    const std::string RDKSHELL_EVENT_APPLICATION_FIRST_FRAME = "onApplicationFirstFrame";
    const std::string RDKSHELL_EVENT_APPLICATION_SUSPENDED = "onApplicationSuspended";
    const std::string RDKSHELL_EVENT_APPLICATION_RESUMED = "onApplicationResumed";
    const std::string RDKSHELL_EVENT_APPLICATION_ACTIVATED = "onApplicationActivated";
    const std::string RDKSHELL_EVENT_USER_INACTIVE = "onUserInactive";
    const std::string RDKSHELL_EVENT_DEVICE_LOW_RAM_WARNING = "onDeviceLowRamWarning";
    const std::string RDKSHELL_EVENT_DEVICE_CRITICALLY_LOW_RAM_WARNING = "onDeviceCriticallyLowRamWarning";
    const std::string RDKSHELL_EVENT_DEVICE_LOW_RAM_WARNING_CLEARED = "onDeviceLowRamWarningCleared";
    const std::string RDKSHELL_EVENT_DEVICE_CRITICALLY_LOW_RAM_WARNING_CLEARED = "onDeviceCriticallyLowRamWarningCleared";
    const std::string RDKSHELL_EVENT_ANIMATION = "onAnimation";
    const std::string RDKSHELL_EVENT_EASTER_EGG = "onEasterEgg";
    const std::string RDKSHELL_EVENT_POWER_KEY = "onPowerKey";
    const std::string RDKSHELL_EVENT_KEY = "onKeyEvent";
    const std::string RDKSHELL_EVENT_SIZE_CHANGE_COMPLETE = "onSizeChangeComplete";
}

