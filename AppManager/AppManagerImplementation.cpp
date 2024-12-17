/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2024 RDK Management
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

#include "AppManagerImplementation.h"
#include "UtilsJsonRpc.h"
#include "tracing/Logging.h"

#define FIREBOLT_ENDPOINT "ws://127.0.0.1:3474/jsonrpc"

namespace WPEFramework {
namespace Plugin {

using namespace AppManagerV1;

SERVICE_REGISTRATION(AppManagerImplementation, 1, 0);

AppManagerImplementation::AppManagerImplementation()
: _adminLock(), mAppManagerNotification(), mRDKShellAdapter(nullptr), mWSEndPoint(nullptr), mRippleConnectionId(-1), mFireboltEndpoint(FIREBOLT_ENDPOINT)
{
    LOGINFO("Create AppManagerImplementation Instance");
}

AppManagerImplementation::~AppManagerImplementation()
{
    LOGINFO("Delete AppManagerImplementation Instance");
}

Core::hresult AppManagerImplementation::Initialize(PluginHost::IShell* service)
{
    mRDKShellAdapter = new RDKShellAdapter();
    bool result = mRDKShellAdapter->Initialize(this, service);
    if (false == result)
    {
        return Core::ERROR_GENERAL;
    }	    
    mWSEndPoint = new WebSocketEndPoint();
    mWSEndPoint->initialize();
    return Core::ERROR_NONE;
}

void AppManagerImplementation::Deinitialize(PluginHost::IShell* service)
{

    mRDKShellAdapter->Deinitialize();
    delete mRDKShellAdapter;
    mRDKShellAdapter = nullptr;

    if (nullptr != mWSEndPoint)
    {
        mWSEndPoint->deinitialize();
        delete mWSEndPoint;
    }
    mWSEndPoint = nullptr;
    mRippleConnectionId = -1;
}

/**
 * Register a notification callback
 */
uint32_t AppManagerImplementation::Register(Exchange::IAppManager::INotification *notification)
{
    ASSERT (nullptr != notification);

    _adminLock.Lock();

    if (std::find(mAppManagerNotification.begin(), mAppManagerNotification.end(), notification) == mAppManagerNotification.end())
    {
        LOGINFO("Register notification");
        mAppManagerNotification.push_back(notification);
        notification->AddRef();
    }

    _adminLock.Unlock();

    return Core::ERROR_NONE;
}

/**
 * Unregister a notification callback
 */
uint32_t AppManagerImplementation::Unregister(Exchange::IAppManager::INotification *notification )
{
    uint32_t status = Core::ERROR_GENERAL;

    ASSERT (nullptr != notification);

    _adminLock.Lock();

    auto itr = std::find(mAppManagerNotification.begin(), mAppManagerNotification.end(), notification);
    if (itr != mAppManagerNotification.end())
    {
        (*itr)->Release();
        LOGINFO("Unregister notification");
        mAppManagerNotification.erase(itr);
        status = Core::ERROR_NONE;
    }
    else
    {
        LOGERR("notification not found");
    }

    _adminLock.Unlock();

    return status;
}

void AppManagerImplementation::dispatchEvent(EventNames event, const JsonObject &params)
{
    Core::IWorkerPool::Instance().Submit(Job::Create(this, event, params));
}

void AppManagerImplementation::Dispatch(EventNames event, const JsonObject params)
{
     _adminLock.Lock();

     std::list<Exchange::IAppManager::INotification*>::const_iterator index(mAppManagerNotification.begin());

     switch(event) {
         case APPSTATECHANGED:
             while (index != mAppManagerNotification.end())
             {
		 LifecycleState newState = (LifecycleState) params["newState"].Number();
		 LifecycleState oldState = (LifecycleState) params["oldState"].Number();
                 (*index)->onAppStateChanged(params["client"].String(), 0, newState, oldState);
                 ++index;
             }
         break;

         default:
             break;
     }

     _adminLock.Unlock();
}

uint32_t AppManagerImplementation::LaunchApp(const string& appId /* @in */, const string& intent /* @in */, const string& launchArgs /* @in */, bool& success /* @out */)
{
    uint32_t status = Core::ERROR_NONE;
    JsonObject parameters, response;
    parameters.FromString(launchArgs);
    parameters["uri"] = intent;
    parameters["callsign"] = appId;
    status = mRDKShellAdapter->launch(parameters, response);
    success = response["success"].Boolean();
    return status;
}

uint32_t AppManagerImplementation::CloseApp(const string& appId /* @in */, bool& success /* @out */)
{
    uint32_t status = Core::ERROR_NONE;
    JsonObject parameters, response;
    parameters["callsign"] = appId;
    status = mRDKShellAdapter->suspend(parameters, response);
    success = response["success"].Boolean();
    return status;
}

uint32_t AppManagerImplementation::TerminateApp(const string& appId /* @in */, bool& success /* @out */)
{
    uint32_t status = Core::ERROR_NONE;
    JsonObject parameters, response;
    parameters["callsign"] = appId;
    status = mRDKShellAdapter->destroy(parameters, response);
    success = response["success"].Boolean();
    return status;
}

uint32_t AppManagerImplementation::GetLoadedApps(string& appData /* @out */, bool& success/* @out */)
{
    uint32_t status = Core::ERROR_NONE;
    JsonObject parameters, response;
    status = mRDKShellAdapter->getClients(parameters, response);
    response.ToString(appData);
    success = response["success"].Boolean();
    return status;
}

uint32_t AppManagerImplementation::SendIntent(const string& appId /* @in */, const string& intent /* @in */, bool& success /* @out */)
{
    uint32_t status = Core::ERROR_NONE;
    if (mRippleConnectionId == -1)
    {
        mRippleConnectionId = mWSEndPoint->connect(mFireboltEndpoint, true);
        if (mRippleConnectionId == -1)
        {
            success = false;
            return Core::ERROR_GENERAL;
        }
    }
    //MESSAGE FORMAT
    //string sessionRequestMessage = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"LifecycleManagement.session\",\"params\":{\"session\":{\"app\":{\"id\":\"app1\",\"url\":\"https://www.google.com\"},\"runtime\":{\"id\":\"WebKitBrowser-1\"},\"launch\":{\"intent\":{\"action\":\"launch\",\"context\":{\"source\":\"user\"}}}}}}";

    string sessionRequestMessagePre = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"LifecycleManagement.session\",\"params\":{\"session\":{\"app\":{\"id\":\"";
    string sessionRequestMessagePost = "\",\"url\":\"\"},\"runtime\":{\"id\":\"WebKitBrowser-1\"},\"launch\":{\"intent\":";
    std::stringstream ss;
    ss << sessionRequestMessagePre << appId << sessionRequestMessagePost << intent << "}}}}";
    std::string response("");
    bool ret = mWSEndPoint->send(mRippleConnectionId, ss.str(), response);
    std::cout << "Response from ripple is " << ret << ":" << response << std::endl;
    success = ret;
    status = ret?Core::ERROR_NONE:Core::ERROR_GENERAL;
    return status;
}

uint32_t AppManagerImplementation::PreloadApp(const string& appId /* @in */, const string& launchArgs /* @in */, bool& success /* @out */)
{
    uint32_t status = Core::ERROR_NONE;
    JsonObject parameters, response;
    parameters.FromString(launchArgs);
    parameters["callsign"] = appId;
    parameters["suspend"] = true;
    status = mRDKShellAdapter->launch(parameters, response);
    success = response["success"].Boolean();
    return status;
}

void AppManagerImplementation::onAppStateChanged(std::string client, Exchange::IAppManager::LifecycleState newState, Exchange::IAppManager::LifecycleState oldState)
{
    JsonObject eventDetails;
    eventDetails["client"] = client;
    eventDetails["oldState"] = (int) oldState;
    eventDetails["newState"] = (int) newState;
    dispatchEvent(APPSTATECHANGED, eventDetails);
}
} // namespace Plugin
} // namespace WPEFramework
