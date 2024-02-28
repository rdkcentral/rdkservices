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

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {

ENUM_CONVERSION_BEGIN(Plugin::WebBridge::context)

    { Plugin::WebBridge::context::NONE, _TXT("none") },
    { Plugin::WebBridge::context::ADDED,   _TXT("added") },
    { Plugin::WebBridge::context::WRAPPED, _TXT("wrapped") },

ENUM_CONVERSION_END(Plugin::WebBridge::context);

    namespace Plugin {

        namespace {

            static Metadata<WebBridge> metadata(
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

        SERVICE_REGISTRATION(WebBridge, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH)

        class Registration : public Core::JSON::Container {
        public:
            Registration(Registration&&) = delete;
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
                CallContext(CallContext&&) = delete;
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

                void Clear() {
                    Callsign.Clear();
                    ChannelId.Clear();
                    Token.Clear();
                    OriginalId.Clear();
                }

            public:
                Core::JSON::String Callsign;
                Core::JSON::DecUInt32 ChannelId;
                Core::JSON::String Token;
                Core::JSON::DecUInt32 OriginalId;
            };

        public:
            Message(Message&&) = delete;
            Message(const Message&) = delete;
            Message& operator= (const Message&) = delete;

            Message()
                : Core::JSONRPC::Message() {
                Add(_T("context"), &Context);
                Add(_T("request"), &Request);
                Add(_T("response"), &Response);

                Core::JSONRPC::Message::JSONRPC.Clear();
                Request.JSONRPC.Clear();
                Response.JSONRPC.Clear();
            }
            ~Message() override = default;

            void Clear()
            {
                Request.Id.Clear();
                Request.Designator.Clear();
                Request.Parameters.Clear();
                Request.Result.Clear();
                Request.Error.Clear();
                Request.JSONRPC.Clear();

                Response.Id.Clear();
                Response.Designator.Clear();
                Response.Parameters.Clear();
                Response.Result.Clear();
                Response.Error.Clear();
                Response.JSONRPC.Clear();

                Context.Clear();

                Core::JSONRPC::Message::Id.Clear();
                Core::JSONRPC::Message::Designator.Clear();
                Core::JSONRPC::Message::Parameters.Clear();
                Core::JSONRPC::Message::Result.Clear();
                Core::JSONRPC::Message::Error.Clear();
                Core::JSONRPC::Message::JSONRPC.Clear();
            }

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
            _service->AddRef();

            _mode = config.Context.Value();
            _timeOut = (config.TimeOut.Value() * Core::Time::TicksPerMillisecond);

#ifndef USE_THUNDER_R4
            if (message.length() != 0) {
                Deinitialize(service);
            }
#endif

            // On success return empty, to indicate there is no error text.
            return (message);
        }

        void WebBridge::Deinitialize(PluginHost::IShell* service) /* override */
        {
            if (_service != nullptr) {
                ASSERT(_service == service);

                _service->Release();
                _service = nullptr;
            }
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
#ifndef USE_THUNDER_R4
        Core::ProxyType<Core::JSONRPC::Message> WebBridge::Invoke(const string& token, const uint32_t channelId, const Core::JSONRPC::Message& inbound) /* override */
        {
            string method;
            Registration info;

            Core::ProxyType<Core::JSONRPC::Message> message(PluginHost::IFactories::Instance().JSONRPC());
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
                Subscribe(channelId, info.Event.Value(), info.Callsign.Value(), *message);
                break;
            case state::STATE_UNREGISTRATION:
                info.FromString(inbound.Parameters.Value());
                Unsubscribe(channelId, info.Event.Value(), info.Callsign.Value(), *message);
                break;
            case state::STATE_EXISTS:
                message->Result = Core::NumberType<uint32_t>(Core::ERROR_UNKNOWN_KEY).Text();
                break;
            case state::STATE_NONE_EXISTING:
                message->Result = Core::NumberType<uint32_t>(Core::ERROR_NONE).Text();
                break;
            case state::STATE_CUSTOM:
                // Let's on behalf of the request forward it and update 
                uint32_t newId = Core::InterlockedIncrement(_sequenceId);
                Core::Time waitTill = Core::Time::Now() + _timeOut;

                _pendingRequests.emplace(std::piecewise_construct,
                    std::forward_as_tuple(newId),
                    std::forward_as_tuple(channelId, message->Id.Value(), waitTill));

                message->Id = newId;
                message->Parameters = inbound.Parameters;
                message->Designator = inbound.Designator;

                TRACE(Trace::Information, (_T("Request: [%d] from [%d], method: [%s]"), message->Id.Value(), channelId, method.c_str()));

                _service->Submit(_javascriptService, Core::ProxyType<Core::JSON::IElement>(message));

                // Wait for ID to return, we can not report anything back yet...
                message.Release();

                if (_timeOut != 0) {
                    _cleaner.Schedule(waitTill);
                }

                break;
            }

            return message;
        }

        void WebBridge::Activate(PluginHost::IShell* /* service */) /* override */ {
            // We did what we needed to do in the Intialize.
        }

        void WebBridge::Deactivate() /* override */ {
            // We did what we needed to do in the Deintialize.
        }
#else
        Core::hresult 
            WebBridge::Invoke(
                IDispatcher::ICallback* callback,
                const uint32_t channelId,
                const uint32_t id,
                const string& token,
                const string& method,
                const string& parameters,
                string& response) /* override */
        {
            uint32_t result(Core::ERROR_BAD_REQUEST);
            Core::JSONRPC::Handler* handler(PluginHost::JSONRPC::Handler(method));
            string realMethod(Core::JSONRPC::Message::Method(method));

            if (handler == nullptr) {
                result = Core::ERROR_INVALID_RANGE;
            }
            else if (realMethod == _T("exists")) {
                result = Core::ERROR_NONE;
                if (handler->Exists(realMethod) == Core::ERROR_NONE) {
                    response = _T("1");
                }
                else {
                    response = _T("0");
                }
            }
            else if (handler->Exists(realMethod) == Core::ERROR_NONE) {

                // Let's on behalf of the request forward it and update 
                string messageToSend(parameters);
                Core::ProxyType<Message> message(g_BridgeMessages.Element());
                uint32_t newId = Core::_InterlockedIncrement(_sequenceId);
                Core::Time waitTill = Core::Time::Now() + _timeOut;

                _pendingRequests.emplace(std::piecewise_construct,
                     std::forward_as_tuple(newId),
                     std::forward_as_tuple(callback, channelId, id, waitTill));

                switch (_mode) {
                case WebBridge::context::ADDED: {
                    message->Context.ChannelId = channelId;
                    message->Context.OriginalId = id;
                    message->Context.Token = token;
                    message->Context.Callsign = _callsign;
                }
                case WebBridge::context::NONE: {
                    break;
                }
                case WebBridge::context::WRAPPED: {
                    Message wrapper;

                    wrapper.Context.ChannelId = channelId;
                    wrapper.Context.OriginalId = id;
                    wrapper.Context.Token = token;
                    wrapper.Parameters = parameters;
                    wrapper.ToString(messageToSend);
                    break;
                }
                }

                message->Id = newId;
                message->Parameters = messageToSend;
                message->Designator = method;

                TRACE(Trace::Information, (_T("Request: [%d] from [%d], method: [%s]"), newId, channelId, method.c_str()));

                _service->Submit(_javascriptService, Core::ProxyType<Core::JSON::IElement>(message));

                // Wait for ID to return, we can not report anything back yet...
                message.Release();

                if (_timeOut != 0) {
#ifndef USE_THUNDER_R4
                    _cleaner.Schedule(waitTill);
#else
                    _cleaner.Reschedule(waitTill);
#endif
                }

                result = ~0; // No resposne to report yet....
            }

            return (result);
        }
        Core::hresult WebBridge::Revoke(ICallback* callback) /* override*/ {
            // Remove the interface from the pendings..
            _adminLock.Lock();

            PendingMap::iterator index = _pendingRequests.begin();

            while (index != _pendingRequests.end()) {
                if (index->second != callback) {
                    index++;
                }
                else {
                    index = _pendingRequests.erase(index);
                }
            }

            _adminLock.Lock();

            return (PluginHost::JSONRPC::Revoke(callback));
        }

#endif
        // -------------------------------------------------------------------------------------------------------
        //   IWebSocket methods
        // -------------------------------------------------------------------------------------------------------
        Core::ProxyType<Core::JSON::IElement> WebBridge::Inbound(const string& /* identifier */) /* override */ {
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
                        PluginHost::JSONRPC::Event(eventName, message->Parameters.Value());
                    }
                }
                else {
                    // This is the response to an invoked method, Let's see who should get this repsonse :-)
                    _adminLock.Lock();
                    PendingMap::iterator index = _pendingRequests.find(message->Id.Value());
                    if (index != _pendingRequests.end()) {
                        uint32_t requestId, channelId;
                        IDispatcher::ICallback* callback;

                        channelId = index->second.ChannelId();
                        requestId = index->second.SequenceId();
                        callback = index->second.Callback();

                        ASSERT(callback != nullptr);

                        TRACE(Trace::Information, (_T("Response: [%d] to [%d]"), requestId, channelId));

#ifndef USE_THUNDER_R4

                        // Oke, there is someone waiting for a response!
                        message->Id = requestId;
                        _service->Submit(channelId, Core::proxy_cast<Core::JSON::IElement>(message));
#else
                        if (callback != nullptr) {
                            // Oke, there is someone waiting for a response!
                            callback->Response(channelId, requestId, message->Result.Value());
                            callback->Release();
                        }

#endif /* USE_THUNDER_R4 */
                        _pendingRequests.erase(index);
                    }
                    _adminLock.Unlock();
                }
            }

            // We will never report anything back here :-)
            return (Core::ProxyType<Core::JSON::IElement>());
        }

        // -------------------------------------------------------------------------------------------------------
        //   Private methods
        // -------------------------------------------------------------------------------------------------------
        void WebBridge::Dispatch() {
            // Lets see if there are still any pending request we should report Missing In Action :-)
            Core::Time now(Core::Time::Now());
            Core::Time nextSlot;

            _adminLock.Lock();
            PendingMap::iterator index(_pendingRequests.begin());
            while (index != _pendingRequests.end()) {
                if (now >= index->second.Issued()) {
                    // Send and Error to the requester..
                    IDispatcher::ICallback* callback = index->second.Callback();

                    ASSERT(callback != nullptr);

                    if (callback != nullptr) {
                        callback->Error(index->second.ChannelId(), index->second.SequenceId(), Core::ERROR_TIMEDOUT, _T("There is no response form the server within time!!!"));
                        callback->Release();
                    }

                    TRACE(Trace::Warning, (_T("Got a timeout on channelId [%d] for request [%d]"), index->second.ChannelId(), index->second.SequenceId()));

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
#ifndef USE_THUNDER_R4
                _cleaner.Schedule(nextSlot);
#else
                _cleaner.Reschedule(nextSlot);
#endif
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

                while (index.Next() == true) {
                    string entry = index.Current().Value();

                    PluginHost::JSONRPC::RegisterMethod(Core::JSONRPC::Message::Version(entry), Core::JSONRPC::Message::Method(entry));
                }
            }

            return (result);
        }

    } // namespace Plugin
} // namespace WPEFramework
