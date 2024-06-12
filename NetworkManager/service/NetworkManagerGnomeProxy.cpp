#include "NetworkManagerImplementation.h"
#include "NetworkManagerGnomeWIFI.h"
#include <libnm/NetworkManager.h>
#include <fstream>
#include <sstream>

static NMClient *client;

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace std;
GMainLoop *g_loop;
static std::vector<Exchange::INetworkManager::InterfaceDetails> interfaceList;

namespace WPEFramework
{
    namespace Plugin
    {
        wifiManager *wifi = nullptr;
        const float signalStrengthThresholdExcellent = -50.0f;
        const float signalStrengthThresholdGood = -60.0f;
        const float signalStrengthThresholdFair = -67.0f;
        NetworkManagerImplementation* _instance = nullptr;

        void NetworkManagerInternalEventHandler(const char *owner, int eventId, void *data, size_t len)
        {
	    return;
        }

        void NetworkManagerImplementation::platform_init()
        {
            ::_instance = this;
            GError *error = NULL;
            GMainContext *context = g_main_context_new();
            // initialize the NMClient object
            client = nm_client_new(NULL, &error);
            if (client == NULL) {
                NMLOG_TRACE("Error initializing NMClient: %s\n", error->message);
                g_error_free(error);
                return;
            }
            g_loop = g_main_loop_new(context, FALSE);
            wifi = wifiManager::getInstance();
            return;
        }

        uint32_t GetInterfacesName(string &wifiInterface, string &ethernetInterface) {
            string line;
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;

            ifstream file("/etc/device.properties");
            if (!file.is_open()) {
                NMLOG_TRACE("Error opening file\n");
                return rc;
            }   

            while (std::getline(file, line)) {
                // Remove newline character if present
                if (!line.empty() && line.back() == '\n') {
                    line.pop_back();
                }

                istringstream iss(line);
                string token;
                getline(iss, token, '=');

                if (token == "WIFI_INTERFACE") {
                    std::getline(iss, wifiInterface, '=');
                } else if (token == "ETHERNET_INTERFACE") {
                    std::getline(iss, ethernetInterface, '=');
                }
            }   

            file.close();

            return rc;
        }

        uint32_t NetworkManagerImplementation::GetAvailableInterfaces (Exchange::INetworkManager::IInterfaceDetailsIterator*& interfacesItr/* @out */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            string interfaces[2];
            string wifiInterface;
            string ethernetInterface;
            NMDeviceType type;
            InterfaceDetails tmp;
            NMDeviceState state;
            NMDevice *device = NULL;

            if(interfaceList.empty())
            {
                GetInterfacesName(wifiInterface, ethernetInterface);
                interfaces[0] = wifiInterface;
                interfaces[1] = ethernetInterface;
                for (size_t i = 0; i < 2; i++) {
                    if(!interfaces[i].empty())
                    {
                        device = nm_client_get_device_by_iface(client, interfaces[i].c_str());
                        if (device)
                        {
                            if(i == 0)        
                                tmp.m_type = string("WIFI");
                            else
                                tmp.m_type = string("ETHERNET");
                            tmp.m_name = interfaces[i].c_str();
                            tmp.m_mac = nm_device_get_hw_address(device);
                            state = nm_device_get_state(device);
                            tmp.m_isEnabled = (state > NM_DEVICE_STATE_UNAVAILABLE) ? 1 : 0;
                            tmp.m_isConnected = (state > NM_DEVICE_STATE_DISCONNECTED) ? 1: 0;
                            interfaceList.push_back(tmp);
                            g_clear_object(&device);
                        }
                    }
                }
            }

            using Implementation = RPC::IteratorType<Exchange::INetworkManager::IInterfaceDetailsIterator>;
            interfacesItr = Core::Service<Implementation>::Create<Exchange::INetworkManager::IInterfaceDetailsIterator>(interfaceList);
            rc = Core::ERROR_NONE;
            return rc;
        }

        /* @brief Get the active Interface used for external world communication */
        uint32_t NetworkManagerImplementation::GetPrimaryInterface (string& interface /* @out */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            GError *error = NULL;
            NMActiveConnection *activeConn = NULL;
            NMRemoteConnection *remoteConn = NULL;

            activeConn = nm_client_get_primary_connection(client);
            if (activeConn == NULL) {
                NMLOG_TRACE("Error getting primary connection: %s\n", error->message);
                g_error_free(error);
                return rc;
            }   
            remoteConn = nm_active_connection_get_connection(activeConn);

            interface = nm_connection_get_interface_name(NM_CONNECTION(remoteConn));
            if(!interface.empty())
                rc = Core::ERROR_NONE;
            return rc;
        }

        /* @brief Set the active Interface used for external world communication */
        uint32_t NetworkManagerImplementation::SetPrimaryInterface (const string& interface/* @in */)
        {
            uint32_t rc = Core::ERROR_NONE;
            const GPtrArray *connections = nm_client_get_connections(client);
            NMConnection *conn = NULL;
            NMSettingConnection *settings;
            NMRemoteConnection *remoteConnection;
            for (guint i = 0; i < connections->len; i++) {
                NMConnection *connection = NM_CONNECTION(connections->pdata[i]);
                settings = nm_connection_get_setting_connection(connection);

                /* Check if the interface name matches */
                if (g_strcmp0(nm_setting_connection_get_interface_name(settings), interface.c_str()) == 0) {
                    conn = connection;
                    break;
                }
            }
            g_object_set(settings,
                    NM_SETTING_CONNECTION_AUTOCONNECT,
                    true,
                    NM_SETTING_CONNECTION_AUTOCONNECT_PRIORITY,
                    NM_SETTING_CONNECTION_AUTOCONNECT_PRIORITY_MAX,
                    NULL);
            const char *uuid = nm_connection_get_uuid(conn);
            remoteConnection = nm_client_get_connection_by_uuid(client, uuid);
            nm_remote_connection_commit_changes(remoteConnection, false, NULL, NULL);

            return rc;
        }

        uint32_t NetworkManagerImplementation::EnableInterface (const string& interface/* @in */)
        {
            uint32_t rc = Core::ERROR_NONE;
            const GPtrArray *devices = nm_client_get_devices(client);
            NMDevice *device = NULL;

            for (guint i = 0; i < devices->len; ++i) {
                device = NM_DEVICE(g_ptr_array_index(devices, i));

                // Get the device details
                const char *name = nm_device_get_iface(device);

                // Check if the device name matches
                if (g_strcmp0(name, interface.c_str()) == 0) {
                    nm_device_set_managed(device, true);

                    NMLOG_TRACE("Interface %s status set to enabled\n",
                            interface.c_str());
                }
            }

            // Cleanup
            if(device)
                g_clear_object(&device);
            return rc;
        }

        uint32_t NetworkManagerImplementation::DisableInterface (const string& interface/* @in */)
        {
            uint32_t rc = Core::ERROR_NONE;
            const GPtrArray *devices = nm_client_get_devices(client);
            NMDevice *device = NULL;

            for (guint i = 0; i < devices->len; ++i) {
                device = NM_DEVICE(g_ptr_array_index(devices, i));

                // Get the device details
                const char *name = nm_device_get_iface(device);

                // Check if the device name matches
                if (g_strcmp0(name, interface.c_str()) == 0) {
                    nm_device_set_managed(device, false);

                    NMLOG_TRACE("Interface %s status set to disabled\n",
                            interface.c_str());
                }
            }
 
            // Cleanup
            if(device)
                g_clear_object(&device);
            return rc;
        } 

        /* @brief Get IP Address Of the Interface */
        uint32_t NetworkManagerImplementation::GetIPSettings(const string& interface /* @in */, const string& ipversion /* @in */, IPAddressInfo& result /* @out */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            GError *error = NULL;
            NMActiveConnection *conn = NULL;
            NMIPConfig *ip4_config = NULL;
            NMIPConfig *ip6_config = NULL;
            const gchar *gateway = NULL;
            char **dns_arr = NULL;
            NMDhcpConfig *dhcp4_config = NULL;
            NMDhcpConfig *dhcp6_config = NULL;
            const char* dhcpserver;
            NMSettingConnection *settings;
            NMIPAddress *address;

            const GPtrArray *connections = nm_client_get_active_connections(client);


            for (guint i = 0; i < connections->len; i++) {
                NMActiveConnection *connection = NM_ACTIVE_CONNECTION(connections->pdata[i]);
                settings = nm_connection_get_setting_connection(NM_CONNECTION(nm_active_connection_get_connection(connection)));

                /* Check if the interface name matches */
                if (g_strcmp0(nm_setting_connection_get_interface_name(settings), interface.c_str()) == 0) {
                    conn = connection;
                    break;
                }
            }
            if (conn == NULL) {
                NMLOG_TRACE("Error getting primary connection: %s\n", error->message);
                g_error_free(error);
                return rc;
            }   

            if(0 == strcmp(ipversion.c_str(), "IPv4"))
            {
                ip4_config = nm_active_connection_get_ip4_config(conn);
                if (ip4_config != NULL) {
                    const GPtrArray *p; 
                    int              i;
                    p = nm_ip_config_get_addresses(ip4_config);
                    for (i = 0; i < p->len; i++) {
                        address = static_cast<NMIPAddress*>(p->pdata[i]);
                    }
                    gateway = nm_ip_config_get_gateway(ip4_config);
                }   
                dns_arr =   (char **)nm_ip_config_get_nameservers(ip4_config);

                dhcp4_config = nm_active_connection_get_dhcp4_config(conn);
                dhcpserver = nm_dhcp_config_get_one_option (dhcp4_config,
                               "dhcp_server_identifier");

                result.m_ipAddrType     = ipversion.c_str();
                if(dhcpserver)
                    result.m_dhcpServer     = dhcpserver;
                result.m_v6LinkLocal    = "";
                result.m_ipAddress      = nm_ip_address_get_address(address);
                result.m_prefix         = nm_ip_address_get_prefix(address);
                result.m_gateway        = gateway;
                if((*(&dns_arr[0]))!=NULL)
                    result.m_primaryDns     = *(&dns_arr[0]);
                if((*(&dns_arr[1]))!=NULL )
                    result.m_secondaryDns   = *(&dns_arr[1]);

                rc = Core::ERROR_NONE;
            }
            else if(0 == strcmp(ipversion.c_str(), "IPv6"))
            {
                NMIPAddress *a;
                ip6_config = nm_active_connection_get_ip6_config(conn);
                if (ip6_config != NULL) {
                    const GPtrArray *p; 
                    int              i;
                    p = nm_ip_config_get_addresses(ip6_config);
                    for (i = 0; i < p->len; i++) {
                        a = static_cast<NMIPAddress*>(p->pdata[i]);
                        result.m_ipAddress      = nm_ip_address_get_address(a);
                        NMLOG_TRACE("\tinet6 %s/%d\n", nm_ip_address_get_address(a), nm_ip_address_get_prefix(a));
                    }
                    gateway = nm_ip_config_get_gateway(ip6_config);

                    dns_arr =   (char **)nm_ip_config_get_nameservers(ip6_config);

                    dhcp6_config = nm_active_connection_get_dhcp6_config(conn);
                    dhcpserver = nm_dhcp_config_get_one_option (dhcp6_config,
                               "dhcp_server_identifier");
                    result.m_ipAddrType     = ipversion.c_str();
                    if(dhcpserver)
                        result.m_dhcpServer     = dhcpserver;
                    result.m_v6LinkLocal    = "";
                    result.m_prefix         = 0;
                    result.m_gateway        = gateway;
                    if((*(&dns_arr[0]))!=NULL)
                    result.m_primaryDns     = *(&dns_arr[0]);
                    if((*(&dns_arr[1]))!=NULL )
                    result.m_secondaryDns   = *(&dns_arr[1]);
                }
                rc = Core::ERROR_NONE;
            }
            return rc;
        }


        // Callback for nm_client_deactivate_connection_async
        static void on_deactivate_complete(GObject *source_object, GAsyncResult *res, gpointer user_data) {
            GError *error = NULL;

            // Check if the operation was successful
            if (!nm_client_deactivate_connection_finish(NM_CLIENT(source_object), res, &error)) {
                NMLOG_TRACE("Deactivating connection failed: %s\n", error->message);
                g_error_free(error);
            } else {
                NMLOG_TRACE("Deactivating connection successful\n");
            }
        }

        // Callback for nm_client_activate_connection_async
        static void on_activate_complete(GObject *source_object, GAsyncResult *res, gpointer user_data) {
            GError *error = NULL;

            // Check if the operation was successful
            if (!nm_client_activate_connection_finish(NM_CLIENT(source_object), res, &error)) {
                NMLOG_TRACE("Activating connection failed: %s\n", error->message);
                g_error_free(error);
            } else {
                NMLOG_TRACE("Activating connection successful\n");
            }

            g_main_loop_quit((GMainLoop*)user_data);
        }


        /* @brief Set IP Address Of the Interface */
        uint32_t NetworkManagerImplementation::SetIPSettings(const string& interface /* @in */, const string &ipversion /* @in */, const IPAddressInfo& address /* @in */)
        {
            uint32_t rc = Core::ERROR_NONE;
            const GPtrArray *connections = nm_client_get_connections(client);
            NMSettingIP4Config *s_ip4;
            NMSettingIP6Config *s_ip6;
            NMConnection *conn = NULL;
            NMSettingConnection *settings;
            NMRemoteConnection *remote_connection;
            NMSetting *setting;
            const char *uuid;
            NMDevice *device      = NULL;
            const char *spec_object;

            for (guint i = 0; i < connections->len; i++) {
                NMConnection *connection = NM_CONNECTION(connections->pdata[i]);
                settings = nm_connection_get_setting_connection(connection);

                /* Check if the interface name matches */
                if (g_strcmp0(nm_setting_connection_get_interface_name(settings), interface.c_str()) == 0) {
                    conn = connection;
                    break;
                }
            }
            if (!address.m_autoConfig)
            {
                if (strcasecmp("IPv4", ipversion.c_str()) == 0)
                {
                    NMSettingIPConfig *ip4_config = nm_connection_get_setting_ip4_config(conn);
                    if (ip4_config == NULL) 
                    {
                        ip4_config = (NMSettingIPConfig *)nm_setting_ip4_config_new();
                    }
                    NMIPAddress *ipAddress;
                    setting = nm_connection_get_setting_by_name(conn, "ipv4");
                    ipAddress = nm_ip_address_new(AF_INET, address.m_ipAddress.c_str(), address.m_prefix, NULL);
                    nm_setting_ip_config_clear_addresses(ip4_config);
                    nm_setting_ip_config_add_address(NM_SETTING_IP_CONFIG(setting), ipAddress);
                    nm_setting_ip_config_clear_dns(ip4_config);
                    nm_setting_ip_config_add_dns(ip4_config, address.m_primaryDns.c_str());
                    nm_setting_ip_config_add_dns(ip4_config, address.m_secondaryDns.c_str());

                    g_object_set(G_OBJECT(ip4_config),
                            NM_SETTING_IP_CONFIG_GATEWAY, address.m_gateway.c_str(),
                            NM_SETTING_IP_CONFIG_NEVER_DEFAULT,
                            FALSE,
                            NULL);
                }
                else
                {
                    //FIXME : Add IPv6 support here
                    printf("Setting IPv6 is not supported at this point in time. This is just a place holder\n");
                    rc = Core::ERROR_NOT_SUPPORTED;
                }
            }
            else
            {
                if (strcmp("IPv4", ipversion.c_str()) == 0)
                {
                    s_ip4 = (NMSettingIP4Config *)nm_setting_ip4_config_new();
                    g_object_set(G_OBJECT(s_ip4), NM_SETTING_IP_CONFIG_METHOD, NM_SETTING_IP4_CONFIG_METHOD_AUTO, NULL);
                    nm_connection_add_setting(conn, NM_SETTING(s_ip4));
                }
                else
                {
                    s_ip6 = (NMSettingIP6Config *)nm_setting_ip6_config_new();
                    g_object_set(G_OBJECT(s_ip6), NM_SETTING_IP_CONFIG_METHOD, NM_SETTING_IP6_CONFIG_METHOD_AUTO, NULL);
                    nm_connection_add_setting(conn, NM_SETTING(s_ip6));
                }
            }
            device = nm_client_get_device_by_iface(client, interface.c_str());
            uuid = nm_connection_get_uuid(conn);
            remote_connection = nm_client_get_connection_by_uuid(client, uuid);
            NMActiveConnection *active_connection = NULL;

            const GPtrArray *acv_connections = nm_client_get_active_connections(client);
            for (guint i = 0; i < acv_connections->len; i++) {
                NMActiveConnection *connection1 = NM_ACTIVE_CONNECTION(acv_connections->pdata[i]);
                settings = nm_connection_get_setting_connection(NM_CONNECTION(nm_active_connection_get_connection(connection1)));

                /* Check if the interface name matches */
                if (g_strcmp0(nm_setting_connection_get_interface_name(settings), interface.c_str()) == 0) {
                    active_connection = connection1;
                    break;
                }
            }
 
            spec_object = nm_object_get_path(NM_OBJECT(active_connection));
            nm_remote_connection_commit_changes(remote_connection, false, NULL, NULL);
            nm_client_deactivate_connection_async(client, active_connection, NULL, on_deactivate_complete, NULL);
            nm_client_activate_connection_async(client, conn, device, spec_object, NULL, on_activate_complete, g_loop);
            g_main_loop_run(g_loop);
            return rc;
        }

        static void on_scan_done(GObject *source_object, GAsyncResult *result, gpointer user_data)
        {
            GError *error = NULL;
            GBytes *ssid = NULL;
            NMAccessPoint *ap = NULL;
            int strength = 0;
            std::string freq;
            guint security;
            guint32 flags, wpaFlags, rsnFlags, ap_freq;
            JsonArray ssidList = JsonArray();
            JsonObject ssidObj;
            gboolean success = nm_device_wifi_request_scan_finish(NM_DEVICE_WIFI(source_object), result, &error);
            if (success)
            {
                NMDeviceWifi *wifi_device = NM_DEVICE_WIFI(source_object);
                const GPtrArray *access_points = nm_device_wifi_get_access_points(wifi_device);
                NMLOG_INFO("Number of Access Points Scanned=%d\n",access_points->len);
                for (guint i = 0; i < access_points->len; i++)
                {
                    char* ssid_str = NULL;
                    ap = (NMAccessPoint*)access_points->pdata[i];
                    ssid = nm_access_point_get_ssid(ap);
                    if (ssid)
                    {
                        ssid_str = nm_utils_ssid_to_utf8((const guint8*)g_bytes_get_data(ssid, NULL), g_bytes_get_size(ssid));
                        strength = nm_access_point_get_strength(ap);
                        ap_freq   = nm_access_point_get_frequency(ap);
                        flags     = nm_access_point_get_flags(ap);
                        wpaFlags = nm_access_point_get_wpa_flags(ap);
                        rsnFlags = nm_access_point_get_rsn_flags(ap);
                        if (ap_freq >= 2400 && ap_freq < 5000) {
                            freq = "2.4";
                        }
                        else if (ap_freq >= 5000 && ap_freq < 6000) {
                            freq = "5";
                        }
                        else if (ap_freq >= 6000) {
                            freq = "6";
                        }
                        else {
                            freq = "Not available";
                        }
			if ((flags == NM_802_11_AP_FLAGS_NONE) && (wpaFlags == NM_802_11_AP_SEC_NONE) && (rsnFlags == NM_802_11_AP_SEC_NONE))
                        {
                            security = 0;
                        }
                        else if( (flags & NM_802_11_AP_FLAGS_PRIVACY) && ((wpaFlags & NM_802_11_AP_SEC_PAIR_WEP40) || (rsnFlags & NM_802_11_AP_SEC_PAIR_WEP40)) )
                        {
                            security = 1;
                        }
                        else if( (flags & NM_802_11_AP_FLAGS_PRIVACY) && ((wpaFlags & NM_802_11_AP_SEC_PAIR_WEP104) || (rsnFlags & NM_802_11_AP_SEC_PAIR_WEP104)) )
                        {
                            security = 2;
                        }
                        else if((wpaFlags & NM_802_11_AP_SEC_PAIR_TKIP) || (rsnFlags & NM_802_11_AP_SEC_PAIR_TKIP))
                        {
                            security = 3;
                        }
                        else if((wpaFlags & NM_802_11_AP_SEC_PAIR_CCMP) || (rsnFlags & NM_802_11_AP_SEC_PAIR_CCMP))
                        {
                            security = 4;
                        }
                        else if ((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_PSK) && (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X))
                        {
                            security = 12;
                        }
                        else if(rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_PSK)
                        {
                            security = 11;
                        }
                        else if((wpaFlags & NM_802_11_AP_SEC_GROUP_CCMP) || (rsnFlags & NM_802_11_AP_SEC_GROUP_CCMP))
                        {
                            security = 6;
                        }
                        else if((wpaFlags & NM_802_11_AP_SEC_GROUP_TKIP) || (rsnFlags & NM_802_11_AP_SEC_GROUP_TKIP))
                        {
                            security = 5;;
                        }
                        else
                        {
                            NMLOG_WARNING("security mode not defined");
                        }
		    }
                    if(ssid_str)
                    {
                        string ssidString(ssid_str);
                        ssidObj["ssid"] = ssidString;
                        ssidObj["security"] = security;
                        ssidObj["signalStrength"] = strength;
                        ssidObj["frequency"] = freq;
                        ssidList.Add(ssidObj);
                    }
                }
            }
            else
            {
                NMLOG_ERROR("Error requesting Wi-Fi scan: %s\n", error->message);
            }
            string json;
            ssidList.ToString(json);
            NMLOG_INFO("Scanned APIs are  = %s",json.c_str());
            ::_instance->ReportAvailableSSIDsEvent(json);
            g_main_loop_quit((GMainLoop *)user_data);
        }

        uint32_t NetworkManagerImplementation::StartWiFiScan(const WiFiFrequency frequency /* @in */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
	    GMainLoop *loop;
            loop = g_main_loop_new(NULL, FALSE);
            NMDevice *wifi_device;
            wifi_device = nm_client_get_device_by_iface(client, "wlan0");
            nm_device_wifi_request_scan_options_async(NM_DEVICE_WIFI(wifi_device), NULL, NULL, on_scan_done, loop);//TODO Explore further on the API and check whether w            which all options can be passed as Argument. Example : We can pass SSID as an option and scan for that SSID alone
            g_main_loop_run(loop);
            rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::StopWiFiScan(void)
        {
            uint32_t rc = Core::ERROR_NONE;
	    //TODO
            //Explore nm_device_wifi_request_scan_finish and other API which can be used stop scan
            NMLOG_INFO ("StopWiFiScan is success\n");
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetKnownSSIDs(IStringIterator*& ssids /* @out */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
	    NMClient *g_cli = nm_client_new(NULL, NULL);
            const GPtrArray *connections = nm_client_get_connections(g_cli);
            std::list<string> ssidList;
            for (guint i = 0; i < connections->len; i++)
            {
                NMConnection *connection = NM_CONNECTION(connections->pdata[i]);

                if (NM_IS_SETTING_WIRELESS(nm_connection_get_setting_wireless(connection)))
                {
                    GBytes *ssid_bytes = nm_setting_wireless_get_ssid(nm_connection_get_setting_wireless(connection));
                    if (ssid_bytes)
                    {
                        gsize ssid_size;
                        const char *ssid = (const char*)g_bytes_get_data(ssid_bytes, &ssid_size);
                        if (ssid)
                        {
                            ssidList.push_back(ssid);
                            rc = Core::ERROR_NONE;
                        }
                    }
                }
            }
            if (!ssidList.empty())
            {
                NMLOG_INFO ("GetKnownSSIDs success\n");
                ssids = Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(ssidList);
            }
            return rc;
        }

	static void add_callback(GObject *s, GAsyncResult *result, gpointer user_data)
        {

            GError *error = NULL;
            NMRemoteConnection *connection = NM_REMOTE_CONNECTION(s);

            if (!nm_remote_connection_delete_finish(connection, result, &error)) {
                NMLOG_ERROR ("AddToKnownSSIDs Failed\n");
            }
            else
            {
                NMLOG_INFO ("AddToKnownSSIDs is success\n");
            }

            g_object_unref(connection);
            g_main_loop_quit((GMainLoop *)user_data);

        }

        uint32_t NetworkManagerImplementation::AddToKnownSSIDs(const WiFiConnectTo& ssid /* @in */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
	    GMainLoop *loop;
            loop = g_main_loop_new(NULL, FALSE);
            NMClient *g_cli = nm_client_new(NULL, NULL);
            NMSettingWirelessSecurity *s_secure;
            NMSettingWireless *s_wireless;
            const char *uuid = nm_utils_uuid_generate();
            s_secure = (NMSettingWirelessSecurity *)nm_setting_wireless_security_new();
            NMSettingConnection *s_con =  (NMSettingConnection *)nm_setting_connection_new();
            g_object_set(G_OBJECT(s_con),
                 NM_SETTING_CONNECTION_UUID,
                 uuid,
                 NM_SETTING_CONNECTION_ID,
                 ssid.m_ssid.c_str(),
                 NM_SETTING_CONNECTION_TYPE,
                 "802-11-wireless",
                 NULL);
            NMConnection *connection = nm_simple_connection_new();
            nm_connection_add_setting(connection, NM_SETTING(s_con));
            s_wireless = (NMSettingWireless *)nm_setting_wireless_new();
            GString *ssid_str = g_string_new(ssid.m_ssid.c_str());
            g_object_set(G_OBJECT(s_wireless),
                 NM_SETTING_WIRELESS_SSID,
                 ssid_str,
                 NULL);
            nm_connection_add_setting(connection, NM_SETTING(s_wireless));
            s_secure = (NMSettingWirelessSecurity *)nm_setting_wireless_security_new();
	    switch(ssid.m_securityMode)
            {
                case WIFI_SECURITY_WPA_PSK_AES:
                {
                     g_object_set(G_OBJECT(s_secure),
                         NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
                         "wpa-psk",
                         NM_SETTING_WIRELESS_SECURITY_PSK,
                         ssid.m_passphrase.c_str(),
                         NULL);
                    break;
                }
                case WIFI_SECURITY_WPA2_PSK_AES:
                {
                     g_object_set(G_OBJECT(s_secure),
                         NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
                         "wpa-psk",
                         NM_SETTING_WIRELESS_SECURITY_PSK,
                         ssid.m_passphrase.c_str(),
                         NULL);
                    break;
                }
                default:
                {
                    g_object_set(G_OBJECT(s_secure),
                         NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
                         "wpa-psk",
                         NM_SETTING_WIRELESS_SECURITY_PSK,
                         ssid.m_passphrase.c_str(),
                         NULL);
                    break;
                }
            }
            nm_connection_add_setting(connection, NM_SETTING(s_secure));
            nm_client_add_connection_async(g_cli, connection, NULL, NULL, add_callback, loop);
            rc = Core::ERROR_NONE;
            g_main_loop_unref(loop);

            return rc;
        }

        uint32_t NetworkManagerImplementation::RemoveKnownSSID(const string& ssid /* @in */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
	    NMRemoteConnection* remoteConnection;
            NMClient *g_cli = nm_client_new(NULL, NULL);
            const GPtrArray* connections = nm_client_get_connections(g_cli);
            for (guint i = 0; i < connections->len; i++)
            {
                remoteConnection = NM_REMOTE_CONNECTION(connections->pdata[i]);
                NMConnection *connection = NM_CONNECTION(connections->pdata[i]);
                if (NM_IS_SETTING_WIRELESS(nm_connection_get_setting_wireless(connection)))
                {
                    GBytes *ssid_bytes = nm_setting_wireless_get_ssid(nm_connection_get_setting_wireless(connection));
                    if (ssid_bytes)
                    {
                        gsize ssid_size;
                        const char *ssid_str = (const char*)g_bytes_get_data(ssid_bytes, &ssid_size);
                        if (ssid == ssid_str)
                        {
                            GError *error = NULL;
                            nm_remote_connection_delete(remoteConnection, NULL, &error);
                            if (error)
                            {
                                NMLOG_ERROR("RemoveKnownSSID failed");
                                g_error_free(error);
                            }
                            else
                            {
                                NMLOG_INFO("RemoveKnownSSID is success");
                                rc = Core::ERROR_NONE;
                            }
                            break;
                        }
                    }
                }
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::WiFiConnect(const WiFiConnectTo& ssid /* @in */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            if(wifi->wifiConnect(ssid))
                rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::WiFiDisconnect(void)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            if(wifi->wifiDisconnect())
                rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetConnectedSSID(WiFiSSIDInfo&  ssidInfo /* @out */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            if(wifi->wifiConnectedSSIDInfo(ssidInfo))
                rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetWiFiSignalStrength(string& ssid /* @out */, string& signalStrength /* @out */, WiFiSignalQuality& quality /* @out */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
	    float signalStrengthOut = 0.0f;
	    NMClient *g_cli = nm_client_new(NULL, NULL);

            const GPtrArray *devices = nm_client_get_devices(g_cli);

            /* Go through the device array and process Wi-Fi devices */
            for (guint i = 0; i < devices->len; i++)
            {
                NMDevice *device = (NMDevice *)g_ptr_array_index(devices, i);
                if (NM_IS_DEVICE_WIFI(device))
                {
                    NMAccessPoint   *active_ap = NULL;
                    GBytes          *active_ssid;
                    char            *active_ssid_str = NULL;

                    /* Get active AP */
                    if (nm_device_get_state(device) == NM_DEVICE_STATE_ACTIVATED)
                    {
                        if ((active_ap = nm_device_wifi_get_active_access_point(NM_DEVICE_WIFI(device))))
                        {
                            active_ssid = nm_access_point_get_ssid(active_ap);
                            if (active_ssid)
                            {
                                active_ssid_str = nm_utils_ssid_to_utf8(static_cast<const guint8*>(g_bytes_get_data(active_ssid, NULL)), g_bytes_get_size(active_ssid));
                                ssid = active_ssid_str;
                            }

                            char *strength_str;
                            guint8      strength;
                            strength  = nm_access_point_get_strength(active_ap);
                            strength_str  = g_strdup_printf("%u", strength);
                            signalStrength  = strength_str;
                            g_free(strength_str);
                            if(!signalStrength.empty())
                                signalStrengthOut = std::stof(signalStrength.c_str());

                            if (signalStrengthOut == 0)
                                quality = WIFI_SIGNAL_DISCONNECTED;
                            else if (signalStrengthOut >= signalStrengthThresholdExcellent && signalStrengthOut < 0)
                                quality = WIFI_SIGNAL_EXCELLENT;
                            else if (signalStrengthOut >= signalStrengthThresholdGood && signalStrengthOut < signalStrengthThresholdExcellent)
                                quality = WIFI_SIGNAL_GOOD;
                            else if (signalStrengthOut >= signalStrengthThresholdFair && signalStrengthOut < signalStrengthThresholdGood)
                                quality = WIFI_SIGNAL_FAIR;
                            else
				quality = WIFI_SIGNAL_WEAK;

                            NMLOG_INFO ("GetWiFiSignalStrength success\n");
                            g_free(active_ssid_str);
                            rc = Core::ERROR_NONE;
                            break;
                        }
                    }
                }
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::StartWPS(const WiFiWPS& method /* @in */, const string& wps_pin /* @in */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::StopWPS(void)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetWifiState(WiFiState &state)
        {
            uint32_t rc = Core::ERROR_NONE;

            state = Exchange::INetworkManager::WIFI_STATE_CONNECTED;
            return rc;
        }
    }
}
