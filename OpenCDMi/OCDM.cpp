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

#include "OCDM.h"
#include <interfaces/IDRM.h>

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 2

namespace WPEFramework {

namespace {

    static Plugin::Metadata<Plugin::OCDM> metadata(
        // Version (Major, Minor, Patch)
        API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
        // Preconditions
        {},
        // Terminations
        {},
        // Controls
        {}
    );
}

namespace OCDM {

    Exchange::IMemory* MemoryObserver(const RPC::IRemoteConnection* connection)
    {
        class MemoryObserverImpl : public Exchange::IMemory {
        public:
            MemoryObserverImpl() = delete;
            MemoryObserverImpl(const MemoryObserverImpl&) = delete;
            MemoryObserverImpl& operator=(const MemoryObserverImpl&) = delete;

            MemoryObserverImpl(const RPC::IRemoteConnection* connection)
                : _main(connection  == 0 ? Core::ProcessInfo().Id() : connection->RemoteId())
            {
            }
            ~MemoryObserverImpl() = default;

        public:
            uint64_t Resident() const override
            {
                return _main.Resident();
            }
            uint64_t Allocated() const override
            {
                return _main.Allocated();
            }
            uint64_t Shared() const override
            {
                return _main.Shared();
            }
            uint8_t Processes() const override
            {
                return (IsOperational() ? 1 : 0);
            }
#ifndef USE_THUNDER_R4
            virtual bool IsOperational() const
#else
            virtual const bool IsOperational() const
#endif /* USE_THUNDER_R4 */
            {
                return _main.IsActive();
            }

            BEGIN_INTERFACE_MAP(MemoryObserverImpl)
            INTERFACE_ENTRY(Exchange::IMemory)
            END_INTERFACE_MAP

        private:
            Core::ProcessInfo _main;
        };

        Exchange::IMemory* result = Core::Service<MemoryObserverImpl>::Create<Exchange::IMemory>(connection);

        return (result);
    }
}

namespace Plugin {

    SERVICE_REGISTRATION(OCDM, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    static Core::ProxyPoolType<Web::JSONBodyType<OCDM::Data>> jsonDataFactory(1);
    static Core::ProxyPoolType<Web::JSONBodyType<OCDM::Data::System>> jsonSystemFactory(1);

    /* virtual */ const string OCDM::Initialize(PluginHost::IShell* service)
    {
		#ifdef __WINDOWS__
        ForceLinkingOfOpenCDM();
		#endif

        string message;

        ASSERT(service != nullptr);
        ASSERT(_service == nullptr);
        ASSERT(_memory == nullptr);
        ASSERT(_opencdmi == nullptr);
        ASSERT(_connectionId == 0);

        _service = service;
        _service->AddRef();
        _skipURL = static_cast<uint8_t>(_service->WebPrefix().length());

        // Register the Process::Notification stuff. The Remote process might die before we get a
        // change to "register" the sink for these events !!! So do it ahead of instantiation.
        _service->Register(&_notification);

        _opencdmi = _service->Root<Exchange::IContentDecryption>(_connectionId, WPEFramework::RPC::CommunicationTimeOut, _T("OCDMImplementation"));

        if (_opencdmi == nullptr) {
            message = _T("OCDM could not be instantiated.");
        } else {
            RegisterAll();
            _opencdmi->Initialize(_service);

            ASSERT(_connectionId != 0);
            const RPC::IRemoteConnection *connection = _service->RemoteConnection(_connectionId);

            if (connection != nullptr) {

                _memory = WPEFramework::OCDM::MemoryObserver(connection);
                ASSERT(_memory != nullptr);
                _opencdmi->Register(&_notification);
                connection->Release();
            }
            else {
                message = _T("OCDM crashed at initialize!");
            }
        }

        if(message.length() != 0) {
            Deinitialize(service);
        }

        return message;
    }

    /*virtual*/ void OCDM::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        _service->Unregister(&_notification);

        if(_opencdmi != nullptr) {

            if(_memory != nullptr) {
                _memory->Release();
                _memory = nullptr;
            }

            _opencdmi->Unregister(&_notification);
            _opencdmi->Deinitialize(service);
            RPC::IRemoteConnection* connection(_service->RemoteConnection(_connectionId));

            UnregisterAll();

            VARIABLE_IS_NOT_USED uint32_t result = _opencdmi->Release();
            _opencdmi = nullptr;
            // It should have been the last reference we are releasing,
            // so it should end up in a DESCRUCTION_SUCCEEDED, if not we
            // are leaking...
            ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);

            // If this was running in a (container) proccess...
            if (connection != nullptr) {

                // Lets trigger the cleanup sequence for
                // out-of-process code. Which will guard
                // that unwilling processes, get shot if
                // not stopped friendly :~)
                connection->Terminate();
                connection->Release();
            }
        }

        PluginHost::ISubSystem* subSystem = service->SubSystems();

        if (subSystem != nullptr) {
            if(subSystem->IsActive(PluginHost::ISubSystem::DECRYPTION) == true) {
                subSystem->Set(PluginHost::ISubSystem::NOT_DECRYPTION, nullptr);
                subSystem->Release();
            }
        }

        _service->Release();
        _service = nullptr;
        _connectionId = 0;
    }

    /* virtual */ string OCDM::Information() const
    {
        // No additional info to report.
        return string();
    }

    /* virtual */ void OCDM::Inbound(Web::Request&)
    {
    }

    /* virtual */ Core::ProxyType<Web::Response> OCDM::Process(const Web::Request& request)
    {
        ASSERT(_skipURL <= request.Path.length());

        TRACE(Trace::Information, (string(_T("Received OCDM request"))));

        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());
        Core::TextSegmentIterator index(Core::TextFragment(request.Path, _skipURL, static_cast<uint32_t>(request.Path.length() - _skipURL)), false, '/');

        result->ErrorCode = Web::STATUS_BAD_REQUEST;
        result->Message = _T("Unsupported GET request.");

        // By default, we are in front of any element, jump onto the first element, which is if, there is something an empty slot.
        index.Next();

        if (request.Verb == Web::Request::HTTP_GET) {

            if (index.Next() == false) {
                Core::ProxyType<Web::JSONBodyType<Data>> data(jsonDataFactory.Element());

                RPC::IStringIterator* systems(_opencdmi->Systems());

                if (systems == nullptr) {
                    TRACE(Trace::Error, (_T("Could not load the Systems. %d"), __LINE__));
                } else {
                    string element;
                    while (systems->Next(element) == true) {
                        RPC::IStringIterator* index(_opencdmi->Designators(element));
                        if (index == nullptr) {
                            TRACE(Trace::Error, (_T("Could not load the Designators. %d"), __LINE__));
                        } else {
                            data->Systems.Add(Data::System(systems->Current(), index));
                            index->Release();
                        }
                    }
                    systems->Release();
                }
                result->ErrorCode = Web::STATUS_OK;
                result->Message = _T("Available Systems.");
                result->Body(data);
            } else {
                if (index.Current().Text() == _T("Sessions")) {

                    // Core::ProxyType<Web::JSONBodyType<WifiControl::NetworkList> > networks (jsonResponseFactoryNetworkList.Element());

                    // virtual RPC::IStringIterator* Sessions(const string& keySystem);
                    result->ErrorCode = Web::STATUS_OK;
                    result->Message = _T("Active Sessions.");
                    // result->Body(networks);

                } else if (index.Current().Text() == _T("Designators")) {
                    if (index.Next() == true) {

                        Core::ProxyType<Web::JSONBodyType<OCDM::Data::System>> data(jsonSystemFactory.Element());
                        RPC::IStringIterator* entries(_opencdmi->Designators(index.Current().Text()));

                        if (entries == nullptr) {
                            TRACE(Trace::Error, (_T("Could not load the Designators. %d"), __LINE__));
                        } else {
                            data->Name = index.Current().Text();
                            data->Load(entries);
                            entries->Release();
                        }

                        result->Body(data);
                        result->ErrorCode = Web::STATUS_OK;
                        result->Message = _T("Available Systems.");
                    }
                }
            }
        }

        return result;
    }

    void OCDM::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {

            ASSERT(_service != nullptr);

            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service,
                PluginHost::IShell::DEACTIVATED,
                PluginHost::IShell::FAILURE));
        }
    }
}
} //namespace WPEFramework::Plugin
