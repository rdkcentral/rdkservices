#include "../nm_adapter.h"

#include <net/if.h>
#include <arpa/inet.h>
#include "UtilsIarm.h"
#include "UtilsJsonRpc.h"
#include "UtilsString.h"
#include "UtilscRunScript.h"
#include "UtilsgetRFCConfig.h"
// Define this to use netlink calls (where there may be an alternative method but netlink could provide
// the information or perform the action required)
//#define USE_NETLINK
#define MAX_IP_ADDRESS_LEN 46
#define NETSRVMGR_INTERFACES_MAX 16
#define MAX_ENDPOINTS 5
#define MAX_ENDPOINT_SIZE 512
#define MAX_URI_LEN 512
#define MAX_HOST_NAME_LEN 128

#define IARM_BUS_NM_SRV_MGR_NAME "NET_SRV_MGR"
#define IARM_BUS_NETSRVMGR_API_isConnectedToInternet "isConnectedToInternet"
#define IARM_BUS_NETSRVMGR_API_getIPSettings "getIPSettings"

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

            IARM_BUS_NetSrvMgr_Iface_Settings_t m_ipv4WifiCache;
            IARM_BUS_NetSrvMgr_Iface_Settings_t m_ipv6WifiCache;
            IARM_BUS_NetSrvMgr_Iface_Settings_t m_ipv4EthCache;
            IARM_BUS_NetSrvMgr_Iface_Settings_t m_ipv6EthCache;

class iarmAdapter : public NetworkmanagerAdapter
{
    public:
            string m_stunEndPoint;
            string m_isHybridDevice;
            string m_defaultInterface;
            string m_gatewayInterface;
            string m_publicIPAddress;
            string m_ipversion;
            uint16_t m_stunPort;
            uint16_t m_stunBindTimeout;
            uint16_t m_stunCacheTimeout;
            bool m_stunSync;
            uint32_t m_apiVersionNumber;
            std::atomic<bool> m_useIpv4WifiCache;
            std::atomic<bool> m_useIpv6WifiCache;
            std::atomic<bool> m_useIpv4EthCache;
            std::atomic<bool> m_useIpv6EthCache;
            std::atomic<bool> m_useStbIPCache;
            string m_stbIpCache;
            std::atomic<bool> m_useDefInterfaceCache;
            string m_defInterfaceCache;
            string m_defIpversionCache;
            std::atomic<bool> m_useInterfacesCache;
   std::atomic_bool m_isPluginInited{false};
    iarmAdapter()
    {
        m_isPluginInited = true;
        Initialize();
    }
    ~iarmAdapter()
    {
    }
        bool getIPIARMWrapper(IARM_BUS_NetSrvMgr_Iface_Settings_t& iarmData, const string interface, const string ipversion)
        {
           strncpy(iarmData.interface, interface.c_str(), 16);
           strncpy(iarmData.ipversion, ipversion.c_str(), 16);
           if (IARM_RESULT_SUCCESS == IARM_Bus_Call (IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_getIPSettings, (void *)&iarmData, sizeof(iarmData)))
               return true;

           return false;
        }
        bool getIPSettingsInternal(const JsonObject& parameters, JsonObject& response,int& errCode)
        {
            string interface = "";
            string ipversion = "";
            bool result = false;

            getStringParameter("interface", interface);
            getStringParameter("ipversion", ipversion);
            if (interface.empty())
                interface = m_defInterfaceCache;

            if (ipversion.empty())
                ipversion = m_defIpversionCache;

            IARM_BUS_NetSrvMgr_Iface_Settings_t iarmData = { 0 };
            strncpy(iarmData.interface, interface.c_str(), 16);
            strncpy(iarmData.ipversion, ipversion.c_str(), 16);
            iarmData.isSupported = true;

            if (Utils::String::equal(ipversion, "ipv4") && Utils::String::equal(interface, "wifi"))
            {
                if ((!m_useIpv4WifiCache) && (getIPIARMWrapper(m_ipv4WifiCache, interface, ipversion)))
                    m_useIpv4WifiCache = true;

                if (m_useIpv4WifiCache)
                {
                    memcpy(&iarmData, &m_ipv4WifiCache, sizeof(m_ipv4WifiCache));
                    result = true;
                }
            }
            else if (Utils::String::equal(ipversion, "ipv4") && Utils::String::equal(interface, "ethernet"))
            {
                if ((!m_useIpv4EthCache) && (getIPIARMWrapper(m_ipv4EthCache, interface, ipversion)))
                    m_useIpv4EthCache = true;

                if (m_useIpv4EthCache)
                {
                    memcpy(&iarmData, &m_ipv4EthCache, sizeof(m_ipv4EthCache));
                    result = true;
                }
            }
            else if (Utils::String::equal(ipversion, "ipv6") && Utils::String::equal(interface, "wifi"))
            {
                if ((!m_useIpv6WifiCache) && (getIPIARMWrapper(m_ipv6WifiCache, interface, ipversion)))
                    m_useIpv6WifiCache = true;

                if (m_useIpv6WifiCache)
                {
                    memcpy(&iarmData, &m_ipv6WifiCache, sizeof(m_ipv6WifiCache));
                    result = true;
                }
            }
            else if (Utils::String::equal(ipversion, "ipv6") && Utils::String::equal(interface, "ethernet"))
            {
                if ((!m_useIpv6EthCache) && (getIPIARMWrapper(m_ipv6EthCache, interface, ipversion)))
                    m_useIpv6EthCache = true;

                if (m_useIpv6EthCache)
                {
                    memcpy(&iarmData, &m_ipv6EthCache, sizeof(m_ipv6EthCache));
                    result = true;
                }
            }
            else if (getIPIARMWrapper(iarmData, interface, ipversion))
            {
                result = true;
                m_defInterfaceCache = string(iarmData.interface);
                m_defIpversionCache = string(iarmData.ipversion);
            }

            if (result == true)
            {
                response["interface"] = string(iarmData.interface);
                response["ipversion"] = string(iarmData.ipversion);
                response["autoconfig"] = iarmData.autoconfig;
                response["ipaddr"] = string(iarmData.ipaddress,MAX_IP_ADDRESS_LEN - 1);
                response["netmask"] = string(iarmData.netmask,MAX_IP_ADDRESS_LEN - 1);
                response["dhcpserver"] = string(iarmData.dhcpserver,MAX_IP_ADDRESS_LEN - 1);
                response["gateway"] = string(iarmData.gateway,MAX_IP_ADDRESS_LEN - 1);
                response["primarydns"] = string(iarmData.primarydns,MAX_IP_ADDRESS_LEN - 1);
                response["secondarydns"] = string(iarmData.secondarydns,MAX_IP_ADDRESS_LEN - 1);
                errCode = iarmData.errCode;

                m_ipversion = string(iarmData.ipversion);
            }

            return result;
        }

        uint32_t getIPSettings(const JsonObject& parameters, JsonObject& response)
        {
            JsonObject internal;
            JsonObject InternalResponse;
            int errCode;
            bool result = false;
            string interface = "";
            string ipversion = "";
            if(m_isPluginInited)
            {
                getDefaultStringParameter("interface", interface,"");
                internal ["interface"] = interface;
                getDefaultStringParameter("ipversion", ipversion,"");
                internal ["ipversion"] = ipversion;

                if (getIPSettingsInternal(internal, InternalResponse, errCode))
                {
                    if (NETWORK_IPADDRESS_ACQUIRED == errCode)
                    {
                        response["interface"] = InternalResponse["interface"];
                        response["ipversion"] = InternalResponse["ipversion"];
                        response["autoconfig"] = InternalResponse["autoconfig"];
                        response["ipaddr"] = InternalResponse["ipaddr"];
                        response["netmask"] = InternalResponse["netmask"];
                        response["gateway"] = InternalResponse["gateway"];
                        response["primarydns"] = InternalResponse["primarydns"];
                        response["secondarydns"] = InternalResponse["secondarydns"];
                        result = true;
                    }
                }
            }
            else
            {
                LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
            }

            returnResponse(result)
        }
    int32_t IsConnectedToInternet(const string &message, string &result /* @out */) override
    {
        printf("inside IsConnectedToInternet\n");
        bool isconnected = false;

        if(m_isPluginInited)
        {
            if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_isConnectedToInternet, (void*) &isconnected, sizeof(isconnected)))
            {
                LOGINFO("%s :: isconnected = %d \n",__FUNCTION__,isconnected);
            }
            else
            {
                LOGWARN("Call to %s for %s failed\n", IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_isConnectedToInternet);
            }
        }
        else
        {
            LOGWARN ("Network plugin not initialised yet returning from %s", __FUNCTION__);
        }
        return 0;
    }

    void eventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
//        if (Network::_instance)
//            Network::_instance->iarmEventHandler(owner, eventId, data, len);
//        else
            LOGWARN("WARNING - cannot handle IARM events without a Network plugin instance!");
    }
//  const string Initialize(PluginHost::IShell*  service )
    const string Initialize( )
    {
//        m_service = service;
//        m_service->AddRef();

        string msg;
        if (Utils::IARM::init())
        {
#if 0
            IARM_Result_t res;
            IARM_Result_t retVal = IARM_RESULT_SUCCESS;
            //#ifndef NET_DISABLE_NETSRVMGR_CHECK
            char c;
            uint32_t retry = 0;
            do{
                retVal = IARM_Bus_Call_with_IPCTimeout(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_isAvailable, (void *)&c, sizeof(c), (1000*10));
                if(retVal != IARM_RESULT_SUCCESS){
                    LOGERR("NetSrvMgr is not available. Failed to activate Network Plugin, retry = %d", retry);
                    usleep(500*1000);
                    retry++;
                }
            }while((retVal != IARM_RESULT_SUCCESS) && (retry < 20));
            //#endif

            if(retVal != IARM_RESULT_SUCCESS)
            {
                msg = "NetSrvMgr is not available";
                LOGERR("NETWORK_NOT_READY: The NetSrvMgr Component is not available.Retrying in separate thread");
                retryIarmEventRegistration();
            }
            else {
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_ENABLED_STATUS, eventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_CONNECTION_STATUS, eventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_IPADDRESS, eventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_DEFAULT_INTERFACE, eventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETWORK_MANAGER_EVENT_INTERNET_CONNECTION_CHANGED, eventHandler) );
                LOGINFO("Successfully activated Network Plugin");
                m_isPluginInited = true;
            }
#endif
        }
        else
        {
            msg = "IARM bus is not available";
            LOGERR("IARM bus is not available. Failed to activate Network Plugin");
        }

        return msg;
    }


};
