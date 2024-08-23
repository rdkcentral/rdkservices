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

#include "XCast.h"
#include "tracing/Logging.h"
#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"
#ifdef RFC_ENABLED
#include "rfcapi.h"
#endif //RFC_ENABLED
#include <syscall.h>
#include <cstring>
//#include "RtXcastConnector.h"
using namespace std;

// Events
// com.comcast.xcast_1
#define EVT_ON_LAUNCH_REQUEST         "onApplicationLaunchRequest"
#define EVT_ON_HIDE_REQUEST           "onApplicationHideRequest"
#define EVT_ON_RESUME_REQUEST         "onApplicationResumeRequest"
#define EVT_ON_STOP_REQUEST           "onApplicationStopRequest"
#define EVT_ON_STATE_REQUEST          "onApplicationStateRequest"
//Methods
#define METHOD_ON_APPLICATION_STATE_CHANGED  "onApplicationStateChanged"
#define METHOD_GET_API_VERSION_NUMBER        "getApiVersionNumber"
#define METHOD_SET_ENABLED "setEnabled"
#define METHOD_GET_ENABLED "getEnabled"
#define METHOD_GET_STANDBY_BEHAVIOR "getStandbyBehavior"
#define METHOD_SET_STANDBY_BEHAVIOR "setStandbyBehavior"
#define METHOD_GET_FRIENDLYNAME "getFriendlyName"
#define METHOD_SET_FRIENDLYNAME "setFriendlyName"
#define METHOD_GET_PROTOCOLVERSION "getProtocolVersion"

#define METHOD_REG_APPLICATIONS "registerApplications"
#define METHOD_UNREG_APPLICATIONS "unregisterApplications"

#define LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS  5000  //5 seconds
#define LOCATE_CAST_SECOND_TIMEOUT_IN_MILLIS 15000  //15 seconds
#define LOCATE_CAST_THIRD_TIMEOUT_IN_MILLIS  30000  //30 seconds
#define LOCATE_CAST_FINAL_TIMEOUT_IN_MILLIS  60000  //60 seconds

#define SERVER_DETAILS  "127.0.0.1:9998"
#define SYSTEM_CALLSIGN "org.rdk.System"
#define SYSTEM_CALLSIGN_VER SYSTEM_CALLSIGN".1"
#define SECURITY_TOKEN_LEN_MAX 1024
#define THUNDER_RPC_TIMEOUT 2000

/*
 * The maximum additionalDataUrl length
 */
#define DIAL_MAX_ADDITIONALURL (1024)

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 17

namespace WPEFramework {

namespace {

    static Plugin::Metadata<Plugin::XCast> metadata(
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

/**
 *Register XCast module as wpeframework plugin
 **/
SERVICE_REGISTRATION(XCast, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

//static int locateCastObjectRetryCount = 0;
#ifdef XCAST_ENABLED_BY_DEFAULT
bool XCast::m_xcastEnable = true;
#else
bool XCast::m_xcastEnable = false;
#endif
string XCast::m_friendlyName = "";
#ifdef XCAST_ENABLED_BY_DEFAULT_IN_STANDBY
bool XCast::m_standbyBehavior = true;
#else
bool XCast::m_standbyBehavior = false;
#endif

IARM_Bus_PWRMgr_PowerState_t XCast::m_powerState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;

XCast::XCast() : PluginHost::JSONRPC()
        , _notification(this)
        , m_apiVersionNumber(API_VERSION_NUMBER_MAJOR)
{

}

XCast::~XCast()
{
        LOGINFO("Xcast: Dtor ");
        if (nullptr != m_SystemPluginObj)
        {
            delete m_SystemPluginObj;
            m_SystemPluginObj = nullptr;
        }
        _service = nullptr;
}

void XCast::RegisterAll()
{
    LOGINFO("XcastService::Register methods and create onLocateCastTimer ");
    Register(METHOD_GET_API_VERSION_NUMBER, &XCast::getApiVersionNumber, this);
    Register(METHOD_ON_APPLICATION_STATE_CHANGED , &XCast::applicationStateChanged, this);
    Register(METHOD_SET_ENABLED, &XCast::setEnabled, this);
    Register(METHOD_GET_ENABLED, &XCast::getEnabled, this);
    Register(METHOD_GET_STANDBY_BEHAVIOR, &XCast::getStandbyBehavior, this);
    Register(METHOD_SET_STANDBY_BEHAVIOR, &XCast::setStandbyBehavior, this);
    Register(METHOD_GET_FRIENDLYNAME, &XCast::getFriendlyName, this);
    Register(METHOD_SET_FRIENDLYNAME, &XCast::setFriendlyName, this);
    Register(METHOD_REG_APPLICATIONS, &XCast::registerApplications, this);
    Register(METHOD_UNREG_APPLICATIONS, &XCast::unregisterApplications, this);
    Register(METHOD_GET_PROTOCOLVERSION, &XCast::getProtocolVersion, this);

    //m_locateCastTimer.connect( bind( &XCast::onLocateCastTimer, this ));
}

void XCast::InitializeIARM()
{
    if (Utils::IARM::init())
    {
        IARM_Result_t res;
        IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED, powerModeChange) );
        IARM_Bus_PWRMgr_GetPowerState_Param_t param;
        res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_GetPowerState,
                (void *)&param, sizeof(param));
        if (res == IARM_RESULT_SUCCESS)
        {
            m_powerState = param.curState;
        }
        LOGINFO("XcastService::m_powerState:%d ",m_powerState);
    }
}

void XCast::DeinitializeIARM()
{
    if (Utils::IARM::isConnected())
    {
        IARM_Result_t res;
        IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED, powerModeChange) );
    }
    Unregister(METHOD_GET_API_VERSION_NUMBER);
    Unregister(METHOD_ON_APPLICATION_STATE_CHANGED);
    Unregister(METHOD_SET_ENABLED);
    Unregister(METHOD_GET_ENABLED);
    Unregister(METHOD_GET_STANDBY_BEHAVIOR);
    Unregister(METHOD_SET_STANDBY_BEHAVIOR);
    Unregister(METHOD_GET_FRIENDLYNAME);
    Unregister(METHOD_SET_FRIENDLYNAME);

}
void XCast::powerModeChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    if (strcmp(owner, IARM_BUS_PWRMGR_NAME)  == 0) {
        if (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED ) {
            IARM_Bus_PWRMgr_EventData_t *param = (IARM_Bus_PWRMgr_EventData_t *)data;
            LOGINFO("Event IARM_BUS_PWRMGR_EVENT_MODECHANGED: State Changed %d -- > %d\r",
                    param->data.state.curState, param->data.state.newState);
            m_powerState = param->data.state.newState;
            LOGWARN("creating worker thread for threadPowerModeChangeEvent m_powerState :%d",m_powerState);
            std::thread powerModeChangeThread = std::thread(threadPowerModeChangeEvent);
            powerModeChangeThread.detach();
        }
    }
}

const string XCast::Initialize(PluginHost::IShell *service)
{
    ASSERT(_service == nullptr);

    _connectionId = 0;
    _service = service;
    _skipURL = static_cast<uint8_t>(_service->WebPrefix().length());

    _service->Register(&_notification);

    _xcast = _service->Root<Exchange::IXCast>(_connectionId, 5000, _T("XCastImplementation"));

    std::string message;
    if(_xcast != nullptr) {
        #ifndef UNIT_TESTING
            ASSERT(_connectionId != 0);
        #endif

        PluginHost::IStateControl* stateControl(_xcast->QueryInterface<PluginHost::IStateControl>());

        if (stateControl == nullptr) {
            _xcast->Release();
            _xcast = nullptr;
        } else {
            if (stateControl->Configure(_service) != Core::ERROR_NONE) {
                _xcast->Release();
                _xcast = nullptr;
            }
            stateControl->Release();
        }
    }

    if(_xcast != nullptr) {
        _xcast->Register(&_notification);
        RegisterAll();
        InitializeIARM();

        getSystemPlugin();
        // subscribe for event
        m_SystemPluginObj->Subscribe<JsonObject>(1000, "onFriendlyNameChanged", &XCast::onFriendlyNameUpdateHandler, this);
        if (Core::ERROR_NONE == updateSystemFriendlyName())
        {
            LOGINFO("XCast::Initialize m_friendlyName:  %s\n ",m_friendlyName.c_str());
        }
    }
    else
    {
        message = _T("XCast could not be instantiated.");
        _service->Unregister(&_notification);
        _service = nullptr;
    }

    return message;
}

void XCast::Deinitialize(PluginHost::IShell* service)
{
    ASSERT(_service == service);
    ASSERT(_xcast != nullptr);
    
    if(_xcast)
        _xcast->Unregister(&_notification);

    if(_service)
        _service->Unregister(&_notification);

    if(_xcast) {
        if(_xcast->Release() != Core::ERROR_DESTRUCTION_SUCCEEDED) {
            ASSERT(_connectionId != 0);
            LOGINFO("XCast Plugin is not properly destructed. %d", _connectionId);

            if(_service) {
                RPC::IRemoteConnection* connection(_service->RemoteConnection(_connectionId));

                // The process can disappear in the meantime...
                if (connection != nullptr) {
                    // But if it did not dissapear in the meantime, forcefully terminate it. Shoot to kill :-)
                    connection->Terminate();
                    connection->Release();
                }
            }
        }
    }

    _xcast = nullptr;
    _service = nullptr;
}

void XCast::Deactivated(RPC::IRemoteConnection* connection)
{
    if (connection->Id() == _connectionId) {
        ASSERT(_service != nullptr);
        LOGINFO("XCast::Deactivated - %p", this);
        Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
    }
}

uint32_t XCast::getApiVersionNumber(const JsonObject& parameters, JsonObject& response)
{
    LOGINFOMETHOD();
    response["version"] = m_apiVersionNumber;
    returnResponse(true);
}

uint32_t XCast::applicationStateChanged(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::ApplicationStateChanged () ");
    string app,id,state,error;
    bool returnStatus = false;
    getStringParameter("applicationName",app);
    getStringParameter("state", state);
    
    if (parameters.HasLabel("applicationId"))
    {
        getStringParameter("applicationId", id);
    }
    if (parameters.HasLabel("error"))
    {
        getStringParameter("error", error);
    }
    if (!app.empty() && !state.empty() && (nullptr != _xcast))
    {
        if (app == "NetflixApp")
            app = "Netflix";
        
        LOGINFO("XcastService::ApplicationStateChanged  ARGS = %s : %s : %s : %s ", app.c_str(), id.c_str() , state.c_str() , error.c_str());
        auto result = _xcast->applicationStateChanged(app,state,id,error);
        if (Core::ERROR_NONE == result)
        {
            returnStatus = true;
        }
    }
    returnResponse(returnStatus);
}

uint32_t XCast::setEnabled(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::setEnabled ");
    bool enabled = false,
        returnStatus = false;
    if (parameters.HasLabel("enabled"))
    {
        getBoolParameter("enabled", enabled);
    }
    else
    {
        returnResponse(false);
    }
    m_xcastEnable= enabled;
    if (m_xcastEnable && ( (m_standbyBehavior == true) || ((m_standbyBehavior == false)&&(m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON))))
    {
        enabled = true;
    }
    else
    {
        enabled = false;
    }

    if ( nullptr != _xcast )
    {
        auto result = _xcast->enableCastService(m_friendlyName,enabled);
        if (Core::ERROR_NONE == result)
        {
            returnStatus = true;
        }
    }
    returnResponse(returnStatus);
}

uint32_t XCast::getEnabled(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::getEnabled ");
    response["enabled"] = m_xcastEnable;
    returnResponse(true);
}

uint32_t XCast::setStandbyBehavior(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::setStandbyBehavior \n ");
    std::string paramStr;
    bool enabled = false;
    if (parameters.HasLabel("standbybehavior"))
    {
        getStringParameter("standbybehavior", paramStr);
        if(paramStr == "active")
            enabled = true;
    }
    else
    {
    returnResponse(false);
    }
    m_standbyBehavior = enabled;
    LOGINFO("XcastService::setStandbyBehavior m_standbyBehavior : %d", m_standbyBehavior);
    returnResponse(true);
}

uint32_t XCast::getStandbyBehavior(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::getStandbyBehavior m_standbyBehavior :%d",m_standbyBehavior);
    if(m_standbyBehavior)
        response["standbybehavior"] = "active";
    else
        response["standbybehavior"] = "inactive";

    returnResponse(true);
}

uint32_t XCast::setFriendlyName(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::setFriendlyName \n ");
    std::string paramStr;
    bool enabledStatus = false,
        returnStatus = false;

    if (parameters.HasLabel("friendlyname"))
    {
        getStringParameter("friendlyname",paramStr);
        if(nullptr != _xcast)
        {
            m_friendlyName = paramStr;
            LOGINFO("XcastService::setFriendlyName  :%s",m_friendlyName.c_str());
            if (m_xcastEnable && ( (m_standbyBehavior == true) || ((m_standbyBehavior == false)&&(m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON))))
            {
                enabledStatus = true;                
            }
            else
            {
                enabledStatus = false;
            }
            _xcast->enableCastService(m_friendlyName,enabledStatus);
            returnStatus = true;
        }
    }
    returnResponse(returnStatus);
}

uint32_t XCast::getFriendlyName(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::getFriendlyName :%s ",m_friendlyName.c_str());
    response["friendlyname"] = m_friendlyName;
    returnResponse(true);
}

void XCast::getSystemPlugin()
{
    LOGINFO("Entering..!!!");
    if(nullptr == m_SystemPluginObj)
    {
        string token;
        // TODO: use interfaces and remove token
        auto security = _service->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
        if (nullptr != security)
        {
            string payload = "http://localhost";
            if (security->CreateToken( static_cast<uint16_t>(payload.length()),
                                    reinterpret_cast<const uint8_t*>(payload.c_str()),
                                    token) == Core::ERROR_NONE)
            {
                LOGINFO("got security token\n");
            }
            else
            {
                LOGERR("failed to get security token\n");
            }
            security->Release();
        }
        else
        {
            LOGERR("No security agent\n");
        }

        string query = "token=" + token;
        Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));
        m_SystemPluginObj = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(_T(SYSTEM_CALLSIGN_VER), (_T(SYSTEM_CALLSIGN_VER)), false, query);
        if (nullptr == m_SystemPluginObj)
        {
            LOGERR("JSONRPC: %s: initialization failed", SYSTEM_CALLSIGN_VER);
        }
        else
        {
            LOGINFO("JSONRPC: %s: initialization ok", SYSTEM_CALLSIGN_VER);
        }
    }
    LOGINFO("Exiting..!!!");
}

int XCast::updateSystemFriendlyName()
{
    JsonObject params, Result;
    LOGINFO("Entering..!!!");

    if (nullptr == m_SystemPluginObj)
    {
        LOGERR("m_SystemPluginObj not yet instantiated");
        return Core::ERROR_GENERAL;
    }

    uint32_t ret = m_SystemPluginObj->Invoke<JsonObject, JsonObject>(THUNDER_RPC_TIMEOUT, _T("getFriendlyName"), params, Result);

    if (Core::ERROR_NONE == ret)
    {
        if (Result["success"].Boolean())
        {
            m_friendlyName = Result["friendlyName"].String();
        }
        else
        {
            ret = Core::ERROR_GENERAL;
            LOGERR("getSystemFriendlyName call failed");
        }
    }
    else
    {
        LOGERR("getiSystemFriendlyName call failed E[%u]", ret);
    }
    return ret;
}

uint32_t XCast::getProtocolVersion(const JsonObject& parameters, JsonObject& response)
{
    string protocolVersion;
    bool returnStatus = false;
    LOGINFO("XcastService::getProtocolVersion");
    if (nullptr != _xcast)
    {
        auto result = _xcast->getProtocolVersion(protocolVersion);
        if (result == Core::ERROR_NONE)
        {
            returnStatus = true;
            response["version"] = protocolVersion.c_str();
        }
    }
    returnResponse(returnStatus);
}

bool XCast::getEntryFromAppLaunchParamList (const char* appName, DynamicAppConfig& retAppConfig)
{
    bool isEntryFound = false;
    {lock_guard<mutex> lck(m_appConfigMutex);
        for (DynamicAppConfig* regAppLaunchParam : m_appConfigCache) {
            if (0 == strcmp (regAppLaunchParam->appName, appName)) {
                isEntryFound = true;
                strncpy (retAppConfig.appName, regAppLaunchParam->appName, sizeof(retAppConfig.appName));
                retAppConfig.appName[sizeof(retAppConfig.appName) - 1] = '\0';

                strncpy (retAppConfig.query, regAppLaunchParam->query, sizeof(retAppConfig.query));
                retAppConfig.query[sizeof(retAppConfig.query) - 1] = '\0';

                strncpy (retAppConfig.payload, regAppLaunchParam->payload, sizeof(retAppConfig.payload));
                retAppConfig.payload[sizeof(retAppConfig.payload) - 1] = '\0';
                break;
            }
        }
    }
    return isEntryFound;
}

void XCast::dumpDynamicAppConfigCache(string strListName, std::vector<DynamicAppConfig*> appConfigList) {
    /*Check if existing cache need to be updated*/
    std::vector<int> entriesTodelete;
    LOGINFO ("=================Current dynamic %s size: %d is:===========================", strListName.c_str(), (int)appConfigList.size());
    for (DynamicAppConfig* pDynamicAppConfig : appConfigList) {
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

bool XCast::deleteFromDynamicAppCache(vector<string>& appsToDelete) {
    bool ret = true;
    {lock_guard<mutex> lck(m_appConfigMutex);
        /*Check if existing cache need to be updated*/
        std::vector<int> entriesTodelete;
        for (string appNameToDelete : appsToDelete) {
            bool found = false;
            int index = 0;
            for (DynamicAppConfig* pDynamicAppConfigOld : m_appConfigCache) {
                if (0 == strcmp(pDynamicAppConfigOld->appName, appNameToDelete.c_str())){
                    entriesTodelete.push_back(index);
                    found = true;
                    break;
                }
                index ++;
            }
            if (!found) {
                LOGINFO("%s not existing in the dynamic cache", appNameToDelete.c_str());
            }
        }
        std::sort(entriesTodelete.begin(), entriesTodelete.end(), std::greater<int>());
        for (int indexToDelete : entriesTodelete) {
            LOGINFO("Going to delete the entry: %d from m_appConfigCache size: %d", indexToDelete, (int)m_appConfigCache.size());
            //Delete the old unwanted item here.
            DynamicAppConfig* pDynamicAppConfigOld = m_appConfigCache[indexToDelete];
            m_appConfigCache.erase (m_appConfigCache.begin()+indexToDelete);
            free (pDynamicAppConfigOld); pDynamicAppConfigOld = NULL;
        }
        entriesTodelete.clear();

    }
    //Even if requested app names not there return true.
    return ret;
}

bool XCast::deleteFromDynamicAppCache(JsonArray applications)
{
    bool ret = false;
    std::string itrName = "";
    vector<string> appsToDelete;
    for (int iIndex = 0; iIndex < applications.Length(); iIndex++) {
        itrName = applications[iIndex].String();
        LOGINFO("App name to delete: %s, size:%d", itrName.c_str(), (int)strlen (itrName.c_str()));
        appsToDelete.push_back(itrName);
    }
    //If empty list is passed, dynamic cache is cleared. This will clear static list also
    //Net result will be not app will be able to launch.
    if(!appsToDelete.size()){
        LOGINFO ("Empty unregister list is passed clearing the dynamic cache");
        {lock_guard<mutex> lck(m_appConfigMutex);
            m_appConfigCache.clear();
        }
        ret = true;
    } else {
        //Remove specified appl list from dynamic app cache
        ret = deleteFromDynamicAppCache (appsToDelete);
        appsToDelete.clear();
    }
    
    return ret;
}

void XCast::updateDynamicAppCache(JsonArray applications)
{
    LOGINFO("XcastService::UpdateDynamicAppCache");

    JsonObject itrApp;

    JsonArray jNames;
    std::string itrName = "";

    JsonArray jPrefixes;
    std::string itrPrefix = "";

    JsonArray jCors;
    std::string itrCor = "";

    JsonObject jProperties;
    bool jAllowStop = false;

    JsonObject jLaunchParam;
    std::string jQuery = "";
    std::string jPayload = "";

    std::vector <DynamicAppConfig*> appConfigList;
    if (applications.Length() != 0) {
        /* iterate over ints */
        LOGINFO("Applications:");

        for (int iIndex = 0; iIndex < applications.Length(); iIndex++) {
            std::vector <DynamicAppConfig*> appConfigListTemp;
            LOGINFO("Application: %d", iIndex);
            itrApp = applications[iIndex].Object();
            if (!itrApp.HasLabel("names")) {
                LOGINFO ("Invalid names format at application index %d. Skipping the application", iIndex);
                continue;
            }
            else {
                jNames = itrApp["names"].Array();
                for (int i = 0; i < jNames.Length(); i++) {
                    itrName = jNames[i].String().c_str();
                    LOGINFO("%s, size:%d", itrName.c_str(), (int)strlen (itrName.c_str()));
                    DynamicAppConfig* pDynamicAppConfig = (DynamicAppConfig*) malloc (sizeof(DynamicAppConfig));
                    memset ((void*)pDynamicAppConfig, '0', sizeof(DynamicAppConfig));
                    memset (pDynamicAppConfig->appName, '\0', sizeof(pDynamicAppConfig->appName));
                    strncpy (pDynamicAppConfig->appName, itrName.c_str(), sizeof(pDynamicAppConfig->appName) - 1);
                    memset (pDynamicAppConfig->prefixes, '\0', sizeof(pDynamicAppConfig->prefixes));
                    memset (pDynamicAppConfig->cors, '\0', sizeof(pDynamicAppConfig->cors));
                    memset (pDynamicAppConfig->query, '\0', sizeof(pDynamicAppConfig->query));
                    memset (pDynamicAppConfig->payload, '\0', sizeof(pDynamicAppConfig->payload));
                    appConfigListTemp.push_back (pDynamicAppConfig);
                }
            }

            if (!itrApp.HasLabel("prefixes")) {
                LOGINFO ("Invalid prefixes format at application index %d", iIndex);
            }
            else {
                jPrefixes = itrApp["prefixes"].Array();
                for (int i = 0; i < jPrefixes.Length(); i++) {
                    itrPrefix = jPrefixes[i].String().c_str();
                    LOGINFO("%s, size:%d", itrPrefix.c_str(), (int)strlen (itrPrefix.c_str()));
                    for (DynamicAppConfig* pDynamicAppConfig : appConfigListTemp) {
                        strncpy (pDynamicAppConfig->prefixes, itrPrefix.c_str(), sizeof(pDynamicAppConfig->prefixes) - 1);
                    }
                }
            }

            if (!itrApp.HasLabel("cors")) {
                LOGINFO ("Invalid cors format at application index %d. Skipping the application", iIndex);
                continue;
            }
            else {
                jCors = itrApp["cors"].Array();
                for (int i = 0; i < jCors.Length(); i++) {
                    itrCor = jCors[i].String().c_str();
                    LOGINFO("%s, size:%d", itrCor.c_str(), (int)strlen (itrCor.c_str()));
                    for (DynamicAppConfig* pDynamicAppConfig : appConfigListTemp) {
                        strncpy (pDynamicAppConfig->cors, itrCor.c_str(), sizeof(pDynamicAppConfig->cors) - 1);
                    }
                }
            }

            if (!itrApp.HasLabel("properties")) {
                LOGINFO ("Invalid property format at application index %d", iIndex);
            }
            else {
                jProperties = itrApp["properties"].Object();
                if (!jProperties.HasLabel("allowStop")) {
                    LOGINFO ("Invalid allowStop format at application index %d", iIndex);
                }
                else {
                    jAllowStop = jProperties["allowStop"].Boolean();
                    LOGINFO("allowStop: %d", jAllowStop);
                }
                for (DynamicAppConfig* pDynamicAppConfig : appConfigListTemp) {
                    pDynamicAppConfig->allowStop = jAllowStop;
                }
            }

            if (!itrApp.HasLabel("launchParameters")) {
                LOGINFO ("Invalid Launch param format at application index %d", iIndex);
            }
            else {
                jLaunchParam = itrApp["launchParameters"].Object();
                if (!jLaunchParam.HasLabel("query")) {
                    LOGINFO ("Invalid query format at application index %d", iIndex);
                }
                else {
                    jQuery = itrApp["query"].String();
                    if ("null" == jQuery || jQuery.empty()){
                        //Replace this condition with IsNull check of wpeframwork json
                        jQuery = "";
                    }
                    LOGINFO("query: %s, size:%d", jQuery.c_str(), (int)strlen (jQuery.c_str()));
                }
                if (!jLaunchParam.HasLabel("payload")) {
                    LOGINFO ("Invalid payload format at application index %d", iIndex);
                }
                else {
                    jPayload = itrApp["payload"].String();
                    if ("null" == jPayload || jPayload.empty()){
                        //Replace this condition with IsNull check of wpeframwork json
                        jPayload = "";
                    }
                    LOGINFO("payload: %s, size:%d", jPayload.c_str(), (int)strlen (jPayload.c_str()));
                }
                //Set launchParameters in list for later usage
                for (DynamicAppConfig* pDynamicAppConfig : appConfigListTemp) {
                    if (jLaunchParam.HasLabel("query")) {
                        strncpy (pDynamicAppConfig->query, jQuery.c_str(), sizeof(pDynamicAppConfig->query) - 1);
                    }
                    if (jLaunchParam.HasLabel("payload")) {
                        strncpy (pDynamicAppConfig->payload, jPayload.c_str(), sizeof(pDynamicAppConfig->payload) - 1);
                    }
                }

            }
            for (DynamicAppConfig* pDynamicAppConfig : appConfigListTemp) {
                appConfigList.push_back(pDynamicAppConfig);
            }
            appConfigListTemp.clear();
        }
        dumpDynamicAppConfigCache(string("appConfigList"), appConfigList);
        vector<string> appsToDelete;
        for (DynamicAppConfig* pDynamicAppConfig : appConfigList) {
            appsToDelete.push_back(string(pDynamicAppConfig->appName));
        }
        deleteFromDynamicAppCache (appsToDelete);

        LOGINFO("appConfigList count: %d", (int)appConfigList.size());
        //Update the new entries here.
        {lock_guard<mutex> lck(m_appConfigMutex);
            for (DynamicAppConfig* pDynamicAppConfig : appConfigList) {
                m_appConfigCache.push_back(pDynamicAppConfig);
            }
            LOGINFO("m_appConfigCache count: %d", (int)m_appConfigCache.size());
        }
        //Clear the tempopary list here
        appsToDelete.clear();
        appConfigList.clear();
    }
    dumpDynamicAppConfigCache(string("m_appConfigCache"), m_appConfigCache);
    return;
}

uint32_t XCast::registerApplications(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::registerApplications \n ");
    bool hasAppReq = parameters.HasLabel("applications"),
        returnStatus = false;
    if (hasAppReq)
    {
        LOGINFO ("\nInput string is:%s\n", parameters["applications"].String().c_str());

        if(nullptr != _xcast)
        {
            LOGINFO("%s:%d _xcast Not NULL", __FUNCTION__, __LINE__);
            /*Disable cast service before registering Applications*/
            _xcast->enableCastService(m_friendlyName,false);

            m_isDynamicRegistrationsRequired = true;
            //Register dynamic application list to app cache map
            updateDynamicAppCache(parameters["applications"].Array());
            std::vector<DynamicAppConfig*> appConfigList;
            {lock_guard<mutex> lck(m_appConfigMutex);
                appConfigList = m_appConfigCache;
            }
            dumpDynamicAppConfigCache(string("m_appConfigCache"), appConfigList);
            //Pass the dynamic cache to xdial process
            registerApplicationsInternal(m_appConfigCache);

            /*Reenabling cast service after registering Applications*/
            if (m_xcastEnable && ( (m_standbyBehavior == true) || ((m_standbyBehavior == false)&&(m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON)) ) ) {
                LOGINFO("Enable CastService  m_xcastEnable: %d m_standbyBehavior: %d m_powerState:%d", m_xcastEnable, m_standbyBehavior, m_powerState);
                _xcast->enableCastService(m_friendlyName,true);
            }
            else {
                LOGINFO("CastService not enabled m_xcastEnable: %d m_standbyBehavior: %d m_powerState:%d", m_xcastEnable, m_standbyBehavior, m_powerState);
            }
            returnStatus = true;
        }
    }
    returnResponse(returnStatus);
}

uint32_t XCast::unregisterApplications(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::unregisterApplications \n ");
    bool hasAppReq = parameters.HasLabel("applications"),
        returnStatus = false;
    if (hasAppReq)
    {
        LOGINFO ("\nInput string is:%s\n", parameters["applications"].String().c_str());
        if(_xcast)
        {
            LOGINFO("%s:%d _xcast Not NULL", __FUNCTION__, __LINE__);
            /*Disable cast service before registering Applications*/
            _xcast->enableCastService(m_friendlyName,false);
            m_isDynamicRegistrationsRequired = true;
            //Remove app names from cache map
            returnStatus = deleteFromDynamicAppCache (parameters["applications"].Array());   
            std::vector<DynamicAppConfig*> appConfigList;
            {lock_guard<mutex> lck(m_appConfigMutex);
                appConfigList = m_appConfigCache;
            }
            dumpDynamicAppConfigCache(string("m_appConfigCache"), appConfigList);
            //Pass the dynamic cache to xdial process
            registerApplicationsInternal (appConfigList);

            /*Reenabling cast service after registering Applications*/
            if (m_xcastEnable && ( (m_standbyBehavior == true) || ((m_standbyBehavior == false)&&(m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON)) ) ) {
                LOGINFO("Enable CastService  m_xcastEnable: %d m_standbyBehavior: %d m_powerState:%d", m_xcastEnable, m_standbyBehavior, m_powerState);
                _xcast->enableCastService(m_friendlyName,true);
            }
            else {
                LOGINFO("CastService not enabled m_xcastEnable: %d m_standbyBehavior: %d m_powerState:%d", m_xcastEnable, m_standbyBehavior, m_powerState);
            }
        }
    }
    returnResponse(returnStatus);
}

#if 0
//Timer Functions
void XCast::onLocateCastTimer()
{
    int status = _xcast->connectToRemoteService();
    if(status != 0)
    {
        if(locateCastObjectRetryCount < 4)
        {
            locateCastObjectRetryCount++;
        }
        if(locateCastObjectRetryCount == 1)
        {
            LOGINFO("Retry after 5 sec...");
            m_locateCastTimer.setInterval(LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS);
        }
        if(locateCastObjectRetryCount == 2)
        {
            LOGINFO("Retry after 15 sec...");
            m_locateCastTimer.setInterval(LOCATE_CAST_SECOND_TIMEOUT_IN_MILLIS);
        }
        if(locateCastObjectRetryCount == 3)
        {
            LOGINFO("Retry after 30 sec...");
            m_locateCastTimer.setInterval(LOCATE_CAST_THIRD_TIMEOUT_IN_MILLIS);
        }
        if(locateCastObjectRetryCount == 4)
        {
            LOGINFO("Retry after 60 sec...");
            m_locateCastTimer.setInterval(LOCATE_CAST_FINAL_TIMEOUT_IN_MILLIS);
        }
        return ;
    }// err != RT_OK
    locateCastObjectRetryCount = 0;
    m_locateCastTimer.stop();

    if (NULL != _xcast)
    {
        std::vector<DynamicAppConfig*> appConfigList;
        {lock_guard<mutex> lck(m_appConfigMutex);
            appConfigList = m_appConfigCache;
        }
        dumpDynamicAppConfigCache(string("m_appConfigCache"), appConfigList);
        LOGINFO("XCast::onLocateCastTimer : calling registerApplications");
        registerApplicationsInternal(appConfigList);
    }
    else {
        LOGINFO("XCast::onLocateCastTimer :_xcast: %p",  _xcast);
    }

    bool enabledStatus = false;
    if (m_xcastEnable && ( (m_standbyBehavior == true) || ((m_standbyBehavior == false)&&(m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON)) ) ) {
        enabledStatus = true;
    }
    _xcast->enableCastService(m_friendlyName,enabledStatus);
    LOGINFO("XCast::onLocateCastTimer : Timer still active ? %d ",m_locateCastTimer.isActive());
}
#endif

uint32_t XCast::registerApplicationsInternal(std::vector<DynamicAppConfig*> appConfigEntries)
{
    std::list<Exchange::IXCast::ApplicationInfo> appInfoList;
    Exchange::IXCast::IApplicationInfoIterator* appInfoLists{};
    uint32_t rc = Core::ERROR_UNAVAILABLE;

    for (auto appConfig : appConfigEntries)
    {
        Exchange::IXCast::ApplicationInfo appinfo;
        appinfo.appName = appConfig->appName;
        appinfo.prefixes = appConfig->prefixes;
        appinfo.cors = appConfig->cors;
        appinfo.query = appConfig->query;
        appinfo.payload = appConfig->payload;
        appinfo.allowStop = appConfig->allowStop;
        appInfoList.emplace_back(appinfo);
    }
    appInfoLists = (Core::Service<RPC::IteratorType<Exchange::IXCast::IApplicationInfoIterator>>::Create<Exchange::IXCast::IApplicationInfoIterator>(appInfoList));

    if (nullptr != _xcast)
    {
        rc = _xcast->registerApplications(appInfoLists);
    }

    if (appInfoLists)
    {
        appInfoLists->Release();
    }
    return rc;
}

#if 0
void XCast::onRtServiceDisconnected()
{
    LOGINFO("RT communication failure. Reconnecting.. ");
    m_locateCastTimer.start(LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS);
}
#endif

void XCast::getUrlFromAppLaunchParams (const char *app_name, const char *payload, const char *query_string, const char *additional_data_url, char *url) {
    LOGINFO("getUrlFromAppLaunchParams : Application launch request: appName: %s  query: [%s], payload: [%s], additionalDataUrl [%s]\n",
        app_name, query_string, payload, additional_data_url);

    int url_len = DIAL_MAX_PAYLOAD+DIAL_MAX_ADDITIONALURL+100;
    memset (url, '\0', url_len);
    if(strcmp(app_name,"YouTube") == 0) {
        if ((payload != NULL) && (additional_data_url != NULL)){
            snprintf( url, url_len, "https://www.youtube.com/tv?%s&additionalDataUrl=%s", payload, additional_data_url);
        }else if (payload != NULL){
            snprintf( url, url_len, "https://www.youtube.com/tv?%s", payload);
        }else{
            snprintf( url, url_len, "https://www.youtube.com/tv");
        }
    }
    else if(strcmp(app_name,"YouTubeTV") == 0) {
        if ((payload != NULL) && (additional_data_url != NULL)){
            snprintf( url, url_len, "https://www.youtube.com/tv/upg?%s&additionalDataUrl=%s", payload, additional_data_url);
        }else if (payload != NULL){
            snprintf( url, url_len, "https://www.youtube.com/tv/upg?%s", payload);
        }else{
            snprintf( url, url_len, "https://www.youtube.com/tv/upg?");
        }
    }
    else if(strcmp(app_name,"YouTubeKids") == 0) {
        if ((payload != NULL) && (additional_data_url != NULL)){
            snprintf( url, url_len, "https://www.youtube.com/tv_kids?%s&additionalDataUrl=%s", payload, additional_data_url);
        }else if (payload != NULL){
            snprintf( url, url_len, "https://www.youtube.com/tv_kids?%s", payload);
        }else{
            snprintf( url, url_len, "https://www.youtube.com/tv_kids?");
        }
    }
    else if(strcmp(app_name,"Netflix") == 0) {
        memset( url, 0, url_len );
        strncat( url, "source_type=12", url_len - strlen(url) - 1);
        if(payload != NULL)
        {
            const char * pUrlEncodedParams;
            pUrlEncodedParams = payload;
            if( pUrlEncodedParams ){
                strncat( url, "&dial=", url_len - strlen(url) - 1);
                strncat( url, pUrlEncodedParams, url_len - strlen(url) - 1);
            }
        }

        if(additional_data_url != NULL){
            strncat(url, "&additionalDataUrl=", url_len - strlen(url) - 1);
            strncat(url, additional_data_url, url_len - strlen(url) - 1);
        }
    }
    else {
            memset( url, 0, url_len );
            url_len -= DIAL_MAX_ADDITIONALURL+1; //save for &additionalDataUrl
            url_len -= 1; //save for nul byte
            LOGINFO("query_string=[%s]\r\n", query_string);
            int has_query = query_string && strlen(query_string);
            int has_payload = 0;
            if (has_query) {
                strcat(url, query_string);
                url_len -= strlen(query_string);
            }
            if(payload && strlen(payload)) {
                if (has_query) url_len -=1;  //for &
                const char payload_key[] = "dialpayload=";
                url_len -= sizeof(payload_key) - 1;
                url_len -= strlen(payload);
                if(url_len >= 0){
                    if (has_query) strcat(url, "&");
                    strcat(url, payload_key);
                    strcat(url, payload);
                    has_payload = 1;
                }
                else {
                    LOGINFO("there is no enough room for payload\r\n");
                }
            }

        if(additional_data_url != NULL){
                if (has_query || has_payload) strcat(url, "&");
                strcat(url, "additionalDataUrl=");
            strcat(url, additional_data_url);
            }
            LOGINFO(" url is [%s]\r\n", url);
    }
}
} // namespace Plugin
} // namespace WPEFramework