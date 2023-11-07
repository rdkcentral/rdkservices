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

namespace WPEFramework {
namespace Plugin {
    class MessageControl : public PluginHost::JSONRPC, public PluginHost::IPluginExtended, public PluginHost::IWebSocket {
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
            , public Exchange::IMessageControl::ICollect::ICallback {
        private:
            enum state {
                ATTACHING,
                DETACHING,
                OBSERVING
            };
            using ObservingMap = std::unordered_map<uint32_t, state>;

        public:
            Observer() = delete;
            Observer(const Observer&) = delete;
            Observer& operator= (const Observer&) = delete;

            explicit Observer(MessageControl& parent)
                : _parent(parent)
                , _adminLock()
                , _observing()
                , _job(*this) {
            }
            ~Observer() override {
                _job.Revoke();
            }

        public:
            //
            // Exchange::IMessageControl::INotification
            // ----------------------------------------------------------
            void Message(const Exchange::IMessageControl::messagetype type,
                const string& module, const string& category, const string& fileName,
                const uint16_t lineNumber, const string& className,
                const uint64_t timestamp, const string& message) override {
                _parent.Message(static_cast<Core::Messaging::Metadata::type>(type), module, category , fileName, lineNumber, className, timestamp, message);
            }

            //
            // RPC::IRemoteConnection::INotification
            // ----------------------------------------------------------
            void Activated(RPC::IRemoteConnection* connection) override {

                uint32_t id = connection->Id();

                _adminLock.Lock();

                // Seems the ID is already in here, thats odd, and impossible :-)
                ObservingMap::iterator index = _observing.find(id);

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
            void Deactivated(RPC::IRemoteConnection* connection) override {

                uint32_t id = connection->Id();

                _adminLock.Lock();

                // Seems the ID is already in here, thats odd, and impossible :-)
                ObservingMap::iterator index = _observing.find(id);

                if (index != _observing.end()) {
                    if (index->second == state::ATTACHING) {
                        _observing.erase(index);
                    }
                    else if (index->second == state::OBSERVING) {
                        _observing.emplace(std::piecewise_construct,
                            std::make_tuple(id),
                            std::make_tuple(state::DETACHING));
                    }
                }

                _adminLock.Unlock();

                _job.Submit();
            }

            BEGIN_INTERFACE_MAP(Observer)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                INTERFACE_ENTRY(Exchange::IMessageControl::ICollect::ICallback)
            END_INTERFACE_MAP

        private:
            friend class Core::ThreadPool::JobType<Observer&>;

            void Dispatch()
            {
                _adminLock.Lock();

                ObservingMap::iterator index = _observing.begin();

                while (index != _observing.end()) {
                    if (index->second == state::ATTACHING) {
                        index->second = state::OBSERVING;
                        _parent.Attach(index->first);
                        index++;
                    }
                    else if (index->second == state::DETACHING) {
                        _parent.Detach(index->first);
                        index = _observing.erase(index);
                    }
                    else {
                        index++;
                    }
                }

                _adminLock.Unlock();
            }

        private:
            MessageControl& _parent;
            Core::CriticalSection _adminLock;
            ObservingMap _observing;
            Core::WorkerPool::JobType<Observer&> _job;
        };

    public:
        MessageControl(const MessageControl&) = delete;
        MessageControl& operator=(const MessageControl&) = delete;

        MessageControl();
        ~MessageControl() override = default;

        BEGIN_INTERFACE_MAP(MessageControl)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_ENTRY(PluginHost::IPluginExtended)
            INTERFACE_ENTRY(PluginHost::IWebSocket)
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
        void Announce(Publishers::IPublish* output) {

            _outputLock.Lock();

            ASSERT(std::find(_outputDirector.begin(), _outputDirector.end(), output) == _outputDirector.end());

            _outputDirector.emplace_back(output);

            _outputLock.Unlock();
        }
        void Message(const Core::Messaging::Metadata::type type,
            const string & module, const string& category, const string & fileName,
            const uint16_t lineNumber, const string & className,
            const uint64_t timestamp, const string & message) {

            // Time to start sending it to all interested parties...
            _outputLock.Lock();

            for (auto& entry : _outputDirector) {
                entry->Message(type, module, category, fileName, lineNumber, className, timestamp, message);
            }

            _webSocketExporter.Message(type, module, category, fileName, lineNumber, className, timestamp, message);

            _outputLock.Unlock();
        }
        void Attach(const uint32_t id) {
            _adminLock.Lock();
            _collect->Attach(id);
            _adminLock.Unlock();
        }
        void Detach(const uint32_t id) {
            _adminLock.Lock();
            _collect->Detach(id);
            _adminLock.Unlock();

            if (id == _connectionId) {
                ASSERT(_service != nullptr);
                Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
            }
        }

    private:
        Core::CriticalSection _adminLock;
        Core::CriticalSection _outputLock;
        Config _config;
        OutputList _outputDirector;
        Publishers::WebSocketOutput _webSocketExporter;
        Exchange::IMessageControl* _control;
        Exchange::IMessageControl::ICollect* _collect;
        Core::Sink<Observer> _observer;
        uint32_t _connectionId;
        PluginHost::IShell* _service;
    };

} // namespace Plugin
} // namespace WPEFramework
