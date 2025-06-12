#include "L2Tests.h"
#include "L2TestsMock.h"
#include <condition_variable>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
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

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define SERVER_TIMEOUT_SEC (30)

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

class ServerMock {
public:
    ServerMock()
        : mSocket(-1)
    {
        memset(&mAddress, 0, sizeof(mAddress));
    }

    ~ServerMock()
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
        mAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
        mAddress.sin_port = htons(SERVER_PORT);

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

extern "C" int __real_pclose(FILE* pipe);

AnalyticsTest::AnalyticsTest()
    : L2TestMocks()
{
    Core::JSONRPC::Message message;
    string response;
    uint32_t status = Core::ERROR_GENERAL;

    // Activate plugin in constructor
    status = ActivateService("org.rdk.System");
    EXPECT_EQ(Core::ERROR_NONE, status);

    JsonObject paramsJson;
    JsonObject resultJson;


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


    sleep(5);

    int file_status = remove("/tmp/AnalyticsStore.db");
    // Check if the file has been successfully removed
    if (file_status != 0) {
        TEST_LOG("Error deleting file[/tmp/AnalyticsStore.db]");
    } else {
        TEST_LOG("File[/tmp/AnalyticsStore.db] successfully deleted");
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

    ServerMock server;
    EXPECT_TRUE(server.Start());

    uint32_t status = InvokeServiceMethod("org.rdk.Analytics", "sendEvent", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    // TimeZone not set, check if data will not come
    string eventMsg = server.AwaitData(SERVER_TIMEOUT_SEC);
    EXPECT_EQ(eventMsg, "");

    // Set TimeZone to FINAL what allows event to be decorated and sent to server
    paramsJson.Clear();
    paramsJson["timeZone"] = "America/New_York";
    paramsJson["accuracy"] = "FINAL";
    status = InvokeServiceMethod("org.rdk.System", "setTimeZoneDST", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);

    eventMsg = server.AwaitData(SERVER_TIMEOUT_SEC);
    EXPECT_NE(eventMsg, "");

    // Check if the event message contains the expected fields
    JsonArray eventArray;
    eventArray.FromString(eventMsg);
    EXPECT_EQ(eventArray.Length(), 1);
    JsonObject eventObj = eventArray[0].Object();
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

    // Set TimeZone to FINAL what allows event to be decorated and sent to server
    paramsJson.Clear();
    paramsJson["timeZone"] = "America/New_York";
    paramsJson["accuracy"] = "FINAL";
    status = InvokeServiceMethod("org.rdk.System", "setTimeZoneDST", paramsJson, resultJson);
    EXPECT_EQ(status, Core::ERROR_NONE);


    ServerMock server;
    EXPECT_TRUE(server.Start());

    string eventsMsg = server.AwaitData(SERVER_TIMEOUT_SEC);

    // Check if the event message contains the expected fields
    JsonArray eventArray;
    eventArray.FromString(eventsMsg);
    int retry = 3;
    while (eventArray.Length() < 4 && retry-- > 0) {
        // If not all events are received, wait for the rest
        string eventsMsg2 = server.AwaitData(SERVER_TIMEOUT_SEC);
        JsonArray eventArray2;
        eventArray2.FromString(eventsMsg2);
        for (int i = 0; i < eventArray2.Length(); ++i) {
            eventArray.Add(eventArray2[i]);
        }
    }
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