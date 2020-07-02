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

#pragma once

#include <mutex>
#include "Module.h"
#include "utils.h"
#include <rdkshell/rdkshellevents.h>
#include <rdkshell/rdkshell.h>
#include <rdkshell/linuxkeys.h>
#include "AbstractPlugin.h"

namespace WPEFramework {

    namespace Plugin {

        class RDKShell :  public AbstractPlugin {
        public:
            RDKShell();
            virtual ~RDKShell();
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override;

        public/*members*/:
            static RDKShell* _instance;

        public /*constants*/:
            static const short API_VERSION_NUMBER_MAJOR;
            static const short API_VERSION_NUMBER_MINOR;
            static const string SERVICE_NAME;
            //methods
            static const string RDKSHELL_METHOD_MOVE_TO_FRONT;
            static const string RDKSHELL_METHOD_MOVE_TO_BACK;
            static const string RDKSHELL_METHOD_MOVE_BEHIND;
            static const string RDKSHELL_METHOD_SET_FOCUS;
            static const string RDKSHELL_METHOD_KILL;
            static const string RDKSHELL_METHOD_ADD_KEY_INTERCEPT;
            static const string RDKSHELL_METHOD_REMOVE_KEY_INTERCEPT;
            static const string RDKSHELL_METHOD_ADD_KEY_LISTENER;
            static const string RDKSHELL_METHOD_REMOVE_KEY_LISTENER;
            static const string RDKSHELL_METHOD_ADD_KEY_METADATA_LISTENER;
            static const string RDKSHELL_METHOD_REMOVE_KEY_METADATA_LISTENER;
            static const string RDKSHELL_METHOD_INJECT_KEY;
            static const string RDKSHELL_METHOD_INJECT_KEYS;
            static const string RDKSHELL_METHOD_GET_SCREEN_RESOLUTION;
            static const string RDKSHELL_METHOD_SET_SCREEN_RESOLUTION;
            static const string RDKSHELL_METHOD_CREATE_DISPLAY;
            static const string RDKSHELL_METHOD_GET_CLIENTS;
            static const string RDKSHELL_METHOD_GET_Z_ORDER;
            static const string RDKSHELL_METHOD_GET_BOUNDS;
            static const string RDKSHELL_METHOD_SET_BOUNDS;
            static const string RDKSHELL_METHOD_GET_VISIBILITY;
            static const string RDKSHELL_METHOD_SET_VISIBILITY;
            static const string RDKSHELL_METHOD_GET_OPACITY;
            static const string RDKSHELL_METHOD_SET_OPACITY;
            static const string RDKSHELL_METHOD_GET_SCALE;
            static const string RDKSHELL_METHOD_SET_SCALE;
            static const string RDKSHELL_METHOD_ADD_ANIMATION;
            static const string RDKSHELL_METHOD_REMOVE_ANIMATION;
            static const string RDKSHELL_METHOD_ENABLE_INACTIVITY_REPORTING;
            static const string RDKSHELL_METHOD_SET_INACTIVITY_INTERVAL;
            static const string RDKSHELL_METHOD_SCALE_TO_FIT;

            // events
            static const string RDKSHELL_EVENT_ON_USER_INACTIVITY;

        private/*registered methods (wrappers)*/:

            //methods ("parameters" here is "params" from the curl request)
            uint32_t moveToFrontWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t moveToBackWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t moveBehindWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setFocusWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t killWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t addKeyInterceptWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t removeKeyInterceptWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t addKeyListenersWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t removeKeyListenersWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t addKeyMetadataListenerWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t removeKeyMetadataListenerWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t injectKeyWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t injectKeysWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getScreenResolutionWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setScreenResolutionWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t createDisplayWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getClientsWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getZOrderWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getBoundsWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setBoundsWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getVisibilityWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setVisibilityWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getOpacityWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setOpacityWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getScaleWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setScaleWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t addAnimationWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t removeAnimationWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t enableInactivityReportingWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setInactivityIntervalWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t scaleToFitWrapper(const JsonObject& parameters, JsonObject& response);
            void notify(const std::string& event, const JsonObject& parameters);

        private/*internal methods*/:
            RDKShell(const RDKShell&) = delete;
            RDKShell& operator=(const RDKShell&) = delete;

            bool moveToFront(const string& client);
            bool moveToBack(const string& client);
            bool moveBehind(const string& client, const string& target);
            bool setFocus(const string& client);
            bool kill(const string& client);
            bool addKeyIntercept(const uint32_t& keyCode, const JsonArray& modifiers, const string& client);
            bool removeKeyIntercept(const uint32_t& keyCode, const JsonArray& modifiers, const string& client);
            bool addKeyListeners(const string& client, const JsonArray& listeners);
            bool removeKeyListeners(const string& client, const JsonArray& listeners);
            bool addAnyKeyListener(const string& client, const JsonArray& listeners);
            bool injectKey(const uint32_t& keyCode, const JsonArray& modifiers);
            bool injectKeys(const JsonArray& keyInputs);
            bool getScreenResolution(JsonObject& out);
            bool setScreenResolution(const unsigned int w, const unsigned int h);
            bool createDisplay(const string& client, const string& displayName);
            bool getClients(JsonArray& clients);
            bool getZOrder(JsonArray& clients);
            bool getBounds(const string& client, JsonObject& bounds);
            bool setBounds(const string& client, const unsigned int x, const unsigned int y, const unsigned int w, const unsigned int h);
            bool getVisibility(const string& client, bool& visibility);
            bool setVisibility(const string& client, const bool visible);
            bool getOpacity(const string& client, unsigned int& opacity);
            bool setOpacity(const string& client, const unsigned int opacity);
            bool getScale(const string& client, double& scaleX, double& scaleY);
            bool setScale(const string& client, const double scaleX, const double scaleY);
            bool removeAnimation(const string& client);
            bool addAnimationList(const JsonArray& animations);
            bool enableInactivityReporting(const bool enable);
            bool setInactivityInterval(const string interval);

        private/*classes */:

            class RdkShellListener :  public RdkShell::RdkShellEventListener {

              public:
                RdkShellListener(RDKShell* shell)
                    : mShell(*shell)
                {
                }

                ~RdkShellListener()
                {
                }

                // rdkshell events listeners
                virtual void onApplicationLaunched(const std::string& client);
                virtual void onApplicationConnected(const std::string& client);
                virtual void onApplicationDisconnected(const std::string& client);
                virtual void onApplicationTerminated(const std::string& client);
                virtual void onApplicationFirstFrame(const std::string& client);
                virtual void onUserInactive(const double minutes);

              private:
                  RDKShell& mShell;
            };

            class MonitorClients : public PluginHost::IPlugin::INotification {
              private:
                  MonitorClients() = delete;
                  MonitorClients(const MonitorClients&) = delete;
                  MonitorClients& operator=(const MonitorClients&) = delete;

              public:
                  MonitorClients(RDKShell* shell)
                      : mShell(*shell)
                  {
                      ASSERT(mShell != nullptr);
                  }
                  ~MonitorClients()
                  {
                  }

              public:
                  BEGIN_INTERFACE_MAP(MonitorClients)
                  INTERFACE_ENTRY(PluginHost::IPlugin::INotification)
                  END_INTERFACE_MAP

              private:
                  virtual void StateChange(PluginHost::IShell* shell);

              private:
                  RDKShell& mShell;
            };

        private/*members*/:
            bool mRemoteShell;
            MonitorClients* mClientsMonitor;
            std::shared_ptr<RdkShell::RdkShellEventListener> mEventListener;
            //std::mutex m_callMutex;
        };
    } // namespace Plugin
} // namespace WPEFramework
