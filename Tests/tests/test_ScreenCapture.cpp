#include <gtest/gtest.h>

#include "ScreenCapture.h"

#include "FactoriesImplementation.h"
#include "ServiceMock.h"
#include "FrameBufferMock.h"

using namespace WPEFramework;

class ScreenCaptureFixture : public ::testing::Test {
protected:
    
    Core::JSONRPC::Connection connection;
    FrameBufferApiImplMock frameBufferApiImplMock;

    string response;
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    ScreenCaptureFixture()
        : connection(1, 0)
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

    }
    virtual ~ScreenCaptureFixture()
    {
        PluginHost::IFactories::Assign(nullptr);
    }

    virtual void SetUp()
    {
    	FrameBufferApi::getInstance().impl = &frameBufferApiImplMock;
    }

    virtual void TearDown()
    {
    	FrameBufferApi::getInstance().impl = nullptr;
    }
};

TEST_F(ScreenCaptureFixture, RegisteredMethods)
{
    Core::ProxyType<Plugin::ScreenCapture> plugin(Core::ProxyType<Plugin::ScreenCapture>::Create());
    Core::JSONRPC::Handler* handler(&((Core::JSONRPC::Handler&)(*(plugin))));

    EXPECT_EQ(Core::ERROR_NONE, handler->Exists(_T("uploadScreenCapture")));
}

TEST_F(ScreenCaptureFixture, FrameBufferUpload)
{
    FBContext fbcontext;
    //VncServerFramebufferAPI *api = nullptr;
    PixelFormat pixelFormat = {32, 24, 0, 1, 255, 255, 255, 16, 8, 0};
    vnc_uint8_t* frameBuffer = (vnc_uint8_t*) malloc(5120 * 720);
    
    Core::Event uploadComplete(false, true);
    
    EXPECT_CALL(frameBufferApiImplMock, fbCreate(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](FBContext** fbctx) {
                *fbctx = &fbcontext;
                return ErrNone;
            }));

    EXPECT_CALL(frameBufferApiImplMock, fbInit(::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](FBContext* fbctx, VncServerFramebufferAPI* server, void* serverctx) {
                //api = server;
                if (nullptr != server)
                {
                    server->framebufferUpdateReady(&fbcontext);
                    server->framebufferDetailsChanged(&fbcontext, nullptr, 1280, 720, 5120, nullptr);
                    server->paletteChanged(&fbcontext, nullptr);
                    server->logMsg(&fbcontext, "msg");
                }
                return ErrNone;
            }));

    EXPECT_CALL(frameBufferApiImplMock, fbGetPixelFormat(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](FBContext* fbctx) {
                return &pixelFormat;
            }));

    EXPECT_CALL(frameBufferApiImplMock, fbGetWidth(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](FBContext* fbctx) {
                return 1280;
            }));

    EXPECT_CALL(frameBufferApiImplMock, fbGetHeight(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](FBContext* fbctx) {
                return 720;
            }));

    EXPECT_CALL(frameBufferApiImplMock, fbGetStride(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](FBContext* fbctx) {
                return 5120;
            }));

    EXPECT_CALL(frameBufferApiImplMock, fbGetFramebuffer(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](FBContext* fbctx) {
                return frameBuffer;
            }));

    EXPECT_CALL(frameBufferApiImplMock, fbDestroy(::testing::_))
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

                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.ScreenCapture.uploadComplete\","
                                          "\"params\":"
                                          "{\"status\":false,\"message\":\"Upload Failed: 6:'Couldn't resolve host name'\",\"call_guid\":\"\"}"
                                          "}")));
                uploadComplete.SetEvent();

                return Core::ERROR_NONE;
            }));


    Core::ProxyType<Plugin::ScreenCapture> plugin(Core::ProxyType<Plugin::ScreenCapture>::Create());
    Core::JSONRPC::Handler* handler(&((Core::JSONRPC::Handler&)(*(plugin))));
    
    dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    dispatcher->Activate(&service);

    // Initialize
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    
    handler->Subscribe(0, _T("uploadComplete"), _T("org.rdk.ScreenCapture"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler->Invoke(connection, _T("uploadScreenCapture"), _T("{\"url\":\"http://non-existent-host.com\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    
    //uploadComplete.SetEvent();
    EXPECT_EQ(Core::ERROR_NONE, uploadComplete.Lock());

    handler->Unsubscribe(0, _T("uploadComplete"), _T("org.rdk.ScreenCapture"), message);

    // Deinitialize
    plugin->Deinitialize(nullptr);
    free(frameBuffer);
}
