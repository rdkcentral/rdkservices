#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <interfaces/ISystemMode.h>

#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);

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
        /** @brief Pointer to the IShell interface */
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

/*
   uint32_t SystemModeTest::CreateUserSettingInterfaceObjectUsingComRPCConnection()
{
    uint32_t return_value =  Core::ERROR_GENERAL;
    Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> Engine_UserSettings;
    Core::ProxyType<RPC::CommunicatorClient> Client_UserSettings;

    TEST_LOG("Creating Engine_UserSettings");
    Engine_UserSettings = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
    Client_UserSettings = Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(Engine_UserSettings));

    TEST_LOG("Creating Engine_UserSettings Announcements");
#if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
    Engine_UserSettings->Announcements(mClient_UserSettings->Announcement());
#endif
    if (!Client_UserSettings.IsValid())
    {
        TEST_LOG("Invalid Client_UserSettings");
    }
    else
    {
        m_controller_usersettings = Client_UserSettings->Open<PluginHost::IShell>(_T("org.rdk.UserSettings"), ~0, 3000);
        if (m_controller_usersettings)
        {
        m_usersettingsplugin = m_controller_usersettings->QueryInterface<Exchange::IUserSettings>();
        return_value = Core::ERROR_NONE;
        }
    }
    return return_value;
}
*/

TEST_F(SystemModeTest,GetStatedefault)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(SYSTEMMODE_CALLSIGN,SYSTEMMODEL2TEST_CALLSIGN);
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result;
    std::string message;
    std::string reply;	

    params["systemMode"] = "DeviceOptimize";
    status = InvokeServiceMethod("org.rdk.SystemMode", "GetState", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    JsonObject  result_final ;
    result_final["message"] = result;
    EXPECT_EQ(result_final,result);	
	
    //result.ToString(reply);
	
    TEST_LOG("RamTest Status %u, results %s", status, reply.c_str()); 	
    std::cout<<"Ram Test reply :"<<reply<<std::endl ;
    /*
    EXPECT_TRUE(result["success"].Boolean());
    if (result.HasLabel("state")) {
	    EXPECT_STREQ("Video", result["state"].String().c_str());
    }
   */
    params["state"]  = "Game";
    status = InvokeServiceMethod("org.rdk.SystemMode", "RequestState", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    result.ToString(reply);
    TEST_LOG("RamTest Status %u, results %s", status, reply.c_str()); 	

    status = InvokeServiceMethod("org.rdk.SystemMode", "GetState", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
 
}


