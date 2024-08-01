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
        , _service(nullptr)
        , _engine(Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create())
        , _communicatorClient(Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(_engine)))
        , _psController(nullptr)
        , _csController(nullptr)
        , _psObject(nullptr)
        , _psCache(nullptr)
        , _psInspector(nullptr)
        , _psLimit(nullptr)
        , _csObject(nullptr)
        , _storeNotification(*this)
        , _notification(*this)
        , _adminLock()
    {
        RegisterAll();
        LOGINFO("SecureStore constructor success\n");
    }
    SecureStore::~SecureStore()
    {
        UnregisterAll();
        LOGINFO("Disconnect from the COM-RPC socket\n");
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
        else
        {
            return nullptr;
        }
    }

    const string SecureStore::Initialize(PluginHost::IShell* service)
    {
        LOGINFO("SecureStore Initialize\n");
        string result;

        ASSERT(service != nullptr);
        ASSERT(nullptr == _service);

        _service = service;
        _service->AddRef();

        if (!_communicatorClient.IsValid())
        {
            LOGWARN("Invalid _communicatorClient\n");
        }
        else
        {
            #if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
                _engine->Announcements(_communicatorClient->Announcement());
            #endif
            _psController = _communicatorClient->Open<PluginHost::IShell>("org.rdk.PersistentStore", ~0, 3000);
            if (_psController)
            {
                // Get interface for IStore2
                _psObject = _psController->QueryInterface<Exchange::IStore2>();
                if(_psObject)
                {
                    LOGINFO("Connect success to _psObject\n");
                    _psObject->AddRef();
                    _psObject->Register(&_storeNotification);
                }
                else
                {
                    LOGERR("Connect fail to _psObject\n");
                }

                // Get interface for IStoreInspector
                _psInspector = _psController->QueryInterface<Exchange::IStoreInspector>();
                if(_psInspector)
                {
                    LOGINFO("Connect success to _psInspector\n");
                    _psInspector->AddRef();
                }
                else
                {
                    LOGERR("Connect fail to _psInspector\n");
                }

                // Get interface for IStoreLimit
                _psLimit = _psController->QueryInterface<Exchange::IStoreLimit>();
                if(_psLimit)
                {
                    LOGINFO("Connect success to _psLimit\n");
                    _psLimit->AddRef();
                }
                else
                {
                    LOGERR("Connect fail to _psLimit\n");
                }

                // Get interface for IStoreCache
                _psCache = _psController->QueryInterface<Exchange::IStoreCache>();
                if(_psCache)
                {
                    LOGINFO("Connect success to _psCache\n");
                    _psCache->AddRef();
                }
                else
                {
                    LOGERR("Connect fail to _psCache\n");
                }
            }

            _csController = _communicatorClient->Open<PluginHost::IShell>("org.rdk.CloudStore", ~0, 3000);
            if (_csController)
            {
                _csObject = _csController->QueryInterface<Exchange::IStore2>();
                if(_csObject)
                {
                    LOGINFO("Connect success to _csObject\n");
                    _csObject->AddRef();
                    _csObject->Register(&_storeNotification);
                }
                else
                {
                    LOGERR("Connect fail to _csObject\n");
                }
            }
        }

        //_service->Register(&_notification);

        LOGINFO("SecureStore Initialize complete\n");
        return result;
    }

    void SecureStore::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);
        SYSLOG(Logging::Shutdown, (string(_T("SecureStore::Deinitialize"))));

        // Disconnect from the COM-RPC socket
        if (_psController)
        {
            _psController->Release();
            _psController = nullptr;
        }
        if (_csController)
        {
            _csController->Release();
            _csController = nullptr;
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
            _psObject->Unregister(&_storeNotification);
            _psObject->Release();
        }
        if(_psInspector)
        {
            _psInspector->Release();
        }
        if(_psLimit)
        {
            _psLimit->Release();
        }
        if(_psCache)
        {
            _psCache->Release();
        }
        if(_csObject)
        {
            _csObject->Unregister(&_storeNotification);
            _csObject->Release();
        }

        //_service->Unregister(&_notification);

        _service->Release();
        _service = nullptr;
        SYSLOG(Logging::Shutdown, (string(_T("SecureStore de-initialised"))));
    }

    string SecureStore::Information() const
    {
        return (string());
    }
} // namespace Plugin
} // namespace WPEFramework
