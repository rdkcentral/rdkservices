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

#include "platformcaps.h"

#include "platformcapsdata.h"

#include <regex>

namespace WPEFramework {
namespace Plugin {

bool PlatformCaps::Load(PluginHost::IShell* service, const string &query) {
  bool result = true;

  Reset();

  std::smatch m;
  std::regex_search(query, m,
      std::regex("^(AccountInfo|DeviceInfo)(\\.(\\w*)){0,1}"));

  if (query.empty() || !m.empty()) {
    if (query.empty() || (m[1] == _T("AccountInfo"))) {
      if (!accountInfo.Load(service, m.size() > 3 ? m[3] : string())) {
        result = false;
      }
      Add(_T("AccountInfo"), &accountInfo);
    }

    if (query.empty() || (m[1] == _T("DeviceInfo"))) {
      if (!deviceInfo.Load(service, m.size() > 3 ? m[3] : string())) {
        result = false;
      }
      Add(_T("DeviceInfo"), &deviceInfo);
    }
  } else {
    TRACE(Trace::Error, (_T("%s Bad query '%s'\n"),
        __FILE__, query.c_str()));
    result = false;
  }

  success = result;
  Add(_T("success"), &success);

  return result;
}

bool PlatformCaps::AccountInfo::Load(PluginHost::IShell* service, const string &query) {
  bool result = true;

  Reset();

  PlatformCapsData data(service);

  if (query.empty() || query == _T("accountId")) {
    accountId = data.GetAccountId();
    Add(_T("accountId"), &accountId);
  }

  if (query.empty() || query == _T("x1DeviceId")) {
    x1DeviceId = data.GetX1DeviceId();
    Add(_T("x1DeviceId"), &x1DeviceId);
  }

  if (query.empty() || query == _T("XCALSessionTokenAvailable")) {
    XCALSessionTokenAvailable = data.XCALSessionTokenAvailable();
    Add(_T("XCALSessionTokenAvailable"), &XCALSessionTokenAvailable);
  }

  if (query.empty() || query == _T("experience")) {
    experience = data.GetExperience();
    Add(_T("experience"), &experience);
  }

  if (query.empty() || query == _T("deviceMACAddress")) {
    deviceMACAddress = data.GetDdeviceMACAddress();
    Add(_T("deviceMACAddress"), &deviceMACAddress);
  }

  if (query.empty() || query == _T("firmwareUpdateDisabled")) {
    firmwareUpdateDisabled = data.GetFirmwareUpdateDisabled();
    Add(_T("firmwareUpdateDisabled"), &firmwareUpdateDisabled);
  }

  return result;
}

bool PlatformCaps::DeviceInfo::Load(PluginHost::IShell* service, const string &query) {
  bool result = true;

  Reset();

  PlatformCapsData data(service);

  if (query.empty() || query == _T("quirks")) {
    quirks.Clear();
    auto q = data.GetQuirks();
    for (const auto &value: q)
      quirks.Add() = value;
    Add(_T("quirks"), &quirks);
  }

  if (query.empty() || query == _T("mimeTypeExclusions")) {
    mimeTypeExclusions.Reset();
    std::map <string, std::list<string>> hash;
    data.AddDashExclusionList(hash);
    if (!hash.empty()) {
      for (auto &it: hash) {
        JsonArray jsonArray;
        for (auto &jt: it.second) {
          jsonArray.Add() = jt;
        }
        mimeTypeExclusions[it.first.c_str()] = jsonArray;
      }
      Add(_T("mimeTypeExclusions"), &mimeTypeExclusions);
    }
  }

  if (query.empty() || query == _T("features")) {
    features.Reset();
    auto hash = data.DeviceCapsFeatures();
    if (!hash.empty()) {
      for (auto &it: hash) {
        features[it.first.c_str()] = it.second;
      }
      Add(_T("features"), &features);
    }
  }

  if (query.empty() || query == _T("mimeTypes")) {
    mimeTypes.Clear();
    auto q = data.GetMimeTypes();
    for (const auto &value: q)
      mimeTypes.Add() = value;
    Add(_T("mimeTypes"), &mimeTypes);
  }

  if (query.empty() || query == _T("model")) {
    model = data.GetModel();
    Add(_T("model"), &model);
  }

  if (query.empty() || query == _T("deviceType")) {
    deviceType = data.GetDeviceType();
    Add(_T("deviceType"), &deviceType);
  }

  if (query.empty() || query == _T("supportsTrueSD")) {
    supportsTrueSD = data.SupportsTrueSD();
    Add(_T("supportsTrueSD"), &supportsTrueSD);
  }

  if (query.empty() || query == _T("webBrowser")) {
    auto b = data.GetBrowser();
    webBrowser.browserType = std::get<0>(b);
    webBrowser.version = std::get<1>(b);
    webBrowser.userAgent = std::get<2>(b);
    Add(_T("webBrowser"), &webBrowser);
  }

  if (query.empty() || query == _T("HdrCapability")) {
    HdrCapability = data.GetHDRCapability();
    Add(_T("HdrCapability"), &HdrCapability);
  }

  if (query.empty() || query == _T("canMixPCMWithSurround")) {
    canMixPCMWithSurround = data.CanMixPCMWithSurround();
    Add(_T("canMixPCMWithSurround"), &canMixPCMWithSurround);
  }

  if (query.empty() || query == _T("publicIP")) {
    publicIP = data.GetPublicIP();
    Add(_T("publicIP"), &publicIP);
  }

  return result;
}

} // namespace Plugin
} // namespace WPEFramework
