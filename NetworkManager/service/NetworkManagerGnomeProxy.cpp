#include "NetworkManagerImplementation.h"
#include <libnm/NetworkManager.h>

static NMClient *client;

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace std;

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
            return;
        }

        uint32_t NetworkManagerImplementation::GetAvailableInterfaces (Exchange::INetworkManager::IInterfaceDetailsIterator*& interfacesItr/* @out */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            const GPtrArray *devices = nm_client_get_devices(client);
            if (devices->len != 0) {
                std::vector<InterfaceDetails> interfaceList;
                for (guint i = 0; i < devices->len; i++) {
                    InterfaceDetails tmp;
                    NMDevice *device = (NMDevice *)g_ptr_array_index(devices, i); 
                    const char *interfaceName = nm_device_get_iface(device);
                    if (strcmp("enp0s3", interfaceName) == 0)
                    {
                        tmp.m_type = string("ETHERNET");
                    }
                    else if ("wlan0" == interfaceName)
                        tmp.m_type = string("WIFI");
                    tmp.m_name = interfaceName;
                    tmp.m_mac = nm_device_get_hw_address(device);
                    NMDeviceState state = nm_device_get_state(device);
                    tmp.m_isEnabled = (state > NM_DEVICE_STATE_UNAVAILABLE) ? 1 : 0;
                    NMConnectivityState connectivity  = nm_device_get_connectivity (device, AF_INET);
                    tmp.m_isConnected = (connectivity > NM_CONNECTIVITY_LIMITED) ? 1: 0;
                    interfaceList.push_back(tmp);

                    using Implementation = RPC::IteratorType<Exchange::INetworkManager::IInterfaceDetailsIterator>;
                    interfacesItr = Core::Service<Implementation>::Create<Exchange::INetworkManager::IInterfaceDetailsIterator>(interfaceList);

                    rc = Core::ERROR_NONE;
                }
            }
            else   
            {
                printf("Call to %s for %s failed", "nm_client_get_devices", __FUNCTION__);
            }
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
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::SetInterfaceEnabled (const string& interface/* @in */, const bool& isEnabled /* @in */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        } 

        /* @brief Get IP Address Of the Interface */
        uint32_t NetworkManagerImplementation::GetIPSettings(const string& interface /* @in */, const string& ipversion /* @in */, IPAddressInfo& result /* @out */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            GError *error = NULL;
            NMActiveConnection *active_conn = NULL;
            NMIPConfig *ip4_config = NULL;
            NMIPConfig *ip6_config = NULL;
            const gchar *gateway = NULL;
            char **dns_arr = NULL;
            NMDhcpConfig *dhcp4_config = NULL;
            NMDhcpConfig *dhcp6_config = NULL;
            GHashTable * ght ;
            const char* dhcpserver;
            NMSettingConnection *settings;

            const GPtrArray *connections = nm_client_get_active_connections(client);


            for (guint i = 0; i < connections->len; i++) {
                NMActiveConnection *connection = NM_ACTIVE_CONNECTION(connections->pdata[i]);
                settings = nm_connection_get_setting_connection(NM_CONNECTION(nm_active_connection_get_connection(connection)));

                /* Check if the interface name matches */
                if (g_strcmp0(nm_setting_connection_get_interface_name(settings), interface.c_str()) == 0) {
                    active_conn = connection;
                    break;
                }
            }
            if (active_conn == NULL) {
                fprintf(stderr, "Error getting primary connection: %s\n", error->message);
                g_error_free(error);
                return 1;
            }   

            if(0 == strcmp(ipversion.c_str(), "IPv4"))
            {
                ip4_config = nm_active_connection_get_ip4_config(active_conn);
                if (ip4_config != NULL) {
                    const GPtrArray *p; 
                    int              i;
                    p = nm_ip_config_get_addresses(ip4_config);
                    for (i = 0; i < p->len; i++) {
                        NMIPAddress *a = static_cast<NMIPAddress*>(p->pdata[i]);
                        g_print("\tinet4 %s/%d, %s \n", nm_ip_address_get_address(a), nm_ip_address_get_prefix(a), PREFIX_TO_NETMASK( nm_ip_address_get_prefix(a)));
                    }
                    gateway = nm_ip_config_get_gateway(ip4_config);
                }   
                dns_arr =   (char **)nm_ip_config_get_nameservers(ip4_config);

                dhcp4_config = nm_active_connection_get_dhcp4_config(active_conn);
                ght = nm_dhcp_config_get_options(dhcp4_config);
                dhcpserver = nm_dhcp_config_get_one_option (dhcp4_config,
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

                rc = Core::ERROR_NONE;
            }
            else if(0 == strcmp(ipversion.c_str(), "IPv6"))
            {
                NMIPAddress *a;
                ip6_config = nm_active_connection_get_ip6_config(active_conn);
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

                    dhcp6_config = nm_active_connection_get_dhcp6_config(active_conn);
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

        /* @brief Set IP Address Of the Interface */
        uint32_t NetworkManagerImplementation::SetIPSettings(const string& interface /* @in */, const string &ipversion /* @in */, const IPAddressInfo& address /* @in */)
        {
            uint32_t rc = Core::ERROR_NONE;
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
