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

#include "HelloWorldImplementation.h"
//#include <sys/prctl.h>
#include "UtilsJsonRpc.h"
//#include <mutex>
#include "tracing/Logging.h"

namespace WPEFramework {
namespace Plugin {

SERVICE_REGISTRATION(HelloWorldImplementation, 1, 0);

HelloWorldImplementation::HelloWorldImplementation()
: _adminLock(), mMessage("")
{
    LOGINFO("Create HelloWorldImplementation Instance");
}

HelloWorldImplementation::~HelloWorldImplementation()
{
    mMessage = "";
    LOGINFO("Delete HelloWorldImplementation Instance");
}

/**
 * Register a notification callback
 */
uint32_t HelloWorldImplementation::Register(Exchange::IHelloWorld::INotification *notification)
{
    ASSERT (nullptr != notification);

    _adminLock.Lock();

    // Make sure we can't register the same notification callback multiple times
    if (std::find(_helloWorldNotification.begin(), _helloWorldNotification.end(), notification) == _helloWorldNotification.end())
    {
        LOGINFO("Register notification");
        _helloWorldNotification.push_back(notification);
        notification->AddRef();
    }

    _adminLock.Unlock();

    return Core::ERROR_NONE;
}

/**
 * Unregister a notification callback
 */
uint32_t HelloWorldImplementation::Unregister(Exchange::IHelloWorld::INotification *notification )
{
    uint32_t status = Core::ERROR_GENERAL;

    ASSERT (nullptr != notification);

    _adminLock.Lock();

    // Make sure we can't unregister the same notification callback multiple times
    auto itr = std::find(_helloWorldNotification.begin(), _helloWorldNotification.end(), notification);
    if (itr != _helloWorldNotification.end())
    {
        (*itr)->Release();
        LOGINFO("Unregister notification");
        _helloWorldNotification.erase(itr);
        status = Core::ERROR_NONE;
    }
    else
    {
        LOGERR("notification not found");
    }

    _adminLock.Unlock();

    return status;
}

void HelloWorldImplementation::dispatchEvent(EventNames event, const JsonValue &params)
{
    Core::IWorkerPool::Instance().Submit(Job::Create(this, event, params));
}

void HelloWorldImplementation::Dispatch(EventNames event, const JsonValue params)
{
     _adminLock.Lock();

     std::list<Exchange::IHelloWorld::INotification*>::const_iterator index(_helloWorldNotification.begin());

     switch(event) {
         case ECHOMESSAGE:
             while (index != _helloWorldNotification.end())
             {
                 (*index)->onEcho(params.String());
                 ++index;
             }
         break;

         default:
             break;
     }

     _adminLock.Unlock();
}

Core::hresult HelloWorldImplementation::GetHelloWorldMessage(string& result)
{
    uint32_t status = Core::ERROR_NONE;
    result = mMessage;
    return status;
}

Core::hresult HelloWorldImplementation::SetHelloWorldMessage(const string& message)
{
    uint32_t status = Core::ERROR_NONE;
    mMessage = message;
    return status;
}

Core::hresult HelloWorldImplementation::LogHelloWorldMessage()
{
    uint32_t status = Core::ERROR_NONE;
    std::cout << "Hello world message printing " << mMessage << std::endl;
    return status;
}

Core::hresult HelloWorldImplementation::LogHelloWorldMessageComRpcOnly()
{
    uint32_t status = Core::ERROR_NONE;
    std::cout << "Hello world message printing via com-rpc " << mMessage << std::endl;
    return status;
}

Core::hresult HelloWorldImplementation::LogHelloWorldMessageJsonRpcOnly()
{
    uint32_t status = Core::ERROR_NONE;
    std::cout << "Hello world message printing via json-rpc " << mMessage << std::endl;
    return status;
}

Core::hresult HelloWorldImplementation::Echo(const string& message)
{
    uint32_t status = Core::ERROR_NONE;
    dispatchEvent(ECHOMESSAGE, JsonValue((string)message));
    return status;
}

} // namespace Plugin
} // namespace WPEFramework
