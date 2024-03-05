#pragma once
#include <string>
#include <map>
#include <vector>
#include <sys/sysinfo.h>
#include <memory>
#define RDKSHELL_FLAGS_SHIFT        8
#define RDKSHELL_FLAGS_CONTROL      16
#define RDKSHELL_FLAGS_ALT          32

class RDKShellImpl {
public:
    RDKShellImpl()
    {
    }
    virtual ~RDKShellImpl() = default;
    virtual bool keyCodeFromWayland(uint32_t waylandKeyCode, uint32_t waylandFlags, uint32_t &mappedKeyCode, uint32_t &mappedFlags) = 0;
};

class RDKShell {
protected:
    static RDKShellImpl* impl;
public:

    RDKShell();
    RDKShell(const RDKShell &obj) = delete; // deleted copy constructor so that copy of the instance cannot be created.
    static void setImpl(RDKShellImpl* newImpl);
    static bool keyCodeFromWayland(uint32_t waylandKeyCode, uint32_t waylandFlags, uint32_t &mappedKeyCode, uint32_t &mappedFlags);
};
extern bool (*keyCodeFromWayland)(uint32_t waylandKeyCode, uint32_t waylandFlags, uint32_t &mappedKeyCode, uint32_t &mappedFlags);

namespace RdkShell
{
    class RdkShellEvent
    {
       public:
          virtual void onApplicationLaunched(const std::string& client) = 0;
          virtual void onApplicationConnected(const std::string& client) = 0;
          virtual void onApplicationDisconnected(const std::string& client) = 0;
          virtual void onApplicationTerminated(const std::string& client) = 0;
          virtual void onApplicationFirstFrame(const std::string& client) = 0;
          virtual void onApplicationSuspended(const std::string& client) = 0;
          virtual void onApplicationResumed(const std::string& client) = 0;
          virtual void onApplicationActivated(const std::string& client) = 0;
          virtual void onUserInactive(const double minutes) = 0;
          virtual void onDeviceLowRamWarning(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb) = 0;
          virtual void onDeviceCriticallyLowRamWarning(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb) = 0;
          virtual void onDeviceLowRamWarningCleared(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb) = 0;
          virtual void onDeviceCriticallyLowRamWarningCleared(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb) = 0;
          virtual void onPowerKey() = 0;
          virtual void onSizeChangeComplete(const std::string& client) = 0;
          virtual void onEasterEgg(const std::string& name, const std::string& actionJson) = 0;
    };


    class RdkShellData
    {
       public:
              RdkShellData& operator=(bool value) {return *this; }
              RdkShellData& operator=(const std::string& value) {return *this; }

    };

    class RdkShellEventListener
    {
        protected:
          static RdkShellEvent* impl;
        public:
          RdkShellEventListener();
          RdkShellEventListener(const RdkShellEventListener &obj) = delete; // deleted copy constructor so that copy of the instance cannot be created.
          static void setImpl(RdkShellEvent* newImpl);
          virtual void onApplicationLaunched(const std::string& client);
          virtual void onApplicationConnected(const std::string& client);
          virtual void onApplicationDisconnected(const std::string& client);
          virtual void onApplicationTerminated(const std::string& client);
          virtual void onApplicationFirstFrame(const std::string& client);
          virtual void onApplicationSuspended(const std::string& client);
          virtual void onApplicationResumed(const std::string& client);
          virtual void onApplicationActivated(const std::string& client);
          virtual void onUserInactive(const double minutes);
          virtual void onDeviceLowRamWarning(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb);
          virtual void onDeviceCriticallyLowRamWarning(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb);
          virtual void onDeviceLowRamWarningCleared(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb);
          virtual void onDeviceCriticallyLowRamWarningCleared(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb);
          virtual void onEasterEgg(const std::string& name, const std::string& actionJson);
          virtual void onPowerKey();
          virtual void onSizeChangeComplete(const std::string& client);
          const std::string RDKSHELL_EVENT_SIZE_CHANGE_COMPLETE = "onSizeChangeComplete";
    };


   struct RdkShellEasterEggKeyDetails
    {
        RdkShellEasterEggKeyDetails() {}
        RdkShellEasterEggKeyDetails(uint32_t code, uint32_t modifiers, uint32_t holdTime): keyCode(code), keyModifiers(modifiers), keyHoldTime(holdTime) {}
        uint32_t keyCode;
        uint32_t keyModifiers;
        uint32_t keyHoldTime;
    };

    struct RdkShellEasterEggDetails
    {
        std::string id;
        std::vector<RdkShellEasterEggKeyDetails> keySequence;
        uint32_t duration;
        std::string api;
    };

    class RdkShellApiImpl
    {
            public:
                    virtual bool systemRam(uint32_t& freeKb, uint32_t& totalKb, uint32_t& availableKb, uint32_t& usedSwapKb) = 0;
                    virtual void setMemoryMonitor(std::map<std::string, RdkShellData> &configuration) = 0;
                    virtual void addEasterEgg(std::vector<RdkShellEasterEggKeyDetails>& details, std::string name, uint32_t timeout, std::string actionJson) = 0;
                    virtual void removeEasterEgg(std::string name) = 0;
                    virtual void getEasterEggs(std::vector<RdkShellEasterEggDetails>& easterEggs) = 0;
                    virtual void initialize() = 0;
                    virtual void update() = 0;
                    virtual void draw() = 0;
                    virtual void deinitialize() = 0;
                    virtual double microseconds() = 0;
                    virtual double milliseconds() = 0;
                    virtual double seconds() = 0;
    };

    class LoggerImpl
    {
            public:
                    virtual void enableFlushing(bool enable) = 0;
                    virtual bool isFlushingEnabled() = 0;
    };

    class Logger
    {
        protected:
            static LoggerImpl* impl;
        public:
            Logger();
            Logger(const Logger &obj) = delete; // deleted copy constructor so that copy of the instance cannot be created.
            static void setImpl(LoggerImpl* newImpl);
            static void enableFlushing(bool enable);
            static bool isFlushingEnabled();
    };

    class RdkShellApi
    {
      protected:
         static RdkShellApiImpl* impl;

      public:
          RdkShellApi();
          RdkShellApi(const RdkShellApi& obj) = delete;

          static void setImpl(RdkShellApiImpl* newImpl);

          static void addEasterEgg(std::vector<RdkShellEasterEggKeyDetails>& details, std::string name, uint32_t timeout, std::string actionJson);
          static void removeEasterEgg(std::string name);
          static void getEasterEggs(std::vector<RdkShellEasterEggDetails>& easterEggs);
          static void initialize();
          static void update();
          static void draw();
          static void deinitialize();
          static double seconds();
          static double milliseconds();
          static double microseconds();
          static bool systemRam(uint32_t& freeKb, uint32_t& totalKb, uint32_t& availableKb, uint32_t& usedSwapKb);
          static void setMemoryMonitor(std::map<std::string, RdkShellData> &configuration);
    };

    extern void (*addEasterEgg)(std::vector<RdkShellEasterEggKeyDetails>&, std::string, uint32_t, std::string);
    extern void (*removeEasterEgg)(std::string);
    extern void (*getEasterEggs)(std::vector<RdkShellEasterEggDetails>&);
    extern void (*initialize)();
    extern void (*update)();
    extern void (*draw)();
    extern void (*deinitialize)();
    extern double (*seconds)();
    extern double (*milliseconds)();
    extern double (*microseconds)();
    extern bool (*systemRam)(uint32_t&, uint32_t&, uint32_t&, uint32_t&);
    extern void (*setMemoryMonitor)(std::map<std::string, RdkShellData>&);

    #define RDKSHELL_APPLICATION_MIME_TYPE_NATIVE "application/native"
    #define RDKSHELL_APPLICATION_MIME_TYPE_DAC_NATIVE "application/dac.native"
    #define RDKSHELL_APPLICATION_MIME_TYPE_HTML "application/html"
    #define RDKSHELL_APPLICATION_MIME_TYPE_LIGHTNING "application/lightning"

  class CompositorControllerImpl
  {
    public:
     virtual bool setLogLevel(const std::string level) = 0;
     virtual bool isErmEnabled() = 0;
     virtual bool scaleToFit(const std::string& client, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height) = 0;
     virtual bool ignoreKeyInputs(bool ignore) = 0;
     virtual bool moveToFront(const std::string& client) = 0;
     virtual bool moveToBack(const std::string& client) = 0;
     virtual bool getClients(std::vector<std::string>& clients) = 0;
     virtual bool moveBehind(const std::string& client, const std::string& target) = 0;
     virtual bool getOpacity(const std::string& client, unsigned int& opacity) = 0;
     virtual bool setOpacity(const std::string& client, const unsigned int opacity) = 0;
     virtual bool setFocus(const std::string& client) = 0;
     virtual bool removeKeyIntercept(const std::string& client, const uint32_t& keyCode, const uint32_t& flags) = 0;
     virtual bool removeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags) = 0;
     virtual bool removeKeyMetadataListener(const std::string& client) = 0;
     virtual bool injectKey(const uint32_t& keyCode, const uint32_t& flags) = 0;
     virtual bool getScreenResolution(uint32_t &width, uint32_t &height) = 0;
     virtual bool setScreenResolution(const uint32_t width, const uint32_t height) = 0;
     virtual bool getVirtualResolution(const std::string& client, uint32_t &virtualWidth, uint32_t &virtualHeight) = 0;
     virtual bool setVirtualResolution(const std::string& client, const uint32_t virtualWidth, const uint32_t virtualHeight) = 0;
     virtual bool getZOrder(std::vector<std::string>&clients) = 0;
     virtual bool setVisibility(const std::string& client, const bool visible) = 0;
     virtual bool getVisibility(const std::string& client, bool& visible) = 0;
     virtual bool getHolePunch(const std::string& client, bool& holePunch) = 0;
     virtual bool setHolePunch(const std::string& client, const bool holePunch) = 0;
     virtual bool getBounds(const std::string& client, uint32_t &x, uint32_t &y, uint32_t &width, uint32_t &height) = 0;
     virtual bool setBounds(const std::string& client, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height ) = 0;
     virtual bool getScale(const std::string& client, double &scaleX, double &scaleY) = 0;
     virtual bool setScale(const std::string& client, double scaleX, double scaleY) = 0;
     virtual bool setCursorSize(uint32_t width, uint32_t height) = 0;
     virtual bool getCursorSize(uint32_t& width, uint32_t& height) = 0;
     virtual bool getFocused(std::string& client) = 0;
     virtual void enableInactivityReporting(const bool enable) = 0;
     virtual bool showCursor() = 0;
     virtual bool hideCursor() = 0;
     virtual void setKeyRepeatConfig(bool enabled, int32_t initialDelay, int32_t repeatInterval) = 0;
     virtual bool getBlockedAVApplications(std::vector<std::string>& apps) = 0;
     virtual bool setAVBlocked(std::string callsign, bool blockAV) = 0;
     virtual bool enableInputEvents(const std::string& client, bool enable) = 0;
     virtual bool getLogLevel(std::string& level) = 0;
     virtual bool enableKeyRepeats(bool enable) = 0;
     virtual bool getKeyRepeatsEnabled(bool& enable) = 0;
     virtual bool screenShot(uint8_t* &data, uint32_t &size) = 0;
     virtual bool getLastKeyPress(uint32_t &keyCode, uint32_t &modifiers, uint64_t &timestampInSeconds) = 0;
     virtual bool getVirtualDisplayEnabled(const std::string& client, bool &enabled) = 0;
     virtual bool enableVirtualDisplay(const std::string& client, const bool enable) = 0;
     virtual bool isSurfaceModeEnabled() = 0;
     virtual bool getTopmost(std::string& client) = 0;
     virtual bool setTopmost(const std::string& client, bool topmost, bool focus = false) = 0;
     virtual bool showFullScreenImage(std::string file) = 0;
     virtual bool hideFullScreenImage() = 0;
     virtual bool showWatermark() = 0;
     virtual bool hideWatermark() = 0;
     virtual bool showSplashScreen(uint32_t displayTimeInSeconds) = 0;
     virtual bool hideSplashScreen() = 0;
     virtual bool setMimeType(const std::string& client, const std::string& mimeType) = 0;
     virtual bool getMimeType(const std::string& client, std::string& mimeType) = 0;
     virtual bool launchApplication(const std::string& client, const std::string& uri, const std::string& mimeType, bool topmost = false, bool focus = false) = 0;
     virtual bool suspendApplication(const std::string& client) = 0;
     virtual bool resumeApplication(const std::string& client) = 0;
     virtual void setEventListener(std::shared_ptr<RdkShellEventListener> listener) = 0;
     virtual void resetInactivityTime() = 0;
     virtual bool removeListener(const std::string& client, std::shared_ptr<RdkShellEventListener> listener) = 0;
     virtual bool addListener(const std::string& client, std::shared_ptr<RdkShellEventListener> listener) = 0;
     virtual bool addAnimation(const std::string& client, double duration, std::map<std::string, RdkShellData> &animationProperties) = 0;
     virtual bool removeAnimation(const std::string& client) = 0;
     virtual bool createDisplay(const std::string& client, const std::string& displayName, uint32_t displayWidth=0, uint32_t displayHeight=0,
                bool virtualDisplayEnabled=false, uint32_t virtualWidth=0, uint32_t virtualHeight=0, bool topmost = false, bool focus = false , bool autodestroy = true) = 0;
     virtual bool generateKey(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey="") = 0;
      virtual bool generateKey(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey="", double duration) = 0;
     virtual bool addKeyMetadataListener(const std::string& client) = 0;
     virtual bool removeNativeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags) = 0;
     virtual bool addNativeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::map<std::string, RdkShellData> &listenerProperties) = 0;
     virtual bool addKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::map<std::string, RdkShellData> &listenerProperties) = 0;
     virtual bool addKeyIntercept(const std::string& client, const uint32_t& keyCode, const uint32_t& flags) = 0;
     virtual bool kill(const std::string& client) = 0;
     virtual void setInactivityInterval(const double minutes) = 0;
  };


    class CompositorController
    {
            protected:
                    static CompositorControllerImpl* impl;
            public:

                    CompositorController();
                    CompositorController(const CompositorController &obj) = delete; // deleted copy constructor so that copy of the instance cannot be created.
                    static void setImpl(CompositorControllerImpl* newImpl);
                    static bool setVisibility(const std::string& client, const bool visible);
                    static bool moveToFront(const std::string& client);
                    static bool moveToBack(const std::string& client);
                    static bool moveBehind(const std::string& client, const std::string& target);
                    static bool setFocus(const std::string& client);
                    static bool getFocused(std::string& client);
                    static bool kill(const std::string& client);
                    static bool addKeyIntercept(const std::string& client, const uint32_t& keyCode, const uint32_t& flags);
                    static bool removeKeyIntercept(const std::string& client, const uint32_t& keyCode, const uint32_t& flags);
                    static bool addKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::map<std::string, RdkShellData> &listenerProperties);
                    static bool addNativeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::map<std::string, RdkShellData> &listenerProperties);
                    static bool removeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags);
                    static bool removeNativeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags);
                    static bool addKeyMetadataListener(const std::string& client);
                    static bool removeKeyMetadataListener(const std::string& client);
                    static bool injectKey(const uint32_t& keyCode, const uint32_t& flags);
                    static bool generateKey(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey="");
                    static bool generateKey(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey="", double duration);
                    static bool getScreenResolution(uint32_t &width, uint32_t &height);
                    static bool setScreenResolution(const uint32_t width, const uint32_t height);
                    static bool getClients(std::vector<std::string>& clients);
                    static bool getZOrder(std::vector<std::string>&clients);
                    static bool getBounds(const std::string& client, uint32_t &x, uint32_t &y, uint32_t &width, uint32_t &height);
                    static bool setBounds(const std::string& client, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height );
                    static bool getVisibility(const std::string& client, bool& visible);
                    static bool getOpacity(const std::string& client, unsigned int& opacity);
                    static bool setOpacity(const std::string& client, const unsigned int opacity);
                    static bool getScale(const std::string& client, double &scaleX, double &scaleY);
                    static bool setScale(const std::string& client, double scaleX, double scaleY);
                    static bool getHolePunch(const std::string& client, bool& holePunch);
                    static bool setHolePunch(const std::string& client, const bool holePunch);
                    static bool scaleToFit(const std::string& client, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height);
                    static bool createDisplay(const std::string& client, const std::string& displayName, uint32_t displayWidth = 0, uint32_t displayHeight = 0,
                    bool virtualDisplayEnabled = false, uint32_t virtualWidth = 0, uint32_t virtualHeight = 0, bool topmost = false, bool focus = false, bool autodestroy = true);
                    static bool addAnimation(const std::string& client, double duration, std::map<std::string, RdkShellData>& animationProperties);
                    static bool removeAnimation(const std::string& client);
                    static bool addListener(const std::string& client, std::shared_ptr<RdkShellEventListener> listener);
                    static bool removeListener(const std::string& client, std::shared_ptr<RdkShellEventListener> listener);
                    static void enableInactivityReporting(const bool enable);
                    static void setInactivityInterval(const double minutes);
                    static void resetInactivityTime();
                    static void setEventListener(std::shared_ptr<RdkShellEventListener> listener);
                    static bool launchApplication(const std::string& client, const std::string& uri, const std::string& mimeType, bool topmost = false, bool focus = false);
                    static bool suspendApplication(const std::string& client);
                    static bool resumeApplication(const std::string& client);
                    static bool getMimeType(const std::string& client, std::string& mimeType);
                    static bool setMimeType(const std::string& client, const std::string& mimeType);
                    static bool hideSplashScreen();
                    static bool showSplashScreen(uint32_t displayTimeInSeconds);
                    static bool hideWatermark();
                    static bool showWatermark();
                    static bool hideFullScreenImage();
                    static bool showFullScreenImage(std::string file);
                    static bool setLogLevel(const std::string level);
                    static bool getLogLevel(std::string& level);
                    static bool setTopmost(const std::string& client, bool topmost, bool focus = false);
                    static bool getTopmost(std::string& client);
                    static bool isSurfaceModeEnabled();
                    static bool enableKeyRepeats(bool enable);
                    static bool getKeyRepeatsEnabled(bool& enable);
                    static bool getVirtualResolution(const std::string& client, uint32_t& virtualWidth, uint32_t& virtualHeight);
                    static bool setVirtualResolution(const std::string& client, const uint32_t virtualWidth, const uint32_t virtualHeight);
                    static bool enableVirtualDisplay(const std::string& client, const bool enable);
                    static bool getVirtualDisplayEnabled(const std::string& client, bool& enabled);
                    static bool getLastKeyPress(uint32_t& keyCode, uint32_t& modifiers, uint64_t& timestampInSeconds);
                    static bool ignoreKeyInputs(bool ignore);
                    static bool screenShot(uint8_t*& data, uint32_t& size);
                    static bool enableInputEvents(const std::string& client, bool enable);
                    static bool showCursor();
                    static bool hideCursor();
                    static bool setCursorSize(uint32_t width, uint32_t height);
                    static bool getCursorSize(uint32_t& width, uint32_t& height);
                    static void setKeyRepeatConfig(bool enabled, int32_t initialDelay, int32_t repeatInterval);
                    static bool setAVBlocked(std::string callsign, bool blockAV);
                    static bool getBlockedAVApplications(std::vector<std::string>& apps);
                    static bool isErmEnabled();
    };


}

