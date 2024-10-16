#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../ContentProtection.h"
#include "DispatcherMock.h"
#include "ServiceMock.h"
#include "WorkerPoolImplementation.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::IsTrue;
using ::testing::NiceMock;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::Test;
using ::WPEFramework::Core::ProxyType;
using ::WPEFramework::Core::Service;
using ::WPEFramework::Exchange::IContentProtection;
using ::WPEFramework::Exchange::IWatermark;
using ::WPEFramework::Plugin::ContentProtection;
using ::WPEFramework::PluginHost::IDispatcher;
using ::WPEFramework::PluginHost::IPlugin;

class AContentProtection : public Test {
protected:
    ProxyType<WorkerPoolImplementation> workerPool;
    NiceMock<DispatcherMock>* secManager;
    NiceMock<DispatcherMock>* watermark;
    NiceMock<ServiceMock>* service;
    IPlugin* plugin;
    AContentProtection()
        : workerPool(ProxyType<WorkerPoolImplementation>::Create(
              WPEFramework::Core::Thread::DefaultStackSize()))
        , secManager(Service<NiceMock<DispatcherMock>>::Create<
              NiceMock<DispatcherMock>>())
        , watermark(Service<NiceMock<DispatcherMock>>::Create<
              NiceMock<DispatcherMock>>())
        , service(Service<NiceMock<ServiceMock>>::Create<
              NiceMock<ServiceMock>>())
        , plugin(Service<ContentProtection>::Create<IPlugin>())
    {
        WPEFramework::Core::IWorkerPool::Assign(&(*workerPool));
        ON_CALL(*secManager, Subscribe(_, _, _))
            .WillByDefault(Return(WPEFramework::Core::ERROR_NONE));
        ON_CALL(*secManager, Unsubscribe(_, _, _))
            .WillByDefault(Return(WPEFramework::Core::ERROR_NONE));
        ON_CALL(*watermark, Subscribe(_, _, _))
            .WillByDefault(Return(WPEFramework::Core::ERROR_NONE));
        ON_CALL(*watermark, Unsubscribe(_, _, _))
            .WillByDefault(Return(WPEFramework::Core::ERROR_NONE));
        ON_CALL(*service, QueryInterfaceByCallsign(_, _))
            .WillByDefault(Invoke(
                [&](const uint32_t id, const string& name) {
                    void* result = nullptr;
                    if (id == IDispatcher::ID) {
                        if (name == "SecManager") {
                            secManager->AddRef();
                            result = secManager;
                        } else if (name == "Watermark") {
                            watermark->AddRef();
                            result = watermark;
                        }
                    }
                    return result;
                }));
    }
    ~AContentProtection() override
    {
        WPEFramework::Core::IWorkerPool::Assign(nullptr);
        secManager->Release();
        watermark->Release();
        service->Release();
        plugin->Release();
    }
};

TEST_F(AContentProtection, OpensSecManagerPlaybackSession)
{
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto contentProtection = plugin->QueryInterface<IContentProtection>();
    ASSERT_THAT(contentProtection, NotNull());
    uint32_t sessionId;
    string reqMethod, reqParameters, response;
    ON_CALL(*secManager, Invoke(_, _, _, _, _, _))
        .WillByDefault(Invoke(
            [&](const uint32_t, const uint32_t, const string&,
                const string& method, const string& parameters,
                string& response) {
                reqMethod = method;
                reqParameters = parameters;
                response = "{\"sessionId\":111,\"success\":true}";
                return WPEFramework::Core::ERROR_NONE;
            }));
    ASSERT_THAT(contentProtection->OpenDrmSession("Ab",
                    IContentProtection::KeySystem::PLAYREADY,
                    "xx", "{\"c\":{}}", sessionId, response),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(reqMethod, Eq("openPlaybackSession"));
    EXPECT_THAT(reqParameters, Eq("{\"c\":{},\"clientId\":\"Ab\","
                                  "\"keySystem\":\"playready\","
                                  "\"licenseRequest\":\"xx\"}"));
    EXPECT_THAT(sessionId, Eq(111));
    contentProtection->Release();
    plugin->Deinitialize(service);
}

TEST_F(AContentProtection, OpensSecManagerPlaybackSessionAndSetsState)
{
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto contentProtection = plugin->QueryInterface<IContentProtection>();
    ASSERT_THAT(contentProtection, NotNull());
    uint32_t sessionId;
    string reqMethod, reqParameters, response;
    ON_CALL(*secManager, Invoke(_, _, _, _, _, _))
        .WillByDefault(Invoke(
            [&](const uint32_t, const uint32_t, const string&, const string&,
                const string&, string& response) {
                response = "{\"sessionId\":7,\"success\":true}";
                return WPEFramework::Core::ERROR_NONE;
            }));
    ASSERT_THAT(contentProtection->OpenDrmSession("x",
                    IContentProtection::KeySystem::PLAYREADY,
                    "", "{}", sessionId, response),
        Eq(WPEFramework::Core::ERROR_NONE));
    ON_CALL(*secManager, Invoke(_, _, _, _, _, _))
        .WillByDefault(Invoke(
            [&](const uint32_t, const uint32_t, const string&,
                const string& method, const string& parameters,
                string& response) {
                reqMethod = method;
                reqParameters = parameters;
                response = "{\"success\":true}";
                return WPEFramework::Core::ERROR_NONE;
            }));
    ASSERT_THAT(contentProtection->SetDrmSessionState(
                    sessionId, IContentProtection::State::INACTIVE),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(reqMethod, Eq("setPlaybackSessionState"));
    EXPECT_THAT(reqParameters,
        Eq("{\"clientId\":\"x\",\"sessionId\":7,"
           "\"sessionState\":\"inactive\"}"));
    contentProtection->Release();
    plugin->Deinitialize(service);
}

TEST_F(AContentProtection, OpensSecManagerPlaybackSessionClosesAndCantSetState)
{
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto contentProtection = plugin->QueryInterface<IContentProtection>();
    ASSERT_THAT(contentProtection, NotNull());
    uint32_t sessionId;
    string reqMethod, reqParameters, response;
    ON_CALL(*secManager, Invoke(_, _, _, _, _, _))
        .WillByDefault(Invoke(
            [&](const uint32_t, const uint32_t, const string&, const string&,
                const string&, string& response) {
                response = "{\"sessionId\":7,\"success\":true}";
                return WPEFramework::Core::ERROR_NONE;
            }));
    ASSERT_THAT(contentProtection->OpenDrmSession("x",
                    IContentProtection::KeySystem::PLAYREADY,
                    "", "{}", sessionId, response),
        Eq(WPEFramework::Core::ERROR_NONE));
    ON_CALL(*secManager, Invoke(_, _, _, _, _, _))
        .WillByDefault(Invoke(
            [&](const uint32_t, const uint32_t, const string&,
                const string& method, const string& parameters,
                string& response) {
                reqMethod = method;
                reqParameters = parameters;
                response = "{\"success\":true}";
                return WPEFramework::Core::ERROR_NONE;
            }));
    ASSERT_THAT(contentProtection->CloseDrmSession(sessionId),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(reqMethod, Eq("closePlaybackSession"));
    EXPECT_THAT(reqParameters,
        Eq("{\"clientId\":\"x\",\"sessionId\":7}"));
    EXPECT_THAT(contentProtection->SetDrmSessionState(
                    sessionId, IContentProtection::State::INACTIVE),
        Eq(WPEFramework::Core::ERROR_GENERAL));
    contentProtection->Release();
    plugin->Deinitialize(service);
}

TEST_F(AContentProtection, SetsSecManagerPlaybackSpeedState)
{
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    auto contentProtection = plugin->QueryInterface<IContentProtection>();
    ASSERT_THAT(contentProtection, NotNull());
    string reqMethod, reqParameters, response;
    ON_CALL(*secManager, Invoke(_, _, _, _, _, _))
        .WillByDefault(Invoke(
            [&](const uint32_t, const uint32_t, const string&,
                const string& method, const string& parameters,
                string& response) {
                reqMethod = method;
                reqParameters = parameters;
                response = "{\"success\":true}";
                return WPEFramework::Core::ERROR_NONE;
            }));
    ASSERT_THAT(contentProtection->SetPlaybackPosition(3, -2, 10),
        Eq(WPEFramework::Core::ERROR_NONE));
    EXPECT_THAT(reqMethod, Eq("setPlaybackSpeedState"));
    EXPECT_THAT(reqParameters,
        Eq("{\"sessionId\":3,\"playbackSpeed\":-2,\"playbackPosition\":10}"));
    contentProtection->Release();
    plugin->Deinitialize(service);
}

TEST_F(AContentProtection, DeletesWatermarkWhenOnRemoveWatermarkEvent)
{
    IDispatcher::ICallback* cb = nullptr;
    ON_CALL(*secManager, Subscribe(_, _, _))
        .WillByDefault(Invoke(
            [&](IDispatcher::ICallback* callback, const string& event,
                const string&) {
                if (event == "onRemoveWatermark") {
                    cb = callback;
                }
                return WPEFramework::Core::ERROR_NONE;
            }));
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    ASSERT_THAT(cb, NotNull());
    string reqMethod, reqParameters;
    ON_CALL(*watermark, Invoke(_, _, _, _, _, _))
        .WillByDefault(Invoke(
            [&](const uint32_t, const uint32_t, const string&,
                const string& method, const string& parameters,
                string& response) {
                reqMethod = method;
                reqParameters = parameters;
                response = "{\"success\":true}";
                return WPEFramework::Core::ERROR_NONE;
            }));
    cb->Event("onRemoveWatermark", "", "{\"graphicId\":123}");
    EXPECT_THAT(reqMethod, Eq("deleteWatermark"));
    EXPECT_THAT(reqParameters, Eq("{\"id\":123}"));
    plugin->Deinitialize(service);
}

TEST_F(AContentProtection, UpdatesWatermarkWhenCreateWatermarkSucceeds)
{
    IDispatcher::ICallback *cb1 = nullptr, *cb2 = nullptr;
    ON_CALL(*secManager, Subscribe(_, _, _))
        .WillByDefault(Invoke(
            [&](IDispatcher::ICallback* callback, const string& event,
                const string&) {
                if (event == "onAddWatermark") {
                    cb1 = callback;
                }
                return WPEFramework::Core::ERROR_NONE;
            }));
    ON_CALL(*watermark, Subscribe(_, _, _))
        .WillByDefault(Invoke(
            [&](IDispatcher::ICallback* callback, const string& event,
                const string&) {
                if (event == "onWatermarkRequestStatus") {
                    cb2 = callback;
                }
                return WPEFramework::Core::ERROR_NONE;
            }));
    ASSERT_THAT(plugin->Initialize(service), Eq(""));
    ASSERT_THAT(cb1, NotNull());
    ASSERT_THAT(cb2, NotNull());
    string reqMethod, reqParameters;
    ON_CALL(*watermark, Invoke(_, _, _, _, _, _))
        .WillByDefault(Invoke(
            [&](const uint32_t, const uint32_t, const string&,
                const string& method, const string& parameters,
                string& response) {
                reqMethod = method;
                reqParameters = parameters;
                response = "{\"success\":true}";
                return WPEFramework::Core::ERROR_NONE;
            }));
    cb1->Event("onAddWatermark", "",
        "{\"graphicId\":123,\"zIndex\":100,"
        "\"graphicImageBufferKey\":1,\"graphicImageSize\":1000}");
    ASSERT_THAT(reqMethod, Eq("createWatermark"));
    EXPECT_THAT(reqParameters, Eq("{\"id\":123,\"zorder\":100}"));
    cb2->Event("onWatermarkRequestStatus", "",
        "{\"id\":123,\"type\":\"create\",\"success\":true}");
    ASSERT_THAT(reqMethod, Eq("updateWatermark"));
    EXPECT_THAT(reqParameters, Eq("{\"id\":123,\"key\":1,\"size\":1000}"));
    plugin->Deinitialize(service);
}
