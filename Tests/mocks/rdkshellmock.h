#pragma once

#include <gmock/gmock.h>

#include "rdkshell.h"

class RDKShellImplMock : public RDKShellImpl{
public:
      RDKShellImplMock()
	      : RDKShellImpl()
     {

     }
     virtual ~RDKShellImplMock() = default;
     MOCK_METHOD(bool, keyCodeFromWayland, (uint32_t waylandKeyCode, uint32_t waylandFlags, uint32_t &mappedKeyCode, uint32_t &mappedFlags), (override));
};

class RdkShellApiImplMock : public RdkShell::RdkShellApiImpl
{
	public:
		RdkShellApiImplMock() : RdkShell::RdkShellApiImpl()
	{
	}
        virtual ~RdkShellApiImplMock() = default;
	MOCK_METHOD(bool, systemRam, (uint32_t& freeKb, uint32_t& totalKb, uint32_t& availableKb, uint32_t& usedSwapKb), (override));
	MOCK_METHOD(void, setMemoryMonitor, ((std::map<std::string, RdkShell::RdkShellData> &configuration)), (override));
	MOCK_METHOD(void, addEasterEgg, (std::vector<RdkShell::RdkShellEasterEggKeyDetails>& details, std::string name, uint32_t timeout, std::string actionJson), (override));
	MOCK_METHOD(void, removeEasterEgg, (std::string name), (override));
	MOCK_METHOD(void, getEasterEggs, (std::vector<RdkShell::RdkShellEasterEggDetails>& easterEggs), (override));
	MOCK_METHOD(void, initialize, (), (override));
	MOCK_METHOD(void, update, (), (override));
	MOCK_METHOD(void, draw, (), (override));
	MOCK_METHOD(void, deinitialize, (), (override));
	MOCK_METHOD(double,  microseconds, (), (override));
	MOCK_METHOD(double, milliseconds, (), (override));
	MOCK_METHOD(double, seconds, (), (override));
};

class LoggerImplMock : public RdkShell::LoggerImpl
{
	public:
		LoggerImplMock() : RdkShell::LoggerImpl()
	       {
	       }
	       virtual ~LoggerImplMock() = default;
	       MOCK_METHOD(void, enableFlushing, (bool enable), (override));
               MOCK_METHOD(bool, isFlushingEnabled, (), (override));
};

class RdkShellEventImplMock : public RdkShell::RdkShellEvent
{
  public:
    RdkShellEventImplMock()
            : RdkShell::RdkShellEvent()
    {

    }
    virtual ~RdkShellEventImplMock() = default;
    MOCK_METHOD(void, onApplicationLaunched, (const std::string& client), (override));
    MOCK_METHOD(void, onApplicationConnected, (const std::string& client), (override));
    MOCK_METHOD(void, onApplicationDisconnected, (const std::string& client), (override));
    MOCK_METHOD(void, onApplicationTerminated, (const std::string& client), (override));
    MOCK_METHOD(void, onApplicationFirstFrame, (const std::string& client), (override));
    MOCK_METHOD(void, onApplicationSuspended, (const std::string& client), (override));
    MOCK_METHOD(void, onApplicationResumed, (const std::string& client), (override));
    MOCK_METHOD(void, onApplicationActivated, (const std::string& client), (override));
    MOCK_METHOD(void, onUserInactive, (const double minutes), (override));
    MOCK_METHOD(void, onDeviceLowRamWarning, (const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb), (override));
    MOCK_METHOD(void, onDeviceCriticallyLowRamWarning, (const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb), (override));
    MOCK_METHOD(void, onDeviceLowRamWarningCleared, (const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb), (override));
    MOCK_METHOD(void, onDeviceCriticallyLowRamWarningCleared, (const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb), (override));
    MOCK_METHOD(void, onPowerKey, (), (override));
    MOCK_METHOD(void, onSizeChangeComplete, (const std::string& client), (override));
    MOCK_METHOD(void, onEasterEgg, (const std::string& name, const std::string& actionJson), (override));
}; 

class CompositorImplMock : public RdkShell::CompositorControllerImpl {
public:
    CompositorImplMock()
	    : RdkShell::CompositorControllerImpl()
    {

    }
    virtual ~CompositorImplMock() = default;

    MOCK_METHOD(bool, setLogLevel, (const std::string level), (override));
    MOCK_METHOD(bool, isErmEnabled, (), (override));
    MOCK_METHOD(bool, scaleToFit, (const std::string& client, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height), (override));
    MOCK_METHOD(bool, ignoreKeyInputs, (bool ignore), (override));
    MOCK_METHOD(bool, moveToFront, (const std::string& client), (override));
    MOCK_METHOD(bool, moveToBack, (const std::string& client), (override));
    MOCK_METHOD(bool, getClients, (std::vector<std::string>& clients), (override));
    MOCK_METHOD(bool, moveBehind, (const std::string& client, const std::string& target), (override));
    MOCK_METHOD(bool, getOpacity, (const std::string& client, unsigned int& opacity), (override));
    MOCK_METHOD(bool, setOpacity, (const std::string& client, const unsigned int opacity), (override));
    MOCK_METHOD(bool, setFocus, (const std::string& client), (override));
    MOCK_METHOD(bool, removeKeyIntercept, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags), (override));
    MOCK_METHOD(bool, removeKeyListener, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags), (override));
    MOCK_METHOD(bool, removeKeyMetadataListener, (const std::string& client), (override));
    MOCK_METHOD(bool, injectKey, (const uint32_t& keyCode, const uint32_t& flags), (override));
    MOCK_METHOD(bool, setScreenResolution, (const uint32_t width, const uint32_t height), (override));
    MOCK_METHOD(bool, getScreenResolution, (uint32_t &width, uint32_t &height), (override));
    MOCK_METHOD(bool, setVirtualResolution, (const std::string& client, const uint32_t virtualWidth, const uint32_t virtualHeight), (override));
    MOCK_METHOD(bool, getVirtualResolution, (const std::string& client, uint32_t &virtualWidth, uint32_t &virtualHeight), (override));
    MOCK_METHOD(bool, getZOrder, (std::vector<std::string>&clients), (override));
    MOCK_METHOD(bool, getVisibility, (const std::string& client, bool& visible), (override));
    MOCK_METHOD(bool, setVisibility, (const std::string& client, const bool visible), (override));
    MOCK_METHOD(bool, getHolePunch, (const std::string& client, bool& holePunch), (override));
    MOCK_METHOD(bool, setHolePunch, (const std::string& client, const bool holePunch), (override));
    MOCK_METHOD(bool, getBounds, (const std::string& client, uint32_t &x, uint32_t &y, uint32_t &width, uint32_t &height), (override));
    MOCK_METHOD(bool, getScale, (const std::string& client, double &scaleX, double &scaleY), (override));
    MOCK_METHOD(bool, setCursorSize, (uint32_t width, uint32_t height), (override));
    MOCK_METHOD(bool, getCursorSize, (uint32_t& width, uint32_t& height), (override));
    MOCK_METHOD(bool, setBounds, (const std::string& client, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height ), (override));
    MOCK_METHOD(bool, setScale, (const std::string& client, double scaleX, double scaleY), (override));
    MOCK_METHOD(bool, getFocused, (std::string& client), (override));
    MOCK_METHOD(void, enableInactivityReporting, (const bool enable), (override));
    MOCK_METHOD(bool, showCursor, (), (override));
    MOCK_METHOD(bool, hideCursor, (), (override));
    MOCK_METHOD(void, setKeyRepeatConfig, (bool enabled, int32_t initialDelay, int32_t repeatInterval), (override));
    MOCK_METHOD(bool, getBlockedAVApplications, (std::vector<std::string>& apps), (override));
    MOCK_METHOD(bool, setAVBlocked, (std::string callsign, bool blockAV), (override));
    MOCK_METHOD(bool, enableInputEvents, (const std::string& client, bool enable), (override));
    MOCK_METHOD(bool, getLogLevel, (std::string& level), (override));
    MOCK_METHOD(bool, enableKeyRepeats, (bool enable), (override));
    MOCK_METHOD(bool, getKeyRepeatsEnabled, (bool& enable), (override));
    MOCK_METHOD(bool, screenShot, (uint8_t* &data, uint32_t &size), (override));
    MOCK_METHOD(bool, getLastKeyPress, (uint32_t &keyCode, uint32_t &modifiers, uint64_t &timestampInSeconds), (override));
    MOCK_METHOD(bool, getVirtualDisplayEnabled, (const std::string& client, bool &enabled), (override));
    MOCK_METHOD(bool, enableVirtualDisplay, (const std::string& client, const bool enable), (override));
    MOCK_METHOD(bool, isSurfaceModeEnabled, (), (override));
    MOCK_METHOD(bool, getTopmost, (std::string& client), (override));
    MOCK_METHOD(bool, setTopmost, (const std::string& client, bool topmost, bool focus), (override));
    MOCK_METHOD(bool, showFullScreenImage, (std::string file), (override));
    MOCK_METHOD(bool, hideFullScreenImage, (), (override));
    MOCK_METHOD(bool, showWatermark, (), (override));
    MOCK_METHOD(bool, hideWatermark, (), (override));
    MOCK_METHOD(bool, showSplashScreen, (uint32_t displayTimeInSeconds), (override));
    MOCK_METHOD(bool, hideSplashScreen, (), (override));
    MOCK_METHOD(bool, setMimeType, (const std::string& client, const std::string& mimeType), (override));
    MOCK_METHOD(bool, getMimeType, (const std::string& client, std::string& mimeType), (override));
    MOCK_METHOD(bool, launchApplication, (const std::string& client, const std::string& uri, const std::string& mimeType, bool topmost, bool focus), (override));
    MOCK_METHOD(bool, suspendApplication, (const std::string& client), (override));
    MOCK_METHOD(bool, resumeApplication, (const std::string& client), (override));
    MOCK_METHOD(void, setEventListener, (std::shared_ptr<RdkShell::RdkShellEventListener> listener), (override));
    MOCK_METHOD(void, resetInactivityTime, (), (override));
    MOCK_METHOD(bool, removeListener, (const std::string& client, std::shared_ptr<RdkShell::RdkShellEventListener> listener), (override));
    MOCK_METHOD(bool, addListener, (const std::string& client, std::shared_ptr<RdkShell::RdkShellEventListener> listener), (override));
    MOCK_METHOD(bool, addAnimation, (const std::string& client, double duration, (std::map<std::string, RdkShell::RdkShellData> &animationProperties)), (override));
    MOCK_METHOD(bool, removeAnimation,(const std::string& client), (override));
    MOCK_METHOD(bool, createDisplay, (const std::string& client, const std::string& displayName, uint32_t displayWidth, uint32_t displayHeight,
                bool virtualDisplayEnabled, uint32_t virtualWidth, uint32_t virtualHeight, bool topmost, bool focus, bool autodestroy), (override));
    MOCK_METHOD(bool, generateKey, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey), (override));
    MOCK_METHOD(bool, generateKey, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey, double duration), (override));
    MOCK_METHOD(bool, addKeyMetadataListener, (const std::string& client), (override));
    MOCK_METHOD(bool, removeNativeKeyListener, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags), (override));
    MOCK_METHOD(bool, addNativeKeyListener, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags, (std::map<std::string, RdkShell::RdkShellData> &listenerProperties)), (override));
    MOCK_METHOD(bool, addKeyListener, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags, (std::map<std::string, RdkShell::RdkShellData> &listenerProperties)), (override));
    MOCK_METHOD(bool, addKeyIntercept, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags), (override));
    MOCK_METHOD(bool, kill, (const std::string& client), (override));
    MOCK_METHOD(void, setInactivityInterval, (const double minutes), (override));
};

