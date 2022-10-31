#ifndef RDKSHELL_RDK_COMPOSITOR_H
#define RDKSHELL_RDK_COMPOSITOR_H

#include <string>
#include <thread>
#include <mutex>
#include <functional>
#include <unordered_map>
//#include "westeros-compositor.h"
#include "inputevent.h"
#include "application.h"
//#include "rdkshellrect.h"

namespace RdkShell
{

    class FrameBuffer;

    class RdkCompositor
    {
        public:

            RdkCompositor();
            virtual ~RdkCompositor();
            bool createDisplay(const std::string& displayName, const std::string& clientName,
                uint32_t width, uint32_t height, bool virtualDisplayEnabled, uint32_t virtualWidth, uint32_t virtualHeight){return true;}
          //  void draw(bool &needsHolePunch, RdkShellRect& rect){return;}
            void onKeyPress(uint32_t keycode, uint32_t flags, uint64_t metadata){return;}
            void onKeyRelease(uint32_t keycode, uint32_t flags, uint64_t metadata){return;}
            void onPointerMotion(uint32_t x, uint32_t y){return;}
            void onPointerButtonPress(uint32_t keyCode, uint32_t x, uint32_t y){return;}
            void onPointerButtonRelease(uint32_t keyCode, uint32_t x, uint32_t y){return;}
            void setPosition(int32_t x, int32_t y){return;}
            void position(int32_t &x, int32_t &y){return;}
            void setSize(uint32_t width, uint32_t height){return;}
            void size(uint32_t &width, uint32_t &height){return;}
            void setOpacity(double opacity){return;}
            void scale(double &scaleX, double &scaleY){return;}
            void setScale(double scaleX, double scaleY){return;}
            void opacity(double& opacity){return;}
            void setVisible(bool visible){return;}
            void visible(bool &visible){return;}
            void setAnimating(bool animating){return;}
            void setHolePunch(bool holePunchEnabled){return;}
            void holePunch(bool &holePunchEnabled){return;}
            void keyMetadataEnabled(bool &enabled){return;}
            void setKeyMetadataEnabled(bool enable){return;}
            int registerInputEventListener(std::function<void(const RdkShell::InputEvent&)> listener){return 0;}
            void unregisterInputEventListener(int tag){return;}
            int registerStateChangeEventListener(std::function<void(uint32_t)> listener){return 0;}
            void unregisterStateChangeEventListener(int tag){return;}
            void displayName(std::string& name) const;
            void closeApplication(){return;}
            void launchApplication(){return;}
            bool resumeApplication(){return true;}
            bool suspendApplication(){return true;}
            void setApplication(const std::string& application){return;}
            bool isKeyPressed(){return true;}
            void getVirtualResolution(uint32_t &virtualWidth, uint32_t &virtualHeight){return;}
            void setVirtualResolution(uint32_t virtualWidth, uint32_t virtualHeight){return;}
            void enableVirtualDisplay(bool enable){return;}
            bool getVirtualDisplayEnabled(){return true;}
            void updateSurfaceCount(bool status){return;}
            uint32_t getSurfaceCount(void){return 0;}
            void enableInputEvents(bool enable){return;}
            bool getInputEventsEnabled() const{return true;}
            void setFocused(bool focused){return;}

        private:
           // void prepareHolePunchRects(std::vector<WstRect> wstrects, RdkShellRect& rect);
            uint32_t mSurfaceCount;
        protected:
           // static void invalidate(WstCompositor *context, void *userData);
           // static void clientStatus(WstCompositor *context, int status, int pid, int detail, void *userData);
           // static void dispatch( WstCompositor *wctx, void *userData );
            void onInvalidate(){return;}
            void onClientStatus(int status, int pid, int detail){return;}
            void onSizeChangeComplete(){return;}
            void processKeyEvent(bool keyPressed, uint32_t keycode, uint32_t flags, uint64_t metadata){return;}
            void broadcastInputEvent(const RdkShell::InputEvent &inputEvent){return;}
            void broadcastStateChangeEvent(uint32_t state){return;}
            void launchApplicationInBackground(){return;}
            void shutdownApplication(){return;}
          //  static bool loadExtensions(WstCompositor *compositor, const std::string& clientName);
           // void drawDirect(bool &needsHolePunch, RdkShellRect& rect){return;}
           // void drawFbo(bool &needsHolePunch, RdkShellRect& rect){return;}
            void updateWaylandState(){return;}
            
            std::string mDisplayName;
          //  WstCompositor *mWstContext;
            uint32_t mWidth;
            uint32_t mHeight;
            int32_t mPositionX;
            int32_t mPositionY;
            float mMatrix[16];
            double mOpacity;
            bool mVisible;
            bool mAnimating;
            bool mHolePunch;
            double mScaleX;
            double mScaleY;
            bool mEnableKeyMetadata;
            int mInputListenerTags;
            std::mutex mInputLock;
            std::unordered_map<int, std::function<void(const RdkShell::InputEvent&)>> mInputListeners;
            int mStateChangeListenerTags;
            std::mutex mStateChangeLock;
            std::unordered_map<int, std::function<void(uint32_t)>> mStateChangeListeners;
            std::string mApplicationName;
            std::thread mApplicationThread;
            RdkShell::ApplicationState mApplicationState;
            int32_t mApplicationPid;
            bool mApplicationThreadStarted;
            bool mApplicationClosedByCompositor;
            std::recursive_mutex mApplicationMutex;
            bool mReceivedKeyPress;
            bool mVirtualDisplayEnabled;
            uint32_t mVirtualWidth;
            uint32_t mVirtualHeight;
            std::shared_ptr<FrameBuffer> mFbo;
            bool mSizeChangeRequestPresent;
            bool mInputEventsEnabled;
            bool mSuspendedBeforeStart;
            bool mFocused;
    };
}

#endif //RDKSHELL_RDK_COMPOSITOR_
