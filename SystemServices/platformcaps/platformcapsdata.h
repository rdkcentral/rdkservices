/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#pragma once

#include "../Module.h"

namespace WPEFramework {
namespace Plugin {

class PlatformCapsData {
public:
  typedef std::tuple <string, string, string> BrowserInfo;
  
  PlatformCapsData(PluginHost::IShell* service) : jsonRpc(service) {}

public:
  /**
   * Things ported from the XRE Receiver onConnect
   */
  void AddDashExclusionList(std::map<string, std::list <string>> & hash) const;
  std::list <string> GetQuirks() const;
  BrowserInfo GetBrowser() const;
  bool SupportsTrueSD() const;
  bool CanMixPCMWithSurround();
  bool GetFirmwareUpdateDisabled() const;
  std::map <string, uint8_t> DeviceCapsFeatures() const;
  std::list <string> GetMimeTypes() const;

private:
  std::map <string, uint8_t> getAvailablePlugins() const;
  bool verifyLibraries(const JsonArray &libraries) const;

public:
  /**
   * RPC
   */
  string GetModel();
  string GetDeviceType();
  string GetHDRCapability();
  string GetAccountId();
  string GetX1DeviceId();
  bool XCALSessionTokenAvailable();
  string GetExperience();
  string GetDdeviceMACAddress();
  string GetPublicIP();

private:
  class JsonRpc {
  private:
    struct JSONRPCDirectLink {
     private:
      uint32_t mId{ 0 };
      std::string mCallSign{};
      PluginHost::IDispatcher* dispatcher_{ nullptr };

      Core::ProxyType<Core::JSONRPC::Message> Message() const
      {
        return (Core::ProxyType<Core::JSONRPC::Message>(PluginHost::IFactories::Instance().JSONRPC()));
      }

      template <typename PARAMETERS>
      bool ToMessage(PARAMETERS& parameters, Core::ProxyType<Core::JSONRPC::Message>& message) const
      {
        return ToMessage((Core::JSON::IElement*)(&parameters), message);
      }
      bool ToMessage(Core::JSON::IElement* parameters, Core::ProxyType<Core::JSONRPC::Message>& message) const
      {
        if (!parameters->IsSet())
          return true;
        string values;
        if (!parameters->ToString(values)) {
          std::cout << "Failed to convert params to string\n";
          return false;
        }
        if (values.empty() != true) {
          message->Parameters = values;
        }
        return true;
      }
      template <typename RESPONSE>
      bool FromMessage(RESPONSE& response, const Core::ProxyType<Core::JSONRPC::Message>& message, bool isResponseString = false) const
      {
        return FromMessage((Core::JSON::IElement*)(&response), message, isResponseString);
      }
      bool FromMessage(Core::JSON::IElement* response, const Core::ProxyType<Core::JSONRPC::Message>& message, bool isResponseString = false) const
      {
        Core::OptionalType<Core::JSON::Error> error;
        if (!isResponseString && !response->FromString(message->Result.Value(), error)) {
          std::cout << "Failed to parse response!!! Error: '" << error.Value().Message() << "'\n";
          return false;
        }
        return true;
      }

     public:
      JSONRPCDirectLink(PluginHost::IShell* service, std::string callsign)
          : mCallSign(callsign)
      {
        if (service)
          dispatcher_ = service->QueryInterfaceByCallsign<PluginHost::IDispatcher>(mCallSign);
      }
      ~JSONRPCDirectLink()
      {
        if (dispatcher_)
          dispatcher_->Release();
      }

      template <typename PARAMETERS>
      uint32_t Get(const uint32_t waitTime, const string& method, PARAMETERS& respObject)
      {
        JsonObject empty;
        return Invoke(waitTime, method, empty, respObject);
      }

      template <typename PARAMETERS>
      uint32_t Set(const uint32_t waitTime, const string& method, const PARAMETERS& sendObject)
      {
        JsonObject empty;
        return Invoke(waitTime, method, sendObject, empty);
      }

      template <typename PARAMETERS, typename RESPONSE>
      uint32_t Invoke(const uint32_t waitTime, const string& method, const PARAMETERS& parameters, RESPONSE& response, bool isResponseString = false)
      {
        if (dispatcher_ == nullptr) {
          std::cout << "No JSON RPC dispatcher for " << mCallSign << '\n';
          return Core::ERROR_GENERAL;
        }

        auto message = Message();

        message->JSONRPC = Core::JSONRPC::Message::DefaultVersion;
        message->Id = Core::JSON::DecUInt32(++mId);
        message->Designator = Core::JSON::String(mCallSign + ".1." + method);

        ToMessage(parameters, message);

        const uint32_t channelId = ~0;
        auto resp = dispatcher_->Invoke("", channelId, *message);
        if (resp->Error.IsSet()) {
          std::cout << "Call failed: " << message->Designator.Value() << " error: " << resp->Error.Text.Value() << "\n";
          return resp->Error.Code;
        }

        if (!FromMessage(response, resp, isResponseString))
          return Core::ERROR_GENERAL;

        return Core::ERROR_NONE;
      }
    };
      
    typedef JSONRPCDirectLink Client;
    typedef WPEFramework::Core::ProxyType<Client> ClientProxy;

  public:
    JsonRpc(PluginHost::IShell* service) : _service(service) {}
    JsonRpc(const JsonRpc &) = delete;
    JsonRpc &operator=(const JsonRpc &) = delete;

  public:
    JsonObject invoke(const string &callsign,
                      const string &method, const uint32_t waitTime);

    bool activate(const string &callsign, const uint32_t waitTime);

    ClientProxy getClient(const string &callsign);

  private:
    std::map <string, ClientProxy> clients;
    PluginHost::IShell* _service;
  };

private:
  JsonRpc jsonRpc;
};

} // namespace Plugin
} // namespace WPEFramework
