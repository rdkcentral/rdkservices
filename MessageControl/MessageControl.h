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

#pragma once

#include "Module.h"
#include "MessageOutput.h"
#include <functional>

namespace WPEFramework {

namespace Plugin {

    class MessageControl : public PluginHost::JSONRPC, public PluginHost::IPluginExtended, public PluginHost::IWebSocket, public Exchange::IMessageControl {
    private:
        using Cleanups  = std::vector<uint32_t>;

        class WorkerThread : public Core::Thread {
        public:
            WorkerThread() = delete;
            WorkerThread(const WorkerThread&) = delete;
            WorkerThread& operator= (const WorkerThread&) = delete;

            WorkerThread(MessageControl& parent)
                : Core::Thread()
                , _parent(parent)
            {
            }
            ~WorkerThread() override = default;

        private:
            uint32_t Worker() override
            {
                _parent.Dispatch();

                return (Core::infinite);
            }

        private:
            MessageControl& _parent;
        };

    private:
        struct ICollect {

            struct ICallback {

                virtual void Message(const Core::Messaging::MessageInfo& metadata, const string& text) = 0;

                virtual ~ICallback() = default;
            };

            virtual uint32_t Callback(ICallback* callback) = 0;

            virtual ~ICollect() = default;
        };

    private:
        using OutputList = std::vector<Publishers::IPublish*>;

        class Config : public Core::JSON::Container {
        private:
            class NetworkNode : public Core::JSON::Container {
            public:
                NetworkNode();
                NetworkNode(const NetworkNode& copy);
                ~NetworkNode() = default;

            public:
                Core::NodeId NodeId() const {
                    return (Core::NodeId(Binding.Value().c_str(), Port.Value()));
                }

            public:
                Core::JSON::DecUInt16 Port;
                Core::JSON::String Binding;
            };

        public:
            Config()
                : Core::JSON::Container()
                , Console(false)
                , SysLog(false)
                , FileName()
                , Abbreviated(true)
                , MaxExportConnections(Publishers::WebSocketOutput::DefaultMaxConnections)
                , Remote()
            {
                Add(_T("console"), &Console);
                Add(_T("syslog"), &SysLog);
                Add(_T("filepath"), &FileName);
                Add(_T("abbreviated"), &Abbreviated);
                Add(_T("maxexportconnections"), &MaxExportConnections);
                Add(_T("remote"), &Remote);
            }
            ~Config() = default;

            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

            Core::JSON::Boolean Console;
            Core::JSON::Boolean SysLog;
            Core::JSON::String FileName;
            Core::JSON::Boolean Abbreviated;
            Core::JSON::DecUInt16 MaxExportConnections;
            NetworkNode Remote;
        };

        class Observer
            : public RPC::IRemoteConnection::INotification
            , public Plugin::MessageControl::ICollect::ICallback {
        private:
            enum state {
                ATTACHING,
                DETACHING,
                OBSERVING
            };
            using Observers = std::unordered_map<uint32_t, state>;

        public:
            Observer() = delete;
            Observer(const Observer&) = delete;
            Observer& operator= (const Observer&) = delete;

            explicit Observer(MessageControl& parent)
                : _parent(parent)
                , _adminLock()
                , _observing()
                , _job(*this)
            {
            }
            ~Observer() override {
                _job.Revoke();
            }

        public:
            //
            // Exchange::IMessageControl::INotification
            // ----------------------------------------------------------
            void Message(const Core::Messaging::MessageInfo& metadata, const string& message) override {
                _parent.Message(metadata, message);
            }

            //
            // RPC::IRemoteConnection::INotification
            // ----------------------------------------------------------
            void Activated(RPC::IRemoteConnection* connection) override
            {
                uint32_t id = connection->Id();

                _adminLock.Lock();

                // Seems the ID is already in here, thats odd, and impossible :-)
                Observers::iterator index = _observing.find(id);

                if (index == _observing.end()) {
                    _observing.emplace(std::piecewise_construct,
                        std::make_tuple(id),
                        std::make_tuple(state::ATTACHING));
                }
                else if (index->second == state::DETACHING) {
                    index->second = state::OBSERVING;
                }

                _adminLock.Unlock();

                _job.Submit();
            }

            void Deactivated(RPC::IRemoteConnection* connection) override
            {
                ASSERT(connection != nullptr);

                RPC::IMonitorableProcess* controlled (connection->QueryInterface<RPC::IMonitorableProcess>());

                if (controlled != nullptr) {
                    // This is a connection that is controleld by WPEFramework. For this we can wait till we
                    // receive a terminated.
                    controlled->Release();
                }
                else {
                    // We have no clue where this is coming from, just assume that if there are message files
                    // with this ID that it can be closed.
                    Drop(connection->Id());
               }
            }

            void Terminated(RPC::IRemoteConnection* connection) override
            {
                ASSERT(connection != nullptr);
                Drop(connection->Id());
            }

            BEGIN_INTERFACE_MAP(Observer)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

        private:
            friend class Core::ThreadPool::JobType<Observer&>;

            void Drop (const uint32_t id)
            {
                _adminLock.Lock();

                // Seems the ID is already in here, thats odd, and impossible :-)
                Observers::iterator index = _observing.find(id);

                ASSERT(index != _observing.end());

                if (index != _observing.end()) {
                    if (index->second == state::ATTACHING) {
                        _observing.erase(index);
                    }
                    else if (index->second == state::OBSERVING) {
                        index->second = state::DETACHING;
                    }
                }

                _adminLock.Unlock();

                _job.Submit();
            }

            void Dispatch()
            {
                _adminLock.Lock();

                bool done = false;

                while (done == false) {
                    Observers::iterator index = _observing.begin();

                    while (done == false) {
                        if (index->second == state::ATTACHING) {
                            const uint32_t id = index->first;
                            index->second = state::OBSERVING;
                            _adminLock.Unlock();
                            _parent.Attach(id);
                            _adminLock.Lock();
                            break;
                        }
                        else if (index->second == state::DETACHING) {
                            const uint32_t id = index->first;
                            _observing.erase(index);
                            _adminLock.Unlock();
                            _parent.Detach(id);
                            _adminLock.Lock();
                            break;
                        }
                        else {
                            index++;

                            if (index == _observing.end()) {
                                done = true;
                            }
                        }
                    }
                }

                _adminLock.Unlock();
            }

        private:
            MessageControl& _parent;
            Core::CriticalSection _adminLock;
            Observers _observing;
            Core::WorkerPool::JobType<Observer&> _job;
        };

    public:
        MessageControl(const MessageControl&) = delete;
        MessageControl& operator=(const MessageControl&) = delete;

        MessageControl();

        ~MessageControl() override
        {
            _worker.Stop();
            _worker.Wait(Core::Thread::STOPPED, Core::infinite);
            _client.ClearInstances();
        }

        BEGIN_INTERFACE_MAP(MessageControl)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_ENTRY(PluginHost::IPluginExtended)
            INTERFACE_ENTRY(PluginHost::IWebSocket)
            INTERFACE_ENTRY(Exchange::IMessageControl)
        END_INTERFACE_MAP

    public:
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

        bool Attach(PluginHost::Channel&) override;
        void Detach(PluginHost::Channel&) override;
        Core::ProxyType<Core::JSON::IElement> Inbound(const string& identifier) override;
        Core::ProxyType<Core::JSON::IElement> Inbound(const uint32_t ID, const Core::ProxyType<Core::JSON::IElement>& element) override;

    private:
        void Announce(Publishers::IPublish* output)
        {
            _outputLock.Lock();

            ASSERT(std::find(_outputDirector.begin(), _outputDirector.end(), output) == _outputDirector.end());

            _outputDirector.emplace_back(output);

            _outputLock.Unlock();
        }

        void Message(const Core::Messaging::MessageInfo& metadata, const string& message)
        {
            // Time to start sending it to all interested parties...
            _outputLock.Lock();

            for (auto& entry : _outputDirector) {
                entry->Message(metadata, message);
            }

            _webSocketExporter.Message(metadata, message);

            _outputLock.Unlock();
        }

    public:
        uint32_t Callback(Plugin::MessageControl::ICollect::ICallback* callback)
        {
            _adminLock.Lock();

            ASSERT((_callback != nullptr) ^ (callback != nullptr));

            if (_callback != nullptr) {
                _worker.Block();
                _client.SkipWaiting();
                _adminLock.Unlock();
                _worker.Wait(Core::Thread::BLOCKED, Core::infinite);
                _adminLock.Lock();
            }

            _callback = callback;

            if (_callback != nullptr) {
                _worker.Run();
            }

            _adminLock.Unlock();

            return (Core::ERROR_NONE);
        }

        void Attach(const uint32_t id)
        {
            _adminLock.Lock();

            _client.AddInstance(id);
            Cleanup();

            _adminLock.Unlock();
        }

        void Detach(const uint32_t id)
        {
            _adminLock.Lock();

            _client.RemoveInstance(id);
            _cleaning.emplace_back(id);
            Cleanup();

            _adminLock.Unlock();
        }

    public:
        void Cleanup()
        {
            // Also have a list through the cleanup we should do, just to make sure
            Cleanups::iterator index = _cleaning.begin();

            while (index != _cleaning.end()) {
                bool destructed = true;
                string filter (_dispatcherIdentifier + '.' + Core::NumberType<uint32_t>(*index).Text() + _T(".*"));
                Core::Directory directory (_dispatcherBasePath.c_str(), filter.c_str());

                while ((destructed == true) && (directory.Next() == true)) {
                    Core::File file (directory.Current());
                    destructed = destructed && (Core::File(directory.Current()).Destroy());
                }

                if (destructed == true) {
                    index = _cleaning.erase(index);
                }
                else {
                    index++;
                }
            }
        }

        uint32_t Enable(const messagetype type, const string& category, const string& module, const bool enabled) override
        {
            _client.Enable({static_cast<Core::Messaging::Metadata::type>(type), category, module}, enabled);

            return (Core::ERROR_NONE);
        }

        uint32_t Controls(Exchange::IMessageControl::IControlIterator*& controls) const override
        {
            std::list<Exchange::IMessageControl::Control> list;
            Messaging::MessageUnit::Iterator index;
            _client.Controls(index);

            while (index.Next() == true) {
                list.push_back( { static_cast<messagetype>(index.Type()), index.Category(), index.Module(), index.Enabled() } );
            }

            using Implementation = RPC::IteratorType<Exchange::IMessageControl::IControlIterator>;
            controls = Core::Service<Implementation>::Create<Exchange::IMessageControl::IControlIterator>(list);

            return (Core::ERROR_NONE);
        }

    private:
        void Dispatch()
        {
            _client.WaitForUpdates(Core::infinite);

            _client.PopMessagesAndCall([this](const Core::ProxyType<Core::Messaging::MessageInfo>& metadata, const Core::ProxyType<Core::Messaging::IEvent>& message) {
                // Turn data into piecies to trasfer over the wire
                Message(*metadata, message->Data());
            });
        }

    private:
        Core::CriticalSection _adminLock;
        Core::CriticalSection _outputLock;
        Config _config;
        OutputList _outputDirector;
        Publishers::WebSocketOutput _webSocketExporter;
        MessageControl::ICollect::ICallback* _callback;
        Core::Sink<Observer> _observer;
        PluginHost::IShell* _service;
        const string _dispatcherIdentifier;
        const string _dispatcherBasePath;
        Messaging::MessageClient _client;
        WorkerThread _worker;
        Messaging::TraceFactoryType<Core::Messaging::IStore::Tracing, Messaging::TextMessage> _tracingFactory;
        Messaging::TraceFactoryType<Core::Messaging::IStore::Logging, Messaging::TextMessage> _loggingFactory;
        Messaging::TraceFactoryType<Core::Messaging::IStore::WarningReporting, Messaging::TextMessage> _warningReportingFactory;
        Cleanups _cleaning;
    };

} // namespace Plugin
}
