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


static gdialService* gdialCastObj = NULL;
RtXcastConnector * RtXcastConnector::_instance = nullptr;

//XDIALCAST EVENT CALLBACK
/**
 * Callback function for application launch request from an app
 */
void RtXcastConnector::onApplicationLaunchRequestWithLaunchParam(string appName,string strPayLoad, string strQuery, string strAddDataUrl)
{
    if ( nullptr != m_observer )
    {
        m_observer->onXcastApplicationLaunchRequestWithLaunchParam(appName,strPayLoad,strQuery,strAddDataUrl);
    }
}

void RtXcastConnector::onApplicationLaunchRequest(string appName, string parameter)
{
    if ( nullptr != m_observer )
    {
        if (!strcmp(appName.c_str(),"Netflix"))
        {
            appName = "NetflixApp";
        }
        m_observer->onXcastApplicationLaunchRequest(appName,parameter);
    }
}

void RtXcastConnector::onApplicationStopRequest(string appName, string appID)
{
    if ( nullptr != m_observer )
    {
        if (!strcmp(appName.c_str(),"Netflix"))
        {
            appName = "NetflixApp";
        }
        m_observer->onXcastApplicationStopRequest(appName,appID);
    }
}

void RtXcastConnector::onApplicationHideRequest(string appName, string appID)
{
    if ( nullptr != m_observer )
    {
        if (!strcmp(appName.c_str(),"Netflix"))
        {
            appName = "NetflixApp";
        }
        m_observer->onXcastApplicationHideRequest(appName,appID);
    }
}

void RtXcastConnector::onApplicationResumeRequest(string appName, string appID)
{
    if ( nullptr != m_observer )
    {
        if (!strcmp(appName.c_str(),"Netflix"))
        {
            appName = "NetflixApp";
        }
        m_observer->onXcastApplicationResumeRequest(appName,appID);
    }
}

void RtXcastConnector::onApplicationStateRequest(string appName, string appID)
{
    if ( nullptr != m_observer )
    {
        if (!strcmp(appName.c_str(),"Netflix"))
        {
            appName = "NetflixApp";
        }
        m_observer->onXcastApplicationStateRequest(appName,appID);
    }
}

RtXcastConnector::~RtXcastConnector()
{
    _instance = nullptr;
    m_observer = nullptr;
}

bool RtXcastConnector::initialize()
{
    // @@@ TODO for GDial command line arguments @@@
    std::vector<std::string> gdial_args;
    gdialCastObj = gdialService::getInstance(this,gdial_args);
    return (nullptr != gdialCastObj) ? true:false;
}
void RtXcastConnector::shutdown()
{
    LOGINFO("Shutting down rtRemote connectivity");
    gdialService::destroyInstance();
    gdialCastObj = nullptr;
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
    if(gdialCastObj != NULL)
    {
        gdialCastObj->ApplicationStateChanged( app, state, id, error);
        status = 1;
    }
    else
        LOGINFO(" gdialCastObj is NULL ");
    return status;
}//app && state not empty

void RtXcastConnector::enableCastService(string friendlyname,bool enableService)
{
    LOGINFO("XcastService::enableCastService ARGS = %s : %d ", friendlyname.c_str(), enableService);
    if(gdialCastObj != NULL)
    {
        std::string activation = enableService ? "true": "false";
        gdialCastObj->ActivationChanged( activation, friendlyname);
        LOGINFO("XcastService send onActivationChanged");
    }
    else
        LOGINFO(" gdialCastObj is NULL ");    
}

void RtXcastConnector::updateFriendlyName(string friendlyname)
{
    LOGINFO("XcastService::updateFriendlyName ARGS = %s ", friendlyname.c_str());
    if(gdialCastObj != NULL)
    {
        gdialCastObj->FriendlyNameChanged( friendlyname);
        LOGINFO("XcastService send onFriendlyNameChanged");
    }
    else
        LOGINFO(" gdialCastObj is NULL ");
}

string RtXcastConnector::getProtocolVersion(void)
{
    LOGINFO("XcastService::getProtocolVersion ");
    std::string strVersion;
    if(gdialCastObj != NULL)
    {
        strVersion = gdialCastObj->getProtocolVersion();
        LOGINFO("XcastService getProtocolVersion version:%s ",strVersion.c_str());
    }
    else
    {
        LOGINFO(" XcastService getProtocolVersion gdialCastObj is NULL so returns 2.1");
	    strVersion = "2.1";
    }
    return strVersion;
}

void RtXcastConnector::registerApplications(std::vector<DynamicAppConfig*>& appConfigList)
{
    LOGINFO("XcastService::RegisterAppEntryList");

    RegisterAppEntryList *appReqList = new RegisterAppEntryList;

    for (DynamicAppConfig* pDynamicAppConfig : appConfigList)
    {
        RegisterAppEntry* appReq = new RegisterAppEntry;
        
        appReq->Names = pDynamicAppConfig->appName;
        appReq->prefixes = pDynamicAppConfig->prefixes;
        appReq->cors = pDynamicAppConfig->cors;
        appReq->allowStop = pDynamicAppConfig->allowStop;

        appReqList->pushBack(appReq);
    }

    if(gdialCastObj != NULL)
    {
        gdialCastObj->RegisterApplications(appReqList);
        LOGINFO("XcastService send onRegisterAppEntryList");
    }
    else
    {
        LOGINFO(" gdialCastObj is NULL ");
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