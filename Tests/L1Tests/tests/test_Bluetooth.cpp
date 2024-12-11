#include <gtest/gtest.h>
#include "Bluetooth.h"
#include "BluetoothMocks.h"
#include "FactoriesImplementation.h"
#include <vector>
#include "IarmBusMock.h"
#include "ServiceMock.h"

// Declare the mock instance globally for C function overrides
MockBluetoothManager* mockBluetoothManagerInstance = nullptr;

using namespace WPEFramework;

using ::testing::NiceMock;

class BluetoothTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::Bluetooth> bluetooth;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;
    static IarmBusImplMock   *p_iarmBusImplMock;

    BluetoothTest()
        : bluetooth(Core::ProxyType<Plugin::Bluetooth>::Create())
        , handler(*bluetooth)
        , connection(1, 0)
    {
    }

    static void SetUpTestCase() {
        std::cout << "Setup once before start test" << std::endl;

	if(mockBluetoothManagerInstance == nullptr) {
            p_iarmBusImplMock  = new NiceMock <IarmBusImplMock>;
            IarmBus::setImpl(p_iarmBusImplMock);
	}

	if(mockBluetoothManagerInstance == nullptr) {
	    mockBluetoothManagerInstance = new MockBluetoothManager();
	}
    }

    static void TearDownTestCase() {
        // Called once after all test cases have run
        std::cout << "Tearing down after all tests are run." << std::endl;
        // Clean up tasks such as releasing resources or resetting state
	if (p_iarmBusImplMock != nullptr) {
	    delete p_iarmBusImplMock;
	    p_iarmBusImplMock = nullptr;
	}

	if(mockBluetoothManagerInstance != nullptr) {
	    delete mockBluetoothManagerInstance;
	    mockBluetoothManagerInstance = nullptr;
	}
    }

    void SetUp() override {
    }

    void TearDown() override {
    }

    virtual ~BluetoothTest() = default;
};

IarmBusImplMock* BluetoothTest::p_iarmBusImplMock = nullptr;

// Test Case: Check registered JSONRPC methods
TEST_F(BluetoothTest, RegisteredMethods) {	
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("startScan")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("stopScan")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getApiVersionNumber")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isDiscoverable")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDiscoveredDevices")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPairedDevices")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getConnectedDevices")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("connect")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("disconnect")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setAudioStream")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("pair")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("unpair")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("enable")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("disable")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setDiscoverable")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getName")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setName")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sendAudioPlaybackCommand")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("respondToEvent")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDeviceInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getAudioInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDeviceVolumeMuteInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setDeviceVolumeMuteInfo")));
}

TEST_F(BluetoothTest, GetApiVersionNumber_Response) {
    // API_VERSION_NUMBER_MAJOR is not defined in header file. So, this test case will be failed if API_VERSION_NUMBER_MAJOR is changed in future.
    const int expectedVersion = 1;

    // Call the method
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getApiVersionNumber"), _T("{}"), response));

    // Verify the response contains the expected version
    EXPECT_EQ(response, "{\"version\":" + std::to_string(expectedVersion) + ",\"success\":true}");
}

// Test Case: StartScanWrapper when adapters are available and scan starts successfully
TEST_F(BluetoothTest, StartScanWrapper_SuccessWithAdapters) {
    // Mock the behavior of the Bluetooth Manager when there is one available adapter
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<0>(1), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Mock the behavior to simulate that the scan starts successfully
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StartDeviceDiscovery(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Invoke the startScan method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"), _T("{\"timeout\":30}"), response));
    EXPECT_EQ(response, "{\"status\":\"AVAILABLE\",\"success\":true}");
}

// Test Case: StartScanWrapper when no adapters are available
TEST_F(BluetoothTest, StartScanWrapper_NoAdapters) {
    // Mock the behavior when there are no adapters available
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<0>(0), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Invoke the startScan method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"), _T("{\"timeout\":30}"), response));
    EXPECT_EQ(response, "{\"status\":\"NO_BLUETOOTH_HARDWARE\",\"success\":true}");
}

// Test Case: StartScanWrapper when getting the number of adapters fails
TEST_F(BluetoothTest, StartScanWrapper_GetAdaptersFailed) {
    // Mock the behavior when fetching the number of adapters fails
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));

    // Invoke the startScan method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"), _T("{\"timeout\":30}"), response));
    EXPECT_EQ(response, "{\"status\":\"NO_BLUETOOTH_HARDWARE\",\"success\":true}");
}

TEST_F(BluetoothTest, StartScanWrapper_InvalidTimeout) {
    // Mock expectations
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<0>(1), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StartDeviceDiscovery(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StopDeviceDiscovery(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Invoke the startScan method with an invalid timeout
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"), _T("{\"timeout\":-1}"), response));

    // Check the response
    EXPECT_EQ(response, "{\"status\":\"AVAILABLE\",\"success\":true}");
}

TEST_F(BluetoothTest, StartScanWrapper_DiscoveryFailed) {
    // Mock the behavior when there is one available adapter
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<0>(1), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Mock the behavior when starting the discovery fails
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StartDeviceDiscovery(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));

    // Expectation based on the current behavior of startScanWrapper
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"), _T("{\"timeout\":30}"), response));

    // Verify the response matches the actual implementation
    EXPECT_EQ(response, "{\"status\":\"AVAILABLE\",\"success\":true}");
}

TEST_F(BluetoothTest, StartScanWrapper_ProfileParsingWithReset) {
    // Mock the behavior when there is one available adapter
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(6)
        .WillRepeatedly(::testing::DoAll(::testing::SetArgPointee<0>(1), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Mock the behavior for starting device discovery
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StartDeviceDiscovery(::testing::Eq(0), ::testing::_))
        .Times(6)
        .WillRepeatedly(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Mock the behavior for stopping device discovery
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StopDeviceDiscovery(::testing::_, ::testing::_))
        .Times(6)
        .WillRepeatedly(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Test AUDIO_AND_HID profile
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"),
        _T("{\"timeout\":-1, \"profile\":\"LOUDSPEAKER, KEYBOARD\"}"), response));
    EXPECT_EQ(response, "{\"status\":\"AVAILABLE\",\"success\":true}");

    // Test AUDIO_OUTPUT profile
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"),
        _T("{\"timeout\":-1, \"profile\":\"HEADPHONES\"}"), response));
    EXPECT_EQ(response, "{\"status\":\"AVAILABLE\",\"success\":true}");

    // Test AUDIO_INPUT profile
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"),
        _T("{\"timeout\":-1, \"profile\":\"SMARTPHONE\"}"), response));
    EXPECT_EQ(response, "{\"status\":\"AVAILABLE\",\"success\":true}");

    // Test HID profile
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"),
        _T("{\"timeout\":-1, \"profile\":\"KEYBOARD\"}"), response));
    EXPECT_EQ(response, "{\"status\":\"AVAILABLE\",\"success\":true}");

    // Test LE profile
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"),
        _T("{\"timeout\":-1, \"profile\":\"LE TILE\"}"), response));
    EXPECT_EQ(response, "{\"status\":\"AVAILABLE\",\"success\":true}");

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"),
        _T("{\"timeout\":-1, \"profile\":\"DEFAULT\"}"), response));
    EXPECT_EQ(response, "{\"status\":\"AVAILABLE\",\"success\":true}");
}

TEST_F(BluetoothTest, StartScanWrapper_DiscoveryInProgress) {
    // Mock the behavior when there is one available adapter
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(1) // Only called during the first invocation
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<0>(1), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Mock the behavior for starting device discovery (only called once initially)
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StartDeviceDiscovery(::testing::Eq(0), ::testing::_))
        .Times(1) // Should only be called once
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // First call: Start discovery successfully
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"),
        _T("{\"timeout\":30, \"profile\":\"LOUDSPEAKER\"}"), response));
    EXPECT_EQ(response, "{\"status\":\"AVAILABLE\",\"success\":true}");

    // Second call: Attempt to start discovery while already running
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"),
        _T("{\"timeout\":30, \"profile\":\"HEADPHONES\"}"), response));
    EXPECT_EQ(response, "{\"status\":\"AVAILABLE\",\"success\":true}");
}

TEST_F(BluetoothTest, StartScanWrapper_MissingParameters) {
    // No mocks are needed since the parameters are missing, and the logic fails early.

    // Invoke the method without specifying required parameters
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("startScan"), _T("{}"), response));

    // Verify that the response indicates failure
    EXPECT_EQ(response.empty(), true);    
}

// Test Case: Adapters Available, Adapter is Discoverable
TEST_F(BluetoothTest, IsDiscoverableWrapper_Success) {
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<0>(1), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_IsAdapterDiscoverable(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<1>(true), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isDiscoverable"), _T("{}"), response));
    EXPECT_EQ(response, "{\"discoverable\":true,\"success\":true}");
}

// Test Case: Adapter Not Discoverable
TEST_F(BluetoothTest, IsDiscoverableWrapper_NotDiscoverable) {
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<0>(1), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_IsAdapterDiscoverable(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<1>(false), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isDiscoverable"), _T("{}"), response));
    EXPECT_EQ(response, "{\"discoverable\":false,\"success\":true}");
}

TEST_F(BluetoothTest, IsDiscoverableWrapper_DiscoverableCheckFails) {
    // Mock the behavior for fetching adapters
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<0>(1), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Mock the behavior when checking discoverable status fails
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_IsAdapterDiscoverable(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));

    // Call the method
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isDiscoverable"), _T("{}"), response));

    // Verify the response matches the actual implementation
    EXPECT_EQ(response, "{\"discoverable\":false,\"success\":true}");
}

TEST_F(BluetoothTest, IsDiscoverableWrapper_NoAdapters) {
    // Mock the behavior when no adapters are available
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<0>(0), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Call the method
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isDiscoverable"), _T("{}"), response));

    // Verify the response matches the actual implementation
    EXPECT_EQ(response, "{\"discoverable\":false,\"success\":true}");
}

TEST_F(BluetoothTest, IsDiscoverableWrapper_GetAdaptersFails) {
    // Mock the behavior when fetching adapters fails
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));

    // Call the method
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isDiscoverable"), _T("{}"), response));

    // Verify the response matches the actual implementation
    EXPECT_EQ(response, "{\"discoverable\":false,\"success\":true}");
}

// Test Case: StopScanWrapper Success - Discovery Is Running
TEST_F(BluetoothTest, StopScanWrapper_Success_DiscoveryRunning) {
    // Simulate discovery running
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<0>(1), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StartDeviceDiscovery(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Start discovery to set m_discoveryRunning to true
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"), _T("{\"timeout\":30}"), response));
    EXPECT_EQ(response, "{\"status\":\"AVAILABLE\",\"success\":true}");

    // Mock successful stop
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StopDeviceDiscovery(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Stop discovery
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopScan"), _T("{}"), response));
    EXPECT_EQ(response, "{\"success\":true}");
}

// Test Case: StopScanWrapper Failure - StopDeviceDiscovery Fails
TEST_F(BluetoothTest, StopScanWrapper_Failure_StopDeviceDiscoveryFails) {
    // Simulate discovery running
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetNumberOfAdapters(::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<0>(1), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StartDeviceDiscovery(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Start discovery to set m_discoveryRunning to true
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startScan"), _T("{\"timeout\":30}"), response));
    EXPECT_EQ(response, "{\"status\":\"AVAILABLE\",\"success\":true}");

    // Mock failure in stopping discovery
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StopDeviceDiscovery(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));

    // Attempt to stop discovery
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopScan"), _T("{}"), response));

    // Verify the response matches the actual implementation
    EXPECT_EQ(response, "{\"success\":true}");
}

TEST_F(BluetoothTest, GetDiscoveredDevicesWrapper_Success_DevicesDiscovered) {
    // Mock discovered devices
    BTRMGR_DiscoveredDevicesList_t mockDevices = {};
    mockDevices.m_numOfDevices = 2;

    // Fill device 1
    mockDevices.m_deviceProperty[0].m_deviceHandle = 1001;
    strcpy(mockDevices.m_deviceProperty[0].m_name, "Device1");
    mockDevices.m_deviceProperty[0].m_deviceType = BTRMGR_DEVICE_TYPE_HID;
    mockDevices.m_deviceProperty[0].m_isConnected = true;
    mockDevices.m_deviceProperty[0].m_isPairedDevice = false;
    mockDevices.m_deviceProperty[0].m_ui32DevClassBtSpec = 259;
    mockDevices.m_deviceProperty[0].m_ui16DevAppearanceBleSpec = 1023;

    // Fill device 2
    mockDevices.m_deviceProperty[1].m_deviceHandle = 1002;
    strcpy(mockDevices.m_deviceProperty[1].m_name, "Device2");
    mockDevices.m_deviceProperty[1].m_deviceType = BTRMGR_DEVICE_TYPE_HEADPHONES;
    mockDevices.m_deviceProperty[1].m_isConnected = false;
    mockDevices.m_deviceProperty[1].m_isPairedDevice = true;
    mockDevices.m_deviceProperty[1].m_ui32DevClassBtSpec = 512;
    mockDevices.m_deviceProperty[1].m_ui16DevAppearanceBleSpec = 2047;

    // Mock BTRMGR_GetDiscoveredDevices
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDiscoveredDevices(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<1>(mockDevices), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Mock BTRMGR_GetDeviceTypeAsString
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(2)
        .WillRepeatedly(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                case BTRMGR_DEVICE_TYPE_HEADPHONES:
                    return "HEADPHONES";
                default:
                    return "UNKNOWN";
            }
        }));

    // Call the method
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDiscoveredDevices"), _T("{}"), response));

    // Verify the response
    EXPECT_EQ(response, R"({"discoveredDevices":[{"deviceID":"1001","name":"Device1","deviceType":"HUMAN INTERFACE DEVICE","connected":true,"paired":false,"rawDeviceType":"259","rawBleDeviceType":"1023"},{"deviceID":"1002","name":"Device2","deviceType":"HEADPHONES","connected":false,"paired":true,"rawDeviceType":"512","rawBleDeviceType":"2047"}],"success":true})");
}

TEST_F(BluetoothTest, GetDiscoveredDevicesWrapper_Success_NoDevicesDiscovered) {
    // Mock no discovered devices
    BTRMGR_DiscoveredDevicesList_t mockDevices = {};
    mockDevices.m_numOfDevices = 0;

    // Mock BTRMGR_GetDiscoveredDevices
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDiscoveredDevices(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<1>(mockDevices), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Call the method
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDiscoveredDevices"), _T("{}"), response));

    // Verify the response
    EXPECT_EQ(response, R"({"discoveredDevices":[],"success":true})");
}

TEST_F(BluetoothTest, GetDiscoveredDevicesWrapper_Failure) {
    // Mock BTRMGR_GetDiscoveredDevices failure
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDiscoveredDevices(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));

    // Call the method
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDiscoveredDevices"), _T("{}"), response));

    // Verify the response
    EXPECT_EQ(response, R"({"discoveredDevices":[],"success":true})");
}

TEST_F(BluetoothTest, GetPairedDevicesWrapper_Success) {
    // Mock paired devices
    BTRMGR_PairedDevicesList_t mockDevices = {};
    mockDevices.m_numOfDevices = 2;

    // Fill device 1
    mockDevices.m_deviceProperty[0].m_deviceHandle = 1001;
    strcpy(mockDevices.m_deviceProperty[0].m_name, "Device1");
    mockDevices.m_deviceProperty[0].m_deviceType = BTRMGR_DEVICE_TYPE_HID;
    mockDevices.m_deviceProperty[0].m_isConnected = true;
    mockDevices.m_deviceProperty[0].m_ui32DevClassBtSpec = 259;
    mockDevices.m_deviceProperty[0].m_ui16DevAppearanceBleSpec = 1023;

    // Fill device 2 (headphones device)
    mockDevices.m_deviceProperty[1].m_deviceHandle = 1002;
    strcpy(mockDevices.m_deviceProperty[1].m_name, "Device2");
    mockDevices.m_deviceProperty[1].m_deviceType = BTRMGR_DEVICE_TYPE_HEADPHONES;  // Updated device type to HEADPHONES
    mockDevices.m_deviceProperty[1].m_isConnected = false;
    mockDevices.m_deviceProperty[1].m_ui32DevClassBtSpec = 512;
    mockDevices.m_deviceProperty[1].m_ui16DevAppearanceBleSpec = 2047;

    // Mock BTRMGR_GetPairedDevices
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetPairedDevices(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<1>(mockDevices), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Mock BTRMGR_GetDeviceTypeAsString for paired devices
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(2)
        .WillRepeatedly(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                case BTRMGR_DEVICE_TYPE_HEADPHONES:
                    return "HEADPHONES";
                default:
                    return "UNKNOWN";
            }
        }));

    // Call the method
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPairedDevices"), _T("{}"), response));

    // Verify the response
    EXPECT_EQ(response, R"({"pairedDevices":[{"deviceID":"1001","name":"Device1","deviceType":"HUMAN INTERFACE DEVICE","connected":true,"rawDeviceType":"259","rawBleDeviceType":"1023"},{"deviceID":"1002","name":"Device2","deviceType":"HEADPHONES","connected":false,"rawDeviceType":"512","rawBleDeviceType":"2047"}],"success":true})");
}

// Test Case: GetPairedDevicesWrapper when no paired devices are available
TEST_F(BluetoothTest, GetPairedDevicesWrapper_NoPairedDevices) {
    // Mock no paired devices
    BTRMGR_PairedDevicesList_t mockDevices = {};
    mockDevices.m_numOfDevices = 0;

    // Mock BTRMGR_GetPairedDevices
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetPairedDevices(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<1>(mockDevices), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Call the method
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPairedDevices"), _T("{}"), response));

    // Verify the response
    EXPECT_EQ(response, R"({"pairedDevices":[],"success":true})");
}

// Test Case: GetPairedDevicesWrapper when getting paired devices fails
TEST_F(BluetoothTest, GetPairedDevicesWrapper_GetDevicesFailed) {
    // Mock the failure of BTRMGR_GetPairedDevices
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetPairedDevices(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));

    // Call the method
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPairedDevices"), _T("{}"), response));

    // Verify the response (the current implementation returns success with empty pairedDevices)
    EXPECT_EQ(response, R"({"pairedDevices":[],"success":true})");
}

// Test Case: getConnectedDevicesWrapper when devices are connected successfully
TEST_F(BluetoothTest, GetConnectedDevicesWrapper_Success) {
    // Mock connected devices list
    BTRMGR_ConnectedDevicesList_t mockDevices = {};
    mockDevices.m_numOfDevices = 2;

    // Fill device 1 (connected)
    mockDevices.m_deviceProperty[0].m_deviceHandle = 1001;
    strcpy(mockDevices.m_deviceProperty[0].m_name, "Device1");
    mockDevices.m_deviceProperty[0].m_deviceType = BTRMGR_DEVICE_TYPE_HID;
    mockDevices.m_deviceProperty[0].m_powerStatus = BTRMGR_DEVICE_POWER_ACTIVE;
    mockDevices.m_deviceProperty[0].m_ui32DevClassBtSpec = 259;
    mockDevices.m_deviceProperty[0].m_ui16DevAppearanceBleSpec = 1023;

    // Fill device 2 (connected)
    mockDevices.m_deviceProperty[1].m_deviceHandle = 1002;
    strcpy(mockDevices.m_deviceProperty[1].m_name, "Device2");
    mockDevices.m_deviceProperty[1].m_deviceType = BTRMGR_DEVICE_TYPE_HEADPHONES;
    mockDevices.m_deviceProperty[1].m_powerStatus = BTRMGR_DEVICE_POWER_ACTIVE;
    mockDevices.m_deviceProperty[1].m_ui32DevClassBtSpec = 512;
    mockDevices.m_deviceProperty[1].m_ui16DevAppearanceBleSpec = 2047;

    // Mock BTRMGR_GetConnectedDevices success
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetConnectedDevices(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<1>(mockDevices), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Mock BTRMGR_GetDeviceTypeAsString
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(2)
        .WillRepeatedly(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                case BTRMGR_DEVICE_TYPE_HEADPHONES:
                    return "HEADPHONES";
                default:
                    return "UNKNOWN";
            }
        }));

    // Call the method and verify response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getConnectedDevices"), _T("{}"), response));
    EXPECT_EQ(response, R"({"connectedDevices":[{"deviceID":"1001","name":"Device1","deviceType":"HUMAN INTERFACE DEVICE","activeState":"1","rawDeviceType":"259","rawBleDeviceType":"1023"},{"deviceID":"1002","name":"Device2","deviceType":"HEADPHONES","activeState":"1","rawDeviceType":"512","rawBleDeviceType":"2047"}],"success":true})");
}

// Test Case: getConnectedDevicesWrapper when no devices are connected
TEST_F(BluetoothTest, GetConnectedDevicesWrapper_NoDevices) {
    // Mock empty connected devices list
    BTRMGR_ConnectedDevicesList_t mockDevices = {};
    mockDevices.m_numOfDevices = 0;

    // Mock BTRMGR_GetConnectedDevices success
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetConnectedDevices(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::DoAll(::testing::SetArgPointee<1>(mockDevices), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Call the method and verify response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getConnectedDevices"), _T("{}"), response));
    EXPECT_EQ(response, R"({"connectedDevices":[],"success":true})");
}

// Test Case: getConnectedDevicesWrapper when getting the connected devices fails
TEST_F(BluetoothTest, GetConnectedDevicesWrapper_Failure) {
    // Mock failure scenario for getting connected devices
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetConnectedDevices(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));  // Simulating failure

    // Call the method and verify response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getConnectedDevices"), _T("{}"), response));
    EXPECT_EQ(response, "{\"connectedDevices\":[],\"success\":true}");  // Adjusted to match actual behavior
}

TEST_F(BluetoothTest, connectWrapper_Connect_AllDevices) {
    // Define a list of device types and their corresponding deviceIDs
    struct Device {
        long long int deviceID;
        string deviceType;
    };

    std::vector<Device> devices = {
        {1001, "LE TILE"},
        {1003, "JOYSTICK"},
        {1004, "TABLET"},
	{1005, "UNKNOWN DEVICE"}
    };

    // Loop through the device types and test connection for each
    for (const auto& device : devices) {
        string enable = "CONNECT";  // Hardcoded in connectWrapper
        long long int deviceID = device.deviceID;
        string deviceType = device.deviceType;

        // Prepare the JSON parameters for the test
        JsonObject params;
        params["deviceID"] = std::to_string(deviceID);  // deviceID as string
        params["enable"] = enable;
        params["deviceType"] = deviceType;

        // Convert JsonObject to string
        string paramsStr;
        params.ToString(paramsStr);

	// Mock the behavior for different device types
        if (deviceType == "LE TILE" || deviceType == "JOYSTICK") {
            // Mock for LE TILE and JOYSTICK device types (Connect)
            EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_ConnectToDevice(::testing::Eq(0), ::testing::Eq(deviceID), ::testing::_))
                .Times(1)
                .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));
        } else if (deviceType == "TABLET") {
            // Mock for TABLET (Audio streaming)
            EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StartAudioStreamingIn(::testing::Eq(0), ::testing::Eq(deviceID), BTRMGR_DEVICE_OP_TYPE_AUDIO_INPUT))
                .Times(1)
                .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));
        } else if(deviceType == "UNKNOWN DEVICE" ) {
	        EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StartAudioStreamingOut(::testing::Eq(0), ::testing::Eq(deviceID), ::testing::_))
		.Times(1)
		.WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));
	}

        // Invoke the connect method and check the response
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("connect"), paramsStr, response));
        EXPECT_EQ(response, "{\"success\":true}");
    }
}

TEST_F(BluetoothTest, connectWrapper_Connect_DefaultDeviceType_Smartphone) {
    // Device without specifying deviceType (defaults to SMARTPHONE)
    long long int deviceID = 1001;  // Example deviceID
    string enable = "CONNECT";  // Hardcoded in connectWrapper
    string deviceType = "SMARTPHONE";  // This will be set by default

    // Prepare the JSON parameters for the test (without deviceType)
    JsonObject params;
    params["deviceID"] = std::to_string(deviceID);  // deviceID as string
    params["enable"] = enable;

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Mock the behavior based on the default deviceType "SMARTPHONE"
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StartAudioStreamingOut(::testing::Eq(0), ::testing::Eq(deviceID), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Invoke the connect method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("connect"), paramsStr, response));
    EXPECT_EQ(response, "{\"success\":true}");
}

TEST_F(BluetoothTest, connectWrapper_Failure) {
    // Device without specifying deviceType (defaults to SMARTPHONE)
    long long int deviceID = 1001;  // Example deviceID
    string enable = "CONNECT";  // Hardcoded in connectWrapper
    string deviceType = "SMARTPHONE";  // This will be set by default

    // Prepare the JSON parameters for the test (without deviceType)
    JsonObject params;
    params["deviceID"] = std::to_string(deviceID);  // deviceID as string
    params["enable"] = enable;

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Mock the behavior based on the default deviceType "SMARTPHONE"
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StartAudioStreamingOut(::testing::Eq(0), ::testing::Eq(deviceID), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));

    // Invoke the connect method and check the response
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("connect"), paramsStr, response));
    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, connectWrapper_MissingParameters) {
    // Prepare the JSON parameters with missing "deviceID"
    JsonObject params;
    // deviceID is omitted to simulate the missing parameter case

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Invoke the connect method with missing parameters
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("connect"), paramsStr, response));

    // Verify that the response is empty
    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, disconnectWrapper_Disconnect_AllDevices) {
    // Define a list of device types and their corresponding deviceIDs
    struct Device {
        long long int deviceID;
        string deviceType;
    };

    std::vector<Device> devices = {
        {1001, "LE TILE"},
        {1003, "JOYSTICK"},
        {1004, "TABLET"},
        {1005, "UNKNOWN DEVICE"}
    };

    // Loop through the device types and test disconnection for each
    for (const auto& device : devices) {
        string enable = "DISCONNECT";  // Hardcoded in disconnectWrapper
        long long int deviceID = device.deviceID;
        string deviceType = device.deviceType;

        // Prepare the JSON parameters for the test
        JsonObject params;
        params["deviceID"] = std::to_string(deviceID);  // deviceID as string
        params["enable"] = enable;
        params["deviceType"] = deviceType;

        // Convert JsonObject to string
        string paramsStr;
        params.ToString(paramsStr);

        // Mock the behavior for different device types
        if (deviceType == "LE TILE" || deviceType == "JOYSTICK") {
            // Mock for LE TILE and JOYSTICK device types (Disconnect)
            EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_DisconnectFromDevice(::testing::Eq(0), ::testing::Eq(deviceID)))
                .Times(1)
                .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));
        } else if (deviceType == "TABLET") {
            // Mock for TABLET (Stop Audio streaming)
            EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StopAudioStreamingIn(::testing::Eq(0), ::testing::Eq(deviceID)))
                .Times(1)
                .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));
        } else if (deviceType == "UNKNOWN DEVICE") {
            // Mock for UNKNOWN DEVICE (Stop Audio streaming)
            EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StopAudioStreamingOut(::testing::Eq(0), ::testing::Eq(deviceID)))
                .Times(1)
                .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));
        }

        // Invoke the disconnect method and check the response
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("disconnect"), paramsStr, response));
        EXPECT_EQ(response, "{\"success\":true}");
    }
}

TEST_F(BluetoothTest, disconnectWrapper_DefaultDeviceType_Smartphone) {
    // Device without specifying deviceType (defaults to SMARTPHONE)
    long long int deviceID = 1004;  // Example deviceID
    string enable = "DISCONNECT";  // Hardcoded in disconnectWrapper
    string deviceType = "SMARTPHONE";  // This will be set by default

    // Prepare the JSON parameters for the test (without deviceType)
    JsonObject params;
    params["deviceID"] = std::to_string(deviceID);  // deviceID as string
    params["enable"] = enable;

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Mock the behavior based on the default deviceType "SMARTPHONE"
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StopAudioStreamingOut(::testing::Eq(0), ::testing::Eq(deviceID)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Invoke the disconnect method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("disconnect"), paramsStr, response));
    EXPECT_EQ(response, "{\"success\":true}");
}

TEST_F(BluetoothTest, disconnectWrapper_MissingParameters) {
    // Prepare the JSON parameters for the test with missing deviceID
    JsonObject params;

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Invoke the disconnect method and check the response
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("disconnect"), paramsStr, response));
    EXPECT_EQ(response.empty(), true);  // Response should be empty since deviceID is missing
}

TEST_F(BluetoothTest, setAudioStreamWrapper_ValidParams) {
    // Define the two possible audio stream names
    string primaryStream = "PRIMARY";
    string auxiliaryStream = "AUXILIARY";

    long long int deviceID = 1001;  // Example deviceID
    string enable = "CONNECT";  // Hardcoded in setAudioStreamWrapper
    unsigned char adapterIdx = 0;  // Example adapter index

    // Prepare the JSON parameters for "PRIMARY"
    JsonObject paramsPrimary;
    paramsPrimary["deviceID"] = std::to_string(deviceID);  // deviceID as string
    paramsPrimary["audioStreamName"] = primaryStream;  // Primary stream

    // Convert JsonObject to string for "PRIMARY"
    string paramsStrPrimary;
    paramsPrimary.ToString(paramsStrPrimary);

    // Mock the behavior for "PRIMARY"
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetAudioStreamingOutType(adapterIdx, BTRMGR_STREAM_PRIMARY))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Invoke the method for "PRIMARY" and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setAudioStream"), paramsStrPrimary, response));
    EXPECT_EQ(response, "{\"success\":true}");

    // Prepare the JSON parameters for "AUXILIARY"
    JsonObject paramsAuxiliary;
    paramsAuxiliary["deviceID"] = std::to_string(deviceID);  // deviceID as string
    paramsAuxiliary["audioStreamName"] = auxiliaryStream;  // Auxiliary stream

    // Convert JsonObject to string for "AUXILIARY"
    string paramsStrAuxiliary;
    paramsAuxiliary.ToString(paramsStrAuxiliary);

    // Mock the behavior for "AUXILIARY"
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetAudioStreamingOutType(adapterIdx, BTRMGR_STREAM_AUXILIARY))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Invoke the method for "AUXILIARY" and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setAudioStream"), paramsStrAuxiliary, response));
    EXPECT_EQ(response, "{\"success\":true}");
}

TEST_F(BluetoothTest, setAudioStreamWrapper_SetAudioStreamingOutType_Failure) {
    long long int deviceID = 1001;  // Example deviceID
    string validStreamName = "PRIMARY";  // Valid stream name for testing
    unsigned char adapterIdx = 0;  // Example adapter index

    // Prepare the JSON parameters for the test with valid audioStreamName
    JsonObject params;
    params["deviceID"] = std::to_string(deviceID);  // deviceID as string
    params["audioStreamName"] = validStreamName;  // Valid stream name

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Mock the behavior for BTRMGR_SetAudioStreamingOutType to return failure
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetAudioStreamingOutType(adapterIdx, BTRMGR_STREAM_PRIMARY))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));  // Simulate failure

    // Invoke the setAudioStream method and check the response
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setAudioStream"), paramsStr, response));
    EXPECT_EQ(response.empty(), true);  // Response should be empty since there was a failure
}

TEST_F(BluetoothTest, setAudioStreamWrapper_MissingParameters) {
    // Test case for missing parameters

    // Prepare the JSON parameters with missing "deviceID"
    JsonObject params;
    // Missing both deviceID and audioStreamName to simulate the missing parameter case

    string paramsStr;
    params.ToString(paramsStr);

    // Invoke the method and check the response
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setAudioStream"), paramsStr, response));

    // Verify that the response is empty
    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, pairWrapper_Success) {
    // Test case for success scenario
    long long int deviceID = 1001;  // Device ID

    // Prepare the JSON parameters for the test
    JsonObject params;
    params["deviceID"] = std::to_string(deviceID);  // deviceID as string

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Mock the behavior of BTRMGR_PairDevice for the deviceID
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_PairDevice(::testing::Eq(0), ::testing::Eq(deviceID)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Invoke the pair method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("pair"), paramsStr, response));
    EXPECT_EQ(response, "{\"success\":true}");
}

TEST_F(BluetoothTest, pairWrapper_MissingParameters) {
    // Prepare the JSON parameters with missing "deviceID"
    JsonObject params;
    // deviceID is omitted to simulate the missing parameter case

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Invoke the pair method with missing parameters
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("pair"), paramsStr, response));

    // Verify that the response is empty
    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, pairWrapper_Failure) {
    // Prepare a device ID for testing failure
    long long int deviceID = 1001; // Example deviceID for failure case

    // Prepare the JSON parameters for the test
    JsonObject params;
    params["deviceID"] = std::to_string(deviceID);  // deviceID as string

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Mock the behavior of BTRMGR_PairDevice to simulate failure
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_PairDevice(::testing::Eq(0), ::testing::Eq(deviceID)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));

    // Invoke the pair method and check the response for failure
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("pair"), paramsStr, response));

    // Verify the failure response
    EXPECT_EQ(response.empty(), true);
}

// Test case for successful unpairing
TEST_F(BluetoothTest, unpairWrapper_Success) {
    long long int deviceID = 1001;

    // Prepare the JSON parameters for the test
    JsonObject params;
    params["deviceID"] = std::to_string(deviceID);  // deviceID as string

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Mock the behavior for unpairing (BTRMGR_UnpairDevice)
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_UnpairDevice(::testing::Eq(0), ::testing::Eq(deviceID)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Invoke the unpair method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("unpair"), paramsStr, response));
    EXPECT_EQ(response, "{\"success\":true}");
}

// Test case for unpairing with failure in BTRMGR_UnpairDevice
TEST_F(BluetoothTest, unpairWrapper_Failure) {
    long long int deviceID = 1001;

    // Prepare the JSON parameters for the test
    JsonObject params;
    params["deviceID"] = std::to_string(deviceID);  // deviceID as string

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Mock the behavior for unpairing (BTRMGR_UnpairDevice) with failure
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_UnpairDevice(::testing::Eq(0), ::testing::Eq(deviceID)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));  // Simulating failure

    // Invoke the unpair method and check the response
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("unpair"), paramsStr, response));  // Expecting failure response
    EXPECT_EQ(response.empty(), true);  // Response should be empty on failure
}

// Test case for missing parameters (deviceID)
TEST_F(BluetoothTest, unpairWrapper_MissingParameters) {
    // Prepare the JSON parameters with missing "deviceID"
    JsonObject params;
    // deviceID is omitted to simulate the missing parameter case

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Invoke the unpair method with missing parameters
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("unpair"), paramsStr, response));

    // Verify that the response is empty
    EXPECT_EQ(response.empty(), true);
}

// Test case for successful enableWrapper invocation
TEST_F(BluetoothTest, enableWrapper_Success) {
    // Mock the behavior for BTRMGR_SetAdapterPowerStatus (success)
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetAdapterPowerStatus(::testing::Eq(0), ::testing::Eq(true)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));  // Success case

    // Invoke the enableWrapper method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enable"), "", response));
    EXPECT_EQ(response, "{\"success\":true}");  // Expected success response
}

// Test case for enableWrapper failure due to BTRMGR_SetAdapterPowerStatus failure
TEST_F(BluetoothTest, enableWrapper_Failure) {
    // Mock the behavior for BTRMGR_SetAdapterPowerStatus (failure)
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetAdapterPowerStatus(::testing::Eq(0), ::testing::Eq(true)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));  // Failure case

    // Invoke the enableWrapper method and check the response
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("enable"), "", response));  // Expect failure response
    EXPECT_EQ(response.empty(), true);  // Response should be empty on failure
}

// Test case for successful disableWrapper invocation
TEST_F(BluetoothTest, disableWrapper_Success) {
    // Mock the behavior for BTRMGR_SetAdapterPowerStatus (success)
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetAdapterPowerStatus(::testing::Eq(0), ::testing::Eq(false)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));  // Success case

    // Invoke the disableWrapper method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("disable"), "", response));
    EXPECT_EQ(response, "{\"success\":true}");  // Expected success response
}

// Test case for disableWrapper failure due to BTRMGR_SetAdapterPowerStatus failure
TEST_F(BluetoothTest, disableWrapper_Failure) {
    // Mock the behavior for BTRMGR_SetAdapterPowerStatus (failure)
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetAdapterPowerStatus(::testing::Eq(0), ::testing::Eq(false)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));  // Failure case

    // Invoke the disableWrapper method and check the response
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("disable"), "", response));  // Expect failure response
    EXPECT_EQ(response.empty(), true);  // Response should be empty on failure
}

TEST_F(BluetoothTest, setDiscoverableWrapper_Success) {
    // Prepare the JSON parameters
    int timeout = 30;  // timeout value
    bool discoverable = true;  // discoverable flag

    JsonObject params;
    params["timeout"] = timeout;  // timeout value
    params["discoverable"] = discoverable;  // discoverable flag

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Mock the behavior of BTRMGR_SetAdapterDiscoverable for success
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetAdapterDiscoverable(::testing::Eq(0), ::testing::Eq(discoverable), ::testing::Eq(timeout)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Invoke the setDiscoverableWrapper method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setDiscoverable"), paramsStr, response));
    EXPECT_EQ(response, "{\"success\":true}");

    // Now test for discoverable = false
    discoverable = false;  // discoverable flag set to false

    // Update JSON parameters for discoverable = false
    params["discoverable"] = discoverable;

    // Convert JsonObject to string again
    params.ToString(paramsStr);

    // Mock the behavior of BTRMGR_SetAdapterDiscoverable for success with discoverable = false
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetAdapterDiscoverable(::testing::Eq(0), ::testing::Eq(discoverable), ::testing::Eq(timeout)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Invoke the setDiscoverableWrapper method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setDiscoverable"), paramsStr, response));
    EXPECT_EQ(response, "{\"success\":true}");
}

TEST_F(BluetoothTest, setDiscoverableWrapper_DefaultTimeout) {
    // Prepare the JSON parameters with default timeout value
    bool discoverable = true;  // discoverable flag

    JsonObject params;
    params["discoverable"] = discoverable;  // discoverable flag

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Mock the behavior of BTRMGR_SetAdapterDiscoverable for success
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetAdapterDiscoverable(::testing::Eq(0), ::testing::Eq(discoverable), ::testing::Eq(-1)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Invoke the setDiscoverableWrapper method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setDiscoverable"), paramsStr, response));
    EXPECT_EQ(response, "{\"success\":true}");
}

TEST_F(BluetoothTest, setDiscoverableWrapper_Failure) {
    // Prepare the JSON parameters for failure scenario
    int timeout = 30;  // timeout value
    bool discoverable = true;  // discoverable flag

    JsonObject params;
    params["timeout"] = timeout;  // timeout value
    params["discoverable"] = discoverable;  // discoverable flag

    // Convert JsonObject to string
    string paramsStr;
    params.ToString(paramsStr);

    // Mock the behavior of BTRMGR_SetAdapterDiscoverable for failure
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetAdapterDiscoverable(::testing::Eq(0), ::testing::Eq(discoverable), ::testing::Eq(timeout)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));

    // Invoke the setDiscoverableWrapper method and check the response for failure
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setDiscoverable"), paramsStr, response));
    EXPECT_EQ(response.empty(), true);  // Response should be empty on failure
}

TEST_F(BluetoothTest, setDiscoverableWrapper_MissingParameters) {
    // Prepare the JSON parameters with missing "discoverable" parameter
    JsonObject params;

    // Convert JsonObject to string (missing "discoverable" field)
    string paramsStr;
    params.ToString(paramsStr);

    // Invoke the setDiscoverable method with missing parameters and check the response
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setDiscoverable"), paramsStr, response));

    // Check that the response is empty as no discoverable parameter is provided
    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, getNameWrapper_Success) {
    // Define the expected adapter name
    const char* expectedAdapterName = "Adapter1";

    // Set the mock behavior for this test case with EXPECT_CALL
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetAdapterName(::testing::Eq(0), ::testing::_))
        .Times(1)  // Ensure the mock is called exactly once
        .WillOnce([&](unsigned char aui8AdapterIdx, char* pNameOfAdapter) {
            // Set the adapter name for this test
            strcpy(pNameOfAdapter, expectedAdapterName);  // This is the name we want to test
            return BTRMGR_RESULT_SUCCESS;
        });

    // Prepare the parameters (no need to pass anything for this test case)
    JsonObject params;
    string paramsStr;
    params.ToString(paramsStr);

    // Invoke the getName method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getName"), paramsStr, response));

    // Check that the response contains the expected adapter name
    EXPECT_EQ(response, "{\"name\":\"Adapter1\",\"success\":true}");
}

TEST_F(BluetoothTest, getNameWrapper_Failure) {
    // Set the mock behavior to simulate BTRMGR_GetAdapterName failure
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetAdapterName(::testing::Eq(0), ::testing::_))
        .Times(1)  // Ensure the mock is called exactly once
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));  // Simulate failure

    // Prepare the parameters (no need to pass anything for this test case)
    JsonObject params;
    string paramsStr;
    params.ToString(paramsStr);

    // Invoke the getName method and check the response
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getName"), paramsStr, response));

    // Check that the response is empty since the method failed
    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, setNameWrapper_Success) {
    // Define the expected adapter name
    const char* expectedAdapterName = "NewAdapterName";

    // Set the mock behavior for BTRMGR_SetAdapterName
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetAdapterName(::testing::Eq(0), ::testing::_))
        .Times(1)  // Ensure the mock is called exactly once
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));  // Simulate success

    // Prepare the JSON parameters with the "name" field
    JsonObject params;
    params["name"] = expectedAdapterName;

    string paramsStr;
    params.ToString(paramsStr);  // Convert to string for the handler

    // Invoke the setName method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setName"), paramsStr, response));

    // Check that the response contains the expected success status
    EXPECT_EQ(response, "{\"success\":true}");
}

TEST_F(BluetoothTest, setNameWrapper_Failure) {
    // Set up the mock behavior for BTRMGR_SetAdapterName to return failure
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetAdapterName(::testing::Eq(0), ::testing::_))
        .Times(1)  // Ensure the mock is called exactly once
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));  // Simulate failure

    // Prepare the JSON parameters with the "name" field
    JsonObject params;
    params["name"] = "NewAdapterName";

    string paramsStr;
    params.ToString(paramsStr);

    // Invoke the setName method and check the response
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setName"), paramsStr, response));

    //EXPECT_EQ(response.empty(), true);
    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, setNameWrapper_MissingParameters) {
    // Prepare the JSON parameters with the "name" field missing
    JsonObject params;
    // "name" field is omitted to simulate missing parameter

    string paramsStr;
    params.ToString(paramsStr);  // Convert to string for the handler

    // Invoke the setName method with missing parameters
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setName"), paramsStr, response));

    // Check that the response is empty because of missing parameters
    EXPECT_EQ(response.empty(), false);
}

TEST_F(BluetoothTest, sendAudioPlaybackCommandWrapper_AllCommands) {
    // Define the device ID
    long long int deviceID = 1001;

    std::vector<string> commands = {
        {"PAUSE"},
        {"RESUME"},
        {"STOP"},
        {"SKIP_NEXT"},
        {"SKIP_PREV"},
        {"AUDIO_MUTE"},
        {"AUDIO_UNMUTE"},
        {"VOLUME_UP"},
        {"VOLUME_DOWN"}
    };

    std::map<std::string, BTRMGR_MediaControlCommand_t> commandMap = {
        {"PAUSE", BTRMGR_MEDIA_CTRL_PAUSE},
        {"RESUME", BTRMGR_MEDIA_CTRL_PLAY},  // Maps RESUME to PLAY
        {"STOP", BTRMGR_MEDIA_CTRL_STOP},
        {"SKIP_NEXT", BTRMGR_MEDIA_CTRL_NEXT},
        {"SKIP_PREV", BTRMGR_MEDIA_CTRL_PREVIOUS},
        {"AUDIO_MUTE", BTRMGR_MEDIA_CTRL_MUTE},
        {"AUDIO_UNMUTE", BTRMGR_MEDIA_CTRL_UNMUTE},
        {"VOLUME_UP", BTRMGR_MEDIA_CTRL_VOLUMEUP},
        {"VOLUME_DOWN", BTRMGR_MEDIA_CTRL_VOLUMEDOWN}
    };

    // Loop through the commands and test each one
    for (const auto& command : commands) {
        // Prepare the JSON parameters
        JsonObject params;
        params["deviceID"] = std::to_string(deviceID);
        params["command"] = command;

        // Convert JsonObject to string
        string paramsStr;
        params.ToString(paramsStr);

	BTRMGR_MediaControlCommand_t mediaCtrlCmd = commandMap[command];

        // Mock the call to BTRMGR_MediaControl with the corresponding command
        EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_MediaControl(::testing::Eq(0), ::testing::Eq(deviceID), ::testing::Eq(mediaCtrlCmd)))
            .Times(1)
            .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

        // Invoke the method and check the response
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendAudioPlaybackCommand"), paramsStr, response));
        EXPECT_EQ(response, "{\"success\":true}");  // Expect success response
    }
}

TEST_F(BluetoothTest, sendAudioPlaybackCommandWrapper_PLAY) {
    // Define the device ID and command
    long long int deviceID = 1001;
    std::string command = "PLAY";  // The command to be tested

    // Prepare the JSON parameters for the PLAY command
    JsonObject params;
    params["deviceID"] = std::to_string(deviceID);
    params["command"] = command;

    // Convert JsonObject to string
    std::string paramsStr;
    params.ToString(paramsStr);

    // Expect the call to BTRMGR_StartAudioStreamingIn for the PLAY command
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StartAudioStreamingIn(::testing::Eq(0), ::testing::Eq(deviceID), ::testing::Eq(BTRMGR_DEVICE_OP_TYPE_AUDIO_INPUT)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Invoke the method and check the response
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendAudioPlaybackCommand"), paramsStr, response));
    EXPECT_EQ(response, "{\"success\":true}");  // Expect success response
}

TEST_F(BluetoothTest, sendAudioPlaybackCommandWrapper_RESTART_Failure) {
    // Define the device ID and command
    long long int deviceID = 1001;
    std::string command = "RESTART";  // The command to be tested

    // Prepare the JSON parameters for the RESTART command
    JsonObject params;
    params["deviceID"] = std::to_string(deviceID);
    params["command"] = command;

    // Convert JsonObject to string
    std::string paramsStr;
    params.ToString(paramsStr);

    // Invoke the method and check the response
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("sendAudioPlaybackCommand"), paramsStr, response));  // Expect error response for failure
    EXPECT_EQ(response.empty(), true);  // Expect the response to be empty since the command failed
}

TEST_F(BluetoothTest, sendAudioPlaybackCommandWrapper_MediaControl_Failure) {
    // Define the device ID and command
    long long int deviceID = 1001;
    std::string command = "PAUSE";  // The command to be tested (can be any supported command like PAUSE)

    // Prepare the JSON parameters for the PAUSE command
    JsonObject params;
    params["deviceID"] = std::to_string(deviceID);
    params["command"] = command;

    // Convert JsonObject to string
    std::string paramsStr;
    params.ToString(paramsStr);

    // Expect the call to BTRMGR_MediaControl and simulate a failure for the command
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_MediaControl(::testing::Eq(0), ::testing::Eq(deviceID), ::testing::Eq(BTRMGR_MEDIA_CTRL_PAUSE)))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));  // Simulate failure

    // Invoke the method and check the response
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("sendAudioPlaybackCommand"), paramsStr, response));  // Expect error response for failure
    EXPECT_EQ(response.empty(), true);  // Expect the response to be empty since the command failed
}

TEST_F(BluetoothTest, sendAudioPlaybackCommandWrapper_MissingBothParameters) {
    // Prepare the JSON parameters with both "deviceID" and "command" missing
    JsonObject params;
    // No parameters at all

    // Convert JsonObject to string
    std::string paramsStr;
    params.ToString(paramsStr);

    // Invoke the method with missing parameters and check the response
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("sendAudioPlaybackCommand"), paramsStr, response));
    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, setEventResponseWrapper_Success) {
    // Define the structure to hold event type and response value
    struct EventScenario {
        std::string eventType;
        std::string responseValue;
    };

    // Prepare the test data for a successful scenario
    JsonObject params;
    params["deviceID"] = "1001";  // Example device ID
    string paramsStr;

    // List of event type and response value pairs
    std::vector<EventScenario> scenarios = {
        {"onPairingRequest", "ACCEPTED"},
        {"onPairingRequest", "REJECTED"},
        {"onConnectionRequest", "ACCEPTED"},
        {"onConnectionRequest", "REJECTED"},
        {"onPlaybackRequest", "ACCEPTED"},
        {"onPlaybackRequest", "REJECTED"}
    };

    // Iterate through each scenario
    for (const auto& scenario : scenarios) {
        // Update the parameters for the current scenario
        params["eventType"] = scenario.eventType;
        params["responseValue"] = scenario.responseValue;
        params.ToString(paramsStr);

        // Define expected behavior of BTRMGR_SetEventResponse
        EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetEventResponse(::testing::Eq(0), ::testing::_))
            .Times(1)
            .WillOnce([&](unsigned char deviceID, BTRMGR_EventResponse_t* eventResp) {
                // Check the values passed to the mock function
                if (scenario.eventType == "onPairingRequest") {
                    EXPECT_EQ(eventResp->m_eventType, BTRMGR_EVENT_RECEIVED_EXTERNAL_PAIR_REQUEST);
                } else if (scenario.eventType == "onConnectionRequest") {
                    EXPECT_EQ(eventResp->m_eventType, BTRMGR_EVENT_RECEIVED_EXTERNAL_CONNECT_REQUEST);
                } else if (scenario.eventType == "onPlaybackRequest") {
                    EXPECT_EQ(eventResp->m_eventType, BTRMGR_EVENT_RECEIVED_EXTERNAL_PLAYBACK_REQUEST);
                }

                // Set the expected response value based on ACCEPTED/REJECTED
                eventResp->m_eventResp = (scenario.responseValue == "ACCEPTED") ? 1 : 0;

                return BTRMGR_RESULT_SUCCESS;  // Mock successful return
            });

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("respondToEvent"), paramsStr, response));

        // Check that the response is as expected
        EXPECT_EQ(response, "{\"success\":true}");
    }
}

TEST_F(BluetoothTest, setEventResponseWrapper_UnknownEvent) {
    // Prepare the test data for an "Unknown" event type
    JsonObject params;
    params["deviceID"] = "1001";  // Example device ID
    params["eventType"] = "unknown";  // Unknown event type
    params["responseValue"] = "ACCEPTED";  // Response value for the event
    string paramsStr;
    params.ToString(paramsStr);

    // Define expected behavior of BTRMGR_SetEventResponse for an unknown event type
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetEventResponse(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce([&](unsigned char deviceID, BTRMGR_EventResponse_t* eventResp) {
            // Verify that eventType is unknown
            EXPECT_EQ(eventResp->m_eventType, BTRMGR_EVENT_MAX); // The expected type for unknown events

            // Set the expected response value to 0 (assuming unknown event type results in no action)
            eventResp->m_eventResp = 0;

            return BTRMGR_RESULT_SUCCESS;  // Mock successful return
        });

    // Invoke the method with the unknown event type
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("respondToEvent"), paramsStr, response));

    // Verify that the response is as expected
    EXPECT_EQ(response, "{\"success\":true}");
}

TEST_F(BluetoothTest, setEventResponseWrapper_SetEventResponseFailure) {
    // Prepare the test data for a failure scenario
    JsonObject params;
    params["deviceID"] = "1001";  // Example device ID
    params["eventType"] = "onPairingRequest";  // Valid event type
    params["responseValue"] = "ACCEPTED";  // Response value for the event
    string paramsStr;
    params.ToString(paramsStr);

    // Define expected behavior of BTRMGR_SetEventResponse for a failure scenario
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetEventResponse(::testing::Eq(0), ::testing::_))
        .Times(1)
        .WillOnce([&](unsigned char deviceID, BTRMGR_EventResponse_t* eventResp) {
            // Simulate failure of the BTRMGR_SetEventResponse function
            return BTRMGR_RESULT_GENERIC_FAILURE;  // Return failure
        });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("respondToEvent"), paramsStr, response));

    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, setEventResponseWrapper_MissingParameters) {
    // Prepare the test data for a missing parameters scenario
    JsonObject params;
    // Missing "eventType" and "responseValue", only deviceID is provided
    params["deviceID"] = "1001";  // Example device ID
    string paramsStr;
    params.ToString(paramsStr);

    // Define the expected behavior when parameters are missing
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetEventResponse(::testing::Eq(0), ::testing::_))
        .Times(0);  // Ensure that BTRMGR_SetEventResponse is not called

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("respondToEvent"), paramsStr, response));

    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, getDeviceInfoWrapper_Success) {
    JsonObject params;
    params["deviceID"] = "1001";  // Example device ID
    string paramsStr;
    params.ToString(paramsStr);

    // Define expected behavior of BTRMGR_GetDeviceProperties
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceProperties(::testing::Eq(0), ::testing::_ , ::testing::_))
        .Times(1)
        .WillOnce([&](unsigned char aui8AdapterIdx, BTRMgrDeviceHandle deviceHandle, BTRMGR_DevicesProperty_t* deviceProperty) {
            // Populate deviceProperty structure with mock data
            deviceProperty->m_deviceHandle = 1001;
            deviceProperty->m_deviceType = BTRMGR_DEVICE_TYPE_SMARTPHONE;
            strcpy(deviceProperty->m_name, "Test Device");
            strcpy(deviceProperty->m_deviceAddress, "00:11:22:33:44:55");
            deviceProperty->m_rssi = -40;
            deviceProperty->m_signalLevel = BTRMGR_RSSI_GOOD;
            deviceProperty->m_vendorID = 1234;
            deviceProperty->m_isPaired = 1;
            deviceProperty->m_isConnected = 1;
            deviceProperty->m_isLowEnergyDevice = 0;
            deviceProperty->m_batteryLevel = 80;
            strcpy(deviceProperty->m_modalias, "TestModAlias");
            strcpy(deviceProperty->m_firmwareRevision, "1.0.0");
            deviceProperty->m_serviceInfo.m_numOfService = 2;
            deviceProperty->m_serviceInfo.m_profileInfo[0].m_profile[0] = 'A';
            deviceProperty->m_serviceInfo.m_profileInfo[1].m_profile[0] = 'B';
            return BTRMGR_RESULT_SUCCESS;  // Mock successful return
        });

    // Mock the call to BTRMGR_GetDeviceTypeAsString for valid device types
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_SMARTPHONE:
                    return "SMARTPHONE";
                case BTRMGR_DEVICE_TYPE_HEADPHONES:
                    return "HEADPHONES";
                default:
                    return "UNKNOWN";
            }
        }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), paramsStr, response));

    std::string expectedResponse = "{\"deviceInfo\":{\"deviceID\":\"1001\",\"name\":\"Test Device\",\"deviceType\":\"SMARTPHONE\","
                                  "\"manufacturer\":\"1234\",\"MAC\":\"00:11:22:33:44:55\",\"signalStrength\":\"3\","
                                  "\"rssi\":\"-40\",\"batteryLevel\":\"80\",\"modalias\":\"TestModAlias\","
                                  "\"firmwareRevision\":\"1.0.0\",\"supportedProfile\":\"A;B\"},\"success\":true}";

    EXPECT_EQ(response, expectedResponse);
}

TEST_F(BluetoothTest, getDeviceInfoWrapper_Failure) {
    // Prepare the test data for a failure scenario
    JsonObject params;
    params["deviceID"] = "1001";  // Example device ID that will be used in the test
    string paramsStr;
    params.ToString(paramsStr);

    // Mock the failure of BTRMGR_GetDeviceProperties
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceProperties(::testing::Eq(0), ::testing::_ , ::testing::_))
        .Times(1)
        .WillOnce([](unsigned char aui8AdapterIdx, BTRMgrDeviceHandle deviceHandle, BTRMGR_DevicesProperty_t* deviceProperty) {
            // Simulate failure by returning an error code
            return BTRMGR_RESULT_GENERIC_FAILURE;
        });

    // Invoke the method under test with the test parameters
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), paramsStr, response));
    EXPECT_EQ(response, "{\"deviceInfo\":{},\"success\":true}");
}

TEST_F(BluetoothTest, getDeviceInfoWrapper_MissingParameter) {
    // Prepare the test data for a missing parameter scenario
    JsonObject params;
    // Missing "deviceID" parameter in this case.
    // params["deviceID"] = "1001";  // Comment out to simulate missing parameter.

    string paramsStr;
    params.ToString(paramsStr);

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getDeviceInfo"), paramsStr, response));
    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, getMediaTrackInfoWrapper_Success) {
    // Prepare the test data for a successful scenario
    JsonObject params;
    params["deviceID"] = "1001";  // Example device ID (BTRMgrDeviceHandle)
    string paramsStr;
    params.ToString(paramsStr);

    // Mock the behavior of BTRMGR_GetMediaTrackInfo
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetMediaTrackInfo(::testing::Eq(0), ::testing::Eq(1001), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](unsigned char aui8AdapterIdx, BTRMgrDeviceHandle deviceID, BTRMGR_MediaTrackInfo_t* trackInfo) {
            // Populate the track info with test data
            strcpy(trackInfo->pcAlbum, "Test Album");
            strcpy(trackInfo->pcGenre, "Rock");
            strcpy(trackInfo->pcTitle, "Test Track");
            strcpy(trackInfo->pcArtist, "Test Artist");
            trackInfo->ui32TrackNumber = 1;
            trackInfo->ui32Duration = 300;  // Duration in seconds
            trackInfo->ui32NumberOfTracks = 10;
            return BTRMGR_RESULT_SUCCESS;  // Return success
        }));

    // Invoke the method under test (getMediaTrackInfoWrapper)
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getAudioInfo"), paramsStr, response));
    EXPECT_EQ(response, "{\"trackInfo\":{\"album\":\"Test Album\",\"genre\":\"Rock\",\"title\":\"Test Track\",\"artist\":\"Test Artist\",\"ui32Duration\":\"300\",\"ui32TrackNumber\":\"1\",\"ui32NumberOfTracks\":\"10\"},\"success\":true}");
}

TEST_F(BluetoothTest, getMediaTrackInfoWrapper_Failure) {
    // Prepare the test data for the failure scenario
    JsonObject params;
    params["deviceID"] = "1001";  // Example device ID
    string paramsStr;
    params.ToString(paramsStr);

    // Define expected behavior of BTRMGR_GetMediaTrackInfo to simulate failure
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetMediaTrackInfo(::testing::Eq(0), ::testing::Eq(1001), ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE)); // Simulate failure in the mock

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getAudioInfo"), paramsStr, response));
    EXPECT_EQ(response, "{\"trackInfo\":{},\"success\":true}");
}

TEST_F(BluetoothTest, getMediaTrackInfoWrapper_MissingParameters) {
    // Prepare the test data for the missing parameter scenario
    JsonObject params;  // Device ID is intentionally missing in this case
    string paramsStr;
    params.ToString(paramsStr);

    // Define expected behavior in case the parameters are missing
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetMediaTrackInfo(::testing::_, ::testing::_, ::testing::_))
        .Times(0);  // The mock should not be called if parameters are missing

    // Invoke the method with missing parameters (deviceID)
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getAudioInfo"), paramsStr, response));

    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, getDeviceVolumeMuteInfoWrapper_Success) {
    // List of device types to test
    std::vector<std::string> deviceTypes = {
        "LOUDSPEAKER", "HIFI AUDIO DEVICE", "TABLET", "JOYSTICK", "LE", "DEFAULT"
    };

    // Prepare the test data for a successful scenario, including deviceType
    JsonObject params;
    params["deviceID"] = "1001";  // Example device ID
    string paramsStr;

    // Iterate through each device type for testing
    for (const auto& deviceType : deviceTypes) {
        params["deviceType"] = deviceType;  // Set the device type
        params.ToString(paramsStr);  // Convert params to string

        // Mock expected behavior of BTRMGR_GetDeviceVolumeMute to return successful data
        EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceVolumeMute(::testing::Eq(0), ::testing::Eq(1001), 
				::testing::_, ::testing::_, ::testing::_))
            .Times(1)
            .WillOnce(::testing::Invoke([](unsigned char adapterIdx, BTRMgrDeviceHandle deviceHandle, 
					    BTRMGR_DeviceOperationType_t deviceOpType, unsigned char* pui8Volume, unsigned char* pui8Mute) {
                // Simulate successful return of volume and mute status
                *pui8Volume = 75;  // Set volume level
                *pui8Mute = 0;  // Set mute status to false (not muted)
                return BTRMGR_RESULT_SUCCESS;  // Return success
            }));

        // Invoke the method
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceVolumeMuteInfo"), paramsStr, response));

        // Verify the response to ensure it contains the expected values
        EXPECT_EQ(response, "{\"volumeinfo\":{\"volume\":\"75\",\"mute\":false},\"success\":true}");
    }
}

TEST_F(BluetoothTest, getDeviceVolumeMuteInfoWrapper_Failure) {
    // Prepare the test data for the failure scenario
    JsonObject params;
    params["deviceID"] = "1001";  // Example device ID
    params["deviceType"] = "SMARTPHONE";  // Device type for the test
    string paramsStr;
    params.ToString(paramsStr);

    // Define expected behavior of BTRMGR_GetDeviceVolumeMute to simulate failure
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceVolumeMute(::testing::Eq(0), ::testing::Eq(1001), ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE));  // Simulate failure in the mock

    // Combine method invocation and validation in one line
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceVolumeMuteInfo"), paramsStr, response));
}

TEST_F(BluetoothTest, getDeviceVolumeMuteInfoWrapper_MissingParameters) {
    // Prepare the test data for the failure scenario with missing parameters
    JsonObject params;
    // We are intentionally omitting the deviceID and deviceType to simulate the missing parameters scenario
    string paramsStr;
    params.ToString(paramsStr);

    // Invoke the method under test with missing parameters
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getDeviceVolumeMuteInfo"), paramsStr, response));
    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, setDeviceVolumeMuteProperties_Success) {
    // Prepare the test data for a successful scenario
    JsonObject params;
    params["deviceID"] = "1001";  // Example device ID
    params["deviceType"] = "SMARTPHONE";  // Example device type
    params["volume"] = "75";  // Example volume level
    params["mute"] = "false";  // Example mute status (not muted)

    string paramsStr;
    params.ToString(paramsStr);

    // Mock the BTRMGR_SetDeviceVolumeMute function to simulate success
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetDeviceVolumeMute(
            ::testing::_,                      // Allow any adapter index
            ::testing::_,                      // Allow any device ID
            ::testing::_,                      // Allow any device operation type
            ::testing::_,                      // Allow any volume value
            ::testing::_                       // Allow any mute value
        ))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_SUCCESS));  // Simulate success

    // Invoke the method under test
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setDeviceVolumeMuteInfo"), paramsStr, response));

    // Verify the response is as expected for success
    EXPECT_EQ(response, "{\"success\":true}");
}

TEST_F(BluetoothTest, setDeviceVolumeMuteProperties_Failure) {
    // Prepare the test data for a failure scenario
    JsonObject params;
    params["deviceID"] = "1001";  // Example device ID
    params["deviceType"] = "SMARTPHONE";  // Example device type
    params["volume"] = "75";  // Example volume level
    params["mute"] = "false";  // Example mute status (not muted)

    string paramsStr;
    params.ToString(paramsStr);

    // Mock the BTRMGR_SetDeviceVolumeMute function to simulate failure
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_SetDeviceVolumeMute(
            ::testing::_,                      // Allow any adapter index
            ::testing::_,                      // Allow any device ID
            ::testing::_,                      // Allow any device operation type
            ::testing::_,                      // Allow any volume value
            ::testing::_                       // Allow any mute value
        ))
        .Times(1)
        .WillOnce(::testing::Return(BTRMGR_RESULT_GENERIC_FAILURE)); // Simulate failure

    // Invoke the method under test
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setDeviceVolumeMuteInfo"), paramsStr, response));
    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, setDeviceVolumeMuteProperties_MissingParameters) {
    // Prepare the test data for a failure scenario with missing parameters
    JsonObject params;
    params["deviceID"] = "1001";  // Example device ID
    // Missing deviceType and volume
    string paramsStr;
    params.ToString(paramsStr);

    // Invoke the method under test with missing parameters
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setDeviceVolumeMuteInfo"), paramsStr, response));
    EXPECT_EQ(response.empty(), true);
}

TEST_F(BluetoothTest, EventCallbackTest) {
    ASSERT_NE(mockBluetoothManagerInstance->evBluetoothHandler, nullptr);

    {
	BTRMGR_EventMessage_t eventMsg;
        eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_DISCOVERY_COMPLETE;
        // Perform assertions on the result or any other necessary verifications
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS,mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    // Test for BTRMGR_EVENT_DEVICE_PAIRING_COMPLETE
    {
        BTRMGR_EventMessage_t eventMsg;
        eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_PAIRING_COMPLETE;
        strncpy(eventMsg.m_discoveredDevice.m_name, "Device Name", sizeof(eventMsg.m_discoveredDevice.m_name) - 1);
        eventMsg.m_discoveredDevice.m_name[sizeof(eventMsg.m_discoveredDevice.m_name) - 1] = '\0';
        eventMsg.m_discoveredDevice.m_deviceHandle = 123;
        eventMsg.m_discoveredDevice.m_deviceType = BTRMGR_DEVICE_TYPE_HID;
        eventMsg.m_discoveredDevice.m_ui32DevClassBtSpec = 456;
        eventMsg.m_discoveredDevice.m_ui16DevAppearanceBleSpec = 789;
        eventMsg.m_discoveredDevice.m_isLastConnectedDevice = true;
        eventMsg.m_discoveredDevice.m_isPairedDevice = true;
        eventMsg.m_discoveredDevice.m_isConnected = false;

	EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                default:
                    return "UNKNOWN";
            }
        }));

	// Perform assertions on the result or any other necessary verifications
	ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    // Test for BTRMGR_EVENT_DEVICE_UNPAIRING_COMPLETE
    {
        BTRMGR_EventMessage_t eventMsg;
        eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_UNPAIRING_COMPLETE;
        strncpy(eventMsg.m_discoveredDevice.m_name, "Device Name", sizeof(eventMsg.m_discoveredDevice.m_name) - 1);
        eventMsg.m_discoveredDevice.m_name[sizeof(eventMsg.m_discoveredDevice.m_name) - 1] = '\0';
        eventMsg.m_discoveredDevice.m_deviceHandle = 123;
        eventMsg.m_discoveredDevice.m_deviceType = BTRMGR_DEVICE_TYPE_HID;
        eventMsg.m_discoveredDevice.m_ui32DevClassBtSpec = 456;
        eventMsg.m_discoveredDevice.m_ui16DevAppearanceBleSpec = 789;
        eventMsg.m_discoveredDevice.m_isLastConnectedDevice = true;
        eventMsg.m_discoveredDevice.m_isPairedDevice = true;
        eventMsg.m_discoveredDevice.m_isConnected = false;

        EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                default:
                    return "UNKNOWN";
            }
        }));

        // Perform assertions on the result or any other necessary verifications
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    // Test for BTRMGR_EVENT_DEVICE_DISCONNECT_COMPLETE
    {
        BTRMGR_EventMessage_t eventMsg;
        eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_DISCONNECT_COMPLETE;
        strncpy(eventMsg.m_discoveredDevice.m_name, "Device Name", sizeof(eventMsg.m_discoveredDevice.m_name) - 1);
        eventMsg.m_discoveredDevice.m_name[sizeof(eventMsg.m_discoveredDevice.m_name) - 1] = '\0';
        eventMsg.m_discoveredDevice.m_deviceHandle = 123;
        eventMsg.m_discoveredDevice.m_deviceType = BTRMGR_DEVICE_TYPE_HID;
        eventMsg.m_discoveredDevice.m_ui32DevClassBtSpec = 456;
        eventMsg.m_discoveredDevice.m_ui16DevAppearanceBleSpec = 789;
        eventMsg.m_discoveredDevice.m_isLastConnectedDevice = true;
        eventMsg.m_discoveredDevice.m_isPairedDevice = true;
        eventMsg.m_discoveredDevice.m_isConnected = false;

        EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                default:
                    return "UNKNOWN";
            }
        }));

        // Perform assertions on the result or any other necessary verifications
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    // Test for BTRMGR_EVENT_DEVICE_DISCOVERY_STARTED
    {
        BTRMGR_EventMessage_t eventMsg;
        eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_DISCOVERY_STARTED;
        // Perform assertions on the result or any other necessary verifications
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS,mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    // Test for BTRMGR_EVENT_RECEIVED_EXTERNAL_PAIR_REQUEST
    {
	BTRMGR_EventMessage_t eventMsg;
        eventMsg.m_eventType = BTRMGR_EVENT_RECEIVED_EXTERNAL_PAIR_REQUEST;
        strncpy(eventMsg.m_externalDevice.m_name, "Device Name", sizeof(eventMsg.m_externalDevice.m_name) - 1);
        eventMsg.m_externalDevice.m_name[sizeof(eventMsg.m_externalDevice.m_name) - 1] = '\0';
        eventMsg.m_externalDevice.m_deviceHandle = 123;
        eventMsg.m_externalDevice.m_deviceType = BTRMGR_DEVICE_TYPE_HID;
        eventMsg.m_externalDevice.m_vendorID = 456;
        strncpy(eventMsg.m_externalDevice.m_deviceAddress, "00:11:22:33:44:55", sizeof(eventMsg.m_externalDevice.m_deviceAddress) - 1);
        eventMsg.m_externalDevice.m_deviceAddress[sizeof(eventMsg.m_externalDevice.m_deviceAddress) - 1] = '\0';

        eventMsg.m_externalDevice.m_serviceInfo.m_numOfService = 2;
        strncpy(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[0].m_profile, "Profile1", sizeof(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[0].m_profile) - 1);
        eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[0].m_profile[sizeof(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[0].m_profile) - 1] = '\0';
        strncpy(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[1].m_profile, "Profile2", sizeof(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[1].m_profile) - 1);
        eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[1].m_profile[sizeof(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[1].m_profile) - 1] = '\0';

        eventMsg.m_externalDevice.m_externalDevicePIN = 1234;

        EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
            .Times(1)
            .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
                switch (deviceType) {
                    case BTRMGR_DEVICE_TYPE_HID:
                        return "HUMAN INTERFACE DEVICE";
                    default:
                        return "UNKNOWN";
            }
        }));

        // Perform assertions on the result or any other necessary verifications
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
	BTRMGR_EventMessage_t eventMsg;
	eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_PAIRING_FAILED;
	strncpy(eventMsg.m_discoveredDevice.m_name, "Device Name", sizeof(eventMsg.m_discoveredDevice.m_name) - 1);
	eventMsg.m_discoveredDevice.m_name[sizeof(eventMsg.m_discoveredDevice.m_name) - 1] = '\0';
	eventMsg.m_discoveredDevice.m_deviceHandle = 123;
	eventMsg.m_discoveredDevice.m_deviceType = BTRMGR_DEVICE_TYPE_HID;
	eventMsg.m_discoveredDevice.m_ui32DevClassBtSpec = 456;
	eventMsg.m_discoveredDevice.m_ui16DevAppearanceBleSpec = 789;
	eventMsg.m_discoveredDevice.m_isLastConnectedDevice = true;
	eventMsg.m_discoveredDevice.m_isPairedDevice = true;
	eventMsg.m_discoveredDevice.m_isConnected = false;

	EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
	    .Times(1)
            .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
                switch (deviceType) {
                    case BTRMGR_DEVICE_TYPE_HID:
                        return "HUMAN INTERFACE DEVICE";
                    default:
                        return "UNKNOWN";
            }
        }));

	// Perform assertions on the result or any other necessary verifications
	ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
        BTRMGR_EventMessage_t eventMsg;
        eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_UNPAIRING_FAILED;
        strncpy(eventMsg.m_discoveredDevice.m_name, "Device Name", sizeof(eventMsg.m_discoveredDevice.m_name) - 1);
        eventMsg.m_discoveredDevice.m_name[sizeof(eventMsg.m_discoveredDevice.m_name) - 1] = '\0';
        eventMsg.m_discoveredDevice.m_deviceHandle = 123;
        eventMsg.m_discoveredDevice.m_deviceType = BTRMGR_DEVICE_TYPE_HID;
        eventMsg.m_discoveredDevice.m_ui32DevClassBtSpec = 456;
        eventMsg.m_discoveredDevice.m_ui16DevAppearanceBleSpec = 789;
        eventMsg.m_discoveredDevice.m_isLastConnectedDevice = true;
        eventMsg.m_discoveredDevice.m_isPairedDevice = true;
        eventMsg.m_discoveredDevice.m_isConnected = false;

        EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
            .Times(1)
            .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
                switch (deviceType) {
                    case BTRMGR_DEVICE_TYPE_HID:
                        return "HUMAN INTERFACE DEVICE";
                    default:
                        return "UNKNOWN";
            }
        }));

        // Perform assertions on the result or any other necessary verifications
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
        BTRMGR_EventMessage_t eventMsg;
        eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_CONNECTION_FAILED;
        strncpy(eventMsg.m_discoveredDevice.m_name, "Device Name", sizeof(eventMsg.m_discoveredDevice.m_name) - 1);
        eventMsg.m_discoveredDevice.m_name[sizeof(eventMsg.m_discoveredDevice.m_name) - 1] = '\0';
        eventMsg.m_discoveredDevice.m_deviceHandle = 123;
        eventMsg.m_discoveredDevice.m_deviceType = BTRMGR_DEVICE_TYPE_HID;
        eventMsg.m_discoveredDevice.m_ui32DevClassBtSpec = 456;
        eventMsg.m_discoveredDevice.m_ui16DevAppearanceBleSpec = 789;
        eventMsg.m_discoveredDevice.m_isLastConnectedDevice = true;
        eventMsg.m_discoveredDevice.m_isPairedDevice = true;
        eventMsg.m_discoveredDevice.m_isConnected = false;

        EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
            .Times(1)
            .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
                switch (deviceType) {
                    case BTRMGR_DEVICE_TYPE_HID:
                        return "HUMAN INTERFACE DEVICE";
                    default:
                        return "UNKNOWN";
            }
        }));

        // Perform assertions on the result or any other necessary verifications
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
	BTRMGR_EventMessage_t eventMsg;
	eventMsg.m_eventType = BTRMGR_EVENT_RECEIVED_EXTERNAL_PLAYBACK_REQUEST;
	strncpy(eventMsg.m_externalDevice.m_name, "Device Name", sizeof(eventMsg.m_externalDevice.m_name) - 1);
	eventMsg.m_externalDevice.m_name[sizeof(eventMsg.m_externalDevice.m_name) - 1] = '\0';
	eventMsg.m_externalDevice.m_deviceHandle = 123;
	eventMsg.m_externalDevice.m_deviceType = BTRMGR_DEVICE_TYPE_HID;
	eventMsg.m_externalDevice.m_vendorID = 456;
	strncpy(eventMsg.m_externalDevice.m_deviceAddress, "00:11:22:33:44:55", sizeof(eventMsg.m_externalDevice.m_deviceAddress) - 1);
	eventMsg.m_externalDevice.m_deviceAddress[sizeof(eventMsg.m_externalDevice.m_deviceAddress) - 1] = '\0';

	eventMsg.m_externalDevice.m_serviceInfo.m_numOfService = 2;
	strncpy(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[0].m_profile, "Profile1", sizeof(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[0].m_profile) - 1);
	eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[0].m_profile[sizeof(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[0].m_profile) - 1] = '\0';
	strncpy(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[1].m_profile, "Profile2", sizeof(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[1].m_profile) - 1);
	eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[1].m_profile[sizeof(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[1].m_profile) - 1] = '\0';

	EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                default:
                    return "UNKNOWN";
        }
        }));
        // Perform assertions on the result or any other necessary verifications
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
        BTRMGR_EventMessage_t eventMsg;
        eventMsg.m_eventType = BTRMGR_EVENT_RECEIVED_EXTERNAL_CONNECT_REQUEST;
        strncpy(eventMsg.m_externalDevice.m_name, "Device Name", sizeof(eventMsg.m_externalDevice.m_name) - 1);
        eventMsg.m_externalDevice.m_name[sizeof(eventMsg.m_externalDevice.m_name) - 1] = '\0';
        eventMsg.m_externalDevice.m_deviceHandle = 123;
        eventMsg.m_externalDevice.m_deviceType = BTRMGR_DEVICE_TYPE_HID;
        eventMsg.m_externalDevice.m_vendorID = 456;
        strncpy(eventMsg.m_externalDevice.m_deviceAddress, "00:11:22:33:44:55", sizeof(eventMsg.m_externalDevice.m_deviceAddress) - 1);
        eventMsg.m_externalDevice.m_deviceAddress[sizeof(eventMsg.m_externalDevice.m_deviceAddress) - 1] = '\0';

        eventMsg.m_externalDevice.m_serviceInfo.m_numOfService = 2;
        strncpy(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[0].m_profile, "Profile1", sizeof(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[0].m_profile) - 1);
        eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[0].m_profile[sizeof(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[0].m_profile) - 1] = '\0';
        strncpy(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[1].m_profile, "Profile2", sizeof(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[1].m_profile) - 1);
        eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[1].m_profile[sizeof(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[1].m_profile) - 1] = '\0';

        EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                default:
                    return "UNKNOWN";
        }
        }));
        // Perform assertions on the result or any other necessary verifications
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
        BTRMGR_EventMessage_t eventMsg;
        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_TRACK_STARTED;
        eventMsg.m_mediaInfo.m_deviceHandle = 123;
        eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaPosition = 10;
        eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaDuration = 60;

        // Perform assertions on the result or any other necessary verifications
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
	BTRMGR_EventMessage_t eventMsg;
	eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_TRACK_PAUSED;
	eventMsg.m_mediaInfo.m_deviceHandle = 123;
	eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaPosition = 20;
	eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaDuration = 60;
	ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
	BTRMGR_EventMessage_t eventMsg;
	eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_TRACK_STOPPED;
	eventMsg.m_mediaInfo.m_deviceHandle = 123;
	eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaPosition = 30;
	eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaDuration = 60;

	ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
	BTRMGR_EventMessage_t eventMsg;
	eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_PLAYBACK_ENDED;
	eventMsg.m_mediaInfo.m_deviceHandle = 123;

	ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
        BTRMGR_EventMessage_t eventMsg;
        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_TRACK_PLAYING;
        eventMsg.m_mediaInfo.m_deviceHandle = 123;
        eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaPosition = 10;
        eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaDuration = 60;

        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
        BTRMGR_EventMessage_t eventMsg;
        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_TRACK_POSITION;
        eventMsg.m_mediaInfo.m_deviceHandle = 123;
        eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaPosition = 20;
        eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaDuration = 60;

        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
	BTRMGR_EventMessage_t eventMsg;
	eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_TRACK_CHANGED;
	eventMsg.m_mediaInfo.m_deviceHandle = 123;
	strncpy(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcAlbum, "Album", sizeof(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcAlbum) - 1);
	eventMsg.m_mediaInfo.m_mediaTrackInfo.pcAlbum[sizeof(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcAlbum) - 1] = '\0';
	strncpy(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcGenre, "Genre", sizeof(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcGenre) - 1);
	eventMsg.m_mediaInfo.m_mediaTrackInfo.pcGenre[sizeof(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcGenre) - 1] = '\0';
	strncpy(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcTitle, "Title", sizeof(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcTitle) - 1);
	eventMsg.m_mediaInfo.m_mediaTrackInfo.pcTitle[sizeof(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcTitle) - 1] = '\0';
	strncpy(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcArtist, "Artist", sizeof(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcArtist) - 1);
	eventMsg.m_mediaInfo.m_mediaTrackInfo.pcArtist[sizeof(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcArtist) - 1] = '\0';
	eventMsg.m_mediaInfo.m_mediaTrackInfo.ui32Duration = 180;
	eventMsg.m_mediaInfo.m_mediaTrackInfo.ui32TrackNumber = 3;
	eventMsg.m_mediaInfo.m_mediaTrackInfo.ui32NumberOfTracks = 10;

	ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
	BTRMGR_EventMessage_t eventMsg;
	eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_FOUND;
	eventMsg.m_pairedDevice.m_deviceHandle = 123;
	strncpy(eventMsg.m_pairedDevice.m_name, "Device Name", sizeof(eventMsg.m_pairedDevice.m_name) - 1);
	eventMsg.m_pairedDevice.m_name[sizeof(eventMsg.m_pairedDevice.m_name) - 1] = '\0';
	eventMsg.m_pairedDevice.m_deviceType = BTRMGR_DEVICE_TYPE_HID;
	eventMsg.m_pairedDevice.m_ui32DevClassBtSpec = 456;
	eventMsg.m_pairedDevice.m_ui16DevAppearanceBleSpec = 789;
	eventMsg.m_pairedDevice.m_isLastConnectedDevice = true;

	EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                default:
                    return "UNKNOWN";
        }
        }));

	ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
        BTRMGR_EventMessage_t eventMsg;
	eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_OUT_OF_RANGE;
	eventMsg.m_pairedDevice.m_deviceHandle = 123;
	strncpy(eventMsg.m_pairedDevice.m_name, "Device Name", sizeof(eventMsg.m_pairedDevice.m_name) - 1);
	eventMsg.m_pairedDevice.m_name[sizeof(eventMsg.m_pairedDevice.m_name) - 1] = '\0';
	eventMsg.m_pairedDevice.m_deviceType = BTRMGR_DEVICE_TYPE_HID;
	eventMsg.m_pairedDevice.m_ui32DevClassBtSpec = 456;
	eventMsg.m_pairedDevice.m_ui16DevAppearanceBleSpec = 789;
	eventMsg.m_pairedDevice.m_isLastConnectedDevice = true;

	EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                default:
                    return "UNKNOWN";
        }
        }));

	ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    {
	BTRMGR_EventMessage_t eventMsg;
	eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_DISCOVERY_UPDATE;
	eventMsg.m_discoveredDevice.m_deviceHandle = 123;
	eventMsg.m_discoveredDevice.m_isDiscovered = true;
	strncpy(eventMsg.m_discoveredDevice.m_name, "Device Name", sizeof(eventMsg.m_discoveredDevice.m_name) - 1);
	eventMsg.m_discoveredDevice.m_name[sizeof(eventMsg.m_discoveredDevice.m_name) - 1] = '\0';
	eventMsg.m_discoveredDevice.m_deviceType = BTRMGR_DEVICE_TYPE_HID;
	eventMsg.m_discoveredDevice.m_ui32DevClassBtSpec = 456;
	eventMsg.m_discoveredDevice.m_ui16DevAppearanceBleSpec = 789;
	eventMsg.m_discoveredDevice.m_isLastConnectedDevice = true;
	eventMsg.m_discoveredDevice.m_isPairedDevice = true;

	EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                default:
                    return "UNKNOWN";
        }
        }));

        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    // Test for BTRMGR_EVENT_DEVICE_MEDIA_STATUS
    {
        BTRMGR_EventMessage_t eventMsg;
	eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_MEDIA_STATUS;
	eventMsg.m_mediaInfo.m_deviceHandle = 123;
	strncpy(eventMsg.m_mediaInfo.m_name, "Device Name", sizeof(eventMsg.m_mediaInfo.m_name) - 1);
	eventMsg.m_mediaInfo.m_name[sizeof(eventMsg.m_mediaInfo.m_name) - 1] = '\0';
	eventMsg.m_mediaInfo.m_deviceType = BTRMGR_DEVICE_TYPE_HID;
	eventMsg.m_mediaInfo.m_mediaDevStatus.m_ui8mediaDevVolume = 50;
	eventMsg.m_mediaInfo.m_mediaDevStatus.m_ui8mediaDevMute = false;
	eventMsg.m_mediaInfo.m_mediaDevStatus.m_enmediaCtrlCmd = BTRMGR_MEDIA_CTRL_VOLUMEUP;

	EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                default:
                    return "UNKNOWN";
        }
        }));

	ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

	EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                default:
                    return "UNKNOWN";
        }
        }));

	eventMsg.m_mediaInfo.m_mediaDevStatus.m_enmediaCtrlCmd = BTRMGR_MEDIA_CTRL_VOLUMEDOWN;
	ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

	EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                default:
                    return "UNKNOWN";
        }
        }));

        eventMsg.m_mediaInfo.m_mediaDevStatus.m_enmediaCtrlCmd = BTRMGR_MEDIA_CTRL_MUTE;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

	EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                default:
                    return "UNKNOWN";
        }
        }));

        eventMsg.m_mediaInfo.m_mediaDevStatus.m_enmediaCtrlCmd = BTRMGR_MEDIA_CTRL_UNMUTE;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

	EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_GetDeviceTypeAsString(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](BTRMGR_DeviceType_t deviceType) -> const char* {
            switch (deviceType) {
                case BTRMGR_DEVICE_TYPE_HID:
                    return "HUMAN INTERFACE DEVICE";
                default:
                    return "UNKNOWN";
        }
        }));

        eventMsg.m_mediaInfo.m_mediaDevStatus.m_enmediaCtrlCmd = BTRMGR_MEDIA_CTRL_UNKNOWN;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }

    // Test for not implemented events
    {
        BTRMGR_EventMessage_t eventMsg;
        eventMsg.m_eventType = BTRMGR_EVENT_MAX;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_OP_READY;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_DEVICE_OP_INFORMATION;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_PLAYER_NAME;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_PLAYER_VOLUME;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_PLAYER_EQUALIZER_OFF;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_PLAYER_EQUALIZER_ON;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_PLAYER_SHUFFLE_OFF;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_PLAYER_SHUFFLE_ALLTRACKS;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_PLAYER_SHUFFLE_GROUP;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_PLAYER_REPEAT_OFF;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_PLAYER_REPEAT_SINGLETRACK;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_PLAYER_REPEAT_ALLTRACKS;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_PLAYER_REPEAT_GROUP;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_ALBUM_INFO;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_ARTIST_INFO;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_GENRE_INFO;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_COMPILATION_INFO;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_PLAYLIST_INFO;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));

        eventMsg.m_eventType = BTRMGR_EVENT_MEDIA_TRACKLIST_INFO;
        ASSERT_EQ(BTRMGR_RESULT_SUCCESS, mockBluetoothManagerInstance->evBluetoothHandler(eventMsg));
    }
}
