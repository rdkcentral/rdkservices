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

#include "Module.h"

namespace WPEFramework {

    namespace Plugin {

        enum class SecurityMode{
            NONE,
            WEP_64,
            WEP_128,
            WPA_PSK_TKIP,
            WPA_PSK_AES,
            WPA2_PSK_TKIP,
            WPA2_PSK_AES,
            WPA_ENTERPRISE_TKIP,
            WPA_ENTERPRISE_AES,
            WPA2_ENTERPRISE_TKIP,
            WPA2_ENTERPRISE_AES,
            WPA_WPA2_PSK,
            WPA_WPA2_ENTERPRISE,
            WPA3_PSK_AES,
            WPA3_SAE,
            NOT_SUPPORTED=99
        };

        /**
         * \brief The state of the wifi.
         *
         * Has to match WifiStatusCode_t.
         */
        enum class WifiState {
            UNINSTALLED,
            DISABLED,
            DISCONNECTED,
            PAIRING,
            CONNECTING,
            CONNECTED,
            FAILED
        };

        /**
         * \brief Error codes for wifi.
         *
         * Has to match WifiErrorCode_t.
         */
        enum class ErrorCode {
            SSID_CHANGED,
            CONNECTION_LOST,
            CONNECTION_FAILED,
            CONNECTION_INTERRUPTED,
            INVALID_CREDENTIALS,
            NO_SSID,
            UNKNOWN
        };

        enum class PairingState {
            UNPAIRED,
            PAIRING_STARTED,
            PAIRED
        };

        enum class PairingType {
            WPS,
            WPA,
            WEP,
            WPA2,
            NONE
        };
    } // namespace Plugin
} // namespace WPEFramework
