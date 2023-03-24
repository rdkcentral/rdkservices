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

#include "WifiManagerState.h"
#include "UtilsJsonRpc.h"
#include "../WifiManager.h" // Need access to WifiManager::getInstance so can't use 'WifiManagerInterface.h'

using namespace WPEFramework::Plugin;
using namespace WifiManagerImpl;

WifiManagerState::WifiManagerState()
{
}

void WifiManagerState::Initialize()
{
   DBusClient &dbus = DBusClient::getInstance();
   if (getWifiInterfaceName().empty())
   {
      LOGWARN("No 'wifi' interface found");
      // TODO: throw an exception?
   }
   else
   {
      // register for status updates
      dbus.registerStatusChanged(std::bind(&WifiManagerState::statusChanged, this, std::placeholders::_1, std::placeholders::_2));
      // get current wifi status
      InterfaceStatus status;
      const std::string& iname = getWifiInterfaceName();
      if (dbus.networkconfig1_GetStatus(iname, status))
      {
         updateWifiStatus(status);
      }
      else
      {
         LOGWARN("failed to get interface '%s' status", iname.c_str());
      }
   }
}

WifiManagerState::~WifiManagerState()
{
}

namespace
{
   /*
   `0`: UNINSTALLED - The device was in an installed state and was uninstalled; or, the device does not have a Wifi radio installed
   `1`: DISABLED - The device is installed but not yet enabled
   `2`: DISCONNECTED - The device is installed and enabled, but not yet connected to a network
   `3`: PAIRING - The device is in the process of pairing, but not yet connected to a network
   `4`: CONNECTING - The device is attempting to connect to a network
   `5`: CONNECTED - The device is successfully connected to a network
   */
   // for the moment, only state we need is 'CONNECTED' (the only state that Amazon app needs)
   const std::map<InterfaceStatus, WifiState> statusToState{
       {Disabled, WifiState::DISABLED},
       {Disconnected, WifiState::DISCONNECTED},
       {Associating, WifiState::CONNECTING},
       {Dormant, WifiState::DISCONNECTED},
       {Binding, WifiState::CONNECTING},
       {Assigned, WifiState::CONNECTED},
       {Scanning, WifiState::CONNECTING}};
}

uint32_t WifiManagerState::getCurrentState(const JsonObject &parameters, JsonObject &response)
{
   // this is used by Amazon, but only 'state' is used by Amazon app and needs to be provided; the rest is not important
   LOGINFOMETHOD();
   response["state"] = static_cast<int>(m_wifi_state.load());
   returnResponse(true);
}

static bool extractSsid(const std::string &netid, std::string &out_ssid)
{
   size_t pos = netid.find(":");
   if (pos != std::string::npos)
   {
      out_ssid = netid.substr(pos + 1);
      return true;
   }
   else
   {
      return false;
   }
}

bool WifiManagerState::fetchSsid(std::string &out_ssid)
{
   const std::string &wifiInterface = getWifiInterfaceName();
   bool ret = false;
   if (!wifiInterface.empty())
   {
      std::string netid;
      if (DBusClient::getInstance().networkconfig1_GetParam(wifiInterface, "netid", netid))
      {
         std::string ssid;
         if (extractSsid(netid, ssid))
         {
            out_ssid = ssid;
            if (ssid != m_latest_ssid)
            {
               WifiManager::getInstance().onSSIDsChanged();
               m_latest_ssid = ssid;
            }
            ret = true;
         }
         else
         {
            LOGWARN("failed to parse ssid from netid: %s", netid.c_str());
         }
      }
   }
   return ret;
}

uint32_t WifiManagerState::getConnectedSSID(const JsonObject &parameters, JsonObject &response)
{
   // this is used by Amazon, but only 'ssid' is used by Amazon app and needs to be returned; the rest is not important
   LOGINFOMETHOD();
   std::string ssid;
   if (fetchSsid(ssid))
   {
      // only 'ssid' is used by Amazon app and needs to be returned; the rest can be empty for now
      response["ssid"] = ssid;
      response["bssid"] = string("");
      response["rate"] = string("");
      response["noise"] = string("");
      response["security"] = string("");
      response["signalStrength"] = string("");
      response["frequency"] = string("");
      returnResponse(true);
   }
   else
   {
      returnResponse(false);
   }
}

void WifiManagerState::statusChanged(const std::string &interface, InterfaceStatus status)
{
   if (interface == getWifiInterfaceName())
   {
      updateWifiStatus(status);
   }
}

void WifiManagerState::updateWifiStatus(WifiManagerImpl::InterfaceStatus status)
{
   bool state_changed = false;
   auto lookup = statusToState.find(status);

   if (lookup != statusToState.end())
   {
      if (lookup->second != m_wifi_state)
      {
         m_wifi_state = lookup->second;
         state_changed = true;
      }
   }
   else
   {
      LOGWARN("unknown status: %d", status);
   }

   if (state_changed)
   {
      if (m_wifi_state == WifiState::CONNECTED)
      {
         static std::string _;
         // fetchSsid will raise onSSIDsChanged event in case ssid was changed
         fetchSsid(_);
      }
      // Hardcode 'isLNF' for the moment (at the moment, the same is done in default rdk implementation)
      WifiManager::getInstance().onWIFIStateChanged(m_wifi_state, false);
   }
}

uint32_t WifiManagerState::setEnabled(const JsonObject &parameters, JsonObject &response)
{
   LOGINFOMETHOD();
   returnResponse(false);
}

uint32_t WifiManagerState::getSupportedSecurityModes(const JsonObject &parameters, JsonObject &response)
{
   LOGINFOMETHOD();
   returnResponse(false);
}

std::string WifiManagerState::fetchWifiInterfaceName()
{
   DBusClient &dbus = DBusClient::getInstance();
   std::vector<std::string> interfaces;
   if (dbus.networkconfig1_GetInterfaces(interfaces))
   {
      for (auto &intf : interfaces)
      {
         std::string type;
         if (dbus.networkconfig1_GetParam(intf, "type", type) && type == "wifi")
         {
            return intf;
         }
      }
   }
   else
   {
      LOGWARN("failed to fetch interfaces via networkconfig1_GetInterfaces");
   }
   return "";
}

const std::string &WifiManagerState::getWifiInterfaceName()
{
   static std::string name = WifiManagerState::fetchWifiInterfaceName();
   return name;
}