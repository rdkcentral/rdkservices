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
#include <string.h>
#include <iostream>

#include <glib.h>
#include <NetworkManager.h>
#include <libnm/NetworkManager.h>
#include "NetworkManagerLogger.h"
#include "INetworkManager.h"
#include "NetworkManagerGnomeWIFI.h"

namespace WPEFramework
{
    namespace Plugin
    {
        NMDevice* wifiManager::getNmDevice()
        {
            NMDevice *wifiDevice = NULL;

            GPtrArray *devices = const_cast<GPtrArray *>(nm_client_get_devices(client));
            if (devices == NULL) {
                NMLOG_ERROR("Failed to get device list.");
                return wifiDevice;
            }

            for (guint j = 0; j < devices->len; j++) {
                NMDevice *device = NM_DEVICE(devices->pdata[j]);
                if (nm_device_get_device_type(device) == NM_DEVICE_TYPE_WIFI)
                {
                    wifiDevice = device;
                    //NMLOG_TRACE("Wireless Device found ifce : %s !", nm_device_get_iface (wifiDevice));
                    break;
                }
            }

            if (wifiDevice == NULL || !NM_IS_DEVICE_WIFI(wifiDevice))
            {
                NMLOG_ERROR("Wireless Device not found !");
            }

            return wifiDevice;
        }

        /* Convert flags to string */
        static void apFlagsToString(guint32 flags, std::string &flagStr)
        {

            flagStr = "";

            if (flags & NM_802_11_AP_SEC_PAIR_WEP40)
                flagStr += "pair_wpe40 ";
            if (flags & NM_802_11_AP_SEC_PAIR_WEP104)
                flagStr += "pair_wpe104 ";
            if (flags & NM_802_11_AP_SEC_PAIR_TKIP)
                flagStr += "pair_tkip ";
            if (flags & NM_802_11_AP_SEC_PAIR_CCMP)
                flagStr += "pair_ccmp ";
            if (flags & NM_802_11_AP_SEC_GROUP_WEP40)
                flagStr += "group_wpe40 ";
            if (flags & NM_802_11_AP_SEC_GROUP_WEP104)
                flagStr += "group_wpe104 ";
            if (flags & NM_802_11_AP_SEC_GROUP_TKIP)
                flagStr += "group_tkip ";
            if (flags & NM_802_11_AP_SEC_GROUP_CCMP)
                flagStr += "group_ccmp ";
            if (flags & NM_802_11_AP_SEC_KEY_MGMT_PSK)
                flagStr += "psk ";
            if (flags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)
            flagStr += "802.1X ";
            if (flags & NM_802_11_AP_SEC_KEY_MGMT_SAE)
            flagStr += "sae ";
            if (flags & NM_802_11_AP_SEC_KEY_MGMT_OWE)
                flagStr += "owe " ;
            if (flags & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM)
                flagStr += "owe_transition_mode ";
            if (flags & NM_802_11_AP_SEC_KEY_MGMT_EAP_SUITE_B_192)
                flagStr += "wpa-eap-suite-b-192 ";

            if (flagStr.size() <= 0)
                flagStr = "none";
        }

        bool static getConnectedSSID(NMDeviceWifi *wifiDevice, std::string& ssidin)
        {
            GBytes *ssid;
            NMAccessPoint *activeAP = nm_device_wifi_get_active_access_point(wifiDevice);
            if(activeAP == NULL) {
                return false;
            }

            ssid = nm_access_point_get_ssid(activeAP);
            gsize size;
            const guint8 *ssidData = static_cast<const guint8 *>(g_bytes_get_data(ssid, &size));
            std::string ssidTmp(reinterpret_cast<const char *>(ssidData), size);
            ssidin = ssidTmp;
            NMLOG_INFO("connected ssid: %s", ssidin.c_str());
            return true;
        }

        static void getApInfo(NMAccessPoint *AccessPoint, Exchange::INetworkManager::WiFiSSIDInfo &wifiInfo)
        {
            guint32     flags, wpaFlags, rsnFlags, freq, bitrate;
            guint8      strength;
            GBytes     *ssid;
            const char *hwaddr;
            NM80211Mode mode;
            /* Get AP properties */
            flags     = nm_access_point_get_flags(AccessPoint);
            wpaFlags = nm_access_point_get_wpa_flags(AccessPoint);
            rsnFlags = nm_access_point_get_rsn_flags(AccessPoint);
            ssid      = nm_access_point_get_ssid(AccessPoint);
            hwaddr    = nm_access_point_get_bssid(AccessPoint);
            freq      = nm_access_point_get_frequency(AccessPoint);
            mode      = nm_access_point_get_mode(AccessPoint);
            bitrate   = nm_access_point_get_max_bitrate(AccessPoint);
            strength  = nm_access_point_get_strength(AccessPoint);

            switch(flags)
            {
                case NM_802_11_AP_FLAGS_NONE:
                    NMLOG_INFO("ap type : point has no special capabilities");
                    break;
                case NM_802_11_AP_FLAGS_PRIVACY:
                    NMLOG_INFO("ap type : access point requires authentication and encryption");
                    break;
                case NM_802_11_AP_FLAGS_WPS:
                    NMLOG_INFO("ap type : access point supports some WPS method");
                    break;
                case NM_802_11_AP_FLAGS_WPS_PBC:
                    NMLOG_INFO("ap type : access point supports push-button WPS");
                    break;
                case NM_802_11_AP_FLAGS_WPS_PIN:
                    NMLOG_INFO("ap type : access point supports PIN-based WPS");
                    break;
                default:
                    NMLOG_ERROR("ap type : 802.11 flags unknown!");
            }

            /* Convert to strings */
            if (ssid) {
                gsize size;
                const guint8 *ssidData = static_cast<const guint8 *>(g_bytes_get_data(ssid, &size));
                std::string ssidTmp(reinterpret_cast<const char *>(ssidData), size);
                wifiInfo.m_ssid = ssidTmp;
                NMLOG_INFO("ssid: %s", wifiInfo.m_ssid.c_str());
            }
            else
            {
            wifiInfo.m_ssid = "-----";
            NMLOG_TRACE("ssid: %s", wifiInfo.m_ssid.c_str());
            }

            wifiInfo.m_bssid = (hwaddr != nullptr) ? hwaddr : "-----";
            NMLOG_INFO("bssid: %s", wifiInfo.m_bssid.c_str());


            if (freq >= 2400 && freq < 5000) {
                wifiInfo.m_frequency = Exchange::INetworkManager::WiFiFrequency::WIFI_FREQUENCY_2_4_GHZ;
                NMLOG_INFO("freq: WIFI_FREQUENCY_2_4_GHZ");
            }
            else if (freq >= 5000 && freq < 6000) {
                wifiInfo.m_frequency =  Exchange::INetworkManager::WiFiFrequency::WIFI_FREQUENCY_5_GHZ;
                NMLOG_INFO("freq: WIFI_FREQUENCY_5_GHZ");
            }
            else if (freq >= 6000) {
                wifiInfo.m_frequency = Exchange::INetworkManager::WiFiFrequency::WIFI_FREQUENCY_6_GHZ;
                NMLOG_INFO("freq: WIFI_FREQUENCY_6_GHZ");
            }
            else {
                wifiInfo.m_frequency = Exchange::INetworkManager::WiFiFrequency::WIFI_FREQUENCY_WHATEVER;
                NMLOG_INFO("freq: No available !");
            }

            wifiInfo.m_rate = std::to_string(bitrate);
            NMLOG_INFO("bitrate : %s kbit/s", wifiInfo.m_rate.c_str());

            wifiInfo.m_signalStrength = std::to_string(static_cast<u_int8_t>(strength));
            NMLOG_INFO("sterngth: %s %%", wifiInfo.m_signalStrength.c_str());
            //TODO signal strenght to dBm

            std::string security_str = "";
            if ((flags == NM_802_11_AP_FLAGS_NONE) && (wpaFlags == NM_802_11_AP_SEC_NONE) && (rsnFlags == NM_802_11_AP_SEC_NONE))
            {
                wifiInfo.m_securityMode = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_NONE;
            }
            else if( (flags & NM_802_11_AP_FLAGS_PRIVACY) && ((wpaFlags & NM_802_11_AP_SEC_PAIR_WEP40) || (rsnFlags & NM_802_11_AP_SEC_PAIR_WEP40)) )
            {
                wifiInfo.m_securityMode = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WEP_64;
            }
            else if( (flags & NM_802_11_AP_FLAGS_PRIVACY) && ((wpaFlags & NM_802_11_AP_SEC_PAIR_WEP104) || (rsnFlags & NM_802_11_AP_SEC_PAIR_WEP104)) )
            {
                wifiInfo.m_securityMode = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WEP_128;
            }
            else if((wpaFlags & NM_802_11_AP_SEC_PAIR_TKIP) || (rsnFlags & NM_802_11_AP_SEC_PAIR_TKIP))
            {
                wifiInfo.m_securityMode = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_PSK_TKIP;
            }
            else if((wpaFlags & NM_802_11_AP_SEC_PAIR_CCMP) || (rsnFlags & NM_802_11_AP_SEC_PAIR_CCMP))
            {
                wifiInfo.m_securityMode = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_PSK_AES;
            }
            else if ((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_PSK) && (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X))
            {
                wifiInfo.m_securityMode = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_WPA2_ENTERPRISE;
            }
            else if(rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_PSK)
            {
                wifiInfo.m_securityMode = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_WPA2_PSK;
            }
            else if((wpaFlags & NM_802_11_AP_SEC_GROUP_CCMP) || (rsnFlags & NM_802_11_AP_SEC_GROUP_CCMP))
            {
                wifiInfo.m_securityMode = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA2_PSK_AES;
            }
            else if((wpaFlags & NM_802_11_AP_SEC_GROUP_TKIP) || (rsnFlags & NM_802_11_AP_SEC_GROUP_TKIP))
            {
                wifiInfo.m_securityMode = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA2_PSK_TKIP;
            }
            else
            {
                NMLOG_WARNING("security mode not defined");
            }

            if (!(flags & NM_802_11_AP_FLAGS_PRIVACY) && (wpaFlags != NM_802_11_AP_SEC_NONE) && (rsnFlags != NM_802_11_AP_SEC_NONE))
                security_str += ("Encrypted: ");

            if ((flags & NM_802_11_AP_FLAGS_PRIVACY) && (wpaFlags == NM_802_11_AP_SEC_NONE)
                && (rsnFlags == NM_802_11_AP_SEC_NONE))
                security_str += ("WEP ");
            if (wpaFlags != NM_802_11_AP_SEC_NONE)
                security_str += ("WPA ");
            if ((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_PSK)
                || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
                security_str += ("WPA2 ");
            }
            if (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_SAE) {
                security_str += ("WPA3 ");
            }
            if ((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE)
                || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM)) {
                security_str += ("OWE ");
            }
            if ((wpaFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)
                || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
                security_str += ("802.1X ");
            }

            NMLOG_INFO("security: %s", (security_str.size() > 0)? security_str.c_str(): "none");
            std::string flagStr;
            apFlagsToString(wpaFlags, flagStr);
            apFlagsToString(rsnFlags, flagStr);
            NMLOG_INFO("WPA flags: %s", flagStr.c_str());
            NMLOG_INFO("RSN flags: %s", flagStr.c_str());
            NMLOG_TRACE("D-Bus path: %s", nm_object_get_path(NM_OBJECT(AccessPoint)));
            NMLOG_INFO("Mode: %s", mode == NM_802_11_MODE_ADHOC   ? "Ad-Hoc": mode == NM_802_11_MODE_INFRA ? "Infrastructure": "Unknown");
        }

        bool wifiManager::isWifiConnected()
        {
            if(!createClientNewConnection())
                return false;

            NMDeviceWifi *wifiDevice = NM_DEVICE_WIFI(getNmDevice());
            if(wifiDevice == NULL) {
                NMLOG_TRACE("NMDeviceWifi * NULL !");
                return false;
            }

            NMAccessPoint *activeAP = nm_device_wifi_get_active_access_point(wifiDevice);
            if(activeAP == NULL) {
                NMLOG_ERROR("No active access point found !");
                return false;
            }
            else
                NMLOG_TRACE("active access point found !");
            return true;
        }

        bool wifiManager::wifiConnectedSSIDInfo(Exchange::INetworkManager::WiFiSSIDInfo &ssidinfo)
        {
            if(!createClientNewConnection())
                return false;

            NMDeviceWifi *wifiDevice = NM_DEVICE_WIFI(getNmDevice());
            if(wifiDevice == NULL) {
                NMLOG_TRACE("NMDeviceWifi * NULL !");
                return false;
            }

            NMAccessPoint *activeAP = nm_device_wifi_get_active_access_point(wifiDevice);
            if(activeAP == NULL) {
                NMLOG_ERROR("No active access point found !");
                return false;
            }
            else
                NMLOG_TRACE("active access point found !");

            getApInfo(activeAP, ssidinfo);
            return true;
        }

        static void wifiDisconnectCb(GObject *object, GAsyncResult *result, gpointer user_data)
        {
            NMDevice     *device = NM_DEVICE(object);
            GError       *error = NULL;
            wifiManager *_wifiManager = (static_cast<wifiManager*>(user_data));

            NMLOG_TRACE("Disconnecting... ");
            if (!nm_device_disconnect_finish(device, result, &error)) {
                if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
                    return;

                NMLOG_ERROR("Device '%s' (%s) disconnecting failed: %s",
                            nm_device_get_iface(device),
                            nm_object_get_path(NM_OBJECT(device)),
                            error->message);
                g_error_free(error);
                _wifiManager->quit(device);
            }
            //_wifiManager->quit(device);
        }

        void static disconnectGsignalCb(NMDevice *device, GParamSpec *pspec, wifiManager *info)
        {
            if(NM_IS_DEVICE_WIFI(device))
            {
                NMDeviceState state = nm_device_get_state(device);
                switch(state)
                {
                    case NM_DEVICE_STATE_DEACTIVATING:
                        NMLOG_INFO("Device '%s' disconnecting", nm_device_get_iface(device));
                    break;
                    case NM_DEVICE_STATE_DISCONNECTED:
                        NMLOG_INFO("Device '%s' disconnected", nm_device_get_iface(device));
                        info->quit(device);
                    break;
                    case NM_DEVICE_STATE_ACTIVATED:
                        NMLOG_INFO("Device '%s' connected", nm_device_get_iface(device));
                        info->quit(device);
                    break;
                    case NM_DEVICE_STATE_FAILED:
                    case NM_DEVICE_STATE_UNKNOWN:
                    case NM_DEVICE_STATE_UNAVAILABLE:
                        NMLOG_INFO("Device '%s' Failed state", nm_device_get_iface(device));
                        info->quit(device);
                    break;
                    default:
                    break;
                }
            }
        }

        static void connectGsignalCb(NMDevice *device, GParamSpec *pspec, wifiManager *info)
        {
            if(NM_IS_DEVICE_WIFI(device))
            {
                NMDeviceState state = nm_device_get_state(device);
                switch(state)
                {
                    case NM_DEVICE_STATE_DEACTIVATING:
                        NMLOG_INFO("Device '%s' disconnecting", nm_device_get_iface(device));
                    break;
                    case NM_DEVICE_STATE_CONFIG:
                            NMLOG_INFO("Device '%s' configuring", nm_device_get_iface(device));
                    break;
                    case NM_DEVICE_STATE_DISCONNECTED:
                        NMLOG_INFO("Device '%s' disconnected", nm_device_get_iface(device));
                    break;
                    case NM_DEVICE_STATE_ACTIVATED:
                        NMLOG_INFO("Device '%s' connected", nm_device_get_iface(device));
                        info->quit(device);
                    break;
                    case NM_DEVICE_STATE_FAILED:
                    case NM_DEVICE_STATE_UNKNOWN:
                    case NM_DEVICE_STATE_UNAVAILABLE:
                        NMLOG_INFO("Device '%s' Failed state", nm_device_get_iface(device));
                        info->quit(device);
                    break;
                    default:
                    break;
                }
            }
        }

        bool wifiManager::wifiDisconnect()
        {
            if(!createClientNewConnection())
                return false;

            NMDevice *wifiNMDevice = getNmDevice();
            if(wifiNMDevice == NULL) {
                NMLOG_TRACE("NMDeviceWifi NULL !");
                return false;
            }

            wifiDeviceStateGsignal = g_signal_connect(wifiNMDevice, "notify::" NM_DEVICE_STATE, G_CALLBACK(disconnectGsignalCb), this);
            nm_device_disconnect_async(wifiNMDevice, NULL, wifiDisconnectCb, this);
            wait(loop);
            NMLOG_TRACE("Exit");
            return true;
        }

        bool wifiManager::quit(NMDevice *wifiNMDevice)
        {
            if (wifiNMDevice && wifiDeviceStateGsignal > 0) {
                g_signal_handler_disconnect(wifiNMDevice, wifiDeviceStateGsignal);
                wifiDeviceStateGsignal = 0;
            }

            if(!g_main_loop_is_running(loop)) {
                NMLOG_ERROR("g_main_loop_is not running");
                return false;
            }

            g_main_loop_quit(loop);
            return false;
        }

        bool wifiManager::wait(GMainLoop *loop)
        {
            if(g_main_loop_is_running(loop)) {
                NMLOG_WARNING("g_main_loop_is running");
                return false;
            }
            g_main_loop_run(loop);
            return true;
        }

        static NMAccessPoint *checkSSIDAvailable(NMDevice *device, const GPtrArray *aps, const char *ssid)
        {
            NMAccessPoint   *AccessPoint = NULL;
            aps = nm_device_wifi_get_access_points(NM_DEVICE_WIFI(device));
            for (guint i = 0; i < aps->len; i++)
            {
                NMAccessPoint *candidate_ap = static_cast<NMAccessPoint *>(g_ptr_array_index(aps, i));
                if (ssid)
                {
                    GBytes *ssidGBytes;
                    ssidGBytes = nm_access_point_get_ssid(candidate_ap);
                    if (!ssidGBytes)
                        continue;
                    gsize size;
                    const guint8 *ssidData = static_cast<const guint8 *>(g_bytes_get_data(ssidGBytes, &size));
                    std::string ssidstr(reinterpret_cast<const char *>(ssidData), size);
                    //g_bytes_unref(ssidGBytes);
                    NMLOG_TRACE("ssid <  %s  >", ssidstr.c_str());
                    if (strcmp(ssid, ssidstr.c_str()) == 0)
                    {
                        AccessPoint = candidate_ap;
                        break;
                    }
                }
            }

            return AccessPoint;
        }

        static void wifiConnectCb(GObject *client, GAsyncResult *result, gpointer user_data)
        {
            GError *error = NULL;
            wifiManager *_wifiManager = (static_cast<wifiManager*>(user_data));

            if (_wifiManager->createNewConnection) {
                NMLOG_TRACE("nm_client_add_and_activate_connection_finish");
                nm_client_add_and_activate_connection_finish(NM_CLIENT(_wifiManager->client), result, &error);
            }
            else {
                NMLOG_TRACE("nm_client_activate_connection_finish ");
                nm_client_activate_connection_finish(NM_CLIENT(_wifiManager->client), result, &error);
            }

            if (error) {
                if (_wifiManager->createNewConnection) {
                    NMLOG_ERROR("Failed to add/activate new connection: %s", error->message);
                } else {
                    NMLOG_ERROR("Failed to activate connection: %s", error->message);
                }
                g_main_loop_quit(_wifiManager->loop);
            }
        }

        static void wifiConnectionUpdate(GObject *source_object, GAsyncResult *res, gpointer user_data)
        {
            NMRemoteConnection        *remote_con = NM_REMOTE_CONNECTION(source_object);
            wifiManager *_wifiManager = (static_cast<wifiManager*>(user_data));
            GVariant *ret = NULL;
            GError *error = NULL;

            ret = nm_remote_connection_update2_finish(remote_con, res, &error);

            if (!ret) {
                NMLOG_ERROR("Error: %s.", error->message);
                g_error_free(error);
                _wifiManager->quit(NULL);
                return;
            }
            _wifiManager->createNewConnection = false; // no need to create new connection
            nm_client_activate_connection_async(
                _wifiManager->client, NM_CONNECTION(remote_con), _wifiManager->wifidevice, _wifiManager->objectPath, NULL, wifiConnectCb, _wifiManager);
        }

        bool wifiManager::createClientNewConnection()
        {
            GError *error = NULL;
            if(client != nullptr)
            {
                g_object_unref(client);
                client = nullptr;
            }

            client = nm_client_new(NULL, &error);
            if (!client || !loop) {
                NMLOG_ERROR("Could not connect to NetworkManager: %s.", error->message);
                g_error_free(error);
                return false;
            }
            return true;
        }

        bool wifiManager::wifiConnect(Exchange::INetworkManager::WiFiConnectTo wifiData)
        {
            const char *ssid_in = wifiData.m_ssid.c_str();
            const char* password_in = wifiData.m_passphrase.c_str();
            NMAccessPoint *AccessPoint = NULL;
            GPtrArray *allaps = NULL;
            const char *conName = ssid_in;
            NMConnection *connection = NULL;
            NMSettingConnection  *sConnection = NULL;
            NMSetting8021x *s8021X = NULL;
            NMSettingWireless *sWireless = NULL;
            NMSettingWirelessSecurity *sSecurity = NULL;
            NM80211ApFlags apFlags;
            NM80211ApSecurityFlags apWpaFlags;
            NM80211ApSecurityFlags apRsnFlags;
            const char  *ifname     = NULL;
            const GPtrArray  *availableConnections;
            bool SSIDmatch = false;
            Exchange::INetworkManager::WiFiSSIDInfo apinfo;

            if(!createClientNewConnection())
                return false;

            if (strlen(ssid_in) > 32)
            {
                NMLOG_WARNING("ssid length grater than 32");
                return false;
            }

            NMDevice *device = NULL;
            device = getNmDevice();
            if(device == NULL)
                return false;
            wifidevice = device;

            std::string activeSSID;
            if(getConnectedSSID(NM_DEVICE_WIFI(wifidevice), activeSSID))
            {
                if(strcmp(ssid_in, activeSSID.c_str()) == 0)
                {
                    NMLOG_WARNING("ssid already connected !");
                    return true;
                }
                else
                {
                    NMLOG_WARNING("wifi already connected with %s AP", activeSSID.c_str());
                }
            }
            //NMLOG_TRACE("Wireless Device found ifce : %s !", ifname = nm_device_get_iface(device));
            AccessPoint = checkSSIDAvailable(device, allaps, ssid_in);
            // TODO Scann hidden ssid also for lnf
            if(AccessPoint == NULL) {
                NMLOG_WARNING("No network with SSID '%s' found !", ssid_in);
                return false;
            }

            getApInfo(AccessPoint, apinfo);

            availableConnections = nm_device_get_available_connections(device);
            for (guint i = 0; i < availableConnections->len; i++)
            {
                NMConnection *currentConnection = static_cast<NMConnection*>(g_ptr_array_index(availableConnections, i));
                const char   *id        = nm_connection_get_id(NM_CONNECTION(currentConnection));

                if (conName) {
                    if (!id || strcmp(id, conName))
                        continue;

                    SSIDmatch = TRUE;
                }

                if (nm_access_point_connection_valid(AccessPoint, NM_CONNECTION(currentConnection))) {
                    connection = g_object_ref(currentConnection);
                    NMLOG_INFO("Connection '%s' exists !", conName);
                    break;
                }
            }

            if (SSIDmatch && !connection)
            {
                NMLOG_ERROR("Connection '%s' exists but properties don't match", conName);
                //TODO Remove Connection
                return false;
            }

            if (!connection)
            {
                NMLOG_TRACE("creating new connection '%s' ", conName);
                connection = nm_simple_connection_new();
                if (conName) {
                    sConnection = (NMSettingConnection *) nm_setting_connection_new();
                    nm_connection_add_setting(connection, NM_SETTING(sConnection));
                    const char *uuid = nm_utils_uuid_generate();;

                    g_object_set(G_OBJECT(sConnection),
                        NM_SETTING_CONNECTION_UUID,
                        uuid,
                        NM_SETTING_CONNECTION_ID,
                        conName,
                        NM_SETTING_CONNECTION_TYPE,
                        "802-11-wireless",
                        NULL);
                }

                sWireless = (NMSettingWireless *)nm_setting_wireless_new();
                GBytes *ssid = g_bytes_new(ssid_in, strlen(ssid_in));
                g_object_set(G_OBJECT(sWireless),
                    NM_SETTING_WIRELESS_SSID,
                    ssid,
                    NULL);
                //g_bytes_unref(ssid);
               /* For lnf network need to include
                *
                * 'bssid' parameter is used to restrict the connection only to the BSSID
                *  g_object_set(s_wifi, NM_SETTING_WIRELESS_BSSID, bssid, NULL);
                *  g_object_set(s_wifi, NM_SETTING_WIRELESS_SSID, ssid, NM_SETTING_WIRELESS_HIDDEN, hidden, NULL);
                */
                nm_connection_add_setting(connection, NM_SETTING(sWireless));
            }

            apFlags = nm_access_point_get_flags(AccessPoint);
            apWpaFlags = nm_access_point_get_wpa_flags(AccessPoint);
            apRsnFlags = nm_access_point_get_rsn_flags(AccessPoint);

            // check ap flag ty securti we supporting
            if(apFlags != NM_802_11_AP_FLAGS_NONE && strlen(password_in) < 1 )
            {
                NMLOG_ERROR("This ap(%s) security need password please add password!", ssid_in);
                return false;
            }

            if ( (apRsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE) || (apRsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM)) {

                NMLOG_ERROR("Ap wifi security OWE");
                return false;
            }

            if( (apWpaFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X) || (apRsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X) )
            {
                GError *error = NULL;
                NMLOG_INFO("Ap securtity mode is 802.1X");

                NMLOG_TRACE("802.1x Identity : %s", wifiData.m_identity.c_str());
                NMLOG_TRACE("802.1x CA cert path : %s", wifiData.m_caCert.c_str());
                NMLOG_TRACE("802.1x Client cert path : %s", wifiData.m_clientCert.c_str());
                NMLOG_TRACE("802.1x Private key path : %s", wifiData.m_privateKey.c_str());
                NMLOG_TRACE("802.1x Private key psswd : %s", wifiData.m_privateKeyPasswd.c_str());

                s8021X = (NMSetting8021x *) nm_setting_802_1x_new();
                nm_connection_add_setting(connection, NM_SETTING(s8021X));

                g_object_set(s8021X, NM_SETTING_802_1X_IDENTITY, wifiData.m_identity.c_str(), NULL);
                nm_setting_802_1x_add_eap_method(s8021X, "tls");
                if(!wifiData.m_caCert.empty() && !nm_setting_802_1x_set_ca_cert(s8021X,
                                            wifiData.m_caCert.c_str(),
                                            NM_SETTING_802_1X_CK_SCHEME_PATH,
                                            NULL,
                                            &error))
                {
                    NMLOG_ERROR("ca certificate add failed: %s", error->message);
                    g_error_free(error);
                    return false;
                }

                if(!wifiData.m_clientCert.empty() && !nm_setting_802_1x_set_client_cert(s8021X,
                                            wifiData.m_clientCert.c_str(),
                                            NM_SETTING_802_1X_CK_SCHEME_PATH,
                                            NULL,
                                            &error))
                {
                    NMLOG_ERROR("client certificate add failed: %s", error->message);
                    g_error_free(error);
                    return false;
                }

                if(!wifiData.m_privateKey.empty() && !nm_setting_802_1x_set_private_key(s8021X,
                                                wifiData.m_privateKey.c_str(),
                                                wifiData.m_privateKeyPasswd.c_str(),
                                                NM_SETTING_802_1X_CK_SCHEME_PATH,
                                                NULL,
                                                &error))
                {
                    NMLOG_ERROR("client private key add failed: %s", error->message);
                    g_error_free(error);
                    return false;
                }

                sSecurity = (NMSettingWirelessSecurity *) nm_setting_wireless_security_new();
                nm_connection_add_setting(connection, NM_SETTING(sSecurity));
                g_object_set(G_OBJECT(sSecurity), NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,"wpa-eap", NULL);
            }
            else if ((apFlags & NM_802_11_AP_FLAGS_PRIVACY) || (apWpaFlags != NM_802_11_AP_SEC_NONE )|| (apRsnFlags != NM_802_11_AP_SEC_NONE )) 
            {
                std::string flagStr;
                apFlagsToString(apWpaFlags, flagStr);
                apFlagsToString(apRsnFlags, flagStr);
                NMLOG_INFO("%s ap securtity mode ( %s) supported !", ssid_in, flagStr.c_str());

                if (password_in) 
                {
                    sSecurity = (NMSettingWirelessSecurity *) nm_setting_wireless_security_new();
                    nm_connection_add_setting(connection, NM_SETTING(sSecurity));

                    if (apWpaFlags == NM_802_11_AP_SEC_NONE && apRsnFlags == NM_802_11_AP_SEC_NONE)
                    {
                        nm_setting_wireless_security_set_wep_key(sSecurity, 0, password_in);
                        NMLOG_ERROR("wifi security WEP mode not supported ! need to add wep-key-type");
                        return false;
                    }
                    else if ((apWpaFlags & NM_802_11_AP_SEC_KEY_MGMT_PSK) 
                            || (apRsnFlags & NM_802_11_AP_SEC_KEY_MGMT_PSK) || (apRsnFlags & NM_802_11_AP_SEC_KEY_MGMT_SAE)) {

                        g_object_set(G_OBJECT(sSecurity), NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,"wpa-psk", NULL);
                        g_object_set(G_OBJECT(sSecurity), NM_SETTING_WIRELESS_SECURITY_PSK, password_in, NULL);
                    }
                }
                else
                {
                    NMLOG_ERROR("This AccessPoint(%s) need password please add password!", ssid_in);
                    return false;
                }
            }
            else
            {
                /* for open network every flag value will be zero */
                if (apFlags == NM_802_11_AP_FLAGS_NONE && apWpaFlags == NM_802_11_AP_SEC_NONE && apRsnFlags == NM_802_11_AP_SEC_NONE) {
                    NMLOG_INFO("open network no password requied");
                }
                else {
                    NMLOG_ERROR("wifi security mode not supported !");
                    return false;
                }
            }

            objectPath = nm_object_get_path(NM_OBJECT(AccessPoint));
            wifiDeviceStateGsignal = g_signal_connect(device, "notify::" NM_DEVICE_STATE, G_CALLBACK(connectGsignalCb), this);
            GVariant *nmDbusConnection = nm_connection_to_dbus(connection, NM_CONNECTION_SERIALIZE_ALL);
            if (NM_IS_REMOTE_CONNECTION(connection))
            {
                nm_remote_connection_update2(NM_REMOTE_CONNECTION(connection),
                                            nmDbusConnection,
                                            NM_SETTINGS_UPDATE2_FLAG_BLOCK_AUTOCONNECT, // autoconnect right away
                                            NULL,
                                            NULL,
                                            wifiConnectionUpdate,
                                            this);
            }
            else
            {
                createNewConnection = true;
                nm_client_add_and_activate_connection_async(client, connection, device, objectPath, NULL, wifiConnectCb, this);
            }

            wait(loop);
            return true;
        }
    } // namespace Plugin
} // namespace WPEFramework
