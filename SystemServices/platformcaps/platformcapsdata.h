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
    typedef WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> Client;
    typedef WPEFramework::Core::ProxyType<Client> ClientProxy;

  public:
    JsonRpc() = default;
    JsonRpc(const JsonRpc &) = delete;
    JsonRpc &operator=(const JsonRpc &) = delete;

  public:
    JsonObject invoke(const string &callsign,
                      const string &method, const uint32_t waitTime);

    bool activate(const string &callsign, const uint32_t waitTime);

    ClientProxy getClient(const string &callsign);

  private:
    std::map <string, ClientProxy> clients;
  };

private:
  JsonRpc jsonRpc;
};

} // namespace Plugin
} // namespace WPEFramework
