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

#include "WebBridge.h"

namespace WPEFramework {

    ENUM_CONVERSION_BEGIN(Plugin::WebBridge::context)

        { Plugin::WebBridge::context::NONE,    _TXT("none")    },
        { Plugin::WebBridge::context::ADDED,   _TXT("added")   },
        { Plugin::WebBridge::context::WRAPPED, _TXT("wrapped") },

    ENUM_CONVERSION_END(Plugin::WebBridge::context);

namespace Plugin {

    SERVICE_REGISTRATION(WebBridge, 1, 0);

    class Registration : public Core::JSON::Container {
    public:
        Registration(const Registration&) = delete;
        Registration& operator=(const Registration&) = delete;

        Registration()
            : Core::JSON::Container()
            , Event()
            , Callsign()
        {
            Add(_T("event"), &Event);
            Add(_T("id"), &Callsign);
        }
        ~Registration() override = default;

    public:
        Core::JSON::String Event;
        Core::JSON::String Callsign;
    };
    class Message : public Core::JSONRPC::Message {
    public:
        class CallContext : public Core::JSON::Container {
        public:
            CallContext(const CallContext&) = delete;
            CallContext& operator= (const CallContext&) = delete;

            CallContext()
                : Core::JSON::Container()
                , Callsign()
                , ChannelId(0)
                , Token()
                , OriginalId(0) {
                Add(_T("callsign"), &Callsign);
                Add(_T("channel"), &ChannelId);
                Add(_T("token"), &Token);
                Add(_T("id"), &OriginalId);
            }
            ~CallContext() override = default;

        public:
            Core::JSON::String Callsign;
            Core::JSON::DecUInt32 ChannelId;
            Core::JSON::String Token;
            Core::JSON::DecUInt32 OriginalId;
        };

    public:
        Message(const Message&) = delete;
        Message& operator= (const Message&) = delete;

        Message()
            : Core::JSONRPC::Message() {
            Add(_T("context"), &Context);
            Add(_T("request"), &Request);
            Add(_T("response"), &Response);
        }
        Message(const Core::JSONRPC::Message& inbound)
            : Core::JSONRPC::Message() 
            , Request(inbound) {
            Add(_T("context"), &Context);
            Add(_T("request"), &Request);
            Add(_T("response"), &Response);
        }
        ~Message() override = default;

    public:
        CallContext Context;
        Core::JSONRPC::Message Request;
        Core::JSONRPC::Message Response;
    };

    static Core::ProxyPoolType<Message> g_BridgeMessages(8);

    // -------------------------------------------------------------------------------------------------------
    //   IPluginExtended methods
    // -------------------------------------------------------------------------------------------------------
    const string WebBridge::Initialize(PluginHost::IShell* service) /* override */
    {
        ASSERT(_service == nullptr);
        ASSERT(service != nullptr);

        string message;

        Config config;
        config.FromString(service->ConfigLine());
        _skipURL = static_cast<uint8_t>(service->WebPrefix().length());
        _callsign = service->Callsign();
        _service = service;
        _mode = config.Context.Value();
        _timeOut = (config.TimeOut.Value() * Core::Time::TicksPerMillisecond);

        // On success return empty, to indicate there is no error text.
        return (message);
    }

    void WebBridge::Deinitialize(PluginHost::IShell* service) /* override */
    {
        ASSERT(_service == service);

        _service = nullptr;
    }

    string WebBridge::Information() const /* override */
    {
        // No additional info to report.
        return (string());
    }

    bool WebBridge::Attach(PluginHost::Channel& channel) /* override */ {
        bool assigned = false;

        // The expectation is that the JavaScript service opens up a connection to us, so we can forward the 
        // incomming requests, to be handled by the Service.
        if (_javascriptService == 0) {
            Web::ProtocolsArray protocols = channel.Protocols();
            if (std::find(protocols.begin(), protocols.end(), string(_T("json"))) != protocols.end()) {
                _javascriptService = channel.Id();
                assigned = true;
            }
        }
        return(assigned);
    }

    void WebBridge::Detach(PluginHost::Channel& channel) /* override */ {
        // Hopefull this does not happen as than we are loosing the actual service :-) We could do proper error handling
        // if this happens :-)
        _javascriptService = 0;
    }

    // -------------------------------------------------------------------------------------------------------
    //   IDispatcher methods
    // -------------------------------------------------------------------------------------------------------
    Core::ProxyType<Core::JSONRPC::Message> WebBridge::Invoke(const Core::JSONRPC::Context& context, const Core::JSONRPC::Message& inbound) /* override */
    {
        string method;
        Registration info;

        Core::ProxyType<Message> message(g_BridgeMessages.Element());
        string designator(inbound.Designator.Value());

        if (inbound.Id.IsSet() == true) {
            message->JSONRPC = Core::JSONRPC::Message::DefaultVersion;
            message->Id = inbound.Id.Value();
        }

        switch (Destination(designator, method)) {
        case state::STATE_INCORRECT_HANDLER:
            message->Error.SetError(Core::ERROR_INVALID_DESIGNATOR);
            message->Error.Text = _T("Destined invoke failed.");
            break;
        case state::STATE_INCORRECT_VERSION:
            message->Error.SetError(Core::ERROR_INVALID_SIGNATURE);
            message->Error.Text = _T("Requested version is not supported.");
            break;
        case state::STATE_UNKNOWN_METHOD:
            message->Error.SetError(Core::ERROR_UNKNOWN_KEY);
            message->Error.Text = _T("Unknown method.");
            break;
        case state::STATE_REGISTRATION:
            info.FromString(inbound.Parameters.Value());
            Subscribe(context.ChannelId(), info.Event.Value(), info.Callsign.Value(), *message);
            break;
        case state::STATE_UNREGISTRATION:
            info.FromString(inbound.Parameters.Value());
            Unsubscribe(context.ChannelId(), info.Event.Value(), info.Callsign.Value(), *message);
            break;
        case state::STATE_EXISTS:
            message->Result = Core::NumberType<uint32_t>(Core::ERROR_UNKNOWN_KEY).Text();
            break;
        case state::STATE_NONE_EXISTING:
            message->Result = Core::NumberType<uint32_t>(Core::ERROR_NONE).Text();
            break;
        case state::STATE_CUSTOM:
            // Let's on behalf of the request forward it and update 
            uint32_t newId = Core::_InterlockedIncrement(_sequenceId);
            Core::Time waitTill = Core::Time::Now() + _timeOut;

            _pendingRequests.emplace(std::piecewise_construct,
                std::forward_as_tuple(newId),
                std::forward_as_tuple(context.ChannelId(), message->Id.Value(), waitTill));

            message->Id = newId;

            switch (_mode) {
            case WebBridge::context::ADDED: {
                message->Context.ChannelId = context.ChannelId();
                message->Context.OriginalId = context.Sequence();
                message->Context.Token = context.Token();
                message->Context.Callsign = _callsign;
            }
            case WebBridge::context::NONE: {
                message->Parameters = inbound.Parameters;
                message->Designator = inbound.Designator;
                break;
            }
            case WebBridge::context::WRAPPED: {
                Message wrapper(inbound);
                wrapper.Remove(_T("response"));
                message->Remove(_T("request"));
                message->Remove(_T("response"));

                wrapper.Context.ChannelId = context.ChannelId();
                wrapper.Context.Token = context.Token();

                string fullParameters;
                wrapper.ToString(fullParameters);
                message->Parameters = fullParameters;
                message->Designator = _callsign;
                break;
            }
            }

            TRACE(Trace::Information, (_T("Request: [%d] from [%d], method: [%s]"), newId, context.ChannelId(), method.c_str()));

            _service->Submit(_javascriptService, Core::ProxyType<Core::JSON::IElement>(message));

            // Wait for ID to return, we can not report anything back yet...
            message.Release();

            if (_timeOut != 0) {
                _cleaner.Reschedule(waitTill);
            }

            break;
        }

        return (Core::ProxyType<Core::JSONRPC::Message>(message));
    }

    void WebBridge::Activate(PluginHost::IShell* /* service */) /* override */ {
        // We did what we needed to do in the Intialize.
    }

    void WebBridge::Deactivate() /* override */ {
        // We did what we needed to do in the Deintialize.
    }

    // -------------------------------------------------------------------------------------------------------
    //   IWebSocket methods
    // -------------------------------------------------------------------------------------------------------
    Core::ProxyType<Core::JSON::IElement> WebBridge::Inbound(const string& /* identifier */) /* override */{
        // There is a message coming in over the JSON WebSocket path!, give it storage space..
        return (Core::ProxyType<Core::JSON::IElement>(PluginHost::IFactories::Instance().JSONRPC()));
    }

    Core::ProxyType<Core::JSON::IElement> WebBridge::Inbound(const uint32_t ID, const Core::ProxyType<Core::JSON::IElement>& element) /* override */ {

        Core::ProxyType<Core::JSONRPC::Message> message(element);

        ASSERT(message.IsValid() == true);

        if (message.IsValid()) {

            if (message->Id.IsSet() == false) {

                string eventName(message->Method());

                // Check for control messages between server and us..
                if (InternalMessage(message) == false) {

                    // This is an event, we need event handling..
                    _adminLock.Lock();

                    ObserverMap::iterator index = _observers.find(eventName);

                    if (index != _observers.end()) {
                        for (const Observer& entry : index->second) {
                            Core::ProxyType<Core::JSONRPC::Message> outbound(PluginHost::IFactories::Instance().JSONRPC());
                            outbound->Designator = (entry.Designator().empty() == false ? entry.Designator() + '.' + eventName : eventName);
                            outbound->Parameters = message->Parameters.Value();

                            _service->Submit(entry.Id(), Core::ProxyType<Core::JSON::IElement>(outbound));
                        }
                    }

                    _adminLock.Unlock();
                }                    
            }
            else {
                uint32_t requestId, channelId = 0;

                // This is the response to an invoked method, Let's see who should get this repsonse :-)
                _adminLock.Lock();
                PendingMap::iterator index = _pendingRequests.find(message->Id.Value());
                if (index != _pendingRequests.end()) {
                    channelId = index->second.ChannelId();
                    requestId = index->second.SequenceId();
                    _pendingRequests.erase(index);
                }
                _adminLock.Unlock();

                if (channelId != 0) {
                    TRACE(Trace::Information, (_T("Response: [%d] to [%d]"), requestId, channelId));

                    // Oke, there is someone waiting for a response!
                    message->Id = requestId;
                    _service->Submit(channelId, Core::ProxyType<Core::JSON::IElement>(message));
                }
            }
        }

        // We will never report anything back here :-)
        return (Core::ProxyType<Core::JSON::IElement>());
    }

    // -------------------------------------------------------------------------------------------------------
    //   Private methods
    // -------------------------------------------------------------------------------------------------------
    void WebBridge::Cleanup() {
        // Lets see if there are still any pending request we should report Missing In Action :-)
        Core::Time now (Core::Time::Now());
        Core::Time nextSlot;

        _adminLock.Lock();
        PendingMap::iterator index(_pendingRequests.begin());
        while (index != _pendingRequests.end()) {
            if (now >= index->second.Issued()) {
                // Send and Error to the requester..
                Core::ProxyType<Core::JSONRPC::Message> message(PluginHost::IFactories::Instance().JSONRPC());
                message->Error.SetError(Core::ERROR_TIMEDOUT);
                message->Error.Text = _T("There is no response form the server within time!!!");
                message->Id = index->second.SequenceId();

                TRACE(Trace::Warning, (_T("Got a timeout on channelId [%d] for request [%d]"), index->second.ChannelId(), message->Id.Value()));

                _service->Submit(index->second.ChannelId(), Core::ProxyType<Core::JSON::IElement>(message));
                index = _pendingRequests.erase(index);
            }
            else {
                if ((nextSlot.IsValid() == false) || (nextSlot > index->second.Issued())) {
                    nextSlot = index->second.Issued();
                }
                index++;
            }
        }
        _adminLock.Unlock();

        if (nextSlot.IsValid()) {
            _cleaner.Reschedule(nextSlot);
        }
    }

    bool WebBridge::InternalMessage(const Core::ProxyType<Core::JSONRPC::Message>& message) {
        bool result = false;

        string eventName(message->Method());

        if (eventName == "registerjsonrpcmethods") {
            result = true;
            Core::JSON::ArrayType<Core::JSON::String> parameter;
            parameter.FromString(message->Parameters.Value());
            Core::JSON::ArrayType<Core::JSON::String>::Iterator index(parameter.Elements());

            _supportedVersions.clear();

            while (index.Next() == true) {
                string entry = index.Current().Value();
                uint8_t version = Core::JSONRPC::Message::Version(entry);
                string method = Core::JSONRPC::Message::Method(entry);
                VersionMap::iterator placement = _supportedVersions.find(version);

                if (placement == _supportedVersions.end()) {
                    auto newEntry = _supportedVersions.emplace(std::piecewise_construct,
                        std::forward_as_tuple(version),
                        std::forward_as_tuple());

                    newEntry.first->second.push_back(method);
                }
                else if (std::find(placement->second.begin(), placement->second.end(), method) == placement->second.end()) {
                    // Check if this label does not already exist
                    placement->second.push_back(method);
                }
            }
        }

        return (result);
    }

} // namespace Plugin
} // namespace WPEFramework
