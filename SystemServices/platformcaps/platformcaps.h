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

class PlatformCaps : public Core::JSON::Container {
public:
  class WebBrowser : public Core::JSON::Container {
  public:
    WebBrowser()
        : Core::JSON::Container(), browserType(), version(), userAgent() {
      Add(_T("browserType"), &browserType);
      Add(_T("version"), &version);
      Add(_T("userAgent"), &userAgent);
    }

    Core::JSON::String browserType;
    Core::JSON::String version;
    Core::JSON::String userAgent;
  };

  class AccountInfo : public Core::JSON::Container {
  public:
    AccountInfo() = default;

    /**
     * @param query - e.g. "accountId", "" (all)
     * @return
     */
    bool Load(PluginHost::IShell* service, const string &query = string());

    Core::JSON::String accountId;
    Core::JSON::String x1DeviceId;
    Core::JSON::Boolean XCALSessionTokenAvailable;
    Core::JSON::String experience;
    Core::JSON::String deviceMACAddress;
    Core::JSON::Boolean firmwareUpdateDisabled;
  };

  class DeviceInfo : public Core::JSON::Container {
  public:
    DeviceInfo() = default;

    /**
     * @param query - e.g. "deviceType", "" (all)
     * @return
     */
    bool Load(PluginHost::IShell* service, const string &query = string());

    Core::JSON::ArrayType <Core::JSON::String> quirks;
    JsonObject mimeTypeExclusions;
    JsonObject features;
    Core::JSON::ArrayType <Core::JSON::String> mimeTypes;
    Core::JSON::String model;
    Core::JSON::String deviceType;
    Core::JSON::Boolean supportsTrueSD;
    WebBrowser webBrowser;
    Core::JSON::String HdrCapability;
    Core::JSON::Boolean canMixPCMWithSurround;
    Core::JSON::String publicIP;
  };

public:
  PlatformCaps() = default;

  /**
   * @param query - e.g. "AccountInfo.accountId", "DeviceInfo", "" (all)
   * @return
   */
  bool Load(PluginHost::IShell* service, const string &query = string());

  AccountInfo accountInfo;
  DeviceInfo deviceInfo;
  Core::JSON::Boolean success;
};

} // namespace Plugin
} // namespace WPEFramework
