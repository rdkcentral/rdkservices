#include <gtest/gtest.h>

#include "TraceControl.h"

#include "COMLinkMock.h"
#include "FactoriesImplementation.h"
#include "ServiceMock.h"

using namespace WPEFramework;

using ::testing::NiceMock;

namespace {
const string callSign = _T("TraceControl");
const string webPrefix = _T("/Service/TraceControl");
const string volatilePath = _T("/tmp/");
}

class TraceControlWebTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::TraceControl> plugin;
    PluginHost::IWeb* interface;

    TraceControlWebTest()
        : plugin(Core::ProxyType<Plugin::TraceControl>::Create())
    {
        interface = static_cast<PluginHost::IWeb*>(plugin->QueryInterface(PluginHost::IWeb::ID));
        Trace::TraceUnit::Instance().Open(volatilePath);
    }

    virtual ~TraceControlWebTest()
    {
        interface->Release();
        plugin.Release();
        Trace::TraceUnit::Instance().Close();
    }
};

class TraceControlWebInitializedTest : public TraceControlWebTest {
protected:
    NiceMock<FactoriesImplementation> factoriesImplementation;
    NiceMock<ServiceMock> service;
    NiceMock<COMLinkMock> comLinkMock;

    TraceControlWebInitializedTest()
        : TraceControlWebTest()
    {
        ON_CALL(service, ConfigLine())
            .WillByDefault(::testing::Return("{}"));
        ON_CALL(service, WebPrefix())
            .WillByDefault(::testing::Return(webPrefix));
        ON_CALL(service, VolatilePath())
            .WillByDefault(::testing::Return(volatilePath));
        ON_CALL(service, Callsign())
            .WillByDefault(::testing::Return(callSign));
        ON_CALL(service, COMLink())
            .WillByDefault(::testing::Return(&comLinkMock));

        PluginHost::IFactories::Assign(&factoriesImplementation);
        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }
    virtual ~TraceControlWebInitializedTest() override
    {
        plugin->Deinitialize(&service);
        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(TraceControlWebInitializedTest, httpGetPut)
{
    //HTTP_GET - Get status for all modules
    Web::Request request;
    request.Verb = Web::Request::HTTP_GET;
    request.Path = webPrefix;
    auto httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
    EXPECT_EQ(httpResponse->ErrorCode, Web::STATUS_OK);
    EXPECT_EQ(httpResponse->Message, "OK");
    EXPECT_EQ(httpResponse->ContentType, Web::MIMETypes::MIME_JSON);
    //Validate the status for all modules. The state should be one of disabled/enabled/tristated
    auto body = httpResponse->Body<Web::JSONBodyType<Plugin::TraceControl::Data>>();
    ASSERT_TRUE(body.IsValid());
    string bodyStr;
    body->ToString(bodyStr);
    EXPECT_THAT(bodyStr, ::testing::MatchesRegex("\\{"
                                                 "\"settings\":"
                                                 "\\[(\\{\"module\":\"[^\"]+\",\"category\":\"[^\"]+\",\"state\":\"(disabled|enabled|tristated)\"\\},{0,}){0,}\\]"
                                                 "\\}"));

    //HTTP_PUT - Set all module state to "disabled"
    request.Verb = Web::Request::HTTP_PUT;
    request.Path = webPrefix + "/off";
    httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
    EXPECT_EQ(httpResponse->ErrorCode, Web::STATUS_OK);
    EXPECT_EQ(httpResponse->Message, "OK");
    request.Verb = Web::Request::HTTP_GET;
    request.Path = webPrefix;
    httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
    EXPECT_EQ(httpResponse->ErrorCode, Web::STATUS_OK);
    EXPECT_EQ(httpResponse->Message, "OK");
    EXPECT_EQ(httpResponse->ContentType, Web::MIMETypes::MIME_JSON);
    //Validate the status for all modules. The state should be disabled
    body = httpResponse->Body<Web::JSONBodyType<Plugin::TraceControl::Data>>();
    body->ToString(bodyStr);
    EXPECT_THAT(bodyStr, ::testing::MatchesRegex("\\{"
                                                 "\"settings\":"
                                                 "\\[(\\{\"module\":\"[^\"]+\",\"category\":\"[^\"]+\",\"state\":\"disabled\"\\},{0,}){0,}\\]"
                                                 "\\}"));

    //HTTP_PUT - Set Plugin_TraceControl module state to "enabled"
    request.Verb = Web::Request::HTTP_PUT;
    request.Path = webPrefix + "/Plugin_TraceControl/on";
    httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
    EXPECT_EQ(httpResponse->ErrorCode, Web::STATUS_OK);
    EXPECT_EQ(httpResponse->Message, "OK");
    request.Verb = Web::Request::HTTP_GET;
    request.Path = webPrefix;
    httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
    EXPECT_EQ(httpResponse->ErrorCode, Web::STATUS_OK);
    EXPECT_EQ(httpResponse->Message, "OK");
    EXPECT_EQ(httpResponse->ContentType, Web::MIMETypes::MIME_JSON);
    //Validate the status for Plugin_TraceControl module. The state should be enabled
    //For the rest of the modules the state should be "disabled"
    body = httpResponse->Body<Web::JSONBodyType<Plugin::TraceControl::Data>>();
    body->ToString(bodyStr);
    EXPECT_THAT(bodyStr, ::testing::Not(::testing::ContainsRegex("\\{\"module\":\"Plugin_TraceControl\",\"category\":\"[^\"]+\",\"state\":\"disabled\"\\}")));

    //HTTP_PUT - Set Plugin_TraceControl module, Information category state to "disabled"
    request.Verb = Web::Request::HTTP_PUT;
    request.Path = webPrefix + "/Plugin_TraceControl/Information/off";
    httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
    EXPECT_EQ(httpResponse->ErrorCode, Web::STATUS_OK);
    EXPECT_EQ(httpResponse->Message, "OK");
    request.Verb = Web::Request::HTTP_GET;
    request.Path = webPrefix;
    httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
    EXPECT_EQ(httpResponse->ErrorCode, Web::STATUS_OK);
    EXPECT_EQ(httpResponse->Message, "OK");
    EXPECT_EQ(httpResponse->ContentType, Web::MIMETypes::MIME_JSON);
    //Validate the status for Plugin_TraceControl module, Information category. The state should be disabled
    //For the rest of the modules the state should be "disabled"
    body = httpResponse->Body<Web::JSONBodyType<Plugin::TraceControl::Data>>();
    body->ToString(bodyStr);
    EXPECT_THAT(bodyStr, ::testing::HasSubstr("{\"module\":\"Plugin_TraceControl\",\"category\":\"Information\",\"state\":\"disabled\"}"));
}

TEST_F(TraceControlWebInitializedTest, httpGetPut_negative)
{
    //HTTP_PUT - No data
    Web::Request request;
    request.Verb = Web::Request::HTTP_PUT;
    request.Path = webPrefix;
    auto httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
    EXPECT_EQ(httpResponse->ErrorCode, Web::STATUS_BAD_REQUEST);

    //HTTP_PUT - Module - No data
    request.Verb = Web::Request::HTTP_PUT;
    request.Path = webPrefix + "/Test";
    httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
    EXPECT_EQ(httpResponse->ErrorCode, Web::STATUS_BAD_REQUEST);

    //HTTP_PUT - Module - Category - No data
    request.Verb = Web::Request::HTTP_PUT;
    request.Path = webPrefix + "/Test_mod/Test_Cat";
    httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
    EXPECT_EQ(httpResponse->ErrorCode, Web::STATUS_BAD_REQUEST);

    //HTTP_PUT - Module - Category - Wrong data
    request.Verb = Web::Request::HTTP_PUT;
    request.Path = webPrefix + "/Test_mod/Test_cat/Test_on";
    httpResponse = interface->Process(request);
    ASSERT_TRUE(httpResponse.IsValid());
    EXPECT_EQ(httpResponse->ErrorCode, Web::STATUS_BAD_REQUEST);
}
