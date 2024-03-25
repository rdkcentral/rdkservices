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
public:

    static RDKShell& getInstance()
    {
        static RDKShell instance;
        return instance;
    }

    RDKShellImpl* impl;

    static bool keyCodeFromWayland(uint32_t waylandKeyCode, uint32_t waylandFlags, uint32_t &mappedKeyCode, uint32_t &mappedFlags)
    {
	    return getInstance().impl->keyCodeFromWayland(waylandKeyCode, waylandFlags, mappedKeyCode, mappedFlags);
    }

};
constexpr auto keyCodeFromWayland = &RDKShell::keyCodeFromWayland;
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
        public:
          static RdkShellEventListener& getInstance()
          {
           static RdkShellEventListener instance;
           return instance;
          }

          RdkShellEvent* impl;	  
          virtual void onApplicationLaunched(const std::string& client)
	  {
		  getInstance().impl->onApplicationLaunched(client);
	  }
          virtual void onApplicationConnected(const std::string& client)
	  {
		  getInstance().impl->onApplicationConnected(client);
	  }
          virtual void onApplicationDisconnected(const std::string& client) 
	  {
		  getInstance().impl->onApplicationDisconnected(client);
	  }
          virtual void onApplicationTerminated(const std::string& client)
	  {
		  getInstance().impl->onApplicationTerminated(client);
	  }
          virtual void onApplicationFirstFrame(const std::string& client)
	  {
		  getInstance().impl->onApplicationFirstFrame(client);
	  }
          virtual void onApplicationSuspended(const std::string& client)
	  {
                 getInstance().impl->onApplicationSuspended(client);
	  }
          virtual void onApplicationResumed(const std::string& client)
	  {
		  getInstance().impl->onApplicationResumed(client);
	  }
          virtual void onApplicationActivated(const std::string& client)
	  {
		  getInstance().impl->onApplicationActivated(client);
	  }
          virtual void onUserInactive(const double minutes)
	  {
		  getInstance().impl->onUserInactive(minutes);
	  }
          virtual void onDeviceLowRamWarning(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb) 
	  {
		  getInstance().impl->onDeviceLowRamWarning(freeKb, availableKb, usedSwapKb);
	  }
          virtual void onDeviceCriticallyLowRamWarning(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb) 
	  {
		  getInstance().impl->onDeviceCriticallyLowRamWarning(freeKb, availableKb, usedSwapKb);
	  }
          virtual void onDeviceLowRamWarningCleared(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb)
	  {
		  getInstance().impl->onDeviceLowRamWarningCleared(freeKb, availableKb, usedSwapKb);
	  }
          virtual void onDeviceCriticallyLowRamWarningCleared(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb)
	  {
		  getInstance().impl->onDeviceCriticallyLowRamWarningCleared(freeKb, availableKb, usedSwapKb);
	  }
          virtual void onEasterEgg(const std::string& name, const std::string& actionJson)
	  {
		  getInstance().impl->onEasterEgg(name, actionJson);
          }
          virtual void onPowerKey() 
	  {
		  getInstance().impl->onPowerKey();
	  }
          virtual void onSizeChangeComplete(const std::string& client) 
	  {
		 getInstance().impl->onSizeChangeComplete(client);
	  }
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
        public:
	  static Logger& getInstance()
          {
           static Logger instance;
           return instance;
          }

            LoggerImpl* impl;
            static void enableFlushing(bool enable)
            {
		getInstance().impl->enableFlushing(enable);
	    }
            static bool isFlushingEnabled()
	    {
	        return getInstance().impl->isFlushingEnabled();
	    }
    };

    class RdkShellApi
    {
      public:
	  static RdkShellApi& getInstance()
          {
           static RdkShellApi instance;
           return instance;
          }

          RdkShellApiImpl* impl;

          static void addEasterEgg(std::vector<RdkShellEasterEggKeyDetails>& details, std::string name, uint32_t timeout, std::string actionJson)
	  {
		  getInstance().impl->addEasterEgg(details, name, timeout, actionJson);
	  }
          static void removeEasterEgg(std::string name)
	  {
		  getInstance().impl->removeEasterEgg(name);
	  }
          static void getEasterEggs(std::vector<RdkShellEasterEggDetails>& easterEggs)
	  {
		  getInstance().impl->getEasterEggs(easterEggs);
	  }
          static void initialize()
	  {
		  getInstance().impl->initialize();
	  }
          static void update()
	  {
		  getInstance().impl->update();
	  }
          static void draw()
	  {
		  getInstance().impl->draw();
	  }
          static void deinitialize()
	  {
		  getInstance().impl->deinitialize();
	  }
          static double seconds()
	  {
		  return getInstance().impl->seconds();
	  }
          static double milliseconds()
	  {
		  return getInstance().impl->milliseconds();
	  }
          static double microseconds()
	  {
		  return getInstance().impl->microseconds();
	  }
          static bool systemRam(uint32_t& freeKb, uint32_t& totalKb, uint32_t& availableKb, uint32_t& usedSwapKb)
	  {
		  return getInstance().impl->systemRam(freeKb, totalKb, availableKb, usedSwapKb);
	  }
          static void setMemoryMonitor(std::map<std::string, RdkShellData> &configuration)
	  {
		  getInstance().impl->setMemoryMonitor(configuration);
	  }
    };
    constexpr auto addEasterEgg = &RdkShellApi::addEasterEgg;
    constexpr auto removeEasterEgg = &RdkShellApi::removeEasterEgg;
    constexpr auto getEasterEggs = &RdkShellApi::getEasterEggs;
    constexpr auto initialize = &RdkShellApi::initialize;
    constexpr auto deinitialize = &RdkShellApi::deinitialize;
    constexpr auto update = &RdkShellApi::update;
    constexpr auto draw = &RdkShellApi::draw;
    constexpr auto seconds = &RdkShellApi::seconds;
    constexpr auto milliseconds = &RdkShellApi::milliseconds;
    constexpr auto microseconds = &RdkShellApi::microseconds;
    constexpr auto systemRam = &RdkShellApi::systemRam;
    constexpr auto setMemoryMonitor = &RdkShellApi::setMemoryMonitor;

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
     virtual bool generateKey(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey, double duration) = 0;
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
        public:
	
          static CompositorController& getInstance()
          {
           static CompositorController instance;
           return instance;
          }

	    CompositorControllerImpl* impl;
	   
	    static bool setVisibility(const std::string& client, const bool visible)
	    {
               return getInstance().impl->setVisibility(client, visible);
             }
            static bool moveToFront(const std::string& client)
            {
               return getInstance().impl->moveToFront(client);
	    }
            static bool moveToBack(const std::string& client)
            {
	        return getInstance().impl->moveToBack(client);
	    }
            static bool moveBehind(const std::string& client, const std::string& target)
	    {
		return getInstance().impl->moveBehind(client, target);
	    }
            static bool setFocus(const std::string& client)
	    {
		return getInstance().impl->setFocus(client);
	    }
            static bool getFocused(std::string& client)
	    {
		return getInstance().impl->getFocused(client);
	    }
            static bool kill(const std::string& client)
	    {
		    return getInstance().impl->kill(client);
	    }
            static bool addKeyIntercept(const std::string& client, const uint32_t& keyCode, const uint32_t& flags)
	    {
		    return getInstance().impl->addKeyIntercept(client, keyCode, flags);
	    }
            static bool removeKeyIntercept(const std::string& client, const uint32_t& keyCode, const uint32_t& flags)
	    {
                return getInstance().impl->removeKeyIntercept(client, keyCode, flags);
	    }
            static bool addKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::map<std::string, RdkShellData> &listenerProperties)
	    {
		    return getInstance().impl->addKeyListener(client, keyCode, flags, listenerProperties);
	    }
            static bool addNativeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::map<std::string, RdkShellData> &listenerProperties)
	    {
		    return getInstance().impl->addNativeKeyListener(client, keyCode, flags, listenerProperties);
	    }
            static bool removeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags)
	    {
		    return getInstance().impl->removeKeyListener(client, keyCode, flags);
	    }
            static bool removeNativeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags)
	    {
		    return getInstance().impl->removeNativeKeyListener(client, keyCode, flags);
	    }
            static bool addKeyMetadataListener(const std::string& client)
	    {
		    return getInstance().impl->addKeyMetadataListener(client);
	    }
            static bool removeKeyMetadataListener(const std::string& client)
            {
		    return getInstance().impl->removeKeyMetadataListener(client);
	    }
            static bool injectKey(const uint32_t& keyCode, const uint32_t& flags)
	    {
		    return getInstance().impl->injectKey(keyCode, flags);
	    }
            static bool generateKey(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey="")
	    {
		    return getInstance().impl->generateKey(client, keyCode, flags, virtualKey);
	    }
	    static bool generateKey(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey, double duration)
            {
                    return getInstance().impl->generateKey(client, keyCode, flags, virtualKey, duration);
            }
            static bool getScreenResolution(uint32_t &width, uint32_t &height)
	    {
		    return getInstance().impl->getScreenResolution(width,height);
	    }
            static bool setScreenResolution(const uint32_t width, const uint32_t height)
	    {
		    return getInstance().impl->setScreenResolution(width,height);
	    }
            static bool getClients(std::vector<std::string>& clients)
	    {
		    return getInstance().impl->getClients(clients);
	    }
            static bool getZOrder(std::vector<std::string>&clients)
	    {
		    return getInstance().impl->getZOrder(clients);
	    }
            static bool getBounds(const std::string& client, uint32_t &x, uint32_t &y, uint32_t &width, uint32_t &height)
            {
		    return getInstance().impl->getBounds(client, x, y, width, height);
            }
            static bool setBounds(const std::string& client, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height )
            {
		    return getInstance().impl->setBounds(client, x, y, width, height);
            }
            static bool getVisibility(const std::string& client, bool& visible)
	    {
		    return getInstance().impl->getVisibility(client, visible);
	    }
            static bool getOpacity(const std::string& client, unsigned int& opacity)
	    {
		    return getInstance().impl->getOpacity(client, opacity);
	    }
            static bool setOpacity(const std::string& client, const unsigned int opacity)
	    {
		    return getInstance().impl->setOpacity(client, opacity);
	    }
            static bool getScale(const std::string& client, double &scaleX, double &scaleY)
	    {
		    return getInstance().impl->getScale(client, scaleX, scaleY);
	    }
            static bool setScale(const std::string& client, double scaleX, double scaleY)
	    {
		    return getInstance().impl->setScale(client, scaleX, scaleY);
	    }
            static bool getHolePunch(const std::string& client, bool& holePunch)
	    {
		    return getInstance().impl->getHolePunch(client, holePunch);
	    }
            static bool setHolePunch(const std::string& client, const bool holePunch)
	    {
		    return getInstance().impl->setHolePunch(client, holePunch);
	    }
            static bool scaleToFit(const std::string& client, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height)
	    {
                 return getInstance().impl->scaleToFit(client,x,y,width,height);
	    }
            static bool createDisplay(const std::string& client, const std::string& displayName, uint32_t displayWidth=0, uint32_t displayHeight=0,
                bool virtualDisplayEnabled=false, uint32_t virtualWidth=0, uint32_t virtualHeight=0, bool topmost = false, bool focus = false , bool autodestroy = true)
	    {
                 return getInstance().impl->createDisplay(client, displayName, displayWidth, displayHeight, virtualDisplayEnabled, virtualWidth, virtualHeight, topmost, focus, autodestroy);
	    }
            static bool addAnimation(const std::string& client, double duration, std::map<std::string, RdkShellData> &animationProperties)
	    {
		    return getInstance().impl->addAnimation(client, duration, animationProperties);
	    }
            static bool removeAnimation(const std::string& client)
	    {
		    return getInstance().impl->removeAnimation(client);
	    }
            static bool addListener(const std::string& client, std::shared_ptr<RdkShellEventListener> listener)
	    {
		    return getInstance().impl->addListener(client, listener);
	    }
            static bool removeListener(const std::string& client, std::shared_ptr<RdkShellEventListener> listener)
	    {
		    return getInstance().impl->removeListener(client, listener);
	    }
            static void enableInactivityReporting(const bool enable)
	    {
		    getInstance().impl->enableInactivityReporting(enable);
	    }
            static void setInactivityInterval(const double minutes)
	    {
		    getInstance().impl->setInactivityInterval(minutes);
	    }
            static void resetInactivityTime()
	    {
		    getInstance().impl->resetInactivityTime();
	    }
            static void setEventListener(std::shared_ptr<RdkShellEventListener> listener)
	    {
		    getInstance().impl->setEventListener(listener);
	    }
            static bool launchApplication(const std::string& client, const std::string& uri, const std::string& mimeType, bool topmost = false, bool focus = false)
	    {
		    return getInstance().impl->launchApplication(client, uri, mimeType, topmost, focus);
	    }
            static bool suspendApplication(const std::string& client)
	    {
		    return getInstance().impl->suspendApplication(client);
	    }
            static bool resumeApplication(const std::string& client)
	    {
		    return getInstance().impl->resumeApplication(client);
	    }
            static bool getMimeType(const std::string& client, std::string& mimeType)
	    {
		    return getInstance().impl->getMimeType(client, mimeType);
	    }
            static bool setMimeType(const std::string& client, const std::string& mimeType)
	    {
		    return getInstance().impl->setMimeType(client, mimeType);
	    }
            static bool hideSplashScreen()
	    {
		    return getInstance().impl->hideSplashScreen();
	    }
            static bool showSplashScreen(uint32_t displayTimeInSeconds)
	    {
		    return getInstance().impl->showSplashScreen(displayTimeInSeconds);
	    }
            static bool hideWatermark()
	    {
		    return getInstance().impl->hideWatermark();
	    }
            static bool showWatermark()
	    {
		    return getInstance().impl->showWatermark();
	    }
            static bool hideFullScreenImage()
	    {
		    return getInstance().impl->hideFullScreenImage();
	    }
            static bool showFullScreenImage(std::string file)
	    {
		    return getInstance().impl->showFullScreenImage(file);
	    }
            static bool setLogLevel(const std::string level)
            {
		    return getInstance().impl->setLogLevel(level);
            }
            static bool getLogLevel(std::string& level)
            {
                    return getInstance().impl->getLogLevel(level);
            }
            static bool setTopmost(const std::string& client, bool topmost, bool focus = false)
	    {
		    return getInstance().impl->setTopmost(client, topmost, focus);
	    }
            static bool getTopmost(std::string& client)
	    {
		    return getInstance().impl->getTopmost(client);
	    }
            static bool isSurfaceModeEnabled()
	    {
		    return getInstance().impl->isSurfaceModeEnabled();
	    }
            static bool enableKeyRepeats(bool enable)
	    {
		    return getInstance().impl->enableKeyRepeats(enable);
	    }
            static bool getKeyRepeatsEnabled(bool& enable)
	    {
		    return getInstance().impl->getKeyRepeatsEnabled(enable);
	    }
            static bool getVirtualResolution(const std::string& client, uint32_t &virtualWidth, uint32_t &virtualHeight)
            {
		    return getInstance().impl->getVirtualResolution(client, virtualWidth, virtualHeight);
            }
            static bool setVirtualResolution(const std::string& client, const uint32_t virtualWidth, const uint32_t virtualHeight)
            {
		    return getInstance().impl->setVirtualResolution(client, virtualWidth, virtualHeight);
            }
            static bool enableVirtualDisplay(const std::string& client, const bool enable)
            {
		    return getInstance().impl->enableVirtualDisplay(client, enable);
	    }
            static bool getVirtualDisplayEnabled(const std::string& client, bool &enabled)
            {
		    return getInstance().impl->getVirtualDisplayEnabled(client, enabled);
	    }
            static bool getLastKeyPress(uint32_t &keyCode, uint32_t &modifiers, uint64_t &timestampInSeconds)
	    {
		    return getInstance().impl->getLastKeyPress(keyCode, modifiers, timestampInSeconds);
	    }
            static bool ignoreKeyInputs(bool ignore)
	    {
                 return getInstance().impl->ignoreKeyInputs(ignore);
	    }
            static bool screenShot(uint8_t* &data, uint32_t &size)
	    {
		    return getInstance().impl->screenShot(data, size);
	    }
            static bool enableInputEvents(const std::string& client, bool enable)
	    {
		    return getInstance().impl->enableInputEvents(client, enable);
	    }
            static bool showCursor()
            {
                    return getInstance().impl->showCursor();
            }
            static bool hideCursor()
            {
                    return getInstance().impl->hideCursor();
            }
            static bool setCursorSize(uint32_t width, uint32_t height)
            {
		    return getInstance().impl->setCursorSize(width, height);
            }
            static bool getCursorSize(uint32_t& width, uint32_t& height)
            {
		    return getInstance().impl->getCursorSize(width, height);
            }
            static void setKeyRepeatConfig(bool enabled, int32_t initialDelay, int32_t repeatInterval)
	    {
		    getInstance().impl->setKeyRepeatConfig(enabled, initialDelay, repeatInterval);
	    }
            static bool setAVBlocked(std::string callsign, bool blockAV)
	    {
		    return getInstance().impl->setAVBlocked(callsign, blockAV);
	    }
            static bool getBlockedAVApplications(std::vector<std::string>& apps)
	    {
		    return getInstance().impl->getBlockedAVApplications(apps);
	    }
            static bool isErmEnabled()
	    {
		    return getInstance().impl->isErmEnabled();
	    }
    };


}

