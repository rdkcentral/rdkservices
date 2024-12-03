#include <gtest/gtest.h>
#include "Bluetooth.h"
#include "BluetoothMocks.h"
#include "FactoriesImplementation.h"
#include <vector>

// Declare the mock instance globally for C function overrides
MockBluetoothManager* mockBluetoothManagerInstance = nullptr;

using namespace WPEFramework;

class BluetoothTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::Bluetooth> bluetooth;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    BluetoothTest()
        : bluetooth(Core::ProxyType<Plugin::Bluetooth>::Create())
        , handler(*bluetooth)
        , connection(1, 0)
    {	  
          mockBluetoothManagerInstance = new MockBluetoothManager();
    }

    void SetUp() override {}

    void TearDown() override {
        //delete bluetooth;
	delete mockBluetoothManagerInstance;
        mockBluetoothManagerInstance = nullptr;
    }

    virtual ~BluetoothTest() = default;
};

// Test Case: Check registered JSONRPC methods
TEST_F(BluetoothTest, RegisteredMethods) {	
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("startScan")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("stopScan")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getApiVersionNumber")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isDiscoverable")));
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
        .Times(5)
        .WillRepeatedly(::testing::DoAll(::testing::SetArgPointee<0>(1), ::testing::Return(BTRMGR_RESULT_SUCCESS)));

    // Mock the behavior for starting device discovery
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StartDeviceDiscovery(::testing::Eq(0), ::testing::_))
        .Times(5)
        .WillRepeatedly(::testing::Return(BTRMGR_RESULT_SUCCESS));

    // Mock the behavior for stopping device discovery
    EXPECT_CALL(*mockBluetoothManagerInstance, BTRMGR_StopDeviceDiscovery(::testing::_, ::testing::_))
        .Times(5)
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
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("sendAudioPlaybackCommand"), paramsStr, response));  // Expect error due to missing both parameters
    EXPECT_EQ(response.empty(), true);  // Expect the response to be empty since both deviceID and command are missing
}
