#include "NetworkManagerImplementation.h"
#include "WiFiSignalStrengthMonitor.h"

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

        void NetworkManagerImplementation::platform_init()
        {
            LOG_ENTRY_FUNCTION();
            char c;

            ::_instance = this;
        }

        uint32_t NetworkManagerImplementation::GetAvailableInterfaces (Exchange::INetworkManager::IInterfaceDetailsIterator*& interfacesItr/* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        /* @brief Get the active Interface used for external world communication */
        uint32_t NetworkManagerImplementation::GetPrimaryInterface (string& interface /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        /* @brief Set the active Interface used for external world communication */
        uint32_t NetworkManagerImplementation::SetPrimaryInterface (const string& interface/* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::SetInterfaceState(const string& interface/* @in */, const bool& enable /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetInterfaceState(const string& interface/* @in */, bool& isEnabled /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        /* @brief Get IP Address Of the Interface */
        uint32_t NetworkManagerImplementation::GetIPSettings(const string& interface /* @in */, const string& ipversion /* @in */, IPAddressInfo& result /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        /* @brief Set IP Address Of the Interface */
        uint32_t NetworkManagerImplementation::SetIPSettings(const string& interface /* @in */, const string &ipversion /* @in */, const IPAddressInfo& address /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::StartWiFiScan(const WiFiFrequency frequency /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::StopWiFiScan(void)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetKnownSSIDs(IStringIterator*& ssids /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::AddToKnownSSIDs(const WiFiConnectTo& ssid /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::RemoveKnownSSID(const string& ssid /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::WiFiConnect(const WiFiConnectTo& ssid /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::WiFiDisconnect(void)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetConnectedSSID(WiFiSSIDInfo&  ssidInfo /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetWiFiSignalStrength(string& ssid /* @out */, string& signalStrength /* @out */, WiFiSignalQuality& quality /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }

        uint32_t NetworkManagerImplementation::StartWPS(const WiFiWPS& method /* @in */, const string& wps_pin /* @in */)
        {
            LOG_ENTRY_FUNCTION();
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
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            return rc;
        }
    }
}
