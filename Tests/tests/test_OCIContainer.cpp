#include <gtest/gtest.h>

#include "OCIContainer.h"
#include "ServiceMock.h"
#include "DobbyMock.h"
#include "FactoriesImplementation.h"

using namespace WPEFramework;
using ::testing::NiceMock;

namespace{
    const int32_t descriptor1 = 91;
    const int32_t descriptor2 = 94;
    const string containerId1 = "com.bskyb.epgui";
    const string containerId2 = "Netflix";
    const int stateRunning = 2;
    const int listenerId = 5;
}

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
    DobbyProxyMock dobbymock;
    IpcServiceMock ipcservicemock;

    OCIContainerInitializedTest()
        : OCIContainerTest()
    {
        DobbyProxy::getInstance().impl = &dobbymock;
        IpcService::getInstance().impl = &ipcservicemock;

        EXPECT_CALL(ipcservicemock, start())
            .WillOnce(::testing::Return(true));

        EXPECT_CALL(dobbymock, registerListener(::testing::_, ::testing::_))
            .WillOnce(::testing::Return(listenerId));

        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }

    virtual ~OCIContainerInitializedTest() override
    {
        EXPECT_CALL(dobbymock, unregisterListener(listenerId))
            .WillOnce(::testing::Return());

        plugin->Deinitialize(&service);
    }
};



TEST_F(OCIContainerTest, RegisteredMethods){
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

 TEST_F(OCIContainerInitializedTest, listContainersTest){

    std::list<std::pair<int32_t, std::string>> containerslist = {{descriptor1, containerId1},{descriptor2, containerId2}};
    EXPECT_CALL(dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("listContainers"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"containers\":[{\"Descriptor\":91,\"Id\":\"com.bskyb.epgui\"},{\"Descriptor\":94,\"Id\":\"Netflix\"}],\"success\":true}"));
}

 TEST_F(OCIContainerInitializedTest, getContainerStateTest){

    std::list<std::pair<int32_t, std::string>> containerslist = {{descriptor1, containerId1},{descriptor2, containerId2}};
    EXPECT_CALL(dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_CALL(dobbymock, getContainerState(descriptor1))
        .WillOnce(::testing::Return(stateRunning));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getContainerState"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\"}"), response));
    EXPECT_EQ(response, string("{\"containerId\":\"com.bskyb.epgui\",\"state\":\"Running\",\"success\":true}"));
}


 TEST_F(OCIContainerInitializedTest, getContainerInfoTest){

    std::list<std::pair<int32_t, std::string>> containerslist = {{descriptor1, containerId1},{descriptor2, containerId2}};
    EXPECT_CALL(dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_CALL(dobbymock, getContainerInfo(descriptor1))
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


TEST_F(OCIContainerInitializedTest, startContainerTest){

    EXPECT_CALL(dobbymock, startContainerFromBundle(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(descriptor1));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startContainer"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\", \"bundlePath\":\"/containers/myBundle\","
        "\"command\":\"command\", \"westerosSocket\":\"/usr/mySocket\", \"envvar\":[\"FOO=BAR\"]}"), response));
    EXPECT_EQ(response, string("{\"descriptor\":91,\"success\":true}"));
}

TEST_F(OCIContainerInitializedTest, startContainerFromDobbySpecTest){

     EXPECT_CALL(dobbymock, startContainerFromSpec(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(descriptor1));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startContainerFromDobbySpec"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\", \"dobbySpec\":\"/containers/dobbySpec\","
        "\"command\":\"command\", \"westerosSocket\":\"/usr/mySocket\"}"), response));
    EXPECT_EQ(response, string("{\"descriptor\":91,\"success\":true}"));
}

TEST_F(OCIContainerInitializedTest, stopContainerTest){

    std::list<std::pair<int32_t, std::string>> containerslist = {{descriptor1, containerId1},{descriptor2, containerId2}};
    EXPECT_CALL(dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_CALL(dobbymock, stopContainer(descriptor1, ::testing::_))
        .WillOnce(::testing::Return(true));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopContainer"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\", \"force\":true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(OCIContainerInitializedTest, pauseContainerTest){

    std::list<std::pair<int32_t, std::string>> containerslist = {{descriptor1, containerId1},{descriptor2, containerId2}};
    EXPECT_CALL(dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_CALL(dobbymock, pauseContainer(descriptor1))
        .WillOnce(::testing::Return(true));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("pauseContainer"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

}

TEST_F(OCIContainerInitializedTest, resumeContainerTest){

    std::list<std::pair<int32_t, std::string>> containerslist = {{descriptor1, containerId1},{descriptor2, containerId2}};
    EXPECT_CALL(dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_CALL(dobbymock, resumeContainer(descriptor1))
        .WillOnce(::testing::Return(true));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resumeContainer"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

}

TEST_F(OCIContainerInitializedTest, executeCommandTest){

    std::list<std::pair<int32_t, std::string>> containerslist = {{descriptor1, containerId1},{descriptor2, containerId2}};
    EXPECT_CALL(dobbymock, listContainers())
        .WillOnce(::testing::Return(containerslist));

    EXPECT_CALL(dobbymock, execInContainer(descriptor1, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(true));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("executeCommand"), _T("{"
        "\"containerId\": \"com.bskyb.epgui\", \"options\":\"--cwd=PATH\", \"command\":\"/bin/sleep 5\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}