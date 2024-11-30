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

TEST_F(BluetoothTest, GetApiVersionNumber_Response) {
    // API_VERSION_NUMBER_MAJOR is not defined in header file. So, this test case will be failed if API_VERSION_NUMBER_MAJOR is changed in future.	
    const int expectedVersion = 1;

    // Call the method
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getApiVersionNumber"), _T("{}"), response));

    // Verify the response contains the expected version
    EXPECT_EQ(response, "{\"version\":" + std::to_string(expectedVersion) + ",\"success\":true}");
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
