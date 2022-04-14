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
#include <cjson/cJSON.h>
#include "RtXcastConnector.h"
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

#define LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS  5000  //5 seconds
#define LOCATE_CAST_SECOND_TIMEOUT_IN_MILLIS 15000  //15 seconds
#define LOCATE_CAST_THIRD_TIMEOUT_IN_MILLIS  30000  //30 seconds
#define LOCATE_CAST_FINAL_TIMEOUT_IN_MILLIS  60000  //60 seconds

/*
 * The maximum DIAL payload accepted per the DIAL 1.6.1 specification.
 */
#define DIAL_MAX_PAYLOAD (4096)

/*
 * The maximum additionalDataUrl length
 */
#define DIAL_MAX_ADDITIONALURL (1024)


namespace WPEFramework {

namespace Plugin {

SERVICE_REGISTRATION(XCast, 1, 0);

static RtXcastConnector * _rtConnector  = RtXcastConnector::getInstance();
static int locateCastObjectRetryCount = 0;
bool XCast::isCastEnabled = false;
bool XCast::m_xcastEnable= false;
string XCast::m_friendlyName = "";
bool XCast::m_standbyBehavior = false;
bool XCast::m_enableStatus = false;
string strDyAppConfig = "";

IARM_Bus_PWRMgr_PowerState_t XCast::m_powerState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;

XCast::XCast() : AbstractPlugin()
, m_apiVersionNumber(1)
{
    InitializeIARM();
    XCast::checkRFCServiceStatus();
    if(XCast::isCastEnabled)
    {
        LOGINFO("XcastService::Register methods and create onLocateCastTimer ");
        registerMethod(METHOD_GET_API_VERSION_NUMBER, &XCast::getApiVersionNumber, this);
        registerMethod(METHOD_ON_APPLICATION_STATE_CHANGED , &XCast::applicationStateChanged, this);
        registerMethod(METHOD_SET_ENABLED, &XCast::setEnabled, this);
        registerMethod(METHOD_GET_ENABLED, &XCast::getEnabled, this);
        registerMethod(METHOD_GET_STANDBY_BEHAVIOR, &XCast::getStandbyBehavior, this);
        registerMethod(METHOD_SET_STANDBY_BEHAVIOR, &XCast::setStandbyBehavior, this);
        registerMethod(METHOD_GET_FRIENDLYNAME, &XCast::getFriendlyName, this);
        registerMethod(METHOD_SET_FRIENDLYNAME, &XCast::setFriendlyName, this);
        registerMethod(METHOD_REG_APPLICATIONS, &XCast::registerApplications, this);
        registerMethod(METHOD_GET_PROTOCOLVERSION, &XCast::getProtocolVersion, this);
        
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
         IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED) );
     }
     Unregister(METHOD_GET_API_VERSION_NUMBER);
     Unregister(METHOD_ON_APPLICATION_STATE_CHANGED);
     Unregister(METHOD_SET_ENABLED);
     Unregister(METHOD_GET_ENABLED);
     Unregister(METHOD_GET_STANDBY_BEHAVIOR);
     Unregister(METHOD_SET_STANDBY_BEHAVIOR);
     Unregister(METHOD_GET_FRIENDLYNAME);
     Unregister(METHOD_SET_FRIENDLYNAME);

     DeinitializeIARM();
     if ( m_locateCastTimer.isActive())
     {
         m_locateCastTimer.stop();
     }
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
        _rtConnector->enableCastService(m_friendlyName,false);
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
    m_xcastEnable= enabled;
    if (m_xcastEnable && ( (m_standbyBehavior == true) || ((m_standbyBehavior == false)&&(m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON)) ) )
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
            if (m_xcastEnable && ( (m_standbyBehavior == true) || ((m_standbyBehavior == false)&&(m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON)) ) ) {
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

               _rtConnector->registerApplications (parameters["applications"].String());

               /*Save the config*/
               strDyAppConfig.assign(parameters["applications"].String());
               /*Reenabling cast service after registering Applications*/
               if (m_xcastEnable && ( (m_standbyBehavior == true) || ((m_standbyBehavior == false)&&(m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON)) ) ) {
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

    if ((!strDyAppConfig.empty()) && (NULL != _rtConnector)) {
        if (_rtConnector->IsDynamicAppListEnabled()) {
            LOGINFO("XCast::onLocateCastTimer : strDyAppConfig: %s", strDyAppConfig.c_str());
            _rtConnector->registerApplications (strDyAppConfig);
        }
        else {
            LOGINFO("XCast::onLocateCastTimer : DynamicAppList not enabled");
        }
    }
    else {
        LOGINFO("XCast::onLocateCastTimer : strDyAppConfig: %s _rtConnector: %p", strDyAppConfig.c_str(), _rtConnector);
    }
    if (m_xcastEnable && ( (m_standbyBehavior == true) || ((m_standbyBehavior == false)&&(m_powerState == IARM_BUS_PWRMGR_POWERSTATE_ON)) ) ) {
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

    memset (url, '\0', sizeof(url));
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
        memset( url, 0, sizeof(url) );
        strcat( url, "source_type=12" );
        if(payload != NULL)
        {
            const char * pUrlEncodedParams;
            pUrlEncodedParams = payload;
            if( pUrlEncodedParams ){
                strcat( url, "&dial=");
                strcat( url, pUrlEncodedParams );
            }
        }

        if(additional_data_url != NULL){
            strcat(url, "&additionalDataUrl=");
            strcat(url, additional_data_url);
        }
    }
    else {
        int url_len = DIAL_MAX_PAYLOAD+DIAL_MAX_ADDITIONALURL+100;
        {
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
}

void XCast::onXcastApplicationLaunchRequestWithLaunchParam (string appName,
        string strPayLoad, string strQuery, string strAddDataUrl)
{
    //TODO
    LOGINFO ("XcastService::onXcastApplicationLaunchRequestWithLaunchParam ");
    JsonObject params;
    JsonObject urlParam;
    char url[DIAL_MAX_PAYLOAD+DIAL_MAX_ADDITIONALURL+100] = {0,};

    if(_rtConnector) {
        RegAppLaunchParams reqParam;
        _rtConnector->getEntryFromAppLaunchParamList (appName.c_str(), &reqParam);

        /*Replacing with App requested payload and query*/
        if (reqParam.query && reqParam.payload) {
            getUrlFromAppLaunchParams (appName.c_str(),
                               reqParam.payload,
                               reqParam.query,
                               strAddDataUrl.c_str(), url);
        }
        else if(reqParam.payload){
            getUrlFromAppLaunchParams (appName.c_str(),
                               reqParam.payload,
                               strQuery.c_str(),
                               strAddDataUrl.c_str(), url);
        }
        else if(reqParam.query) {
            getUrlFromAppLaunchParams (appName.c_str(),
                               strPayLoad.c_str(),
                               reqParam.query,
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
