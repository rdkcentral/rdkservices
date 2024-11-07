/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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
 */
 
#include "XCastImplementation.h"
#include <sys/prctl.h>
#include "UtilsJsonRpc.h"
#include "rfcapi.h"
#if defined(SECURITY_TOKEN_ENABLED) && ((SECURITY_TOKEN_ENABLED == 0) || (SECURITY_TOKEN_ENABLED == false))
#define GetSecurityToken(a, b) 0
#define GetToken(a, b, c) 0
#else
#include <securityagent/securityagent.h>
#include <securityagent/SecurityTokenUtil.h>
#endif

#define SERVER_DETAILS "127.0.0.1:9998"
#define NETWORK_CALLSIGN_VER "org.rdk.Network.1"
#define THUNDER_RPC_TIMEOUT 5000
#define MAX_SECURITY_TOKEN_SIZE 1024

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 2

#define LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS  5000  //5 seconds
#define LOCATE_CAST_SECOND_TIMEOUT_IN_MILLIS 10000  //10 seconds

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(XCastImplementation, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    XCastImplementation *XCastImplementation::_instance = nullptr;
    XCastManager* XCastImplementation::m_xcast_manager = nullptr;
    static std::vector <DynamicAppConfig*> appConfigListCache;
    static std::mutex m_appConfigMutex;
    static bool xcastEnableCache = false;
    static string friendlyNameCache = "Living Room";
    static string m_activeInterfaceName = "";
    static bool m_isDynamicRegistrationsRequired = false;

    XCastImplementation::XCastImplementation() : _adminLock()
    {
        LOGINFO("##### API VER[%d : %d : %d] #####", API_VERSION_NUMBER_MAJOR,API_VERSION_NUMBER_MINOR,API_VERSION_NUMBER_PATCH);
        m_locateCastTimer.connect( bind( &XCastImplementation::onLocateCastTimer, this ));
    }

    XCastImplementation::~XCastImplementation()
    {
        Deinitialize();
        if (nullptr != mShell)
        {
            mShell->Release();
            mShell = nullptr;
        }
    }

    void XCastImplementation::Register(Exchange::IXCast::INotification* sink)
    {
        _adminLock.Lock();

        // Make sure a sink is not registered multiple times.
        ASSERT(std::find(_notificationClients.begin(), _notificationClients.end(), sink) == _notificationClients.end());

        _notificationClients.push_back(sink);
        sink->AddRef();

        _adminLock.Unlock();

        TRACE_L1("Registered a sink on the xcast inprocess %p", sink);
    }

    void XCastImplementation::Unregister(Exchange::IXCast::INotification* sink)
    {
        _adminLock.Lock();
        std::list<Exchange::IXCast::INotification*>::iterator index(std::find(_notificationClients.begin(), _notificationClients.end(), sink));

        if (index != _notificationClients.end()) {
            (*index)->Release();
            _notificationClients.erase(index);
            TRACE_L1("Unregistered a sink on the xcast inprocess %p", sink);
        }
        _adminLock.Unlock();
    }

    uint32_t XCastImplementation::Initialize(bool networkStandbyMode)
    {
        if(nullptr == m_xcast_manager)
        {
            m_networkStandbyMode = networkStandbyMode;
            m_xcast_manager  = XCastManager::getInstance();
            if(nullptr != m_xcast_manager)
            {
                m_xcast_manager->setService(this);
                if( false == connectToGDialService())
                {
                    startTimer(LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS);
                }
            }
        }
        return Core::ERROR_NONE;
    }

    void XCastImplementation::Deinitialize(void)
    {
        if(nullptr != m_xcast_manager)
        {
            stopTimer();
            m_xcast_manager->shutdown();
            m_xcast_manager = nullptr;
        }
    }

    uint32_t XCastImplementation::Configure(PluginHost::IShell* service)
    {
        LOGINFO("Configuring XCast");
        ASSERT(service != nullptr);
        mShell = service;
        mShell->AddRef();
        return Core::ERROR_NONE;
    }

    uint32_t XCastImplementation::enableCastService(string friendlyname,bool enableService) const
    {
        LOGINFO("XcastService::enableCastService: ARGS = %s : %d", friendlyname.c_str(), enableService);
        if (nullptr != m_xcast_manager)
        {
            m_xcast_manager->enableCastService(friendlyname,enableService);
        }
        xcastEnableCache = enableService;
        friendlyNameCache = friendlyname;
        return Core::ERROR_NONE;
    }

    uint32_t XCastImplementation::applicationStateChanged(const string& appName, const string& appstate, const string& appId, const string& error) const
    {
        LOGINFO("ApplicationStateChanged  ARGS = %s : %s : %s : %s ", appName.c_str(), appId.c_str() , appstate.c_str() , error.c_str());
        uint32_t status = Core::ERROR_GENERAL;
        if(!appName.empty() && !appstate.empty() && (nullptr != m_xcast_manager))
        {
            LOGINFO("XcastService::ApplicationStateChanged  ARGS = %s : %s : %s : %s ", appName.c_str(), appId.c_str() , appstate.c_str() , error.c_str());
            m_xcast_manager->applicationStateChanged(appName, appstate, appId, error);
            status = Core::ERROR_NONE;
        }
        return status;
    }

    uint32_t XCastImplementation::getProtocolVersion(std::string &protocolVersion) const
    {
        LOGINFO("XcastService::getProtocolVersion");
        if (nullptr != m_xcast_manager)
        {
            protocolVersion = m_xcast_manager->getProtocolVersion();
        }
        return Core::ERROR_NONE;
    }

    uint32_t XCastImplementation::registerApplications(Exchange::IXCast::IApplicationInfoIterator* const appLists)
    {
        LOGINFO("XcastService::registerApplications");
        std::vector <DynamicAppConfig*> appConfigListTemp;
        uint32_t status = Core::ERROR_GENERAL;

        if ((nullptr != m_xcast_manager) && (appLists))
        {
            Exchange::IXCast::ApplicationInfo entry{};

            while (appLists->Next(entry) == true)
            {
                DynamicAppConfig* pDynamicAppConfig = (DynamicAppConfig*) malloc (sizeof(DynamicAppConfig));
                if (pDynamicAppConfig)
                {
                    memset ((void*)pDynamicAppConfig, '0', sizeof(DynamicAppConfig));
                    memset (pDynamicAppConfig->appName, '\0', sizeof(pDynamicAppConfig->appName));
                    memset (pDynamicAppConfig->prefixes, '\0', sizeof(pDynamicAppConfig->prefixes));
                    memset (pDynamicAppConfig->cors, '\0', sizeof(pDynamicAppConfig->cors));
                    memset (pDynamicAppConfig->query, '\0', sizeof(pDynamicAppConfig->query));
                    memset (pDynamicAppConfig->payload, '\0', sizeof(pDynamicAppConfig->payload));

                    strncpy (pDynamicAppConfig->appName, entry.appName.c_str(), sizeof(pDynamicAppConfig->appName) - 1);
                    strncpy (pDynamicAppConfig->prefixes, entry.prefixes.c_str(), sizeof(pDynamicAppConfig->prefixes) - 1);
                    strncpy (pDynamicAppConfig->cors, entry.cors.c_str(), sizeof(pDynamicAppConfig->cors) - 1);
                    pDynamicAppConfig->allowStop = entry.allowStop;
                    strncpy (pDynamicAppConfig->query, entry.query.c_str(), sizeof(pDynamicAppConfig->query) - 1);
                    strncpy (pDynamicAppConfig->payload, entry.payload.c_str(), sizeof(pDynamicAppConfig->payload) - 1);
                    appConfigListTemp.push_back (pDynamicAppConfig);
                }
            }
            m_isDynamicRegistrationsRequired = true;
            m_xcast_manager->registerApplications(appConfigListTemp);
            {
                lock_guard<mutex> lck(m_appConfigMutex);
                for (DynamicAppConfig* pDynamicAppConfigOld : appConfigListCache)
                {
                    free (pDynamicAppConfigOld);
                    pDynamicAppConfigOld = NULL;
                }
                appConfigListCache.clear();
                appConfigListCache = appConfigListTemp;
                dumpDynamicAppCacheList(string("registeredAppsFromUser"), appConfigListCache);
            }
            status = Core::ERROR_NONE;
        }
        return status;
    }

    void XCastImplementation::dumpDynamicAppCacheList(string strListName, std::vector<DynamicAppConfig*> appConfigList)
    {
        LOGINFO ("=================Current Apps[%s] size[%d] ===========================", strListName.c_str(), (int)appConfigList.size());
        for (DynamicAppConfig* pDynamicAppConfig : appConfigList)
        {
            LOGINFO ("Apps: appName:%s, prefixes:%s, cors:%s, allowStop:%d, query:%s, payload:%s",
                    pDynamicAppConfig->appName,
                    pDynamicAppConfig->prefixes,
                    pDynamicAppConfig->cors,
                    pDynamicAppConfig->allowStop,
                    pDynamicAppConfig->query,
                    pDynamicAppConfig->payload);
        }
        LOGINFO ("=================================================================");
    }

    uint32_t XCastImplementation::setNetworkStandbyMode(bool nwStandbymode)
    {
        LOGINFO("nwStandbymode: %d", nwStandbymode);
        if (nullptr != m_xcast_manager)
        {
            m_xcast_manager->setNetworkStandbyMode(nwStandbymode);
            m_networkStandbyMode = nwStandbymode;
        }
        return Core::ERROR_NONE;
    }

    void XCastImplementation::dispatchEvent(Event event, string callsign, const JsonObject &params)
    {
        Core::IWorkerPool::Instance().Submit(Job::Create(this, event, callsign, params));
    }

    void XCastImplementation::Dispatch(Event event, string callsign, const JsonObject params)
    {
        _adminLock.Lock();
        std::list<Exchange::IXCast::INotification*>::iterator index(_notificationClients.begin());
        while (index != _notificationClients.end())
        {
            switch(event)
            {
                case LAUNCH_REQUEST_WITH_PARAMS:
                {
                    string appName = params["appName"].String();
                    string strPayLoad = params["strPayLoad"].String();
                    string strQuery = params["strQuery"].String();
                    string strAddDataUrl = params["strAddDataUrl"].String();
                    (*index)->onApplicationLaunchRequestWithLaunchParam(appName,strPayLoad,strQuery,strAddDataUrl);
                }
                break;
                case LAUNCH_REQUEST:
                {
                    string appName = params["appName"].String();
                    string parameter = params["parameter"].String();
                    (*index)->onApplicationLaunchRequest(appName,parameter);
                }
                break;
                case STOP_REQUEST:
                {
                    string appName = params["appName"].String();
                    string appId = params["appId"].String();
                    (*index)->onApplicationStopRequest(appName,appId);
                }
                break;
                case HIDE_REQUEST:
                {
                    string appName = params["appName"].String();
                    string appId = params["appId"].String();
                    (*index)->onApplicationHideRequest(appName,appId);
                }
                break;
                case STATE_REQUEST:
                {
                    string appName = params["appName"].String();
                    string appId = params["appId"].String();
                    (*index)->onApplicationStateRequest(appName,appId);
                }
                break;
                case RESUME_REQUEST:
                {
                    string appName = params["appName"].String();
                    string appId = params["appId"].String();
                    (*index)->onApplicationResumeRequest(appName,appId);
                }
                break;
                case UPDATE_POWERSTATE:
                {
                    string powerState = params["powerstate"].String();
                    (*index)->onUpdatePowerStateRequest(powerState);
                }
                break;
                default: break;
            }
            ++index;
        }
        _adminLock.Unlock();
    }

    void XCastImplementation::onXcastApplicationLaunchRequestWithLaunchParam (string appName, string strPayLoad, string strQuery, string strAddDataUrl)
    {
        LOGINFO("Notify LaunchRequestWithParam, appName: %s, strPayLoad: %s, strQuery: %s, strAddDataUrl: %s",
                appName.c_str(),strPayLoad.c_str(),strQuery.c_str(),strAddDataUrl.c_str());
        JsonObject params;
        params["appName"]  = appName.c_str();
        params["strPayLoad"]  = strPayLoad.c_str();
        params["strQuery"]  = strQuery.c_str();
        params["strAddDataUrl"]  = strAddDataUrl.c_str();
        dispatchEvent(LAUNCH_REQUEST_WITH_PARAMS, "", params);
    }

    void XCastImplementation::onXcastApplicationLaunchRequest(string appName, string parameter)
    {
        LOGINFO("Notify LaunchRequest, appName: %s, parameter: %s",appName.c_str(),parameter.c_str());
        JsonObject params;
        params["appName"]  = appName.c_str();
        params["parameter"]  = parameter.c_str();
        dispatchEvent(LAUNCH_REQUEST, "", params);
    }

    void XCastImplementation::onXcastApplicationStopRequest(string appName, string appId)
    {
        LOGINFO("Notify StopRequest, appName: %s, appId: %s",appName.c_str(),appId.c_str());
        JsonObject params;
        params["appName"]  = appName.c_str();
        params["appId"]  = appId.c_str();
        dispatchEvent(STOP_REQUEST, "", params);
    }

    void XCastImplementation::onXcastApplicationHideRequest(string appName, string appId)
    {
        LOGINFO("Notify StopRequest, appName: %s, appId: %s",appName.c_str(),appId.c_str());
        JsonObject params;
        params["appName"]  = appName.c_str();
        params["appId"]  = appId.c_str();
        dispatchEvent(HIDE_REQUEST, "", params);
    }

    void XCastImplementation::onXcastApplicationResumeRequest(string appName, string appId)
    {
        LOGINFO("Notify StopRequest, appName: %s, appId: %s",appName.c_str(),appId.c_str());
        JsonObject params;
        params["appName"]  = appName.c_str();
        params["appId"]  = appId.c_str();
        dispatchEvent(RESUME_REQUEST, "", params);
    }

    void XCastImplementation::onXcastApplicationStateRequest(string appName, string appId)
    {
        LOGINFO("Notify StopRequest, appName: %s, appId: %s",appName.c_str(),appId.c_str());
        JsonObject params;
        params["appName"]  = appName.c_str();
        params["appId"]  = appId.c_str();
        dispatchEvent(STATE_REQUEST, "", params);
    }

    void XCastImplementation::onXcastUpdatePowerStateRequest(string powerState)
    {
        LOGINFO("Notify updatePowerState, state: %s",powerState.c_str());
        JsonObject params;
        params["powerstate"]  = powerState.c_str();
        dispatchEvent(UPDATE_POWERSTATE, "", params);
    }

    void XCastImplementation::onGDialServiceStopped(void)
    {
        LOGINFO("Timer triggered to monitor the GDial, check after 5sec");
        startTimer(LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS);
    }

    bool XCastImplementation::connectToGDialService(void)
    {
        std::string interface,ipaddress;
        bool status = false;

        getDefaultNameAndIPAddress(interface,ipaddress);
        if (!interface.empty())
        {
            status = m_xcast_manager->initialize(interface,m_networkStandbyMode);
            if( true == status)
            {
                m_activeInterfaceName = interface;
            }
        }
        LOGINFO("GDialService[%u]IF[%s]IP[%s]",status,interface.c_str(),ipaddress.c_str());
        return status;
    }

    std::string XCastImplementation::getSecurityToken()
    {
        if (nullptr == mShell)
        {
            return (std::string(""));
        }

        std::string token;
        auto security = mShell->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
        if (nullptr != security)
        {
            std::string payload = "http://localhost";
            if (security->CreateToken(static_cast<uint16_t>(payload.length()),
                                        reinterpret_cast<const uint8_t *>(payload.c_str()),
                                        token) == Core::ERROR_NONE)
            {
                LOGINFO("got security token - %s", token.empty() ? "" : token.c_str());
            }
            else
            {
                LOGERR("failed to get security token");
            }
            security->Release();
        }
        else
        {
            LOGERR("No security agent\n");
        }

        std::string query = "token=" + token;
        Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));
        return query;
    }

    // Thunder plugins communication
    void XCastImplementation::getThunderPlugins()
    {
        string token = getSecurityToken();

        if (nullptr == m_ControllerObj)
        {
            if(token.empty())
            {
                m_ControllerObj = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>("", "", false);
            }
            else
            {
                m_ControllerObj = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>("","", false, token);
            }

            if (nullptr != m_ControllerObj)
            {
                LOGINFO("JSONRPC: Controller: initialization ok");
                bool isSubscribed = false;
                auto ev_ret = m_ControllerObj->Subscribe<JsonObject>(THUNDER_RPC_TIMEOUT, _T("statechange"),&XCastImplementation::eventHandler_pluginState,this);
                if (ev_ret == Core::ERROR_NONE)
                {
                    LOGINFO("Controller - statechange event subscribed");
                    isSubscribed = true;
                }
                else
                {
                    LOGERR("Controller - statechange event failed to subscribe : %d",ev_ret);
                }

                if (!isPluginActivated(NETWORK_CALLSIGN_VER))
                {
                    activatePlugin(NETWORK_CALLSIGN_VER);
                    _networkPluginState = PLUGIN_DEACTIVATED;
                }
                else
                {
                    _networkPluginState = PLUGIN_ACTIVATED;
                }

                if (false == isSubscribed)
                {
                    delete m_ControllerObj;
                    m_ControllerObj = nullptr;
                }
            }
            else
            {
                LOGERR("Unable to get Controller obj");
            }
        }

        if (nullptr == m_NetworkPluginObj)
        {
            std::string callsign = NETWORK_CALLSIGN_VER;
            if(token.empty())
            {
                m_NetworkPluginObj = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(NETWORK_CALLSIGN_VER),"");
            }
            else
            {
                m_NetworkPluginObj = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(NETWORK_CALLSIGN_VER),"", false, token);
            }
 
            if (nullptr == m_NetworkPluginObj)
            {
                LOGERR("JSONRPC: %s: initialization failed", NETWORK_CALLSIGN_VER);
            }
            else
            {
                LOGINFO("JSONRPC: %s: initialization ok", NETWORK_CALLSIGN_VER);
                // Network monitor so we can know ip address of host inside container
                if(m_NetworkPluginObj)
                {
                    bool isSubscribed = false;
                    auto ev_ret = m_NetworkPluginObj->Subscribe<JsonObject>(THUNDER_RPC_TIMEOUT, _T("onDefaultInterfaceChanged"), &XCastImplementation::eventHandler_onDefaultInterfaceChanged,this);
                    if ( Core::ERROR_NONE == ev_ret )
                    {
                        LOGINFO("Network - Default Interface changed event : subscribed");
                        ev_ret = m_NetworkPluginObj->Subscribe<JsonObject>(THUNDER_RPC_TIMEOUT, _T("onIPAddressStatusChanged"), &XCastImplementation::eventHandler_ipAddressChanged,this);
                        if ( Core::ERROR_NONE == ev_ret )
                        {
                            LOGINFO("Network - IP address status changed event : subscribed");
                            isSubscribed = true;
                        }
                        else
                        {
                            LOGERR("Network - IP address status changed event : failed to subscribe : %d", ev_ret);
                        }
                    }
                    else
                    {
                        LOGERR("Network - Default Interface changed event : failed to subscribe : %d", ev_ret);
                    }
                    if (false == isSubscribed)
                    {
                        LOGERR("Network events subscription failed");
                        delete m_NetworkPluginObj;
                        m_NetworkPluginObj = nullptr;
                    }
                }
            }
        }
        LOGINFO("Exiting..!!!");
    }

    void XCastImplementation::eventHandler_pluginState(const JsonObject& parameters)
    {
        LOGINFO("Plugin state changed");

        if( 0 == strncmp(parameters["callsign"].String().c_str(), NETWORK_CALLSIGN_VER, parameters["callsign"].String().length()))
        {
            if ( 0 == strncmp( parameters["state"].String().c_str(),"Deactivated", parameters["state"].String().length()))
            {
                LOGINFO("%s plugin got deactivated with reason : %s",parameters["callsign"].String().c_str(), parameters["reason"].String().c_str());
                _instance->activatePlugin(parameters["callsign"].String());
            }
        }
    }

    int XCastImplementation::activatePlugin(string callsign)
    {
        JsonObject result, params;
        params["callsign"] = callsign;
        int rpcRet = Core::ERROR_GENERAL;
        if (nullptr != m_ControllerObj)
        {
            rpcRet =  m_ControllerObj->Invoke("activate", params, result);
            if(Core::ERROR_NONE == rpcRet)
                {
                LOGINFO("Activated %s plugin", callsign.c_str());
            }
            else
            {
                LOGERR("Could not activate %s plugin.  Failed with %d", callsign.c_str(), rpcRet);
            }
        }
        else
        {
            LOGERR("Controller not active");
        }
        return rpcRet;
    }

    int XCastImplementation::deactivatePlugin(string callsign)
    {
        JsonObject result, params;
        params["callsign"] = callsign;
        int rpcRet = Core::ERROR_GENERAL;
        if (nullptr != m_ControllerObj)
        {
            rpcRet =  m_ControllerObj->Invoke("deactivate", params, result);
            if(Core::ERROR_NONE == rpcRet)
            {
                LOGINFO("Deactivated %s plugin", callsign.c_str());
            }
            else
            {
                LOGERR("Could not deactivate %s plugin.  Failed with %d", callsign.c_str(), rpcRet);
            }
        }
        else
        {
            LOGERR("Controller not active");
        }
        return rpcRet;
    }

    bool XCastImplementation::isPluginActivated(string callsign)
    {
        std::string method = "status@" + callsign;
        bool isActive = false;
        Core::JSON::ArrayType<PluginHost::MetaData::Service> response;
        if (nullptr != m_ControllerObj)
        {
            int ret  = m_ControllerObj->Get(THUNDER_RPC_TIMEOUT, method, response);
            isActive = (ret == Core::ERROR_NONE && response.Length() > 0 && response[0].JSONState == PluginHost::IShell::ACTIVATED);
            LOGINFO("Plugin \"%s\" is %s, error=%d", callsign.c_str(), isActive ? "active" : "not active", ret);
        }
        else
        {
            LOGERR("Controller not active");
        }
        return isActive;
    }

    void XCastImplementation::startTimer(int interval)
    {
        stopTimer();
        m_locateCastTimer.start(interval);
    }

    void XCastImplementation::stopTimer()
    {
        if (m_locateCastTimer.isActive())
        {
            m_locateCastTimer.stop();
        }
    }

    bool XCastImplementation::isTimerActive()
    {
        return (m_locateCastTimer.isActive());
    }

    //Timer Functions
    void XCastImplementation::onLocateCastTimer()
    {
        if( false == connectToGDialService())
        {
            LOGINFO("Retry after 10 sec...");
            m_locateCastTimer.setInterval(LOCATE_CAST_SECOND_TIMEOUT_IN_MILLIS);
            return ;
        }
        stopTimer();

        if ((NULL != m_xcast_manager) && m_isDynamicRegistrationsRequired )
        {
            std::vector<DynamicAppConfig*> appConfigList;
            lock_guard<mutex> lck(m_appConfigMutex);
            appConfigList = appConfigListCache;
            dumpDynamicAppCacheList(string("CachedAppsFromTimer"), appConfigList);
            LOGINFO("> calling registerApplications");
            m_xcast_manager->registerApplications (appConfigList);
        }
        else {
            LOGINFO("m_xcast_manager: %p: m_isDynamicRegistrationsRequired[%u]",
                    m_xcast_manager,
                    m_isDynamicRegistrationsRequired);
        }
        m_xcast_manager->enableCastService(friendlyNameCache,xcastEnableCache);
        LOGINFO("XCast::onLocateCastTimer : Timer still active ? %d ",m_locateCastTimer.isActive());
    }

    bool XCastImplementation::getDefaultNameAndIPAddress(std::string& interface, std::string& ipaddress)
    {
        // Read host IP from thunder service and save it into external_network.json
        JsonObject Params, Result, Params0, Result0;
        bool returnValue = false;

        getThunderPlugins();

        if (nullptr == m_NetworkPluginObj)
        {
            LOGINFO("WARN::Unable to get Network plugin handle not yet");
            return false;
        }

        uint32_t ret = m_NetworkPluginObj->Invoke<JsonObject, JsonObject>(THUNDER_RPC_TIMEOUT, _T("getDefaultInterface"), Params0, Result0);
        if (Core::ERROR_NONE == ret)
        {
            if (Result0["success"].Boolean())
            {
                interface = Result0["interface"].String();
            }
            else
            {
                LOGERR("XCastImplementation: failed to load interface");
            }
        }

        Params.Set(_T("interface"), interface);
        Params.Set(_T("ipversion"), string("IPv4"));

        ret = m_NetworkPluginObj->Invoke<JsonObject, JsonObject>(THUNDER_RPC_TIMEOUT, _T("getIPSettings"), Params, Result);
        if (Core::ERROR_NONE == ret)
        {
            if (Result["success"].Boolean())
            {
                ipaddress = Result["ipaddr"].String();
                LOGINFO("ipAddress = %s",ipaddress.c_str());
                returnValue = true;
            }
            else
            {
                LOGERR("getIPSettings failed");
            }
        }
        else
        {
            LOGERR("Failed to invoke method \"getIPSettings\". Error: %d",ret);
        }
        return returnValue;
    }

    void XCastImplementation::updateNWConnectivityStatus(std::string nwInterface, bool nwConnected, std::string ipaddress)
    {
        bool status = false;
        if(nwConnected)
        {
            if(nwInterface.compare("ETHERNET")==0){
                LOGINFO("Connectivity type Ethernet");
                status = true;
            }
            else if(nwInterface.compare("WIFI")==0){
                LOGINFO("Connectivity type WIFI");
                status = true;
            }
            else{
                LOGERR("Connectivity type Unknown");
            }
        }
        else
        {
            LOGERR("Connectivity type Unknown");
        }
        if (!m_locateCastTimer.isActive())
        {
            if (status)
            {
                if ((0 != nwInterface.compare(m_activeInterfaceName)) ||
                    ((0 == nwInterface.compare(m_activeInterfaceName)) && !ipaddress.empty()))
                {
                    if (m_xcast_manager)
                    {
                        LOGINFO("Stopping GDialService");
                        m_xcast_manager->deinitialize();
                    }
                    LOGINFO("Timer started to monitor active interface");
                    startTimer(LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS);
                }
            }
        }
    }

    void XCastImplementation::eventHandler_onDefaultInterfaceChanged(const JsonObject& parameters)
    {
        std::string oldInterfaceName, newInterfaceName;
        oldInterfaceName = parameters["oldInterfaceName"].String();
        newInterfaceName = parameters["newInterfaceName"].String();

        LOGINFO("XCast onDefaultInterfaceChanged, old interface: %s, new interface: %s", oldInterfaceName.c_str(), newInterfaceName.c_str());
        updateNWConnectivityStatus(newInterfaceName.c_str(), true);
    }

    void XCastImplementation::eventHandler_ipAddressChanged(const JsonObject& parameters)
    {
        if(parameters["status"].String() == "ACQUIRED")
        {
            string interface = parameters["interface"].String();
            string ipv4Address = parameters["ip4Address"].String();
            bool isAcquired = false;
            if (!ipv4Address.empty())
            {
                isAcquired = true;
            }
            updateNWConnectivityStatus(interface.c_str(), isAcquired, ipv4Address.c_str());
        }
    }
}  // namespace Plugin
}  // namespace WPEFramework
