#include <gtest/gtest.h>

#include "Messenger.h"

#include "FactoriesImplementation.h"
#include "ServiceMock.h"

using namespace WPEFramework;

// Make Subscribe/Unsubscribe public
class Messenger : public Plugin::Messenger {
public:
    Messenger() = default;
    ~Messenger() = default;
    Messenger(const Messenger&) = delete;
    Messenger& operator=(const Messenger&) = delete;

    void Subscribe(Core::JSONRPC::Handler& handler, const uint32_t channelId, const string& eventName, const string& callsign, Core::JSONRPC::Message& response) override
    {
        Plugin::Messenger::Subscribe(handler, channelId, eventName, callsign, response);
    }
    void Unsubscribe(Core::JSONRPC::Handler& handler, const uint32_t channelId, const string& eventName, const string& callsign, Core::JSONRPC::Message& response) override
    {
        Plugin::Messenger::Unsubscribe(handler, channelId, eventName, callsign, response);
    }
};

class MessengerTestFixture : public ::testing::Test {
protected:
    FactoriesImplementation factoriesImplementation;

    ServiceMock service;

    Core::ProxyType<Messenger> plugin;
    Core::JSONRPC::Handler& handler;

    Core::JSONRPC::Connection connection;
    Core::JSONRPC::Message message;
    string response;

    MessengerTestFixture()
        : plugin(Core::ProxyType<Messenger>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
    }
    virtual ~MessengerTestFixture()
    {
    }

    virtual void SetUp()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);
    }

    virtual void TearDown()
    {
        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(MessengerTestFixture, registeredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("join")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("leave")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("send")));
}

TEST_F(MessengerTestFixture, activate_join_roomupdate_leave_roomupdate_deactivate)
{
    Core::Event created(false, true);
    Core::Event destroyed(false, true);

    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return("{\"root\":{\"mode\":\"Off\"}}"));
    EXPECT_CALL(service, Callsign())
        .Times(1)
        .WillOnce(::testing::Return(string()));
    EXPECT_CALL(service, Locator())
        .Times(1)
        .WillOnce(::testing::Return(string()));
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"Messenger.roomupdate\","
                                          "\"params\":"
                                          "{"
                                          "\"room\":\"room1\","
                                          "\"secure\":\"insecure\","
                                          "\"action\":\"created\""
                                          "}"
                                          "}")));

                created.SetEvent();

                return Core::ERROR_NONE;
            }))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"Messenger.roomupdate\","
                                          "\"params\":"
                                          "{"
                                          "\"room\":\"room1\","
                                          "\"secure\":\"insecure\","
                                          "\"action\":\"destroyed\""
                                          "}"
                                          "}")));

                destroyed.SetEvent();

                return Core::ERROR_NONE;
            }));

    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);
    dispatcher->Activate(&service);
    plugin->Subscribe(handler, 0, _T("roomupdate"), _T("Messenger"), message);

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("join"), _T("{\"user\":\"user1\",\"room\":\"room1\"}"), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{\"roomid\":\"[a-z0-9]+\"\\}"));

    EXPECT_EQ(Core::ERROR_NONE, created.Lock(10000)); // 10s

    string paramsStr = response;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("leave"), paramsStr, response));
    EXPECT_EQ(response, _T(""));

    EXPECT_EQ(Core::ERROR_NONE, destroyed.Lock(10000)); // 10s

    plugin->Unsubscribe(handler, 0, _T("roomupdate"), _T("Messenger"), message);
    dispatcher->Deactivate();
    dispatcher->Release();
    plugin->Deinitialize(&service);
}

TEST_F(MessengerTestFixture, activate_join_roomupdateOnSubscribe_deactivate)
{
    Core::Event created(false, true);

    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return("{\"root\":{\"mode\":\"Off\"}}"));
    EXPECT_CALL(service, Callsign())
        .Times(1)
        .WillOnce(::testing::Return(string()));
    EXPECT_CALL(service, Locator())
        .Times(1)
        .WillOnce(::testing::Return(string()));
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"Messenger.roomupdate\","
                                          "\"params\":"
                                          "{"
                                          "\"room\":\"room2\","
                                          "\"secure\":\"insecure\","
                                          "\"action\":\"created\""
                                          "}"
                                          "}")));

                created.SetEvent();

                return Core::ERROR_NONE;
            }));

    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);
    dispatcher->Activate(&service);

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("join"), _T("{\"user\":\"user2\",\"room\":\"room2\"}"), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{\"roomid\":\"[a-z0-9]+\"\\}"));

    plugin->Subscribe(handler, 0, _T("roomupdate"), _T("Messenger"), message);

    EXPECT_EQ(Core::ERROR_NONE, created.Lock(10000)); // 10s

    plugin->Unsubscribe(handler, 0, _T("roomupdate"), _T("Messenger"), message);
    dispatcher->Deactivate();
    dispatcher->Release();
    plugin->Deinitialize(&service);
}

TEST_F(MessengerTestFixture, activate_join_userupdate_join_userupdate_leave_userupdate_deactivate)
{
    Core::Event joined3(false, true);
    Core::Event joined4(false, true);
    Core::Event left4(false, true);

    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return("{\"root\":{\"mode\":\"Off\"}}"));
    EXPECT_CALL(service, Callsign())
        .Times(1)
        .WillOnce(::testing::Return(string()));
    EXPECT_CALL(service, Locator())
        .Times(1)
        .WillOnce(::testing::Return(string()));
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(3)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"[a-z0-9]+\\.Messenger\\.userupdate\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"user\":\"user3\","
                                                             "\"action\":\"joined\""
                                                             "\\}"
                                                             "\\}")));

                joined3.SetEvent();

                return Core::ERROR_NONE;
            }))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"[a-z0-9]+\\.Messenger\\.userupdate\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"user\":\"user4\","
                                                             "\"action\":\"joined\""
                                                             "\\}"
                                                             "\\}")));

                joined4.SetEvent();

                return Core::ERROR_NONE;
            }))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"[a-z0-9]+\\.Messenger\\.userupdate\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"user\":\"user4\","
                                                             "\"action\":\"left\""
                                                             "\\}"
                                                             "\\}")));

                left4.SetEvent();

                return Core::ERROR_NONE;
            }));

    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);
    dispatcher->Activate(&service);

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("join"), _T("{\"user\":\"user3\",\"room\":\"room3\"}"), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{\"roomid\":\"[a-z0-9]+\"\\}"));

    JsonObject params;
    params.FromString(response);
    string roomid1 = params["roomid"].String();
    plugin->Subscribe(handler, 0, _T("userupdate"), (roomid1 + _T(".Messenger")), message);

    EXPECT_EQ(Core::ERROR_NONE, joined3.Lock(10000)); // 10s

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("join"), _T("{\"user\":\"user4\",\"room\":\"room3\"}"), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{\"roomid\":\"[a-z0-9]+\"\\}"));

    EXPECT_EQ(Core::ERROR_NONE, joined4.Lock(10000)); // 10s

    string paramsStr = response;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("leave"), paramsStr, response));
    EXPECT_EQ(response, _T(""));

    EXPECT_EQ(Core::ERROR_NONE, left4.Lock(10000)); // 10s

    plugin->Unsubscribe(handler, 0, _T("userupdate"), (roomid1 + _T(".Messenger")), message);
    dispatcher->Deactivate();
    dispatcher->Release();
    plugin->Deinitialize(&service);
}

TEST_F(MessengerTestFixture, activate_join_join_send_message_leave_sendToUserWhoLeft_deactivate)
{
    Core::Event message6_1(false, true);
    Core::Event message6_2(false, true);
    Core::Event message5(false, true);

    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(::testing::Return("{\"root\":{\"mode\":\"Off\"}}"));
    EXPECT_CALL(service, Callsign())
        .Times(1)
        .WillOnce(::testing::Return(string()));
    EXPECT_CALL(service, Locator())
        .Times(1)
        .WillOnce(::testing::Return(string()));
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(3)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"[a-z0-9]+\\.Messenger\\.message\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"user\":\"user6\","
                                                             "\"message\":\"Hi, user5\""
                                                             "\\}"
                                                             "\\}")));

                message6_1.SetEvent();

                return Core::ERROR_NONE;
            }))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"[a-z0-9]+\\.Messenger\\.message\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"user\":\"user6\","
                                                             "\"message\":\"Hi, user5\""
                                                             "\\}"
                                                             "\\}")));

                message6_2.SetEvent();

                return Core::ERROR_NONE;
            }))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"[a-z0-9]+\\.Messenger\\.message\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"user\":\"user5\","
                                                             "\"message\":\"Hi, user6\""
                                                             "\\}"
                                                             "\\}")));

                message5.SetEvent();

                return Core::ERROR_NONE;
            }));

    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);
    dispatcher->Activate(&service);

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("join"), _T("{\"user\":\"user5\",\"room\":\"room4\"}"), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{\"roomid\":\"[a-z0-9]+\"\\}"));

    JsonObject params5;
    params5.FromString(response);
    string roomid5 = params5["roomid"].String();
    plugin->Subscribe(handler, 0, _T("message"), (roomid5 + _T(".Messenger")), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("join"), _T("{\"user\":\"user6\",\"room\":\"room4\"}"), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{\"roomid\":\"[a-z0-9]+\"\\}"));

    JsonObject params6;
    params6.FromString(response);
    string roomid6 = params6["roomid"].String();
    plugin->Subscribe(handler, 0, _T("message"), (roomid6 + _T(".Messenger")), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("send"), ("{\"message\":\"Hi, user5\",\"roomid\":\"" + roomid6 + "\"}"), response));
    EXPECT_EQ(response, _T(""));

    EXPECT_EQ(Core::ERROR_NONE, message6_1.Lock(10000)); // 10s
    EXPECT_EQ(Core::ERROR_NONE, message6_2.Lock(10000)); // 10s

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("leave"), ("{\"roomid\":\"" + roomid6 + "\"}"), response));
    EXPECT_EQ(response, _T(""));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("send"), ("{\"message\":\"Hi, user6\",\"roomid\":\"" + roomid5 + "\"}"), response));
    EXPECT_EQ(response, _T(""));

    EXPECT_EQ(Core::ERROR_NONE, message5.Lock(10000)); // 10s

    plugin->Unsubscribe(handler, 0, _T("message"), (roomid5 + _T(".Messenger")), message);
    dispatcher->Deactivate();
    dispatcher->Release();
    plugin->Deinitialize(&service);
}
