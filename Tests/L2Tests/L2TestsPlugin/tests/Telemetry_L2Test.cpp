#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>

#define JSON_TIMEOUT   (1000)
#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);
#define TELEMETRY_CALLSIGN  _T("org.rdk.Telemetry.1")
#define TELEMETRYL2TEST_CALLSIGN _T("L2tests.1")

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;

typedef enum : uint32_t {
    TELEMETRYL2TEST_ONREPORTUPLOAD = 0x00000001,
    TELEMETRYL2TEST_STATE_INVALID = 0x00000000
}TelemetryL2test_async_events_t;
/**
 * @brief Internal test mock class
 *
 * Note that this is for internal test use only and doesn't mock any actual
 * concrete interface.
 */
class AsyncHandlerMock_Telemetry
{
    public:
        AsyncHandlerMock_Telemetry()
        {
        }

        MOCK_METHOD(void, onReportUpload, (const JsonObject &message));
};

/* Telemetry L2 test class declaration */
class Telemetry_L2test : public L2TestMocks {
protected:
    IARM_EventHandler_t powerEventHandler = nullptr;

    Telemetry_L2test();
    virtual ~Telemetry_L2test() override;

    public:
      void onReportUpload(const JsonObject &message);

        /**
         * @brief waits for various status change on asynchronous calls
         */
      uint32_t WaitForRequestStatus(uint32_t timeout_ms,TelemetryL2test_async_events_t expected_status);

    private:
        /** @brief Mutex */
        std::mutex m_mutex;

        /** @brief Condition variable */
        std::condition_variable m_condition_variable;

        /** @brief Event signalled flag */
        uint32_t m_event_signalled;
};

/**
 * @brief Constructor for Telemetry L2 test class
 */
Telemetry_L2test::Telemetry_L2test()
        : L2TestMocks()
{
        uint32_t status = Core::ERROR_GENERAL;
        m_event_signalled = TELEMETRYL2TEST_STATE_INVALID;

        ON_CALL(*p_iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED)) {
                        EXPECT_TRUE(handler != nullptr);
                        powerEventHandler = handler;
                    }
                    return IARM_RESULT_SUCCESS;
                }));

         /* Activate plugin in constructor */
         status = ActivateService("org.rdk.Telemetry");
         EXPECT_EQ(Core::ERROR_NONE, status);

}

/**
 * @brief Destructor for Telemetry L2 test class
 */
Telemetry_L2test::~Telemetry_L2test()
{
    uint32_t status = Core::ERROR_GENERAL;
    m_event_signalled = TELEMETRYL2TEST_STATE_INVALID;

    status = DeactivateService("org.rdk.Telemetry");
    EXPECT_EQ(Core::ERROR_NONE, status);
}

/**
 *
 * @param[in] message from Telemetry on the change
 */
void Telemetry_L2test::onReportUpload(const JsonObject &message)
{
    TEST_LOG("onReportUpload event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    std::string str;
    message.ToString(str);

    TEST_LOG("onReportUpload received: %s\n", str.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= TELEMETRYL2TEST_ONREPORTUPLOAD;
    m_condition_variable.notify_one();
}

/**
 * @brief waits for various status change on asynchronous calls
 *
 * @param[in] timeout_ms timeout for waiting
 */
uint32_t Telemetry_L2test::WaitForRequestStatus(uint32_t timeout_ms,TelemetryL2test_async_events_t expected_status)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto now = std::chrono::system_clock::now();
    std::chrono::milliseconds timeout(timeout_ms);
    uint32_t signalled = TELEMETRYL2TEST_STATE_INVALID;

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

/********************************************************
************Test case Details **************************
** 1. Triggered UploadReport Method 
** 2. Verify the response of UploadReport Method
** 3. Subscribe and Triggered onReportUpload Event
** 3. Verify the event onReportUpload getting triggered
*******************************************************/

TEST_F(Telemetry_L2test,TelemetryReportupload)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(TELEMETRY_CALLSIGN, TELEMETRYL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_Telemetry> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params,thresholds;
    struct _rbusObject rbObject;
    struct _rbusValue rbValue;

    JsonObject result;
    uint32_t signalled = TELEMETRYL2TEST_STATE_INVALID;
    std::string message;
    JsonObject expected_status;

    ON_CALL(*p_rBusApiImplMock, rbus_open(::testing::_, ::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    ON_CALL(*p_rBusApiImplMock, rbusValue_GetString(::testing::_, ::testing::_))
        .WillByDefault(
            ::testing::Return( "SUCCESS"));

    ON_CALL(*p_rBusApiImplMock, rbusObject_GetValue(::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](rbusObject_t object, char const* name) {
                EXPECT_EQ(object, &rbObject);
                EXPECT_EQ(string(name), _T("UPLOAD_STATUS"));
                return &rbValue;
            }));

    ON_CALL(*p_rBusApiImplMock, rbusMethod_InvokeAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                callback(handle, methodName, RBUS_ERROR_SUCCESS, &rbObject);

                return RBUS_ERROR_SUCCESS;
            }));

    /* errorCode and errorDescription should not be set */
    EXPECT_FALSE(result.HasLabel("errorCode"));
    EXPECT_FALSE(result.HasLabel("errorDescription"));

    /* Register for OnReportUpload event. */
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onReportUpload"),
                                           &AsyncHandlerMock_Telemetry::onReportUpload,
                                           &async_handler);

    EXPECT_EQ(Core::ERROR_NONE, status);

    message = "{\"telemetryUploadStatus\":\"UPLOAD_SUCCESS\"}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onReportUpload(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &Telemetry_L2test::onReportUpload));

    /* uploadReport method takes no parameters & Always Expecting NULL */
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "uploadReport", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    /**Called rbusMethod_InvokeAsync Twice, where once it returns "RBUS_ERROR_SUCCESS" for IARM ModeChanged
       & again returns "RBUS_ERROR_BUS_ERROR" for "ERROR_RPC_CALL_FAILED" Errorcheck**/

    EXPECT_CALL(*p_rBusApiImplMock, rbusMethod_InvokeAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                EXPECT_EQ(string(methodName), _T("Device.X_RDKCENTRAL-COM_T2.UploadDCMReport"));
                return RBUS_ERROR_SUCCESS;
            }))
        .WillOnce(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                return RBUS_ERROR_BUS_ERROR;
            }));

    ON_CALL(*p_rBusApiImplMock, rbus_close(::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    /* UploadReport is called by powerEventHandler internally in the below case */
    IARM_Bus_PWRMgr_EventData_t param;
    param.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    param.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP;
    powerEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &param, 0);

    /* "ERROR_RPC_CALL_FAILED" -- ErrorCheck */
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "uploadReport", params, result);
    EXPECT_EQ(Core::ERROR_RPC_CALL_FAILED, status);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,TELEMETRYL2TEST_ONREPORTUPLOAD);
    EXPECT_TRUE(signalled & TELEMETRYL2TEST_ONREPORTUPLOAD);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onReportUpload"));
}

/********************************************************
************Test case Details **************************
** 1. Triggered setReportProfileStatus Method 
** 2. Verify the response of setReportProfileStatus Method
*******************************************************/

TEST_F(Telemetry_L2test, TelemetrysetReportProfileStatus){

JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(TELEMETRY_CALLSIGN, TELEMETRYL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_Telemetry> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    JsonObject expected_status;

    /* Without params and with Params as "No status" expecting Fail case*/
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "setReportProfileStatus", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    params["status"] = "No status";
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "setReportProfileStatus", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    /* With Params as "STARTED" expecting Success*/
    params["status"] = "STARTED";
     status = InvokeServiceMethod("org.rdk.Telemetry.1", "setReportProfileStatus", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    /* With Params as "COMPLETE" expecting Success*/
    params["status"] = "COMPLETE";
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "setReportProfileStatus", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    /*mocking RFC parameter and ErrorCheck for "NO Status" */
    EXPECT_CALL(*p_rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                return WDMP_FAILURE;
            }));

    params["status"] = "STARTED";
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "setReportProfileStatus", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str()); 

}

/********************************************************
************Test case Details **************************
** 1. Triggered logApplicationEvent Method 
** 2. Verify the response of logApplicationEvent Method
*******************************************************/

TEST_F(Telemetry_L2test, TelemetrylogApplicationEvent){
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(TELEMETRY_CALLSIGN, TELEMETRYL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_Telemetry> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    JsonObject expected_status;

    /* Without params expecting Fail case*/
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "logApplicationEvent", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    /*With one Param  expecting Fail case */
    params["eventName"] = "";
    params[""] = "";
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "logApplicationEvent", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    /*With both Params expecting Success*/
    params["eventName"] = ".....";
    params["eventValue"] = ".....";
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "logApplicationEvent", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

}

/********************************************************
************Test case Details **************************
** 1. Triggered abortReport Method 
** 2. Verify the response of abortReport Method
*******************************************************/

TEST_F(Telemetry_L2test, TelemetryAbortReport){

    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(TELEMETRY_CALLSIGN, TELEMETRYL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_Telemetry> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    JsonObject expected_status;
    struct _rbusObject rbObject;

    status = InvokeServiceMethod("org.rdk.Telemetry.1", "abortReport", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_STREQ("null", result["value"].String().c_str());

    ON_CALL(*p_rBusApiImplMock, rbus_open(::testing::_, ::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    ON_CALL(*p_rBusApiImplMock, rbusMethod_InvokeAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                callback(handle, methodName, RBUS_ERROR_SUCCESS, &rbObject);

                return RBUS_ERROR_SUCCESS;
            }));

    status = InvokeServiceMethod("org.rdk.Telemetry.1", "abortReport", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_STREQ("null", result["value"].String().c_str());

    /**Called rbusMethod_InvokeAsync Twice, where once it returns "RBUS_ERROR_SUCCESS" for IARM ModeChanged
       & again returns "RBUS_ERROR_BUS_ERROR" for "ERROR_RPC_CALL_FAILED" Errorcheck**/

    EXPECT_CALL(*p_rBusApiImplMock, rbusMethod_InvokeAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                return RBUS_ERROR_BUS_ERROR;
            }))
        .WillOnce(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                return RBUS_ERROR_BUS_ERROR;
            }));

    /* abortReport is called by powerEventHandler internally in the below case */
    IARM_Bus_PWRMgr_EventData_t param;
    param.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    param.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    powerEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &param, 0);

    /* "ERROR_RPC_CALL_FAILED" -- ErrorCheck */
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "abortReport", params, result);
    EXPECT_EQ(Core::ERROR_RPC_CALL_FAILED, status);

}

/********************************************************
************Test case Details **************************
** 1. TelemetryMock for t2_init & t2_event_s as per the components
** 2. Triggered setReportProfileStatus Method 
** 3. Verify the response of setReportProfileStatus Method
** 4. Triggered logApplicationEvent Method 
** 5. Verify the response of logApplicationEvent Method
*******************************************************/

TEST_F(Telemetry_L2test, TelemetryImplementationMock){

    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(TELEMETRY_CALLSIGN, TELEMETRYL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_Telemetry> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    JsonObject expected_status;


    EXPECT_CALL(*p_telemetryApiImplMock, t2_init(::testing::_))
         .Times(::testing::AnyNumber());

    EXPECT_CALL(*p_rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                return WDMP_FAILURE;
            }));

    EXPECT_CALL(*p_telemetryApiImplMock, t2_event_s(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillOnce(::testing::Invoke(
            [](char* marker, char* value) {
                EXPECT_EQ(string(marker), _T("NAME"));
                EXPECT_EQ(string(value), _T("VALUE"));
                return T2ERROR_SUCCESS;
               }));  

    status = InvokeServiceMethod("org.rdk.Telemetry.1", "setReportProfileStatus", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);

    params["status"] = "No status";
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "setReportProfileStatus", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);

    params["status"] = "STARTED";
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "setReportProfileStatus", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status); 


    status = InvokeServiceMethod("org.rdk.Telemetry.1", "logApplicationEvent", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);

    /*If param has eventName but not evenValue*/
    params["eventName"] = "";
    params[""] = "";
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "logApplicationEvent", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);

    /*If param have eventname and eventvalue.*/
    params["eventName"] = ".....";
    params["eventValue"] = ".....";
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "logApplicationEvent", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
}

/********************************************************
************Test case Details **************************
** 1. RbusOpenFailure ErrorCheck 
** 2. Triggered abortReport Method 
** 3. Verify the response of abortReport Method
** 4. Triggered uploadReport Method 
** 5. Verify the response of uploadReport Method
*******************************************************/

TEST_F(Telemetry_L2test, TelemetryRbusOpeningErrorCheck){

    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(TELEMETRY_CALLSIGN, TELEMETRYL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_Telemetry> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    JsonObject expected_status;

    EXPECT_CALL(*p_rBusApiImplMock, rbus_open(::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [&](rbusHandle_t* handle, char const* componentName) {
                EXPECT_TRUE(nullptr != handle);
                EXPECT_EQ(string(componentName), _T("TelemetryThunderPlugin"));
                return RBUS_ERROR_BUS_ERROR;
            }))
        .WillOnce(::testing::Invoke(
            [&](rbusHandle_t* handle, char const* componentName) {
                EXPECT_TRUE(nullptr != handle);
                EXPECT_EQ(string(componentName), _T("TelemetryThunderPlugin"));
                return RBUS_ERROR_BUS_ERROR;
            }));

    /* "ERROR_OPENING_FAILED" -- ErrorCheck */
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "abortReport", params, result);
    EXPECT_EQ(Core::ERROR_OPENING_FAILED, status);

    /* "ERROR_OPENING_FAILED" -- ErrorCheck */
    status = InvokeServiceMethod("org.rdk.Telemetry.1", "uploadReport", params, result);
    EXPECT_EQ(Core::ERROR_OPENING_FAILED, status);
 
}
/********************************************************
************Test case Details **************************
** 1. ReportUploadStatus ErrorCheck 
** 2. Triggered uploadReport Method 
** 3. Verify the response of uploadReport Method
*******************************************************/

TEST_F(Telemetry_L2test, TelemetryReportUploadErrorCheck){

    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(TELEMETRY_CALLSIGN, TELEMETRYL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_Telemetry> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    JsonObject expected_status;
    struct _rbusObject rbObject;

    ON_CALL(*p_rBusApiImplMock, rbus_open(::testing::_, ::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    ON_CALL(*p_rBusApiImplMock, rbusValue_GetString(::testing::_, ::testing::_))
        .WillByDefault(
            ::testing::Return( "SUCCESS"));

    /**Called rbusObject_GetValue where it returns nullptr to verify "No 'UPLOAD_STATUS' value" for onReportUploadStatus **/
    EXPECT_CALL(*p_rBusApiImplMock, rbusObject_GetValue(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](rbusObject_t object, char const* name) {
                EXPECT_EQ(object, &rbObject);
                EXPECT_EQ(string(name), _T("UPLOAD_STATUS"));
                return nullptr;
            }));

    ON_CALL(*p_rBusApiImplMock, rbusMethod_InvokeAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                callback(handle, methodName, RBUS_ERROR_SUCCESS, &rbObject);
                return RBUS_ERROR_SUCCESS;
            }));


    status = InvokeServiceMethod("org.rdk.Telemetry.1", "uploadReport", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
 
}