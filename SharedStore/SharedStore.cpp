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

#include "SharedStore.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {

namespace {

    static Plugin::Metadata<Plugin::SharedStore> metadata(
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

    SERVICE_REGISTRATION(SharedStore, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    SharedStore::SharedStore()
        : PluginHost::JSONRPC()
        , _service(nullptr)
        , _psEngine(Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create())
        , _psCommunicatorClient(Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(_psEngine)))
        , _csEngine(Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create())
        , _csCommunicatorClient(Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(_csEngine)))
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
        LOGINFO("SharedStore constructor success\n");
    }
    SharedStore::~SharedStore()
    {
        UnregisterAll();
        LOGINFO("Disconnect from the COM-RPC socket\n");
    }

    Exchange::IStore2* SharedStore::getRemoteStoreObject(ScopeType eScope)
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

    const string SharedStore::Initialize(PluginHost::IShell* service)
    {
        LOGINFO("SharedStore Initialize\n");
        string result;

        ASSERT(service != nullptr);
        ASSERT(nullptr == _service);

        _service = service;
        _service->AddRef();

        if (!_psCommunicatorClient.IsValid())
        {
            LOGWARN("Invalid _psCommunicatorClient\n");
        }
        else
        {
            #if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
                _psEngine->Announcements(_psCommunicatorClient->Announcement());
            #endif
            _psController = _psCommunicatorClient->Open<PluginHost::IShell>("org.rdk.PersistentStore", ~0, 3000);
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
        }

        // Establish communication with CloudStore
        if (!_csCommunicatorClient.IsValid())
        {
            LOGWARN("Invalid _csCommunicatorClient\n");
        }
        else
        {
            #if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
                _csEngine->Announcements(_csCommunicatorClient->Announcement());
            #endif
            _csController = _csCommunicatorClient->Open<PluginHost::IShell>("org.rdk.CloudStore", ~0, 3000);
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

        _service->Register(&_notification);

        LOGINFO("SharedStore Initialize complete\n");
        return result;
    }

    void SharedStore::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);
        SYSLOG(Logging::Shutdown, (string(_T("SharedStore::Deinitialize"))));

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
        _psCommunicatorClient->Close(RPC::CommunicationTimeOut);
        if (_psCommunicatorClient.IsValid())
        {
            _psCommunicatorClient.Release();
        }
        if(_psEngine.IsValid())
        {
            _psEngine.Release();
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
        _csCommunicatorClient->Close(RPC::CommunicationTimeOut);
        if (_csCommunicatorClient.IsValid())
        {
            _csCommunicatorClient.Release();
        }
        if(_csEngine.IsValid())
        {
            _csEngine.Release();
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

        _service->Unregister(&_notification);

        _service->Release();
        _service = nullptr;
        SYSLOG(Logging::Shutdown, (string(_T("SharedStore de-initialised"))));
    }

    string SharedStore::Information() const
    {
        return (string());
    }
} // namespace Plugin
} // namespace WPEFramework
