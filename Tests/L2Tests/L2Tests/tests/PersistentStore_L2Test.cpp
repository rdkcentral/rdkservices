#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>
#include "readprocMockInterface.h"
#include <string>
#include <functional>

#define JSON_TIMEOUT   (1000)
#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);
#define PERSISTENTSTORE_CALLSIGN  _T("org.rdk.PersistentStore.1")
#define PERSISTENTL2TEST_CALLSIGN _T("L2tests.1")

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;

typedef enum : uint32_t {
    PERSISTENTSTOREL2TEST_ONVALUE_CHANGE = 0x00000001,
    PERSISTENTSTOREL2TEST_ONSTORAGE_EXCEEDED=0x00000002,
    PERSISTENTSTOREL2TEST_STATE_INVALID = 0x00000000
}PersistentStoreL2test_async_events_t;


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
 * @brief Internal test mock class
 *
 * Note that this is for internal test use only and doesn't mock any actual
 * concrete interface.
 */

class AsyncHandlerMock_PStore
{
    public:
		AsyncHandlerMock_PStore()
        {
        }
	    MOCK_METHOD(void, onValueChanged, (const JsonObject &message));
		MOCK_METHOD(void, onStorageExceeded, (const JsonObject &message));
        
};

/* PersistentStore L2 test class declaration */

class PersistentStore_L2Test : public L2TestMocks {
protected:
    Core::JSONRPC::Message message;
    string response; 
    
    virtual ~PersistentStore_L2Test() override;

    public:
        PersistentStore_L2Test();
        void onValueChanged(const JsonObject &message);
        void onStorageExceeded(const JsonObject &message);
        
        uint32_t WaitForRequestStatus(uint32_t timeout_ms,PersistentStoreL2test_async_events_t expected_status);

    private:
        /** @brief Mutex */
        std::mutex m_mutex;
        
        /** @brief Condition variable */
        std::condition_variable m_condition_variable;

        /** @brief Event signalled flag */
        uint32_t m_event_signalled;

};

/**
 * @brief Constructor for PersistentStore L2 test class
 */
PersistentStore_L2Test::PersistentStore_L2Test()
        : L2TestMocks()
{
        TEST_LOG("*********************PersistentStore Constructor**************\n");
        uint32_t status = Core::ERROR_GENERAL;
        m_event_signalled = PERSISTENTSTOREL2TEST_STATE_INVALID;
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
    m_event_signalled = PERSISTENTSTOREL2TEST_STATE_INVALID;
    
    status = DeactivateService("org.rdk.PersistentStore");
    EXPECT_EQ(Core::ERROR_NONE, status);
}

/**
 * @brief waits for various status change on asynchronous calls
 *
 * @param[in] timeout_ms timeout for waiting
 */

uint32_t PersistentStore_L2Test::WaitForRequestStatus(uint32_t timeout_ms, PersistentStoreL2test_async_events_t expected_status)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto now = std::chrono::system_clock::now();
    std::chrono::milliseconds timeout(timeout_ms);
    uint32_t signalled = PERSISTENTSTOREL2TEST_STATE_INVALID;

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
    
    /* Notify the requester thread. */

    m_event_signalled |= PERSISTENTSTOREL2TEST_ONVALUE_CHANGE;
    m_condition_variable.notify_one();
}

/**
 * @brief called when onStorageExceeded
 * changed notification received
 *
 * @param[in] message from Persistent Store on the change
 */

void PersistentStore_L2Test::onStorageExceeded(const JsonObject &message)
{
    TEST_LOG("onStorageExceeded triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    std::string str;
    message.ToString(str);
    
    /* Notify the requester thread. */

    m_event_signalled |= PERSISTENTSTOREL2TEST_ONSTORAGE_EXCEEDED;
    m_condition_variable.notify_one();
}

/********************************************************
************Test case Details **************************
** 1. Get value from PersistentStore
** 2. Set value
** 3. On Value changed event triggered 
*******************************************************/

TEST_F(PersistentStore_L2Test, PersistentStoreOnValuechange)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(PERSISTENTSTORE_CALLSIGN, PERSISTENTL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_PStore> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    JsonObject expected_status;
    JsonObject params_get;
    uint32_t signalled = PERSISTENTSTOREL2TEST_STATE_INVALID;

    status = InvokeServiceMethod("org.rdk.PersistentStore.1", "getValue", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

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
    
    signalled = WaitForRequestStatus(JSON_TIMEOUT,PERSISTENTSTOREL2TEST_ONVALUE_CHANGE);
    EXPECT_TRUE(signalled & PERSISTENTSTOREL2TEST_ONVALUE_CHANGE);

    // Unregister for events.
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onValueChanged"));
}

/********************************************************
************Test case Details **************************
** 1. Get value from PersistentStore
** 2. Set value
** 3. On Storage Exceeded event triggered when maxsize is exceeded
*******************************************************/

TEST_F(PersistentStore_L2Test, PersistentStoreStorageExceeded)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(PERSISTENTSTORE_CALLSIGN, PERSISTENTL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_PStore> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    JsonObject expected_status;
    uint32_t signalled = PERSISTENTSTOREL2TEST_STATE_INVALID;


    status = InvokeServiceMethod("org.rdk.PersistentStore.1", "getValue", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    // Subscribe to the "onStorageExceeded" event.
	status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                    _T("onStorageExceeded"),
                                     &AsyncHandlerMock_PStore::onStorageExceeded,
                                     &async_handler);
                                           
    EXPECT_EQ(Core::ERROR_NONE, status);

    message = "";
    expected_status.FromString(message);
    EXPECT_CALL(async_handler, onStorageExceeded(MatchRequestStatus(expected_status)))
        .WillOnce(Invoke(this, &PersistentStore_L2Test::onStorageExceeded));


    params["namespace"] = "namespace1";
    params["key"] = "key1";
    params["value"] = "12345678";
    
    //onStorageExceeded will get invoked when maxsize is exceeded
    status = InvokeServiceMethod("org.rdk.PersistentStore.1", "setValue", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

    //Deletes the specified namespace
    status = InvokeServiceMethod("org.rdk.PersistentStore.1", "deleteNamespace", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

    signalled = WaitForRequestStatus(JSON_TIMEOUT,PERSISTENTSTOREL2TEST_ONSTORAGE_EXCEEDED);
    EXPECT_TRUE(signalled & PERSISTENTSTOREL2TEST_ONSTORAGE_EXCEEDED);
 
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onStorageExceeded"));
}

/********************************************************
************Test case Details **************************
** 1. Set key value with specified namespace
** 2. Corrupt the file by adding some text data by file open and write
** 3. deactivate & activate service
** 4. Verify data corruption by getValue
*******************************************************/

TEST_F(PersistentStore_L2Test, PersistentStoreDataCorruption)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(PERSISTENTSTORE_CALLSIGN, PERSISTENTL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_PStore> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params, params_get;
    JsonObject result;

    status = InvokeServiceMethod("org.rdk.PersistentStore.1", "getValue", params, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());


    params["namespace"] = "ns3";
    params["key"] = "key3";
    params["value"] = "012";

    //Sets the value of a key in the the specified namespace
    status = InvokeServiceMethod("org.rdk.PersistentStore.1", "setValue", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    
    //Returns the value of a key from the specified namespace
    params_get["namespace"] = "ns3";
    params_get["key"] = "key3";
    status = InvokeServiceMethod("org.rdk.PersistentStore.1", "getValue", params_get, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());   
    EXPECT_STREQ("012", result["value"].String().c_str());

    //Corrupt the db file
    char str[] = "Hello PersistentStore";
    FILE *f;
    f =  fopen("/opt/secure/persistent/rdkservicestore" , "w");
        if(f !=nullptr){
        std::cout<<"File exists"<< std::endl;
        }
    fwrite(str , 1 , sizeof(str) , f);
    fclose(f);

    //Returns the value of a key from the specified namespace
    params_get["namespace"] = "ns3";
    params_get["key"] = "key3";
    status = InvokeServiceMethod("org.rdk.PersistentStore.1", "getValue", params_get, result);
    EXPECT_EQ(Core::ERROR_GENERAL, status);
    EXPECT_FALSE(result["success"].Boolean());
    EXPECT_STREQ("null", result["value"].String().c_str());

    // Delete the database file
     if (Core::File(string("/opt/secure/persistent/rdkservicestore")).Exists()) {

       EXPECT_TRUE(Core::File(string("/opt/secure/persistent/rdkservicestore")).Destroy());
       
    }
    EXPECT_FALSE(Core::File(string("/opt/secure/persistent/rdkservicestore")).Exists());
     
}
