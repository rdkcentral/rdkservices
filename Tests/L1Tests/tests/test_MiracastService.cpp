#include <gtest/gtest.h>
#include "ServiceMock.h"
#include "MiracastService.h"
#include "WrapsMock.h"
#include "WpaCtrlMock.h"

using ::testing::NiceMock;
using namespace WPEFramework;

namespace {

const char *p2p_events[] = {
       "P2P-DEVICE-FOUND 2c:33:58:9c:73:2d p2p_dev_addr=2c:33:58:9c:73:2d pri_dev_type=1-0050F200-0 name='Sample-Test-Android-1' config_methods=0x11e8 dev_capab=0x25 group_capab=0x82 wfd_dev_info=0x01101c440006 new=0",
       "P2P-DEVICE-FOUND 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0 wfd_dev_info=0x01101c440032 vendor_elems=1 new=1",
       "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0",
       "P2P-GO-NEG-REQUEST 96:52:44:b6:7d:14 dev_passwd_id=4 go_intent=13",
       "P2P-GO-NEG-SUCCESS role=client freq=2437 ht40=0 peer_dev=96:52:44:b6:7d:14 peer_iface=96:52:44:b6:fd:14 wps_method=PBC",
       "P2P-GROUP-FORMATION-SUCCESS",
       "P2P-GROUP-STARTED p2p-p2p0-0 GO ssid=\"DIRECT-sY-Element-Xumo-TV\" freq=5180 passphrase=\"iiR5PNdp\" go_dev_addr=96:52:44:b6:7d:14"
};

int current_p2p_event = 0,
    overall_register_events = static_cast<int>(sizeof(p2p_events) / sizeof(p2p_events[0]));
}

static struct wpa_ctrl global_wpa_ctrl_handle;

class MiracastServiceTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::MiracastService> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;
    ServiceMock service;
    NiceMock<WpaCtrlImplMock> wpactrlImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;

    MiracastServiceTest()
        : plugin(Core::ProxyType<Plugin::MiracastService>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
	    WpaCtrl::getInstance().impl = &wpactrlImplMock;
	    Wraps::getInstance().impl = &wrapsImplMock;

	    std::ofstream deviceProperties("/etc/device.properties");
	    deviceProperties << "WIFI_P2P_CTRL_INTERFACE=p2p0\n";
	    deviceProperties.close();

	    EXPECT_CALL(service, QueryInterfaceByCallsign(::testing::_, ::testing::_))
		    .Times(::testing::AnyNumber())
		    .WillRepeatedly(::testing::Invoke(
					    [&](const uint32_t, const string& name) -> void* {
					    return nullptr;
					    }));
	    ON_CALL(wpactrlImplMock, wpa_ctrl_open(::testing::_))
		    .WillByDefault(::testing::Invoke([&](const char *ctrl_path) { return &global_wpa_ctrl_handle; }));
	    ON_CALL(wpactrlImplMock, wpa_ctrl_close(::testing::_))
		    .WillByDefault(::testing::Invoke([&](struct wpa_ctrl *) { return; }));
	    ON_CALL(wpactrlImplMock, wpa_ctrl_request(::testing::_, ::testing::_, ::testing::_,::testing::_, ::testing::_, ::testing::_))
		    .WillByDefault(::testing::Invoke([&](struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len, char *reply, size_t *reply_len, void(*msg_cb)(char *msg, size_t len)) { return false; }));
	    ON_CALL(wpactrlImplMock, wpa_ctrl_pending(::testing::_))
		    .WillByDefault(::testing::Invoke([&](struct wpa_ctrl *ctrl) { return true; }));
	    ON_CALL(wpactrlImplMock, wpa_ctrl_attach(::testing::_))
		    .WillByDefault(::testing::Invoke([&](struct wpa_ctrl *ctrl) { return false; }));
	    ON_CALL(wpactrlImplMock, wpa_ctrl_recv(::testing::_, ::testing::_, ::testing::_))
		    .WillByDefault(::testing::Invoke(
			[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				if (current_p2p_event >= 0 && current_p2p_event < overall_register_events )
				{
					const char* sourceString = p2p_events[current_p2p_event];
					strncpy(reply, sourceString, *reply_len);
					++current_p2p_event;
				}
				return false;
				}));
	    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
		    .Times(::testing::AnyNumber())
		    .WillRepeatedly(::testing::Invoke(
			[&](const char* command, const char* type)
			{
				char buffer[1024];
				memset(buffer, 0, sizeof(buffer));
				if ( 0 == strncmp(command,"awk '$6 == ",strlen("awk '$6 == ")))
				{
					const char ip_addr_str[] = "192.168.59.165";
					strcpy(buffer, ip_addr_str);
				}
				else if ( 0 == strncmp(command,"/sbin/udhcpc -v -i",strlen("/sbin/udhcpc -v -i")))
				{
					const char response_str[] = "udhcpc: sending select for 192.168.49.165\nudhcpc: lease of 192.168.49.165 obtained, lease time 3599\ndeleting routers\nroute add default gw 192.168.49.1 dev p2p-p2p0-0\nadding dns 192.168.49.1";
					strcpy(buffer, response_str);
				}
				FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
				return pipe;
			}));
	    ON_CALL(wrapsImplMock, system(::testing::_))
		    .WillByDefault(::testing::Invoke(
			[&](const char* command) {
			    return 0;
			}));

	    EXPECT_EQ(string(""), plugin->Initialize(&service));
    }
    virtual ~MiracastServiceTest() override
    {
	    plugin->Deinitialize(nullptr);
	    WpaCtrl::getInstance().impl = nullptr;
    }
};

TEST_F(MiracastServiceTest, RegisteredMethods)
{
   EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEnable")));
   EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getEnable")));
   EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("acceptClientConnection")));
   EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("stopClientConnection")));
   EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("updatePlayerState")));
   EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setLogging")));
   EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setP2PBackendDiscovery")));
}

TEST_F(MiracastServiceTest, setEnable)
{
    WpaCtrl::getInstance().impl = &wpactrlImplMock;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));
    EXPECT_EQ(response, string("{\"message\":\"Successfully enabled the WFD Discovery\",\"success\":true}"));
}

TEST_F(MiracastServiceTest, getEnabledStatus)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnable"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"enabled\":true,\"success\":true}"));
}

TEST_F(MiracastServiceTest, setDisable)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(response, string("{\"message\":\"Successfully disabled the WFD Discovery\",\"success\":true}"));
}

TEST_F(MiracastServiceTest, getDisabledStatus)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": false}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnable"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"enabled\":false,\"success\":true}"));
}

TEST_F(MiracastServiceTest, acceptClientConnection)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("acceptClientConnection"), _T("{\"requestStatus\": Accept}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(MiracastServiceTest, rejectClientConnection)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("acceptClientConnection"), _T("{\"requestStatus\": Reject}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(MiracastServiceTest, PlayerStateAsInitiated)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updatePlayerState"), _T("{\"mac\": \"A1:B2:C3:D4:E5:F6\",\"state\":\"INITIATED\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(MiracastServiceTest, PlayerStateAsInprogress)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updatePlayerState"), _T("{\"mac\": \"A1:B2:C3:D4:E5:F6\",\"state\":\"INPROGRESS\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(MiracastServiceTest, PlayerStateAsPlaying)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updatePlayerState"), _T("{\"mac\": \"A1:B2:C3:D4:E5:F6\",\"state\":\"PLAYING\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(MiracastServiceTest, PlayerStateAsStopped)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updatePlayerState"), _T("{\"mac\": \"A1:B2:C3:D4:E5:F6\",\"state\":\"STOPPED\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
