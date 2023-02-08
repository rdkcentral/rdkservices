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

namespace
{
	const string configureVoiceParams  = "{\"urlAll\":\"ws://voiceserver.com/voice/ptt\","
                                         "\"urlPtt\":\"vrng://vrex-next-gen-api.vrexcore.net/vrex/speech/websocket\","
                                         "\"urlHf\":\"ws://voiceserver.com/voice/hf\","
                                         "\"urlMicTap\":\"ws://voiceserver.com/voice/mictap\","
                                         "\"enable\":true,"
                                         "\"prv\":true,"
                                         "\"wwFeedback\":false,"
                                         "\"ptt\":{\"enable\":false},"
                                         "\"ff\":{\"enable\":false},"
                                         "\"mic\":{\"enable\":false}}";
    const string setVoiceInitParams    = "{\"capabilities\":[\"PRV\"],"
									     "\"language\":\"eng-USA\"}";
    const string sessionNoType         = "{\"transcription\":\"Watch Comedy Central\"}";
    const string sessionTypeBad        = "{\"transcription\":\"Watch Comedy Central\","
                                         "\"type\":\"null\"}";
    const string sessionTypePtt        = "{\"transcription\":\"Watch Comedy Central\","
                                         "\"type\":\"ptt\"}";
    const string sessionTypeFf         = "{\"transcription\":\"Watch Comedy Central\","
                                         "\"type\":\"ff\"}";
    const string sessionTypeMic        = "{\"transcription\":\"Watch Comedy Central\","
                                         "\"type\":\"mic\"}";
    const string onSessionBeginParams  = "{\"remoteId\":1,"
                                         "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                         "\"deviceType\":\"ptt\","
                                         "\"keywordVerification\":true}";
    const string onSessionEndParams    = "{\"serverStats\":{\"dnsTime\":1.0,"
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
    const string onStreamBeginParams   = "{\"remoteId\":1,"
                                         "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\"}";
    const string onStreamEndParams     = "{\"remoteId\":1,"
                                         "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                         "\"reason\":0}";
    const string onServerMessageParams = "{\"msgType\":\"ars\","
                                         "\"trx\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                         "\"created\":91890278389232,"
                                         "\"msgPayload\":{}}";
    const string onKeywordVerParams    = "{\"remoteId\":1,"
                                         "\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\","
                                         "\"verified\":true}";
    const string sessionTermParams     = "{\"sessionId\":\"1b11359e-23fe-4f2f-9ba8-cc19b87203cf\"}";
    const string successResponse       = "{\"success\":true}";
    const string voiceStatusResponse   = "{\"maskPii\":true,"
                                         "\"capabilities\":[\"PRV\"],"
                                         "\"urlPtt\":\"proxyUrlPtt\","
                                         "\"urlHf\":\"proxyUrlHf\","
                                         "\"prv\":true,"
                                         "\"wwFeedback\":false,"
                                         "\"ptt\":{\"status\":\"ready\"},"
                                         "\"ff\":{\"status\":\"ready\"},"
                                         "\"mic\":{\"status\":\"ready\"},"
                                         "\"success\":true}";
    const string sessionTypesResponse  = "{\"types\":[\"ptt_transcription\"],"
										"\"success\":true}";
}

class VoiceControlTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::VoiceControl> plugin_;
    Core::JSONRPC::Handler&               handler_;
    Core::JSONRPC::Connection             connection_;
    string                                response_;
    NiceMock<IarmBusImplMock>             iarmBusImplMock_;

    VoiceControlTest()
        : plugin_(Core::ProxyType<Plugin::VoiceControl>::Create())
        , handler_(*plugin_)
        , connection_(1, 0)
    {
        IarmBus::getInstance().impl = &iarmBusImplMock_;
    }

    virtual ~VoiceControlTest() override
    {
        IarmBus::getInstance().impl = nullptr;
    }
};

static ::testing::AssertionResult isValidCtrlmVoiceIarmEvent(IARM_EventId_t ctrlmVoiceIarmEventId)
{
    switch (ctrlmVoiceIarmEventId) {
        case CTRLM_VOICE_IARM_EVENT_JSON_SESSION_BEGIN:
        case CTRLM_VOICE_IARM_EVENT_JSON_STREAM_BEGIN:
        case CTRLM_VOICE_IARM_EVENT_JSON_KEYWORD_VERIFICATION:
        case CTRLM_VOICE_IARM_EVENT_JSON_SERVER_MESSAGE:
        case CTRLM_VOICE_IARM_EVENT_JSON_STREAM_END:
        case CTRLM_VOICE_IARM_EVENT_JSON_SESSION_END:
            return ::testing::AssertionSuccess();
        default:
            return ::testing::AssertionFailure();
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
        ON_CALL(iarmBusImplMock_, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                    EXPECT_TRUE(isValidCtrlmVoiceIarmEvent(eventId));
                    voiceEventHandler_ = handler;
                    return IARM_RESULT_SUCCESS;
                }));
        
        ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
            .WillByDefault(
                [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                    EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                    EXPECT_EQ(string(methodName), string(CTRLM_VOICE_IARM_CALL_STATUS));
                    auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);

                    strncpy(param->result, voiceStatusResponse.c_str(), sizeof(param->result));
                    return IARM_RESULT_SUCCESS;
                });

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
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_EQ(string(methodName), string(CTRLM_VOICE_IARM_CALL_STATUS));
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                strncpy(param->result, voiceStatusResponse.c_str(), sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("voiceStatus"), _T("{}"), response_));
    EXPECT_EQ(response_, voiceStatusResponse);
}

TEST_F(VoiceControlTest, configureVoice)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_EQ(string(methodName), string(CTRLM_VOICE_IARM_CALL_CONFIGURE_VOICE));
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                strncpy(param->result, successResponse.c_str(), sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("configureVoice"), _T(configureVoiceParams), response_));
    EXPECT_EQ(response_, successResponse);
}

TEST_F(VoiceControlTest, setVoiceInit)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_EQ(string(methodName), string(CTRLM_VOICE_IARM_CALL_SET_VOICE_INIT));
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                strncpy(param->result, successResponse.c_str(), sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("setVoiceInit"), _T(setVoiceInitParams), response_));
    EXPECT_EQ(response_, successResponse);
}

TEST_F(VoiceControlTest, sendVoiceMessage)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_EQ(string(methodName), string(CTRLM_VOICE_IARM_CALL_SEND_VOICE_MESSAGE));
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                strncpy(param->result, successResponse.c_str(), sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("sendVoiceMessage"), _T("{}"), response_));
    EXPECT_EQ(response_, successResponse);
}

TEST_F(VoiceControlTest, voiceSessionTypes)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_EQ(string(methodName), string(CTRLM_VOICE_IARM_CALL_SESSION_TYPES));
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                strncpy(param->result, sessionTypesResponse.c_str(), sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("voiceSessionTypes"), _T("{}"), response_));
    EXPECT_EQ(response_, sessionTypesResponse);
}

TEST_F(VoiceControlTest, voiceSessionByTextWithInvalidType)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_EQ(string(methodName), string(CTRLM_VOICE_IARM_CALL_SESSION_REQUEST));
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                EXPECT_THAT(param->payload, ::testing::ContainsRegex(_T("\"type\":\"\"")));
                strncpy(param->result, successResponse.c_str(), sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("voiceSessionByText"), sessionTypeBad, response_));
}

TEST_F(VoiceControlTest, voiceSessionByTextWithFfType)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_EQ(string(methodName), string(CTRLM_VOICE_IARM_CALL_SESSION_REQUEST));
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                EXPECT_THAT(param->payload, ::testing::ContainsRegex(_T("\"type\":\"ff_transcription\"")));
                strncpy(param->result, successResponse.c_str(), sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("voiceSessionByText"), sessionTypeFf, response_));
}

TEST_F(VoiceControlTest, voiceSessionByTextWithMicType)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_EQ(string(methodName), string(CTRLM_VOICE_IARM_CALL_SESSION_REQUEST));
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                EXPECT_THAT(param->payload, ::testing::ContainsRegex(_T("\"type\":\"mic_transcription\"")));
                strncpy(param->result, successResponse.c_str(), sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("voiceSessionByText"), sessionTypeMic, response_));
}

TEST_F(VoiceControlTest, voiceSessionByTextWithNoOrPttType)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_EQ(string(methodName), string(CTRLM_VOICE_IARM_CALL_SESSION_REQUEST));
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                EXPECT_THAT(param->payload, ::testing::ContainsRegex(_T("\"type\":\"ptt_transcription\"")));
                strncpy(param->result, successResponse.c_str(), sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("voiceSessionByText"), sessionNoType,  response_));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("voiceSessionByText"), sessionTypePtt, response_));
}

TEST_F(VoiceControlTest, voiceSessionRequest)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_EQ(string(methodName), string(CTRLM_VOICE_IARM_CALL_SESSION_REQUEST));
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                EXPECT_THAT(param->payload, ::testing::ContainsRegex(_T("\"transcription\":\"Watch Comedy Central\"")));
                strncpy(param->result, successResponse.c_str(), sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("voiceSessionRequest"), sessionTypePtt, response_));
    EXPECT_EQ(response_, successResponse);
}

TEST_F(VoiceControlTest, voiceSessionTerminate)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_EQ(string(methodName), string(CTRLM_VOICE_IARM_CALL_SESSION_TERMINATE));
                auto param = static_cast<ctrlm_voice_iarm_call_json_t*>(arg);
                strncpy(param->result, successResponse.c_str(), sizeof(param->result));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("voiceSessionTerminate"), _T(sessionTermParams), response_));
    EXPECT_EQ(response_, successResponse);
}

TEST_F(VoiceControlInitializedEventTest, onSessionBegin)
{
    EXPECT_CALL(service_, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.VoiceControl.onSessionBegin\","
                                          "\"params\":")) + onSessionBeginParams + "}");
                return Core::ERROR_NONE;
            }));

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
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
										  "\"method\":\"org.rdk.VoiceControl.onSessionEnd\","
                                          "\"params\":")) + onSessionEndParams + "}");
                return Core::ERROR_NONE;
            }));

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
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
										  "\"method\":\"org.rdk.VoiceControl.onStreamBegin\","
                                          "\"params\":")) + onStreamBeginParams + "}");
                return Core::ERROR_NONE;
            }));

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
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
										  "\"method\":\"org.rdk.VoiceControl.onStreamEnd\","
                                          "\"params\":")) + onStreamEndParams + "}");
                return Core::ERROR_NONE;
            }));

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
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
										  "\"method\":\"org.rdk.VoiceControl.onServerMessage\","
                                          "\"params\":")) + onServerMessageParams + "}");
                return Core::ERROR_NONE;
            }));

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
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
										  "\"method\":\"org.rdk.VoiceControl.onKeywordVerification\","
                                          "\"params\":")) + onKeywordVerParams + "}");
                return Core::ERROR_NONE;
            }));

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
