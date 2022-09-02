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

#include "RtXcastConnector.h"
#include "Module.h"
#include "UtilsJsonRpc.h"
#include "rfcapi.h"
#include <cjson/cJSON.h>

using namespace std;
using namespace WPEFramework;
#define LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS  5000  //5 seconds
#define LOCATE_CAST_SECOND_TIMEOUT_IN_MILLIS 15000  //15 seconds
#define LOCATE_CAST_THIRD_TIMEOUT_IN_MILLIS  30000  //30 seconds
#define LOCATE_CAST_FINAL_TIMEOUT_IN_MILLIS  60000  //60 seconds
#define EVENT_LOOP_ITERATION_IN_100MS     100000

//default app list to store
#define defaultDynamicAppsString "[{\"names\":[\"Netflix\"],\"cors\":[\".netflix.com\"],\"properties\":{\"allowStop\" :true}},\
                                   {\"names\":[\"YouTube\", \"YouTubeKids\", \"YouTubeTV\"],\"cors\":[\".youtube.com\"],\"properties\":{\"allowStop\" :true}},\
                                   {\"names\":[\"AmazonInstantVideo\"],\"cors\":[\".amazonprime.com\"],\"properties\":{\"allowStop\" :true}},\
                                   {\"names\":[\"com.spotify.Spotify.TV\"],\"prefixes\":[\"com.spotify\"],\"cors\":[\".spotify.com\"],\"properties\":{\"allowStop\" :true}},\
                                   {\"names\":[\"Pairing\"],\"cors\":[\".comcast.com\"],\"properties\":{\"allowStop\" :true}}]"

static rtObjectRef xdialCastObj = NULL;
RtXcastConnector * RtXcastConnector::_instance = nullptr;

void RtXcastConnector::remoteDisconnectCallback( void* context) {
    RtNotifier * observer = static_cast<RtNotifier *> (context);
    LOGINFO ( "remoteDisconnectCallback: Remote  disconnected... ");
    observer->onRtServiceDisconnected();
}

void RtXcastConnector::processRtMessages(){
    LOGINFO("Entering Event Loop");
    while(true)
    {
        rtError err = rtRemoteProcessSingleItem();
        if (err != RT_OK && err != RT_ERROR_QUEUE_EMPTY) {
            LOGERR("Failed to gete item from Rt queue");
        }
        {
            //Queue needs to be deactivated ?
            lock_guard<mutex> lock(m_threadlock);
            if (!m_runEventThread ) break;
        }
        /*
         Ideally this should be part of wpe process main message loop,
         will reconsider once we decide on connectivity with dial server
        */
        usleep(EVENT_LOOP_ITERATION_IN_100MS);
    }
    LOGINFO("Exiting Event Loop");
}
void RtXcastConnector::threadRun(RtXcastConnector *rtCtx){
        RtXcastConnector * observer = static_cast<RtXcastConnector *> (rtCtx);
        observer->processRtMessages();
}
//XDIALCAST EVENT CALLBACK
/**
 * Callback function for application launch request from an app
 */
rtError RtXcastConnector::onApplicationLaunchRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context)
{
    if (numArgs == 1)
    {
        RtNotifier * observer = static_cast<RtNotifier *> (context);
        rtObjectRef appObject = args[0].toObject();
        rtString appName = appObject.get<rtString>("applicationName");
        rtString rtparamIsUrl = appObject.get<rtString>("isUrl");
        if (0 == strcmp(rtparamIsUrl.cString(), "false")) {

            rtString rtPayload = appObject.get<rtString>("payload");
            rtString rtQuery = appObject.get<rtString>("query");
            rtString rtAddDataUrl = appObject.get<rtString>("addDataUrl");
            observer->onXcastApplicationLaunchRequestWithLaunchParam(
                                         appName.cString(),
                                         rtPayload.cString(),
                                         rtQuery.cString(),
                                         rtAddDataUrl.cString());
        }
        else {
            if (!strcmp(appName.cString(),"Netflix"))
                appName = "NetflixApp";
            rtString rtparams = appObject.get<rtString>("parameters");
            observer->onXcastApplicationLaunchRequest(appName.cString() , rtparams.cString());
        }
    }
    else
        LOGERR(" *** Error: received unknown event");
    if (result)
        *result = rtValue(true);
    return RT_OK;
}
/**
 * Callback function for application stop request from an app
 */
rtError RtXcastConnector::onApplicationStopRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context)
{
    if (numArgs == 1)
    {
        RtNotifier * observer = static_cast<RtNotifier *> (context);
        rtObjectRef appObject = args[0].toObject();
        rtString appName = appObject.get<rtString>("applicationName");
        if (!strcmp(appName.cString(),"Netflix"))
            appName = "NetflixApp";
        rtString appID = appObject.get<rtString>("applicationId");
        observer->onXcastApplicationStopRequest(appName.cString(),appID.cString());
    }
    else
        LOGERR(" *** Error: received unknown event" );
    if (result)
        *result = rtValue(true);
    return RT_OK;
}
/**
 * Callback function for application hide request from an app
 */
rtError RtXcastConnector::onApplicationHideRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context)
{
    if (numArgs == 1)
    {
        RtNotifier * observer = static_cast<RtNotifier *> (context);
        rtObjectRef appObject = args[0].toObject();
        rtString appName = appObject.get<rtString>("applicationName");
        if (!strcmp(appName.cString(),"Netflix"))
            appName = "NetflixApp";
        rtString appID = appObject.get<rtString>("applicationId");
        observer->onXcastApplicationHideRequest(appName.cString(), appID.cString());
    }
    else
        LOGERR(" *** Error: received unknown event");
    
    if (result)
        *result = rtValue(true);
    
    return RT_OK;
    
}
/**
 * Callback function for application state request from an app
 */
rtError RtXcastConnector::onApplicationStateRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context)
{
    if (numArgs == 1)
    {
        RtNotifier * observer = static_cast<RtNotifier *> (context);
        rtObjectRef appObject = args[0].toObject();
        rtString appName = appObject.get<rtString>("applicationName");
        if (!strcmp(appName.cString(),"Netflix"))
            appName = "NetflixApp";
        
        rtString appID = appObject.get<rtString>("applicationId");
        observer->onXcastApplicationStateRequest(appName.cString(),appID.cString());
    }
    else
        LOGERR(" *** Error: received unknown event");
    
    if (result)
        *result = rtValue(true);
    
    return RT_OK;
}
/**
 * Callback function for application resume request from an app
 */
rtError RtXcastConnector::onApplicationResumeRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context)
{
    if (numArgs == 1)
    {
        RtNotifier * observer = static_cast<RtNotifier *> (context);
        rtObjectRef appObject = args[0].toObject();
        rtString appName = appObject.get<rtString>("applicationName");
        if (!strcmp(appName.cString(),"Netflix"))
            appName = "NetflixApp";
        
        rtString appID = appObject.get<rtString>("applicationId");
        observer->onXcastApplicationResumeRequest(appName.cString(),appID.cString());
        
    }
    else
        LOGERR(" *** Error: received unknown event");
    
    if (result)
        *result = rtValue(true);
    
    return RT_OK;
}
/**
 * Callback function when remote service exits
 */
rtError RtXcastConnector::onRtServiceByeCallback(int numArgs, const rtValue* args, rtValue* result, void* context)
{
    if (numArgs == 1)
    {
        rtObjectRef appObject = args[0].toObject();
        rtString serviceName = appObject.get<rtString>("serviceName");
        LOGINFO("Received RtService Bye Event! Service: %s", serviceName.cString());
    }
    else
        LOGERR(" *** Error: received unknown event");
    
    if (result)
        *result = rtValue(true);
    
    return RT_OK;
    
}

//Timer Functions
//Timer Fired when rt remote connectivity is broken.
int RtXcastConnector::connectToRemoteService()
{
    rtError err = RT_ERROR;
    
    const char * serviceName = "com.comcast.xdialcast";
    
    LOGINFO("connectToRemoteService entry " );
    err = rtRemoteLocateObject(rtEnvironmentGetGlobal(), serviceName, xdialCastObj, 0, &RtXcastConnector::remoteDisconnectCallback, m_observer);
    if(err == RT_OK && xdialCastObj != NULL)
    {
        rtError e = xdialCastObj.send("on", "onApplicationLaunchRequest" , new rtFunctionCallback(RtXcastConnector::onApplicationLaunchRequestCallback, m_observer));
        LOGINFO("Registered onApplicationLaunchRequest ; response %d" ,e );
        e = xdialCastObj.send("on", "onApplicationStopRequest" , new rtFunctionCallback(RtXcastConnector::onApplicationStopRequestCallback, m_observer));
        LOGINFO("Registered onApplicationStopRequest %d", e );
        e = xdialCastObj.send("on", "onApplicationHideRequest" , new rtFunctionCallback( RtXcastConnector::onApplicationHideRequestCallback, m_observer));
        LOGINFO("Registered onApplicationHideRequest %d", e );
        e = xdialCastObj.send("on", "onApplicationResumeRequest" , new rtFunctionCallback( RtXcastConnector::onApplicationResumeRequestCallback, m_observer));
        LOGINFO("Registered onApplicationResumeRequest %d", e );
        e = xdialCastObj.send("on", "onApplicationStateRequest" , new rtFunctionCallback( RtXcastConnector::onApplicationStateRequestCallback, m_observer));
        LOGINFO("Registed onApplicationStateRequest %d", e );
        e = xdialCastObj.send("on", "bye" , new rtFunctionCallback(RtXcastConnector::onRtServiceByeCallback, m_observer));
        LOGINFO("Registed rtService bye event %d", e );
    }
    else
        LOGINFO("response of rtRemoteLocateObject %d ",   err);
    return (err == RT_OK) ? 0 : 1;
}


RtXcastConnector::~RtXcastConnector()
{
    _instance = nullptr;
    m_observer = nullptr;
}

bool RtXcastConnector::initialize()
{
    rtError err;
    rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
    err = rtRemoteInit(env);
    m_IsDefaultDynamicAppListEnabled = false;
    if(err != RT_OK){
        LOGINFO("Xcastservice: rtRemoteInit failed : Reason %s", rtStrError(err));
    }
    else {
        m_runEventThread = true;
        m_eventMtrThread = std::thread(threadRun, this);
    }
    initDynamicAppCache();
    return (err == RT_OK) ? true:false;
}
void RtXcastConnector::shutdown()
{
    LOGINFO("Shutting down rtRemote connectivity");
    {
        lock_guard<mutex> lock(m_threadlock);
        m_runEventThread = false;
    }
    if (m_eventMtrThread.joinable())
        m_eventMtrThread.join();    

    rtRemoteShutdown(rtEnvironmentGetGlobal());
    if(RtXcastConnector::_instance != nullptr)
    {
        delete RtXcastConnector::_instance;
        RtXcastConnector::_instance = nullptr;
    }
}

int RtXcastConnector::applicationStateChanged( string app, string state, string id, string error)
{
    int status = 0;
    LOGINFO("XcastService::ApplicationStateChanged  ARGS = %s : %s : %s : %s ", app.c_str(), id.c_str() , state.c_str() , error.c_str());
    if(xdialCastObj != NULL)
    {
        rtObjectRef e = new rtMapObject;
        e.set("applicationName", app.c_str());
        e.set("applicationId", id.c_str());
        e.set("state",state.c_str());
        e.set("error",error.c_str());
        xdialCastObj.send("onApplicationStateChanged", e);
        status = 1;
    }
    else
        LOGINFO(" xdialCastObj is NULL ");
    return status;
}//app && state not empty
void RtXcastConnector::enableCastService(string friendlyname,bool enableService)
{
    LOGINFO("XcastService::enableCastService ARGS = %s : %d ", friendlyname.c_str(), enableService);
    if(xdialCastObj != NULL)
    {
        rtObjectRef e = new rtMapObject;
        e.set("activation",(enableService ? "true": "false"));
        e.set("friendlyname",friendlyname.c_str());
        int ret = xdialCastObj.send("onActivationChanged", e);
        LOGINFO("XcastService send onActivationChanged:%d",ret);
    }
    else
        LOGINFO(" xdialCastObj is NULL ");
    
}

void RtXcastConnector::updateFriendlyName(string friendlyname)
{
    LOGINFO("XcastService::updateFriendlyName ARGS = %s ", friendlyname.c_str());
    if(xdialCastObj != NULL)
    {
        rtObjectRef rtObj = new rtMapObject;
        rtObj.set("friendlyname",friendlyname.c_str());
        int ret = xdialCastObj.send("onFriendlyNameChanged", rtObj);
        LOGINFO("XcastService send onFriendlyNameChanged ret:%d",ret);
    }
    else
        LOGINFO(" xdialCastObj is NULL ");
}

bool RtXcastConnector::getEntryFromAppLaunchParamList (const char* appName, DynamicAppConfig* reqParam){
    bool isEntryFound = false;
    {lock_guard<mutex> lck(m_appConfigMutex);
        for (DynamicAppConfig* regAppLaunchParam : m_appConfigCache) {
            if (0 == strcmp (regAppLaunchParam->appName, appName)) {
                isEntryFound = true;
                int iNameLen = strlen (regAppLaunchParam->appName);
                reqParam->appName = (char*) malloc (iNameLen+1);
                memset (reqParam->appName, '\0', iNameLen+1);
                strcpy (reqParam->appName, regAppLaunchParam->appName);

                if (regAppLaunchParam->query) {
                    int iQueryLen = strlen (regAppLaunchParam->query);
                    reqParam->query = (char*) malloc (iQueryLen+1);
                    memset (reqParam->query, '\0', iQueryLen+1);
                    strcpy (reqParam->query, regAppLaunchParam->query);
                }

                if (regAppLaunchParam->payload) {
                    int iPayLoad = strlen (regAppLaunchParam->payload);
                    reqParam->payload = (char*) malloc (iPayLoad+1);
                    memset (reqParam->payload, '\0', iPayLoad+1);
                    strcpy (reqParam->payload, regAppLaunchParam->payload);
                }
                break;
            }
        }
    }
    return isEntryFound;
}

string RtXcastConnector::getProtocolVersion(void)
{
    LOGINFO("XcastService::getProtocolVersion ");
    rtString strVersion ;
    int ret = 0;
    if(xdialCastObj != NULL && (xdialCastObj.sendReturns("getProtocolVersion", strVersion) == RT_OK))
    {
            LOGINFO("XcastService getProtocolVersion ret:%d version:%s ",ret,strVersion.cString());
    }
    else
    {
        LOGINFO(" XcastService getProtocolVersion  xdialCastObj is NULL sendReturns ret :%d not RT_OK so returns 2.1",ret);
	strVersion = "2.1";
    }
    return strVersion.cString();
}

void RtXcastConnector::DumpDynamicAppConfigCache(string strListName, std::vector<DynamicAppConfig*> appConfigList) {
    LOGINFO("appConfigList count: %d", appConfigList.size());
    /*Check if existing cache need to be updated*/
    std::vector<int> entriesTodelete;
    LOGINFO ("=================Current dynamic %s is:===========================", strListName.c_str());
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

bool RtXcastConnector::deleteFromDynamicAppCache(vector<string>& appsToDelete) {
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
            ret = (ret&&found);
        }
        std::sort(entriesTodelete.begin(), entriesTodelete.end(), std::greater<int>());
        for (int indexToDelete : entriesTodelete) {
            LOGINFO("Going to delete the entry: %d from m_appConfigCache size: %d", indexToDelete, m_appConfigCache.size());
            //Delete the old unwanted item here.
            DynamicAppConfig* pDynamicAppConfigOld = m_appConfigCache[indexToDelete];
            m_appConfigCache.erase (m_appConfigCache.begin()+indexToDelete);
            free (pDynamicAppConfigOld->appName); pDynamicAppConfigOld->appName=NULL;
            free (pDynamicAppConfigOld->prefixes); pDynamicAppConfigOld->prefixes=NULL;
            free (pDynamicAppConfigOld->cors); pDynamicAppConfigOld->cors=NULL;
            free (pDynamicAppConfigOld->query); pDynamicAppConfigOld->query = NULL;
            free (pDynamicAppConfigOld->payload); pDynamicAppConfigOld->payload = NULL;
            free (pDynamicAppConfigOld); pDynamicAppConfigOld = NULL;
        }
        entriesTodelete.clear();

    }
    return ret;
}

bool RtXcastConnector::deleteFromDynamicAppCache(string strAppNames)
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
        //Remove specified appl list from dynamic app cache
        ret = deleteFromDynamicAppCache (appsToDelete);
        appsToDelete.clear();
        cJSON_Delete(applications);
    }
    return ret;
}

void RtXcastConnector::updateDynamicAppCache(string strApps)
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
                    DynamicAppConfig* pDynamicAppConfig = (DynamicAppConfig*) malloc (sizeof(DynamicAppConfig));
                    memset (pDynamicAppConfig, '0', sizeof(DynamicAppConfig));
                    int iFieldLen = strlen (itrName->valuestring);
                    pDynamicAppConfig->appName = (char*) malloc (iFieldLen+1);
                    memset (pDynamicAppConfig->appName, '\0', iFieldLen+1);
                    strcpy (pDynamicAppConfig->appName, itrName->valuestring);
                    pDynamicAppConfig->prefixes = NULL;
                    pDynamicAppConfig->cors = NULL;
                    pDynamicAppConfig->query = NULL;
                    pDynamicAppConfig->payload = NULL;
                    appConfigListTemp.push_back (pDynamicAppConfig);
                }
                LOGINFO("");
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
                        int iFieldLen = strlen (itrPrefix->valuestring);
                        pDynamicAppConfig->prefixes = (char*) malloc (iFieldLen+1);
                        memset (pDynamicAppConfig->prefixes, '\0', iFieldLen+1);
                        strcpy (pDynamicAppConfig->prefixes, itrPrefix->valuestring);
                    }
                }
                LOGINFO("");
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
                        int iFieldLen = strlen (itrCor->valuestring);
                        pDynamicAppConfig->cors = (char*) malloc (iFieldLen+1);
                        memset (pDynamicAppConfig->cors, '\0', iFieldLen+1);
                        strcpy (pDynamicAppConfig->cors, itrCor->valuestring);
                    }
                }
                LOGINFO("");
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
                    LOGINFO("");
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
                    LOGINFO("");
                }
                //Set launchParameters in list for later usage
                for (DynamicAppConfig* pDynamicAppConfig : appConfigListTemp) {
                    int iFieldLen = strlen (jQuery->valuestring);
                    pDynamicAppConfig->query = (char*) malloc (iFieldLen+1);
                    memset (pDynamicAppConfig->query, '\0', iFieldLen+1);
                    strcpy (pDynamicAppConfig->query, jQuery->valuestring);

                    iFieldLen = strlen (jPayload->valuestring);
                    pDynamicAppConfig->payload = (char*) malloc (iFieldLen+1);
                    memset (pDynamicAppConfig->payload, '\0', iFieldLen+1);
                    strcpy (pDynamicAppConfig->payload, jPayload->valuestring);
                }

            }
            for (DynamicAppConfig* pDynamicAppConfig : appConfigListTemp) {
                appConfigList.push_back(pDynamicAppConfig);
            }
            appConfigListTemp.clear();
            iIndex++;
        }
        LOGINFO("");
        DumpDynamicAppConfigCache(string("appConfigList"), appConfigList);
        vector<string> appsToDelete;
        for (DynamicAppConfig* pDynamicAppConfig : appConfigList) {
            appsToDelete.push_back(string(pDynamicAppConfig->appName));
        }
        deleteFromDynamicAppCache (appsToDelete);

        LOGINFO("appConfigList count: %d", appConfigList.size());
        //Update the new entries here.
        for (DynamicAppConfig* pDynamicAppConfig : appConfigList) {
            m_appConfigCache.push_back(pDynamicAppConfig);
        }
        LOGINFO("m_appConfigCache count: %d", m_appConfigCache.size());

        //Clear the tempopary list here
        appsToDelete.clear();
        appConfigList.clear();
        cJSON_Delete(applications);
    }
    DumpDynamicAppConfigCache(string("m_appConfigCache"), m_appConfigCache);
    return;
}


void RtXcastConnector::initDynamicAppCache()
{
    //If current cache is empty load with default values
    if ((!m_IsDefaultDynamicAppListEnabled) &&
        (0 == m_appConfigCache.size())) {
        LOGINFO("XcastService::initDynamicApplications m_appConfigCache is empty load with default values");
        updateDynamicAppCache(string(defaultDynamicAppsString));
    }
}

void RtXcastConnector::registerApplications()
{
    LOGINFO("XcastService::registerApplications");

    rtArrayObject *appReqList = new rtArrayObject;
    {lock_guard<mutex> lck(m_appConfigMutex);
        for (DynamicAppConfig* pDynamicAppConfig : m_appConfigCache) {
            //populate the rtParam here
            rtObjectRef appReq = new rtMapObject;

            rtArrayObject *appNameList = new rtArrayObject;
            appNameList->pushBack (pDynamicAppConfig->appName);
            appReq.set ("Names", rtValue(appNameList));

            if (NULL != pDynamicAppConfig->prefixes) {
                rtArrayObject *appPrefixes = new rtArrayObject;
                appPrefixes->pushBack (pDynamicAppConfig->prefixes);
                appReq.set ("prefixes", rtValue(appPrefixes));
            }

            if (NULL != pDynamicAppConfig->cors) {
                rtArrayObject *appCors = new rtArrayObject;
                appCors->pushBack (pDynamicAppConfig->cors);
                appReq.set ("cors", rtValue(appCors));
            }

            rtObjectRef appProp = new rtMapObject;
            appProp.set("allowStop",pDynamicAppConfig->allowStop);
            appReq.set ("properties", rtValue(appProp));

            appReqList->pushBack(rtValue(appReq));
        }
    }


    DumpDynamicAppConfigCache(string("m_appConfigCache"), m_appConfigCache);
    if((xdialCastObj != NULL) && (m_appConfigCache.size() > 0))
    {
        LOGINFO("%s:%d xdialCastObj Not NULL", __FUNCTION__, __LINE__);
        int ret = xdialCastObj.send("onRegisterApplications", appReqList);
        LOGINFO("XcastService send onRegisterApplications ret:%d",ret);
    }
    else
        LOGINFO(" xdialCastObj is NULL ");
}

void RtXcastConnector::unregisterApplications(string strApps)
{
    LOGINFO("XcastService::unregisterApplications");

    cJSON *itrAppName = NULL;

    if (!strApps.empty()) {
        cJSON *applications = cJSON_Parse(strApps.c_str());
        if (!cJSON_IsArray(applications)) {
            LOGINFO ("applications array passed: %s", strApps.c_str());
            LOGINFO ("\nInvalid applications array. exititng\n");
            cJSON_Delete(applications);
            return;
        }

        rtObjectRef appReq = new rtMapObject;
        appReq.set ("Names", rtValue(applications));
        /* iterate over ints */
        LOGINFO("Applications:\n");
        int iIndex = 0;
        rtArrayObject *appReqList = new rtArrayObject;
        appReqList->pushBack(rtValue(appReq));

        if(xdialCastObj != NULL)
        {
            LOGINFO("%s:%d xdialCastObj Not NULL strApps:%s", __FUNCTION__, __LINE__, strApps.c_str());
            int ret = xdialCastObj.send("onUnregisterApplications", appReqList);
            LOGINFO("XcastService send onRegisterApplications ret:%d",ret);
        }
        else
            LOGINFO(" xdialCastObj is NULL ");

        cJSON_ArrayForEach(itrAppName, applications) {
            LOGINFO("Application: %d \n", iIndex);
            if (!cJSON_IsString(itrAppName)) {
                LOGINFO ("\nInvalid name format at application index. Skipping%d\n", iIndex);
                continue;
            }
            LOGINFO("%s, ", itrAppName->valuestring);
            //Using itrAppName->valuestring remove entry from m_appLaunchParamList, saved for later use.
            iIndex++;
        }
        LOGINFO("\n");

        cJSON_Delete(applications);

    }
}


RtXcastConnector * RtXcastConnector::getInstance()
{
    if(RtXcastConnector::_instance == nullptr)
    {
        RtXcastConnector::_instance = new RtXcastConnector();
    }
    return RtXcastConnector::_instance;
}

bool RtXcastConnector::IsDynamicAppListEnabled()
{
    bool ret = false;
#ifdef RFC_ENABLED
    RFC_ParamData_t param;
    WDMP_STATUS wdmpStatus = getRFCParameter(const_cast<char *>("Xcast"), "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.XDial.DynamicAppList", &param);
    if (wdmpStatus == WDMP_SUCCESS || wdmpStatus == WDMP_ERR_DEFAULT_VALUE)
    {
        if( param.type == WDMP_BOOLEAN )
        {
            if(strncasecmp(param.value,"true",4) == 0 )
                ret = true;
        }
    }

    LOGINFO(" IsDynamicAppListEnabled enabled ? %d , call value %d ", ret, wdmpStatus);
#endif //RFC_ENABLED

    return ret;
}

bool RtXcastConnector::IsAppEnabled(char* strAppName)
{
    bool ret = false;
    char* strfound = NULL;
#ifdef RFC_ENABLED
    RFC_ParamData_t param;
    WDMP_STATUS wdmpStatus = getRFCParameter(const_cast<char *>("Xcast"), "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.XDial.AppList", &param);
    if (wdmpStatus == WDMP_SUCCESS || wdmpStatus == WDMP_ERR_DEFAULT_VALUE)
    {
        if (NULL != strAppName) {
            strfound = strstr(param.value, strAppName);
        }
        if (strfound) {
            ret = true;
        }
    }

    LOGINFO(" IsAppEnabled for %s enabled ? %d , call value %d ", strAppName, ret, wdmpStatus);
#endif //RFC_ENABLED

    return ret;
}
