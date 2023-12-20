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
#include "FactoriesImplementation.h"
#include "VoiceControl.h"

#include "IarmBusMock.h"
#include "ServiceMock.h"

using namespace WPEFramework;
using ::testing::NiceMock;
using ::testing::Test;
using ::testing::_;
using ::testing::Invoke;
using ::testing::StrEq;
using ::testing::ContainsRegex;
using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;

class VoiceControlTest : public Test {
protected:
    Core::ProxyType<Plugin::VoiceControl> plugin_;
    Core::JSONRPC::Handler&               handler_;
    Core::JSONRPC::Connection             connection_;
    string                                response_;
    IarmBusImplMock   *p_iarmBusImplMock = nullptr ;

    VoiceControlTest()
        : plugin_(Core::ProxyType<Plugin::VoiceControl>::Create())
        , handler_(*plugin_)
        , connection_(1, 0)
    {
        p_iarmBusImplMock  = new NiceMock <IarmBusImplMock>;
        IarmBus::setImpl(p_iarmBusImplMock);
    }

    virtual ~VoiceControlTest() override
    {
        IarmBus::setImpl(nullptr);
        if (p_iarmBusImplMock != nullptr)
        {
            delete p_iarmBusImplMock;
            p_iarmBusImplMock = nullptr;
        }
    }
};

static AssertionResult isValidCtrlmVoiceIarmEvent(IARM_EventId_t ctrlmVoiceIarmEventId)
{
    switch (ctrlmVoiceIarmEventId) {
        case CTRLM_VOICE_IARM_EVENT_JSON_SESSION_BEGIN:
        case CTRLM_VOICE_IARM_EVENT_JSON_STREAM_BEGIN:
        case CTRLM_VOICE_IARM_EVENT_JSON_KEYWORD_VERIFICATION:
        case CTRLM_VOICE_IARM_EVENT_JSON_SERVER_MESSAGE:
        case CTRLM_VOICE_IARM_EVENT_JSON_STREAM_END:
        case CTRLM_VOICE_IARM_EVENT_JSON_SESSION_END:
            return AssertionSuccess();
        default:
            return AssertionFailure();
    }
}

class VoiceControlInitializedEventTest : public VoiceControlTest {
protected:
    IARM_EventHandler_t               voiceEventHandler_;
    NiceMock<ServiceMock>             service_;
    NiceMock<FactoriesImplementation> factoriesImplementation_;
    Core::JSONRPC::Message            message_;
    PluginHost::IDispatcher*          dispatcher_;

    VoiceControlInitializedEventTest() :
        VoiceControlTest()
    {
        EXPECT_CALL(*p_iarmBusImplMock, IARM_Bus_RegisterEventHandler(StrEq(CTRLM_MAIN_IARM_BUS_NAME), _, _))
            .WillRepeatedly(Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    EXPECT_TRUE(isValidCtrlmVoiceIarmEvent(eventId));
                    voiceEventHandler_ = handler;
                    return IARM_RESULT_SUCCESS;
                }));
        ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_VOICE_IARM_CALL_STATUS), _, _))
            .WillByDefault(Invoke(
                [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                    auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                    strncpy(param->result, "{\"maskPii\":true}", sizeof(param->result));
                    return IARM_RESULT_SUCCESS;
                }));

        EXPECT_EQ(string(""), plugin_->Initialize(nullptr));
        PluginHost::IFactories::Assign(&factoriesImplementation_);
        dispatcher_ = static_cast<PluginHost::IDispatcher*>(plugin_->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher_->Activate(&service_);
    }

    virtual ~VoiceControlInitializedEventTest() override
    {
        plugin_->Deinitialize(nullptr);
        dispatcher_->Deactivate();
        dispatcher_->Release();
        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(VoiceControlTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("voiceStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("configureVoice")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("setVoiceInit")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("sendVoiceMessage")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("voiceSessionByText")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("voiceSessionTypes")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("voiceSessionRequest")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("voiceSessionTerminate")));
}

TEST_F(VoiceControlTest, voiceStatus)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_VOICE_IARM_CALL_STATUS), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                const string result =  "{\"maskPii\":true,"
                                       "\"capabilities\":[\"PRV\"],"
                                       "\"urlPtt\":\"proxyUrlPtt\","
                                       "\"urlHf\":\"proxyUrlHf\","
                                       "\"prv\":true,"
                                       "\"wwFeedback\":false,"
                                       "\"ptt\":{\"status\":\"ready\"},"
                                       "\"ff\":{\"status\":\"ready\"},"
                                       "\"mic\":{\"status\":\"ready\"},"
                                       "\"success\":true}";
                strncpy(param->result, result.c_str(), sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("voiceStatus"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"maskPii\":true,"
                         "\"capabilities\":[\"PRV\"],"
                         "\"urlPtt\":\"proxyUrlPtt\","
                         "\"urlHf\":\"proxyUrlHf\","
                         "\"prv\":true,"
                         "\"wwFeedback\":false,"
                         "\"ptt\":{\"status\":\"ready\"},"
                         "\"ff\":{\"status\":\"ready\"},"
                         "\"mic\":{\"status\":\"ready\"},"
                         "\"success\":true}");
}

TEST_F(VoiceControlTest, configureVoice)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_VOICE_IARM_CALL_CONFIGURE_VOICE), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                strncpy(param->result, "{\"success\":true}", sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("configureVoice"),
                                _T("{\"urlAll\":\"ws://voiceserver.com/voice/ptt\","
                                "\"urlPtt\":\"vrng://vrex-next-gen-api.vrexcore.net/vrex/speech/websocket\","
                                "\"urlHf\":\"ws://voiceserver.com/voice/hf\","
                                "\"urlMicTap\":\"ws://voiceserver.com/voice/mictap\","
                                "\"enable\":true,"
                                "\"prv\":true,"
                                "\"wwFeedback\":false,"
                                "\"ptt\":{\"enable\":false},"
                                "\"ff\":{\"enable\":false},"
                                "\"mic\":{\"enable\":false}}"),
                                response_));
    EXPECT_EQ(response_, "{\"success\":true}");
}

TEST_F(VoiceControlTest, setVoiceInit)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_VOICE_IARM_CALL_SET_VOICE_INIT), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                strncpy(param->result, "{\"success\":true}", sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("setVoiceInit"),
                                _T("{\"capabilities\":[\"PRV\"],\"language\":\"eng-USA\"}"),
                                response_));
    EXPECT_EQ(response_, "{\"success\":true}");
}

TEST_F(VoiceControlTest, sendVoiceMessage)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_VOICE_IARM_CALL_SEND_VOICE_MESSAGE), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                strncpy(param->result, "{\"success\":true}", sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("sendVoiceMessage"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"success\":true}");
}

TEST_F(VoiceControlTest, voiceSessionTypes)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_VOICE_IARM_CALL_SESSION_TYPES), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                const string result = "{\"types\":[\"ptt_transcription\"],\"success\":true}";
                strncpy(param->result, result.c_str(), sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("voiceSessionTypes"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"types\":[\"ptt_transcription\"],\"success\":true}");
}

TEST_F(VoiceControlTest, voiceSessionByTextWithInvalidType)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_VOICE_IARM_CALL_SESSION_REQUEST), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                EXPECT_THAT(param->payload, ContainsRegex(_T("\"type\":\"\"")));
                strncpy(param->result, "{\"success\":true}", sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                 _T("voiceSessionByText"),
                                 _T("{\"transcription\":\"Watch Comedy Central\","
                                 "\"type\":\"null\"}"),
                                 response_));
}

TEST_F(VoiceControlTest, voiceSessionByTextWithFfType)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_VOICE_IARM_CALL_SESSION_REQUEST), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                EXPECT_THAT(param->payload, ContainsRegex(_T("\"type\":\"ff_transcription\"")));
                strncpy(param->result, "{\"success\":true}", sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("voiceSessionByText"),
                                _T("{\"transcription\":\"Watch Comedy Central\","
                                "\"type\":\"ff\"}"),
                                response_));
}

TEST_F(VoiceControlTest, voiceSessionByTextWithMicType)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_VOICE_IARM_CALL_SESSION_REQUEST), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                EXPECT_THAT(param->payload, ContainsRegex(_T("\"type\":\"mic_transcription\"")));
                strncpy(param->result, "{\"success\":true}", sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("voiceSessionByText"),
                                _T("{\"transcription\":\"Watch Comedy Central\","
                                "\"type\":\"mic\"}"),
                                response_));
}

TEST_F(VoiceControlTest, voiceSessionByTextWithNoOrPttType)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_VOICE_IARM_CALL_SESSION_REQUEST), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                EXPECT_THAT(param->payload, ::testing::ContainsRegex(_T("\"type\":\"ptt_transcription\"")));
                strncpy(param->result, "{\"success\":true}", sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("voiceSessionByText"),
                                "{\"transcription\":\"Watch Comedy Central\"}",
                                response_));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("voiceSessionByText"),
                                "{\"transcription\":\"Watch Comedy Central\",\"type\":\"ptt\"}",
                                response_));
}

TEST_F(VoiceControlTest, voiceSessionRequest)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_VOICE_IARM_CALL_SESSION_REQUEST), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                EXPECT_THAT(param->payload, ::testing::ContainsRegex(_T("\"transcription\":\"Watch Comedy Central\"")));
                strncpy(param->result, "{\"success\":true}", sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("voiceSessionByText"),
                                "{\"transcription\":\"Watch Comedy Central\",\"type\":\"ptt\"}",
                                response_));
    EXPECT_EQ(response_, "{\"success\":true}");
}

TEST_F(VoiceControlTest, voiceSessionTerminate)
{
    ON_CALL(*p_iarmBusImplMock, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_VOICE_IARM_CALL_SESSION_TERMINATE), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                strncpy(param->result, "{\"success\":true}", sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("voiceSessionTerminate"),
                                _T("{\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\"}"),
                                response_));
    EXPECT_EQ(response_, "{\"success\":true}");
}

TEST_F(VoiceControlInitializedEventTest, onSessionBegin)
{
    EXPECT_CALL(service_, Submit(::testing::_, ::testing::_))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.VoiceControl.onSessionBegin\","
                                          "\"params\":{\"remoteId\":1,"
                                          "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                          "\"deviceType\":\"ptt\","
                                          "\"keywordVerification\":true}}")));
                return Core::ERROR_NONE;
            }));

    const string onSessionBeginParams = "{\"remoteId\":1,"
                                        "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                        "\"deviceType\":\"ptt\","
                                        "\"keywordVerification\":true}";
    size_t len = sizeof(ctrlm_voice_iarm_event_json_t) + onSessionBeginParams.length() + 1;
    ctrlm_voice_iarm_event_json_t *eventData = (ctrlm_voice_iarm_event_json_t *) malloc(len);

    memset(eventData, 0, len);
    eventData->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
    sprintf(eventData->payload, "%s", onSessionBeginParams.c_str());

    handler_.Subscribe(0, _T("onSessionBegin"), _T("org.rdk.VoiceControl"), message_);
    voiceEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_SESSION_BEGIN, eventData, len);
    handler_.Unsubscribe(0, _T("onSessionBegin"), _T("org.rdk.VoiceControl"), message_);

    free(eventData);
}

TEST_F(VoiceControlInitializedEventTest, onSessionEnd)
{
    EXPECT_CALL(service_, Submit(::testing::_, ::testing::_))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.VoiceControl.onSessionEnd\","
                                          "\"params\":{\"serverStats\":{\"dnsTime\":1.0,"
                                          "\"serverIp\":\"...\","
                                          "\"connectTime\":1.0},"
                                          "\"remoteId\":1,"
                                          "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                          "\"result\":\"success\","
                                          "\"success\":{\"transcription\":\"Comedy Central\"},"
                                          "\"error\":{\"protocolErrorCode\":200,"
                                          "\"protocolLibraryErrorCode\":\"...\","
                                          "\"serverErrorCode\":1,"
                                          "\"serverErrorString\":\"Error\","
                                          "\"internalErrorCode\":0},"
                                          "\"abort\":{\"reason\":1},"
                                          "\"shortUtterance\":{\"reason\":1}}}")));
                return Core::ERROR_NONE;
            }));

    const string onSessionEndParams = "{\"serverStats\":{\"dnsTime\":1.0,"
                                      "\"serverIp\":\"...\","
                                      "\"connectTime\":1.0},"
                                      "\"remoteId\":1,"
                                      "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                      "\"result\":\"success\","
                                      "\"success\":{\"transcription\":\"Comedy Central\"},"
                                      "\"error\":{\"protocolErrorCode\":200,"
                                      "\"protocolLibraryErrorCode\":\"...\","
                                      "\"serverErrorCode\":1,"
                                      "\"serverErrorString\":\"Error\","
                                      "\"internalErrorCode\":0},"
                                      "\"abort\":{\"reason\":1},"
                                      "\"shortUtterance\":{\"reason\":1}}";
    size_t len = sizeof(ctrlm_voice_iarm_event_json_t) + onSessionEndParams.length() + 1;
    ctrlm_voice_iarm_event_json_t *eventData = (ctrlm_voice_iarm_event_json_t *) malloc(len);

    memset(eventData, 0, len);
    eventData->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
    sprintf(eventData->payload, "%s", onSessionEndParams.c_str());

    handler_.Subscribe(0, _T("onSessionEnd"), _T("org.rdk.VoiceControl"), message_);
    voiceEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_SESSION_END, eventData, len);
    handler_.Unsubscribe(0, _T("onSessionEnd"), _T("org.rdk.VoiceControl"), message_);

    free(eventData);
}

TEST_F(VoiceControlInitializedEventTest, onStreamBegin)
{
    EXPECT_CALL(service_, Submit(::testing::_, ::testing::_))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.VoiceControl.onStreamBegin\","
                                          "\"params\":{\"remoteId\":1,"
                                          "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\"}}")));
                return Core::ERROR_NONE;
            }));

    const string onStreamBeginParams = "{\"remoteId\":1,"
                                       "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\"}";
    size_t len = sizeof(ctrlm_voice_iarm_event_json_t) + onStreamBeginParams.length() + 1;
    ctrlm_voice_iarm_event_json_t *eventData = (ctrlm_voice_iarm_event_json_t *) malloc(len);

    memset(eventData, 0, len);
    eventData->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
    sprintf(eventData->payload, "%s", onStreamBeginParams.c_str());

    handler_.Subscribe(0, _T("onStreamBegin"), _T("org.rdk.VoiceControl"), message_);
    voiceEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_STREAM_BEGIN, eventData, len);
    handler_.Unsubscribe(0, _T("onStreamBegin"), _T("org.rdk.VoiceControl"), message_);

    free(eventData);
}

TEST_F(VoiceControlInitializedEventTest, onStreamEnd)
{
    EXPECT_CALL(service_, Submit(::testing::_, ::testing::_))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.VoiceControl.onStreamEnd\","
                                          "\"params\":{\"remoteId\":1,"
                                          "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                          "\"reason\":0}}")));
                return Core::ERROR_NONE;
            }));

    const string onStreamEndParams = "{\"remoteId\":1,"
                                     "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                     "\"reason\":0}";
    size_t len = sizeof(ctrlm_voice_iarm_event_json_t) + onStreamEndParams.length() + 1;
    ctrlm_voice_iarm_event_json_t *eventData = (ctrlm_voice_iarm_event_json_t *) malloc(len);

    memset(eventData, 0, len);
    eventData->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
    sprintf(eventData->payload, "%s", onStreamEndParams.c_str());

    handler_.Subscribe(0, _T("onStreamEnd"), _T("org.rdk.VoiceControl"), message_);
    voiceEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_STREAM_END, eventData, len);
    handler_.Unsubscribe(0, _T("onStreamEnd"), _T("org.rdk.VoiceControl"), message_);

    free(eventData);
}

TEST_F(VoiceControlInitializedEventTest, onServerMessage)
{
    EXPECT_CALL(service_, Submit(::testing::_, ::testing::_))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.VoiceControl.onServerMessage\","
                                          "\"params\":{\"msgType\":\"ars\","
                                          "\"trx\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                          "\"created\":91890278389232,"
                                          "\"msgPayload\":{}}}")));
                return Core::ERROR_NONE;
            }));

    const string onServerMessageParams = "{\"msgType\":\"ars\","
                                         "\"trx\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                         "\"created\":91890278389232,"
                                         "\"msgPayload\":{}}";
    size_t len = sizeof(ctrlm_voice_iarm_event_json_t) + onServerMessageParams.length() + 1;
    ctrlm_voice_iarm_event_json_t *eventData = (ctrlm_voice_iarm_event_json_t *) malloc(len);

    memset(eventData, 0, len);
    eventData->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
    sprintf(eventData->payload, "%s", onServerMessageParams.c_str());

    handler_.Subscribe(0, _T("onServerMessage"), _T("org.rdk.VoiceControl"), message_);
    voiceEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_SERVER_MESSAGE, eventData, len);
    handler_.Unsubscribe(0, _T("onServerMessage"), _T("org.rdk.VoiceControl"), message_);

    free(eventData);
}

TEST_F(VoiceControlInitializedEventTest, onKeywordVerification)
{
    EXPECT_CALL(service_, Submit(::testing::_, ::testing::_))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.VoiceControl.onKeywordVerification\","
                                          "\"params\":{\"remoteId\":1,"
                                          "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                          "\"verified\":true}}")));
                return Core::ERROR_NONE;
            }));

    const string onKeywordVerParams = "{\"remoteId\":1,"
                                      "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                      "\"verified\":true}";
    size_t len = sizeof(ctrlm_voice_iarm_event_json_t) + onKeywordVerParams.length() + 1;
    ctrlm_voice_iarm_event_json_t *eventData = (ctrlm_voice_iarm_event_json_t *) malloc(len);

    memset(eventData, 0, len);
    eventData->api_revision = CTRLM_VOICE_IARM_BUS_API_REVISION;
    sprintf(eventData->payload, "%s", onKeywordVerParams.c_str());

    handler_.Subscribe(0, _T("onKeywordVerification"), _T("org.rdk.VoiceControl"), message_);
    voiceEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_JSON_KEYWORD_VERIFICATION, eventData, len);
    handler_.Unsubscribe(0, _T("onKeywordVerification"), _T("org.rdk.VoiceControl"), message_);

    free(eventData);
}
