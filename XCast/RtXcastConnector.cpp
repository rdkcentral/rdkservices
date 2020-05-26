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
#include "utils.h"

using namespace std;
using namespace WPEFramework;
#define LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS  5000  //5 seconds
#define LOCATE_CAST_SECOND_TIMEOUT_IN_MILLIS 15000  //15 seconds
#define LOCATE_CAST_THIRD_TIMEOUT_IN_MILLIS  30000  //30 seconds
#define LOCATE_CAST_FINAL_TIMEOUT_IN_MILLIS  60000  //60 seconds


static rtObjectRef xdialCastObj = NULL;
RtXcastConnector * RtXcastConnector::_instance = nullptr;

static void remoteDisconnectCallback( void* data) {
    char* serviceName = (char* ) data;
    LOGINFO ( "remoteDisconnectCallback: Remote %s disconnected... ",  serviceName);
    RtXcastConnector::getInstance()->onRtServiceDisconnected();
}

//XDIALCAST EVENT CALLBACK
/**
 * Callback function for application launch request from an app
 */
rtError RtXcastConnector::onApplicationLaunchRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context)
{
    LOGINFO ("RtXcastConnector::onXcastApplicationLaunchRequest ");
    if (numArgs == 1)
    {
        RtNotifier * observer = static_cast<RtNotifier *> (context);
        rtObjectRef appObject = args[0].toObject();
        rtString appName = appObject.get<rtString>("applicationName");
        if (!strcmp(appName.cString(),"Netflix"))
            appName = "NetflixApp";
        rtString rtparams = appObject.get<rtString>("parameters");
        observer->onXcastApplicationLaunchRequest(appName.cString() , rtparams.cString());
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
        LOGINFO("RtXcastConnector::onXcastApplicationStopRequest ");
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
        LOGINFO("RtXcastConnector::onXcastApplicationHideRequest : ");
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
    LOGINFO(" Received a callback event" );
    if (numArgs == 1)
    {
        LOGINFO("RtXcastConnector::onXcastApplicationStateRequest: ");
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
    
    err = rtRemoteLocateObject(rtEnvironmentGetGlobal(), serviceName, xdialCastObj, 3000, &remoteDisconnectCallback, (void *)serviceName);
    if(err == RT_OK && xdialCastObj != NULL)
    {
        rtError e = xdialCastObj.send("on", "onApplicationLaunchRequest" , new rtFunctionCallback(&RtXcastConnector::onApplicationLaunchRequestCallback, m_observer));
        LOGINFO("Registered onApplicationLaunchRequest ; response %d" ,e );
        e = xdialCastObj.send("on", "onApplicationStopRequest" , new rtFunctionCallback(&RtXcastConnector::onApplicationStopRequestCallback, m_observer));
        LOGINFO("Registered onApplicationStopRequest %d", e );
        e = xdialCastObj.send("on", "onApplicationHideRequest" , new rtFunctionCallback( &RtXcastConnector::onApplicationHideRequestCallback, m_observer));
        LOGINFO("Registered onApplicationHideRequest %d", e );
        e = xdialCastObj.send("on", "onApplicationResumeRequest" , new rtFunctionCallback( &RtXcastConnector::onApplicationResumeRequestCallback, m_observer));
        LOGINFO("Registered onApplicationResumeRequest %d", e );
        e = xdialCastObj.send("on", "onApplicationStateRequest" , new rtFunctionCallback( &RtXcastConnector::onApplicationStateRequestCallback, m_observer));
        LOGINFO("Registed onApplicationStateRequest %d", e );
        e = xdialCastObj.send("on", "bye" , new rtFunctionCallback( &RtXcastConnector::onRtServiceByeCallback, m_observer));
        LOGINFO("Registed rtService bye event %d", e );
        enableCastService();
        
    }
    else
        LOGINFO("response of rtRemoteLocateObject %d ",   err);
    return (err == RT_OK) ? 0 : 1;
}


RtXcastConnector::~RtXcastConnector()
{
    LOGINFO("Dtr");
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
    return (err == RT_OK) ? true:false;
}
void RtXcastConnector::shutdown()
{
    LOGINFO("Shutting down rtRemote connectivity");
    rtRemoteShutdown(rtEnvironmentGetGlobal());
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
        xdialCastObj.send("applicationStateChanged", e);
        status = 1;
    }
    return status;
}//app && state not empty
void RtXcastConnector::enableCastService(bool enableService)
{
    if(xdialCastObj != NULL)
    {
        rtObjectRef e = new rtMapObject;
        e.set("activation",(enableService ? "true": "false"));
        xdialCastObj.send("onActivationChanged", e);
    }
    else
        LOGINFO(" xdialCastObj is NULL ");
    
}

void RtXcastConnector::onRtServiceDisconnected()
{
    LOGINFO("RT communication failure. Reconnecting.. ");
}


RtXcastConnector * RtXcastConnector::getInstance()
{
    if(RtXcastConnector::_instance == nullptr)
    {
        RtXcastConnector::_instance = new RtXcastConnector();
    }
    return RtXcastConnector::_instance;
}

void RtXcastConnector::sendPingMessage(){
    rtObjectRef pingObj = new rtMapObject;
    rtError err = xdialCastObj.send("ping",pingObj);
    if(err != RT_OK) {
        if (err == RT_ERROR_STREAM_CLOSED)
        {
            LOGINFO("RT_ERROR_STREAM_CLOSED xdialCast Disconnected");
            onRtServiceDisconnected();
        }
        else {
            LOGINFO("Caught an unknown exception: %s",rtStrError(err));
        }
        
    }
}

