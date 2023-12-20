#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <thread>
#include <chrono>
#include "readprocMockInterface.h"

#define JSON_TIMEOUT   (1000)
#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);
#define SYSTEM_CALLSIGN  _T("org.rdk.System.1")
#define L2TEST_CALLSIGN _T("L2tests.1")

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;

typedef enum : uint32_t {
    SYSTEMSERVICEL2TEST_SYSTEMSTATE_CHANGED = 0x00000001,
    SYSTEMSERVICEL2TEST_THERMALSTATE_CHANGED=0x00000010,
    SYSTEMSERVICEL2TEST_FIRMWARE_UPDATE=0x00000100,
    SYSTEMSERVICEL2TEST_LOGUPLOADSTATE_CHANGED=0x00001000,
    SYSTEMSERVICEL2TEST_FIRMWAREUPDATESTATE_CHANGED=0x00010000,
    SYSTEMSERVICEL2TEST_FIRMWAREPENDING_REBOOT=0x00100000,
    SYSTEMSERVICEL2TEST_POWER_REBOOT=0x01000000,
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
        MOCK_METHOD(void, onPwrMgrReboot, (const JsonObject &message));
        MOCK_METHOD(void, reportFirmwareUpdateInfoReceived, (const JsonObject &message));
        MOCK_METHOD(void, onFirmwareUpdateStateChange, (const JsonObject &message));
        MOCK_METHOD(void, onFirmwarePendingReboot, (const JsonObject &message));
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
         * @brief called when power reboot
         * changed notification received because of state change
         */
      void onPwrMgrReboot(const JsonObject &message);

        /**
         * @brief called when firmwareupdate
         * changed notification received because of state change
         */
      void reportFirmwareUpdateInfoReceived(const JsonObject &message);

        /**
         * @brief called when Firmware state
         * changed notification received from IARM
         */
      void onFirmwareUpdateStateChange(const JsonObject &message);

        /**
         * @brief called when Firmware Pending Reboot method invoked
         * changed notification received from IARM
         */
      void onFirmwarePendingReboot(const JsonObject &message);
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
                if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_EVENT_REBOOTING)) {
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
 * @brief called when power reboot status called
 * changed notification received because of state change
 *
 * @param[in] message from system service on the change
 */
void SystemService_L2Test::onPwrMgrReboot(const JsonObject &message)
{
    TEST_LOG("onPwrMgrReboot event triggered ******\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    std::string str;
    message.ToString(str);

    TEST_LOG("onPwrMgrReboot received: %s\n", str.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= SYSTEMSERVICEL2TEST_POWER_REBOOT;
    m_condition_variable.notify_one();
}

/**
 * @brief called when firmware update status called
 * changed notification received because of state change
 *
 * @param[in] message from system service on the change
 */
void SystemService_L2Test::reportFirmwareUpdateInfoReceived(const JsonObject &message)
{
    TEST_LOG("reportFirmwareUpdateInfoReceived event triggered ******\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    std::string str;
    message.ToString(str);

    TEST_LOG("reportFirmwareUpdateInfoReceived received: %s\n", str.c_str());
    /* Notify the requester thread. */
    m_event_signalled |= SYSTEMSERVICEL2TEST_FIRMWARE_UPDATE;
    m_condition_variable.notify_one();
    TEST_LOG("reportFirmwareUpdateInfoReceived m_event_signalled: %d\n", m_event_signalled);
}

/**
 * @brief called when Firmware state
 * changed notification received from IARM
 *
 * @param[in] message from system service on the change
 */
void SystemService_L2Test::onFirmwareUpdateStateChange(const JsonObject &message)
{
   TEST_LOG("onFirmwareUpdateStateChange event triggered ***\n");
   std::unique_lock<std::mutex> lock(m_mutex);

    std::string str;
    message.ToString(str);

    TEST_LOG("onFirmwareUpdateStateChange received: %s\n", str.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= SYSTEMSERVICEL2TEST_FIRMWAREUPDATESTATE_CHANGED;
    m_condition_variable.notify_one();
}

/**
 * @brief called when Firmware Pending Reboot method invoked
 * changed notification received from IARM
 *
 * @param[in] message from system service on the change
 */
void SystemService_L2Test::onFirmwarePendingReboot(const JsonObject &message)
{
   TEST_LOG("onFirmwarePendingReboot event triggered ***\n");
   std::unique_lock<std::mutex> lock(m_mutex);

    std::string str;
    message.ToString(str);

    TEST_LOG("onFirmwarePendingReboot received: %s\n", str.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= SYSTEMSERVICEL2TEST_FIRMWAREPENDING_REBOOT;
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

/**
 * @brief search the string present in the file
 *
 * @param[in] file path to be searched
 * @param[in] string to be serached
 * @return true if the argument and data match, false otherwise
 */
bool CheckIfStringExistsInFile(const std::string& filePath, const std::string& searchString)
{
    std::ifstream file(filePath);
    bool return_value = false;
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
    }
    else{
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(searchString) != std::string::npos) {
            // The string is present in the file
            return_value =  true;
            break;
            }
        }
    }

    // The string is not found in the file
    file.close();
    return return_value;
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
       .WillByDefault(
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
** 1. Subscribe for reboot request
** 2. Request Reboot
** 3. Verify reboot request async callback notified
** 4. Get previous reboot reason
*******************************************************/

TEST_F(SystemService_L2Test,SystemServiceOnRebootRequest)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEM_CALLSIGN, L2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock> async_handler;
    std::ofstream file("/opt/persistent/previousreboot.info");
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    uint32_t signalled = SYSTEMSERVICEL2TEST_STATE_INVALID;
    std::string message;
    JsonObject expected_status;

    /* Subscribe for reboot requested event. */
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                    _T("onRebootRequest"),
                    &AsyncHandlerMock::onPwrMgrReboot,
                    &async_handler);

    EXPECT_EQ(Core::ERROR_NONE, status);

      // Ignore the application shutdown process here because it would add extra time
      // to the test execution and is not relevant to this particular test case.
      ON_CALL(*p_wrapsImplMock, system(::testing::StrEq("pgrep nrdPluginApp")))
      .WillByDefault(::testing::Return(-1));

      ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_Reboot)));
                auto rebootParam = static_cast<IARM_Bus_PWRMgr_RebootParam_t*>(arg);
                EXPECT_THAT(string(rebootParam->requestor), "SystemServices");
                EXPECT_THAT(string(rebootParam->reboot_reason_custom), "FIRMWARE_FAILURE");
                EXPECT_THAT(string(rebootParam->reboot_reason_other), "FIRMWARE_FAILURE");
                return IARM_RESULT_SUCCESS;
            });

    params["rebootReason"] = "FIRMWARE_FAILURE";

    status = InvokeServiceMethod("org.rdk.System.1", "reboot", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("0", result["IARM_Bus_Call_STATUS"].Value().c_str());

    /* errorCode and errorDescription should not be set */
    EXPECT_FALSE(result.HasLabel("errorCode"));
    EXPECT_FALSE(result.HasLabel("errorDescription"));

    /* Setting previous reboot reason */
    file << "{\n\"timestamp\":\"Tue Nov 21 13:51:39 UTC 2023\",\n \"source\":\"HAL_CDL_notify_mgr_event\",\n \"reason\":\"FIRMWARE_FAILURE\",\n \"customReason\":\"Unknown\",\n \"otherReason\":\"Rebooting the box due to VL_CDL_MANAGER_EVENT_REBOOT...!\"\n}\n";
    file.close();

    /* Request status for TempThreashold. */
    message = "{\"requestedApp\":\"SystemServices\",\"rebootReason\":\"FIRMWARE_FAILURE\"}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onPwrMgrReboot(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &SystemService_L2Test::onPwrMgrReboot));

    IARM_Bus_PWRMgr_RebootParam_t param;
    strncpy(param.requestor, "SystemServices", sizeof(param.requestor));
    strncpy(param.reboot_reason_other, "FIRMWARE_FAILURE", sizeof(param.reboot_reason_other));
    powerEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_REBOOTING, &param, 0);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,SYSTEMSERVICEL2TEST_POWER_REBOOT);
    EXPECT_TRUE(signalled & SYSTEMSERVICEL2TEST_POWER_REBOOT);

    /* Get previous reboot reason */
    status = InvokeServiceMethod("org.rdk.System.1", "getPreviousRebootReason", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("FIRMWARE_FAILURE", result["reason"].Value().c_str());

    /* errorCode and errorDescription should not be set */
    EXPECT_FALSE(result.HasLabel("errorCode"));
    EXPECT_FALSE(result.HasLabel("errorDescription"));

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onRebootRequest"));

}

/********************************************************
************Test case Details **************************
** 1. set the power state
** 2. Request power state
** 3. getdevicepower state
** 4. verify the IARM_BUS_PWRMGR_POWERSTATE_ON
*******************************************************/

TEST_F(SystemService_L2Test,SystemServiceSystemPowerState)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEM_CALLSIGN, L2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock> async_handler;

    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    uint32_t signalled = SYSTEMSERVICEL2TEST_STATE_INVALID;
    std::string message;
    JsonObject expected_status;

    /* Subscribe for power state requested event. */
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                   _T("onSystemPowerStateChanged"),
                                   &AsyncHandlerMock::onSystemPowerStateChanged,
                                   &async_handler);

    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_SetPowerState)));
                auto param = static_cast<IARM_Bus_PWRMgr_SetPowerState_Param_t*>(arg);
                EXPECT_EQ(param->newState, IARM_BUS_PWRMGR_POWERSTATE_ON);
                return IARM_RESULT_SUCCESS;
            });
	params["powerState"] = "ON";

    status = InvokeServiceMethod("org.rdk.System.1","setPowerState", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);

    /* errorCode and errorDescription should not be set */
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_FALSE(result.HasLabel("errorCode"));
    EXPECT_FALSE(result.HasLabel("errorDescription"));

    /*request status for system powerstate*/
    message = "{\"powerState\":\"ON\",\"currentPowerState\":\"DEEP_SLEEP\"}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onSystemPowerStateChanged(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &SystemService_L2Test::onSystemPowerStateChanged));

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    param.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    powerEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &param, 0);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,SYSTEMSERVICEL2TEST_SYSTEMSTATE_CHANGED);
    EXPECT_TRUE(signalled & SYSTEMSERVICEL2TEST_SYSTEMSTATE_CHANGED);

    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_GetPowerState)));
                auto param = static_cast<IARM_Bus_PWRMgr_GetPowerState_Param_t*>(arg);
                param->curState = IARM_BUS_PWRMGR_POWERSTATE_ON;
                return IARM_RESULT_SUCCESS;
            });

    status = InvokeServiceMethod("org.rdk.System.1","getPowerState", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("ON", result["powerState"].Value().c_str());

    EXPECT_FALSE(result.HasLabel("errorCode"));
    EXPECT_FALSE(result.HasLabel("errorDescription"));

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onSystemPowerStateChanged"));
}

/********************************************************
************Test case Details **************************
** 1. invoke the get firmware update info
** 2. request for the firmware update
** 3. once the notify is send
** 4. verify the firmware update
*******************************************************/
TEST_F(SystemService_L2Test,SystemServiceSystemgetFirmwareUpdateInfo )
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEM_CALLSIGN, L2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock> async_handler;

    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    uint32_t signalled = SYSTEMSERVICEL2TEST_STATE_INVALID;
    std::string message;
    JsonObject expected_status;

    const string xconfFile = _T("/lib/rdk/xconfImageCheck.sh");
    Core::File file(xconfFile);
    file.Create();
    file.Close();
    EXPECT_TRUE(Core::File(string(_T("/lib/rdk/xconfImageCheck.sh"))).Exists());

    std::ofstream versionFile("/version.txt");
    versionFile << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\nSDK_VERSION=17.3\nMEDIARITE=8.3.53\nYOCTO_VERSION=dunfell\nVERSION=000.36.0.0\nBUILD_TIME=\"2022-08-05 16:14:54\"\n";
    versionFile.close();

    std::ofstream confHttpCodeFile("/tmp/xconf_httpcode_thunder.txt");
    confHttpCodeFile <<"460\n";
    confHttpCodeFile.close();

    status = InvokeServiceMethod("org.rdk.System.1","getFirmwareUpdateInfo", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);

    /* errorCode and errorDescription should not be set */
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_FALSE(result.HasLabel("errorCode"));
    EXPECT_FALSE(result.HasLabel("errorDescription"));

    /* request the firmware update info received event*/
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                     _T("onFirmwareUpdateInfoReceived"),
                     &AsyncHandlerMock::reportFirmwareUpdateInfoReceived,
                     &async_handler);
    EXPECT_EQ(Core::ERROR_NONE, status);

    /* request status for firmware update*/
    message = "{\"status\":0,\"responseString\":\"\",\"rebootImmediately\":false,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, reportFirmwareUpdateInfoReceived(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &SystemService_L2Test::reportFirmwareUpdateInfoReceived));

    signalled = WaitForRequestStatus(JSON_TIMEOUT,SYSTEMSERVICEL2TEST_FIRMWARE_UPDATE);
    EXPECT_TRUE(signalled & SYSTEMSERVICEL2TEST_FIRMWARE_UPDATE);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onFirmwareUpdateInfoReceived"));
}

/********************************************************
************Test case Details **************************
** 1. Get FirmwareUpdateState from systemservice
** 2. onFirmwareUpdateStateChange event triggered from IARM
** 3. Verify that onFirmwareUpdateStateChange event is notified
*******************************************************/

TEST_F(SystemService_L2Test,SystemServiceUpdatefirmwareStateChange)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEM_CALLSIGN, L2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    uint32_t signalled = SYSTEMSERVICEL2TEST_STATE_INVALID;
    JsonObject result;
    std::string message;
    JsonObject expected_status;

    const string UpdatefirmwareStateFile = _T("/lib/rdk/deviceInitiatedFWDnld.sh");
    Core::File file_1(UpdatefirmwareStateFile);
    if(file_1.Exists())
    {
        EXPECT_TRUE(file_1.Destroy());
    }
    file_1.Create();
    file_1.Close();
    const std::string logFilePath_1 = _T("/opt/logs/swupdate.log");
    Core::File logfile_1(logFilePath_1);
    if(logfile_1.Exists())
    {
        EXPECT_TRUE(logfile_1.Destroy());
    }

    /* Register for Firmware Update State change event. */
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onFirmwareUpdateStateChange"),
                                           &AsyncHandlerMock::onFirmwareUpdateStateChange,
                                           &async_handler);
    EXPECT_EQ(Core::ERROR_NONE, status);

    std::ofstream UpdatefirmwareStateFile_1("/lib/rdk/deviceInitiatedFWDnld.sh");
    if (UpdatefirmwareStateFile_1.is_open()) {
    UpdatefirmwareStateFile_1 << "echo UpdatefirmwareStateChange";
    }
    UpdatefirmwareStateFile_1.close();

    status = InvokeServiceMethod("org.rdk.System.1", "updateFirmware", params, result);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(Core::ERROR_NONE, status);

    const std::string firmwareupdatestatelog = "/opt/logs/swupdate.log";
    // Check if "UpdatefirmwareStateChange" is present in the log file
    EXPECT_TRUE(CheckIfStringExistsInFile(firmwareupdatestatelog, "UpdatefirmwareStateChange"));

    message = "{\"firmwareUpdateStateChange\": 2}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onFirmwareUpdateStateChange(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &SystemService_L2Test::onFirmwareUpdateStateChange));

    IARM_Bus_SYSMgr_EventData_t sysEventData;
    sysEventData.data.systemStates.stateId = IARM_BUS_SYSMGR_SYSSTATE_FIRMWARE_UPDATE_STATE;
    sysEventData.data.systemStates.state = IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_DOWNLOADING;
    systemStateChanged(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, &sysEventData, 0);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,SYSTEMSERVICEL2TEST_FIRMWAREUPDATESTATE_CHANGED);
    EXPECT_TRUE(signalled & SYSTEMSERVICEL2TEST_FIRMWAREUPDATESTATE_CHANGED);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onFirmwareUpdateStateChange"));
}

/********************************************************
************Test case Details **************************
** 1. Get FirmwarePendingReboot from systemservice
** 2. onFirmwarePendingReboot event triggered from IARM
** 3. Verify that onFirmwarePendingReboot event is notified
*******************************************************/
TEST_F(SystemService_L2Test,SystemServiceUpdatefirmwarePendingReboot)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEM_CALLSIGN, L2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    uint32_t signalled = SYSTEMSERVICEL2TEST_STATE_INVALID;
    JsonObject result;
    std::string message;
    JsonObject expected_status;

    const string UpdatefirmwarePendingRebootFile = _T("/lib/rdk/deviceInitiatedFWDnld.sh");
    Core::File file_2(UpdatefirmwarePendingRebootFile);
    if(file_2.Exists())
    {
        EXPECT_TRUE(file_2.Destroy());
    }
    file_2.Create();
    file_2.Close();

    const std::string logFilePath_2 = _T("/opt/logs/swupdate.log");
    Core::File logfile_2(logFilePath_2);
    if(logfile_2.Exists())
    {
        EXPECT_TRUE(logfile_2.Destroy());
    }

    /*Register for Firmware Pending Reboot event. */
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onFirmwarePendingReboot"),
                                           &AsyncHandlerMock::onFirmwarePendingReboot,
                                           &async_handler);
    EXPECT_EQ(Core::ERROR_NONE, status);

    std::ofstream UpdatefirmwarePendingRebootFile_1("/lib/rdk/deviceInitiatedFWDnld.sh");
    if (UpdatefirmwarePendingRebootFile_1.is_open()) {
    UpdatefirmwarePendingRebootFile_1 << "echo UpdatefirmwarePendingReboot";
    }
    UpdatefirmwarePendingRebootFile_1.close();

    status = InvokeServiceMethod("org.rdk.System.1", "updateFirmware", params, result);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(Core::ERROR_NONE, status);

    const std::string firmwarependingrebootlog = "/opt/logs/swupdate.log";
    // Check if "UpdatefirmwareStateChange" is present in the log file
    EXPECT_TRUE(CheckIfStringExistsInFile(firmwarependingrebootlog, "UpdatefirmwarePendingReboot"));

    message = "{\"fireFirmwarePendingReboot\": 600}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onFirmwarePendingReboot(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &SystemService_L2Test::onFirmwarePendingReboot));

    IARM_Bus_SYSMgr_EventData_t sysEventData;
    sysEventData.data.systemStates.stateId = IARM_BUS_SYSMGR_SYSSTATE_FIRMWARE_UPDATE_STATE;
    sysEventData.data.systemStates.state = IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_CRITICAL_REBOOT;
    systemStateChanged(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, &sysEventData, 0);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,SYSTEMSERVICEL2TEST_FIRMWAREPENDING_REBOOT);
    EXPECT_TRUE(signalled & SYSTEMSERVICEL2TEST_FIRMWAREPENDING_REBOOT);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onFirmwarePendingReboot"));
}
