#pragma once

#include "gmock/gmock.h"
#include "rdkshell/compositorcontroller.h"

using namespace RdkShell;

struct ClientInfo
    {
        int32_t x;
        int32_t y;
        uint32_t width;
        uint32_t height;
        double sx;
        double sy;
        double opacity;
        int32_t zorder;
        bool visible;
    };

class CompositorControllerMock : public CompositorController {
public:
    virtual ~CompositorControllerMock() = default;

    MOCK_METHOD(void, initialize, ());
    MOCK_METHOD(bool, moveToFront, (const std::string& client));
    MOCK_METHOD(bool, moveToBack, (const std::string& client));
    MOCK_METHOD(bool, moveBehind, (const std::string& client, const std::string& target));
    MOCK_METHOD(bool, setFocus, (const std::string& client));
    MOCK_METHOD(bool, getFocused, (std::string& client));
    MOCK_METHOD(bool, kill, (const std::string& client));
    MOCK_METHOD(bool, addKeyIntercept, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags));
    MOCK_METHOD(bool, removeKeyIntercept, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags));
    MOCK_METHOD(bool, addKeyListener, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags, (std::map<std::string, RdkShellData> &listenerProperties)));
    MOCK_METHOD(bool, addNativeKeyListener, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags, (std::map<std::string, RdkShellData> &listenerProperties)));
    MOCK_METHOD(bool, removeKeyListener, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags));
    MOCK_METHOD(bool, removeAllKeyListeners, ());
    MOCK_METHOD(bool, removeAllKeyIntercepts, ());
    MOCK_METHOD(bool, removeNativeKeyListener, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags));
    MOCK_METHOD(bool, addKeyMetadataListener, (const std::string& client));
    MOCK_METHOD(bool, removeKeyMetadataListener, (const std::string& client));
    MOCK_METHOD(bool, injectKey, (const uint32_t& keyCode, const uint32_t& flags));
    MOCK_METHOD(bool, generateKey, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey));
    MOCK_METHOD(bool, getScreenResolution, (uint32_t &width, uint32_t &height));
    MOCK_METHOD(bool, setScreenResolution, (const uint32_t width, const uint32_t height));
    MOCK_METHOD(bool, getClients, (std::vector<std::string>& clients));
    MOCK_METHOD(bool, getZOrder, (std::vector<std::string>&clients));
    MOCK_METHOD(bool, getBounds, (const std::string& client, uint32_t &x, uint32_t &y, uint32_t &width, uint32_t &height));
    MOCK_METHOD(bool, setBounds, (const std::string& client, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height));
    MOCK_METHOD(bool, getVisibility, (const std::string& client, bool& visible));
    MOCK_METHOD(bool, setVisibility, (const std::string& client, const bool visible));
    MOCK_METHOD(bool, getOpacity, (const std::string& client, unsigned int& opacity));
    MOCK_METHOD(bool, setOpacity, (const std::string& client, const unsigned int opacity));
    MOCK_METHOD(bool, getScale, (const std::string& client, double &scaleX, double &scaleY));
    MOCK_METHOD(bool, setScale, (const std::string& client, double scaleX, double scaleY));
    MOCK_METHOD(bool, getHolePunch, (const std::string& client, bool& holePunch));
    MOCK_METHOD(bool, setHolePunch, (const std::string& client, const bool holePunch));
    MOCK_METHOD(bool, scaleToFit, (const std::string& client, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height));
    MOCK_METHOD(void, onKeyPress, (uint32_t keycode, uint32_t flags, uint64_t metadata, bool physicalKeyPress));
    MOCK_METHOD(void, onKeyRelease, (uint32_t keycode, uint32_t flags, uint64_t metadata, bool physicalKeyPress));
    MOCK_METHOD(void, onPointerMotion, (uint32_t x, uint32_t y));
    MOCK_METHOD(void, onPointerButtonPress, (uint32_t keyCode, uint32_t x, uint32_t y));
    MOCK_METHOD(void, onPointerButtonRelease, (uint32_t keyCode, uint32_t x, uint32_t y));
    MOCK_METHOD(bool, createDisplay, (const std::string& client, const std::string& displayName, uint32_t displayWidth, uint32_t displayHeight,
        bool virtualDisplayEnabled, uint32_t virtualWidth, uint32_t virtualHeight, bool topmost, bool focus , bool autodestroy));
    MOCK_METHOD(bool, addAnimation, (const std::string& client, double duration, (std::map<std::string, RdkShellData> &animationProperties)));
    MOCK_METHOD(bool, removeAnimation, (const std::string& client));
    MOCK_METHOD(bool, addListener, (const std::string& client, std::shared_ptr<RdkShellEventListener> listener));
    MOCK_METHOD(bool, removeListener, (const std::string& client, std::shared_ptr<RdkShellEventListener> listener));
    MOCK_METHOD(bool, onEvent, (RdkCompositor* eventCompositor, const std::string& eventName));
    MOCK_METHOD(void, enableInactivityReporting, (const bool enable));
    MOCK_METHOD(void, setInactivityInterval, (const double minutes));
    MOCK_METHOD(void, resetInactivityTime, ());
    MOCK_METHOD(double, getInactivityTimeInMinutes, ());
    MOCK_METHOD(void, setEventListener, (std::shared_ptr<RdkShellEventListener> listener));
    MOCK_METHOD(std::shared_ptr<RdkCompositor>, getCompositor, (const std::string& displayName));
    MOCK_METHOD(bool, launchApplication, (const std::string& client, const std::string& uri, const std::string& mimeType, bool topmost, bool focus));
    MOCK_METHOD(bool, suspendApplication, (const std::string& client));
    MOCK_METHOD(bool, resumeApplication, (const std::string& client));
    MOCK_METHOD(bool, closeApplication, (const std::string& client));
    MOCK_METHOD(bool, getMimeType, (const std::string& client, std::string& mimeType));
    MOCK_METHOD(bool, setMimeType, (const std::string& client, const std::string& mimeType));
    MOCK_METHOD(bool, hideSplashScreen, ());
    MOCK_METHOD(bool, showSplashScreen, (uint32_t displayTimeInSeconds));
    MOCK_METHOD(bool, hideWatermark, ());
    MOCK_METHOD(bool, showWatermark, ());
    MOCK_METHOD(bool, hideFullScreenImage, ());
    MOCK_METHOD(bool, showFullScreenImage, (std::string file));
    MOCK_METHOD(bool, draw, ());
    MOCK_METHOD(bool, update, ());
    MOCK_METHOD(bool, setLogLevel, (const std::string level));
    MOCK_METHOD(bool, getLogLevel, (std::string& level));
    MOCK_METHOD(bool, setTopmost, (const std::string& client, bool topmost, bool focus));
    MOCK_METHOD(bool, getTopmost, (std::string& client));
    MOCK_METHOD(bool, sendEvent, (const std::string& eventName, (std::vector<std::map<std::string, RdkShellData>>& data)));
    MOCK_METHOD(bool, isSurfaceModeEnabled, ());
    MOCK_METHOD(bool, enableKeyRepeats, (bool enable));
    MOCK_METHOD(bool, getKeyRepeatsEnabled, (bool& enable));
    MOCK_METHOD(bool, getVirtualResolution, (const std::string& client, uint32_t &virtualWidth, uint32_t &virtualHeight));
    MOCK_METHOD(bool, setVirtualResolution, (const std::string& client, const uint32_t virtualWidth, const uint32_t virtualHeight));
    MOCK_METHOD(bool, enableVirtualDisplay, (const std::string& client, const bool enable));
    MOCK_METHOD(bool, getVirtualDisplayEnabled, (const std::string& client, bool &enabled));
    MOCK_METHOD(bool, getLastKeyPress, (uint32_t &keyCode, uint32_t &modifiers, uint64_t &timestampInSeconds));
    MOCK_METHOD(bool, ignoreKeyInputs, (bool ignore));
    MOCK_METHOD(bool, updateWatermarkImage, (uint32_t imageId, int32_t key, int32_t imageSize));
    MOCK_METHOD(bool, createWatermarkImage, (uint32_t imageId, uint32_t zorder));
    MOCK_METHOD(bool, deleteWatermarkImage, (uint32_t imageId));
    MOCK_METHOD(bool, adjustWatermarkImage, (uint32_t imageId, uint32_t zorder));
    MOCK_METHOD(bool, alwaysShowWatermarkImageOnTop, (bool show));
    MOCK_METHOD(bool, screenShot, (uint8_t* &data, uint32_t &size));
    MOCK_METHOD(bool, enableInputEvents, (const std::string& client, bool enable));
    MOCK_METHOD(bool, showCursor, ());
    MOCK_METHOD(bool, hideCursor, ());
    MOCK_METHOD(bool, setCursorSize, (uint32_t width, uint32_t height));
    MOCK_METHOD(bool, getCursorSize, (uint32_t& width, uint32_t& height));
    MOCK_METHOD(void, setKeyRepeatConfig, (bool enabled, int32_t initialDelay, int32_t repeatInterval));
    MOCK_METHOD(bool, setAVBlocked, (std::string callsign, bool blockAV));
    MOCK_METHOD(bool, getBlockedAVApplications, (std::vector<std::string>& apps));
    MOCK_METHOD(bool, isErmEnabled, ());
    MOCK_METHOD(bool, getClientInfo, (const std::string& client, ClientInfo& ci));
    MOCK_METHOD(bool, setClientInfo, (const std::string& client, const ClientInfo& ci));
};

