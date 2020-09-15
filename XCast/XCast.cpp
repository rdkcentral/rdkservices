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
#include "utils.h"
#ifdef RFC_ENABLED
#include "rfcapi.h"
#endif //RFC_ENABLED
#include <syscall.h>
#include <cstring>
#include "RtXcastConnector.h"
using namespace std;

#if defined(HAS_PERSISTENT_IN_HDD)
#define XCAST_SETTING_ENABLED_FILE "/tmp/mnt/diska3/persistent/ds/xcastData"
#else
#define XCAST_SETTING_ENABLED_FILE "/opt/persistent/ds/xcastData"
#endif
#define XCAST_SETTING_ENABLED "xcastEnabled"
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

#define LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS  5000  //5 seconds
#define LOCATE_CAST_SECOND_TIMEOUT_IN_MILLIS 15000  //15 seconds
#define LOCATE_CAST_THIRD_TIMEOUT_IN_MILLIS  30000  //30 seconds
#define LOCATE_CAST_FINAL_TIMEOUT_IN_MILLIS  60000  //60 seconds

namespace WPEFramework {

namespace Plugin {

SERVICE_REGISTRATION(XCast, 1, 0);

static RtXcastConnector * _rtConnector  = RtXcastConnector::getInstance();
static int locateCastObjectRetryCount = 0;
bool XCast::isCastEnabled = false;
bool XCast::m_xcastEnableSettings = false;
IARM_Bus_PWRMgr_PowerState_t XCast::m_powerState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;

XCast::XCast() : AbstractPlugin()
, m_apiVersionNumber(1)
{
    InitializeIARM();
    m_xcastEnableSettings = XCast::checkXcastSettingsStatus();
    LOGINFO("XcastService::m_xcastEnableSettings :%d ",m_xcastEnableSettings);
    XCast::checkRFCServiceStatus();
    if(XCast::isCastEnabled)
    {
        registerMethod(METHOD_GET_API_VERSION_NUMBER, &XCast::getApiVersionNumber, this);
        registerMethod(METHOD_ON_APPLICATION_STATE_CHANGED , &XCast::applicationStateChanged, this);
        registerMethod(METHOD_SET_ENABLED, &XCast::setEnabled, this);
        registerMethod(METHOD_GET_ENABLED, &XCast::getEnabled, this);
        
        m_locateCastTimer.connect( bind( &XCast::onLocateCastTimer, this ));
        m_locateCastTimer.setSingleShot(true);
    }
}

XCast::~XCast()
{
    Unregister(METHOD_GET_API_VERSION_NUMBER);
    Unregister(METHOD_ON_APPLICATION_STATE_CHANGED);
    DeinitializeIARM();
    if ( m_locateCastTimer.isActive())
    {
        m_locateCastTimer.stop();
    }
}
const void XCast::InitializeIARM()
{
     LOGINFO();
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
     LOGINFO();
     if (Utils::IARM::isConnected())
     {
         IARM_Result_t res;
         IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED) );
     }
}
void XCast::powerModeChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
     LOGINFO();
     if (strcmp(owner, IARM_BUS_PWRMGR_NAME)  == 0) {
         if (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED ) {
             IARM_Bus_PWRMgr_EventData_t *param = (IARM_Bus_PWRMgr_EventData_t *)data;
             LOGINFO("Event IARM_BUS_PWRMGR_EVENT_MODECHANGED: State Changed %d -- > %d\r",
                     param->data.state.curState, param->data.state.newState);
            m_powerState = param->data.state.newState;
            if(m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON && m_xcastEnableSettings)
                _rtConnector->enableCastService(true);
            else
                _rtConnector->enableCastService(false);
         }
    }
}

const string XCast::Initialize(PluginHost::IShell* /* service */)
{
    LOGINFO("Activate plugin.");
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
    if( XCast::isCastEnabled){
        _rtConnector->shutdown();
    }
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
    //persist value
    m_xcastEnableSettings = enabled;
    persistEnabledSettings(enabled);
    //apply settings
    if (enabled && m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON)
        _rtConnector->enableCastService(true);
    else
        _rtConnector->enableCastService(false);
    returnResponse(true);
}
uint32_t XCast::getEnabled(const JsonObject& parameters, JsonObject& response)
{
    LOGINFO("XcastService::getEnabled ");
    response["enabled"] = m_xcastEnableSettings;
    returnResponse(true);
}
void XCast::persistEnabledSettings(bool enableStatus)
{
    Core::File file;
    file = XCAST_SETTING_ENABLED_FILE;
    file.Open(false);
    if (!file.IsOpen())
        file.Create();
    JsonObject enableSetting;
    enableSetting.IElement::FromFile(file);
    file.Destroy();
    file.Create();
    enableSetting[XCAST_SETTING_ENABLED] = enableStatus;
    enableSetting.IElement::ToFile(file);
    file.Close();
    return;
}
//Timer Functions
void XCast::onLocateCastTimer()
{
    int status = _rtConnector->connectToRemoteService();
    
    
    if(status != 0)
    {
        locateCastObjectRetryCount++;
        if(locateCastObjectRetryCount == 1)
        {
            LOGINFO("Retry after 5 sec...");
            m_locateCastTimer.start(LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS);
        }
        if(locateCastObjectRetryCount == 2)
        {
            LOGINFO("Retry after 15 sec...");
            m_locateCastTimer.start(LOCATE_CAST_SECOND_TIMEOUT_IN_MILLIS);
        }
        if(locateCastObjectRetryCount == 3)
        {
            LOGINFO("Retry after 30 sec...");
            m_locateCastTimer.start(LOCATE_CAST_THIRD_TIMEOUT_IN_MILLIS);
        }
        if(locateCastObjectRetryCount == 4)
        {
            LOGINFO("Retry after 60 sec...");
            m_locateCastTimer.start(LOCATE_CAST_FINAL_TIMEOUT_IN_MILLIS);
        }
        return ;
    }// err != RT_OK
    locateCastObjectRetryCount = 0;
    m_locateCastTimer.stop();
    if (m_xcastEnableSettings && m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON)
        _rtConnector->enableCastService(true);
    else
        _rtConnector->enableCastService(false);
    
   LOGINFO("XCast::onLocateCastTimer : Timer still active ? %d ",m_locateCastTimer.isActive());
}

void XCast::onRtServiceDisconnected() 
{
    LOGINFO("RT communication failure. Reconnecting.. ");
    m_locateCastTimer.start(LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS);
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
    LOGINFO();
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
bool XCast::checkXcastSettingsStatus()
{
    LOGINFO();
    Core::File file;
    JsonObject parameters;
    bool xcastEnableStatus = false;
    file = XCAST_SETTING_ENABLED_FILE;
    file.Open(false);
    if (!file.IsOpen())
    {
        LOGINFO("XcastService::persistance file not present create with default setting true);
        file.Create();
        JsonObject parameters;
        parameters[XCAST_SETTING_ENABLED] = true;
        xcastEnableStatus = true;
        parameters.IElement::ToFile(file);
    }
    else
    {
        parameters.IElement::FromFile(file);
        if( parameters.HasLabel(XCAST_SETTING_ENABLED))
        {
            getBoolParameter(XCAST_SETTING_ENABLED, xcastEnableStatus);
            LOGINFO("XcastService:: xcastEnableStatus  :%d",xcastEnableStatus);
        }
        else
        {
            LOGINFO("XcastService:: XCAST_SETTING_ENABLED not present create with default setting true");
            parameters[XCAST_SETTING_ENABLED] = true;
            xcastEnableStatus = true;
            parameters.IElement::ToFile(file);
        }
    }
    file.Close();
    return xcastEnableStatus;
}
} // namespace Plugin
} // namespace WPEFramework
