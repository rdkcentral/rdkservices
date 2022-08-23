#include "SwitchBoard.h"
#include "UtilsJsonRpc.h"
#include "UtilsCStr.h"
#include "UtilsLogging.h"
#include "UtilsString.h"
#include "UtilsSecurityToken.h"
#include <iostream>
#define SERVER_DETAILS "127.0.0.1:9998"
#define SUBSCRIPTION_CALLSIGN "org.rdk.RDKShell"
#define SUBSCRIPTION_CALLSIGN_VER SUBSCRIPTION_CALLSIGN ".1"

#define SUBSCRIPTION_LOW_MEMORY_EVENT "onDeviceLowRamWarning"
#define SUBSCRIPTION_CRITICAL_MEMORY_EVENT "onDeviceCriticallyLowRamWarning"
#define SUBSCRIPTION_ONKEY_EVENT "onKeyEvent"
#define SUBSCRIPTION_ONLAUNCHED_EVENT "onLaunched"
#define SUBSCRIPTION_ONDESTROYED_EVENT "onDestroyed"

#define REVISION "1.0"
#define RECONNECTION_TIME_IN_MILLISECONDS 5500
#define THUNDER_TIMEOUT 2000

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0


namespace WPEFramework
{

    namespace {

        static Plugin::Metadata<Plugin::SwitchBoard> metadata(
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
    namespace Plugin
    {
        SERVICE_REGISTRATION(SwitchBoard, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        static std::string gThunderAccessValue = SERVER_DETAILS;
        static std::string sThunderSecurityToken;
        void SwitchBoard::onCriticalMemoryEvent(const JsonObject &parameters)
        {
            string message;
            parameters.ToString(message);
            LOGINFO(" [ %s]: %s Res app running ? %d ", __FUNCTION__, C_STR(message), m_isResAppRunning);

            if (parameters.HasLabel("ram") && m_isResAppRunning)
            {
                PluginHost::WorkerPool::Instance().Submit(Job::Create(this, OFFLOAD));
            }
        }
        void SwitchBoard::onLowMemoryEvent(const JsonObject &parameters)
        {
            string message;
            parameters.ToString(message);
            LOGINFO(" [ %s]: %s Res app running ? %d ", __FUNCTION__, C_STR(message), m_isResAppRunning);
            // Let us offload all unused applications.
            PluginHost::WorkerPool::Instance().Submit(Job::Create(this, KEEP_ACTIVE_APP));
        }

        void SwitchBoard::onKeyEvent(const JsonObject &parameters)
        {
            string message, clients;
            if (parameters.HasLabel("keycode"))
            {
                if (!parameters["keyDown"].Boolean() && parameters["keycode"].Number() == HOME_KEY)
                {
                    // Case 1. Is there an active app and not is not residentapp
                    bool isResActiveApp = Utils::String::stringContains(activeCallsign, "residentapp");
                    if (!activeCallsign.empty() && !isResActiveApp)
                    {
                        if (!m_isResAppRunning && !m_launchInitiated)
                        {
                            LOGINFO(" Hot key ... Launching resident app ");
                            PluginHost::WorkerPool::Instance().Submit(Job::Create(this, RESTORE_RES_APP));
                        }
                    }
                    else
                        LOGINFO(" [onKeyEvent] Case 1 is not applicable. Active app is  [%s] ", C_STR(activeCallsign));
                    // Case 2. There is no active app and resident app is not launched.
                    if (activeCallsign.empty())
                    {
                        PluginHost::WorkerPool::Instance().Submit(Job::Create(this, LAUNCH));
                        activeCallsign = "ResidentApp";
                    }
                    else
                        LOGINFO(" [onKeyEvent] Case 2 is not applicable");
                }
            }
        }
        void SwitchBoard::onLaunched(const JsonObject &parameters)
        {
            if (parameters.HasLabel("client"))
            {
                activeCallsign = parameters["client"].String();
                LOGINFO(" Launch notification  ...%s  ", C_STR(activeCallsign));

                if (Utils::String::stringContains(activeCallsign, "residentapp"))
                    updateState(true, false);
            }
        }
        void SwitchBoard::updateState(bool running, bool started)
        {
            m_callMutex.Lock();
            m_isResAppRunning = running;
            m_launchInitiated = started;
            m_callMutex.Unlock();
        }
        void SwitchBoard::onDestroyed(const JsonObject &parameters)
        {
            // Case 1.Focused app is not referenceapp.
            LOGINFO(" m_isResAppRunning =%d m_launchInitiated = %d ", m_isResAppRunning, m_launchInitiated);
            if (parameters.HasLabel("client"))
            {
                string destroyedApp = parameters["client"].String();
                if (!Utils::String::stringContains(destroyedApp, "residentapp") &&
                    !m_isResAppRunning && !m_launchInitiated)
                {
                    launchResidentApp();
                }
                else if (Utils::String::stringContains(destroyedApp, "residentapp"))
                {
                    updateState(false, false);
                    m_onHomeScreen = false;
                }
            }
        }

        void SwitchBoard::Dispatch(JOBTYPE jobType)
        {
            string message, clients;
            JsonObject req, res;
            uint32_t status;

            LOGINFO(" [Dispatch] Currently active [%s]", C_STR(activeCallsign));

            switch (jobType)
            {
            case RESTORE_RES_APP:
            {
                string currApp = activeCallsign;
                LOGINFO(" [Dispatch] Restoring ResidentApp..");
                m_onHomeScreen = true;
                launchResidentApp();
                LOGINFO(" [Dispatch] Offloading active app... ");
                offloadApplication(currApp);
            }
            break;
            case REMOVE_ACTIVE_APP:
                LOGINFO(" [Dispatch] Removing active app ");
                offloadApplication(activeCallsign);
                break;
            case LAUNCH:
                launchResidentApp();
                break;
            case OFFLOAD:
                if (m_onHomeScreen)
                {
                    offloadApplication("ResidentApp");
                    m_onHomeScreen = false;
                }
                else
                {
                    LOGINFO("Skipping residentUI offloading. : current active app is %s", C_STR(activeCallsign));
                }
                break;
            case KEEP_ACTIVE_APP:
            {
                JsonObject req, res;
                string clients;
                status = m_remoteObject->Invoke<JsonObject, JsonObject>(THUNDER_TIMEOUT, "getClients", req, res);
                if (Core::ERROR_NONE == status)
                    clients = res["clients"].String();

                // offlaod the apps from the excluison list
                for (string app : callsigns)
                {
                    LOGINFO(" [ %s]: Checking %s against %s", __FUNCTION__, C_STR(app), C_STR(activeCallsign));
                    if (!Utils::String::stringContains(activeCallsign, app) &&
                        Utils::String::stringContains(clients, app))

                    {
                        LOGINFO("Removing application %s", C_STR(app));
                        offloadApplication(app);
                    }
                }
            }
            }
        }
        void SwitchBoard::launchResidentApp()
        {
            JsonObject req, res;
            uint32_t status;
            string message;

            req["callsign"] = "ResidentApp";
            req["type"] = "ResidentApp";
            req["visible"] = true;
            req["focus"] = true;
            req["uri"] = m_homeURL;

            status = m_remoteObject->Invoke<JsonObject, JsonObject>(THUNDER_TIMEOUT, _T("launch"), req, res);
            res.ToString(message);
            updateState(true, true);
            LOGINFO(" Launched residentapp . status : %d,  msg %s ",
                    (status == Core::ERROR_NONE), C_STR(message));
        }
        SwitchBoard::SwitchBoard() : 
                                   m_subscribedToEvents(false),
                                   m_remoteObject(nullptr),
                                   m_isResAppRunning(false),
                                   m_launchInitiated(false),
                                   m_onHomeScreen(false),
                                   m_lowMem(100),
                                   m_criticalMem(50)
        {
            LOGINFO();
            m_timer.connect(std::bind(&SwitchBoard::onTimer, this));
        }
        SwitchBoard::~SwitchBoard()
        {
        }

        void SwitchBoard::offloadApplication(const string callsign)
        {
            JsonObject req, res;
            uint32_t status;

            req["callsign"] = callsign.c_str();
            status = m_remoteObject->Invoke<JsonObject, JsonObject>(THUNDER_TIMEOUT, _T("destroy"), req, res);
            LOGINFO("Offloading  %s  result : %s", C_STR(callsign), (Core::ERROR_NONE == status) ? "Success" : "Failure");
        }
        const string SwitchBoard::Initialize(PluginHost::IShell *service)
        {
            LOGINFO();

            LOGINFO(" Revision %s ", REVISION);
            m_timer.start(RECONNECTION_TIME_IN_MILLISECONDS);
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), _T(SERVER_DETAILS));
            m_remoteObject = new WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>(_T(SUBSCRIPTION_CALLSIGN_VER));

            configurations.FromString(service->ConfigLine());
            LOGINFO("Home URL is set ?[%d] , Low memory limit set ? [%d] , Critical memory limit set ? [%d]",
                    configurations.Homeurl.IsSet(),
                    configurations.Lowmem.IsSet(), configurations.Criticalmem.IsSet());

            m_homeURL = configurations.Homeurl.IsSet() ? configurations.Homeurl.Value() : m_homeURL;

            m_lowMem = configurations.Lowmem.IsSet() ? configurations.Lowmem.Value() : m_lowMem;

            m_criticalMem = configurations.Criticalmem.IsSet() ? configurations.Criticalmem.Value() : m_criticalMem;

            LOGINFO("Home URL is set to [%s] .", C_STR(m_homeURL));

            if (configurations.Callsigns.IsSet() && configurations.Callsigns.Length() > 1)
            {
                Core::JSON::ArrayType<Core::JSON::String>::Iterator index(configurations.Callsigns.Elements());
                while (index.Next())
                {
                    callsigns.push_back(index.Current().Value());
                }
                LOGINFO("Total callsign length is %d", configurations.Callsigns.Length());
            }
            else
            {
                LOGINFO("Callsigns is not found in configuration. Adding cobalt to the list.");
                callsigns.push_back("Cobalt");
            }

            return "";
        }

        void SwitchBoard::Deinitialize(PluginHost::IShell * /* service */)
        {

            LOGINFO();
            if (m_subscribedToEvents)
            {
                m_remoteObject->Unsubscribe(THUNDER_TIMEOUT, _T(SUBSCRIPTION_LOW_MEMORY_EVENT));
                m_remoteObject->Unsubscribe(THUNDER_TIMEOUT, _T(SUBSCRIPTION_CRITICAL_MEMORY_EVENT));
                m_remoteObject->Unsubscribe(THUNDER_TIMEOUT, _T(SUBSCRIPTION_ONKEY_EVENT));
                m_remoteObject->Unsubscribe(THUNDER_TIMEOUT, _T(SUBSCRIPTION_ONLAUNCHED_EVENT));
                m_remoteObject->Unsubscribe(THUNDER_TIMEOUT, _T(SUBSCRIPTION_ONDESTROYED_EVENT));
                m_subscribedToEvents = false;
                // delete m_remoteObject;
            }
            if (m_timer.isActive())
            {
                m_timer.stop();
            }
        }

        string SwitchBoard::Information() const
        {
            return (string("{\"service \": \"org.rdk.SwitchBoard\"}"));
        }
        void SwitchBoard::onTimer()
        {
            m_callMutex.Lock();
            if (!m_subscribedToEvents)
            {
                SubscribeToEvents();
            }
            if (m_subscribedToEvents)
            {
                setMemoryLimits();
                if (m_timer.isActive())
                {
                    m_timer.stop();
                    LOGINFO(" Timer stopped.");
                }
                LOGINFO("Subscription completed.");
            }
            m_callMutex.Unlock();
        }
        void SwitchBoard::SubscribeToEvents()
        {
            LOGINFO(" Attempting event subscription");

            if (Utils::isPluginActivated(SUBSCRIPTION_CALLSIGN))
            {
                uint32_t status = Core::ERROR_NONE;

                std::string serviceCallsign = "org.rdk.RDKShell.1";

                m_remoteObject->Subscribe<JsonObject>(THUNDER_TIMEOUT, _T(SUBSCRIPTION_LOW_MEMORY_EVENT), &SwitchBoard::onLowMemoryEvent, this);
                m_remoteObject->Subscribe<JsonObject>(THUNDER_TIMEOUT, _T(SUBSCRIPTION_CRITICAL_MEMORY_EVENT), &SwitchBoard::onCriticalMemoryEvent, this);
                m_remoteObject->Subscribe<JsonObject>(THUNDER_TIMEOUT, _T(SUBSCRIPTION_ONKEY_EVENT), &SwitchBoard::onKeyEvent, this);
                m_remoteObject->Subscribe<JsonObject>(THUNDER_TIMEOUT, _T(SUBSCRIPTION_ONLAUNCHED_EVENT), &SwitchBoard::onLaunched, this);
                m_remoteObject->Subscribe<JsonObject>(THUNDER_TIMEOUT, _T(SUBSCRIPTION_ONDESTROYED_EVENT), &SwitchBoard::onDestroyed, this);

                m_subscribedToEvents = true;

                JsonObject req, res;
                status = m_remoteObject->Invoke<JsonObject, JsonObject>(THUNDER_TIMEOUT, "getClients", req, res);
                if (Core::ERROR_NONE == status)
                {
                    string clients = res["clients"].String();
                    m_onHomeScreen = m_isResAppRunning = (clients.find("residentapp") != std::string::npos);
                }
                else
                {
                    LOGINFO(" Failed to invoke getClients.");
                }
            }
            else
            {
                LOGINFO(" RDKShell is not yet active. Wait for it.. ");
            }
        }
        void SwitchBoard::setMemoryLimits()
        {
            JsonObject req, res;
            uint32_t status = Core::ERROR_NONE;

            req["enable"] = true;
            req["lowRam"] = m_lowMem;
            req["criticallyLowRam"] = m_criticalMem;
            status = m_remoteObject->Invoke<JsonObject, JsonObject>(THUNDER_TIMEOUT, "setMemoryMonitor", req, res);
            if (Core::ERROR_NONE == status)
            {
                LOGINFO(" Memory limits are set at %dM and %dM respectively.. ", m_lowMem, m_criticalMem);
            }
        }
    }

}
