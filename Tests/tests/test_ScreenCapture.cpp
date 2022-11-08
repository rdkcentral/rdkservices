#include <gtest/gtest.h>

#include "ScreenCapture.h"

#include "FactoriesImplementation.h"
#include "ServiceMock.h"
#include "FrameBufferMock.h"

using namespace WPEFramework;

class ScreenCaptureTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::ScreenCapture> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    ScreenCaptureTest()
        : plugin(Core::ProxyType<Plugin::ScreenCapture>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~ScreenCaptureTest() = default;
};

class ScreenCaptureEventTest : public ScreenCaptureTest {
protected:
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    ScreenCaptureEventTest()
        : ScreenCaptureTest()
    {
        EXPECT_EQ(string(""), plugin->Initialize(nullptr));

        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }
    virtual ~ScreenCaptureEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);

        plugin->Deinitialize(nullptr);
    }
};

class ScreenCaptureFrameBufferTest : public ScreenCaptureEventTest {
protected:
    FrameBufferApiMock frameBufferApiMock;

    ScreenCaptureFrameBufferTest()
        : ScreenCaptureEventTest()
    {
    }
    virtual ~ScreenCaptureFrameBufferTest() override = default;
};

TEST_F(ScreenCaptureTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("uploadScreenCapture")));
}

TEST_F(ScreenCaptureFrameBufferTest, FrameBufferUpload)
{
    FBContext fbcontext;
    PixelFormat pixelFormat = {32, 24, 0, 1, 255, 255, 255, 16, 8, 0};
    vnc_uint8_t* frameBuffer = (vnc_uint8_t*) malloc(5120 * 720);
    memset(frameBuffer, 0xff, 5120 * 720);
    
    Core::Event uploadComplete(false, true);
    
    EXPECT_CALL(frameBufferApiMock, fbCreate(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](FBContext** fbctx) {
                *fbctx = &fbcontext;
                return ErrNone;
            }));

    EXPECT_CALL(frameBufferApiMock, fbInit(::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](FBContext* fbctx, VncServerFramebufferAPI* server, void* serverctx) {
                if (nullptr != server)
                {
                    server->framebufferUpdateReady(&fbcontext);
                    server->framebufferDetailsChanged(&fbcontext, nullptr, 1280, 720, 5120, nullptr);
                    server->paletteChanged(&fbcontext, nullptr);
                    server->logMsg(&fbcontext, "msg");
                }
                return ErrNone;
            }));

    ON_CALL(frameBufferApiMock, fbGetPixelFormat(::testing::_))
        .WillByDefault(
            ::testing::Return(&pixelFormat));

    ON_CALL(frameBufferApiMock, fbGetWidth(::testing::_))
        .WillByDefault(
            ::testing::Return(1280));

    ON_CALL(frameBufferApiMock, fbGetHeight(::testing::_))
        .WillByDefault(
            ::testing::Return(720));

    ON_CALL(frameBufferApiMock, fbGetStride(::testing::_))
        .WillByDefault(
            ::testing::Return(5120));

    EXPECT_CALL(frameBufferApiMock, fbGetFramebuffer(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](FBContext* fbctx) {
                return frameBuffer;
            }));

    EXPECT_CALL(frameBufferApiMock, fbDestroy(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](FBContext* fbctx) {
            	return ErrNone;
            }));
            
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));

                EXPECT_EQ(text, string(_T(
                	"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.ScreenCapture.uploadComplete\",\"params\":{\"status\":true,\"message\":\"Success\",\"call_guid\":\"\"}}"
                )));

                uploadComplete.SetEvent();

                return Core::ERROR_NONE;
            }));

    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_TRUE(sockfd != -1);
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(11111);
    ASSERT_FALSE(bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0);
    ASSERT_FALSE(listen(sockfd, 10) < 0);

    std::thread thread = std::thread([&]() {
        auto addrlen = sizeof(sockaddr);
        const int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
        ASSERT_FALSE(connection < 0);
        char buffer[2048] = { 0 };
        ASSERT_TRUE(read(connection, buffer, 2048) > 0);

        std::string reqHeader(buffer);
        EXPECT_TRUE(std::string::npos != reqHeader.find("Content-Type: image/png"));

        std::string response = _T("HTTP/1.0 200 OK\r\n\r\n");                                
        send(connection, response.c_str(), response.size(), 0);

        close(connection);
    });

    handler.Subscribe(0, _T("uploadComplete"), _T("org.rdk.ScreenCapture"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadScreenCapture"), _T("{\"url\":\"http://127.0.0.1:11111\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, uploadComplete.Lock());

    handler.Unsubscribe(0, _T("uploadComplete"), _T("org.rdk.ScreenCapture"), message);

    free(frameBuffer);

    thread.join();
    close(sockfd);
}
