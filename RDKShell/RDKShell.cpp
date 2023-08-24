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
#include <memory>
#include <iostream>
#include <mutex>
#include <thread>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <rdkshell/compositorcontroller.h>
#include <rdkshell/application.h>
#include <rdkshell/logger.h>
#include <interfaces/IFocus.h>
#include <interfaces/IMemory.h>
#include <interfaces/IBrowser.h>
#include <interfaces/IStore.h>
#include <interfaces/IStoreCache.h>
#include <rdkshell/logger.h>
#include <plugins/System.h>
#include <rdkshell/eastereggs.h>
#include <rdkshell/linuxkeys.h>

#include "UtilsJsonRpc.h"
#include "UtilsLOG_MILESTONE.h"
#include "UtilsUnused.h"
#include "UtilsgetRFCConfig.h"
#include "UtilsString.h"

#ifdef RDKSHELL_READ_MAC_ON_STARTUP
#include "FactoryProtectHal.h"
#endif //RDKSHELL_READ_MAC_ON_STARTUP


#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 4
#define API_VERSION_NUMBER_PATCH 2

const string WPEFramework::Plugin::RDKShell::SERVICE_NAME = "org.rdk.RDKShell";
//methods
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_MOVE_TO_FRONT = "moveToFront";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_MOVE_TO_BACK = "moveToBack";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_MOVE_BEHIND = "moveBehind";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_FOCUS = "setFocus";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_KILL = "kill";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ADD_KEY_INTERCEPT = "addKeyIntercept";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ADD_KEY_INTERCEPTS = "addKeyIntercepts";
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
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SHOW_SPLASH_LOGO = "showSplashLogo";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_HIDE_SPLASH_LOGO = "hideSplashLogo";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_REMOVE_ANIMATION = "removeAnimation";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ADD_ANIMATION = "addAnimation";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ENABLE_INACTIVITY_REPORTING = "enableInactivityReporting";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_INACTIVITY_INTERVAL = "setInactivityInterval";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_RESET_INACTIVITY_TIME = "resetInactivityTime";
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
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SHOW_WATERMARK = "showWatermark";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SHOW_FULL_SCREEN_IMAGE = "showFullScreenImage";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_HIDE_FULL_SCREEN_IMAGE = "hideFullScreenImage";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_LAUNCH_FACTORY_APP = "launchFactoryApp";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_LAUNCH_FACTORY_APP_SHORTCUT = "launchFactoryAppShortcut";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_LAUNCH_RESIDENT_APP = "launchResidentApp";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_TOGGLE_FACTORY_APP = "toggleFactoryApp";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_EXIT_AGING_MODE = "exitAgingMode";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ENABLE_KEYREPEATS = "enableKeyRepeats";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_KEYREPEATS_ENABLED = "getKeyRepeatsEnabled";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_TOPMOST = "setTopmost";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_VIRTUAL_RESOLUTION = "getVirtualResolution";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_VIRTUAL_RESOLUTION = "setVirtualResolution";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ENABLE_VIRTUAL_DISPLAY = "enableVirtualDisplay";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_VIRTUAL_DISPLAY_ENABLED = "getVirtualDisplayEnabled";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_LAST_WAKEUP_KEY = "getLastWakeupKey";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_HIDE_ALL_CLIENTS = "hideAllClients";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_IGNORE_KEY_INPUTS = "ignoreKeyInputs";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_SCREENSHOT = "getScreenshot";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ENABLE_EASTER_EGGS = "enableEasterEggs";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ENABLE_LOGS_FLUSHING = "enableLogsFlushing";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_LOGS_FLUSHING_ENABLED = "getLogsFlushingEnabled";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ADD_EASTER_EGGS = "addEasterEggs";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_REMOVE_EASTER_EGGS = "removeEasterEggs";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_EASTER_EGGS = "getEasterEggs";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SHOW_CURSOR = "showCursor";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_HIDE_CURSOR = "hideCursor";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_CURSOR_SIZE = "getCursorSize";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_CURSOR_SIZE = "setCursorSize";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ENABLE_INPUT_EVENTS = "enableInputEvents";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_AV_BLOCKED = "setAVBlocked";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_AV_BLOCKED_APPS = "getBlockedAVApplications";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_KEY_REPEAT_CONFIG = "keyRepeatConfig";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_GRAPHICS_FRAME_RATE = "getGraphicsFrameRate";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_GRAPHICS_FRAME_RATE = "setGraphicsFrameRate";
#ifdef HIBERNATE_SUPPORT_ENABLED
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_CHECKPOINT = "checkpoint";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_RESTORE = "restore";
#endif

const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_USER_INACTIVITY = "onUserInactivity";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_LAUNCHED = "onApplicationLaunched";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_CONNECTED = "onApplicationConnected";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_DISCONNECTED = "onApplicationDisconnected";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_TERMINATED = "onApplicationTerminated";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_FIRST_FRAME = "onApplicationFirstFrame";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_SUSPENDED = "onApplicationSuspended";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_RESUMED = "onApplicationResumed";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_APP_ACTIVATED = "onApplicationActivated";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_LAUNCHED = "onLaunched";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_SUSPENDED = "onSuspended";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_DESTROYED = "onDestroyed";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_PLUGIN_SUSPENDED = "onPluginSuspended";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_DEVICE_LOW_RAM_WARNING = "onDeviceLowRamWarning";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_DEVICE_CRITICALLY_LOW_RAM_WARNING = "onDeviceCriticallyLowRamWarning";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_DEVICE_LOW_RAM_WARNING_CLEARED = "onDeviceLowRamWarningCleared";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_DEVICE_CRITICALLY_LOW_RAM_WARNING_CLEARED = "onDeviceCriticallyLowRamWarningCleared";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_EASTER_EGG = "onEasterEgg";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_WILL_DESTROY = "onWillDestroy";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_SCREENSHOT_COMPLETE = "onScreenshotComplete";
#ifdef HIBERNATE_SUPPORT_ENABLED
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_CHECKPOINTED = "onCheckpointed";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_EVENT_ON_RESTORED = "onRestored";
#endif

using namespace std;
using namespace RdkShell;
using namespace Utils;
extern int gCurrentFramerate;
bool receivedResolutionRequest = false;
bool receivedFullScreenImageRequest= false;
std::string fullScreenImagePath;
bool receivedShowWatermarkRequest = false;
bool receivedShowSplashScreenRequest = false;
unsigned int gSplashScreenDisplayTime = 0;
unsigned int resolutionWidth = 1280;
unsigned int resolutionHeight = 720;
bool gRdkShellSurfaceModeEnabled = false;
static std::string sThunderSecurityToken;
std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> gSystemServiceConnection;
bool gSystemServiceEventsSubscribed = false;
static bool sResidentAppFirstActivated = false;
bool sPersistentStoreWaitProcessed = false;
bool sPersistentStoreFirstActivated = false;
bool sPersistentStorePreLaunchChecked=false;
bool sFactoryModeStart = false;
bool sFactoryModeBlockResidentApp = false;
bool sForceResidentAppLaunch = false;
static bool sRunning = true;
bool needsScreenshot = false;

#ifdef HIBERNATE_NATIVE_APPS_ON_SUSPENDED
std::mutex nativeAppWasResumedMutex;
map<string,bool> nativeAppWasResumed;
#endif

#define ANY_KEY 65536
#define RDKSHELL_THUNDER_TIMEOUT 20000
#define RDKSHELL_POWER_TIME_WAIT 2.5
#define THUNDER_ACCESS_DEFAULT_VALUE "127.0.0.1:9998"
#define RDKSHELL_WILLDESTROY_EVENT_WAITTIME 1
#define RDKSHELL_TRY_LOCK_WAIT_TIME_IN_MS 250

static std::string gThunderAccessValue = THUNDER_ACCESS_DEFAULT_VALUE;
static uint32_t gWillDestroyEventWaitTime = RDKSHELL_WILLDESTROY_EVENT_WAITTIME;
#define SYSTEM_SERVICE_CALLSIGN "org.rdk.System"
#define RESIDENTAPP_CALLSIGN "ResidentApp"
#define PERSISTENT_STORE_CALLSIGN "org.rdk.PersistentStore"
#define LISA_CALLSIGN "LISA"

#define RECONNECTION_TIME_IN_MILLISECONDS 5000

#define REMOTECONTROL_CALLSIGN "org.rdk.RemoteControl.1"
#define KEYCODE_INVALID -1
#define RETRY_INTERVAL_250MS 250000

#define RDKSHELL_SURFACECLIENT_DISPLAYNAME "rdkshell_display"
enum FactoryAppLaunchStatus
{
    NOTLAUNCHED = 0,
    STARTED,
    COMPLETED
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

FactoryAppLaunchStatus sFactoryAppLaunchStatus = NOTLAUNCHED;

namespace WPEFramework {

    namespace {

        static Plugin::Metadata<Plugin::RDKShell> metadata(
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

        namespace {
            // rdk Shell should use inter faces
#ifndef USE_THUNDER_R4
            class Job : public Core::IDispatchType<void> {
#else
            class Job : public Core::IDispatch {
#endif /* USE_THUNDER_R4 */
            public:
                Job(std::function<void()> work)
                    : _work(work)
                {
                }
                void Dispatch() override
                {
                    _work();
                }

            private:
                std::function<void()> _work;
            };
            uint32_t cloneService(PluginHost::IShell* shell, const string& basecallsign, const string& newcallsign)
            {
                uint32_t result = Core::ERROR_ASYNC_FAILED;
                Core::Event event(false, true);
#ifndef USE_THUNDER_R4
                Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatchType<void>>(Core::ProxyType<Job>::Create([&]() {
#else
                Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create([&]() {
#endif /* USE_THUNDER_R4 */
                    auto interface = shell->QueryInterfaceByCallsign<PluginHost::IController>("");
                    if (interface == nullptr) {
                        result = Core::ERROR_UNAVAILABLE;
                        std::cout << "no IController" << std::endl;
                    } else {
                        result = interface->Clone(basecallsign, newcallsign);
                        std::cout << "IController clone status " << result << std::endl;
                        interface->Release();
                    }
                    event.SetEvent();
                })));
                event.Lock();
                return result;
            }
            uint32_t getValue(PluginHost::IShell* shell, const string& ns, const string& key, string& value)
            {
                uint32_t result;
                auto interface = shell->QueryInterfaceByCallsign<Exchange::IStore>(PERSISTENT_STORE_CALLSIGN);
                if (interface == nullptr) {
                    result = Core::ERROR_UNAVAILABLE;
                    std::cout << "no IStore" << std::endl;
                } else {
                    result = interface->GetValue(ns, key, value);
                    std::cout << "IStore status " << result << " for get " << key << std::endl;
                    interface->Release();
                }
                return result;
            }
            uint32_t setValue(PluginHost::IShell* shell, const string& ns, const string& key, const string& value)
            {
                uint32_t result;
                auto interface = shell->QueryInterfaceByCallsign<Exchange::IStore>(PERSISTENT_STORE_CALLSIGN);
		auto storeInterface = shell->QueryInterfaceByCallsign<Exchange::IStoreCache>(PERSISTENT_STORE_CALLSIGN);
                if (interface == nullptr || storeInterface == nullptr) {
                    result = Core::ERROR_UNAVAILABLE;
                    std::cout << "no IStore" << std::endl;
                } else {
                    result = interface->SetValue(ns, key, value);
                    std::cout << "IStore status " << result << " for set " << key << std::endl;
		    result = storeInterface->FlushCache();
		    std::cout << "flushCache status " << result << std::endl;
                    interface->Release();
		    storeInterface->Release();
                }
                return result;
            }
            uint32_t getConfig(PluginHost::IShell* shell, const string& callsign, string& config)
            {
                uint32_t result;
                auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                if (interface == nullptr) {
                    result = Core::ERROR_UNAVAILABLE;
                    std::cout << "no IShell for " << callsign << std::endl;
                } else {
                    result = Core::ERROR_NONE;
                    config = interface->ConfigLine();
                    std::cout << "IShell config " << config << " for " << callsign << std::endl;
                    interface->Release();
                }
                return result;
            }
            uint32_t setConfig(PluginHost::IShell* shell, const string& callsign, const string& config)
            {
                uint32_t result;
                auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                if (interface == nullptr) {
                    result = Core::ERROR_UNAVAILABLE;
                    std::cout << "no IShell for " << callsign << std::endl;
                } else {
                    result = interface->ConfigLine(config);
                    std::cout << "IShell config status " << result << " for " << callsign << std::endl;
                    interface->Release();
                }
                return result;
            }
            uint32_t getServiceState(PluginHost::IShell* shell, const string& callsign, PluginHost::IShell::state& state)
            {
                uint32_t result;
                auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                if (interface == nullptr) {
                    result = Core::ERROR_UNAVAILABLE;
                    std::cout << "no IShell for " << callsign << std::endl;
                } else {
                    result = Core::ERROR_NONE;
                    state = interface->State();
                    std::cout << "IShell state " << state << " for " << callsign << std::endl;
                    interface->Release();
                }
                return result;
            }
            uint32_t activate(PluginHost::IShell* shell, const string& callsign)
            {
                uint32_t result = Core::ERROR_ASYNC_FAILED;
                Core::Event event(false, true);
#ifndef USE_THUNDER_R4
                Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatchType<void>>(Core::ProxyType<Job>::Create([&]() {
#else
                Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create([&]() {
#endif /* USE_THUNDER_R4 */
                    auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                    if (interface == nullptr) {
                        result = Core::ERROR_UNAVAILABLE;
                        std::cout << "no IShell for " << callsign << std::endl;
                    } else {
                        result = interface->Activate(PluginHost::IShell::reason::REQUESTED);
                        std::cout << "IShell activate status " << result << " for " << callsign << std::endl;
                        interface->Release();
                    }
                    event.SetEvent();
                })));
                event.Lock();
                return result;
            }
            uint32_t deactivate(PluginHost::IShell* shell, const string& callsign)
            {
                uint32_t result = Core::ERROR_ASYNC_FAILED;
                Core::Event event(false, true);
#ifndef USE_THUNDER_R4
                Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatchType<void>>(Core::ProxyType<Job>::Create([&]() {
#else
                Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create([&]() {
#endif /* USE_THUNDER_R4 */
                    auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                    if (interface == nullptr) {
                        result = Core::ERROR_UNAVAILABLE;
                        std::cout << "no IShell for " << callsign << std::endl;
                    } else {
                        result = interface->Deactivate(PluginHost::IShell::reason::REQUESTED);
                        std::cout << "IShell deactivate status " << result << " for " << callsign << std::endl;
                        interface->Release();
                    }
                    event.SetEvent();
                })));
                event.Lock();
                return result;
            }
        }

        struct JSONRPCDirectLink
        {
        private:
          uint32_t mId { 0 };
          std::string mCallSign { };
          PluginHost::IDispatcher * dispatcher_ {nullptr};

          Core::ProxyType<Core::JSONRPC::Message> Message() const
          {
            return (Core::ProxyType<Core::JSONRPC::Message>(PluginHost::IFactories::Instance().JSONRPC()));
          }

          template <typename PARAMETERS>
          bool ToMessage(PARAMETERS& parameters, Core::ProxyType<Core::JSONRPC::Message>& message) const
          {
            return ToMessage((Core::JSON::IElement*)(&parameters), message);
          }
          bool ToMessage(Core::JSON::IElement* parameters, Core::ProxyType<Core::JSONRPC::Message>& message) const
          {
            if (!parameters->IsSet())
              return true;
            string values;
            if (!parameters->ToString(values))
            {
              std::cout << "Failed to convert params to string\n";
              return false;
            }
            if (values.empty() != true)
            {
              message->Parameters = values;
            }
            return true;
          }
          template <typename RESPONSE>
          bool FromMessage(RESPONSE& response, const Core::ProxyType<Core::JSONRPC::Message>& message, bool isResponseString=false) const
          {
            return FromMessage((Core::JSON::IElement*)(&response), message, isResponseString);
          }
          bool FromMessage(Core::JSON::IElement* response, const Core::ProxyType<Core::JSONRPC::Message>& message, bool isResponseString=false) const
          {
            Core::OptionalType<Core::JSON::Error> error;
            if ( !isResponseString && !response->FromString(message->Result.Value(), error) )
            {
              std::cout << "Failed to parse response!!! Error: '" <<  error.Value().Message() << "'\n";
              return false;
            }
            return true;
          }

        public:
          JSONRPCDirectLink(PluginHost::IShell* service, std::string callsign)
            : mCallSign(callsign)
          {
            if (service)
              dispatcher_ = service->QueryInterfaceByCallsign<PluginHost::IDispatcher>(mCallSign);
          }
      
          JSONRPCDirectLink(PluginHost::IShell* service)
            : JSONRPCDirectLink(service, "Controller")
          {
          }
          ~JSONRPCDirectLink()
          {
            if (dispatcher_)
              dispatcher_->Release();
          }

          template <typename PARAMETERS>
          uint32_t Get(const uint32_t waitTime, const string& method, PARAMETERS& respObject)
          {
            JsonObject empty;
            return Invoke(waitTime, method, empty, respObject);
          }

          template <typename PARAMETERS>
          uint32_t Set(const uint32_t waitTime, const string& method, const PARAMETERS& sendObject)
          {
            JsonObject empty;
            return Invoke(waitTime, method, sendObject, empty);
          }

          template <typename PARAMETERS, typename RESPONSE>
          uint32_t Invoke(const uint32_t waitTime, const string& method, const PARAMETERS& parameters, RESPONSE& response, bool isResponseString=false)
          {
            if (dispatcher_ == nullptr) {
              std::cout << "No JSON RPC dispatcher for " << mCallSign << '\n';
              return Core::ERROR_GENERAL;
            }

            auto message = Message();

            message->JSONRPC = Core::JSONRPC::Message::DefaultVersion;
            message->Id = Core::JSON::DecUInt32(++mId);
            message->Designator = Core::JSON::String(mCallSign + ".1." + method);

            ToMessage(parameters, message);

            const uint32_t channelId = ~0;
#ifndef USE_THUNDER_R4
            auto resp =  dispatcher_->Invoke(sThunderSecurityToken, channelId, *message);
#else
            Core::JSONRPC::Context context(channelId, message->Id.Value(), sThunderSecurityToken) ;
            auto resp = dispatcher_->Invoke(context, *message);
#endif /* USE_THUNDER_R4 */
            if (resp->Error.IsSet()) {
              std::cout << "Call failed: " << message->Designator.Value() << " error: " <<  resp->Error.Text.Value() << "\n";
              return resp->Error.Code;
            }

            if (!FromMessage(response, resp, isResponseString))
              return Core::ERROR_GENERAL;

            return Core::ERROR_NONE;
          }
        };

        class StateControlNotification: public PluginHost::IStateControl::INotification
        {
          RDKShell& mRDKShell;
          std::string mCallSign;
          bool mLaunchEnabled { false };
        public:
          StateControlNotification(std::string callsign, RDKShell& shell)
            : mRDKShell(shell)
            , mCallSign(callsign)
          {
          }

          void enableLaunch(bool enable)
          {
            mLaunchEnabled = enable;
          }

          void StateChange(const PluginHost::IStateControl::state state) override
          {
            bool isSuspended = (state == PluginHost::IStateControl::SUSPENDED);
            if (mLaunchEnabled)
            {
               JsonObject params;
               params["client"] = mCallSign;
               params["launchType"] = (isSuspended)?"suspend":"resume";
               mRDKShell.notify(RDKShell::RDKSHELL_EVENT_ON_LAUNCHED, params);
               mLaunchEnabled = false;
            }

            if (isSuspended)
            {
                JsonObject params;
                params["client"] = mCallSign;
                mRDKShell.notify(RDKShell::RDKSHELL_EVENT_ON_PLUGIN_SUSPENDED, params);

#ifdef HIBERNATE_NATIVE_APPS_ON_SUSPENDED
                RFC_ParamData_t param;
                if (Utils::getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AppHibernate.Enable", param)
                    && strncasecmp(param.value, "true", 4) == 0)
                {
                    nativeAppWasResumedMutex.lock();
                    if ((mCallSign.find("Netflix") != std::string::npos || mCallSign.find("Cobalt") != std::string::npos)
                        && nativeAppWasResumed.find(mCallSign) != nativeAppWasResumed.end()
                        && nativeAppWasResumed[mCallSign])
                    {
                        // call RDKShell.checkpoint
                        std::thread requestsThread =
                            std::thread([=]()
                                        {
                        JsonObject checkpointParams;
                        JsonObject checkpointResponse;
                        checkpointParams["callsign"] = mCallSign;
                        mRDKShell.getThunderControllerClient("org.rdk.RDKShell.1")->Invoke<JsonObject, JsonObject>(0, "checkpoint", checkpointParams, checkpointResponse); });

                        requestsThread.detach();
                    }
                    nativeAppWasResumedMutex.unlock();
                }
#endif
            }
#ifdef HIBERNATE_NATIVE_APPS_ON_SUSPENDED
            nativeAppWasResumedMutex.lock();
            nativeAppWasResumed[mCallSign] = (state == PluginHost::IStateControl::RESUMED);
            nativeAppWasResumedMutex.unlock();
#endif

          }

          BEGIN_INTERFACE_MAP(Notification)
          INTERFACE_ENTRY(PluginHost::IStateControl::INotification)
          END_INTERFACE_MAP
        };

        std::map<std::string, StateControlNotification*> gStateNotifications;

        struct RDKShellStartupConfig
        {
            std::string rfc;
            std::string thunderApi;
            JsonObject params;
        };

        std::map<std::string, PluginData> gActivePluginsData;
        std::map<std::string, PluginStateChangeData*> gPluginsEventListener;
        std::vector<RDKShellStartupConfig> gStartupConfigs;
        std::map<std::string, bool> gDestroyApplications;
        std::map<std::string, bool> gExternalDestroyApplications;
        std::map<std::string, bool> gLaunchApplications;
        std::map<std::string, AppLastExitReason> gApplicationsExitReason;
        std::map<std::string, std::string> gPluginDisplayNameMap;
        
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

        SERVICE_REGISTRATION(RDKShell, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        RDKShell* RDKShell::_instance = nullptr;
        std::mutex gRdkShellMutex;
        std::mutex gPluginDataMutex;
        std::mutex gLaunchDestroyMutex;
        std::mutex gDestroyMutex;

        std::mutex gLaunchMutex;
        std::mutex gExitReasonMutex;
	std::mutex gSubscribeMutex;
        int32_t gLaunchCount = 0;

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

        void RDKShell::launchRequestThread(RDKShellApiRequest apiRequest)
        {
	    std::thread rdkshellRequestsThread = std::thread([=]() {
                JsonObject result;
                std::string requestName = apiRequest.mName;
                if (requestName.compare("launchFactoryApp") == 0)
                {
                    launchFactoryAppWrapper(apiRequest.mRequest, result);
                }
		        else if (requestName.compare("launchResidentApp") == 0)
                {
                    launchResidentAppWrapper(apiRequest.mRequest, result);
                }
		        else if (requestName.compare("toggleFactoryApp") == 0)
                {
                    toggleFactoryAppWrapper(apiRequest.mRequest, result);
                }
		        else if (requestName.compare("exitAgingMode") == 0)
                {
                    exitAgingModeWrapper(apiRequest.mRequest, result);
                }
                else if (requestName.compare("launchFactoryAppShortcut") == 0)
                {
                    launchFactoryAppShortcutWrapper(apiRequest.mRequest, result);
                }
		else if (requestName.compare("deactivateresidentapp") == 0)
                {
                    auto deactivateStatus = deactivate(mCurrentService, "ResidentApp");
                    std::cout << "deactivating resident app status " << deactivateStatus << std::endl;
                }
		else if (requestName.compare("susbscribeSystemEvent") == 0)
                {
                   gSubscribeMutex.lock();
		   subscribeForSystemEvent("onSystemPowerStateChanged");
		   gSubscribeMutex.unlock();
                   std::cout << "subscribed system event " << std::endl;
                   JsonObject joGetParams;
                    JsonObject joGetResult;
                    joGetParams["params"] = JsonObject();
                    std::string getPowerStateInvoke = "org.rdk.System.1.getPowerState";
                    auto thunderController = getThunderControllerClient();
                    thunderController->Invoke<JsonObject, JsonObject>(5000, getPowerStateInvoke.c_str(), joGetParams, joGetResult);
                    const std::string currentPowerState = joGetResult["powerState"].String();
                    if (currentPowerState == "ON")
                    {
                        JsonObject request, response;
                        request["callsign"] = "ResidentApp";
                        request["visible"] = true;
                        getThunderControllerClient("org.rdk.RDKShell.1")->Invoke<JsonObject, JsonObject>(0, "setVisibility", request, response);
                    }
		 }
                else
                {
                    std::string api("org.rdk.RDKShell.1.");
                    api.append(requestName);
                    auto thunderController = getThunderControllerClient();
                    JsonObject joResult;
                    thunderController->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, api.c_str(), apiRequest.mRequest, joResult);
                } 
            });
            rdkshellRequestsThread.detach();
        }

        void lockRdkShellMutex()
        {
            bool lockAcquired = false;
            double startTime = RdkShell::milliseconds();
            while (!lockAcquired && (RdkShell::milliseconds() - startTime) < RDKSHELL_TRY_LOCK_WAIT_TIME_IN_MS)
            {
                lockAcquired = gRdkShellMutex.try_lock();
            }
            if (!lockAcquired)
            {
                std::cout << "unable to get lock for defaulting to normal lock\n";
                gRdkShellMutex.lock();
            }
            /*else
            {
                std::cout << "lock was acquired via try\n";
            }*/
        }

        static bool isClientExists(std::string client)
        {
            bool exist = false;
            lockRdkShellMutex();
            for (unsigned int i=0; i<gCreateDisplayRequests.size(); i++)
            {
              if (gCreateDisplayRequests[i]->mClient.compare(client) == 0)
              {
                exist = true;
                break;
              }
            }
            gRdkShellMutex.unlock();

            if (!exist)
            {
                std::vector<std::string> clientList;
                lockRdkShellMutex();
                CompositorController::getClients(clientList);
                gRdkShellMutex.unlock();
                std::string newClient(client);
                transform(newClient.begin(), newClient.end(), newClient.begin(), ::tolower);
                if (std::find(clientList.begin(), clientList.end(), newClient) != clientList.end())
                {
                    exist = true;
                }
            }
            return exist;
        }
       
        static void updateSurfaceClientIdentifiers(PluginHost::IShell* shell)
        {
          uint32_t status = 0;
          auto thunderController = RDKShell::getThunderControllerClient();
          string configString;
          Core::JSON::ArrayType<PluginHost::MetaData::Service> availablePluginResult;
          status = thunderController->Get<Core::JSON::ArrayType<PluginHost::MetaData::Service>>(RDKSHELL_THUNDER_TIMEOUT, "status", availablePluginResult);
          if(status > 0)
	  {
            std::cout<<"pluginfo status falied"<<std::endl;
	  }
	  else
          {
           for (uint16_t i = 0; i < availablePluginResult.Length(); i++)
           {
            PluginHost::MetaData::Service service = availablePluginResult[i];
	    std::string configLine;
	    service.Configuration.ToString(configLine);
	    JsonObject serviceConfig = JsonObject(configLine.c_str());
	    if (serviceConfig.HasLabel("clientidentifier"))
	    { 
	     JsonObject configSet;
             std::string pluginName = service.Callsign.Value();
             status = getConfig(shell, pluginName, configString);
             configSet.FromString(configString);
             configSet["clientidentifier"] = RDKSHELL_SURFACECLIENT_DISPLAYNAME;
             string configSetAsString;
             configSet.ToString(configSetAsString);
             Core::JSON::String configSetAsJsonString;
             configSetAsJsonString.FromString(configSetAsString);
             status = setConfig(shell, pluginName, configSetAsJsonString.Value());
             if(status > 0)
             {
                std::cout<<"clientidentifier config set failed"<<std::endl;
	     }
            }
	   }
	  }
       }

        std::string toLower(const std::string& clientName)
        {
            std::string displayName = clientName;
            std::transform(displayName.begin(), displayName.end(), displayName.begin(), [](unsigned char c){ return std::tolower(c); });
            return displayName;
        }

        static bool ignoreResidentAppLaunch(bool activated=false)
	{
            bool ignoreLaunch = false;  
            if (sFactoryModeBlockResidentApp && !sForceResidentAppLaunch)
            {
                // not first launch
                if (sResidentAppFirstActivated)
                {
                    if(sFactoryAppLaunchStatus != NOTLAUNCHED)
                    {
                        ignoreLaunch = true;
                    }
                }
                else
                {
	            if (activated)
	            {		    
                        sResidentAppFirstActivated = true;
                    }
                    if (sFactoryModeStart)
                    {
                        ignoreLaunch = true;
                    }
                }
            }
	    return ignoreLaunch;
	}

        void RDKShell::MonitorClients::StateChange(PluginHost::IShell* service)
        {
            if (service)
            {
                PluginHost::IShell::state currentState(service->State());

                gExitReasonMutex.lock();
                if ((currentState == PluginHost::IShell::DEACTIVATED) || (currentState == PluginHost::IShell::DESTROYED))
                {
                     gApplicationsExitReason[service->Callsign()] = AppLastExitReason::DEACTIVATED;
#ifdef HIBERNATE_NATIVE_APPS_ON_SUSPENDED
                    nativeAppWasResumedMutex.lock();
                    nativeAppWasResumed[service->Callsign()] = false;
                    nativeAppWasResumedMutex.unlock();
#endif
                }
                if(service->Reason() == PluginHost::IShell::FAILURE)
                {
                    gApplicationsExitReason[service->Callsign()] = AppLastExitReason::CRASH;
                }
                gExitReasonMutex.unlock();

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
                       std::string serviceCallsign = service->Callsign();
                       if ((serviceCallsign == RESIDENTAPP_CALLSIGN) && ignoreResidentAppLaunch())
		       {      
		           std::cout << "Resident app activation early !!! " << std::endl;
			   return;
		       }
                       if (!isClientExists(serviceCallsign))
                       {
                           std::shared_ptr<CreateDisplayRequest> request = std::make_shared<CreateDisplayRequest>(serviceCallsign, clientidentifier);
                           gRdkShellMutex.lock();
                           gCreateDisplayRequests.push_back(request);
                           gRdkShellMutex.unlock();
                           sem_wait(&request->mSemaphore);
                       }
                       gRdkShellMutex.lock();
                       RdkShell::CompositorController::addListener(clientidentifier, mShell.mEventListener);
                       gRdkShellMutex.unlock();
                       gPluginDataMutex.lock();
                       std::string className = service->ClassName();
                       PluginData pluginData;
                       pluginData.mClassName = className;
                       if (gActivePluginsData.find(serviceCallsign) == gActivePluginsData.end())
                       {
                           gActivePluginsData[serviceCallsign] = pluginData;
                       }
                       gPluginDataMutex.unlock();
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
                else if (currentState == PluginHost::IShell::ACTIVATED && service->Callsign() == RESIDENTAPP_CALLSIGN)
                {
                    bool ignoreLaunch = ignoreResidentAppLaunch(true);
                    if (ignoreLaunch)
                    {
                        std::cout << "deactivating resident app as factory mode is set" << std::endl;
                        JsonObject destroyRequest;
                        destroyRequest["callsign"] = "ResidentApp";
                        RDKShellApiRequest apiRequest;
                        apiRequest.mName = "destroy";
                        apiRequest.mRequest = destroyRequest;
                        RDKShell* rdkshellPlugin = RDKShell::_instance;
                        rdkshellPlugin->launchRequestThread(apiRequest);
                    }
                }
                else if (currentState == PluginHost::IShell::ACTIVATED && service->Callsign() == PERSISTENT_STORE_CALLSIGN && !sPersistentStoreFirstActivated)
                {
                    std::cout << "persistent store activated\n";
                    gRdkShellMutex.lock();
                    sPersistentStoreFirstActivated = true;
                    gRdkShellMutex.unlock();
                }
		else if (currentState == PluginHost::IShell::ACTIVATED && service->Callsign() == SYSTEM_SERVICE_CALLSIGN)
                {
                        RDKShellApiRequest apiRequest;
                        apiRequest.mName = "susbscribeSystemEvent";
                        RDKShell* rdkshellPlugin = RDKShell::_instance;
                        rdkshellPlugin->launchRequestThread(apiRequest);
                }
                else if (currentState == PluginHost::IShell::DEACTIVATION)
                {
                    gLaunchDestroyMutex.lock();
                    if (gDestroyApplications.find(service->Callsign()) == gDestroyApplications.end())
                    {
                        gExternalDestroyApplications[service->Callsign()] = true;
                    }
                    gLaunchDestroyMutex.unlock();
                    StateControlNotification* notification = nullptr;
                    gPluginDataMutex.lock();
                    auto notificationIt = gStateNotifications.find(service->Callsign());
                    if (notificationIt != gStateNotifications.end())
                    {
                        notification = notificationIt->second;
                        gStateNotifications.erase(notificationIt);
                    }
                    gPluginDataMutex.unlock();
                    if (notification)
                    {
                        PluginHost::IStateControl* stateControl(service->QueryInterface<PluginHost::IStateControl>());
                        if (stateControl != nullptr)
                        {
                            stateControl->Unregister(notification);
                            stateControl->Release();
                        }
                        notification->Release();
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
                        std::shared_ptr<KillClientRequest> request = std::make_shared<KillClientRequest>(service->Callsign());
                        gRdkShellMutex.lock();
                        gKillClientRequests.push_back(request);
                        gRdkShellMutex.unlock();
                        sem_wait(&request->mSemaphore);
                        gRdkShellMutex.lock();
                        RdkShell::CompositorController::removeListener(clientidentifier, mShell.mEventListener);
                        gRdkShellMutex.unlock();
                    }
                    
                    gPluginDataMutex.lock();
                    std::map<std::string, PluginData>::iterator pluginToRemove = gActivePluginsData.find(service->Callsign());
                    if (pluginToRemove != gActivePluginsData.end())
                    {
                        gActivePluginsData.erase(pluginToRemove);
                    }
                    std::map<std::string, PluginStateChangeData*>::iterator pluginStateChangeEntry = gPluginsEventListener.find(service->Callsign());
                    if (pluginStateChangeEntry != gPluginsEventListener.end())
                    {
                        PluginStateChangeData* stateChangeData = pluginStateChangeEntry->second;
                        if (nullptr != stateChangeData)
                        {
                            stateChangeData->resetConnection();
                            delete stateChangeData;
                        }
                        pluginStateChangeEntry->second = nullptr;
                        gPluginsEventListener.erase(pluginStateChangeEntry);
                    }
                    gPluginDataMutex.unlock();
                    gLaunchDestroyMutex.lock();
                    if (gExternalDestroyApplications.find(service->Callsign()) != gExternalDestroyApplications.end())
                    {
                        gExternalDestroyApplications.erase(service->Callsign());
                    }
                    gLaunchDestroyMutex.unlock();
                }
            }
        }

#ifdef USE_THUNDER_R4
       void RDKShell::MonitorClients::Activation(const string& callsign, PluginHost::IShell* service)
       {
           StateChange(service);
       }
       void RDKShell::MonitorClients::Activated(const string& callsign, PluginHost::IShell* service)
       {
            StateChange(service);
       }
       void RDKShell::MonitorClients::Deactivation(const string& callsign, PluginHost::IShell* service)
       {
           StateChange(service);
       }
       void RDKShell::MonitorClients::Deactivated(const string& callsign, PluginHost::IShell* service)
       {
            StateChange(service);
       }
       void RDKShell::MonitorClients::Unavailable(const string& callsign, PluginHost::IShell* service)
       {}
#endif /* USE_THUNDER_R4 */

        bool RDKShell::ScreenCapture::Capture(ICapture::IStore& storer)
        {
            mCaptureStorers.push_back(&storer);

            JsonObject parameters, response;
            mShell->getScreenshotWrapper(parameters, response);
            return true;
        }

        void RDKShell::ScreenCapture::onScreenCapture(const unsigned char *data, unsigned int width, unsigned int height)
        {
            if (mCaptureStorers.size() > 0)
            {
                for (unsigned int n = 0; n < mCaptureStorers.size(); n++)
                {    
                    mCaptureStorers[n]->R8_G8_B8_A8(data, width, height);
                }
                mCaptureStorers.clear();
            }
        }

        RDKShell::RDKShell()
                : PluginHost::JSONRPC(),
                mEnableUserInactivityNotification(true),
                mClientsMonitor(Core::Service<MonitorClients>::Create<MonitorClients>(this)),
                mCurrentService(nullptr), mLastWakeupKeyCode(0),
                mLastWakeupKeyModifiers(0),
                mLastWakeupKeyTimestamp(0),
                mEnableEasterEggs(true),
                mScreenCapture(this),
                mErmEnabled(false)
        {
            LOGINFO("ctor");
            RDKShell::_instance = this;
            mEventListener = std::make_shared<RdkShellListener>(this);

            mRemoteShell = false;
            Register(RDKSHELL_METHOD_MOVE_TO_FRONT, &RDKShell::moveToFrontWrapper, this);
            Register(RDKSHELL_METHOD_MOVE_TO_BACK, &RDKShell::moveToBackWrapper, this);
            Register(RDKSHELL_METHOD_MOVE_BEHIND, &RDKShell::moveBehindWrapper, this);
            Register(RDKSHELL_METHOD_SET_FOCUS, &RDKShell::setFocusWrapper, this);
            Register(RDKSHELL_METHOD_KILL, &RDKShell::killWrapper, this);
            Register(RDKSHELL_METHOD_ADD_KEY_INTERCEPT, &RDKShell::addKeyInterceptWrapper, this);
            Register(RDKSHELL_METHOD_ADD_KEY_INTERCEPTS, &RDKShell::addKeyInterceptsWrapper, this);
            Register(RDKSHELL_METHOD_REMOVE_KEY_INTERCEPT, &RDKShell::removeKeyInterceptWrapper, this);
            Register(RDKSHELL_METHOD_ADD_KEY_LISTENER, &RDKShell::addKeyListenersWrapper, this);
            Register(RDKSHELL_METHOD_REMOVE_KEY_LISTENER, &RDKShell::removeKeyListenersWrapper, this);
            Register(RDKSHELL_METHOD_ADD_KEY_METADATA_LISTENER, &RDKShell::addKeyMetadataListenerWrapper, this);
            Register(RDKSHELL_METHOD_REMOVE_KEY_METADATA_LISTENER, &RDKShell::removeKeyMetadataListenerWrapper, this);
            Register(RDKSHELL_METHOD_INJECT_KEY, &RDKShell::injectKeyWrapper, this);
            Register(RDKSHELL_METHOD_GENERATE_KEYS, &RDKShell::generateKeyWrapper, this);
            Register(RDKSHELL_METHOD_GET_SCREEN_RESOLUTION, &RDKShell::getScreenResolutionWrapper, this);
            Register(RDKSHELL_METHOD_SET_SCREEN_RESOLUTION, &RDKShell::setScreenResolutionWrapper, this);
            Register(RDKSHELL_METHOD_CREATE_DISPLAY, &RDKShell::createDisplayWrapper, this);
            Register(RDKSHELL_METHOD_GET_CLIENTS, &RDKShell::getClientsWrapper, this);
            Register(RDKSHELL_METHOD_GET_Z_ORDER, &RDKShell::getZOrderWrapper, this);
            Register(RDKSHELL_METHOD_GET_BOUNDS, &RDKShell::getBoundsWrapper, this);
            Register(RDKSHELL_METHOD_SET_BOUNDS, &RDKShell::setBoundsWrapper, this);
            Register(RDKSHELL_METHOD_GET_VISIBILITY, &RDKShell::getVisibilityWrapper, this);
            Register(RDKSHELL_METHOD_SET_VISIBILITY, &RDKShell::setVisibilityWrapper, this);
            Register(RDKSHELL_METHOD_GET_OPACITY, &RDKShell::getOpacityWrapper, this);
            Register(RDKSHELL_METHOD_SET_OPACITY, &RDKShell::setOpacityWrapper, this);
            Register(RDKSHELL_METHOD_GET_SCALE, &RDKShell::getScaleWrapper, this);
            Register(RDKSHELL_METHOD_SET_SCALE, &RDKShell::setScaleWrapper, this);
            Register(RDKSHELL_METHOD_GET_HOLE_PUNCH, &RDKShell::getHolePunchWrapper, this);
            Register(RDKSHELL_METHOD_SET_HOLE_PUNCH, &RDKShell::setHolePunchWrapper, this);
            Register(RDKSHELL_METHOD_GET_LOG_LEVEL, &RDKShell::getLogLevelWrapper, this);
            Register(RDKSHELL_METHOD_SET_LOG_LEVEL, &RDKShell::setLogLevelWrapper, this);
            Register(RDKSHELL_METHOD_SHOW_SPLASH_LOGO, &RDKShell::showSplashLogoWrapper, this);
            Register(RDKSHELL_METHOD_HIDE_SPLASH_LOGO, &RDKShell::hideSplashLogoWrapper, this);
            Register(RDKSHELL_METHOD_REMOVE_ANIMATION, &RDKShell::removeAnimationWrapper, this);
            Register(RDKSHELL_METHOD_ADD_ANIMATION, &RDKShell::addAnimationWrapper, this);
            Register(RDKSHELL_METHOD_ENABLE_INACTIVITY_REPORTING, &RDKShell::enableInactivityReportingWrapper, this);
            Register(RDKSHELL_METHOD_SET_INACTIVITY_INTERVAL, &RDKShell::setInactivityIntervalWrapper, this);
            Register(RDKSHELL_METHOD_RESET_INACTIVITY_TIME, &RDKShell::resetInactivityTimeWrapper, this);
            Register(RDKSHELL_METHOD_SCALE_TO_FIT, &RDKShell::scaleToFitWrapper, this);
            Register(RDKSHELL_METHOD_LAUNCH, &RDKShell::launchWrapper, this);
            Register(RDKSHELL_METHOD_LAUNCH_APP, &RDKShell::launchApplicationWrapper, this);
            Register(RDKSHELL_METHOD_SUSPEND, &RDKShell::suspendWrapper, this);
            Register(RDKSHELL_METHOD_SUSPEND_APP, &RDKShell::suspendApplicationWrapper, this);
            Register(RDKSHELL_METHOD_RESUME_APP, &RDKShell::resumeApplicationWrapper, this);
            Register(RDKSHELL_METHOD_DESTROY, &RDKShell::destroyWrapper, this);
            Register(RDKSHELL_METHOD_GET_AVAILABLE_TYPES, &RDKShell::getAvailableTypesWrapper, this);
            Register(RDKSHELL_METHOD_GET_STATE, &RDKShell::getState, this);
            Register(RDKSHELL_METHOD_GET_SYSTEM_MEMORY, &RDKShell::getSystemMemoryWrapper, this);
            Register(RDKSHELL_METHOD_GET_SYSTEM_RESOURCE_INFO, &RDKShell::getSystemResourceInfoWrapper, this);
            Register(RDKSHELL_METHOD_SET_MEMORY_MONITOR, &RDKShell::setMemoryMonitorWrapper, this);
            Register(RDKSHELL_METHOD_SHOW_WATERMARK, &RDKShell::showWatermarkWrapper, this);
            Register(RDKSHELL_METHOD_SHOW_FULL_SCREEN_IMAGE, &RDKShell::showFullScreenImageWrapper, this);
            Register(RDKSHELL_METHOD_HIDE_FULL_SCREEN_IMAGE, &RDKShell::hideFullScreenImageWrapper, this);
            Register(RDKSHELL_METHOD_LAUNCH_FACTORY_APP, &RDKShell::launchFactoryAppWrapper, this);
            Register(RDKSHELL_METHOD_LAUNCH_FACTORY_APP_SHORTCUT, &RDKShell::launchFactoryAppShortcutWrapper, this);
            Register(RDKSHELL_METHOD_LAUNCH_RESIDENT_APP, &RDKShell::launchResidentAppWrapper, this);
            Register(RDKSHELL_METHOD_TOGGLE_FACTORY_APP, &RDKShell::toggleFactoryAppWrapper, this);
            Register(RDKSHELL_METHOD_EXIT_AGING_MODE, &RDKShell::exitAgingModeWrapper, this);
            Register(RDKSHELL_METHOD_ENABLE_KEYREPEATS, &RDKShell::enableKeyRepeatsWrapper, this);
            Register(RDKSHELL_METHOD_GET_KEYREPEATS_ENABLED, &RDKShell::getKeyRepeatsEnabledWrapper, this);
            Register(RDKSHELL_METHOD_SET_TOPMOST, &RDKShell::setTopmostWrapper, this);
            Register(RDKSHELL_METHOD_GET_VIRTUAL_RESOLUTION, &RDKShell::getVirtualResolutionWrapper, this);
            Register(RDKSHELL_METHOD_SET_VIRTUAL_RESOLUTION, &RDKShell::setVirtualResolutionWrapper, this);
            Register(RDKSHELL_METHOD_ENABLE_VIRTUAL_DISPLAY, &RDKShell::enableVirtualDisplayWrapper, this);
            Register(RDKSHELL_METHOD_GET_VIRTUAL_DISPLAY_ENABLED, &RDKShell::getVirtualDisplayEnabledWrapper, this);
            Register(RDKSHELL_METHOD_GET_LAST_WAKEUP_KEY, &RDKShell::getLastWakeupKeyWrapper, this);            
            Register(RDKSHELL_METHOD_HIDE_ALL_CLIENTS, &RDKShell::hideAllClientsWrapper, this);
            Register(RDKSHELL_METHOD_GET_SCREENSHOT, &RDKShell::getScreenshotWrapper, this);
            Register(RDKSHELL_METHOD_ENABLE_EASTER_EGGS, &RDKShell::enableEasterEggsWrapper, this);
            Register(RDKSHELL_METHOD_ENABLE_LOGS_FLUSHING, &RDKShell::enableLogsFlushingWrapper, this);
            Register(RDKSHELL_METHOD_GET_LOGS_FLUSHING_ENABLED, &RDKShell::getLogsFlushingEnabledWrapper, this);
            Register(RDKSHELL_METHOD_IGNORE_KEY_INPUTS, &RDKShell::ignoreKeyInputsWrapper, this);
            Register(RDKSHELL_METHOD_SHOW_CURSOR, &RDKShell::showCursorWrapper, this);
            Register(RDKSHELL_METHOD_HIDE_CURSOR, &RDKShell::hideCursorWrapper, this);
            Register(RDKSHELL_METHOD_GET_CURSOR_SIZE, &RDKShell::getCursorSizeWrapper, this);
            Register(RDKSHELL_METHOD_SET_CURSOR_SIZE, &RDKShell::setCursorSizeWrapper, this);
            Register(RDKSHELL_METHOD_ADD_EASTER_EGGS, &RDKShell::addEasterEggsWrapper, this);
            Register(RDKSHELL_METHOD_REMOVE_EASTER_EGGS, &RDKShell::removeEasterEggsWrapper, this);
            Register(RDKSHELL_METHOD_GET_EASTER_EGGS, &RDKShell::getEasterEggsWrapper, this);
            Register(RDKSHELL_METHOD_ENABLE_INPUT_EVENTS, &RDKShell::enableInputEventsWrapper, this);
            Register(RDKSHELL_METHOD_KEY_REPEAT_CONFIG, &RDKShell::keyRepeatConfigWrapper, this);
            Register(RDKSHELL_METHOD_GET_GRAPHICS_FRAME_RATE, &RDKShell::getGraphicsFrameRateWrapper, this);
            Register(RDKSHELL_METHOD_SET_GRAPHICS_FRAME_RATE, &RDKShell::setGraphicsFrameRateWrapper, this);
            Register(RDKSHELL_METHOD_SET_AV_BLOCKED, &RDKShell::setAVBlockedWrapper, this);
            Register(RDKSHELL_METHOD_GET_AV_BLOCKED_APPS, &RDKShell::getBlockedAVApplicationsWrapper, this);
#ifdef HIBERNATE_SUPPORT_ENABLED
            Register(RDKSHELL_METHOD_CHECKPOINT, &RDKShell::checkpointWrapper, this);
            Register(RDKSHELL_METHOD_RESTORE, &RDKShell::restoreWrapper, this);
#endif
      	    m_timer.connect(std::bind(&RDKShell::onTimer, this));
        }

        RDKShell::~RDKShell()
        {
            //LOGINFO("dtor");
        }

        const string RDKShell::Initialize(PluginHost::IShell* service )
        {
            std::cout << "initializing\n";
            char* waylandDisplay = getenv("WAYLAND_DISPLAY");
            if (NULL != waylandDisplay)
            {
                std::cout << "RDKShell WAYLAND_DISPLAY is set to: " << waylandDisplay <<" unsetting WAYLAND_DISPLAY\n";
                unsetenv("WAYLAND_DISPLAY");
            }
            else
            {
                std::cout << "RDKShell WAYLAND_DISPLAY is not set\n";
            }

            mCurrentService = service;
            CompositorController::setEventListener(mEventListener);
            bool factoryMacMatched = false;
#ifdef RFC_ENABLED
            #ifdef RDKSHELL_READ_MAC_ON_STARTUP
            char* mac = new char[19];
            tFHError retAPIStatus;
            std::cout << "calling factory hal init\n";
            factorySD1_init();
            retAPIStatus = getEthernetMAC(mac);
            if(retAPIStatus == E_OK)
            {
                if (strncasecmp(mac,"00:00:00:00:00:00",17) == 0)
                {
                    std::cout << "launching factory app as mac is matching... " << std::endl;
                    factoryMacMatched = true;
                }
                else
                {
                    std::cout << "mac match failed... mac from hal - " << mac << std::endl;
                }
            }
            else
            {
                std::cout << "reading stb mac hal api failed... " << std::endl;
            }
            #else
            RFC_ParamData_t macparam;
            bool macret = Utils::getRFCConfig("Device.DeviceInfo.X_COMCAST-COM_STB_MAC", macparam);
            if (true == macret)
            {
                if (strncasecmp(macparam.value,"00:00:00:00:00:00",17) == 0)
                {
                    std::cout << "launching factory app as mac is matching " << std::endl;
                    factoryMacMatched = true;
                }
                else
                {
                    std::cout << "mac match failed. mac from rfc - " << macparam.value << std::endl;
                }
            }
            else
            {
                std::cout << "reading stb mac rfc failed " << std::endl;
            }
            #endif //RDKSHELL_READ_MAC_ON_STARTUP
#else
            std::cout << "rfc is disabled and unable to check for stb mac " << std::endl;
#endif
#ifdef RFC_ENABLED
            RFC_ParamData_t param;
            bool ret = Utils::getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Power.UserInactivityNotification.Enable", param);
            if (true == ret && param.type == WDMP_BOOLEAN && (strncasecmp(param.value,"true",4) == 0))
            {
              mEnableUserInactivityNotification = true;
              enableInactivityReporting(true);
              ret = Utils::getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Power.UserInactivityNotification.TimeMinutes", param);
              if (true == ret)
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
#else
            mEnableUserInactivityNotification = true;
            enableInactivityReporting(true);
#endif

            // TODO: use interfaces and remove token
            auto security = mCurrentService->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
            if (security != nullptr) {
                string payload = "http://localhost";
                string token;
                if (security->CreateToken(
                        static_cast<uint16_t>(payload.length()),
                        reinterpret_cast<const uint8_t*>(payload.c_str()),
                        token)
                    == Core::ERROR_NONE) {
                    sThunderSecurityToken = token;
                    std::cout << "RDKShell got security token" << std::endl;
                } else {
                    std::cout << "RDKShell failed to get security token" << std::endl;
                }
                security->Release();
            } else {
                std::cout << "No security agent" << std::endl;
            }

            service->Register(mClientsMonitor);

            static PluginHost::IShell* pluginService = nullptr;
            pluginService = service;

            bool waitForPersistentStore = false;
            char* waitValue = getenv("RDKSHELL_WAIT_FOR_PERSISTENT_STORE");
            if (NULL != waitValue)
            {
                std::cout << "waiting for persistent store\n";
                waitForPersistentStore = true;
            }
            if (factoryMacMatched)
            {
                waitForPersistentStore = false;
            }

            char* blockResidentApp = getenv("RDKSHELL_BLOCK_RESIDENTAPP_FACTORYMODE");
            if (NULL != blockResidentApp)
            {
                std::cout << "block resident app on factory mode\n";
                sFactoryModeBlockResidentApp = true;
            }

            mErmEnabled = CompositorController::isErmEnabled();
            shellThread = std::thread([=]() {
                bool isRunning = true;
                gRdkShellMutex.lock();
                RdkShell::initialize();
                if (!waitForPersistentStore)
                {
                    PluginHost::ISubSystem* subSystems(pluginService->SubSystems());
                    if (subSystems != nullptr)
                    {
                        std::cout << "setting platform and graphics\n";
                        fflush(stdout);
                        RDKShell* rdkshellPlugin = RDKShell::_instance;
                        if (factoryMacMatched || ((nullptr != rdkshellPlugin) && (rdkshellPlugin->checkForBootupFactoryAppLaunch())))
                        {
                            sFactoryModeStart = true;
                        }
                        subSystems->Set(PluginHost::ISubSystem::PLATFORM, nullptr);
                        subSystems->Set(PluginHost::ISubSystem::GRAPHICS, nullptr);
                        subSystems->Release();
                        if (sFactoryModeStart) 
                        {
                            JsonObject request, response;
                            std::cout << "about to launch factory app on start without persistent store wait\n";
                            gRdkShellMutex.unlock();
                            if (sFactoryModeBlockResidentApp)
                            {
                                request["nokillresapp"] = "true";
                            }
                            request["resetagingtime"] = "true";
                            RDKShellApiRequest apiRequest;
                            apiRequest.mName = "launchFactoryApp";
                            apiRequest.mRequest = request;
                            rdkshellPlugin->launchRequestThread(apiRequest);
                            gRdkShellMutex.lock();
                        }
                        else
                        {
                          std::cout << "not launching factory app as conditions not matched\n";
                        }
                    }
                }
                isRunning = sRunning;
                gRdkShellMutex.unlock();
                gRdkShellSurfaceModeEnabled = CompositorController::isSurfaceModeEnabled();
                while(isRunning) {
                  const double maxSleepTime = (1000 / gCurrentFramerate) * 1000;
                  double startFrameTime = RdkShell::microseconds();
                  gRdkShellMutex.lock();
                  if (!sPersistentStorePreLaunchChecked)
                  {
                      if (!sPersistentStoreFirstActivated)
                      {
                          PluginHost::IShell::state state;
                          getServiceState(mCurrentService, PERSISTENT_STORE_CALLSIGN, state);
                          if (state == PluginHost::IShell::state::ACTIVATED) {
                              sPersistentStoreFirstActivated = true;
                          }
                      }
                      sPersistentStorePreLaunchChecked = true;
                  }

                  if (waitForPersistentStore && !sPersistentStoreWaitProcessed && sPersistentStoreFirstActivated)
                  {
                    PluginHost::ISubSystem* subSystems(pluginService->SubSystems());
                    RDKShell* rdkshellPlugin = RDKShell::_instance;
                    if (subSystems != nullptr)
                    {
                        if ((nullptr != rdkshellPlugin) && rdkshellPlugin->checkForBootupFactoryAppLaunch())
                        {
                            sFactoryModeStart = true;
                        }
                        std::cout << "setting platform and graphics after wait\n";
                        subSystems->Set(PluginHost::ISubSystem::PLATFORM, nullptr);
                        subSystems->Set(PluginHost::ISubSystem::GRAPHICS, nullptr);
                        subSystems->Release();
                    }
                    sPersistentStoreWaitProcessed = true;
                    if (sFactoryModeStart)
                    {
                        JsonObject request, response;
                        std::cout << "About to launch factory app after persistent store wait\n";
                        gRdkShellMutex.unlock();
                        if (sFactoryModeBlockResidentApp)
                        {
                            request["nokillresapp"] = "true";
                        }
                        request["resetagingtime"] = "true";
                        RDKShellApiRequest apiRequest;
                        apiRequest.mName = "launchFactoryApp";
                        apiRequest.mRequest = request;
                        rdkshellPlugin->launchRequestThread(apiRequest);
                        gRdkShellMutex.lock();
                    }
                    else
                    {
                        std::cout << "Not launching factory app as conditions not matched\n";
                    }
                  }
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
                  if (receivedFullScreenImageRequest)
                  {
                    CompositorController::showFullScreenImage(fullScreenImagePath);
                    fullScreenImagePath = "";
                    receivedFullScreenImageRequest = false;
                  }
                  if (receivedShowWatermarkRequest)
                  {
                    CompositorController::showWatermark();
                    receivedShowWatermarkRequest = false;
                  }
                  if (receivedShowSplashScreenRequest)
                  {
                    CompositorController::showSplashScreen(gSplashScreenDisplayTime);
                    gSplashScreenDisplayTime = 0;
                    receivedShowSplashScreenRequest = false;
                  }
                  RdkShell::draw();
                  if (needsScreenshot)
                  {
                      uint8_t* data = nullptr;
                      uint32_t size;
                      string screenshotBase64;
                      CompositorController::screenShot(data, size);
		      Utils::String::imageEncoder(&data[0], size, true, screenshotBase64);
                      std::cout << "Screenshot success size:" << size << std::endl;
                      JsonObject params;
                      params["imageData"] = screenshotBase64;

                      // Calling Notify instead of  RDKShell::notify to avoid logging of entire screen content
                      LOGINFO("Notify %s", RDKSHELL_EVENT_ON_SCREENSHOT_COMPLETE.c_str());
                      Notify(RDKSHELL_EVENT_ON_SCREENSHOT_COMPLETE, params);

                      unsigned int width = 0,height = 0;
                      if (CompositorController::getScreenResolution(width, height))
                          mScreenCapture.onScreenCapture(&data[0], width, height);

                      free(data);
                      needsScreenshot = false;
                  }
                  RdkShell::update();
                  isRunning = sRunning;
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
            char* thunderAccessValue = getenv("THUNDER_ACCESS_VALUE");
            if (NULL != thunderAccessValue)
            {
                gThunderAccessValue = thunderAccessValue;
            }
            loadStartupConfig();
            invokeStartupThunderApis();
            char* willDestroyWaitTimeValue = getenv("RDKSHELL_WILLDESTROY_EVENT_WAITTIME");
            if (NULL != willDestroyWaitTimeValue)
            {
                gWillDestroyEventWaitTime = atoi(willDestroyWaitTimeValue); 
            }

            m_timer.setInterval(RECONNECTION_TIME_IN_MILLISECONDS);
            m_timer.start();
            std::cout << "Started SystemServices connection timer" << std::endl;
            char* rdkshelltype = getenv("RDKSHELL_COMPOSITOR_TYPE");
            if((rdkshelltype != NULL) && (strcmp(rdkshelltype , "surface") == 0))
            {
	      updateSurfaceClientIdentifiers(mCurrentService);
	    }
#ifdef ENABLE_RIALTO_FEATURE
        LOGWARN("Creating rialto connector");
        RialtoConnector *rialtoBridge = new RialtoConnector();
        rialtoConnector = std::shared_ptr<RialtoConnector>(rialtoBridge);
#endif //  ENABLE_RIALTO_FEATURE
            return "";
        }

        void RDKShell::loadStartupConfig()
        {
#ifdef RFC_ENABLED
            const char* startupConfigFileName = getenv("RDKSHELL_STARTUP_CONFIG");
            if (startupConfigFileName)
            {
                std::ifstream startupConfigFile;
                try
                {
                    startupConfigFile.open(startupConfigFileName, std::ifstream::binary);
                }
                catch (...)
                {
                    std::cout << "RDKShell startup config file read error : [unable to open/read file (" <<  startupConfigFileName << ")]\n";
                    return;
                }
                std::stringstream strStream;
                strStream << startupConfigFile.rdbuf();
                JsonObject startupConfigData;
                try
                {
                    startupConfigData = strStream.str();
                }
                catch(...)
                {
                    std::cout << "RDKShell startup config file read error : [json format is incorrect (" <<  startupConfigFileName << ")]\n";
                    startupConfigFile.close();
                    return;
                }
                startupConfigFile.close();
                
                if (startupConfigData.HasLabel("rdkshellStartup") && (startupConfigData["rdkshellStartup"].Content() == JsonValue::type::ARRAY))
                {
                    const JsonArray& jsonValue = startupConfigData["rdkshellStartup"].Array();
      
                    for (int k = 0; k < jsonValue.Length(); k++)
                    {
                        std::string name("");
                        std::string actionJson("");
      
                        if (!(jsonValue[k].Content() == JsonValue::type::OBJECT))
                        {
                            std::cout << "one of rdkshell startup config entry is of wrong format" << std::endl;
                            continue;
                        }
                        const JsonObject& configEntry = jsonValue[k].Object();
      
                        //check for entry validity
                        if (!(configEntry.HasLabel("RFC") && configEntry.HasLabel("thunderApi")))
                        {
                            std::cout << "one of rdkshell startup config entry is of wrong format or not having RFC/thunderApi parameter" << std::endl;
                            continue;
                        }
      
                        //populate rfc entry
                        const JsonValue& rfcValue = configEntry["RFC"];
                        if (!(rfcValue.Content() == JsonValue::type::STRING))
                        {
                            std::cout << "rfc type is non-string type and so ignoring entry" << std::endl;
                            continue;
                        }
                        std::string rfc = rfcValue.String();

                        //populate thunder api entry
                        const JsonValue& thunderApiValue = configEntry["thunderApi"];
                        if (!(thunderApiValue.Content() == JsonValue::type::STRING))
                        {
                            std::cout << "thunder api type is non-string type and so ignoring entry" << std::endl;
                            continue;
                        }
                        std::string thunderApi = thunderApiValue.String();

                        //populate params
                        JsonObject params;
                        if (configEntry.HasLabel("params"))
                        {
                            const JsonValue& paramsValue = configEntry["params"];
                            if (!(paramsValue.Content() == JsonValue::type::OBJECT))
                            {
                                std::cout << "one of rdkshell config entry has non-object type params" << std::endl;
                                continue;
                            }
                            params =  paramsValue.Object();
                        }
                        RDKShellStartupConfig config; 
                        config.rfc = rfc;
                        config.thunderApi = thunderApi;
                        config.params = params;
                        gStartupConfigs.push_back(config);
                    }
                }
                else
                {
                    std::cout << "Ignored file read due to rdkshellStartup entry is not present";
                }
            }
            else
            {
              std::cout << "Ignored file read due to rdkshell staup config environment variable not set\n";
            }
#else
            std::cout << "rfc is not enabled and not loading startup configs " << std::endl;
#endif
        }

        void RDKShell::invokeStartupThunderApis()
        {
#ifdef RFC_ENABLED
            for (std::vector<RDKShellStartupConfig>::iterator iter = gStartupConfigs.begin() ; iter != gStartupConfigs.end(); iter++)
            {
                std::string rfc = iter->rfc;
                std::string thunderApi = iter->thunderApi;
                JsonObject& apiParams = iter->params;

                RFC_ParamData_t rfcParam;
                bool ret = Utils::getRFCConfig((char*)rfc.c_str(), rfcParam);
                if (true == ret && (strncasecmp(rfcParam.value,"true",4) == 0))
                {
                    std::cout << "invoking thunder api " << thunderApi << std::endl;
                    uint32_t status = 0;
                    JsonObject joResult;
                    status = getThunderControllerClient()->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, thunderApi.c_str(), apiParams, joResult);
                    if (status > 0)
                    {
                        std::cout << "invoking thunder api " << thunderApi << " failed - " << status << std::endl;
                    }
                }
                else
                {
                    std::cout << "rfc " << rfc << " not enabled " << std::endl;
                }
            }
#else
            std::cout << "rfc is not enabled and not invoking thunder apis " << std::endl;
#endif
            gStartupConfigs.clear();
        }

        void RDKShell::Deinitialize(PluginHost::IShell* service)
        {
            LOGINFO("Deinitialize");
            gRdkShellMutex.lock();
            RdkShell::deinitialize();
            sRunning = false;
            gRdkShellMutex.unlock();
            shellThread.join();
            std::vector<std::string> clientList;
            CompositorController::getClients(clientList);
            std::vector<std::string>::iterator ptr;
            for(ptr=clientList.begin();ptr!=clientList.end();ptr++)
            {
               RdkShell::CompositorController::removeListener((*ptr),mEventListener);
            }
            mCurrentService = nullptr;
            service->Unregister(mClientsMonitor);
            mClientsMonitor->Release();
            RDKShell::_instance = nullptr;
            mRemoteShell = false;
            CompositorController::setEventListener(nullptr);
            mEventListener = nullptr;
            mEnableUserInactivityNotification = false;
            gActivePluginsData.clear();
            gRdkShellMutex.lock();
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
            gRdkShellMutex.unlock();
            gExternalDestroyApplications.clear();
        }

        string RDKShell::Information() const
        {
            return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
        }

        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > RDKShell::getThunderControllerClient(std::string callsign, std::string localidentifier)
        {
            string query = "token=" + sThunderSecurityToken;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(gThunderAccessValue)));
            std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > thunderClient = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >(callsign.c_str(), localidentifier.c_str(), false, query);
            return thunderClient;
        }

        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> RDKShell::getPackagerPlugin()
        {
            string query = "token=" + sThunderSecurityToken;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(gThunderAccessValue)));
            return make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>("Packager.1", "", false, query);
        }

        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> RDKShell::getOCIContainerPlugin()
        {
            string query = "token=" + sThunderSecurityToken;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(gThunderAccessValue)));
            return make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>("org.rdk.OCIContainer.1", "", false, query);
        }

        void RDKShell::pluginEventHandler(const JsonObject& parameters)
        {
            std::string message;
            parameters.ToString(message);
            if (parameters.HasLabel("powerState"))
            {
                std::string powerState = parameters["powerState"].String();
                std::string prevState = parameters["currentPowerState"].String();
		std::cout << "powerState and PrevState inside pluginEventHandler " << prevState << "to" << powerState << std::endl;
                if ((powerState.compare("STANDBY") == 0) || (powerState.compare("LIGHT_SLEEP") == 0) || (powerState.compare("DEEP_SLEEP") == 0))
                {
                    std::cout << "Received power state change to sleep " << std::endl;
                    JsonObject request, response;
                    request["visible"] = false;
                    getThunderControllerClient("org.rdk.RDKShell.1")->Invoke<JsonObject, JsonObject>(0, "launchResidentApp", request, response);
                }
 
                if ((prevState == "STANDBY" || prevState == "LIGHT_SLEEP" || prevState == "DEEP_SLEEP" || prevState == "OFF")
                    && powerState == "ON")
                {
		    std::cout << "received power state change to ON" << std::endl;
                    JsonObject request, response;
                    request["callsign"] = "ResidentApp";
                    request["visible"] = true;
                    getThunderControllerClient("org.rdk.RDKShell.1")->Invoke<JsonObject, JsonObject>(0, "setVisibility", request, response);
                    gRdkShellMutex.lock();
                    CompositorController::getLastKeyPress(mLastWakeupKeyCode, mLastWakeupKeyModifiers, mLastWakeupKeyTimestamp);
                    gRdkShellMutex.unlock();
                }
            }
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

        void RDKShell::RdkShellListener::onApplicationActivated(const std::string& client)
        {
            std::cout << "RDKShell onApplicationActivated event received for " << client << std::endl;
            JsonObject params;
            params["client"] = client;
            mShell.notify(RDKSHELL_EVENT_ON_APP_ACTIVATED, params);
        }

        void RDKShell::RdkShellListener::onUserInactive(const double minutes)
        {
          std::cout << "RDKShell onUserInactive event received ..." << minutes << std::endl;
          JsonObject params;
          params["minutes"] = std::to_string(minutes);
          mShell.notify(RDKSHELL_EVENT_ON_USER_INACTIVITY, params);
        }

        void RDKShell::RdkShellListener::onDeviceLowRamWarning(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb)
        {
          std::cout << "RDKShell onDeviceLowRamWarning event received ..." << "free memory - " << freeKb << ", available memory - " << availableKb << ", swap used - " << usedSwapKb << std::endl;
          JsonObject params;
          params["ram"] = freeKb;
          params["availablememory"] = availableKb;
          params["usedswap"] = usedSwapKb;
          mShell.notify(RDKSHELL_EVENT_DEVICE_LOW_RAM_WARNING, params);
        }

        void RDKShell::RdkShellListener::onDeviceCriticallyLowRamWarning(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb)
        {
          std::cout << "RDKShell onDeviceCriticallyLowRamWarning event received ..." << "free memory - " << freeKb << ", available memory - " << availableKb << ", swap used - " << usedSwapKb << std::endl;
          JsonObject params;
          params["ram"] = freeKb;
          params["availablememory"] = availableKb;
          params["usedswap"] = usedSwapKb;
          mShell.notify(RDKSHELL_EVENT_DEVICE_CRITICALLY_LOW_RAM_WARNING, params);
        }

        void RDKShell::RdkShellListener::onDeviceLowRamWarningCleared(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb)
        {
          std::cout << "RDKShell onDeviceLowRamWarningCleared event received ..." << "free memory - " << freeKb << ", available memory - " << availableKb << ", swap used - " << usedSwapKb << std::endl;
          JsonObject params;
          params["ram"] = freeKb;
          params["availablememory"] = availableKb;
          params["usedswap"] = usedSwapKb;
          mShell.notify(RDKSHELL_EVENT_DEVICE_LOW_RAM_WARNING_CLEARED, params);
        }

        void RDKShell::RdkShellListener::onDeviceCriticallyLowRamWarningCleared(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb)
        {
          std::cout << "RDKShell onDeviceCriticallyLowRamWarningCleared event received ..." << "free memory - " << freeKb << ", available memory - " << availableKb << ", swap used - " << usedSwapKb << std::endl;
          JsonObject params;
          params["ram"] = freeKb;
          params["availablememory"] = availableKb;
          params["usedswap"] = usedSwapKb;
          mShell.notify(RDKSHELL_EVENT_DEVICE_CRITICALLY_LOW_RAM_WARNING_CLEARED, params);
        }

        void RDKShell::RdkShellListener::onEasterEgg(const std::string& name, const std::string& actionJson)
        {
          std::cout << "RDKShell onEasterEgg event received ..." << name << std::endl;
          if (false == mShell.mEnableEasterEggs)
          {
              std::cout << "easter eggs disabled and not processing event" << std::endl;
              return;
          }
          
          if (actionJson.length() == 0)
          {
            JsonObject params;
            params["name"] = name;
            mShell.notify(RDKSHELL_EVENT_ON_EASTER_EGG, params);
          }
          else
          {
            try
            {
              JsonObject actionObject = JsonObject(actionJson.c_str());
              if (actionObject.HasLabel("invoke"))
              {
                std::string invoke = actionObject["invoke"].String();
                size_t lastPositionOfDot = invoke.find_last_of(".");
                auto thunderController = getThunderControllerClient();
                bool isRDKShellPluginRequest = false;
                if (lastPositionOfDot != std::string::npos)
                {
                    std::string callsign = invoke.substr(0, lastPositionOfDot);
                    std::cout << "callsign will be " << callsign << std::endl;
                    if (callsign.compare("org.rdk.RDKShell.1") != 0)
                    {
                        activate(mShell.mCurrentService, callsign);
                    }
                    else
                    {
                        isRDKShellPluginRequest = true;
                    }
                }

                std::cout << "invoking method " << invoke.c_str() << std::endl;
                JsonObject joResult;
                if (isRDKShellPluginRequest)
                {
                    RDKShellApiRequest apiRequest;
                    apiRequest.mName = invoke.substr(19);
                    apiRequest.mRequest = actionObject.HasLabel("params")?actionObject["params"].Object():JsonObject();
                    mShell.launchRequestThread(apiRequest);
                }
                else
                {
                    uint32_t status = 0;
                    if (actionObject.HasLabel("params"))
                    {
                        // setting wait Time to 2 seconds
                        gRdkShellMutex.unlock();
                        status = thunderController->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, invoke.c_str(), actionObject["params"].Object(), joResult);
                        gRdkShellMutex.lock();
                    }
                    else
                    {
                      JsonObject joParams;
                      joParams["params"] = JsonObject();
                      // setting wait Time to 2 seconds
                      gRdkShellMutex.unlock();
                      status = thunderController->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, invoke.c_str(), joParams, joResult);
                      gRdkShellMutex.lock();
                    }
                    if (status > 0)
                    {
                        std::cout << "failed to invoke " << invoke << "on easter egg.  status: " << status << std::endl;
                    }
                }
              }
            }
            catch(...)
            {
              std::cout << "error in parsing action for easter egg " << std::endl;
            }
          }
        }

        void RDKShell::RdkShellListener::onPowerKey()
        {
            static double lastPowerKeyTime = 0;
            double currentTime = RdkShell::seconds();
            double powerKeyTime = currentTime - lastPowerKeyTime;
            if (powerKeyTime < RDKSHELL_POWER_TIME_WAIT)
            {
                std::cout << "power key pressed too fast, ignoring " << powerKeyTime << std::endl;
            }
            JsonObject joGetParams;
            JsonObject joGetResult;
            joGetParams["params"] = JsonObject();
            std::string getPowerStateInvoke = "org.rdk.System.1.getPowerState";
            auto thunderController = getThunderControllerClient();
            uint32_t status = thunderController->Invoke<JsonObject, JsonObject>(5000, getPowerStateInvoke.c_str(), joGetParams, joGetResult);

            std::cout << "get power state status: " << status << std::endl;

            if (status > 0)
            {
                std::cout << "error getting the power state\n";
                lastPowerKeyTime = RdkShell::seconds();
                return;
            }

            if (!joGetResult.HasLabel("powerState"))
            {
                std::cout << "the power state was not returned\n";
                lastPowerKeyTime = RdkShell::seconds();
                return;
            }

            const std::string currentPowerState = joGetResult["powerState"].String();
            std::cout << "the current power state is " << currentPowerState << std::endl;
            std::string newPowerState = "ON";
            if (currentPowerState == "ON")
            {
                newPowerState = "STANDBY";
            }

            JsonObject joSetParams;
            JsonObject joSetResult;
            joSetParams.Set("powerState",newPowerState.c_str());
            joSetParams.Set("standbyReason","power button pressed");
            std::string setPowerStateInvoke = "org.rdk.System.1.setPowerState";

            std::cout << "attempting to set the power state to " << newPowerState << std::endl;
            status = thunderController->Invoke<JsonObject, JsonObject>(5000, setPowerStateInvoke.c_str(), joSetParams, joSetResult);
            std::cout << "get power state status second: " << status << std::endl;
            if (status > 0)
            {
                std::cout << "error setting the power state\n";
            }
            lastPowerKeyTime = RdkShell::seconds();
        }

        void RDKShell::RdkShellListener::onSizeChangeComplete(const std::string& client)
        {
            std::cout << "RDKShell onSizeChangeComplete event received ..." << client << std::endl;
            JsonObject params;
            params["client"] = client;
            mShell.notify(RDKSHELL_EVENT_SIZE_CHANGE_COMPLETE, params);
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

                    ociContainerPlugin->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, "getContainerInfo", param, containerInfoResult);

                    // If success is false, the container isn't running so nothing to do
                    if (containerInfoResult["success"].Boolean())
                    {
                        auto containerInfo = containerInfoResult["info"].Object();

                        // Dobby knows about that container - what's it doing?
                        if (containerInfo["state"] == "running" || containerInfo["state"] == "starting")
                        {
                            ociContainerPlugin->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, "stopContainer", param, stopContainerResult);
                        }
                        else if (containerInfo["state"] == "paused")
                        {
                            // Paused, so force stop
                            param["force"] = true;
                            ociContainerPlugin->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, "stopContainer", param, stopContainerResult);
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
#ifdef ENABLE_RIALTO_FEATURE
                            rialtoConnector->deactivateSession(client);
                            //Should we wait for the state change ? Naaah
#endif //ENABLE_RIALTO_FEATURE
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

        uint32_t RDKShell::addKeyInterceptsWrapper(const JsonObject& parameters, JsonObject& response)
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
                lockRdkShellMutex();
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
                lockRdkShellMutex();
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
            lockRdkShellMutex();
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
                lockRdkShellMutex();
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

        uint32_t RDKShell::showSplashLogoWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("displayTime"))
            {
                result = false;
                response["message"] = "please specify display time parameter";
            }
            if (result)
            {
                uint32_t displayTime = parameters["displayTime"].Number();
                lockRdkShellMutex();
                gSplashScreenDisplayTime = displayTime;
                receivedShowSplashScreenRequest = true;
                gRdkShellMutex.unlock();
                if (false == result) {
                    response["message"] = "failed to show splash screen";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::hideSplashLogoWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            LOG_MILESTONE("HIDE_SPLASH_SCREEN");
            bool result = true;

            lockRdkShellMutex();
            result = CompositorController::hideSplashScreen();
            gRdkShellMutex.unlock();

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

        uint32_t RDKShell::resetInactivityTimeWrapper(const JsonObject& parameters, JsonObject& response)
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
                lockRdkShellMutex();
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

            double launchStartTime = RdkShell::seconds();
            bool result = true;
	    bool autoDestroy = true;

            if (!parameters.HasLabel("callsign"))
            {
                result = false;
                response["message"] = "please specify callsign";
            }

            string appCallsign("");
            /*if (result)
            {
                bool launchInProgress = false;
                int32_t currentLaunchCount = 0;
                gLaunchMutex.lock();
                if (gLaunchCount > 0)
                {
                    launchInProgress = true;
                }
                else
                {
                    gLaunchCount++;
                }
                currentLaunchCount = gLaunchCount;
                gLaunchMutex.unlock();
                std::cout << "the current launch count is " << currentLaunchCount << std::endl;
                if (launchInProgress)
                {
                    const string appCallsign = parameters["callsign"].String();
                    std::cout << "launch is in progress.  not able to launch another app: " << appCallsign << std::endl;
                    response["message"] = "failed to launch application.  another launch is in progress";
                    returnResponse(false);
                }
            }*/
            if (result)
            {
                appCallsign = parameters["callsign"].String();
                if (appCallsign.compare("SearchAndDiscovery") == 0)
                {
                    LOG_MILESTONE("PLUI_LAUNCH_START");
                }
                bool isApplicationBeingDestroyed = false;
                gLaunchDestroyMutex.lock();
                if (gDestroyApplications.find(appCallsign) != gDestroyApplications.end())
                {
                    isApplicationBeingDestroyed = true;
                }
                else
                {
                    gLaunchApplications[appCallsign] = true;
                }
                gLaunchDestroyMutex.unlock();
                if (isApplicationBeingDestroyed)
                {
                    gLaunchMutex.lock();
                    gLaunchCount = 0;
                    gLaunchMutex.unlock();
                    response["message"] = "failed to launch application due to active destroy request";
                    returnResponse(false);
                }
                RDKShellLaunchType launchType = RDKShellLaunchType::UNKNOWN;
                const string callsign = parameters["callsign"].String();
                const string callsignWithVersion = callsign + ".1";
                string type;
                if (parameters.HasLabel("type"))
                {
                    type = parameters["type"].String();
                }
                string version = "0.0";
                string uri;
                int32_t x = 0;
                int32_t y = 0;
                uint32_t width = 0;
                uint32_t height = 0;
                bool suspend = false;
                bool visible = true;
                bool focused = true;
                bool blockAV = false;
                string configuration;
                string behind;

                // Ensure cloned plugin displays are in a sub-dir based on
                // plugin classname
                string displayName;
                if (type.empty())
                {
                    displayName = "wst-" + callsign;
                }
                else
                {
                    string xdgDir;
                    Core::SystemInfo::GetEnvironment(_T("XDG_RUNTIME_DIR"), xdgDir);
                    string displaySubdir = xdgDir + "/" + type;
                    Core::Directory(displaySubdir.c_str()).CreatePath();

                    // don't add XDG_RUNTIME_DIR to display name
                    displayName = type + "/" + "wst-" + callsign;
                }

                if (gRdkShellSurfaceModeEnabled)
                {
                    displayName = "rdkshell_display";
                }
                bool scaleToFit = false;
                bool setSuspendResumeStateOnLaunch = true;
                bool holePunch = true;
                bool topmost = false;
                bool focus = false;

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
                if (parameters.HasLabel("topmost"))
                {
                    topmost = parameters["topmost"].Boolean();
                }
                if (parameters.HasLabel("focus"))
                {
                    focus = parameters["focus"].Boolean();
                }
                if (parameters.HasLabel("autodestroy"))
                {
                  autoDestroy = parameters["autodestroy"].Boolean();
                }
                if (parameters.HasLabel("blockAV"))
                {
                    blockAV = parameters["blockAV"].Boolean();
                    if (true == mErmEnabled)
                    {
                        setAVBlocked(callsign, blockAV);
                    }
                }

                //check to see if plugin already exists
                bool newPluginFound = false;
                bool originalPluginFound = false;
                for (std::map<std::string, PluginData>::iterator pluginDataEntry = gActivePluginsData.begin(); pluginDataEntry != gActivePluginsData.end(); pluginDataEntry++)
                {
                    std::string pluginName = pluginDataEntry->first;
                    if (pluginName == callsign)
                    {
                      newPluginFound = true;
                      break;
                    }
                    else if (pluginName == type)
                    {
                      originalPluginFound = true;
                    }
                }
                if ((false == newPluginFound) && (false == originalPluginFound)) {
                    PluginHost::IShell::state state;
                    if (getServiceState(mCurrentService, callsign, state) == Core::ERROR_NONE) {
                        newPluginFound = true;
                    }
                    if (getServiceState(mCurrentService, type, state) == Core::ERROR_NONE) {
                        originalPluginFound = true;
                    }
                }
		if (!newPluginFound && !originalPluginFound)
                {
                    response["message"] = "failed to launch application.  type not found";
                    gLaunchMutex.lock();
                    gLaunchCount = 0;
                    gLaunchMutex.unlock();
		    gLaunchDestroyMutex.lock();
                    gLaunchApplications.erase(appCallsign);
		    gLaunchDestroyMutex.unlock();
                    std::cout << "new launch count loc1: 0\n";
                    returnResponse(false);
                }
                else if (!newPluginFound)
                {
                    std::cout << "attempting to clone type: " << type << " into " << callsign << std::endl;
                    uint32_t status = cloneService(mCurrentService, type, callsign);

                    std::cout << "clone status: " << status << std::endl;
                    if (status > 0)
                    {
                        std::cout << "trying status one more time...\n";
                        status = cloneService(mCurrentService, type, callsign);
                        std::cout << "clone status: " << status << std::endl;
                    }

                    launchType = RDKShellLaunchType::CREATE;
                    {
                        bool lockAcquired = false;
                        double startTime = RdkShell::milliseconds();
                        while (!lockAcquired && (RdkShell::milliseconds() - startTime) < RDKSHELL_TRY_LOCK_WAIT_TIME_IN_MS)
                        {
                            lockAcquired = gRdkShellMutex.try_lock();
                        }
                        if (!lockAcquired)
                        {
                            std::cout << "unable to get lock for create display, defaulting to normal lock\n";
                            gRdkShellMutex.lock();
                        }
                        else
                        {
                            std::cout << "lock was acquired via try for create display\n";
                        }
                    }
                    gRdkShellMutex.unlock();
                    if (!isClientExists(callsign))
                    {
                        std::shared_ptr<CreateDisplayRequest> request = std::make_shared<CreateDisplayRequest>(callsign, displayName, width, height);
                        request->mAutoDestroy = autoDestroy;
                        lockRdkShellMutex();
                        gPluginDisplayNameMap[callsign] = displayName;
                        std::cout << "Added displayname : "<<displayName<< std::endl;
                        gCreateDisplayRequests.push_back(request);
                        gRdkShellMutex.unlock();
                        sem_wait(&request->mSemaphore);
                    }
                }

                string configString;

                uint32_t status = 0;
                status = getConfig(mCurrentService, callsign, configString);

                std::cout << "config status: " << status << std::endl;
                if (status > 0)
                {
                    std::cout << "trying status one more time...\n";
                    status = getConfig(mCurrentService, callsign, configString);
                    std::cout << "config status: " << status << std::endl;
                }

                JsonObject configSet;
                configSet.FromString(configString);

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
                    if (!newPluginFound && !suspend)
                    {
                        setSuspendResumeStateOnLaunch = false;
                    }

#ifdef RFC_ENABLED
                    RFC_ParamData_t param;
                    if (Utils::getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Dobby.Netflix.Enable", param))
                    {
                        JsonObject root;
                        if (strncasecmp(param.value, "true", 4) == 0)
                        {
                            std::cout << "dobby rfc true - launching netflix in container mode " << std::endl;
                            root = configSet["root"].Object();
                            root["mode"] = JsonValue("Container");
                        }
                        else
                        {
                            std::cout << "dobby rfc false - launching netflix in local mode " << std::endl;
                            root = configSet["root"].Object();
                            root["mode"] = JsonValue("Local");
                        }
                        configSet["root"] = root;
                    }
                    else
                    {
                        std::cout << "reading netflix dobby rfc failed " << std::endl;
                    }
#else
                    std::cout << "rfc is disabled and unable to check for netflix container mode " << std::endl;
#endif
                }

                if (type == "Cobalt")
                {
                    if (configuration.find("\"preload\"") == std::string::npos)
                    {
                        // Enable preload for l2s
                        bool preload = suspend;
                        std::cout << "setting Cobalt preload: " << preload << "\n";
                        configSet["preload"] = JsonValue(preload);
                    }

#ifdef RFC_ENABLED
                    RFC_ParamData_t param;
                    if (Utils::getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Dobby.Cobalt.Enable", param))
                    {
                        JsonObject root;
                        if (strncasecmp(param.value, "true", 4) == 0)
                        {
                            std::cout << "dobby rfc true - launching cobalt in container mode " << std::endl;
                            root = configSet["root"].Object();
                            root["mode"] = JsonValue("Container");
                        }
                        else
                        {
                            std::cout << "dobby rfc false - launching cobalt in local mode " << std::endl;
                            root = configSet["root"].Object();
                            root["outofprocess"] = JsonValue(true);
                        }
                        configSet["root"] = root;
                    }
                    else
                    {
                        std::cout << "reading cobalt dobby rfc failed " << std::endl;
                    }
#else
                    std::cout << "rfc is disabled and unable to check for cobalt container mode " << std::endl;
#endif
                }

                // One RFC controls all WPE-based apps
                if (!type.empty() && (type == "HtmlApp" || type == "LightningApp"))
                {
#ifdef RFC_ENABLED
                    RFC_ParamData_t param;
                    if (Utils::getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Dobby.WPE.Enable", param))
                    {
                        JsonObject root;
                        if (strncasecmp(param.value, "true", 4) == 0)
                        {
                            std::cout << "dobby WPE rfc true - launching " << type << " in container mode " << std::endl;
                            root = configSet["root"].Object();
                            root["mode"] = JsonValue("Container");
                        }
                        else
                        {
                            std::cout << "dobby WPE rfc false - launching " << type << " in out-of-process mode " << std::endl;
                            root = configSet["root"].Object();
                            root["outofprocess"] = JsonValue(true);
                        }
                        configSet["root"] = root;
                    }
                    else
                    {
                        std::cout << "reading dobby WPE rfc failed - launching " << type << " in default mode" << std::endl;
                    }
#else
                    std::cout << "rfc is disabled and unable to check for " << type << " container mode " << std::endl;
#endif
                }

                if (!type.empty() && type == "SearchAndDiscoveryApp" )
                {
#ifdef RFC_ENABLED
                    RFC_ParamData_t param;
                    if (Utils::getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Dobby.SAD.Enable", param))
                    {
                        JsonObject root;
                        if (strncasecmp(param.value, "true", 4) == 0)
                        {
                            std::cout << "dobby SAD rfc true - launching " << type << " in container mode " << std::endl;
                            root = configSet["root"].Object();
                            root["mode"] = JsonValue("Container");
                        }
                        else
                        {
                            std::cout << "dobby SAD rfc false - launching " << type << " in out-of-process mode " << std::endl;
                            root = configSet["root"].Object();
                            root["outofprocess"] = JsonValue(true);
                        }
                        configSet["root"] = root;
                    }
                    else
                    {
                        std::cout << "reading dobby SAD rfc failed - launching " << type << " in default mode" << std::endl;
                    }
#else
                    std::cout << "rfc is disabled and unable to check for " << type << " container mode " << std::endl;
#endif
                }

                string configSetAsString;
                configSet.ToString(configSetAsString);
                Core::JSON::String configSetAsJsonString;
                configSetAsJsonString.FromString(configSetAsString);
                status = setConfig(mCurrentService, callsign, configSetAsJsonString.Value());

                std::cout << "set status: " << status << std::endl;
                if (status > 0)
                {
                    std::cout << "trying status one more time...\n";
                    status = setConfig(mCurrentService, callsign, configSetAsJsonString.Value());
                    std::cout << "set status: " << status << std::endl;
                }

                if (launchType == RDKShellLaunchType::UNKNOWN)
                {
                    status = 0;
                    PluginHost::IShell::state state;
                    status = getServiceState(mCurrentService, callsign, state);

                    std::cout << "get status: " << status << std::endl;
                    if (status > 0)
                    {
                        std::cout << "trying status one more time...\n";
                        status = getServiceState(mCurrentService, callsign, state);
                        std::cout << "get status: " << status << std::endl;
                    }

                    if (status == 0)
                    {
                        if (state == PluginHost::IShell::state::DEACTIVATED ||
                            state == PluginHost::IShell::state::DEACTIVATION ||
                            state == PluginHost::IShell::state::PRECONDITION)
                        {
                            launchType = RDKShellLaunchType::ACTIVATE;
                            status = activate(mCurrentService, callsign);

                            std::cout << "activate 1 status: " << status << std::endl;
                            if (status > 0)
                            {
                                std::cout << "trying status one more time...\n";
                                status = activate(mCurrentService, callsign);
                                std::cout << "activate 1 status: " << status << std::endl;
                            }
                        }
                    }
                    else
                    {
                        launchType = RDKShellLaunchType::ACTIVATE;
                        status = activate(mCurrentService, callsign);
                        std::cout << "activate 2 status: " << status << std::endl;
                        if (status > 0)
                        {
                            std::cout << "trying status one more time...\n";
                            status = activate(mCurrentService, callsign);
                            std::cout << "activate 2 status: " << status << std::endl;
                        }
                    }
                }
                else
                {
                    status = activate(mCurrentService, callsign);

                    std::cout << "activate 3 status: " << status << std::endl;
                    if (status > 0)
                    {
                        std::cout << "trying status one more time...\n";
                        status = activate(mCurrentService, callsign);
                        std::cout << "activate 3 status: " << status << std::endl;
                    }
                }

                bool deferLaunch = false;
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
                    {
                        bool lockAcquired = false;
                        double startTime = RdkShell::milliseconds();
                        while (!lockAcquired && (RdkShell::milliseconds() - startTime) < RDKSHELL_TRY_LOCK_WAIT_TIME_IN_MS)
                        {
                            lockAcquired = gRdkShellMutex.try_lock();
                        }
                        if (!lockAcquired)
                        {
                            std::cout << "unable to get lock for get bounds, defaulting to normal lock\n";
                            gRdkShellMutex.lock();
                        }
                        else
                        {
                            std::cout << "lock was acquired via try for get bounds\n";
                        }
                    }
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
                    {
                        bool lockAcquired = false;
                        double startTime = RdkShell::milliseconds();
                        while (!lockAcquired && (RdkShell::milliseconds() - startTime) < RDKSHELL_TRY_LOCK_WAIT_TIME_IN_MS)
                        {
                            lockAcquired = gRdkShellMutex.try_lock();
                        }
                        if (!lockAcquired)
                        {
                            std::cout << "unable to get lock for set bounds, defaulting to normal lock\n";
                            gRdkShellMutex.lock();
                        }
                        else
                        {
                            std::cout << "lock was acquired via try for set bounds\n";
                        }
                    }
                    std::cout << "setting the desired bounds\n";
                    CompositorController::setBounds(callsign, 0, 0, 1, 1); //forcing a compositor resize flush
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

                    gPluginDataMutex.lock();
                    {
                      auto notificationIt = gStateNotifications.find(callsign);
                      if (notificationIt == gStateNotifications.end()) {
                        PluginHost::IStateControl* stateControl(mCurrentService->QueryInterfaceByCallsign<PluginHost::IStateControl>(callsign));
                        if (stateControl) {
                          auto* handler = new Core::Sink<StateControlNotification>(callsign, *this);
                          stateControl->Register(handler);
                          stateControl->Release();
                          gStateNotifications[callsign] = handler;
                        }
                      } else {
                        notificationIt->second->enableLaunch(true);
                        deferLaunch = true;
                      }
                    }
                    gPluginDataMutex.unlock();
 
                    if (setSuspendResumeStateOnLaunch)
                    {
                        if (suspend)
                        {
                            if (launchType == RDKShellLaunchType::UNKNOWN)
                            {
                                gPluginDataMutex.lock();
                                std::map<std::string, PluginStateChangeData*>::iterator pluginStateChangeEntry = gPluginsEventListener.find(callsign);
                                if (pluginStateChangeEntry != gPluginsEventListener.end())
                                {
                                    PluginStateChangeData* data = pluginStateChangeEntry->second;
                                    data->enableLaunch(true);
                                    deferLaunch = true;
                                }
                                gPluginDataMutex.unlock();
                                launchType = RDKShellLaunchType::SUSPEND;
                            }

                            WPEFramework::Core::JSON::String stateString;
                            stateString = "suspended";
                            status = JSONRPCDirectLink(mCurrentService, callsign).Set<WPEFramework::Core::JSON::String>(RDKSHELL_THUNDER_TIMEOUT, "state", stateString);
                            
                            std::cout << "setting the state to suspended\n";
                            visible = false;
                        }
                        else
                        {
                            if (launchType == RDKShellLaunchType::UNKNOWN)
                            {
                                gPluginDataMutex.lock();
                                std::map<std::string, PluginStateChangeData*>::iterator pluginStateChangeEntry = gPluginsEventListener.find(callsign);
                                if (pluginStateChangeEntry != gPluginsEventListener.end())
                                {
                                    PluginStateChangeData* data = pluginStateChangeEntry->second;
                                    data->enableLaunch(true);
                                    deferLaunch = true;
                                }
                                gPluginDataMutex.unlock();
                                launchType = RDKShellLaunchType::RESUME;
                            }
                            
                            WPEFramework::Core::JSON::String stateString;
                            stateString = "resumed";
                            status = JSONRPCDirectLink(mCurrentService, callsign).Set<WPEFramework::Core::JSON::String>(RDKSHELL_THUNDER_TIMEOUT, "state", stateString);

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

                    setTopmost(callsign, topmost, focus);
                    JsonObject urlResult;
                    if (!uri.empty())
                    {
                        WPEFramework::Core::JSON::String urlString;
                        urlString = uri;
                        status = JSONRPCDirectLink(mCurrentService, callsign).Set<WPEFramework::Core::JSON::String>(RDKSHELL_THUNDER_TIMEOUT, "url",urlString);
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
                    std::cout << "Application:" << callsign << " took " << (RdkShell::seconds() - launchStartTime)*1000 << " milliseconds to launch " << std::endl;
                    gLaunchMutex.lock();
                    gLaunchCount = 0;
                    gLaunchMutex.unlock();
                    if (setSuspendResumeStateOnLaunch && deferLaunch && ((launchType == SUSPEND) || (launchType == RESUME)))
                    {
                        std::cout << "deferring application launch " << std::endl;
                    }
                    else
                    {
                        onLaunched(callsign, launchTypeString);
                    }
                    response["launchType"] = launchTypeString;
                }
                
            }
            if (!result) 
            {
                response["message"] = "failed to launch application";
            }
            gLaunchMutex.lock();
            gLaunchCount = 0;
            gLaunchMutex.unlock();
	    gLaunchDestroyMutex.lock();
            gLaunchApplications.erase(appCallsign);
	    gLaunchDestroyMutex.unlock();
            std::cout << "new launch count at loc2 is 0\n";

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
                uint32_t status;
                const string callsign = parameters["callsign"].String();
                std::cout << "about to suspend " << callsign << std::endl;
		string client;
                if (parameters.HasLabel("client"))
                {
                    client = parameters["client"].String();
                }
                else
                {
                    client = parameters["callsign"].String();
                }
                bool isApplicationBeingDestroyed = false;
            	gLaunchDestroyMutex.lock();
            	if (gDestroyApplications.find(client) != gDestroyApplications.end())
            	{
                    isApplicationBeingDestroyed = true;
            	}
            	if (gExternalDestroyApplications.find(client) != gExternalDestroyApplications.end())
            	{
                    isApplicationBeingDestroyed = true;
            	}
            	gLaunchDestroyMutex.unlock();
            	if (isApplicationBeingDestroyed)
            	{
                    std::cout << "ignoring suspend for " << client << " as it is being destroyed " << std::endl;
		    result=false;
		    response["message"] = "failed to suspend application";
                    returnResponse(result);
            	}
                gDestroyMutex.lock();
                PluginHost::IStateControl* stateControl(mCurrentService->QueryInterfaceByCallsign<PluginHost::IStateControl>(callsign));
                if (stateControl)
		{
                    stateControl->Request(PluginHost::IStateControl::SUSPEND);
                    stateControl->Release();
                    gDestroyMutex.unlock();
                    status = Core::ERROR_NONE;
                }
		else
		{
                    gDestroyMutex.unlock();
                    WPEFramework::Core::JSON::String stateString;
                    stateString = "suspended";
                    const string callsignWithVersion = callsign + ".1";
                    status = getThunderControllerClient(callsignWithVersion)->Set<WPEFramework::Core::JSON::String>(RDKSHELL_THUNDER_TIMEOUT, "state", stateString);
                }
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
		setVisibility(callsign, false);
                bool isApplicationBeingLaunched = false;
		gLaunchDestroyMutex.lock();
                if (gLaunchApplications.find(callsign) != gLaunchApplications.end())
                {
                    isApplicationBeingLaunched = true;
                }
                else
                {
                    gDestroyApplications[callsign] = true;
                }
		gLaunchDestroyMutex.unlock();
                if (isApplicationBeingLaunched)
                {
                    std::cout << "failed to destroy " << callsign << " as launch in progress" << std::endl;
                    response["message"] = "failed to destroy application as same application being launched";
                    returnResponse(false);
                }
                std::cout << "destroying " << callsign << std::endl;
                gDestroyMutex.lock();
                uint32_t status = deactivate(mCurrentService, callsign);
                gDestroyMutex.unlock();
                if (status > 0)
                {
                    std::cout << "failed to destroy " << callsign << ".  status: " << status << std::endl;
                    result = false;
                }
                else
                {
                    if (callsign == "factoryapp")
                    {
                        removeFactoryModeEasterEggs();
                        sFactoryModeStart = false;
                        sFactoryAppLaunchStatus = NOTLAUNCHED;
                    }
                    onDestroyed(callsign);
                }
		gLaunchDestroyMutex.lock();
                gDestroyApplications.erase(callsign);
		gLaunchDestroyMutex.unlock();
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
                string uri = parameters["uri"].String();
                const string mimeType = parameters["mimeType"].String();

                if (mimeType == RDKSHELL_APPLICATION_MIME_TYPE_DAC_NATIVE)
                {
                    // Starting a DAC app. Get the info from Packager
                    LOGINFO("Starting DAC app");
                    string bundlePath;
#ifdef ENABLE_RIALTO_FEATURE
                    string appId;
#endif //ENABLE_RIALTO_FEATURE
                    {
                      // find the bundle location
                      JsonObject infoParams;
                      JsonObject infoResult;

                      string id = uri;
                      string version = "1.0";
                      string dactype = "dac";
                      // uri can optionally contain version and dactype
                      // like: id;version;dactype
                      auto delimiterPos = uri.find(";");
                      if (delimiterPos != string::npos)
                      {
                        id = uri.substr(0, delimiterPos);
                        version = uri.substr(delimiterPos + 1);
                        uri = id;
                        delimiterPos = version.find(";");
                        if (delimiterPos != string::npos) {
                          dactype = version.substr(delimiterPos + 1);
                          version = version.substr(0, delimiterPos);
                        }
                      }
                      infoParams.Set("id", id.c_str());
                      infoParams.Set("version", version.c_str());
                      infoParams.Set("type", dactype.c_str());
                      LOGINFO("Querying LISA about dac app: %s %s %s", dactype.c_str(), id.c_str(), version.c_str());
                      uint32_t status = JSONRPCDirectLink(mCurrentService, LISA_CALLSIGN).Invoke<JsonObject, JsonObject>(
                          RDKSHELL_THUNDER_TIMEOUT, "getStorageDetails", infoParams, infoResult);
                      if (status == 0)
                      {
                        if (infoResult.HasLabel("apps") && infoResult["apps"].Object().HasLabel("path"))
                        {
                          bundlePath = infoResult["apps"].Object()["path"].String();
                        }
                        if (bundlePath.empty())
                        {
                          LOGINFO("LISA reports app is not installed");
                        }
                      }
                      else
                      {
                        LOGINFO("LISA not active");
                      }
#ifdef ENABLE_RIALTO_FEATURE
                      appId = id;
#endif // ENABLE_RIALTO_FEATURE
                    }

                    if (bundlePath.empty())
                    {
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

                      bundlePath = infoResult["bundlePath"].String();
                    }

                    // We know where the app lives and are ready to start it,
                    // create a display with rdkshell
                    if (!createDisplay(client, "wst-"+uri))
                    {
                        response["message"] = "Could not create display";
                        returnResponse(false);
                    }

                    string runtimeDir = getenv("XDG_RUNTIME_DIR");
                    string display = runtimeDir + "/" + (gRdkShellSurfaceModeEnabled ? RDKSHELL_SURFACECLIENT_DISPLAYNAME : "wst-"+uri);

                    // Set mime type
                    if (!setMimeType(client, mimeType))
                    {
                        LOGWARN("Failed to set mime type - non fatal...");
                    }
#ifdef ENABLE_RIALTO_FEATURE

                    if(!rialtoConnector->initialized())
                    {
                        LOGWARN("Initializing rialto connector....");
                        rialtoConnector->initialize();
                    }
                    LOGWARN("Creating app session ....");
                    if(!rialtoConnector->createAppSession(client,display, appId))
                    {
                        response["message"] = "Rialto app session initialisation failed";
                        returnResponse(false);
                    }
                    if(!rialtoConnector->waitForStateChange(appId,RialtoServerStates::ACTIVE, RIALTO_TIMEOUT_MILLIS))
                    {
                        response["message"] = "Rialto app session not ready.";
                        returnResponse(false);
                    }
#endif //ENABLE_RIALTO_FEATURE
                    // Start container
                    auto ociContainerPlugin = getOCIContainerPlugin();
                    if (!ociContainerPlugin)
                    {
#ifdef ENABLE_RIALTO_FEATURE
                        rialtoConnector->deactivateSession(client);
#endif //ENABLE_RIALTO_FEATURE
                        response["message"] = "OCIContainer initialisation failed";
                        returnResponse(false);
                    }

                    JsonObject ociContainerResult;
                    JsonObject param;

                    // Container ID set to client so we can find the container
                    // when suspend/resume/killing which use client id
                    param["containerId"] = client;
                    param["bundlePath"] = bundlePath;
                    param["westerosSocket"] = display;

                    ociContainerPlugin->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, "startContainer", param, ociContainerResult);

                    if (!ociContainerResult["success"].Boolean())
                    {
                        // Something went wrong starting the container, destory the display we just created
                        kill(client);
#ifdef ENABLE_RIALTO_FEATURE
                        rialtoConnector->deactivateSession(client);
#endif //ENABLE_RIALTO_FEATURE
                        response["message"] = "Could not start Dobby container";
                        returnResponse(false);
                    }
                }
                else if (mimeType == RDKSHELL_APPLICATION_MIME_TYPE_NATIVE)
                {
                    bool topmost = false;
                    bool focus = false;

                    if (parameters.HasLabel("topmost"))
                    {
                        topmost = parameters["topmost"].Boolean();
                    }
                    if (parameters.HasLabel("focus"))
                    {
                        focus = parameters["focus"].Boolean();
                    }

                    gRdkShellMutex.lock();
                    result = CompositorController::launchApplication(client, uri, mimeType, topmost, focus);
		    RdkShell::CompositorController::addListener(client, mEventListener);
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
                    lockRdkShellMutex();
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

                    ociContainerPlugin->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, "pauseContainer", param, ociContainerResult);

                    if (!ociContainerResult["success"].Boolean())
                    {
                        response["message"] = "Could not pause container";
                        returnResponse(false);
                    }
#ifdef ENABLE_RIALTO_FEATURE
                    rialtoConnector->suspendSession(client);
                    if(!rialtoConnector->waitForStateChange(client,RialtoServerStates::INACTIVE, RIALTO_TIMEOUT_MILLIS))
                    {
                        response["message"] = "Rialto app session could not be set inactive.";
                        returnResponse(false);
		    }
#endif //ENABLE_RIALTO_FEATURE
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
                    lockRdkShellMutex();
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
#ifdef ENABLE_RIALTO_FEATURE
                    rialtoConnector->resumeSession(client);
                    if(!rialtoConnector->waitForStateChange(client,RialtoServerStates::ACTIVE,RIALTO_TIMEOUT_MILLIS))
                    {
                        response["message"] = "Rialto app session not ready.";
                        returnResponse(false);
                    }
#endif //ENABLE_RIALTO_FEATURE
                    JsonObject ociContainerResult;
                    JsonObject param;

                    param["containerId"] = client;
                    ociContainerPlugin->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, "resumeContainer", param, ociContainerResult);

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
            auto thunderController = getThunderControllerClient();
            thunderController->Get<Core::JSON::ArrayType<PluginHost::MetaData::Service>>(RDKSHELL_THUNDER_TIMEOUT, method.c_str(), joResult);

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

        uint32_t RDKShell::setTopmostWrapper(const JsonObject& parameters, JsonObject& response)
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

      uint32_t RDKShell::showWatermarkWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            bool displayWatermark = true;
            if (parameters.HasLabel("show"))
            {
                displayWatermark  = parameters["show"].Boolean();
            }
            result = showWatermark(displayWatermark);
            if (!result)
            {
                response["message"] = "failed to perform show watermark";
            }
            returnResponse(result);
        }

        uint32_t RDKShell::enableEasterEggsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            bool enable = true;
            if (!parameters.HasLabel("enable"))
            {
                response["message"] = "enable parameter is not present";
                returnResponse(false);
            }
            enable = parameters["enable"].Boolean();
            mEnableEasterEggs = enable;
            returnResponse(result);
        }

        uint32_t RDKShell::showFullScreenImageWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("path"))
            {
                result = false;
                response["message"] = "please specify path";
            }
            if (result)
            {
                std::string path = parameters["path"].String();
                result = showFullScreenImage(path);
                if (!result)
                {
                    response["message"] = "failed to perform showw fullscreen";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::hideFullScreenImageWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            lockRdkShellMutex();
            result = CompositorController::hideFullScreenImage();
            gRdkShellMutex.unlock();

            returnResponse(result);
        }

        uint32_t RDKShell::getState(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            string method = "status";
            Core::JSON::ArrayType<PluginHost::MetaData::Service> joResult;
            auto thunderController = getThunderControllerClient();
            thunderController->Get<Core::JSON::ArrayType<PluginHost::MetaData::Service>>(RDKSHELL_THUNDER_TIMEOUT, method.c_str(), joResult);

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
                            uint32_t stateStatus = 0;

#ifdef HIBERNATE_SUPPORT_ENABLED
                            if(service.JSONState != PluginHost::MetaData::Service::state::HIBERNATED)
                            {
#endif
                                stateStatus = getThunderControllerClient(callsignWithVersion)->Get<WPEFramework::Core::JSON::String>(RDKSHELL_THUNDER_TIMEOUT, "state", stateString);
#ifdef HIBERNATE_SUPPORT_ENABLED
                            }
                            else
                            {
                                stateString = "checkpointed";
                            }
#endif


                            if (stateStatus == 0)
                            {
                                WPEFramework::Core::JSON::String urlString;
                                uint32_t urlStatus = 1;
#ifdef HIBERNATE_SUPPORT_ENABLED
                                if(service.JSONState != PluginHost::MetaData::Service::state::HIBERNATED)
                                {
#endif
                                    urlStatus = getThunderControllerClient(callsignWithVersion)->Get<WPEFramework::Core::JSON::String>(RDKSHELL_THUNDER_TIMEOUT, "url",urlString);
#ifdef HIBERNATE_SUPPORT_ENABLED
                                }
#endif

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
                                gExitReasonMutex.lock();
                                if (gApplicationsExitReason.find(callsign) != gApplicationsExitReason.end())
                                {
                                    typeObject["lastExitReason"] = (int)gApplicationsExitReason[callsign];
                                }
                                else
                                {
                                    typeObject["lastExitReason"] = (int)AppLastExitReason::UNDEFINED;
                                }
                                gExitReasonMutex.unlock();

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
            uint32_t freeKb=0, usedSwapKb=0, totalKb=0, availableKb=0;
            result = systemMemory(freeKb, totalKb, availableKb, usedSwapKb);
            if (!result) {
              response["message"] = "failed to get system Ram";
            }
            else
            {
              response["freeRam"] = freeKb;
              response["swapRam"] = usedSwapKb;
              response["totalRam"] = totalKb;
              response["availablememory"] = availableKb;
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
            auto thunderController = getThunderControllerClient();
            thunderController->Get<Core::JSON::ArrayType<PluginHost::MetaData::Service>>(RDKSHELL_THUNDER_TIMEOUT, method.c_str(), joResult);

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
                            uint32_t stateStatus = getThunderControllerClient(callsignWithVersion)->Get<WPEFramework::Core::JSON::String>(RDKSHELL_THUNDER_TIMEOUT, "state", stateString);

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
              std::map<std::string, RdkShellData> configuration;
              configuration["enable"] = parameters["enable"].Boolean();

              if (parameters.HasLabel("interval"))
              {
                configuration["interval"] = std::stod(parameters["interval"].String());
              }
              if (parameters.HasLabel("lowRam"))
              {
                configuration["lowRam"] = std::stod(parameters["lowRam"].String());
              }
              if (parameters.HasLabel("criticallyLowRam"))
              {
                configuration["criticallyLowRam"] = std::stod(parameters["criticallyLowRam"].String());
              }
              RdkShell::setMemoryMonitor(configuration);
            }
            returnResponse(result);
        }

        uint32_t RDKShell::launchFactoryAppWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            if (NOTLAUNCHED != sFactoryAppLaunchStatus)
            {
                std::cout << "factory app is already running, do nothing";
                response["message"] = " factory app already running";
                returnResponse(false);
            }
            sFactoryAppLaunchStatus = STARTED;

            subscribeForSystemEvent("onSystemPowerStateChanged");

            if (parameters.HasLabel("startup"))
            {
                bool startup = parameters["startup"].Boolean();
                if (startup)
                {
                    std::string valueString;

                    std::cout << "attempting to check aging flag \n";
                    uint32_t status = getValue(mCurrentService, "FactoryTest", "AgingState", valueString);
                    std::cout << "get status: " << status << std::endl;

                    if (status > 0)
                    {
                        response["message"] = " unable to check aging flag";
                        sFactoryAppLaunchStatus = NOTLAUNCHED;
                        returnResponse(false);
                    }

                    if (valueString != "true")
                    {
                        std::cout << "aging value is " << valueString << std::endl;
                        response["message"] = " aging is not set for startup";
                        sFactoryAppLaunchStatus = NOTLAUNCHED;
                        returnResponse(false);
                    }
                }
            }

            char* factoryAppUrl = getenv("RDKSHELL_FACTORY_APP_URL");
            if (NULL != factoryAppUrl)
            {
                if (parameters.HasLabel("resetagingtime"))
                {
                    std::cout << "attempting to set aging total time to 0 \n";
                    uint32_t agingTotalTimeSetStatus = setValue(mCurrentService, "FactoryTest", "AgingTotalTime", "0");
                    std::cout << "aging total time set status: " <<  agingTotalTimeSetStatus << std::endl;
                }

                killAllApps(true);
                if (!parameters.HasLabel("nokillresapp"))
                {
                    JsonObject destroyRequest, destroyResponse;
                    destroyRequest["callsign"] = "ResidentApp";
                    destroyWrapper(destroyRequest, destroyResponse);
                }
                JsonObject launchRequest, configuration;
                launchRequest["callsign"] = "factoryapp";
                launchRequest["type"] = "ResidentApp";
                launchRequest["uri"] = std::string(factoryAppUrl);
                configuration["uri"] = launchRequest["uri"];
                launchRequest["focused"] = true;
                launchRequest["configuration"] = configuration;
                std::cout << "launching " << launchRequest["callsign"].String().c_str() << std::endl;
                launchWrapper(launchRequest, response);
                bool launchFactoryResult = response.HasLabel("success")?response["success"].Boolean():false;
                if (true == launchFactoryResult)
                {
                    std::cout << "Launching factory application succeeded " << std::endl;
                }
                else
                {
                    std::cout << "Launching factory application failed " << std::endl;
                    response["message"] = " launching factory application failed ";
                    sFactoryAppLaunchStatus = NOTLAUNCHED;
                    returnResponse(false);
                }
                addFactoryModeEasterEggs();

                std::cout << "attempting to set factory mode flag \n";
                uint32_t setStatus = setValue(mCurrentService, "FactoryTest", "FactoryMode", "true");
                std::cout << "set status: " << setStatus << std::endl;

                std::cout << "attempting to set factory allow exit flag \n";
                uint32_t setExitStatus = setValue(mCurrentService, "FactoryTest", "AllowExit", "true");
                std::cout << "set status: " << setExitStatus << std::endl;

                sFactoryAppLaunchStatus = COMPLETED;
                returnResponse(true);
            }
            else
            {
                std::cout << "factory app url is empty " << std::endl;
                response["message"] = " factory app url is empty";
                sFactoryAppLaunchStatus = NOTLAUNCHED;
                returnResponse(false);
            }
        }

        uint32_t RDKShell::launchFactoryAppShortcutWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            std::string factoryAppCallsign("factoryapp");
            bool isFactoryAppRunning = false;
            std::map<std::string, PluginData>::iterator pluginsEntry = gActivePluginsData.find(factoryAppCallsign);
            if (pluginsEntry != gActivePluginsData.end())
            {
                std::cout << "factory app is running" << std::endl;
                isFactoryAppRunning = true;
            }
            if (isFactoryAppRunning)
            {
                std::cout << "nothing to do since factory app is running in shortcut check\n";
                returnResponse(true);
            }

            std::string valueString;

            std::cout << "attempting to check flag \n";
            uint32_t status = getValue(mCurrentService, "FactoryTest", "ToFacFlag", valueString);
            std::cout << "get status: " << status << std::endl;

            if (status > 0)
            {
                response["message"] = " unable to check toFac flag";
                returnResponse(false);
            }

            if (valueString != "M" && valueString != "m")
            {
                std::cout << "toFac value is " << valueString << std::endl;
                response["message"] = " toFac not in the correct mode";
                returnResponse(false);
            }

            return launchFactoryAppWrapper(parameters, response);
        }

        uint32_t RDKShell::launchResidentAppWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            std::string factoryAppCallsign("factoryapp");
            bool isFactoryAppRunning = false;
            gPluginDataMutex.lock();
            std::map<std::string, PluginData>::iterator pluginsEntry = gActivePluginsData.find(factoryAppCallsign);
            if (pluginsEntry != gActivePluginsData.end())
            {
                std::cout << "factory app is already running" << std::endl;
                isFactoryAppRunning = true;
            }
            gPluginDataMutex.unlock();
            if (!isFactoryAppRunning)
            {
                std::cout << "nothing to do since factory app is not running\n";
                returnResponse(true);
            }
            else
            {
                std::string valueString;

                std::cout << "attempting to check aging flag \n";
                uint32_t agingStatus = getValue(mCurrentService, "FactoryTest", "AgingState", valueString);
                std::cout << "aging get status: " << agingStatus << std::endl;

                if (agingStatus == 0)
                {
                    std::cout << "aging value is " << valueString << std::endl;
                    if (valueString == "true")
                    {
                        std::cout << "do not exit the factory app\n";
                        response["message"] = " aging is true, do not exit the factory app";
                        returnResponse(false);
                    }
                }
                else
                {
                    std::cout << "aging value is not set\n";
                }

                std::cout << "attempting to check factory exit flag\n";
                uint32_t factoryExitStatus = getValue(mCurrentService, "FactoryTest", "AllowExit", valueString);
                std::cout << "factory exit get status: " << factoryExitStatus << std::endl;

                if (factoryExitStatus == 0)
                {
                    std::cout << "exit value is " << valueString << std::endl;
                    if (valueString == "false")
                    {
                        std::cout << "factory exit flag is false.  not allowing the exit of the factory app\n";
                        response["message"] = "factory exit flag is false";
                        returnResponse(false);
                    }
                    else
                    {
                        std::cout << "factory exit flag is true.  allowing the factory app to exit\n";
                    }
                }
                else
                {
                    std::cout << "factory exit flag not found.  not allowing the exit of the factory app\n";
                    response["message"] = "factory exit flag not found";
                    returnResponse(false);
                }
            }
            setVisibility("factoryapp", false);
            std::cout << "RDKShell sending onWillDestroyEvent for factoryapp" << std::endl;
            JsonObject params;
            params["callsign"] = "factoryapp";
            notify(RDKSHELL_EVENT_ON_WILL_DESTROY, params);
            sleep(gWillDestroyEventWaitTime);

            killAllApps();
            //try to kill factoryapp once more if kill apps missed killing due to timeout
            JsonObject destroyRequest, destroyResponse;
            destroyRequest["callsign"] = "factoryapp";
            destroyWrapper(destroyRequest, destroyResponse);

            std::cout << "attempting to stop hdmi input...\n";
            JsonObject joStopHdmiParams;
            JsonObject joStopHdmiResult;
            std::string stopHdmiInvoke = "org.rdk.HdmiInput.1.stopHdmiInput";

            std::cout << "attempting to stop hdmi input \n";
            uint32_t stopHdmiStatus = getThunderControllerClient()->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, stopHdmiInvoke.c_str(), joStopHdmiParams, joStopHdmiResult);
            std::cout << "stopHdmiStatus status: " << stopHdmiStatus << std::endl;

            sForceResidentAppLaunch = true;
            string configString;

            int32_t status = 0;
            status = getConfig(mCurrentService, "ResidentApp", configString);

            std::cout << "config resident app status: " << status << std::endl;
            std::string updatedUrl;
            if (status > 0)
            {
                std::cout << "trying resident app config status one more time...\n";
                status = getConfig(mCurrentService, "ResidentApp", configString);
                std::cout << "trying resident app config status: " << status << std::endl;
            }
            else
            {
                JsonObject configSet;
                configSet.FromString(configString);
                updatedUrl = configSet["url"].String();
                if (updatedUrl.find("?") != string::npos)
                {
                    updatedUrl.append("&adjustPowerStateAtStartup=false");
                }
                else
	        {
                    updatedUrl.append("?adjustPowerStateAtStartup=false");
	        }
            }

            bool ret = true;
            std::string callsign("ResidentApp");
            status = activate(mCurrentService, callsign);
            std::cout << "activate resident app status: " << status << std::endl;
            if (status > 0)
            {
                std::cout << "trying status one more time...\n";
                status = activate(mCurrentService, callsign);
                std::cout << "activate resident app status: " << status << std::endl;
                if (status > 0)
                {
                    response["message"] = "resident app launch failed";
                    ret = false;
                }
                else
                {
                    ret = true;
                }
            }

            if (parameters.HasLabel("visible"))
            {
                setVisibility("ResidentApp", parameters["visible"].Boolean());
            }

            if (!updatedUrl.empty())
            {
                WPEFramework::Core::JSON::String urlString;
                urlString = updatedUrl;
                status = JSONRPCDirectLink(mCurrentService, "ResidentApp").Set<WPEFramework::Core::JSON::String>(RDKSHELL_THUNDER_TIMEOUT, "url",urlString);
                std::cout << "set url status " << updatedUrl << " " << status << std::endl;
                if (status > 0)
                {
                    std::cout << "failed to set url to " << updatedUrl << " with status code " << status << std::endl;
                }
            }

            std::cout << "attempting to set factory mode flag \n";
            uint32_t setStatus = setValue(mCurrentService, "FactoryTest", "FactoryMode", "false");
            std::cout << "set status: " << setStatus << std::endl;
            sForceResidentAppLaunch = false;
            returnResponse(ret);
        }

        uint32_t RDKShell::toggleFactoryAppWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool ret = true;
            std::string callsign("factoryapp");
            bool isFactoryAppRunning = false;
            std::map<std::string, PluginData>::iterator pluginsEntry = gActivePluginsData.find(callsign);
            if (pluginsEntry != gActivePluginsData.end())
            {
                std::cout << "factory app is already running" << std::endl;
                isFactoryAppRunning = true;
            }
            if (isFactoryAppRunning)
            {
                sForceResidentAppLaunch = true;
                launchResidentAppWrapper(parameters, response);
                sForceResidentAppLaunch = false;
            }
            else
            {
                launchFactoryAppWrapper(parameters, response);
            }
            ret = response.HasLabel("success")?response["success"].Boolean():false;
            returnResponse(ret);
        }

        void RDKShell::addFactoryModeEasterEggs()
        {
            RdkShellEasterEggKeyDetails key1(56, 0, 0);
            RdkShellEasterEggKeyDetails key2(48, 0, 0);
            RdkShellEasterEggKeyDetails key3(56, 0, 0);
            RdkShellEasterEggKeyDetails key4(55, 0, 0);
            std::vector<RdkShellEasterEggKeyDetails> keyDetails;
            keyDetails.push_back(key1);
            keyDetails.push_back(key2);
            keyDetails.push_back(key3);
            keyDetails.push_back(key4);
            addEasterEgg(keyDetails, "AGING_MODE_EXIT1", 10, "{\"invoke\":\"org.rdk.RDKShell.1.exitAgingMode\"}");

            keyDetails.clear();
            RdkShellEasterEggKeyDetails key5(38, 0, 0);
            RdkShellEasterEggKeyDetails key6(38, 0, 0);
            RdkShellEasterEggKeyDetails key7(40, 0, 0);
            RdkShellEasterEggKeyDetails key8(40, 0, 0);
            keyDetails.push_back(key5);
            keyDetails.push_back(key6);
            keyDetails.push_back(key7);
            keyDetails.push_back(key8);
            addEasterEgg(keyDetails, "AGING_MODE_EXIT2", 10, "{\"invoke\":\"org.rdk.RDKShell.1.exitAgingMode\"}");
        }

        void RDKShell::removeFactoryModeEasterEggs()
        {
            removeEasterEgg("AGING_MODE_EXIT1");
            removeEasterEgg("AGING_MODE_EXIT2");
        }

        uint32_t RDKShell::exitAgingModeWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool ret = true;
            std::string valueString;

            std::cout << "attempting to check aging state flag \n";
            uint32_t status = getValue(mCurrentService, "FactoryTest", "AgingState", valueString);
            std::cout << "get status: " << status << std::endl;

            if (status > 0)
            {
                response["message"] = " unable to check aging flag";
                returnResponse(false);
            }

            if (valueString != "true")
            {
                std::cout << "aging value is " << valueString << std::endl;
                response["message"] = " aging value is not true";
                returnResponse(false);
            }

            std::cout << "attempting to set check aging state flag to false\n";
            status = setValue(mCurrentService, "FactoryTest", "AgingState", "false");
            std::cout << "set status: " << status << std::endl;

            JsonObject request, res;
            launchResidentAppWrapper(request, res);

            returnResponse(ret);
        }

        uint32_t RDKShell::getVirtualResolutionWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKShell::enableKeyRepeatsWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKShell::getKeyRepeatsEnabledWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKShell::setVirtualResolutionWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKShell::enableVirtualDisplayWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKShell::getVirtualDisplayEnabledWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKShell::getLastWakeupKeyWrapper(const JsonObject& parameters, JsonObject& response)
        {
             LOGINFOMETHOD();
             std::string serviceCallsign = SYSTEM_SERVICE_CALLSIGN;
             serviceCallsign.append(".2");
             auto systemServiceConnection = RDKShell::getThunderControllerClient(serviceCallsign);
             JsonObject request, result;
             uint32_t status = systemServiceConnection->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, "getWakeupReason", request, result);
             if (Core::ERROR_NONE == status && result.HasLabel("wakeupReason"))
             {
                std::string wakeupreason = result["wakeupReason"].String();
                if(wakeupreason.compare("WAKEUP_REASON_IR") == 0)
                {
                     JsonObject req, res;
                     uint32_t status = systemServiceConnection->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, "getLastWakeupKeyCode", req, res);
                     if (Core::ERROR_NONE == status && res.HasLabel("wakeupKeyCode"))
                     {
                         unsigned int key = res["wakeupKeyCode"].Number();
                         unsigned long flags = 0;
                         uint32_t mappedKeyCode = key, mappedFlags = 0;
                         keyCodeFromWayland(key, flags, mappedKeyCode, mappedFlags);
                         response["keyCode"] = JsonValue(mappedKeyCode);
                         response["modifiers"] = JsonValue(mappedFlags);
                         std::cout << "Got LastWakeupKey, keyCode: " << mappedKeyCode << " modifiers: " << mappedFlags << std::endl;
                         returnResponse(true);
                      }
                  }
                  else if (wakeupreason.compare("WAKEUP_REASON_RCU_BT") == 0)
                  {
			std::string remoteControlCallsign = REMOTECONTROL_CALLSIGN;
	                auto remoteControlConnection = RDKShell::getThunderControllerClient(remoteControlCallsign);
			int16_t keyCode = KEYCODE_INVALID, retry = 12;

			while( keyCode == KEYCODE_INVALID )
			{
				JsonObject req, res, stat;
				req.Set("netType",1);

				uint32_t status = remoteControlConnection->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, "getNetStatus", req, res);
				if (Core::ERROR_NONE == status && res.HasLabel("status"))
				{
					stat = res["status"].Object();

					if(stat.HasLabel("remoteData"))
					{
					   JsonArray remoteArray = stat["remoteData"].Array();
					   for (int k = 0; k < remoteArray.Length(); k++)
					   {
						JsonObject remote = remoteArray[k].Object();
						if (remote.HasLabel("wakeupKeyCode"))
						{
							keyCode = remote["wakeupKeyCode"].Number();
							std::cout << "wakeupKeyCode-keyCode: " << keyCode << std::endl;
						}
						else
							std::cout << "wakeupKeyCode missing in remoteInfo\n" << std::endl;
					   }
					 }
					 else
						std::cout << "remoteData missing in status\n" << std::endl;
				}
				else
					std::cout << "getNetStatus failed\n" << std::endl;

				retry--;
				if ( (retry == 0) || (keyCode != KEYCODE_INVALID) )
				break;
				usleep(RETRY_INTERVAL_250MS);
			   }

			   if ( keyCode != KEYCODE_INVALID )
			   {
				unsigned long flags = 0;
				uint32_t mappedKeyCode = keyCode, mappedFlags = 0;
				keyCodeFromWayland(keyCode, flags, mappedKeyCode, mappedFlags);
				response["keyCode"] = JsonValue(mappedKeyCode);
				response["modifiers"] = JsonValue(mappedFlags);
				std::cout << "Got LastWakeupKey, keyCode: " << mappedKeyCode << " modifiers: " << mappedFlags << std::endl;
				returnResponse(true);
			   }
		    }
		    else
			std::cout << "wakeup reason is not IR/BT RCU\n" << std::endl;
                }
		else
			std::cout << "wakeup reason not available\n" << std::endl;

             response["message"] = "unable to get wakeup key";
             returnResponse(false);
        }

	uint32_t RDKShell::hideAllClientsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            if (!parameters.HasLabel("hide"))
            {
                response["message"] = "please specify hide parameter";
                returnResponse(false);
            }
            bool hide = parameters["hide"].Boolean();
            lockRdkShellMutex();
            std::vector<std::string> clientList;
            CompositorController::getClients(clientList);
            for (size_t i=0; i<clientList.size(); i++)
            {
                CompositorController::setVisibility(clientList[i], !hide);
            }
            gRdkShellMutex.unlock();
            returnResponse(true);
        }

        uint32_t RDKShell::getScreenshotWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            lockRdkShellMutex();
            needsScreenshot = true;
            gRdkShellMutex.unlock();
            returnResponse(result);
        }

        uint32_t RDKShell::enableLogsFlushingWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKShell::getLogsFlushingEnabledWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool enabled = false;
            getLogsFlushingEnabled(enabled);
            response["enabled"] = enabled;

            returnResponse(true);
        }

        uint32_t RDKShell::showCursorWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = showCursor();
            returnResponse(result);
        }

        uint32_t RDKShell::hideCursorWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = hideCursor();
            returnResponse(result);
        }

        uint32_t RDKShell::setCursorSizeWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKShell::getCursorSizeWrapper(const JsonObject& parameters, JsonObject& response)
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
	
        uint32_t RDKShell::ignoreKeyInputsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            if (!parameters.HasLabel("ignore"))
            {
                response["message"] = "please specify ignore parameter";
                returnResponse(false);
            }
            bool ignoreKeyValue = parameters["ignore"].Boolean();
            lockRdkShellMutex();
            bool ret = CompositorController::ignoreKeyInputs(ignoreKeyValue);
            gRdkShellMutex.unlock();
            if (!ret)
            {
                response["message"] = "key ignore is not allowed";
            }
            returnResponse(ret);
        }

        uint32_t RDKShell::enableInputEventsWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKShell::keyRepeatConfigWrapper(const JsonObject& parameters, JsonObject& response)
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

            gRdkShellMutex.lock();
            CompositorController::setKeyRepeatConfig(enabled, initialDelay, repeatInterval);
            gRdkShellMutex.unlock();
            returnResponse(true);
        }

	uint32_t RDKShell::getGraphicsFrameRateWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            lockRdkShellMutex();
            unsigned int value = gCurrentFramerate;
            gRdkShellMutex.unlock();
            response["framerate"] = value;
            returnResponse(true);
        }

        uint32_t RDKShell::setGraphicsFrameRateWrapper(const JsonObject& parameters, JsonObject& response)
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
                lockRdkShellMutex();
                gCurrentFramerate = framerate;
                gRdkShellMutex.unlock();
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getBlockedAVApplicationsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool status = true;

            if (true == mErmEnabled)
            {
                JsonArray appsList = JsonArray();
                status = getBlockedAVApplications(appsList);
                if (true == status)
                {
                    response["getBlockedAVApplications"]=appsList;
                }
            }
            else
            {
                response["message"] = "ERM not enabled";
            }
            returnResponse(status);
        }

        uint32_t RDKShell::setAVBlockedWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool status = true;

            if (true == mErmEnabled)
            {
                const JsonArray apps = parameters.HasLabel("applications") ? parameters["applications"].Array() : JsonArray();
                JsonArray result;
                for (int i=0; i< apps.Length(); i++) {
                    const JsonObject& appInfo = apps[i].Object();
                    if (appInfo.HasLabel("callsign") && appInfo.HasLabel("callsign"))
                    {
                        std::string app = appInfo["callsign"].String();
                        bool blockAV    = appInfo["blocked"].Boolean();
                        cout<<"callsign : "<< app << std::endl;
                        cout<<"blocked  : "<<std::boolalpha << blockAV << std::endl;

                        status = (status && setAVBlocked(app, blockAV));
                        cout<< "EssRMgrAddToBlackList returned : "<<std::boolalpha <<status<< std::endl;
                    }
                    else
                    {
                        std::string jsonstr;
                        appInfo.ToString(jsonstr);
                        cout<<"ERROR: callsign and callsign status required in "<< jsonstr << std::endl;
                    }
                }
            }
            else
            {
                response["message"] = "ERM not enabled";
            }
            returnResponse(status);
        }

#ifdef HIBERNATE_SUPPORT_ENABLED
        uint32_t RDKShell::checkpointWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool status = false;
            if (parameters.HasLabel("callsign"))
            {
                std::string callsign = parameters["callsign"].String();
                bool isApplicationBeingDestroyed = false;

                gLaunchDestroyMutex.lock();
                if (gDestroyApplications.find(callsign) != gDestroyApplications.end())
                {
                    isApplicationBeingDestroyed = true;
                }
                if (gExternalDestroyApplications.find(callsign) != gExternalDestroyApplications.end())
                {
                    isApplicationBeingDestroyed = true;
                }
                gLaunchDestroyMutex.unlock();

                if (isApplicationBeingDestroyed)
                {
                    std::cout << "ignoring checkpoint for " << callsign << " as it is being destroyed " << std::endl;
                    status = false;
                    response["message"] = "failed to checkpoint application, is being destroyed";
                    returnResponse(status);
                }

                if( callsign.find("Netflix") != string::npos || callsign.find("Cobalt") != string::npos )
                {
                    //Check if native app is suspended
                    WPEFramework::Core::JSON::String stateString;
                    const string callsignWithVersion = callsign + ".1";
                    auto thunderPlugin = getThunderControllerClient(callsignWithVersion);
                    uint32_t stateStatus = 0;
                    stateStatus = thunderPlugin->Get<WPEFramework::Core::JSON::String>(RDKSHELL_THUNDER_TIMEOUT, "state", stateString);
                    if(stateStatus || stateString != "suspended")
                    {
                        std::cout << "ignoring checkpoint for " << callsign << " as it is not suspended " << std::endl;
                        status = false;
                        response["message"] = "failed to checkpoint native application, not suspended";
                        returnResponse(status);
                    }
                }

                std::thread requestsThread =
                std::thread([=]()
                {
                    auto thunderController = RDKShell::getThunderControllerClient();
                    JsonObject request, result, eventMsg;
                    request["callsign"] = callsign;
                    request["timeout"] = RDKSHELL_THUNDER_TIMEOUT;
                    if(parameters.HasLabel("timeout"))
                    {
                        request["timeout"] = parameters["timeout"];
                    }
                    if(parameters.HasLabel("procsequence"))
                    {
                        request["procsequence"] = parameters["procsequence"];
                    }
                    uint32_t errCode = thunderController->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, "hibernate", request, result);
                    if(errCode > 0)
                    {
                        eventMsg["success"] = false;
                        eventMsg["message"] = result;
                    }
                    else
                    {
                        eventMsg["success"] = true;
                    }
                    notify(RDKShell::RDKSHELL_EVENT_ON_CHECKPOINTED, eventMsg);
                });
                requestsThread.detach();
                status = true;
            }

            returnResponse(status);
        }

        uint32_t RDKShell::restoreWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool status = false;
            if (parameters.HasLabel("callsign"))
            {
                std::string callsign = parameters["callsign"].String();
                std::thread requestsThread =
                std::thread([=]()
                {
                    auto thunderController = RDKShell::getThunderControllerClient();
                    JsonObject request, result, eventMsg;
                    request["callsign"] = callsign;

                    uint32_t errCode = thunderController->Invoke<JsonObject, JsonObject>(RDKSHELL_THUNDER_TIMEOUT, "activate", request, result);
                    if(errCode > 0)
                    {
                        eventMsg["success"] = false;
                        eventMsg["message"] = result;
                    }
                    else
                    {
                        eventMsg["success"] = true;
                    }
                    notify(RDKShell::RDKSHELL_EVENT_ON_RESTORED, eventMsg);
                });
                requestsThread.detach();
                status = true;
            }

            returnResponse(status);
        }
#endif

        // Registered methods end

        // Events begin
        void RDKShell::notify(const std::string& event, const JsonObject& parameters)
        {
            sendNotify(event.c_str(), parameters);
        }
        // Events end

        bool RDKShell::checkForBootupFactoryAppLaunch()
        {
            std::cout << "inside of checkForBootupFactoryAppLaunch\n";
#ifdef RFC_ENABLED
            #ifdef RDKSHELL_READ_MAC_ON_STARTUP
            char* mac = new char[19];
            tFHError retAPIStatus;
            retAPIStatus = getEthernetMAC(mac);
            if(retAPIStatus == E_OK)
            {
                if (strncasecmp(mac,"00:00:00:00:00:00",17) == 0)
                {
                    std::cout << "launching factory app as mac is matching... " << std::endl;
                    return true;
                }
                else
                {
                    std::cout << "mac match failed... mac from hal - " << mac << std::endl;
                }
            }
            else
            {
                std::cout << "reading stb mac via hal failed " << std::endl;
            }
            #else
            RFC_ParamData_t param;
            bool ret = Utils::getRFCConfig("Device.DeviceInfo.X_COMCAST-COM_STB_MAC", param);
            if (true == ret)
            {
                if (strncasecmp(param.value,"00:00:00:00:00:00",17) == 0)
                {
                    std::cout << "launching factory app as mac is matching " << std::endl;
                    return true;
                }
                else
                {
                    std::cout << "mac match failed. mac from rfc - " << param.value << std::endl;
                }
            }
            else
            {
                std::cout << "reading stb mac rfc failed " << std::endl;
            }
            #endif //RDKSHELL_READ_MAC_ON_STARTUP
#else
            std::cout << "rfc is disabled and unable to check for stb mac " << std::endl;
#endif

            if (sPersistentStoreFirstActivated)
            {
                std::string valueString;

                std::cout << "attempting to check aging state \n";
                uint32_t status = getValue(mCurrentService, "FactoryTest", "AgingState", valueString);
                std::cout << "get status for aging state: " << status << std::endl;

                if ((status == 0))
                {
                  std::cout << "aging result: " << valueString << std::endl;
                  if (valueString == "true")
                  {
                    std::cout << "launching factory app as aging state is set " << std::endl;
                    return true;
                  }
                }

                std::cout << "attempting to check factory mode \n";
                status = getValue(mCurrentService, "FactoryTest", "FactoryMode", valueString);
                std::cout << "get status for factory mode: " << status << std::endl;

                if ((status == 0))
                {
                  std::cout << "factory mode " << valueString << std::endl;
                  if (valueString == "true")
                  {
                    std::cout << "launching factory app as factory mode is set " << std::endl;
                    return true;
                  }
                }
            }
            else
            {
              std::cout << "persistent store not loaded during factory app check\n";
            }
            return false;
        }

        void RDKShell::killAllApps(bool enableDestroyEvent)
        {
            JsonObject stateRequest, stateResponse;
            getState(stateRequest, stateResponse);
            const JsonArray stateList = stateResponse.HasLabel("state")?stateResponse["state"].Array():JsonArray();

            if (enableDestroyEvent)
            {
                for (int i=0; i<stateList.Length(); i++)
                {
                    const JsonObject& stateInfo = stateList[i].Object();
                    if (stateInfo.HasLabel("callsign"))
                    {
                        const string callsign = stateInfo["callsign"].String();
                        std::cout << "RDKShell sending onWillDestroyEvent for " << callsign << std::endl;
                        JsonObject params;
                        params["callsign"] = callsign;
                        notify(RDKSHELL_EVENT_ON_WILL_DESTROY, params);
                    }
                }
                sleep(1);
            }

            for (int i=0; i<stateList.Length(); i++)
            {
                const JsonObject& stateInfo = stateList[i].Object();
                if (stateInfo.HasLabel("callsign"))
                {
                   JsonObject destroyRequest, destroyResponse;
                   destroyRequest["callsign"] = stateInfo["callsign"].String();
                   destroyWrapper(destroyRequest, destroyResponse);
                }
            }
        }

        bool RDKShell::setAVBlocked(const string callsign, bool blockAV)
        {
            bool status = true;

            gRdkShellMutex.lock();
            std::map<std::string, std::string>::iterator displayNameItr = gPluginDisplayNameMap.find(callsign);
            if (displayNameItr != gPluginDisplayNameMap.end())
            {
                std::string clientId(callsign + ',' + displayNameItr->second);
                std::cout << "setAVBlocked callsign: " << callsign << " clientIdentifier:<"<<clientId<<">blockAV:"<<std::boolalpha << blockAV << std::endl;
                status = CompositorController::setAVBlocked(clientId, blockAV);
            }
            else
            {
                status = false;
                std::cout << "display not found for " << callsign << std::endl;
            }
            gRdkShellMutex.unlock();
            if (false == status)
            {
                std::cout << "setAVBlocked failed for " << callsign << std::endl;
            }

            return status;
        }

        bool RDKShell::getBlockedAVApplications(JsonArray& appsList)
        {
            bool status = true;

            std::vector<std::string> apps;
            gRdkShellMutex.lock();
            status = CompositorController::getBlockedAVApplications(apps);
            gRdkShellMutex.unlock();
            if (true == status)
            {
                std::string appCallSign;
                for (std::vector<std::string>::iterator appsItr = apps.begin(); appsItr != apps.end(); appsItr++)
                {
                    appCallSign = *appsItr;
                    std::string::size_type pos = appCallSign.find(',');
                    if (pos != std::string::npos)
                    {
                        appsList.Add(appCallSign.substr(0, pos));
                    }
                }
            }

            return status;
        }

        // Internal methods begin

        bool RDKShell::moveToFront(const string& client)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::moveToFront(client);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::moveToBack(const string& client)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::moveToBack(client);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::moveBehind(const string& client, const string& target)
        {
            bool ret = false;
            lockRdkShellMutex();
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
            bool isApplicationBeingDestroyed = false;
            gLaunchDestroyMutex.lock();
            if (gDestroyApplications.find(client) != gDestroyApplications.end())
            {
                isApplicationBeingDestroyed = true;
            }
            gLaunchDestroyMutex.unlock();
            if (isApplicationBeingDestroyed)
            {
                std::cout << "ignoring setFocus for " << client << " as it is being destroyed " << std::endl;
                return false;
            }
            std::string previousFocusedClient;
            lockRdkShellMutex();
            CompositorController::getFocused(previousFocusedClient);
            ret = CompositorController::setFocus(client);
            gRdkShellMutex.unlock();
            std::string clientLower = toLower(client);

            if (previousFocusedClient != clientLower)
            {
                std::map<std::string, PluginData> activePluginsData;
                gPluginDataMutex.lock();
                activePluginsData = gActivePluginsData;
                gPluginDataMutex.unlock();

                if (!previousFocusedClient.empty())
                {
                    std::map<std::string, PluginData>::iterator previousFocusedIterator;

                    for (previousFocusedIterator = activePluginsData.begin(); previousFocusedIterator != activePluginsData.end(); previousFocusedIterator++)
                    {
                        std::string compositorName = toLower(previousFocusedIterator->first);
                        if (compositorName == previousFocusedClient)
                        {
                            std::cout << "setting the focus of " << compositorName << " to false " << std::endl;
                            Exchange::IFocus *focusedCallsign = mCurrentService->QueryInterfaceByCallsign<Exchange::IFocus>(previousFocusedIterator->first);
                            if (focusedCallsign != NULL)
                            {
                                uint32_t status = focusedCallsign->Focused(false);
                                std::cout << "result of set focus to false: " << status << std::endl;
                                focusedCallsign->Release();
                            }
                            break;
                        }
                    }
                }

                std::map<std::string, PluginData>::iterator focusedEntry = activePluginsData.find(client);
                if (focusedEntry != activePluginsData.end())
                {
                    std::cout << "setting the focus of " << client << " to true " << std::endl;
                    Exchange::IFocus *focusedCallsign = mCurrentService->QueryInterfaceByCallsign<Exchange::IFocus>(client);
                    if (focusedCallsign != NULL)
                    {
                        uint32_t status = focusedCallsign->Focused(true);
                        focusedCallsign->Release();
                        std::cout << "result of set focus to true: " << status << std::endl;
                    }
                }
            }
            return ret;
        }

        bool RDKShell::kill(const string& client)
        {
            bool ret = false;
            lockRdkShellMutex();
            RdkShell::CompositorController::removeListener(client, mEventListener);
            std::shared_ptr<KillClientRequest> request = std::make_shared<KillClientRequest>(client);
            gKillClientRequests.push_back(request);
            gPluginDisplayNameMap.erase(client);
            std::cout << "removed displayname : "<<client<< std::endl;
            gRdkShellMutex.unlock();
            sem_wait(&request->mSemaphore);
            ret = request->mResult;
            return ret;
        }

        bool RDKShell::addKeyIntercepts(const JsonArray& intercepts)
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
            gRdkShellMutex.unlock();
            return result;
        }

        bool RDKShell::removeKeyListeners(const string& client, const JsonArray& keys)
        {
            gRdkShellMutex.lock();

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
            gRdkShellMutex.unlock();
            return result;
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
                lockRdkShellMutex();
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
                gRdkShellMutex.unlock();
            }
            return ret;
        }

        bool RDKShell::getScreenResolution(JsonObject& out)
        {
            unsigned int width=0,height=0;
            bool ret = false;
            lockRdkShellMutex();
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
            lockRdkShellMutex();
            receivedResolutionRequest = true;
            resolutionWidth = w;
            resolutionHeight = h;
            gRdkShellMutex.unlock();
            return true;
        }

        bool RDKShell::setMimeType(const string& client, const string& mimeType)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::setMimeType(client, mimeType);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getMimeType(const string& client, string& mimeType)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::getMimeType(client, mimeType);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::createDisplay(const string& client, const string& displayName, const uint32_t displayWidth, const uint32_t displayHeight,
            const bool virtualDisplay, const uint32_t virtualWidth, const uint32_t virtualHeight, const bool topmost, const bool focus)
        {
            bool ret = false;
            if (!isClientExists(client))
            {
                lockRdkShellMutex();
                std::shared_ptr<CreateDisplayRequest> request = std::make_shared<CreateDisplayRequest>(client, displayName, displayWidth, displayHeight, virtualDisplay, virtualWidth, virtualHeight);
                gCreateDisplayRequests.push_back(request);
                gRdkShellMutex.unlock();
                sem_wait(&request->mSemaphore);
                ret = request->mResult;
            }
            else
            {
                std::cout << "Client " << client  << "already exist " << std::endl;
            }
            lockRdkShellMutex();
            RdkShell::CompositorController::addListener(client, mEventListener);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getClients(JsonArray& clients)
        {
            std::vector<std::string> clientList;
            lockRdkShellMutex();
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
            lockRdkShellMutex();
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
            lockRdkShellMutex();
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
            lockRdkShellMutex();
            std::cout << "setting the bounds\n";
            ret = CompositorController::setBounds(client, 0, 0, 1, 1); //forcing a compositor resize flush
            ret = CompositorController::setBounds(client, x, y, w, h);
            gRdkShellMutex.unlock();
            std::cout << "bounds set\n";
            usleep(68000);
            std::cout << "all set\n";
            return ret;
        }

        bool RDKShell::getVisibility(const string& client, bool& visible)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::getVisibility(client, visible);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setVisibility(const string& client, const bool visible)
        {
            bool ret = false;
            {
                bool lockAcquired = false;
                double startTime = RdkShell::milliseconds();
                while (!lockAcquired && (RdkShell::milliseconds() - startTime) < RDKSHELL_TRY_LOCK_WAIT_TIME_IN_MS)
                {
                    lockAcquired = gRdkShellMutex.try_lock();
                }
                if (!lockAcquired)
                {
                    std::cout << "unable to get lock for visibility, defaulting to normal lock\n";
                    gRdkShellMutex.lock();
                }
                else
                {
                    std::cout << "lock was acquired via try for visibility\n";
                }
            }
            ret = CompositorController::setVisibility(client, visible);
            gRdkShellMutex.unlock();
            
            bool isApplicationBeingDestroyed = false;
            gLaunchDestroyMutex.lock();
            if (gDestroyApplications.find(client) != gDestroyApplications.end())
            {
                isApplicationBeingDestroyed = true;
            }
            if (gExternalDestroyApplications.find(client) != gExternalDestroyApplications.end())
            {
                 isApplicationBeingDestroyed = true;
            }
            gLaunchDestroyMutex.unlock();
            if (isApplicationBeingDestroyed)
            {
                std::cout << "ignoring setvisibility for " << client << " as it is being destroyed " << std::endl;
                return false;
            }
            std::map<std::string, PluginData> activePluginsData;
            gPluginDataMutex.lock();
            activePluginsData = gActivePluginsData;
            gPluginDataMutex.unlock();
            std::map<std::string, PluginData>::iterator pluginsEntry = activePluginsData.find(client);
            if (pluginsEntry != activePluginsData.end())
            {
                PluginData& pluginData = pluginsEntry->second;
                if (pluginData.mClassName.compare("WebKitBrowser") == 0)
                {
                    std::cout << "setting the visiblity of " << client << " to " << visible << std::endl;
                    uint32_t status = 0;
                    gLaunchDestroyMutex.lock();
                    if (gDestroyApplications.find(client) != gDestroyApplications.end())
                    {
                        isApplicationBeingDestroyed = true;
                    }
                    gLaunchDestroyMutex.unlock();
					if (isApplicationBeingDestroyed)
                    {
                        std::cout << "ignoring setvisibility for " << client << " as it is being destroyed " << std::endl;
						return false;
                    }
                    gDestroyMutex.lock();
                    Exchange::IWebBrowser *browser = mCurrentService->QueryInterfaceByCallsign<Exchange::IWebBrowser>(client);
                    if (browser != NULL)
                    {
                        status = browser->Visibility(visible ? Exchange::IWebBrowser::VisibilityType::VISIBLE : Exchange::IWebBrowser::VisibilityType::HIDDEN);
                        browser->Release();
                    }
                    else
                    {
                        status = 1;
                    }
                    gDestroyMutex.unlock();
                    if (status > 0)
                    {
                        std::cout << "failed to set visibility property to browser " << client << " with status code " << status << std::endl;
                    }
                }
            }

            return ret;
        }

        bool RDKShell::getOpacity(const string& client, unsigned int& opacity)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::getOpacity(client, opacity);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setOpacity(const string& client, const unsigned int opacity)
        {
            bool ret = false;
            lockRdkShellMutex();
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
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getScale(const string& client, double& scaleX, double& scaleY)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::getScale(client, scaleX, scaleY);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setScale(const string& client, const double scaleX, const double scaleY)
        {
            bool ret = false;
            lockRdkShellMutex();
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
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getHolePunch(const string& client, bool& holePunch)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::getHolePunch(client, holePunch);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setHolePunch(const string& client, const bool holePunch)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::setHolePunch(client, holePunch);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::removeAnimation(const string& client)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::removeAnimation(client);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::addAnimationList(const JsonArray& animations)
        {
            lockRdkShellMutex();
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
            lockRdkShellMutex();
            CompositorController::enableInactivityReporting(enable);
            gRdkShellMutex.unlock();
            return true;
        }

        bool RDKShell::setInactivityInterval(const uint32_t interval)
        {
            lockRdkShellMutex();
            try
            {
              CompositorController::setInactivityInterval((double)interval);
            }
            catch (...) 
            {
              std::cout << "RDKShell unable to set inactivity interval  " << std::endl;
            }
            gRdkShellMutex.unlock();
            return true;
        }

        bool RDKShell::resetInactivityTime()
        {
            lockRdkShellMutex();
            try
            {
              CompositorController::resetInactivityTime();
              std::cout << "RDKShell inactivity time reset" << std::endl;
            }
            catch (...)
            {
              std::cout << "RDKShell unable to reset inactivity time  " << std::endl;
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

        bool RDKShell::systemMemory(uint32_t &freeKb, uint32_t & totalKb, uint32_t & availableKb, uint32_t & usedSwapKb)
        {
            lockRdkShellMutex();
            bool ret = RdkShell::systemRam(freeKb, totalKb, availableKb, usedSwapKb);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::pluginMemoryUsage(const string callsign, JsonArray& memoryInfo)
        {
            JsonObject memoryDetails;
            Exchange::IMemory* pluginMemoryInterface(mCurrentService->QueryInterfaceByCallsign<Exchange::IMemory>(callsign.c_str()));
            memoryDetails["callsign"] = callsign;
            memoryDetails["ram"] = -1;
            if (nullptr != pluginMemoryInterface)
            {
                memoryDetails["ram"] = pluginMemoryInterface->Resident()/1024;
            }
            else
            {
                std::cout << "Memory information not available for " << callsign << std::endl;
            }
            memoryInfo.Add(memoryDetails);
            return true;
        }


        bool RDKShell::getKeyRepeatsEnabled(bool& enable)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::getKeyRepeatsEnabled(enable);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::enableKeyRepeats(const bool enable)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::enableKeyRepeats(enable);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setTopmost(const string& callsign, const bool topmost, const bool focus)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::setTopmost(callsign, topmost, focus);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getVirtualResolution(const std::string& client, uint32_t &virtualWidth, uint32_t &virtualHeight)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::getVirtualResolution(client, virtualWidth, virtualHeight);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setVirtualResolution(const std::string& client, const uint32_t virtualWidth, const uint32_t virtualHeight)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::setVirtualResolution(client, virtualWidth, virtualHeight);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::enableVirtualDisplay(const std::string& client, const bool enable)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::enableVirtualDisplay(client, enable);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getVirtualDisplayEnabled(const std::string& client, bool &enabled)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::getVirtualDisplayEnabled(client, enabled);
            gRdkShellMutex.unlock();
            return ret;
        }

        PluginStateChangeData::PluginStateChangeData(std::string callsign, std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> pluginConnection, RDKShell* rdkshell):mCallSign(callsign), mRDKShell(*rdkshell)
        {
            mPluginConnection = pluginConnection;
            mLaunchEnabled = false;
        }

        PluginStateChangeData::~PluginStateChangeData()
        {
            mPluginConnection = nullptr;
        }

        void PluginStateChangeData::enableLaunch(bool enable)
        {
            mLaunchEnabled = enable;
        }

        void PluginStateChangeData::resetConnection()
        {
            mPluginConnection = nullptr;
        }

        void PluginStateChangeData::onStateChangeEvent(const JsonObject& params)
        {
            bool isSuspended = params["suspended"].Boolean();
            if (mLaunchEnabled)
            {
               JsonObject params;
               params["client"] = mCallSign;
               params["launchType"] = (isSuspended)?"suspend":"resume";
               mRDKShell.notify(RDKShell::RDKSHELL_EVENT_ON_LAUNCHED, params);
               mLaunchEnabled = false;
            }

            if (isSuspended)
            {
                JsonObject params;
                params["client"] = mCallSign;
                mRDKShell.notify(RDKShell::RDKSHELL_EVENT_ON_PLUGIN_SUSPENDED, params);
            }
        }

        bool RDKShell::showWatermark(const bool enable)
        {
            bool ret = true;
            lockRdkShellMutex();
            if (enable)
            {
                receivedShowWatermarkRequest = true;
            }
            else
            {
                ret = CompositorController::hideWatermark();
            }
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::showFullScreenImage(std::string& path)
        {
            bool ret = true;
            lockRdkShellMutex();
            fullScreenImagePath = path;
            receivedFullScreenImageRequest = true;
            gRdkShellMutex.unlock();
            return ret;
        }

        void RDKShell::onTimer()
        {
            if (gSystemServiceEventsSubscribed)
            {
                if (m_timer.isActive()) {
                    m_timer.stop();
                    std::cout << "Stopped SystemServices connection timer" << std::endl;
                }
            }
            else
            {
                if (Core::ERROR_NONE == subscribeForSystemEvent("onSystemPowerStateChanged"))
                {  
                    m_timer.stop();
                    std::cout << "Stopped SystemServices connection timer after subscription" << std::endl;
                }
            }
        }

        int32_t RDKShell::subscribeForSystemEvent(std::string event)
        {
            int32_t status = Core::ERROR_GENERAL;

            PluginHost::IShell::state state;
            if ((getServiceState(mCurrentService, SYSTEM_SERVICE_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED))
            {
                std::cout << "SystemService is already activated" << std::endl;

                if (nullptr == gSystemServiceConnection)
                {  
                    std::string serviceCallsign = SYSTEM_SERVICE_CALLSIGN;
                    serviceCallsign.append(".2");
                    gSystemServiceConnection = RDKShell::getThunderControllerClient(serviceCallsign);
                }
            }

            if (nullptr != gSystemServiceConnection)
            {
                if (!gSystemServiceEventsSubscribed)
                {
                    std::string eventName("onSystemPowerStateChanged");
                    status = gSystemServiceConnection->Subscribe<JsonObject>(RDKSHELL_THUNDER_TIMEOUT, _T(eventName), &RDKShell::pluginEventHandler, this);

                    if (Core::ERROR_NONE == status)
                    {
                        std::cout << "RDKShell subscribed to onSystemPowerStateChanged event " << std::endl;
                        gSystemServiceEventsSubscribed = true;
                    }
                    else
                    { 
                        std::cout << "Subscribe for SystemServices event failed with " << status << std::endl;
                        gSystemServiceConnection.reset();
                    }

                }
                else
                    std::cout << "Already Subscribed to SystemServices events" << std::endl;
            }
            else
                std::cout << "No Connection to SystemServices" << std::endl;

            return status;
        }
        
        void RDKShell::enableLogsFlushing(const bool enable)
        {
            gRdkShellMutex.lock();
            Logger::enableFlushing(enable);
            gRdkShellMutex.unlock();
        }

        void RDKShell::getLogsFlushingEnabled(bool &enabled)
        {
            gRdkShellMutex.lock();
            enabled = Logger::isFlushingEnabled();
            gRdkShellMutex.unlock();
        }

        bool RDKShell::showCursor()
        {
            gRdkShellMutex.lock();
            bool ret = CompositorController::showCursor();
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::hideCursor()
        {
            gRdkShellMutex.lock();
            bool ret = CompositorController::hideCursor();
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setCursorSize(uint32_t width, uint32_t height)
        {
            gRdkShellMutex.lock();
            bool ret = CompositorController::setCursorSize(width, height);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getCursorSize(uint32_t& width, uint32_t& height)
        {
            gRdkShellMutex.lock();
            bool ret = CompositorController::getCursorSize(width, height);
            gRdkShellMutex.unlock();
            return ret;
        }

        uint32_t RDKShell::addEasterEggsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("keySequence"))
            {
                result = false;
                response["message"] = "please specify keySequence";
            }
            if (!parameters.HasLabel("id"))
            {
                result = false;
                response["message"] = "please specify id";
            }
            if (!parameters.HasLabel("duration"))
            {
                result = false;
                response["message"] = "please specify duration";
            }
            if (!parameters.HasLabel("api"))
            {
                result = false;
                response["message"] = "please specify api";
            }
            if (!result)
            {
                returnResponse(result);
            }

            std::vector<RdkShellEasterEggKeyDetails> keyDetails;
            const JsonArray keyInputs = parameters["keySequence"].Array();
            for (int i=0; i<keyInputs.Length(); i++)
            {
                const JsonObject& keyInputInfo = keyInputs[i].Object();
                uint32_t keyCode, flags=0;
                std::string virtualKey("");
                if (keyInputInfo.HasLabel("keyCode") && keyInputInfo.HasLabel("hold"))
                {
                    keyCode = keyInputInfo["keyCode"].Number();
                    flags = keyInputInfo.HasLabel("modifiers") ? keyInputInfo["modifiers"].Number() : 0;
                    const uint32_t holdTime = keyInputInfo["hold"].Number();
                    keyDetails.push_back(RdkShellEasterEggKeyDetails(keyCode, flags, holdTime));
                }
                else
                {
                    continue;
                }
            }
            std::string id = parameters["id"].String();
            const uint32_t duration = parameters["duration"].Number();
            JsonObject api = parameters["api"].Object();
            std::string apiString("");
            if (!api.ToString(apiString))
            {
                response["message"] = "api is not in proper json format";
                returnResponse(false);
            }
            gRdkShellMutex.lock();
            addEasterEgg(keyDetails, id, duration, apiString);
            gRdkShellMutex.unlock();
            keyDetails.clear();
            returnResponse(result);
        }

        uint32_t RDKShell::removeEasterEggsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("id"))
            {
                result = false;
                response["message"] = "please specify id";
            }
            std::string id = parameters["id"].String();
            gRdkShellMutex.lock();
            removeEasterEgg(id);
            gRdkShellMutex.unlock();
            returnResponse(result);
        }

        uint32_t RDKShell::getEasterEggsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            JsonArray easterEggs;
            std::vector<RdkShellEasterEggDetails> easterEggsList;     
            gRdkShellMutex.lock();
            getEasterEggs(easterEggsList);
            gRdkShellMutex.unlock();
            for (size_t i=0; i<easterEggsList.size(); i++)
	    {
                RdkShellEasterEggDetails& easterEgg = easterEggsList[i];
                JsonObject easterEggResponse;
		easterEggResponse["id"] = easterEgg.id;
                JsonArray keySequenceDetails;
                for (size_t j=0; j<easterEgg.keySequence.size(); j++)
                {
                    JsonObject keyDetails;
                    RdkShellEasterEggKeyDetails& details = easterEgg.keySequence[j];
                    keyDetails["keyCode"] = details.keyCode;
                    keyDetails["hold"] = details.keyHoldTime;
                    keyDetails["modifiers"] = details.keyModifiers;
                    keySequenceDetails.Add(keyDetails);
                }
		easterEggResponse["keySequence"] = keySequenceDetails;
		easterEggResponse["duration"] = easterEgg.duration;
		easterEggResponse["api"] = JsonObject(easterEgg.api.c_str());
                easterEggs.Add(easterEggResponse);
            }
            response["easterEggs"] = easterEggs;
            returnResponse(result);
        }

        bool RDKShell::enableInputEvents(const JsonArray& clients, bool enable)
        {
            bool result = true;

            gRdkShellMutex.lock();
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
            gRdkShellMutex.unlock();

            return result;
        }
        // Internal methods end
    } // namespace Plugin
} // namespace WPEFramework
