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
#include <interfaces/IMemory.h>

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
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GENERATE_KEYS = "generateKey";
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
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_HOLE_PUNCH = "getHolePunch";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_HOLE_PUNCH = "setHolePunch";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_LOG_LEVEL = "getLogLevel";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_LOG_LEVEL = "setLogLevel";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_REMOVE_ANIMATION = "removeAnimation";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ADD_ANIMATION = "addAnimation";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ENABLE_INACTIVITY_REPORTING = "enableInactivityReporting";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_INACTIVITY_INTERVAL = "setInactivityInterval";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SCALE_TO_FIT = "scaleToFit";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_LAUNCH = "launch";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_LAUNCH_APP = "launchApplication";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SUSPEND = "suspend";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SUSPEND_APP = "suspendApplication";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_RESUME_APP = "resumeApplication";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_DESTROY = "destroy";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_AVAILABLE_TYPES = "getAvailableTypes";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_STATE = "getState";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_SYSTEM_MEMORY = "getSystemMemory";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_SYSTEM_RESOURCE_INFO = "getSystemResourceInfo";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_MEMORY_MONITOR = "setMemoryMonitor";

const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_USER_INACTIVITY = "onUserInactivity";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_LAUNCHED = "onApplicationLaunched";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_CONNECTED = "onApplicationConnected";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_DISCONNECTED = "onApplicationDisconnected";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_TERMINATED = "onApplicationTerminated";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_FIRST_FRAME = "onApplicationFirstFrame";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_SUSPENDED = "onApplicationSuspended";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_RESUMED = "onApplicationResumed";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_LAUNCHED = "onLaunched";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_SUSPENDED = "onSuspended";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_DESTROYED = "onDestroyed";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_DEVICE_LOW_RAM_WARNING = "onDeviceLowRamWarning";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_DEVICE_CRITICALLY_LOW_RAM_WARNING = "onDeviceCriticallyLowRamWarning";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_DEVICE_LOW_RAM_WARNING_CLEARED = "onDeviceLowRamWarningCleared";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_DEVICE_CRITICALLY_LOW_RAM_WARNING_CLEARED = "onDeviceCriticallyLowRamWarningCleared";

using namespace std;
using namespace RdkShell;
using namespace Utils;
extern int gCurrentFramerate;
bool receivedResolutionRequest = false;
unsigned int resolutionWidth = 1280;
unsigned int resolutionHeight = 720;


#define ANY_KEY 65536

enum RDKShellLaunchType
{
    UNKNOWN = 0,
    CREATE,
    ACTIVATE,
    SUSPEND,
    RESUME
};

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
                    /*PluginHost::ISubSystem* subSystems(service->SubSystems());
                    if (subSystems != nullptr)
                    {
                        subSystems->Set(PluginHost::ISubSystem::PLATFORM, nullptr);
                        subSystems->Set(PluginHost::ISubSystem::GRAPHICS, nullptr);
                        subSystems->Release();
                    }*/
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
                : AbstractPlugin(), mClientsMonitor(Core::Service<MonitorClients>::Create<MonitorClients>(this)), mEnableUserInactivityNotification(false), mCurrentService(nullptr)
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
            registerMethod(RDKSHELL_METHOD_GENERATE_KEYS, &RDKShell::generateKeyWrapper, this);
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
            registerMethod(RDKSHELL_METHOD_GET_HOLE_PUNCH, &RDKShell::getHolePunchWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_HOLE_PUNCH, &RDKShell::setHolePunchWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_LOG_LEVEL, &RDKShell::getLogLevelWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_LOG_LEVEL, &RDKShell::setLogLevelWrapper, this);
            registerMethod(RDKSHELL_METHOD_REMOVE_ANIMATION, &RDKShell::removeAnimationWrapper, this);
            registerMethod(RDKSHELL_METHOD_ADD_ANIMATION, &RDKShell::addAnimationWrapper, this);
            registerMethod(RDKSHELL_METHOD_ENABLE_INACTIVITY_REPORTING, &RDKShell::enableInactivityReportingWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_INACTIVITY_INTERVAL, &RDKShell::setInactivityIntervalWrapper, this);
            registerMethod(RDKSHELL_METHOD_SCALE_TO_FIT, &RDKShell::scaleToFitWrapper, this);
            registerMethod(RDKSHELL_METHOD_LAUNCH, &RDKShell::launchWrapper, this);
            registerMethod(RDKSHELL_METHOD_LAUNCH_APP, &RDKShell::launchApplicationWrapper, this);
            registerMethod(RDKSHELL_METHOD_SUSPEND, &RDKShell::suspendWrapper, this);
            registerMethod(RDKSHELL_METHOD_SUSPEND_APP, &RDKShell::suspendApplicationWrapper, this);
            registerMethod(RDKSHELL_METHOD_RESUME_APP, &RDKShell::resumeApplicationWrapper, this);
            registerMethod(RDKSHELL_METHOD_DESTROY, &RDKShell::destroyWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_AVAILABLE_TYPES, &RDKShell::getAvailableTypesWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_STATE, &RDKShell::getState, this);
            registerMethod(RDKSHELL_METHOD_GET_SYSTEM_MEMORY, &RDKShell::getSystemMemoryWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_SYSTEM_RESOURCE_INFO, &RDKShell::getSystemResourceInfoWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_MEMORY_MONITOR, &RDKShell::setMemoryMonitorWrapper, this);
        }

        RDKShell::~RDKShell()
        {
            LOGINFO("dtor");
            mClientsMonitor->Release();
            RDKShell::_instance = nullptr;
            mRemoteShell = false;
            CompositorController::setEventListener(nullptr);
            mEventListener = nullptr;
            mEnableUserInactivityNotification = false;
        }

        const string RDKShell::Initialize(PluginHost::IShell* service )
        {
            LOGINFO();

            std::cout << "initializing\n";

            mCurrentService = service;
            CompositorController::setEventListener(mEventListener);
            RFC_ParamData_t param;
            bool ret = getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Power.UserInactivityNotification.Enable", param);
            if (true == ret && param.type == WDMP_BOOLEAN && (strncasecmp(param.value,"true",4) == 0))
            {
              mEnableUserInactivityNotification = true;
              ret = getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Power.UserInactivityNotification.TimeMinutes", param);
              if (true == ret && param.type == WDMP_STRING)
              {
                try
                {
                  CompositorController::setInactivityInterval(std::stod(param.value));
                }
                catch (...)
                { 
                  std::cout << "RDKShell unable to set inactivity interval  " << std::endl;
                }
              }
            }

            service->Register(mClientsMonitor);

            static PluginHost::IShell* pluginService = nullptr;
            pluginService = service;

            shellThread = std::thread([]() {
                gRdkShellMutex.lock();
                RdkShell::initialize();
                PluginHost::ISubSystem* subSystems(pluginService->SubSystems());
                if (subSystems != nullptr)
                {
                    std::cout << "setting platform and graphics\n";
                    subSystems->Set(PluginHost::ISubSystem::PLATFORM, nullptr);
                    subSystems->Set(PluginHost::ISubSystem::GRAPHICS, nullptr);
                    subSystems->Release();
                }
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

            return "";
        }

        void RDKShell::Deinitialize(PluginHost::IShell* service)
        {
            LOGINFO();

            mCurrentService = nullptr;
            service->Unregister(mClientsMonitor);
        }

        string RDKShell::Information() const
        {
            return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
        }

        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > RDKShell::getThunderControllerClient(std::string callsign)
        {
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
            std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > thunderClient = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >(callsign.c_str(), "");
            return thunderClient;
        }

        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> RDKShell::getPackagerPlugin()
        {
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
            return make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>("Packager.1", "");
        }

        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> RDKShell::getOCIContainerPlugin()
        {
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
            return make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>("org.rdk.OCIContainer.1", "");
        }

        void RDKShell::RdkShellListener::onApplicationLaunched(const std::string& client)
        {
          std::cout << "RDKShell onApplicationLaunched event received ..." << client << std::endl;
          JsonObject params;
          params["client"] = client;
          mShell.notify(RDKSHELL_EVENT_ON_APP_LAUNCHED, params);
        }

        void RDKShell::RdkShellListener::onApplicationConnected(const std::string& client)
        {
          std::cout << "RDKShell onApplicationConnected event received ..." << client << std::endl;
          JsonObject params;
          params["client"] = client;
          mShell.notify(RDKSHELL_EVENT_ON_APP_CONNECTED, params);
        }

        void RDKShell::RdkShellListener::onApplicationDisconnected(const std::string& client)
        {
          std::cout << "RDKShell onApplicationDisconnected event received ..." << client << std::endl;
          JsonObject params;
          params["client"] = client;
          mShell.notify(RDKSHELL_EVENT_ON_APP_DISCONNECTED, params);
        }

        void RDKShell::RdkShellListener::onApplicationTerminated(const std::string& client)
        {
          std::cout << "RDKShell onApplicationTerminated event received ..." << client << std::endl;
          JsonObject params;
          params["client"] = client;
          mShell.notify(RDKSHELL_EVENT_ON_APP_TERMINATED, params);
        }

        void RDKShell::RdkShellListener::onApplicationFirstFrame(const std::string& client)
        {
          std::cout << "RDKShell onApplicationFirstFrame event received ..." << client << std::endl;
          JsonObject params;
          params["client"] = client;
          mShell.notify(RDKSHELL_EVENT_ON_APP_FIRST_FRAME, params);
        }

        void RDKShell::RdkShellListener::onApplicationSuspended(const std::string& client)
        {
          std::cout << "RDKShell onApplicationSuspended event received for " << client << std::endl;
          JsonObject params;
          params["client"] = client;
          mShell.notify(RDKSHELL_EVENT_ON_APP_SUSPENDED, params);
        }

        void RDKShell::RdkShellListener::onApplicationResumed(const std::string& client)
        {
          std::cout << "RDKShell onApplicationResumed event received for " << client << std::endl;
          JsonObject params;
          params["client"] = client;
          mShell.notify(RDKSHELL_EVENT_ON_APP_RESUMED, params);
        }

        void RDKShell::RdkShellListener::onUserInactive(const double minutes)
        {
          std::cout << "RDKShell onUserInactive event received ..." << minutes << std::endl;
          JsonObject params;
          params["minutes"] = std::to_string(minutes);
          mShell.notify(RDKSHELL_EVENT_ON_USER_INACTIVITY, params);
        }

        void RDKShell::RdkShellListener::onDeviceLowRamWarning(const int32_t freeKb)
        {
          std::cout << "RDKShell onDeviceLowRamWarning event received ..." << freeKb << std::endl;
          JsonObject params;
          params["ram"] = freeKb;
          mShell.notify(RDKSHELL_EVENT_DEVICE_LOW_RAM_WARNING, params);
        }

        void RDKShell::RdkShellListener::onDeviceCriticallyLowRamWarning(const int32_t freeKb)
        {
          std::cout << "RDKShell onDeviceCriticallyLowRamWarning event received ..." << freeKb << std::endl;
          JsonObject params;
          params["ram"] = freeKb;
          mShell.notify(RDKSHELL_EVENT_DEVICE_CRITICALLY_LOW_RAM_WARNING, params);
        }

        void RDKShell::RdkShellListener::onDeviceLowRamWarningCleared(const int32_t freeKb)
        {
          std::cout << "RDKShell onDeviceLowRamWarningCleared event received ..." << freeKb << std::endl;
          JsonObject params;
          params["ram"] = freeKb;
          mShell.notify(RDKSHELL_EVENT_DEVICE_LOW_RAM_WARNING_CLEARED, params);
        }

        void RDKShell::RdkShellListener::onDeviceCriticallyLowRamWarningCleared(const int32_t freeKb)
        {
          std::cout << "RDKShell onDeviceCriticallyLowRamWarningCleared event received ..." << freeKb << std::endl;
          JsonObject params;
          params["ram"] = freeKb;
          mShell.notify(RDKSHELL_EVENT_DEVICE_CRITICALLY_LOW_RAM_WARNING_CLEARED, params);
        }

        // Registered methods (wrappers) begin
        uint32_t RDKShell::moveToFrontWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
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
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }

                // Get the client mime type
                std::string mimeType;
                getMimeType(client, mimeType);

                // Kill the display
                result = kill(client);
                if (!result)
                {
                    response["message"] = "failed to kill client";
                    returnResponse(false);
                }

                // App was a DAC app, so kill the container if it's still running
                if (mimeType == RDKSHELL_APPLICATION_MIME_TYPE_DAC_NATIVE)
                {
                    LOGINFO("Killing container");

                    auto ociContainerPlugin = getOCIContainerPlugin();
                    if (!ociContainerPlugin)
                    {
                        response["message"] = "OCIContainer plugin initialisation failed";
                        returnResponse(false);
                    }

                    JsonObject containerInfoResult;
                    JsonObject stopContainerResult;
                    JsonObject param;
                    param["containerId"] = client;

                    ociContainerPlugin->Invoke<JsonObject, JsonObject>(2000, "getContainerInfo", param, containerInfoResult);

                    // If success is false, the container isn't running so nothing to do
                    if (containerInfoResult["success"].Boolean())
                    {
                        auto containerInfo = containerInfoResult["info"].Object();

                        // Dobby knows about that container - what's it doing?
                        if (containerInfo["state"] == "running" || containerInfo["state"] == "starting")
                        {
                            ociContainerPlugin->Invoke<JsonObject, JsonObject>(2000, "stopContainer", param, stopContainerResult);
                        }
                        else if (containerInfo["state"] == "paused")
                        {
                            // Paused, so force stop
                            param["force"] = true;
                            ociContainerPlugin->Invoke<JsonObject, JsonObject>(2000, "stopContainer", param, stopContainerResult);
                        }
                        else
                        {
                            response["message"] = "Container is not in a state that can be stopped";
                            returnResponse(false);
                        }

                        if (!stopContainerResult["success"].Boolean())
                        {
                            result = false;
                            response["message"] = "Failed to stop container";
                        }
                    }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                //optional param?
                const JsonArray modifiers = parameters.HasLabel("modifiers") ? parameters["modifiers"].Array() : JsonArray();

                const uint32_t keyCode = parameters["keyCode"].Number();
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                //optional param?
                const JsonArray modifiers = parameters.HasLabel("modifiers") ? parameters["modifiers"].Array() : JsonArray();

                const uint32_t keyCode = parameters["keyCode"].Number();
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                const JsonArray keys = parameters["keys"].Array();
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                const JsonArray keys = parameters["keys"].Array();
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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

            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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

            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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

        uint32_t RDKShell::generateKeyWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("keys"))
            {
                result = false;
                response["message"] = "please specify keyInputs";
            }

            if (result)
            {
                const JsonArray keyInputs = parameters["keys"].Array();

                string client = parameters.HasLabel("client") ? parameters["client"].String() : "";
                if (client.empty())
                {
                  client = parameters.HasLabel("callsign") ? parameters["callsign"].String() : "";
                }
                result = generateKey(client, keyInputs);

                if (false == result) {
                  response["message"] = "failed to generate keys";
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result) {
              JsonObject bounds;
              string client;
              if (parameters.HasLabel("client"))
              {
                client = parameters["client"].String();
              }
              else
              {
                client = parameters["callsign"].String();
              }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }

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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
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
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
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
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
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
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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

        uint32_t RDKShell::getHolePunchWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
                bool holePunch = true;
                result = getHolePunch(client, holePunch);
                if (false == result) {
                  response["message"] = "failed to get hole punch";
                }
                else {
                  response["holePunch"] = holePunch;
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::setHolePunchWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (!parameters.HasLabel("holePunch"))
            {
                result = false;
                response["message"] = "please specify hole punch (holePunch = true/false)";
            }
            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
                const bool holePunch  = parameters["holePunch"].Boolean();

                result = setHolePunch(client, holePunch);

                if (false == result) {
                  response["message"] = "failed to set hole punch";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getLogLevelWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            std::string logLevel = "INFO";
            gRdkShellMutex.lock();
            result = CompositorController::getLogLevel(logLevel);
            gRdkShellMutex.unlock();
            if (false == result) {
                response["message"] = "failed to get log level";
            }
            else {
                response["logLevel"] = logLevel;
            }
            returnResponse(result);
        }

        uint32_t RDKShell::setLogLevelWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("logLevel"))
            {
                result = false;
                response["message"] = "please specify log level (logLevel = DEBUG/INFO/WARN/ERROR/FATAL)";
            }
            if (result)
            {
                std::string logLevel  = parameters["logLevel"].String();
                std::string currentLogLevel = "INFO";
                gRdkShellMutex.lock();
                result = CompositorController::setLogLevel(logLevel);
                CompositorController::getLogLevel(currentLogLevel);
                gRdkShellMutex.unlock();
                if (false == result) {
                    response["message"] = "failed to set log level";
                }
                else
                {
                    response["logLevel"] = currentLogLevel;
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::removeAnimationWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            bool result = true;
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
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
            if (false == mEnableUserInactivityNotification)
            {
                result = false;
                response["message"] = "feature is not enabled";
            }

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
            if (false == mEnableUserInactivityNotification)
            {
                result = false;
                response["message"] = "feature is not enabled";
            }

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
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }

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

        uint32_t RDKShell::launchWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify callsign";
            }

            if (result)
            {
                RDKShellLaunchType launchType = RDKShellLaunchType::UNKNOWN;
                const string callsign = parameters["callsign"].String();
                const string callsignWithVersion = callsign + ".1";
                string type;
                string version = "0.0";
                string uri;
                int32_t x = 0;
                int32_t y = 0;
                uint32_t width = 0;
                uint32_t height = 0;
                bool suspend = false;
                bool visible = true;
                bool focused = true;
                string configuration;
                string behind;
                string displayName = "wst-" + callsign;
                bool scaleToFit = false;
                bool setSuspendResumeStateOnLaunch = true;
                bool holePunch = true;

                if (parameters.HasLabel("type"))
                {
                    type = parameters["type"].String();
                }
                if (parameters.HasLabel("version"))
                {
                    version = parameters["version"].String();
                }
                if (parameters.HasLabel("uri"))
                {
                    uri = parameters["uri"].String();
                }
                if (parameters.HasLabel("configuration"))
                {
                    configuration = parameters["configuration"].String();
                }
                if (parameters.HasLabel("behind"))
                {
                    behind = parameters["behind"].String();
                }
                if (parameters.HasLabel("displayName"))
                {
                    displayName = parameters["displayName"].String();
                }
                if (parameters.HasLabel("suspend"))
                {
                    suspend = parameters["suspend"].Boolean();
                }
                if (parameters.HasLabel("visible"))
                {
                    visible = parameters["visible"].Boolean();
                }
                if (parameters.HasLabel("focused"))
                {
                    focused = parameters["focused"].Boolean();
                }
                if (parameters.HasLabel("scaleToFit"))
                {
                    scaleToFit = parameters["scaleToFit"].Boolean();
                }
                if (parameters.HasLabel("w"))
                {
                    width = parameters["w"].Number();
                }
                if (parameters.HasLabel("h"))
                {
                    height = parameters["h"].Number();
                }
                if (parameters.HasLabel("holePunch"))
                {
                    holePunch = parameters["holePunch"].Boolean();
                }

                //check to see if plugin already exists
                bool newPluginFound = false;
                bool originalPluginFound = false;
                {
                    Core::JSON::ArrayType<PluginHost::MetaData::Service> availablePluginResult;
                    uint32_t status = getThunderControllerClient()->Get<Core::JSON::ArrayType<PluginHost::MetaData::Service>>(2000, "status", availablePluginResult);

                    for (uint16_t i = 0; i < availablePluginResult.Length(); i++)
                    {
                        PluginHost::MetaData::Service service = availablePluginResult[i];
                        std::string pluginName = service.Callsign.Value();
                        pluginName.erase(std::remove(pluginName.begin(),pluginName.end(),'\"'),pluginName.end());
                        if (!pluginName.empty() && pluginName == callsign)
                        {
                            newPluginFound = true;
                            break;
                        }
                        else if (!pluginName.empty() && pluginName == type)
                        {
                            originalPluginFound = true;
                        }
                    }
                }

                if (!newPluginFound && !originalPluginFound)
                {
                    response["message"] = "failed to launch application.  type not found";
                    returnResponse(false);
                }
                else if (!newPluginFound)
                {
                    std::cout << "attempting to clone type: " << type << " into " << callsign << std::endl;
                    JsonObject joParams;
                    joParams.Set("callsign", type);
                    joParams.Set("newcallsign",callsign.c_str());
                    JsonObject joResult;
                    // setting wait Time to 2 seconds
                    uint32_t status = getThunderControllerClient()->Invoke(2000, "clone", joParams, joResult);

                    string strParams;
                    string strResult;
                    joParams.ToString(strParams);
                    joResult.ToString(strResult);
                    launchType = RDKShellLaunchType::CREATE;
                    RdkShell::CompositorController::createDisplay(callsign, displayName, width, height);
                }

                WPEFramework::Core::JSON::String configString;

                uint32_t status = 0;
                string method = "configuration@" + callsign;
                Core::JSON::ArrayType<PluginHost::MetaData::Service> joResult;
                status = getThunderControllerClient()->Get<WPEFramework::Core::JSON::String>(2000, method.c_str(), configString);

                JsonObject configSet;
                configSet.FromString(configString.Value());

                if (!configuration.empty())
                {
                    JsonObject configurationOverrides;
                    configurationOverrides.FromString(configuration);
                    JsonObject::Iterator configurationIterator = configurationOverrides.Variants();
                    while (configurationIterator.Next())
                    {
                        configSet[configurationIterator.Label()] = configurationIterator.Current();
                    }
                }
                configSet["clientidentifier"] = displayName;
                if (!type.empty() && type == "Netflix")
                {
                    std::cout << "setting launchtosuspend for Netflix: " << suspend << std::endl;
                    configSet["launchtosuspend"] = suspend;
                    if (!suspend)
                    {
                        setSuspendResumeStateOnLaunch = false;
                    }
                }

                status = getThunderControllerClient()->Set<JsonObject>(2000, method.c_str(), configSet);

                if (launchType == RDKShellLaunchType::UNKNOWN)
                {
                    status = 0;
                    string statusMethod = "status@"+callsign;
                    Core::JSON::ArrayType<PluginHost::MetaData::Service> serviceResults;
                    status = getThunderControllerClient()->Get<Core::JSON::ArrayType<PluginHost::MetaData::Service> >(2000, statusMethod.c_str(),serviceResults);

                    if (status == 0 && serviceResults.Length() > 0)
                    {
                        PluginHost::MetaData::Service service = serviceResults[0];
                        if (service.JSONState == PluginHost::MetaData::Service::state::DEACTIVATED ||
                            service.JSONState == PluginHost::MetaData::Service::state::DEACTIVATION ||
                            service.JSONState == PluginHost::MetaData::Service::state::PRECONDITION)
                        {
                            launchType = RDKShellLaunchType::ACTIVATE;
                            JsonObject activateParams;
                            activateParams.Set("callsign",callsign.c_str());
                            JsonObject activateResult;
                            status = getThunderControllerClient()->Invoke(2000, "activate", activateParams, joResult);
                        }
                    }
                    else
                    {
                        launchType = RDKShellLaunchType::ACTIVATE;
                        JsonObject activateParams;
                        activateParams.Set("callsign",callsign.c_str());
                        JsonObject activateResult;
                        status = getThunderControllerClient()->Invoke(2000, "activate", activateParams, joResult);
                    }
                }
                else
                {
                    JsonObject activateParams;
                    activateParams.Set("callsign",callsign.c_str());
                    JsonObject activateResult;
                    status = getThunderControllerClient()->Invoke(2000, "activate", activateParams, joResult);
                }

                if (status > 0)
                {
                    result = false;
                    std::cout << "error activating plugin.  status code is " << status << std::endl;
                }
                else
                {
                    uint32_t tempX = 0;
                    uint32_t tempY = 0;
                    uint32_t screenWidth = 0;
                    uint32_t screenHeight = 0;
                    gRdkShellMutex.lock();
                    CompositorController::getBounds(callsign, tempX, tempY, screenWidth, screenHeight);
                    gRdkShellMutex.unlock();
                    width = screenWidth;
                    height = screenHeight;
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
                        width = parameters["w"].Number();
                    }
                    if (parameters.HasLabel("h"))
                    {
                        height = parameters["h"].Number();
                    }
                    gRdkShellMutex.lock();
                    CompositorController::setBounds(callsign, x, y, width, height);
                    gRdkShellMutex.unlock();

                    if (scaleToFit)
                    {
                        std::cout << "scaling app to fit full screen" << std::endl;
                        double sx = 1.0;
                        double sy = 1.0;
                        if (width != screenWidth)
                        {
                            sx = (double)screenWidth / (double)width;
                        }
                        if (height != screenHeight)
                        {
                            sy = (double)screenHeight / (double)height;
                        }
                        setScale(callsign, sx, sy);
                    }

                    if (!behind.empty())
                    {
                        //check to ensure behind is a active client
                        std::cout << "moving " << callsign << " behind " << behind << std::endl;
                        bool moveBehindResult = moveBehind(callsign, behind);
                        if (!moveBehindResult)
                        {
                            std::cout << "unable to move behind " << behind << std::endl;
                        }
                    }
                    if (setSuspendResumeStateOnLaunch)
                    {
                        if (suspend)
                        {

                            WPEFramework::Core::JSON::String stateString;
                            stateString = "suspended";
                            status = getThunderControllerClient(callsignWithVersion)->Set<WPEFramework::Core::JSON::String>(2000, "state", stateString);
                            
                            std::cout << "setting the state to suspended\n";
                            if (launchType == RDKShellLaunchType::UNKNOWN)
                            {
                                launchType = RDKShellLaunchType::SUSPEND;
                            }
                            visible = false;
                        }
                        else
                        {
                            WPEFramework::Core::JSON::String stateString;
                            stateString = "resumed";
                            status = getThunderControllerClient(callsignWithVersion)->Set<WPEFramework::Core::JSON::String>(2000, "state", stateString);
                            if (launchType == RDKShellLaunchType::UNKNOWN)
                            {
                                launchType = RDKShellLaunchType::RESUME;
                            }
                            
                            std::cout << "setting the state to resumed\n";
                        }
                    }
                    setVisibility(callsign, visible);
                    setHolePunch(callsign, holePunch);
                    if (!visible)
                    {
                        focused = false;
                    }
                    if (focused)
                    {
                        std::cout << "setting the focused app to " << callsign << std::endl;
                        setFocus(callsign);
                    }

                    JsonObject urlResult;
                    if (!uri.empty())
                    {
                        WPEFramework::Core::JSON::String urlString;
                        urlString = uri;
                        status = getThunderControllerClient(callsignWithVersion)->Set<WPEFramework::Core::JSON::String>(2000, "url",urlString);
                        if (status > 0)
                        {
                            std::cout << "failed to set url to " << uri << " with status code " << status << std::endl;
                        }
                    }
                }

                if (status > 0 || !result)
                {
                    result = false;
                }
                else
                {
                    string launchTypeString;
                    switch (launchType)
                    {
                        case CREATE:
                            launchTypeString = "create";
                            break;
                        case ACTIVATE:
                            launchTypeString = "activate";
                            break;
                        case SUSPEND:
                            launchTypeString = "suspend";
                            break;
                        case RESUME:
                            launchTypeString = "resume";
                            break;
                        default:
                            launchTypeString = "unknown";
                            break;
                    }
                    onLaunched(callsign, launchTypeString);
                    response["launchType"] = launchTypeString;
                }
                
            }
            if (!result) 
            {
                response["message"] = "failed to launch application";
            }
            returnResponse(result);
        }

        uint32_t RDKShell::suspendWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify callsign";
            }
            if (result)
            {
                const string callsign = parameters["callsign"].String();
                std::cout << "about to suspend " << callsign << std::endl;

                WPEFramework::Core::JSON::String stateString;
                stateString = "suspended";
                const string callsignWithVersion = callsign + ".1";
                uint32_t status = getThunderControllerClient(callsignWithVersion)->Set<WPEFramework::Core::JSON::String>(2000, "state", stateString);

                if (status > 0)
                {
                    std::cout << "failed to suspend " << callsign << ".  status: " << status << std::endl;
                    result = false;
                }
                else
                {
                    setVisibility(callsign, false);
                    onSuspended(callsign);
                }
            }
            if (!result)
            {
                response["message"] = "failed to suspend application";
            }
            returnResponse(result);
        }

        uint32_t RDKShell::destroyWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify callsign";
            }
            if (result)
            {
                const string callsign = parameters["callsign"].String();
                std::cout << "destroying " << callsign << std::endl;
                JsonObject joParams;
                joParams.Set("callsign",callsign.c_str());
                JsonObject joResult;
                // setting wait Time to 2 seconds
                uint32_t status = getThunderControllerClient()->Invoke(2000, "deactivate", joParams, joResult);
                if (status > 0)
                {
                    std::cout << "failed to destroy " << callsign << ".  status: " << status << std::endl;
                    result = false;
                }
                else
                {
                    onDestroyed(callsign);
                }
            }
            if (!result)
            {
                response["message"] = "failed to destroy application";
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

                if (mimeType == RDKSHELL_APPLICATION_MIME_TYPE_DAC_NATIVE)
                {
                    // Starting a DAC app. Get the info from Packager
                    LOGINFO("Starting DAC app");

                    auto packagerPlugin = getPackagerPlugin();
                    if (!packagerPlugin)
                    {
                        response["message"] = "Packager initialisation failed";
                        returnResponse(false);
                    }

                    // See if the app is actually installed
                    JsonObject installParams;
                    JsonObject installResult;

                    installParams.Set("pkgId", uri.c_str());
                    packagerPlugin->Invoke<JsonObject, JsonObject>(1000, "isInstalled", installParams, installResult);

                    if (!installResult.Get("available").Boolean())
                    {
                        response["message"] = "Packager reports app is not installed";
                        returnResponse(false);
                    }

                    // App is installed, find the bundle location
                    JsonObject infoParams;
                    JsonObject infoResult;

                    infoParams.Set("pkgId", uri.c_str());
                    packagerPlugin->Invoke<JsonObject, JsonObject>(1000, "getPackageInfo", infoParams, infoResult);

                    string bundlePath = infoResult["bundlePath"].String();

                    // We know where the app lives and are ready to start it,
                    // create a display with rdkshell
                    if (!createDisplay(client, uri))
                    {
                        response["message"] = "Could not create display";
                        returnResponse(false);
                    }

                    string runtimeDir = getenv("XDG_RUNTIME_DIR");
                    string display = runtimeDir + "/" + uri;

                    // Set mime type
                    if (!setMimeType(client, mimeType))
                    {
                        LOGWARN("Failed to set mime type - non fatal...");
                    }

                    // Start container
                    auto ociContainerPlugin = getOCIContainerPlugin();
                    if (!ociContainerPlugin)
                    {
                        response["message"] = "OCIContainer initialisation failed";
                        returnResponse(false);
                    }

                    JsonObject ociContainerResult;
                    JsonObject param;

                    param["containerId"] = uri;
                    param["bundlePath"] = bundlePath;
                    param["westerosSocket"] = display;

                    ociContainerPlugin->Invoke<JsonObject, JsonObject>(2000, "startContainer", param, ociContainerResult);

                    if (!ociContainerResult["success"].Boolean())
                    {
                        // Something went wrong starting the container, destory the display we just created
                        kill(client);
                        response["message"] = "Could not start Dobby container";
                        returnResponse(false);
                    }
                }
                else if (mimeType == RDKSHELL_APPLICATION_MIME_TYPE_NATIVE)
                {
                    gRdkShellMutex.lock();
                    result = CompositorController::launchApplication(client, uri, mimeType);
                    gRdkShellMutex.unlock();

                    if (!result)
                    {
                        response["message"] = "failed to launch application";
                    }
                }
                else
                {
                    result = false;
                    response["message"] = "Unsupported MIME type";
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
                if (!getMimeType(client, mimeType))
                {
                    response["message"] = "Could not determine app mime type";
                    returnResponse(false);
                }

                if (mimeType == RDKSHELL_APPLICATION_MIME_TYPE_NATIVE)
                {
                    gRdkShellMutex.lock();
                    result = CompositorController::suspendApplication(client);
                    gRdkShellMutex.unlock();
                }
                else if (mimeType == RDKSHELL_APPLICATION_MIME_TYPE_DAC_NATIVE)
                {
                    // Pause the container with Dobby
                    LOGINFO("Pausing DAC app");

                    auto ociContainerPlugin = getOCIContainerPlugin();

                    if (!ociContainerPlugin)
                    {
                        response["message"] = "OCIContainer initialisation failed";
                        returnResponse(false);
                    }

                    JsonObject ociContainerResult;
                    JsonObject param;
                    param["containerId"] = client;

                    ociContainerPlugin->Invoke<JsonObject, JsonObject>(2000, "pauseContainer", param, ociContainerResult);

                    if (!ociContainerResult["success"].Boolean())
                    {
                        response["message"] = "Could not pause container";
                        returnResponse(false);
                    }
                }
                else
                {
                    response["message"] = "Unsupported mime type";
                    returnResponse(false);
                }

                // Make the application hidden
                result = setVisibility(client, false);
                if (!result)
                {
                    response["message"] = "failed to suspend application";
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
                if (!getMimeType(client, mimeType))
                {
                    response["message"] = "Could not determine app mime type";
                    returnResponse(false);
                }

                if (mimeType == RDKSHELL_APPLICATION_MIME_TYPE_NATIVE)
                {
                    gRdkShellMutex.lock();
                    result = CompositorController::resumeApplication(client);
                    gRdkShellMutex.unlock();
                }
                else if (mimeType == RDKSHELL_APPLICATION_MIME_TYPE_DAC_NATIVE)
                {
                    // Resume the container with Dobby
                    LOGINFO("Resuming DAC app");

                    auto ociContainerPlugin = getOCIContainerPlugin();
                    if (!ociContainerPlugin)
                    {
                        response["message"] = "OCIContainer initialisation failed";
                        returnResponse(false);
                    }

                    JsonObject ociContainerResult;
                    JsonObject param;

                    param["containerId"] = client;
                    ociContainerPlugin->Invoke<JsonObject, JsonObject>(2000, "resumeContainer", param, ociContainerResult);

                    if (!ociContainerResult["success"].Boolean())
                    {
                        response["message"] = "Could not resume container";
                        returnResponse(false);
                    }
                }
                else
                {
                    response["message"] = "Unsupported mime type";
                    returnResponse(false);
                }

                // Make the application visible
                result = setVisibility(client, true);

                if (!result)
                {
                    response["message"] = "failed to resume application";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getAvailableTypesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            string method = "status";
            Core::JSON::ArrayType<PluginHost::MetaData::Service> joResult;
            uint32_t status = getThunderControllerClient()->Get<Core::JSON::ArrayType<PluginHost::MetaData::Service>>(2000, method.c_str(), joResult);

            JsonArray availableTypes;
            for (uint16_t i = 0; i < joResult.Length(); i++)
            {
                PluginHost::MetaData::Service service = joResult[i];
                std::string configLine;
                service.Configuration.ToString(configLine);
                if (!configLine.empty())
                {
                    JsonObject serviceConfig = JsonObject(configLine.c_str());
                    if (serviceConfig.HasLabel("clientidentifier"))
                    {
                        std::string typeName;
                        service.Callsign.ToString(typeName);
                        typeName.erase(std::remove(typeName.begin(),typeName.end(),'\"'),typeName.end());
                        availableTypes.Add(typeName);
                    }
                }
            }
            response["types"] = availableTypes;

            returnResponse(result);
        }

        uint32_t RDKShell::getState(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            string method = "status";
            Core::JSON::ArrayType<PluginHost::MetaData::Service> joResult;
            uint32_t status = getThunderControllerClient()->Get<Core::JSON::ArrayType<PluginHost::MetaData::Service>>(2000, method.c_str(), joResult);


            JsonArray stateArray;
            for (uint16_t i = 0; i < joResult.Length(); i++)
            {
                PluginHost::MetaData::Service service = joResult[i];
                if (service.JSONState != PluginHost::MetaData::Service::state::DEACTIVATED &&
                    service.JSONState != PluginHost::MetaData::Service::state::DEACTIVATION &&
                    service.JSONState != PluginHost::MetaData::Service::state::PRECONDITION)
                {
                    std::string configLine;
                    service.Configuration.ToString(configLine);
                    if (!configLine.empty())
                    {
                        JsonObject serviceConfig = JsonObject(configLine.c_str());
                        if (serviceConfig.HasLabel("clientidentifier"))
                        {
                            std::string callsign;
                            service.Callsign.ToString(callsign);
                            callsign.erase(std::remove(callsign.begin(),callsign.end(),'\"'),callsign.end());

                            WPEFramework::Core::JSON::String stateString;
                            const string callsignWithVersion = callsign + ".1";
                            uint32_t stateStatus = getThunderControllerClient(callsignWithVersion)->Get<WPEFramework::Core::JSON::String>(2000, "state", stateString);

                            if (stateStatus == 0)
                            {
                                WPEFramework::Core::JSON::String urlString;
                                uint32_t urlStatus = getThunderControllerClient(callsignWithVersion)->Get<WPEFramework::Core::JSON::String>(2000, "url",urlString);

                                JsonObject typeObject;
                                typeObject["callsign"] = callsign;
                                typeObject["state"] = stateString.Value();
                                if (urlStatus == 0)
                                {
                                    typeObject["uri"] = urlString.Value();
                                }
                                else
                                {
                                    typeObject["uri"] = "";
                                }
                                stateArray.Add(typeObject);
                            }
                        }
                    }
                }
            }
            response["state"] = stateArray;

            returnResponse(result);
        }

        uint32_t RDKShell::getSystemMemoryWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            uint32_t freeKb=0, usedSwapKb=0, totalKb=0;
            result = systemMemory(freeKb, totalKb, usedSwapKb);
            if (!result) {
              response["message"] = "failed to get system Ram";
            }
            else
            {
              response["freeRam"] = freeKb;
              response["swapRam"] = usedSwapKb;
              response["totalRam"] = totalKb;
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getSystemResourceInfoWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            JsonArray memoryInfo;

            string method = "status";
            Core::JSON::ArrayType<PluginHost::MetaData::Service> joResult;
            uint32_t status = getThunderControllerClient()->Get<Core::JSON::ArrayType<PluginHost::MetaData::Service>>(2000, method.c_str(), joResult);

            /*std::cout << "DEACTIVATED: " << PluginHost::MetaData::Service::state::DEACTIVATED << std::endl;
                    std::cout << "DEACTIVATION: " << PluginHost::MetaData::Service::state::DEACTIVATION << std::endl;
                    std::cout << "ACTIVATED: " << PluginHost::MetaData::Service::state::ACTIVATED << std::endl;
                    std::cout << "ACTIVATION: " << PluginHost::MetaData::Service::state::ACTIVATION << std::endl;
                    std::cout << "DESTROYED: " << PluginHost::MetaData::Service::state::DESTROYED << std::endl;
                    std::cout << "PRECONDITION: " << PluginHost::MetaData::Service::state::PRECONDITION << std::endl;
                    std::cout << "SUSPENDED: " << PluginHost::MetaData::Service::state::SUSPENDED << std::endl;
                    std::cout << "RESUMED: " << PluginHost::MetaData::Service::state::RESUMED << std::endl;*/

            JsonArray stateArray;
            for (uint16_t i = 0; i < joResult.Length(); i++)
            {
                PluginHost::MetaData::Service service = joResult[i];
                if (service.JSONState != PluginHost::MetaData::Service::state::DEACTIVATED &&
                    service.JSONState != PluginHost::MetaData::Service::state::DEACTIVATION &&
                    service.JSONState != PluginHost::MetaData::Service::state::PRECONDITION)
                {
                    std::string configLine;
                    service.Configuration.ToString(configLine);
                    if (!configLine.empty())
                    {
                        JsonObject serviceConfig = JsonObject(configLine.c_str());
                        if (serviceConfig.HasLabel("clientidentifier"))
                        {
                            std::string callsign;
                            service.Callsign.ToString(callsign);
                            callsign.erase(std::remove(callsign.begin(),callsign.end(),'\"'),callsign.end());

                            WPEFramework::Core::JSON::String stateString;
                            const string callsignWithVersion = callsign + ".1";
                            uint32_t stateStatus = getThunderControllerClient(callsignWithVersion)->Get<WPEFramework::Core::JSON::String>(2000, "state", stateString);

                            if (stateStatus == 0)
                            {
                                result = pluginMemoryUsage(callsign, memoryInfo);
                            }
                        }
                    }
                }
            }

            response["types"] = memoryInfo;

            returnResponse(result);
        }

        uint32_t RDKShell::setMemoryMonitorWrapper(const JsonObject& parameters, JsonObject& response)
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
              bool enable = parameters["enable"].Boolean();
              if (parameters.HasLabel("interval"))
              {
                RdkShell::setMemoryMonitor(enable, std::stod(parameters["interval"].String()));
              }
              else
              {
                RdkShell::setMemoryMonitor(enable, 5); //default to 5 second interval
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
            std::vector<std::string> clientList;
            CompositorController::getClients(clientList);
            bool targetFound = false;
            for (size_t i=0; i<clientList.size(); i++)
            {
                if (strcasecmp(clientList[i].c_str(),target.c_str()) == 0)
                {
                    targetFound = true;
                    break;
                }
            }
            if (targetFound)
            {
                ret = CompositorController::moveBehind(client, target);
            }
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

        bool RDKShell::generateKey(const string& client, const JsonArray& keyInputs)
        {
            bool ret = false;
            for (int i=0; i<keyInputs.Length(); i++) {
                const JsonObject& keyInputInfo = keyInputs[i].Object();
                if (keyInputInfo.HasLabel("keyCode"))
                {
                  const uint32_t keyCode = keyInputInfo["keyCode"].Number();
                  const uint32_t delay = keyInputInfo["delay"].Number();
                  sleep(delay);
                  const JsonArray modifiers = keyInputInfo.HasLabel("modifiers") ? keyInputInfo["modifiers"].Array() : JsonArray();
                  std::string keyClient = keyInputInfo.HasLabel("client")? keyInputInfo["client"].String(): client;
                  if (keyClient.empty())
                  {
                    keyClient = keyInputInfo.HasLabel("callsign")? keyInputInfo["callsign"].String(): "";
                  }
                  uint32_t flags = 0;
                  for (int k=0; k<modifiers.Length(); k++) {
                    flags |= getKeyFlag(modifiers[k].String());
                  }
                  gRdkShellMutex.lock();
                  ret = CompositorController::generateKey(keyClient, keyCode, flags);
                  gRdkShellMutex.unlock();
                }
            }
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
            gRdkShellMutex.lock();
            receivedResolutionRequest = true;
            resolutionWidth = w;
            resolutionHeight = h;
            gRdkShellMutex.unlock();
            return true;
        }

        bool RDKShell::setMimeType(const string& client, const string& mimeType)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::setMimeType(client, mimeType);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getMimeType(const string& client, string& mimeType)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::getMimeType(client, mimeType);
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

        bool RDKShell::getHolePunch(const string& client, bool& holePunch)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::getHolePunch(client, holePunch);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setHolePunch(const string& client, const bool holePunch)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::setHolePunch(client, holePunch);
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
                    if (animationInfo.HasLabel("delay"))
                    {
                        try
                        {
                          double duration = std::stod(animationInfo["delay"].String());
                          animationProperties["delay"] = duration;
                        }
                        catch (...)
                        {
                          std::cout << "RDKShell unable to set delay for animation  " << std::endl;
                        }
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
            try
            {
              CompositorController::setInactivityInterval(std::stod(interval));
            }
            catch (...) 
            {
              std::cout << "RDKShell unable to set inactivity interval  " << std::endl;
            }
            gRdkShellMutex.unlock();
            return true;
        }

        void RDKShell::onLaunched(const std::string& client, const string& launchType)
        {
            std::cout << "RDKShell onLaunched event received for " << client << std::endl;
            JsonObject params;
            params["client"] = client;
            params["launchType"] = launchType;
            notify(RDKSHELL_EVENT_ON_LAUNCHED, params);
        }

        void RDKShell::onSuspended(const std::string& client)
        {
            std::cout << "RDKShell onSuspended event received for " << client << std::endl;
            JsonObject params;
            params["client"] = client;

            notify(RDKSHELL_EVENT_ON_SUSPENDED, params);
        }

        void RDKShell::onDestroyed(const std::string& client)
        {
            std::cout << "RDKShell onDestroyed event received for " << client << std::endl;
            JsonObject params;
            params["client"] = client;
            notify(RDKSHELL_EVENT_ON_DESTROYED, params);
        }

        bool RDKShell::systemMemory(uint32_t &freeKb, uint32_t & totalKb, uint32_t & usedSwapKb)
        {
            gRdkShellMutex.lock();
            bool ret = RdkShell::systemRam(freeKb, totalKb, usedSwapKb);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::pluginMemoryUsage(const string callsign, JsonArray& memoryInfo)
        {
            JsonObject memoryDetails;
            PluginHost::IShell* plugin(mCurrentService->QueryInterfaceByCallsign<PluginHost::IShell>(callsign.c_str()));
            memoryDetails["callsign"] = callsign;
            memoryDetails["ram"] = -1;
            if (nullptr != plugin)
            {
                Exchange::IMemory* memory = plugin->QueryInterface<Exchange::IMemory>();

                if (memory != nullptr)
                {
                    memoryDetails["ram"] = memory->Resident()/1024;
                }
                else
                {
                    std::cout << "Memory information not available for " << callsign << std::endl;
                }
                memoryInfo.Add(memoryDetails);
            }
            return true;
        }

        // Internal methods end
    } // namespace Plugin
} // namespace WPEFramework
