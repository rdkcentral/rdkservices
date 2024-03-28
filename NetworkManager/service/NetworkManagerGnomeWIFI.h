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
#include "NetworkManagerLogger.h"
#include "INetworkManager.h"
#include <iostream>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

namespace WPEFramework
{
    namespace Plugin
    {
        class wifiManager
        {
        public:
            static wifiManager* getInstance()
            {
                static wifiManager instance;
                return &instance;
            }

            bool isWifiConnected();
            bool wifiDisconnect();
            bool wifiConnectedSSIDInfo(Exchange::INetworkManager::WiFiSSIDInfo &ssidinfo);
            bool wifiConnect(Exchange::INetworkManager::WiFiConnectTo wifiData);
            bool quit(NMDevice *wifiNMDevice);
            bool wait(GMainLoop *loop);
        private:
            NMDevice *getNmDevice();

        private:
            wifiManager() : client(nullptr), loop(nullptr), createNewConnection(false) {
                loop = g_main_loop_new(NULL, FALSE);
            }
            ~wifiManager() {
                NMLOG_TRACE("~wifiManager");
                if(client != nullptr)
                    g_object_unref(client);
                if (loop != NULL) {
                    g_main_loop_unref(loop);
                    loop = NULL;
                }
            }

            wifiManager(wifiManager const&) = delete;
            void operator=(wifiManager const&) = delete;

            bool createClientNewConnection();

        public:
            NMClient *client;
            GMainLoop *loop;
            gboolean createNewConnection;
            const char* objectPath;
            NMDevice *wifidevice;
            guint wifiDeviceStateGsignal = 0;
        };
    }
}
