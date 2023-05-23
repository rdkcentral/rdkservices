#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include "Network.h"

#include "FactoriesImplementation.h"
#include "ServiceMock.h"
#include "IarmBusMock.h"
#include "WrapsMock.h"

using namespace std;
using namespace WPEFramework;

using ::testing::NiceMock;

extern "C" FILE* __real_popen(const char* command, const char* type);

class NetworkTestBase : public ::testing::Test {
public:

    NiceMock<WrapsImplMock> wrapsImplMock;
    NiceMock<IarmBusImplMock> iarmBusImplMock;

    NetworkTestBase()
    {
        Wraps::getInstance().impl = &wrapsImplMock;
        IarmBus::getInstance().impl = &iarmBusImplMock;

        ofstream file("/etc/device.properties");
        file << "DEVICE_TYPE=mediaclient\n";
        file << "WIFI_SUPPORT=true\n";
        file << "MOCA_INTERFACE=true\n";
        file <<"WIFI_INTERFACE==wlan0\n";
        file <<"MOCA_INTERFACE=eth0\n";
        file <<"ETHERNET_INTERFACE=eth0\n";
        file.close();

        ON_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
             [&](const char* command, const char* type) -> FILE* {
                return __real_popen(command, type);
            }));
    }
};

class NetworkTest : public NetworkTestBase {
protected:

    Core::ProxyType<Plugin::Network> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    Core::JSONRPC::Message message;
    ServiceMock service;

    NetworkTest() : plugin(Core::ProxyType<Plugin::Network>::Create()),
                    handler(*plugin),
                    connection(1, 0)
    {
        IARM_EventHandler_t interfaceEnabled;
        IARM_EventHandler_t interfaceConnection;
        IARM_EventHandler_t interfaceIpaddress;
        IARM_EventHandler_t defaultInterface;
        IARM_EventHandler_t internetStateChanged;

        EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call_with_IPCTimeout)
            .Times(::testing::AnyNumber())
            .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen, int timeout) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_isAvailable)));
                return IARM_RESULT_SUCCESS;
     });

        ON_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
            if ((string(IARM_BUS_NM_SRV_MGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_ENABLED_STATUS)) {
                interfaceEnabled = handler;
            }
            if ((string(IARM_BUS_NM_SRV_MGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_CONNECTION_STATUS)) {
                interfaceConnection = handler;
            }
            if ((string(IARM_BUS_NM_SRV_MGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_IPADDRESS)) {
                interfaceIpaddress = handler;
            }
            if ((string(IARM_BUS_NM_SRV_MGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_NETWORK_MANAGER_EVENT_DEFAULT_INTERFACE)) {
                defaultInterface = handler;
            }
            if ((string(IARM_BUS_NM_SRV_MGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_NETWORK_MANAGER_EVENT_INTERNET_CONNECTION_CHANGED)) {
                internetStateChanged = handler;
            }
            return IARM_RESULT_SUCCESS;
        }));

        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }

    virtual ~NetworkTest() override
    {
        Wraps::getInstance().impl = nullptr;
        IarmBus::getInstance().impl = nullptr;
    }
};

TEST_F(NetworkTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getStbIp")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getInterfaces")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isInterfaceEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setInterfaceEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDefaultInterface")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getIPSettings")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isConnectedToInternet")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPublicIP")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSTBIPFamily")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setConnectivityTestEndpoints")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getCaptivePortalURI")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("startConnectivityMonitoring")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("stopConnectivityMonitoring")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setDefaultInterface")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setIPSettings")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("trace")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("traceNamedEndpoint")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getNamedEndpoints")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("pingNamedEndpoint")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("ping")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getQuirks")));
}

TEST_F(NetworkTest, getStbIp)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getSTBip)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_EventData_t*>(arg);
                memcpy(&param->activeIfaceIpaddr, "192.168.1.101", sizeof("192.168.1.101"));
                EXPECT_EQ(string(param->activeIfaceIpaddr), string(_T("192.168.1.101")));

                return IARM_RESULT_SUCCESS;
           });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStbIp"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"ip\":\"192.168.1.101\",\"success\":true}"));
}

TEST_F(NetworkTest, getInterfaces)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getInterfaceList)));
                auto param = static_cast<IARM_BUS_NetSrvMgr_InterfaceList_t *>(arg);

				param->size = 1;
                memcpy(&param->interfaces[0].name, "eth0", sizeof("eth0"));
                memcpy(&param->interfaces[0].mac, "AA:AA:AA:AA:AA:AA", sizeof("AA:AA:AA:AA:AA:AA"));
                param->interfaces[0].flags = 69699;

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInterfaces"), _T("{}"), response));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"interface\":\"ETHERNET\"")));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"macAddress\":\"AA:AA:AA:AA:AA:AA\"")));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"enabled\":true")));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"connected\":true")));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(NetworkTest, isInterfaceEnabled)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_isInterfaceEnabled)));
                auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_EventData_t  *>(arg);

                memcpy(&param->setInterface, "ETHERNET", sizeof("ETHERNET"));
	            param->isInterfaceEnabled = true;

                EXPECT_EQ(string(param->setInterface), string(_T("ETHERNET")));
                EXPECT_EQ(param->isInterfaceEnabled, true);

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isInterfaceEnabled"), _T("{\"interface\": \"ETHERNET\"}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"enabled\":true")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(NetworkTest, getDefaultInterface)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getDefaultInterface)));
                auto param = static_cast<IARM_BUS_NetSrvMgr_DefaultRoute_t *>(arg);
                memcpy(&param->interface, "eth0", sizeof("eth0"));
                memcpy(&param->gateway, "192.168.1.1", sizeof("192.168.1.1"));

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDefaultInterface"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"interface\":\"ETHERNET\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(NetworkTest, getIPSettings)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getIPSettings)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_Settings_t  *>(arg);
	            memcpy(&param->interface, "ETHERNET", sizeof("ETHERNET"));
                memcpy(&param->ipversion, "IPV4", sizeof("IPV4"));
                param->autoconfig  = true;
                memcpy(&param->ipaddress, "192.168.1.101", sizeof("192.168.1.101"));
                memcpy(&param->netmask, "255.255.255.0", sizeof("255.255.255.0"));
	            memcpy(&param->gateway, "192.168.1.1", sizeof("192.168.1.1"));
                memcpy(&param->primarydns, "192.168.1.1", sizeof("192.168.1.1"));
                memcpy(&param->secondarydns, "192.168.1.2", sizeof("192.168.1.2"));

                EXPECT_EQ(string(param->interface), string(_T("ETHERNET")));
                EXPECT_EQ(string(param->ipversion), string(_T("IPV4")));
                EXPECT_EQ(string(param->ipaddress), string(_T("192.168.1.101")));
                EXPECT_EQ(string(param->netmask), string(_T("255.255.255.0")));
                EXPECT_EQ(string(param->gateway), string(_T("192.168.1.1")));
                EXPECT_EQ(string(param->primarydns), string(_T("192.168.1.1")));
                EXPECT_EQ(string(param->secondarydns), string(_T("192.168.1.2")));
                EXPECT_EQ(param->autoconfig, true);

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getIPSettings"), _T("{\"interface\": \"ETHERNET\",\"ipversion\": \"IPV4\"}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"interface\":\"ETHERNET\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"ipversion\":\"IPV4\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"autoconfig\":true")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"ipaddr\":\"192.168.1.101\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"netmask\":\"255.255.255.0\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"gateway\":\"192.168.1.1\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"primarydns\":\"192.168.1.1\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"secondarydns\":\"192.168.1.2\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(NetworkTest, isConnectedToInternet)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_isConnectedToInternet)));
                *((bool*) arg) = true;

                EXPECT_EQ(*((bool*) arg), true);

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isConnectedToInternet"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"connectedToInternet\":true")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(NetworkTest, getPublicIP)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getPublicIP)));
                auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_StunRequest_t   *>(arg);
                memcpy(&param->public_ip, "69.136.49.95", sizeof("69.136.49.95"));

                EXPECT_EQ(string(param->public_ip), string(_T("69.136.49.95")));

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPublicIP"), _T("{\"iface\": \"WIFI\", \"ipv6\": false}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"public_ip\":\"69.136.49.95\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(NetworkTest, setInterfaceEnabled)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_setInterfaceEnabled)));

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setInterfaceEnabled"), _T("{\"interface\": \"WIFI\", \"enabled\": true, \"persist\": true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(NetworkTest, getSTBIPFamily)
{

    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getIPSettings)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_Settings_t  *>(arg);
                memcpy(&param->interface, "ETHERNET", sizeof("ETHERNET"));
                memcpy(&param->ipversion, "IPV4", sizeof("IPV4"));
                param->autoconfig  = true;
                memcpy(&param->ipaddress, "192.168.1.101", sizeof("192.168.1.101"));
                memcpy(&param->netmask, "255.255.255.0", sizeof("255.255.255.0"));
                memcpy(&param->gateway, "192.168.1.1", sizeof("192.168.1.1"));
                memcpy(&param->primarydns, "192.168.1.1", sizeof("192.168.1.1"));
                memcpy(&param->secondarydns, "192.168.1.2", sizeof("192.168.1.2"));

                EXPECT_EQ(string(param->interface), string(_T("ETHERNET")));
                EXPECT_EQ(string(param->ipversion), string(_T("IPV4")));
                EXPECT_EQ(string(param->ipaddress), string(_T("192.168.1.101")));
                EXPECT_EQ(string(param->netmask), string(_T("255.255.255.0")));
                EXPECT_EQ(string(param->gateway), string(_T("192.168.1.1")));
                EXPECT_EQ(string(param->primarydns), string(_T("192.168.1.1")));
                EXPECT_EQ(string(param->secondarydns), string(_T("192.168.1.2")));
                EXPECT_EQ(param->autoconfig, true);

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getIPSettings"), _T("{\"interface\": \"ETHERNET\",\"ipversion\": \"IPV4\"}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSTBIPFamily"), _T("{\"family\": \"AF_INET\"}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"ip\":\"192.168.1.101\"")));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(NetworkTest, setConnectivityTestEndpoints)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_setConnectivityTestEndpoints)));

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setConnectivityTestEndpoints"), _T("{\"endpoints\": [\"http://clients3.google.com/generate_204\"]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(NetworkTest, getInternetConnectionState)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getInternetConnectionState)));
                auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_InternetConnectivityStatus_t *>(arg);

				param->connectivityState = LIMITED_INTERNET;
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInternetConnectionState"), _T("{}"), response));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"state\":1")));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));

    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
    .Times(::testing::AnyNumber())
    .WillRepeatedly(
        [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
            EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
            EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getInternetConnectionState)));
            auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_InternetConnectivityStatus_t *>(arg);

			param->connectivityState = CAPTIVE_PORTAL;
			memcpy(param->captivePortalURI, "http://10.0.0.1/captiveportal.jst", sizeof("http://10.0.0.1/captiveportal.jst"));
            return IARM_RESULT_SUCCESS;
        });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInternetConnectionState"), _T("{}"), response));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"state\":2")));

	EXPECT_THAT(response, ::testing::ContainsRegex(_T("captiveportal.jst")));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("10.0.0.1")));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(NetworkTest, getCaptivePortalURI)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
    .Times(::testing::AnyNumber())
    .WillRepeatedly(
        [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
            EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
            EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getInternetConnectionState)));
            auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_InternetConnectivityStatus_t *>(arg);

			param->connectivityState = LIMITED_INTERNET;
            return IARM_RESULT_SUCCESS;
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getCaptivePortalURI"), _T("{}"), response));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"URI\":\"\"")));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
	EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
    .Times(::testing::AnyNumber())
    .WillRepeatedly(
        [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
            EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
            EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getInternetConnectionState)));
            auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_InternetConnectivityStatus_t *>(arg);

			param->connectivityState = CAPTIVE_PORTAL;
			memcpy(param->captivePortalURI, "http://10.0.0.1/captiveportal.jst", sizeof("http://10.0.0.1/captiveportal.jst"));
            return IARM_RESULT_SUCCESS;
        });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getCaptivePortalURI"), _T("{}"), response));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"URI\"")));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("captiveportal.jst")));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("10.0.0.1")));
	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(NetworkTest, startConnectivityMonitoring)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_monitorConnectivity)));
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startConnectivityMonitoring"), _T("{\"interval\": 900}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("startConnectivityMonitoring"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("")));
}

TEST_F(NetworkTest, stopConnectivityMonitoring)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_stopConnectivityMonitoring)));

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopConnectivityMonitoring"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(NetworkTest, setDefaultInterface)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_setDefaultInterface)));

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setDefaultInterface"), _T("{\"interface\": \"WIFI\", \"persist\": true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(NetworkTest, setIPSettings)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_setIPSettings)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_Settings_t  *>(arg);
                param->isSupported = true;
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setIPSettings"), _T("{\"interface\": \"WIFI\", \"ipversion\": \"IPV4\",\"autoconfig\": true,\"ipaddr\": \"192.168.1.101\",\"netmask\": \"255.255.255.0\",\"gateway\": \"192.168.1.1\",\"primarydns\": \"192.168.1.1\",\"secondarydns\": \"192.168.1.2\"}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"supported\":true")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

//TEST_F(NetworkTest, trace)
//{
//    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
//        .Times(::testing::AnyNumber())
//        .WillRepeatedly(
//            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
//                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
//                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getDefaultInterface)));
//
//                auto param = static_cast<IARM_BUS_NetSrvMgr_DefaultRoute_t *>(arg);
//                memcpy(&param->interface, "eth0", sizeof("eth0"));
//                memcpy(&param->gateway, "45.57.221.20", sizeof("45.57.221.20"));
//
//                return IARM_RESULT_SUCCESS;
//            });
//    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDefaultInterface"), _T("{}"), response));
//    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("trace"), _T("{\"endpoint\":\"45.57.221.20\", \"packets\":5}"), response));
//	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
//}
//
//TEST_F(NetworkTest, traceNamedEndpoint)
//{
//	EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
//        .Times(::testing::AnyNumber())
//        .WillRepeatedly(
//            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
//                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
//                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getDefaultInterface)));
//
//                auto param = static_cast<IARM_BUS_NetSrvMgr_DefaultRoute_t *>(arg);
//                memcpy(&param->interface, "eth0", sizeof("eth0"));
//				memcpy(&param->gateway, "45.57.221.20", sizeof("45.57.221.20"));
//
//                return IARM_RESULT_SUCCESS;
//            });
//    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDefaultInterface"), _T("{}"), response));
//    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("traceNamedEndpoint"), _T("{\"endpointName\": \"CMTS\", \"packets\": 5}"), response));
//    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
//}

TEST_F(NetworkTest, getNamedEndpoints)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getNamedEndpoints"), _T("{}"), response));
	EXPECT_EQ(response, string("{\"endpoints\":[\"CMTS\"],\"success\":true}"));
}

//TEST_F(NetworkTest, pingNamedEndpoint)
//{
//	EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
//        .Times(::testing::AnyNumber())
//        .WillRepeatedly(
//            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
//                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
//                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getDefaultInterface)));
//
//                auto param = static_cast<IARM_BUS_NetSrvMgr_DefaultRoute_t *>(arg);
//                memcpy(&param->interface, "eth0", sizeof("eth0"));
//                memcpy(&param->gateway, "127.0.0.1", sizeof("127.0.0.1"));
//
//                return IARM_RESULT_SUCCESS;
//            });
//    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDefaultInterface"), _T("{}"), response));
//    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("pingNamedEndpoint"), _T("{\"endpointName\": \"CMTS\", \"packets\": 5, \"guid\": \"...\"}"), response));
//   	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"target\":\"127.0.0.1\"")));
//	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
//	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"packetsTransmitted\":5")));
//	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"packetsReceived\":5")));
//	EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"packetLoss\":\" 0\"")));
//}
//
//TEST_F(NetworkTest, ping)
//{
//	EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
//        .Times(::testing::AnyNumber())
//        .WillRepeatedly(
//            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
//                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
//                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getDefaultInterface)));
//
//                auto param = static_cast<IARM_BUS_NetSrvMgr_DefaultRoute_t *>(arg);
//                memcpy(&param->interface, "eth0", sizeof("eth0"));
//                memcpy(&param->gateway, "192.168.1.1", sizeof("192.168.1.1"));
//
//                return IARM_RESULT_SUCCESS;
//            });
//    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDefaultInterface"), _T("{}"), response));
//    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("ping"), _T("{\"endpoint\": \"127.0.0.1\", \"packets\": 5, \"guid\": \"...\"}"), response));
//    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"target\":\"127.0.0.1\"")));
//    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
//    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"packetsTransmitted\":5")));
//    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"packetsReceived\":5")));
//    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"packetLoss\":\" 0\"")));
//}

TEST_F(NetworkTest, getQuirks)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getQuirks"), _T("{}"), response));
	EXPECT_EQ(response, string("{\"quirks\":[\"RDK-20093\"],\"success\":true}"));
}
