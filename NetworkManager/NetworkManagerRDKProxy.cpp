#include "NetworkManagerImplementation.h"
#include "NetworkManagerConnectivity.h"
#include "WiFiSignalStrengthMonitor.h"
#include "libIBus.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace std;

namespace WPEC = WPEFramework::Core;
namespace WPEJ = WPEFramework::Core::JSON;

#define IARM_BUS_NM_SRV_MGR_NAME "NET_SRV_MGR"

#define MAX_IP_ADDRESS_LEN          46
#define NETSRVMGR_INTERFACES_MAX    16
#define MAX_ENDPOINTS                5
#define MAX_ENDPOINT_SIZE          512
#define MAX_URI_LEN                512
#define MAX_HOST_NAME_LEN          128
#define DEFAULT_PING_PACKETS        15
#define CIDR_NETMASK_IP_LEN         32
#define INTERFACE_SIZE              10
#define INTERFACE_LIST              50
#define MAX_IP_FAMILY_SIZE          10


#define BUFF_LENGTH_64              65
#define BUFF_LENGTH_256            257
#define BUFF_MAX                  1025
#define BUFF_MAC                    18
#define BUFF_MIN                    17
#define BUFF_LENGTH_4                4
#define SSID_SIZE                   33
#define BSSID_BUFF                  20
#define BUFF_LENGTH_24              24
#define PASSPHRASE_BUFF            385
#define MAX_SSIDLIST_BUF     (48*1024)
#define MAX_FILE_PATH_LEN         4096

typedef enum _NetworkManager_EventId_t {
    IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_ENABLED=50,
    IARM_BUS_NETWORK_MANAGER_EVENT_SET_INTERFACE_CONTROL_PERSISTENCE,
    IARM_BUS_NETWORK_MANAGER_EVENT_WIFI_INTERFACE_STATE,
    IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_ENABLED_STATUS,
    IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_CONNECTION_STATUS,
    IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_IPADDRESS,
    IARM_BUS_NETWORK_MANAGER_EVENT_DEFAULT_INTERFACE,
    IARM_BUS_NETWORK_MANAGER_EVENT_INTERNET_CONNECTION_CHANGED,
    IARM_BUS_NETWORK_MANAGER_MAX,
} IARM_Bus_NetworkManager_EventId_t;

typedef enum _IARM_Bus_NMgr_WiFi_EventId_t {
    IARM_BUS_WIFI_MGR_EVENT_onWIFIStateChanged = 1,
    IARM_BUS_WIFI_MGR_EVENT_onError,
    IARM_BUS_WIFI_MGR_EVENT_onSSIDsChanged,
    IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDs,
    IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDsIncr,
    IARM_BUS_WIFI_MGR_EVENT_onWIFILNFStateChanged,
    IARM_BUS_WIFI_MGR_EVENT_MAX,                /*!< Maximum event id*/
} IARM_Bus_NMgr_WiFi_EventId_t;


typedef struct {
    char name[16];
    char mac[20];
    unsigned int flags;
} NetSrvMgr_Interface_t;

typedef struct {
    unsigned char         size;
    NetSrvMgr_Interface_t interfaces[NETSRVMGR_INTERFACES_MAX];
} IARM_BUS_NetSrvMgr_InterfaceList_t;

typedef enum _NetworkManager_GetIPSettings_ErrorCode_t
{
  NETWORK_IPADDRESS_ACQUIRED,
  NETWORK_IPADDRESS_NOTFOUND,
  NETWORK_NO_ROUTE_INTERFACE,
  NETWORK_NO_DEFAULT_ROUTE,
  NETWORK_DNS_NOT_CONFIGURED,
  NETWORK_INVALID_IPADDRESS,
} NetworkManager_GetIPSettings_ErrorCode_t;

typedef struct {
    char interface[16];
    char ipversion[16];
    bool autoconfig;
    char ipaddress[MAX_IP_ADDRESS_LEN];
    char netmask[MAX_IP_ADDRESS_LEN];
    char gateway[MAX_IP_ADDRESS_LEN];
    char dhcpserver[MAX_IP_ADDRESS_LEN];
    char primarydns[MAX_IP_ADDRESS_LEN];
    char secondarydns[MAX_IP_ADDRESS_LEN];
    bool isSupported;
    NetworkManager_GetIPSettings_ErrorCode_t errCode;
} IARM_BUS_NetSrvMgr_Iface_Settings_t;

typedef struct {
    char interface[16];
    char gateway[MAX_IP_ADDRESS_LEN];
} IARM_BUS_NetSrvMgr_DefaultRoute_t;

typedef struct {
    char interface[16];
    bool status;
} IARM_BUS_NetSrvMgr_Iface_EventInterfaceStatus_t;

typedef IARM_BUS_NetSrvMgr_Iface_EventInterfaceStatus_t IARM_BUS_NetSrvMgr_Iface_EventInterfaceEnabledStatus_t;
typedef IARM_BUS_NetSrvMgr_Iface_EventInterfaceStatus_t IARM_BUS_NetSrvMgr_Iface_EventInterfaceConnectionStatus_t;

typedef struct {
    char interface[16];
    char ip_address[MAX_IP_ADDRESS_LEN];
    bool is_ipv6;
    bool acquired;
} IARM_BUS_NetSrvMgr_Iface_EventInterfaceIPAddress_t;

typedef struct {
    char oldInterface[16];
    char newInterface[16];
} IARM_BUS_NetSrvMgr_Iface_EventDefaultInterface_t;

typedef struct
{
    char server[MAX_HOST_NAME_LEN];
    uint16_t port;
    bool ipv6;
    char interface[16];
    uint16_t bind_timeout;
    uint16_t cache_timeout;
    bool sync;
    char public_ip[MAX_IP_ADDRESS_LEN];
} IARM_BUS_NetSrvMgr_Iface_StunRequest_t;

typedef struct
{
    bool disableConnectivityTest;
} IARM_BUS_NetSrvMgr_configurePNI_t;



/*! Supported values are NONE - 0, WPA - 1, WEP - 2*/
typedef enum _SsidSecurity
{
    NET_WIFI_SECURITY_NONE = 0,
    NET_WIFI_SECURITY_WEP_64,
    NET_WIFI_SECURITY_WEP_128,
    NET_WIFI_SECURITY_WPA_PSK_TKIP,
    NET_WIFI_SECURITY_WPA_PSK_AES,
    NET_WIFI_SECURITY_WPA2_PSK_TKIP,
    NET_WIFI_SECURITY_WPA2_PSK_AES,
    NET_WIFI_SECURITY_WPA_ENTERPRISE_TKIP,
    NET_WIFI_SECURITY_WPA_ENTERPRISE_AES,
    NET_WIFI_SECURITY_WPA2_ENTERPRISE_TKIP,
    NET_WIFI_SECURITY_WPA2_ENTERPRISE_AES,
    NET_WIFI_SECURITY_WPA_WPA2_PSK,
    NET_WIFI_SECURITY_WPA_WPA2_ENTERPRISE,
    NET_WIFI_SECURITY_WPA3_PSK_AES,
    NET_WIFI_SECURITY_WPA3_SAE,
    NET_WIFI_SECURITY_NOT_SUPPORTED = 99,
} SsidSecurity;


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

/*! Error code: A recoverable, unexpected error occurred,
 * as defined by one of the following values */
typedef enum _WiFiErrorCode_t {
    WIFI_SSID_CHANGED,              /**< The SSID of the network changed */
    WIFI_CONNECTION_LOST,           /**< The connection to the network was lost */
    WIFI_CONNECTION_FAILED,         /**< The connection failed for an unknown reason */
    WIFI_CONNECTION_INTERRUPTED,    /**< The connection was interrupted */
    WIFI_INVALID_CREDENTIALS,       /**< The connection failed due to invalid credentials */
    WIFI_NO_SSID,                   /**< The SSID does not exist */
    WIFI_UNKNOWN,                   /**< Any other error */
    WIFI_AUTH_FAILED                /**< The connection failed due to auth failure */
} WiFiErrorCode_t;

typedef enum _WiFiLNFState_t {
    LNF_SSID_CONNECTED,                    /**< Connected to the LNF network */
    LNF_SSID_CONNECT_FAILED,               /**< failed connect to the LNF network */
    LNF_ENDPOINT_BACKOFF_TIME,             /**< lnf in last requested backoff time */
    LNF_ENDPOINT_CONNECTED,                /**< lnf connected to end point and get the LFAT */
    LNF_RECEIVED_PRIVATE_SSID_INFO,        /**< lnf process Got private cridential*/
    LNF_SWITCHING_TO_PRIVATE,              /**< lnf connected to private*/
    LNF_SSID_DISCONNECTED,                 /**< disconnected form lnf ssid */
    LNF_ERROR                              /**< common lnf error replay from lnf library */
} WiFiLNFState_t;

typedef struct _IARM_Bus_WiFiSrvMgr_WPS_Parameters_t
{
    bool pbc;
    char pin[9];
    bool status;
} IARM_Bus_WiFiSrvMgr_WPS_Parameters_t;


/*! LNF states  */
typedef enum _WiFiLNFStatusCode_t {
    LNF_UNITIALIZED,                       /**< Network manager hasn't started the LNF process */
    LNF_IN_PROGRESS,                       /**< Network manager has started LNF, and waiting for operation to complete */
    CONNECTED_LNF,                         /**< Connected to the LNF network */
    CONNECTED_PRIVATE,                     /** Connected to a network that is not LNF */
    DISCONNECTED_NO_LNF_GATEWAY_DETECTED,  /**< unable to connect to LNF network */
    DISCONNECTED_GET_LFAT_FAILED,          /**< client wasn't able to acquire an LFAT */
    DISCONNECTED_CANT_CONNECT_TO_PRIVATE // client could obtain LFAT, but couldn't connect to private network */
                    /**< The device has encountered an unrecoverable error with the wifi adapter */
} WiFiLNFStatusCode_t;


typedef struct _setWiFiAdapter
{
    bool enable;
} setWiFiAdapter;

typedef struct _WiFiConnection
{
    char ssid[SSID_SIZE];
    char bssid[BSSID_BUFF];
    char security[BUFF_LENGTH_64];
    char passphrase[PASSPHRASE_BUFF];
    SsidSecurity security_mode;
    char security_WEPKey[PASSPHRASE_BUFF];
    char security_PSK[PASSPHRASE_BUFF];
    char eapIdentity[BUFF_LENGTH_256];
    char carootcert[MAX_FILE_PATH_LEN];
    char clientcert[MAX_FILE_PATH_LEN];
    char privatekey[MAX_FILE_PATH_LEN];
} WiFiConnection;

typedef struct _WiFiConnectedSSIDInfo
{
    char ssid[SSID_SIZE];     /**< The name of connected SSID. */
    char bssid[BSSID_BUFF];   /**< The the Basic Service Set ID (mac address). */
    char band[BUFF_MIN];      /**< The frequency band at which the client is conneted to. */
    int securityMode;         /**< Current WiFi Security Mode used for connection. */
    int  frequency;                    /**< The Frequency wt which the client is connected to. */
    float rate;               /**< The Physical data rate in Mbps */
    float noise;              /**< The average noise strength in dBm. */
    float signalStrength;     /**< The RSSI value in dBm. */

} WiFiConnectedSSIDInfo_t;

typedef struct _WiFiPairedSSIDInfo
{
    char ssid[SSID_SIZE];      /**< The name of connected SSID. */
    char bssid[BSSID_BUFF];    /**< The the Basic Service Set ID (mac address). */
    char security[BUFF_LENGTH_64];    /**< security of AP */
    SsidSecurity secMode;
} WiFiPairedSSIDInfo_t;


typedef enum _WiFiConnectionTypeCode_t {
    WIFI_CON_UNKNOWN,
    WIFI_CON_WPS,
    WIFI_CON_MANUAL,
    WIFI_CON_LNF,
    WIFI_CON_PRIVATE,
    WIFI_CON_MAX,
} WiFiConnectionTypeCode_t;


typedef struct _IARM_Bus_WiFiSrvMgr_Param_t {
    union {
        WiFiLNFStatusCode_t wifiLNFStatus;
        WiFiStatusCode_t wifiStatus;
        setWiFiAdapter setwifiadapter;
        WiFiConnection connect;
        WiFiConnection saveSSID;
        WiFiConnection clearSSID;
        WiFiConnectedSSIDInfo_t getConnectedSSID;
        WiFiPairedSSIDInfo_t getPairedSSIDInfo;
        WiFiConnectionTypeCode_t connectionType;
        struct getPairedSSID {
            char ssid[SSID_SIZE];
        } getPairedSSID;
        bool isPaired;
    } data;
    bool status;
} IARM_Bus_WiFiSrvMgr_Param_t;

typedef struct _IARM_BUS_NetSrvMgr_Iface_EventData_t {
   union {
        char activeIface[INTERFACE_SIZE];
        char allNetworkInterfaces[INTERFACE_LIST];
        char setInterface[INTERFACE_SIZE];
        char activeIfaceIpaddr[MAX_IP_ADDRESS_LEN];
        };
   char interfaceCount;
   bool isInterfaceEnabled;
   bool persist;
   char ipfamily[MAX_IP_FAMILY_SIZE];
} IARM_BUS_NetSrvMgr_Iface_EventData_t;

typedef struct _IARM_BUS_WiFiSrvMgr_EventData_t {
    union {
        struct _WIFI_STATECHANGE_DATA {
            WiFiStatusCode_t state;
        } wifiStateChange;
        struct _WIFI_ERROR {
            WiFiErrorCode_t code;
        } wifiError;
        struct _WIFI_SSID_LIST {
            char ssid_list[MAX_SSIDLIST_BUF];
            bool more_data;
        } wifiSSIDList;
        struct _WIFI_LNF_STATE {
            WiFiLNFState_t state;
            float backOffTime;
        }wifilnfState;
    } data;
} IARM_BUS_WiFiSrvMgr_EventData_t;

typedef struct _wifiSsidData_t {
    char jdata[MAX_SSIDLIST_BUF];    /**< Buffer containing the serialized data. */
    size_t jdataLen;                 /**< Length of the data buffer. */
} wifiSsidData_t;

typedef struct _IARM_Bus_WiFiSrvMgr_SsidList_Param_t {
    wifiSsidData_t curSsids;
    bool status;
} IARM_Bus_WiFiSrvMgr_SsidList_Param_t;


#define IARM_BUS_NETSRVMGR_API_getInterfaceList             "getInterfaceList"
#define IARM_BUS_NETSRVMGR_API_getDefaultInterface          "getDefaultInterface"
#define IARM_BUS_NETSRVMGR_API_setDefaultInterface          "setDefaultInterface"
#define IARM_BUS_NETSRVMGR_API_isInterfaceEnabled           "isInterfaceEnabled"
#define IARM_BUS_NETSRVMGR_API_setInterfaceEnabled          "setInterfaceEnabled"
#define IARM_BUS_NETSRVMGR_API_setIPSettings                "setIPSettings"
#define IARM_BUS_NETSRVMGR_API_getIPSettings                "getIPSettings"
#define IARM_BUS_NETSRVMGR_API_isAvailable                  "isAvailable"
#define IARM_BUS_WIFI_MGR_API_getAvailableSSIDsAsync        "getAvailableSSIDsAsync"        /**< Retrieve array of strings representing SSIDs */
#define IARM_BUS_WIFI_MGR_API_getAvailableSSIDsAsyncIncr    "getAvailableSSIDsAsyncIncr"    /**< Retrieve array of strings representing SSIDs in an incremental way */
#define IARM_BUS_WIFI_MGR_API_stopProgressiveWifiScanning   "stopProgressiveWifiScanning"   /**< Stop any in-progress wifi progressive scanning thread */
#define IARM_BUS_WIFI_MGR_API_initiateWPSPairing2           "initiateWPSPairing2"           /**< Initiate connection via WPS via either Push Button or PIN */
#define IARM_BUS_WIFI_MGR_API_cancelWPSPairing              "cancelWPSPairing"              /**< Cancel in-progress WPS */
#define IARM_BUS_WIFI_MGR_API_getConnectedSSID              "getConnectedSSID"              /**< Return properties of the currently connected SSID */
#define IARM_BUS_WIFI_MGR_API_saveSSID                      "saveSSID"                      /**< Save SSID and passphrase */
#define IARM_BUS_WIFI_MGR_API_clearSSID                     "clearSSID"                     /**< Clear given SSID */
#define IARM_BUS_WIFI_MGR_API_connect                       "connect"                       /**< Connect with given or saved SSID and passphrase */
#define IARM_BUS_WIFI_MGR_API_disconnectSSID                "disconnectSSID"                /**< Disconnect from current SSID */
#define IARM_BUS_WIFI_MGR_API_getCurrentState               "getCurrentState"               /**< Retrieve current state */


namespace WPEFramework
{
    namespace Plugin
    {
        const float signalStrengthThresholdExcellent = -50.0f;
        const float signalStrengthThresholdGood = -60.0f;
        const float signalStrengthThresholdFair = -67.0f;
        NetworkManagerImplementation* _instance = nullptr;

        Exchange::INetworkManager::WiFiState to_wifi_state(WiFiStatusCode_t code) {
            switch (code)
            {
                case WIFI_DISCONNECTED:
                    return Exchange::INetworkManager::WIFI_STATE_DISCONNECTED;
                case WIFI_PAIRING:
                    return Exchange::INetworkManager::WIFI_STATE_PAIRING;
                case WIFI_CONNECTING:
                    return Exchange::INetworkManager::WIFI_STATE_CONNECTING;
                case WIFI_CONNECTED:
                    return Exchange::INetworkManager::WIFI_STATE_CONNECTED;
                case WIFI_FAILED:
                    return Exchange::INetworkManager::WIFI_STATE_CONNECTION_FAILED;
                case WIFI_UNINSTALLED:
                    return Exchange::INetworkManager::WIFI_STATE_UNINSTALLED;
                case WIFI_DISABLED:
                    return Exchange::INetworkManager::WIFI_STATE_DISABLED;
            }
            return Exchange::INetworkManager::WIFI_STATE_CONNECTION_FAILED;
        }

        Exchange::INetworkManager::WiFiState errorcode_to_wifi_state(WiFiErrorCode_t code) {
            switch (code)
            {
                case WIFI_SSID_CHANGED:
                    return Exchange::INetworkManager::WIFI_STATE_SSID_CHANGED;
                case WIFI_CONNECTION_LOST:
                    return Exchange::INetworkManager::WIFI_STATE_CONNECTION_LOST;
                case WIFI_CONNECTION_INTERRUPTED:
                    return Exchange::INetworkManager::WIFI_STATE_CONNECTION_INTERRUPTED;
                case WIFI_INVALID_CREDENTIALS:
                    return Exchange::INetworkManager::WIFI_STATE_INVALID_CREDENTIALS;
                case WIFI_AUTH_FAILED:
                    return Exchange::INetworkManager::WIFI_STATE_AUTHENTICATION_FAILED;
		case WIFI_NO_SSID:
		    return Exchange::INetworkManager::WIFI_STATE_SSID_NOT_FOUND;
                case WIFI_UNKNOWN:
                    return Exchange::INetworkManager::WIFI_STATE_ERROR;
            }
            return Exchange::INetworkManager::WIFI_STATE_CONNECTION_FAILED;
        }

        void NetworkManagerInternalEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            LOG_ENTRY_FUNCTION();
            string interface;
            if (_instance)
            {
 //               ::_instance->Event();
                if (strcmp(owner, IARM_BUS_NM_SRV_MGR_NAME) != 0)
                {
                    NMLOG_ERROR("ERROR - unexpected event: owner %s, eventId: %d, data: %p, size: %d.", owner, (int)eventId, data, (int)len);
                    return;
                }
                if (data == nullptr || len == 0)
                {
                    NMLOG_ERROR("ERROR - event with NO DATA: eventId: %d, data: %p, size: %d.", (int)eventId, data, (int)len);
                    return;
                }

                switch (eventId)
                {
                    case IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_ENABLED_STATUS:
                    {
                        IARM_BUS_NetSrvMgr_Iface_EventInterfaceEnabledStatus_t *e = (IARM_BUS_NetSrvMgr_Iface_EventInterfaceEnabledStatus_t*) data;
                        interface = e->interface;
                        NMLOG_INFO ("IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_ENABLED_STATUS :: %s", interface.c_str());
                        if(interface == "eth0" || interface == "wlan0")
                        {
                            if (e->status)
                                ::_instance->ReportInterfaceStateChangedEvent(Exchange::INetworkManager::INTERFACE_ADDED, interface);
                            else
                                ::_instance->ReportInterfaceStateChangedEvent(Exchange::INetworkManager::INTERFACE_REMOVED, interface);
                        }
                        break;
                    }
                    case IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_CONNECTION_STATUS:
                    {
                        IARM_BUS_NetSrvMgr_Iface_EventInterfaceConnectionStatus_t *e = (IARM_BUS_NetSrvMgr_Iface_EventInterfaceConnectionStatus_t*) data;
                        interface = e->interface;
                        NMLOG_INFO ("IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_CONNECTION_STATUS :: %s", interface.c_str());
                        if(interface == "eth0" || interface == "wlan0")
                        {
                            if (e->status) {
                                if (interface == "wlan0") {
                                    // ip address change event not coming when wifi reconnected
                                    ::_instance->connectivityMonitor.startConnectivityMonitor(true);
                                }
                                ::_instance->ReportInterfaceStateChangedEvent(Exchange::INetworkManager::INTERFACE_LINK_UP, interface);
                            }
                            else {
                               ::_instance->ReportInterfaceStateChangedEvent(Exchange::INetworkManager::INTERFACE_LINK_DOWN, interface);
                               /* when ever interface down we start connectivity monitor to post noInternet event */
                               ::_instance->connectivityMonitor.startConnectivityMonitor(false); // false = interface is down, auto exit after retry
                            }
                        }
                        break;
                    }
                    case IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_IPADDRESS:
                    {
                        IARM_BUS_NetSrvMgr_Iface_EventInterfaceIPAddress_t *e = (IARM_BUS_NetSrvMgr_Iface_EventInterfaceIPAddress_t*) data;
                        interface = e->interface;
                        NMLOG_INFO ("IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_IPADDRESS :: %s -- %s", interface.c_str(), e->ip_address);

                        if(interface == "eth0" || interface == "wlan0") {
                            ::_instance->ReportIPAddressChangedEvent(interface, e->acquired, e->is_ipv6, string(e->ip_address));
                            if(e->acquired)
                            {
                                /* if ip address acquired we start connectivity monitor */
                                ::_instance->connectivityMonitor.startConnectivityMonitor(true); // true = interface is up
                            }
                        }
                        break;
                    }
                    case IARM_BUS_NETWORK_MANAGER_EVENT_DEFAULT_INTERFACE:
                    {
                        string oldInterface;
                        string newInterface;
                        IARM_BUS_NetSrvMgr_Iface_EventDefaultInterface_t *e = (IARM_BUS_NetSrvMgr_Iface_EventDefaultInterface_t*) data;
                        oldInterface = e->oldInterface;
                        newInterface = e->newInterface;
                        NMLOG_INFO ("IARM_BUS_NETWORK_MANAGER_EVENT_DEFAULT_INTERFACE %s :: %s..", oldInterface.c_str(), newInterface.c_str());
                        if(oldInterface != "eth0" || oldInterface != "wlan0")
                            oldInterface == ""; /* assigning "null" if the interface is not eth0 or wlan0 */
                        if(newInterface != "eth0" || newInterface != "wlan0")
                            newInterface == ""; /* assigning "null" if the interface is not eth0 or wlan0 */

                        ::_instance->ReportActiveInterfaceChangedEvent(oldInterface, newInterface);
                        break;
                    }
                    case IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDs:
                    {
                        IARM_BUS_WiFiSrvMgr_EventData_t *e = (IARM_BUS_WiFiSrvMgr_EventData_t*) data;
                        NMLOG_INFO ("IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDs");
                        std::string serialized(e->data.wifiSSIDList.ssid_list);
                        JsonObject eventDocument;
                        WPEC::OptionalType<WPEJ::Error> error;
                        if (!WPEJ::IElement::FromString(serialized, eventDocument, error)) {
                            NMLOG_ERROR("Failed to parse JSON document containing SSIDs. Due to: %s", WPEJ::ErrorDisplayMessage(error).c_str());
                            break;
                        }
                        if ((!eventDocument.HasLabel("getAvailableSSIDs")) || (eventDocument["getAvailableSSIDs"].Content() != WPEJ::Variant::type::ARRAY)) {
                            NMLOG_ERROR("JSON document does not have key 'getAvailableSSIDs' as array");
                            break;
                        }

                        JsonArray ssids = eventDocument["getAvailableSSIDs"].Array();
                        string json;
                        ssids.ToString(json);

                        ::_instance->ReportAvailableSSIDsEvent(json);
                    }
                    case IARM_BUS_WIFI_MGR_EVENT_onWIFIStateChanged:
                    {
                        IARM_BUS_WiFiSrvMgr_EventData_t* e = (IARM_BUS_WiFiSrvMgr_EventData_t *) data;
                        Exchange::INetworkManager::WiFiState state = Exchange::INetworkManager::WIFI_STATE_DISCONNECTED;
                        NMLOG_INFO("Event IARM_BUS_WIFI_MGR_EVENT_onWIFIStateChanged received; state=%d", e->data.wifiStateChange.state);

                        state = to_wifi_state(e->data.wifiStateChange.state);
                        if(e->data.wifiStateChange.state == WIFI_CONNECTED)
                             ::_instance->m_wifiSignalMonitor.startWiFiSignalStrengthMonitor(DEFAULT_WIFI_SIGNAL_TEST_INTERVAL_SEC);
                        ::_instance->ReportWiFiStateChangedEvent(state);
                        break;
                    }
                    case IARM_BUS_WIFI_MGR_EVENT_onError:
                    {
                        IARM_BUS_WiFiSrvMgr_EventData_t* e = (IARM_BUS_WiFiSrvMgr_EventData_t *) data;
                        Exchange::INetworkManager::WiFiState state = errorcode_to_wifi_state(e->data.wifiError.code);
                        NMLOG_INFO("Event IARM_BUS_WIFI_MGR_EVENT_onError received; code=%d", e->data.wifiError.code);
			::_instance->ReportWiFiStateChangedEvent(state);
                        break;
                    }
                    default:
                    {
                        NMLOG_INFO("Event %d received; Unhandled", eventId);
                        break;
                    }
                }
            }
            else
                NMLOG_WARNING("WARNING - cannot handle IARM events without a Network plugin instance!");
        }

        void NetworkManagerImplementation::platform_init()
        {
            LOG_ENTRY_FUNCTION();
            char c;

            ::_instance = this;

            IARM_Result_t res = IARM_Bus_Init("netsrvmgr-thunder");
            NMLOG_INFO("IARM_Bus_Init: %d", res);
            if (res == IARM_RESULT_SUCCESS || res == IARM_RESULT_INVALID_STATE /* already inited or connected */) {
                res = IARM_Bus_Connect();
                NMLOG_INFO("IARM_Bus_Connect: %d", res);
            } else {
                NMLOG_ERROR("IARM_Bus_Init failure: %d", res);
            }


            IARM_Result_t retVal = IARM_RESULT_SUCCESS;

            uint32_t retry = 0;
            do{
                retVal = IARM_Bus_Call_with_IPCTimeout(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_isAvailable, (void *)&c, sizeof(c), (1000*10));
                if(retVal != IARM_RESULT_SUCCESS){
                    NMLOG_INFO("NetSrvMgr is not available. Failed to activate Network Plugin, retry = %d", retry);
                    usleep(500*1000);
                    retry++;
                }
            }while((retVal != IARM_RESULT_SUCCESS) && (retry < 50));

            if(retVal != IARM_RESULT_SUCCESS)
            {
                string msg = "NetSrvMgr is not available";
                NMLOG_INFO("NETWORK_NOT_READY: The NetSrvMgr Component is not available.Retrying in separate thread ::%s::", msg.c_str());
            }
            else {
                IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_ENABLED_STATUS, NetworkManagerInternalEventHandler);
                IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_CONNECTION_STATUS, NetworkManagerInternalEventHandler);
                IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_IPADDRESS, NetworkManagerInternalEventHandler);
                IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_DEFAULT_INTERFACE, NetworkManagerInternalEventHandler);
                IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERNET_CONNECTION_CHANGED, NetworkManagerInternalEventHandler);
                IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_EVENT_onWIFIStateChanged, NetworkManagerInternalEventHandler);
                IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_EVENT_onError, NetworkManagerInternalEventHandler);
                IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_EVENT_onAvailableSSIDs, NetworkManagerInternalEventHandler);
            }
        }

        uint32_t NetworkManagerImplementation::GetAvailableInterfaces (Exchange::INetworkManager::IInterfaceDetailsIterator*& interfacesItr/* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_BUS_NetSrvMgr_InterfaceList_t list;
            if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getInterfaceList, (void*)&list, sizeof(list)))
            {
                std::vector<InterfaceDetails> interfaceList;
                for (int i = 0; i < list.size; i++)
                {
                    NMLOG_INFO ("Interface Name = %s", list.interfaces[i].name);
                    string interfaceName(list.interfaces[i].name);
                    if (("eth0" == interfaceName) || ("wlan0" == interfaceName))
                    {
                        InterfaceDetails tmp;
                        /* Update the interface as per RDK NetSrvMgr */
                        if ("eth0" == interfaceName)
                            tmp.m_type = string("ETHERNET");
                        else if ("wlan0" == interfaceName)
                            tmp.m_type = string("WIFI");

                        tmp.m_name         = interfaceName;
                        tmp.m_mac          = string(list.interfaces[i].mac);
                        tmp.m_isEnabled    = ((list.interfaces[i].flags & IFF_UP) != 0);
                        tmp.m_isConnected  = ((list.interfaces[i].flags & IFF_RUNNING) != 0);
                        interfaceList.push_back(tmp);
                    }
                }
                using Implementation = RPC::IteratorType<Exchange::INetworkManager::IInterfaceDetailsIterator>;
                interfacesItr = Core::Service<Implementation>::Create<Exchange::INetworkManager::IInterfaceDetailsIterator>(interfaceList);

                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("Call to %s for %s failed", IARM_BUS_NM_SRV_MGR_NAME, __FUNCTION__);
            }

            return rc;
        }

        /* @brief Get the active Interface used for external world communication */
        uint32_t NetworkManagerImplementation::GetPrimaryInterface (string& interface /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_BUS_NetSrvMgr_DefaultRoute_t defaultRoute = {0};
            if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getDefaultInterface, (void*)&defaultRoute, sizeof(defaultRoute)))
            {
                NMLOG_INFO ("Call to %s for %s returned interface = %s, gateway = %s", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getDefaultInterface, defaultRoute.interface, defaultRoute.gateway);
                interface = m_defaultInterface = defaultRoute.interface;
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("Call to %s for %s failed", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getDefaultInterface);
            }
            return rc;
        }

        /* @brief Set the active Interface used for external world communication */
        uint32_t NetworkManagerImplementation::SetPrimaryInterface (const string& interface/* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_BUS_NetSrvMgr_Iface_EventData_t iarmData = { 0 };
            iarmData.persist = true;

            /* Netsrvmgr returns eth0 & wlan0 as primary interface but when we want to set., we must set ETHERNET or WIFI*/
            //TODO: Fix netsrvmgr to accept eth0 & wlan0
            if ("wlan0" == interface)
                strncpy(iarmData.setInterface, "WIFI", INTERFACE_SIZE);
            else if ("eth0" == interface)
                strncpy(iarmData.setInterface, "ETHERNET", INTERFACE_SIZE);
            else
            {
                rc = Core::ERROR_BAD_REQUEST;
                return rc;
            }

            if (IARM_RESULT_SUCCESS == IARM_Bus_Call (IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setDefaultInterface, (void *)&iarmData, sizeof(iarmData)))
            {
                NMLOG_INFO ("Call to %s for %s success", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setDefaultInterface);
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("Call to %s for %s failed", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setDefaultInterface);
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::SetInterfaceState(const string& interface/* @in */, const bool& enable /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_BUS_NetSrvMgr_Iface_EventData_t iarmData = { 0 };

            /* Netsrvmgr returns eth0 & wlan0 as primary interface but when we want to set., we must set ETHERNET or WIFI*/
            //TODO: Fix netsrvmgr to accept eth0 & wlan0
            if ("wlan0" == interface)
                strncpy(iarmData.setInterface, "WIFI", INTERFACE_SIZE);
            else if ("eth0" == interface)
                strncpy(iarmData.setInterface, "ETHERNET", INTERFACE_SIZE);
            else
            {
                rc = Core::ERROR_BAD_REQUEST;
                return rc;
            }

            iarmData.isInterfaceEnabled = enable;
            iarmData.persist = true;
            if (IARM_RESULT_SUCCESS == IARM_Bus_Call (IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setInterfaceEnabled, (void *)&iarmData, sizeof(iarmData)))
            {
                NMLOG_INFO ("Call to %s for %s success", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setInterfaceEnabled);
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("Call to %s for %s failed", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setInterfaceEnabled);
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetInterfaceState(const string& interface/* @in */, bool &isEnabled /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_BUS_NetSrvMgr_Iface_EventData_t iarmData = { 0 };

            /* Netsrvmgr returns eth0 & wlan0 as primary interface but when we want to set., we must set ETHERNET or WIFI*/
            //TODO: Fix netsrvmgr to accept eth0 & wlan0
            if ("wlan0" == interface)
                strncpy(iarmData.setInterface, "WIFI", INTERFACE_SIZE);
            else if ("eth0" == interface)
                strncpy(iarmData.setInterface, "ETHERNET", INTERFACE_SIZE);
            else
            {
                rc = Core::ERROR_BAD_REQUEST;
                return rc;
            }

            if (IARM_RESULT_SUCCESS == IARM_Bus_Call (IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_isInterfaceEnabled, (void *)&iarmData, sizeof(iarmData)))
            {
                NMLOG_TRACE("Call to %s for %s success", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_isInterfaceEnabled);
                isEnabled = iarmData.isInterfaceEnabled;
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("Call to %s for %s failed", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setInterfaceEnabled);
            }
            return rc;
        }

        /* function to convert netmask to prefix */
        uint32_t NetmaskToPrefix (const char* netmask_str)
        {
            uint32_t prefix_len = 0;
            uint32_t netmask1 = 0;
            uint32_t netmask2 = 0;
            uint32_t netmask3 = 0;
            uint32_t netmask4 = 0;
            uint32_t netmask = 0;
            sscanf(netmask_str, "%d.%d.%d.%d", &netmask1, &netmask2, &netmask3, &netmask4);
            netmask = netmask1 << 24;
            netmask |= netmask2 << 16;
            netmask |= netmask3 << 8;
            netmask |= netmask4;
            while (netmask)
            {
                if (netmask & 0x80000000)
                {
                    prefix_len++;
                    netmask <<= 1;
                } else
                {
                    break;
                }
            }
            return prefix_len;
        }


        /* @brief Get IP Address Of the Interface */
        uint32_t NetworkManagerImplementation::GetIPSettings(const string& interface /* @in */, const string& ipversion /* @in */, IPAddressInfo& result /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_BUS_NetSrvMgr_Iface_Settings_t iarmData = { 0 };
            /* Netsrvmgr returns eth0 & wlan0 as primary interface but when we want to set., we must set ETHERNET or WIFI*/
            //TODO: Fix netsrvmgr to accept eth0 & wlan0
            if ("wlan0" == interface)
                strncpy(iarmData.interface, "WIFI", INTERFACE_SIZE);
            else if ("eth0" == interface)
                strncpy(iarmData.interface, "ETHERNET", INTERFACE_SIZE);

            strncpy(iarmData.ipversion, ipversion.c_str(), 16);
            iarmData.isSupported = true;
            NMLOG_INFO("NetworkManagerImplementation::GetIPSettings - Before Calling IARM");
            if (IARM_RESULT_SUCCESS == IARM_Bus_Call (IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getIPSettings, (void *)&iarmData, sizeof(iarmData)))
            {
                NMLOG_INFO("NetworkManagerImplementation::GetIPSettings - IARM Success.. Filling the data");
                result.m_ipAddrType     = string(iarmData.ipversion);
                result.m_autoConfig     = iarmData.autoconfig;
                result.m_dhcpServer     = string(iarmData.dhcpserver,MAX_IP_ADDRESS_LEN - 1);
                result.m_v6LinkLocal    = "";
                result.m_ipAddress      = string(iarmData.ipaddress,MAX_IP_ADDRESS_LEN - 1);
                if (0 == strcasecmp("ipv4", iarmData.ipversion))
                    result.m_prefix = NetmaskToPrefix(iarmData.netmask);
		else if (0 == strcasecmp("ipv6", iarmData.ipversion))
                    result.m_prefix = std::atoi(iarmData.netmask);
                result.m_gateway        = string(iarmData.gateway,MAX_IP_ADDRESS_LEN - 1);
                result.m_primaryDns     = string(iarmData.primarydns,MAX_IP_ADDRESS_LEN - 1);
                result.m_secondaryDns   = string(iarmData.secondarydns,MAX_IP_ADDRESS_LEN - 1);
                NMLOG_INFO("NetworkManagerImplementation::GetIPSettings - IARM Success.. Filled the data");
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR("NetworkManagerImplementation::GetIPSettings - Calling IARM Failed");
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

        /* @brief Set IP Address Of the Interface */
        uint32_t NetworkManagerImplementation::SetIPSettings(const string& interface /* @in */, const string &ipversion /* @in */, const IPAddressInfo& address /* @in */)
        {
            uint32_t rc = Core::ERROR_NONE;
            if (0 == strcasecmp("ipv4", ipversion.c_str()))
            {
                IARM_BUS_NetSrvMgr_Iface_Settings_t iarmData = {0};
                /* Netsrvmgr returns eth0 & wlan0 as primary interface but when we want to set., we must set ETHERNET or WIFI*/
                //TODO: Fix netsrvmgr to accept eth0 & wlan0
                if ("wlan0" == interface)
                    strncpy(iarmData.interface, "WIFI", INTERFACE_SIZE);
                else if ("eth0" == interface)
                    strncpy(iarmData.interface, "ETHERNET", INTERFACE_SIZE);
                else
                {
                    rc = Core::ERROR_BAD_REQUEST;
                    return rc;
                }
                /* IP version */
                strncpy(iarmData.ipversion, ipversion.c_str(), 16);

                if (!address.m_autoConfig)
                {
                    //Lets validate the prefix Address
                    if (address.m_prefix >= 32)
                    {
                        rc = Core::ERROR_BAD_REQUEST;
                    }
                    else
                    {
                        string netmask = CIDR_PREFIXES[address.m_prefix];

                        //Lets validate the IP Address
                        struct in_addr tmpIPAddress, tmpGWAddress, tmpNetmask;
                        struct in_addr bcastAddr1, bcastAddr2;

                        if (inet_pton(AF_INET, address.m_ipAddress.c_str(), &tmpIPAddress) == 1 &&
                            inet_pton(AF_INET, netmask.c_str(), &tmpNetmask) == 1 &&
                            inet_pton(AF_INET, address.m_gateway.c_str(), &tmpGWAddress) == 1)
                        {
                            bcastAddr1.s_addr = tmpIPAddress.s_addr | ~tmpNetmask.s_addr;
                            bcastAddr2.s_addr = tmpGWAddress.s_addr | ~tmpNetmask.s_addr;

                            if (tmpIPAddress.s_addr == tmpGWAddress.s_addr)
                            {
                                NMLOG_INFO("Interface and Gateway IP are same , return false");
                                rc = Core::ERROR_BAD_REQUEST;
                            }
                            if (bcastAddr1.s_addr != bcastAddr2.s_addr)
                            {
                                NMLOG_INFO("Interface and Gateway IP is not in same broadcast domain, return false");
                                rc = Core::ERROR_BAD_REQUEST;
                            }
                            if (tmpIPAddress.s_addr == bcastAddr1.s_addr)
                            {
                                NMLOG_INFO("Interface and Broadcast IP is same, return false");
                                rc = Core::ERROR_BAD_REQUEST;
                            }
                            if (tmpGWAddress.s_addr == bcastAddr2.s_addr)
                            {
                                NMLOG_INFO("Gateway and Broadcast IP is same, return false");
                                rc = Core::ERROR_BAD_REQUEST;
                            }
                        }
                        else
                        {
                            rc = Core::ERROR_BAD_REQUEST;
                            NMLOG_INFO ("Given Input Address are not appropriate");
                        }

                        if (Core::ERROR_NONE == rc)
                        {
                            strncpy(iarmData.ipaddress, address.m_ipAddress.c_str(), 16);
                            strncpy(iarmData.netmask, netmask.c_str(), 16);
                            strncpy(iarmData.gateway, address.m_gateway.c_str(), 16);
                            strncpy(iarmData.primarydns, address.m_primaryDns.c_str(), 16);
                            strncpy(iarmData.secondarydns, address.m_secondaryDns.c_str(), 16);
                        }
                    }
                }
                else
                {
                    iarmData.autoconfig = address.m_autoConfig;
                }
                if (Core::ERROR_NONE == rc)
                {
                    if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setIPSettings, (void *) &iarmData, sizeof(iarmData)))
                    {
                        NMLOG_INFO("Set IP Successfully");
                    }
                    else
                    {
                        NMLOG_ERROR("Setting IP Failed");
                        rc = Core::ERROR_RPC_CALL_FAILED;
                    }
                }
            }
            else
            {
                //FIXME : Add IPv6 support here
                NMLOG_WARNING ("Setting IPv6 is not supported at this point in time. This is just a place holder");
                rc = Core::ERROR_NOT_SUPPORTED;
            }

            return rc;
        }

        uint32_t NetworkManagerImplementation::StartWiFiScan(const WiFiFrequency frequency /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_Bus_WiFiSrvMgr_SsidList_Param_t param;
            IARM_Result_t retVal = IARM_RESULT_SUCCESS;

            memset(&param, 0, sizeof(param));
            (void) frequency;

            retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_getAvailableSSIDsAsync, (void *)&param, sizeof(IARM_Bus_WiFiSrvMgr_SsidList_Param_t));

            if(retVal == IARM_RESULT_SUCCESS) {
                NMLOG_INFO ("Scan started");
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("StartScan failed");
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::StopWiFiScan(void)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));

            if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_stopProgressiveWifiScanning, (void*) &param, sizeof(IARM_Bus_WiFiSrvMgr_Param_t)))
            {
                NMLOG_INFO ("StopScan Success");
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("StopScan failed");
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetKnownSSIDs(IStringIterator*& ssids /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_Result_t retVal = IARM_RESULT_SUCCESS;
            IARM_Bus_WiFiSrvMgr_Param_t param;

            memset(&param, 0, sizeof(param));

            /* Must add new method to get all the known SSIDs but for now RDK-NM supports only one active SSID. So we repurpose this method */
            retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_getConnectedSSID, (void *)&param, sizeof(param));

            if(retVal == IARM_RESULT_SUCCESS)
            {
                auto &connectedSsid = param.data.getConnectedSSID;
                std::list<string> ssidList;
                ssidList.push_back(string(connectedSsid.ssid));
                NMLOG_INFO ("GetKnownSSIDs Success");

                ssids = Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(ssidList);
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("GetKnownSSIDs failed");
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::AddToKnownSSIDs(const WiFiConnectTo& ssid /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));

            strncpy(param.data.connect.ssid, ssid.m_ssid.c_str(), SSID_SIZE - 1);
            strncpy(param.data.connect.passphrase, ssid.m_passphrase.c_str(), PASSPHRASE_BUFF - 1);
            param.data.connect.security_mode = (SsidSecurity) ssid.m_securityMode;

            IARM_Result_t retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_saveSSID, (void *)&param, sizeof(param));
            if((retVal == IARM_RESULT_SUCCESS) && param.status)
            {
                NMLOG_INFO ("AddToKnownSSIDs Success");
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("AddToKnownSSIDs failed");
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::RemoveKnownSSID(const string& ssid /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));

            /* Currently RDK-NM supports only one saved SSID. So when you say clear, it jsut clears it. No need to pass input at this point in time.
               Will be updated to pass specific ssid when we support more than 1 ssid.
             */
            (void)ssid;

            IARM_Result_t retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_clearSSID, (void *)&param, sizeof(param));
            if((retVal == IARM_RESULT_SUCCESS) && param.status)
            {
                NMLOG_INFO ("RemoveKnownSSID Success");
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("RemoveKnownSSID failed");
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::WiFiConnect(const WiFiConnectTo& ssid /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_Result_t retVal = IARM_RESULT_SUCCESS;
            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));

            if(ssid.m_ssid.length() || ssid.m_passphrase.length())
            {
                ssid.m_ssid.copy(param.data.connect.ssid, sizeof(param.data.connect.ssid) - 1);
                ssid.m_passphrase.copy(param.data.connect.passphrase, sizeof(param.data.connect.passphrase) - 1);
                param.data.connect.security_mode = (SsidSecurity)ssid.m_securityMode;
                if(!ssid.m_identity.empty())
                    ssid.m_identity.copy(param.data.connect.eapIdentity, sizeof(param.data.connect.eapIdentity) - 1);
                if(!ssid.m_caCert.empty())
                    ssid.m_caCert.copy(param.data.connect.carootcert, sizeof(param.data.connect.carootcert) - 1);
                if(!ssid.m_clientCert.empty())
                    ssid.m_clientCert.copy(param.data.connect.clientcert, sizeof(param.data.connect.clientcert) - 1);
                if(!ssid.m_privateKey.empty())
                    ssid.m_privateKey.copy(param.data.connect.privatekey, sizeof(param.data.connect.privatekey) - 1);
            }

            retVal = IARM_Bus_Call( IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_connect, (void *)&param, sizeof(param));

            if((retVal == IARM_RESULT_SUCCESS) && param.status)
            {
                NMLOG_INFO ("WiFiConnect Success");
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("WiFiConnect failed");
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::WiFiDisconnect(void)
        {
            LOG_ENTRY_FUNCTION();
            IARM_Result_t retVal = IARM_RESULT_SUCCESS;
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));

            retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_disconnectSSID, (void *)&param, sizeof(param));
            if ((retVal == IARM_RESULT_SUCCESS) && param.status)
            {
                NMLOG_INFO ("WiFiDisconnect started");
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("WiFiDisconnect failed");
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetConnectedSSID(WiFiSSIDInfo&  ssidInfo /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_Result_t retVal = IARM_RESULT_SUCCESS;
            IARM_Bus_WiFiSrvMgr_Param_t param;

            memset(&param, 0, sizeof(param));

            /* Must add new method to get all the known SSIDs but for now RDK-NM supports only one active SSID. So we repurpose this method */
            retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_getConnectedSSID, (void *)&param, sizeof(param));

            if(retVal == IARM_RESULT_SUCCESS)
            {
                auto &connectedSsid = param.data.getConnectedSSID;

                ssidInfo.m_ssid             = string(connectedSsid.ssid);
                ssidInfo.m_bssid            = string(connectedSsid.bssid);
                ssidInfo.m_securityMode     = (WIFISecurityMode) connectedSsid.securityMode;
                ssidInfo.m_signalStrength   = to_string(connectedSsid.signalStrength);
                ssidInfo.m_frequency        = ((((float)connectedSsid.frequency)/1000) < 3.0) ? WIFI_FREQUENCY_2_4_GHZ : WIFI_FREQUENCY_5_GHZ;
                ssidInfo.m_rate             = to_string(connectedSsid.rate);
                ssidInfo.m_noise            = to_string(connectedSsid.noise);

                NMLOG_INFO ("GetConnectedSSID Success");
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("GetConnectedSSID failed");
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetWiFiSignalStrength(string& ssid /* @out */, string& signalStrength /* @out */, WiFiSignalQuality& quality /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            WiFiSSIDInfo  ssidInfo{};
            float signalStrengthOut = 0.0f;

            if (Core::ERROR_NONE == GetConnectedSSID(ssidInfo))
            {
                ssid            = ssidInfo.m_ssid;
                signalStrength  = ssidInfo.m_signalStrength;

                if (!signalStrength.empty())
		{
                    signalStrengthOut = std::stof(signalStrength.c_str());
		    NMLOG_INFO ("WiFiSignalStrength in dB = %s",signalStrengthOut);
		}

                if (signalStrengthOut == 0)
		{
                    quality = WIFI_SIGNAL_DISCONNECTED;
		    signalStrength = "0";
		}
                else if (signalStrengthOut >= signalStrengthThresholdExcellent && signalStrengthOut < 0)
		{
                    quality = WIFI_SIGNAL_EXCELLENT;
		    signalStrength = "100";
		}
                else if (signalStrengthOut >= signalStrengthThresholdGood && signalStrengthOut < signalStrengthThresholdExcellent)
		{
                    quality = WIFI_SIGNAL_GOOD;
		    signalStrength = "75";
		}
                else if (signalStrengthOut >= signalStrengthThresholdFair && signalStrengthOut < signalStrengthThresholdGood)
		{
                    quality = WIFI_SIGNAL_FAIR;
		    signalStrength = "50";
		}
                else
		{
                    quality = WIFI_SIGNAL_WEAK;
		    signalStrength = "25";
		}

                NMLOG_INFO ("GetWiFiSignalStrength success");
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("GetWiFiSignalStrength failed");
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::StartWPS(const WiFiWPS& method /* @in */, const string& wps_pin /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_Bus_WiFiSrvMgr_WPS_Parameters_t wps_parameters;
            if (method == WIFI_WPS_PBC)
            {
                wps_parameters.pbc = true;
            }
            else if (method == WIFI_WPS_PIN)
            {
                snprintf(wps_parameters.pin, sizeof(wps_parameters.pin), "%s", wps_pin.c_str());
                wps_parameters.pbc = false;
            }
            else if (method == WIFI_WPS_SERIALIZED_PIN)
            {
                snprintf(wps_parameters.pin, sizeof(wps_parameters.pin), "xxxxxxxx");
                wps_parameters.pbc = false;
            }

            if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_initiateWPSPairing2, (void *)&wps_parameters, sizeof(wps_parameters)))
            {
                NMLOG_INFO ("StartWPS is success");
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("StartWPS: Failed");                
            }

            return rc;
        }

        uint32_t NetworkManagerImplementation::StopWPS(void)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));

            if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_cancelWPSPairing, (void *)&param, sizeof(param)))
            {
                NMLOG_INFO ("StopWPS is success");
                rc = Core::ERROR_NONE;
            }
            else
            {
                NMLOG_ERROR ("StopWPS: Failed");                
            }

            return rc;
        }

        uint32_t NetworkManagerImplementation::GetWifiState(WiFiState &state)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            IARM_Result_t retVal = IARM_RESULT_SUCCESS;
            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));

            if(IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_getCurrentState, (void *)&param, sizeof(param)))
            {
                state = to_wifi_state(param.data.wifiStatus);
                rc = Core::ERROR_NONE;
            }
            return rc;
        }
    }
}
