/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#include "XCastImplementation.h"
#include <sys/prctl.h>
#include "UtilsJsonRpc.h"

#define XCAST_IMPL_MAJOR_VERSION 1
#define XCAST_IMPL_MINOR_VERSION 0

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(XCastImplementation, XCAST_IMPL_MAJOR_VERSION, XCAST_IMPL_MINOR_VERSION);

    RtXcastConnector* XCastImplementation::_rtConnector = nullptr;

    XCastImplementation::XCastImplementation() : _adminLock()
    {
        if(nullptr == _rtConnector)
        {
            _rtConnector  = RtXcastConnector::getInstance();
            if(nullptr != _rtConnector)
            {
                _rtConnector->setService(this);
                if( _rtConnector->initialize())
                {
                    //We give few seconds delay before the timer is fired.
                    //m_locateCastTimer.start(LOCATE_CAST_FIRST_TIMEOUT_IN_MILLIS);
                }
            }
        }
    }

    XCastImplementation::~XCastImplementation()
    {
        if(_rtConnector) {
            delete _rtConnector;
            _rtConnector = nullptr;
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

    uint32_t XCastImplementation::Request(PluginHost::IStateControl::command command)
    {
        // Not implemented
        return Core::ERROR_GENERAL;
    }

    uint32_t XCastImplementation::Configure(PluginHost::IShell* service)
    {
        return Core::ERROR_NONE;
    }

    uint32_t XCastImplementation::applicationStateChanged(const string& appName, const string& appstate, const string& appId, const string& error) const
    {
        uint32_t status = Core::ERROR_GENERAL;
        if(!appName.empty() && !appstate.empty() && (nullptr != _rtConnector))
        {
            LOGINFO("XcastService::ApplicationStateChanged  ARGS = %s : %s : %s : %s ", appName.c_str(), appId.c_str() , appstate.c_str() , error.c_str());
            _rtConnector->applicationStateChanged(appName, appstate, appId, error);
            status = Core::ERROR_NONE;
        }
        return status;
    }

    uint32_t XCastImplementation::enableCastService(string friendlyname,bool enableService) const
    {
        LOGINFO("XcastService::enableCastService");
        if (nullptr != _rtConnector)
        {
            _rtConnector->enableCastService(friendlyname,enableService);
        }
        return Core::ERROR_NONE;
    }
    
    uint32_t XCastImplementation::getProtocolVersion(string &protocolVersion) const
    {
        LOGINFO("XcastService::getProtocolVersion");
        if (nullptr != _rtConnector)
        {
            protocolVersion = _rtConnector->getProtocolVersion();
        }
        return Core::ERROR_NONE;
    }

    uint32_t XCastImplementation::registerApplications(IApplicationInfoIterator* const appLists)
    {
        LOGINFO("XcastService::registerApplications");
        std::vector <DynamicAppConfig*> appConfigListTemp;
        uint32_t status = Core::ERROR_GENERAL;

        if ((nullptr != _rtConnector) && (appLists))
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
            _rtConnector->registerApplications(appConfigListTemp);
            status = Core::ERROR_NONE;
        }
        return status;
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
} // namespace Plugin
} // namespace WPEFramework
