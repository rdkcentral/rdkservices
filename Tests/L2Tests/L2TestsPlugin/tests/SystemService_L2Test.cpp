#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>

#define JSON_TIMEOUT   (1000)
#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);
#define SYSTEM_CALLSIGN  _T("org.rdk.System.1")
#define L2TEST_CALLSIGN _T("L2tests.1")

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;

typedef enum : uint32_t {
    SYSTEMSERVICEL2TEST_SYSTEMSTATE_CHANGED = 0x00000001,
    SYSTEMSERVICEL2TEST_THERMALSTATE_CHANGED=0x00000002,
    SYSTEMSERVICEL2TEST_LOGUPLOADSTATE_CHANGED=0x00000004,
    SYSTEMSERVICEL2TEST_STATE_INVALID = 0x00000000
}SystemServiceL2test_async_events_t;
/**
 * @brief Internal test mock class
 *
 * Note that this is for internal test use only and doesn't mock any actual
 * concrete interface.
 */
class AsyncHandlerMock
{
    public:
        AsyncHandlerMock()
        {
        }

        MOCK_METHOD(void, onTemperatureThresholdChanged, (const JsonObject &message));
        MOCK_METHOD(void, onLogUploadChanged, (const JsonObject &message));
        MOCK_METHOD(void, onSystemPowerStateChanged, (const JsonObject &message));
};

/* Systemservice L2 test class declaration */
class SystemService_L2Test : public L2TestMocks {
protected:
    IARM_EventHandler_t systemStateChanged = nullptr;
    IARM_EventHandler_t thermMgrEventsHandler = nullptr;
    IARM_EventHandler_t powerEventHandler = nullptr;

    SystemService_L2Test();
    virtual ~SystemService_L2Test() override;

    public:
        /**
         * @brief called when Temperature threshold
         * changed notification received from IARM
         */
      void onTemperatureThresholdChanged(const JsonObject &message);

        /**
         * @brief called when Uploadlog status
         * changed notification received because of state change
         */
      void onLogUploadChanged(const JsonObject &message);

        /**
         * @brief called when System state
         * changed notification received from IARM
         */
      void onSystemPowerStateChanged(const JsonObject &message);

        /**
         * @brief waits for various status change on asynchronous calls
         */
      uint32_t WaitForRequestStatus(uint32_t timeout_ms,SystemServiceL2test_async_events_t expected_status);

    private:
        /** @brief Mutex */
        std::mutex m_mutex;

        /** @brief Condition variable */
        std::condition_variable m_condition_variable;

        /** @brief Event signalled flag */
        uint32_t m_event_signalled;
};


/**
 * @brief Constructor for SystemServices L2 test class
 */
SystemService_L2Test::SystemService_L2Test()
        : L2TestMocks()
{
        uint32_t status = Core::ERROR_GENERAL;
        m_event_signalled = SYSTEMSERVICEL2TEST_STATE_INVALID;

        /* Set all the asynchronouse event handler with IARM bus to handle various events*/
        ON_CALL(*p_iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                if ((string(IARM_BUS_SYSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE)) {
                    systemStateChanged = handler;
                }
                if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED)) {
                    thermMgrEventsHandler = handler;
                }
                if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED)) {
                    powerEventHandler = handler;
                }
                return IARM_RESULT_SUCCESS;
            }));

         /* Activate plugin in constructor */
         status = ActivateService("org.rdk.System");
         EXPECT_EQ(Core::ERROR_NONE, status);

}

/**
 * @brief Destructor for SystemServices L2 test class
 */
SystemService_L2Test::~SystemService_L2Test()
{
    uint32_t status = Core::ERROR_GENERAL;
    m_event_signalled = SYSTEMSERVICEL2TEST_STATE_INVALID;

    status = DeactivateService("org.rdk.System");
    EXPECT_EQ(Core::ERROR_NONE, status);
}

/**
 * @brief called when Temperature threshold
 * changed notification received from IARM
 *
 * @param[in] message from system service on the change
 */
void SystemService_L2Test::onTemperatureThresholdChanged(const JsonObject &message)
{
    TEST_LOG("onTemperatureThresholdChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    std::string str;
    message.ToString(str);

    TEST_LOG("onTemperatureThresholdChanged received: %s\n", str.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= SYSTEMSERVICEL2TEST_THERMALSTATE_CHANGED;
    m_condition_variable.notify_one();
}

/**
 * @brief called when Uploadlog status
 * changed notification received because of state change
 *
 * @param[in] message from system service on the change
 */
void SystemService_L2Test::onLogUploadChanged(const JsonObject &message)
{
  TEST_LOG("onLogUploadChanged event triggered ******\n");
   std::unique_lock<std::mutex> lock(m_mutex);

    std::string str;
    message.ToString(str);

    TEST_LOG("onLogUploadChanged received: %s\n", str.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= SYSTEMSERVICEL2TEST_LOGUPLOADSTATE_CHANGED;
    m_condition_variable.notify_one();
}

/**
 * @brief called when System state
 * changed notification received from IARM
 *
 * @param[in] message from system service on the change
 */
void SystemService_L2Test::onSystemPowerStateChanged(const JsonObject &message)
{
    TEST_LOG("onSystemPowerStateChanged event triggered ******\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    std::string str;
    message.ToString(str);

    TEST_LOG("onSystemPowerStateChanged received: %s\n", str.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= SYSTEMSERVICEL2TEST_SYSTEMSTATE_CHANGED;;
    m_condition_variable.notify_one();
}

/**
 * @brief waits for various status change on asynchronous calls
 *
 * @param[in] timeout_ms timeout for waiting
 */
uint32_t SystemService_L2Test::WaitForRequestStatus(uint32_t timeout_ms,SystemServiceL2test_async_events_t expected_status)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto now = std::chrono::system_clock::now();
    std::chrono::milliseconds timeout(timeout_ms);
    uint32_t signalled = SYSTEMSERVICEL2TEST_STATE_INVALID;

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
** 1. Get temperature from systemservice
** 2. Set temperature threshold
** 3. Temperature threshold change event triggered from IARM
** 4. Verify that threshold change event is notified
*******************************************************/

TEST_F(SystemService_L2Test,SystemServiceGetSetTemperature)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEM_CALLSIGN, L2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params,thresholds;
    JsonObject result;
    uint32_t signalled = SYSTEMSERVICEL2TEST_STATE_INVALID;
    std::string message;
    JsonObject expected_status;

    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
   .   WillByDefault(
          [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
              EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
              EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_GetThermalState)));
               auto param = static_cast<IARM_Bus_PWRMgr_GetThermalState_Param_t*>(arg);
               param->curTemperature = 100;
               return IARM_RESULT_SUCCESS;
     });

    status = InvokeServiceMethod("org.rdk.System.1", "getCoreTemperature", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("100.000000", result["temperature"].Value().c_str());

    /* errorCode and errorDescription should not be set */
    EXPECT_FALSE(result.HasLabel("errorCode"));
    EXPECT_FALSE(result.HasLabel("errorDescription"));

    /* Register for temperature threshold change event. */
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onTemperatureThresholdChanged"),
                                           &AsyncHandlerMock::onTemperatureThresholdChanged,
                                           &async_handler);

    EXPECT_EQ(Core::ERROR_NONE, status);

    /* Set Threshold */
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_SetTemperatureThresholds)));
                auto param = static_cast<IARM_Bus_PWRMgr_SetTempThresholds_Param_t*>(arg);
                EXPECT_EQ(param->tempHigh, 95);
                EXPECT_EQ(param->tempCritical, 99);
                return IARM_RESULT_SUCCESS;
            });

    thresholds["WARN"] = 95;
    thresholds["MAX"] = 99;
    params["thresholds"] = thresholds;

    status = InvokeServiceMethod("org.rdk.System.1", "setTemperatureThresholds", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_TRUE(result["success"].Boolean());

    /* errorCode and errorDescription should not be set */
    EXPECT_FALSE(result.HasLabel("errorCode"));
    EXPECT_FALSE(result.HasLabel("errorDescription"));


    /* Request status for TempThreashold. */
    message = "{\"thresholdType\":\"WARN\",\"exceeded\":true,\"temperature\":\"100.000000\"}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onTemperatureThresholdChanged(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &SystemService_L2Test::onTemperatureThresholdChanged));

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.therm.newLevel = IARM_BUS_PWRMGR_TEMPERATURE_HIGH;
    param.data.therm.curLevel = IARM_BUS_PWRMGR_TEMPERATURE_NORMAL;
    param.data.therm.curTemperature = 100;
    thermMgrEventsHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED, &param, 0);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,SYSTEMSERVICEL2TEST_THERMALSTATE_CHANGED);
    EXPECT_TRUE(signalled & SYSTEMSERVICEL2TEST_THERMALSTATE_CHANGED);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onTemperatureThresholdChanged"));
}

/********************************************************
************Test case Details **************************
** 1. Start Log upload
** 2. Subscribe for powerstate change
** 3. Subscribe for LoguploadUpdates
** 4. Trigger system power state change from ON -> DEEP_SLEEP
** 5. Verify UPLOAD_ABORTED event triggered because of power state
** 6. Verify Systemstate event triggered and verify the response
*******************************************************/
TEST_F(SystemService_L2Test,SystemServiceUploadLogsAndSystemPowerStateChange)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEM_CALLSIGN,L2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    uint32_t signalled = SYSTEMSERVICEL2TEST_STATE_INVALID;
    std::string message;
    JsonObject expected_status;

    const string uploadStbLogFile = _T("/lib/rdk/uploadSTBLogs.sh");
    Core::File file(uploadStbLogFile);
    file.Create();
    EXPECT_TRUE(Core::File(string(_T("/lib/rdk/uploadSTBLogs.sh"))).Exists());

    ON_CALL(*p_rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                pstParamData->type = WDMP_BOOLEAN;
                strncpy(pstParamData->value, "true", sizeof(pstParamData->value));
                return WDMP_SUCCESS;
            }));


    std::ofstream deviceProperties("/etc/device.properties");
    deviceProperties << "BUILD_TYPE=dev\n";
    deviceProperties << "FORCE_MTLS=true\n";
    deviceProperties.close();
    EXPECT_TRUE(Core::File(string(_T("/etc/device.properties"))).Exists());

    std::ofstream dcmPropertiesFile("/opt/dcm.properties");
    dcmPropertiesFile << "LOG_SERVER=logs.xcal.tv\n";
    dcmPropertiesFile << "DCM_LOG_SERVER=stblogger.ccp.xcal.tv\n";
    dcmPropertiesFile << "DCM_LOG_SERVER_URL=https://xconf.xcal.tv/loguploader/getSettings\n";
    dcmPropertiesFile << "DCM_SCP_SERVER=stbscp.ccp.xcal.tv\n";
    dcmPropertiesFile << "HTTP_UPLOAD_LINK=https://ssr.ccp.xcal.tv/cgi-bin/S3.cgi\n";
    dcmPropertiesFile << "DCA_UPLOAD_URL=https://stbrtl.r53.xcal.tv\n";
    dcmPropertiesFile.close();
    EXPECT_TRUE(Core::File(string(_T("/opt/dcm.properties"))).Exists());

    std::ofstream tmpDcmSettings("/tmp/DCMSettings.conf");
    tmpDcmSettings << "LogUploadSettings:UploadRepository:uploadProtocol=https\n";
    tmpDcmSettings << "LogUploadSettings:UploadRepository:URL=https://example.com/upload\n";
    tmpDcmSettings << "LogUploadSettings:UploadOnReboot=true\n";
    tmpDcmSettings.close();
    EXPECT_TRUE(Core::File(string(_T("/tmp/DCMSettings.conf"))).Exists());

    status = InvokeServiceMethod("org.rdk.System.1", "uploadLogsAsync", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_TRUE(result["success"].Boolean());

    /* errorCode and errorDescription should not be set */
    EXPECT_FALSE(result.HasLabel("errorCode"));
    EXPECT_FALSE(result.HasLabel("errorDescription"));

    /* Register for abortlog event. */
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onLogUpload"),
                                           &AsyncHandlerMock::onLogUploadChanged,
                                           &async_handler);

    EXPECT_EQ(Core::ERROR_NONE, status);

    /* Register for Powerstate change event. */
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onSystemPowerStateChanged"),
                                           &AsyncHandlerMock::onSystemPowerStateChanged,
                                           &async_handler);

    EXPECT_EQ(Core::ERROR_NONE, status);

    /* Request status for Onlogupload. */
    message = "{\"logUploadStatus\":\"UPLOAD_ABORTED\"}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onLogUploadChanged(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &SystemService_L2Test::onLogUploadChanged));

    /* Request status for Onlogupload. */
    message = "{\"powerState\":\"DEEP_SLEEP\",\"currentPowerState\":\"ON\"}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onSystemPowerStateChanged(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &SystemService_L2Test::onSystemPowerStateChanged));

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    param.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    powerEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &param, 0);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,SYSTEMSERVICEL2TEST_LOGUPLOADSTATE_CHANGED);
    EXPECT_TRUE(signalled & SYSTEMSERVICEL2TEST_LOGUPLOADSTATE_CHANGED);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,SYSTEMSERVICEL2TEST_SYSTEMSTATE_CHANGED);
    EXPECT_TRUE(signalled & SYSTEMSERVICEL2TEST_SYSTEMSTATE_CHANGED);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onLogUpload"));
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onSystemPowerStateChanged"));

}
/********************************************************
************Test case Details **************************
** 1. setBootLoaderSplashScreen success
** 2. setBootLoaderSplashScreen fail
** 3. setBootLoaderSplashScreen invalid path
** 4. setBootLoaderSplashScreen empty path
*******************************************************/

TEST_F(SystemService_L2Test,setBootLoaderSplashScreen)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEM_CALLSIGN,L2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    uint32_t signalled = SYSTEMSERVICEL2TEST_STATE_INVALID;
    std::string message;
    JsonObject expected_status;
    params["path"] = "/tmp/osd1";


    std::ofstream file("/tmp/osd1");
    file << "testing setBootLoaderSplashScreen";
    file.close();

    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
	    .Times(::testing::AnyNumber())
	    .WillRepeatedly(
			    [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
			    EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
			    EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_SetBlSplashScreen)));
			    auto param = static_cast<IARM_Bus_MFRLib_SetBLSplashScreen_Param_t*>(arg);
			    std::string path = param->path;
			    EXPECT_EQ(path, "/tmp/osd1");
			    return IARM_RESULT_SUCCESS;
			    });

    status = InvokeServiceMethod("org.rdk.System.1", "setBootLoaderSplashScreen", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());


    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
	    .Times(::testing::AnyNumber())
	    .WillRepeatedly(
			    [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
			    EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
			    EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_SetBlSplashScreen)));
			    auto param = static_cast<IARM_Bus_MFRLib_SetBLSplashScreen_Param_t*>(arg);
			    std::string path = param->path;
			    EXPECT_EQ(path, "/tmp/osd1");
			    return IARM_RESULT_OOM;
			    });

    status = InvokeServiceMethod("org.rdk.System.1", "setBootLoaderSplashScreen", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    if (result.HasLabel("error")) {
	    EXPECT_STREQ("{\"message\":\"Update failed\",\"code\":\"-32002\"}", result["error"].String().c_str());
    }


    params["path"] = "/tmp/osd2";
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
	    .Times(::testing::AnyNumber())
	    .WillRepeatedly(
			    [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
			    EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
			    EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_SetBlSplashScreen)));
			    auto param = static_cast<IARM_Bus_MFRLib_SetBLSplashScreen_Param_t*>(arg);
			    std::string path = param->path;
			    EXPECT_EQ(path, "/tmp/osd2");
			    return IARM_RESULT_OOM;
			    });

    status = InvokeServiceMethod("org.rdk.System.1", "setBootLoaderSplashScreen", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    if (result.HasLabel("error")) {
	    EXPECT_STREQ("{\"message\":\"Invalid path\",\"code\":\"-32001\"}", result["error"].String().c_str());
    }


    params["path"] = "";
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
	    .Times(::testing::AnyNumber())
	    .WillRepeatedly(
			    [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
			    EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
			    EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_SetBlSplashScreen)));
			    auto param = static_cast<IARM_Bus_MFRLib_SetBLSplashScreen_Param_t*>(arg);
			    std::string path = param->path;
			    EXPECT_EQ(path, "");
			    return IARM_RESULT_OOM;
			    });

    status = InvokeServiceMethod("org.rdk.System.1", "setBootLoaderSplashScreen", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    if (result.HasLabel("error")) {
	    EXPECT_STREQ("{\"message\":\"Invalid path\",\"code\":\"-32001\"}", result["error"].String().c_str());
    }
  
}
