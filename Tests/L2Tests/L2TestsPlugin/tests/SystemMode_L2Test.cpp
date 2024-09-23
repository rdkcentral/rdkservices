#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <interfaces/ISystemMode.h>
#include "HdmiCec.h"

#define TEST_LOG(x, ...) fprintf( stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);

#define JSON_TIMEOUT   (1000)
#define SYSTEMMODE_CALLSIGN  _T("org.rdk.SystemMode")
#define SYSTEMMODEL2TEST_CALLSIGN _T("L2tests.1")

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;
using ::WPEFramework::Exchange::ISystemMode;


class AsyncHandlerMock
{
    public:
        AsyncHandlerMock()
        {
        }
};

class SystemModeTest : public L2TestMocks {
protected:
    virtual ~SystemModeTest() override;

    public:
    SystemModeTest();

    protected:
        /** @brief Pointer to the  IShell interface */
        PluginHost::IShell *m_controller_usersettings;

        /** @brief Pointer to the IUserSettings interface */
        //Exchange::IUserSettings *m_usersettingsplugin;
};

SystemModeTest:: SystemModeTest():L2TestMocks()
{
        Core::JSONRPC::Message message;
        string response;
        uint32_t status = Core::ERROR_GENERAL;

         /* Activate plugin in constructor */
         status = ActivateService("org.rdk.SystemMode");
         EXPECT_EQ(Core::ERROR_NONE, status);
}

/**
 * @brief Destructor for SystemServices L2 test class
 */
SystemModeTest::~SystemModeTest()
{
    uint32_t status = Core::ERROR_GENERAL;

    status = DeactivateService("org.rdk.SystemMode");
    EXPECT_EQ(Core::ERROR_NONE, status);
}

TEST_F(SystemModeTest,getStatedefault)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEMMODE_CALLSIGN,SYSTEMMODEL2TEST_CALLSIGN);
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    std::string reply;	

    params["systemMode"] = "device_optimize";
    status = InvokeServiceMethod("org.rdk.SystemMode", "getState", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    if (result.HasLabel("state")) {
	    EXPECT_STREQ("VIDEO", result["state"].String().c_str());
    }

}

TEST_F(SystemModeTest,getStateNegativeCase)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEMMODE_CALLSIGN,SYSTEMMODEL2TEST_CALLSIGN);
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    std::string reply;	
//Case 1 empty systemMode
    params["systemMode"] = "";
    status = InvokeServiceMethod("org.rdk.SystemMode", "getState", params, result);
    EXPECT_FALSE(result["success"].Boolean());
    if (result.HasLabel("error")) {
	    EXPECT_STREQ("{\"code\":1,\"message\":\"ERROR_GENERAL\"}", result["error"].String().c_str());
    }

//Case 2  Invalid systemMode
    params["systemMode"] = "abc";
    status = InvokeServiceMethod("org.rdk.SystemMode", "getState", params, result);
    EXPECT_FALSE(result["success"].Boolean());
    if (result.HasLabel("error")) {
	    EXPECT_STREQ("{\"code\":1,\"message\":\"ERROR_GENERAL\"}", result["error"].String().c_str());
    }

}

TEST_F(SystemModeTest,requestStateGame)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEMMODE_CALLSIGN,SYSTEMMODEL2TEST_CALLSIGN);
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    std::string reply;	

    params["systemMode"] = "device_optimize";    
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
	    .Times(::testing::AnyNumber())
	    .WillRepeatedly(
			    [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
			    EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
			    EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DSMGR_API_dsSetAllmEnabled)));
			    auto param = static_cast<dsSetAllmEnabledParam_t*>(arg);
			    param->result =dsERR_NONE;
			    return IARM_RESULT_SUCCESS;
			    });

    params["state"]  = "game";	
    status = InvokeServiceMethod("org.rdk.SystemMode", "requestState", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
 
    status = InvokeServiceMethod("org.rdk.SystemMode", "getState", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    if (result.HasLabel("state")) {
	    EXPECT_STREQ("GAME", result["state"].String().c_str());
    }

 
}

TEST_F(SystemModeTest,requestStateVideo)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEMMODE_CALLSIGN,SYSTEMMODEL2TEST_CALLSIGN);
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    std::string reply;	

    params["systemMode"] = "device_optimize";
    params["state"]  = "video";

    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
	    .Times(::testing::AnyNumber())
	    .WillRepeatedly(
			    [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
			    EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
			    EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DSMGR_API_dsSetAllmEnabled)));
			    auto param = static_cast<dsSetAllmEnabledParam_t*>(arg);
			    param->result =dsERR_NONE;
			    return IARM_RESULT_SUCCESS;
			    });

    status = InvokeServiceMethod("org.rdk.SystemMode", "requestState", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

    status = InvokeServiceMethod("org.rdk.SystemMode", "getState", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    if (result.HasLabel("state")) {
	    EXPECT_STREQ("VIDEO", result["state"].String().c_str());
    }

}

TEST_F(SystemModeTest,requestStateNegativeCase)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEMMODE_CALLSIGN,SYSTEMMODEL2TEST_CALLSIGN);
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    std::string reply;	

// case1  empty systemMode and state
    params["systemMode"] = "";
    params["state"]  = "";
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
	    .Times(::testing::AnyNumber())
	    .WillRepeatedly(
			    [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
			    EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
			    EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DSMGR_API_dsSetAllmEnabled)));
			    auto param = static_cast<dsSetAllmEnabledParam_t*>(arg);
			    param->result =dsERR_NONE;
			    return IARM_RESULT_SUCCESS;
			    });

    status = InvokeServiceMethod("org.rdk.SystemMode", "requestState", params, result);
    if (result.HasLabel("error")) {
	    EXPECT_STREQ("{\"code\":1,\"message\":\"ERROR_GENERAL\"}", result["error"].String().c_str());
    }
// Case 2 Invalid systemMode and state

    params["systemMode"] = "abc";
    params["state"]  = "bcd";
    EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_Call)
	    .Times(::testing::AnyNumber())
	    .WillRepeatedly(
			    [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
			    EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
			    EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DSMGR_API_dsSetAllmEnabled)));
			    auto param = static_cast<dsSetAllmEnabledParam_t*>(arg);
			    param->result =dsERR_NONE;
			    return IARM_RESULT_SUCCESS;
			    });

    status = InvokeServiceMethod("org.rdk.SystemMode", "requestState", params, result);
    EXPECT_FALSE(result["success"].Boolean());
    if (result.HasLabel("error")) {
	    EXPECT_STREQ("{\"code\":1,\"message\":\"ERROR_GENERAL\"}", result["error"].String().c_str());
    }
   
 
}


