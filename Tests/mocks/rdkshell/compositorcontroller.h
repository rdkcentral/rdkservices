#pragma once
#ifndef RDKSHELL_COMPOSITOR_CONTROLLER_H
#define RDKSHELL_COMPOSITOR_CONTROLLER_H

#include "rdkshelldata.h"

#include "rdkshellevents.h"
#include "rdkcompositor.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include "rdkshell.h"
#include "application.h"
#include "logger.h"
#include "linuxkeys.h"
#include "eastereggs.h"
#include "string.h"
#include <iostream>
#include <map>
#include <ctime>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define RDKSHELL_ANY_KEY 65536
#define RDKSHELL_DEFAULT_INACTIVITY_TIMEOUT_IN_SECONDS 15*60
#define RDKSHELL_WILDCARD_KEY_CODE 255
#define RDKSHELL_WATERMARK_ID 65536


namespace RdkShell
{

   
   
    class CompositorController
    {
        public:
            static void initialize() {return;}
            static bool moveToFront(const std::string& client) {return true;}
            static bool moveToBack(const std::string& client) {return true;}
            static bool moveBehind(const std::string& client, const std::string& target){return true;}
            static bool setFocus(const std::string& client){return true;}
            static bool getFocused(std::string& client){return true;}
            static bool kill(const std::string& client){return true;}
            static bool addKeyIntercept(const std::string& client, const uint32_t& keyCode, const uint32_t& flags){return true;}
            static bool removeKeyIntercept(const std::string& client, const uint32_t& keyCode, const uint32_t& flags){return true;}
            static bool addKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::map<std::string, RdkShellData> &listenerProperties){return true;}
            static bool addNativeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::map<std::string, RdkShellData> &listenerProperties){return true;}
            static bool removeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags){return true;}
            static bool removeAllKeyListeners(){return true;}
            static bool removeAllKeyIntercepts(){return true;}
            static bool removeNativeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags){return true;}
            static bool addKeyMetadataListener(const std::string& client){return true;}
            static bool removeKeyMetadataListener(const std::string& client){return true;}
            static bool injectKey(const uint32_t& keyCode, const uint32_t& flags){return true;}
            static bool generateKey(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey=""){return true;}
            static bool getScreenResolution(uint32_t &width, uint32_t &height){return true;}
            static bool setScreenResolution(const uint32_t width, const uint32_t height){return true;}
            static bool getClients(std::vector<std::string>& clients){return true;}
            static bool getZOrder(std::vector<std::string>&clients){return true;}
            static bool getBounds(const std::string& client, uint32_t &x, uint32_t &y, uint32_t &width, uint32_t &height){return true;}
            static bool setBounds(const std::string& client, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height){return true;}
            static bool getVisibility(const std::string& client, bool& visible){return true;}
            static bool setVisibility(const std::string& client, const bool visible){return true;}
            static bool getOpacity(const std::string& client, unsigned int& opacity){return true;}
            static bool setOpacity(const std::string& client, const unsigned int opacity){return true;}
            static bool getScale(const std::string& client, double &scaleX, double &scaleY){return true;}
            static bool setScale(const std::string& client, double scaleX, double scaleY){return true;}
            static bool getHolePunch(const std::string& client, bool& holePunch){return true;}
            static bool setHolePunch(const std::string& client, const bool holePunch){return true;}
            static bool scaleToFit(const std::string& client, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height){return true;}
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
            static bool setLogLevel(const std::string level){return true;}
            static bool getLogLevel(std::string& level){return true;}
            static bool setTopmost(const std::string& client, bool topmost, bool focus = false){return true;}
            static bool getTopmost(std::string& client){return true;}
            static bool sendEvent(const std::string& eventName, std::vector<std::map<std::string, RdkShellData>>& data){return true;}
            static bool isSurfaceModeEnabled(){return true;}
            static bool enableKeyRepeats(bool enable){return true;}
            static bool getKeyRepeatsEnabled(bool& enable){return true;}
            static bool getVirtualResolution(const std::string& client, uint32_t &virtualWidth, uint32_t &virtualHeight){return true;}
            static bool setVirtualResolution(const std::string& client, const uint32_t virtualWidth, const uint32_t virtualHeight){return true;}
            static bool enableVirtualDisplay(const std::string& client, const bool enable){return true;}
            static bool getVirtualDisplayEnabled(const std::string& client, bool &enabled){return true;}
            static bool getLastKeyPress(uint32_t &keyCode, uint32_t &modifiers, uint64_t &timestampInSeconds){return true;}
            static bool ignoreKeyInputs(bool ignore){return true;}
            static bool updateWatermarkImage(uint32_t imageId, int32_t key, int32_t imageSize){return true;}
            static bool createWatermarkImage(uint32_t imageId, uint32_t zorder=0){return true;}
            static bool deleteWatermarkImage(uint32_t imageId){return true;}
            static bool adjustWatermarkImage(uint32_t imageId, uint32_t zorder){return true;}
            static bool alwaysShowWatermarkImageOnTop(bool show=false){return true;}
            static bool screenShot(uint8_t* &data, uint32_t &size){return true;}
            static bool enableInputEvents(const std::string& client, bool enable){return true;}
            static bool showCursor(){return true;}
            static bool hideCursor(){return true;}
            static bool setCursorSize(uint32_t width, uint32_t height){return true;}
            static bool getCursorSize(uint32_t& width, uint32_t& height){return true;}
            static void setKeyRepeatConfig(bool enabled, int32_t initialDelay, int32_t repeatInterval){return;}
	    static bool setAVBlocked(std::string callsign, bool blockAV){return true;}
            static bool getBlockedAVApplications(std::vector<std::string>& apps){return true;}
            static bool isErmEnabled(){return true;}
    };


}

#endif //RDKSHELL_COMPOSITOR_CONTROLLER_H


