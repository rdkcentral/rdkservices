#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>

#define JSON_TIMEOUT   (1000)
#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);
#define WAREHOUSE_CALLSIGN  _T("org.rdk.Warehouse.1")
#define WAREHOUSEL2TEST_CALLSIGN  _T("L2tests.1")

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;

typedef enum : uint32_t {
    WAREHOUSEL2TEST_RESETDONE= 0x00000001,
    WAREHOUSEL2TEST_STATE_INVALID = 0x00000000
}WarehouseL2test_async_events_t;

/**
 * @brief Internal test mock class
 *
 * Note that this is for internal test use only and doesn't mock any actual
 * concrete interface.
 */
class AsyncHandlerMock_Warehouse 
{
    public:
        AsyncHandlerMock_Warehouse ()
        {
        }

        MOCK_METHOD(void, resetDone, (const JsonObject &message));
};

/* WareHouse L2 test class declaration */
class Warehouse_L2Test : public L2TestMocks {
    protected:
        Core::JSONRPC::Message message;
        string response;
        IARM_EventHandler_t whMgrStatusChangeEventsHandler = nullptr;
        IARM_EventHandler_t powerEventHandler = nullptr;
        
        virtual ~Warehouse_L2Test() override;

    public:
        Warehouse_L2Test();
        void resetDone(const JsonObject &message);
        /**
         * @brief waits for various status change on asynchronous calls
         */
        uint32_t WaitForRequestStatus(uint32_t timeout_ms,WarehouseL2test_async_events_t expected_status);

    private:
        /** @brief Mutex */
        std::mutex m_mutex;

        /** @brief Condition variable */
        std::condition_variable m_condition_variable;

        /** @brief Event signalled flag */
        uint32_t m_event_signalled;
};

/**
 * @brief Constructor for WareHouse L2 test class
 */
Warehouse_L2Test::Warehouse_L2Test()
        : L2TestMocks()
{
        uint32_t status = Core::ERROR_GENERAL;
        m_event_signalled = WAREHOUSEL2TEST_STATE_INVALID;

       ON_CALL(*p_iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_EVENT_WAREHOUSEOPS_STATUSCHANGED)) {
                    whMgrStatusChangeEventsHandler = handler;
                }
                // if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED)) {
                //     whMgrStatusChangeEventsHandler = handler;
                // }
                return IARM_RESULT_SUCCESS;
            }));

    /* Activate plugin in constructor */
    status = ActivateService("org.rdk.Warehouse");
    EXPECT_EQ(Core::ERROR_NONE, status);
}

/**
 * @brief Destructor for WareHouse L2 test class
 */
Warehouse_L2Test:: ~Warehouse_L2Test()
{
    uint32_t status = Core::ERROR_GENERAL;
    m_event_signalled = WAREHOUSEL2TEST_STATE_INVALID;

    status = DeactivateService("org.rdk.Warehouse");
    EXPECT_EQ(Core::ERROR_NONE, status);
}

/**
 * @brief called when Temperature threshold
 * changed notification received from IARM
 *
 * @param[in] message from system service on the change
 */
void Warehouse_L2Test::resetDone(const JsonObject &message)
{
    TEST_LOG("resetDone event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    std::string str;
    message.ToString(str);

    TEST_LOG("resetDone received: %s\n", str.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= WAREHOUSEL2TEST_RESETDONE;
    m_condition_variable.notify_one();
}

/**
 * @brief waits for various status change on asynchronous calls
 *
 * @param[in] timeout_ms timeout for waiting
 */
uint32_t Warehouse_L2Test::WaitForRequestStatus(uint32_t timeout_ms,WarehouseL2test_async_events_t expected_status)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto now = std::chrono::system_clock::now();
    std::chrono::milliseconds timeout(timeout_ms);
    uint32_t signalled = WAREHOUSEL2TEST_STATE_INVALID;

   while (!(expected_status & m_event_signalled))
   {
      if (m_condition_variable.wait_until(lock, now + timeout) == std::cv_status::timeout)
      {
         TEST_LOG("Timeout waiting for request status event");
         break;
      }
   }

    signalled = m_event_signalled;

    return signalled;
}

/**
 * @brief Compare two request status objects
 *
 * @param[in] data Expected value
 * @return true if the argument and data match, false otherwise
 */
MATCHER_P(MatchRequestStatus, data, "")
{
    bool match = true;
    std::string expected;
    std::string actual;

    data.ToString(expected);
    arg.ToString(actual);
    TEST_LOG(" rec = %s, arg = %s",expected.c_str(),actual.c_str());
    EXPECT_STREQ(expected.c_str(),actual.c_str());

    return match;
}

TEST_F(Warehouse_L2Test,WarehouseResetDone)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(WAREHOUSE_CALLSIGN,WAREHOUSEL2TEST_CALLSIGN );
    StrictMock<AsyncHandlerMock_Warehouse > async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    uint32_t signalled = WAREHOUSEL2TEST_STATE_INVALID;
    std::string message;
    JsonObject expected_status;

    status = InvokeServiceMethod("org.rdk.Warehouse.1", "resetDevice", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(6)
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(IARM_BUS_PWRMGR_API_WareHouseClear));
                return IARM_RESULT_SUCCESS;
            })
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(IARM_BUS_PWRMGR_API_FactoryReset));
                return IARM_RESULT_SUCCESS;
             })
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(IARM_BUS_PWRMGR_API_WareHouseClear));
                return IARM_RESULT_SUCCESS;
            })
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(IARM_BUS_PWRMGR_API_ColdFactoryReset));
                //Return error for the Negative test case
                return IARM_RESULT_SUCCESS;
            })
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(IARM_BUS_PWRMGR_API_UserFactoryReset));
                return IARM_RESULT_SUCCESS;
            })
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(IARM_BUS_PWRMGR_API_UserFactoryReset));
                //Return error for the Negative test case
                return IARM_RESULT_INVALID_PARAM;
            });

    /* errorCode and errorDescription should not be set */
    EXPECT_FALSE(result.HasLabel("errorCode"));
    EXPECT_FALSE(result.HasLabel("errorDescription"));

    /* Register for resetDone event. */
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                        _T("resetDone"),
                                        &AsyncHandlerMock_Warehouse::resetDone,
                                        &async_handler);
    EXPECT_EQ(Core::ERROR_NONE, status);

    message = "{\"success\":true}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, resetDone(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &Warehouse_L2Test::resetDone));

    IARM_BUS_PWRMgr_WareHouseOpn_EventData_t eventData = {IARM_BUS_PWRMGR_WAREHOUSE_RESET, IARM_BUS_PWRMGR_WAREHOUSE_COMPLETED};
    whMgrStatusChangeEventsHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_WAREHOUSEOPS_STATUSCHANGED, &eventData, 0);

    /* resetDevice method takes 2 parameters as "suppressReboot" & "resetType" */
    params["suppressReboot"] = "true";
    params["resetType"] = "WAREHOUSE_CLEAR";
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "resetDevice", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    params["suppressReboot"] = "true";
    params["resetType"] = "FACTORY";
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "resetDevice", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

    message = "{\"success\":true}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, resetDone(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &Warehouse_L2Test::resetDone))
        .WillOnce(Invoke(this, &Warehouse_L2Test::resetDone));

    params["suppressReboot"] = "false";
    params["resetType"] = "WAREHOUSE_CLEAR";
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "resetDevice", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

    params["suppressReboot"] = "true";
    params["resetType"] = "COLD";
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "resetDevice", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

    message = "{\"success\":true}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, resetDone(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &Warehouse_L2Test::resetDone))
        .WillOnce(Invoke(this, &Warehouse_L2Test::resetDone));

    params["suppressReboot"] = "true";
    params["resetType"] = "USERFACTORY";
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "resetDevice", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

    message = "{\"success\":true}";
    message = "{\"success\":false,\"error\":\"Reset failed\"}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, resetDone(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &Warehouse_L2Test::resetDone));

    params["suppressReboot"] = "true";
    params["resetType"] = "USERFACTORY";
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "resetDevice", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

    signalled = WaitForRequestStatus(JSON_TIMEOUT,WAREHOUSEL2TEST_RESETDONE);
    EXPECT_TRUE(signalled & WAREHOUSEL2TEST_RESETDONE);

    /* Unregister for events. */
     jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("resetDone"));
}


TEST_F(Warehouse_L2Test,WarehouseLightReset)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(WAREHOUSE_CALLSIGN,WAREHOUSEL2TEST_CALLSIGN );
    StrictMock<AsyncHandlerMock_Warehouse > async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    uint32_t signalled = WAREHOUSEL2TEST_STATE_INVALID;
    std::string message;
    JsonObject expected_status;

    status = InvokeServiceMethod("org.rdk.Warehouse.1", "lightReset", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(2)
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_SYSMGR_NAME)));
                if (string(methodName) == string(_T(IARM_BUS_SYSMGR_API_RunScript)))
                {
                    auto* runScriptParam = static_cast<IARM_Bus_SYSMgr_RunScript_t*>(arg);
                    runScriptParam->return_value = -1;
                }
                return IARM_RESULT_SUCCESS;
            })
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_SYSMGR_NAME)));
                if (string(methodName) == string(_T(IARM_BUS_SYSMGR_API_RunScript)))
                {
                    auto* runScriptParam = static_cast<IARM_Bus_SYSMgr_RunScript_t*>(arg);
                    runScriptParam->return_value = 0;
                    EXPECT_EQ(string(runScriptParam->script_path), string("rm -rf /opt/netflix/* /opt/QT/home/data/* /opt/hn_service_settings.conf /opt/apps/common/proxies.conf /opt/lib/bluetooth /opt/persistent/rdkservicestore"));
                }
                return IARM_RESULT_SUCCESS;
            });

    /* errorCode and errorDescription should not be set */
    EXPECT_FALSE(result.HasLabel("errorCode"));
    EXPECT_FALSE(result.HasLabel("errorDescription"));

    /* lightReset method takes no parameters & Always Expecting NULL */
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "lightReset", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    status = InvokeServiceMethod("org.rdk.Warehouse.1", "lightReset", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());
}

TEST_F(Warehouse_L2Test,Warehouse_getHardwareTestResults_executeHardwareTest)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(WAREHOUSE_CALLSIGN,WAREHOUSEL2TEST_CALLSIGN );
    StrictMock<AsyncHandlerMock_Warehouse > async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    uint32_t signalled = WAREHOUSEL2TEST_STATE_INVALID;
    std::string message;
    JsonObject expected_status;

    EXPECT_CALL(*p_rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                EXPECT_EQ(string(pcCallerID), string("Warehouse"));
                EXPECT_EQ(string(pcParameterName), string("Device.DeviceInfo.X_RDKCENTRAL-COM_xOpsDeviceMgmt.hwHealthTest.Results"));
                strncpy(pstParamData->value, "test", sizeof(pstParamData->value));
                return WDMP_SUCCESS;
            }));
    EXPECT_CALL(*p_rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(string(pcCallerID), _T("Warehouse"));
                EXPECT_EQ(string(pcParameterName), _T("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.hwHealthTest.Enable"));
                EXPECT_EQ(string(pcParameterValue), _T("false"));
                EXPECT_EQ(eDataType, WDMP_BOOLEAN);
                return WDMP_SUCCESS;
            }));

    params["testResults"] = "";
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "getHardwareTestResults", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("test", result["testResults"].String().c_str());

    EXPECT_CALL(*p_rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(string(pcCallerID), _T("Warehouse"));
                EXPECT_EQ(string(pcParameterName), _T("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.hwHealthTest.Enable"));
                EXPECT_EQ(string(pcParameterValue), _T("true"));
                EXPECT_EQ(eDataType, WDMP_BOOLEAN);
                return WDMP_SUCCESS;
            }))
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(string(pcCallerID), _T("Warehouse"));
                EXPECT_EQ(string(pcParameterName), _T("Device.DeviceInfo.X_RDKCENTRAL-COM_xOpsDeviceMgmt.hwHealthTest.ExecuteTest"));
                EXPECT_EQ(string(pcParameterValue), _T("1"));
                EXPECT_EQ(eDataType, WDMP_INT);
                return WDMP_SUCCESS;
            }));
    
    params["testResults"] = "";
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "executeHardwareTest", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("true", result["success"].String().c_str());

    EXPECT_CALL(*p_rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                EXPECT_EQ(string(pcCallerID), string("Warehouse"));
                EXPECT_EQ(string(pcParameterName), string("Device.DeviceInfo.X_RDKCENTRAL-COM_xOpsDeviceMgmt.hwHealthTest.Results"));
                strncpy(pstParamData->value, "test", sizeof(pstParamData->value));
                return WDMP_SUCCESS;
            }));
    EXPECT_CALL(*p_rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(string(pcCallerID), _T("Warehouse"));
                EXPECT_EQ(string(pcParameterName), _T("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.hwHealthTest.Enable"));
                EXPECT_EQ(string(pcParameterValue), _T("false"));
                EXPECT_EQ(eDataType, WDMP_BOOLEAN);
                return WDMP_SUCCESS;
            }));


    params["testResults"] = "Timezone: NA 2021-04-15 10:35:06 Test execution start, remote trigger ver. 0011 2021-04-15 10:35:10 Test result: Audio/Video Decoder:PASSED 2021-04-15 10:35:06 Test result: Dynamic RAM:PASSED 2021-04-15 10:35:06 Test result: Flash Memory:PASSED 2021-04-15 10:35:06 Test result: HDMI Output:PASSED 2021-04-15 10:35:38 Test result: IR Remote Interface:WARNING_IR_Not_Detected 2021-04-15 10:35:06 Test result: Bluetooth:PASSED 2021-04-15 10:35:06 Test result: SD Card:PASSED 2021-04-15 10:35:06 Test result: WAN:PASSED 2021-04-15 10:35:38 Test execution completed:PASSED";
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "getHardwareTestResults", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("test", result["testResults"].String().c_str());
   
}

extern "C" FILE* __real_popen(const char* command, const char* type);
extern "C" int __real_pclose(FILE* pipe);

TEST_F(Warehouse_L2Test,getdeviceInfo)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(WAREHOUSE_CALLSIGN,WAREHOUSEL2TEST_CALLSIGN );
    StrictMock<AsyncHandlerMock_Warehouse > async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    uint32_t signalled = WAREHOUSEL2TEST_STATE_INVALID;
    std::string message;
    JsonObject expected_status;

  
   status = InvokeServiceMethod("org.rdk.Warehouse.1", "getDeviceInfo", params, result);
   EXPECT_EQ(Core::ERROR_NONE, status);
   EXPECT_TRUE(result["success"].Boolean());
   EXPECT_STREQ("null", result["value"].String().c_str());

    const string deviceInfoScript = _T("/lib/rdk/getDeviceDetails.sh");
    const uint8_t deviceInfoContent[] = "echo \"bluetooth_mac=12:34:56:78:90:AB\n"
                                    "boxIP=192.168.1.0\n"
                                    "build_type=VBN\n"
                                    "estb_mac=12:34:56:78:90:AB\n"
                                    "eth_mac=12:34:56:78:90:AB\n"
                                    "friendly_id=Abc XYZ\n"
                                    "imageVersion=ABCADS_VBN_2022010101sdy__MH_SPLASH_TEST_2\n"
                                    "model_number=ABC123ADS\n"
                                    "wifi_mac=12:34:56:78:90:AB\"\n";

    ON_CALL(*p_wrapsImplMock, popen(::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const char* command, const char* type) -> FILE* {
                EXPECT_EQ(string(command), string(_T("sh /lib/rdk/getDeviceDetails.sh read")));
                return __real_popen(command, type);
            }));
    ON_CALL(*p_wrapsImplMock, pclose(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](FILE* pipe){
                return __real_pclose(pipe);
            }));

    //Create fake device info script & Invoke getDeviceInfo
    Core::File file(deviceInfoScript);
    file.Create();
    //file.Open(false);
    file.Write(deviceInfoContent, sizeof(deviceInfoContent));            

    status = InvokeServiceMethod("org.rdk.Warehouse.1", "getDeviceInfo", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());   
    EXPECT_STREQ("null", result["value"].String().c_str());       

    file.Destroy();

}


TEST_F(Warehouse_L2Test, Warehouse_internalReset)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(WAREHOUSE_CALLSIGN,WAREHOUSEL2TEST_CALLSIGN );
    StrictMock<AsyncHandlerMock_Warehouse > async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    uint32_t signalled = WAREHOUSEL2TEST_STATE_INVALID;
    std::string message;
    JsonObject expected_status;

    //Invoke internalReset - No pass phrase
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "internalReset", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    params["passPhrase"] = "Test Phrase";
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "internalReset", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());


    //Invoke internalReset - Incorrect pass phrase
    params["passPhrase"] = "Incorrect pass phrase";
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "internalReset", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(2)
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_SYSMGR_NAME)));
                if (string(methodName) == string(_T(IARM_BUS_SYSMGR_API_RunScript)))
                {
                    auto* runScriptParam = static_cast<IARM_Bus_SYSMgr_RunScript_t*>(arg);
                    runScriptParam->return_value = -1;
                }
                return IARM_RESULT_SUCCESS;
            })
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_SYSMGR_NAME)));
                if (string(methodName) == string(_T(IARM_BUS_SYSMGR_API_RunScript)))
                {
                    auto* runScriptParam = static_cast<IARM_Bus_SYSMgr_RunScript_t*>(arg);
                    runScriptParam->return_value = 0;
                    EXPECT_EQ(string(runScriptParam->script_path), string("rm -rf /opt/drm /opt/www/whitebox /opt/www/authService && /rebootNow.sh -s WarehouseService &"));
                }
                return IARM_RESULT_SUCCESS;
            });

    //Invoke internalReset - correct pass phrase
    params["passPhrase"] = "FOR TEST PURPOSES ONLY";
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "internalReset", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("script returned: -1", result["error"].String().c_str());

    //Invoke internalReset - Correct pass phrase - Return success
    params["passPhrase"] = "FOR TEST PURPOSES ONLY";
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "internalReset", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());
}


TEST_F(Warehouse_L2Test,iscleanTest)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(WAREHOUSE_CALLSIGN,WAREHOUSEL2TEST_CALLSIGN );
    StrictMock<AsyncHandlerMock_Warehouse > async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    uint32_t signalled = WAREHOUSEL2TEST_STATE_INVALID;
    std::string message;
    JsonObject expected_status;
    const string userPrefFile = _T("/opt/user_preferences.conf");
    const uint8_t userPrefLang[] = "[General]\nui_language=US_en\n";
    //const uint8_t userPrefLang[] = "";
    const string customDataFile = _T("/lib/rdk/wh_api_5.conf");
    const uint8_t customDataFileContent[] = "[files]\n/opt/user_preferences.conf\n";
    //No conf file
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "isClean", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_FALSE(result["clean"].Boolean());
    EXPECT_STREQ("[]", result["files"].String().c_str());
    //Empty conf file
    Core::File fileConf(customDataFile);
    Core::Directory(fileConf.PathName().c_str()).CreatePath();
    fileConf.Create();
    params["age"]=300;
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "isClean", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_FALSE(result["clean"].Boolean());
    EXPECT_STREQ("[]", result["files"].String().c_str());
    //Create empty conf file
    fileConf.Write(customDataFileContent, sizeof(customDataFileContent));
    params["age"]=300;
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "isClean", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_TRUE(result["clean"].Boolean());
    EXPECT_STREQ("[]", result["files"].String().c_str());
   
    //Add test data to conf file
    Core::File filePref(userPrefFile);
    Core::Directory(filePref.PathName().c_str()).CreatePath();
    filePref.Create();
    filePref.Open();
    filePref.Write(userPrefLang, sizeof(userPrefLang));
    
    params["age"]=-4;
    status = InvokeServiceMethod("org.rdk.Warehouse.1", "isClean", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_FALSE(result["clean"].Boolean());
    EXPECT_STREQ("[\"\\/opt\\/user_preferences.conf\"]", result["files"].String().c_str());
    fileConf.Destroy();
    filePref.Destroy();
}