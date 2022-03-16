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
namespace Plugin {

    SERVICE_REGISTRATION(WebBridge, 1, 0);

    class Registration : public Core::JSON::Container {
    public:
        Registration() : Core::JSON::Container() , Event() , Callsign()
        {
            Add(_T("event"), &Event);
            Add(_T("id"), &Callsign);
        }
        Core::JSON::String Event;
        Core::JSON::String Callsign;
    };

    class Envelope: public Core::JSONRPC::Message {
    public:
      Envelope() : Core::JSONRPC::Message() {
        Remove(_T("params"));
        Remove(_T("result"));
        Add(_T("params"), &Params);
        Add(_T("result"), &Result);
      }
      struct CallContext : public Core::JSON::Container {
        CallContext() : Core::JSON::Container(), ChannelId(0), Token() {
          Add(_T("channel"), &ChannelId);
          Add(_T("token"), &Token);
        }
        Core::JSON::DecUInt32 ChannelId;
        Core::JSON::String Token;
      };
      struct Parameters : public Core::JSON::Container {
        Parameters() : Core::JSON::Container() {
          Add(_T("context"), &Context);
          Add(_T("request"), &Request);
        }
        CallContext Context;
        Core::JSONRPC::Message Request;
      };
      struct ResultType: public Core::JSON::Container {
        ResultType() : Core::JSON::Container() {
          Add(_T("context"), &Context);
          Add(_T("response"), &Response);
        }
        CallContext Context;
        Core::JSONRPC::Message Response;
      };
      Parameters Params;
      ResultType Result;

      static Core::ProxyType<Envelope> NewFromElement(const Core::ProxyType<Core::JSON::IElement> & element)
      {
        // TODO: is there a better way to do this?
        Core::ProxyType<Envelope> message(_messagePool.Element());
        string s;
        element->ToString(s);
        message->FromString(s);
        return message;
      }

      static Core::ProxyType<Envelope> NewFromPool()
      {
        Core::ProxyType<Envelope> message(_messagePool.Element());
        message->Remove(_T("response"));
        message->Remove(_T("result"));
        return message;
      }

      Core::ProxyType<Core::JSONRPC::Message> GetInnerResponse() const
      {
        auto res = Core::ProxyType<Core::JSONRPC::Message>(WPEFramework::PluginHost::IFactories::Instance().JSONRPC());
        res->JSONRPC = this->Result.Response.JSONRPC;
        res->Id = this->Result.Response.Id;
        if (this->Result.Response.Error.IsSet())
          res->Error = this->Result.Response.Error;
        if (this->Result.Response.Result.IsSet())
          res->Result = this->Result.Response.Result;
        return res;
      }

    private:
      static Core::ProxyPoolType<Envelope> _messagePool;
    };

    Core::ProxyPoolType<Envelope> Envelope::_messagePool(8);


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

        #if 0
        // The expectation is that the JavaScript service opens up a connection to us, so we can forward the 
        // incomming requests, to be handled by the Service.
        if (_javascriptService == 0) {
            Web::ProtocolsArray protocols = channel.Protocols();
            if (std::find(protocols.begin(), protocols.end(), string(_T("json"))) != protocols.end()) {
                _javascriptService = channel.Id();
                assigned = true;
            }
        }
        #endif

         if ((channel.Protocol() == _T("json")) && (_serviceSideChannelId == 0)) {
            _serviceSideChannelId = channel.Id();
            assigned = true;
        }

        return assigned;
    }

    void WebBridge::Detach(PluginHost::Channel& channel) /* override */ {
        // Hopefull this does not happen as than we are loosing the actual service :-) We could do proper error handling
        // if this happens :-)
        _serviceSideChannelId = 0;
    }

    // -------------------------------------------------------------------------------------------------------
    //   IDispatcher methods
    // -------------------------------------------------------------------------------------------------------
    Core::ProxyType<Core::JSONRPC::Message> WebBridge::Invoke(const Core::JSONRPC::Context& ctx, const Core::JSONRPC::Message& inbound) /* override */
    {
        string method;
        Registration info;

        Core::ProxyType<Core::JSONRPC::Message> message(PluginHost::IFactories::Instance().JSONRPC());
        if (inbound.Id.IsSet())
          message->Id = inbound.Id;

        // TODO: what are all these various states for?
        switch (Destination(string(inbound.Designator.Value()), method)) {
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
            Subscribe(ctx.ChannelId(), info.Event.Value(), info.Callsign.Value(), *message);
            break;
        case state::STATE_UNREGISTRATION:
            info.FromString(inbound.Parameters.Value());
            Unsubscribe(ctx.ChannelId(), info.Event.Value(), info.Callsign.Value(), *message);
            break;
        case state::STATE_EXISTS:
            message->Result = Core::NumberType<uint32_t>(Core::ERROR_UNKNOWN_KEY).Text();
            return message;
            break;
        case state::STATE_NONE_EXISTING:
            message->Result = Core::NumberType<uint32_t>(Core::ERROR_NONE).Text();
            return message;
            break;
        case state::STATE_CUSTOM:
            submitMessageToRemoteService(ctx, inbound);
            break;
        }

        return (Core::ProxyType<Core::JSONRPC::Message>(message));
    }

    void WebBridge::submitMessageToRemoteService(const Core::JSONRPC::Context & ctx, const Core::JSONRPC::Message & req)
    {
      // create a wrapper for the incoming request, take care to only
      // copy fields that are actually set
      Core::ProxyType<Envelope> message = Envelope::NewFromPool();
      message->Params.Context.Token = ctx.Token();
      message->Params.Context.ChannelId = ctx.ChannelId();
      message->Params.Request.JSONRPC = req.JSONRPC;
      message->Params.Request.Id = req.Id;
      message->Params.Request.Parameters = req.Parameters;

      // reconstruct the full service.version.method string, it has been decomposed
      // before being passed in
      string versionAndMethodName = req.Designator.Value();
      message->Params.Request.Designator = _callsign + "." + versionAndMethodName;
      message->Designator = _callsign;

      _service->Submit(_serviceSideChannelId, Core::ProxyType<Core::JSON::IElement>(message));
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
              const Core::ProxyType<Envelope> envelope = Envelope::NewFromElement(element);
              if (envelope.IsValid()) {
                auto innerResponse = envelope->GetInnerResponse(); 
                _service->Submit(envelope->Result.Context.ChannelId, Core::ProxyType<Core::JSON::IElement>(innerResponse));
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
