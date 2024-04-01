#include <gtest/gtest.h>
#include "FactoriesImplementation.h"
#include "ServiceMock.h"
#include "MiracastService.h"
#include "WrapsMock.h"
#include "WpaCtrlMock.h"

using ::testing::NiceMock;
using namespace WPEFramework;
namespace
{
	static void removeFile(const char* fileName)
	{
		if (std::remove(fileName) != 0)
		{
			printf("File %s failed to remove\n", fileName);
			perror("Error deleting file");
		}
		else
		{
			printf("File %s successfully deleted\n", fileName);
		}
	}
	static void removeEntryFromFile(const char* fileName, const char* entryToRemove)
	{
		std::ifstream inputFile(fileName);
		if (!inputFile.is_open())
		{
			printf("Error: Unable to open file: %s\n",fileName);
			return;
		}

		std::vector<std::string> lines;
		std::string line;
		while (std::getline(inputFile, line)) {
			if (line != entryToRemove) {
				lines.push_back(line);
			}
		}
		inputFile.close();

		std::ofstream outputFile(fileName);
		if (!outputFile.is_open())
		{
			printf("Error: Unable to open file: %s for writing\n",fileName);
			return;
		}

		for (const auto& line : lines) {
			outputFile << line << "\n";
		}
		outputFile.close();

		printf("Entry removed from file: %s\n",fileName);
	}
	static void createFile(const char* fileName, const char* fileContent)
	{
		removeFile(fileName);

		std::ofstream fileContentStream(fileName);
		fileContentStream << fileContent;
		fileContentStream << "\n";
		fileContentStream.close();
	}
}

static struct wpa_ctrl global_wpa_ctrl_handle;

class MiracastServiceTest : public ::testing::Test {
	protected:
		Core::ProxyType<Plugin::MiracastService> plugin;
		Core::JSONRPC::Handler& handler;
		Core::JSONRPC::Connection connection;
		string response;
		ServiceMock service;
		WrapsImplMock *p_wrapsImplMock   = nullptr;
		WpaCtrlApiImplMock *p_wpaCtrlImplMock = nullptr;

		MiracastServiceTest()
			: plugin(Core::ProxyType<Plugin::MiracastService>::Create())
			  , handler(*(plugin))
			  , connection(1, 0)
	{
		p_wrapsImplMock  = new NiceMock <WrapsImplMock>;
		Wraps::setImpl(p_wrapsImplMock);

		p_wpaCtrlImplMock  = new NiceMock <WpaCtrlApiImplMock>;
		WpaCtrlApi::setImpl(p_wpaCtrlImplMock);

		EXPECT_CALL(service, QueryInterfaceByCallsign(::testing::_, ::testing::_))
			.Times(::testing::AnyNumber())
			.WillRepeatedly(::testing::Invoke([&](const uint32_t, const string& name) -> void* { return nullptr; }));
		ON_CALL(*p_wpaCtrlImplMock, wpa_ctrl_open(::testing::_))
			.WillByDefault(::testing::Invoke([&](const char *ctrl_path) { return &global_wpa_ctrl_handle; }));
		ON_CALL(*p_wpaCtrlImplMock, wpa_ctrl_close(::testing::_))
			.WillByDefault(::testing::Invoke([&](struct wpa_ctrl *) { return; }));
		ON_CALL(*p_wpaCtrlImplMock, wpa_ctrl_request(::testing::_, ::testing::_, ::testing::_,::testing::_, ::testing::_, ::testing::_))
			.WillByDefault(::testing::Invoke([&](struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len, char *reply, size_t *reply_len, void(*msg_cb)(char *msg, size_t len)) { return false; }));
		ON_CALL(*p_wpaCtrlImplMock, wpa_ctrl_pending(::testing::_))
			.WillByDefault(::testing::Invoke([&](struct wpa_ctrl *ctrl) { return true; }));
		ON_CALL(*p_wpaCtrlImplMock, wpa_ctrl_attach(::testing::_))
			.WillByDefault(::testing::Invoke([&](struct wpa_ctrl *ctrl) { return false; }));
		ON_CALL(*p_wrapsImplMock, system(::testing::_))
			.WillByDefault(::testing::Invoke([&](const char* command) {return 0;}));
	}
		virtual ~MiracastServiceTest() override
		{
			WpaCtrlApi::setImpl(nullptr);
			if (p_wpaCtrlImplMock != nullptr)
			{
				delete p_wpaCtrlImplMock;
				p_wpaCtrlImplMock = nullptr;
			}

			Wraps::setImpl(nullptr);
			if (p_wrapsImplMock != nullptr)
			{
				delete p_wrapsImplMock;
				p_wrapsImplMock = nullptr;
			}
		}
};

class MiracastServiceEventTest : public MiracastServiceTest {
	protected:
		NiceMock<ServiceMock> service;
		Core::JSONRPC::Message message;
		NiceMock<FactoriesImplementation> factoriesImplementation;
		PluginHost::IDispatcher* dispatcher;

		MiracastServiceEventTest()
			: MiracastServiceTest()
		{
			PluginHost::IFactories::Assign(&factoriesImplementation);

			dispatcher = static_cast<PluginHost::IDispatcher*>(
					plugin->QueryInterface(PluginHost::IDispatcher::ID));
			dispatcher->Activate(&service);
		}

		virtual ~MiracastServiceEventTest() override
		{
			dispatcher->Deactivate();
			dispatcher->Release();

			PluginHost::IFactories::Assign(nullptr);
		}
};

TEST_F(MiracastServiceTest, P2PCtrlInterfaceNameNotFound)
{
	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");

	//EXPECT_EQ(string("WIFI_P2P_CTRL_INTERFACE not configured in device properties file"), plugin->Initialize(&service));
	EXPECT_EQ("WIFI_P2P_CTRL_INTERFACE not configured in device properties file", plugin->Initialize(&service));
	plugin->Deinitialize(nullptr);
}

TEST_F(MiracastServiceTest, P2PCtrlInterfacePathNotFound)
{
	removeFile("/var/run/wpa_supplicant/p2p0");
	createFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");

	//EXPECT_EQ(string("Invalid P2P Ctrl iface configured"), plugin->Initialize(&service));
	EXPECT_EQ("Invalid P2P Ctrl iface configured", plugin->Initialize(&service));
	plugin->Deinitialize(nullptr);

	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
}

TEST_F(MiracastServiceTest, RegisteredMethods)
{
	createFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	createFile("/var/run/wpa_supplicant/p2p0","p2p0");

	EXPECT_EQ(string(""), plugin->Initialize(&service));

	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEnable")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getEnable")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("acceptClientConnection")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("stopClientConnection")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("updatePlayerState")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setLogging")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setP2PBackendDiscovery")));

	plugin->Deinitialize(nullptr);

	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	removeFile("/var/run/wpa_supplicant/p2p0");
}

TEST_F(MiracastServiceTest, P2P_DiscoveryStatus)
{
	createFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	createFile("/var/run/wpa_supplicant/p2p0","p2p0");

	EXPECT_EQ(string(""), plugin->Initialize(&service));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));
	EXPECT_EQ(response, string("{\"message\":\"Successfully enabled the WFD Discovery\",\"success\":true}"));

	/* @return      :  {"message":"WFD Discovery already enabled.","success":false}*/
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnable"), _T("{}"), response));
	EXPECT_EQ(response, string("{\"enabled\":true,\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": false}"), response));
	EXPECT_EQ(response, string("{\"message\":\"Successfully disabled the WFD Discovery\",\"success\":true}"));

	/* @return      :  {"message":"WFD Discovery already disabled.","success":false}*/
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": false}"), response));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEnable"), _T("{}"), response));
	EXPECT_EQ(response, string("{\"enabled\":false,\"success\":true}"));

	plugin->Deinitialize(nullptr);

	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	removeFile("/var/run/wpa_supplicant/p2p0");
}

TEST_F(MiracastServiceTest, BackendDiscoveryStatus)
{
	createFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	createFile("/var/run/wpa_supplicant/p2p0","p2p0");

	EXPECT_EQ(string(""), plugin->Initialize(&service));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setP2PBackendDiscovery"), _T("{\"enabled\": true}"), response));

	/* @return      :  {"message":"Invalid parameter passed","success":false}*/
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setP2PBackendDiscovery"), _T("{\"enable\": true}"), response));

	plugin->Deinitialize(nullptr);

	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	removeFile("/var/run/wpa_supplicant/p2p0");
}

TEST_F(MiracastServiceTest, Logging)
{
	createFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	createFile("/var/run/wpa_supplicant/p2p0","p2p0");

	EXPECT_EQ(string(""), plugin->Initialize(&service));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setLogging"), _T("{\"separate_logger\": {\"status\":\"ENABLE\",\"logfilename\": \"GTest\"}}"), response));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setLogging"), _T("{\"level\": \"VERBOSE\"}"), response));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setLogging"), _T("{\"level\": \"WARNING\"}"), response));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setLogging"), _T("{\"level\": \"ERROR\"}"), response));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setLogging"), _T("{\"level\": \"FATAL\"}"), response));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setLogging"), _T("{\"level\": \"TRACE\"}"), response));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setLogging"), _T("{\"level\": \"INFO\"}"), response));
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setLogging"), _T("{\"level\": \"UNKNOWN\"}"), response));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setLogging"), _T("{\"separate_logger\": {\"status\":\"DISABLE\",\"logfilename\": \"GTest\"}}"), response));
	plugin->Deinitialize(nullptr);

	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	removeFile("/var/run/wpa_supplicant/p2p0");
	removeFile("/opt/logs/GTest.log");
}

TEST_F(MiracastServiceEventTest, stopClientConnection)
{
	createFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	createFile("/var/run/wpa_supplicant/p2p0","p2p0");

	EXPECT_EQ(string(""), plugin->Initialize(&service));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));

	EXPECT_CALL(*p_wpaCtrlImplMock, wpa_ctrl_recv(::testing::_, ::testing::_, ::testing::_))
		.WillOnce(::testing::Invoke(
					[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
					strncpy(reply, "P2P-DEVICE-FOUND 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0 wfd_dev_info=0x01101c440032 vendor_elems=1 new=1", *reply_len);
					return false;
					}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-REQUEST 96:52:44:b6:7d:14 dev_passwd_id=4 go_intent=13", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-SUCCESS role=client freq=2437 ht40=0 x=96:52:44:b6:7d:14 peer_iface=96:52:44:b6:fd:14 wps_method=PBC", *reply_len);
				return false;
				}))

	.WillRepeatedly(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				return true;
				}));

	Core::Event connectRequest(false, true);

	EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
		.Times(1)
		.WillOnce(::testing::Invoke(
					[&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
					string text;
					EXPECT_TRUE(json->ToString(text));
					EXPECT_EQ(text,string(_T("{"
									"\"jsonrpc\":\"2.0\","
									"\"method\":\"client.events.onClientConnectionRequest\","
									"\"params\":{\"mac\":\"96:52:44:b6:7d:14\","
									"\"name\":\"Sample-Test-Android-2\""
									"}}"
								)));
					connectRequest.SetEvent();
					return Core::ERROR_NONE;
					}));

	handler.Subscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);

	EXPECT_EQ(Core::ERROR_NONE, connectRequest.Lock(10000));

	/* @return      :  {"message":"Supported 'requestStatus' parameter values are Accept or Reject","success":false}*/
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("acceptClientConnection"), _T("{\"requestStatus\": Timeout}"), response));
	/* @return      :  {"message":"Invalid parameter passed","success":false}*/
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("acceptClientConnection"), _T("{\"request\": Accept}"), response));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("acceptClientConnection"), _T("{\"requestStatus\": Accept}"), response));

	sleep(2);

	/* @return      :  {"message":"Invalid MAC and Name","success":false}*/
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("stopClientConnection"), _T("{\"name\": \"Sample-Test\",\"mac\": \"96:52:44:b6:7d:14\"}"), response));
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("stopClientConnection"), _T("{\"name\": \"Sample-Test-Android-2\",\"mac\": \"96:52:44:b6:7d\"}"), response));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopClientConnection"), _T("{\"name\": \"Sample-Test-Android-2\",\"mac\": \"96:52:44:b6:7d:14\"}"), response));

	handler.Unsubscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);

	plugin->Deinitialize(nullptr);

	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	removeFile("/var/run/wpa_supplicant/p2p0");
}

TEST_F(MiracastServiceEventTest, P2P_GOMode_onClientConnectionAndLaunchRequest)
{
	createFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	createFile("/var/run/wpa_supplicant/p2p0","p2p0");

	EXPECT_EQ(string(""), plugin->Initialize(&service));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));

	EXPECT_CALL(*p_wrapsImplMock, popen(::testing::_, ::testing::_))
		.Times(::testing::AnyNumber())
		.WillRepeatedly(::testing::Invoke(
					[&](const char* command, const char* type)
					{
					char buffer[1024] = {0};
					if ( 0 == strncmp(command,"awk '$6 == ",strlen("awk '$6 == ")))
					{
					strcpy(buffer, "192.168.59.165");
					}
					return (fmemopen(buffer, strlen(buffer), "r"));
					}));

	EXPECT_CALL(*p_wpaCtrlImplMock, wpa_ctrl_recv(::testing::_, ::testing::_, ::testing::_))
		.WillOnce(::testing::Invoke(
					[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
					strncpy(reply, "P2P-DEVICE-FOUND 2c:33:58:9c:73:2d p2p_dev_addr=2c:33:58:9c:73:2d pri_dev_type=1-0050F200-0 name='Sample-Test-Android-1' config_methods=0x11e8 dev_capab=0x25 group_capab=0x82 wfd_dev_info=0x01101c440006 new=0", *reply_len);
					return false;
					}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-DEVICE-LOST 2c:33:58:9c:73:2d p2p_dev_addr=2c:33:58:9c:73:2d pri_dev_type=1-0050F200-0 name='Sample-Test-Android-1' config_methods=0x11e8 dev_capab=0x25 group_capab=0x82 wfd_dev_info=0x01101c440006 new=0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-DEVICE-FOUND 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0 wfd_dev_info=0x01101c440032 vendor_elems=1 new=1", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-REQUEST 96:52:44:b6:7d:14 dev_passwd_id=4 go_intent=13", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-SUCCESS role=client freq=2437 ht40=0 x=96:52:44:b6:7d:14 peer_iface=96:52:44:b6:fd:14 wps_method=PBC", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GROUP-FORMATION-SUCCESS", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-FIND-STOPPED", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				// Here using lo to avoid the operation not permitted error for unknown interfaces
				strncpy(reply, "P2P-GROUP-STARTED lo GO ssid=\"DIRECT-UU-Element-Xumo-TV\" freq=2437 psk=12c3ce3d8976152df796e5f42fc646723471bf1aab8d72a546fa3dce60dc14a3 go_dev_addr=96:52:44:b6:7d:14 ip_addr=192.168.49.200 ip_mask=255.255.255.0 go_ip_addr=192.168.49.1", *reply_len);
				return false;
				}))

	.WillRepeatedly(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				return true;
				}));

	Core::Event connectRequest(false, true);
	Core::Event P2PGrpStart(false, true);

	EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
		.Times(2)
		.WillOnce(::testing::Invoke(
					[&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
					string text;
					EXPECT_TRUE(json->ToString(text));
					EXPECT_EQ(text,string(_T("{"
									"\"jsonrpc\":\"2.0\","
									"\"method\":\"client.events.onClientConnectionRequest\","
									"\"params\":{\"mac\":\"96:52:44:b6:7d:14\","
									"\"name\":\"Sample-Test-Android-2\""
									"}}"
								)));
					connectRequest.SetEvent();
					return Core::ERROR_NONE;
					}))

	.WillOnce(::testing::Invoke(
				[&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
				string text;
				EXPECT_TRUE(json->ToString(text));
				EXPECT_EQ(text,string(_T("{"
								"\"jsonrpc\":\"2.0\","
								"\"method\":\"client.events.onLaunchRequest\","
								"\"params\":{\"device_parameters\":{\"source_dev_ip\":\"192.168.59.165\","
								"\"source_dev_mac\":\"96:52:44:b6:7d:14\","
								"\"source_dev_name\":\"Sample-Test-Android-2\","
								"\"sink_dev_ip\":\"192.168.59.1\""
								"}}}"
							)));
				P2PGrpStart.SetEvent();
				return Core::ERROR_NONE;
				}));


	handler.Subscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);
	handler.Subscribe(0, _T("onLaunchRequest"), _T("client.events"), message);

	EXPECT_EQ(Core::ERROR_NONE, connectRequest.Lock(10000));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("acceptClientConnection"), _T("{\"requestStatus\": Accept}"), response));

	EXPECT_EQ(Core::ERROR_NONE, P2PGrpStart.Lock(10000));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updatePlayerState"), _T("{\"mac\": \"96:52:44:b6:7d:14\",\"state\":\"INITIATED\"}"), response));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updatePlayerState"), _T("{\"mac\": \"96:52:44:b6:7d:14\",\"state\":\"INPROGRESS\"}"), response));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updatePlayerState"), _T("{\"mac\": \"96:52:44:b6:7d:14\",\"state\":\"PLAYING\"}"), response));

	/* @return      :  {"message":"Failed as MiracastPlayer already Launched.","success":false}*/
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": false}"), response));

	/* @return      :  {"message":"stopClientConnection received after Launch","success":false}*/
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("stopClientConnection"), _T("{\"name\": \"Sample-Test-Android-2\",\"mac\": \"96:52:44:b6:7d:14\"}"), response));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updatePlayerState"), _T("{\"mac\": \"96:52:44:b6:7d:14\",\"state\":\"STOPPED\"}"), response));

	handler.Unsubscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);
	handler.Unsubscribe(0, _T("onLaunchRequest"), _T("client.events"), message);

	plugin->Deinitialize(nullptr);

	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	removeFile("/var/run/wpa_supplicant/p2p0");
}

TEST_F(MiracastServiceEventTest, onClientConnectionRequestRejected)
{
	createFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	createFile("/var/run/wpa_supplicant/p2p0","p2p0");

	EXPECT_EQ(string(""), plugin->Initialize(&service));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));

	EXPECT_CALL(*p_wpaCtrlImplMock, wpa_ctrl_recv(::testing::_, ::testing::_, ::testing::_))
		.WillOnce(::testing::Invoke(
					[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
					strncpy(reply, "P2P-DEVICE-FOUND 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0 wfd_dev_info=0x01101c440032 vendor_elems=1 new=1", *reply_len);
					return false;
					}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-REQUEST 96:52:44:b6:7d:14 dev_passwd_id=4 go_intent=13", *reply_len);
				return false;
				}))

	.WillRepeatedly(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				return true;
				}));

	Core::Event connectRequest(false, true);

	EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
		.Times(1)
		.WillOnce(::testing::Invoke(
					[&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
					string text;
					EXPECT_TRUE(json->ToString(text));
					EXPECT_EQ(text,string(_T("{"
									"\"jsonrpc\":\"2.0\","
									"\"method\":\"client.events.onClientConnectionRequest\","
									"\"params\":{\"mac\":\"96:52:44:b6:7d:14\","
									"\"name\":\"Sample-Test-Android-2\""
									"}}"
								)));
					connectRequest.SetEvent();
					return Core::ERROR_NONE;
					}));
	handler.Subscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);

	EXPECT_EQ(Core::ERROR_NONE, connectRequest.Lock(10000));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("acceptClientConnection"), _T("{\"requestStatus\": Reject}"), response));

	handler.Unsubscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);

	plugin->Deinitialize(nullptr);

	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	removeFile("/var/run/wpa_supplicant/p2p0");
}

TEST_F(MiracastServiceEventTest, P2P_GO_NEGOTIATION_FAIL_onClientConnectionError)
{
	createFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	createFile("/var/run/wpa_supplicant/p2p0","p2p0");

	EXPECT_EQ(string(""), plugin->Initialize(&service));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));

	EXPECT_CALL(*p_wpaCtrlImplMock, wpa_ctrl_recv(::testing::_, ::testing::_, ::testing::_))
		.WillOnce(::testing::Invoke(
					[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
					strncpy(reply, "P2P-DEVICE-FOUND 2c:33:58:9c:73:2d p2p_dev_addr=2c:33:58:9c:73:2d pri_dev_type=1-0050F200-0 name='Sample-Test-Android-1' config_methods=0x11e8 dev_capab=0x25 group_capab=0x82 wfd_dev_info=0x01101c440006 new=0", *reply_len);
					return false;
					}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-DEVICE-FOUND 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0 wfd_dev_info=0x01101c440032 vendor_elems=1 new=1", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-REQUEST 96:52:44:b6:7d:14 dev_passwd_id=4 go_intent=13", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-FAILURE 96:52:44:b6:7d:14", *reply_len);
				return false;
				}))
	.WillRepeatedly(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				return true;
				}));

	Core::Event connectRequest(false, true);
	Core::Event P2PGoFail(false, true);

	EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
		.Times(2)
		.WillOnce(::testing::Invoke(
					[&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
					string text;
					EXPECT_TRUE(json->ToString(text));
					EXPECT_EQ(text,string(_T("{"
									"\"jsonrpc\":\"2.0\","
									"\"method\":\"client.events.onClientConnectionRequest\","
									"\"params\":{\"mac\":\"96:52:44:b6:7d:14\","
									"\"name\":\"Sample-Test-Android-2\""
									"}"
									"}")));
					connectRequest.SetEvent();
					return Core::ERROR_NONE;
					}))
	.WillOnce(::testing::Invoke(
				[&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
				string text;
				EXPECT_TRUE(json->ToString(text));
				EXPECT_EQ(text,string(_T("{"
								"\"jsonrpc\":\"2.0\","
								"\"method\":\"client.events.onClientConnectionError\","
								"\"params\":{\"mac\":\"96:52:44:b6:7d:14\","
								"\"name\":\"Sample-Test-Android-2\","
								"\"error_code\":\"102\","
								"\"reason\":\"P2P GROUP NEGOTIATION FAILURE.\""
								"}"
								"}")));
				P2PGoFail.SetEvent();
				return Core::ERROR_NONE;
				}));

	handler.Subscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);
	handler.Subscribe(0, _T("onClientConnectionError"), _T("client.events"), message);

	EXPECT_EQ(Core::ERROR_NONE, connectRequest.Lock(10000));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("acceptClientConnection"), _T("{\"requestStatus\": Accept}"), response));

	EXPECT_EQ(Core::ERROR_NONE, P2PGoFail.Lock(10000));

	handler.Unsubscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);
	handler.Unsubscribe(0, _T("onClientConnectionError"), _T("client.events"), message);

	plugin->Deinitialize(nullptr);

	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	removeFile("/var/run/wpa_supplicant/p2p0");
}

TEST_F(MiracastServiceEventTest, P2P_GO_FORMATION_FAIL_onClientConnectionError)
{
	createFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	createFile("/var/run/wpa_supplicant/p2p0","p2p0");

	EXPECT_EQ(string(""), plugin->Initialize(&service));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));

	EXPECT_CALL(*p_wpaCtrlImplMock, wpa_ctrl_recv(::testing::_, ::testing::_, ::testing::_))
		.WillOnce(::testing::Invoke(
					[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
					strncpy(reply, "P2P-DEVICE-FOUND 2c:33:58:9c:73:2d p2p_dev_addr=2c:33:58:9c:73:2d pri_dev_type=1-0050F200-0 name='Sample-Test-Android-1' config_methods=0x11e8 dev_capab=0x25 group_capab=0x82 wfd_dev_info=0x01101c440006 new=0", *reply_len);
					return false;
					}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-DEVICE-FOUND 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0 wfd_dev_info=0x01101c440032 vendor_elems=1 new=1", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-REQUEST 96:52:44:b6:7d:14 dev_passwd_id=4 go_intent=13", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-SUCCESS role=client freq=2437 ht40=0 x=96:52:44:b6:7d:14 peer_iface=96:52:44:b6:fd:14 wps_method=PBC", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GROUP-FORMATION-FAILURE", *reply_len);
				return false;
				}))

	.WillRepeatedly(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				return true;
				}));

	Core::Event connectRequest(false, true);
	Core::Event P2PGoFail(false, true);

	EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
		.Times(2)
		.WillOnce(::testing::Invoke(
					[&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
					string text;
					EXPECT_TRUE(json->ToString(text));
					EXPECT_EQ(text,string(_T("{"
									"\"jsonrpc\":\"2.0\","
									"\"method\":\"client.events.onClientConnectionRequest\","
									"\"params\":{\"mac\":\"96:52:44:b6:7d:14\","
									"\"name\":\"Sample-Test-Android-2\""
									"}"
									"}")));
					connectRequest.SetEvent();
					return Core::ERROR_NONE;
					}))
	.WillOnce(::testing::Invoke(
				[&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
				string text;
				EXPECT_TRUE(json->ToString(text));
				EXPECT_EQ(text,string(_T("{"
								"\"jsonrpc\":\"2.0\","
								"\"method\":\"client.events.onClientConnectionError\","
								"\"params\":{\"mac\":\"96:52:44:b6:7d:14\","
								"\"name\":\"Sample-Test-Android-2\","
								"\"error_code\":\"103\","
								"\"reason\":\"P2P GROUP FORMATION FAILURE.\""
								"}"
								"}")));
				P2PGoFail.SetEvent();
				return Core::ERROR_NONE;
				}));

	handler.Subscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);
	handler.Subscribe(0, _T("onClientConnectionError"), _T("client.events"), message);

	EXPECT_EQ(Core::ERROR_NONE, connectRequest.Lock(10000));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("acceptClientConnection"), _T("{\"requestStatus\": Accept}"), response));

	EXPECT_EQ(Core::ERROR_NONE, P2PGoFail.Lock(10000));

	handler.Unsubscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);
	handler.Unsubscribe(0, _T("onClientConnectionError"), _T("client.events"), message);

	plugin->Deinitialize(nullptr);

	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	removeFile("/var/run/wpa_supplicant/p2p0");
}

TEST_F(MiracastServiceEventTest, P2P_ClientMode_onClientConnectionAndLaunchRequest)
{
	createFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	createFile("/var/run/wpa_supplicant/p2p0","p2p0");

	EXPECT_EQ(string(""), plugin->Initialize(&service));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));

	EXPECT_CALL(*p_wrapsImplMock, popen(::testing::_, ::testing::_))
		.Times(::testing::AnyNumber())
		.WillRepeatedly(::testing::Invoke(
					[&](const char* command, const char* type)
					{
					char buffer[1024] = {0};
					if ( 0 == strncmp(command,"/sbin/udhcpc -v -i",strlen("/sbin/udhcpc -v -i")))
					{
					strcpy(buffer, "udhcpc: sending select for 192.168.49.165\tudhcpc: lease of 192.168.49.165 obtained, lease time 3599\tdeleting routers\troute add default gw 192.168.49.1 dev lo\tadding dns 192.168.49.1");
					//strcpy(buffer, "192.168.49.1");
					}
					return (fmemopen(buffer, strlen(buffer), "r"));
					}));

	EXPECT_CALL(*p_wpaCtrlImplMock, wpa_ctrl_recv(::testing::_, ::testing::_, ::testing::_))
		.WillOnce(::testing::Invoke(
					[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
					strncpy(reply, "P2P-DEVICE-FOUND 2c:33:58:9c:73:2d p2p_dev_addr=2c:33:58:9c:73:2d pri_dev_type=1-0050F200-0 name='Sample-Test-Android-1' config_methods=0x11e8 dev_capab=0x25 group_capab=0x82 wfd_dev_info=0x01101c440006 new=0", *reply_len);
					return false;
					}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-DEVICE-FOUND 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0 wfd_dev_info=0x01101c440032 vendor_elems=1 new=1", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-DEVICE-LOST 2c:33:58:9c:73:2d p2p_dev_addr=2c:33:58:9c:73:2d pri_dev_type=1-0050F200-0 name='Sample-Test-Android-1' config_methods=0x11e8 dev_capab=0x25 group_capab=0x82 wfd_dev_info=0x01101c440006 new=0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-REQUEST 96:52:44:b6:7d:14 dev_passwd_id=4 go_intent=13", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-SUCCESS role=client freq=2437 ht40=0 x=96:52:44:b6:7d:14 peer_iface=96:52:44:b6:fd:14 wps_method=PBC", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GROUP-FORMATION-SUCCESS", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-FIND-STOPPED", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				// Here using lo to avoid the operation not permitted error for unknown interfaces
				strncpy(reply, "P2P-GROUP-STARTED lo client ssid=\"DIRECT-UU-Galaxy A23 5G\" freq=2437 psk=12c3ce3d8976152df796e5f42fc646723471bf1aab8d72a546fa3dce60dc14a3 go_dev_addr=96:52:44:b6:7d:14 [PERSISTENT]", *reply_len);
				return false;
				}))

	.WillRepeatedly(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				return true;
				}));

	Core::Event connectRequest(false, true);
	Core::Event P2PGrpStart(false, true);

	EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
		.Times(2)
		.WillOnce(::testing::Invoke(
					[&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
					string text;
					EXPECT_TRUE(json->ToString(text));
					EXPECT_EQ(text,string(_T("{"
									"\"jsonrpc\":\"2.0\","
									"\"method\":\"client.events.onClientConnectionRequest\","
									"\"params\":{\"mac\":\"96:52:44:b6:7d:14\","
									"\"name\":\"Sample-Test-Android-2\""
									"}}"
								)));
					connectRequest.SetEvent();
					return Core::ERROR_NONE;
					}))

	.WillOnce(::testing::Invoke(
				[&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
				string text;
				EXPECT_TRUE(json->ToString(text));
				EXPECT_EQ(text,string(_T("{"
                                                                "\"jsonrpc\":\"2.0\","
                                                                "\"method\":\"client.events.onLaunchRequest\","
                                                                "\"params\":{\"device_parameters\":{\"source_dev_ip\":\"192.168.49.1\","
                                                                "\"source_dev_mac\":\"96:52:44:b6:7d:14\","
                                                                "\"source_dev_name\":\"Sample-Test-Android-2\","
                                                                "\"sink_dev_ip\":\"192.168.49.165\""
                                                                "}}}"
                                                        )));
                                P2PGrpStart.SetEvent();
				return Core::ERROR_NONE;
				}));


	handler.Subscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);
	handler.Subscribe(0, _T("onLaunchRequest"), _T("client.events"), message);

	EXPECT_EQ(Core::ERROR_NONE, connectRequest.Lock(10000));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("acceptClientConnection"), _T("{\"requestStatus\": Accept}"), response));

	EXPECT_EQ(Core::ERROR_NONE, P2PGrpStart.Lock(10000));

	handler.Unsubscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);
	handler.Unsubscribe(0, _T("onLaunchRequest"), _T("client.events"), message);

	plugin->Deinitialize(nullptr);

	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	removeFile("/var/run/wpa_supplicant/p2p0");
}

TEST_F(MiracastServiceEventTest, P2P_ClientMode_GENERIC_FAILURE)
{
	createFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	createFile("/var/run/wpa_supplicant/p2p0","p2p0");

	EXPECT_EQ(string(""), plugin->Initialize(&service));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));

	EXPECT_CALL(*p_wrapsImplMock, popen(::testing::_, ::testing::_))
		.Times(::testing::AnyNumber())
		.WillRepeatedly(::testing::Invoke(
					[&](const char* command, const char* type)
					{
					char buffer[1024] = {0};
					if ( 0 == strncmp(command,"/sbin/udhcpc -v -i",strlen("/sbin/udhcpc -v -i")))
					{
					strcpy(buffer, "P2P GENERIC FAILURE");
					}
					return (fmemopen(buffer, strlen(buffer), "r"));
					}));

	EXPECT_CALL(*p_wpaCtrlImplMock, wpa_ctrl_recv(::testing::_, ::testing::_, ::testing::_))
		.WillOnce(::testing::Invoke(
					[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
					strncpy(reply, "P2P-DEVICE-FOUND 2c:33:58:9c:73:2d p2p_dev_addr=2c:33:58:9c:73:2d pri_dev_type=1-0050F200-0 name='Sample-Test-Android-1' config_methods=0x11e8 dev_capab=0x25 group_capab=0x82 wfd_dev_info=0x01101c440006 new=0", *reply_len);
					return false;
					}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-DEVICE-FOUND 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0 wfd_dev_info=0x01101c440032 vendor_elems=1 new=1", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-DEVICE-LOST 2c:33:58:9c:73:2d p2p_dev_addr=2c:33:58:9c:73:2d pri_dev_type=1-0050F200-0 name='Sample-Test-Android-1' config_methods=0x11e8 dev_capab=0x25 group_capab=0x82 wfd_dev_info=0x01101c440006 new=0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-REQUEST 96:52:44:b6:7d:14 dev_passwd_id=4 go_intent=13", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-SUCCESS role=client freq=2437 ht40=0 x=96:52:44:b6:7d:14 peer_iface=96:52:44:b6:fd:14 wps_method=PBC", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GROUP-FORMATION-SUCCESS", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-FIND-STOPPED", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				// Here using lo to avoid the operation not permitted error for unknown interfaces
				strncpy(reply, "P2P-GROUP-STARTED lo client ssid=\"DIRECT-UU-Galaxy A23 5G\" freq=2437 psk=12c3ce3d8976152df796e5f42fc646723471bf1aab8d72a546fa3dce60dc14a3 go_dev_addr=96:52:44:b6:7d:14 [PERSISTENT]", *reply_len);
				return false;
				}))

	.WillRepeatedly(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				return true;
				}));

	Core::Event connectRequest(false, true);
	Core::Event P2PGenericFail(false, true);

	EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
		.Times(2)
		.WillOnce(::testing::Invoke(
					[&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
					string text;
					EXPECT_TRUE(json->ToString(text));
					EXPECT_EQ(text,string(_T("{"
									"\"jsonrpc\":\"2.0\","
									"\"method\":\"client.events.onClientConnectionRequest\","
									"\"params\":{\"mac\":\"96:52:44:b6:7d:14\","
									"\"name\":\"Sample-Test-Android-2\""
									"}}"
								)));
					connectRequest.SetEvent();
					return Core::ERROR_NONE;
					}))

	.WillOnce(::testing::Invoke(
				[&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
				string text;
				EXPECT_TRUE(json->ToString(text));
				EXPECT_EQ(text,string(_T("{"
                                                                "\"jsonrpc\":\"2.0\","
                                                                "\"method\":\"client.events.onClientConnectionError\","
                                                                "\"params\":{\"mac\":\"96:52:44:b6:7d:14\","
                                                                "\"name\":\"Sample-Test-Android-2\","
                                                                "\"error_code\":\"104\","
                                                                "\"reason\":\"P2P GENERIC FAILURE.\""
                                                                "}"
                                                                "}"
							)));
				P2PGenericFail.SetEvent();
				return Core::ERROR_NONE;
				}));


	handler.Subscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);
	handler.Subscribe(0, _T("onClientConnectionError"), _T("client.events"), message);

	EXPECT_EQ(Core::ERROR_NONE, connectRequest.Lock(10000));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("acceptClientConnection"), _T("{\"requestStatus\": Accept}"), response));

	EXPECT_EQ(Core::ERROR_NONE, P2PGenericFail.Lock(10000));

	handler.Unsubscribe(0, _T("onClientConnectionRequest"), _T("client.events"), message);
	handler.Unsubscribe(0, _T("onClientConnectionError"), _T("client.events"), message);

	plugin->Deinitialize(nullptr);

	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	removeFile("/var/run/wpa_supplicant/p2p0");
}

TEST_F(MiracastServiceEventTest, P2P_GOMode_AutoConnect)
{
	createFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	createFile("/var/run/wpa_supplicant/p2p0","p2p0");
	createFile("/opt/miracast_autoconnect","GTest");

	EXPECT_EQ(string(""), plugin->Initialize(&service));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEnable"), _T("{\"enabled\": true}"), response));

	EXPECT_CALL(*p_wrapsImplMock, popen(::testing::_, ::testing::_))
		.Times(::testing::AnyNumber())
		.WillRepeatedly(::testing::Invoke(
					[&](const char* command, const char* type)
					{
					char buffer[1024] = {0};
					if ( 0 == strncmp(command,"awk '$6 == ",strlen("awk '$6 == ")))
					{
					strcpy(buffer, "192.168.59.165");
					}
					return (fmemopen(buffer, strlen(buffer), "r"));
					}));

	EXPECT_CALL(*p_wpaCtrlImplMock, wpa_ctrl_recv(::testing::_, ::testing::_, ::testing::_))
	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-DEVICE-FOUND 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0 wfd_dev_info=0x01101c440032 vendor_elems=1 new=1", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-PROV-DISC-PBC-REQ 96:52:44:b6:7d:14 p2p_dev_addr=96:52:44:b6:7d:14 pri_dev_type=10-0050F204-5 name='Sample-Test-Android-2' config_methods=0x188 dev_capab=0x25 group_capab=0x0", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-REQUEST 96:52:44:b6:7d:14 dev_passwd_id=4 go_intent=13", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GO-NEG-SUCCESS role=client freq=2437 ht40=0 x=96:52:44:b6:7d:14 peer_iface=96:52:44:b6:fd:14 wps_method=PBC", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-GROUP-FORMATION-SUCCESS", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				strncpy(reply, "P2P-FIND-STOPPED", *reply_len);
				return false;
				}))

	.WillOnce(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				// Here using lo to avoid the operation not permitted error for unknown interfaces
				strncpy(reply, "P2P-GROUP-STARTED lo GO ssid=\"DIRECT-UU-Element-Xumo-TV\" freq=2437 psk=12c3ce3d8976152df796e5f42fc646723471bf1aab8d72a546fa3dce60dc14a3 go_dev_addr=96:52:44:b6:7d:14 ip_addr=192.168.49.200 ip_mask=255.255.255.0 go_ip_addr=192.168.49.1", *reply_len);
				return false;
				}))

	.WillRepeatedly(::testing::Invoke(
				[&](struct wpa_ctrl *ctrl, char *reply, size_t *reply_len) {
				return true;
				}));

	sleep(10);

	plugin->Deinitialize(nullptr);

	removeEntryFromFile("/etc/device.properties","WIFI_P2P_CTRL_INTERFACE=p2p0");
	removeFile("/var/run/wpa_supplicant/p2p0");
	removeFile("/opt/miracast_autoconnect");
}

