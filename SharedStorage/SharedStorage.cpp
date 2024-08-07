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

#include "SharedStorage.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {

namespace {

    static Plugin::Metadata<Plugin::SharedStorage> metadata(
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

    SERVICE_REGISTRATION(SharedStorage, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    SharedStorage::SharedStorage()
        : PluginHost::JSONRPC()
        , _service(nullptr)
        , _psObject(nullptr)
        , _psCache(nullptr)
        , _psInspector(nullptr)
        , _psLimit(nullptr)
        , _csObject(nullptr)
        , _storeNotification(*this)
        , m_PersistentStoreRef(nullptr)
        , m_CloudStoreRef(nullptr)
    {
        RegisterAll();
    }
    SharedStorage::~SharedStorage()
    {
        UnregisterAll();
    }

    Exchange::IStore2* SharedStorage::getRemoteStoreObject(ScopeType eScope)
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
            TRACE(Trace::Error, (_T("%s: Unknown scope: %d"), __FUNCTION__, static_cast<int>(eScope)));
            return nullptr;
        }
    }

    const string SharedStorage::Initialize(PluginHost::IShell* service)
    {
        SYSLOG(Logging::Startup, (_T("SharedStorage::Initialize: PID=%u"), getpid()));
        string message;

        ASSERT(service != nullptr);
        ASSERT(nullptr == _service);

        _service = service;
        _service->AddRef();

        m_PersistentStoreRef = service->QueryInterfaceByCallsign<PluginHost::IPlugin>("org.rdk.PersistentStore");
        if(nullptr != m_PersistentStoreRef)
        {
            // Get interface for IStore2
            _psObject = m_PersistentStoreRef->QueryInterface<Exchange::IStore2>();
            // Get interface for IStoreInspector
            _psInspector = m_PersistentStoreRef->QueryInterface<Exchange::IStoreInspector>();
            // Get interface for IStoreLimit
            _psLimit = m_PersistentStoreRef->QueryInterface<Exchange::IStoreLimit>();
            // Get interface for IStoreCache
            _psCache = m_PersistentStoreRef->QueryInterface<Exchange::IStoreCache>();
            if ( (nullptr == _psObject) || (nullptr == _psInspector) || (nullptr == _psLimit) || (nullptr == _psCache) )
            {
                message = _T("SharedStorage plugin could not be initialized.");
                TRACE(Trace::Error, (_T("%s: Can't get PersistentStore interface"), __FUNCTION__));
                m_PersistentStoreRef->Release();
                m_PersistentStoreRef = nullptr;
            }
            else
            {
                _psObject->Register(&_storeNotification);
            }
        }
        else
        {
            message = _T("SharedStorage plugin could not be initialized.");
            TRACE(Trace::Error, (_T("%s: Can't get PersistentStore interface"), __FUNCTION__));
        }

        // Establish communication with CloudStore
        m_CloudStoreRef = service->QueryInterfaceByCallsign<PluginHost::IPlugin>("org.rdk.CloudStore");
        if(nullptr != m_CloudStoreRef)
        {
            // Get interface for IStore2
            _csObject = m_CloudStoreRef->QueryInterface<Exchange::IStore2>();
            if (nullptr == _csObject)
            {
                message = _T("SharedStorage plugin could not be initialized.");
                TRACE(Trace::Error, (_T("%s: Can't get CloudStore interface"), __FUNCTION__));
                m_CloudStoreRef->Release();
                m_CloudStoreRef = nullptr;
            }
            else
            {
                _csObject->Register(&_storeNotification);
            }
        }
        else
        {
            message = _T("SharedStorage plugin could not be initialized.");
            TRACE(Trace::Error, (_T("%s: Can't get CloudStore interface"), __FUNCTION__));
        }

        if (message.length() != 0) {
            Deinitialize(service);
        }
        SYSLOG(Logging::Startup, (string(_T("SharedStorage Initialize complete"))));
        return message;
    }

    void SharedStorage::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);
        SYSLOG(Logging::Shutdown, (string(_T("SharedStorage::Deinitialize"))));

        if (nullptr != m_PersistentStoreRef)
        {
            m_PersistentStoreRef->Release();
            m_PersistentStoreRef = nullptr;
        }
        // Disconnect from the interface
        if(_psObject)
        {
            _psObject->Unregister(&_storeNotification);
            _psObject->Release();
            _psObject = nullptr;
        }
        if(_psInspector)
        {
            _psInspector->Release();
            _psInspector = nullptr;
        }
        if(_psLimit)
        {
            _psLimit->Release();
            _psLimit = nullptr;
        }
        // Disconnect from the COM-RPC socket
        if (nullptr != m_CloudStoreRef)
        {
            m_CloudStoreRef->Release();
            m_CloudStoreRef = nullptr;
        }
        if(_psCache)
        {
            _psCache->Release();
            _psCache = nullptr;
        }
        if(_csObject)
        {
            _csObject->Unregister(&_storeNotification);
            _csObject->Release();
            _csObject = nullptr;
        }

        _service->Release();
        _service = nullptr;
        SYSLOG(Logging::Shutdown, (string(_T("SharedStorage Deinitialize complete"))));
    }

    string SharedStorage::Information() const
    {
        return (string());
    }
} // namespace Plugin
} // namespace WPEFramework
