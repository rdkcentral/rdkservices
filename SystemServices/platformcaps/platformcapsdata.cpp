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

#include "platformcapsdata.h"

#include <regex>
#include <mutex>
#include <fstream>
#include <algorithm>

#include "host.hpp"
#include "videoOutputPort.hpp"
#include "audioOutputPort.hpp"

#include "rfcapi.h"

/**
 * TODO: define these!!!
 *    DISABLE_TRUE_SD, YOCTO_BUILD, AAMP_SUPPORTED
 *    SOUND_QUIRK, IP_VIDEO_NOT_SUPPORTED_QUIRK, QUIRK_XRE_12902
 *    ENABLE_HEADERS2, USE_AVE, ENABLE_MTHEORY_BROWSER
 *    HAS_HEVC4_SUPPORT, HAS_HEVC1_SUPPORT, BUILD_RNE
 */
#define YOCTO_BUILD
#define AAMP_SUPPORTED

#ifdef YOCTO_BUILD
#define DEFAULT_WEBBACKEND_CONFIG_FILE "/home/root/webbackend.conf"
#else
#define DEFAULT_WEBBACKEND_CONFIG_FILE "/mnt/nfs/env/webbackend.conf"
#endif
#define OVERRIDE_WEBBACKEND_CONFIG_FILE "/opt/webbackend.conf"

#define DEFAULT_PLUGIN_CONFIG_FILE "/home/root/xrepluginregistry.conf"
#define DEVICE_PLUGIN_CONFIG_FILE "/home/root/xrepluginregistry.device.conf"
#define OVERRIDE_PLUGIN_CONFIG_FILE "/opt/xrepluginregistry.conf"

#ifdef YOCTO_BUILD
#define PLUGIN_LIBRARY_PATH "/usr/lib"
#else
#define PLUGIN_LIBRARY_PATH "/usr/local/lib/plugins"
#endif

const string SWUpdateConf = "/opt/swupdate.conf";
const string DeviceRunXREProperties = "/etc/device.runXRE.properties";
const string DeviceProperties = "/etc/device.properties";

const std::list <string> PlaybackTypes{
    "LINEAR_TV", "VOD", "IVOD", "DVR", "CDVR", "IPDVR", "EAS"
};

namespace {
  std::map <string, string> getProperties(const string &filename) {
    std::map <string, string> result;

    std::ifstream fs(filename, std::ifstream::in);
    std::string::size_type p;
    std::string line;

    if (!fs.fail()) {
      while (std::getline(fs, line)) {
        if (!line.empty() && ((p = line.find('=')) > 0)) {
          result[line.substr(0, p)] = line.substr(p + 1, std::string::npos);
        }
      }
    }

    return result;
  }

  std::map <string, string> getDeviceProperties() {
    std::map <string, string> result;

    static std::once_flag flag;
    std::call_once(flag, [&](){
      auto props = getProperties(DeviceRunXREProperties);
      result.insert(props.begin(), props.end());

      props = getProperties(DeviceProperties);
      result.insert(props.begin(), props.end());
    });

    return result;
  }

  bool getRFC(const string &name, string &value) {
    bool result = false;

    RFC_ParamData_t param = {0};

    WDMP_STATUS status = getRFCParameter("PlatformCapsData", name.c_str(), &param);
    if (status == WDMP_SUCCESS) {
      value = param.value;
      result = true;
    }

    return result;
  }
}

namespace WPEFramework {
namespace Plugin {

/**
 * mimeTypeExclusions
 * see xre/generic/src/protocol/xreapplication.cpp
 * see xre/generic/scripts/generic/runXRE
 */

void PlatformCapsData::AddDashExclusionList(std::map <string, std::list <string>> &hash) const {
#ifdef AAMP_SUPPORTED
  std::vector <std::string> dashInclusions;

  auto properties = getDeviceProperties();

  if ((properties.find("ONLY_AVE_SUPPORTED") == properties.end()) ||
      (properties.find("DISABLE_DASH_SUPPORT") == properties.end())) {
    string str;

    if (getRFC(
        "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AAMP_CFG.DashPlaybackInclusions",
        str)) {
      TRACE(Trace::Information, (_T("%s dash inclusions: %s\n"),
          __FILE__, str.c_str()
      ));

      const std::regex re(":+");
      std::sregex_token_iterator it(
          str.begin(), str.end(), re, -1);
      dashInclusions = {it, {}};
    } else {
      TRACE(Trace::Error, (_T("%s No dash rfc\n"),
          __FILE__));
    }
  }

  if (std::find(dashInclusions.begin(), dashInclusions.end(), "*") ==
      dashInclusions.end()) {
    for (auto &it: PlaybackTypes) {
      if (std::find(dashInclusions.begin(), dashInclusions.end(), it) ==
          dashInclusions.end()) {
        hash[it] = {"application/dash+xml"};
      }
    }
  }
#else
  TRACE(Trace::Error, (_T("%s No aamp\n"), __FILE__));
#endif
}

/**
 * quirks
 * see xre/generic/src/receiver/xreglobals.cpp
 */

std::list <string> PlatformCapsData::GetQuirks() const {
  std::list <string> result{
      "XRE-4621", "XRE-4826", "XRE-4896", "XRE-5553",
      "XRE-5743",
#ifdef SOUND_QUIRK
      "BPV-7790",
#endif
      "XRE-6350", "XRE-6827", "XRE-7267", "XRE-7366",
      "XRE-7415", "XRE-7389", "DELIA-6142", "RDK-2222",
      "XRE-7924", "DELIA-8978", "XRE-7711", "RDK-2849",
      "DELIA-9338", "ZYN-172", "XRE-8970", "XRE-9001",
      "DELIA-17939", "DELIA-17204", "CPC-1594", "DELIA-21775",
      "XRE-11602", "CPC-1767", "CPC-1824",
#ifndef IP_VIDEO_NOT_SUPPORTED_QUIRK
      "XRE-10057",
#endif
      "RDK-21197",
#ifdef QUIRK_XRE_12902
      "XRE-12902",
#endif
      "CPC-2004", "DELIA-27583", "XRE-12919", "DELIA-28101",
      "XRE-13590", "XRE-13692", "XRE-13722", "DELIA-30269",
      "RDK-22801", "CPC-2404", "XRE-14664", "XRE-14921",
      "XRE-14963", "RDK-26425", "RDK-28990", "RDK-32261",
  };

  if (getenv("KEEP_RECEIVER_PROCESS_ON_STANDBY"))
    result.emplace_back(_T("RDK-22152"));

  if (getenv("RFC_ENABLE_HDMICEC_DAEMON"))
    result.emplace_back(_T("RDK-22967"));

  if (getenv("RFC_ENABLE_XDIALCAST"))
    result.emplace_back(_T("RDK-21397"));

  if (getenv("RFC_ENABLE_BT_AUX_AUDIO")) {
    result.emplace_back(_T("RDK-3575"));
    result.emplace_back(_T("RDK-24936"));
  }

  return result;
}

/**
 * webBrowser
 * see xre/generic/src/protocol/xreapplication.cpp
 */

PlatformCapsData::BrowserInfo PlatformCapsData::GetBrowser() const {
  BrowserInfo result;

  string type;

  Core::File file(OVERRIDE_WEBBACKEND_CONFIG_FILE);
  if (!file.Open(true))
    file = DEFAULT_WEBBACKEND_CONFIG_FILE;

  if (file.IsOpen() || file.Open(true)) {
    JsonObject json;
    json.IElement::FromFile(file);

    type = json["html_view"].String();
  } else {
    TRACE(WPEFramework::Trace::Error, (_T("%s File '%s' : %"PRIu32"\n"),
        __FILE__, file.Name().c_str(), file.ErrorCode()));
  }

  if (type == "rdkbrowser") {
    result = {"WPE", "1.0.0.0",
              "Mozilla/5.0 (Linux; x86_64 GNU/Linux) AppleWebKit/601.1 "
              "(KHTML, like Gecko) Version/8.0 Safari/601.1 WPE"};
  } else if (type == "cef") {
    result = {"CEF", "1.0.0.0",
              "Mozilla/5.0 (Unknown; Linux i686) AppleWebKit/537.21 "
              "(KHTML, like Gecko) NativeXREReceiver"};
  } else if (type == "webkit") {
    result = {"qtwebkit", "1.0.0.0",
              "Mozilla/5.0 (Unknown; Linux i686) AppleWebKit/537.21 "
              "(KHTML, like Gecko) NativeXREReceiver"};
  } else {
    result = {"WPE", "1.0.0.0",
              "Mozilla/5.0 (Linux; x86_64 GNU/Linux) AppleWebKit/601.1 "
              "(KHTML, like Gecko) Version/8.0 Safari/601.1 WPE"};
  }

  return result;
}

/**
 * supportsTrueSD
 * see xre/generic/src/receiver/xresettings.cpp
 */

bool PlatformCapsData::SupportsTrueSD() const {
#ifdef DISABLE_TRUE_SD
  return false;
#else
  return true;
#endif
}

/**
 * canMixPCMWithSurround
 * see xre/generic/src/receiver/localinput.cpp
 */

bool PlatformCapsData::CanMixPCMWithSurround() {
  bool result = false;

  try {
    device::List<device::VideoOutputPort> vPorts =
        device::Host::getInstance().getVideoOutputPorts();
    for (size_t i = 0; i < vPorts.size();) {
      device::AudioOutputPort &aPort = vPorts.at(i).getAudioOutputPort();
      result = aPort.isAudioMSDecode();
      break;
    }
  } catch (...) {
    result = false;
    TRACE(Trace::Error, 
        (_T("Exception Caught with device settings calls to get the MS11 Decode status..")));
  }

  TRACE(Trace::Information, (_T("canMixPCMWithSurround: %s"), result ? "YES" : "NO"));

  return result;
}

/**
 * firmwareUpdateDisabled
 * see xre/generic/scripts/generic/runXRE
 */

bool PlatformCapsData::GetFirmwareUpdateDisabled() const {
  Core::File file(SWUpdateConf);
  return (file.Exists());
}

/**
 * features
 * see generic/src/protocol/xreapplication.cpp
 * see generic/src/receiver/xreglobals.cpp
 */

std::map<string, uint8_t> PlatformCapsData::DeviceCapsFeatures() const {
  std::map<string, uint8_t> result;

  result["htmlview.headers"] = 1;
  result["htmlview.cookies"] = 1;
  result["htmlview.httpCookies"] = 1;
  result["htmlview.evaluateJavaScript"] = 1;
  result["htmlview.callJavaScriptWithResult"] = 1;
  result["htmlview.urlpatterns"] = 1;
  result["htmlview.disableCSSAnimations"] = 1;
  result["htmlview.postMessage"] = 1;
#ifdef ENABLE_HEADERS2
  result["htmlview.headers2"] = 1;
#endif
  result["allowSelfSignedWithIPAddress"] = 1;
  result["connection.supportsSecure"] = 1;
#if defined(USE_AVE)
  result["video.aveLiveFetchHoldTime"] = 1;
#endif
#ifdef ENABLE_MTHEORY_BROWSER
  result["htmlview.mtheory"] = 1;
#endif
  result["keySource"] = 1;

  auto properties = getDeviceProperties();
  auto it = properties.find("OPEN_BROWSING");
  if (it != properties.end()) {
    if (it->second == "0" || it->second == "false") {
      result["openBrowsing"] = 0;
    }
  }

  auto pluginsList = getAvailablePlugins();
  for (auto &it: pluginsList) {
    result[it.first] = it.second;
  }

#ifdef HAS_HEVC4_SUPPORT
  result["hls-hevc-4k"] = 1;
#endif
#ifdef HAS_HEVC1_SUPPORT
  result["hls-hevc-1k"] = 1;
#endif
#ifdef BUILD_RNE
  result["rdk_rne"] = 1;
#endif

  it = properties.find("UHD_4K_DECODE");
  if (it != properties.end()) {
    result["uhd_4k_decode"] = atoi(it->second.c_str());
  } else {
    result["uhd_4k_decode"] = 0;
  }

  return result;
}

/**
 * mimeTypes
 */

std::list<string> PlatformCapsData::GetMimeTypes() const {
  std::list<string> result;

  // TODO

  return result;
}

/**
 * see generic/src/plugins/xrepluginmanager.cpp
 */

std::map<string, uint8_t> PlatformCapsData::getAvailablePlugins() const {
  std::map <string, uint8_t> result;
  std::list <string> removePlugins;
  JsonArray plugins, devicePlugins;
  Core::File file;

  file = OVERRIDE_PLUGIN_CONFIG_FILE;
  if (!file.Open(true))
    file = DEFAULT_PLUGIN_CONFIG_FILE;

  if (file.IsOpen() || file.Open(true)) {
    JsonObject json;
    json.IElement::FromFile(file);
    plugins = json["pluginregistry"].Array();
    file.Close();

    TRACE(Trace::Information,
          (_T("%s Using plugin configuration file: %s\n"),
              __FILE__, file.Name().c_str()
          ));
  } else {
    TRACE(Trace::Information,
          (_T("%s No plugin configuration file found\n"),
              __FILE__
          ));
  }

  file = DEVICE_PLUGIN_CONFIG_FILE;

  if (file.Open(true)) {
    JsonObject json;
    json.IElement::FromFile(file);
    devicePlugins = json["pluginregistry"].Array();
    file.Close();

    TRACE(Trace::Information,
          (_T("%s Using device plugin configuration file: %s\n"),
              __FILE__, file.Name().c_str()
          ));
  } else {
    TRACE(Trace::Information,
          (_T("%s No device plugin configuration file found\n"),
              __FILE__
          ));
  }

  TRACE(Trace::Information,
        (_T("%s device plugin count: %d\n"),
            __FILE__, devicePlugins.Length()
        ));

  auto index(devicePlugins.Elements());

  while (index.Next() == true) {
    auto hash = index.Current().Object();

    if (hash.HasLabel("appName") &&
        hash.HasLabel("enabled") &&
        hash["enabled"] == "false") {
      removePlugins.emplace_back(hash["appName"].String());
    } else if (hash.HasLabel("appName") &&
               hash.HasLabel("libraries") &&
               hash.HasLabel("version") &&
               !(hash.HasLabel("enabled") &&
                 hash["enabled"] == "false") &&
               verifyLibraries(hash["libraries"].Array())) {
      result.emplace(hash["appName"].String(),
                     atoi(hash["version"].String().c_str()));
    }
  }

  TRACE(Trace::Information,
        (_T("%s plugin count: %d\n"),
            __FILE__, plugins.Length()
        ));

  index = plugins.Elements();

  while (index.Next() == true) {
    auto hash = index.Current().Object();

    if (hash.HasLabel("appName") &&
        hash.HasLabel("libraries") &&
        hash.HasLabel("version") &&
        !(hash.HasLabel("enabled") &&
          hash["enabled"] == "false")) {
      auto appName = hash["appName"].String();

      if (result.find(appName) != result.end()) {
        TRACE(Trace::Information,
              (_T("%s the plugin with name %s "
                  "was already added under the device configuration.  "
                  "the generic conf will be ignored\n"),
                  __FILE__, appName.c_str()
              ));
      } else if (std::find(removePlugins.begin(),
                           removePlugins.end(), appName) != removePlugins.end()) {
        TRACE(Trace::Information,
              (_T("%s the plugin with name %s should not be added.\n"),
                  __FILE__, appName.c_str()
              ));
      } else if (verifyLibraries(hash["libraries"].Array())) {
        result.emplace(appName,
                       atoi(hash["version"].String().c_str()));
      }
    }
  }

  return result;
}

bool PlatformCapsData::verifyLibraries(const JsonArray &libraries) const {
  bool verified = true;

  auto index(libraries.Elements());

  while (index.Next() == true) {
    auto lib = index.Current().String();

    if (lib[0] != '/')
      lib = string(PLUGIN_LIBRARY_PATH) + "/" + lib;

    Core::File pluginLib(lib);
    if (!pluginLib.Exists()) {
      verified = false;
      TRACE(Trace::Information,
            (_T("%s Failed to find device library %s\n"),
                __FILE__, lib.c_str()
            ));
      break;
    }
  }

  return verified;
}

} // namespace Plugin
} // namespace WPEFramework
