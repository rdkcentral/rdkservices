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

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <thread>
#include <string>
#include <map>
#include <NetworkManager.h>
#include <libnm/NetworkManager.h>
#include "Module.h"
#include "NetworkManagerGnomeEvents.h"
#include "NetworkManagerLogger.h"
#include "NetworkManagerGnomeUtils.h"
#include "NetworkManagerImplementation.h"
#include "INetworkManager.h"

namespace WPEFramework
{
    namespace Plugin
    {

    extern NetworkManagerImplementation* _instance;
    static GnomeNetworkManagerEvents *_nmEventInstance = nullptr;

    // const char* ifnameEth = "enx207bd51e02ad";
    // const char* ifnameWlan = "wlp0s20f3";

    static void primaryConnectionCb(NMClient *client, GParamSpec *param, NMEvents *nmEvents)
    {
        NMActiveConnection *primaryConn;
        const char *activeConnId = NULL;
        const char *connectionTyp = NULL;
        primaryConn = nm_client_get_primary_connection(client);
        nmEvents->activeConn = primaryConn;
        if (primaryConn)
        {
            activeConnId = nm_active_connection_get_id(primaryConn);
            connectionTyp = nm_active_connection_get_connection_type(primaryConn);
            NMLOG_INFO("active connection - %s (%s)", activeConnId, connectionTyp);
            std::string newIface ="";

            if (0 == strncmp("802-3-ethernet", connectionTyp, sizeof("802-3-ethernet")))
                newIface = "eth0";
            else if(0 == strncmp("802-11-wireless", connectionTyp, sizeof("802-11-wireless")))
                newIface = "wlan0";
            else
                NMLOG_WARNING("active connection not an ethernet/wifi %s", connectionTyp);

            GnomeNetworkManagerEvents::onActiveInterfaceChangeCb(newIface);
        }
        else
            NMLOG_ERROR("now there's no active connection");
    }

    static void deviceStateChangeCb(NMDevice *device, GParamSpec *pspec, NMEvents *nmEvents)
    {
        NMDeviceState deviceState;
        deviceState = nm_device_get_state(device);
        std::string ifname = nm_device_get_iface(device);
        if(ifname == nmEvents->ifnameWlan0)
        {
            if(!NM_IS_DEVICE_WIFI(device)) {
                NMLOG_FATAL("not a wifi device !");
                return;
            }
            NMDeviceStateReason reason = nm_device_get_state_reason(device);
            std::string wifiState;
           // NMLOG_TRACE("\033[0;31m NMDeviceStateReason %d \033[0m",reason);
            switch (reason)
            {
                case NM_DEVICE_STATE_REASON_SUPPLICANT_AVAILABLE:
                    wifiState = "WIFI_STATE_UNINSTALLED";
                    GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_UNINSTALLED);
                    break;
                case NM_DEVICE_STATE_REASON_SSID_NOT_FOUND:
                    wifiState = "WIFI_STATE_SSID_NOT_FOUND";
                    GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_SSID_NOT_FOUND);
                    break;
                case NM_DEVICE_STATE_REASON_SUPPLICANT_TIMEOUT:         // supplicant took too long to authenticate
                case NM_DEVICE_STATE_REASON_NO_SECRETS:
                    wifiState = "WIFI_STATE_AUTHENTICATION_FAILED";
                    GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_AUTHENTICATION_FAILED);
                    break;
                case NM_DEVICE_STATE_REASON_SUPPLICANT_FAILED:          //  802.1x supplicant failed
                    wifiState = "WIFI_STATE_ERROR";
                    GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_ERROR);
                    break;
                case NM_DEVICE_STATE_REASON_SUPPLICANT_CONFIG_FAILED:   // 802.1x supplicant configuration failed
                    wifiState = "WIFI_STATE_CONNECTION_INTERRUPTED";
                    GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_CONNECTION_INTERRUPTED);
                    break;
                case NM_DEVICE_STATE_REASON_SUPPLICANT_DISCONNECT:      // 802.1x supplicant disconnected
                    wifiState = "WIFI_STATE_INVALID_CREDENTIALS";
                    GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_INVALID_CREDENTIALS);
                    break;
                default:
                {
                    switch (deviceState)
                    {
                    case NM_DEVICE_STATE_UNKNOWN:
                        wifiState = "WIFI_STATE_UNINSTALLED";
                        GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_UNINSTALLED);
                        break;
                    case NM_DEVICE_STATE_UNMANAGED:
                        wifiState = "WIFI_STATE_DISABLED";
                        GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_DISABLED);
                        break;
                    case NM_DEVICE_STATE_UNAVAILABLE:
                    case NM_DEVICE_STATE_DISCONNECTED:
                        wifiState = "WIFI_STATE_DISCONNECTED";
                        GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_DISCONNECTED);
                        break;
                    case NM_DEVICE_STATE_PREPARE:
                        wifiState = "WIFI_STATE_PAIRING";
                        GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_PAIRING);
                        break;
                    case NM_DEVICE_STATE_CONFIG:
                        wifiState = "WIFI_STATE_CONNECTING";
                        GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_CONNECTING);
                        break;
                    case NM_DEVICE_STATE_IP_CONFIG:
                        GnomeNetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_ACQUIRING_IP,"wlan0");
                        break;
                    case NM_DEVICE_STATE_ACTIVATED:
                        wifiState = "WIFI_STATE_CONNECTED";
                        GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_CONNECTED);
                        break;
                    case NM_DEVICE_STATE_DEACTIVATING:
                        wifiState = "WIFI_STATE_CONNECTION_LOST";
                        GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_CONNECTION_LOST);
                        break;
                    case NM_DEVICE_STATE_FAILED:
                        wifiState = "WIFI_STATE_CONNECTION_FAILED";
                        GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_CONNECTION_FAILED);
                        break;
                    case NM_DEVICE_STATE_NEED_AUTH:
                        //GnomeNetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_CONNECTION_INTERRUPTED);
                        //wifiState = "WIFI_STATE_CONNECTION_INTERRUPTED";
                        break;
                    default:
                        wifiState = "Un handiled";
                    }
                }
            }
            NMLOG_TRACE("wifi state: %s", wifiState.c_str());
        }
        else if(ifname == nmEvents->ifnameEth0)
        {
            switch (deviceState)
            {
                case NM_DEVICE_STATE_UNKNOWN:
                case NM_DEVICE_STATE_UNMANAGED:
                    GnomeNetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_DISABLED, "eth0");
                break;
                case NM_DEVICE_STATE_UNAVAILABLE:
                case NM_DEVICE_STATE_DISCONNECTED:
                    GnomeNetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_LINK_DOWN, "eth0");
                break;
                case NM_DEVICE_STATE_PREPARE:
                    GnomeNetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_LINK_UP, "eth0");
                break;
                case NM_DEVICE_STATE_IP_CONFIG:
                    GnomeNetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_ACQUIRING_IP,"eth0");
                case NM_DEVICE_STATE_NEED_AUTH:
                case NM_DEVICE_STATE_SECONDARIES:
                case NM_DEVICE_STATE_ACTIVATED:
                case NM_DEVICE_STATE_DEACTIVATING:
                default:
                    NMLOG_WARNING("Unhandiled state change");
            }
        }

        NMLOG_TRACE("%s state: (%d)", ifname.c_str(), deviceState);

    }

    static void ip4ChangedCb(NMIPConfig *ipConfig, GParamSpec *pspec, gpointer userData)
    {
        if (!ipConfig) {
            NMLOG_ERROR("IP config is null");
            return;
        }

        NMDevice *device = (NMDevice*)userData;
        if((device == NULL) || (!NM_IS_DEVICE(device)))
        return;

        const char* iface = nm_device_get_iface(device);
        if(iface == NULL)
            return;
        std::string ifname = iface;

        GPtrArray *addresses = nm_ip_config_get_addresses(ipConfig);
        if (!addresses) {
            NMLOG_ERROR("No addresses found");
            return;
        }
        else {
            if(addresses->len == 0) {
                GnomeNetworkManagerEvents::onAddressChangeCb(ifname, "", false, false);
                return;
            }
        }

        for (guint i = 0; i < addresses->len; ++i) {
            NMIPAddress *address = (NMIPAddress *)g_ptr_array_index(addresses, i);
            if (nm_ip_address_get_family(address) == AF_INET) {
                const char *ipAddress = nm_ip_address_get_address(address);
                if(ipAddress != NULL)
                    GnomeNetworkManagerEvents::onAddressChangeCb(iface, ipAddress, true, false);
            }
        }
    }

    static void ip6ChangedCb(NMIPConfig *ipConfig, GParamSpec *pspec, gpointer userData)
    {
        if (!ipConfig) {
            NMLOG_ERROR("ip config is null");
            return;
        }

        NMDevice *device = (NMDevice*)userData;
        if( ((device != NULL) && NM_IS_DEVICE(device)) )
        {
            const char* iface = nm_device_get_iface(device);
            if(iface == NULL)
                return;
            std::string ifname = iface;
            GPtrArray *addresses = nm_ip_config_get_addresses(ipConfig);
            if (!addresses) {
                NMLOG_ERROR("No addresses found");
                return;
            }
            else {
                if(addresses->len == 0) {
                    GnomeNetworkManagerEvents::onAddressChangeCb(ifname, "", false, true);
                    return;
                }
            }

            for (guint i = 0; i < addresses->len; ++i) {
                NMIPAddress *address = (NMIPAddress *)g_ptr_array_index(addresses, i);
                if (nm_ip_address_get_family(address) == AF_INET6) {
                    const char *ipAddress = nm_ip_address_get_address(address);
                    //int prefix = nm_ip_address_get_prefix(address);
                    if(ipAddress != NULL) {
                        GnomeNetworkManagerEvents::onAddressChangeCb(iface, ipAddress, true, true);
                    }
                }
            }
        }
    }

    static void deviceAddedCB(NMClient *client, NMDevice *device, NMEvents *nmEvents)
    {
        if( ((device != NULL) && NM_IS_DEVICE(device)) )
        {
            std::string ifname = nm_device_get_iface(device);
            if(ifname == nmEvents->ifnameWlan0) {
                GnomeNetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_ADDED, "wlan0");
            }
            else if(ifname == nmEvents->ifnameEth0) {
                GnomeNetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_ADDED, "eth0");
            }
            else {
                // TODO Check and remove if not needed
                GnomeNetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_ADDED, ifname);
            }
            /* ip events added only for eth0 and wlan0 */
            if((ifname == nmEvents->ifnameEth0) || (ifname == nmEvents->ifnameWlan0))
            {
                g_signal_connect(device, "notify::" NM_DEVICE_STATE, G_CALLBACK(deviceStateChangeCb), nmEvents);
                // TODO call notify::" NM_DEVICE_ACTIVE_CONNECTION if needed
                NMIPConfig *ipv4Config = nm_device_get_ip4_config(device);
                NMIPConfig *ipv6Config = nm_device_get_ip6_config(device);
                if (ipv4Config) {
                    g_signal_connect(ipv4Config, "notify::addresses", G_CALLBACK(ip4ChangedCb), device);
                }

                if (ipv6Config) {
                    g_signal_connect(ipv6Config, "notify::addresses", G_CALLBACK(ip6ChangedCb), device);
                }
            }
        }
        else
            NMLOG_TRACE("device error null");
    } 

    static void deviceRemovedCB(NMClient *client, NMDevice *device, NMEvents *nmEvents)
    {
        if( ((device != NULL) && NM_IS_DEVICE(device)) )
        {
            std::string ifname = nm_device_get_iface(device);
            if(ifname == nmEvents->ifnameWlan0) {
                GnomeNetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_REMOVED,"wlan0");
                g_signal_handlers_disconnect_by_func(device, (gpointer)deviceStateChangeCb, nmEvents);
            }
            else if(ifname == nmEvents->ifnameEth0) {
                GnomeNetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_REMOVED, "eth0");
                g_signal_handlers_disconnect_by_func(device, (gpointer)deviceStateChangeCb, nmEvents);
            }
            else {
                // TODO Check and remove if not needed
                GnomeNetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_REMOVED, ifname);
            }
        }

        //     guint disconnected_count = g_signal_handlers_disconnect_matched( _nmEventInstance->activeConn,
        //                                                                     G_SIGNAL_MATCH_FUNC,
        //                                                                     0, 0, NULL,
        //                                                                     (gpointer)onActiveConnectionStateChanged,
        //                                                                     NULL );
        //     NMLOG_ERROR("Disconnected %u signal handlers\n", disconnected_count);

    }

    static void clientStateChangedCb (NMClient *client, GParamSpec *pspec, gpointer user_data)
    {

        switch (nm_client_get_state (client)) {
        case NM_STATE_DISCONNECTED:
            NMLOG_WARNING("internet connection down");
            break;
        case NM_STATE_CONNECTED_GLOBAL:
            NMLOG_TRACE("global internet connection success");
            break;
        default:
            break;
	    }
    }

    static void managerRunningCb (NMClient *client, GParamSpec *pspec, gpointer user_data)
    {
        if (nm_client_get_nm_running (client)) {
            NMLOG_INFO("network manager daemon is running");
        } else {
            NMLOG_FATAL("network manager daemon not running !");
            // TODO  check need any client reconnection or not ?
        }
    }

    void* GnomeNetworkManagerEvents::networkMangerEventMonitor(void *arg)
    {
        if(arg == nullptr)
        {
            NMLOG_FATAL("function argument error: nm event monitor failed");
            return nullptr;
        }

        NMEvents *nmEvents = static_cast<NMEvents *>(arg);
        primaryConnectionCb(nmEvents->client, NULL, nmEvents);
        g_signal_connect (nmEvents->client, "notify::" NM_CLIENT_NM_RUNNING,G_CALLBACK (managerRunningCb), nmEvents);
        g_signal_connect(nmEvents->client, "notify::" NM_CLIENT_STATE, G_CALLBACK (clientStateChangedCb),nmEvents);
        g_signal_connect(nmEvents->client, "notify::" NM_CLIENT_PRIMARY_CONNECTION, G_CALLBACK(primaryConnectionCb), nmEvents);

       const GPtrArray *devices = nullptr;
        devices = nm_client_get_devices(nmEvents->client);

        g_signal_connect(nmEvents->client, NM_CLIENT_DEVICE_ADDED, G_CALLBACK(deviceAddedCB), nmEvents);
        g_signal_connect(nmEvents->client, NM_CLIENT_DEVICE_REMOVED, G_CALLBACK(deviceRemovedCB), nmEvents);

        for (u_int count = 0; count < devices->len; count++)
        {
            NMDevice *device = NM_DEVICE(g_ptr_array_index(devices, count));
            if( ((device != NULL) && NM_IS_DEVICE(device)) )
            {
                g_signal_connect(device, "notify::" NM_DEVICE_STATE, G_CALLBACK(deviceStateChangeCb), nmEvents);
                //g_signal_connect(device, "notify::" NM_DEVICE_ACTIVE_CONNECTION, G_CALLBACK(deviceActiveConnChangeCb), NULL);
                std::string ifname = nm_device_get_iface(device);
                if((ifname == nmEvents->ifnameEth0) || (ifname == nmEvents->ifnameWlan0)) /* ip events added only for eth0 and wlan0 */
                {
                    NMIPConfig *ipv4Config = nm_device_get_ip4_config(device);
                    NMIPConfig *ipv6Config = nm_device_get_ip6_config(device);
                    if (ipv4Config) {
                        g_signal_connect(ipv4Config, "notify::addresses", G_CALLBACK(ip4ChangedCb), device);
                    }

                    if (ipv6Config) {
                        g_signal_connect(ipv6Config, "notify::addresses", G_CALLBACK(ip6ChangedCb), device);
                    }

                    if(NM_IS_DEVICE_WIFI(device)) {
                        nmEvents->wifiDevice = NM_DEVICE_WIFI(device);
                        g_signal_connect(nmEvents->wifiDevice, "notify::" NM_DEVICE_WIFI_LAST_SCAN, G_CALLBACK(GnomeNetworkManagerEvents::onAvailableSSIDsCb), nmEvents);
                    }
                }
                else
                    NMLOG_TRACE("device type not eth/wifi");
            }
        }

        NMLOG_INFO("registered all networkmnager dbus events");
        g_main_loop_run(nmEvents->loop);
        //g_main_loop_unref(nmEvents->loop);
        return nullptr;
    }

    bool GnomeNetworkManagerEvents::startNetworkMangerEventMonitor()
    {
        NMLOG_TRACE("starting gnome event monitor");
        if (NULL == nmEvents.client) {
            NMLOG_ERROR("Client Connection NULL DBUS event Failed!");
            return false;
        }
        if(!isEventThrdActive) {
            isEventThrdActive = true;
            // Create event monitor thread
            eventThrdID = g_thread_new("nm_event_thrd", GnomeNetworkManagerEvents::networkMangerEventMonitor, &nmEvents);
        }
        return true;
    }

    void GnomeNetworkManagerEvents::stopNetworkMangerEventMonitor()
    {
        // g_signal_handlers_disconnect_by_func(client, G_CALLBACK(primaryConnectionCb), NULL);
        if (nmEvents.loop != NULL) {
            g_main_loop_quit(nmEvents.loop);
        }
        if (eventThrdID) {
            g_thread_join(eventThrdID);  // Wait for the thread to finish
            eventThrdID = NULL;  // Reset the thread ID
            NMLOG_WARNING("gnome event monitor stoped");
        }
        isEventThrdActive = false;
    }

    GnomeNetworkManagerEvents::~GnomeNetworkManagerEvents()
    {
        NMLOG_INFO("~GnomeNetworkManagerEvents");
        stopNetworkMangerEventMonitor();
        if(nmEvents.client != nullptr)
            g_object_unref(nmEvents.client);
        if (nmEvents.loop != NULL) {
            g_main_loop_unref(nmEvents.loop);
            nmEvents.loop = NULL;
        }
    }

    GnomeNetworkManagerEvents* GnomeNetworkManagerEvents::getInstance()
    {
        static GnomeNetworkManagerEvents instance;
        return &instance;
    }

    GnomeNetworkManagerEvents::GnomeNetworkManagerEvents()
    {
        NMLOG_TRACE("GnomeNetworkManagerEvents");
        GError *error = NULL;
        nmEvents.client = nm_client_new(NULL, &error);
        if(!nmEvents.client || error )
        {
            if (error) {
                NMLOG_ERROR("Could not connect to NetworkManager: %s", error->message);
                g_error_free(error);
            }
            NMLOG_INFO("networkmanger client connection failed");
            return;
        }

        NMLOG_INFO("libnm client connection success version: %s", nm_client_get_version(nmEvents.client));
        nmEvents.loop = g_main_loop_new(NULL, FALSE);
        if(nmEvents.loop == NULL) {
            NMLOG_FATAL("GMain loop failed Fatal Error: Event will not work");
            return;
        }
        _nmEventInstance = this;
        std::string wifiInterface = "wlan0", ethernetInterface = "eth0";
        nmUtils::GetInterfacesName(wifiInterface, ethernetInterface);
        nmEvents.ifnameEth0 = ethernetInterface;
        nmEvents.ifnameWlan0 = wifiInterface;
    }

    /* Gnome networkmanger new events */

    void GnomeNetworkManagerEvents::onActiveInterfaceChangeCb(std::string newIface)
    {
        static std::string oldIface = "Unknown";

        if(oldIface != newIface)
        {
            if(_instance != nullptr)
                _instance->ReportActiveInterfaceChangedEvent(oldIface, newIface);
            NMLOG_INFO("old interface - %s new interface - %s", oldIface.c_str(), newIface.c_str());
            oldIface = newIface;
        }
    }

    void GnomeNetworkManagerEvents::onInterfaceStateChangeCb(uint8_t newState, std::string iface)
    {
        std::string state = "";
        switch (newState)
        {
            case Exchange::INetworkManager::INTERFACE_ADDED:
                state = "INTERFACE_ADDED";
                break;
            case Exchange::INetworkManager::INTERFACE_LINK_UP:
                state = "INTERFACE_LINK_UP";
                break;
            case Exchange::INetworkManager::INTERFACE_LINK_DOWN:
                state = "INTERFACE_LINK_DOWN";
                break;
            case Exchange::INetworkManager::INTERFACE_ACQUIRING_IP:
                state = "INTERFACE_ACQUIRING_IP";
                break;
            case Exchange::INetworkManager::INTERFACE_REMOVED:
                state = "INTERFACE_REMOVED";
                break;
            case Exchange::INetworkManager::INTERFACE_DISABLED:
                state = "INTERFACE_DISABLED";
                break;
            default:
                state = "Unknown";
        }
        NMLOG_INFO("%s interface state changed - %s", iface.c_str(), state.c_str());
        if(_instance != nullptr)
            _instance->ReportInterfaceStateChangedEvent(static_cast<Exchange::INetworkManager::InterfaceState>(newState), iface);
    }

    void GnomeNetworkManagerEvents::onWIFIStateChanged(uint8_t state)
    {
        if(_instance != nullptr)
            _instance->ReportWiFiStateChangedEvent(static_cast<Exchange::INetworkManager::WiFiState>(state));
    }

    void GnomeNetworkManagerEvents::onAddressChangeCb(std::string iface, std::string ipAddress, bool acqired, bool isIPv6)
    {
        static std::map<std::string, std::string> ipv6Map;
        static std::map<std::string, std::string> ipv4Map;

        if (isIPv6)
        {
            if (ipAddress.empty()) {
                ipAddress = ipv6Map[iface];
                ipv6Map[iface].clear();
            }
            else {
                if (ipv6Map[iface].find(ipAddress) == std::string::npos) { // same ip comes multiple time so avoding that
                    if (!ipv6Map[iface].empty())
                        ipv6Map[iface] += " ";
                    ipv6Map[iface] += ipAddress; // SLAAC protocol may include multip ipv6 address
                }
                else
                    return; // skip same ip event posting
            }
        }
        else
        {
            if (ipAddress.empty())
                ipAddress = ipv4Map[iface];
            else
                ipv4Map[iface] = ipAddress;
        }

        if(_instance != nullptr)
            _instance->ReportIPAddressChangedEvent(iface, acqired, true, ipAddress);
        NMLOG_INFO("iface:%s - ipaddress:%s - %s - isIPv6:%s", iface.c_str(), ipAddress.c_str(), acqired?"acquired":"lost", isIPv6?"true":"false");
    }

    void GnomeNetworkManagerEvents::onAvailableSSIDsCb(NMDeviceWifi *wifiDevice, GParamSpec *pspec, gpointer userData)
    {
        NMLOG_INFO("wifi scanning completed ...");
        if(!NM_IS_DEVICE_WIFI(wifiDevice))
        {
            NMLOG_ERROR("Not a wifi object ");
            return;
        }
        JsonArray ssidList = JsonArray();
        string ssidListJson;
        NMAccessPoint *ap = nullptr;
        const GPtrArray *accessPoints = nm_device_wifi_get_access_points(wifiDevice);
        for (guint i = 0; i < accessPoints->len; i++)
        {
            JsonObject ssidObj;
            ap = static_cast<NMAccessPoint*>(accessPoints->pdata[i]);
            ssidObj = nmUtils::apToJsonObject(ap);
            ssidList.Add(ssidObj);
        }

        ssidList.ToString(ssidListJson);
        if(_nmEventInstance->debugLogs) {
            _nmEventInstance->debugLogs = false;
            NMLOG_TRACE("Number of Access Points Available = %d", static_cast<int>(accessPoints->len));
            NMLOG_TRACE("Scanned APIs are  = %s",ssidListJson.c_str());
        }

        if(!_nmEventInstance->stopWifiScan) {
            _instance->ReportAvailableSSIDsEvent(ssidListJson);
        }
    }

    void GnomeNetworkManagerEvents::setwifiScanOptions(bool doScan, bool enableLogs)
    {
        stopWifiScan.store(doScan);
        if(stopWifiScan)
        {
            NMLOG_WARNING("stop periodic wifi scan result");
        }
        debugLogs = enableLogs;
    }

    }   // Plugin
}   // WPEFramework
