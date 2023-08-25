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
#include <cjson/cJSON.h>
#include "RtXcastConnector.h"
#include "UtilsSynchroIarm.hpp"

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


/*
 * The maximum additionalDataUrl length
 */
#define DIAL_MAX_ADDITIONALURL (1024)

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 4

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

SERVICE_REGISTRATION(XCast, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

static RtXcastConnector * _rtConnector  = RtXcastConnector::getInstance();
bool XCast::isCastEnabled = false;
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
bool XCast::m_enableStatus = false;

IARM_Bus_PWRMgr_PowerState_t XCast::m_powerState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;

XCast::XCast() : m_apiVersionNumber(1), m_isDynamicRegistrationsRequired(false)
{
    InitializeIARM();
    XCast::checkRFCServiceStatus();
    if(XCast::isCastEnabled)
    {
        LOGINFO("XcastService::Register methods and create onLocateCastTimer ");
        Utils::Synchro::RegisterLockedApi(METHOD_GET_API_VERSION_NUMBER, &XCast::getApiVersionNumber, this);
        Utils::Synchro::RegisterLockedApi(METHOD_ON_APPLICATION_STATE_CHANGED , &XCast::applicationStateChanged, this);
        Utils::Synchro::RegisterLockedApi(METHOD_SET_ENABLED, &XCast::setEnabled, this);
        Utils::Synchro::RegisterLockedApi(METHOD_GET_ENABLED, &XCast::getEnabled, this);
        Utils::Synchro::RegisterLockedApi(METHOD_GET_STANDBY_BEHAVIOR, &XCast::getStandbyBehavior, this);
        Utils::Synchro::RegisterLockedApi(METHOD_SET_STANDBY_BEHAVIOR, &XCast::setStandbyBehavior, this);
        Utils::Synchro::RegisterLockedApi(METHOD_GET_FRIENDLYNAME, &XCast::getFriendlyName, this);
        Utils::Synchro::RegisterLockedApi(METHOD_SET_FRIENDLYNAME, &XCast::setFriendlyName, this);
        Utils::Synchro::RegisterLockedApi(METHOD_REG_APPLICATIONS, &XCast::registerApplications, this);
        Utils::Synchro::RegisterLockedApi(METHOD_UNREG_APPLICATIONS, &XCast::unregisterApplications, this);
        Utils::Synchro::RegisterLockedApi(METHOD_GET_PROTOCOLVERSION, &XCast::getProtocolVersion, this);
        
        m_locateCastTimer.connect( bind( &XCast::onLocateCastTimer, this ));
    }
}

XCast::~XCast()
{
}
const void XCast::InitializeIARM()
{
     if (Utils::IARM::init())
     {
         IARM_Result_t res;
         IARM_CHECK( Utils::Synchro::RegisterLockedIarmEventHandler<XCast>(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED, powerModeChange) );
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
         IARM_CHECK( Utils::Synchro::RemoveLockedEventHandler<XCast>(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED, powerModeChange) );
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
            if(m_standbyBehavior == false)
            {
                if(m_xcastEnable && ( m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON))
                    _rtConnector->enableCastService(m_friendlyName,true);
                else
                    _rtConnector->enableCastService(m_friendlyName,false);
            }
         }
    }
}

const string XCast::Initialize(PluginHost::IShell* /* service */)
{
    LOGINFO("XCast:: Initialize  plugin called \n");
    _rtConnector  = RtXcastConnector::getInstance();
    _rtConnector->setService(this);
    if (XCast::isCastEnabled)
    {
        //TODO add rt intialization.
        if( _rtConnector->initialize())
        {
            //We give few seconds delay before the timer is fired.
            m_locateCastTimer.start(LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS);
        }
    }
    else
    {
        LOGINFO(" Cast service is disabled. Not initializing");
    }
    // On success return empty, to indicate there is no error text.
    return (string());
}

void XCast::Deinitialize(PluginHost::IShell* /* service */)
{
    LOGINFO("XCast::Deinitialize  called \n ");
    if ( m_locateCastTimer.isActive())
    {
        m_locateCastTimer.stop();
    }
    if( XCast::isCastEnabled){
        _rtConnector->enableCastService(m_friendlyName,false);
        _rtConnector->shutdown();
    }
    DeinitializeIARM();
}

string XCast::Information() const
{
    // No additional info to report.
    return (string());
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
    if(!app.empty() && !state.empty())
    {
        if (app == "NetflixApp")
            app = "Netflix";
        
        LOGINFO("XcastService::ApplicationStateChanged  ARGS = %s : %s : %s : %s ", app.c_str(), id.c_str() , state.c_str() , error.c_str());
        _rtConnector->applicationStateChanged(app, state, id, error);
     returnResponse(true);
    }//app && state not empty
    else{
       returnResponse(false);
    }
}

uint32_t XCast::setEnabled(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::setEnabled ");
    bool enabled = false;
    if (parameters.HasLabel("enabled"))
    {
         getBoolParameter("enabled", enabled);
    }
    else
    {
         returnResponse(false);
    }
    m_xcastEnable= enabled;
    if (m_xcastEnable && ( m_standbyBehavior || m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON ))
        _rtConnector->enableCastService(m_friendlyName,true);
    else
        _rtConnector->enableCastService(m_friendlyName,false);
    returnResponse(true);
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
    if (parameters.HasLabel("friendlyname"))
    {
         getStringParameter("friendlyname",paramStr);
         if(_rtConnector)
         {
            m_friendlyName = paramStr;
            LOGINFO("XcastService::setFriendlyName  :%s",m_friendlyName.c_str());
            if (m_xcastEnable && ( m_standbyBehavior || m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON )) {
               _rtConnector->enableCastService(m_friendlyName,true);
            }
            else {
                _rtConnector->enableCastService(m_friendlyName,false);
            }
         }
         else
            returnResponse(false);
    }
    else
    {
         returnResponse(false);
    }
    returnResponse(true);
}
uint32_t XCast::getFriendlyName(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::getFriendlyNamem_friendlyName :%s ",m_friendlyName.c_str());
    response["friendlyname"] = m_friendlyName;
    returnResponse(true);
}


uint32_t XCast::getProtocolVersion(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::getProtocolVersion");
    response["version"] = _rtConnector->getProtocolVersion();
    returnResponse(true);
}

bool XCast::getEntryFromAppLaunchParamList (const char* appName, DynamicAppConfig& retAppConfig){
    bool isEntryFound = false;
    {lock_guard<mutex> lck(m_appConfigMutex);
        for (DynamicAppConfig* regAppLaunchParam : m_appConfigCache) {
            if (0 == strcmp (regAppLaunchParam->appName, appName)) {
                isEntryFound = true;
                strcpy (retAppConfig.appName, regAppLaunchParam->appName);
                strcpy (retAppConfig.query, regAppLaunchParam->query);
                strcpy (retAppConfig.payload, regAppLaunchParam->payload);
                break;
            }
        }
    }
    return isEntryFound;
}

void XCast::dumpDynamicAppConfigCache(string strListName, std::vector<DynamicAppConfig*> appConfigList) {
    /*Check if existing cache need to be updated*/
    std::vector<int> entriesTodelete;
    LOGINFO ("=================Current dynamic %s size: %d is:===========================", strListName.c_str(), appConfigList.size());
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
            LOGINFO("Going to delete the entry: %d from m_appConfigCache size: %d", indexToDelete, m_appConfigCache.size());
            //Delete the old unwanted item here.
            DynamicAppConfig* pDynamicAppConfigOld = m_appConfigCache[indexToDelete];
            m_appConfigCache.erase (m_appConfigCache.begin()+indexToDelete);
            delete pDynamicAppConfigOld; pDynamicAppConfigOld = NULL;
        }
        entriesTodelete.clear();

    }
    //Even if requested app names not there return true.
    return ret;
}

bool XCast::deleteFromDynamicAppCache(string strAppNames)
{
    bool ret = false;
    cJSON *itrName = NULL;
    if (!strAppNames.empty()) {
        cJSON *applications = cJSON_Parse(strAppNames.c_str());
        if (!cJSON_IsArray(applications)) {
            LOGINFO ("deleteFromDynamicAppCache::applications array passed: %s", strAppNames.c_str());
            LOGINFO ("deleteFromDynamicAppCache::Invalid applications array exititng");
            cJSON_Delete(applications);
            return ret;
        }
        int iIndex = 0;
        vector<string> appsToDelete;
        cJSON_ArrayForEach(itrName, applications) {
            if (!cJSON_IsString(itrName)) {
                LOGINFO ("Invalid name format at application index. Skipping%d", iIndex);
                continue;
            }
            LOGINFO("App name to delete: %s, size:%d", itrName->valuestring, strlen (itrName->valuestring));
            appsToDelete.push_back(string(itrName->valuestring));
            iIndex++;
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
        cJSON_Delete(applications);
    }
    return ret;
}

void XCast::updateDynamicAppCache(string strApps)
{
    LOGINFO("XcastService::UpdateDynamicAppCache");

    cJSON *itrApp = NULL;

    cJSON *jNames = NULL;
    cJSON *itrName = NULL;

    cJSON *jPrefixes = NULL;
    cJSON *itrPrefix = NULL;

    cJSON *jCors = NULL;
    cJSON *itrCor = NULL;

    cJSON *jProperties = NULL;
    cJSON *jAllowStop = NULL;

    cJSON *jLaunchParam = NULL;
    cJSON *jQuery = NULL;
    cJSON *jPayload = NULL;

    std::vector <DynamicAppConfig*> appConfigList;
    if (!strApps.empty()) {
        cJSON *applications = cJSON_Parse(strApps.c_str());
        if (!cJSON_IsArray(applications)) {
            LOGINFO ("applications array passed: %s", strApps.c_str());
            LOGINFO ("Invalid applications array exititng");
            cJSON_Delete(applications);
            return;
        }

        /* iterate over ints */
        LOGINFO("Applications:");
        int iIndex = 0;

        cJSON_ArrayForEach(itrApp, applications) {
            std::vector <DynamicAppConfig*> appConfigListTemp;
            LOGINFO("Application: %d", iIndex);
            if (!cJSON_IsObject(itrApp)) {
                LOGINFO ("Invalid appliaction format at index. Skipping%d", iIndex);
                continue;
            }
            jNames = cJSON_GetObjectItem(itrApp, "names");
            if (!cJSON_IsArray(jNames)) {
                LOGINFO ("Invalid names format at application index %d. Skipping the application", iIndex);
                continue;
            }
            else {
                cJSON_ArrayForEach(itrName, jNames) {
                    if (!cJSON_IsString(itrName)) {
                        LOGINFO ("Invalid name format at application index. Skipping%d", iIndex);
                        continue;
                    }
                    LOGINFO("%s, size:%d", itrName->valuestring, strlen (itrName->valuestring));
                    DynamicAppConfig* pDynamicAppConfig = new DynamicAppConfig(itrName->valuestring);
                    appConfigListTemp.push_back (pDynamicAppConfig);
                }
            }

            jPrefixes = cJSON_GetObjectItem(itrApp, "prefixes");
            if (!cJSON_IsArray(jPrefixes)) {
                LOGINFO ("Invalid prefixes format at application index %d", iIndex);
            }
            else {
                cJSON_ArrayForEach(itrPrefix, jPrefixes) {
                    if (!cJSON_IsString(itrPrefix)) {
                        LOGINFO ("Invalid prefix format at application index. Skipping%d", iIndex);
                        continue;
                    }
                    LOGINFO("%s, size:%d", itrPrefix->valuestring, strlen (itrPrefix->valuestring));
                    for (DynamicAppConfig* pDynamicAppConfig : appConfigListTemp) {
                        strcpy (pDynamicAppConfig->prefixes, itrPrefix->valuestring);
                    }
                }
            }

            jCors = cJSON_GetObjectItem(itrApp, "cors");
            if (!cJSON_IsArray(jCors)) {
                LOGINFO ("Invalid cors format at application index %d. Skipping the application", iIndex);
                continue;
            }
            else {
                cJSON_ArrayForEach(itrCor, jCors) {
                    if (!cJSON_IsString(itrCor)) {
                        LOGINFO ("Invalid cor format at application index. Skipping%d", iIndex);
                        continue;
                    }
                    LOGINFO("%s, size:%d", itrCor->valuestring, strlen (itrCor->valuestring));
                    for (DynamicAppConfig* pDynamicAppConfig : appConfigListTemp) {
                        strcpy (pDynamicAppConfig->cors, itrCor->valuestring);
                    }
                }
            }

            jProperties = cJSON_GetObjectItem(itrApp, "properties");
            if (!cJSON_IsObject(jProperties)) {
                LOGINFO ("Invalid property format at application index %d", iIndex);
            }
            else {
                jAllowStop = cJSON_GetObjectItem(jProperties, "allowStop");
                if (!cJSON_IsBool(jAllowStop)) {
                    LOGINFO ("Invalid allowStop format at application index %d", iIndex);
                }
                else {
                    LOGINFO("allowStop: %d", jAllowStop->valueint);
                    for (DynamicAppConfig* pDynamicAppConfig : appConfigListTemp) {
                        pDynamicAppConfig->allowStop = jAllowStop->valueint;
                    }
                }
            }

            jLaunchParam = cJSON_GetObjectItem(itrApp, "launchParameters");
            if (!cJSON_IsObject(jLaunchParam)) {
                LOGINFO ("Invalid Launch param format at application index %d", iIndex);
            }
            else {
                jQuery = cJSON_GetObjectItem(jLaunchParam, "query");
                if (!cJSON_IsString(jQuery)) {
                    LOGINFO ("Invalid query format at application index %d", iIndex);
                }
                else {
                    LOGINFO("query: %s, size:%d", jQuery->valuestring, strlen (jQuery->valuestring));
                }
                jPayload = cJSON_GetObjectItem(jLaunchParam, "payload");
                if (!cJSON_IsString(jPayload)) {
                    LOGINFO ("Invalid payload format at application index %d", iIndex);
                }
                else {
                    LOGINFO("payload: %s, size:%d", jPayload->valuestring, strlen (jPayload->valuestring));
                }
                //Set launchParameters in list for later usage
                for (DynamicAppConfig* pDynamicAppConfig : appConfigListTemp) {
                    if (cJSON_IsString(jQuery)) {
                        strcpy (pDynamicAppConfig->query, jQuery->valuestring);
                    }
                    if (cJSON_IsString(jPayload)) {
                        strcpy (pDynamicAppConfig->payload, jPayload->valuestring);
                    }
                }

            }
            for (DynamicAppConfig* pDynamicAppConfig : appConfigListTemp) {
                appConfigList.push_back(pDynamicAppConfig);
            }
            appConfigListTemp.clear();
            iIndex++;
        }
        dumpDynamicAppConfigCache(string("appConfigList"), appConfigList);
        vector<string> appsToDelete;
        for (DynamicAppConfig* pDynamicAppConfig : appConfigList) {
            appsToDelete.push_back(string(pDynamicAppConfig->appName));
        }
        deleteFromDynamicAppCache (appsToDelete);

        LOGINFO("appConfigList count: %d", appConfigList.size());
        //Update the new entries here.
        {lock_guard<mutex> lck(m_appConfigMutex);
            for (DynamicAppConfig* pDynamicAppConfig : appConfigList) {
                m_appConfigCache.push_back(pDynamicAppConfig);
            }
            LOGINFO("m_appConfigCache count: %d", m_appConfigCache.size());
        }
        //Clear the tempopary list here
        appsToDelete.clear();
        appConfigList.clear();
        cJSON_Delete(applications);
    }
    dumpDynamicAppConfigCache(string("m_appConfigCache"), m_appConfigCache);
    return;
}

uint32_t XCast::registerApplications(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::registerApplications \n ");
    bool hasAppReq = parameters.HasLabel("applications");
    if (hasAppReq) {
       LOGINFO ("\nInput string is:%s\n", parameters["applications"].String().c_str());

       if(_rtConnector)
       {
           LOGINFO("%s:%d _rtConnector Not NULL", __FUNCTION__, __LINE__);
           if(_rtConnector->IsDynamicAppListEnabled()) {
               /*Disable cast service before registering Applications*/
               _rtConnector->enableCastService(m_friendlyName,false);

               m_isDynamicRegistrationsRequired = true;
               //Register dynamic application list to app cache map
               updateDynamicAppCache(parameters["applications"].String());
               std::vector<DynamicAppConfig*> appConfigList;
               {lock_guard<mutex> lck(m_appConfigMutex);
                   appConfigList = m_appConfigCache;
               }
               dumpDynamicAppConfigCache(string("m_appConfigCache"), appConfigList);
               //Pass the dynamic cache to xdial process
               _rtConnector->registerApplications (m_appConfigCache);

               /*Reenabling cast service after registering Applications*/
               if (m_xcastEnable && ( m_standbyBehavior || m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON )) {
                   LOGINFO("Enable CastService  m_xcastEnable: %d m_standbyBehavior: %d m_powerState:%d", m_xcastEnable, m_standbyBehavior, m_powerState);
                   _rtConnector->enableCastService(m_friendlyName,true);
               }
               else {
                   LOGINFO("CastService not enabled m_xcastEnable: %d m_standbyBehavior: %d m_powerState:%d", m_xcastEnable, m_standbyBehavior, m_powerState);
               }
               returnResponse(true);
           }
           else {
               returnResponse(false);
           }
       }
       else
           returnResponse(false);
    }
    else {
        returnResponse(false);
    }
}

uint32_t XCast::unregisterApplications(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::unregisterApplications \n ");
    bool hasAppReq = parameters.HasLabel("applications");
    if (hasAppReq) {
       LOGINFO ("\nInput string is:%s\n", parameters["applications"].String().c_str());

       if(_rtConnector)
       {
	       LOGINFO("%s:%d _rtConnector Not NULL", __FUNCTION__, __LINE__);
           if(_rtConnector->IsDynamicAppListEnabled()) {
               /*Disable cast service before registering Applications*/
               _rtConnector->enableCastService(m_friendlyName,false);
               m_isDynamicRegistrationsRequired = true;
               //Remove app names from cache map
               bool ret = deleteFromDynamicAppCache (parameters["applications"].String());
               std::vector<DynamicAppConfig*> appConfigList;
               {lock_guard<mutex> lck(m_appConfigMutex);
                   appConfigList = m_appConfigCache;
               }
               dumpDynamicAppConfigCache(string("m_appConfigCache"), appConfigList);
               //Pass the dynamic cache to xdial process
               _rtConnector->registerApplications (appConfigList);

               /*Reenabling cast service after registering Applications*/
               if (m_xcastEnable && ( m_standbyBehavior || m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON)) {
                   LOGINFO("Enable CastService  m_xcastEnable: %d m_standbyBehavior: %d m_powerState:%d", m_xcastEnable, m_standbyBehavior, m_powerState);
                   _rtConnector->enableCastService(m_friendlyName,true);
               }
               else {
                   LOGINFO("CastService not enabled m_xcastEnable: %d m_standbyBehavior: %d m_powerState:%d", m_xcastEnable, m_standbyBehavior, m_powerState);
               }
               returnResponse(ret);
           }
           else {
               returnResponse(false);
           }
       }
       else
           returnResponse(false);
    }
    else {
        returnResponse(false);
    }
}

//Timer Functions
void XCast::onLocateCastTimer()
{
    if (_rtConnector == nullptr) {
        LOGINFO("XCast::onLocateCastTimer :_rtConnector is NULL");
        return;
    }
    int status = _rtConnector->connectToRemoteService();
    if(status != 0)
    {
        LOGINFO("Retry after 5 sec...");
        m_locateCastTimer.setInterval(LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS);
        return ;
    }// err != RT_OK
    m_locateCastTimer.stop();

    if (_rtConnector->IsDynamicAppListEnabled() && m_isDynamicRegistrationsRequired) {

        std::vector<DynamicAppConfig*> appConfigList;
        {lock_guard<mutex> lck(m_appConfigMutex);
            appConfigList = m_appConfigCache;
        }
        dumpDynamicAppConfigCache(string("m_appConfigCache"), appConfigList);
        LOGINFO("XCast::onLocateCastTimer : calling registerApplications");
        _rtConnector->registerApplications (appConfigList);
    }
    else {
        LOGINFO("XCast::onLocateCastTimer : DynamicAppList not enabled");
    }

    if (m_xcastEnable && ( m_standbyBehavior || m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON )) {
        _rtConnector->enableCastService(m_friendlyName,true);
    }
    else {
        _rtConnector->enableCastService(m_friendlyName,false);
    }
    
    LOGINFO("XCast::onLocateCastTimer : Timer still active ? %d ",m_locateCastTimer.isActive());
}

void XCast::onRtServiceDisconnected() 
{
    LOGINFO("RT communication failure. Reconnecting.. ");
    m_locateCastTimer.start(LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS);
}

void XCast::getUrlFromAppLaunchParams (const char *app_name, const char *payload, const char *query_string, const char *additional_data_url, char *url) {
    LOGINFO("getUrlFromAppLaunchParams : Application launch request: appName: %s  query: [%s], payload: [%s], additionalDataUrl [%s]\n",
        app_name, query_string, payload, additional_data_url);

    int url_len = DIAL_MAX_PAYLOAD+DIAL_MAX_ADDITIONALURL+100;
    memset (url, '\0', url_len);
    if(strcmp(app_name,"YouTube") == 0) {
        if ((payload != NULL) && (additional_data_url != NULL)){
            sprintf( url, "https://www.youtube.com/tv?%s&additionalDataUrl=%s", payload, additional_data_url);
        }else if (payload != NULL){
            sprintf( url, "https://www.youtube.com/tv?%s", payload);
        }else{
            sprintf( url, "https://www.youtube.com/tv");
        }
    }
    else if(strcmp(app_name,"YouTubeTV") == 0) {
        if ((payload != NULL) && (additional_data_url != NULL)){
            sprintf( url, "https://www.youtube.com/tv/upg?%s&additionalDataUrl=%s", payload, additional_data_url);
        }else if (payload != NULL){
            sprintf( url, "https://www.youtube.com/tv/upg?%s", payload);
        }else{
            sprintf( url, "https://www.youtube.com/tv/upg?");
        }
    }
    else if(strcmp(app_name,"YouTubeKids") == 0) {
        if ((payload != NULL) && (additional_data_url != NULL)){
            sprintf( url, "https://www.youtube.com/tv_kids?%s&additionalDataUrl=%s", payload, additional_data_url);
        }else if (payload != NULL){
            sprintf( url, "https://www.youtube.com/tv_kids?%s", payload);
        }else{
            sprintf( url, "https://www.youtube.com/tv_kids?");
        }
    }
    else if(strcmp(app_name,"Netflix") == 0) {
        memset( url, 0, url_len );
        bool first = true;
        if(payload != NULL && strlen(payload))
        {
            first = false;
            strcat( url, "dial=");
            strcat( url, payload );
        }

        if(additional_data_url != NULL && strlen(additional_data_url)){
            if (!first) {
                strcat(url, "&");
            }
            first = false;
            strcat(url, "additionalDataUrl=");
            strcat(url, additional_data_url);
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

void XCast::onXcastApplicationLaunchRequestWithLaunchParam (string appName,
        string strPayLoad, string strQuery, string strAddDataUrl)
{
    //TODO
    LOGINFO ("XcastService::onXcastApplicationLaunchRequestWithLaunchParam ");
    if(strAddDataUrl.size() > DIAL_MAX_ADDITIONALURL){
        LOGWARN ("%s - current additional data size (%d) exceeds maximum allowed size (%d) ", __PRETTY_FUNCTION__, strAddDataUrl.size(), DIAL_MAX_ADDITIONALURL);
        return;
    }
    if(strPayLoad.size() > DIAL_MAX_PAYLOAD) {
        LOGWARN ("%s - current payload size (%d) exceeds maximum allowed size (%d) ", __PRETTY_FUNCTION__, strPayLoad.size(), DIAL_MAX_PAYLOAD);
        return;
    }
    JsonObject params;
    JsonObject urlParam;
    char url[DIAL_MAX_PAYLOAD+DIAL_MAX_ADDITIONALURL+100] = {0,};

    if(_rtConnector) {
        DynamicAppConfig appConfig {};
        getEntryFromAppLaunchParamList (appName.c_str(), appConfig);

        /*Replacing with App requested payload and query*/
        if (('\0' != appConfig.query[0]) && ('\0' != appConfig.payload[0])) {
            getUrlFromAppLaunchParams (appName.c_str(),
                               appConfig.payload,
                               appConfig.query,
                               strAddDataUrl.c_str(), url);
        }
        else if(('\0' != appConfig.payload[0])){
            getUrlFromAppLaunchParams (appName.c_str(),
                               appConfig.payload,
                               strQuery.c_str(),
                               strAddDataUrl.c_str(), url);
        }
        else if(('\0' != appConfig.query[0])) {
            getUrlFromAppLaunchParams (appName.c_str(),
                               strPayLoad.c_str(),
                               appConfig.query,
                               strAddDataUrl.c_str(), url);
        }
        else {
            getUrlFromAppLaunchParams (appName.c_str(),
                               strPayLoad.c_str(),
                               strQuery.c_str(),
                               strAddDataUrl.c_str(), url);
        }


        string strUrl = std::string (url);
        if (appName == "Netflix") {
            appName.assign("NetflixApp");
            urlParam["pluginUrl"]=strUrl;
        }
        else {
            urlParam["url"]=strUrl;
        }

        params["applicationName"]= appName;
        params["parameters"]= urlParam;

        sendNotify(EVT_ON_LAUNCH_REQUEST, params);
    }
}

void XCast::onXcastApplicationLaunchRequest(string appName, string parameter) 
{
    //TODO 
    LOGINFO ("XcastService::onXcastApplicationLaunchRequest ");
    JsonObject params;
    JsonObject urlParam;
    if (appName == "NetflixApp")
        urlParam["pluginUrl"]=parameter;
    else
        urlParam["url"]=parameter;
    
    params["applicationName"]= appName;
    params["parameters"]= urlParam;
    
    sendNotify(EVT_ON_LAUNCH_REQUEST, params);
}
void XCast::onXcastApplicationStopRequest(string appName, string appID) 
{
    //TODO
    LOGINFO("XcastService::onXcastApplicationStopRequest ");
    
    JsonObject params;
    params["applicationName"] = appName;
    params["applicationId"]= appID;
    
    sendNotify(EVT_ON_STOP_REQUEST, params);
}
void XCast::onXcastApplicationHideRequest(string appName, string appID) 
{
    LOGINFO("XcastService::onXcastApplicationHideRequest : ");
    if (appName.compare("Netflix") == 0 )
        appName = "NetflixApp";
    
    
    JsonObject params;
    params["applicationName"] = appName;
    params["applicationId"]= appID;
    
    sendNotify(EVT_ON_HIDE_REQUEST, params);
}
void XCast::onXcastApplicationStateRequest(string appName, string appID) 
{
    LOGINFO("XcastService::onXcastApplicationStateRequest: ");
    if (appName.compare("Netflix") == 0 )
        appName = "NetflixApp";
    
    JsonObject params;
    params["applicationName"] = appName;
    params["applicationId"]= appID;
    
    sendNotify(EVT_ON_STATE_REQUEST , params);
    
}
void XCast::onXcastApplicationResumeRequest(string appName, string appID) 
{
    LOGINFO("XcastService::onXcastApplicationResumeRequest ");
    if (appName.compare("Netflix") == 0 )
        appName = "NetflixApp";
    
    JsonObject params;
    params["applicationName"] = appName;
    params["applicationId"]= appID;
    sendNotify(EVT_ON_RESUME_REQUEST, params);
}

bool XCast::checkRFCServiceStatus()
{
#ifdef RFC_ENABLED
    RFC_ParamData_t param;
    WDMP_STATUS wdmpStatus = getRFCParameter(const_cast<char *>("Xcast"), "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.XDial.Enable", &param);
    if (wdmpStatus == WDMP_SUCCESS || wdmpStatus == WDMP_ERR_DEFAULT_VALUE)
    {
        if( param.type == WDMP_BOOLEAN )
        {
            if(strncasecmp(param.value,"true",4) == 0 )
                XCast::isCastEnabled = true;
        }
    }

    LOGINFO(" Is cast enabled ? %d , call value %d ", isCastEnabled, wdmpStatus);
#else
    XCast::isCastEnabled = true;;
#endif //RFC_ENABLED
    
    return XCast::isCastEnabled;
}

} // namespace Plugin
} // namespace WPEFramework
