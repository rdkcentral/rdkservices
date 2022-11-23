#pragma once
#include <stdint.h>
#include <typeindex>
#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <ctime>
#include <sys/types.h>
#include <thread>
#include <mutex>
#include <memory>
#define RDKSHELL_FPS 40
#define RDKSHELL_SPLASH_SCREEN_FILE_CHECK "/tmp/.rdkshellsplash"
#define RDKSHELL_FLAGS_SHIFT        8
#define RDKSHELL_FLAGS_CONTROL      16
#define RDKSHELL_FLAGS_ALT          32
#ifndef COMMANDS_H_
#define COMMANDS_H
static int gCurrentFramerate = RDKSHELL_FPS;
#endif
class shell {
public:
    shell()
    {
    }
    virtual ~shell() = default;
    virtual size_t b64_get_encoded_buffer_size( const size_t decoded_size ) = 0;
    virtual size_t b64url_get_encoded_buffer_size( const size_t decoded_size ) = 0;
    virtual void b64_encode( const uint8_t *input, const size_t input_size, uint8_t *output ) = 0;
    virtual void logMilestone(const char *msg_code) = 0;
    virtual bool keyCodeFromWayland(uint32_t waylandKeyCode, uint32_t waylandFlags, uint32_t &mappedKeyCode, uint32_t &mappedFlags) = 0;
};

class Shell {
public:

    static Shell& getInstance()
    {
        static Shell instance;
        return instance;
    }

    shell* impl;

    static size_t b64_get_encoded_buffer_size( const size_t decoded_size )
    {
	    return getInstance().impl->b64_get_encoded_buffer_size(decoded_size);
    }
    
    static size_t b64url_get_encoded_buffer_size( const size_t decoded_size )
    {
            return getInstance().impl->b64url_get_encoded_buffer_size(decoded_size);
    }
  
    static void b64_encode( const uint8_t *input, const size_t input_size, uint8_t *output )
    {
            return getInstance().impl->b64_encode(input, input_size, output);
    }
    static void logMilestone(const char *msg_code)
    {
	    return getInstance().impl->logMilestone(msg_code);
    }
    static bool keyCodeFromWayland(uint32_t waylandKeyCode, uint32_t waylandFlags, uint32_t &mappedKeyCode, uint32_t &mappedFlags)
    {
	    return getInstance().impl->keyCodeFromWayland(waylandKeyCode, waylandFlags, mappedKeyCode, mappedFlags);
    }

};
constexpr auto b64_get_encoded_buffer_size = &Shell::b64_get_encoded_buffer_size;
constexpr auto b64url_get_encoded_buffer_size = &Shell::b64url_get_encoded_buffer_size;
constexpr auto b64_encode = &Shell::b64_encode;
constexpr auto logMilestone = &Shell::logMilestone;
constexpr auto keyCodeFromWayland = &Shell::keyCodeFromWayland;
namespace RdkShell
{
    union RdkShellDataInfo
    {
        bool        booleanData;
        int8_t      integer8Data;
        int32_t     integer32Data;
        int64_t     integer64Data;
        uint8_t     unsignedInteger8Data;
        uint32_t    unsignedInteger32Data;
        uint64_t    unsignedInteger64Data;
        float       floatData;
        double      doubleData;
        std::string* stringData;
        void*       pointerData;
        RdkShellDataInfo() {}
        ~RdkShellDataInfo() {}
    };

    class RdkShellData
    {
        public:
            RdkShellData(): mDataTypeIndex(typeid(void*))
           {
                   mData.pointerData = nullptr;
                   mData.stringData = nullptr;
           }
            ~RdkShellData()
            {
              if (mDataTypeIndex == typeid(std::string))
              {
                if (nullptr != mData.stringData)
               {
                 delete mData.stringData;
                 mData.stringData = nullptr;
               }
              }
            }
            RdkShellData(bool data);
            RdkShellData(int8_t data);
            RdkShellData(int32_t data);
            RdkShellData(int64_t data);
            RdkShellData(uint8_t data);
            RdkShellData(uint32_t data);
            RdkShellData(uint64_t data);
            RdkShellData(float data);
            RdkShellData(double data);
            RdkShellData(std::string data);
            RdkShellData(void* data);

            bool toBoolean() const{return true;}
            int8_t toInteger8() const{return 0;}
            int32_t toInteger32() const{return 0;}
            int64_t toInteger64() const{return 0;}
            uint8_t toUnsignedInteger8() const{return 0;}
            uint32_t toUnsignedInteger32() const{return 0;}
            uint64_t toUnsignedInteger64() const{return 0;}
            float toFloat() const{return 0.0;}
            double toDouble() const{return 0.0;}
            std::string toString() const{return "a";}
            void* toVoidPointer() const{return NULL;}

            RdkShellData& operator=(bool value){return *this;}
            RdkShellData& operator=(int8_t value){return *this;}
            RdkShellData& operator=(int32_t value){return *this;}
            RdkShellData& operator=(int64_t value){return *this;}
            RdkShellData& operator=(uint8_t value){return *this;}
            RdkShellData& operator=(uint32_t value){return *this;}
            RdkShellData& operator=(uint64_t value){return *this;}
            RdkShellData& operator=(float value){return *this;}
            RdkShellData& operator=(double value){return *this;}
            RdkShellData& operator=(const char* value){return *this;}
            RdkShellData& operator=(const std::string& value){return *this;}
            RdkShellData& operator=(void* value){return *this;}
            RdkShellData& operator=(const RdkShellData& value){return *this;}

            std::type_index dataTypeIndex();

        private:
            std::type_index mDataTypeIndex;
            RdkShellDataInfo mData;

            void setData(std::type_index typeIndex, void* data){return;}
    };
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
   enum LogLevel {
        Debug,
        Information,
        Warn,
        Error,
        Fatal
    };

    class Logger
    {
        public:
            static void log(LogLevel level, const char* format, ...){return;}
            static void logLevel(std::string& level)
           {
                   std::cout << "getcall" << std::endl;
                   level = sLogLevel;
           }
            static void enableFlushing(bool enable){return;}
            static bool isFlushingEnabled(){return true;}
       private:
            static LogLevel sLogLevel;
            static bool sFlushingEnabled;
    };

    static const char* logLevelStrings[] =
    {
      "DEBUG",
      "INFO",
      "WARN",
      "ERROR",
      "FATAL"
    };

    static const int numLogLevels = sizeof(logLevelStrings)/sizeof(logLevelStrings[0]);

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

    class EasterEgg
    {
    };

    class rdk
    {
      public:
      static void populateEasterEggDetails(){return;}
      static void checkEasterEggs(uint32_t keyCode, uint32_t flags, double time){return;}
      static void resolveWaitingEasterEggs(){return;}
      static void addEasterEgg(std::vector<RdkShellEasterEggKeyDetails>& details, std::string name, uint32_t timeout, std::string actionJson){return;}
      static void removeEasterEgg(std::string name){return;}
      static void getEasterEggs(std::vector<RdkShellEasterEggDetails>& easterEggs){return;}
      static void initialize(){return;}
      static void run(){return;}
      static void update(){return;}
      static void draw(){return;}
      static void deinitialize(){return;}
      static double seconds(){return 0.0;}
      static double milliseconds(){return 0.0;}
      static double microseconds(){return 0.0;}
      static bool systemRam(uint32_t& freeKb, uint32_t& totalKb, uint32_t& availableKb, uint32_t& usedSwapKb){return true;}
      static void setMemoryMonitor(std::map<std::string, RdkShellData> &configuration){return;}
    };
    constexpr auto populateEasterEggDetails = &rdk::populateEasterEggDetails;
    constexpr auto checkEasterEggs = &rdk::checkEasterEggs;
    constexpr auto resolveWaitingEasterEggs = &rdk::resolveWaitingEasterEggs;
    constexpr auto addEasterEgg = &rdk::addEasterEgg;
    constexpr auto removeEasterEgg = &rdk::removeEasterEgg;
    constexpr auto getEasterEggs = &rdk::getEasterEggs;
    constexpr auto initialize = &rdk::initialize;
    constexpr auto run = &rdk::run;
    constexpr auto deinitialize = &rdk::deinitialize;
    constexpr auto update = &rdk::update;
    constexpr auto draw = &rdk::draw;
    constexpr auto seconds = &rdk::seconds;
    constexpr auto milliseconds = &rdk::milliseconds;
    constexpr auto microseconds = &rdk::microseconds;
    constexpr auto systemRam = &rdk::systemRam;
    constexpr auto setMemoryMonitor = &rdk::setMemoryMonitor;

    #define RDKSHELL_APPLICATION_MIME_TYPE_NATIVE "application/native"
    #define RDKSHELL_APPLICATION_MIME_TYPE_DAC_NATIVE "application/dac.native"
    #define RDKSHELL_APPLICATION_MIME_TYPE_HTML "application/html"
    #define RDKSHELL_APPLICATION_MIME_TYPE_LIGHTNING "application/lightning"

    enum class ApplicationState
    {
        Unknown,
        Running,
        Suspended,
        Stopped
    };


  class RdkCompositor;

  class CompControl
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
  };


    class CompositorController
    {
        public:
	
          static CompositorController& getInstance()
          {
           static CompositorController instance;
           return instance;
          }

	  CompControl* impl;
	    static bool setVisibility(const std::string& client, const bool visible)
	    {
               return getInstance().impl->setVisibility(client, visible);
             }
            static void initialize() {return;}
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
            static bool kill(const std::string& client){return true;}
            static bool addKeyIntercept(const std::string& client, const uint32_t& keyCode, const uint32_t& flags){return true;}
            static bool removeKeyIntercept(const std::string& client, const uint32_t& keyCode, const uint32_t& flags)
	    {
                return getInstance().impl->removeKeyIntercept(client, keyCode, flags);
	    }
            static bool addKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::map<std::string, RdkShellData> &listenerProperties){return true;}
            static bool addNativeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::map<std::string, RdkShellData> &listenerProperties){return true;}
            static bool removeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags)
	    {
		    return getInstance().impl->removeKeyListener(client, keyCode, flags);
	    }
            static bool removeAllKeyListeners(){return true;}
            static bool removeAllKeyIntercepts(){return true;}
            static bool removeNativeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags){return true;}
            static bool addKeyMetadataListener(const std::string& client){return true;}
            static bool removeKeyMetadataListener(const std::string& client)
            {
		    return getInstance().impl->removeKeyMetadataListener(client);
	    }
            static bool injectKey(const uint32_t& keyCode, const uint32_t& flags)
	    {
		    return getInstance().impl->injectKey(keyCode, flags);
	    }
            static bool generateKey(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey=""){return true;}
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
            static void onKeyPress(uint32_t keycode, uint32_t flags, uint64_t metadata, bool physicalKeyPress=true){return;}
            static void onKeyRelease(uint32_t keycode, uint32_t flags, uint64_t metadata, bool physicalKeyPress=true){return;}
            static void onPointerMotion(uint32_t x, uint32_t y){return;}
            static void onPointerButtonPress(uint32_t keyCode, uint32_t x, uint32_t y){return;}
            static void onPointerButtonRelease(uint32_t keyCode, uint32_t x, uint32_t y){return;}
            static bool createDisplay(const std::string& client, const std::string& displayName, uint32_t displayWidth=0, uint32_t displayHeight=0,
                bool virtualDisplayEnabled=false, uint32_t virtualWidth=0, uint32_t virtualHeight=0, bool topmost = false, bool focus = false , bool autodestroy = true){return true;}
            static bool addAnimation(const std::string& client, double duration, std::map<std::string, RdkShellData> &animationProperties){return true;}
            static bool removeAnimation(const std::string& client){return true;}
            static bool addListener(const std::string& client, std::shared_ptr<RdkShellEventListener> listener){return true;}
            static bool removeListener(const std::string& client, std::shared_ptr<RdkShellEventListener> listener){return true;}
            static bool onEvent(RdkCompositor* eventCompositor, const std::string& eventName){return true;}
            static void enableInactivityReporting(const bool enable){return;}
            static void setInactivityInterval(const double minutes){return;}
            static void resetInactivityTime(){return;}
            static double getInactivityTimeInMinutes(){return 0.0;}
            static void setEventListener(std::shared_ptr<RdkShellEventListener> listener){return;}
            static std::shared_ptr<RdkCompositor> getCompositor(const std::string& displayName){return NULL;}
            static bool launchApplication(const std::string& client, const std::string& uri, const std::string& mimeType, bool topmost = false, bool focus = false){return true;}
            static bool suspendApplication(const std::string& client){return true;}
            static bool resumeApplication(const std::string& client){return true;}
            static bool closeApplication(const std::string& client){return true;}
            static bool getMimeType(const std::string& client, std::string& mimeType){return true;}
            static bool setMimeType(const std::string& client, const std::string& mimeType){return true;}
            static bool hideSplashScreen(){return true;}
            static bool showSplashScreen(uint32_t displayTimeInSeconds){return true;}
            static bool hideWatermark(){return true;}
            static bool showWatermark(){return true;}
            static bool hideFullScreenImage(){return true;}
            static bool showFullScreenImage(std::string file){return true;}
            static bool draw(){return true;}
            static bool update(){return true;}
            static bool setLogLevel(const std::string level)
            {
		    return getInstance().impl->setLogLevel(level);
            }
            static bool getLogLevel(std::string& level)
            {
                    return getInstance().impl->getLogLevel(level);
            }
            static bool setTopmost(const std::string& client, bool topmost, bool focus = false){return true;}
            static bool getTopmost(std::string& client){return true;}
            static bool sendEvent(const std::string& eventName, std::vector<std::map<std::string, RdkShellData>>& data){return true;}
            static bool isSurfaceModeEnabled(){return true;}
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
            static bool enableVirtualDisplay(const std::string& client, const bool enable){return true;}
            static bool getVirtualDisplayEnabled(const std::string& client, bool &enabled){return true;}
            static bool getLastKeyPress(uint32_t &keyCode, uint32_t &modifiers, uint64_t &timestampInSeconds){return true;}
            static bool ignoreKeyInputs(bool ignore)
	    {
                 return getInstance().impl->ignoreKeyInputs(ignore);
	    }
            static bool updateWatermarkImage(uint32_t imageId, int32_t key, int32_t imageSize){return true;}
            static bool createWatermarkImage(uint32_t imageId, uint32_t zorder=0){return true;}
            static bool deleteWatermarkImage(uint32_t imageId){return true;}
            static bool adjustWatermarkImage(uint32_t imageId, uint32_t zorder){return true;}
            static bool alwaysShowWatermarkImageOnTop(bool show=false){return true;}
            static bool screenShot(uint8_t* &data, uint32_t &size){return true;}
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
		    return getInstance().impl->setKeyRepeatConfig(enabled, initialDelay, repeatInterval);
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

