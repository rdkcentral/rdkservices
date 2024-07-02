#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <interfaces/IStore2.h>
#include <interfaces/IUserSettings.h>

#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);

#define JSON_TIMEOUT   (1000)
#define USERSETTING_CALLSIGN  _T("org.rdk.UserSettings")
#define USERSETTINGL2TEST_CALLSIGN _T("L2tests.1")

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;
using ::WPEFramework::Exchange::IStore2;
using ::WPEFramework::Exchange::IUserSettings;

typedef enum : uint32_t {
    UserSettings_OnAudioDescriptionChanged = 0x00000001,
    UserSettings_OnPreferredAudioLanguagesChanged = 0x00000002,
    UserSettings_OnPresentationLanguageChanged = 0x00000003,
    UserSettings_OnCaptionsChanged = 0x00000004,
    UserSettings_OnPreferredCaptionsLanguagesChanged = 0x00000005,
    UserSettings_OnPreferredClosedCaptionServiceChanged = 0x00000006,
    UserSettings_OnPrivacyModeChanged = 0x00000007,
    UserSettings_StateInvalid = 0x00000000
}UserSettingsL2test_async_events_t;

class AsyncHandlerMock_UserSetting
{
    public:
        AsyncHandlerMock_UserSetting()
        {
        }
        MOCK_METHOD(void, OnAudioDescriptionChanged, (const bool enabled));
        MOCK_METHOD(void, OnPreferredAudioLanguagesChanged, (const string preferredLanguages));
        MOCK_METHOD(void, OnPresentationLanguageChanged, (const string presentationLanguages));
        MOCK_METHOD(void, OnCaptionsChanged, (bool enabled));
        MOCK_METHOD(void, OnPreferredCaptionsLanguagesChanged, (const string preferredLanguages));
        MOCK_METHOD(void, OnPreferredClosedCaptionServiceChanged, (const string service));
        MOCK_METHOD(void, OnPrivacyModeChanged, (const string privacyMode));

};

class NotificationHandler : public Exchange::IUserSettings::INotification {
    private:
        /** @brief Mutex */
        std::mutex m_mutex;

        /** @brief Condition variable */
        std::condition_variable m_condition_variable;

        /** @brief Event signalled flag */
        uint32_t m_event_signalled;

        BEGIN_INTERFACE_MAP(Notification)
        INTERFACE_ENTRY(Exchange::IUserSettings::INotification)
        END_INTERFACE_MAP

    public:
        NotificationHandler(){}
        ~NotificationHandler(){}

        void OnAudioDescriptionChanged(const bool enabled) override
        {
            TEST_LOG("OnAudioDescriptionChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);
            std::string str = enabled ? "true" : "false";

            TEST_LOG("OnAudioDescriptionChanged received: %s\n", str.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_OnAudioDescriptionChanged;
            m_condition_variable.notify_one();
        }

        void OnPreferredAudioLanguagesChanged(const string& preferredLanguages) override
        {
            TEST_LOG("OnPreferredAudioLanguagesChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);

            TEST_LOG("OnPreferredAudioLanguagesChanged received: %s\n", preferredLanguages.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_OnPreferredAudioLanguagesChanged;
            m_condition_variable.notify_one();

        }

        void OnPresentationLanguageChanged(const string& presentationLanguages) override
        {
            TEST_LOG("OnPresentationLanguageChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);

            TEST_LOG("OnPresentationLanguageChanged received: %s\n", presentationLanguages.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_OnPresentationLanguageChanged;
            m_condition_variable.notify_one();

        }

        void OnCaptionsChanged(bool enabled) override
        {
            TEST_LOG("OnCaptionsChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);
            std::string str = enabled ? "true" : "false";

            TEST_LOG("OnCaptionsChanged received: %s\n", str.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_OnCaptionsChanged;
            m_condition_variable.notify_one();

        }

        void OnPreferredCaptionsLanguagesChanged(const string& preferredLanguages) override
        {
            TEST_LOG("OnPreferredCaptionsLanguagesChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);

            TEST_LOG("OnPreferredAudioLanguagesChanged received: %s\n", preferredLanguages.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_OnPreferredCaptionsLanguagesChanged;
            m_condition_variable.notify_one();

        }

        void OnPreferredClosedCaptionServiceChanged(const string& service) override
        {
            TEST_LOG("OnPreferredClosedCaptionServiceChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);

            TEST_LOG("OnPreferredClosedCaptionServiceChanged received: %s\n", service.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_OnPreferredClosedCaptionServiceChanged;
            m_condition_variable.notify_one();

        }

        void OnPrivacyModeChanged(const string& privacyMode) override
        {
            TEST_LOG("OnPrivacyModeChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);

            TEST_LOG("OnPrivacyModeChanged received: %s\n", service.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_OnPrivacyModeChanged;
            m_condition_variable.notify_one();

        }

        uint32_t WaitForRequestStatus(uint32_t timeout_ms, UserSettingsL2test_async_events_t expected_status)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            auto now = std::chrono::system_clock::now();
            std::chrono::milliseconds timeout(timeout_ms);
            uint32_t signalled = UserSettings_StateInvalid;

            while (!(expected_status & m_event_signalled))
            {
              if (m_condition_variable.wait_until(lock, now + timeout) == std::cv_status::timeout)
              {
                 TEST_LOG("Timeout waiting for request status event");
                 break;
              }
            }

            signalled = m_event_signalled;
            return signalled;
        }
    };

class UserSettingTest : public L2TestMocks {
protected:
    virtual ~UserSettingTest() override;

    public:
    UserSettingTest();

      void OnAudioDescriptionChanged(const bool enabled);
      void OnPreferredAudioLanguagesChanged(const string preferredLanguages);
      void OnPresentationLanguageChanged(const string presentationLanguages);
      void OnCaptionsChanged(bool enabled);
      void OnPreferredCaptionsLanguagesChanged(const string preferredLanguages);
      void OnPreferredClosedCaptionServiceChanged(const string service);

      uint32_t WaitForRequestStatus(uint32_t timeout_ms,UserSettingsL2test_async_events_t expected_status);

    private:
        /** @brief Mutex */
        std::mutex m_mutex;

        /** @brief Condition variable */
        std::condition_variable m_condition_variable;

        /** @brief Event signalled flag */
        uint32_t m_event_signalled;
};

UserSettingTest:: UserSettingTest():L2TestMocks()
{
        Core::JSONRPC::Message message;
        string response;
        uint32_t status = Core::ERROR_GENERAL;

         /* Activate plugin in constructor */
         status = ActivateService("org.rdk.PersistentStore");
         EXPECT_EQ(Core::ERROR_NONE, status);
         status = ActivateService("org.rdk.UserSettings");
         EXPECT_EQ(Core::ERROR_NONE, status);
}

/**
 * @brief Destructor for SystemServices L2 test class
 */
UserSettingTest::~UserSettingTest()
{
    uint32_t status = Core::ERROR_GENERAL;

    ON_CALL(*p_rBusApiImplMock, rbus_close(::testing::_ ))
        .WillByDefault(
           ::testing::Return(RBUS_ERROR_SUCCESS));

    status = DeactivateService("org.rdk.UserSettings");
    EXPECT_EQ(Core::ERROR_NONE, status);
}

uint32_t UserSettingTest::WaitForRequestStatus(uint32_t timeout_ms, UserSettingsL2test_async_events_t expected_status)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto now = std::chrono::system_clock::now();
    std::chrono::milliseconds timeout(timeout_ms);
    uint32_t signalled = UserSettings_StateInvalid;

   while (!(expected_status & m_event_signalled))
   {
      if (m_condition_variable.wait_until(lock, now + timeout) == std::cv_status::timeout)
      {
         TEST_LOG("Timeout waiting for request status event");
         break;
      }
   }

    signalled = m_event_signalled;
    return signalled;
}

void UserSettingTest::OnAudioDescriptionChanged(const bool enabled)
{
    TEST_LOG("OnAudioDescriptionChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    std::string str = enabled ? "true" : "false";
    TEST_LOG("OnAudioDescriptionChanged received: %s\n", str.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_OnAudioDescriptionChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::OnPreferredAudioLanguagesChanged(const string preferredLanguages)
{
    TEST_LOG("OnPreferredAudioLanguagesChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    TEST_LOG("OnPreferredAudioLanguagesChanged received: %s\n", preferredLanguages.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_OnPreferredAudioLanguagesChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::OnPresentationLanguageChanged(const string presentationLanguages)
{
    TEST_LOG("OnPresentationLanguageChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    TEST_LOG("OnPresentationLanguageChanged received: %s\n", presentationLanguages.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_OnPresentationLanguageChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::OnCaptionsChanged(bool enabled)
{
    TEST_LOG("OnCaptionsChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    std::string str = enabled ? "true" : "false";
    TEST_LOG("OnCaptionsChanged received: %s\n", str.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_OnCaptionsChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::OnPreferredCaptionsLanguagesChanged(const string preferredLanguages)
{
    TEST_LOG("OnPreferredCaptionsLanguagesChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    TEST_LOG("OnPreferredAudioLanguagesChanged received: %s\n", preferredLanguages.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_OnPreferredCaptionsLanguagesChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::OnPreferredClosedCaptionServiceChanged(const string service)
{
    TEST_LOG("OnPreferredClosedCaptionServiceChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    TEST_LOG("OnPreferredClosedCaptionServiceChanged received: %s\n", service.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_OnPreferredClosedCaptionServiceChanged;
    m_condition_variable.notify_one();
}

MATCHER_P(MatchRequestStatusString, data, "")
{
    std::string actual = arg;
    TEST_LOG("Expected: %s, Actual: %s", data.c_str(), actual.c_str());
    EXPECT_STREQ(data.c_str(), actual.c_str());
    return data == actual;

}

MATCHER_P(MatchRequestStatusBool, expected, "")
{
    bool actual = arg;
    std::string expected_str = expected ? "true" : "false";
    std::string actual_str = actual ? "true" : "false";
    TEST_LOG("Expected: %s, Actual: %s", expected_str.c_str(), actual_str.c_str());
    EXPECT_STREQ(expected_str.c_str(), actual_str.c_str());
    return expected == actual;
}

TEST_F(UserSettingTest,AudioDescriptionSuccess)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USERSETTING_CALLSIGN, USERSETTINGL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_UserSetting> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result_json;
    Core::JSON::Boolean result_bool;
    bool expected_enabled = true;
    uint32_t signalled = UserSettings_StateInvalid;

    TEST_LOG("Testing AudioDescriptionSuccess");

    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                       _T("onaudiodescriptionchanged"),
                                       [this, &async_handler](const JsonObject& parameters) {
                                           bool enabled = parameters["enabled"].Boolean();
                                           async_handler.OnAudioDescriptionChanged(enabled);
                                       });

    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, OnAudioDescriptionChanged(MatchRequestStatusBool(expected_enabled)))
        .WillOnce(Invoke(this, &UserSettingTest::OnAudioDescriptionChanged));

    params["value"] = true;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setaudiodescription", params, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_OnAudioDescriptionChanged);
    EXPECT_TRUE(signalled & UserSettings_OnAudioDescriptionChanged);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onaudiodescriptionchanged"));

    EXPECT_EQ(status,Core::ERROR_NONE);
    if (status != Core::ERROR_NONE) {
        TEST_LOG("Expected Core::ERROR_NONE (0) but got: %u", status);
    }

    status = InvokeServiceMethod("org.rdk.UserSettings", "getaudiodescription", result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);

    TEST_LOG("result_bool: %d", result_bool.Value());
    EXPECT_TRUE(result_bool.Value());
}

TEST_F(UserSettingTest,PreferredAudioLanguagesSuccess)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USERSETTING_CALLSIGN, USERSETTINGL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_UserSetting> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result_json;
    Core::JSON::String result_string;
    std::string preferredLanguages = "en,fr,es";
    uint32_t signalled = UserSettings_StateInvalid;

    TEST_LOG("Testing PreferredAudioLanguagesSuccess");

    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onpreferredaudiolanguageschanged"),
                                           [&async_handler](const JsonObject& parameters) {
                                           std::string preferredLanguages = parameters["preferredlanguages"].String();
                                           async_handler.OnPreferredAudioLanguagesChanged(preferredLanguages);
                                       });

    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, OnPreferredAudioLanguagesChanged(MatchRequestStatusString(preferredLanguages)))
        .WillOnce(Invoke(this, &UserSettingTest::OnPreferredAudioLanguagesChanged));

    params["value"] = preferredLanguages;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setpreferredaudiolanguages", params, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_OnPreferredAudioLanguagesChanged);
    EXPECT_TRUE(signalled & UserSettings_OnPreferredAudioLanguagesChanged);

    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onpreferredaudiolanguageschanged"));

    EXPECT_EQ(status,Core::ERROR_NONE);
    if (status != Core::ERROR_NONE) {
        TEST_LOG("Expected Core::ERROR_NONE (0) but got: %u", status);
    }

    status = InvokeServiceMethod("org.rdk.UserSettings", "getpreferredaudiolanguages", result_string);
    EXPECT_EQ(status,Core::ERROR_NONE);

    EXPECT_EQ(result_string.Value(), preferredLanguages);
}

TEST_F(UserSettingTest,PresentationLanguageSuccess)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USERSETTING_CALLSIGN, USERSETTINGL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_UserSetting> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result_json;
    Core::JSON::String result_string;
    std::string presentationLanguage = "en";
    uint32_t signalled = UserSettings_StateInvalid;

    TEST_LOG("Testing PresentationLanguageSuccess");

    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onpresentationlanguagechanged"),
                                           [&async_handler](const JsonObject& parameters) {
                                           std::string presentationLanguage = parameters["presentationlanguages"].String();
                                           async_handler.OnPresentationLanguageChanged(presentationLanguage);
                                       });

    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, OnPresentationLanguageChanged(MatchRequestStatusString(presentationLanguage)))
        .WillOnce(Invoke(this, &UserSettingTest::OnPresentationLanguageChanged));

    params["value"] = "en";
    status = InvokeServiceMethod("org.rdk.UserSettings", "setpresentationlanguage", params, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_OnPresentationLanguageChanged);
    EXPECT_TRUE(signalled & UserSettings_OnPresentationLanguageChanged);

    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onpresentationlanguagechanged"));

    EXPECT_EQ(status,Core::ERROR_NONE);
    if (status != Core::ERROR_NONE) {
        TEST_LOG("Expected Core::ERROR_NONE (0) but got: %u", status);
    }

    status = InvokeServiceMethod("org.rdk.UserSettings", "getpresentationlanguage", result_string);
    EXPECT_EQ(status,Core::ERROR_NONE);

    EXPECT_EQ(result_string.Value(), presentationLanguage);
}

TEST_F(UserSettingTest,SetCaptionsSuccess)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USERSETTING_CALLSIGN, USERSETTINGL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_UserSetting> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result_json;
    Core::JSON::Boolean result_bool;
    bool expected_enabled = true;
    uint32_t signalled = UserSettings_StateInvalid;

    TEST_LOG("Testing SetCaptionsSuccess");

    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                       _T("oncaptionschanged"),
                                       [this, &async_handler](const JsonObject& parameters) {
                                           bool enabled = parameters["enabled"].Boolean();
                                           async_handler.OnCaptionsChanged(enabled);
                                       });

    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, OnCaptionsChanged(MatchRequestStatusBool(expected_enabled)))
        .WillOnce(Invoke(this, &UserSettingTest::OnCaptionsChanged));

    params["value"] = true;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setcaptions", params, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_OnCaptionsChanged);
    EXPECT_TRUE(signalled & UserSettings_OnCaptionsChanged);

    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("oncaptionschanged"));

    EXPECT_EQ(status,Core::ERROR_NONE);
    if (status != Core::ERROR_NONE) {
        TEST_LOG("Expected Core::ERROR_NONE (0) but got: %u", status);
    }

    status = InvokeServiceMethod("org.rdk.UserSettings", "getcaptions", result_bool);
    EXPECT_EQ(status,Core::ERROR_NONE);

    TEST_LOG("result_bool: %d", result_bool.Value());
    EXPECT_TRUE(result_bool.Value());
}

TEST_F(UserSettingTest,SetPreferredCaptionsLanguagesSuccess)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USERSETTING_CALLSIGN, USERSETTINGL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_UserSetting> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result_json;
    Core::JSON::String result_string;
    string preferredLanguages = "en,es";
    uint32_t signalled = UserSettings_StateInvalid;

    TEST_LOG("Testing SetPreferredCaptionsLanguagesSuccess");

    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onpreferredcaptionslanguageschanged"),
                                           [&async_handler](const JsonObject& parameters) {
                                           std::string preferredLanguages = parameters["preferredlanguages"].String();
                                           async_handler.OnPreferredCaptionsLanguagesChanged(preferredLanguages);
                                       });

    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, OnPreferredCaptionsLanguagesChanged(MatchRequestStatusString(preferredLanguages)))
        .WillOnce(Invoke(this, &UserSettingTest::OnPreferredCaptionsLanguagesChanged));

    params["value"] = "en,es";
    status = InvokeServiceMethod("org.rdk.UserSettings", "setpreferredcaptionslanguages", params, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_OnPreferredCaptionsLanguagesChanged);
    EXPECT_TRUE(signalled & UserSettings_OnPreferredCaptionsLanguagesChanged);

    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onpreferredcaptionslanguageschanged"));

    EXPECT_EQ(status,Core::ERROR_NONE);
    if (status != Core::ERROR_NONE) {
        TEST_LOG("Expected Core::ERROR_NONE (0) but got: %u", status);
    }

    status = InvokeServiceMethod("org.rdk.UserSettings", "getpreferredcaptionslanguages", result_string);
    EXPECT_EQ(status,Core::ERROR_NONE);

    EXPECT_EQ(result_string.Value(), preferredLanguages);
}

TEST_F(UserSettingTest,SetPreferredClosedCaptionServiceSuccess)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USERSETTING_CALLSIGN, USERSETTINGL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_UserSetting> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params;
    JsonObject result_json;
    Core::JSON::String result_string;
    string expected_service = "CC3";
    uint32_t signalled = UserSettings_StateInvalid;

    TEST_LOG("Testing SetPreferredClosedCaptionServiceSuccess");

    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onpreferredclosedcaptionservicechanged"),
                                           [&async_handler](const JsonObject& parameters) {
                                               std::string preferredService = parameters["service"].String();
                                           async_handler.OnPreferredClosedCaptionServiceChanged(preferredService);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, OnPreferredClosedCaptionServiceChanged(MatchRequestStatusString(expected_service)))
        .WillOnce(Invoke(this, &UserSettingTest::OnPreferredClosedCaptionServiceChanged));

    params["value"] = "CC3";
    status = InvokeServiceMethod("org.rdk.UserSettings", "setpreferredclosedcaptionservice", params, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_OnPreferredClosedCaptionServiceChanged);
    EXPECT_TRUE(signalled & UserSettings_OnPreferredClosedCaptionServiceChanged);

    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onpreferredclosedcaptionservicechanged"));

    EXPECT_EQ(status,Core::ERROR_NONE);
    if (status != Core::ERROR_NONE) {
        TEST_LOG("Expected Core::ERROR_NONE (0) but got: %u", status);
    }

    status = InvokeServiceMethod("org.rdk.UserSettings", "getpreferredclosedcaptionservice", result_string);
    EXPECT_EQ(status,Core::ERROR_NONE);

    EXPECT_EQ(result_string.Value(), expected_service);
}

TEST_F(UserSettingTest,AudioDescriptionSuccessUsingComRpcConnection)
{
    Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> mEngine_UserSettings;
    Core::ProxyType<RPC::CommunicatorClient> mClient_UserSettings;
    PluginHost::IShell *mController_UserSettings;
    uint32_t status = Core::ERROR_GENERAL;
    Core::Sink<NotificationHandler> mNotification;

    TEST_LOG("Creating mEngine_UserSettings");
    mEngine_UserSettings = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
    mClient_UserSettings = Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(mEngine_UserSettings));

    TEST_LOG("Creating mEngine_UserSettings Announcements");
#if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
    mEngine_UserSettings->Announcements(mClient_UserSettings->Announcement());
#endif

    if (!mClient_UserSettings.IsValid())
    {
        TEST_LOG("Invalid mClient_UserSettings");
    }
    else
    {
        mController_UserSettings = mClient_UserSettings->Open<PluginHost::IShell>(_T("org.rdk.UserSettings"), ~0, 3000);
        if (mController_UserSettings)
        {
            auto UserSettingsPlugin = mController_UserSettings->QueryInterface<Exchange::IUserSettings>();
            uint32_t signalled = UserSettings_StateInvalid;

            UserSettingsPlugin->AddRef();
            UserSettingsPlugin->Register(&mNotification);

            if (UserSettingsPlugin)
            {
                status = UserSettingsPlugin->SetAudioDescription(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                signalled = mNotification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_OnAudioDescriptionChanged);
                EXPECT_TRUE(signalled & UserSettings_OnAudioDescriptionChanged);

                bool Enable = false;
                status = UserSettingsPlugin->GetAudioDescription(Enable);
                EXPECT_EQ(Enable, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                UserSettingsPlugin->Unregister(&mNotification);
                UserSettingsPlugin->Release();
            }
            else
            {
                TEST_LOG("UserSettingsPlugin is NULL");
            }
            mController_UserSettings->Release();
        }
        else
        {
            TEST_LOG("mController_UserSettings is NULL");
        }
    }
}

TEST_F(UserSettingTest,PreferredAudioLanguagesSuccessUsingComRpcConnection)
{
    Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> mEngine_UserSettings;
    Core::ProxyType<RPC::CommunicatorClient> mClient_UserSettings;
    PluginHost::IShell *mController_UserSettings;
    Core::Sink<NotificationHandler> mNotification;
    uint32_t status = Core::ERROR_GENERAL;

    TEST_LOG("Creating mEngine_UserSettings");
    mEngine_UserSettings = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
    mClient_UserSettings = Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(mEngine_UserSettings));

    TEST_LOG("Creating mEngine_UserSettings Announcements");
#if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
    mEngine_UserSettings->Announcements(mClient_UserSettings->Announcement());
#endif

    if (!mClient_UserSettings.IsValid())
    {
        TEST_LOG("Invalid mClient_UserSettings");
    }
    else
    {
        mController_UserSettings = mClient_UserSettings->Open<PluginHost::IShell>(_T("org.rdk.UserSettings"), ~0, 3000);
        if (mController_UserSettings)
        {
            auto UserSettingsPlugin = mController_UserSettings->QueryInterface<Exchange::IUserSettings>();
            uint32_t signalled = UserSettings_StateInvalid;

            UserSettingsPlugin->AddRef();
            UserSettingsPlugin->Register(&mNotification);

            if (UserSettingsPlugin)
            {
                const string preferredLanguages = "eng";
                status = UserSettingsPlugin->SetPreferredAudioLanguages(preferredLanguages);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                signalled = mNotification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_OnPreferredAudioLanguagesChanged);
                EXPECT_TRUE(signalled & UserSettings_OnPreferredAudioLanguagesChanged);

                string preferredLanguages1 = "";
                status = UserSettingsPlugin->GetPreferredAudioLanguages(preferredLanguages1);
                EXPECT_EQ(preferredLanguages1, preferredLanguages);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                UserSettingsPlugin->Unregister(&mNotification);
                UserSettingsPlugin->Release();
            }
            else
            {
                TEST_LOG("UserSettingsPlugin is NULL");
            }
            mController_UserSettings->Release();
        }
        else
        {
            TEST_LOG("mController_UserSettings is NULL");
        }
    }
}

TEST_F(UserSettingTest,PresentationLanguageSuccessUsingComRpcConnection)
{
    Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> mEngine_UserSettings;
    Core::ProxyType<RPC::CommunicatorClient> mClient_UserSettings;
    PluginHost::IShell *mController_UserSettings;
    uint32_t status = Core::ERROR_GENERAL;
    Core::Sink<NotificationHandler> mNotification;


    TEST_LOG("Creating mEngine_UserSettings");
    mEngine_UserSettings = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
    mClient_UserSettings = Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(mEngine_UserSettings));

    TEST_LOG("Creating mEngine_UserSettings Announcements");
#if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
    mEngine_UserSettings->Announcements(mClient_UserSettings->Announcement());
#endif

    if (!mClient_UserSettings.IsValid())
    {
        TEST_LOG("Invalid mClient_UserSettings");
    }
    else
    {
        mController_UserSettings = mClient_UserSettings->Open<PluginHost::IShell>(_T("org.rdk.UserSettings"), ~0, 3000);
        if (mController_UserSettings)
        {
            auto UserSettingsPlugin = mController_UserSettings->QueryInterface<Exchange::IUserSettings>();
            uint32_t signalled = UserSettings_StateInvalid;

            UserSettingsPlugin->AddRef();
            UserSettingsPlugin->Register(&mNotification);

            if (UserSettingsPlugin)
            {
                const string presentationLanguage = "fra";
                status = UserSettingsPlugin->SetPresentationLanguage(presentationLanguage);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                signalled = mNotification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_OnPresentationLanguageChanged);
                EXPECT_TRUE(signalled & UserSettings_OnPresentationLanguageChanged);

                string presentationLanguage1 = "";
                status = UserSettingsPlugin->GetPresentationLanguage(presentationLanguage1);
                EXPECT_EQ(presentationLanguage1, presentationLanguage);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                UserSettingsPlugin->Unregister(&mNotification);
                UserSettingsPlugin->Release();
            }
            else
            {
                TEST_LOG("UserSettingsPlugin is NULL");
            }
            mController_UserSettings->Release();
        }
        else
        {
            TEST_LOG("mController_UserSettings is NULL");
        }
    }
}

TEST_F(UserSettingTest,CaptionsSuccessUsingComRpcConnection)
{
    Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> mEngine_UserSettings;
    Core::ProxyType<RPC::CommunicatorClient> mClient_UserSettings;
    PluginHost::IShell *mController_UserSettings;
    uint32_t status = Core::ERROR_GENERAL;
    Core::Sink<NotificationHandler> mNotification;

    TEST_LOG("Creating mEngine_UserSettings");
    mEngine_UserSettings = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
    mClient_UserSettings = Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(mEngine_UserSettings));

    TEST_LOG("Creating mEngine_UserSettings Announcements");
#if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
    mEngine_UserSettings->Announcements(mClient_UserSettings->Announcement());
#endif

    if (!mClient_UserSettings.IsValid())
    {
        TEST_LOG("Invalid mClient_UserSettings");
    }
    else
    {
        mController_UserSettings = mClient_UserSettings->Open<PluginHost::IShell>(_T("org.rdk.UserSettings"), ~0, 3000);
        if (mController_UserSettings)
        {
            auto UserSettingsPlugin = mController_UserSettings->QueryInterface<Exchange::IUserSettings>();
            uint32_t signalled = UserSettings_StateInvalid;

            UserSettingsPlugin->AddRef();
            UserSettingsPlugin->Register(&mNotification);

            if (UserSettingsPlugin)
            {
                status = UserSettingsPlugin->SetCaptions(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                signalled = mNotification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_OnCaptionsChanged);
                EXPECT_TRUE(signalled & UserSettings_OnCaptionsChanged);

                bool Enable = false;
                status = UserSettingsPlugin->GetCaptions(Enable);
                EXPECT_EQ(Enable, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                UserSettingsPlugin->Unregister(&mNotification);
                UserSettingsPlugin->Release();
            }
            else
            {
                TEST_LOG("UserSettingsPlugin is NULL");
            }
            mController_UserSettings->Release();
        }
        else
        {
            TEST_LOG("mController_UserSettings is NULL");
        }
    }
}

TEST_F(UserSettingTest,PreferredCaptionsLanguagesSuccessUsingComRpcConnection)
{
    Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> mEngine_UserSettings;
    Core::ProxyType<RPC::CommunicatorClient> mClient_UserSettings;
    PluginHost::IShell *mController_UserSettings;
    uint32_t status = Core::ERROR_GENERAL;
    Core::Sink<NotificationHandler> mNotification;

    TEST_LOG("Creating mEngine_UserSettings");
    mEngine_UserSettings = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
    mClient_UserSettings = Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(mEngine_UserSettings));

    TEST_LOG("Creating mEngine_UserSettings Announcements");
#if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
    mEngine_UserSettings->Announcements(mClient_UserSettings->Announcement());
#endif

    if (!mClient_UserSettings.IsValid())
    {
        TEST_LOG("Invalid mClient_UserSettings");
    }
    else
    {
        mController_UserSettings = mClient_UserSettings->Open<PluginHost::IShell>(_T("org.rdk.UserSettings"), ~0, 3000);
        if (mController_UserSettings)
        {
            const string preferredCaptionsLanguages = "eng";
            auto UserSettingsPlugin = mController_UserSettings->QueryInterface<Exchange::IUserSettings>();
            uint32_t signalled = UserSettings_StateInvalid;

            UserSettingsPlugin->AddRef();
            UserSettingsPlugin->Register(&mNotification);

            if (UserSettingsPlugin)
            {
                status = UserSettingsPlugin->SetPreferredCaptionsLanguages(preferredCaptionsLanguages);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                signalled = mNotification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_OnPreferredCaptionsLanguagesChanged);
                EXPECT_TRUE(signalled & UserSettings_OnPreferredCaptionsLanguagesChanged);

                string preferredCaptionsLanguages1 = "";
                status = UserSettingsPlugin->GetPreferredCaptionsLanguages(preferredCaptionsLanguages1);
                EXPECT_EQ(preferredCaptionsLanguages1, preferredCaptionsLanguages);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                UserSettingsPlugin->Unregister(&mNotification);
                UserSettingsPlugin->Release();
            }
            else
            {
                TEST_LOG("UserSettingsPlugin is NULL");
            }
            mController_UserSettings->Release();
        }
        else
        {
            TEST_LOG("mController_UserSettings is NULL");
        }
    }
}

TEST_F(UserSettingTest,PreferredClosedCaptionServiceSuccessUsingComRpcConnection)
{
    Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> mEngine_UserSettings;
    Core::ProxyType<RPC::CommunicatorClient> mClient_UserSettings;
    PluginHost::IShell *mController_UserSettings;
    uint32_t status = Core::ERROR_GENERAL;
    Core::Sink<NotificationHandler> mNotification;

    TEST_LOG("Creating mEngine_UserSettings");
    mEngine_UserSettings = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
    mClient_UserSettings = Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(mEngine_UserSettings));

    TEST_LOG("Creating mEngine_UserSettings Announcements");
#if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
    mEngine_UserSettings->Announcements(mClient_UserSettings->Announcement());
#endif

    if (!mClient_UserSettings.IsValid())
    {
        TEST_LOG("Invalid mClient_UserSettings");
    }
    else
    {
        mController_UserSettings = mClient_UserSettings->Open<PluginHost::IShell>(_T("org.rdk.UserSettings"), ~0, 3000);
        if (mController_UserSettings)
        {
            auto UserSettingsPlugin = mController_UserSettings->QueryInterface<Exchange::IUserSettings>();
            uint32_t signalled = UserSettings_StateInvalid;

            UserSettingsPlugin->AddRef();
            UserSettingsPlugin->Register(&mNotification);

            if (UserSettingsPlugin)
            {
                const string preferredClosedCaptionService = "CC3";
                status = UserSettingsPlugin->SetPreferredClosedCaptionService(preferredClosedCaptionService);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                signalled = mNotification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_OnPreferredClosedCaptionServiceChanged);
                EXPECT_TRUE(signalled & UserSettings_OnPreferredClosedCaptionServiceChanged);

                string preferredClosedCaptionService1 = "";
                status = UserSettingsPlugin->GetPreferredClosedCaptionService(preferredClosedCaptionService1);
                EXPECT_EQ(preferredClosedCaptionService1, preferredClosedCaptionService);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                UserSettingsPlugin->Unregister(&mNotification);
                UserSettingsPlugin->Release();
            }
            else
            {
                TEST_LOG("UserSettingsPlugin is NULL");
            }
            mController_UserSettings->Release();
        }
        else
        {
            TEST_LOG("mController_UserSettings is NULL");
        }
    }
}


