/*
 * Copyright (c) 2022, 2023 LIBERTY GLOBAL all rights reserved.
 */

#ifndef LGINETWORK_CLIENT_HPP_
#define LGINETWORK_CLIENT_HPP_

#include <thread>
#include <vector>
#include <map>
#include <gio/gio.h>
#include <glib.h>
#ifdef GDBUS_USE_CODEGEN_IMPL
    #include "gdbus-codegen-impl/lginetwork_dbus_api.h"
#else
    #include "lginetwork_dbus_api.h"
#endif

#include "../Module.h"

#define NETWORK_CONFIG_DBUS_INTERFACE_NAME        "com.lgi.rdk.utils.networkconfig1"
#define NETWORK_CONFIG_DBUS_INTERFACE_OBJECT_PATH "/com/lgi/rdk/utils/networkconfig1"

namespace lginet
{

/** Interface configuration parameters **/
static const std::string ParamType                              = "type";
static const std::string ParamMac                               = "mac";
static const std::string ParamIpv4Ip                            = "ipv4.ip";
static const std::string ParamIpv4Mask                          = "ipv4.mask";
static const std::string ParamIpv4PrefixLength                  = "ipv4.prefix_length";
static const std::string ParamIpv4Gateway                       = "ipv4.gateway";
static const std::string ParamIpv4DhcpServer                    = "ipv4.dhcpserver";
static const std::string ParamIpv4DhcpOptionPrefix              = "ipv4.dhcpoption.";
static const std::string ParamIpv6Ip                            = "ipv6.ip";
static const std::string ParamIpv6PrefixLength                  = "ipv6.prefix_length";
static const std::string ParamIpv6Assignment                    = "ipv6.assignment";
static const std::string ParamIpv6AssignmentDHCPv6              = "DHCPv6";
static const std::string ParamIpv6AssignmentSLAAC               = "SLAAC";
static const std::string ParamIpv6Gateway                       = "ipv6.gateway";
static const std::string ParamIpv6DhcpServer                    = "ipv6.dhcpserver";
static const std::string ParamIpv6DhcpOptionPrefix              = "ipv6.dhcpoption.";
static const std::string ParamDns                               = "dns";
static const std::string ParamNetid                             = "netid";
static const std::string ParamStrength                          = "strength";
static const std::string ParamLinkSpeed                         = "link_speed";
static const std::string ParamWakeOn                            = "wakeon";
static const std::string ParamIpTimeout                         = "ip.timeout";
static const std::string ParamWifiTimeout                       = "wifi.timeout";
static const std::string ParamWifiCountry                       = "wifi.country";
static const std::string ParamWifiCountryLoader                 = "wifi.country.loader";
static const std::string ParamWifiLinkSpeedRx                   = "wifi.link_speed.rx";
static const std::string ParamWifiLinkSpeedTx                   = "wifi.link_speed.tx";
static const std::string ParamWifiRssi                          = "wifi.rssi";
static const std::string ParamWifiOperatingStandards            = "wifi.operating_standards";
static const std::string ParamHwTemperature                     = "hw.temperature";
static const std::string ParamArpEntries                        = "arp.entries";
static const std::string ParamDuplex                            = "duplex";
static const std::string ParamHostname                          = "hostname";
static const std::string ParamConnectivity                      = "connectivity";
static const std::string ParamNetworkChanged                    = "network.changed";
static const std::string ParamWolProcessTimeout                 = "wol_process.timeout";

typedef void (*StatusChangeEventHandler)(const std::string id, const std::string status);
typedef void (*NetworkingEventEventHandler)(const std::string id, const std::string status,
                                            const std::map<std::string, std::string> params);
typedef void (*IpConfigurationChangedEvent)(const std::string id);

class LgiNetworkClient
{
    friend class DbusHandlerCallbacks;

public:
    LgiNetworkClient();
    ~LgiNetworkClient();
    LgiNetworkClient(const LgiNetworkClient&) = delete;
    LgiNetworkClient& operator=(LgiNetworkClient const&) = delete;
    LgiNetworkClient(const LgiNetworkClient&&) = delete;
    LgiNetworkClient& operator=(LgiNetworkClient const&&) = delete;
    int Run();
    void Stop();

    std::vector<std::string>* getInterfaces();
    bool getParamsForInterface(const std::string iface, std::vector<std::pair<std::string, std::string>>& params);
    bool getSpecificParamsForInterface(const std::string iface, std::map<std::string, std::string>& params);
    bool isInterfaceEnabled(const std::string iface);
    bool setInterfaceEnabled(const std::string iface, bool newstate);
    std::string getDefaultInterface();

    StatusChangeEventHandler onStatusChangeEvent;
    NetworkingEventEventHandler onNetworkingEvent;
    IpConfigurationChangedEvent onHandleIpv4ConfigurationChangedEvent;
    IpConfigurationChangedEvent onHandleIpv6ConfigurationChangedEvent;

private:

    void onHandleNetworkingEvent(LgiNetworkClient*  aNetworkConfigProxy,
                                        const gchar*    aId,
                                        const gchar*    aEvent,
                                        guint           aCount,
                                        GVariant*       aParams);
    void onHandleIpv4ConfigurationChanged(LgiNetworkClient*  aNetworkConfigProxy,
                                        const gchar*    aId);
    void onHandleIpv6ConfigurationChanged(LgiNetworkClient*  aNetworkConfigProxy,
                                        const gchar*    aId);
    void onHandleStatusChanged(LgiNetworkClient*  aNetworkConfigProxy,
                                        const gchar *aId,
                                        const gchar *aIfaceStatus);

    void connectSignal(const char* signalName, GCallback callback);
    void disconnectAllSignals();

    Networkconfig1* m_interface;
    std::vector<gulong> m_signalHandles;
};

} // namespace lginet

#endif // #ifdef LGINETWORK_CLIENT_HPP_