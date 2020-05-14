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

//I have put several "TODO(MROLLINS)" in the code below to mark areas of concern I encountered
//  when refactoring the servicemanager's version of displaysettings into this new thunder plugin format

#include "XCast.h"
#include "tracing/Logging.h"
#include "utils.h"
#include "rfcapi.h"
#include <syscall.h>
#include <strings.h>
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
#define METHOD_GET_QUIRKS                    "getQuirks"
#define METHOD_GET_API_VERSION_NUMBER        "getApiVersionNumber"

#define LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS  5000  //5 seconds
#define LOCATE_CAST_SECOND_TIMEOUT_IN_MILLIS 15000  //15 seconds
#define LOCATE_CAST_THIRD_TIMEOUT_IN_MILLIS  30000  //30 seconds
#define LOCATE_CAST_FINAL_TIMEOUT_IN_MILLIS  60000  //60 seconds

namespace WPEFramework {

    namespace Plugin {

        SERVICE_REGISTRATION(XCast, 1, 0);

        static int locateCastObjectRetryCount = 0;
        static rtObjectRef xdialCastObj = NULL;
        XCast * XCast::_instance = nullptr;
        bool XCast::isCastEnabled = false;


        static void remoteDisconnectCallback( void* data) {
                char* serviceName = (char* ) data;
                 LOGINFO ( "remoteDisconnectCallback: Remote %s disconnected... ",  serviceName);
                XCast::_instance->onRtServiceDisconnected();
        }

        //XDIALCAST EVENT CALLBACK
        static rtError onApplicationLaunchRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context)
        {
                if (numArgs == 1)
                {
                        rtObjectRef appObject = args[0].toObject();
                        XCast::_instance->onXcastApplicationLaunchRequest(appObject);
                }
                else
                        rtLogError("*** Error: received unknown event");
                if (result)
                        *result = rtValue(true);
                return RT_OK;
        }
        static rtError onApplicationStopRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context)
        {
                if (numArgs == 1)
                {
                        rtObjectRef appObject = args[0].toObject();
                        XCast::_instance->onXcastApplicationStopRequest(appObject);
                }
                else
                     rtLogError("*** Error: received unknown event");
                if (result)
                      *result = rtValue(true);
                return RT_OK;
        }
        static rtError onApplicationHideRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context)
        {
            if (numArgs == 1)
            {
                rtObjectRef appObject = args[0].toObject();
                XCast::_instance->onXcastApplicationHideRequest(appObject);
            }
            else
                rtLogError("*** Error: received unknown event");

            if (result)
                *result = rtValue(true);

            return RT_OK;

        }
        static rtError onApplicationStateRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context)
        {
            if (numArgs == 1)
            {
                rtObjectRef appObject = args[0].toObject();
                XCast::_instance->onXcastApplicationStateRequest(appObject);
            }
            else
                rtLogError("*** Error: received unknown event");

            if (result)
                *result = rtValue(true);

            return RT_OK;
        }
        static rtError onApplicationResumeRequestCallback(int numArgs, const rtValue* args, rtValue* result, void* context)
        {
           if (numArgs == 1)
           {
                rtObjectRef appObject = args[0].toObject();
                XCast::_instance->onXcastApplicationResumeRequest(appObject);
           }
           else
                rtLogError("*** Error: received unknown event");

            if (result)
                *result = rtValue(true);

            return RT_OK;
        }
        static rtError onRtServiceByeCallback(int numArgs, const rtValue* args, rtValue* result, void* context)
        {
           if (numArgs == 1)
           {
                rtObjectRef appObject = args[0].toObject();
                rtString serviceName = appObject.get<rtString>("serviceName");
                LOGINFO("Received RtService Bye Event! Service: %s", serviceName.cString());
           }
           else
                rtLogError("*** Error: received unknown event");

           if (result)
                *result = rtValue(true);

            return RT_OK;

        }

        void XCast::registerForXcastEvents(void *context)
        {
                LOGINFO("registerForXcastEvents: ");

                if(xdialCastObj != NULL)
                {
                        rtError e = xdialCastObj.send("on", "onApplicationLaunchRequest" , new rtFunctionCallback(&onApplicationLaunchRequestCallback, context));
                        LOGINFO("Registered onApplicationLaunchRequest ; response %d" ,e );
                        e = xdialCastObj.send("on", "onApplicationStopRequest" , new rtFunctionCallback(&onApplicationStopRequestCallback, context));
                        LOGINFO("Registered onApplicationStopRequest %d", e );
                        e = xdialCastObj.send("on", "onApplicationHideRequest" , new rtFunctionCallback(&onApplicationHideRequestCallback, context));
                        LOGINFO("Registered onApplicationHideRequest %d", e );
                        e = xdialCastObj.send("on", "onApplicationResumeRequest" , new rtFunctionCallback(&onApplicationResumeRequestCallback, context));
                        LOGINFO("Registered onApplicationResumeRequest %d", e );
                        e = xdialCastObj.send("on", "onApplicationStateRequest" , new rtFunctionCallback(&onApplicationStateRequestCallback, context));
                        LOGINFO("Registed onApplicationStateRequest %d", e );
                        e = xdialCastObj.send("on", "bye" , new rtFunctionCallback(&onRtServiceByeCallback, context));
                        LOGINFO("Registed rtService bye event %d", e );
                }

        }

        XCast::XCast() : AbstractPlugin()
        , m_apiVersionNumber(1)
        {
            LOGINFO("New Instance");
            XCast::checkServiceStatus();
            if(XCast::isCastEnabled)
            {
                registerMethod(METHOD_GET_QUIRKS, &XCast::getQuirks, this);
                registerMethod(METHOD_GET_API_VERSION_NUMBER, &XCast::getApiVersionNumber, this);
                registerMethod(METHOD_ON_APPLICATION_STATE_CHANGED , &XCast::applicationStateChanged, this);

                m_locateCastTimer.connect( bind( &XCast::onLocateCastTimer, this ));
                m_locateCastTimer.setSingleShot(true);
            }
            _instance = this;

        }

        XCast::~XCast()
        {
            LOGINFO("Dtr");
            Unregister(METHOD_GET_QUIRKS);
            Unregister(METHOD_GET_API_VERSION_NUMBER);
            Unregister(METHOD_ON_APPLICATION_STATE_CHANGED);

            if ( m_locateCastTimer.isActive())
            {
                m_locateCastTimer.stop();
            }
        }

        const string XCast::Initialize(PluginHost::IShell* /* service */)
        {
            LOGINFO("Activate");
            if (XCast::isCastEnabled)
            {
                rtError err;
                rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
                err = rtRemoteInit(env);

                if(err != RT_OK){
                         std::cout<<"Xcastservice: rtRemoteInit failed"<<rtStrError(err)<<std::endl;
                }
                else
                {
                    //We give few seconds delay before the timer is fired.
                    m_locateCastTimer.start(LOCATE_CAST_THIRD_TIMEOUT_IN_MILLIS);
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
            LOGINFO("Deactivate");
            if( XCast::isCastEnabled)
                rtRemoteShutdown(rtEnvironmentGetGlobal());
        }

        string XCast::Information() const
        {
            // No additional info to report.
            return (string());
        }


        //Begin methods
        uint32_t XCast::getQuirks(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            JsonArray array;
            response["quirks"] = array;
            returnResponse(true);
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
                if(xdialCastObj != NULL)
                {
                    rtObjectRef e = new rtMapObject;
                    e.set("applicationName", app.c_str());
                    e.set("applicationId", id.c_str());
                    e.set("state",state.c_str());
                    e.set("error",error.c_str());
                    xdialCastObj.send("onApplicationStateChanged", e);
                    returnResponse(true);
                }
            }//app && state not empty
            returnResponse(false);
        }
        //Timer Functions
        void XCast::onLocateCastTimer()
        {
                rtError err = RT_ERROR;

                const char * serviceName = "com.comcast.xdialcast";

                err = rtRemoteLocateObject(rtEnvironmentGetGlobal(), serviceName, xdialCastObj, 3000, &remoteDisconnectCallback, (void *)serviceName);
                LOGINFO("XCast::onLocateCastTimer : response of rtRemoteLocateObject %d ", err);
                if(err != RT_OK)
                {
                        locateCastObjectRetryCount++;
                        LOGINFO("Xcastservice: Remote Object com.comcast.xdialcast locate error: %s", rtStrError(err));
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
                registerForXcastEvents(this);
                m_locateCastTimer.stop();
                LOGINFO("XCast::onLocateCastTimer : Timer still active ? %d ",m_locateCastTimer.isActive());
        }

        void XCast::onRtServiceDisconnected()
        {
                LOGINFO("RT communication failure. Reconnecting.. ");
                m_locateCastTimer.start(LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS);
        }

        void XCast::onXcastApplicationLaunchRequest(rtObjectRef appObject)
        {
                LOGINFO ("XcastService::onXcastApplicationLaunchRequest ");
                rtString appName = appObject.get<rtString>("applicationName");
                if (!strcmp(appName.cString(),"Netflix"))
                        appName = "NetflixApp";
                rtString rtparams = appObject.get<rtString>("parameters");
                LOGINFO ("Received ApplicationLaunchRequest  AppName: %s Params: %s ", appName.cString(), rtparams.cString());

                JsonObject params;
                params["applicationName"] = appName.cString();

                if (appName == "NetflixApp")
                        params["pluginUrl"]=rtparams.cString();
                if (appName == "YouTube")
                        params["url"]=rtparams.cString();

                params["parameters"]= rtparams.cString();

                sendNotify(EVT_ON_LAUNCH_REQUEST, params);
        }
        void XCast::onXcastApplicationStopRequest(rtObjectRef appObject)
        {
                LOGINFO("XcastService::onXcastApplicationStopRequest ");
                rtString appName = appObject.get<rtString>("applicationName");
                if (!strcmp(appName.cString(),"Netflix"))
                        appName = "NetflixApp";

                rtString appID = appObject.get<rtString>("applicationId");
                LOGINFO("Received ApplicationStopRequest  AppName: %s App Id : %s", appName.cString() , appID.cString());

                JsonObject params;
                params["applicationName"] = appName.cString();
                params["applicationId"]= appID.cString();

                sendNotify(EVT_ON_STOP_REQUEST, params);
        }
        void XCast::onXcastApplicationHideRequest(rtObjectRef appObject)
        {
            LOGINFO("XcastService::onXcastApplicationHideRequest : ");
            rtString appName = appObject.get<rtString>("applicationName");
            if (!strcmp(appName.cString(),"Netflix"))
               appName = "NetflixApp";
            rtString appID = appObject.get<rtString>("applicationId");
            LOGINFO("Received ApplicationHideRequest  AppName: %s AppID: %s", appName.cString(), appID.cString());


            JsonObject params;
            params["applicationName"] = appName.cString();
            params["applicationId"]= appID.cString();

            sendNotify(EVT_ON_HIDE_REQUEST, params);
        }
        void XCast::onXcastApplicationStateRequest(rtObjectRef appObject)
        {
            LOGINFO("XcastService::onXcastApplicationStateRequest: ");
            rtString appName = appObject.get<rtString>("applicationName");
            if (!strcmp(appName.cString(),"Netflix"))
                     appName = "NetflixApp";

            rtString appID = appObject.get<rtString>("applicationId");
            LOGINFO("Received onXcastApplicationStateRequest  AppName: %s AppID: %s", appName.cString(), appID.cString());

            JsonObject params;
            params["applicationName"] = appName.cString();
            params["applicationId"]= appID.cString();

            string json_str;
            params.ToString(json_str);
            sendNotify(EVT_ON_STATE_REQUEST , params);

        }
        void XCast::onXcastApplicationResumeRequest(rtObjectRef appObject)
        {
            LOGINFO("XcastService::onXcastApplicationResumeRequest ");
            rtString appName = appObject.get<rtString>("applicationName");
            if (!strcmp(appName.cString(),"Netflix"))
               appName = "NetflixApp";
            rtString appID = appObject.get<rtString>("applicationId");
            LOGINFO("Received ApplicationResumeRequest  AppName: %s AppID: %s" , appName.cString(), appID.cString());

            JsonObject params;
            params["applicationName"] = appName.cString();
            params["applicationId"]= appID.cString();
            sendNotify(EVT_ON_RESUME_REQUEST, params);
        }

        bool XCast::checkServiceStatus()
        {
                LOGINFO();
                RFC_ParamData_t param;
                WDMP_STATUS wdmpStatus = getRFCParameter(const_cast<char *>("Xcast"), "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.XDial.Enable", &param);
                if (wdmpStatus == WDMP_SUCCESS || wdmpStatus == WDMP_ERR_DEFAULT_VALUE)
                {
                        if( param.type == WDMP_BOOLEAN )
                        {
                                if(strncasecmp(param.value,"true",4) == 0 )
                                        isCastEnabled = true;
                        }
                }
                LOGINFO(" Is cast enabled ? %d , call value %d ", isCastEnabled, wdmpStatus);
                return isCastEnabled;
        }
    } // namespace Plugin
} // namespace WPEFramework
