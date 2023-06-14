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

#include "RDKWindowManager.h"
#include <string>
#include <iostream>
#include <mutex>
#include <thread>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <rdkshell/compositorcontroller.h>
#include <rdkshell/application.h>
#include <rdkshell/logger.h>
#include <rdkshell/logger.h>
#include <rdkshell/linuxkeys.h>
#include "UtilsJsonRpc.h"
#include "UtilsUnused.h"
#include "UtilsString.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

const string WPEFramework::Plugin::RDKWindowManager::SERVICE_NAME = "org.rdk.RDKWindowManager";
//methods
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_MOVE_TO_FRONT = "moveToFront";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_MOVE_TO_BACK = "moveToBack";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_MOVE_BEHIND = "moveBehind";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SET_FOCUS = "setFocus";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_KILL = "kill";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_ADD_KEY_INTERCEPT = "addKeyIntercept";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_ADD_KEY_INTERCEPTS = "addKeyIntercepts";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_REMOVE_KEY_INTERCEPT = "removeKeyIntercept";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_ADD_KEY_LISTENER = "addKeyListener";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_REMOVE_KEY_LISTENER = "removeKeyListener";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_ADD_KEY_METADATA_LISTENER = "addKeyMetadataListener";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_REMOVE_KEY_METADATA_LISTENER = "removeKeyMetadataListener";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_INJECT_KEY = "injectKey";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GENERATE_KEYS = "generateKey";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_SCREEN_RESOLUTION = "getScreenResolution";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SET_SCREEN_RESOLUTION = "setScreenResolution";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_CREATE_DISPLAY = "createDisplay";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_CLIENTS = "getClients";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_Z_ORDER = "getZOrder";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_BOUNDS = "getBounds";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SET_BOUNDS = "setBounds";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_VISIBILITY = "getVisibility";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SET_VISIBILITY = "setVisibility";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_OPACITY = "getOpacity";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SET_OPACITY = "setOpacity";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_SCALE = "getScale";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SET_SCALE = "setScale";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_HOLE_PUNCH = "getHolePunch";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SET_HOLE_PUNCH = "setHolePunch";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_LOG_LEVEL = "getLogLevel";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SET_LOG_LEVEL = "setLogLevel";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_ENABLE_INACTIVITY_REPORTING = "enableInactivityReporting";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SET_INACTIVITY_INTERVAL = "setInactivityInterval";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_RESET_INACTIVITY_TIME = "resetInactivityTime";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SCALE_TO_FIT = "scaleToFit";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_ENABLE_KEYREPEATS = "enableKeyRepeats";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_KEYREPEATS_ENABLED = "getKeyRepeatsEnabled";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SET_TOPMOST = "setTopmost";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_VIRTUAL_RESOLUTION = "getVirtualResolution";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SET_VIRTUAL_RESOLUTION = "setVirtualResolution";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_ENABLE_VIRTUAL_DISPLAY = "enableVirtualDisplay";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_VIRTUAL_DISPLAY_ENABLED = "getVirtualDisplayEnabled";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_HIDE_ALL_CLIENTS = "hideAllClients";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_IGNORE_KEY_INPUTS = "ignoreKeyInputs";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_ENABLE_LOGS_FLUSHING = "enableLogsFlushing";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_LOGS_FLUSHING_ENABLED = "getLogsFlushingEnabled";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SHOW_CURSOR = "showCursor";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_HIDE_CURSOR = "hideCursor";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_CURSOR_SIZE = "getCursorSize";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SET_CURSOR_SIZE = "setCursorSize";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_ENABLE_INPUT_EVENTS = "enableInputEvents";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_KEY_REPEAT_CONFIG = "keyRepeatConfig";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_GET_GRAPHICS_FRAME_RATE = "getGraphicsFrameRate";
const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_METHOD_SET_GRAPHICS_FRAME_RATE = "setGraphicsFrameRate";

const string WPEFramework::Plugin::RDKWindowManager::RDK_WINDOW_MANAGER_EVENT_ON_USER_INACTIVITY = "onUserInactivity";

using namespace std;
using namespace RdkShell;
using namespace Utils;
extern int gCurrentFramerate;
bool receivedResolutionRequest = false;
unsigned int resolutionWidth = 1280;
unsigned int resolutionHeight = 720;

static bool sRunning = true;

#define ANY_KEY 65536
#define KEYCODE_INVALID -1
#define TRY_LOCK_WAIT_TIME_IN_MS 250

namespace WPEFramework {

    namespace {

        static Plugin::Metadata<Plugin::RDKWindowManager> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

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

        SERVICE_REGISTRATION(RDKWindowManager, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        RDKWindowManager* RDKWindowManager::_instance = nullptr;
        std::mutex gRdkWindowManagerMutex;
        static std::thread shellThread;

        struct CreateDisplayRequest
        {
            CreateDisplayRequest(std::string client, std::string displayName, uint32_t displayWidth=0, uint32_t displayHeight=0, bool virtualDisplayEnabled=false, uint32_t virtualWidth=0, uint32_t virtualHeight=0, bool topmost = false, bool focus = false): mClient(client), mDisplayName(displayName), mDisplayWidth(displayWidth), mDisplayHeight(displayHeight), mVirtualDisplayEnabled(virtualDisplayEnabled), mVirtualWidth(virtualWidth),mVirtualHeight(virtualHeight), mTopmost(topmost), mFocus(focus), mResult(false) , mAutoDestroy(true)
            {
                sem_init(&mSemaphore, 0, 0);
            }
  
            ~CreateDisplayRequest()
            {
                sem_destroy(&mSemaphore);
            }

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
            KillClientRequest(std::string client): mClient(client), mResult(false)
            {
                sem_init(&mSemaphore, 0, 0);
            }
  
            ~KillClientRequest()
            {
                sem_destroy(&mSemaphore);
            }

            std::string mClient;
            sem_t mSemaphore;
            bool mResult;
        };

        std::vector<std::shared_ptr<CreateDisplayRequest>> gCreateDisplayRequests;
        std::vector<std::shared_ptr<KillClientRequest>> gKillClientRequests;

        void lockRdkWindowManagerMutex()
        {
            bool lockAcquired = false;
            double startTime = RdkShell::milliseconds();
            while (!lockAcquired && (RdkShell::milliseconds() - startTime) < TRY_LOCK_WAIT_TIME_IN_MS)
            {
                lockAcquired = gRdkWindowManagerMutex.try_lock();
            }
            if (!lockAcquired)
            {
                std::cout << "unable to get lock for defaulting to normal lock\n";
                gRdkWindowManagerMutex.lock();
            }
            /*else
            {
                std::cout << "lock was acquired via try\n";
            }*/
        }

        static bool isClientExists(std::string client)
        {
            bool exist = false;
            lockRdkWindowManagerMutex();
            for (unsigned int i=0; i<gCreateDisplayRequests.size(); i++)
            {
              if (gCreateDisplayRequests[i]->mClient.compare(client) == 0)
              {
                exist = true;
                break;
              }
            }
            gRdkWindowManagerMutex.unlock();

            if (!exist)
            {
                std::vector<std::string> clientList;
                lockRdkWindowManagerMutex();
                CompositorController::getClients(clientList);
                gRdkWindowManagerMutex.unlock();
                std::string newClient(client);
                transform(newClient.begin(), newClient.end(), newClient.begin(), ::tolower);
                if (std::find(clientList.begin(), clientList.end(), newClient) != clientList.end())
                {
                    exist = true;
                }
            }
            return exist;
        }

        std::string toLower(const std::string& clientName)
        {
            std::string displayName = clientName;
            std::transform(displayName.begin(), displayName.end(), displayName.begin(), [](unsigned char c){ return std::tolower(c); });
            return displayName;
        }

        RDKWindowManager::RDKWindowManager()
                : PluginHost::JSONRPC(),
                mEnableUserInactivityNotification(true),
                mCurrentService(nullptr)
        {
            LOGINFO("ctor");
            RDKWindowManager::_instance = this;
            mEventListener = std::make_shared<RdkShellListener>(this);
            Register(RDK_WINDOW_MANAGER_METHOD_MOVE_TO_FRONT, &RDKWindowManager::moveToFrontWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_MOVE_TO_BACK, &RDKWindowManager::moveToBackWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_MOVE_BEHIND, &RDKWindowManager::moveBehindWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SET_FOCUS, &RDKWindowManager::setFocusWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_KILL, &RDKWindowManager::killWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_ADD_KEY_INTERCEPT, &RDKWindowManager::addKeyInterceptWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_ADD_KEY_INTERCEPTS, &RDKWindowManager::addKeyInterceptsWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_REMOVE_KEY_INTERCEPT, &RDKWindowManager::removeKeyInterceptWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_ADD_KEY_LISTENER, &RDKWindowManager::addKeyListenersWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_REMOVE_KEY_LISTENER, &RDKWindowManager::removeKeyListenersWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_ADD_KEY_METADATA_LISTENER, &RDKWindowManager::addKeyMetadataListenerWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_REMOVE_KEY_METADATA_LISTENER, &RDKWindowManager::removeKeyMetadataListenerWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_INJECT_KEY, &RDKWindowManager::injectKeyWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GENERATE_KEYS, &RDKWindowManager::generateKeyWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_SCREEN_RESOLUTION, &RDKWindowManager::getScreenResolutionWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SET_SCREEN_RESOLUTION, &RDKWindowManager::setScreenResolutionWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_CREATE_DISPLAY, &RDKWindowManager::createDisplayWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_CLIENTS, &RDKWindowManager::getClientsWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_Z_ORDER, &RDKWindowManager::getZOrderWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_BOUNDS, &RDKWindowManager::getBoundsWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SET_BOUNDS, &RDKWindowManager::setBoundsWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_VISIBILITY, &RDKWindowManager::getVisibilityWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SET_VISIBILITY, &RDKWindowManager::setVisibilityWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_OPACITY, &RDKWindowManager::getOpacityWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SET_OPACITY, &RDKWindowManager::setOpacityWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_SCALE, &RDKWindowManager::getScaleWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SET_SCALE, &RDKWindowManager::setScaleWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_HOLE_PUNCH, &RDKWindowManager::getHolePunchWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SET_HOLE_PUNCH, &RDKWindowManager::setHolePunchWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_LOG_LEVEL, &RDKWindowManager::getLogLevelWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SET_LOG_LEVEL, &RDKWindowManager::setLogLevelWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_ENABLE_INACTIVITY_REPORTING, &RDKWindowManager::enableInactivityReportingWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SET_INACTIVITY_INTERVAL, &RDKWindowManager::setInactivityIntervalWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_RESET_INACTIVITY_TIME, &RDKWindowManager::resetInactivityTimeWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SCALE_TO_FIT, &RDKWindowManager::scaleToFitWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_ENABLE_KEYREPEATS, &RDKWindowManager::enableKeyRepeatsWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_KEYREPEATS_ENABLED, &RDKWindowManager::getKeyRepeatsEnabledWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SET_TOPMOST, &RDKWindowManager::setTopmostWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_VIRTUAL_RESOLUTION, &RDKWindowManager::getVirtualResolutionWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SET_VIRTUAL_RESOLUTION, &RDKWindowManager::setVirtualResolutionWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_ENABLE_VIRTUAL_DISPLAY, &RDKWindowManager::enableVirtualDisplayWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_VIRTUAL_DISPLAY_ENABLED, &RDKWindowManager::getVirtualDisplayEnabledWrapper, this);          
            Register(RDK_WINDOW_MANAGER_METHOD_HIDE_ALL_CLIENTS, &RDKWindowManager::hideAllClientsWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_ENABLE_LOGS_FLUSHING, &RDKWindowManager::enableLogsFlushingWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_LOGS_FLUSHING_ENABLED, &RDKWindowManager::getLogsFlushingEnabledWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_IGNORE_KEY_INPUTS, &RDKWindowManager::ignoreKeyInputsWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SHOW_CURSOR, &RDKWindowManager::showCursorWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_HIDE_CURSOR, &RDKWindowManager::hideCursorWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_CURSOR_SIZE, &RDKWindowManager::getCursorSizeWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SET_CURSOR_SIZE, &RDKWindowManager::setCursorSizeWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_ENABLE_INPUT_EVENTS, &RDKWindowManager::enableInputEventsWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_KEY_REPEAT_CONFIG, &RDKWindowManager::keyRepeatConfigWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_GET_GRAPHICS_FRAME_RATE, &RDKWindowManager::getGraphicsFrameRateWrapper, this);
            Register(RDK_WINDOW_MANAGER_METHOD_SET_GRAPHICS_FRAME_RATE, &RDKWindowManager::setGraphicsFrameRateWrapper, this);
        }

        RDKWindowManager::~RDKWindowManager()
        {
            //LOGINFO("dtor");
        }

        const string RDKWindowManager::Initialize(PluginHost::IShell* service )
        {
            std::cout << "initializing\n";
            char* waylandDisplay = getenv("WAYLAND_DISPLAY");
            if (NULL != waylandDisplay)
            {
                std::cout << "RDKWindowManager WAYLAND_DISPLAY is set to: " << waylandDisplay <<" unsetting WAYLAND_DISPLAY\n";
                unsetenv("WAYLAND_DISPLAY");
            }
            else
            {
                std::cout << "RDKWindowManager WAYLAND_DISPLAY is not set\n";
            }

            mCurrentService = service;
            CompositorController::setEventListener(mEventListener);

            mEnableUserInactivityNotification = true;
            enableInactivityReporting(true);

            static PluginHost::IShell* pluginService = nullptr;
            pluginService = service;

            shellThread = std::thread([=]() {
                bool isRunning = true;
                gRdkWindowManagerMutex.lock();
                RdkShell::initialize();
                PluginHost::ISubSystem* subSystems(pluginService->SubSystems());
                if (subSystems != nullptr)
                {
                    std::cout << "setting platform and graphics\n";
                    fflush(stdout);
                    subSystems->Set(PluginHost::ISubSystem::PLATFORM, nullptr);
                    subSystems->Set(PluginHost::ISubSystem::GRAPHICS, nullptr);
                    subSystems->Release();
                }
                isRunning = sRunning;
                gRdkWindowManagerMutex.unlock();
                while(isRunning) {
                  const double maxSleepTime = (1000 / gCurrentFramerate) * 1000;
                  double startFrameTime = RdkShell::microseconds();
                  gRdkWindowManagerMutex.lock();

                  while (gCreateDisplayRequests.size() > 0)
                  {
		              std::shared_ptr<CreateDisplayRequest> request = gCreateDisplayRequests.front();
                      if (!request)
                      {
                          gCreateDisplayRequests.erase(gCreateDisplayRequests.begin());
                          continue;
                      }
                      request->mResult = CompositorController::createDisplay(request->mClient, request->mDisplayName, request->mDisplayWidth, request->mDisplayHeight, request->mVirtualDisplayEnabled, request->mVirtualWidth, request->mVirtualHeight, request->mTopmost, request->mFocus , request->mAutoDestroy);
                      gCreateDisplayRequests.erase(gCreateDisplayRequests.begin());
                      sem_post(&request->mSemaphore);
                  }
                  while (gKillClientRequests.size() > 0)
                  {
	                  std::shared_ptr<KillClientRequest> request = gKillClientRequests.front();
                      if (!request)
                      {
                          gKillClientRequests.erase(gKillClientRequests.begin());
                          continue;
                      }
                      request->mResult = CompositorController::kill(request->mClient);
                      gKillClientRequests.erase(gKillClientRequests.begin());
                      sem_post(&request->mSemaphore);
                  }
                  if (receivedResolutionRequest)
                  {
                    CompositorController::setScreenResolution(resolutionWidth, resolutionHeight);
                    receivedResolutionRequest = false;
                  }
                  RdkShell::draw();
                  RdkShell::update();
                  isRunning = sRunning;
                  gRdkWindowManagerMutex.unlock();
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

        void RDKWindowManager::Deinitialize(PluginHost::IShell* service)
        {
            LOGINFO("Deinitialize");
            gRdkWindowManagerMutex.lock();
            RdkShell::deinitialize();
            sRunning = false;
            gRdkWindowManagerMutex.unlock();
            shellThread.join();
            std::vector<std::string> clientList;
            CompositorController::getClients(clientList);
            std::vector<std::string>::iterator ptr;
            for(ptr=clientList.begin();ptr!=clientList.end();ptr++)
            {
               RdkShell::CompositorController::removeListener((*ptr),mEventListener);
            }
            mCurrentService = nullptr;
            RDKWindowManager::_instance = nullptr;
            CompositorController::setEventListener(nullptr);
            mEventListener = nullptr;
            mEnableUserInactivityNotification = false;
            gRdkWindowManagerMutex.lock();
            for (unsigned int i=0; i<gCreateDisplayRequests.size(); i++)
            {
                sem_destroy(&gCreateDisplayRequests[i]->mSemaphore);
                gCreateDisplayRequests[i] = nullptr;
            }
            gCreateDisplayRequests.clear();
            for (unsigned int i=0; i<gKillClientRequests.size(); i++)
            {
                sem_destroy(&gKillClientRequests[i]->mSemaphore);
                gKillClientRequests[i] = nullptr;
            }
            gKillClientRequests.clear();
            gRdkWindowManagerMutex.unlock();
        }

        string RDKWindowManager::Information() const
        {
            return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
        }

        void RDKWindowManager::RdkShellListener::onUserInactive(const double minutes)
        {
          std::cout << "RDKWindowManager onUserInactive event received ..." << minutes << std::endl;
          JsonObject params;
          params["minutes"] = std::to_string(minutes);
          mShell.notify(RDK_WINDOW_MANAGER_EVENT_ON_USER_INACTIVITY, params);
        }

        // Registered methods (wrappers) begin
        uint32_t RDKWindowManager::moveToFrontWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::moveToBackWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::moveBehindWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::setFocusWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::killWrapper(const JsonObject& parameters, JsonObject& response)
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

                // Kill the display
                result = kill(client);
                if (!result)
                {
                    response["message"] = "failed to kill client";
                    returnResponse(false);
                }
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::addKeyInterceptWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::addKeyInterceptsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("intercepts"))
            {
                result = false;
                response["message"] = "please specify intercepts";
            }
            if (result)
            {
                const JsonArray intercepts = parameters["intercepts"].Array();
                result = addKeyIntercepts(intercepts);
                if (false == result)
                {
                    response["message"] = "failed to add some key intercepts due to missing parameters or wrong format ";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::removeKeyInterceptWrapper(const JsonObject& parameters, JsonObject& response)
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

                uint32_t keyCode = parameters["keyCode"].Number();
                // check for * parameter
                JsonValue keyCodeValue = parameters["keyCode"];
                if (keyCodeValue.Content() == JsonValue::type::STRING)
                {
                    std::string keyCodeStringValue = parameters["keyCode"].String();
                    if (keyCodeStringValue.compare("*") == 0)
                    {
                        keyCode = 255;
                    }
                }
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

        uint32_t RDKWindowManager::addKeyListenersWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::removeKeyListenersWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::addKeyMetadataListenerWrapper(const JsonObject& parameters, JsonObject& response)
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
                gRdkWindowManagerMutex.lock();
                result = CompositorController::addKeyMetadataListener(client);
                gRdkWindowManagerMutex.unlock();
                if (false == result) {
                  response["message"] = "failed to add key metadata listeners";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::removeKeyMetadataListenerWrapper(const JsonObject& parameters, JsonObject& response)
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
                lockRdkWindowManagerMutex();
                result = CompositorController::removeKeyMetadataListener(client);
                gRdkWindowManagerMutex.unlock();
                if (false == result) {
                  response["message"] = "failed to remove key metadata listeners";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::injectKeyWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::generateKeyWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::getScreenResolutionWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::setScreenResolutionWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::createDisplayWrapper(const JsonObject& parameters, JsonObject& response)
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
                uint32_t displayWidth = 0;
                if (parameters.HasLabel("displayWidth"))
                {
                    displayWidth = parameters["displayWidth"].Number();
                }
                uint32_t displayHeight = 0;
                if (parameters.HasLabel("displayHeight"))
                {
                    displayHeight = parameters["displayHeight"].Number();
                }

                bool virtualDisplay = false;
                if (parameters.HasLabel("virtualDisplay"))
                {
                    virtualDisplay = parameters["virtualDisplay"].Boolean();
                }

                uint32_t virtualWidth = 0;
                if (parameters.HasLabel("virtualWidth"))
                {
                    virtualWidth = parameters["virtualWidth"].Number();
                }
                uint32_t virtualHeight = 0;
                if (parameters.HasLabel("virtualHeight"))
                {
                    virtualHeight = parameters["virtualHeight"].Number();
                }

                bool topmost = false;
                if (parameters.HasLabel("topmost"))
                {
                    topmost = parameters["topmost"].Boolean();
                }
                bool focus = false;
                if (parameters.HasLabel("focus"))
                {
                    focus = parameters["focus"].Boolean();
                }

                result = createDisplay(client, displayName, displayWidth, displayHeight,
                    virtualDisplay, virtualWidth, virtualHeight, topmost, focus);
                if (false == result) {
                  response["message"] = "failed to create display";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::getClientsWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::getZOrderWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::getBoundsWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::setBoundsWrapper(const JsonObject& parameters, JsonObject& response)
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
                lockRdkWindowManagerMutex();
                CompositorController::getBounds(client, x, y, w, h);
                gRdkWindowManagerMutex.unlock();
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

        uint32_t RDKWindowManager::getVisibilityWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::setVisibilityWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::getOpacityWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::setOpacityWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::getScaleWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::setScaleWrapper(const JsonObject& parameters, JsonObject& response)
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
                bool decodeSuccess = true;
                try
                {
                    if (parameters.HasLabel("sx"))
                    {
                        scaleX = std::stod(parameters["sx"].String());
                    }
                    if (parameters.HasLabel("sy"))
                    {
                        scaleY = std::stod(parameters["sy"].String());
                    }
                }
                catch(...)
                {
                    decodeSuccess = false;
                    std::cout << "error decoding sx or sy " << std::endl;
                }
                if (false == decodeSuccess)
                {
                      response["message"] = "failed to set scale due to invalid parameters";
                      result = false;
                }
                else
                {
                    result = setScale(client, scaleX, scaleY);
                    if (false == result) {
                      response["message"] = "failed to set scale";
                    }
                }
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::getHolePunchWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::setHolePunchWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::getLogLevelWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            std::string logLevel = "INFO";
            lockRdkWindowManagerMutex();
            result = CompositorController::getLogLevel(logLevel);
            gRdkWindowManagerMutex.unlock();
            if (false == result) {
                response["message"] = "failed to get log level";
            }
            else {
                response["logLevel"] = logLevel;
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::setLogLevelWrapper(const JsonObject& parameters, JsonObject& response)
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
                lockRdkWindowManagerMutex();
                result = CompositorController::setLogLevel(logLevel);
                CompositorController::getLogLevel(currentLogLevel);
                gRdkWindowManagerMutex.unlock();
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

        uint32_t RDKWindowManager::enableInactivityReportingWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKWindowManager::setInactivityIntervalWrapper(const JsonObject& parameters, JsonObject& response)
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
                const unsigned int interval = parameters["interval"].Number();

                result = setInactivityInterval(interval);
                // Just realized: we need one more string& param for the the error message in case setScreenResolution() fails internally
                // Also, we might not need a "non-wrapper" method at all, nothing prevents us from implementing it right here

                if (false == result) {
                  response["message"] = "failed to set inactivity interval";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::resetInactivityTimeWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (false == mEnableUserInactivityNotification)
            {
                result = false;
                response["message"] = "feature is not enabled";
            }
            if (result)
            {
                result = resetInactivityTime();
                if (false == result) {
                  response["message"] = "failed to reset inactivity time";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::scaleToFitWrapper(const JsonObject& parameters, JsonObject& response)
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
                lockRdkWindowManagerMutex();
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
                gRdkWindowManagerMutex.unlock();

                if (!result) {
                  response["message"] = "failed to scale to fit";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::setTopmostWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client") && !parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify client or callsign";
            }
            else if (!parameters.HasLabel("topmost"))
            {
                result = false;
                response["message"] = "please specify topmost (topmost = true/false)";
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
                const bool topmost = parameters["topmost"].Boolean();
                bool focus = false;
                if (parameters.HasLabel("focus"))
                {
                    focus = parameters["focus"].Boolean();
                }
                result = setTopmost(client, topmost, focus);
                if (false == result)
                {
                    response["message"] = "failed to set topmost";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::getVirtualResolutionWrapper(const JsonObject& parameters, JsonObject& response)
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
                uint32_t width = 0;
                uint32_t height = 0;
                if (!getVirtualResolution(client, width, height))
                {
                    response["message"] = "failed to get virtual resolution";
                    result = false;
                }
                else
                {
                    response["width"] = width;
                    response["height"] = height;
                    result = true;
                }
            }

            returnResponse(result);
        }

        uint32_t RDKWindowManager::enableKeyRepeatsWrapper(const JsonObject& parameters, JsonObject& response)
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
              result = enableKeyRepeats(enable);
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::getKeyRepeatsEnabledWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = false, enable=false;
            result = getKeyRepeatsEnabled(enable);
            if (false == result) {
              response["message"] = "failed to get key repeats";
            }
            else {
              response["keyRepeat"] = enable;
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::setVirtualResolutionWrapper(const JsonObject& parameters, JsonObject& response)
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

                uint32_t width = 0;
                uint32_t height = 0;
                getVirtualResolution(client, width, height);

                if (parameters.HasLabel("width"))
                {
                    width = parameters["width"].Number();
                }
                if (parameters.HasLabel("height"))
                {
                    height = parameters["height"].Number();
                }

                result = setVirtualResolution(client, width, height);
            }

            returnResponse(result);
        }

        uint32_t RDKWindowManager::enableVirtualDisplayWrapper(const JsonObject& parameters, JsonObject& response)
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

                if (!parameters.HasLabel("enable"))
                {
                    response["message"] = "please specify enable parameter";
                    result = false;
                }
                else
                {
                    bool enable = parameters["enable"].Boolean();
                    result = enableVirtualDisplay(client, enable);
                }
            }

            returnResponse(result);
        }

        uint32_t RDKWindowManager::getVirtualDisplayEnabledWrapper(const JsonObject& parameters, JsonObject& response)
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

                bool enabled = false;
                if (!getVirtualDisplayEnabled(client, enabled))
                {
                    response["message"] = "failed to call getVirtualDisplayEnabled";
                    result = false;
                }
                else
                {
                    response["enabled"] = enabled;
                    result = true;
                }
            }

            returnResponse(result);
        }


	uint32_t RDKWindowManager::hideAllClientsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            if (!parameters.HasLabel("hide"))
            {
                response["message"] = "please specify hide parameter";
                returnResponse(false);
            }
            bool hide = parameters["hide"].Boolean();
            lockRdkWindowManagerMutex();
            std::vector<std::string> clientList;
            CompositorController::getClients(clientList);
            for (size_t i=0; i<clientList.size(); i++)
            {
                CompositorController::setVisibility(clientList[i], !hide);
            }
            gRdkWindowManagerMutex.unlock();
            returnResponse(true);
        }

        uint32_t RDKWindowManager::enableLogsFlushingWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("enable"))
            {
                response["message"] = "please specify enable parameter";
                result = false;
            }
            else
            {
                bool enable = parameters["enable"].Boolean();
                enableLogsFlushing(enable);
                result = true;
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::getLogsFlushingEnabledWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool enabled = false;
            getLogsFlushingEnabled(enabled);
            response["enabled"] = enabled;

            returnResponse(true);
        }

        uint32_t RDKWindowManager::showCursorWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = showCursor();
            returnResponse(result);
        }

        uint32_t RDKWindowManager::hideCursorWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = hideCursor();
            returnResponse(result);
        }

        uint32_t RDKWindowManager::setCursorSizeWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!(parameters.HasLabel("width") && parameters.HasLabel("height")))
            {
                response["message"] = "please specify width and height parameters";
                result = false;
            }
            else
            {
                result = setCursorSize(parameters["width"].Number(), parameters["height"].Number());
            }
            returnResponse(result);
        }

        uint32_t RDKWindowManager::getCursorSizeWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = false;
            uint32_t width = 0;
            uint32_t height = 0;

            if (!getCursorSize(width, height))
            {
                response["message"] = "failed to get cursor size";
                result = false;
            }
            else
            {
                response["width"] = width;
                response["height"] = height;
                result = true;
            }
            returnResponse(result);
        }
	
        uint32_t RDKWindowManager::ignoreKeyInputsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            if (!parameters.HasLabel("ignore"))
            {
                response["message"] = "please specify ignore parameter";
                returnResponse(false);
            }
            bool ignoreKeyValue = parameters["ignore"].Boolean();
            lockRdkWindowManagerMutex();
            bool ret = CompositorController::ignoreKeyInputs(ignoreKeyValue);
            gRdkWindowManagerMutex.unlock();
            if (!ret)
            {
                response["message"] = "key ignore is not allowed";
            }
            returnResponse(ret);
        }

        uint32_t RDKWindowManager::enableInputEventsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("clients"))
            {
                response["message"] = "please specify clients parameter";
                result = false;
            }
            else if (!parameters.HasLabel("enable"))
            {
                response["message"] = "please specify enable parameter";
                result = false;
            }
            else
            {
                result = enableInputEvents(parameters["clients"].Array(), parameters["enable"].Boolean());
            }

            returnResponse(result);
        }

        uint32_t RDKWindowManager::keyRepeatConfigWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            if (parameters.HasLabel("input"))
            {
                string input = parameters["input"].String();

                if (input != "default" || input != "keyboard")
                {
                    response["message"] = "not supported input type";
                    returnResponse(false);
                }
            }

            bool enabled = false;
            int32_t initialDelay = 0;
            int32_t repeatInterval = 0;

            if (parameters.HasLabel("enabled"))
            {
                enabled = parameters["enabled"].Boolean();
            }
            else
            {
                response["message"] = "please specify enabled parameter";
                returnResponse(false);
            }

            if (parameters.HasLabel("initialDelay"))
            {
                initialDelay = parameters["initialDelay"].Number();
            }
            else
            {
                response["message"] = "please specify initialDelay parameter";
                returnResponse(false);
            }

            if (parameters.HasLabel("repeatInterval"))
            {
                repeatInterval = parameters["repeatInterval"].Number();
            }
            else
            {
                response["message"] = "please specify repeatInterval parameter";
                returnResponse(false);
            }

            gRdkWindowManagerMutex.lock();
            CompositorController::setKeyRepeatConfig(enabled, initialDelay, repeatInterval);
            gRdkWindowManagerMutex.unlock();
            returnResponse(true);
        }

	uint32_t RDKWindowManager::getGraphicsFrameRateWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            lockRdkWindowManagerMutex();
            unsigned int value = gCurrentFramerate;
            gRdkWindowManagerMutex.unlock();
            response["framerate"] = value;
            returnResponse(true);
        }

        uint32_t RDKWindowManager::setGraphicsFrameRateWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("framerate"))
            {
                result = false;
                response["message"] = "please specify frame rate";
            }
            if (result)
            {
                unsigned int framerate = parameters["framerate"].Number();
                lockRdkWindowManagerMutex();
                gCurrentFramerate = framerate;
                gRdkWindowManagerMutex.unlock();
            }
            returnResponse(result);
        }

        // Registered methods end

        // Events begin
        void RDKWindowManager::notify(const std::string& event, const JsonObject& parameters)
        {
            sendNotify(event.c_str(), parameters);
        }
        // Events end

        // Internal methods begin

        bool RDKWindowManager::moveToFront(const string& client)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::moveToFront(client);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::moveToBack(const string& client)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::moveToBack(client);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::moveBehind(const string& client, const string& target)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
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
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::setFocus(const string& client)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::setFocus(client);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::kill(const string& client)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            RdkShell::CompositorController::removeListener(client, mEventListener);
            std::shared_ptr<KillClientRequest> request = std::make_shared<KillClientRequest>(client);
            gKillClientRequests.push_back(request);
            std::cout << "removed displayname : "<<client<< std::endl;
            gRdkWindowManagerMutex.unlock();
            sem_wait(&request->mSemaphore);
            ret = request->mResult;
            return ret;
        }

        bool RDKWindowManager::addKeyIntercepts(const JsonArray& intercepts)
        {
            bool ret = true;
            for (int i=0; i<intercepts.Length(); i++)
            {
                if (!(intercepts[i].Content() == JsonValue::type::OBJECT))
                {
                    std::cout << "ignoring entry " << i+1 << "due to wrong format " << std::endl;
                    ret = false;
                    continue;
                }
                const JsonObject& interceptEntry = intercepts[i].Object();
                if (!interceptEntry.HasLabel("keys") || !interceptEntry.HasLabel("client"))
                {
                    std::cout << "ignoring entry " << i+1 << "due to missing client or keys parameter " << std::endl;
                    ret = false;
                    continue;
                }
                const JsonArray& keys = interceptEntry["keys"].Array();
                std::string client = interceptEntry["client"].String();
                for (int k=0; k<keys.Length(); k++)
                {
                    if (!(keys[k].Content() == JsonValue::type::OBJECT))
                    {
                        std::cout << "ignoring key << " << k+1 << " in entry " << i+1 << "due to wrong format " << std::endl;
                        ret = false;
                        continue;
                    }
                    const JsonObject& keyEntry = keys[k].Object();
                    if (!keyEntry.HasLabel("keyCode"))
                    {
                        std::cout << "ignoring key << " << k+1 << " in entry " << i+1 << "due to missing key code parameter " << std::endl;
                        ret = false;
                        continue;
                    }
                    const JsonArray modifiers = keyEntry.HasLabel("modifiers") ? keyEntry["modifiers"].Array() : JsonArray();
                    const uint32_t keyCode = keyEntry["keyCode"].Number();
                    ret = addKeyIntercept(keyCode, modifiers, client);
                }
            }
            return ret;
        }

        bool RDKWindowManager::addKeyIntercept(const uint32_t& keyCode, const JsonArray& modifiers, const string& client)
        {
            uint32_t flags = 0;
            for (int i=0; i<modifiers.Length(); i++) {
              flags |= getKeyFlag(modifiers[i].String());
            }
            bool ret = false;
            gRdkWindowManagerMutex.lock();
            ret = CompositorController::addKeyIntercept(client, keyCode, flags);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::removeKeyIntercept(const uint32_t& keyCode, const JsonArray& modifiers, const string& client)
        {
            uint32_t flags = 0;
            for (int i=0; i<modifiers.Length(); i++) {
              flags |= getKeyFlag(modifiers[i].String());
            }
            bool ret = false;
            gRdkWindowManagerMutex.lock();
            ret = CompositorController::removeKeyIntercept(client, keyCode, flags);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::addKeyListeners(const string& client, const JsonArray& keys)
        {
            gRdkWindowManagerMutex.lock();

            bool result = true;

            for (int i=0; i<keys.Length(); i++) {

                result = false;
                const JsonObject& keyInfo = keys[i].Object();

                if (keyInfo.HasLabel("keyCode") && keyInfo.HasLabel("nativeKeyCode"))
                {
                    std::cout << "ERROR: keyCode and nativeKeyCode can't be set both at the same time" << std::endl;
                }
                else if (keyInfo.HasLabel("keyCode") || keyInfo.HasLabel("nativeKeyCode"))
                {
                    uint32_t keyCode = 0;

                    if (keyInfo.HasLabel("keyCode"))
                    {
                        std::string keystring = keyInfo["keyCode"].String();
                        if (keystring.compare("*") == 0)
                        {
                          keyCode = ANY_KEY;
                        }
                        else
                        {
                          keyCode = keyInfo["keyCode"].Number();
                        }
                    }
                    else
                    {
                        std::string keystring = keyInfo["nativeKeyCode"].String();
                        if (keystring.compare("*") == 0)
                        {
                            keyCode = ANY_KEY;
                        }
                        else
                        {
                            keyCode = keyInfo["nativeKeyCode"].Number();
                        }
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

                    if (keyInfo.HasLabel("keyCode"))
                    {
                        result = CompositorController::addKeyListener(client, keyCode, flags, properties);
                    }
                    else
                    {
                        result = CompositorController::addNativeKeyListener(client, keyCode, flags, properties);
                    }
                }
                else
                {
                    std::cout << "ERROR: Neither keyCode nor nativeKeyCode provided" << std::endl;
                }

                if (result == false)
                {
                    break;
                }
            }
            gRdkWindowManagerMutex.unlock();
            return result;
        }

        bool RDKWindowManager::removeKeyListeners(const string& client, const JsonArray& keys)
        {
            gRdkWindowManagerMutex.lock();

            bool result = true;

            for (int i=0; i<keys.Length(); i++) {

                result = false;
                const JsonObject& keyInfo = keys[i].Object();

                if (keyInfo.HasLabel("keyCode") && keyInfo.HasLabel("nativeKeyCode"))
                {
                    std::cout << "ERROR: keyCode and nativeKeyCode can't be set both at the same time" << std::endl;
                }
                else if (keyInfo.HasLabel("keyCode") || keyInfo.HasLabel("nativeKeyCode"))
                {
                    uint32_t keyCode = 0;
                    if (keyInfo.HasLabel("keyCode"))
                    {
                        std::string keystring = keyInfo["keyCode"].String();
                        if (keystring.compare("*") == 0)
                        {
                          keyCode = ANY_KEY;
                        }
                        else
                        {
                          keyCode = keyInfo["keyCode"].Number();
                        }
                    }
                    else
                    {
                        std::string keystring = keyInfo["nativeKeyCode"].String();
                        if (keystring.compare("*") == 0)
                        {
                          keyCode = ANY_KEY;
                        }
                        else
                        {
                          keyCode = keyInfo["nativeKeyCode"].Number();
                        }
                    }

                    const JsonArray modifiers = keyInfo.HasLabel("modifiers") ? keyInfo["modifiers"].Array() : JsonArray();
                    uint32_t flags = 0;
                    for (int i=0; i<modifiers.Length(); i++) {
                      flags |= getKeyFlag(modifiers[i].String());
                    }

                    if (keyInfo.HasLabel("keyCode"))
                    {
                        result = CompositorController::removeKeyListener(client, keyCode, flags);
                    }
                    else
                    {
                        result = CompositorController::removeNativeKeyListener(client, keyCode, flags);
                    }
                }
                else
                {
                    std::cout << "ERROR: Neither keyCode nor nativeKeyCode provided" << std::endl;
                }

                if (result == false)
                {
                    break;
                }
            }
            gRdkWindowManagerMutex.unlock();
            return result;
        }

        bool RDKWindowManager::injectKey(const uint32_t& keyCode, const JsonArray& modifiers)
        {
            bool ret = false;
            uint32_t flags = 0;
            for (int i=0; i<modifiers.Length(); i++) {
              flags |= getKeyFlag(modifiers[i].String());
            }
            gRdkWindowManagerMutex.lock();
            ret = CompositorController::injectKey(keyCode, flags);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::generateKey(const string& client, const JsonArray& keyInputs)
        {
            bool ret = false;
            for (int i=0; i<keyInputs.Length(); i++) {
                const JsonObject& keyInputInfo = keyInputs[i].Object();
                uint32_t keyCode, flags=0;
                std::string virtualKey("");
                if (keyInputInfo.HasLabel("key"))
                {
                  virtualKey = keyInputInfo["key"].String();
                }
                else if (keyInputInfo.HasLabel("keyCode"))
                {
                  keyCode = keyInputInfo["keyCode"].Number();
                  const JsonArray modifiers = keyInputInfo.HasLabel("modifiers") ? keyInputInfo["modifiers"].Array() : JsonArray();
                  for (int k=0; k<modifiers.Length(); k++) {
                    flags |= getKeyFlag(modifiers[k].String());
                  }
                }
                else
                {
                  continue;
                }
                const uint32_t delay = keyInputInfo["delay"].Number();
                sleep(delay);
                std::string keyClient = keyInputInfo.HasLabel("client")? keyInputInfo["client"].String(): client;
                if (keyClient.empty())
                {
                  keyClient = keyInputInfo.HasLabel("callsign")? keyInputInfo["callsign"].String(): "";
                }
                lockRdkWindowManagerMutex();
		bool targetFound = false;
                if (keyClient != "")
                {
                  std::vector<std::string> clientList;
                  CompositorController::getClients(clientList);
                  transform(keyClient.begin(), keyClient.end(), keyClient.begin(), ::tolower);
                  if (std::find(clientList.begin(), clientList.end(), keyClient) != clientList.end())
                  {
                    targetFound = true;
                  }
                }
                if (targetFound || keyClient == "")
                {
                  ret = CompositorController::generateKey(keyClient, keyCode, flags, virtualKey);
                }
                gRdkWindowManagerMutex.unlock();
            }
            return ret;
        }

        bool RDKWindowManager::getScreenResolution(JsonObject& out)
        {
            unsigned int width=0,height=0;
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::getScreenResolution(width, height);
            gRdkWindowManagerMutex.unlock();
            if (true == ret) {
              out["w"] = width;
              out["h"] = height;
              return true;
            }
            return false;
        }

        bool RDKWindowManager::setScreenResolution(const unsigned int w, const unsigned int h)
        {
            lockRdkWindowManagerMutex();
            receivedResolutionRequest = true;
            resolutionWidth = w;
            resolutionHeight = h;
            gRdkWindowManagerMutex.unlock();
            return true;
        }

        bool RDKWindowManager::createDisplay(const string& client, const string& displayName, const uint32_t displayWidth, const uint32_t displayHeight,
            const bool virtualDisplay, const uint32_t virtualWidth, const uint32_t virtualHeight, const bool topmost, const bool focus)
        {
            bool ret = false;
            if (!isClientExists(client))
            {
                lockRdkWindowManagerMutex();
                std::shared_ptr<CreateDisplayRequest> request = std::make_shared<CreateDisplayRequest>(client, displayName, displayWidth, displayHeight, virtualDisplay, virtualWidth, virtualHeight);
                gCreateDisplayRequests.push_back(request);
                gRdkWindowManagerMutex.unlock();
                sem_wait(&request->mSemaphore);
                ret = request->mResult;
            }
            else
            {
                std::cout << "Client " << client  << "already exist " << std::endl;
            }
            lockRdkWindowManagerMutex();
            RdkShell::CompositorController::addListener(client, mEventListener);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::getClients(JsonArray& clients)
        {
            std::vector<std::string> clientList;
            lockRdkWindowManagerMutex();
            CompositorController::getClients(clientList);
            gRdkWindowManagerMutex.unlock();
            for (size_t i=0; i<clientList.size(); i++) {
              clients.Add(clientList[i]);
            }
            return true;
        }

        bool RDKWindowManager::getZOrder(JsonArray& clients)
        {
            std::vector<std::string> zOrderList;
            lockRdkWindowManagerMutex();
            CompositorController::getZOrder(zOrderList);
            gRdkWindowManagerMutex.unlock();
            for (size_t i=0; i<zOrderList.size(); i++) {
              clients.Add(zOrderList[i]);
            }
            return true;
        }

        bool RDKWindowManager::getBounds(const string& client, JsonObject& bounds)
        {
            unsigned int x=0,y=0,width=0,height=0;
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::getBounds(client, x, y, width, height);
            gRdkWindowManagerMutex.unlock();
            if (true == ret) {
              bounds["x"] = x;
              bounds["y"] = y;
              bounds["w"] = width;
              bounds["h"] = height;
              return true;
            }
            return false;
        }

        bool RDKWindowManager::setBounds(const std::string& client, const unsigned int x, const unsigned int y, const unsigned int w, const unsigned int h)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            std::cout << "setting the bounds\n";
            ret = CompositorController::setBounds(client, 0, 0, 1, 1); //forcing a compositor resize flush
            ret = CompositorController::setBounds(client, x, y, w, h);
            gRdkWindowManagerMutex.unlock();
            std::cout << "bounds set\n";
            usleep(68000);
            std::cout << "all set\n";
            return ret;
        }

        bool RDKWindowManager::getVisibility(const string& client, bool& visible)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::getVisibility(client, visible);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::setVisibility(const string& client, const bool visible)
        {
            bool ret = false;
            {
                bool lockAcquired = false;
                double startTime = RdkShell::milliseconds();
                while (!lockAcquired && (RdkShell::milliseconds() - startTime) < TRY_LOCK_WAIT_TIME_IN_MS)
                {
                    lockAcquired = gRdkWindowManagerMutex.try_lock();
                }
                if (!lockAcquired)
                {
                    std::cout << "unable to get lock for visibility, defaulting to normal lock\n";
                    gRdkWindowManagerMutex.lock();
                }
                else
                {
                    std::cout << "lock was acquired via try for visibility\n";
                }
            }
            ret = CompositorController::setVisibility(client, visible);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::getOpacity(const string& client, unsigned int& opacity)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::getOpacity(client, opacity);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::setOpacity(const string& client, const unsigned int opacity)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            std::vector<std::string> clientList;
            CompositorController::getClients(clientList);
            bool targetFound = false;
            std::string newClient(client);
            std::transform(newClient.begin(), newClient.end(), newClient.begin(), ::tolower);
            if (std::find(clientList.begin(), clientList.end(), newClient) != clientList.end())
            {
              targetFound = true;
            }
            if (targetFound)
            {
              ret = CompositorController::setOpacity(newClient, opacity);
            }
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::getScale(const string& client, double& scaleX, double& scaleY)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::getScale(client, scaleX, scaleY);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::setScale(const string& client, const double scaleX, const double scaleY)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            std::vector<std::string> clientList;
            CompositorController::getClients(clientList);
            std::string newClient(client);
            bool targetFound = false;
            transform(newClient.begin(), newClient.end(), newClient.begin(), ::tolower);
            if (std::find(clientList.begin(), clientList.end(), newClient) != clientList.end())
            {
              targetFound = true;
            }
            if (targetFound)
            {
              ret = CompositorController::setScale(newClient, scaleX, scaleY);
            }
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::getHolePunch(const string& client, bool& holePunch)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::getHolePunch(client, holePunch);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::setHolePunch(const string& client, const bool holePunch)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::setHolePunch(client, holePunch);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::enableInactivityReporting(const bool enable)
        {
            lockRdkWindowManagerMutex();
            CompositorController::enableInactivityReporting(enable);
            gRdkWindowManagerMutex.unlock();
            return true;
        }

        bool RDKWindowManager::setInactivityInterval(const uint32_t interval)
        {
            lockRdkWindowManagerMutex();
            try
            {
              CompositorController::setInactivityInterval((double)interval);
            }
            catch (...) 
            {
              std::cout << "RDKWindowManager unable to set inactivity interval  " << std::endl;
            }
            gRdkWindowManagerMutex.unlock();
            return true;
        }

        bool RDKWindowManager::resetInactivityTime()
        {
            lockRdkWindowManagerMutex();
            try
            {
              CompositorController::resetInactivityTime();
              std::cout << "RDKWindowManager inactivity time reset" << std::endl;
            }
            catch (...)
            {
              std::cout << "RDKWindowManager unable to reset inactivity time  " << std::endl;
            }
            gRdkWindowManagerMutex.unlock();
            return true;
        }

        bool RDKWindowManager::getKeyRepeatsEnabled(bool& enable)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::getKeyRepeatsEnabled(enable);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::enableKeyRepeats(const bool enable)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::enableKeyRepeats(enable);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::setTopmost(const string& callsign, const bool topmost, const bool focus)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::setTopmost(callsign, topmost, focus);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::getVirtualResolution(const std::string& client, uint32_t &virtualWidth, uint32_t &virtualHeight)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::getVirtualResolution(client, virtualWidth, virtualHeight);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::setVirtualResolution(const std::string& client, const uint32_t virtualWidth, const uint32_t virtualHeight)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::setVirtualResolution(client, virtualWidth, virtualHeight);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::enableVirtualDisplay(const std::string& client, const bool enable)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::enableVirtualDisplay(client, enable);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::getVirtualDisplayEnabled(const std::string& client, bool &enabled)
        {
            bool ret = false;
            lockRdkWindowManagerMutex();
            ret = CompositorController::getVirtualDisplayEnabled(client, enabled);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }
        
        void RDKWindowManager::enableLogsFlushing(const bool enable)
        {
            gRdkWindowManagerMutex.lock();
            Logger::enableFlushing(enable);
            gRdkWindowManagerMutex.unlock();
        }

        void RDKWindowManager::getLogsFlushingEnabled(bool &enabled)
        {
            gRdkWindowManagerMutex.lock();
            enabled = Logger::isFlushingEnabled();
            gRdkWindowManagerMutex.unlock();
        }

        bool RDKWindowManager::showCursor()
        {
            gRdkWindowManagerMutex.lock();
            bool ret = CompositorController::showCursor();
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::hideCursor()
        {
            gRdkWindowManagerMutex.lock();
            bool ret = CompositorController::hideCursor();
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::setCursorSize(uint32_t width, uint32_t height)
        {
            gRdkWindowManagerMutex.lock();
            bool ret = CompositorController::setCursorSize(width, height);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::getCursorSize(uint32_t& width, uint32_t& height)
        {
            gRdkWindowManagerMutex.lock();
            bool ret = CompositorController::getCursorSize(width, height);
            gRdkWindowManagerMutex.unlock();
            return ret;
        }

        bool RDKWindowManager::enableInputEvents(const JsonArray& clients, bool enable)
        {
            bool result = true;

            gRdkWindowManagerMutex.lock();
            for (int i = 0; i < clients.Length(); i++)
            {
                const string& clientName = clients[i].String();
                if (clientName == "*")
                {
                    std::vector<std::string> clientList;
                   CompositorController::getClients(clientList);
                    for (size_t i = 0; i < clientList.size(); i++)
                    {
                        result = result && CompositorController::enableInputEvents(clientList[i], enable);
                    }

                    break;
                }
                else
                {
                    result = result && CompositorController::enableInputEvents(clientName, enable);
                }
            }
            gRdkWindowManagerMutex.unlock();

            return result;
        }
        // Internal methods end
    } // namespace Plugin
} // namespace WPEFramework
