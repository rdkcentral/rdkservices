#include <gtest/gtest.h>

#include "OCIContainer.h"
#include "ServiceMock.h"
#include "DobbyMock.h"
#include "FactoriesImplementation.h"

using namespace WPEFramework;
using ::testing::NiceMock;

class OCIContainerTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::OCIContainer> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    OCIContainerTest()
        : plugin(Core::ProxyType<Plugin::OCIContainer>::Create())
        , handler(*(plugin))
        , connection(1, 0)
        {
        }
        virtual ~OCIContainerTest() = default;
};

class OCIContainerInitializedTest : public OCIContainerTest {
protected:
    NiceMock<ServiceMock> service;
    DobbyProxyMock    *p_dobbymock = nullptr ;
    IpcServiceMock    *p_ipcservicemock = nullptr ;

    OCIContainerInitializedTest()
        : OCIContainerTest()
    {
        p_dobbymock  = new NiceMock <DobbyProxyMock>;
        DobbyProxy::setImpl(p_dobbymock);

        p_ipcservicemock  = new NiceMock <IpcServiceMock>;
        IpcService::setImpl(p_ipcservicemock);

        EXPECT_CALL(*p_ipcservicemock, start())
            .WillOnce(::testing::Return(true));

        EXPECT_CALL(*p_dobbymock, registerListener(::testing::_, ::testing::_))
            .WillOnce(::testing::Return(5));

        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }

    virtual ~OCIContainerInitializedTest() override
    {
        EXPECT_CALL(*p_dobbymock, unregisterListener(5))
            .WillOnce(::testing::Return());

        plugin->Deinitialize(&service);
        DobbyProxy::setImpl(nullptr);
        if (p_dobbymock != nullptr)
        {
            delete p_dobbymock;
            p_dobbymock = nullptr;
        }

        IpcService::setImpl(nullptr);
        if (p_ipcservicemock != nullptr)
        {
            delete p_ipcservicemock;
            p_ipcservicemock = nullptr;
        }
    }
};



TEST_F(OCIContainerTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("listContainers")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getContainerState")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getContainerInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("startContainer")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("startContainerFromDobbySpec")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("stopContainer")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("pauseContainer")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("resumeContainer")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("executeCommand")));
}

TEST_F(OCIContainerInitializedTest, listContainersTest)
{
    std::list<std::pair<int32_t, std::string>> containerslist = {{91, "com.bskyb.epgui"},{94, "Netflix"}};
    EXPECT_CALL(*p_dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("listContainers"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"containers\":[{\"Descriptor\":91,\"Id\":\"com.bskyb.epgui\"},{\"Descriptor\":94,\"Id\":\"Netflix\"}],\"success\":true}"));
}

TEST_F(OCIContainerInitializedTest, getContainerStateTest)
{
    std::list<std::pair<int32_t, std::string>> containerslist = {{91, "com.bskyb.epgui"},{94, "Netflix"}};
    EXPECT_CALL(*p_dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_CALL(*p_dobbymock, getContainerState(91))
        .WillOnce(::testing::Return(IDobbyProxyEvents::ContainerState::Running));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getContainerState"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\"}"), response));
    EXPECT_EQ(response, string("{\"containerId\":\"com.bskyb.epgui\",\"state\":\"Running\",\"success\":true}"));
}


TEST_F(OCIContainerInitializedTest, getContainerInfoTest)
{
    std::list<std::pair<int32_t, std::string>> containerslist = {{91, "com.bskyb.epgui"},{94, "Netflix"}};
    EXPECT_CALL(*p_dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_CALL(*p_dobbymock, getContainerInfo(91))
        .WillOnce(::testing::Return("{\"cpu\":{\"usage\" :{\"percpu\" :[3661526845,3773518079,4484546066,4700379608],"
   "\"total\" : 16619970598}},\"gpu\":{\"memory\" :{\"failcnt\" : 0,\"limit\" : 209715200,\"max\" : 3911680,\"usage\" : 0}},"
   "\"id\":\"Cobalt-0\",\"ion\":{\"heaps\" :{\"ion.\" :{\"failcnt\" : null,\"limit\" : null,\"max\" : null,\"usage\" : null}}},"
   "\"memory\":{\"user\" :{\"failcnt\" : 0,\"limit\" : 419430400,\"max\" : 73375744,\"usage\" : 53297152}},\"pids\":[13132,13418],"
   "\"processes\":[{\"cmdline\" : \"/usr/libexec/DobbyInit /usr/bin/WPEProcess -l libWPEFrameworkCobaltImpl.so -c"
   " CobaltImplementation -C Cobalt-0 -r /tmp/communicator -i 64 -x 48 -p \"/opt/persistent/rdkservices/Cobalt-0/\" -s"
   " \"/usr/lib/wpeframework/plugins/\" -d \"/usr/share/WPEFramework/Cobalt/\" -a \"/usr/bin/\" -v \"/tmp/Cobalt-0/\" -m"
   " \"/usr/lib/wpeframework/proxystubs/\" -P \"/opt/minidumps/\" \",\"executable\" : \"/usr/libexec/DobbyInit\",\"nsPid\" : 1,"
   "\"pid\" : 13132},{\"cmdline\" : \"WPEProcess -l libWPEFrameworkCobaltImpl.so -c CobaltImplementation -C Cobalt-0 -r /tmp/communicator"
   " -i 64 -x 48 -p \"/opt/persistent/rdkservices/Cobalt-0/\" -s \"/usr/lib/wpeframework/plugins/\" -d \"/usr/share/WPEFramework/Cobalt/\""
   " -a \"/usr/bin/\" -v \"/tmp/Cobalt-0/\" -m \"/usr/lib/wpeframework/proxystubs/\" -P \"/opt/minidumps/\" \",\"executable\" : "
   "\"/usr/bin/WPEProcess\",\"nsPid\" : 6,\"pid\" : 13418}],\"state\":\"running\",\"timestamp\":1298657054196}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getContainerInfo"), _T("{"
         "\"containerId\": \"com.bskyb.epgui\"}"), response));
    EXPECT_EQ(response, string("{\"info\":{\"cpu\":{\"usage\" :{\"percpu\" :[3661526845,3773518079,4484546066,4700379608],"
   "\"total\" : 16619970598}},\"gpu\":{\"memory\" :{\"failcnt\" : 0,\"limit\" : 209715200,\"max\" : 3911680,\"usage\" : 0}},"
   "\"id\":\"Cobalt-0\",\"ion\":{\"heaps\" :{\"ion.\" :{\"failcnt\" : null,\"limit\" : null,\"max\" : null,\"usage\" : null}}},"
   "\"memory\":{\"user\" :{\"failcnt\" : 0,\"limit\" : 419430400,\"max\" : 73375744,\"usage\" : 53297152}},\"pids\":[13132,13418],"
   "\"processes\":[{\"cmdline\" : \"/usr/libexec/DobbyInit /usr/bin/WPEProcess -l libWPEFrameworkCobaltImpl.so -c"
   " CobaltImplementation -C Cobalt-0 -r /tmp/communicator -i 64 -x 48 -p \"/opt/persistent/rdkservices/Cobalt-0/\" -s"
   " \"/usr/lib/wpeframework/plugins/\" -d \"/usr/share/WPEFramework/Cobalt/\" -a \"/usr/bin/\" -v \"/tmp/Cobalt-0/\" -m"
   " \"/usr/lib/wpeframework/proxystubs/\" -P \"/opt/minidumps/\" \",\"executable\" : \"/usr/libexec/DobbyInit\",\"nsPid\" : 1,"
   "\"pid\" : 13132},{\"cmdline\" : \"WPEProcess -l libWPEFrameworkCobaltImpl.so -c CobaltImplementation -C Cobalt-0 -r /tmp/communicator"
   " -i 64 -x 48 -p \"/opt/persistent/rdkservices/Cobalt-0/\" -s \"/usr/lib/wpeframework/plugins/\" -d \"/usr/share/WPEFramework/Cobalt/\""
   " -a \"/usr/bin/\" -v \"/tmp/Cobalt-0/\" -m \"/usr/lib/wpeframework/proxystubs/\" -P \"/opt/minidumps/\" \",\"executable\" : "
   "\"/usr/bin/WPEProcess\",\"nsPid\" : 6,\"pid\" : 13418}],\"state\":\"running\",\"timestamp\":1298657054196},\"success\":true}"));
}


TEST_F(OCIContainerInitializedTest, startContainerTest)
{
    EXPECT_CALL(*p_dobbymock, startContainerFromBundle(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(91));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startContainer"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\", \"bundlePath\":\"/containers/myBundle\","
        "\"command\":\"command\", \"westerosSocket\":\"/usr/mySocket\", \"envvar\":[\"FOO=BAR\"]}"), response));
    EXPECT_EQ(response, string("{\"descriptor\":91,\"success\":true}"));
}

TEST_F(OCIContainerInitializedTest, startContainerFromDobbySpecTest)
{
    EXPECT_CALL(*p_dobbymock, startContainerFromSpec(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(91));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startContainerFromDobbySpec"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\", \"dobbySpec\":\"/containers/dobbySpec\","
        "\"command\":\"command\", \"westerosSocket\":\"/usr/mySocket\"}"), response));
    EXPECT_EQ(response, string("{\"descriptor\":91,\"success\":true}"));
}

TEST_F(OCIContainerInitializedTest, stopContainerTest)
{
    std::list<std::pair<int32_t, std::string>> containerslist = {{91, "com.bskyb.epgui"},{94, "Netflix"}};
    EXPECT_CALL(*p_dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_CALL(*p_dobbymock, stopContainer(91, ::testing::_))
        .WillOnce(::testing::Return(true));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopContainer"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\", \"force\":true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(OCIContainerInitializedTest, pauseContainerTest)
{
    std::list<std::pair<int32_t, std::string>> containerslist = {{91, "com.bskyb.epgui"},{94, "Netflix"}};
    EXPECT_CALL(*p_dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_CALL(*p_dobbymock, pauseContainer(91))
        .WillOnce(::testing::Return(true));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("pauseContainer"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(OCIContainerInitializedTest, resumeContainerTest)
{
    std::list<std::pair<int32_t, std::string>> containerslist = {{91, "com.bskyb.epgui"},{94, "Netflix"}};
    EXPECT_CALL(*p_dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_CALL(*p_dobbymock, resumeContainer(91))
        .WillOnce(::testing::Return(true));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resumeContainer"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(OCIContainerInitializedTest, executeCommandTest)
{
    std::list<std::pair<int32_t, std::string>> containerslist = {{91, "com.bskyb.epgui"},{94, "Netflix"}};
    EXPECT_CALL(*p_dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_CALL(*p_dobbymock, execInContainer(91, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(true));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("executeCommand"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\", \"options\":\"--cwd=PATH\", \"command\":\"/bin/sleep 5\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
