/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#include "gtest/gtest.h"
#include <gmock/gmock.h>

#include "TraceControl.h"
#include "WrapsMock.h"

#include "FactoriesImplementation.h"
#include "ServiceMock.h"
#include "COMLinkMock.h"

using namespace WPEFramework;

namespace {
const string callSign = _T("TraceControl");
const string webPrefix = _T("/Service/TraceControl");
const string volatilePath = _T("/tmp/");
}

class TraceControlWebTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::TraceControl> plugin;
    PluginHost::IWeb* interface;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    WrapsImplMock wrapsImplMock;
    string response;

    TraceControlWebTest()
        : plugin(Core::ProxyType<Plugin::TraceControl>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
        interface = static_cast<PluginHost::IWeb*>(plugin->QueryInterface(PluginHost::IWeb::ID));
        Trace::TraceUnit::Instance().Open(volatilePath);
        Wraps::getInstance().impl = &wrapsImplMock;
    }

    virtual ~TraceControlWebTest()
    {
        Wraps::getInstance().impl = nullptr;
        interface->Release();
        plugin.Release();
        Trace::TraceUnit::Instance().Close();
    }
};

class TraceControlWebInitializedTest : public TraceControlWebTest {
protected:
    FactoriesImplementation factoriesImplementation;
    ServiceMock service;
    COMLinkMock comLinkMock;

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
    }
    virtual ~TraceControlWebInitializedTest() override
    {
        plugin->Deinitialize(&service);
        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(TraceControlWebInitializedTest, httpGetPut)
{
    ON_CALL(service, Background())
        .WillByDefault(::testing::Return(false));
    EXPECT_EQ(string(""), plugin->Initialize(&service));

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

    JsonObject params;
    params.FromString(bodyStr);
    JsonArray arr = params["settings"].Array();
    string jsonString;
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
    params.FromString(bodyStr);
    arr = params["settings"].Array();
    bool bModPresent = false;
    for (unsigned int i = 0; i < arr.Length(); i++)
    {
        arr[i].Object().ToString(jsonString);
        if(arr[i].Object()["module"].String() == "Plugin_TraceControl")
        {
            bModPresent = true;
            EXPECT_THAT(jsonString, ::testing::MatchesRegex(_T("\\{"
                                                        "\"module\":\"Plugin_TraceControl\","
                                                        "\"category\":\".+\","
                                                        "\"state\":\"enabled\""
                                                        "\\}")));
        }
        else
        {
            EXPECT_THAT(jsonString, ::testing::MatchesRegex(_T("\\{"
                                                        "\"module\":\".+\","
                                                        "\"category\":\".+\","
                                                        "\"state\":\"disabled\""
                                                        "\\}")));
        }
    }
    EXPECT_EQ(true, bModPresent);

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
    params.FromString(bodyStr);
    arr = params["settings"].Array();
    bool bModCatPresent = false;
    for (unsigned int i = 0; i < arr.Length(); i++)
    {
        arr[i].Object().ToString(jsonString);
        if(arr[i].Object()["module"].String() == "Plugin_TraceControl")
        {
            if(arr[i].Object()["category"].String() == "Information")
            {
                bModCatPresent = true;
                EXPECT_THAT(jsonString, ::testing::MatchesRegex(_T("\\{"
                                                            "\"module\":\"Plugin_TraceControl\","
                                                            "\"category\":\".+\","
                                                            "\"state\":\"disabled\""
                                                            "\\}")));
            }
            else
            {
                EXPECT_THAT(jsonString, ::testing::MatchesRegex(_T("\\{"
                                                            "\"module\":\"Plugin_TraceControl\","
                                                            "\"category\":\".+\","
                                                            "\"state\":\"enabled\""
                                                            "\\}")));
            }
        }
        else
        {
            EXPECT_THAT(jsonString, ::testing::MatchesRegex(_T("\\{"
                                                        "\"module\":\".+\","
                                                        "\"category\":\".+\","
                                                        "\"state\":\"disabled\""
                                                        "\\}")));
        }
    }
    EXPECT_EQ(true, bModCatPresent);

    //Negative test cases
    //HTTP_PUT - No data
    request.Verb = Web::Request::HTTP_PUT;
    request.Path = webPrefix;
    httpResponse = interface->Process(request);
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
