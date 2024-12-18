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


#include "Module.h"
#include "RDKShellAdapterUtils.h"
#include "RDKShellAdapter.h"
#include <string>
#include <memory>
#include <iostream>
#include <mutex>
#include <thread>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <interfaces/IFocus.h>
#include <interfaces/IBrowser.h>
#include <rdkshell/logger.h>
#include <plugins/System.h>
#include <rdkshell/linuxkeys.h>

#include "UtilsJsonRpc.h"
#include "UtilsLOG_MILESTONE.h"
#include "UtilsUnused.h"
#include "UtilsgetRFCConfig.h"
#include "UtilsString.h"

using namespace std;
using namespace RdkShell;
using namespace Utils;
extern int gCurrentFramerate;

namespace WPEFramework {
    namespace Plugin {

    namespace AppManagerV1 {

        #define APPMANAGER_THUNDER_TIMEOUT 20000
        #define APPMANAGER_TRY_LOCK_WAIT_TIME_IN_MS 250
        #define THUNDER_ACCESS_DEFAULT_VALUE "127.0.0.1:9998"

        std::string sThunderSecurityToken;
        std::string sThunderAccessValue = THUNDER_ACCESS_DEFAULT_VALUE;

        static std::map<string, WPEFramework::Exchange::IAppManager::LifecycleState> sApplicationStateMap;
        static bool sRunning = true;

        sem_t sInitializeSemaphore;
        std::map<std::string, StateControlNotification*> sStateNotifications;
        std::map<std::string, PluginData> sActivePluginsData;
        std::map<std::string, bool> sDestroyApplications;
        std::map<std::string, bool> sExternalDestroyApplications;
        std::map<std::string, bool> sLaunchApplications;
        std::map<std::string, AppLastExitReason> sApplicationsExitReason;
        std::map<std::string, std::string> sPluginDisplayNameMap;
        std::vector<std::shared_ptr<CreateDisplayRequest>> sCreateDisplayRequests;
        std::vector<std::shared_ptr<KillClientRequest>> gKillClientRequests;
        std::mutex sRdkShellMutex;
        std::mutex sPluginDataMutex;
        std::mutex sLaunchDestroyMutex;
        std::mutex sDestroyMutex;
        std::mutex sLaunchMutex;
        std::mutex sExitReasonMutex;
        int32_t sLaunchCount = 0;

        RDKShellAdapter* RDKShellAdapter::_instance = nullptr;
        static std::thread shellThread;

        struct JSONRPCDirectLink
        {
        private:
          uint32_t mId { 0 };
          std::string mCallSign { };
#if ((THUNDER_VERSION >= 4) && (THUNDER_VERSION_MINOR == 4))
          PluginHost::ILocalDispatcher * dispatcher_ {nullptr};
#else
          PluginHost::IDispatcher * dispatcher_ {nullptr};
#endif

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
#if ((THUNDER_VERSION >= 4) && (THUNDER_VERSION_MINOR == 4))
              dispatcher_ = service->QueryInterfaceByCallsign<PluginHost::ILocalDispatcher>(mCallSign);
#else
              dispatcher_ = service->QueryInterfaceByCallsign<PluginHost::IDispatcher>(mCallSign);
#endif
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
#if ((THUNDER_VERSION >= 4) && (THUNDER_VERSION_MINOR == 4))
            string output = "";
            uint32_t result = Core::ERROR_BAD_REQUEST;

            if (dispatcher_  != nullptr) {
                PluginHost::ILocalDispatcher* localDispatcher = dispatcher_->Local();

                ASSERT(localDispatcher != nullptr);

                if (localDispatcher != nullptr)
                    result =  dispatcher_->Invoke(channelId, message->Id.Value(), sThunderSecurityToken, message->Designator.Value(), message->Parameters.Value(),output);
            }

            if (message.IsValid() == true) {
                if (result == static_cast<uint32_t>(~0)) {
                    message.Release();
                }
                else if (result == Core::ERROR_NONE)
                {
                    if (output.empty() == true)
                        message->Result.Null(true);
                    else
                        message->Result = output;
                }
                else
                {
                    message->Error.SetError(result);
                    if (output.empty() == false) {
                        message->Error.Text = output;
                    }
                }
            }
	    // This changes is added for response is getting empty on launching DAC application
	    if (!FromMessage(response, message, isResponseString))
	    {
		return Core::ERROR_GENERAL;
	    }
#elif (THUNDER_VERSION == 2)
            auto resp =  dispatcher_->Invoke(sThunderSecurityToken, channelId, *message);
#else
            Core::JSONRPC::Context context(channelId, message->Id.Value(), sThunderSecurityToken) ;
            auto resp = dispatcher_->Invoke(context, *message);
#endif

#if ((THUNDER_VERSION == 2) || (THUNDER_VERSION >= 4) && (THUNDER_VERSION_MINOR == 2))

            if (resp->Error.IsSet()) {
              std::cout << "Call failed: " << message->Designator.Value() << " error: " <<  resp->Error.Text.Value() << "\n";
              return resp->Error.Code;
            }

            if (!FromMessage(response, resp, isResponseString))
              return Core::ERROR_GENERAL;
#endif
            return Core::ERROR_NONE;
          }
        };

        StateControlNotification::StateControlNotification(std::string callsign, RDKShellAdapter& shell)
                                                           : mRDKShellAdapter(shell)
                                                           , mCallSign(callsign)
                                                           , mLaunchEnabled(false)
        {
        }

	void StateControlNotification::enableLaunch(bool enable)
	{
            mLaunchEnabled = enable;
        }

        void StateControlNotification::StateChange(const PluginHost::IStateControl::state state)
        {
            bool isSuspended = (state == PluginHost::IStateControl::SUSPENDED);
            bool isResumed = (state == PluginHost::IStateControl::RESUMED);
	    fflush(stdout); 
            if (mLaunchEnabled)
            {
               mLaunchEnabled = false;
            }
            if (isSuspended)
            {
                mRDKShellAdapter.updateAppState(mCallSign, WPEFramework::Exchange::IAppManager::LifecycleState::STATE_RUNNING);
            }
            if (isResumed)
            {
                mRDKShellAdapter.updateAppState(mCallSign, WPEFramework::Exchange::IAppManager::LifecycleState::STATE_ACTIVE);
            }
        }
        
        CreateDisplayRequest::CreateDisplayRequest(std::string client, std::string displayName, uint32_t displayWidth, uint32_t displayHeight, bool virtualDisplayEnabled, uint32_t virtualWidth, uint32_t virtualHeight, bool topmost, bool focus): mClient(client), mDisplayName(displayName), mDisplayWidth(displayWidth), mDisplayHeight(displayHeight), mVirtualDisplayEnabled(virtualDisplayEnabled), mVirtualWidth(virtualWidth),mVirtualHeight(virtualHeight), mTopmost(topmost), mFocus(focus), mResult(false) , mAutoDestroy(true)
        {
            sem_init(&mSemaphore, 0, 0);
        }

        CreateDisplayRequest::~CreateDisplayRequest()
        {
            sem_destroy(&mSemaphore);
        }

	KillClientRequest::KillClientRequest(std::string client): mClient(client), mResult(false)
        {
            sem_init(&mSemaphore, 0, 0);
        }

        KillClientRequest::~KillClientRequest()
	{
            sem_destroy(&mSemaphore);
	}

        void lockRdkShellMutex()
        {
            bool lockAcquired = false;
            double startTime = RdkShell::milliseconds();
            while (!lockAcquired && (RdkShell::milliseconds() - startTime) < APPMANAGER_TRY_LOCK_WAIT_TIME_IN_MS)
            {
                lockAcquired = sRdkShellMutex.try_lock();
            }
            if (!lockAcquired)
            {
                std::cout << "unable to get lock for defaulting to normal lock\n";
                sRdkShellMutex.lock();
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
            for (unsigned int i=0; i<sCreateDisplayRequests.size(); i++)
            {
              if (sCreateDisplayRequests[i]->mClient.compare(client) == 0)
              {
                exist = true;
                break;
              }
            }
            sRdkShellMutex.unlock();

            if (!exist)
            {
                std::vector<std::string> clientList;
                lockRdkShellMutex();
                CompositorController::getClients(clientList);
                sRdkShellMutex.unlock();
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
            std::string clientNameLower = clientName;
            std::transform(clientNameLower.begin(), clientNameLower.end(), clientNameLower.begin(), [](unsigned char value){ return std::tolower(value); });
            return clientNameLower;
        }

	void RDKShellAdapter::MonitorClients::handleInitialize(PluginHost::IShell* service)
        {
                if(service)
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
                       if (!isClientExists(serviceCallsign))
                       {
                           std::shared_ptr<CreateDisplayRequest> request = std::make_shared<CreateDisplayRequest>(serviceCallsign, clientidentifier);
                           sRdkShellMutex.lock();
                           sCreateDisplayRequests.push_back(request);
                           sRdkShellMutex.unlock();
                           sem_wait(&request->mSemaphore);
                       }
                       sPluginDataMutex.lock();
                       std::string className = service->ClassName();
                       PluginData pluginData;
                       pluginData.mClassName = className;
                       if (sActivePluginsData.find(serviceCallsign) == sActivePluginsData.end())
                       {
                           sActivePluginsData[serviceCallsign] = pluginData;
                       }
                       sPluginDataMutex.unlock();
                   }
                }
           }

        void  RDKShellAdapter::MonitorClients::handleActivated(PluginHost::IShell* service)
        {
            if (service)
            {
                mShell.updateAppState(service->Callsign(), WPEFramework::Exchange::IAppManager::LifecycleState::STATE_ACTIVE);
            }
        }

        void RDKShellAdapter::MonitorClients::handleDeactivated(PluginHost::IShell* service)
        {
            if (service)
            {
	        sLaunchDestroyMutex.lock();
                if (sDestroyApplications.find(service->Callsign()) == sDestroyApplications.end())
                {
                     sExternalDestroyApplications[service->Callsign()] = true;
                }
                sLaunchDestroyMutex.unlock();
                StateControlNotification* notification = nullptr;
                sPluginDataMutex.lock();
                auto notificationIt = sStateNotifications.find(service->Callsign());
                if (notificationIt != sStateNotifications.end())
                {
                    notification = notificationIt->second;
                    sStateNotifications.erase(notificationIt);
                }
                sPluginDataMutex.unlock();
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
                sExitReasonMutex.lock();
                sApplicationsExitReason[service->Callsign()] = AppLastExitReason::DEACTIVATED;

                if(service->Reason() == PluginHost::IShell::FAILURE)
                {
                    sApplicationsExitReason[service->Callsign()] = AppLastExitReason::CRASH;
                }
                sExitReasonMutex.unlock();
                mShell.updateAppState(service->Callsign(), WPEFramework::Exchange::IAppManager::LifecycleState::STATE_TERMINATED);
            }
        }

        void RDKShellAdapter::MonitorClients::handleDeinitialized(PluginHost::IShell* service)
        {
            if (service)
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
                    sRdkShellMutex.lock();
                    gKillClientRequests.push_back(request);
                    sRdkShellMutex.unlock();
                    sem_wait(&request->mSemaphore);
                }
                
                sPluginDataMutex.lock();
                std::map<std::string, PluginData>::iterator pluginToRemove = sActivePluginsData.find(service->Callsign());
                if (pluginToRemove != sActivePluginsData.end())
                {
                    sActivePluginsData.erase(pluginToRemove);
                }
                sPluginDataMutex.unlock();
                sLaunchDestroyMutex.lock();
                if (sExternalDestroyApplications.find(service->Callsign()) != sExternalDestroyApplications.end())
                {
                    sExternalDestroyApplications.erase(service->Callsign());
                }
                sLaunchDestroyMutex.unlock();
            }
        }


        void RDKShellAdapter::MonitorClients::StateChange(PluginHost::IShell* service)
        {
            if (service)
            {
                PluginHost::IShell::state currentState(service->State());

                sExitReasonMutex.lock();
                if ((currentState == PluginHost::IShell::DEACTIVATED) || (currentState == PluginHost::IShell::DESTROYED))
                {
                     sApplicationsExitReason[service->Callsign()] = AppLastExitReason::DEACTIVATED;
                }
                if(service->Reason() == PluginHost::IShell::FAILURE)
                {
                    sApplicationsExitReason[service->Callsign()] = AppLastExitReason::CRASH;
                }
                sExitReasonMutex.unlock();

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
                       if (!isClientExists(serviceCallsign))
                       {
                           std::shared_ptr<CreateDisplayRequest> request = std::make_shared<CreateDisplayRequest>(serviceCallsign, clientidentifier);
                           sRdkShellMutex.lock();
                           sCreateDisplayRequests.push_back(request);
                           sRdkShellMutex.unlock();
                           sem_wait(&request->mSemaphore);
                       }
                       sPluginDataMutex.lock();
                       std::string className = service->ClassName();
                       PluginData pluginData;
                       pluginData.mClassName = className;
                       if (sActivePluginsData.find(serviceCallsign) == sActivePluginsData.end())
                       {
                           sActivePluginsData[serviceCallsign] = pluginData;
                       }
                       sPluginDataMutex.unlock();
                   }
                }
                else if (currentState == PluginHost::IShell::ACTIVATED && service->Callsign() == "org.rdk.AppManager")
                {
                }
                else if (currentState == PluginHost::IShell::DEACTIVATION)
                {
                    sLaunchDestroyMutex.lock();
                    if (sDestroyApplications.find(service->Callsign()) == sDestroyApplications.end())
                    {
                        sExternalDestroyApplications[service->Callsign()] = true;
                    }
                    sLaunchDestroyMutex.unlock();
                    StateControlNotification* notification = nullptr;
                    sPluginDataMutex.lock();
                    auto notificationIt = sStateNotifications.find(service->Callsign());
                    if (notificationIt != sStateNotifications.end())
                    {
                        notification = notificationIt->second;
                        sStateNotifications.erase(notificationIt);
                    }
                    sPluginDataMutex.unlock();
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
                else if ((currentState == PluginHost::IShell::DEACTIVATED))
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
                        sRdkShellMutex.lock();
                        gKillClientRequests.push_back(request);
                        sRdkShellMutex.unlock();
                        sem_wait(&request->mSemaphore);
                    }
                    
                    sPluginDataMutex.lock();
                    std::map<std::string, PluginData>::iterator pluginToRemove = sActivePluginsData.find(service->Callsign());
                    if (pluginToRemove != sActivePluginsData.end())
                    {
                        sActivePluginsData.erase(pluginToRemove);
                    }
                    sPluginDataMutex.unlock();
                    sLaunchDestroyMutex.lock();
                    if (sExternalDestroyApplications.find(service->Callsign()) != sExternalDestroyApplications.end())
                    {
                        sExternalDestroyApplications.erase(service->Callsign());
                    }
                    sLaunchDestroyMutex.unlock();
                }
            }
        }

#ifdef USE_THUNDER_R4
       void RDKShellAdapter::MonitorClients::Initialize(const string& callsign, PluginHost::IShell* service)
       {
             handleInitialize(service);
       }
       void RDKShellAdapter::MonitorClients::Activation(const string& callsign, PluginHost::IShell* service)
       {}
       void RDKShellAdapter::MonitorClients::Activated(const string& callsign, PluginHost::IShell* service)
       {
	    handleActivated(service);
       }
       void RDKShellAdapter::MonitorClients::Deactivation(const string& callsign, PluginHost::IShell* service)
       {}
       void RDKShellAdapter::MonitorClients::Deactivated(const string& callsign, PluginHost::IShell* service)
       {
            //StateChange(service);
            handleDeactivated(service);
       }
       void RDKShellAdapter::MonitorClients::Deinitialized(const string& callsign, PluginHost::IShell* service)
       {
            handleDeinitialized(service);
       }
       void RDKShellAdapter::MonitorClients::Unavailable(const string& callsign, PluginHost::IShell* service)
       {}
#endif /* USE_THUNDER_R4 */

        RDKShellAdapter::RDKShellAdapter()
                : mClientsMonitor(Core::Service<MonitorClients>::Create<MonitorClients>(this)),
                mCurrentService(nullptr), 
                mErmEnabled(false),
		mEventListener(nullptr)
        {
            LOGINFO("ctor");
            RDKShellAdapter::_instance = this;
        }

        RDKShellAdapter::~RDKShellAdapter()
        {
            //LOGINFO("dtor");
	    sApplicationStateMap.clear();
        }

        void RDKShellAdapter::updateAppState(const string callsign, Exchange::IAppManager::LifecycleState newState)
        {
            std::string newcallsign = toLower(callsign);
            Exchange::IAppManager::LifecycleState oldState;
            if (sApplicationStateMap.find(newcallsign) != sApplicationStateMap.end())
            {
                oldState = sApplicationStateMap[newcallsign];
            }
            else
            {
                oldState = WPEFramework::Exchange::IAppManager::LifecycleState::STATE_RUNNING;
                if (sLaunchApplications.find(callsign) != sLaunchApplications.end())
                {
                    sApplicationStateMap.insert({newcallsign, WPEFramework::Exchange::IAppManager::LifecycleState::STATE_RUNNING});
                }
                else
                {
                    return;
                }
            }
            sApplicationStateMap[newcallsign] = newState;
            if (oldState != newState)
            {		    
                onAppStateChanged(newcallsign, newState, oldState);
            }
        }

        const bool RDKShellAdapter::Initialize(EventListener* listener, PluginHost::IShell* service)
        {
	    mEventListener = listener;	
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

            mCurrentService->Register(mClientsMonitor);

            static PluginHost::IShell* pluginService = nullptr;
            pluginService = mCurrentService;

            mErmEnabled = CompositorController::isErmEnabled();
            sem_init(&sInitializeSemaphore, 0, 0);
            shellThread = std::thread([=]() {
                bool isRunning = true;
                sRdkShellMutex.lock();
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
                sRdkShellMutex.unlock();
                sem_post(&sInitializeSemaphore);
                while(isRunning) {
                  const double maxSleepTime = (1000 / gCurrentFramerate) * 1000;
                  double startFrameTime = RdkShell::microseconds();
                  sRdkShellMutex.lock();
                  while (sCreateDisplayRequests.size() > 0)
                  {
		      std::shared_ptr<CreateDisplayRequest> request = sCreateDisplayRequests.front();
                      if (!request)
                      {
                          sCreateDisplayRequests.erase(sCreateDisplayRequests.begin());
                          continue;
                      }
                      request->mResult = CompositorController::createDisplay(request->mClient, request->mDisplayName, request->mDisplayWidth, request->mDisplayHeight, request->mVirtualDisplayEnabled, request->mVirtualWidth, request->mVirtualHeight, request->mTopmost, request->mFocus , request->mAutoDestroy);
                      sCreateDisplayRequests.erase(sCreateDisplayRequests.begin());
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
                  RdkShell::draw();
                  RdkShell::update();
                  isRunning = sRunning;
                  sRdkShellMutex.unlock();
                  double frameTime = (int)RdkShell::microseconds() - (int)startFrameTime;
                  if (frameTime < maxSleepTime)
                  {
                      int sleepTime = (int)maxSleepTime-(int)frameTime;
                      usleep(sleepTime);
                  }
                }
            });

            char* thunderAccessValue = getenv("THUNDER_ACCESS_VALUE");
            if (NULL != thunderAccessValue)
            {
                sThunderAccessValue = thunderAccessValue;
            }
            sem_wait(&sInitializeSemaphore);
            return true;
        }

        void RDKShellAdapter::Deinitialize()
        {
            LOGINFO("Deinitialize");
	    mEventListener = nullptr;
            PluginHost::IShell* service = mCurrentService;
            sRdkShellMutex.lock();
            RdkShell::deinitialize();
            sRunning = false;
            sRdkShellMutex.unlock();
            shellThread.join();
            std::vector<std::string> clientList;
            CompositorController::getClients(clientList);
            std::vector<std::string>::iterator ptr;
            mCurrentService = nullptr;
            service->Unregister(mClientsMonitor);
            mClientsMonitor->Release();
            RDKShellAdapter::_instance = nullptr;
            CompositorController::setEventListener(nullptr);
            mEventListener = nullptr;
            sActivePluginsData.clear();
            sRdkShellMutex.lock();
            for (unsigned int i=0; i<sCreateDisplayRequests.size(); i++)
            {
                if (sCreateDisplayRequests[i])
                {
                    sem_destroy(&sCreateDisplayRequests[i]->mSemaphore);
                    sCreateDisplayRequests[i] = nullptr;
                }
            }
            sCreateDisplayRequests.clear();
            for (unsigned int i=0; i<gKillClientRequests.size(); i++)
            {
                if (gKillClientRequests[i])
                {
                    sem_destroy(&gKillClientRequests[i]->mSemaphore);
                    gKillClientRequests[i] = nullptr;
                }
            }
            gKillClientRequests.clear();
            sRdkShellMutex.unlock();
            sExternalDestroyApplications.clear();
            sem_destroy(&sInitializeSemaphore);
        }

        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > RDKShellAdapter::getThunderControllerClient(std::string callsign, std::string localidentifier)
        {
            string query = "token=" + sThunderSecurityToken;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(sThunderAccessValue)));
            std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > thunderClient = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >(callsign.c_str(), localidentifier.c_str(), false, query);
            return thunderClient;
        }

        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> RDKShellAdapter::getPackagerPlugin()
        {
            string query = "token=" + sThunderSecurityToken;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(sThunderAccessValue)));
            return make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>("Packager.1", "", false, query);
        }

        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> RDKShellAdapter::getOCIContainerPlugin()
        {
            string query = "token=" + sThunderSecurityToken;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(sThunderAccessValue)));
            return make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>("org.rdk.OCIContainer.1", "", false, query);
        }

        uint32_t RDKShellAdapter::kill(const JsonObject& parameters, JsonObject& response)
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

                    ociContainerPlugin->Invoke<JsonObject, JsonObject>(APPMANAGER_THUNDER_TIMEOUT, "getContainerInfo", param, containerInfoResult);

                    // If success is false, the container isn't running so nothing to do
                    if (containerInfoResult["success"].Boolean())
                    {
                        auto containerInfo = containerInfoResult["info"].Object();

                        // Dobby knows about that container - what's it doing?
                        if (containerInfo["state"] == "running" || containerInfo["state"] == "starting")
                        {
                            ociContainerPlugin->Invoke<JsonObject, JsonObject>(APPMANAGER_THUNDER_TIMEOUT, "stopContainer", param, stopContainerResult);
                        }
                        else if (containerInfo["state"] == "paused")
                        {
                            // Paused, so force stop
                            param["force"] = true;
                            ociContainerPlugin->Invoke<JsonObject, JsonObject>(APPMANAGER_THUNDER_TIMEOUT, "stopContainer", param, stopContainerResult);
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

        uint32_t RDKShellAdapter::getClients(const JsonObject& parameters, JsonObject& response)
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

        uint32_t RDKShellAdapter::launch(const JsonObject& parameters, JsonObject& response)
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
                sLaunchMutex.lock();
                if (sLaunchCount > 0)
                {
                    launchInProgress = true;
                }
                else
                {
                    sLaunchCount++;
                }
                currentLaunchCount = sLaunchCount;
                sLaunchMutex.unlock();
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
                sLaunchDestroyMutex.lock();
                if (sDestroyApplications.find(appCallsign) != sDestroyApplications.end())
                {
                    isApplicationBeingDestroyed = true;
                }
                else
                {
                    sLaunchApplications[appCallsign] = true;
                }
                sLaunchDestroyMutex.unlock();
                if (isApplicationBeingDestroyed)
                {
                    sLaunchMutex.lock();
                    sLaunchCount = 0;
                    sLaunchMutex.unlock();
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
                for (std::map<std::string, PluginData>::iterator pluginDataEntry = sActivePluginsData.begin(); pluginDataEntry != sActivePluginsData.end(); pluginDataEntry++)
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
                if ((false == newPluginFound) && (false == originalPluginFound))
		{
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
                    sLaunchMutex.lock();
                    sLaunchCount = 0;
                    sLaunchMutex.unlock();
		    sLaunchDestroyMutex.lock();
                    sLaunchApplications.erase(appCallsign);
		    sLaunchDestroyMutex.unlock();
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
                        while (!lockAcquired && (RdkShell::milliseconds() - startTime) < APPMANAGER_TRY_LOCK_WAIT_TIME_IN_MS)
                        {
                            lockAcquired = sRdkShellMutex.try_lock();
                        }
                        if (!lockAcquired)
                        {
                            std::cout << "unable to get lock for create display, defaulting to normal lock\n";
                            sRdkShellMutex.lock();
                        }
                        else
                        {
                            std::cout << "lock was acquired via try for create display\n";
                        }
                    }
                    sRdkShellMutex.unlock();
                    if (!isClientExists(callsign))
                    {
                        std::shared_ptr<CreateDisplayRequest> request = std::make_shared<CreateDisplayRequest>(callsign, displayName, width, height);
                        request->mAutoDestroy = autoDestroy;
                        lockRdkShellMutex();
                        sPluginDisplayNameMap[callsign] = displayName;
                        std::cout << "Added displayname : "<<displayName<< std::endl;
                        sCreateDisplayRequests.push_back(request);
                        sRdkShellMutex.unlock();
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
                    // fetching status of plugin is failed, clear the display and close the request here
                    if (status > 0)
		    {
                        kill(callsign);
                        sLaunchMutex.lock();
                        sLaunchCount = 0;
                        sLaunchMutex.unlock();
	                sLaunchDestroyMutex.lock();
                        sLaunchApplications.erase(appCallsign);
	                sLaunchDestroyMutex.unlock();
                        response["message"] = "failed to launch application";
                        returnResponse(false);
		    }
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

		if (!type.empty() && type == "Amazon")
		{
#ifdef RFC_ENABLED
			RFC_ParamData_t param;
			if (Utils::getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Dobby.Amazon.Enable", param))
			{
				JsonObject root;
				if (strncasecmp(param.value, "true", 4) == 0)
				{
					std::cout << "dobby rfc true - launching amazon in container mode " << std::endl;
					root = configSet["root"].Object();
					root["mode"] = JsonValue("Container");
				}
				else
				{
					std::cout << "dobby rfc false - launching amazon in local mode " << std::endl;
					root = configSet["root"].Object();
					root["mode"] = JsonValue("Local");
				}
				configSet["root"] = root;
			}
			else
			{
				std::cout << "reading amazon dobby rfc failed " << std::endl;
			}
#else
			std::cout << "rfc is disabled and unable to check for amazon container mode " << std::endl;
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
                        while (!lockAcquired && (RdkShell::milliseconds() - startTime) < APPMANAGER_TRY_LOCK_WAIT_TIME_IN_MS)
                        {
                            lockAcquired = sRdkShellMutex.try_lock();
                        }
                        if (!lockAcquired)
                        {
                            std::cout << "unable to get lock for get bounds, defaulting to normal lock\n";
                            sRdkShellMutex.lock();
                        }
                        else
                        {
                            std::cout << "lock was acquired via try for get bounds\n";
                        }
                    }
                    CompositorController::getBounds(callsign, tempX, tempY, screenWidth, screenHeight);
                    sRdkShellMutex.unlock();
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
                        while (!lockAcquired && (RdkShell::milliseconds() - startTime) < APPMANAGER_TRY_LOCK_WAIT_TIME_IN_MS)
                        {
                            lockAcquired = sRdkShellMutex.try_lock();
                        }
                        if (!lockAcquired)
                        {
                            std::cout << "unable to get lock for set bounds, defaulting to normal lock\n";
                            sRdkShellMutex.lock();
                        }
                        else
                        {
                            std::cout << "lock was acquired via try for set bounds\n";
                        }
                    }
                    std::cout << "setting the desired bounds\n";
                    CompositorController::setBounds(callsign, 0, 0, 1, 1); //forcing a compositor resize flush
                    CompositorController::setBounds(callsign, x, y, width, height);
                    sRdkShellMutex.unlock();

                    if (scaleToFit)
                    {
                        std::cout << "scaling app to fit full screen" << std::endl;
                        double sx = 1.0;
                        double sy = 1.0;
                        if (width != screenWidth && width !=0)
                        {
                            sx = (double)screenWidth / (double)width;
                        }
                        if (height != screenHeight && height!=0)
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

                    sPluginDataMutex.lock();
                    {
                      auto notificationIt = sStateNotifications.find(callsign);
                      if (notificationIt == sStateNotifications.end()) {
                        PluginHost::IStateControl* stateControl(mCurrentService->QueryInterfaceByCallsign<PluginHost::IStateControl>(callsign));
                        if (stateControl) {
                          auto* handler = new Core::Sink<StateControlNotification>(callsign, *this);
                          stateControl->Register(handler);
                          stateControl->Release();
                          sStateNotifications[callsign] = handler;
                        }
                      } else {
                        notificationIt->second->enableLaunch(true);
                        deferLaunch = true;
                      }
                    }
                    sPluginDataMutex.unlock();
 
                    if (setSuspendResumeStateOnLaunch)
                    {
                        if (suspend)
                        {
                            if (launchType == RDKShellLaunchType::UNKNOWN)
                            {
                                launchType = RDKShellLaunchType::SUSPEND;
                            }

                            WPEFramework::Core::JSON::String stateString;
                            stateString = "suspended";
                            status = JSONRPCDirectLink(mCurrentService, callsign).Set<WPEFramework::Core::JSON::String>(APPMANAGER_THUNDER_TIMEOUT, "state", stateString);
                            
                            std::cout << "setting the state to suspended\n";
                            visible = false;
                        }
                        else
                        {
                            if (launchType == RDKShellLaunchType::UNKNOWN)
                            {
                                launchType = RDKShellLaunchType::RESUME;
                            }
                            
                            WPEFramework::Core::JSON::String stateString;
                            stateString = "resumed";
                            status = JSONRPCDirectLink(mCurrentService, callsign).Set<WPEFramework::Core::JSON::String>(APPMANAGER_THUNDER_TIMEOUT, "state", stateString);

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
                        status = JSONRPCDirectLink(mCurrentService, callsign).Set<WPEFramework::Core::JSON::String>(APPMANAGER_THUNDER_TIMEOUT, "url",urlString);
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
                    sLaunchMutex.lock();
                    sLaunchCount = 0;
                    sLaunchMutex.unlock();
                   			    
                    if (setSuspendResumeStateOnLaunch && deferLaunch && ((launchType == SUSPEND) || (launchType == RESUME)))
                    {
                        std::cout << "deferring application launch " << std::endl;
                    }
                    else
                    {
                        if (!suspend)
		        {       	
                            updateAppState(callsign, WPEFramework::Exchange::IAppManager::LifecycleState::STATE_ACTIVE);
                        }
                    }
                    response["launchType"] = launchTypeString;
                }
                
            }
            if (!result) 
            {
                response["message"] = "failed to launch application";
            }
            sLaunchMutex.lock();
            sLaunchCount = 0;
            sLaunchMutex.unlock();
	    sLaunchDestroyMutex.lock();
            sLaunchApplications.erase(appCallsign);
	    sLaunchDestroyMutex.unlock();
            std::cout << "new launch count at loc2 is 0\n";

            returnResponse(result);
        }

        uint32_t RDKShellAdapter::suspend(const JsonObject& parameters, JsonObject& response)
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
            	sLaunchDestroyMutex.lock();
            	if (sDestroyApplications.find(client) != sDestroyApplications.end())
            	{
                    isApplicationBeingDestroyed = true;
            	}
            	if (sExternalDestroyApplications.find(client) != sExternalDestroyApplications.end())
            	{
                    isApplicationBeingDestroyed = true;
            	}
            	sLaunchDestroyMutex.unlock();
            	if (isApplicationBeingDestroyed)
            	{
                    std::cout << "ignoring suspend for " << client << " as it is being destroyed " << std::endl;
		    result=false;
		    response["message"] = "failed to suspend application";
                    returnResponse(result);
            	}
                sDestroyMutex.lock();
                PluginHost::IStateControl* stateControl(mCurrentService->QueryInterfaceByCallsign<PluginHost::IStateControl>(callsign));
                if (stateControl)
		{
                    stateControl->Request(PluginHost::IStateControl::SUSPEND);
                    stateControl->Release();
                    sDestroyMutex.unlock();
                    status = Core::ERROR_NONE;
                }
		else
		{
                    sDestroyMutex.unlock();
                    WPEFramework::Core::JSON::String stateString;
                    stateString = "suspended";
                    const string callsignWithVersion = callsign + ".1";
                    status = getThunderControllerClient(callsignWithVersion)->Set<WPEFramework::Core::JSON::String>(APPMANAGER_THUNDER_TIMEOUT, "state", stateString);
                }
                if (status > 0)
                {
                    std::cout << "failed to suspend " << callsign << ".  status: " << status << std::endl;
                    result = false;
                }
                else
                {
                    setVisibility(callsign, false);
                    updateAppState(callsign, WPEFramework::Exchange::IAppManager::LifecycleState::STATE_RUNNING);
                }
            }
            if (!result)
            {
                response["message"] = "failed to suspend application";
            }
            returnResponse(result);
        }

        uint32_t RDKShellAdapter::destroy(const JsonObject& parameters, JsonObject& response)
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
		sLaunchDestroyMutex.lock();
                if (sLaunchApplications.find(callsign) != sLaunchApplications.end())
                {
                    isApplicationBeingLaunched = true;
                }
                else
                {
                    sDestroyApplications[callsign] = true;
                }
		sLaunchDestroyMutex.unlock();
                if (isApplicationBeingLaunched)
                {
                    std::cout << "failed to destroy " << callsign << " as launch in progress" << std::endl;
                    response["message"] = "failed to destroy application as same application being launched";
                    returnResponse(false);
                }
                std::cout << "destroying " << callsign << std::endl;
                sDestroyMutex.lock();
                uint32_t status = deactivate(mCurrentService, callsign);
                std::cout << "Status after deactivation is " << status << std::endl;
                sDestroyMutex.unlock();
		sLaunchDestroyMutex.lock();
                sDestroyApplications.erase(callsign);
		sLaunchDestroyMutex.unlock();
                if (status != 0)
                {
                    result = false;
                }			
            }
            if (!result)
            {
                response["message"] = "failed to destroy application";
            }
            returnResponse(result);
        }

        uint32_t RDKShellAdapter::getAvailableTypes(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            string method = "status";
            Core::JSON::ArrayType<PluginHost::MetaData::Service> joResult;
            auto thunderController = getThunderControllerClient();
            thunderController->Get<Core::JSON::ArrayType<PluginHost::MetaData::Service>>(APPMANAGER_THUNDER_TIMEOUT, method.c_str(), joResult);

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

        uint32_t RDKShellAdapter::getState(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            string method = "status";
            Core::JSON::ArrayType<PluginHost::MetaData::Service> joResult;
            auto thunderController = getThunderControllerClient();
            thunderController->Get<Core::JSON::ArrayType<PluginHost::MetaData::Service>>(APPMANAGER_THUNDER_TIMEOUT, method.c_str(), joResult);

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

                                stateStatus = getThunderControllerClient(callsignWithVersion)->Get<WPEFramework::Core::JSON::String>(APPMANAGER_THUNDER_TIMEOUT, "state", stateString);

                            if (stateStatus == 0)
                            {
                                WPEFramework::Core::JSON::String urlString;
                                uint32_t urlStatus = 1;
                                    urlStatus = getThunderControllerClient(callsignWithVersion)->Get<WPEFramework::Core::JSON::String>(APPMANAGER_THUNDER_TIMEOUT, "url",urlString);

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
                                sExitReasonMutex.lock();
                                if (sApplicationsExitReason.find(callsign) != sApplicationsExitReason.end())
                                {
                                    typeObject["lastExitReason"] = (int)sApplicationsExitReason[callsign];
                                }
                                else
                                {
                                    typeObject["lastExitReason"] = (int)AppLastExitReason::UNDEFINED;
                                }
                                sExitReasonMutex.unlock();

                                stateArray.Add(typeObject);
                            }
                        }
                    }
                }
            }
            response["state"] = stateArray;

            returnResponse(result);
        }
        // Registered methods end

        void RDKShellAdapter::killAllApps(bool enableDestroyEvent)
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
                   destroy(destroyRequest, destroyResponse);
                }
            }
        }

        bool RDKShellAdapter::setAVBlocked(const string callsign, bool blockAV)
        {
            bool status = true;

            sRdkShellMutex.lock();
            std::map<std::string, std::string>::iterator displayNameItr = sPluginDisplayNameMap.find(callsign);
            if (displayNameItr != sPluginDisplayNameMap.end())
            {
                std::string clientId(callsign + ',' + displayNameItr->second);
                std::cout << "setAVBlocked callsign: " << callsign << " clientIdentifier:<"<<clientId<<">blockAV:"<<std::boolalpha << blockAV << std::noboolalpha << std::endl;
                status = CompositorController::setAVBlocked(clientId, blockAV);
            }
            else
            {
                status = false;
                std::cout << "display not found for " << callsign << std::endl;
            }
            sRdkShellMutex.unlock();
            if (false == status)
            {
                std::cout << "setAVBlocked failed for " << callsign << std::endl;
            }

            return status;
        }

        // Internal methods begin

        bool RDKShellAdapter::moveBehind(const string& client, const string& target)
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
            sRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShellAdapter::setFocus(const string& client)
        {
            bool ret = false;
            bool isApplicationBeingDestroyed = false;
            sLaunchDestroyMutex.lock();
            if (sDestroyApplications.find(client) != sDestroyApplications.end())
            {
                isApplicationBeingDestroyed = true;
            }
            sLaunchDestroyMutex.unlock();
            if (isApplicationBeingDestroyed)
            {
                std::cout << "ignoring setFocus for " << client << " as it is being destroyed " << std::endl;
                return false;
            }
            std::string previousFocusedClient;
            lockRdkShellMutex();
            CompositorController::getFocused(previousFocusedClient);
            ret = CompositorController::setFocus(client);
            sRdkShellMutex.unlock();
            std::string clientLower = toLower(client);

            if (previousFocusedClient != clientLower)
            {
                std::map<std::string, PluginData> activePluginsData;
                sPluginDataMutex.lock();
                activePluginsData = sActivePluginsData;
                sPluginDataMutex.unlock();

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
	
	bool RDKShellAdapter::getFocused(string& client)
	{
		bool ret = false;
		sRdkShellMutex.lock();
		ret = CompositorController::getFocused(client);
		sRdkShellMutex.unlock();
		return ret;
	}

        bool RDKShellAdapter::kill(const string& client)
        {
            bool ret = false;
            lockRdkShellMutex();
            std::shared_ptr<KillClientRequest> request = std::make_shared<KillClientRequest>(client);
            gKillClientRequests.push_back(request);
            sPluginDisplayNameMap.erase(client);
            std::cout << "removed displayname : "<<client<< std::endl;
            sRdkShellMutex.unlock();
            sem_wait(&request->mSemaphore);
            ret = request->mResult;
            return ret;
        }

        bool RDKShellAdapter::getScreenResolution(JsonObject& out)
        {
            unsigned int width=0,height=0;
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::getScreenResolution(width, height);
            sRdkShellMutex.unlock();
            if (true == ret) {
              out["w"] = width;
              out["h"] = height;
              return true;
            }
            return false;
        }

        bool RDKShellAdapter::setMimeType(const string& client, const string& mimeType)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::setMimeType(client, mimeType);
            sRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShellAdapter::getMimeType(const string& client, string& mimeType)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::getMimeType(client, mimeType);
            sRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShellAdapter::createDisplay(const string& client, const string& displayName, const uint32_t displayWidth, const uint32_t displayHeight,
            const bool virtualDisplay, const uint32_t virtualWidth, const uint32_t virtualHeight, const bool topmost, const bool focus)
        {
            bool ret = false;
            if (!isClientExists(client))
            {
                lockRdkShellMutex();
                std::shared_ptr<CreateDisplayRequest> request = std::make_shared<CreateDisplayRequest>(client, displayName, displayWidth, displayHeight, virtualDisplay, virtualWidth, virtualHeight);
                sCreateDisplayRequests.push_back(request);
                sRdkShellMutex.unlock();
                sem_wait(&request->mSemaphore);
                ret = request->mResult;
            }
            else
            {
                std::cout << "Client " << client  << "already exist " << std::endl;
            }
            return ret;
        }

        bool RDKShellAdapter::getClients(JsonArray& clients)
        {
            std::vector<std::string> clientList;
            lockRdkShellMutex();
            CompositorController::getClients(clientList);
            sRdkShellMutex.unlock();
            std::map<std::string, bool> clientDataPresent;
            for (size_t i=0; i<clientList.size(); i++)
	    {
	      std::string appName = clientList[i]; 	    
              JsonObject clientInfo;
	      clientInfo["app"] = appName;
              
	      if (sApplicationStateMap.find(appName) != sApplicationStateMap.end())
	      {
                  clientInfo["state"] = (int) sApplicationStateMap[appName];
	      }
              else
              {
                  clientInfo["state"] = "UNKNOWN"; 
              }
              clients.Add(clientInfo);
              clientDataPresent.insert({appName, true});
            }
            std::map<string, WPEFramework::Exchange::IAppManager::LifecycleState>::iterator iter = sApplicationStateMap.end();
            for (iter = sApplicationStateMap.begin(); iter != sApplicationStateMap.end(); iter++)
	    {	    
               std::string appName = iter->first;
	       if (clientDataPresent.find(appName) == clientDataPresent.end())
	       {	       
                   JsonObject clientInfo;
	           clientInfo["app"] = appName;
                   clientInfo["state"] = (int) iter->second;
                   clients.Add(clientInfo);
               }
	    }
            clientDataPresent.clear();
            return true;
        }

        bool RDKShellAdapter::setVisibility(const string& client, const bool visible)
        {
            bool ret = false;
            {
                bool lockAcquired = false;
                double startTime = RdkShell::milliseconds();
                while (!lockAcquired && (RdkShell::milliseconds() - startTime) < APPMANAGER_TRY_LOCK_WAIT_TIME_IN_MS)
                {
                    lockAcquired = sRdkShellMutex.try_lock();
                }
                if (!lockAcquired)
                {
                    std::cout << "unable to get lock for visibility, defaulting to normal lock\n";
                    sRdkShellMutex.lock();
                }
                else
                {
                    std::cout << "lock was acquired via try for visibility\n";
                }
            }
            ret = CompositorController::setVisibility(client, visible);
            sRdkShellMutex.unlock();
            
            bool isApplicationBeingDestroyed = false;
            sLaunchDestroyMutex.lock();
            if (sDestroyApplications.find(client) != sDestroyApplications.end())
            {
                isApplicationBeingDestroyed = true;
            }
            if (sExternalDestroyApplications.find(client) != sExternalDestroyApplications.end())
            {
                 isApplicationBeingDestroyed = true;
            }
            sLaunchDestroyMutex.unlock();
            if (isApplicationBeingDestroyed)
            {
                std::cout << "ignoring setvisibility for " << client << " as it is being destroyed " << std::endl;
                return false;
            }
            std::map<std::string, PluginData> activePluginsData;
            sPluginDataMutex.lock();
            activePluginsData = sActivePluginsData;
            sPluginDataMutex.unlock();
            std::map<std::string, PluginData>::iterator pluginsEntry = activePluginsData.find(client);
            if (pluginsEntry != activePluginsData.end())
            {
                PluginData& pluginData = pluginsEntry->second;
                if (pluginData.mClassName.compare("WebKitBrowser") == 0)
                {
                    std::cout << "setting the visiblity of " << client << " to " << visible << std::endl;
                    uint32_t status = 0;
                    sLaunchDestroyMutex.lock();
                    if (sDestroyApplications.find(client) != sDestroyApplications.end())
                    {
                        isApplicationBeingDestroyed = true;
                    }
                    sLaunchDestroyMutex.unlock();
                    if (isApplicationBeingDestroyed)
                    {
                        std::cout << "ignoring setvisibility for " << client << " as it is being destroyed " << std::endl;
			return false;
                    }
                    sDestroyMutex.lock();
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
                    sDestroyMutex.unlock();
                    if (status > 0)
                    {
                        std::cout << "failed to set visibility property to browser " << client << " with status code " << status << std::endl;
                    }
                }
            }

            return ret;
        }

        bool RDKShellAdapter::setOpacity(const string& client, const unsigned int opacity)
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
            sRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShellAdapter::setScale(const string& client, const double scaleX, const double scaleY)
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
            sRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShellAdapter::setHolePunch(const string& client, const bool holePunch)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::setHolePunch(client, holePunch);
            sRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShellAdapter::setTopmost(const string& callsign, const bool topmost, const bool focus)
        {
            bool ret = false;
            lockRdkShellMutex();
            ret = CompositorController::setTopmost(callsign, topmost, focus);
            sRdkShellMutex.unlock();
            return ret;
        }

        void RDKShellAdapter::onAppStateChanged(const std::string& client, Exchange::IAppManager::LifecycleState newState, Exchange::IAppManager::LifecycleState oldState)
	{
            if (mEventListener)
            {		    
                mEventListener->onAppStateChanged(client, newState, oldState);
            }
	}
        // Internal methods end
        } // namespace AppManager
    } // namespace Plugin
} // namespace WPEFramework
