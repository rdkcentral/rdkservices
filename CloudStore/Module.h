/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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
#ifndef MODULE_NAME
#define MODULE_NAME Plugin_CloudStore
#endif

#include <plugins/plugins.h>
#if (((THUNDER_VERSION_MAJOR >= 4) && (THUNDER_VERSION_MINOR == 4)) || ((THUNDER_VERSION >= 4) && !defined(THUNDER_VERSION_MINOR)))
#include <definitions/definitions.h>
#else
#include <interfaces/definitions.h>
#endif

#define URI_ENV "CLOUDSTORE_URI"
#define IARM_INIT_NAME "Thunder_Plugins"
#define URI_RFC "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.CloudStore.Uri"
#define PARTNER_ID_FILENAME "/opt/www/authService/partnerId3.dat"
#define ACCOUNT_ID_FILENAME "/opt/www/authService/said.dat"
#define DEVICE_ID_FILENAME "/opt/www/authService/xdeviceid.dat"
#define SECURITY_AGENT_FILENAME "/tmp/SecurityAgent/token"
#define IARM_TIMEOUT 1000
#define COM_RPC_TIMEOUT 1000
#define JSON_RPC_TIMEOUT 2000
#define GRPC_TIMEOUT 3000

#undef EXTERNAL
#define EXTERNAL
