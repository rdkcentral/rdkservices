#include "NetworkManagerImplementation.h"
#include <libnm/NetworkManager.h>
#include <fstream>
#include <sstream>

static NMClient *client;

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace std;
GMainLoop *loop;
static std::vector<Exchange::INetworkManager::InterfaceDetails> interfaceList;

#define PREFIX_TO_NETMASK(prefix_len) ({ \
    static char netmask_str[16]; \
    uint32_t netmask = 0xffffffff << (32 - (prefix_len)); \
    snprintf(netmask_str, 16, "%u.%u.%u.%u", \
             (netmask >> 24) & 0xff, \
             (netmask >> 16) & 0xff, \
             (netmask >> 8) & 0xff, \
             netmask & 0xff); \
    netmask_str; \
})

namespace WPEFramework
{
    namespace Plugin
    {
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
            GError *error = NULL;
            // initialize the NMClient object
            client = nm_client_new(NULL, &error);
            if (client == NULL) {
                fprintf(stderr, "Error initializing NMClient: %s\n", error->message);
                g_error_free(error);
                return;
            }
            loop = g_main_loop_new(NULL, FALSE);
            return;
        }

        uint32_t GetInterfacesName(string (&interfaces)[2]) {
            string line;
            string wifi_interface;
            string ethernet_interface;
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;

            ifstream file("/etc/device.properties");
            if (!file.is_open()) {
                std::cerr << "Error opening file" << std::endl;
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
                    std::getline(iss, wifi_interface, '=');
                } else if (token == "ETHERNET_INTERFACE") {
                    std::getline(iss, ethernet_interface, '=');
                }

                if (!wifi_interface.empty() && !ethernet_interface.empty()) {
                    rc = Core::ERROR_NONE;           
                    break;
                }
            }   

            interfaces[0] = wifi_interface;
            interfaces[1] = ethernet_interface;
            file.close();

            return rc;
        }

        uint32_t NetworkManagerImplementation::GetAvailableInterfaces (Exchange::INetworkManager::IInterfaceDetailsIterator*& interfacesItr/* @out */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            string interfaces[2];
            GetInterfacesName(interfaces);
            NMDeviceType type;
            InterfaceDetails tmp;
            NMDeviceState state;
            NMDevice *device;
            NMConnectivityState connectivity;

            if(interfaceList.empty() || interfaceList.size() != 2)
            {
                for (auto &element : interfaces) {
                    device = nm_client_get_device_by_iface(client, element.c_str());
                    if (device)
                    {
                        type = nm_device_get_device_type(device);
                        switch (type) {
                            case NM_DEVICE_TYPE_ETHERNET:
                                tmp.m_type = string("ETHERNET");
                                break;
                            case NM_DEVICE_TYPE_WIFI:
                                tmp.m_type = string("WIFI");
                                break;
                        }
                        tmp.m_name = element.c_str();
                        tmp.m_mac = nm_device_get_hw_address(device);
                        state = nm_device_get_state(device);
                        tmp.m_isEnabled = (state > NM_DEVICE_STATE_UNAVAILABLE) ? 1 : 0;
                        connectivity = nm_device_get_connectivity (device, AF_INET);
                        tmp.m_isConnected = (connectivity > NM_CONNECTIVITY_LIMITED) ? 1: 0;
                        interfaceList.push_back(tmp);
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
            NMActiveConnection *active_conn = NULL;
            NMRemoteConnection *newconn = NULL;
            NMSettingConnection *s_con;

            active_conn = nm_client_get_primary_connection(client);
            if (active_conn == NULL) {
                fprintf(stderr, "Error getting primary connection: %s\n", error->message);
                g_error_free(error);
                return rc;
            }   
            newconn = nm_active_connection_get_connection(active_conn);
            s_con = nm_connection_get_setting_connection(NM_CONNECTION(newconn));

            interface = nm_connection_get_interface_name(NM_CONNECTION(newconn));
            if(interface.c_str() != NULL)
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
            NMRemoteConnection *remote_connection;
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
            remote_connection = nm_client_get_connection_by_uuid(client, uuid);
            nm_remote_connection_commit_changes(remote_connection, false, NULL, NULL);

            return rc;
        }

        uint32_t NetworkManagerImplementation::SetInterfaceEnabled (const string& interface/* @in */, const bool& isEnabled /* @in */)
        {
            uint32_t rc = Core::ERROR_NONE;
            const GPtrArray *devices = nm_client_get_devices(client);
            NMDeviceState new_state;
            NMDevice *device;

            if(isEnabled)
                new_state = NM_DEVICE_STATE_ACTIVATED;
            else
                new_state = NM_DEVICE_STATE_DEACTIVATING;
            for (guint i = 0; i < devices->len; ++i) {
                device = NM_DEVICE(g_ptr_array_index(devices, i));

                // Get the device details
                const char *name = nm_device_get_iface(device);

                // Check if the device name matches
                if (g_strcmp0(name, interface.c_str()) == 0) {
                    nm_device_set_managed(device, isEnabled);

                    g_print("Interface %s status set to %s\n",
                            interface.c_str(),
                            (new_state == NM_DEVICE_STATE_ACTIVATED) ? "enabled" : "disabled");
                }
            }

            // Cleanup
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
            GHashTable * ght ;
            const char* dhcpserver;
            NMSettingConnection *settings;
            NMIPAddress *a;

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
                fprintf(stderr, "Error getting primary connection: %s\n", error->message);
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
                        a = static_cast<NMIPAddress*>(p->pdata[i]);
                        g_print("\tinet4 %s/%d, %s \n", nm_ip_address_get_address(a), nm_ip_address_get_prefix(a), PREFIX_TO_NETMASK( nm_ip_address_get_prefix(a)));
                    }
                    gateway = nm_ip_config_get_gateway(ip4_config);
                }   
                dns_arr =   (char **)nm_ip_config_get_nameservers(ip4_config);

                dhcp4_config = nm_active_connection_get_dhcp4_config(conn);
                ght = nm_dhcp_config_get_options(dhcp4_config);
                dhcpserver = nm_dhcp_config_get_one_option (dhcp4_config,
                               "dhcp_server_identifier");

                result.m_ipAddrType     = ipversion.c_str();
                if(dhcpserver)
                    result.m_dhcpServer     = dhcpserver;
                result.m_v6LinkLocal    = "";
                result.m_ipAddress      = nm_ip_address_get_address(a);
                result.m_prefix         = nm_ip_address_get_prefix(a);
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
                        g_print("\tinet6 %s/%d\n", nm_ip_address_get_address(a), nm_ip_address_get_prefix(a));
                    }
                    gateway = nm_ip_config_get_gateway(ip6_config);

                    dns_arr =   (char **)nm_ip_config_get_nameservers(ip6_config);

                    dhcp6_config = nm_active_connection_get_dhcp6_config(conn);
                    ght = nm_dhcp_config_get_options(dhcp6_config);
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

#define CIDR_NETMASK_IP_LEN 32
const string CIDR_PREFIXES[CIDR_NETMASK_IP_LEN] = {
                                                     "128.0.0.0",
                                                     "192.0.0.0",
                                                     "224.0.0.0",
                                                     "240.0.0.0",
                                                     "248.0.0.0",
                                                     "252.0.0.0",
                                                     "254.0.0.0",
                                                     "255.0.0.0",
                                                     "255.128.0.0",
                                                     "255.192.0.0",
                                                     "255.224.0.0",
                                                     "255.240.0.0",
                                                     "255.248.0.0",
                                                     "255.252.0.0",
                                                     "255.254.0.0",
                                                     "255.255.0.0",
                                                     "255.255.128.0",
                                                     "255.255.192.0",
                                                     "255.255.224.0",
                                                     "255.255.240.0",
                                                     "255.255.248.0",
                                                     "255.255.252.0",
                                                     "255.255.254.0",
                                                     "255.255.255.0",
                                                     "255.255.255.128",
                                                     "255.255.255.192",
                                                     "255.255.255.224",
                                                     "255.255.255.240",
                                                     "255.255.255.248",
                                                     "255.255.255.252",
                                                     "255.255.255.254",
                                                     "255.255.255.255",
                                                   };


            // Callback for nm_client_deactivate_connection_async
            static void on_deactivate_complete(GObject *source_object, GAsyncResult *res, gpointer user_data) {
                GError *error = NULL;

                // Check if the operation was successful
                if (!nm_client_deactivate_connection_finish(NM_CLIENT(source_object), res, &error)) {
                    g_print("Deactivating connection failed: %s\n", error->message);
                    g_error_free(error);
                } else {
                    g_print("Deactivating connection successful\n");
                }
            }

            // Callback for nm_client_activate_connection_async
            static void on_activate_complete(GObject *source_object, GAsyncResult *res, gpointer user_data) {
                GError *error = NULL;

                // Check if the operation was successful
                if (!nm_client_activate_connection_finish(NM_CLIENT(source_object), res, &error)) {
                    g_print("Activating connection failed: %s\n", error->message);
                    g_error_free(error);
                } else {
                    g_print("Activating connection successful\n");
                }

                g_main_loop_quit((GMainLoop*)user_data);
            }

            static void modify_connection_cb(GObject *connection, GAsyncResult *result, gpointer user_data)
            {
                GError *error = NULL;

                if (!nm_remote_connection_commit_changes_finish(NM_REMOTE_CONNECTION(connection),
                            result,
                            &error)) {
                    g_print("nm_remote_connection_commit_changes_finish failed: %s\n", error->message);
                } else {
                    g_print(("Connection '%s' (%s) successfully modified.\n"),
                            nm_connection_get_id(NM_CONNECTION(connection)),
                            nm_connection_get_uuid(NM_CONNECTION(connection)));
                } 
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
            NMDevice   *device      = NULL;
            const char  *spec_object;

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
                    //FIXME : Add IPv6 support here
                    printf("Setting IPv6 is not supported at this point in time. This is just a place holder\n");
                    rc = Core::ERROR_NOT_SUPPORTED;

                    /*s_ip6 = (NMSettingIP6Config *)nm_setting_ip6_config_new();
                    g_object_set(G_OBJECT(s_ip6), NM_SETTING_IP_CONFIG_METHOD, NM_SETTING_IP6_CONFIG_METHOD_AUTO, NULL);
                    nm_connection_add_setting(conn, NM_SETTING(s_ip6));*/
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
            nm_client_activate_connection_async(client, conn, device, spec_object, NULL, on_activate_complete, loop);
            g_main_loop_run(loop);
            //g_main_loop_unref(loop);
            return rc;
        }

        uint32_t NetworkManagerImplementation::StartWiFiScan(const WiFiFrequency frequency /* @in */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::StopWiFiScan(void)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetKnownSSIDs(IStringIterator*& ssids /* @out */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::AddToKnownSSIDs(const WiFiConnectTo& ssid /* @in */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::RemoveKnownSSID(const string& ssid /* @in */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::WiFiConnect(const WiFiConnectTo& ssid /* @in */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::WiFiDisconnect(void)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetConnectedSSID(WiFiSSIDInfo&  ssidInfo /* @out */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetWiFiSignalStrength(string& ssid /* @out */, string& signalStrength /* @out */, WiFiSignalQuality& quality /* @out */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
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
    }
}
