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

namespace WPEFramework
{
    namespace Plugin
    {

    typedef struct {
        NMClient *client;
        GMainLoop *loop;
        NMDevice *device;
        NMDeviceWifi *wifiDevice;
        NMActiveConnection *activeConn;
        std::string ifnameWlan0;
        std::string ifnameEth0;
    } NMEvents;

    class GnomeNetworkManagerEvents
    {

    public:
        static void onInterfaceStateChangeCb(uint8_t newState, std::string iface); // ReportInterfaceStateChangedEvent
        static void onAddressChangeCb(std::string iface, std::string ipAddress, bool acqired, bool isIPv6); // ReportIPAddressChangedEvent
        static void onActiveInterfaceChangeCb(std::string newInterface); // ReportActiveInterfaceChangedEvent
        static void onAvailableSSIDsCb(NMDeviceWifi *wifiDevice, GParamSpec *pspec, gpointer userData); // ReportAvailableSSIDsEvent
        static void onWIFIStateChanged(uint8_t state); // ReportWiFiStateChangedEvent

    public:
        GnomeNetworkManagerEvents();
        ~GnomeNetworkManagerEvents();
        static void* networkMangerEventMonitor(void *arg);
        bool startNetworkMangerEventMonitor();
        void stopNetworkMangerEventMonitor();
        void setwifiScanOptions(bool print);

    private:
        std::atomic<bool> isEventThrdActive{false};
        NMEvents nmEvents;
        GThread *eventThrdID;
        std::atomic<bool>printSSIDList = {true}; // set to default print for debug
    };

    }   // Plugin
}   // WPEFramework
