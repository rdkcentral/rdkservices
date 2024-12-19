/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#include <string>
#include <memory>
#include <iostream>
#include <mutex>
#include <thread>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <rdkshell/rdkshell.h>
#include <rdkshell/compositorcontroller.h>
#include <rdkshell/application.h>
#include <rdkshell/logger.h>
#include <interfaces/IAppManager.h>
#include <plugins/System.h>
#include "EventListener.h"

namespace WPEFramework
{
    namespace Plugin
    {
        namespace AppManagerV1
	{
	    class RDKShellAdapter
            {
                private/*classes */:

                    class MonitorClients : public PluginHost::IPlugin::INotification
#if ((THUNDER_VERSION >= 4) && (THUNDER_VERSION_MINOR >= 4))
                    ,  public PluginHost::IPlugin::ILifeTime 
#endif
	           {
                      private:
                          MonitorClients() = delete;
                          MonitorClients(const MonitorClients&) = delete;
                          MonitorClients& operator=(const MonitorClients&) = delete;

                      public:
                          MonitorClients(RDKShellAdapter* shell)
                              : mShell(*shell)
                          {
                          }
                          ~MonitorClients()
                          {
                          }

                      public:
                          BEGIN_INTERFACE_MAP(MonitorClients)
                          INTERFACE_ENTRY(PluginHost::IPlugin::INotification)
#if ((THUNDER_VERSION >= 4) && (THUNDER_VERSION_MINOR >= 4))
	        	  INTERFACE_ENTRY(PluginHost::IPlugin::ILifeTime)
#endif
                          END_INTERFACE_MAP

                      private:
                          virtual void StateChange(PluginHost::IShell* shell);
	        	      void handleInitialize(PluginHost::IShell* shell);
                          void handleActivated(PluginHost::IShell* shell);
                          void handleDeactivated(PluginHost::IShell* shell);
                          void handleDeinitialized(PluginHost::IShell* shell);

#ifdef USE_THUNDER_R4
                          virtual void Initialize(const string& callsign, PluginHost::IShell* plugin);
                          virtual void Activation(const string& name, PluginHost::IShell* plugin);
                          virtual void Deactivation(const string& name, PluginHost::IShell* plugin);
                          virtual void  Activated(const string& callSign,  PluginHost::IShell* plugin);
                          virtual void  Deactivated(const string& callSign,  PluginHost::IShell* plugin);
	        	      virtual void Deinitialized(const string& callsign, PluginHost::IShell* plugin);
                          virtual void  Unavailable(const string& callSign,  PluginHost::IShell* plugin);
#endif /* USE_THUNDER_R4 */
                      private:
                          RDKShellAdapter& mShell;
                    };

                public/*members*/:
	           static RDKShellAdapter* _instance;

                public:
                    RDKShellAdapter();
                    ~RDKShellAdapter();
                    const bool Initialize(EventListener* listener, PluginHost::IShell* service);
                    void Deinitialize();
                    uint32_t launch(const JsonObject& parameters, JsonObject& response);
                    uint32_t kill(const JsonObject& parameters, JsonObject& response);
                    uint32_t getClients(const JsonObject& parameters, JsonObject& response);
                    uint32_t suspend(const JsonObject& parameters, JsonObject& response);
                    uint32_t destroy(const JsonObject& parameters, JsonObject& response);
                    uint32_t getAvailableTypes(const JsonObject& parameters, JsonObject& response);
                    void updateAppState(const string callsign, Exchange::IAppManager::LifecycleState newState);

                private:
                    std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > getThunderControllerClient(std::string callsign="", std::string localidentifier="");
                    std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> getPackagerPlugin();
                    std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> getOCIContainerPlugin();
                    uint32_t createDisplayWrapper(const JsonObject& parameters, JsonObject& response);
                    uint32_t getAvailableTypesWrapper(const JsonObject& parameters, JsonObject& response);
                    uint32_t getState(const JsonObject& parameters, JsonObject& response);
                    void killAllApps(bool enableDestroyEvent);
                    bool setAVBlocked(const string callsign, bool blockAV);
                    bool moveBehind(const string& client, const string& target);
                    bool setFocus(const string& client);
	            bool getFocused(string& client);
                    bool kill(const string& client);
                    bool getScreenResolution(JsonObject& out);
                    bool setMimeType(const string& client, const string& mimeType);
                    bool getMimeType(const string& client, string& mimeType);
                    bool createDisplay(const string& client, const string& displayName, const uint32_t displayWidth, const uint32_t displayHeight, const bool virtualDisplay, const uint32_t virtualWidth, const uint32_t virtualHeight, const bool topmost, const bool focus);
                    bool getClients(JsonArray& clients);
                    bool setVisibility(const string& client, const bool visible);
                    bool setOpacity(const string& client, const unsigned int opacity);
                    bool setScale(const string& client, const double scaleX, const double scaleY);
                    bool setHolePunch(const string& client, const bool holePunch);
                    bool setTopmost(const string& callsign, const bool topmost, const bool focus);

                    void onAppStateChanged(const std::string& client, Exchange::IAppManager::LifecycleState newState, Exchange::IAppManager::LifecycleState oldState);

                    MonitorClients* mClientsMonitor;
                    PluginHost::IShell* mCurrentService;
	    	    bool mErmEnabled;
                    EventListener* mEventListener;
            };

	    struct PluginData
            {
                std::string mClassName;
            };

            class StateControlNotification: public PluginHost::IStateControl::INotification
            {
                public:
                    StateControlNotification(std::string callsign, RDKShellAdapter& shell);
                    void enableLaunch(bool enable);
                    void StateChange(const PluginHost::IStateControl::state state) override;

                    BEGIN_INTERFACE_MAP(Notification)
                    INTERFACE_ENTRY(PluginHost::IStateControl::INotification)
                    END_INTERFACE_MAP

                private:
                    RDKShellAdapter& mRDKShellAdapter;
                    std::string mCallSign;
                    bool mLaunchEnabled;
            };

            struct CreateDisplayRequest
            {
                CreateDisplayRequest(std::string client, std::string displayName, uint32_t displayWidth=0, uint32_t displayHeight=0, bool virtualDisplayEnabled=false, uint32_t virtualWidth=0, uint32_t virtualHeight=0, bool topmost = false, bool focus = false);
                ~CreateDisplayRequest();

                std::string mClient;
                std::string mDisplayName;
                uint32_t mDisplayWidth;
                uint32_t mDisplayHeight;
                bool mVirtualDisplayEnabled;
                uint32_t mVirtualWidth;
                uint32_t mVirtualHeight;
                bool mTopmost;
                bool mFocus;
                sem_t mSemaphore;
                bool mResult;
                bool mAutoDestroy;
            };

            struct KillClientRequest
            {
                KillClientRequest(std::string client);
                ~KillClientRequest();
                std::string mClient;
                sem_t mSemaphore;
                bool mResult;
            };

            enum RDKShellLaunchType
            {
                UNKNOWN = 0,
                CREATE,
                ACTIVATE,
                SUSPEND,
                RESUME
            };

            enum AppLastExitReason
            {
                UNDEFINED = 0,
                CRASH,
                DEACTIVATED
            };
        }
    }
}
