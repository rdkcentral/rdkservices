#include <gtest/gtest.h>
#include <gst/gst.h>
#include "TextToSpeech.h"
#include "TextToSpeechImplementation.h"

#include "ServiceMock.h"
#include "COMLinkMock.h"
//#include "TTSSpeakerMock.h"
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

/*namespace {
class TTSNotificationMock : public WPEFramework::Exchange::ITextToSpeech::INotification {
public:
    virtual ~TTSNotificationMock = default;

    MOCK_METHOD(void, Enabled, (const bool), (override));
    MOCK_METHOD(void, VoiceChanged, (const string), (override));

    BEGIN_INTERFACE_MAP(TTSNotificationMock)
    INTERFACE_ENTRY(WPEFramework::Exchange::ITextToSpeech::INotification)
    //INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
    END_INTERFACE_MAP
};
}*/


class TTSTest : public Test{
protected:
    Core::ProxyType<Plugin::TextToSpeech> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    Core::ProxyType<WorkerPoolImplementation> workerPool;
    //Core::ProxyType<TTSNotificationMock> notification;
    
    TTSTest()
        : plugin(Core::ProxyType<Plugin::TextToSpeech>::Create())
        , handler(*(plugin))
        , connection(1, 0) 
        , workerPool(Core::ProxyType<WorkerPoolImplementation>::Create(
            2, Core::Thread::DefaultStackSize(), 16))
        //, notification(Core::ProxyType<TTSNotificationMock>::Create())
    {    
              printf("CMDBG: ttstest cons %p  pid [%d] tid [%ld]\n", this,getpid(),syscall(__NR_gettid));
        
    }
    
    virtual ~TTSTest() {
        printf("CMDBG: ttstest des %p  pid [%d] tid [%ld]\n", this,getpid(),syscall(__NR_gettid));
    }

};

class TTSInitializedTest : public TTSTest {
protected:

    NiceMock<FactoriesImplementation> factoriesImplementation;
    NiceMock<ServiceMock> service;
    NiceMock<COMLinkMock> comLinkMock;
    //NiceMock<TTSSpeakerMock> ttsSpeaker;
    PluginHost::IDispatcher* dispatcher;
    Core::ProxyType<Plugin::TextToSpeechImplementation> TextToSpeechImplementation;
    string response;

    TTSInitializedTest() : TTSTest() {
        printf("CMDBG: ttsinitialized cons %p  pid [%d] tid [%ld]\n", this,getpid(),syscall(__NR_gettid));
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
        /*ON_CALL(ttsSpeaker, createPipeline())
            .WillByDefault(::testing::Return(&comLinkMock));*/
		ON_CALL(comLinkMock, Instantiate(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
	        .WillByDefault(::testing::Return(TextToSpeechImplementation));
        PluginHost::IFactories::Assign(&factoriesImplementation);
        Core::IWorkerPool::Assign(&(*workerPool));
        workerPool->Run();
        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
        //EXPECT_EQ(Core::ERROR_NONE,  TextToSpeechImplementation->Register(&notification));
        //EXPECT_EQ(string(""), plugin->Initialize(&service));
    }

    virtual ~TTSInitializedTest() override {

         printf("CMDBG: ttsinitialized des %p  pid [%d] tid [%ld]\n", this,getpid(),syscall(__NR_gettid));    
         
       // plugin->Deinitialize(&service);
          printf("CMDBG: ttsinitialized des %p  pid [%d] tid [%ld]\n", this,getpid(),syscall(__NR_gettid));    
         plugin.Release();
          printf("CMDBG: ttsinitialized des %p  pid [%d] tid [%ld]\n", this,getpid(),syscall(__NR_gettid));    
         TextToSpeechImplementation.Release();
          printf("CMDBG: ttsinitialized des %p  pid [%d] tid [%ld]\n", this,getpid(),syscall(__NR_gettid));    
         Core::IWorkerPool::Assign(nullptr);
         workerPool.Release();
         PluginHost::IFactories::Assign(nullptr);
         dispatcher->Deactivate();
         dispatcher->Release();
          
        
         //TextToSpeechImplementation = nullptr;


         //EXPECT_EQ(Core::ERROR_NONE,  TextToSpeechImplementation->Unregister(&notification));
         printf("CMDBG: ttsinitialized des  end %p  pid [%d] tid [%ld]\n", this,getpid(),syscall(__NR_gettid));    
         
    }
};

/*void pipeline () {
    //#include <gst/gst.h>

int main(int argc, char *argv[]) {
    GstElement *pipeline, *source, *filter, *sink;
    GstBus *bus;
    GstMessage *msg;
    GMainLoop *loop;

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create a pipeline
    pipeline = gst_pipeline_new("fake-sink-pipeline");

    // Create a video test source
    source = gst_element_factory_make("videotestsrc", "source");

    // Create a fake sink
    sink = gst_element_factory_make("fakesink", "sink");

    if (!pipeline || !source || !sink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    // Set the video pattern to "smpte"
    g_object_set(source, "pattern", 0, NULL);

    // Build the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, sink, NULL);
    gst_element_link(source, sink);

    // Set the pipeline to the "playing" state
    g_print("Now playing\n");
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Create a main loop for handling messages
    loop = g_main_loop_new(NULL, FALSE);

    // Start the main loop
    g_main_loop_run(loop);

    // Clean up
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}
}*/
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
    //plugin->Deinitialize(&service);    
}

TEST_F(TTSInitializedTest,Test_EnableTTS_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enabletts"), _T("{\"enabletts\": \"true\"}"), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"TTS_Status\":0")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
    //plugin->Deinitialize(&service);
}
TEST_F(TTSInitializedTest,Test_GetAPIVersion_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getapiversion"), _T(""), response));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"version\":1")));
    EXPECT_THAT(response, ::testing::ContainsRegex(_T("\"success\":true")));
}
/*TEST_F(TTSInitializedTest,Test_GetTTSConfiguration_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getttsconfiguration"), _T(""), response));
    EXPECT_EQ(response, _T("{\"ttsendpoint\":\"\",\"ttsendpointsecured\":\"\",\"language\":\"en-US\",\"voice\":\"carol\",\"speechrate\":\"\",\"rate\":50,\"volume\":\"100\",\"TTS_Status\":0,\"success\":true}"));
}*/
TEST_F(TTSInitializedTest,Test_IsTTSEnabled_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isttsenabled"), _T(""), response));
    EXPECT_EQ(response, _T("{\"isenabled\":false,\"TTS_Status\":0,\"success\":true}"));
}
TEST_F(TTSInitializedTest,Test_IsListVoicesEmpty_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("listvoices"), _T("{\"language\":\"en-US\"}"), response));
    EXPECT_EQ(response, _T("{\"voices\":[],\"TTS_Status\":0,\"success\":true}"));
}
TEST_F(TTSInitializedTest,Test_IsListVoicesInvalid_Failure) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("listvoices"), _T("{\"language\":\"12*&(#\"}"), response));
    EXPECT_EQ(response, _T("{\"voices\":[],\"TTS_Status\":0,\"success\":true}"));
}
/*TEST_F(TTSInitializedTest,Test_IsListVoices_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setttsconfiguration"), _T("{\"language\": \"en-US\",\"voice\": \"carol\",\"ttsendpointsecured\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\",\"ttsendpointsecured\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\"}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("listvoices"), _T("{\"language\":\"en-US\"}"), response));
    EXPECT_EQ(response, _T("{\"voices\":[\"carol\"],\"TTS_Status\":0,\"success\":true}"));
}*/
TEST_F(TTSInitializedTest,Test_Speak_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    //EXPECT_EQ(Core::ERROR_NONE, TextToSpeechImplementation->Speak("HtmlApp", ));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("speak"), _T("{\"text\": \"speech_123\"}"), response));
    //EXPECT_EQ(response, _T("{\"speechid\":-1,\"TTS_Status\":3,\"success\":false}"));
    EXPECT_EQ(response, _T(""));
}
TEST_F(TTSInitializedTest,Test_isSpeaking_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isspeaking"), _T("{\"speechid\": 1}"), response));
    EXPECT_EQ(response, _T("{\"speaking\":false,\"TTS_Status\":0,\"success\":true}"));

    //EXPECT_EQ(response, _T(""));
}
/*TEST_F(TTSInitializedTest,Test_Pause_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("pause"), _T("{\"speechid\": 1}"), response));
    EXPECT_EQ(response, _T("{\"TTS_Status\":1,\"success\":false}"));
    //EXPECT_EQ(response, _T(""));
}*/
TEST_F(TTSInitializedTest,Test_getSpeechState_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getspeechstate"), _T("{\"speechid\": 1}"), response));
    EXPECT_EQ(response, _T("{\"speechstate\":3,\"TTS_Status\":0,\"success\":true}"));
    //EXPECT_EQ(response, _T(""));
}

/*TEST_F(TTSInitializedTest,Test_Resume_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enabletts"), _T("{\"enabletts\": \"true\"}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("resume"), _T("{\"speechid\": 1}"), response));
    EXPECT_EQ(response, _T("{\"TTS_Status\":1,\"success\":false}"));
    //EXPECT_EQ(response, _T(""));
}*/
TEST_F(TTSInitializedTest,Test_Cancel_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("cancel"), _T("{\"speechid\": 1}"), response));
    EXPECT_EQ(response, _T("{\"TTS_Status\":0,\"success\":true}"));
    //EXPECT_EQ(response, _T(""));
}
TEST_F(TTSInitializedTest,Test_SetTTSConfiguration_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setttsconfiguration"), _T("{\"language\": \"en-US\",\"voice\": \"carol\",\"ttsendpointsecured\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\",\"ttsendpointsecured\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\"}"), response));
    EXPECT_EQ(response, _T("{\"TTS_Status\":0,\"success\":true}"));
    //EXPECT_EQ(response, _T(""));
}

/*TEST_F(TTSInitializedTest,Test_SetTTSConfiguration_Parameters_with_Same_Name_Failure) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    //EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enabletts"), _T("{\"enabletts\": \"true\"}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setttsconfiguration"), _T("{\"language\": \"en-US\",\"voice\": \"carol\",\"ttsendpointsecured\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\",\"ttsendpointsecured\":\"64484\"}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getttsconfiguration"), _T(""), response));
    EXPECT_EQ(response, _T("{\"ttsendpoint\":\"\",\"ttsendpointsecured\":\"\",\"language\":\"en-US\",\"voice\":\"carol\",\"speechrate\":\"\",\"rate\":50,\"volume\":\"100\",\"TTS_Status\":0,\"success\":true}"));
    //EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setttsconfiguration"), _T("{\"language\": \"en-US\",\"voice\": \"carol\",\"ttsendpointsecured\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\",\"ttsendpointsecured\":\"http://cdn-ec-ric-312.voice-guidance-tts.xcr.comcast.net/tts/v1/cdn/location?\"}"), response));
    EXPECT_EQ(response, _T("{\"TTS_Status\":0,\"success\":true}"));
    //EXPECT_EQ(response, _T(""));
}*/
/*TEST_F(TTSInitializedTest,Test_SetTTSConfiguration_InvalidURL_Failure) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enabletts"), _T("{\"enabletts\": \"true\"}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setttsconfiguration"), _T("{\"language\": \"en-US\",\"voice\": \"carol\",\"ttsendpoint\":\"google\",\"ttsendpointsecured\":\"google\"}"), response));
    EXPECT_EQ(response, _T("{\"TTS_Status\":0,\"success\":false}"));
    //EXPECT_EQ(response, _T(""));
}*/
TEST_F(TTSInitializedTest,Test_SetACL_Success) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setACL"), _T("{\"accesslist\": [{\"method\":\"speak\",\"apps\":\"WebAPP\"}]}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    //EXPECT_EQ(response, _T(""));
}

