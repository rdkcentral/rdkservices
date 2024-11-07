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

using namespace std;
using namespace WPEFramework;
#define LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS  5000  //5 seconds
#define LOCATE_CAST_SECOND_TIMEOUT_IN_MILLIS 15000  //15 seconds
#define LOCATE_CAST_THIRD_TIMEOUT_IN_MILLIS  30000  //30 seconds
#define LOCATE_CAST_FINAL_TIMEOUT_IN_MILLIS  60000  //60 seconds
#define EVENT_LOOP_ITERATION_IN_100MS     100000


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
    if(err != RT_OK){
        LOGINFO("Xcastservice: rtRemoteInit failed : Reason %s", rtStrError(err));
    }
    else {
        lock_guard<mutex> lock(m_threadlock);
        m_runEventThread = true;
        m_eventMtrThread = std::thread(threadRun, this);
    }
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

void RtXcastConnector::registerApplications(std::vector<DynamicAppConfig*>& appConfigList)
{
    LOGINFO("XcastService::registerApplications");

    rtArrayObject *appReqList = new rtArrayObject;
    for (DynamicAppConfig* pDynamicAppConfig : appConfigList) {
        //populate the rtParam here
        rtObjectRef appReq = new rtMapObject;

        rtArrayObject *appNameList = new rtArrayObject;
        appNameList->pushBack (pDynamicAppConfig->appName);
        appReq.set ("Names", rtValue(appNameList));

        if ('\0' != pDynamicAppConfig->prefixes[0]) {
            rtArrayObject *appPrefixes = new rtArrayObject;
            appPrefixes->pushBack (pDynamicAppConfig->prefixes);
            appReq.set ("prefixes", rtValue(appPrefixes));
        }

        if ('\0' != pDynamicAppConfig->cors[0]) {
            rtArrayObject *appCors = new rtArrayObject;
            appCors->pushBack (pDynamicAppConfig->cors);
            appReq.set ("cors", rtValue(appCors));
        }

        rtObjectRef appProp = new rtMapObject;
        appProp.set("allowStop",pDynamicAppConfig->allowStop);
        appReq.set ("properties", rtValue(appProp));

        appReqList->pushBack(rtValue(appReq));
    }

    if(xdialCastObj != NULL)
    {
        int ret = xdialCastObj.send("onRegisterApplications", appReqList);
        LOGINFO("XcastService send onRegisterApplications ret:%d",ret);
    }
    else
    {
        LOGINFO(" xdialCastObj is NULL ");
        delete appReqList;
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
#else
    ret = true;
#endif //RFC_ENABLED

    return ret;
}

bool RtXcastConnector::IsAppEnabled(char* strAppName)
{
    bool ret = false;
#ifdef RFC_ENABLED
    char* strfound = NULL;
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
#else
    ret = true;
#endif //RFC_ENABLED

    return ret;
}
