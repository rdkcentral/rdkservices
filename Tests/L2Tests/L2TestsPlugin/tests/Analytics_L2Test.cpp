#include "L2Tests.h"
#include "L2TestsMock.h"
#include <condition_variable>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <interfaces/IAnalytics.h>
#include <mutex>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define TEST_LOG(x, ...)                                                                                                                         \
    fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); \
    fflush(stderr);

#define JSON_TIMEOUT (1000)
#define ANALYTICS_CALLSIGN _T("org.rdk.Analytics")
#define ANALYTICSL2TEST_CALLSIGN _T("L2tests.1")

#define SIFT_SERVER_IP "127.0.0.1"
#define SIFT_SERVER_PORT 12345
#define SIFT_SERVER_TIMEOUT_SEC (30)

#define ZDUMP_MOCK_OUT "America/New_York  Sun Mar  9 07:00:00 2008 UT = Sun Mar  9 03:00:00 2008 EDT isdst=1 gmtoff=-14400\n" \
                       "America/New_York  Sun Nov  2 05:59:59 2008 UT = Sun Nov  2 01:59:59 2008 EDT isdst=1 gmtoff=-14400\n" \
                       "America/New_York  Sun Nov  2 06:00:00 2008 UT = Sun Nov  2 01:00:00 2008 EST isdst=0 gmtoff=-18000\n" \
                       "America/New_York  Sun Mar  8 06:59:59 2009 UT = Sun Mar  8 01:59:59 2009 EST isdst=0 gmtoff=-18000\n" \
                       "America/New_York  Sun Mar  8 07:00:00 2009 UT = Sun Mar  8 03:00:00 2009 EDT isdst=1 gmtoff=-14400"
#define TIME_MOCK_OUT 1212537600 // 2008-06-03 12:00:00

#define EVENTS_MAP "[ \
                { \
                \"event_name\":\"L2MapTestEvent\", \
                \"event_source\":\"L2Test\", \
                \"mapped_event_name\":\"L2TestEventMappedGeneric\" \
                }, \
                { \
                \"event_name\":\"L2MapTestEvent\", \
                \"event_source\":\"L2Test\", \
                \"event_source_version\":\"1.0.0\", \
                \"event_version\":\"1\", \
                \"mapped_event_name\":\"L2TestEventMappedExact\" \
                }, \
                { \
                \"event_name\":\"L2MapTestEvent\", \
                \"event_source\":\"L2Test\", \
                \"event_version\":\"1\", \
                \"mapped_event_name\":\"L2TestEventMappedGenericSourceVersion\" \
                }, \
                { \
                \"event_name\":\"L2MapTestEvent\", \
                \"event_source\":\"L2Test\", \
                \"event_source_version\":\"1.0.0\", \
                \"mapped_event_name\":\"L2TestEventMappedGenericVersion\" \
                } \
                ]"

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;

char gPipeBuffer[4 * 1024];

class SiftServerMock {
public:
    SiftServerMock()
        : mSocket(-1)
    {
        memset(&mAddress, 0, sizeof(mAddress));
    }

    ~SiftServerMock()
    {
        if (mSocket != -1) {
            close(mSocket);
        }
    }

    bool Start()
    {
        mSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (mSocket == -1) {
            TEST_LOG("Socket error");
            return false;
        }

        int optval = 1;
        if (setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            TEST_LOG("Failed to set SO_REUSEADDR");
            close(mSocket);
            return false;
        }

        mAddress.sin_family = AF_INET;
        mAddress.sin_addr.s_addr = inet_addr(SIFT_SERVER_IP);
        mAddress.sin_port = htons(SIFT_SERVER_PORT);

        if (bind(mSocket, (struct sockaddr*)&mAddress, sizeof(mAddress)) == -1) {
            TEST_LOG("Bind error");
            close(mSocket);
            return false;
        }

        if (listen(mSocket, 1) == -1) {
            TEST_LOG("Listen error");
            close(mSocket);
            return false;
        }

        return true;
    }

    string AwaitData(uint32_t timeoutSec, string errCodeResp = "200 OK")
    {
        fd_set readfds;
        struct timeval timeout;
        timeout.tv_sec = timeoutSec;
        timeout.tv_usec = 0;
        string ret;

        FD_ZERO(&readfds);
        FD_SET(mSocket, &readfds);

        int result = select(mSocket + 1, &readfds, NULL, NULL, &timeout);
        if (result == -1) {
            TEST_LOG("Select error");
            return ret;
        } else if (result == 0) {
            TEST_LOG("Timeout occurred! No data received.\n");
            return ret;
        }

        int client_socket;
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);

        client_socket = accept(mSocket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket == -1) {
            TEST_LOG("Accept error");
            return ret;
        }
        struct timeval client_timeout;
        client_timeout.tv_sec = timeoutSec;
        client_timeout.tv_usec = 0;

        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&client_timeout, sizeof(client_timeout));
        char rxBuf[32 * 1024];
        ssize_t bytes_received = recv(client_socket, rxBuf, sizeof(rxBuf) - 1, 0);
        if (bytes_received == -1) {
            TEST_LOG("Recv error");
        } else {
            rxBuf[bytes_received] = '\0';
            TEST_LOG("Received data: %s\n", rxBuf);

            // Send HTTP code response
            std::string httpResponse = "HTTP/1.1 " + errCodeResp + "\r\n"
                                                                   "Content-Type: application/json\r\n"
                                                                   "Content-Length: 0\r\n"
                                                                   "\r\n";

            send(client_socket, httpResponse.c_str(), httpResponse.length(), 0);
            // Remove header from received data
            char* start = strstr(rxBuf, "\r\n\r\n");
            if (start != NULL) {
                ret = string(start + 4);
            }
        }

        close(client_socket);

        return ret;
    }

private:
    int mSocket;
    struct sockaddr_in mAddress;
};

class AnalyticsTest : public L2TestMocks {
protected:
    virtual ~AnalyticsTest() override;

public:
    AnalyticsTest();
};

AnalyticsTest::AnalyticsTest()
    : L2TestMocks()
{
    Core::JSONRPC::Message message;
    string response;
    uint32_t status = Core::ERROR_GENERAL;

    // Activate plugin in constructor
    status = ActivateService("org.rdk.PersistentStore");
    EXPECT_EQ(Core::ERROR_NONE, status);
    status = ActivateService("org.rdk.System");
    EXPECT_EQ(Core::ERROR_NONE, status);

    JsonObject paramsJson;
    JsonObject resultJson;

    // Set PersistentStore values required by Sift
    paramsJson["namespace"] = "Analytics";
    paramsJson["key"] = "deviceType";
    paramsJson["value"] = "STB";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "modelNumber";
    paramsJson["value"] = "L2Test_STB";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "manufacturer";
    paramsJson["value"] = "L2Test_Manufacturer";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "serialNumber";
    paramsJson["value"] = "L2Test_SerialNumber";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "entertainmentOSVersion";
    paramsJson["value"] = "L2Test_EntertainmentOSVersion";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "deviceAppName";
    paramsJson["value"] = "L2Test_AppName";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "deviceAppVersion";
    paramsJson["value"] = "L2Test_AppVersion";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson.Clear();
    paramsJson["namespace"] = "accountProfile";
    paramsJson["key"] = "proposition";
    paramsJson["value"] = "L2Test_Proposition";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "retailer";
    paramsJson["value"] = "L2Test_Retailer";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "jvagent";
    paramsJson["value"] = "L2Test_JVAgent";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "coam";
    paramsJson["value"] = "true";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "accountType";
    paramsJson["value"] = "L2Test_AccountType";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "operator";
    paramsJson["value"] = "L2Test_Operator";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "detailType";
    paramsJson["value"] = "L2Test_DetailType";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    // Set SystemService fake values
    paramsJson.Clear();
    paramsJson["territory"] = "USA";
    paramsJson["region"] = "US-USA";
    status = InvokeServiceMethod("org.rdk.System", "setTerritory", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    // Mock System.getDeviceInfo by creating custom /lib/rdk/getDeviceDetails.sh
    std::ofstream file("/lib/rdk/getDeviceDetails.sh");
    file << "echo 'estb_mac=01:02:03:04:05:06'\n";
    file << "echo 'model_number=RDK'\n";
    file << "echo 'friendly_id=rdkglobal'\n";
    file.close();

    ON_CALL(*p_wrapsImplMock, v_secure_popen(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const char* direction, const char* command, va_list args) -> FILE* {
                const char* valueToReturn = NULL;
                va_list args2;
                va_copy(args2, args);
                char strFmt[256];
                vsnprintf(strFmt, sizeof(strFmt), command, args2);
                va_end(args2);
                if (strcmp(strFmt, "zdump -v America/New_York") == 0) {
                    valueToReturn = ZDUMP_MOCK_OUT;
                }
                if (valueToReturn != NULL) {
                    memset(gPipeBuffer, 0, sizeof(gPipeBuffer));
                    strcpy(gPipeBuffer, valueToReturn);
                    FILE* pipe = fmemopen(gPipeBuffer, strlen(gPipeBuffer), "r");
                    return pipe;
                } else {
                    return nullptr;
                }
            }));

    ON_CALL(*p_wrapsImplMock, time(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](time_t* arg) -> time_t {
                return TIME_MOCK_OUT;
            }));

    // Mock event mapping file
    std::ofstream eventsMapFile("/tmp/AnalyticsEventsMap.json");
    eventsMapFile << EVENTS_MAP;
    eventsMapFile.close();

    // Activate Analytics plugin
    status = ActivateService("org.rdk.Analytics");
    EXPECT_EQ(Core::ERROR_NONE, status);
}

AnalyticsTest::~AnalyticsTest()
{
    uint32_t status = Core::ERROR_GENERAL;

    ON_CALL(*p_rBusApiImplMock, rbus_close(::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    status = DeactivateService("org.rdk.Analytics");
    EXPECT_EQ(Core::ERROR_NONE, status);

    status = DeactivateService("org.rdk.System");
    EXPECT_EQ(Core::ERROR_NONE, status);

    status = DeactivateService("org.rdk.PersistentStore");
    EXPECT_EQ(Core::ERROR_NONE, status);

    sleep(5);
    int file_status = remove("/tmp/secure/persistent/rdkservicestore");
    // Check if the file has been successfully removed
    if (file_status != 0) {
        TEST_LOG("Error deleting file[/tmp/secure/persistent/rdkservicestore]");
    } else {
        TEST_LOG("File[/tmp/secure/persistent/rdkservicestore] successfully deleted");
    }

    file_status = remove("/tmp/AnalyticsSiftStore.db");
    // Check if the file has been successfully removed
    if (file_status != 0) {
        TEST_LOG("Error deleting file[/tmp/AnalyticsSiftStore.db]");
    } else {
        TEST_LOG("File[/tmp/AnalyticsSiftStore.db] successfully deleted");
    }

    file_status = remove("/opt/persistent/timeZoneDST");
    // Check if the file has been successfully removed
    if (file_status != 0) {
        TEST_LOG("Error deleting file[/opt/persistent/timeZoneDST]");
    } else {
        TEST_LOG("File[/opt/persistent/timeZoneDST] successfully deleted");
    }

    file_status = remove("/tmp/AnalyticsEventsMap.json");
    // Check if the file has been successfully removed
    if (file_status != 0) {
        TEST_LOG("Error deleting file[/tmp/AnalyticsEventsMap.json]");
    } else {
        TEST_LOG("File[/tmp/AnalyticsEventsMap.json] successfully deleted");
    }
}

TEST_F(AnalyticsTest, SendAndReceiveSignleEventQueued)
{
    JsonObject paramsJson;
    JsonObject resultJson;
    paramsJson["eventName"] = "L2TestEvent";
    paramsJson["eventVersion"] = "1";
    paramsJson["eventSource"] = "L2Test";
    paramsJson["eventSourceVersion"] = "1.0.0";
    JsonObject eventPayload;
    eventPayload["data"] = "random data";
    paramsJson["eventPayload"] = eventPayload;

    SiftServerMock siftServer;
    EXPECT_TRUE(siftServer.Start());

    uint32_t status = InvokeServiceMethod("org.rdk.Analytics", "sendEvent", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    // TimeZone not set, check if data will not come
    string eventMsg = siftServer.AwaitData(SIFT_SERVER_TIMEOUT_SEC);
    EXPECT_EQ(eventMsg, "");

    // Set TimeZone to FINAL what allows event to be decorated and sent to Sift server
    paramsJson.Clear();
    paramsJson["timeZone"] = "America/New_York";
    paramsJson["accuracy"] = "FINAL";
    status = InvokeServiceMethod("org.rdk.System", "setTimeZoneDST", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    eventMsg = siftServer.AwaitData(SIFT_SERVER_TIMEOUT_SEC);
    EXPECT_NE(eventMsg, "");

    // Check if the event message contains the expected fields
    JsonArray eventArray;
    eventArray.FromString(eventMsg);
    EXPECT_EQ(eventArray.Length(), 1);
    JsonObject eventObj = eventArray[0].Object();

    EXPECT_TRUE(eventObj.HasLabel("common_schema"));
    EXPECT_TRUE(eventObj.HasLabel("env"));
    EXPECT_TRUE(eventObj.HasLabel("product_name"));
    EXPECT_TRUE(eventObj.HasLabel("product_version"));
    EXPECT_TRUE(eventObj.HasLabel("event_schema"));
    EXPECT_TRUE(eventObj.HasLabel("event_name"));
    EXPECT_TRUE(eventObj.HasLabel("timestamp"));
    EXPECT_TRUE(eventObj.HasLabel("event_id"));
    EXPECT_TRUE(eventObj.HasLabel("event_source"));
    EXPECT_TRUE(eventObj.HasLabel("event_source_version"));
    EXPECT_TRUE(eventObj.HasLabel("logger_name"));
    EXPECT_TRUE(eventObj.HasLabel("logger_version"));
    EXPECT_TRUE(eventObj.HasLabel("partner_id"));
    EXPECT_TRUE(eventObj.HasLabel("device_model"));
    EXPECT_TRUE(eventObj.HasLabel("device_type"));
    EXPECT_TRUE(eventObj.HasLabel("device_timezone"));
    EXPECT_TRUE(eventObj.HasLabel("device_os_name"));
    EXPECT_TRUE(eventObj.HasLabel("device_os_version"));
    EXPECT_TRUE(eventObj.HasLabel("platform"));
    EXPECT_TRUE(eventObj.HasLabel("device_manufacturer"));
    EXPECT_TRUE(eventObj.HasLabel("authenticated"));
    EXPECT_TRUE(eventObj.HasLabel("session_id"));
    EXPECT_TRUE(eventObj.HasLabel("proposition"));
    EXPECT_TRUE(eventObj.HasLabel("retailer"));
    EXPECT_TRUE(eventObj.HasLabel("jv_agent"));
    EXPECT_TRUE(eventObj.HasLabel("coam"));
    EXPECT_TRUE(eventObj.HasLabel("device_serial_number"));
    EXPECT_TRUE(eventObj.HasLabel("device_mac_address"));
    EXPECT_TRUE(eventObj.HasLabel("country"));
    EXPECT_TRUE(eventObj.HasLabel("region"));
    EXPECT_TRUE(eventObj.HasLabel("account_type"));
    EXPECT_TRUE(eventObj.HasLabel("operator"));
    EXPECT_TRUE(eventObj.HasLabel("account_detail_type"));
    EXPECT_TRUE(eventObj.HasLabel("event_payload"));

    EXPECT_EQ(eventObj["common_schema"].String(), "entos/common/v1");
    EXPECT_EQ(eventObj["env"].String(), "prod");
    EXPECT_EQ(eventObj["product_name"].String(), "entos");
    EXPECT_EQ(eventObj["product_version"].String(), "L2Test_EntertainmentOSVersion");
    EXPECT_EQ(eventObj["event_schema"].String(), "entos/L2TestEvent/1");
    EXPECT_EQ(eventObj["event_name"].String(), "L2TestEvent");
    EXPECT_EQ(eventObj["event_source"].String(), "L2Test");
    EXPECT_EQ(eventObj["event_source_version"].String(), "1.0.0");
    EXPECT_EQ(eventObj["logger_name"].String(), "Analytics");
    EXPECT_EQ(eventObj["partner_id"].String(), "rdkglobal");
    EXPECT_EQ(eventObj["device_model"].String(), "RDK");
    EXPECT_EQ(eventObj["device_type"].String(), "STB");
    EXPECT_EQ(eventObj["device_timezone"].Number(), -14400000);
    EXPECT_EQ(eventObj["device_os_name"].String(), "rdk");
    EXPECT_EQ(eventObj["device_os_version"].String(), "L2Test_STB");
    EXPECT_EQ(eventObj["platform"].String(), "L2Test_Proposition");
    EXPECT_EQ(eventObj["device_manufacturer"].String(), "L2Test_Manufacturer");
    EXPECT_EQ(eventObj["proposition"].String(), "L2Test_Proposition");
    EXPECT_EQ(eventObj["retailer"].String(), "L2Test_Retailer");
    EXPECT_EQ(eventObj["jv_agent"].String(), "L2Test_JVAgent");
    EXPECT_EQ(eventObj["coam"].Boolean(), true);
    EXPECT_EQ(eventObj["device_serial_number"].String(), "L2Test_SerialNumber");
    EXPECT_EQ(eventObj["device_mac_address"].String(), "01:02:03:04:05:06");
    EXPECT_EQ(eventObj["country"].String(), "USA");
    EXPECT_EQ(eventObj["region"].String(), "US-USA");
    EXPECT_EQ(eventObj["account_type"].String(), "L2Test_AccountType");
    EXPECT_EQ(eventObj["operator"].String(), "L2Test_Operator");
    EXPECT_EQ(eventObj["account_detail_type"].String(), "L2Test_DetailType");

    JsonObject eventPayloadObj = eventObj["event_payload"].Object();
    EXPECT_TRUE(eventPayloadObj.HasLabel("data"));
    EXPECT_EQ(eventPayloadObj["data"].String(), "random data");
}

TEST_F(AnalyticsTest, SendAndReceiveMultipleEventsQueued)
{
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject paramsJson;
    JsonObject resultJson;
    paramsJson["eventName"] = "L2TestEvent";
    paramsJson["eventVersion"] = "1";
    paramsJson["eventSource"] = "L2Test";
    paramsJson["eventSourceVersion"] = "1.0.0";
    JsonObject eventPayload;
    eventPayload["data"] = "random data";
    paramsJson["eventPayload"] = eventPayload;

    SiftServerMock siftServer;
    EXPECT_TRUE(siftServer.Start());

    // Sift Uploader should send up to 10 events per POST by default
    const int eventsRcvMaxDfl = 10;
    const int eventsSentNbr = 2 * eventsRcvMaxDfl;

    for (int i = 0; i < eventsSentNbr; ++i) {
        status = InvokeServiceMethod("org.rdk.Analytics", "sendEvent", paramsJson, resultJson);
        EXPECT_EQ(status, Core::ERROR_NONE);
    }

    // TimeZone not set, check if data will not come
    string eventMsg = siftServer.AwaitData(SIFT_SERVER_TIMEOUT_SEC);
    EXPECT_EQ(eventMsg, "");

    // Set TimeZone to FINAL what allows event to be decorated and sent to Sift server
    paramsJson.Clear();
    paramsJson["timeZone"] = "America/New_York";
    paramsJson["accuracy"] = "FINAL";
    status = InvokeServiceMethod("org.rdk.System", "setTimeZoneDST", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    for (int i = 0; i < eventsSentNbr; i += eventsRcvMaxDfl) {
        string eventMsg = siftServer.AwaitData(SIFT_SERVER_TIMEOUT_SEC);
        EXPECT_NE(eventMsg, "");

        // Check if the event message contains the expected fields
        JsonArray eventArray;
        eventArray.FromString(eventMsg);
        EXPECT_EQ(eventArray.Length(), eventsRcvMaxDfl);

        for (int n = 0; n < eventsRcvMaxDfl; ++n) {
            JsonObject eventObj = eventArray[n].Object();

            EXPECT_TRUE(eventObj.HasLabel("common_schema"));
            EXPECT_TRUE(eventObj.HasLabel("env"));
            EXPECT_TRUE(eventObj.HasLabel("product_name"));
            EXPECT_TRUE(eventObj.HasLabel("product_version"));
            EXPECT_TRUE(eventObj.HasLabel("event_schema"));
            EXPECT_TRUE(eventObj.HasLabel("event_name"));
            EXPECT_TRUE(eventObj.HasLabel("timestamp"));
            EXPECT_TRUE(eventObj.HasLabel("event_id"));
            EXPECT_TRUE(eventObj.HasLabel("event_source"));
            EXPECT_TRUE(eventObj.HasLabel("event_source_version"));
            EXPECT_TRUE(eventObj.HasLabel("logger_name"));
            EXPECT_TRUE(eventObj.HasLabel("logger_version"));
            EXPECT_TRUE(eventObj.HasLabel("partner_id"));
            EXPECT_TRUE(eventObj.HasLabel("device_model"));
            EXPECT_TRUE(eventObj.HasLabel("device_type"));
            EXPECT_TRUE(eventObj.HasLabel("device_timezone"));
            EXPECT_TRUE(eventObj.HasLabel("device_os_name"));
            EXPECT_TRUE(eventObj.HasLabel("device_os_version"));
            EXPECT_TRUE(eventObj.HasLabel("platform"));
            EXPECT_TRUE(eventObj.HasLabel("device_manufacturer"));
            EXPECT_TRUE(eventObj.HasLabel("authenticated"));
            EXPECT_TRUE(eventObj.HasLabel("session_id"));
            EXPECT_TRUE(eventObj.HasLabel("proposition"));
            EXPECT_TRUE(eventObj.HasLabel("retailer"));
            EXPECT_TRUE(eventObj.HasLabel("jv_agent"));
            EXPECT_TRUE(eventObj.HasLabel("coam"));
            EXPECT_TRUE(eventObj.HasLabel("device_serial_number"));
            EXPECT_TRUE(eventObj.HasLabel("device_mac_address"));
            EXPECT_TRUE(eventObj.HasLabel("country"));
            EXPECT_TRUE(eventObj.HasLabel("region"));
            EXPECT_TRUE(eventObj.HasLabel("account_type"));
            EXPECT_TRUE(eventObj.HasLabel("operator"));
            EXPECT_TRUE(eventObj.HasLabel("account_detail_type"));
            EXPECT_TRUE(eventObj.HasLabel("event_payload"));

            EXPECT_EQ(eventObj["common_schema"].String(), "entos/common/v1");
            EXPECT_EQ(eventObj["env"].String(), "prod");
            EXPECT_EQ(eventObj["product_name"].String(), "entos");
            EXPECT_EQ(eventObj["product_version"].String(), "L2Test_EntertainmentOSVersion");
            EXPECT_EQ(eventObj["event_schema"].String(), "entos/L2TestEvent/1");
            EXPECT_EQ(eventObj["event_name"].String(), "L2TestEvent");
            EXPECT_EQ(eventObj["event_source"].String(), "L2Test");
            EXPECT_EQ(eventObj["event_source_version"].String(), "1.0.0");
            EXPECT_EQ(eventObj["logger_name"].String(), "Analytics");
            EXPECT_EQ(eventObj["partner_id"].String(), "rdkglobal");
            EXPECT_EQ(eventObj["device_model"].String(), "RDK");
            EXPECT_EQ(eventObj["device_type"].String(), "STB");
            EXPECT_EQ(eventObj["device_timezone"].Number(), -14400000);
            EXPECT_EQ(eventObj["device_os_name"].String(), "rdk");
            EXPECT_EQ(eventObj["device_os_version"].String(), "L2Test_STB");
            EXPECT_EQ(eventObj["platform"].String(), "L2Test_Proposition");
            EXPECT_EQ(eventObj["device_manufacturer"].String(), "L2Test_Manufacturer");
            EXPECT_EQ(eventObj["proposition"].String(), "L2Test_Proposition");
            EXPECT_EQ(eventObj["retailer"].String(), "L2Test_Retailer");
            EXPECT_EQ(eventObj["jv_agent"].String(), "L2Test_JVAgent");
            EXPECT_EQ(eventObj["coam"].Boolean(), true);
            EXPECT_EQ(eventObj["device_serial_number"].String(), "L2Test_SerialNumber");
            EXPECT_EQ(eventObj["device_mac_address"].String(), "01:02:03:04:05:06");
            EXPECT_EQ(eventObj["country"].String(), "USA");
            EXPECT_EQ(eventObj["region"].String(), "US-USA");
            EXPECT_EQ(eventObj["account_type"].String(), "L2Test_AccountType");
            EXPECT_EQ(eventObj["operator"].String(), "L2Test_Operator");
            EXPECT_EQ(eventObj["account_detail_type"].String(), "L2Test_DetailType");

            JsonObject eventPayloadObj = eventObj["event_payload"].Object();
            EXPECT_TRUE(eventPayloadObj.HasLabel("data"));
            EXPECT_EQ(eventPayloadObj["data"].String(), "random data");
        }
    }
}

TEST_F(AnalyticsTest, SendAndReceiveMultipleEventsTimeOk)
{
    // Set TimeZone to FINAL what allows event to be decorated and sent to Sift server
    JsonObject paramsJson;
    JsonObject resultJson;
    paramsJson["timeZone"] = "America/New_York";
    paramsJson["accuracy"] = "FINAL";
    uint32_t status = InvokeServiceMethod("org.rdk.System", "setTimeZoneDST", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    sleep(3);

    paramsJson.Clear();
    paramsJson["eventName"] = "L2TestEvent";
    paramsJson["eventVersion"] = "1";
    paramsJson["eventSource"] = "L2Test";
    paramsJson["eventSourceVersion"] = "1.0.0";
    JsonObject eventPayload;
    eventPayload["data"] = "random data";
    paramsJson["eventPayload"] = eventPayload;

    SiftServerMock siftServer;
    EXPECT_TRUE(siftServer.Start());

    const int eventsSentNbr = 6;

    for (int i = 0; i < eventsSentNbr; ++i) {
        status = InvokeServiceMethod("org.rdk.Analytics", "sendEvent", paramsJson, resultJson);
        EXPECT_EQ(status, Core::ERROR_NONE);
    }

    string eventMsg = siftServer.AwaitData(SIFT_SERVER_TIMEOUT_SEC);
    EXPECT_NE(eventMsg, "");

    // Check if the event message contains the expected fields
    JsonArray eventArray;
    eventArray.FromString(eventMsg);
    EXPECT_EQ(eventArray.Length(), eventsSentNbr);

    for (int n = 0; n < eventsSentNbr; ++n) {
        JsonObject eventObj = eventArray[n].Object();

        EXPECT_TRUE(eventObj.HasLabel("common_schema"));
        EXPECT_TRUE(eventObj.HasLabel("env"));
        EXPECT_TRUE(eventObj.HasLabel("product_name"));
        EXPECT_TRUE(eventObj.HasLabel("product_version"));
        EXPECT_TRUE(eventObj.HasLabel("event_schema"));
        EXPECT_TRUE(eventObj.HasLabel("event_name"));
        EXPECT_TRUE(eventObj.HasLabel("timestamp"));
        EXPECT_TRUE(eventObj.HasLabel("event_id"));
        EXPECT_TRUE(eventObj.HasLabel("event_source"));
        EXPECT_TRUE(eventObj.HasLabel("event_source_version"));
        EXPECT_TRUE(eventObj.HasLabel("logger_name"));
        EXPECT_TRUE(eventObj.HasLabel("logger_version"));
        EXPECT_TRUE(eventObj.HasLabel("partner_id"));
        EXPECT_TRUE(eventObj.HasLabel("device_model"));
        EXPECT_TRUE(eventObj.HasLabel("device_type"));
        EXPECT_TRUE(eventObj.HasLabel("device_timezone"));
        EXPECT_TRUE(eventObj.HasLabel("device_os_name"));
        EXPECT_TRUE(eventObj.HasLabel("device_os_version"));
        EXPECT_TRUE(eventObj.HasLabel("platform"));
        EXPECT_TRUE(eventObj.HasLabel("device_manufacturer"));
        EXPECT_TRUE(eventObj.HasLabel("authenticated"));
        EXPECT_TRUE(eventObj.HasLabel("session_id"));
        EXPECT_TRUE(eventObj.HasLabel("proposition"));
        EXPECT_TRUE(eventObj.HasLabel("retailer"));
        EXPECT_TRUE(eventObj.HasLabel("jv_agent"));
        EXPECT_TRUE(eventObj.HasLabel("coam"));
        EXPECT_TRUE(eventObj.HasLabel("device_serial_number"));
        EXPECT_TRUE(eventObj.HasLabel("device_mac_address"));
        EXPECT_TRUE(eventObj.HasLabel("country"));
        EXPECT_TRUE(eventObj.HasLabel("region"));
        EXPECT_TRUE(eventObj.HasLabel("account_type"));
        EXPECT_TRUE(eventObj.HasLabel("operator"));
        EXPECT_TRUE(eventObj.HasLabel("account_detail_type"));
        EXPECT_TRUE(eventObj.HasLabel("event_payload"));

        EXPECT_EQ(eventObj["common_schema"].String(), "entos/common/v1");
        EXPECT_EQ(eventObj["env"].String(), "prod");
        EXPECT_EQ(eventObj["product_name"].String(), "entos");
        EXPECT_EQ(eventObj["product_version"].String(), "L2Test_EntertainmentOSVersion");
        EXPECT_EQ(eventObj["event_schema"].String(), "entos/L2TestEvent/1");
        EXPECT_EQ(eventObj["event_name"].String(), "L2TestEvent");
        EXPECT_EQ(eventObj["event_source"].String(), "L2Test");
        EXPECT_EQ(eventObj["event_source_version"].String(), "1.0.0");
        EXPECT_EQ(eventObj["logger_name"].String(), "Analytics");
        EXPECT_EQ(eventObj["partner_id"].String(), "rdkglobal");
        EXPECT_EQ(eventObj["device_model"].String(), "RDK");
        EXPECT_EQ(eventObj["device_type"].String(), "STB");
        EXPECT_EQ(eventObj["device_timezone"].Number(), -14400000);
        EXPECT_EQ(eventObj["device_os_name"].String(), "rdk");
        EXPECT_EQ(eventObj["device_os_version"].String(), "L2Test_STB");
        EXPECT_EQ(eventObj["platform"].String(), "L2Test_Proposition");
        EXPECT_EQ(eventObj["device_manufacturer"].String(), "L2Test_Manufacturer");
        EXPECT_EQ(eventObj["proposition"].String(), "L2Test_Proposition");
        EXPECT_EQ(eventObj["retailer"].String(), "L2Test_Retailer");
        EXPECT_EQ(eventObj["jv_agent"].String(), "L2Test_JVAgent");
        EXPECT_EQ(eventObj["coam"].Boolean(), true);
        EXPECT_EQ(eventObj["device_serial_number"].String(), "L2Test_SerialNumber");
        EXPECT_EQ(eventObj["device_mac_address"].String(), "01:02:03:04:05:06");
        EXPECT_EQ(eventObj["country"].String(), "USA");
        EXPECT_EQ(eventObj["region"].String(), "US-USA");
        EXPECT_EQ(eventObj["account_type"].String(), "L2Test_AccountType");
        EXPECT_EQ(eventObj["operator"].String(), "L2Test_Operator");
        EXPECT_EQ(eventObj["account_detail_type"].String(), "L2Test_DetailType");

        JsonObject eventPayloadObj = eventObj["event_payload"].Object();
        EXPECT_TRUE(eventPayloadObj.HasLabel("data"));
        EXPECT_EQ(eventPayloadObj["data"].String(), "random data");
    }
}

TEST_F(AnalyticsTest, OnServer400Error)
{
    // Set TimeZone to FINAL what allows event to be decorated and sent to Sift server
    JsonObject paramsJson;
    JsonObject resultJson;
    paramsJson["timeZone"] = "America/New_York";
    paramsJson["accuracy"] = "FINAL";
    uint32_t status = InvokeServiceMethod("org.rdk.System", "setTimeZoneDST", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson.Clear();
    paramsJson["eventName"] = "L2TestEvent";
    paramsJson["eventVersion"] = "1";
    paramsJson["eventSource"] = "L2Test";
    paramsJson["eventSourceVersion"] = "1.0.0";
    JsonObject eventPayload;
    eventPayload["data"] = "random data";
    paramsJson["eventPayload"] = eventPayload;

    SiftServerMock siftServer;
    EXPECT_TRUE(siftServer.Start());

    status = InvokeServiceMethod("org.rdk.Analytics", "sendEvent", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    string eventMsg = siftServer.AwaitData(SIFT_SERVER_TIMEOUT_SEC, "400 Error");
    EXPECT_NE(eventMsg, "");

    // On Err 400 events should be deleted
    eventMsg = siftServer.AwaitData(SIFT_SERVER_TIMEOUT_SEC);
    EXPECT_EQ(eventMsg, "");
}

TEST_F(AnalyticsTest, OnServer500Error)
{
    // Set TimeZone to FINAL what allows event to be decorated and sent to Sift server
    JsonObject paramsJson;
    JsonObject resultJson;
    paramsJson["timeZone"] = "America/New_York";
    paramsJson["accuracy"] = "FINAL";
    uint32_t status = InvokeServiceMethod("org.rdk.System", "setTimeZoneDST", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson.Clear();
    paramsJson["eventName"] = "L2TestEvent";
    paramsJson["eventVersion"] = "1";
    paramsJson["eventSource"] = "L2Test";
    paramsJson["eventSourceVersion"] = "1.0.0";
    JsonObject eventPayload;
    eventPayload["data"] = "random data";
    paramsJson["eventPayload"] = eventPayload;

    SiftServerMock siftServer;
    EXPECT_TRUE(siftServer.Start());

    status = InvokeServiceMethod("org.rdk.Analytics", "sendEvent", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    string eventMsg = siftServer.AwaitData(SIFT_SERVER_TIMEOUT_SEC, "500 Error");
    EXPECT_NE(eventMsg, "");

    // On Err 500 events should be resent
    string eventMsgRep = siftServer.AwaitData(SIFT_SERVER_TIMEOUT_SEC);
    EXPECT_EQ(eventMsgRep, eventMsg);
}

TEST_F(AnalyticsTest, OnPersistentStoreValChange)
{
    // Set TimeZone to FINAL what allows event to be decorated and sent to Sift server
    JsonObject paramsJson;
    JsonObject resultJson;

    paramsJson["timeZone"] = "America/New_York";
    paramsJson["accuracy"] = "FINAL";
    uint32_t status = InvokeServiceMethod("org.rdk.System", "setTimeZoneDST", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    // Wait so the initial Persistent Store val are red
    sleep(5);

    paramsJson.Clear();

    // update PersistentStore values
    paramsJson["namespace"] = "Analytics";
    paramsJson["key"] = "deviceType";
    paramsJson["value"] = "STB2";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "modelNumber";
    paramsJson["value"] = "L2Test_STB2";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "manufacturer";
    paramsJson["value"] = "L2Test_Manufacturer2";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "serialNumber";
    paramsJson["value"] = "L2Test_SerialNumber2";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "entertainmentOSVersion";
    paramsJson["value"] = "L2Test_EntertainmentOSVersion2";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "deviceAppName";
    paramsJson["value"] = "L2Test_AppName2";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "deviceAppVersion";
    paramsJson["value"] = "L2Test_AppVersion2";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson.Clear();
    paramsJson["namespace"] = "accountProfile";
    paramsJson["key"] = "proposition";
    paramsJson["value"] = "L2Test_Proposition2";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "retailer";
    paramsJson["value"] = "L2Test_Retailer2";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "jvagent";
    paramsJson["value"] = "L2Test_JVAgent2";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "coam";
    paramsJson["value"] = "false";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "accountType";
    paramsJson["value"] = "L2Test_AccountType2";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "operator";
    paramsJson["value"] = "L2Test_Operator2";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson["key"] = "detailType";
    paramsJson["value"] = "L2Test_DetailType2";
    status = InvokeServiceMethod("org.rdk.PersistentStore", "setValue", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    paramsJson.Clear();
    paramsJson["eventName"] = "L2TestEvent";
    paramsJson["eventVersion"] = "1";
    paramsJson["eventSource"] = "L2Test";
    paramsJson["eventSourceVersion"] = "1.0.0";
    JsonObject eventPayload;
    eventPayload["data"] = "random data";
    paramsJson["eventPayload"] = eventPayload;

    SiftServerMock siftServer;
    EXPECT_TRUE(siftServer.Start());

    status = InvokeServiceMethod("org.rdk.Analytics", "sendEvent", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    string eventMsg = siftServer.AwaitData(SIFT_SERVER_TIMEOUT_SEC);
    EXPECT_NE(eventMsg, "");

    // Check if the event message contains the expected fields
    JsonArray eventArray;
    eventArray.FromString(eventMsg);
    EXPECT_EQ(eventArray.Length(), 1);
    JsonObject eventObj = eventArray[0].Object();

    EXPECT_TRUE(eventObj.HasLabel("common_schema"));
    EXPECT_TRUE(eventObj.HasLabel("env"));
    EXPECT_TRUE(eventObj.HasLabel("product_name"));
    EXPECT_TRUE(eventObj.HasLabel("product_version"));
    EXPECT_TRUE(eventObj.HasLabel("event_schema"));
    EXPECT_TRUE(eventObj.HasLabel("event_name"));
    EXPECT_TRUE(eventObj.HasLabel("timestamp"));
    EXPECT_TRUE(eventObj.HasLabel("event_id"));
    EXPECT_TRUE(eventObj.HasLabel("event_source"));
    EXPECT_TRUE(eventObj.HasLabel("event_source_version"));
    EXPECT_TRUE(eventObj.HasLabel("logger_name"));
    EXPECT_TRUE(eventObj.HasLabel("logger_version"));
    EXPECT_TRUE(eventObj.HasLabel("partner_id"));
    EXPECT_TRUE(eventObj.HasLabel("device_model"));
    EXPECT_TRUE(eventObj.HasLabel("device_type"));
    EXPECT_TRUE(eventObj.HasLabel("device_timezone"));
    EXPECT_TRUE(eventObj.HasLabel("device_os_name"));
    EXPECT_TRUE(eventObj.HasLabel("device_os_version"));
    EXPECT_TRUE(eventObj.HasLabel("platform"));
    EXPECT_TRUE(eventObj.HasLabel("device_manufacturer"));
    EXPECT_TRUE(eventObj.HasLabel("authenticated"));
    EXPECT_TRUE(eventObj.HasLabel("session_id"));
    EXPECT_TRUE(eventObj.HasLabel("proposition"));
    EXPECT_TRUE(eventObj.HasLabel("retailer"));
    EXPECT_TRUE(eventObj.HasLabel("jv_agent"));
    EXPECT_TRUE(eventObj.HasLabel("coam"));
    EXPECT_TRUE(eventObj.HasLabel("device_serial_number"));
    EXPECT_TRUE(eventObj.HasLabel("device_mac_address"));
    EXPECT_TRUE(eventObj.HasLabel("country"));
    EXPECT_TRUE(eventObj.HasLabel("region"));
    EXPECT_TRUE(eventObj.HasLabel("account_type"));
    EXPECT_TRUE(eventObj.HasLabel("operator"));
    EXPECT_TRUE(eventObj.HasLabel("account_detail_type"));
    EXPECT_TRUE(eventObj.HasLabel("event_payload"));

    EXPECT_EQ(eventObj["common_schema"].String(), "entos/common/v1");
    EXPECT_EQ(eventObj["env"].String(), "prod");
    EXPECT_EQ(eventObj["product_name"].String(), "entos");
    EXPECT_EQ(eventObj["product_version"].String(), "L2Test_EntertainmentOSVersion2");
    EXPECT_EQ(eventObj["event_schema"].String(), "entos/L2TestEvent/1");
    EXPECT_EQ(eventObj["event_name"].String(), "L2TestEvent");
    EXPECT_EQ(eventObj["event_source"].String(), "L2Test");
    EXPECT_EQ(eventObj["event_source_version"].String(), "1.0.0");
    EXPECT_EQ(eventObj["logger_name"].String(), "Analytics");
    EXPECT_EQ(eventObj["partner_id"].String(), "rdkglobal");
    EXPECT_EQ(eventObj["device_model"].String(), "RDK");
    EXPECT_EQ(eventObj["device_type"].String(), "STB2");
    EXPECT_EQ(eventObj["device_timezone"].Number(), -14400000);
    EXPECT_EQ(eventObj["device_os_name"].String(), "rdk");
    EXPECT_EQ(eventObj["device_os_version"].String(), "L2Test_STB2");
    EXPECT_EQ(eventObj["platform"].String(), "L2Test_Proposition2");
    EXPECT_EQ(eventObj["device_manufacturer"].String(), "L2Test_Manufacturer2");
    EXPECT_EQ(eventObj["proposition"].String(), "L2Test_Proposition2");
    EXPECT_EQ(eventObj["retailer"].String(), "L2Test_Retailer2");
    EXPECT_EQ(eventObj["jv_agent"].String(), "L2Test_JVAgent2");
    EXPECT_EQ(eventObj["coam"].Boolean(), false);
    EXPECT_EQ(eventObj["device_serial_number"].String(), "L2Test_SerialNumber2");
    EXPECT_EQ(eventObj["device_mac_address"].String(), "01:02:03:04:05:06");
    EXPECT_EQ(eventObj["country"].String(), "USA");
    EXPECT_EQ(eventObj["region"].String(), "US-USA");
    EXPECT_EQ(eventObj["account_type"].String(), "L2Test_AccountType2");
    EXPECT_EQ(eventObj["operator"].String(), "L2Test_Operator2");
    EXPECT_EQ(eventObj["account_detail_type"].String(), "L2Test_DetailType2");

    JsonObject eventPayloadObj = eventObj["event_payload"].Object();
    EXPECT_TRUE(eventPayloadObj.HasLabel("data"));
    EXPECT_EQ(eventPayloadObj["data"].String(), "random data");
}


TEST_F(AnalyticsTest, EventsMapping)
{
    JsonObject paramsJson;
    JsonObject resultJson;

    // exact mapping
    paramsJson["eventName"] = "L2MapTestEvent";
    paramsJson["eventVersion"] = "1";
    paramsJson["eventSource"] = "L2Test";
    paramsJson["eventSourceVersion"] = "1.0.0";
    
    JsonObject eventPayload;
    eventPayload["data"] = "random data";
    paramsJson["eventPayload"] = eventPayload;

    uint32_t status = InvokeServiceMethod("org.rdk.Analytics", "sendEvent", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    // generic mapping
    paramsJson["eventName"] = "L2MapTestEvent";
    paramsJson["eventVersion"] = "2";
    paramsJson["eventSource"] = "L2Test";
    paramsJson["eventSourceVersion"] = "1.0.1";
    status = InvokeServiceMethod("org.rdk.Analytics", "sendEvent", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    // generic mapping by source version
    paramsJson["eventName"] = "L2MapTestEvent";
    paramsJson["eventVersion"] = "2";
    paramsJson["eventSource"] = "L2Test";
    paramsJson["eventSourceVersion"] = "1.0.0";
    status = InvokeServiceMethod("org.rdk.Analytics", "sendEvent", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    // generic mapping by event version
    paramsJson["eventName"] = "L2MapTestEvent";
    paramsJson["eventVersion"] = "1";
    paramsJson["eventSource"] = "L2Test";
    paramsJson["eventSourceVersion"] = "1.0.1";
    status = InvokeServiceMethod("org.rdk.Analytics", "sendEvent", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);


    SiftServerMock siftServer;
    EXPECT_TRUE(siftServer.Start());

    // Set TimeZone to FINAL what allows event to be decorated and sent to Sift server
    paramsJson.Clear();
    paramsJson["timeZone"] = "America/New_York";
    paramsJson["accuracy"] = "FINAL";
    status = InvokeServiceMethod("org.rdk.System", "setTimeZoneDST", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    string eventsMsg = siftServer.AwaitData(SIFT_SERVER_TIMEOUT_SEC);
    EXPECT_NE(eventsMsg, "");

    // Check if the event message contains the expected fields
    JsonArray eventArray;
    eventArray.FromString(eventsMsg);
    EXPECT_EQ(eventArray.Length(), 4);

    if (eventArray.Length() == 4) {
        JsonObject eventObj = eventArray[0].Object();
        EXPECT_TRUE(eventObj.HasLabel("event_name"));
        EXPECT_EQ(eventObj["event_name"].String(), "L2TestEventMappedExact");

        eventObj = eventArray[1].Object();
        EXPECT_TRUE(eventObj.HasLabel("event_name"));
        EXPECT_EQ(eventObj["event_name"].String(), "L2TestEventMappedGeneric");

        eventObj = eventArray[2].Object();
        EXPECT_TRUE(eventObj.HasLabel("event_name"));
        EXPECT_EQ(eventObj["event_name"].String(), "L2TestEventMappedGenericVersion");

        eventObj = eventArray[3].Object();
        EXPECT_TRUE(eventObj.HasLabel("event_name"));
        EXPECT_EQ(eventObj["event_name"].String(), "L2TestEventMappedGenericSourceVersion");
    }

}