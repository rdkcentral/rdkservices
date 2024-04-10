#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include "Network.h"
#include "NetworkConnectivity.h"

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

    WrapsImplMock   *p_wrapsImplMock   = nullptr ;
    IarmBusImplMock   *p_iarmBusImplMock = nullptr ;

    NetworkTestBase()
    {
        p_wrapsImplMock  = new NiceMock <WrapsImplMock>;
        Wraps::setImpl(p_wrapsImplMock);

        p_iarmBusImplMock  = new NiceMock <IarmBusImplMock>;
        IarmBus::setImpl(p_iarmBusImplMock);

        ofstream file("/etc/device.properties");
        file << "DEVICE_TYPE=mediaclient\n";
        file << "WIFI_SUPPORT=true\n";
        file << "MOCA_INTERFACE=true\n";
        file <<"WIFI_INTERFACE==wlan0\n";
        file <<"MOCA_INTERFACE=eth0\n";
        file <<"ETHERNET_INTERFACE=eth0\n";
        file.close();

        ON_CALL(*p_wrapsImplMock, popen(::testing::_, ::testing::_))
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

        EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call_with_IPCTimeout)
            .Times(::testing::AnyNumber())
            .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen, int timeout) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_isAvailable)));
                return IARM_RESULT_SUCCESS;
     });

        ON_CALL(*p_iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
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
            return IARM_RESULT_SUCCESS;
        }));

        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }

    virtual ~NetworkTest() override
    {
        Wraps::setImpl(nullptr);
        if (p_wrapsImplMock != nullptr)
        {
            delete p_wrapsImplMock;
            p_wrapsImplMock = nullptr;
        }

        IarmBus::setImpl(nullptr);
        if (p_iarmBusImplMock != nullptr)
        {
            delete p_iarmBusImplMock;
            p_iarmBusImplMock = nullptr;
        }
    }
};

class NetworkInitializedEventTest : public NetworkTest {
protected:
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;


    NetworkInitializedEventTest()
        : NetworkTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }

    virtual ~NetworkInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();
        PluginHost::IFactories::Assign(nullptr);

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
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getInternetConnectionState")));
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
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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

TEST_F(NetworkTest, getStbIp_cache)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStbIp"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"ip\":\"192.168.1.101\",\"success\":true}"));

}

TEST_F(NetworkTest, getNullStbIp)
{
         EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getSTBip)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_EventData_t*>(arg);
                EXPECT_EQ(string(param->activeIfaceIpaddr), string(_T("")));

                return IARM_RESULT_SUCCESS;
           });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStbIp"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"ip\":\"\",\"success\":true}"));
}

TEST_F(NetworkTest, getFailedStbIp)
{
         EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getSTBip)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_EventData_t*>(arg);
                EXPECT_EQ(string(param->activeIfaceIpaddr), string(_T("")));

                return IARM_RESULT_IPCCORE_FAIL;
           });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getStbIp"), _T("{}"), response));

}

TEST_F(NetworkTest, getInterfaces)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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

TEST_F(NetworkTest, getInterfacesFailed)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getInterfaceList)));

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getInterfaces"), _T("{}"), response));
}

TEST_F(NetworkTest, isInterfaceEnabled)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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

TEST_F(NetworkTest, isInterfaceEnabled_failed)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("isInterfaceEnabled"), _T("{\"interface\": \"ETHERNET\"}"), response));
}

TEST_F(NetworkTest, getDefaultInterface)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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

TEST_F(NetworkTest, getDefaultInterface_cache)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDefaultInterface"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"interface\":\"ETHERNET\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(NetworkTest, isInterfaceEnabled_WrongIface)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("isInterfaceEnabled"), _T("{\"interface\": \"TEST\"}"), response));
}

TEST_F(NetworkTest, isInterfaceEnabled_Fail)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("isInterfaceEnabled"), _T("{\"interface\": \"TEST\"}"), response));
}

TEST_F(NetworkTest, getIPSettings)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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

TEST_F(NetworkTest, getIP6Settings)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getIPSettings)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_Settings_t  *>(arg);
                    memcpy(&param->interface, "ETHERNET", sizeof("ETHERNET"));
                memcpy(&param->ipversion, "IPV6", sizeof("IPV6"));
                param->autoconfig  = true;
                memcpy(&param->ipaddress, "192.168.1.101", sizeof("192.168.1.101"));
                memcpy(&param->netmask, "255.255.255.0", sizeof("255.255.255.0"));
                    memcpy(&param->gateway, "192.168.1.1", sizeof("192.168.1.1"));
                memcpy(&param->primarydns, "192.168.1.1", sizeof("192.168.1.1"));
                memcpy(&param->secondarydns, "192.168.1.2", sizeof("192.168.1.2"));

                EXPECT_EQ(string(param->interface), string(_T("ETHERNET")));
                EXPECT_EQ(string(param->ipversion), string(_T("IPV6")));
                EXPECT_EQ(string(param->ipaddress), string(_T("192.168.1.101")));
                EXPECT_EQ(string(param->netmask), string(_T("255.255.255.0")));
                EXPECT_EQ(string(param->gateway), string(_T("192.168.1.1")));
                EXPECT_EQ(string(param->primarydns), string(_T("192.168.1.1")));
                EXPECT_EQ(string(param->secondarydns), string(_T("192.168.1.2")));
                EXPECT_EQ(param->autoconfig, true);

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getIPSettings"), _T("{\"interface\": \"ETHERNET\",\"ipversion\": \"IPV6\"}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"interface\":\"ETHERNET\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"ipversion\":\"IPV6\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"autoconfig\":true")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"ipaddr\":\"192.168.1.101\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"netmask\":\"255.255.255.0\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"gateway\":\"192.168.1.1\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"primarydns\":\"192.168.1.1\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"secondarydns\":\"192.168.1.2\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(NetworkTest, getIPSettings_wifi)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getIPSettings)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_Settings_t  *>(arg);
                    memcpy(&param->interface, "WIFI", sizeof("WIFI"));
                memcpy(&param->ipversion, "IPV4", sizeof("IPV4"));
                param->autoconfig  = true;
                memcpy(&param->ipaddress, "192.168.1.101", sizeof("192.168.1.101"));
                memcpy(&param->netmask, "255.255.255.0", sizeof("255.255.255.0"));
                    memcpy(&param->gateway, "192.168.1.1", sizeof("192.168.1.1"));
                memcpy(&param->primarydns, "192.168.1.1", sizeof("192.168.1.1"));
                memcpy(&param->secondarydns, "192.168.1.2", sizeof("192.168.1.2"));

                EXPECT_EQ(string(param->interface), string(_T("WIFI")));
                EXPECT_EQ(string(param->ipversion), string(_T("IPV4")));
                EXPECT_EQ(string(param->ipaddress), string(_T("192.168.1.101")));
                EXPECT_EQ(string(param->netmask), string(_T("255.255.255.0")));
                EXPECT_EQ(string(param->gateway), string(_T("192.168.1.1")));
                EXPECT_EQ(string(param->primarydns), string(_T("192.168.1.1")));
                EXPECT_EQ(string(param->secondarydns), string(_T("192.168.1.2")));
                EXPECT_EQ(param->autoconfig, true);

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getIPSettings"), _T("{\"interface\": \"WIFI\",\"ipversion\": \"IPV4\"}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"interface\":\"WIFI\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"ipversion\":\"IPV4\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"autoconfig\":true")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"ipaddr\":\"192.168.1.101\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"netmask\":\"255.255.255.0\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"gateway\":\"192.168.1.1\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"primarydns\":\"192.168.1.1\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"secondarydns\":\"192.168.1.2\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));

}

TEST_F(NetworkTest, getIP6Settings_wifi)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getIPSettings)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_Settings_t  *>(arg);
                    memcpy(&param->interface, "WIFI", sizeof("WIFI"));
                memcpy(&param->ipversion, "IPV6", sizeof("IPV6"));
                param->autoconfig  = true;
                memcpy(&param->ipaddress, "192.168.1.101", sizeof("192.168.1.101"));
                memcpy(&param->netmask, "255.255.255.0", sizeof("255.255.255.0"));
                    memcpy(&param->gateway, "192.168.1.1", sizeof("192.168.1.1"));
                memcpy(&param->primarydns, "192.168.1.1", sizeof("192.168.1.1"));
                memcpy(&param->secondarydns, "192.168.1.2", sizeof("192.168.1.2"));

                EXPECT_EQ(string(param->interface), string(_T("WIFI")));
                EXPECT_EQ(string(param->ipversion), string(_T("IPV6")));
                EXPECT_EQ(string(param->ipaddress), string(_T("192.168.1.101")));
                EXPECT_EQ(string(param->netmask), string(_T("255.255.255.0")));
                EXPECT_EQ(string(param->gateway), string(_T("192.168.1.1")));
                EXPECT_EQ(string(param->primarydns), string(_T("192.168.1.1")));
                EXPECT_EQ(string(param->secondarydns), string(_T("192.168.1.2")));
                EXPECT_EQ(param->autoconfig, true);

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getIPSettings"), _T("{\"interface\": \"WIFI\",\"ipversion\": \"IPV6\"}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"interface\":\"WIFI\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"ipversion\":\"IPV6\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"autoconfig\":true")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"ipaddr\":\"192.168.1.101\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"netmask\":\"255.255.255.0\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"gateway\":\"192.168.1.1\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"primarydns\":\"192.168.1.1\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"secondarydns\":\"192.168.1.2\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));

}

TEST_F(NetworkTest, getIPSettings_Failed)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getIPSettings"), _T("{\"interface\": \"ETHERNET\",\"ipversion\": \"IPV4\"}"), response));
}

TEST_F(NetworkTest, getIPSettings_WrongIface)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getIPSettings"), _T("{\"interface\": \"TEST\",\"ipversion\": \"IPV4\"}"), response));
}

TEST_F(NetworkTest, getPublicIP)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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

TEST_F(NetworkTest, setInterfaceEnabled_Failed)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_setInterfaceEnabled)));

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setInterfaceEnabled"), _T("{\"interface\": \"WIFI\", \"enabled\": true, \"persist\": true}"), response));
}

TEST_F(NetworkTest, setInterfaceEnabled_WrongIface)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_setInterfaceEnabled)));

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setInterfaceEnabled"), _T("{\"interface\": \"TEST\", \"enabled\": true, \"persist\": true}"), response));
}

TEST_F(NetworkTest, getSTBIPFamily)
{

    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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

TEST_F(NetworkTest, getSTBIPFamily_Error)
{

    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getSTBIPFamily"), _T("{\"test\": \"AF_INET\"}"), response));
}

TEST_F(NetworkTest, getSTBIPFamily_Failed)
{

    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getIPSettings"), _T("{\"interface\": \"ETHERNET\",\"ipversion\": \"IPV4\"}"), response));
}

TEST_F(NetworkTest, setDefaultInterface)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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

TEST_F(NetworkTest, setDefaultInterface_WrongIface)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_setDefaultInterface)));

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setDefaultInterface"), _T("{\"interface\": \"TEST\", \"persist\": true}"), response));
}

TEST_F(NetworkTest, setDefaultInterface_failed)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_setDefaultInterface)));

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setDefaultInterface"), _T("{\"interface\": \"TEST\", \"persist\": true}"), response));
}

TEST_F(NetworkTest, setIPSettings)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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
TEST_F(NetworkTest, trace_fail)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getDefaultInterface)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_DefaultRoute_t *>(arg);
                memcpy(&param->interface, "eth0", sizeof("eth0"));
                memcpy(&param->gateway, "45.57.221.20", sizeof("45.57.221.20"));

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("trace"), _T("{\"endpoint\":\"45.57.221.20\", \"packets\":5}"), response));
}

TEST_F(NetworkTest, trace_noiface)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getDefaultInterface)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_DefaultRoute_t *>(arg);
                memcpy(&param->interface, "eth0", sizeof("eth0"));
                memcpy(&param->gateway, "45.57.221.20", sizeof("45.57.221.20"));

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("trace"), _T("{\"test\":\"45.57.221.20\", \"packets\":5}"), response));
}

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

TEST_F(NetworkTest, traceNamedEndpoint_noendoint)
{
      EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
           [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getDefaultInterface)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_DefaultRoute_t *>(arg);
                memcpy(&param->interface, "eth0", sizeof("eth0"));
                              memcpy(&param->gateway, "45.57.221.20", sizeof("45.57.221.20"));

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDefaultInterface"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("traceNamedEndpoint"), _T("{\"test\": \"CMTS\", \"packets\": 5}"), response));
}

TEST_F(NetworkTest, traceNamedEndpoint_fail)
{
      EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getDefaultInterface)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_DefaultRoute_t *>(arg);
                memcpy(&param->interface, "eth0", sizeof("eth0"));
                              memcpy(&param->gateway, "45.57.221.20", sizeof("45.57.221.20"));

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("traceNamedEndpoint"), _T("{\"endpointName\": \"CMTS\", \"packets\": 5}"), response));
}

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

TEST_F(NetworkTest, pingNamedEndpoint_noarg)
{
        EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getDefaultInterface)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_DefaultRoute_t *>(arg);
                memcpy(&param->interface, "eth0", sizeof("eth0"));
                memcpy(&param->gateway, "127.0.0.1", sizeof("127.0.0.1"));

                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDefaultInterface"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("pingNamedEndpoint"), _T("{\"test\": \"CMTS\", \"packets\": 5, \"guid\": \"...\"}"), response));
}

TEST_F(NetworkTest, pingNamedEndpoint_fail)
{
      EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getDefaultInterface)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_DefaultRoute_t *>(arg);
                memcpy(&param->interface, "eth0", sizeof("eth0"));
                memcpy(&param->gateway, "127.0.0.1", sizeof("127.0.0.1"));

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("pingNamedEndpoint"), _T("{\"endpointName\": \"CMTS\", \"packets\": 5, \"guid\": \"...\"}"), response));
}

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

TEST_F(NetworkTest, ping_noendpoint)
{
        EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("ping"), _T("{\"test\": \"127.0.0.1\", \"packets\": 5, \"guid\": \"...\"}"), response));
}

TEST_F(NetworkTest, ping_fail)
{
      EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getDefaultInterface)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_DefaultRoute_t *>(arg);
                memcpy(&param->interface, "eth0", sizeof("eth0"));
                memcpy(&param->gateway, "192.168.1.1", sizeof("192.168.1.1"));

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("ping"), _T("{\"endpoint\": \"127.0.0.1\", \"packets\": 5, \"guid\": \"...\"}"), response));
}

TEST_F(NetworkTest, ping_fail1)
{
      EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_getDefaultInterface)));

                auto param = static_cast<IARM_BUS_NetSrvMgr_DefaultRoute_t *>(arg);
                memcpy(&param->interface, "eth0", sizeof("eth0"));
                memcpy(&param->gateway, "192.168.1.1", sizeof("192.168.1.1"));

                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("ping"), _T("{\"test\": \"127.0.0.1\", \"packets\": 5, \"guid\": \"...\"}"), response));
}

TEST_F(NetworkTest, getQuirks)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getQuirks"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"quirks\":[\"RDK-20093\"],\"success\":true}"));
}

TEST_F(NetworkTest, getInternetConnectionState)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setConnectivityTestEndpoints"), _T("{\"endpoints\": [\"http://localhost:8000\"]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInternetConnectionState"), _T("{\"ipversion\": \"IPV6\"}"), response));
    EXPECT_EQ(response, string("{\"state\":0,\"ipversion\":\"IPV6\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInternetConnectionState"), _T("{\"ipversion\": \"IPV4\"}"), response));
    EXPECT_EQ(response, string("{\"state\":0,\"ipversion\":\"IPV4\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInternetConnectionState"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"state\":0,\"success\":true}"));
}

TEST_F(NetworkTest, getInternetConnectionState_cache)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setConnectivityTestEndpoints"), _T("{\"endpoints\": [\"http://localhost:8000\"]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInternetConnectionState"), _T("{\"ipversion\": \"IPV4\"}"), response));
    EXPECT_EQ(response, string("{\"state\":0,\"ipversion\":\"IPV4\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInternetConnectionState"), _T("{\"ipversion\": \"IPV4\"}"), response));
    EXPECT_EQ(response, string("{\"state\":0,\"ipversion\":\"IPV4\",\"success\":true}"));
}


TEST_F(NetworkTest, isConnectedToInternet)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setConnectivityTestEndpoints"), _T("{\"endpoints\": [\"http://localhost:8000\"]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isConnectedToInternet"), _T("{\"ipversion\": \"IPV6\"}"), response));
    EXPECT_EQ(response, string("{\"connectedToInternet\":false,\"ipversion\":\"IPV6\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isConnectedToInternet"), _T("{\"ipversion\": \"IPV4\"}"), response));
    EXPECT_EQ(response, string("{\"connectedToInternet\":false,\"ipversion\":\"IPV4\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isConnectedToInternet"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"connectedToInternet\":false,\"success\":true}"));
}

TEST_F(NetworkTest, isConnectedToInternet_cache)
{   
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setConnectivityTestEndpoints"), _T("{\"endpoints\": [\"http://localhost:8000\"]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isConnectedToInternet"), _T("{\"ipversion\": \"IPV6\"}"), response));
    EXPECT_EQ(response, string("{\"connectedToInternet\":false,\"ipversion\":\"IPV6\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isConnectedToInternet"), _T("{\"ipversion\": \"IPV6\"}"), response));
    EXPECT_EQ(response, string("{\"connectedToInternet\":false,\"ipversion\":\"IPV6\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isConnectedToInternet"), _T("{\"ipversion\": \"IPV4\"}"), response));
    EXPECT_EQ(response, string("{\"connectedToInternet\":false,\"ipversion\":\"IPV4\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isConnectedToInternet"), _T("{\"ipversion\": \"IPV4\"}"), response));
    EXPECT_EQ(response, string("{\"connectedToInternet\":false,\"ipversion\":\"IPV4\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isConnectedToInternet"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"connectedToInternet\":false,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isConnectedToInternet"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"connectedToInternet\":false,\"success\":true}"));
}

TEST_F(NetworkTest, getCaptivePortalURI)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getCaptivePortalURI"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"URI\":\"\",\"success\":true}"));
}

TEST_F(NetworkInitializedEventTest, ConnectivityMonitoring)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setConnectivityTestEndpoints"), _T("{\"endpoints\": [\"http://localhost:8000\"]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    Core::Event onInternetStatusChange(false, true);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.Network.onInternetStatusChange\",\"params\":{\"state\":0,\"status\":\"NO_INTERNET\"}}")));
                onInternetStatusChange.SetEvent();
                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onInternetStatusChange"), _T("org.rdk.Network"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startConnectivityMonitoring"), _T("{\"interval\":6}"), response));
    EXPECT_EQ(Core::ERROR_NONE, onInternetStatusChange.Lock());
    handler.Unsubscribe(0, _T("onInternetStatusChange"), _T("org.rdk.Network"), message);
    EXPECT_NE(Core::ERROR_GENERAL, handler.Invoke(connection, _T("stopConnectivityMonitoring"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(NetworkTest, setConnectivityTestEndpoints)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setConnectivityTestEndpoints"), _T("{\"endpoints\": [\"http://clients3.google.com/generate_204\"]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(NetworkTest, setStunEndPoint)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setStunEndPoint"), _T("{}"), response));
    //EXPECT_EQ(response, string("{\"quirks\":[\"RDK-20093\"],\"success\":true}"));
}

TEST_F(NetworkTest, configurePNI)
{
     EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_configurePNI)));
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("configurePNI"), _T("{}"), response));
}

TEST_F(NetworkTest, configurePNI_fail)
{
     EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_configurePNI)));
                return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("configurePNI"), _T("{}"), response));
}

TEST_F(NetworkInitializedEventTest, onInterfaceStatusChanged)
{
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
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
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.Network.onInterfaceStatusChanged\",\"params\":{\"interface\":\"ETHERNET\",\"enabled\":true}}")));
                return Core::ERROR_NONE;
            }));
    IARM_BUS_NetSrvMgr_Iface_EventInterfaceEnabledStatus_t intData;
    intData.status = 1;
    strcpy(intData.interface,"eth0");
    handler.Subscribe(0, _T("onInterfaceStatusChanged"), _T("org.rdk.Network"), message);
    plugin->eventHandler("NET_SRV_MGR", IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_ENABLED_STATUS, static_cast<void*>(&intData), sizeof(intData));
    handler.Unsubscribe(0, _T("onInterfaceStatusChanged"), _T("org.rdk.Network"), message);
}

TEST_F(NetworkInitializedEventTest, onConnectionStatusChanged)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setConnectivityTestEndpoints"), _T("{\"endpoints\": [\"http://localhost:8000\"]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    
    Core::Event onInternetStatusChange(false, true);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.Network.onConnectionStatusChanged\",\"params\":{\"interface\":\"ETHERNET\",\"status\":\"CONNECTED\"}}")));
                return Core::ERROR_NONE;
            }))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.Network.onInternetStatusChange\",\"params\":{\"state\":0,\"status\":\"NO_INTERNET\"}}")));
                onInternetStatusChange.SetEvent();
            return Core::ERROR_NONE;
        }));

    IARM_BUS_NetSrvMgr_Iface_EventInterfaceConnectionStatus_t intData;
    intData.status = 1;
    strcpy(intData.interface,"eth0");
    handler.Subscribe(0, _T("onConnectionStatusChanged"), _T("org.rdk.Network"), message);
    handler.Subscribe(0, _T("onInternetStatusChange"), _T("org.rdk.Network"), message);
    plugin->eventHandler("NET_SRV_MGR", IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_CONNECTION_STATUS, static_cast<void*>(&intData), sizeof(intData));
    handler.Unsubscribe(0, _T("onConnectionStatusChanged"), _T("org.rdk.Network"), message);
    EXPECT_EQ(Core::ERROR_NONE, onInternetStatusChange.Lock());
    handler.Unsubscribe(0, _T("onInternetStatusChange"), _T("org.rdk.Network"), message);
    EXPECT_NE(Core::ERROR_GENERAL, handler.Invoke(connection, _T("stopConnectivityMonitoring"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(NetworkInitializedEventTest, onIPAddressStatusChanged)
{
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.Network.onIPAddressStatusChanged\",\"params\":{\"interface\":\"ETHERNET\",\"ip4Address\":\"192.168.1.10\",\"status\":\"ACQUIRED\"}}")));
                return Core::ERROR_NONE;
            }));
    IARM_BUS_NetSrvMgr_Iface_EventInterfaceIPAddress_t intData;
    intData.is_ipv6 = 0;
    strcpy(intData.interface,"eth0");
    intData.acquired = 1;
    strcpy(intData.ip_address,"192.168.1.10");
    handler.Subscribe(0, _T("onIPAddressStatusChanged"), _T("org.rdk.Network"), message);
    plugin->eventHandler("NET_SRV_MGR", IARM_BUS_NETWORK_MANAGER_EVENT_INTERFACE_IPADDRESS, static_cast<void*>(&intData), sizeof(intData));
    handler.Unsubscribe(0, _T("onIPAddressStatusChanged"), _T("org.rdk.Network"), message);
}

TEST_F(NetworkInitializedEventTest, onDefaultInterfaceChanged)
{
/*
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInterfaces"), _T("{}"), response));
        EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"interface\":\"ETHERNET\"")));
        EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"macAddress\":\"AA:AA:AA:AA:AA:AA\"")));
        EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"enabled\":true")));
        EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"connected\":true")));
        EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
*/
    Core::Event onDefaultInterfaceChanged(false, true);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.Network.onDefaultInterfaceChanged\",\"params\":{\"oldInterfaceName\":\"\",\"newInterfaceName\":\"ETHERNET\"}}")));
                onDefaultInterfaceChanged.SetEvent();
                return Core::ERROR_NONE;
            }));
    IARM_BUS_NetSrvMgr_Iface_EventDefaultInterface_t intData;
    strcpy(intData.newInterface,"eth0");
    handler.Subscribe(0, _T("onDefaultInterfaceChanged"), _T("org.rdk.Network"), message);
    plugin->eventHandler("NET_SRV_MGR", IARM_BUS_NETWORK_MANAGER_EVENT_DEFAULT_INTERFACE, static_cast<void*>(&intData), sizeof(intData));
    EXPECT_EQ(Core::ERROR_NONE, onDefaultInterfaceChanged.Lock());
    handler.Unsubscribe(0, _T("onDefaultInterfaceChanged"), _T("org.rdk.Network"), message);
}

TEST_F(NetworkInitializedEventTest, onInternetStatusChange)
{
    Core::Event onInternetStatusChange(false, true);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.Network.onInternetStatusChange\",\"params\":{\"state\":0,\"status\":\"NO_INTERNET\"}}")));
                onInternetStatusChange.SetEvent();
                return Core::ERROR_NONE;
            }));
    nsm_internetState  intData = NO_INTERNET;
    handler.Subscribe(0, _T("onInternetStatusChange"), _T("org.rdk.Network"), message);
    plugin->notifyInternetStatusChange(intData);
    EXPECT_EQ(Core::ERROR_NONE, onInternetStatusChange.Lock());
    handler.Unsubscribe(0, _T("onInternetStatusChange"), _T("org.rdk.Network"), message);
}
