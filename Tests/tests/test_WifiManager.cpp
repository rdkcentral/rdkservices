#include <gtest/gtest.h>
#include <fstream>
#include "WifiManager.h"

#include "ServiceMock.h"
#include "IarmBusMock.h"
#include "WrapsMock.h"
#include "FactoriesImplementation.h"
#include "WifiManagerDefines.h"

using namespace WPEFramework;

using ::testing::NiceMock;

class WifiManagerTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::WifiManager> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;
    Core::JSONRPC::Message message;
    WrapsImplMock wrapsImplMock;
    NiceMock<IarmBusImplMock> iarmBusImplMock;
    ServiceMock services;

    WifiManagerTest()
        : plugin(Core::ProxyType<Plugin::WifiManager>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
        Wraps::getInstance().impl = &wrapsImplMock;
        IarmBus::getInstance().impl = &iarmBusImplMock;
    }
    virtual ~WifiManagerTest() override
    {
        Wraps::getInstance().impl = nullptr;
        IarmBus::getInstance().impl = nullptr;
    }
};

class WifiInitializedEventTest : public WifiManagerTest {
protected:
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;


    WifiInitializedEventTest()
        : WifiManagerTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&services);
    }

    virtual ~WifiInitializedEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();
        PluginHost::IFactories::Assign(nullptr);

    }
};


TEST_F(WifiManagerTest, TestedAPIsShouldExist)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("startScan")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("stopScan")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("connect")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("disconnect")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("cancelWPSPairing")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("saveSSID")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("clearSSID")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setSignalThresholdChangeEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPairedSSID")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPairedSSIDInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isPaired")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getCurrentState")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getConnectedSSID")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getQuirks")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isSignalThresholdChangeEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSupportedSecurityModes")));
}


TEST_F(WifiManagerTest, setEnabled)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_NETSRVMGR_API_setInterfaceEnabled)));
                auto param = static_cast<IARM_BUS_NetSrvMgr_Iface_EventData_t*>(arg);
                EXPECT_EQ(param->isInterfaceEnabled, true);
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnabled"), _T("{\"enable\":true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(WifiManagerTest, getCurrentState)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_getCurrentState)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                param->data.wifiStatus = WIFI_DISCONNECTED;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getCurrentState"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"state\":2,\"success\":true}")); // WIFI_PAIRING enum 3
}

TEST_F(WifiManagerTest, getPairedSSID)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_getPairedSSIDInfo)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                memcpy(&param->data.getPairedSSID, "rdk.test.wifi.network", sizeof("rdk.test.wifi.network"));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPairedSSID"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"ssid\":\"rdk.test.wifi.network\",\"success\":true}"));
}

TEST_F(WifiManagerTest, getPairedSSIDi_cache)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_getPairedSSIDInfo)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                memcpy(&param->data.getPairedSSID, "rdk.test.wifi.network", sizeof("rdk.test.wifi.network"));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPairedSSID"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"ssid\":\"rdk.test.wifi.network\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPairedSSID"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"ssid\":\"rdk.test.wifi.network\",\"success\":true}"));
}

TEST_F(WifiManagerTest, getPairedSSID_failed)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_getPairedSSIDInfo)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                memcpy(&param->data.getPairedSSID, "rdk.test.wifi.network", sizeof("rdk.test.wifi.network"));
                return IARM_RESULT_IPCCORE_FAIL;
            });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPairedSSID"), _T("{}"), response));
}

TEST_F(WifiManagerTest, getPairedSSIDInfo)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_getPairedSSIDInfo)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                memcpy(&param->data.getPairedSSIDInfo.ssid, "rdk.test.wifi.network", sizeof("rdk.test.wifi.network"));
                memcpy(&param->data.getPairedSSIDInfo.bssid, "ff:ff:ff:ff:ff:ff", sizeof("ff:ff:ff:ff:ff:ff"));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPairedSSIDInfo"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"ssid\":\"rdk.test.wifi.network\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"bssid\":\"ff:ff:ff:ff:ff:ff\"")));
}

TEST_F(WifiManagerTest, getPairedSSIDInfo_cache)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_getPairedSSIDInfo)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                memcpy(&param->data.getPairedSSIDInfo.ssid, "rdk.test.wifi.network", sizeof("rdk.test.wifi.network"));
                memcpy(&param->data.getPairedSSIDInfo.bssid, "ff:ff:ff:ff:ff:ff", sizeof("ff:ff:ff:ff:ff:ff"));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPairedSSIDInfo"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"ssid\":\"rdk.test.wifi.network\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"bssid\":\"ff:ff:ff:ff:ff:ff\"")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPairedSSIDInfo"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"ssid\":\"rdk.test.wifi.network\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"bssid\":\"ff:ff:ff:ff:ff:ff\"")));
}

TEST_F(WifiManagerTest, getPairedSSIDInfo_fail)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_getPairedSSIDInfo)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                memcpy(&param->data.getPairedSSIDInfo.ssid, "rdk.test.wifi.network", sizeof("rdk.test.wifi.network"));
                memcpy(&param->data.getPairedSSIDInfo.bssid, "ff:ff:ff:ff:ff:ff", sizeof("ff:ff:ff:ff:ff:ff"));
                return IARM_RESULT_IPCCORE_FAIL;
            });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPairedSSIDInfo"), _T("{}"), response));
}


TEST_F(WifiManagerTest, getConnectedSSID)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_getConnectedSSID)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                memcpy(&param->data.getConnectedSSID.ssid, "rdk.test.wifi.network", sizeof("rdk.test.wifi.network"));
                memcpy(&param->data.getConnectedSSID.bssid, "ff:ff:ff:ff:ff:ff", sizeof("ff:ff:ff:ff:ff:ff"));
                param->data.getConnectedSSID.rate = 130.000;
                param->data.getConnectedSSID.noise = -88.000;
                param->data.getConnectedSSID.securityMode = NET_WIFI_SECURITY_WPA2_PSK_AES;
                param->data.getConnectedSSID.signalStrength = -33.00;
                param->data.getConnectedSSID.frequency = 2437.00;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getConnectedSSID"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"ssid\":\"rdk.test.wifi.network\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"bssid\":\"ff:ff:ff:ff:ff:ff\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"rate\":\"130.000000\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"noise\":\"-88.000000\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"frequency\":\"2.437000\"")));
}

TEST_F(WifiManagerTest, connect)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_connect)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                param->status = true;
                EXPECT_EQ(string(param->data.connect.ssid), string(_T("123412341234")));
                EXPECT_EQ(string(param->data.connect.passphrase), string(_T("password")));
                EXPECT_EQ(param->data.connect.security_mode, NET_WIFI_SECURITY_WEP_128); //securityMode: 2 (NET_WIFI_SECURITY_WEP_128)
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("connect"), _T("{\"ssid\": \"123412341234\",\"passphrase\": \"password\",\"securityMode\": 2}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(WifiManagerTest, disconnect)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_disconnectSSID)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                param->status = true;
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("disconnect"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"result\":0,\"success\":true}"));
}

TEST_F(WifiManagerTest, saveSSID)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_saveSSID)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                param->status = true;
                EXPECT_EQ(string(param->data.connect.ssid), string(_T("123412341234")));
                EXPECT_EQ(string(param->data.connect.passphrase), string(_T("password")));
                EXPECT_EQ(param->data.connect.security_mode, NET_WIFI_SECURITY_WEP_128); //securityMode: 2 (NET_WIFI_SECURITY_WEP_128)
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("saveSSID"), _T("{\"ssid\": \"123412341234\",\"passphrase\": \"password\",\"securityMode\": 2}"), response));
    EXPECT_EQ(response, string("{\"result\":0,\"success\":true}"));
}

TEST_F(WifiManagerTest, clearSSID)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_clearSSID)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                param->status = true;
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("clearSSID"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"result\":0,\"success\":true}"));
}

TEST_F(WifiManagerTest, isPaired)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_getPairedSSIDInfo)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                memcpy(&param->data.getPairedSSIDInfo.ssid, "123412341234", sizeof("123412341234"));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isPaired"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"result\":0,\"success\":true}"));
}

TEST_F(WifiManagerTest, isPaired_cache)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_getPairedSSIDInfo)));
                auto param = static_cast<IARM_Bus_WiFiSrvMgr_Param_t*>(arg);
                memcpy(&param->data.getPairedSSIDInfo.ssid, "123412341234", sizeof("123412341234"));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isPaired"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"result\":0,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isPaired"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"result\":0,\"success\":true}"));
}

TEST_F(WifiManagerTest, startScan)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_getAvailableSSIDsAsync)));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"), _T("{\"incremental\": false,\"ssid\": \"...\",\"frequency\": \"...\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_getAvailableSSIDsAsyncIncr)));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"), _T("{\"incremental\": true,\"ssid\": \"...\",\"frequency\": \"...\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(WifiManagerTest, stopScan)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_stopProgressiveWifiScanning)));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopScan"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(WifiManagerTest, cancelWPSPairing)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_cancelWPSPairing)));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("cancelWPSPairing"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"result\":\"\",\"success\":true}"));
}

TEST_F(WifiManagerTest, initiateWPSPairing)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_initiateWPSPairing)));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("initiateWPSPairing"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"result\":\"\",\"success\":true}"));
}

TEST_F(WifiManagerTest, initiateWPSPairing2)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_NM_SRV_MGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_WIFI_MGR_API_initiateWPSPairing2)));
                return IARM_RESULT_SUCCESS;
            });


    JsonObject jsonObj;
    JsonObject res;
    jsonObj["method"] = "PIN";
    jsonObj["wps_pin"] = "88888888";
    uint32_t result = plugin->initiateWPSPairing2(jsonObj,res);
    EXPECT_EQ(result, 0);
}


TEST_F(WifiManagerTest, SignalThresholdChangeEnabled)
{
    ServiceMock service;
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isSignalThresholdChangeEnabled"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"result\":1,\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setSignalThresholdChangeEnabled"), _T("{\"enabled\": true, \"interval\":2000}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isSignalThresholdChangeEnabled"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"result\":0,\"success\":true}"));
    plugin->Deinitialize(&service);
}

TEST_F(WifiManagerTest, getSupportedSecurityModes)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSupportedSecurityModes"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(WifiManagerTest, getQuirks)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getQuirks"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(WifiInitializedEventTest, onWIFIStateChanged)
{
    EXPECT_CALL(services, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.WifiManager.onWIFIStateChanged\",\"params\":{\"state\":1,\"isLNF\":false}}")));
                return Core::ERROR_NONE;
            }));
    WPEFramework::Plugin::WifiState intData = WPEFramework::Plugin::WifiState::DISABLED;
    handler.Subscribe(0, _T("onWIFIStateChanged"), _T("org.rdk.WifiManager"), message);
    plugin->onWIFIStateChanged(intData,false);
    handler.Unsubscribe(0, _T("onWIFIStateChanged"), _T("org.rdk.WifiManager"), message);
}

TEST_F(WifiInitializedEventTest, onSSIDsChanged)
{
    EXPECT_CALL(services, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.WifiManager.onSSIDsChanged\",\"params\":{}}")));
                return Core::ERROR_NONE;
            }));
    handler.Subscribe(0, _T("onSSIDsChanged"), _T("org.rdk.WifiManager"), message);
    plugin->onSSIDsChanged();
    handler.Unsubscribe(0, _T("onSSIDsChanged"), _T("org.rdk.WifiManager"), message);
}
TEST_F(WifiInitializedEventTest, onWifiSignalThresholdChanged)
{
    EXPECT_CALL(services, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.WifiManager.onWifiSignalThresholdChanged\",\"params\":{\"signalStrength\":\"-27.000000\",\"strength\":\"Excellent\"}}")));
                return Core::ERROR_NONE;
            }));
    handler.Subscribe(0, _T("onWifiSignalThresholdChanged"), _T("org.rdk.WifiManager"), message);
    plugin->onWifiSignalThresholdChanged(-27.000000,"Excellent");
    handler.Unsubscribe(0, _T("onWifiSignalThresholdChanged"), _T("org.rdk.WifiManager"), message);
}

TEST_F(WifiInitializedEventTest, onError)
{
    EXPECT_CALL(services, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.WifiManager.onError\",\"params\":{\"code\":2}}")));
                return Core::ERROR_NONE;
            }));
    WPEFramework::Plugin::ErrorCode intData = WPEFramework::Plugin::ErrorCode::CONNECTION_FAILED;
    handler.Subscribe(0, _T("onError"), _T("org.rdk.WifiManager"), message);
    plugin->onError(intData);
    handler.Unsubscribe(0, _T("onError"), _T("org.rdk.WifiManager"), message);
}


TEST_F(WifiInitializedEventTest, onAvailableSSIDs)
{
    EXPECT_CALL(services, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.WifiManager.onAvailableSSIDs\",\"params\":{\"ssid\":\"123412341234\",\"security\":2,\"signalStrength\":\"-27.000000\",\"frequency\":\"2.442000\"}}")));
                return Core::ERROR_NONE;
            }));
    JsonObject jsonObj;
    jsonObj["ssid"] = "123412341234";
    jsonObj["security"] = 2;
    jsonObj["signalStrength"] = "-27.000000";
    jsonObj["frequency"] = "2.442000";
    handler.Subscribe(0, _T("onAvailableSSIDs"), _T("org.rdk.WifiManager"), message);
    plugin->onAvailableSSIDs(jsonObj);
    handler.Unsubscribe(0, _T("onAvailableSSIDs"), _T("org.rdk.WifiManager"), message);
}

