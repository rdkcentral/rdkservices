#include <gtest/gtest.h>

#include "AVInput.h"

#include "HdmiInputMock.h"

using namespace WPEFramework;

using ::testing::NiceMock;

class AVInputTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::AVInput> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    AVInputTest()
        : plugin(Core::ProxyType<Plugin::AVInput>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~AVInputTest() = default;
};

class AVInputDsTest : public AVInputTest {
protected:
    NiceMock<HdmiInputImplMock> hdmiInputImplMock;

    AVInputDsTest()
        : AVInputTest()
    {
        device::HdmiInput::getInstance().impl = &hdmiInputImplMock;
    }
    virtual ~AVInputDsTest() override
    {
        device::HdmiInput::getInstance().impl = nullptr;
    }
};

TEST_F(AVInputTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("numberOfInputs")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("currentVideoMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("contentProtected")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEdid2AllmSupport")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getEdid2AllmSupport")));
}

TEST_F(AVInputTest, contentProtected)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("contentProtected"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"isContentProtected\":true,\"success\":true}"));
}

TEST_F(AVInputDsTest, numberOfInputs)
{
    ON_CALL(hdmiInputImplMock, getNumberOfInputs())
        .WillByDefault(::testing::Return(1));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("numberOfInputs"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"numberOfInputs\":1,\"success\":true}"));
}

TEST_F(AVInputDsTest, currentVideoMode)
{
    ON_CALL(hdmiInputImplMock, getCurrentVideoMode())
        .WillByDefault(::testing::Return(string("unknown")));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("currentVideoMode"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"currentVideoMode\":\"unknown\",\"success\":true}"));
}

TEST_F(AVInputDsTest, getEdid2AllmSupport)
{
    //ON_CALL(hdmiInputImplMock, getEdid2AllmSupport(::testing::_,::testing::_))
       // .WillByDefault(::testing::Return(string("unknown")));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEdid2AllmSupport"), _T("{\"portId\": \"0\",\"allmSupport\":true}"), response));
    EXPECT_EQ(response, string("{\"allmSupport\":true,\"success\":true}"));
}


TEST_F(AVInputDsTest, setEdid2AllmSupport)
{
    //ON_CALL(hdmiInputImplMock, setEdid2AllmSupport(::testing::_,::testing::_))
      //  .WillByDefault(::testing::Return(string("unknown")));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEdid2AllmSupport"), _T("{\"portId\": \"0\",\"allmSupport\":true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

