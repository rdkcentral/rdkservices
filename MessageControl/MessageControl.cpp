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

#include "MessageControl.h"
#include "MessageOutput.h"
#include <interfaces/json/JMessageControl.h>

namespace WPEFramework {

    namespace Plugin {

    namespace {

        static Metadata<MessageControl> metadata(
            // Version
            1, 0, 0,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    MessageControl::Config::NetworkNode::NetworkNode()
        : Core::JSON::Container()
        , Port(2200)
        , Binding("0.0.0.0")
    {
        Add(_T("port"), &Port);
        Add(_T("binding"), &Binding);
    }

    MessageControl::Config::NetworkNode::NetworkNode(const NetworkNode& copy)
        : Core::JSON::Container()
        , Port(copy.Port)
        , Binding(copy.Binding)
    {
        Add(_T("port"), &Port);
        Add(_T("binding"), &Binding);
    }

    MessageControl::MessageControl()
        : _adminLock()
        , _outputLock()
        , _config()
        , _outputDirector()
        , _webSocketExporter()
        , _callback(nullptr)
        , _observer(*this)
        , _service(nullptr)
        , _dispatcherIdentifier(Messaging::MessageUnit::Instance().Identifier())
        , _dispatcherBasePath(Messaging::MessageUnit::Instance().BasePath())
        , _client(_dispatcherIdentifier, _dispatcherBasePath, Messaging::MessageUnit::Instance().SocketPort())
        , _worker(*this)
        , _tracingFactory()
        , _loggingFactory()
        , _warningReportingFactory()
    {
        _client.AddInstance(0);
        _client.AddFactory(Core::Messaging::Metadata::type::TRACING, &_tracingFactory);
        _client.AddFactory(Core::Messaging::Metadata::type::LOGGING, &_loggingFactory);
        _client.AddFactory(Core::Messaging::Metadata::type::REPORTING, &_warningReportingFactory);
    }

    const string MessageControl::Initialize(PluginHost::IShell* service)
    {
        string message;

        ASSERT(service != nullptr);

        _config.Clear();
        _config.FromString(service->ConfigLine());

        Core::Messaging::MessageInfo::abbreviate abbreviate;

        if (_config.Abbreviated.Value() == 0) {
            abbreviate = Core::Messaging::MessageInfo::abbreviate::FULL;
        }
        else {
            abbreviate = Core::Messaging::MessageInfo::abbreviate::ABBREVIATED;
        }

        _service = service;
        _service->AddRef();

        if ((service->Background() == false) && (((_config.SysLog.IsSet() == false) && (_config.Console.IsSet() == false)) || (_config.Console.Value() == true))) {
            Announce(new Publishers::ConsoleOutput(abbreviate));
        }
        if ((service->Background() == true) && (((_config.SysLog.IsSet() == false) && (_config.Console.IsSet() == false)) || (_config.SysLog.Value() == true))) {
            Announce(new Publishers::SyslogOutput(abbreviate));
        }
        if (_config.FileName.Value().empty() == false) {
            _config.FileName = service->VolatilePath() + _config.FileName.Value();
            Announce(new Publishers::FileOutput(abbreviate, _config.FileName.Value()));
        }
        if ((_config.Remote.Binding.Value().empty() == false) && (_config.Remote.Port.Value() != 0)) {
            Announce(new Publishers::UDPOutput(Core::NodeId(_config.Remote.NodeId())));
        }

        _webSocketExporter.Initialize(service, _config.MaxExportConnections.Value());

        Exchange::JMessageControl::Register(*this, this);

        _service->Register(&_observer);

        if (Callback(&_observer) != Core::ERROR_NONE) {
            message = _T("MessageControl plugin could not be _configured.");
        }

        return (message);
    }

    void MessageControl::Deinitialize(VARIABLE_IS_NOT_USED PluginHost::IShell* service)
    {
        if (_service != nullptr) {
            ASSERT (_service == service);

            Exchange::JMessageControl::Unregister(*this);

            Callback(nullptr);

            _service->Unregister(&_observer);

            _outputLock.Lock();

            _webSocketExporter.Deinitialize();

            _outputLock.Unlock();

            while (_outputDirector.empty() == false) {
                delete _outputDirector.back();
                _outputDirector.pop_back();
            }

            _service->Release();
            _service = nullptr;
        }
    }

    string MessageControl::Information() const {
        // No additional info to report.
        return (string());
    }

    bool MessageControl::Attach(PluginHost::Channel& channel)
    {
        TRACE(Trace::Information, (Core::Format(_T("Attaching channel ID [%d]"), channel.Id()).c_str()));

        return (_webSocketExporter.Attach(channel.Id()));
    }

    void MessageControl::Detach(PluginHost::Channel& channel)
    {
        TRACE(Trace::Information, (Core::Format(_T("Detaching channel ID [%d]"), channel.Id()).c_str()));
        _webSocketExporter.Detach(channel.Id());
    }

    Core::ProxyType<Core::JSON::IElement> MessageControl::Inbound(const string&) {
        return (_webSocketExporter.Command());
    }

    Core::ProxyType<Core::JSON::IElement> MessageControl::Inbound(const uint32_t ID, const Core::ProxyType<Core::JSON::IElement>& element) {
        return (Core::ProxyType<Core::JSON::IElement>(_webSocketExporter.Received(ID, element)));
    }

} // namespace Plugin
}
