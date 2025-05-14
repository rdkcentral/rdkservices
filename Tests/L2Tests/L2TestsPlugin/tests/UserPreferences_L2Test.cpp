#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <interfaces/IUserSettings.h>

#define JSON_TIMEOUT   (1000)
#define USERSETTING_CALLSIGN  _T("org.rdk.UserSettings")
#define USERSETTINGL2TEST_CALLSIGN _T("L2tests.1")

#define USERPREFERENCE_CALLSIGN  _T("org.rdk.UserPreferences")
#define USERSETTINGL2TEST_CALLSIGN _T("L2tests.1")

#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);


using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;

typedef enum : uint32_t {
    UserSettings_onAudioDescriptionChanged = 0x00000001,
    UserSettings_onPreferredAudioLanguagesChanged = 0x00000002,
    UserSettings_onPresentationLanguageChanged = 0x00000003,
    UserSettings_onCaptionsChanged = 0x00000004,
    UserSettings_onPreferredCaptionsLanguagesChanged = 0x00000005,
    UserSettings_onPreferredClosedCaptionServiceChanged = 0x00000006,
    UserSettings_onPrivacyModeChanged = 0x00000007,
    UserSettings_onPinControlChanged = 0x00000008,
    UserSettings_onViewingRestrictionsChanged = 0x00000009,
    UserSettings_onViewingRestrictionsWindowChanged = 0x0000000a,
    UserSettings_onLiveWatershedChanged = 0x0000000b,
    UserSettings_onPlaybackWatershedChanged = 0x0000000c,
    UserSettings_onBlockNotRatedContentChanged = 0x0000000d,
    UserSettings_onPinOnPurchaseChanged = 0x0000000e,
    UserSettings_StateInvalid = 0x00000000
}UserSettingsL2test_async_events_t;

class AsyncHandlerMock
{
    public:
    AsyncHandlerMock()
        {
        }
        MOCK_METHOD(void, onAudioDescriptionChanged, (const bool enabled));
        MOCK_METHOD(void, onPreferredAudioLanguagesChanged, (const string preferredLanguages));
        MOCK_METHOD(void, onPresentationLanguageChanged, (const string presentationLanguage));
        MOCK_METHOD(void, onCaptionsChanged, (const bool enabled));
        MOCK_METHOD(void, onPreferredCaptionsLanguagesChanged, (const string preferredLanguages));
        MOCK_METHOD(void, onPreferredClosedCaptionServiceChanged, (const string service));
        MOCK_METHOD(void, onPrivacyModeChanged, (const string privacyMode));
        MOCK_METHOD(void, onPinControlChanged, (const bool pinControl));
        MOCK_METHOD(void, onViewingRestrictionsChanged, (const string viewingRestrictions));
        MOCK_METHOD(void, onViewingRestrictionsWindowChanged, (const string viewingRestrictionsWindow));
        MOCK_METHOD(void, onLiveWatershedChanged, (const bool liveWatershed));
        MOCK_METHOD(void, onPlaybackWatershedChanged, (const bool playbackWatershed));
        MOCK_METHOD(void, onBlockNotRatedContentChanged, (const bool blockNotRatedContent));
        MOCK_METHOD(void, onPinOnPurchaseChanged, (const bool pinOnPurchase));

};

class UserpreferencesTest : public L2TestMocks {
protected:
    virtual ~UserpreferencesTest() override;

public:
    UserpreferencesTest();

    // Mock methods for UserPreferences plugin
    void onPresentationLanguageChanged(const string presentationLanguage);
    uint32_t WaitForRequestStatus(uint32_t timeout_ms, UserSettingsL2test_async_events_t expected_status);

    private:
    /** @brief Mutex */
    std::mutex m_mutex;

    /** @brief Condition variable */
    std::condition_variable m_condition_variable;

    /** @brief Event signalled flag */
    uint32_t m_event_signalled;
};

UserpreferencesTest::UserpreferencesTest() : L2TestMocks() {
    uint32_t status = Core::ERROR_GENERAL;
    m_event_signalled = UserSettings_StateInvalid;
    
    // Activate the UserPreferences plugin in the constructor
    status = ActivateService("org.rdk.PersistentStore");
    EXPECT_EQ(Core::ERROR_NONE, status);

    status = ActivateService("org.rdk.UserSettings");
    TEST_LOG("activated user settings");
    EXPECT_EQ(Core::ERROR_NONE, status);

    status = ActivateService("org.rdk.UserPreferences");
    TEST_LOG("activated");
    EXPECT_EQ(Core::ERROR_NONE, status);

}

UserpreferencesTest::~UserpreferencesTest() {
    uint32_t status = Core::ERROR_GENERAL;
    m_event_signalled = UserSettings_StateInvalid;

    // Deactivate the UserPreferences plugin in the destructor
    status = DeactivateService("org.rdk.UserPreferences");
    EXPECT_EQ(Core::ERROR_NONE, status);

    status = DeactivateService("org.rdk.UserSettings");
    TEST_LOG("deactivated user settings");

    status = DeactivateService("org.rdk.PersistentStore");
    EXPECT_EQ(Core::ERROR_NONE, status);

    sleep(5);
    int file_status = remove("/tmp/secure/persistent/rdkservicestore");
    // Check if the file has been successfully removed
    if (file_status != 0)
    {
        TEST_LOG("Error deleting file[/tmp/secure/persistent/rdkservicestore]");
    }
    else
    {
        TEST_LOG("File[/tmp/secure/persistent/rdkservicestore] successfully deleted");
    }
}

void UserpreferencesTest::onPresentationLanguageChanged(const string presentationLanguage) {
    TEST_LOG("onPresentationLanguageChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    TEST_LOG("onPresentationLanguageChanged received: %s\n", presentationLanguage.c_str());

    m_event_signalled |= UserSettings_onPresentationLanguageChanged;
    m_condition_variable.notify_one();
}

uint32_t UserpreferencesTest::WaitForRequestStatus(uint32_t timeout_ms, UserSettingsL2test_async_events_t expected_status) {
    std::unique_lock<std::mutex> lock(m_mutex);
    auto now = std::chrono::system_clock::now();
    std::chrono::milliseconds timeout(timeout_ms);
    uint32_t signalled = UserSettings_StateInvalid;

    while (!(expected_status & m_event_signalled)) {
        if (m_condition_variable.wait_until(lock, now + timeout) == std::cv_status::timeout) {
            TEST_LOG("Timeout waiting for request status event");
            break;
        }
    }

    signalled = m_event_signalled;
    return signalled;
}

MATCHER_P(MatchRequestStatusString, data, "") {
    std::string expected = data;
    std::string actual = arg;
    TEST_LOG("Expected = %s, Actual = %s", expected.c_str(), actual.c_str());
    EXPECT_STREQ(expected.c_str(), actual.c_str());
    return expected == actual;
}

TEST_F(UserpreferencesTest, UserPreferencesGetSetLanguage) {
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USERPREFERENCE_CALLSIGN, USERSETTINGL2TEST_CALLSIGN);
    JSONRPC::LinkType<Core::JSON::IElement> usersettings_jsonrpc(USERSETTING_CALLSIGN, USERSETTINGL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject params, result;
    Core::JSON::String result_string;
    string initialLanguage = "en-US";
    string expectedUILanguage = "US_en";
    string newUILanguage = "CA_en";
    string expectedNewPresentationLanguage = "en-CA";
    uint32_t signalled = UserSettings_StateInvalid;

    status = usersettings_jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onPresentationLanguageChanged"),
                                           [&async_handler](const JsonObject& parameters) {
                                           string presentationLanguage = parameters["presentationLanguage"].String();
                                           async_handler.onPresentationLanguageChanged(presentationLanguage);
    });

    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onPresentationLanguageChanged(MatchRequestStatusString(initialLanguage)))
        .WillOnce(Invoke(this, &UserpreferencesTest::onPresentationLanguageChanged));

    params["presentationLanguage"] = initialLanguage;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setPresentationLanguage", params, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
   
    signalled = WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onPresentationLanguageChanged);
    EXPECT_TRUE(signalled & UserSettings_onPresentationLanguageChanged);
    
    JsonObject getParams;
    status = InvokeServiceMethod("org.rdk.UserPreferences", "getUILanguage", getParams, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ(expectedUILanguage.c_str(), result["ui_language"].Value().c_str());

    EXPECT_CALL(async_handler, onPresentationLanguageChanged(MatchRequestStatusString(expectedNewPresentationLanguage)))
        .WillOnce(Invoke(this, &UserpreferencesTest::onPresentationLanguageChanged));

    JsonObject setUIParams;
    setUIParams["ui_language"] = newUILanguage;
    status = InvokeServiceMethod("org.rdk.UserPreferences", "setUILanguage", setUIParams, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());

    signalled = WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onPresentationLanguageChanged);
    EXPECT_TRUE(signalled & UserSettings_onPresentationLanguageChanged);

    JsonObject getPresParams;
    status = InvokeServiceMethod("org.rdk.UserSettings", "getPresentationLanguage", result_string);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_EQ(expectedNewPresentationLanguage.c_str(), result_string.Value());

    status = InvokeServiceMethod("org.rdk.UserPreferences", "getUILanguage", getParams, result);
    EXPECT_EQ(Core::ERROR_NONE, status);
    EXPECT_TRUE(result["success"].Boolean());
    EXPECT_STREQ(newUILanguage.c_str(), result["ui_language"].Value().c_str());
    
    // Step 7: Unsubscribe
    usersettings_jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onPresentationLanguageChanged"));
}

    