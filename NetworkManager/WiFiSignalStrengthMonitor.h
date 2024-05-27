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

#include <cstdio>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "Module.h"
#include "NetworkManagerLogger.h"

#define DEFAULT_WIFI_SIGNAL_TEST_INTERVAL_SEC       60
namespace WPEFramework
{
    namespace Plugin
    {
        class WiFiSignalStrengthMonitor
        {
            public:
                WiFiSignalStrengthMonitor():isRunning(false) {}
                ~WiFiSignalStrengthMonitor(){ NMLOG_INFO("~WiFiSignalStrengthMonitor"); }
                void startWiFiSignalStrengthMonitor(int interval);
                void getSignalData(std::string &ssid, Exchange::INetworkManager::WiFiSignalQuality &quality, std::string &strengthOut);
            private:
                std::string retrieveValues(const char *command, const char* keyword, char *output_buffer, size_t output_buffer_size);
                std::thread monitorThread;
                std::atomic<bool> stopThread;
                std::atomic<bool> isRunning;
                void monitorThreadFunction(int interval);
        };
    }
}
