#include <gtest/gtest.h>
#include "TextToSpeech.h"
#include "TextToSpeechImplementation.h"

#include "ServiceMock.h"
#include "COMLinkMock.h"
#include "FactoriesImplementation.h"
#include "WorkerPoolImplementation.h"

using namespace WPEFramework;
using ::testing::Test;
using ::testing::NiceMock;

namespace {
const string config = _T("TextToSpeech");
const string callSign = _T("org.rdk.TextToSpeech");
const string webPrefix = _T("/Service/TextToSpeech");
const string volatilePath = _T("/tmp/");
const string dataPath = _T("/tmp/");
}

class TTSTest : public Test{
protected:
    Core::ProxyType<Plugin::TextToSpeech> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    Core::ProxyType<WorkerPoolImplementation> workerPool;

    TTSTest()
        : plugin(Core::ProxyType<Plugin::TextToSpeech>::Create())
        , handler(*(plugin))
        , connection(1, 0)
        , workerPool(Core::ProxyType<WorkerPoolImplementation>::Create(
            2, Core::Thread::DefaultStackSize(), 16)) {
    }

    virtual ~TTSTest() = default;
};

class TTSInitializedTest : public TTSTest {
protected:
    NiceMock<FactoriesImplementation> factoriesImplementation;
    NiceMock<ServiceMock> service;
    NiceMock<COMLinkMock> comLinkMock;
    PluginHost::IDispatcher* dispatcher;
    Core::ProxyType<Plugin::TextToSpeechImplementation> TextToSpeechImplementation;
    string response;

    TTSInitializedTest() : TTSTest() {
        TextToSpeechImplementation = Core::ProxyType<Plugin::TextToSpeechImplementation>::Create();

        ON_CALL(service, ConfigLine())
            .WillByDefault(::testing::Return("{}"));
        ON_CALL(service, WebPrefix())
            .WillByDefault(::testing::Return(webPrefix));
        ON_CALL(service, VolatilePath())
            .WillByDefault(::testing::Return(volatilePath));
        ON_CALL(service, Callsign())
            .WillByDefault(::testing::Return(callSign));
                ON_CALL(service, DataPath())
            .WillByDefault(::testing::Return(dataPath));
        ON_CALL(service, COMLink())
            .WillByDefault(::testing::Return(&comLinkMock));
        ON_CALL(comLinkMock, Instantiate(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Return(TextToSpeechImplementation));

        PluginHost::IFactories::Assign(&factoriesImplementation);
        Core::IWorkerPool::Assign(&(*workerPool));
        workerPool->Run();

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }

    virtual ~TTSInitializedTest() override {
        plugin.Release();
        TextToSpeechImplementation.Release();
        Core::IWorkerPool::Assign(nullptr);
        workerPool.Release();
        PluginHost::IFactories::Assign(nullptr);
        dispatcher->Deactivate();
        dispatcher->Release();
    }
};

TEST_F(TTSInitializedTest,RegisteredMethods) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("enabletts")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("cancel")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getapiversion")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getspeechstate")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getttsconfiguration")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isspeaking")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isttsenabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("listvoices")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("pause")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("resume")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setttsconfiguration")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("speak")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setACL")));
}

TEST_F(TTSInitializedTest,Test_EnableTTS_Default_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enabletts"), _T("{\"enabletts\": \"true\"}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"TTS_Status\":0")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(TTSInitializedTest,Test_EnableTTS_True_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enabletts"), _T("{\"enabletts\": true }"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isttsenabled"), _T(""), response));
    EXPECT_EQ(response, _T("{\"isenabled\":true,\"TTS_Status\":0,\"success\":true}"));
}

TEST_F(TTSInitializedTest,Test_EnableTTS_False_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enabletts"), _T("{\"enabletts\": false }"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isttsenabled"), _T(""), response));
    EXPECT_EQ(response, _T("{\"isenabled\":false,\"TTS_Status\":0,\"success\":true}"));
}

TEST_F(TTSInitializedTest,Test_GetAPIVersion_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getapiversion"), _T(""), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"version\":1")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(TTSInitializedTest,Test_IsTTSEnabled_Default_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isttsenabled"), _T(""), response));
    EXPECT_EQ(response, _T("{\"isenabled\":false,\"TTS_Status\":0,\"success\":true}"));
}

TEST_F(TTSInitializedTest,Test_IsListVoicesEmpty_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("listvoices"), _T("{\"language\":\"en-US\"}"), response));
    EXPECT_EQ(response, _T("{\"voices\":[],\"TTS_Status\":0,\"success\":true}"));
}

TEST_F(TTSInitializedTest,Test_Speak_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("setttsconfiguration"),
        _T("{\"language\": \"en-US\",\"voice\": \"carol\","
            "\"ttsendpointsecured\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\","
            "\"ttsendpointsecured\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\"}"
        ),
        response
    ));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("speak"), _T("{\"text\": \"speech_123\"}"), response));
    sleep(1);

    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"speechid\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"TTS_Status\":0")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(TTSInitializedTest,Test_IsSpeaking_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isspeaking"), _T("{\"speechid\": 1}"), response));
    EXPECT_EQ(response, _T("{\"speaking\":false,\"TTS_Status\":0,\"success\":true}"));
}

TEST_F(TTSInitializedTest,Test_GetSpeechState_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getspeechstate"), _T("{\"speechid\": 1}"), response));
    EXPECT_EQ(response, _T("{\"speechstate\":3,\"TTS_Status\":0,\"success\":true}"));
}

TEST_F(TTSInitializedTest,Test_Cancel_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("cancel"), _T("{\"speechid\": 1}"), response));
    EXPECT_EQ(response, _T("{\"TTS_Status\":0,\"success\":true}"));
}

TEST_F(TTSInitializedTest,Test_SetTTSConfiguration_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(
        connection,
        _T("setttsconfiguration"),
        _T("{\"language\": \"en-US\",\"voice\": \"carol\","
            "\"ttsendpoint\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\","
            "\"ttsendpointsecured\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\"}"
        ),
        response
    ));

    EXPECT_EQ(response, _T("{\"TTS_Status\":0,\"success\":true}"));
}
TEST_F(TTSInitializedTest, Test_SetTTSConfiguration_Update_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(
        connection,
        _T("setttsconfiguration"),
        _T("{\"language\": \"en-US\",\"voice\": \"carol\","
            "\"ttsendpoint\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\","
            "\"ttsendpointsecured\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\","
            "\"volume\": \"95\",\"primvolduckpercent\": \"50\",\"rate\": \"40\",\"speechrate\":\"medium\"}"
        ),
        response
    ));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getttsconfiguration"), _T(""), response));

    EXPECT_EQ(response,
        _T("{\"ttsendpoint\":\"http:\\/\\/cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net\\/tts\\/v1\\/cdn\\/location?\","
            "\"ttsendpointsecured\":\"http:\\/\\/cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net\\/tts\\/v1\\/cdn\\/location?\","
            "\"language\":\"en-US\",\"voice\":\"carol\",\"speechrate\":\"medium\",\"rate\":40,\"volume\":\"95\","
            "\"TTS_Status\":0,\"success\":true}"
        ));
}
TEST_F(TTSInitializedTest,Test_SetACL_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(
        connection,
        _T("setACL"),
        _T("{\"accesslist\": [{\"method\":\"speak\",\"apps\":[\"WebAPP1\"]}]}"),
        response
    ));

    EXPECT_EQ(response, _T("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(
        connection,
        _T("setttsconfiguration"),
        _T("{\"language\": \"en-US\",\"voice\": \"carol\","
            "\"ttsendpoint\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\","
            "\"ttsendpointsecured\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\"}"
        ),
        response
    ));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("speak"), _T("{\"text\": \"speech_123\",\"callsign\":\"WebAPP1\"}"), response));
    sleep(1);
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"speechid\"")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"TTS_Status\":0")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}

TEST_F(TTSInitializedTest, Test_SetACL_Failure) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(
        connection,
        _T("setACL"),
        _T("{\"accesslist\": [{\"method\":\"speak\",\"apps\":\"WebAPP1\"}]}"),
        response
    ));

    EXPECT_EQ(response, _T("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(
        connection,
        _T("setttsconfiguration"),
        _T("{\"language\": \"en-US\",\"voice\": \"carol\","
            "\"ttsendpoint\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\","
            "\"ttsendpointsecured\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\"}"
        ),
        response
    ));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(
        connection,
        _T("speak"),
        _T("{\"text\": \"speech_123\",\"callsign\":\"WebAPP2\"}"),
        response
    ));

    EXPECT_EQ(response, _T(""));
}
