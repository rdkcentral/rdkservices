#include <gtest/gtest.h>

#include "DeviceInfo.h"

#include "IarmBusMock.h"
#include "ManagerMock.h"
#include "ServiceMock.h"

#include "FactoriesImplementation.h"
#include "source/SystemInfo.h"

using namespace WPEFramework;

namespace {
const string webPrefix = _T("/Service/DeviceInfo");
const string iarmName = _T("Thunder_Plugins");
}

class DeviceInfoWebTest : public ::testing::Test {
protected:
    FactoriesImplementation factoriesImplementation;
    IarmBusImplMock iarmBusImplMock;
    ManagerImplMock managerImplMock;
    ServiceMock service;
    Core::Sink<SystemInfo> subSystem;
    Core::ProxyType<Plugin::DeviceInfo> plugin;
    PluginHost::IWeb* interface;

    DeviceInfoWebTest()
        : plugin(Core::ProxyType<Plugin::DeviceInfo>::Create())
        , interface(nullptr)
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);
        IarmBus::getInstance().impl = &iarmBusImplMock;
        device::Manager::getInstance().impl = &managerImplMock;
    }
    virtual ~DeviceInfoWebTest()
    {
        PluginHost::IFactories::Assign(nullptr);
        IarmBus::getInstance().impl = nullptr;
        device::Manager::getInstance().impl = nullptr;
    }

    virtual void SetUp()
    {
        interface = static_cast<PluginHost::IWeb*>(plugin->QueryInterface(PluginHost::IWeb::ID));
        ASSERT_TRUE(interface != nullptr);

        EXPECT_CALL(iarmBusImplMock, IARM_Bus_IsConnected(::testing::_, ::testing::_))
            .Times(1)
            .WillOnce(::testing::Invoke(
                [](const char* memberName, int* isRegistered) {
                    if (iarmName == string(memberName)) {
                        // Return 1 as not interested in all steps of IARM connection
                        *isRegistered = 1;
                        return IARM_RESULT_SUCCESS;
                    }
                    return IARM_RESULT_INVALID_PARAM;
                }));
        EXPECT_CALL(managerImplMock, Initialize())
            .Times(1)
            .WillOnce(::testing::Return());
        EXPECT_CALL(service, WebPrefix())
            .Times(1)
            .WillOnce(::testing::Return(webPrefix));
        ON_CALL(service, Version())
            .WillByDefault(::testing::Return(string()));
        EXPECT_CALL(service, SubSystems())
            .Times(1)
            .WillRepeatedly(::testing::Invoke(
                [&]() {
                    PluginHost::ISubSystem* result = (&subSystem);
                    result->AddRef();
                    return result;
                }));

        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }

    virtual void TearDown()
    {
        ASSERT_TRUE(interface != nullptr);
        interface->Release();

        plugin->Deinitialize(&service);
    }
};

TEST_F(DeviceInfoWebTest, httpGet)
{
    Web::Request request;
    request.Verb = Web::Request::HTTP_GET;
    request.Path = webPrefix;

    auto response = interface->Process(request);
    ASSERT_TRUE(response.IsValid());

    EXPECT_EQ(response->ErrorCode, Web::STATUS_OK);
    EXPECT_EQ(response->Message, "OK");
    EXPECT_EQ(response->ContentType, Web::MIMETypes::MIME_JSON);

    auto body = response->Body<Web::JSONBodyType<Plugin::DeviceInfo::Data>>();
    ASSERT_TRUE(body.IsValid());

    string bodyStr;
    body->ToString(bodyStr);
    EXPECT_THAT(bodyStr, ::testing::MatchesRegex("\\{"
                                                 "\"addresses\":"
                                                 "\\[(\\{\"name\":\".+\",\"mac\":\".+\"(,\"ip\":\\[(\".+\"){1,}\\]){0,}\\}){0,}\\],"
                                                 "\"systeminfo\":"
                                                 "\\{"
                                                 "\"version\":\"#\","
                                                 "\"uptime\":[0-9]+,"
                                                 "\"totalram\":[0-9]+,"
                                                 "\"freeram\":[0-9]+,"
                                                 "\"totalswap\":[0-9]+,"
                                                 "\"freeswap\":[0-9]+,"
                                                 "\"devicename\":\".+\","
                                                 "\"cpuload\":\"[0-9]+\","
                                                 "\"cpuloadavg\":"
                                                 "\\{"
                                                 "\"avg1min\":[0-9]+,"
                                                 "\"avg5min\":[0-9]+,"
                                                 "\"avg15min\":[0-9]+"
                                                 "\\},"
                                                 "\"serialnumber\":\".+\","
                                                 "\"time\":\".+\""
                                                 "\\},"
                                                 "\"sockets\":"
                                                 "\\{\"runs\":[0-9]+\\}"
                                                 "\\}"));
}

TEST_F(DeviceInfoWebTest, httpGetAdresses)
{
    Web::Request request;
    request.Verb = Web::Request::HTTP_GET;
    request.Path = webPrefix + _T("/Adresses");

    auto response = interface->Process(request);
    ASSERT_TRUE(response.IsValid());

    EXPECT_EQ(response->ErrorCode, Web::STATUS_OK);
    EXPECT_EQ(response->Message, "OK");
    EXPECT_EQ(response->ContentType, Web::MIMETypes::MIME_JSON);

    auto body = response->Body<Web::JSONBodyType<Plugin::DeviceInfo::Data>>();
    ASSERT_TRUE(body.IsValid());

    string bodyStr;
    body->ToString(bodyStr);
    EXPECT_THAT(bodyStr, ::testing::MatchesRegex("\\{"
                                                 "\"addresses\":"
                                                 "\\[(\\{\"name\":\".+\",\"mac\":\".+\"(,\"ip\":\\[(\".+\"){1,}\\]){0,}\\}){0,}\\]"
                                                 "\\}"));
}

TEST_F(DeviceInfoWebTest, httpGetSystem)
{
    Web::Request request;
    request.Verb = Web::Request::HTTP_GET;
    request.Path = webPrefix + _T("/System");

    auto response = interface->Process(request);
    ASSERT_TRUE(response.IsValid());

    EXPECT_EQ(response->ErrorCode, Web::STATUS_OK);
    EXPECT_EQ(response->Message, "OK");
    EXPECT_EQ(response->ContentType, Web::MIMETypes::MIME_JSON);

    auto body = response->Body<Web::JSONBodyType<Plugin::DeviceInfo::Data>>();
    ASSERT_TRUE(body.IsValid());

    string bodyStr;
    body->ToString(bodyStr);
    EXPECT_THAT(bodyStr, ::testing::MatchesRegex("\\{"
                                                 "\"systeminfo\":"
                                                 "\\{"
                                                 "\"version\":\"#\","
                                                 "\"uptime\":[0-9]+,"
                                                 "\"totalram\":[0-9]+,"
                                                 "\"freeram\":[0-9]+,"
                                                 "\"totalswap\":[0-9]+,"
                                                 "\"freeswap\":[0-9]+,"
                                                 "\"devicename\":\".+\","
                                                 "\"cpuload\":\"[0-9]+\","
                                                 "\"cpuloadavg\":"
                                                 "\\{"
                                                 "\"avg1min\":[0-9]+,"
                                                 "\"avg5min\":[0-9]+,"
                                                 "\"avg15min\":[0-9]+"
                                                 "\\},"
                                                 "\"serialnumber\":\".+\","
                                                 "\"time\":\".+\""
                                                 "\\}"
                                                 "\\}"));
}

TEST_F(DeviceInfoWebTest, httpGetSockets)
{
    Web::Request request;
    request.Verb = Web::Request::HTTP_GET;
    request.Path = webPrefix + _T("/Sockets");

    auto response = interface->Process(request);
    ASSERT_TRUE(response.IsValid());

    EXPECT_EQ(response->ErrorCode, Web::STATUS_OK);
    EXPECT_EQ(response->Message, "OK");
    EXPECT_EQ(response->ContentType, Web::MIMETypes::MIME_JSON);

    auto body = response->Body<Web::JSONBodyType<Plugin::DeviceInfo::Data>>();
    ASSERT_TRUE(body.IsValid());

    string bodyStr;
    body->ToString(bodyStr);
    EXPECT_THAT(bodyStr, ::testing::MatchesRegex("\\{"
                                                 "\"sockets\":"
                                                 "\\{\"runs\":[0-9]+\\}"
                                                 "\\}"));
}
