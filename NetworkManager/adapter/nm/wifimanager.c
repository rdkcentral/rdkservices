
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>
#include <NetworkManager.h>

/*
#include "src/core/nm-default-daemon.h"
#include "src/core/devices/wifi/nm-wifi-utils.h"
#include "devices/wifi/nm-wifi-utils.h"
#include "devices/wifi/nm-device-wifi.h"
#include "libnm-core-intern/nm-core-internal.h"
*/

typedef struct {
    const char *key;
    const char *str;
    guint32     uint;
} KeyData;

GString *ssid;
const char *password;

/*! Event states associated with WiFi connection  */
typedef enum _WiFiStatusCode_t {
    WIFI_UNINSTALLED,        /**< The device was in an installed state, and was uninstalled */
    WIFI_DISABLED,           /**< The device is installed (or was just installed) and has not yet been enabled */
    WIFI_DISCONNECTED,       /**< The device is not connected to a network */
    WIFI_PAIRING,            /**< The device is not connected to a network, but not yet connecting to a network */
    WIFI_CONNECTING,         /**< The device is attempting to connect to a network */
    WIFI_CONNECTED,          /**< The device is successfully connected to a network */
    WIFI_FAILED              /**< The device has encountered an unrecoverable error with the wifi adapter */
} WiFiStatusCode_t;

/* Convert flags to string */
static char *
ap_wpa_rsn_flags_to_string(guint32 flags)
{
    char *flags_str[16]; /* Enough space for flags and terminating NULL */
    char *ret_str;
    int   i = 0;

    if (flags & NM_802_11_AP_SEC_PAIR_WEP40)
        flags_str[i++] = g_strdup("pair_wpe40");
    if (flags & NM_802_11_AP_SEC_PAIR_WEP104)
        flags_str[i++] = g_strdup("pair_wpe104");
    if (flags & NM_802_11_AP_SEC_PAIR_TKIP)
        flags_str[i++] = g_strdup("pair_tkip");
    if (flags & NM_802_11_AP_SEC_PAIR_CCMP)
        flags_str[i++] = g_strdup("pair_ccmp");
    if (flags & NM_802_11_AP_SEC_GROUP_WEP40)
        flags_str[i++] = g_strdup("group_wpe40");
    if (flags & NM_802_11_AP_SEC_GROUP_WEP104)
        flags_str[i++] = g_strdup("group_wpe104");
    if (flags & NM_802_11_AP_SEC_GROUP_TKIP)
        flags_str[i++] = g_strdup("group_tkip");
    if (flags & NM_802_11_AP_SEC_GROUP_CCMP)
        flags_str[i++] = g_strdup("group_ccmp");
    if (flags & NM_802_11_AP_SEC_KEY_MGMT_PSK)
        flags_str[i++] = g_strdup("psk");
    if (flags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)
        flags_str[i++] = g_strdup("802.1X");
    if (flags & NM_802_11_AP_SEC_KEY_MGMT_SAE)
        flags_str[i++] = g_strdup("sae");
    if (flags & NM_802_11_AP_SEC_KEY_MGMT_OWE)
        flags_str[i++] = g_strdup("owe");
    if (flags & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM)
        flags_str[i++] = g_strdup("owe_transition_mode");
    if (flags & NM_802_11_AP_SEC_KEY_MGMT_EAP_SUITE_B_192)
        flags_str[i++] = g_strdup("wpa-eap-suite-b-192");

    if (i == 0)
        flags_str[i++] = g_strdup("none");

    flags_str[i] = NULL;

    ret_str = g_strjoinv(" ", flags_str);

    i = 0;
    while (flags_str[i])
        g_free(flags_str[i++]);

    return ret_str;
}

static void
show_active_ssid_ap_info(NMAccessPoint *ap)
{
    guint32     flags, wpa_flags, rsn_flags, freq, bitrate;
    guint8      strength;
    GBytes     *ssid;
    const char *hwaddr;
    NM80211Mode mode;
    char       *freq_str, *ssid_str, *bitrate_str, *strength_str, *wpa_flags_str, *rsn_flags_str;
    GString    *security_str;

    /* Get AP properties */
    flags     = nm_access_point_get_flags(ap);
    wpa_flags = nm_access_point_get_wpa_flags(ap);
    rsn_flags = nm_access_point_get_rsn_flags(ap);
    ssid      = nm_access_point_get_ssid(ap);
    hwaddr    = nm_access_point_get_bssid(ap);
    freq      = nm_access_point_get_frequency(ap);
    mode      = nm_access_point_get_mode(ap);
    bitrate   = nm_access_point_get_max_bitrate(ap);
    strength  = nm_access_point_get_strength(ap);

    /* Convert to strings */
    if (ssid)
        ssid_str = nm_utils_ssid_to_utf8(g_bytes_get_data(ssid, NULL), g_bytes_get_size(ssid));
    else
        ssid_str = g_strdup("--");
    freq_str      = g_strdup_printf("%u MHz", freq);
    bitrate_str   = g_strdup_printf("%u Mbit/s", bitrate / 1000);
    strength_str  = g_strdup_printf("%u", strength);
    wpa_flags_str = ap_wpa_rsn_flags_to_string(wpa_flags);
    rsn_flags_str = ap_wpa_rsn_flags_to_string(rsn_flags);

    security_str = g_string_new(NULL);
    if (!(flags & NM_802_11_AP_FLAGS_PRIVACY) && (wpa_flags != NM_802_11_AP_SEC_NONE)
        && (rsn_flags != NM_802_11_AP_SEC_NONE))
        g_string_append(security_str, "Encrypted: ");

    if ((flags & NM_802_11_AP_FLAGS_PRIVACY) && (wpa_flags == NM_802_11_AP_SEC_NONE)
        && (rsn_flags == NM_802_11_AP_SEC_NONE))
        g_string_append(security_str, "WEP ");
    if (wpa_flags != NM_802_11_AP_SEC_NONE)
        g_string_append(security_str, "WPA ");
    if ((rsn_flags & NM_802_11_AP_SEC_KEY_MGMT_PSK)
        || (rsn_flags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
        g_string_append(security_str, "WPA2 ");
    }
    if (rsn_flags & NM_802_11_AP_SEC_KEY_MGMT_SAE) {
        g_string_append(security_str, "WPA3 ");
    }
    if ((rsn_flags & NM_802_11_AP_SEC_KEY_MGMT_OWE)
        || (rsn_flags & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM)) {
        g_string_append(security_str, "OWE ");
    }
    if ((wpa_flags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)
        || (rsn_flags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
        g_string_append(security_str, "802.1X ");
    }

    if (security_str->len > 0)
        g_string_truncate(security_str, security_str->len - 1); /* Chop off last space */

    printf("SSID:       %s\n", ssid_str);
    printf("BSSID:      %s\n", hwaddr);
    printf("Mode:       %s\n",
           mode == NM_802_11_MODE_ADHOC   ? "Ad-Hoc"
           : mode == NM_802_11_MODE_INFRA ? "Infrastructure"
                                          : "Unknown");
    printf("Freq:       %s\n", freq_str);
    printf("Bitrate:    %s\n", bitrate_str);
    printf("Strength:   %s\n", strength_str);
    printf("Security:   %s\n", security_str->str);
    printf("WPA flags:  %s\n", wpa_flags_str);
    printf("RSN flags:  %s\n", rsn_flags_str);
    printf("D-Bus path: %s\n\n", nm_object_get_path(NM_OBJECT(ap)));

    g_free(ssid_str);
    g_free(freq_str);
    g_free(bitrate_str);
    g_free(strength_str);
    g_free(wpa_flags_str);
    g_free(rsn_flags_str);
    g_string_free(security_str, TRUE);
}


static void
getConnectedSSID(NMDevice *device)
{
    NMAccessPoint   *active_ap = NULL;
    const char      *iface;
    const char      *driver;
    guint32          speed;
    GBytes          *active_ssid;
    char            *active_ssid_str = NULL;

    /* Get active AP */
    if (nm_device_get_state(device) == NM_DEVICE_STATE_ACTIVATED) {
        if ((active_ap = nm_device_wifi_get_active_access_point(NM_DEVICE_WIFI(device)))) {
            active_ssid = nm_access_point_get_ssid(active_ap);
            if (active_ssid)
                active_ssid_str = nm_utils_ssid_to_utf8(g_bytes_get_data(active_ssid, NULL),
                                                        g_bytes_get_size(active_ssid));
            else
                active_ssid_str = g_strdup("--");
        }
    }

    iface  = nm_device_get_iface(device);
    driver = nm_device_get_driver(device);
    speed  = nm_device_wifi_get_bitrate(NM_DEVICE_WIFI(device));
    speed /= 1000;

    printf("Device: %s  ----  Driver: %s  ----  Speed: %d Mbit/s  ----  Active AP: %s\n",
           iface,
           driver,
           speed,
           active_ssid_str ?: "none");
    printf("=================================================================================\n");
    g_free(active_ssid_str);

    show_active_ssid_ap_info(active_ap);
}

static void
getSupportedSecurityModes() {
   int i = 0;
   NMUtilsSecurityType type;
   char *sectype[12] = {"NMU_SEC_INVALID", "NMU_SEC_NONE", "NMU_SEC_STATIC_WEP", "NMU_SEC_LEAP", "NMU_SEC_DYNAMIC_WEP", "NMU_SEC_WPA_PSK", "NMU_SEC_WPA_ENTERPRISE", "NMU_SEC_WPA2_PSK", "NMU_SEC_WPA2_ENTERPRISE", "NMU_SEC_SAE", "NMU_SEC_OWE", "NMU_SEC_WPA3_SUITE_B_192"};

   printf("SupportedSecurityModes:\n");
   for (i=0;i<12;i++) {
      printf("%s:%d\n", sectype[i], (type+i));
   }
}

static void
getCurrentState(NMDevice *device) {
    switch (nm_device_get_state(device)) {
        case NM_DEVICE_STATE_UNKNOWN:
        case NM_DEVICE_STATE_UNMANAGED:
        case NM_DEVICE_STATE_UNAVAILABLE:
        case NM_DEVICE_STATE_FAILED:
           printf("state: %d\n", WIFI_FAILED);
           break;
        case NM_DEVICE_STATE_DISCONNECTED:
           printf("state: %d\n", WIFI_DISCONNECTED);
           break;
        case NM_DEVICE_STATE_PREPARE:
        case NM_DEVICE_STATE_CONFIG:
           printf("state: %d\n", WIFI_CONNECTING);
           break;
        case NM_DEVICE_STATE_NEED_AUTH:
        case NM_DEVICE_STATE_IP_CONFIG:
        case NM_DEVICE_STATE_IP_CHECK:
        case NM_DEVICE_STATE_SECONDARIES:
           printf("state: %d\n", WIFI_PAIRING);
           break;
        case NM_DEVICE_STATE_DEACTIVATING:
           printf("state: %d\n", WIFI_DISABLED);
           break;
        case NM_DEVICE_STATE_ACTIVATED:
           printf("state: %d\n", WIFI_CONNECTED);
           break;
        default:
           printf("state: %d\n", WIFI_FAILED);

    }
}

static void
show_active_ssids(NMAccessPoint *ap)
{

    GBytes     *ssid;
    const char *hwaddr;
    char       *ssid_str;

    /* Get SSID and BSSID properties */
    ssid      = nm_access_point_get_ssid(ap);
    hwaddr    = nm_access_point_get_bssid(ap);

    /* Convert to strings */
    if (ssid)
        ssid_str = nm_utils_ssid_to_utf8(g_bytes_get_data(ssid, NULL), g_bytes_get_size(ssid));
    else
        ssid_str = g_strdup("--");


    printf("ssid:%s, bssid:%s\n", ssid_str, hwaddr);
    g_free(ssid_str);
}

static void
startScan(NMDevice *device)
{
    NMAccessPoint   *active_ap = NULL;
    const GPtrArray *aps;
    const char      *iface;
    const char      *driver;
    guint32          speed;
    GBytes          *active_ssid;
    char            *active_ssid_str = NULL;
    int              i;

    /* Get all APs of the Wi-Fi device */
    aps = nm_device_wifi_get_access_points(NM_DEVICE_WIFI(device));

    printf("Available ssid count:%d\n", aps->len);
    /* Print AP details */
    for (i = 0; i < aps->len; i++) {
        NMAccessPoint *ap = g_ptr_array_index(aps, i);
        show_active_ssids(ap);
    }
}

NMConnection *nmconnection_getclient(const char *connection_id, const char *uuid, GString *ssid, const char *password)
{
    NMConnection *connection = NULL;
    NMSettingConnection *s_con;
    NMSettingWireless *s_wireless;
    NMSettingIP4Config *s_ip4;
    NMSettingWirelessSecurity *s_secure;

    connection = nm_simple_connection_new();
    s_wireless = (NMSettingWireless *)nm_setting_wireless_new();
    s_secure = (NMSettingWirelessSecurity *)nm_setting_wireless_security_new();
    s_con = (NMSettingConnection *)nm_setting_connection_new();

    g_object_set(G_OBJECT(s_con),
                 NM_SETTING_CONNECTION_UUID,
                 uuid,
                 NM_SETTING_CONNECTION_ID,
                 connection_id,
                 NM_SETTING_CONNECTION_TYPE,
                 "802-11-wireless",
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_con));

    s_wireless = (NMSettingWireless *)nm_setting_wireless_new();

    g_object_set(G_OBJECT(s_wireless),
                 NM_SETTING_WIRELESS_SSID,
                 ssid,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_wireless));

    s_secure = (NMSettingWirelessSecurity *)nm_setting_wireless_security_new();
    g_object_set(G_OBJECT(s_secure),
                 NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
                 "wpa-psk",
                 NM_SETTING_WIRELESS_SECURITY_PSK,
                 password,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_secure));

    s_ip4 = (NMSettingIP4Config *)nm_setting_ip4_config_new();
    g_object_set(G_OBJECT(s_ip4),
                 NM_SETTING_IP_CONFIG_METHOD,
                 NM_SETTING_IP4_CONFIG_METHOD_AUTO,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_ip4));

    return connection;
}

void cb_add(GObject *client, GAsyncResult *result, gpointer user_data)
{
    NMRemoteConnection *remote;
    GError *error = NULL;

    /* NM responded to our request; either handle the resulting error or
     * print out the object path of the connection we just added.
     */
    remote = nm_client_add_connection_finish(NM_CLIENT(client), result, &error);

    if (error) {
        g_print("Error adding connection: %s", error->message);
        g_error_free(error);
    } else {
        g_print("Added: %s\n", nm_connection_get_path(NM_CONNECTION(remote)));
        g_object_unref(remote);
    }

    /* Tell the mainloop we're done and we can quit now */
    g_main_loop_quit((GMainLoop *)user_data);
}

void cb_disconnect(GObject *client, GAsyncResult *result, gpointer user_data)
{
    NMRemoteConnection *remote;
    GError *error = NULL;

    /* NM responded to our request; either handle the resulting error or
     * print out the object path of the connection we just deactivate.
     */
    remote = nm_client_deactivate_connection_finish(NM_CLIENT(client), result, &error);

    if (error) {
        g_print("Error adding connection: %s", error->message);
        g_error_free(error);
    } else {
        g_print("Added: %s\n", nm_connection_get_path(NM_CONNECTION(remote)));
        g_object_unref(remote);
    }

    /* Tell the mainloop we're done and we can quit now */
    g_main_loop_quit((GMainLoop *)user_data);
}

static void connect_wifi() {
    NMClient *client;
    GMainLoop *loop;
    GError *error = NULL;

    NMConnection *connection;
    NMAccessPoint   *active_ap = NULL;

    const char *uuid;
    loop = g_main_loop_new(NULL, FALSE);

    // Connect to NetworkManager
    client = nm_client_new(NULL, &error);
    if (!client) {
        g_message("Error: Could not connect to NetworkManager: %s.", error->message);
        g_error_free(error);
        return 1;
    }

    /* Create a new connection object */
    uuid = nm_utils_uuid_generate();
    ssid = g_string_new("linksys");
    password = "Te7@Comc@$t@2023";
    connection = nmconnection_getclient("linksys", uuid, ssid, password);

    /* Ask the settings service to add the new connection; we'll quit the
     * mainloop and exit when the callback is called.
     */
    nm_client_add_connection_async(client, connection, TRUE, NULL, cb_add, loop);
    g_object_unref(connection);

        // Wait for the connection to be added
    g_main_loop_run(loop);

    // Clean up
    g_object_unref(client);

}

static void disconnect_wifi() {
    const GPtrArray *active_conns;
    NMActiveConnection *active_conn = NULL;
    NMClient *client;
    NMRemoteConnection *newconn = NULL;
    NMSettingConnection *s_con;
    int i;
    GError *error = NULL;
    GMainLoop *loop;

    client = nm_client_new(NULL, &error);
    if (!client) {
        g_message("Error: Could not connect to NetworkManager: %s.", error->message);
        g_error_free(error);
        return 1;
    }
    active_conns = nm_client_get_active_connections(client);

    for(i=0; i<active_conns->len; i++)
    {
        active_conn = NM_ACTIVE_CONNECTION(g_ptr_array_index(active_conns, i));
        newconn = nm_active_connection_get_connection(active_conn);
        s_con = nm_connection_get_setting_connection(NM_CONNECTION(newconn));

        if (g_strcmp0(nm_setting_connection_get_connection_type(s_con), NM_SETTING_WIRELESS_SETTING_NAME) == 0) {
            nm_client_deactivate_connection_async(client, active_conn, NULL, cb_disconnect, loop);
            break;
        }
    }
    g_main_loop_run(loop);
}

static void savessid() {
    ssid = g_string_new("linksys");
    password = "Te7@Comc@$t@2023";
}

static void clearssid() {
    ssid = g_string_new("");
    password = "";
}

static void setdevice_disable(NMDevice *device) {
    GMainLoop *loop;
    loop = g_main_loop_new(NULL, FALSE);

    nm_device_disconnect_async(device, NULL, cb_add, loop);

    // Wait for the connection to be added
    g_main_loop_run(loop);
}

typedef struct _wifi_pairedSSIDInfo
{
  char  ap_ssid[64];
  char  ap_bssid[64];
  char  ap_security[64];
  char  ap_passphrase[128];
  char  ap_wep_key[128];
}wifi_pairedSSIDInfo_t;

char trimSpace(char *srcStr)
{
  char *tmpPtr1;
  char *tmpPtr2;
  for(tmpPtr2=tmpPtr1=srcStr;*tmpPtr1;tmpPtr1++)
  {
        if(!isspace(*tmpPtr1))
           *tmpPtr2++ = *tmpPtr1;
  }
  *tmpPtr2 = '\0';
  return 1;
}

int wifi_lastConnected(wifi_pairedSSIDInfo_t *pairedSSIDInfo) {
    char buf[512];
    char *saveptr = NULL;
    char *tokenKey;
    char *tokenValue;
    FILE *f = NULL;

    f = fopen("/opt/secure/wifi/wpa_supplicant.conf", "r");
    if (NULL == f)
    {
        return -1; //RETURN_ERR;
    }
    while (fgets(buf, sizeof(buf), f) != NULL)
    {
        tokenKey=strtok_r(buf,"\"=", &saveptr);
        if (tokenKey == NULL)
            continue;
        tokenValue=strtok_r(NULL,"\"=\n", &saveptr);
        trimSpace(tokenKey);
        if((tokenValue != NULL) && (strcasecmp(tokenKey,"ssid") == 0))
        {
            strcpy(pairedSSIDInfo->ap_ssid,tokenValue);
        }
        else if((tokenValue != NULL) && (strcasecmp(tokenKey,"psk") == 0))
                        {
            strcpy(pairedSSIDInfo->ap_passphrase,tokenValue);
        }
        else if((tokenValue != NULL) && (strcasecmp(tokenKey,"bssid") == 0))
        {
            strcpy(pairedSSIDInfo->ap_bssid,tokenValue);
        }
        else if((tokenValue != NULL) && (strcasecmp(tokenKey,"key_mgmt") == 0))
        {
            trimSpace(tokenValue);
            strcpy(pairedSSIDInfo->ap_security,tokenValue);
        }
        else if((tokenValue != NULL) && (strstr(tokenKey,"wep_key") != 0))
        {
           trimSpace(tokenValue);
           strcpy(pairedSSIDInfo->ap_wep_key,tokenValue);
           strcpy(pairedSSIDInfo->ap_passphrase,tokenValue);//Incase of WEP, wep_key is printed for passphrase
        }
    }
    fclose(f);

    if(pairedSSIDInfo->ap_ssid[0] == '\0') {
        return -1;//RETURN_ERR;
    }
    return 0;//RETURN_OK;
}

static void
nm_getpairedssid()
{
    NMClient        *client;
    GError          *error = NULL;
    const GPtrArray *connections;
    int              i;

    if (!(client = nm_client_new(NULL, &error))) {
        g_message("Error: Could not connect to NetworkManager: %s.", error->message);
        g_error_free(error);
        return EXIT_FAILURE;
    }

    if (!nm_client_get_nm_running(client)) {
        g_message("Error: Can't obtain connections: NetworkManager is not running.");
        return EXIT_FAILURE;
    }

    /* Now the connections can be listed. */
    connections = nm_client_get_connections(client);

    printf("***********Connections***************\n");
    printf("ssid|uuid\n\n");

    for (i = 0; i < connections->len; i++) {

                NMSettingConnection *s_con;
                guint64              timestamp;
                char                *timestamp_str;
                char                 timestamp_real_str[64];
                const char          *val1, *val2, *val3, *val4, *val5;

                s_con = nm_connection_get_setting_connection(connections->pdata[i]);
                if (s_con) {
                        struct tm localtime_data;

                        /* Get various info from NMSettingConnection and show it */
                        timestamp     = nm_setting_connection_get_timestamp(s_con);
                        timestamp_str = g_strdup_printf("%" G_GUINT64_FORMAT, timestamp);
                        strftime(timestamp_real_str,
                                         sizeof(timestamp_real_str),
                                         "%c",
                                         localtime_r((time_t *) &timestamp, &localtime_data));

                        val1 = nm_setting_connection_get_id(s_con);
                        val2 = nm_setting_connection_get_uuid(s_con);
                        val3 = nm_setting_connection_get_connection_type(s_con);
                        if(!strcmp(val3, "802-11-wireless"))
                            printf("%-25s | %s \n", val1, val2);

                        g_free(timestamp_str);
                }
        }

    g_object_unref(client);
}

int
main(int argc, char *argv[])
{
    NMClient        *client;
    const GPtrArray *devices;
    int              i;
    GError          *error = NULL;
    wifi_pairedSSIDInfo_t pairedSSIDInfo;

    /* Get NMClient object */
    client = nm_client_new(NULL, &error);
    if (!client) {
        g_message("Error: Could not create NMClient: %s.", error->message);
        g_error_free(error);
        return EXIT_FAILURE;
    }

    /* Get all devices managed by NetworkManager */
    devices = nm_client_get_devices(client);

    /* Go through the array and process Wi-Fi devices */
    for (i = 0; i < devices->len; i++) {
        NMDevice *device = g_ptr_array_index(devices, i);
        if (NM_IS_DEVICE_WIFI(device)) {
           switch(1) {
               case 1:
/*                 startScan(device);
                   getSupportedSecurityModes();
                   getCurrentState(device);
                   getConnectedSSID(device);

                   savessid();
                   clearssid();
		   nm_getpairedssid();*/
                   //setdevice_disable(device);
                   //connect_wifi();

                   //wifi_lastConnected(&pairedSSIDInfo);
                   //printf("getpaired %s:%s\n", pairedSSIDInfo.ap_ssid, pairedSSIDInfo.ap_passphrase);
                   //sleep(10);
                   //disconnect_wifi();
           }
        }
    }

    g_object_unref(client);

    return EXIT_SUCCESS;
}

