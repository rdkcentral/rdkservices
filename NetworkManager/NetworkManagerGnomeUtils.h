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

#pragma once
#include <NetworkManager.h>
#include <libnm/NetworkManager.h>
#include <string.h>
#include <iostream>
#include <atomic>
#include "Module.h"

namespace WPEFramework
{
    namespace Plugin
    {
        class nmUtils
        {

            public:
               static uint32_t GetInterfacesName(string &wifiInterface, string &ethernetInterface);
               static uint8_t wifiSecurityModeFromAp(guint32 flags, guint32 wpaFlags, guint32 rsnFlags);
               static std::string wifiFrequencyFromAp(guint32 apFreq);
               static std::string getSecurityModeString(guint32 flags, guint32 wpaFlags, guint32 rsnFlags);
               static JsonObject apToJsonObject(NMAccessPoint *ap);
               static void printActiveSSIDsOnly(NMDeviceWifi *wifiDevice);
        };

    }
}

/*
    // Set up a timer to check the device state reason every second
    g_timeout_add_seconds(1, check_device_state_reason, NULL);
 */
