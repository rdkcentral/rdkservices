/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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

#include "SecureStore.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {

namespace {

    static Plugin::Metadata<Plugin::SecureStore> metadata(
        // Version (Major, Minor, Patch)
        API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
        // Preconditions
        {},
        // Terminations
        {},
        // Controls
        {});
}

namespace Plugin {
    using namespace JsonData::PersistentStore;

    SERVICE_REGISTRATION(SecureStore, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    SecureStore::SecureStore()
        : PluginHost::JSONRPC()
        , _engine(Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create())
        , _communicatorClient(Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(_engine)))
        , _controller(nullptr)
        , _psObject(nullptr)
        , _psCache(nullptr)
        , _psInspector(nullptr)
        , _psLimit(nullptr)
        , _csObject(nullptr)
        , _storeNotification(*this)
        , _notification(*this)
        , _psRegisteredEventHandlers(false)
        , _csRegisteredEventHandlers(false)
        , _adminLock()
    {
        // _engine = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
        // _communicatorClient = Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(_engine));

        RegisterAll();
        LOGINFO("SecureStore constructor success\n");
    }
    SecureStore::~SecureStore()
    {
        UnregisterAll();
        LOGINFO("Disconnect from the COM-RPC socket\n");
        // Disconnect from the COM-RPC socket
        if (_controller)
        {
            _controller->Release();
            _controller = nullptr;
        }
        _communicatorClient->Close(RPC::CommunicationTimeOut);
        if (_communicatorClient.IsValid())
        {
            _communicatorClient.Release();
        }
        if(_engine.IsValid())
        {
            _engine.Release();
        }
        if(_psObject)
        {
            _psObject->Release();
        }
        if(_csObject)
        {
            _csObject->Release();
        }
        _psRegisteredEventHandlers = false;
        _csRegisteredEventHandlers = false;
    }

    Exchange::IStore2* SecureStore::getRemoteStoreObject(ScopeType eScope)
    {
        if( (eScope == ScopeType::DEVICE) && _psObject)
        {
            return _psObject;
        }
        else if( (eScope == ScopeType::ACCOUNT) && _csObject)
        {
            return _csObject;
        }
        else if( (eScope != ScopeType::DEVICE) && (eScope != ScopeType::ACCOUNT) )
        {
            return nullptr;
        }

        LOGINFO("Connect the store COM-RPC socket\n");
        const std::lock_guard<std::mutex> lock(storeObjMutex);
        Exchange::IStore2** _storeObject = nullptr;
        std::string storeCallSign;
        if(eScope == ScopeType::DEVICE)
        {
            storeCallSign = "org.rdk.PersistentStore";
            _storeObject = &_psObject;
        }
        else if(eScope == ScopeType::ACCOUNT)
        {
            storeCallSign = "org.rdk.CloudStore";
            _storeObject = &_csObject;
        }

        if(!*_storeObject)
        {
            _controller = _communicatorClient->Open<PluginHost::IShell>(storeCallSign, ~0, 5000);
            if (_controller)
            {
                // if(eScope == ScopeType::DEVICE)
                // {
                //     _psObject = _controller->QueryInterface<Exchange::IStore2>();
                //     if(_psObject)
                //     {
                //         LOGINFO("Connect success to _psObject\n");
                //         _psObject->AddRef();
                //         registerEventHandlers();
                //     }
                //     else
                //     {
                //         LOGINFO("Connect fail to _psObject\n");
                //     }
                // }
                // else if(eScope == ScopeType::ACCOUNT)
                // {
                //     _csObject = _controller->QueryInterface<Exchange::IStore2>();
                //     if(_csObject)
                //     {
                //         LOGINFO("Connect success to _csObject\n");
                //         _csObject->AddRef();
                //     }
                //     else
                //     {
                //         LOGINFO("Connect fail to _csObject\n");
                //     }
                // }

                *_storeObject = _controller->QueryInterface<Exchange::IStore2>();
                if(*_storeObject)
                {
                    LOGINFO("Connect success to _storeObject\n");
                    (*_storeObject)->AddRef();
                    registerEventHandlers(eScope);
                }
                else
                {
                    LOGERR("Connect fail to _storeObject\n");
                }
            }
            else
            {
                LOGERR("Failed to create Store Controller\n");
            }
        }
        LOGINFO("_storeObject: %p, _psObject: %p, _csObject: %p", (void *)_storeObject, (void*)_psObject, (void*)_csObject);
        return *_storeObject;
    }

    Exchange::IStoreInspector* SecureStore::getRemoteStoreInspectorObject(ScopeType eScope)
    {
        if( (eScope == ScopeType::DEVICE))
        {
            if(_psInspector)
                return _psInspector;
        }
        else
        {
            return nullptr;
        }

        LOGINFO("Connect the store inspector COM-RPC socket\n");
        const std::lock_guard<std::mutex> lock(storeObjMutex);

        if(!_psInspector)
        {
            _controller = _communicatorClient->Open<PluginHost::IShell>(_T("org.rdk.PersistentStore"), ~0, 5000);
            if (_controller)
            {
                _psInspector = _controller->QueryInterface<Exchange::IStoreInspector>();
                if(_psInspector)
                {
                    LOGINFO("Connect success to _psInspector\n");
                    _psInspector->AddRef();
                }
                else
                {
                    LOGERR("Connect fail to _psInspector\n");
                }
            }
            else
            {
                LOGERR("Failed to create Store Controller\n");
            }
        }
        return _psInspector;
    }

    Exchange::IStoreLimit* SecureStore::getRemoteStoreLimitObject(ScopeType eScope)
    {
        if(eScope == ScopeType::DEVICE)
        {
            if(_psLimit)
                return _psLimit;
        }
        else
        {
            return nullptr;
        }

        LOGINFO("Connect the store limit COM-RPC socket\n");
        const std::lock_guard<std::mutex> lock(storeObjMutex);

        if(!_psLimit)
        {
            _controller = _communicatorClient->Open<PluginHost::IShell>(_T("org.rdk.PersistentStore"), ~0, 5000);
            if (_controller)
            {
                _psLimit = _controller->QueryInterface<Exchange::IStoreLimit>();
                if(_psLimit)
                {
                    LOGINFO("Connect success to _psLimit\n");
                    _psLimit->AddRef();
                }
                else
                {
                    LOGERR("Connect fail to _psLimit\n");
                }
            }
            else
            {
                LOGERR("Failed to create Store Controller\n");
            }
        }
        return _psLimit;
    }

    Exchange::IStoreCache* SecureStore::getRemoteStoreCacheObject()
    {
        if(_psCache)
        {
            return _psCache;
        }

        LOGINFO("Connect the store cache COM-RPC socket\n");
        const std::lock_guard<std::mutex> lock(storeObjMutex);
        _controller = _communicatorClient->Open<PluginHost::IShell>("org.rdk.PersistentStore", ~0, 5000);
        if (_controller)
        {
            _psCache = _controller->QueryInterface<Exchange::IStoreCache>();
            if(_psCache)
            {
                LOGINFO("Connect success to _psCache\n");
                (_psCache)->AddRef();
            }
            else
            {
                LOGERR("Connect fail to _psCache\n");
            }
        }
        else
        {
            LOGERR("Failed to create Store Controller\n");
        }
        return _psCache;
    }

    const string SecureStore::Initialize(PluginHost::IShell* service)
    {
        LOGINFO("SecureStore Initialize\n");
        string result;

        ASSERT(service != nullptr);

        if (!_communicatorClient.IsValid())
        {
            LOGWARN("Invalid _communicatorClient\n");
        }
        else
        {
            #if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
                _engine->Announcements(_communicatorClient->Announcement());
            #endif
            // LOGINFO("Connect the persistent store COM-RPC socket\n");
            // _psObject = getRemoteStoreObject(ScopeType::DEVICE);
            // if(!_psObject)
            // {
            //     LOGERR("Failed to create PersistentStore Controller\n");
            // }

            // LOGINFO("Connect the cloud store COM-RPC socket\n");
            // _csObject = getRemoteStoreObject(ScopeType::ACCOUNT);
            // if(!_csObject)
            // {
            //     LOGERR("Failed to create CloudStore Controller\n");
            // }
        }

        //service->Register(&_notification);

        LOGINFO("SecureStore Initialize complete\n");
        return result;
    }

    void SecureStore::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(service == service);
        service->Unregister(&_notification);
    }

    string SecureStore::Information() const
    {
        return (string());
    }

    void SecureStore::registerEventHandlers(ScopeType eScope)
    {
        if(eScope == ScopeType::DEVICE)
        {
            ASSERT (nullptr != _psObject);

            if(!_psRegisteredEventHandlers && _psObject) {
                _psRegisteredEventHandlers = true;
                _psObject->Register(&_storeNotification);
                LOGINFO("_psObject registerEventHandlers complete");
            }
        }
        else if(eScope == ScopeType::ACCOUNT)
        {
            ASSERT (nullptr != _csObject);

            if(!_csRegisteredEventHandlers && _csObject) {
                _csRegisteredEventHandlers = true;
                _csObject->Register(&_storeNotification);
                LOGINFO("_csObject registerEventHandlers complete");
            }
        }
    }

} // namespace Plugin
} // namespace WPEFramework
