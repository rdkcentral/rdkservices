#include <gtest/gtest.h>
#include "SystemAudioPlayer.h"
#include "SystemAudioPlayerImplementation.h"
#include "ServiceMock.h"
#include "COMLinkMock.h"
#include "FactoriesImplementation.h"
#include "WorkerPoolImplementation.h"

using namespace WPEFramework;
using ::testing::Test;
using ::testing::NiceMock;

namespace {
const string config = _T("SystemAudioPlayer");
const string callSign = _T("org.rdk.SystemAudioPlayer");
const string webPrefix = _T("/Service/SystemAudioPlayer");
const string volatilePath = _T("/tmp/");
const string dataPath = _T("/tmp/");
}

class SAPTest : public Test {
protected:
    Core::ProxyType<Plugin::SystemAudioPlayer> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    Core::ProxyType<WorkerPoolImplementation> workerPool;

    SAPTest()
        : plugin(Core::ProxyType<Plugin::SystemAudioPlayer>::Create())
        , handler(*(plugin))
        , connection(1, 0)
        , workerPool(Core::ProxyType<WorkerPoolImplementation>::Create(
            2, Core::Thread::DefaultStackSize(), 16)) {
    }

    virtual ~SAPTest() = default;
};

class SAPInitializedTest : public SAPTest {
protected:
    NiceMock<FactoriesImplementation> factoriesImplementation;
    NiceMock<ServiceMock> service;
    NiceMock<COMLinkMock> comLinkMock;
    PluginHost::IDispatcher* dispatcher;
    Core::ProxyType<Plugin::SystemAudioPlayerImplementation> SystemAudioPlayerImplementation;
    string response;

    SAPInitializedTest() : SAPTest() {
        SystemAudioPlayerImplementation = Core::ProxyType<Plugin::SystemAudioPlayerImplementation>::Create();

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
            .WillByDefault(::testing::Return(SystemAudioPlayerImplementation));

        PluginHost::IFactories::Assign(&factoriesImplementation);
        Core::IWorkerPool::Assign(&(*workerPool));
        workerPool->Run();

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }

    virtual ~SAPInitializedTest() override {
        plugin.Release();
        SystemAudioPlayerImplementation.Release();
        Core::IWorkerPool::Assign(nullptr);
        workerPool.Release();
        PluginHost::IFactories::Assign(nullptr);
        dispatcher->Deactivate();
        dispatcher->Release();
    }
};

TEST_F(SAPInitializedTest,RegisteredMethods) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("close")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("config")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPlayerSessionId")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isspeaking")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("open")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("pause")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("play")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("playbuffer")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("resume")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setMixerLevels")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setSmartVolControl")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("stop")));
}

/*******************************************************************************************************************
 * Test function for open
 * Open                    :
 *                Opens a player instance and assigns it a unique ID
 *
 *                @return Response object contains id and success status
 * Use case coverage:
 *                @Success : 24
 *                @Failure : 3
 ********************************************************************************************************************/

/**
 * @name  : SAPOpenPCMWebsocketSystem
 * @brief : Open a player instance with pcm, websocket, system 
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:1, success: true}
 */

TEST_F(SAPInitializedTest, SAPOpenPCMWebsocketSystem) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":1,\"success\":true}"));
}

/**
 * @name  : SAPOpenPCMDataSystem
 * @brief : Open a player instance with pcm, websocket, app
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:3, success: true}
 */

TEST_F(SAPInitializedTest,SAPOpenPCMDataSystem) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"data\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":2,\"success\":true}"));
}

/**
 * @name  : SAPOpenPCMWebsocketApp
 * @brief : Open a player instance with pcm, websocket, app
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:3, success: true}
 */

TEST_F(SAPInitializedTest,SAPOpenPCMWebsocketApp) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"app\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":3,\"success\":true}"));
}

/**
 * @name  : SAPOpenPCMDataApp
 * @brief : Open a player instance with pcm, data, app
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:4, success: true}
 */

TEST_F(SAPInitializedTest,SAPOpenPCMDataApp) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"data\",\"playmode\": \"app\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":4,\"success\":true}"));
}

/**
 * @name  : SAPOpenPCMFilesrcSystem
 * @brief : Open a player instance with pcm, filesrc, systen
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:5, success: true}
 */

TEST_F(SAPInitializedTest,SAPOpenPCMFilesrcSystem) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"filesrc\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":5,\"success\":true}"));
}

/**
 * @name  : SAPOpenPCMFilesrcApp
 * @brief : Open a player instance with pcm, filesrc, app
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:6, success: true}
 */

TEST_F(SAPInitializedTest,SAPOpenPCMFilesrcApp) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"filesrc\",\"playmode\": \"app\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":6,\"success\":true}"));
}

/**
 * @name  : SAPOpenPCMHttpsrcSystem
 * @brief : Open a player instance with pcm, httpsrc, system
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:7, success: true}
 */

TEST_F(SAPInitializedTest,SAPOpenPCMHttpsrcSystem) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"httpsrc\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":7,\"success\":true}"));
}

/**
 * @name  : SAPOpenPCMHttpsrcApp
 * @brief : Open a player instance with pcm, httpsrc, app
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:8, success: true}
 */

TEST_F(SAPInitializedTest,SAPOpenPCMHttpsrcApp) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"httpsrc\",\"playmode\": \"app\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":8,\"success\":true}"));
}

/**
 * @name  : SAPMp3WebsocketSystem
 * @brief : Open a player instance with mp3, websocket, system
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:9, success: true}
 */

TEST_F(SAPInitializedTest,SAPMp3WebsocketSystem) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"mp3\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":9,\"success\":true}"));
}

/**
 * @name  : SAPMp3WebsocketApp
 * @brief : Open a player instance with mp3, websocket, app
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:10, success: true}
 */

TEST_F(SAPInitializedTest,SAPMp3WebsocketApp) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"mp3\",\"sourcetype\": \"websocket\",\"playmode\": \"app\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":10,\"success\":true}"));
}

/**
 * @name  : SAPMp3DataSystem
 * @brief : Open a player instance with mp3, data, system
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:11, success: true}
 */

TEST_F(SAPInitializedTest,SAPMp3DataSystem) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"mp3\",\"sourcetype\": \"data\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":11,\"success\":true}"));
}

/**
 * @name  : SAPMp3DataApp
 * @brief : Open a player instance with mp3, data, app
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:12, success: true}
 */

TEST_F(SAPInitializedTest,SAPMp3DataApp) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"mp3\",\"sourcetype\": \"data\",\"playmode\": \"app\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":12,\"success\":true}"));
}

/**
 * @name  : SAPMp3FilesrcSystem
 * @brief : Open a player instance with mp3, filesrc, system
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:13, success: true}
 */

TEST_F(SAPInitializedTest,SAPMp3FilesrcSystem) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"mp3\",\"sourcetype\": \"filesrc\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":13,\"success\":true}"));
}

/**
 * @name  : SAPMp3FilesrcApp
 * @brief : Open a player instance with mp3, filesrc, app
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:14, success: true}
 */

TEST_F(SAPInitializedTest,SAPMp3FilesrcApp) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"mp3\",\"sourcetype\": \"filesrc\",\"playmode\": \"app\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":14,\"success\":true}"));
}

/**
 * @name  : SAPMp3HttpsrcSystem
 * @brief : Open a player instance with mp3, httpsrc, system
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:15, success: true}
 */

TEST_F(SAPInitializedTest,SAPMp3HttpsrcSystem) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"mp3\",\"sourcetype\": \"httpsrc\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":15,\"success\":true}"));
}

/**
 * @name  : SAPMp3HttpsrcApp
 * @brief : Open a player instance with mp3, httpsrc, app
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:16, success: true}
 */

TEST_F(SAPInitializedTest,SAPMp3HttpsrcApp) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"mp3\",\"sourcetype\": \"httpsrc\",\"playmode\": \"app\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":16,\"success\":true}"));
}

/**
 * @name  : SAPWavWebsocketSystem
 * @brief : Open a player instance with wav, websocket, system
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:17, success: true}
 */

TEST_F(SAPInitializedTest,SAPWavWebsocketSystem) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"wav\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":17,\"success\":true}"));
}

/**
 * @name  : SAPWavWebsocketApp
 * @brief : Open a player instance with wav, websocket, app
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:18, success: true}
 */

TEST_F(SAPInitializedTest,SAPWavWebsocketApp) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"wav\",\"sourcetype\": \"websocket\",\"playmode\": \"app\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":18,\"success\":true}"));
}

/**
 * @name  : SAPWavDataSystem
 * @brief : Open a player instance with wav, data, system
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:19, success: true}
 */

TEST_F(SAPInitializedTest,SAPWavDataSystem) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"wav\",\"sourcetype\": \"data\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":19,\"success\":true}"));
}

/**
 * @name  : SAPWavDataApp
 * @brief : Open a player instance with wav, data, app
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:20, success: true}
 */

TEST_F(SAPInitializedTest,SAPWavDataApp) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"wav\",\"sourcetype\": \"data\",\"playmode\": \"app\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":20,\"success\":true}"));
}

/**
 * @name  : SAPWavFileSrcSystem
 * @brief : Open a player instance with wav, filesrc, system
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:21, success: true}
 */

TEST_F(SAPInitializedTest,SAPWavFileSrcSystem) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"wav\",\"sourcetype\": \"filesrc\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":21,\"success\":true}"));
}

/**
 * @name  : SAPWavFileSrcApp
 * @brief : Open a player instance with wav, filesrc, app
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:22, success: true}
 */

TEST_F(SAPInitializedTest,SAPWavFileSrcApp) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"wav\",\"sourcetype\": \"filesrc\",\"playmode\": \"app\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":22,\"success\":true}"));
}

/**
 * @name  : SAPWavHttpSrcSystem
 * @brief : Open a player instance with wav, httpsrc, system
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:23, success: true}
 */

TEST_F(SAPInitializedTest,SAPWavHttpSrcSystem) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"wav\",\"sourcetype\": \"httpsrc\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":23,\"success\":true}"));
}

/**
 * @name  : SAPWavHttpSrcApp
 * @brief : Open a player instance with wav, httpsrc, app
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {id:24, success: true}
 */

TEST_F(SAPInitializedTest,SAPWavHttpSrcApp) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"wav\",\"sourcetype\": \"httpsrc\",\"playmode\": \"app\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"id\":24,\"success\":true}"));
}

/**
 * @name  : SAPOpenInvalidAudioType
 * @brief : Given InvalidAudioType It should return false
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest,SAPOpenInvalidAudioType) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"invalid\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

/**
 * @name  : SAPOpenInvalidSourceType
 * @brief : Given InvalidSourceType It should return false
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest,SAPOpenInvalidSourceType) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"invalid\",\"playmode\": \"system\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

/**
 * @name  : SAPOpenInvalidPlayMode
 * @brief : Given InvalidPlayMode It should return false
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest,SAPOpenInvalidPlayMode) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"invalid\" }"),
         response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}
/*******************************************************************************************************************
 * Test function for config
 * Open                    :
 *                Configures playback for a PCM audio source (audio/x-raw) on the specified player
 *
 *                @return Response object success status
 * Use case coverage:
 *                @Success : 1
 *                @Failure : 2
 ********************************************************************************************************************/
/**
 * @name  : SAPConfig
 * @brief : Sets a config for PCM audio playback
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {success: true}
 */

TEST_F(SAPInitializedTest, SAPConfig) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"filesrc\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("config"),
        _T("{\"id\": ") +
            std::to_string(speechId) +
            _T(", \"pcmconfig\": {\"format\": \"S16LE\",\"rate\": \"22050\",\"channels\": \"1\",\"layout\": \"interleaved\"},")
            _T("\"websocketsecparam\": {\"cafilenames\": [{\"cafilename\": \"/etc/ssl/certs/Xfinity_Subscriber_ECC_Root.pem\"}],")
            _T("\"certfilename\": \"...\",\"keyfilename\": \"...\"}}"),
        response
        ));

        EXPECT_EQ(response, _T("{\"success\":true}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPConfigForOtherAudioType
 * @brief : Sets a config for MP3 audio playback
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPConfigForOtherAudioType) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"mp3\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("config"),
        _T("{\"id\": ") +
            std::to_string(speechId) +
            _T(", \"pcmconfig\": {\"format\": \"S16LE\",\"rate\": \"22050\",\"channels\": \"1\",\"layout\": \"interleaved\"},")
            _T("\"websocketsecparam\": {\"cafilenames\": [{\"cafilename\": \"/etc/ssl/certs/Xfinity_Subscriber_ECC_Root.pem\"}],")
            _T("\"certfilename\": \"...\",\"keyfilename\": \"...\"}}"),
        response
        ));

        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPConfigWithoutOpen
 * @brief : Sets a config without player instance
 * 
 * @param[in]   :  audiotype ,sourcetype, playmode
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest,SAPConfigWithoutOpen) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
    _T("config"),
    _T("{\"id\": 1, "
       "\"pcmconfig\": {\"format\": \"S16LE\", \"rate\": \"22050\", \"channels\": \"1\", \"layout\": \"interleaved\"}, "
       "\"websocketsecparam\": {\"cafilenames\": [{\"cafilename\": \"/etc/ssl/certs/Xfinity_Subscriber_ECC_Root.pem\"}], "
       "\"certfilename\": \"...\", \"keyfilename\": \"...\"}}"),
    response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

/*******************************************************************************************************************
 * Test function for play
 * Open                    :
 *                Plays audio on the specified player.
 *
 *                @return Response object success status
 * Use case coverage:
 *                @Success : 4
 *                @Failure : 3
 ********************************************************************************************************************/

/**
 * @name  : SAPPlayWebsocket
 * @brief : plays a player instance
 * 
 * @param[in]   :  id ,url 
 * @return      :  {success: true}
 */

TEST_F(SAPInitializedTest, SAPPlayWebsocket) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart); 
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("play"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(", \"url\": \"ws://example.com/socket\" }"),
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":true}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPPlayDataSrc
 * @brief : plays a player instance
 * 
 * @param[in]   :  id ,url 
 * @return      :  {success: true}
 */

TEST_F(SAPInitializedTest, SAPPlayDataSrc) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"data\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("play"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(", \"url\": \"data://example/data\" }"),
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":true}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPPlayFilesrc
 * @brief : Opens a player instance with source:filesrc but given http url
 * 
 * @param[in]   :  id ,url 
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPPlayFilesrc) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"wav\",\"sourcetype\": \"filesrc\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("play"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(", \"url\": \"file:///example/data\" }"),
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":true}"));
        sleep(1);
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPPlayHttpsrc
 * @brief : Opens a player instance with source:httpsrc but given http url
 * 
 * @param[in]   :  id ,url 
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPPlayHttpsrc) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"httpsrc\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("play"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(", \"url\": \"http://example/data\" }"),
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":true}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPPlayDifferentSrcUrl
 * @brief : Opens a player instance with source:httpsrc but given http url
 * 
 * @param[in]   :  id ,url 
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPPlayDifferentSrcUrl) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"httpsrc\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("play"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(", \"url\": \"file:///example/data\" }"),
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPPlayWithoutOpen
 * @brief : Sets a play without player instance
 * 
 * @param[in]   :  id ,url
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest,SAPPlayWithoutOpen) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, \
        _T("play"),
        _T("{\"id\": 1, \"url\": \"https://example/data\"}"), 
        response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

/**
 * @name  : SAPPlayInvalidUrl
 * @brief : Play with invalid url
 * 
 * @param[in]   :  id , url
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPPlayInvalidUrl) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));   
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, 
        _T("play"),
        _T("{\"id\": 1,\"url\": \"invalid\" }"), 
        response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}")); 
}

/*******************************************************************************************************************
 * Test function for close
 * Close                    :
 *                Closes the specified Player
 *
 *                @return Response object success status
 * Use case coverage:
 *                @Success : 1
 *                @Failure : 1
 ********************************************************************************************************************/

/**
 * @name  : SAPClose
 * @brief : Closes the player instance
 * 
 * @param[in]   :  id
 * @return      :  {success: true}
 */

TEST_F(SAPInitializedTest, SAPClose) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, 
            _T("close"),
            _T("{\"id\": ") + std::to_string(speechId) + _T("}"), 
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":true}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPCloseWithoutOpen
 * @brief : Close the player Which is not opened
 * 
 * @param[in]   :  id
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest,SAPCloseWithoutOpen) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("close"),
        _T("{\"id\": 1}"),
        response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

/*******************************************************************************************************************
 * Test function for resume
 * Resume                    :
 *                Resumes the specified Player
 *
 *                @return Response object success status
 * Use case coverage:
 *                @Success : 1
 *                @Failure : 1
 ********************************************************************************************************************/

/**
 * @name  : SAPResume
 * @brief : Resume the player instance
 * 
 * @param[in]   :  id
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPResume) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, 
            _T("resume"),
            _T("{\"id\": ") + std::to_string(speechId) + _T("}"), 
            response\
        ));
        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPResumeWithOutOpen
 * @brief : Resume the invalid player
 * 
 * @param[in]   :  id
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest,SAPResumeWithOutOpen) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("resume"),
        _T("{\"id\": 1}"),
        response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

/*******************************************************************************************************************
 * Test function for pause
 * Pause                   :
 *                Closes the specified Player
 *
 *                @return Response object success status
 * Use case coverage:
 *                @Success : 1
 *                @Failure : 1
 ********************************************************************************************************************/

/**
 * @name  : SAPPause
 * @brief : Pause the player instance
 * 
 * @param[in]   :  id
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPPause) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, 
            _T("pause"),
            _T("{\"id\": ") + std::to_string(speechId) + _T("}"), 
            response\
        ));
        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPPauseWithOutOpen
 * @brief : Pause the invalid player
 * 
 * @param[in]   :  id
 * @return      :  {success: false}
 */
TEST_F(SAPInitializedTest,SAPPauseWithoutOpen) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("pause"),
        _T("{\"id\": 1}"),
        response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

/*******************************************************************************************************************
 * Test function for stop
 * Stop                  :
 *                stop the specified Players connection with source
 *
 *                @return Response object success status
 * Use case coverage:
 *                @Success : 1
 *                @Failure : 1
 ********************************************************************************************************************/

/**
 * @name  : SAPStop
 * @brief : Stop the connection between websocket
 * 
 * @param[in]   :  id
 * @return      :  {success: true}
 */

TEST_F(SAPInitializedTest, SAPStop) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, 
            _T("stop"),
            _T("{\"id\": ") + std::to_string(speechId) + _T("}"), 
            response\
        ));
        EXPECT_EQ(response, _T("{\"success\":true}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPStopWithOutOpen
 * @brief : Stop the invalid player
 * 
 * @param[in]   :  id
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest,SAPStopWithoutOpen) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("stop"),
        _T("{\"id\": 1}"),
        response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

/*******************************************************************************************************************
 * Test function for isSpeaking
 * isSpeaking                 :
 *                Checks the player instance is speaking or not
 *
 *                @return Response object success status
 * Use case coverage:
 *                @Success : 1
 *                @Failure : 1
 ********************************************************************************************************************/

/**
 * @name  : SAPIsSpeaking
 * @brief : checks the player instance is speaking or not
 * 
 * @param[in]   :  id
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPIsSpeaking) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("isspeaking"),
            _T("{\"id\": ") + std::to_string(speechId) + _T("}"), 
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPisSpeakingWithoutOpen
 * @brief : Check isSpeaking the invalid player
 * 
 * @param[in]   :  id
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest,SAPisSpeakingWithoutOpen) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("isspeaking"),
        _T("{\"id\": 1}"),
        response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

/*******************************************************************************************************************
 * Test function for playbuffer
 * PlayBuffer                 :
 *                Buffers the audio playback on the specified player
 *
 *                @return Response object success status
 * Use case coverage:
 *                @Success : 1
 *                @Failure : 1
 ********************************************************************************************************************/
/**
 * @name  : SAPPlayBuffer
 * @brief : Buffers the audio playback on the specified player.
 * 
 * @param[in]   :  id , data
 * @return      :  {success: true}
 */

TEST_F(SAPInitializedTest, SAPPlayBuffer) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("playbuffer"), 
            _T("{\"id\": ") + std::to_string(speechId) + _T(",\"data\": \"180\"}"),
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":true}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPPlayBufferWithoutOpen
 * @brief : Buffer on invalid player
 * 
 * @param[in]   :  id , data
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest,SAPPlayBufferWithoutOpen) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("playbuffer"),
        _T("{\"id\": 1 , \"data\": \"180\"}"),
        response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

/*******************************************************************************************************************
 * Test function for setMixerLevel
 * setMixerLevel                 :
 *                Sets the audio level on the specified player. The SystemAudioPlayer plugin can control the volume of the content being played back as well as the primary program audio; 
 *                thus, an application can duck down the volume on the primary program
 *                audio when system audio is played and then restore it back when the system audio playback is complete.
 *
 *                @return Response object success status
 * Use case coverage:
 *                @Success : 1
 *                @Failure : 5
 ********************************************************************************************************************/

/**
 * @name  : SAPSetMixerLevel
 * @brief : SettingMixerlevels for Player
 * 
 * @param[in]   :  id , primaryVolume , player volume
 * @return      :  {success: true}
 */

TEST_F(SAPInitializedTest, SAPSetMixerLevel) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("setMixerLevels"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(",\"primaryVolume\": \"80\" ,\"playerVolume\": \"70\"}"),
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":true}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPSetMixerLevelPriVolLessThanZero
 * @brief : Set Primary Volume less than zero and expects failure
 * 
 * @param[in]   :  id , primaryVolume , player volume
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPSetMixerLevelPriVolLessThanZero) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("setMixerLevels"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(",\"primaryVolume\": \"-1\" ,\"playerVolume\": \"70\"}"),
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPSetMixerLevelPlayerVolLessThanZero
 * @brief : Set Player Volume less than zero and expects failure
 * 
 * @param[in]   :  id , primaryVolume , player volume
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPSetMixerLevelPlayerVolLessThanZero) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

  size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("setMixerLevels"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(",\"primaryVolume\": \"80\" ,\"playerVolume\": \"-1\"}"),
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPSetMixerLevelPriVolGreaterThan100
 * @brief : Set Primary Volume Greater than 100 and expects failure
 * 
 * @param[in]   :  id , primaryVolume , player volume
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPSetMixerLevelPriVolGreaterThan100) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

  size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("setMixerLevels"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(",\"primaryVolume\": \"101\" ,\"playerVolume\": \"70\"}"),
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPSetMixerLevelPlayerVolGreaterThan100
 * @brief : Set Player Volume Greater than 100 and expects failure
 * 
 * @param[in]   :  id , primaryVolume , player volume
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPSetMixerLevelPlayerVolGreaterThan100) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

  size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("setMixerLevels"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(",\"primaryVolume\": \"80\" ,\"playerVolume\": \"101\"}"),
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPSetMixerLevelsWithoutOpen
 * @brief : SettingMixerlevels for invalidPlayer
 * 
 * @param[in]   :  id , primaryVolume , player volume
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest,SAPSetMixerLevelsWithoutOpen) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("setMixerLevels"),
        _T("{\"id\": 1 , \"primaryVolume\": \"180\" ,\"playerVolume\": \"7\" }"), 
        response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

/*******************************************************************************************************************
 * Test function for SetSmartVolControl
 * SetSmartVolControl                 :
 *                Sets the smart volume audio control on the specified player. 
 *                The plugin can control the smart volume of the content being played back as well as the primary program audio; 
 *                thus, an application can duck down the volume on the primary program audio when system audio is played 
 *                and then restore it back when the system audio playback is complete.
 *
 *                @return Response object success status
 * Use case coverage:
 *                @Success : 1
 *                @Failure : 5
 ********************************************************************************************************************/
/**
 * @name  : SAPSetSmartVolControl
 * @brief : SettingSAPSetSmartVolControl for Player
 * 
 * @param[in]   :  id, enable, playerAudioLevelThreshold, playerDetectTimeMs, playerHoldTimeMs, primaryDuckingPercent
 * @return      :  {success: true}
 */

TEST_F(SAPInitializedTest, SAPSetSmartVolControl) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("setSmartVolControl"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(",\"enable\": true, \"playerAudioLevelThreshold\": 0.1, \"playerDetectTimeMs\": 200, \"playerHoldTimeMs\": 1000, \"primaryDuckingPercent\": 1 }"), 
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":true}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPSetSmartVolControlThresholdLessThanZero
 * @brief : Set Thresholdvaluse < 0 which result in failure
 * 
 * @param[in]   :  threshold = -1 
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPSetSmartVolControlThresholdLessThanZero) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("setSmartVolControl"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(",\"enable\": true, \"playerAudioLevelThreshold\": -1, \"playerDetectTimeMs\": 200, \"playerHoldTimeMs\": 1000, \"primaryDuckingPercent\": 1 }"), 
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPSetSmartVolControlDetectTimeMsLessThanZero
 * @brief : Set detectTimeMs < 0 which result in failure
 * 
 * @param[in]   :   detectTimeMs = -1 
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPSetSmartVolControlDetectTimeMsLessThanZero) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("setSmartVolControl"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(",\"enable\": true, \"playerAudioLevelThreshold\": 0.1, \"playerDetectTimeMs\": -1, \"playerHoldTimeMs\": 1000, \"primaryDuckingPercent\": 1 }"), 
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPSetSmartVolControlHoldTimeMsLessThanZero
 * @brief : Set holdTimeMs < 0 which result in failure
 * 
 * @param[in]   :  holdTimeMs = -1 
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPSetSmartVolControlHoldTimeMsLessThanZero) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("setSmartVolControl"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(",\"enable\": true, \"playerAudioLevelThreshold\": 0.1, \"playerDetectTimeMs\": 200, \"playerHoldTimeMs\": -1, \"primaryDuckingPercent\": 1 }"), 
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPSetSmartVolControlDuckPercentLessThanZero
 * @brief : Set primaryDuckingPercent < 0 which result in failure
 * 
 * @param[in]   :  primaryDuckingPercent = -1 
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPSetSmartVolControlDuckPercentLessThanZero) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("setSmartVolControl"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(",\"enable\": true, \"playerAudioLevelThreshold\": 0.1, \"playerDetectTimeMs\": 200, \"playerHoldTimeMs\": 1000, \"primaryDuckingPercent\": -1 }"), 
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPSetSmartVolControlDuckPercentLessThanZero
 * @brief : Set primaryDuckingPercent < 0 which result in failure
 * 
 * @param[in]   :  primaryDuckingPercent = -1 
 * @return      :  {success: false}
 */

TEST_F(SAPInitializedTest, SAPSetSmartVolControlDuckPercentGreaterThan100) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("setSmartVolControl"),
            _T("{\"id\": ") + std::to_string(speechId) + _T(",\"enable\": true, \"playerAudioLevelThreshold\": 0.1, \"playerDetectTimeMs\": 200, \"playerHoldTimeMs\": 1000, \"primaryDuckingPercent\": 101 }"), 
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":false}"));
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}

/**
 * @name  : SAPSetSmartVolControlWithoutOpen
 * @brief : SettingSmartVolControl for invalidPlayer
 * 
 * @param[in]   :  id, enable, playerAudioLevelThreshold, playerDetectTimeMs, playerHoldTimeMs, primaryDuckingPercent
 * @return      :  {success: false}
 */
TEST_F(SAPInitializedTest,SAPSetSmartVolControlWithoutOpen) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("setSmartVolControl"), 
        _T("{\"id\": 1 , \"enable\": true, \"playerAudioLevelThreshold\": 0.1, \"playerDetectTimeMs\": 200, \"playerHoldTimeMs\": 1000, \"primaryDuckingPercent\": 1 }"), 
        response
    ));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

/*******************************************************************************************************************
 * Test function for getPlayerSessionId
 * getPlayerSessionId                 :
 *                Gets the session ID from the provided the URL. The session is the ID returned in open cal.
 *                
 *                @return Response object contains session id and success status
 * Use case coverage:
 *                @Success : 1
 *                @Failure : 2
 ********************************************************************************************************************/
/**
 * @name  : SAPGetPlayerSessionId
 * @brief : Open the player and play the url and get sessionid for it
 * 
 * @param[in]   :  id , url
 * @return      :  {success: true}
 */
TEST_F(SAPInitializedTest, SAPGetPlayerSessionId) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
        _T("open"), 
        _T("{\"audiotype\": \"pcm\",\"sourcetype\": \"websocket\",\"playmode\": \"system\" }"),
         response
    ));

    size_t idPos = response.find("\"id\"");
    if (idPos != string::npos) {
        size_t idStart = response.find(':', idPos) + 1;
        size_t idEnd = response.find(',', idPos);
        std::string idSubstring = response.substr(idStart, idEnd - idStart);
        int speechId = std::stoi(idSubstring);
        std::cout << "Speech ID: " << speechId << std::endl;

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, 
            _T("play"), 
            _T("{\"id\": ") + std::to_string(speechId) + _T(", \"url\": \"ws://example.com/socket\" }"), 
            response
        ));
        EXPECT_EQ(response, _T("{\"success\":true}"));
        
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,
            _T("getPlayerSessionId"), 
            _T("{\"url\": \"ws://example.com/socket\" }"), 
            response
        ));
        size_t sesPos = response.find("\"sessionId\"");
        if (sesPos != string::npos) {
            size_t sesStart = response.find(':', sesPos) + 1;
            size_t sesEnd = response.find(',', sesPos);
            std::string sesSubstring = response.substr(sesStart, sesEnd - sesStart);
            int sessionId = std::stoi(sesSubstring);
            std::cout << "Session ID: " << sessionId << std::endl;
            EXPECT_EQ(speechId,sessionId);
        } else {
            EXPECT_TRUE(false) << "Error: 'sessionid' not found in the response.";
        }
    } else {
        EXPECT_TRUE(false) << "Error: 'id' not found in the response.";
    }
}
/**
 * @name  : SAPGetPlayerSessionIdInvalid
 * @brief : GetSessionId for non-existing url
 * 
 * @param[in]   :  id , url
 * @return      :  {success: true}
 */
TEST_F(SAPInitializedTest, SAPGetPlayerSessionIdInvalid) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));   
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, 
            _T("getPlayerSessionId"), 
            _T("{\"url\": \"https://www.yahoo.com\" }"), 
            response
        ));
    EXPECT_EQ(response, _T("{\"sessionId\":-1,\"success\":true}")); 
}
/**
 * @name  : SAPGetPlayerSessionIdInvalidUrl
 * @brief : GetSessionId for invalid url
 * 
 * @param[in]   :  id , url
 * @return      :  {success: false}
 */
TEST_F(SAPInitializedTest, SAPGetPlayerSessionIdInvalidUrl) {
    EXPECT_EQ(string(""), plugin->Initialize(&service));   
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, 
            _T("getPlayerSessionId"), 
            _T("{\"url\": \"invalid\" }"), 
            response
        ));
    EXPECT_EQ(response, _T("{\"success\":false}")); 
}