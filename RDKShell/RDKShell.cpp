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

#include "RDKShell.h"
#include <string>
#include <iostream>
#include <mutex>
#include <thread>
#include <rdkshell/compositorcontroller.h>
#include <rdkshell/application.h>
#include "rfcapi.h"

const short WPEFramework::Plugin::RDKShell::API_VERSION_NUMBER_MAJOR = 1;
const short WPEFramework::Plugin::RDKShell::API_VERSION_NUMBER_MINOR = 0;
const string WPEFramework::Plugin::RDKShell::SERVICE_NAME = "org.rdk.RDKShell";
//methods
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_MOVE_TO_FRONT = "moveToFront";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_MOVE_TO_BACK = "moveToBack";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_MOVE_BEHIND = "moveBehind";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_FOCUS = "setFocus";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_KILL = "kill";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ADD_KEY_INTERCEPT = "addKeyIntercept";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_REMOVE_KEY_INTERCEPT = "removeKeyIntercept";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ADD_KEY_LISTENER = "addKeyListener";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_REMOVE_KEY_LISTENER = "removeKeyListener";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ADD_KEY_METADATA_LISTENER = "addKeyMetadataListener";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_REMOVE_KEY_METADATA_LISTENER = "removeKeyMetadataListener";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_INJECT_KEY = "injectKey";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_SCREEN_RESOLUTION = "getScreenResolution";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_SCREEN_RESOLUTION = "setScreenResolution";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_CREATE_DISPLAY = "createDisplay";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_CLIENTS = "getClients";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_Z_ORDER = "getZOrder";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_BOUNDS = "getBounds";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_BOUNDS = "setBounds";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_VISIBILITY = "getVisibility";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_VISIBILITY = "setVisibility";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_OPACITY = "getOpacity";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_OPACITY = "setOpacity";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_SCALE = "getScale";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_SCALE = "setScale";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_REMOVE_ANIMATION = "removeAnimation";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ADD_ANIMATION = "addAnimation";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ENABLE_INACTIVITY_REPORTING = "enableInactivityReporting";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_INACTIVITY_INTERVAL = "setInactivityInterval";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SCALE_TO_FIT = "scaleToFit";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_LAUNCH_APPLICATION = "launchApplication";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SUSPEND_APPLICATION = "suspendApplication";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_RESUME_APPLICATION = "resumeApplication";

const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_USER_INACTIVITY = "onUserInactivity";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_LAUNCHED = "onApplicationLaunched";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_CONNECTED = "onApplicationConnected";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_DISCONNECTED = "onApplicationDisconnected";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_TERMINATED = "onApplicationTerminated";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_FIRST_FRAME = "onApplicationFirstFrame";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_SUSPENDED = "onApplicationSuspended";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_RESUMED = "onApplicationResumed";

using namespace std;
using namespace RdkShell;
extern int gCurrentFramerate;
bool receivedResolutionRequest = false;
unsigned int resolutionWidth = 1280;
unsigned int resolutionHeight = 720;

#define ANY_KEY 65536

namespace WPEFramework {
    namespace Plugin {

        uint32_t getKeyFlag(std::string modifier)
        {
          uint32_t flag = 0;
          if (0 == modifier.compare("ctrl"))
          {
            flag = RDKSHELL_FLAGS_CONTROL;
          }
          else if (0 == modifier.compare("shift"))
          {
            flag = RDKSHELL_FLAGS_SHIFT;
          }
          else if (0 == modifier.compare("alt"))
          {
            flag = RDKSHELL_FLAGS_ALT;
          }
          return flag;
        }

        bool getRFCConfig(char* paramName, RFC_ParamData_t& paramOutput)
        {
          WDMP_STATUS wdmpStatus = getRFCParameter("RDKShell", paramName, &paramOutput);
          if (wdmpStatus == WDMP_SUCCESS || wdmpStatus == WDMP_ERR_DEFAULT_VALUE)
          {
            return true;
          }
          return false;
        }

        SERVICE_REGISTRATION(RDKShell, 1, 0);

        RDKShell* RDKShell::_instance = nullptr;
        std::mutex gRdkShellMutex;

        static std::thread shellThread;

        void RDKShell::MonitorClients::StateChange(PluginHost::IShell* service)
        {
            if (service)
            {
                PluginHost::IShell::state currentState(service->State());
                if (currentState == PluginHost::IShell::ACTIVATION)
                {
                   std::string configLine = service->ConfigLine();
                   if (configLine.empty())
                   {
                       return;
                   }
                   JsonObject serviceConfig = JsonObject(configLine.c_str());
                   if (serviceConfig.HasLabel("clientidentifier"))
                   {
                       std::string clientidentifier = serviceConfig["clientidentifier"].String();
                       gRdkShellMutex.lock();
                       RdkShell::CompositorController::createDisplay(service->Callsign(), clientidentifier);
                       RdkShell::CompositorController::addListener(clientidentifier, mShell.mEventListener);
                       gRdkShellMutex.unlock();
                   }
                }
                else if (currentState == PluginHost::IShell::ACTIVATED && service->Callsign() == WPEFramework::Plugin::RDKShell::SERVICE_NAME)
                {
                    PluginHost::ISubSystem* subSystems(service->SubSystems());
                    if (subSystems != nullptr)
                    {
                        subSystems->Set(PluginHost::ISubSystem::PLATFORM, nullptr);
                        subSystems->Set(PluginHost::ISubSystem::GRAPHICS, nullptr);
                        subSystems->Release();
                    }
                }
                else if (currentState == PluginHost::IShell::DEACTIVATED)
                {
                    std::string configLine = service->ConfigLine();
                    if (configLine.empty())
                    {
                        return;
                    }
                    JsonObject serviceConfig = JsonObject(configLine.c_str());
                    if (serviceConfig.HasLabel("clientidentifier"))
                    {
                        std::string clientidentifier = serviceConfig["clientidentifier"].String();
                        gRdkShellMutex.lock();
                        RdkShell::CompositorController::kill(clientidentifier);
                        RdkShell::CompositorController::removeListener(clientidentifier, mShell.mEventListener);
                        gRdkShellMutex.unlock();
                    }
                }
            }
        }

        RDKShell::RDKShell()
                : AbstractPlugin(), mClientsMonitor(Core::Service<MonitorClients>::Create<MonitorClients>(this))
        {
            LOGINFO("ctor");
            RDKShell::_instance = this;
            mEventListener = std::make_shared<RdkShellListener>(this);

            mRemoteShell = false;
            registerMethod(RDKSHELL_METHOD_MOVE_TO_FRONT, &RDKShell::moveToFrontWrapper, this);
            registerMethod(RDKSHELL_METHOD_MOVE_TO_BACK, &RDKShell::moveToBackWrapper, this);
            registerMethod(RDKSHELL_METHOD_MOVE_BEHIND, &RDKShell::moveBehindWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_FOCUS, &RDKShell::setFocusWrapper, this);
            registerMethod(RDKSHELL_METHOD_KILL, &RDKShell::killWrapper, this);
            registerMethod(RDKSHELL_METHOD_ADD_KEY_INTERCEPT, &RDKShell::addKeyInterceptWrapper, this);
            registerMethod(RDKSHELL_METHOD_REMOVE_KEY_INTERCEPT, &RDKShell::removeKeyInterceptWrapper, this);
            registerMethod(RDKSHELL_METHOD_ADD_KEY_LISTENER, &RDKShell::addKeyListenersWrapper, this);
            registerMethod(RDKSHELL_METHOD_REMOVE_KEY_LISTENER, &RDKShell::removeKeyListenersWrapper, this);
            registerMethod(RDKSHELL_METHOD_ADD_KEY_METADATA_LISTENER, &RDKShell::addKeyMetadataListenerWrapper, this);
            registerMethod(RDKSHELL_METHOD_REMOVE_KEY_METADATA_LISTENER, &RDKShell::removeKeyMetadataListenerWrapper, this);
            registerMethod(RDKSHELL_METHOD_INJECT_KEY, &RDKShell::injectKeyWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_SCREEN_RESOLUTION, &RDKShell::getScreenResolutionWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_SCREEN_RESOLUTION, &RDKShell::setScreenResolutionWrapper, this);
            registerMethod(RDKSHELL_METHOD_CREATE_DISPLAY, &RDKShell::createDisplayWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_CLIENTS, &RDKShell::getClientsWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_Z_ORDER, &RDKShell::getZOrderWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_BOUNDS, &RDKShell::getBoundsWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_BOUNDS, &RDKShell::setBoundsWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_VISIBILITY, &RDKShell::getVisibilityWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_VISIBILITY, &RDKShell::setVisibilityWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_OPACITY, &RDKShell::getOpacityWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_OPACITY, &RDKShell::setOpacityWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_SCALE, &RDKShell::getScaleWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_SCALE, &RDKShell::setScaleWrapper, this);
            registerMethod(RDKSHELL_METHOD_REMOVE_ANIMATION, &RDKShell::removeAnimationWrapper, this);
            registerMethod(RDKSHELL_METHOD_ADD_ANIMATION, &RDKShell::addAnimationWrapper, this);
            registerMethod(RDKSHELL_METHOD_ENABLE_INACTIVITY_REPORTING, &RDKShell::enableInactivityReportingWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_INACTIVITY_INTERVAL, &RDKShell::setInactivityIntervalWrapper, this);
            registerMethod(RDKSHELL_METHOD_SCALE_TO_FIT, &RDKShell::scaleToFitWrapper, this);
            registerMethod(RDKSHELL_METHOD_LAUNCH_APPLICATION, &RDKShell::launchApplicationWrapper, this);
            registerMethod(RDKSHELL_METHOD_SUSPEND_APPLICATION, &RDKShell::suspendApplicationWrapper, this);
            registerMethod(RDKSHELL_METHOD_RESUME_APPLICATION, &RDKShell::resumeApplicationWrapper, this);
        }

        RDKShell::~RDKShell()
        {
            LOGINFO("dtor");
            mClientsMonitor->Release();
            RDKShell::_instance = nullptr;
            mRemoteShell = false;
            CompositorController::setEventListener(nullptr);
            mEventListener = nullptr;
        }

        const string RDKShell::Initialize(PluginHost::IShell* service )
        {
            LOGINFO();
            
            CompositorController::setEventListener(mEventListener);
            RFC_ParamData_t param;
            bool ret = getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Power.UserInactivityNotification.Enable", param);
            if (true == ret && param.type == WDMP_BOOLEAN && (strncasecmp(param.value,"true",4) == 0))
            {
              CompositorController::enableInactivityReporting(true);
              ret = getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Power.UserInactivityNotification.TimeMinutes", param);
              if (true == ret && param.type == WDMP_STRING)
              {
                CompositorController::setInactivityInterval(std::stod(param.value));
              }
            }

            shellThread = std::thread([]() {
                gRdkShellMutex.lock();
                RdkShell::initialize();
                gRdkShellMutex.unlock();
                while(true) {
                  const double maxSleepTime = (1000 / gCurrentFramerate) * 1000;
                  double startFrameTime = RdkShell::microseconds();
                  gRdkShellMutex.lock();
                  if (receivedResolutionRequest)
                  {
                    CompositorController::setScreenResolution(resolutionWidth, resolutionHeight);
                    receivedResolutionRequest = false;
                  }
                  RdkShell::draw();
                  RdkShell::update();
                  gRdkShellMutex.unlock();
                  double frameTime = (int)RdkShell::microseconds() - (int)startFrameTime;
                  if (frameTime < maxSleepTime)
                  {
                      int sleepTime = (int)maxSleepTime-(int)frameTime;
                      usleep(sleepTime);
                  }
                }
            });

            service->Register(mClientsMonitor);
            return "";
        }

        void RDKShell::Deinitialize(PluginHost::IShell* service)
        {
            LOGINFO();
            service->Unregister(mClientsMonitor);
        }

        string RDKShell::Information() const
        {
            return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
        }

        void RDKShell::RdkShellListener::onApplicationLaunched(const std::string& client)
        {
          std::cout << "RDKShell onApplicationLaunched event received ..." << client << std::endl;
          JsonObject response;
          response["method"] = RDKSHELL_EVENT_ON_APP_LAUNCHED;
          JsonObject params;
          params["client"] = client;
          response["params"] = params;
          mShell.notify(RDKSHELL_EVENT_ON_APP_LAUNCHED, response);
        }

        void RDKShell::RdkShellListener::onApplicationConnected(const std::string& client)
        {
          std::cout << "RDKShell onApplicationConnected event received ..." << client << std::endl;
          JsonObject response;
          response["method"] = RDKSHELL_EVENT_ON_APP_CONNECTED;
          JsonObject params;
          params["client"] = client;
          response["params"] = params;
          mShell.notify(RDKSHELL_EVENT_ON_APP_CONNECTED, response);
        }

        void RDKShell::RdkShellListener::onApplicationDisconnected(const std::string& client)
        {
          std::cout << "RDKShell onApplicationDisconnected event received ..." << client << std::endl;
          JsonObject response;
          response["method"] = RDKSHELL_EVENT_ON_APP_DISCONNECTED;
          JsonObject params;
          params["client"] = client;
          response["params"] = params;
          mShell.notify(RDKSHELL_EVENT_ON_APP_DISCONNECTED, response);
        }

        void RDKShell::RdkShellListener::onApplicationTerminated(const std::string& client)
        {
          std::cout << "RDKShell onApplicationTerminated event received ..." << client << std::endl;
          JsonObject response;
          response["method"] = RDKSHELL_EVENT_ON_APP_TERMINATED;
          JsonObject params;
          params["client"] = client;
          response["params"] = params;
          mShell.notify(RDKSHELL_EVENT_ON_APP_TERMINATED, response);
        }

        void RDKShell::RdkShellListener::onApplicationFirstFrame(const std::string& client)
        {
          std::cout << "RDKShell onApplicationFirstFrame event received ..." << client << std::endl;
          JsonObject response;
          response["method"] = RDKSHELL_EVENT_ON_APP_FIRST_FRAME;
          JsonObject params;
          params["client"] = client;
          response["params"] = params;
          mShell.notify(RDKSHELL_EVENT_ON_APP_FIRST_FRAME, response);
        }

        void RDKShell::RdkShellListener::onApplicationSuspended(const std::string& client)
        {
          std::cout << "RDKShell onApplicationSuspended event received for " << client << std::endl;
          JsonObject response;
          response["method"] = RDKSHELL_EVENT_ON_APP_SUSPENDED;
          JsonObject params;
          params["client"] = client;
          response["params"] = params;
          mShell.notify(RDKSHELL_EVENT_ON_APP_SUSPENDED, response);
        }

        void RDKShell::RdkShellListener::onApplicationResumed(const std::string& client)
        {
          std::cout << "RDKShell onApplicationResumed event received for " << client << std::endl;
          JsonObject response;
          response["method"] = RDKSHELL_EVENT_ON_APP_RESUMED;
          JsonObject params;
          params["client"] = client;
          response["params"] = params;
          mShell.notify(RDKSHELL_EVENT_ON_APP_RESUMED, response);
        }

        void RDKShell::RdkShellListener::onUserInactive(const double minutes)
        {
          std::cout << "RDKShell onUserInactive event received ..." << minutes << std::endl;
          JsonObject response;
          response["method"] = RDKSHELL_EVENT_ON_USER_INACTIVITY;
          JsonObject params;
          params["minutes"] = std::to_string(minutes);
          response["params"] = params;
          mShell.notify(RDKSHELL_EVENT_ON_USER_INACTIVITY, response);
        }

        // Registered methods (wrappers) begin
        uint32_t RDKShell::moveToFrontWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                result = moveToFront(client);
                if (false == result) {
                  response["message"] = "failed to move front";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::moveToBackWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                result = moveToBack(client);
                if (false == result) {
                  response["message"] = "failed to move back";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::moveBehindWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (!parameters.HasLabel("target"))
            {
                result = false;
                response["message"] = "please specify target";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                const string target  = parameters["target"].String();
                result = moveBehind(client, target);
                if (false == result) {
                  response["message"] = "failed to move behind";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::setFocusWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                result = setFocus(client);
                if (false == result) {
                  response["message"] = "failed to set focus";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::killWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                result = kill(client);
                if (false == result) {
                  response["message"] = "failed to kill client";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::addKeyInterceptWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("keyCode"))
            {
                result = false;
                response["message"] = "please specify keyCode";
            }
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                //optional param?
                const JsonArray modifiers = parameters.HasLabel("modifiers") ? parameters["modifiers"].Array() : JsonArray();

                const uint32_t keyCode = parameters["keyCode"].Number();
                const string client  = parameters["client"].String();
                result = addKeyIntercept(keyCode, modifiers, client);
                if (false == result) {
                  response["message"] = "failed to add key intercept";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::removeKeyInterceptWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("keyCode"))
            {
                result = false;
                response["message"] = "please specify keyCode";
            }
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                //optional param?
                const JsonArray modifiers = parameters.HasLabel("modifiers") ? parameters["modifiers"].Array() : JsonArray();

                const uint32_t keyCode = parameters["keyCode"].Number();
                const string client  = parameters["client"].String();
                result = removeKeyIntercept(keyCode, modifiers, client);
                if (false == result) {
                  response["message"] = "failed to remove key intercept";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::addKeyListenersWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("keys"))
            {
                result = false;
                response["message"] = "please specify keys";
            }
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                const JsonArray keys = parameters["keys"].Array();
                const string client  = parameters["client"].String();
                result = addKeyListeners(client, keys);
                if (false == result) {
                  response["message"] = "failed to add key listeners";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::removeKeyListenersWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("keys"))
            {
                result = false;
                response["message"] = "please specify keys";
            }
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                const JsonArray keys = parameters["keys"].Array();
                const string client  = parameters["client"].String();
                result = removeKeyListeners(client, keys);
                if (false == result) {
                  response["message"] = "failed to remove key listeners";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::addKeyMetadataListenerWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                const string client = parameters["client"].String();
                gRdkShellMutex.lock();
                result = CompositorController::addKeyMetadataListener(client);
                gRdkShellMutex.unlock();
                if (false == result) {
                  response["message"] = "failed to add key metadata listeners";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::removeKeyMetadataListenerWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                const string client = parameters["client"].String();
                gRdkShellMutex.lock();
                result = CompositorController::removeKeyMetadataListener(client);
                gRdkShellMutex.unlock();
                if (false == result) {
                  response["message"] = "failed to remove key metadata listeners";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::injectKeyWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("keyCode"))
            {
                result = false;
                response["message"] = "please specify keyCode";
            }

            if (result)
            {
                //optional param?
                const JsonArray modifiers = parameters.HasLabel("modifiers") ? parameters["modifiers"].Array() : JsonArray();

                const uint32_t keyCode = parameters["keyCode"].Number();
                result = injectKey(keyCode, modifiers);
                if (false == result) {
                  response["message"] = "failed to inject key";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getScreenResolutionWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);

            bool result = true;
            if (!getScreenResolution(response))
            {
                response["message"] = "failed to get screen resolution";
                result = false;
            }
            returnResponse(result);
        }

        uint32_t RDKShell::setScreenResolutionWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("w"))
            {
                result = false;
                response["message"] = "please specify w";
            }
            if (!parameters.HasLabel("h"))
            {
                result = false;
                response["message"] = "please specify h";
            }
            if (result)
            {
                const unsigned int w  = parameters["w"].Number();
                const unsigned int h  = parameters["h"].Number();

                result = setScreenResolution(w, h);
                // Just realized: we need one more string& param for the the error message in case setScreenResolution() fails internally
                // Also, we might not need a "non-wrapper" method at all, nothing prevents us from implementing it right here
                if (false == result) {
                  response["message"] = "failed to set screen resolution";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::createDisplayWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                string displayName("");
                if (parameters.HasLabel("displayName"))
                {
                    displayName = parameters["displayName"].String();
                }
                result = createDisplay(client, displayName);
                if (false == result) {
                  response["message"] = "failed to create display";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getClientsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            bool result = true;
            JsonArray clients;
            if (!getClients(clients))
            {
                response["message"] = "failed to get clients";
                result = false;
            } else {
                response["clients"] = clients;
                result = true;
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getZOrderWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            bool result = true;
            JsonArray clients;
            if (!getZOrder(clients))
            {
                response["message"] = "failed to get clients";
                result = false;
            } else {
                response["clients"] = clients;
                result = true;
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getBoundsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result) {
              JsonObject bounds;
              const string client  = parameters["client"].String();
              if (!getBounds(client, bounds))
              {
                  response["message"] = "failed to get bounds";
                  result = false;
              } else {
                  response["bounds"] = bounds;
                  result = true;
              }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::setBoundsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client  = parameters["client"].String();

                unsigned int x=0,y=0,w=0,h=0;
                gRdkShellMutex.lock();
                CompositorController::getBounds(client, x, y, w, h);
                gRdkShellMutex.unlock();
                if (parameters.HasLabel("x"))
                {
                    x  = parameters["x"].Number();
                }
                if (parameters.HasLabel("y"))
                {
                    y  = parameters["y"].Number();
                }
                if (parameters.HasLabel("w"))
                {
                    w  = parameters["w"].Number();
                }
                if (parameters.HasLabel("h"))
                {
                    h  = parameters["h"].Number();
                }

                result = setBounds(client, x, y, w, h);
                if (false == result) {
                  response["message"] = "failed to set bounds";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getVisibilityWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                bool visible;
                result = getVisibility(client, visible);
                if (false == result) {
                  response["message"] = "failed to get visibility";
                }
                else {
                  response["visible"] = visible;
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::setVisibilityWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (!parameters.HasLabel("visible"))
            {
                result = false;
                response["message"] = "please specify visibility (visible = true/false)";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                const bool visible  = parameters["visible"].Boolean();

                result = setVisibility(client, visible);
                // Just realized: we need one more string& param for the the error message in case setScreenResolution() fails internally
                // Also, we might not need a "non-wrapper" method at all, nothing prevents us from implementing it right here

                if (false == result) {
                  response["message"] = "failed to set visibility";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getOpacityWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                const string client  = parameters["client"].String();
                unsigned int opacity;
                if (!getOpacity(client, opacity))
                {
                    response["message"] = "failed to get opacity";
                    result = false;
                } else {
                    response["opacity"] = opacity;
                    result = true;
                }
            }

            returnResponse(result);
        }

        uint32_t RDKShell::setOpacityWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (!parameters.HasLabel("opacity"))
            {
                result = false;
                response["message"] = "please specify opacity";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                const unsigned int  opacity  = parameters["opacity"].Number();

                result = setOpacity(client, opacity);
                if (false == result) {
                  response["message"] = "failed to set opacity";
                }
                // handle the result
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getScaleWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                const string client  = parameters["client"].String();
                double scaleX = 1.0;
                double scaleY = 1.0;
                if (!getScale(client, scaleX, scaleY))
                {
                    response["message"] = "failed to get scale";
                    result = false;
                } else {
                    response["sx"] = std::to_string(scaleX);
                    response["sy"] = std::to_string(scaleY);
                    result = true;
                }
            }

            returnResponse(result);
        }

        uint32_t RDKShell::setScaleWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (!parameters.HasLabel("sx") && !parameters.HasLabel("sy"))
            {
                result = false;
                response["message"] = "please specify sx and/or sy";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                double scaleX = 1.0;
                double scaleY = 1.0;
                getScale(client, scaleX, scaleY);
                if (parameters.HasLabel("sx"))
                {
                    scaleX = std::stod(parameters["sx"].String());
                }
                if (parameters.HasLabel("sy"))
                {
                    scaleY = std::stod(parameters["sy"].String());
                }

                result = setScale(client, scaleX, scaleY);
                if (false == result) {
                  response["message"] = "failed to set scale";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::removeAnimationWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                const string client  = parameters["client"].String();
                if (!removeAnimation(client))
                {
                    response["message"] = "failed to remove animation";
                    result = false;
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::addAnimationWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (parameters.HasLabel("animations"))
            {
                const JsonArray animations = parameters["animations"].Array();
                result = addAnimationList(animations);
                if (false == result) {
                    response["message"] = "failed to add animation list";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::enableInactivityReportingWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("enable"))
            {
                result = false;
                response["message"] = "please specify enable parameter";
            }
            if (result)
            {
                const bool enable  = parameters["enable"].Boolean();

                result = enableInactivityReporting(enable);

                if (false == result) {
                  response["message"] = "failed to set inactivity notification";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::setInactivityIntervalWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("interval"))
            {
                result = false;
                response["message"] = "please specify interval parameter";
            }
            if (result)
            {
                const string interval = parameters["interval"].String();

                result = setInactivityInterval(interval);
                // Just realized: we need one more string& param for the the error message in case setScreenResolution() fails internally
                // Also, we might not need a "non-wrapper" method at all, nothing prevents us from implementing it right here

                if (false == result) {
                  response["message"] = "failed to set inactivity interval";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::scaleToFitWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client = parameters["client"].String();

                unsigned int x = 0, y = 0;
                unsigned int clientWidth = 0, clientHeight = 0;
                gRdkShellMutex.lock();
                CompositorController::getBounds(client, x, y, clientWidth, clientHeight);
                if (parameters.HasLabel("x"))
                {
                    x = parameters["x"].Number();
                }
                if (parameters.HasLabel("y"))
                {
                    y = parameters["y"].Number();
                }
                if (parameters.HasLabel("w"))
                {
                    clientWidth = parameters["w"].Number();
                }
                if (parameters.HasLabel("h"))
                {
                    clientHeight = parameters["h"].Number();
                }
                result = CompositorController::scaleToFit(client, x, y, clientWidth, clientHeight);
                gRdkShellMutex.unlock();

                if (!result) {
                  response["message"] = "failed to scale to fit";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::launchApplicationWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (!parameters.HasLabel("uri"))
            {
                result = false;
                response["message"] = "please specify uri";
            }
            if (!parameters.HasLabel("mimeType"))
            {
                result = false;
                response["message"] = "please specify mimeType";
            }
            if (result)
            {
                const string client = parameters["client"].String();
                const string uri = parameters["uri"].String();
                const string mimeType = parameters["mimeType"].String();

                gRdkShellMutex.lock();
                result = CompositorController::launchApplication(client, uri, mimeType);
                gRdkShellMutex.unlock();

                if (!result) {
                  response["message"] = "failed to launch application";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::suspendApplicationWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client = parameters["client"].String();
                std::string mimeType;

                gRdkShellMutex.lock();
                result = CompositorController::getMimeType(client, mimeType);
                if (result && mimeType == RDKSHELL_APPLICATION_MIME_TYPE_NATIVE)
                {
                    result = CompositorController::suspendApplication(client);
                }
                gRdkShellMutex.unlock();
                setVisibility(client, false);
                if (mimeType != RDKSHELL_APPLICATION_MIME_TYPE_NATIVE)
                {
                    //suspend and send the event here for non-native applications
                    static bool checkRfcOnce = true;
                    static bool suspendBrowserApps = true;
                    if (checkRfcOnce)
                    {
                        RFC_ParamData_t param;
                        bool ret = getRFCConfig("DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.HTML5Suspend.Enable", param);
                        if (true == ret && param.type == WDMP_BOOLEAN && (strncasecmp(param.value,"false",4) == 0))
                        {
                            suspendBrowserApps = false;
                        }
                        std::cout << "RDKShell will suspend browser apps: " << suspendBrowserApps << std::endl;
                    }
                    if (suspendBrowserApps)
                    {
                        std::string pluginName = client + ".1";
                        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> webkitBrowserPlugin = std::make_shared<WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>>(_T(pluginName.c_str()), _T(""));
                        if (webkitBrowserPlugin != nullptr)
                        {
                            JsonObject visibleResult;
                            JsonObject visibleParam;
                            visibleParam["params"] = "hidden";
                            webkitBrowserPlugin->Invoke<JsonObject, JsonObject>(1000, _T("visibility"), visibleParam, visibleResult);
                            std::cout << "RDKShell sent suspend event to browser\n";
                        }
                    }
                    JsonObject eventResponse;
                    eventResponse["method"] = RDKSHELL_EVENT_ON_APP_SUSPENDED;
                    JsonObject eventParams;
                    eventParams["client"] = client;
                    eventResponse["params"] = eventParams;
                    notify(RDKSHELL_EVENT_ON_APP_SUSPENDED, eventResponse);
                    std::cout << "RDKShell onApplicationSuspended event sent for " << client << std::endl;
                }

                if (!result) {
                  response["message"] = "failed to suspend application";
                }
                else
                {
                    std::cout << client << " was suspended" << std::endl << std::flush;
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::resumeApplicationWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client = parameters["client"].String();
                std::string mimeType;

                gRdkShellMutex.lock();
                result = CompositorController::getMimeType(client, mimeType);
                if (result && mimeType == RDKSHELL_APPLICATION_MIME_TYPE_NATIVE)
                {
                    result = CompositorController::resumeApplication(client);
                }
                gRdkShellMutex.unlock();
                setVisibility(client, true);
                if (mimeType != RDKSHELL_APPLICATION_MIME_TYPE_NATIVE)
                {
                    static bool checkRfcOnce = true;
                    static bool resumeBrowserApps = true;
                    if (checkRfcOnce)
                    {
                        RFC_ParamData_t param;
                        bool ret = getRFCConfig("DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.HTML5Suspend.Enable", param);
                        if (true == ret && param.type == WDMP_BOOLEAN && (strncasecmp(param.value,"false",4) == 0))
                        {
                            resumeBrowserApps = false;
                        }
                        std::cout << "RDKShell will resume browser apps: " << resumeBrowserApps << std::endl;
                    }
                    if (resumeBrowserApps)
                    {
                        std::string pluginName = client + ".1";
                        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> webkitBrowserPlugin = std::make_shared<WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>>(_T(pluginName.c_str()), _T(""));
                        if (webkitBrowserPlugin != nullptr)
                        {
                            JsonObject visibleResult;
                            JsonObject visibleParam;
                            visibleParam["params"] = "visible";
                            webkitBrowserPlugin->Invoke<JsonObject, JsonObject>(1000, _T("visibility"), visibleParam, visibleResult);
                            std::cout << "RDKShell sent resume event to browser\n";
                        }
                    }
                    //resume and send the event here for non-native applications
                    JsonObject eventResponse;
                    eventResponse["method"] = RDKSHELL_EVENT_ON_APP_RESUMED;
                    JsonObject eventParams;
                    eventParams["client"] = client;
                    eventResponse["params"] = eventParams;
                    notify(RDKSHELL_EVENT_ON_APP_RESUMED, eventResponse);
                    std::cout << "RDKShell onApplicationResumed event sent for " << client << std::endl;
                }

                if (!result) {
                  response["message"] = "failed to resume application";
                }
                else
                {
                    std::cout << client << " was resumed" << std::endl << std::flush;
                }
            }
            returnResponse(result);
        }

        // Registered methods begin

        // Events begin
        void RDKShell::notify(const std::string& event, const JsonObject& parameters)
        {
            sendNotify(event.c_str(), parameters);
        }
      // Events end

        // Internal methods begin
        bool RDKShell::moveToFront(const string& client)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::moveToFront(client);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::moveToBack(const string& client)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::moveToBack(client);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::moveBehind(const string& client, const string& target)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::moveBehind(client, target);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setFocus(const string& client)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::setFocus(client);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::kill(const string& client)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            RdkShell::CompositorController::removeListener(client, mEventListener);
            ret = CompositorController::kill(client);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::addKeyIntercept(const uint32_t& keyCode, const JsonArray& modifiers, const string& client)
        {
            uint32_t flags = 0;
            for (int i=0; i<modifiers.Length(); i++) {
              flags |= getKeyFlag(modifiers[i].String());
            }
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::addKeyIntercept(client, keyCode, flags);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::removeKeyIntercept(const uint32_t& keyCode, const JsonArray& modifiers, const string& client)
        {
            uint32_t flags = 0;
            for (int i=0; i<modifiers.Length(); i++) {
              flags |= getKeyFlag(modifiers[i].String());
            }
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::removeKeyIntercept(client, keyCode, flags);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::addKeyListeners(const string& client, const JsonArray& keys)
        {
            gRdkShellMutex.lock();
            for (int i=0; i<keys.Length(); i++) {
                const JsonObject& keyInfo = keys[i].Object();
                if (keyInfo.HasLabel("keyCode"))
                {
                    std::string keystring = keyInfo["keyCode"].String();
                    uint32_t keyCode = 0;
                    if (keystring.compare("*") == 0)
                    {
                      keyCode = ANY_KEY;
                    }
                    else
                    {
                      keyCode = keyInfo["keyCode"].Number();
                    }
                    const JsonArray modifiers = keyInfo.HasLabel("modifiers") ? keyInfo["modifiers"].Array() : JsonArray();
                    uint32_t flags = 0;
                    for (int i=0; i<modifiers.Length(); i++) {
                      flags |= getKeyFlag(modifiers[i].String());
                    }
                    std::map<std::string, RdkShellData> properties;
                    if (keyInfo.HasLabel("activate"))
                    {
                        bool activate = keyInfo["activate"].Boolean();
                        properties["activate"] = activate;
                    }
                    if (keyInfo.HasLabel("propagate"))
                    {
                        bool propagate = keyInfo["propagate"].Boolean();
                        properties["propagate"] = propagate;
                    }
                    CompositorController::addKeyListener(client, keyCode, flags, properties);
                }
            }
            gRdkShellMutex.unlock();
            return true;
        }

        bool RDKShell::removeKeyListeners(const string& client, const JsonArray& keys)
        {
            gRdkShellMutex.lock();
            for (int i=0; i<keys.Length(); i++) {
                const JsonObject& keyInfo = keys[i].Object();
                if (keyInfo.HasLabel("keyCode"))
                {
                    std::string keystring = keyInfo["keyCode"].String();
                    uint32_t keyCode = 0;
                    if (keystring.compare("*") == 0)
                    {
                      keyCode = ANY_KEY;
                    }
                    else
                    {
                      keyCode = keyInfo["keyCode"].Number();
                    }
                    const JsonArray modifiers = keyInfo.HasLabel("modifiers") ? keyInfo["modifiers"].Array() : JsonArray();
                    uint32_t flags = 0;
                    for (int i=0; i<modifiers.Length(); i++) {
                      flags |= getKeyFlag(modifiers[i].String());
                    }
                    CompositorController::removeKeyListener(client, keyCode, flags);
                }
            }
            gRdkShellMutex.unlock();
            return true;
        }

        bool RDKShell::injectKey(const uint32_t& keyCode, const JsonArray& modifiers)
        {
            bool ret = false;
            uint32_t flags = 0;
            for (int i=0; i<modifiers.Length(); i++) {
              flags |= getKeyFlag(modifiers[i].String());
            }
            gRdkShellMutex.lock();
            ret = CompositorController::injectKey(keyCode, flags);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getScreenResolution(JsonObject& out)
        {
            unsigned int width=0,height=0;
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::getScreenResolution(width, height);
            gRdkShellMutex.unlock();
            if (true == ret) {
              out["w"] = width;
              out["h"] = height;
              return true;
            }
            return false;
        }

        bool RDKShell::setScreenResolution(const unsigned int w, const unsigned int h)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            receivedResolutionRequest = true;
            resolutionWidth = w;
            resolutionHeight = h;
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::createDisplay(const string& client, const string& displayName)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::createDisplay(client, displayName);
            RdkShell::CompositorController::addListener(client, mEventListener);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getClients(JsonArray& clients)
        {
            std::vector<std::string> clientList;
            gRdkShellMutex.lock();
            CompositorController::getClients(clientList);
            gRdkShellMutex.unlock();
            for (size_t i=0; i<clientList.size(); i++) {
              clients.Add(clientList[i]);
            }
            return true;
        }

        bool RDKShell::getZOrder(JsonArray& clients)
        {
            std::vector<std::string> zOrderList;
            gRdkShellMutex.lock();
            CompositorController::getZOrder(zOrderList);
            gRdkShellMutex.unlock();
            for (size_t i=0; i<zOrderList.size(); i++) {
              clients.Add(zOrderList[i]);
            }
            return true;
        }

        bool RDKShell::getBounds(const string& client, JsonObject& bounds)
        {
            unsigned int x=0,y=0,width=0,height=0;
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::getBounds(client, x, y, width, height);
            gRdkShellMutex.unlock();
            if (true == ret) {
              bounds["x"] = x;
              bounds["y"] = y;
              bounds["w"] = width;
              bounds["h"] = height;
              return true;
            }
            return false;
        }

        bool RDKShell::setBounds(const std::string& client, const unsigned int x, const unsigned int y, const unsigned int w, const unsigned int h)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::setBounds(client, x, y, w, h);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getVisibility(const string& client, bool& visible)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::getVisibility(client, visible);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setVisibility(const string& client, const bool visible)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::setVisibility(client, visible);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getOpacity(const string& client, unsigned int& opacity)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::getOpacity(client, opacity);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setOpacity(const string& client, const unsigned int opacity)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::setOpacity(client, opacity);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getScale(const string& client, double& scaleX, double& scaleY)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::getScale(client, scaleX, scaleY);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setScale(const string& client, const double scaleX, const double scaleY)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::setScale(client, scaleX, scaleY);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::removeAnimation(const string& client)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::removeAnimation(client);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::addAnimationList(const JsonArray& animations)
        {
            gRdkShellMutex.lock();
            for (int i=0; i<animations.Length(); i++) {
                const JsonObject& animationInfo = animations[i].Object();
                if (animationInfo.HasLabel("client") && animationInfo.HasLabel("duration"))
                {
                    const string client  = animationInfo["client"].String();
                    const double duration = std::stod(animationInfo["duration"].String());
                    std::map<std::string, RdkShellData> animationProperties;
                    if (animationInfo.HasLabel("x"))
                    {
                        int32_t x = animationInfo["x"].Number();
                        animationProperties["x"] = x;
                    }
                    if (animationInfo.HasLabel("y"))
                    {
                        int32_t y = animationInfo["y"].Number();
                        animationProperties["y"] = y;
                    }
                    if (animationInfo.HasLabel("w"))
                    {
                        uint32_t width = animationInfo["w"].Number();
                        animationProperties["w"] = width;
                    }
                    if (animationInfo.HasLabel("h"))
                    {
                        uint32_t height = animationInfo["h"].Number();
                        animationProperties["h"] = height;
                    }
                    if (animationInfo.HasLabel("sx"))
                    {
                        double scaleX = std::stod(animationInfo["sx"].String());
                        animationProperties["sx"] = scaleX;
                    }
                    if (animationInfo.HasLabel("sy"))
                    {
                        double scaleY = std::stod(animationInfo["sy"].String());
                        animationProperties["sy"] = scaleY;
                    }
                    if (animationInfo.HasLabel("a"))
                    {
                        uint32_t opacity = animationInfo["a"].Number();
                        animationProperties["a"] = opacity;
                    }
                    if (animationInfo.HasLabel("tween"))
                    {
                        std::string tween = animationInfo["tween"].String();
                        animationProperties["tween"] = tween;
                    }
                    CompositorController::addAnimation(client, duration, animationProperties);
                }
            }
            gRdkShellMutex.unlock();
            return true;
        }

        bool RDKShell::enableInactivityReporting(const bool enable)
        {
            gRdkShellMutex.lock();
            CompositorController::enableInactivityReporting(enable);
            gRdkShellMutex.unlock();
            return true;
        }

        bool RDKShell::setInactivityInterval(const string interval)
        {
            gRdkShellMutex.lock();
            CompositorController::setInactivityInterval(std::stod(interval));
            gRdkShellMutex.unlock();
            return true;
        }

        // Internal methods end
    } // namespace Plugin
} // namespace WPEFramework
