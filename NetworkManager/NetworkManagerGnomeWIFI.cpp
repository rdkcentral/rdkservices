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
#include "NetworkManagerGnomeUtils.h"

namespace WPEFramework
{
    namespace Plugin
    {

        wifiManager* wifiManager::getInstance()
        {
            static wifiManager instance;
            return &instance;
        }

        wifiManager::~wifiManager() {
            NMLOG_INFO("~wifiManager");
            g_main_context_pop_thread_default(nmContext);
            if(loop != NULL)
                g_main_loop_unref(loop);
            if(client != NULL)
                g_object_unref(client);
        }

        wifiManager::wifiManager() : client(nullptr), loop(nullptr), createNewConnection(false) {
            nmContext = g_main_context_new();
            g_main_context_push_thread_default(nmContext);
            loop = g_main_loop_new(nmContext, FALSE);
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

        static gboolean gmainLoopTimoutCB(gpointer user_data)
        {
            wifiManager *_wifiManager = (static_cast<wifiManager*>(user_data));
            NMLOG_WARNING("GmainLoop ERROR_TIMEDOUT");
            _wifiManager->isSuccess = false;
            g_main_loop_quit(_wifiManager->loop);
            return true;
        }
    
        bool wifiManager::wait(GMainLoop *loop, int timeOutMs)
        {
            if(g_main_loop_is_running(loop)) {
                NMLOG_WARNING("g_main_loop_is running");
                return false;
            }
            source = g_timeout_source_new(10000);  // 10000ms interval
            g_source_set_callback(source, (GSourceFunc)gmainLoopTimoutCB, this, NULL);
            g_source_attach(source, NULL);
            g_main_loop_run(loop);
            if(source != nullptr) {
                if(g_source_is_destroyed(source)) {
                    NMLOG_WARNING("Source has been destroyed");
                }
                else {
                    g_source_destroy(source);
                }
                g_source_unref(source);
            }
            return true;
        }

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
            NMLOG_TRACE("bssid: %s", wifiInfo.m_bssid.c_str());

            if (freq >= 2400 && freq < 5000) {
                wifiInfo.m_frequency = Exchange::INetworkManager::WiFiFrequency::WIFI_FREQUENCY_2_4_GHZ;
                NMLOG_TRACE("freq: WIFI_FREQUENCY_2_4_GHZ");
            }
            else if (freq >= 5000 && freq < 6000) {
                wifiInfo.m_frequency =  Exchange::INetworkManager::WiFiFrequency::WIFI_FREQUENCY_5_GHZ;
                NMLOG_TRACE("freq: WIFI_FREQUENCY_5_GHZ");
            }
            else if (freq >= 6000) {
                wifiInfo.m_frequency = Exchange::INetworkManager::WiFiFrequency::WIFI_FREQUENCY_6_GHZ;
                NMLOG_TRACE("freq: WIFI_FREQUENCY_6_GHZ");
            }
            else {
                wifiInfo.m_frequency = Exchange::INetworkManager::WiFiFrequency::WIFI_FREQUENCY_WHATEVER;
                NMLOG_TRACE("freq: No available !");
            }

            wifiInfo.m_rate = std::to_string(bitrate);
            NMLOG_TRACE("bitrate : %s kbit/s", wifiInfo.m_rate.c_str());
            //TODO signal strenght to dBm
            wifiInfo.m_signalStrength = std::to_string(static_cast<u_int8_t>(strength));
            NMLOG_TRACE("sterngth: %s %%", wifiInfo.m_signalStrength.c_str());
            wifiInfo.m_securityMode = static_cast<Exchange::INetworkManager::WIFISecurityMode>(nmUtils::wifiSecurityModeFromAp(flags, wpaFlags, rsnFlags));
            NMLOG_TRACE("security %s", nmUtils::getSecurityModeString(flags, wpaFlags, rsnFlags).c_str());
            NMLOG_TRACE("Mode: %s", mode == NM_802_11_MODE_ADHOC   ? "Ad-Hoc": mode == NM_802_11_MODE_INFRA ? "Infrastructure": "Unknown");
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
            _wifiManager->isSuccess = true;
            if (!nm_device_disconnect_finish(device, result, &error)) {
                if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
                    return;

                NMLOG_ERROR("Device '%s' (%s) disconnecting failed: %s",
                            nm_device_get_iface(device),
                            nm_object_get_path(NM_OBJECT(device)),
                            error->message);
                g_error_free(error);
                _wifiManager->quit(device);
                 _wifiManager->isSuccess = false;
            }
            _wifiManager->quit(device);
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

            nm_device_disconnect_async(wifiNMDevice, NULL, wifiDisconnectCb, this);
            wait(loop);
            NMLOG_TRACE("Exit");
            return isSuccess;
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
                   // NMLOG_TRACE("ssid <  %s  >", ssidstr.c_str());
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
                 _wifiManager->isSuccess = true;
            }
            else {
                NMLOG_TRACE("nm_client_activate_connection_finish ");
                nm_client_activate_connection_finish(NM_CLIENT(_wifiManager->client), result, &error);
                 _wifiManager->isSuccess = true;
            }

            if (error) {
                 _wifiManager->isSuccess = false;
                if (_wifiManager->createNewConnection) {
                    NMLOG_ERROR("Failed to add/activate new connection: %s", error->message);
                } else {
                    NMLOG_ERROR("Failed to activate connection: %s", error->message);
                }
            }

            g_main_loop_quit(_wifiManager->loop);
        }

        static void wifiConnectionUpdate(GObject *rmObject, GAsyncResult *res, gpointer user_data)
        {
            NMRemoteConnection        *remote_con = NM_REMOTE_CONNECTION(rmObject);
            wifiManager *_wifiManager = (static_cast<wifiManager*>(user_data));
            GVariant *ret = NULL;
            GError *error = NULL;

            ret = nm_remote_connection_update2_finish(remote_con, res, &error);

            if (!ret) {
                NMLOG_ERROR("Error: %s.", error->message);
                g_error_free(error);
                _wifiManager->isSuccess = false;
                _wifiManager->quit(NULL);
                return;
            }
            _wifiManager->createNewConnection = false; // no need to create new connection
            nm_client_activate_connection_async(
                _wifiManager->client, NM_CONNECTION(remote_con), _wifiManager->wifidevice, _wifiManager->objectPath, NULL, wifiConnectCb, _wifiManager);
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
                // TODO send SSID NO AVAILABLE event
                return false;
            }

            getApInfo(AccessPoint, apinfo);

            availableConnections = nm_device_get_available_connections(device);
            for (guint i = 0; i < availableConnections->len; i++)
            {
                NMConnection *currentConnection = static_cast<NMConnection*>(g_ptr_array_index(availableConnections, i));
                const char   *id = nm_connection_get_id(NM_CONNECTION(currentConnection));

                if (conName) {
                    if (!id || strcmp(id, conName))
                        continue;

                    SSIDmatch = TRUE;
                }

                if (nm_access_point_connection_valid(AccessPoint, NM_CONNECTION(currentConnection))) {
                    connection = g_object_ref(currentConnection);
                    NMLOG_TRACE("Connection '%s' exists !", conName);
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
                NMLOG_INFO("%s ap securtity mode (%s) supported !", ssid_in, nmUtils::getSecurityModeString(apFlags,apWpaFlags,apRsnFlags).c_str());

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
            return isSuccess;
        }

        static void addToKnownSSIDsCb(GObject *client, GAsyncResult *result, gpointer user_data)
        {

            GError *error = NULL;
            wifiManager *_wifiManager = (static_cast<wifiManager*>(user_data));
            //NMRemoteConnection *connection = NM_REMOTE_CONNECTION(s);
            if (!nm_client_add_connection_finish(NM_CLIENT(client), result, &error)) {
                NMLOG_ERROR ("AddToKnownSSIDs Failed");
                _wifiManager->isSuccess = false;
            }
            else
            {
                NMLOG_TRACE ("AddToKnownSSIDs is success");
                _wifiManager->isSuccess = true;
            }

            g_main_loop_quit(_wifiManager->loop);
        }

        bool wifiManager::addToKnownSSIDs(const Exchange::INetworkManager::WiFiConnectTo ssidinfo)
        {
            if(!createClientNewConnection())
                return false;

            NMSettingWirelessSecurity *nmSettingsWifiSec;
            NMSettingWireless *nmSettingsWifi;
            const char *uuid = nm_utils_uuid_generate();

            nmSettingsWifiSec = (NMSettingWirelessSecurity *)nm_setting_wireless_security_new();
            NMSettingConnection *nmConnSec =  (NMSettingConnection *)nm_setting_connection_new();
            g_object_set(G_OBJECT(nmConnSec),
                    NM_SETTING_CONNECTION_UUID,
                    uuid,
                    NM_SETTING_CONNECTION_ID,
                    ssidinfo.m_ssid.c_str(),
                    NM_SETTING_CONNECTION_TYPE,
                    "802-11-wireless",
                    NULL);
            NMConnection *connection = nm_simple_connection_new();
            nm_connection_add_setting(connection, NM_SETTING(nmConnSec));
            nmSettingsWifi = (NMSettingWireless *)nm_setting_wireless_new();
            GString *ssidStr = g_string_new(ssidinfo.m_ssid.c_str());
            g_object_set(G_OBJECT(nmSettingsWifi), NM_SETTING_WIRELESS_SSID, ssidStr, NULL);
            
            nm_connection_add_setting(connection, NM_SETTING(nmSettingsWifi));
            nmSettingsWifiSec = (NMSettingWirelessSecurity *)nm_setting_wireless_security_new();
            // TODO chek different securtity mode and portocol and add settings
            switch(ssidinfo.m_securityMode)
            {
                case Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_PSK_AES:
                case Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_WPA2_PSK:
                case Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_PSK_TKIP:
                case Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA2_PSK_AES:
                case Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA2_PSK_TKIP:
                case Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA3_SAE:
                {
                        g_object_set(G_OBJECT(nmSettingsWifiSec), NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,"wpa-psk", NULL);
                        if(!ssidinfo.m_passphrase.empty())
                            g_object_set(G_OBJECT(nmSettingsWifiSec), NM_SETTING_WIRELESS_SECURITY_PSK, ssidinfo.m_passphrase.c_str(), NULL);
                    break;
                }
                case Exchange::INetworkManager::WIFI_SECURITY_NONE:
                     NMLOG_INFO("open wifi network configuration");
                     break;
                default:
                {
                    NMLOG_WARNING("connection wifi securtity type not supported");
                    return false;
                }
            }

            nm_connection_add_setting(connection, NM_SETTING(nmSettingsWifiSec));
            nm_client_add_connection_async(client, connection, true, NULL, addToKnownSSIDsCb, this);
            //wait(loop);
            // TODO change to GmainLoooprun
            g_main_loop_unref(loop);
            return isSuccess;
        }

        static void removeKnownSSIDCb(GObject *client, GAsyncResult *result, gpointer user_data)
        {
            GError *error = NULL;
            wifiManager *_wifiManager = (static_cast<wifiManager*>(user_data));
            NMRemoteConnection *connection = NM_REMOTE_CONNECTION(client);
            if (!nm_remote_connection_delete_finish(connection, result, &error)) {
                NMLOG_ERROR("RemoveKnownSSID failed %s", error->message);
                _wifiManager->isSuccess = false;
            }
            else
            {
                NMLOG_INFO ("RemoveKnownSSID is success");
                _wifiManager->isSuccess = true;
            }

            g_main_loop_quit(_wifiManager->loop);
        }

        bool wifiManager::removeKnownSSID(const string& ssid)
        {

            if(!createClientNewConnection())
                return false;

            if(ssid.empty())
                return false;
            isSuccess = false;

            NMRemoteConnection* remoteConnection;
            const GPtrArray* connections = nm_client_get_connections(client);

            for (guint i = 0; i < connections->len; i++)
            {
                remoteConnection = NM_REMOTE_CONNECTION(connections->pdata[i]);
                NMConnection *connection = NM_CONNECTION(connections->pdata[i]);
                if (NM_IS_SETTING_WIRELESS(nm_connection_get_setting_wireless(connection)))
                {
                    GBytes *ssidBytes = nm_setting_wireless_get_ssid(nm_connection_get_setting_wireless(connection));
                    if (ssidBytes)
                    {
                        gsize ssidSize;
                        const guint8 *ssidData = static_cast<const guint8 *>(g_bytes_get_data(ssidBytes, &ssidSize));
                        std::string ssidstr(reinterpret_cast<const char *>(ssidData), ssidSize);
                        if (ssid == ssidstr)
                        {
                            //nm_remote_connection_delete_async(remoteConnection, NULL, removeKnownSSIDCb, this);
                            // TODO add async
                            GError *error = NULL;
                            nm_remote_connection_delete(remoteConnection, NULL, &error);
                            if (error)
                            {
                                NMLOG_ERROR("RemoveKnownSSID failed %s", error->message);
                                g_error_free(error);
                            }
                            else
                            {
                                NMLOG_INFO("RemoveKnownSSID is success %s", ssid.c_str());
                                isSuccess = true;
                            }
                            break; // if remove all connection with same ssid not to break
                        }
                    }
                }
            }
    
            return isSuccess;
        }

        bool wifiManager::getKnownSSIDs(std::list<string>& ssids)
        {
            if(!createClientNewConnection())
                return false;
            const GPtrArray *connections = nm_client_get_connections(client);
            std::string ssidPrint;
            for (guint i = 0; i < connections->len; i++)
            {
                NMConnection *connection = NM_CONNECTION(connections->pdata[i]);

                if (NM_IS_SETTING_WIRELESS(nm_connection_get_setting_wireless(connection)))
                {
                    GBytes *ssidBytes = nm_setting_wireless_get_ssid(nm_connection_get_setting_wireless(connection));
                    if (ssidBytes)
                    {
                        gsize ssidSize;
                        const guint8 *ssidData = static_cast<const guint8 *>(g_bytes_get_data(ssidBytes, &ssidSize));
                        std::string ssidstr(reinterpret_cast<const char *>(ssidData), ssidSize);
                        if (!ssidstr.empty())
                        {
                            ssids.push_back(ssidstr);
                            ssidPrint += ssidstr;
                            ssidPrint += ", ";
                        }
                    }
                }
            }
            if (!ssids.empty())
            {
                NMLOG_TRACE("known wifi connections are %s", ssidPrint.c_str());
                return true;
            }

            return false;
        }

        static void wifiScanCb(GObject *object, GAsyncResult *result, gpointer user_data)
        {
            GError *error = NULL;
            wifiManager *_wifiManager = (static_cast<wifiManager*>(user_data));
            if(nm_device_wifi_request_scan_finish(NM_DEVICE_WIFI(object), result, &error)) {
                 NMLOG_TRACE("Scanning success");
                 _wifiManager->isSuccess = true;
            }
            else
            {
                NMLOG_ERROR("Scanning Failed");
                _wifiManager->isSuccess = false;
            }
            if (error) {
                NMLOG_ERROR("Scanning Failed Error: %s.", error->message);
                _wifiManager->isSuccess = false;
                g_error_free(error);
            }

            g_main_loop_quit(_wifiManager->loop);
        }

        bool wifiManager::wifiScanRequest(const Exchange::INetworkManager::WiFiFrequency frequency, std::string ssidReq)
        {
            if(!createClientNewConnection())
                return false;
            NMDeviceWifi *wifiDevice = NM_DEVICE_WIFI(getNmDevice());
            if(wifiDevice == NULL) {
                NMLOG_TRACE("NMDeviceWifi * NULL !");
                return false;
            }
            isSuccess = false;
            if(!ssidReq.empty())
            {
                NMLOG_TRACE("staring wifi scanning .. %s", ssidReq.c_str());
                GVariantBuilder builder, array_builder;
                GVariant *options;
                g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
                g_variant_builder_init(&array_builder, G_VARIANT_TYPE("aay"));
                g_variant_builder_add(&array_builder, "@ay",
                                    g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, (const guint8 *) ssidReq.c_str(), ssidReq.length(), 1)
                                    );
                g_variant_builder_add(&builder, "{sv}", "ssids", g_variant_builder_end(&array_builder));
                g_variant_builder_add(&builder, "{sv}", "hidden", g_variant_new_boolean(TRUE));
                options = g_variant_builder_end(&builder);
                nm_device_wifi_request_scan_options_async(NM_DEVICE_WIFI(wifiDevice), options, NULL, wifiScanCb, this);
            }
            else {
                NMLOG_TRACE("staring normal wifi scanning");
                nm_device_wifi_request_scan_async(NM_DEVICE_WIFI(wifiDevice), NULL, wifiScanCb, this);
            }
            wait(loop);
            return isSuccess;
        }
    } // namespace Plugin
} // namespace WPEFramework
