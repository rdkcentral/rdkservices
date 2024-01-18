#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>
#include "readprocMockInterface.h"
#include "SqliteStore.h"
#include "PersistentStore.h"
#include <string>
#include<functional>
#include "ServiceMock.h"
#include "FactoriesImplementation.h"
#include "rdkshell.h"

#define JSON_TIMEOUT   (1000)
#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);
#define PERSISTENTSTORE_CALLSIGN  _T("org.rdk.PersistentStore.1")
#define PERSISTENTL2TEST_CALLSIGN _T("L2tests.1")

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;

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

class AsyncHandlerMock_PStore
{
    public:
		AsyncHandlerMock_PStore()
        {
        }	
		MOCK_METHOD(void, onValueChanged, (const JsonObject &message));
        
};
class PersistentStore_L2Test : public L2TestMocks {
protected:
    Core::JSONRPC::Message message;
    string response;

    virtual ~PersistentStore_L2Test() override;

public:
    PersistentStore_L2Test();
    void onValueChanged(const JsonObject &message);

private:
    /** @brief Mutex */
    std::mutex m_mutex;
};

/**
 * @brief Constructor for PersistentStore L2 test class
 */
PersistentStore_L2Test::PersistentStore_L2Test()
        : L2TestMocks()
{
    TEST_LOG("*********************PersistentStore Constructor**************\n");
    uint32_t status = Core::ERROR_GENERAL;
    Core::JSONRPC::Message message;

    /* Activate plugin in constructor */
    status = ActivateService("org.rdk.PersistentStore");
    EXPECT_EQ(Core::ERROR_NONE, status);
}

/**
 * @brief Destructor for PersistentStore L2 test class
 */
PersistentStore_L2Test::~PersistentStore_L2Test()
{
    TEST_LOG("************PersistentStore Destructor***********\n");
    uint32_t status = Core::ERROR_GENERAL;
    status = DeactivateService("org.rdk.PersistentStore");
    EXPECT_EQ(Core::ERROR_NONE, status);
}

/**
 * @brief called when onValueChanged
 * changed notification received
 *
 * @param[in] message from Persistent Store on the change
 */

void PersistentStore_L2Test::onValueChanged(const JsonObject &message)
{
    TEST_LOG("onValueChanged triggered ***\n");
	std::unique_lock<std::mutex> lock(m_mutex);
	std::string str;
	message.ToString(str);

    TEST_LOG("onValueChanged received: %s\n", str.c_str());
}

/********************************************************
************Test case Details **************************
** 1. Get value from PersistentStore
** 2. Set value
** 3. On Value changed event triggered 
*******************************************************/

TEST_F(PersistentStore_L2Test, PersistentStoregetSetValue)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(PERSISTENTSTORE_CALLSIGN, PERSISTENTL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_PStore> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params,params_get;
    JsonObject result;
    std::string message;
    JsonObject expected_status;

    status = InvokeServiceMethod("org.rdk.PersistentStore.1", "getValue", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());

    // Subscribe to the "onValueChanged" event.
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                   _T("onValueChanged"),
                                   &AsyncHandlerMock_PStore::onValueChanged,
                                   &async_handler);

    EXPECT_EQ(Core::ERROR_NONE, status);

	message = "{\"namespace\":\"ns1\",\"key\":\"key1\",\"value\":\"value1\"}";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onValueChanged(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &PersistentStore_L2Test::onValueChanged));

    // Test the setValue method.
    params["namespace"] = "ns1";
    params["key"] = "key1";
    params["value"] = "value1";

    //onValueChanged will get invoked after SetValue 
    status = InvokeServiceMethod("org.rdk.PersistentStore.1", "setValue", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_TRUE(result["success"].Boolean());

    params_get["namespace"] = "ns1";
    params_get["key"] = "key1";
    status = InvokeServiceMethod("org.rdk.PersistentStore.1", "getValue", params_get, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ("value1", result["value"].String().c_str());

    // Unregister for events.
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onValueChanged"));
}
