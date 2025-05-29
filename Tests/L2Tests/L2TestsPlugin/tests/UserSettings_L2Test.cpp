/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2024 RDK Management
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
    UserSettings_onAudioDescriptionChanged = 0x00000001,
    UserSettings_onPreferredAudioLanguagesChanged = 0x00000002,
    UserSettings_onPresentationLanguageChanged = 0x00000003,
    UserSettings_onCaptionsChanged = 0x00000004,
    UserSettings_onPreferredCaptionsLanguagesChanged = 0x00000005,
    UserSettings_onPreferredClosedCaptionServiceChanged = 0x00000006,
    UserSettings_onPinControlChanged = 0x00000007,
    UserSettings_onViewingRestrictionsChanged = 0x00000008,
    UserSettings_onViewingRestrictionsWindowChanged = 0x00000009,
    UserSettings_onLiveWatershedChanged = 0x0000000a,
    UserSettings_onPlaybackWatershedChanged = 0x0000000b,
    UserSettings_onBlockNotRatedContentChanged = 0x0000000c,
    UserSettings_onPinOnPurchaseChanged = 0x0000000d,
    UserSettings_onHighContrastChanged = 0x0000000e,
    UserSettings_onVoiceGuidanceChanged = 0x0000000f,
    UserSettings_onVoiceGuidanceRateChanged = 0x00000011,
    UserSettings_onVoiceGuidanceHintsChanged = 0x00000012,
    UserSettings_onContentPinChanged = 0x00000013,
    UserSettings_StateInvalid = 0x00000000
}UserSettingsL2test_async_events_t;

class AsyncHandlerMock_UserSetting
{
    public:
        AsyncHandlerMock_UserSetting()
        {
        }
        MOCK_METHOD(void, onAudioDescriptionChanged, (const bool enabled));
        MOCK_METHOD(void, onPreferredAudioLanguagesChanged, (const string preferredLanguages));
        MOCK_METHOD(void, onPresentationLanguageChanged, (const string presentationLanguage));
        MOCK_METHOD(void, onCaptionsChanged, (const bool enabled));
        MOCK_METHOD(void, onPreferredCaptionsLanguagesChanged, (const string preferredLanguages));
        MOCK_METHOD(void, onPreferredClosedCaptionServiceChanged, (const string service));
        MOCK_METHOD(void, onPinControlChanged, (const bool pinControl));
        MOCK_METHOD(void, onViewingRestrictionsChanged, (const string viewingRestrictions));
        MOCK_METHOD(void, onViewingRestrictionsWindowChanged, (const string viewingRestrictionsWindow));
        MOCK_METHOD(void, onLiveWatershedChanged, (const bool liveWatershed));
        MOCK_METHOD(void, onPlaybackWatershedChanged, (const bool playbackWatershed));
        MOCK_METHOD(void, onBlockNotRatedContentChanged, (const bool blockNotRatedContent));
        MOCK_METHOD(void, onPinOnPurchaseChanged, (const bool pinOnPurchase));
        MOCK_METHOD(void, onHighContrastChanged, (const bool enabled));
        MOCK_METHOD(void, onVoiceGuidanceChanged, (const bool enabled));
        MOCK_METHOD(void, onVoiceGuidanceRateChanged, (const bool rate));
        MOCK_METHOD(void, onVoiceGuidanceHintsChanged, (const bool hints));
        MOCK_METHOD(void, onContentPinChanged, (const string& contentPin));
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
            m_event_signalled |= UserSettings_onAudioDescriptionChanged;
            m_condition_variable.notify_one();
        }

        void OnPreferredAudioLanguagesChanged(const string& preferredLanguages) override
        {
            TEST_LOG("OnPreferredAudioLanguagesChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);

            TEST_LOG("OnPreferredAudioLanguagesChanged received: %s\n", preferredLanguages.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onPreferredAudioLanguagesChanged;
            m_condition_variable.notify_one();

        }

        void OnPresentationLanguageChanged(const string& presentationLanguage) override
        {
            TEST_LOG("OnPresentationLanguageChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);

            TEST_LOG("OnPresentationLanguageChanged received: %s\n", presentationLanguage.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onPresentationLanguageChanged;
            m_condition_variable.notify_one();

        }

        void OnCaptionsChanged(bool enabled) override
        {
            TEST_LOG("OnCaptionsChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);
            std::string str = enabled ? "true" : "false";

            TEST_LOG("OnCaptionsChanged received: %s\n", str.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onCaptionsChanged;
            m_condition_variable.notify_one();

        }

        void OnPreferredCaptionsLanguagesChanged(const string& preferredLanguages) override
        {
            TEST_LOG("OnPreferredCaptionsLanguagesChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);

            TEST_LOG("OnPreferredAudioLanguagesChanged received: %s\n", preferredLanguages.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onPreferredCaptionsLanguagesChanged;
            m_condition_variable.notify_one();

        }

        void OnPreferredClosedCaptionServiceChanged(const string& service) override
        {
            TEST_LOG("OnPreferredClosedCaptionServiceChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);

            TEST_LOG("OnPreferredClosedCaptionServiceChanged received: %s\n", service.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onPreferredClosedCaptionServiceChanged;
            m_condition_variable.notify_one();
        }

        void OnPinControlChanged(const bool pinControl) override
        {
            TEST_LOG("OnPinControlChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);
            std::string str = pinControl ? "true" : "false";

            TEST_LOG("OnPinControlChanged received: %s\n", str.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onPinControlChanged;
            m_condition_variable.notify_one();
        }

        void OnViewingRestrictionsChanged(const string& viewingRestrictions) override
        {
            TEST_LOG("OnViewingRestrictionsChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);

            TEST_LOG("OnViewingRestrictionsChanged received: %s\n", viewingRestrictions.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onViewingRestrictionsChanged;
            m_condition_variable.notify_one();

        }

        void OnViewingRestrictionsWindowChanged(const string& viewingRestrictionsWindow) override
        {
            TEST_LOG("OnViewingRestrictionsWindowChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);

            TEST_LOG("OnViewingRestrictionsWindowChanged received: %s\n", viewingRestrictionsWindow.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onViewingRestrictionsWindowChanged;
            m_condition_variable.notify_one();

        }

        void OnLiveWatershedChanged(const bool liveWatershed) override
        {
            TEST_LOG("OnLiveWatershedChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);
            std::string str = liveWatershed ? "true" : "false";

            TEST_LOG("OnLiveWatershedChanged received: %s\n", str.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onLiveWatershedChanged;
            m_condition_variable.notify_one();
        }

        void OnPlaybackWatershedChanged(const bool playbackWatershed) override
        {
            TEST_LOG("OnPlaybackWatershedChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);
            std::string str = playbackWatershed ? "true" : "false";

            TEST_LOG("OnPlaybackWatershedChanged received: %s\n", str.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onPlaybackWatershedChanged;
            m_condition_variable.notify_one();
        }

        void OnBlockNotRatedContentChanged(const bool blockNotRatedContent) override
        {
            TEST_LOG("OnBlockNotRatedContentChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);
            std::string str = blockNotRatedContent ? "true" : "false";

            TEST_LOG("OnBlockNotRatedContentChanged received: %s\n", str.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onBlockNotRatedContentChanged;
            m_condition_variable.notify_one();
        }

        void OnPinOnPurchaseChanged(const bool pinOnPurchase) override
        {
            TEST_LOG("OnPinOnPurchaseChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);
            std::string str = pinOnPurchase ? "true" : "false";

            TEST_LOG("OnPinOnPurchaseChanged received: %s\n", str.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onPinOnPurchaseChanged;
            m_condition_variable.notify_one();
        }

        void OnHighContrastChanged(const bool enabled) override
        {
            TEST_LOG("OnHighContrastChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);
            std::string str = enabled ? "true" : "false";

            TEST_LOG("OnHighContrastChanged received: %s\n", str.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onHighContrastChanged;
            m_condition_variable.notify_one();
        }

        void OnVoiceGuidanceChanged(const bool enabled) override
        {
            TEST_LOG("OnVoiceGuidanceChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);
            std::string str = enabled ? "true" : "false";

            TEST_LOG("OnVoiceGuidanceChanged received: %s\n", str.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onVoiceGuidanceChanged;
            m_condition_variable.notify_one();
        }

        void OnVoiceGuidanceRateChanged(const double rate) override
        {
            TEST_LOG("OnVoiceGuidanceRateChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);

            TEST_LOG("OnVoiceGuidanceRateChanged received: %ld\n", rate);
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onVoiceGuidanceRateChanged;
            m_condition_variable.notify_one();
        }

        void OnVoiceGuidanceHintsChanged(const bool hints) override
        {
            TEST_LOG("OnVoiceGuidanceHintsChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);
            std::string str = hints ? "true" : "false";

            TEST_LOG("OnVoiceGuidanceHintsChanged received: %s\n", str.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onVoiceGuidanceHintsChanged;
            m_condition_variable.notify_one();
        }

        void OnContentPinChanged(const string& contentPin) override
        {
            TEST_LOG("OnContentPinChanged event triggered ***\n");
            std::unique_lock<std::mutex> lock(m_mutex);

            TEST_LOG("OnContentPinChanged received: %s\n", contentPin.c_str());
            /* Notify the requester thread. */
            m_event_signalled |= UserSettings_onContentPinChanged;
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

      void onAudioDescriptionChanged(const bool enabled);
      void onPreferredAudioLanguagesChanged(const string preferredLanguages);
      void onPresentationLanguageChanged(const string presentationLanguage);
      void onCaptionsChanged(bool enabled);
      void onPreferredCaptionsLanguagesChanged(const string preferredLanguages);
      void onPreferredClosedCaptionServiceChanged(const string service);
      void onPinControlChanged(const bool pinControl);
      void onViewingRestrictionsChanged(const string viewingRestrictions);
      void onViewingRestrictionsWindowChanged(const string viewingRestrictionsWindow);
      void onLiveWatershedChanged(const bool liveWatershed);
      void onPlaybackWatershedChanged(const bool playbackWatershed);
      void onBlockNotRatedContentChanged(const bool blockNotRatedContent);
      void onPinOnPurchaseChanged(const bool pinOnPurchase);
      void onHighContrastChanged(const bool enabled);
      void onVoiceGuidanceChanged(const bool enabled);
      void onVoiceGuidanceRateChanged(const double rate);
      void onVoiceGuidanceHintsChanged(const bool hints);
      void onContentPinChanged(const string& contentPin);

      uint32_t WaitForRequestStatus(uint32_t timeout_ms,UserSettingsL2test_async_events_t expected_status);
      uint32_t CreateUserSettingInterfaceObjectUsingComRPCConnection();

    private:
        /** @brief Mutex */
        std::mutex m_mutex;

        /** @brief Condition variable */
        std::condition_variable m_condition_variable;

        /** @brief Event signalled flag */
        uint32_t m_event_signalled;

    protected:
        /** @brief Pointer to the IShell interface */
        PluginHost::IShell *m_controller_usersettings;

        /** @brief Pointer to the IUserSettings interface */
        Exchange::IUserSettings *m_usersettingsplugin;
        Exchange::IUserSettingsInspector *m_usersettings_inspe_plugin;
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

uint32_t UserSettingTest::CreateUserSettingInterfaceObjectUsingComRPCConnection()
{
    uint32_t return_value =  Core::ERROR_GENERAL;
    Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> Engine_UserSettings;
    Core::ProxyType<RPC::CommunicatorClient> Client_UserSettings;

    TEST_LOG("Creating Engine_UserSettings");
    Engine_UserSettings = Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create();
    Client_UserSettings = Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId("/tmp/communicator"), Core::ProxyType<Core::IIPCServer>(Engine_UserSettings));

    TEST_LOG("Creating Engine_UserSettings Announcements");
#if ((THUNDER_VERSION == 2) || ((THUNDER_VERSION == 4) && (THUNDER_VERSION_MINOR == 2)))
    Engine_UserSettings->Announcements(mClient_UserSettings->Announcement());
#endif
    if (!Client_UserSettings.IsValid())
    {
        TEST_LOG("Invalid Client_UserSettings");
    }
    else
    {
        m_controller_usersettings = Client_UserSettings->Open<PluginHost::IShell>(_T("org.rdk.UserSettings"), ~0, 3000);
        if (m_controller_usersettings)
        {
            m_usersettingsplugin = m_controller_usersettings->QueryInterface<Exchange::IUserSettings>();
            m_usersettings_inspe_plugin = m_controller_usersettings->QueryInterface<Exchange::IUserSettingsInspector>();
            return_value = Core::ERROR_NONE;
        }
    }
    return return_value;
}

void UserSettingTest::onAudioDescriptionChanged(const bool enabled)
{
    TEST_LOG("OnAudioDescriptionChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    std::string str = enabled ? "true" : "false";
    TEST_LOG("OnAudioDescriptionChanged received: %s\n", str.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onAudioDescriptionChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onPreferredAudioLanguagesChanged(const string preferredLanguages)
{
    TEST_LOG("OnPreferredAudioLanguagesChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    TEST_LOG("OnPreferredAudioLanguagesChanged received: %s\n", preferredLanguages.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onPreferredAudioLanguagesChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onPresentationLanguageChanged(const string presentationLanguage)
{
    TEST_LOG("OnPresentationLanguageChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    TEST_LOG("OnPresentationLanguageChanged received: %s\n", presentationLanguage.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onPresentationLanguageChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onCaptionsChanged(bool enabled)
{
    TEST_LOG("OnCaptionsChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    std::string str = enabled ? "true" : "false";
    TEST_LOG("OnCaptionsChanged received: %s\n", str.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onCaptionsChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onPreferredCaptionsLanguagesChanged(const string preferredLanguages)
{
    TEST_LOG("OnPreferredCaptionsLanguagesChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    TEST_LOG("OnPreferredAudioLanguagesChanged received: %s\n", preferredLanguages.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onPreferredCaptionsLanguagesChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onPreferredClosedCaptionServiceChanged(const string service)
{
    TEST_LOG("OnPreferredClosedCaptionServiceChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    TEST_LOG("OnPreferredClosedCaptionServiceChanged received: %s\n", service.c_str());

    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onPreferredClosedCaptionServiceChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onPinControlChanged(bool pinControl)
{
    TEST_LOG("OnPinControlChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);
    std::string str = pinControl ? "true" : "false";

    TEST_LOG("OnPinControlChanged received: %s\n", str.c_str());
    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onPinControlChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onViewingRestrictionsChanged(const string viewingRestrictions)
{
    TEST_LOG("OnViewingRestrictionsChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    TEST_LOG("OnViewingRestrictionsChanged received: %s\n", viewingRestrictions.c_str());
    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onViewingRestrictionsChanged;
    m_condition_variable.notify_one();

}

void UserSettingTest::onViewingRestrictionsWindowChanged(const string viewingRestrictionsWindow)
{
    TEST_LOG("OnViewingRestrictionsWindowChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    TEST_LOG("OnViewingRestrictionsWindowChanged received: %s\n", viewingRestrictionsWindow.c_str());
    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onViewingRestrictionsWindowChanged;
    m_condition_variable.notify_one();

}

void UserSettingTest::onLiveWatershedChanged(const bool liveWatershed)
{
    TEST_LOG("OnLiveWatershedChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);
    std::string str = liveWatershed ? "true" : "false";

    TEST_LOG("OnLiveWatershedChanged received: %s\n", str.c_str());
    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onLiveWatershedChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onPlaybackWatershedChanged(const bool playbackWatershed)
{
    TEST_LOG("OnPlaybackWatershedChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);
    std::string str = playbackWatershed ? "true" : "false";

    TEST_LOG("OnPlaybackWatershedChanged received: %s\n", str.c_str());
    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onPlaybackWatershedChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onBlockNotRatedContentChanged(const bool blockNotRatedContent)
{
    TEST_LOG("OnBlockNotRatedContentChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);
    std::string str = blockNotRatedContent ? "true" : "false";

    TEST_LOG("OnBlockNotRatedContentChanged received: %s\n", str.c_str());
    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onBlockNotRatedContentChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onPinOnPurchaseChanged(const bool pinOnPurchase)
{
    TEST_LOG("OnPinOnPurchaseChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);
    std::string str = pinOnPurchase ? "true" : "false";

    TEST_LOG("OnPinOnPurchaseChanged received: %s\n", str.c_str());
    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onPinOnPurchaseChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onHighContrastChanged(const bool enabled)
{
    TEST_LOG("OnHighContrastChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);
    std::string str = enabled ? "true" : "false";

    TEST_LOG("OnHighContrastChanged received: %s\n", str.c_str());
    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onHighContrastChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onVoiceGuidanceChanged(const bool enabled)
{
    TEST_LOG("OnVoiceGuidanceChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);
    std::string str = enabled ? "true" : "false";

    TEST_LOG("OnVoiceGuidanceChanged received: %s\n", str.c_str());
    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onVoiceGuidanceChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onVoiceGuidanceRateChanged(const double rate)
{
    TEST_LOG("OnVoiceGuidanceRateChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    TEST_LOG("OnVoiceGuidanceRateChanged received: %ld\n", rate);
    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onVoiceGuidanceRateChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onVoiceGuidanceHintsChanged(const bool hints)
{
    TEST_LOG("OnVoiceGuidanceHintsChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);
    std::string str = hints ? "true" : "false";

    TEST_LOG("OnVoiceGuidanceHintsChanged received: %s\n", str.c_str());
    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onVoiceGuidanceHintsChanged;
    m_condition_variable.notify_one();
}

void UserSettingTest::onContentPinChanged(const string& contentPin)
{
    TEST_LOG("onContentPinChanged event triggered ***\n");
    std::unique_lock<std::mutex> lock(m_mutex);

    TEST_LOG("onContentPinChanged received: %s\n", contentPin.c_str());
    /* Notify the requester thread. */
    m_event_signalled |= UserSettings_onContentPinChanged;
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

MATCHER_P(MatchRequestStatusDouble, expected, "")
{
    double actual = arg;
    std::string expected_str = std::to_string(expected);
    std::string actual_str = std::to_string(actual);
    TEST_LOG("Expected: %s, Actual: %s", expected_str.c_str(), actual_str.c_str());
    EXPECT_STREQ(expected_str.c_str(), actual_str.c_str());
    return expected == actual;

}

TEST_F(UserSettingTest, SetAndGetMethodsUsingJsonRpcConnectionSuccessCase)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(USERSETTING_CALLSIGN, USERSETTINGL2TEST_CALLSIGN);
    StrictMock<AsyncHandlerMock_UserSetting> async_handler;
    uint32_t status = Core::ERROR_GENERAL;
    uint32_t signalled = UserSettings_StateInvalid;

    bool enabled = true;
    bool pinControl = true;
    bool liveWatershed = true;
    bool playbackWatershed = true;
    bool blockNotRatedContent = true;
    bool pinOnPurchase = true;
    bool hints = true;

    string preferredLanguages = "en";
    string presentationLanguage = "fra";
    string preferredCaptionsLanguages = "en,es";
    string preferredService = "CC3";
    string viewingRestrictions = "ALWAYS";
    string contentPin = "1234";
    double rate = 1;

    Core::JSON::String result_string;
    Core::JSON::Boolean result_bool;
    Core::JSON::Double result_double;
    JsonObject result_json;
    JsonObject paramsMigrationState;

    TEST_LOG("Testing getMigrationState before migrating properties");

    paramsMigrationState["key"] = "PREFERRED_AUDIO_LANGUAGES";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "AUDIO_DESCRIPTION";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "CAPTIONS";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "PREFERRED_CAPTIONS_LANGUAGES";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "PREFERRED_CLOSED_CAPTION_SERVICE";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "PRESENTATION_LANGUAGE";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "HIGH_CONTRAST";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "PIN_CONTROL";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "VIEWING_RESTRICTIONS";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "VIEWING_RESTRICTIONS_WINDOW";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "LIVE_WATERSHED";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "PLAYBACK_WATERSHED";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "BLOCK_NOT_RATED_CONTENT";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "PIN_ON_PURCHASE";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "VOICE_GUIDANCE";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "VOICE_GUIDANCE_RATE";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "VOICE_GUIDANCE_HINTS";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "CONTENT_PIN";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());
    paramsMigrationState.Clear();

    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationStates", paramsMigrationState, result_json);
    EXPECT_EQ(status, Core::ERROR_NONE);

    TEST_LOG("Testing AudioDescriptionSuccess");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                       _T("onAudioDescriptionChanged"),
                                       [this, &async_handler](const JsonObject& parameters) {
                                           bool enabled = parameters["enabled"].Boolean();
                                           async_handler.onAudioDescriptionChanged(enabled);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onAudioDescriptionChanged(MatchRequestStatusBool(enabled)))
    .WillOnce(Invoke(this, &UserSettingTest::onAudioDescriptionChanged));

    JsonObject paramsAudioDes;
    paramsAudioDes["enabled"] = true;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setAudioDescription", paramsAudioDes, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onAudioDescriptionChanged);
    EXPECT_TRUE(signalled & UserSettings_onAudioDescriptionChanged);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onAudioDescriptionChanged"));
    EXPECT_EQ(status,Core::ERROR_NONE);

    status = InvokeServiceMethod("org.rdk.UserSettings", "getAudioDescription", result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());

    TEST_LOG("Testing PreferredAudioLanguagesSuccess");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onPreferredAudioLanguagesChanged"),
                                           [&async_handler](const JsonObject& parameters) {
                                           string preferredLanguages = parameters["preferredLanguages"].String();
                                           async_handler.onPreferredAudioLanguagesChanged(preferredLanguages);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onPreferredAudioLanguagesChanged(MatchRequestStatusString(preferredLanguages)))
    .WillOnce(Invoke(this, &UserSettingTest::onPreferredAudioLanguagesChanged));

    JsonObject paramsAudioLanguage;
    paramsAudioLanguage["preferredLanguages"] = preferredLanguages;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setPreferredAudioLanguages", paramsAudioLanguage, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onPreferredAudioLanguagesChanged);
    EXPECT_TRUE(signalled & UserSettings_onPreferredAudioLanguagesChanged);
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onPreferredAudioLanguagesChanged"));

    status = InvokeServiceMethod("org.rdk.UserSettings", "getPreferredAudioLanguages", result_string);
    EXPECT_EQ(status,Core::ERROR_NONE);
    EXPECT_EQ(result_string.Value(), preferredLanguages);

    TEST_LOG("Testing PresentationLanguageSuccess");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onPresentationLanguageChanged"),
                                           [&async_handler](const JsonObject& parameters) {
                                           string presentationLanguage = parameters["presentationLanguage"].String();
                                           async_handler.onPresentationLanguageChanged(presentationLanguage);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onPresentationLanguageChanged(MatchRequestStatusString(presentationLanguage)))
    .WillOnce(Invoke(this, &UserSettingTest::onPresentationLanguageChanged));

    JsonObject paramsPresLanguage;
    paramsPresLanguage["presentationLanguage"] = presentationLanguage;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setPresentationLanguage", paramsPresLanguage, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onPresentationLanguageChanged);
    EXPECT_TRUE(signalled & UserSettings_onPresentationLanguageChanged);
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onPresentationLanguageChanged"));

    status = InvokeServiceMethod("org.rdk.UserSettings", "getPresentationLanguage", result_string);
    EXPECT_EQ(status,Core::ERROR_NONE);
    EXPECT_EQ(result_string.Value(), presentationLanguage);

    TEST_LOG("Testing SetCaptionsSuccess");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                       _T("onCaptionsChanged"),
                                       [this, &async_handler](const JsonObject& parameters) {
                                           bool enabled = parameters["enabled"].Boolean();
                                           async_handler.onCaptionsChanged(enabled);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onCaptionsChanged(MatchRequestStatusBool(enabled)))
    .WillOnce(Invoke(this, &UserSettingTest::onCaptionsChanged));

    JsonObject paramsCaptions;
    paramsCaptions["enabled"] = true;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setCaptions", paramsCaptions, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onCaptionsChanged);
    EXPECT_TRUE(signalled & UserSettings_onCaptionsChanged);
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onCaptionsChanged"));

    status = InvokeServiceMethod("org.rdk.UserSettings", "getCaptions", result_bool);
    EXPECT_EQ(status,Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());

    TEST_LOG("Testing SetPreferredCaptionsLanguagesSuccess");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onPreferredCaptionsLanguagesChanged"),
                                           [&async_handler](const JsonObject& parameters) {
                                           string preferredCaptionsLanguages = parameters["preferredLanguages"].String();
                                           async_handler.onPreferredCaptionsLanguagesChanged(preferredCaptionsLanguages);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onPreferredCaptionsLanguagesChanged(MatchRequestStatusString(preferredCaptionsLanguages)))
    .WillOnce(Invoke(this, &UserSettingTest::onPreferredCaptionsLanguagesChanged));

    JsonObject paramsPrefLang;
    paramsPrefLang["preferredLanguages"] = preferredCaptionsLanguages;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setPreferredCaptionsLanguages", paramsPrefLang, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onPreferredCaptionsLanguagesChanged);
    EXPECT_TRUE(signalled & UserSettings_onPreferredCaptionsLanguagesChanged);
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("OnPreferredCaptionsLanguagesChanged"));

    status = InvokeServiceMethod("org.rdk.UserSettings", "getPreferredCaptionsLanguages", result_string);
    EXPECT_EQ(status,Core::ERROR_NONE);
    EXPECT_EQ(result_string.Value(), preferredCaptionsLanguages);

    TEST_LOG("Testing SetPreferredClosedCaptionServiceSuccess");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onPreferredClosedCaptionServiceChanged"),
                                           [&async_handler](const JsonObject& parameters) {
                                           string preferredService = parameters["service"].String();
                                           async_handler.onPreferredClosedCaptionServiceChanged(preferredService);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onPreferredClosedCaptionServiceChanged(MatchRequestStatusString(preferredService)))
    .WillOnce(Invoke(this, &UserSettingTest::onPreferredClosedCaptionServiceChanged));

    JsonObject paramspreferredService;
    paramspreferredService["service"] = preferredService;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setPreferredClosedCaptionService", paramspreferredService, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onPreferredClosedCaptionServiceChanged);
    EXPECT_TRUE(signalled & UserSettings_onPreferredClosedCaptionServiceChanged);
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onPreferredClosedCaptionServiceChanged"));

    status = InvokeServiceMethod("org.rdk.UserSettings", "getPreferredClosedCaptionService", result_string);
    EXPECT_EQ(status,Core::ERROR_NONE);
    EXPECT_EQ(result_string.Value(), preferredService);

    TEST_LOG("Testing PinControl Success");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                       _T("onPinControlChanged"),
                                       [this, &async_handler](const JsonObject& parameters) {
                                           bool pinControl = parameters["pinControl"].Boolean();
                                           async_handler.onPinControlChanged(pinControl);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onPinControlChanged(MatchRequestStatusBool(pinControl)))
    .WillOnce(Invoke(this, &UserSettingTest::onPinControlChanged));

    JsonObject paramsPinControl;
    paramsPinControl["pinControl"] = true;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setPinControl", paramsPinControl, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onPinControlChanged);
    EXPECT_TRUE(signalled & UserSettings_onPinControlChanged);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onPinControlChanged"));
    EXPECT_EQ(status,Core::ERROR_NONE);

    status = InvokeServiceMethod("org.rdk.UserSettings", "getPinControl", result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());

    string viewRes = "{\"restrictions\": [{\"scheme\": \"US_TV\", \"restrict\": [\"TV-Y7/FV\"]}, {\"scheme\": \"MPAA\", \"restrict\": []}]}";
    TEST_LOG("Testing SetViewingRestrictions Success");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onViewingRestrictionsChanged"),
                                           [&async_handler](const JsonObject& parameters) {
                                           string viewRes = parameters["viewingRestrictions"].String();
                                           async_handler.onViewingRestrictionsChanged(viewRes);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onViewingRestrictionsChanged(MatchRequestStatusString(viewRes)))
    .WillOnce(Invoke(this, &UserSettingTest::onViewingRestrictionsChanged));

    JsonObject paramsViewRestrictions;
    paramsViewRestrictions["viewingRestrictions"] = viewRes;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setViewingRestrictions", paramsViewRestrictions, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onViewingRestrictionsChanged);
    EXPECT_TRUE(signalled & UserSettings_onViewingRestrictionsChanged);
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onViewingRestrictionsChanged"));

    status = InvokeServiceMethod("org.rdk.UserSettings", "getViewingRestrictions", result_string);
    EXPECT_EQ(status,Core::ERROR_NONE);
    EXPECT_EQ(result_string.Value(), viewRes);

    string viewResWindow = "ALWAYS";
    TEST_LOG("Testing SetViewingRestrictionsWindow Success");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onViewingRestrictionsWindowChanged"),
                                           [&async_handler](const JsonObject& parameters) {
                                           string viewResWindow = parameters["viewingRestrictionsWindow"].String();
                                           async_handler.onViewingRestrictionsWindowChanged(viewResWindow);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onViewingRestrictionsWindowChanged(MatchRequestStatusString(viewResWindow)))
    .WillOnce(Invoke(this, &UserSettingTest::onViewingRestrictionsWindowChanged));

    JsonObject paramsViewResWindow;
    paramsViewResWindow["viewingRestrictionsWindow"] = viewResWindow;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setViewingRestrictionsWindow", paramsViewResWindow, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onViewingRestrictionsWindowChanged);
    EXPECT_TRUE(signalled & UserSettings_onViewingRestrictionsWindowChanged);
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onViewingRestrictionsWindowChanged"));

    status = InvokeServiceMethod("org.rdk.UserSettings", "getViewingRestrictionsWindow", result_string);
    EXPECT_EQ(status,Core::ERROR_NONE);
    EXPECT_EQ(result_string.Value(), viewResWindow);

    TEST_LOG("Testing LiveWatershed Success");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                       _T("onLiveWatershedChanged"),
                                       [this, &async_handler](const JsonObject& parameters) {
                                           bool liveWatershed = parameters["liveWatershed"].Boolean();
                                           async_handler.onLiveWatershedChanged(liveWatershed);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onLiveWatershedChanged(MatchRequestStatusBool(liveWatershed)))
    .WillOnce(Invoke(this, &UserSettingTest::onLiveWatershedChanged));

    JsonObject paramsLiveWatershed;
    paramsLiveWatershed["liveWatershed"] = true;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setLiveWatershed", paramsLiveWatershed, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onLiveWatershedChanged);
    EXPECT_TRUE(signalled & UserSettings_onLiveWatershedChanged);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onLiveWatershedChanged"));
    EXPECT_EQ(status,Core::ERROR_NONE);

    status = InvokeServiceMethod("org.rdk.UserSettings", "getLiveWatershed", result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());

    TEST_LOG("Testing PlaybackWatershed Success");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                       _T("onPlaybackWatershedChanged"),
                                       [this, &async_handler](const JsonObject& parameters) {
                                           bool playbackWatershed = parameters["playbackWatershed"].Boolean();
                                           async_handler.onPlaybackWatershedChanged(playbackWatershed);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onPlaybackWatershedChanged(MatchRequestStatusBool(playbackWatershed)))
    .WillOnce(Invoke(this, &UserSettingTest::onPlaybackWatershedChanged));

    JsonObject paramsPlaybackWatershed;
    paramsPlaybackWatershed["playbackWatershed"] = true;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setPlaybackWatershed", paramsPlaybackWatershed, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onPlaybackWatershedChanged);
    EXPECT_TRUE(signalled & UserSettings_onPlaybackWatershedChanged);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onPlaybackWatershedChanged"));
    EXPECT_EQ(status,Core::ERROR_NONE);

    status = InvokeServiceMethod("org.rdk.UserSettings", "getPlaybackWatershed", result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());

    TEST_LOG("Testing BlockNotRatedContent Success");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                       _T("onBlockNotRatedContentChanged"),
                                       [this, &async_handler](const JsonObject& parameters) {
                                           bool blockNotRatedContent = parameters["blockNotRatedContent"].Boolean();
                                           async_handler.onBlockNotRatedContentChanged(blockNotRatedContent);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onBlockNotRatedContentChanged(MatchRequestStatusBool(blockNotRatedContent)))
    .WillOnce(Invoke(this, &UserSettingTest::onBlockNotRatedContentChanged));

    JsonObject paramsBlockNotRatedContent;
    paramsBlockNotRatedContent["blockNotRatedContent"] = true;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setBlockNotRatedContent", paramsBlockNotRatedContent, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onBlockNotRatedContentChanged);
    EXPECT_TRUE(signalled & UserSettings_onBlockNotRatedContentChanged);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onBlockNotRatedContentChanged"));
    EXPECT_EQ(status,Core::ERROR_NONE);

    status = InvokeServiceMethod("org.rdk.UserSettings", "getBlockNotRatedContent", result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());

    TEST_LOG("Testing PinOnPurchase Success");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                       _T("onPinOnPurchaseChanged"),
                                       [this, &async_handler](const JsonObject& parameters) {
                                           bool pinOnPurchase = parameters["pinOnPurchase"].Boolean();
                                           async_handler.onPinOnPurchaseChanged(pinOnPurchase);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onPinOnPurchaseChanged(MatchRequestStatusBool(pinOnPurchase)))
    .WillOnce(Invoke(this, &UserSettingTest::onPinOnPurchaseChanged));

    JsonObject paramsPinOnPurchase;
    paramsPinOnPurchase["pinOnPurchase"] = true;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setPinOnPurchase", paramsPinOnPurchase, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onPinOnPurchaseChanged);
    EXPECT_TRUE(signalled & UserSettings_onPinOnPurchaseChanged);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onPinOnPurchaseChanged"));
    EXPECT_EQ(status,Core::ERROR_NONE);

    status = InvokeServiceMethod("org.rdk.UserSettings", "getPinOnPurchase", result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());

    TEST_LOG("Testing HighContrastSuccess");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                       _T("onHighContrastChanged"),
                                       [this, &async_handler](const JsonObject& parameters) {
                                           bool enabled = parameters["enabled"].Boolean();
                                           async_handler.onHighContrastChanged(enabled);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onHighContrastChanged(MatchRequestStatusBool(enabled)))
    .WillOnce(Invoke(this, &UserSettingTest::onHighContrastChanged));

    JsonObject paramsHighContrast;
    paramsHighContrast["enabled"] = true;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setHighContrast", paramsHighContrast, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onHighContrastChanged);
    EXPECT_TRUE(signalled & UserSettings_onHighContrastChanged);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onHighContrastChanged"));
    EXPECT_EQ(status,Core::ERROR_NONE);

    status = InvokeServiceMethod("org.rdk.UserSettings", "getHighContrast", result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());

    TEST_LOG("Testing VoiceGuidanceSuccess");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                       _T("onVoiceGuidanceChanged"),
                                       [this, &async_handler](const JsonObject& parameters) {
                                           bool enabled = parameters["enabled"].Boolean();
                                           async_handler.onVoiceGuidanceChanged(enabled);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onVoiceGuidanceChanged(MatchRequestStatusBool(enabled)))
    .WillOnce(Invoke(this, &UserSettingTest::onVoiceGuidanceChanged));

    JsonObject paramsVoiceGuidance;
    paramsVoiceGuidance["enabled"] = true;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setVoiceGuidance", paramsVoiceGuidance, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onVoiceGuidanceChanged);
    EXPECT_TRUE(signalled & UserSettings_onVoiceGuidanceChanged);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onVoiceGuidanceChanged"));
    EXPECT_EQ(status,Core::ERROR_NONE);

    status = InvokeServiceMethod("org.rdk.UserSettings", "getVoiceGuidance", result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());

    TEST_LOG("Testing VoiceGuidanceRateSuccess");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                       _T("onVoiceGuidanceRateChanged"),
                                       [this, &async_handler](const JsonObject& parameters) {
                                           double rate = parameters["rate"].Double();
                                           async_handler.onVoiceGuidanceRateChanged(rate);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onVoiceGuidanceRateChanged(MatchRequestStatusDouble(rate)))
    .WillOnce(Invoke(this, &UserSettingTest::onVoiceGuidanceRateChanged));

    JsonObject paramsVoiceGuidanceRate;
    paramsVoiceGuidanceRate["rate"] = rate;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setVoiceGuidanceRate", paramsVoiceGuidanceRate, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onVoiceGuidanceRateChanged);
    EXPECT_TRUE(signalled & UserSettings_onVoiceGuidanceRateChanged);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onVoiceGuidanceRateChanged"));
    EXPECT_EQ(status,Core::ERROR_NONE);

    status = InvokeServiceMethod("org.rdk.UserSettings", "getVoiceGuidanceRate", result_double);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_EQ(result_double.Value(), rate);

    TEST_LOG("Testing VoiceGuidanceHintsSuccess");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                       _T("onVoiceGuidanceHintsChanged"),
                                       [this, &async_handler](const JsonObject& parameters) {
                                           bool hints = parameters["hints"].Boolean();
                                           async_handler.onVoiceGuidanceHintsChanged(hints);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onVoiceGuidanceHintsChanged(MatchRequestStatusBool(hints)))
    .WillOnce(Invoke(this, &UserSettingTest::onVoiceGuidanceHintsChanged));

    JsonObject paramsVoiceGuidanceHints;
    paramsVoiceGuidanceHints["hints"] = true;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setVoiceGuidanceHints", paramsVoiceGuidanceHints, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onVoiceGuidanceHintsChanged);
    EXPECT_TRUE(signalled & UserSettings_onVoiceGuidanceHintsChanged);

    /* Unregister for events. */
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onVoiceGuidanceHintsChanged"));
    EXPECT_EQ(status,Core::ERROR_NONE);

    status = InvokeServiceMethod("org.rdk.UserSettings", "getVoiceGuidanceHints", result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_TRUE(result_bool.Value());

    TEST_LOG("Testing SetContentPin and GetContentPin methods");
    status = jsonrpc.Subscribe<JsonObject>(JSON_TIMEOUT,
                                           _T("onContentPinChanged"),
                                           [&async_handler](const JsonObject& parameters) {
                                           string contentPin = parameters["contentPin"].String();
                                           async_handler.onContentPinChanged(contentPin);
                                       });
    EXPECT_EQ(Core::ERROR_NONE, status);

    EXPECT_CALL(async_handler, onContentPinChanged(MatchRequestStatusString(contentPin)))
    .WillOnce(Invoke(this, &UserSettingTest::onContentPinChanged));

    JsonObject paramsContentPin;
    paramsContentPin["contentPin"] = contentPin;
    status = InvokeServiceMethod("org.rdk.UserSettings", "setContentPin", paramsContentPin, result_json);
    EXPECT_EQ(status,Core::ERROR_NONE);

    signalled = WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onContentPinChanged);
    EXPECT_TRUE(signalled & UserSettings_onContentPinChanged);
    jsonrpc.Unsubscribe(JSON_TIMEOUT, _T("onContentPinChanged"));

    status = InvokeServiceMethod("org.rdk.UserSettings", "getContentPin", result_string);
    EXPECT_EQ(status,Core::ERROR_NONE);
    EXPECT_EQ(result_string.Value(), contentPin);

    TEST_LOG("Testing getMigrationState after migrating properties");

    paramsMigrationState["key"] = "PREFERRED_AUDIO_LANGUAGES";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "AUDIO_DESCRIPTION";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "CAPTIONS";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "PREFERRED_CAPTIONS_LANGUAGES";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "PREFERRED_CLOSED_CAPTION_SERVICE";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "PRESENTATION_LANGUAGE";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "HIGH_CONTRAST";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "PIN_CONTROL";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "VIEWING_RESTRICTIONS";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "VIEWING_RESTRICTIONS_WINDOW";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "LIVE_WATERSHED";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "PLAYBACK_WATERSHED";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "BLOCK_NOT_RATED_CONTENT";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "PIN_ON_PURCHASE";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "VOICE_GUIDANCE";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "VOICE_GUIDANCE_RATE";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "VOICE_GUIDANCE_HINTS";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    paramsMigrationState["key"] = "CONTENT_PIN";
    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationState", paramsMigrationState, result_bool);
    EXPECT_EQ(status, Core::ERROR_NONE);
    EXPECT_FALSE(result_bool.Value());
    paramsMigrationState.Clear();

    status = InvokeServiceMethod("org.rdk.UserSettings", "getMigrationStates", paramsMigrationState, result_json);
    EXPECT_EQ(status, Core::ERROR_NONE);

}

/* Activating UserSettings and Persistent store plugins and UserSettings namespace has no entries in db.
   So that we can verify whether UserSettings plugin is receiving default values from PersistentStore or not*/
TEST_F(UserSettingTest, VerifyDefaultValues)
{
    uint32_t status = Core::ERROR_GENERAL;
    uint32_t signalled = UserSettings_StateInvalid;
    Core::Sink<NotificationHandler> notification;
    bool defaultBooleanValue = true;
    string defaultStrValue = "eng";
    double defaultDoubleValue;

    if (CreateUserSettingInterfaceObjectUsingComRPCConnection() != Core::ERROR_NONE)
    {
        TEST_LOG("Invalid Client_UserSettings");
    }
    else
    {
        ASSERT_TRUE(m_controller_usersettings!= nullptr);
        if (m_controller_usersettings)
        {
            ASSERT_TRUE(m_usersettingsplugin!= nullptr);
            if (m_usersettingsplugin)
            {
                m_usersettingsplugin->AddRef();
                m_usersettingsplugin->Register(&notification);

                /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetAudioDescription(defaultBooleanValue);
                EXPECT_EQ(defaultBooleanValue, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultStrValue should get empty string and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetPreferredAudioLanguages(defaultStrValue);
                EXPECT_EQ(defaultStrValue, "");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultStrValue should get empty string and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetPresentationLanguage(defaultStrValue);
                EXPECT_EQ(defaultStrValue, "");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetCaptions(defaultBooleanValue);
                EXPECT_EQ(defaultBooleanValue, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultStrValue should get empty string and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetPreferredCaptionsLanguages(defaultStrValue);
                EXPECT_EQ(defaultStrValue, "");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultStrValue should get "AUTO" and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetPreferredClosedCaptionService(defaultStrValue);
                EXPECT_EQ(defaultStrValue, "AUTO");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                 /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                 status = m_usersettingsplugin->GetPinControl(defaultBooleanValue);
                 EXPECT_EQ(defaultBooleanValue, false);
                 EXPECT_EQ(status,Core::ERROR_NONE);
                 if (status != Core::ERROR_NONE)
                 {
                     std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                     TEST_LOG("Err: %s", errorMsg.c_str());
                 }


                 /* defaultStrValue should get "" and the return status is Core::ERROR_NONE */
                 status = m_usersettingsplugin->GetViewingRestrictions(defaultStrValue);
                 EXPECT_EQ(defaultStrValue, "");
                 EXPECT_EQ(status,Core::ERROR_NONE);
                 if (status != Core::ERROR_NONE)
                 {
                     std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                     TEST_LOG("Err: %s", errorMsg.c_str());
                 }

                 /* defaultStrValue should get "" and the return status is Core::ERROR_NONE */
                 status = m_usersettingsplugin->GetViewingRestrictionsWindow(defaultStrValue);
                 EXPECT_EQ(defaultStrValue, "");
                 EXPECT_EQ(status,Core::ERROR_NONE);
                 if (status != Core::ERROR_NONE)
                 {
                     std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                     TEST_LOG("Err: %s", errorMsg.c_str());
                 }

                 /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                 status = m_usersettingsplugin->GetLiveWatershed(defaultBooleanValue);
                 EXPECT_EQ(defaultBooleanValue, false);
                 EXPECT_EQ(status,Core::ERROR_NONE);
                 if (status != Core::ERROR_NONE)
                 {
                     std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                     TEST_LOG("Err: %s", errorMsg.c_str());
                 }

                 /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                 status = m_usersettingsplugin->GetPlaybackWatershed(defaultBooleanValue);
                 EXPECT_EQ(defaultBooleanValue, false);
                 EXPECT_EQ(status,Core::ERROR_NONE);
                 if (status != Core::ERROR_NONE)
                 {
                     std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                     TEST_LOG("Err: %s", errorMsg.c_str());
                 }

                 /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                 status = m_usersettingsplugin->GetBlockNotRatedContent(defaultBooleanValue);
                 EXPECT_EQ(defaultBooleanValue, false);
                 EXPECT_EQ(status,Core::ERROR_NONE);
                 if (status != Core::ERROR_NONE)
                 {
                     std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                     TEST_LOG("Err: %s", errorMsg.c_str());
                 }

                 /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                 status = m_usersettingsplugin->GetPinOnPurchase(defaultBooleanValue);
                 EXPECT_EQ(defaultBooleanValue, false);
                 EXPECT_EQ(status,Core::ERROR_NONE);
                 if (status != Core::ERROR_NONE)
                 {
                     std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                     TEST_LOG("Err: %s", errorMsg.c_str());
                 }

                 /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                 status = m_usersettingsplugin->GetHighContrast(defaultBooleanValue);
                 EXPECT_EQ(defaultBooleanValue, false);
                 EXPECT_EQ(status,Core::ERROR_NONE);
                 if (status != Core::ERROR_NONE)
                 {
                     std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                     TEST_LOG("Err: %s", errorMsg.c_str());
                 }

                 /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                 status = m_usersettingsplugin->GetVoiceGuidance(defaultBooleanValue);
                 EXPECT_EQ(defaultBooleanValue, false);
                 EXPECT_EQ(status,Core::ERROR_NONE);
                 if (status != Core::ERROR_NONE)
                 {
                     std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                     TEST_LOG("Err: %s", errorMsg.c_str());
                 }

                 /* defaultDoubleValue should get '1' and the return status is Core::ERROR_NONE */
                 status = m_usersettingsplugin->GetVoiceGuidanceRate(defaultDoubleValue);
                 EXPECT_EQ(defaultDoubleValue, 1);
                 EXPECT_EQ(status,Core::ERROR_NONE);
                 if (status != Core::ERROR_NONE)
                 {
                     std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                     TEST_LOG("Err: %s", errorMsg.c_str());
                 }

                 /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                 status = m_usersettingsplugin->GetVoiceGuidanceHints(defaultBooleanValue);
                 EXPECT_EQ(defaultBooleanValue, false);
                 EXPECT_EQ(status,Core::ERROR_NONE);
                 if (status != Core::ERROR_NONE)
                 {
                     std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                     TEST_LOG("Err: %s", errorMsg.c_str());
                 }

                 /* defaultStrValue should get "" and the return status is Core::ERROR_NONE */
                 status = m_usersettingsplugin->GetContentPin(defaultStrValue);
                 EXPECT_EQ(defaultStrValue, "");
                 EXPECT_EQ(status,Core::ERROR_NONE);
                 if (status != Core::ERROR_NONE)
                 {
                     std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                     TEST_LOG("Err: %s", errorMsg.c_str());
                 }

                 /* Setting Audio Description value as true.So UserSettings namespace has one entry in db.
                 But we are trying to get PreferredAudioLanguages, which has no entry in db.
                 So GetPreferredAudioLanguages should return the empty string and the return status
                 from Persistant store is  Core::ERROR_UNKNOWN_KEY and return status from usersettings is Core::ERROR_NONE */
                 status = m_usersettingsplugin->SetAudioDescription(defaultBooleanValue);
                 EXPECT_EQ(status,Core::ERROR_NONE);
                 if (status != Core::ERROR_NONE)
                 {
                     std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                     TEST_LOG("Err: %s", errorMsg.c_str());
                 }

                 signalled = notification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onAudioDescriptionChanged);
                 EXPECT_TRUE(signalled & UserSettings_onAudioDescriptionChanged);

                 /* We are trying to get PreferredAudioLanguages, which has no entry in db.
                 Persistant store returns status as Core::ERROR_UNKNOWN_KEY to UserSettings 
                 GetPreferredAudioLanguages should get the empty string.*/
                status = m_usersettingsplugin->GetPreferredAudioLanguages(defaultStrValue);
                EXPECT_EQ(defaultStrValue, "");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* We are trying to get PresentationLanguage, which has no entry in db.
                Persistant store returns status as Core::ERROR_UNKNOWN_KEY to UserSettings 
                GetPreferredAudioLanguages should get the empty string.*/
                status = m_usersettingsplugin->GetPresentationLanguage(defaultStrValue);
                EXPECT_EQ(defaultStrValue, "");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* We are trying to get Captions, which has no entry in db.
                Persistant store returns status as Core::ERROR_UNKNOWN_KEY to UserSettings 
                GetPreferredAudioLanguages should get the empty string.*/
                status = m_usersettingsplugin->GetCaptions(defaultBooleanValue);
                EXPECT_EQ(defaultBooleanValue, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* We are trying to get PreferredCaptionsLanguages, which has no entry in db.
                Persistant store returns status as Core::ERROR_UNKNOWN_KEY to UserSettings 
                GetPreferredAudioLanguages should get the empty string.*/
                status = m_usersettingsplugin->GetPreferredCaptionsLanguages(defaultStrValue);
                EXPECT_EQ(defaultStrValue, "");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* We are trying to get PreferredClosedCaptionService, which has no entry in db.
                Persistant store returns status as Core::ERROR_UNKNOWN_KEY to UserSettings 
                GetPreferredAudioLanguages should get the empty string.*/
                status = m_usersettingsplugin->GetPreferredClosedCaptionService(defaultStrValue);
                EXPECT_EQ(defaultStrValue, "AUTO");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetPinControl(defaultBooleanValue);
                EXPECT_EQ(defaultBooleanValue, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultStrValue should get "" and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetViewingRestrictions(defaultStrValue);
                EXPECT_EQ(defaultStrValue, "");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultStrValue should get "" and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetViewingRestrictionsWindow(defaultStrValue);
                EXPECT_EQ(defaultStrValue, "");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetLiveWatershed(defaultBooleanValue);
                EXPECT_EQ(defaultBooleanValue, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetPlaybackWatershed(defaultBooleanValue);
                EXPECT_EQ(defaultBooleanValue, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetBlockNotRatedContent(defaultBooleanValue);
                EXPECT_EQ(defaultBooleanValue, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetPinOnPurchase(defaultBooleanValue);
                EXPECT_EQ(defaultBooleanValue, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetHighContrast(defaultBooleanValue);
                EXPECT_EQ(defaultBooleanValue, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetVoiceGuidance(defaultBooleanValue);
                EXPECT_EQ(defaultBooleanValue, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultDoubleValue should get '1' and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetVoiceGuidanceRate(defaultDoubleValue);
                EXPECT_EQ(defaultDoubleValue, 1);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultBooleanValue should get false and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetVoiceGuidanceHints(defaultBooleanValue);
                EXPECT_EQ(defaultBooleanValue, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                /* defaultStrValue should get "" and the return status is Core::ERROR_NONE */
                status = m_usersettingsplugin->GetContentPin(defaultStrValue);
                EXPECT_EQ(defaultStrValue, "");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                m_usersettingsplugin->Unregister(&notification);
                m_usersettingsplugin->Release();
            }
            else
            {
                TEST_LOG("m_usersettingsplugin is NULL");
            }
            m_controller_usersettings->Release();
        }
        else
        {
            TEST_LOG("m_controller_usersettings is NULL");
        }
    }
}

TEST_F(UserSettingTest,SetAndGetMethodsUsingComRpcConnectionSuccessCase)
{
    uint32_t status = Core::ERROR_GENERAL;
    bool getBoolValue = false;
    string getStringValue = "";
    double getDoubleValue = 0;
    Core::Sink<NotificationHandler> notification;
    uint32_t signalled = UserSettings_StateInvalid;

    if (CreateUserSettingInterfaceObjectUsingComRPCConnection() != Core::ERROR_NONE)
    {
        TEST_LOG("Invalid Client_UserSettings");
    }
    else
    {
        ASSERT_TRUE(m_controller_usersettings!= nullptr);
        if (m_controller_usersettings)
        {
            ASSERT_TRUE(m_usersettingsplugin!= nullptr);
            if ((m_usersettingsplugin!= nullptr) && (m_usersettings_inspe_plugin!= nullptr))
            {
                m_usersettingsplugin->AddRef();
                m_usersettingsplugin->Register(&notification);
                m_usersettings_inspe_plugin->AddRef();

                bool requiresMigration;
                Exchange::IUserSettingsInspector::IUserSettingsMigrationStateIterator *states;

                TEST_LOG("Testing getMigrationState before migrating properties");
                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PREFERRED_AUDIO_LANGUAGES, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::AUDIO_DESCRIPTION, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::CAPTIONS, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PREFERRED_CAPTIONS_LANGUAGES, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PREFERRED_CLOSED_CAPTION_SERVICE, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PRESENTATION_LANGUAGE, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::HIGH_CONTRAST, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PIN_CONTROL, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::VIEWING_RESTRICTIONS, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::VIEWING_RESTRICTIONS_WINDOW, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::LIVE_WATERSHED, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PLAYBACK_WATERSHED, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::BLOCK_NOT_RATED_CONTENT, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PIN_ON_PURCHASE, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::VOICE_GUIDANCE, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::VOICE_GUIDANCE_RATE, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::VOICE_GUIDANCE_HINTS, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::CONTENT_PIN, requiresMigration);
                EXPECT_EQ(requiresMigration, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationStates(states);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                Exchange::IUserSettingsInspector::SettingsMigrationState migration_states = {};
                if (nullptr != states)
                {
                    while (states->Next(migration_states) == true)
                    {
                        EXPECT_EQ(migration_states.requiresMigration, true);
                    }
                }

                TEST_LOG("Setting and Getting AudioDescription Values");
                status = m_usersettingsplugin->SetAudioDescription(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onAudioDescriptionChanged);
                EXPECT_TRUE(signalled & UserSettings_onAudioDescriptionChanged);

                status = m_usersettingsplugin->GetAudioDescription(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting PreferredAudioLanguages Values");
                status = m_usersettingsplugin->SetPreferredAudioLanguages("eng");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onPreferredAudioLanguagesChanged);
                EXPECT_TRUE(signalled & UserSettings_onPreferredAudioLanguagesChanged);

                status = m_usersettingsplugin->GetPreferredAudioLanguages(getStringValue);
                EXPECT_EQ(getStringValue, "eng");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting PresentationLanguage Values");
                status = m_usersettingsplugin->SetPresentationLanguage("fra");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onPresentationLanguageChanged);
                EXPECT_TRUE(signalled & UserSettings_onPresentationLanguageChanged);

                status = m_usersettingsplugin->GetPresentationLanguage(getStringValue);
                EXPECT_EQ(getStringValue, "fra");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting Captions Values");
                getBoolValue = false;
                status = m_usersettingsplugin->SetCaptions(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onCaptionsChanged);
                EXPECT_TRUE(signalled & UserSettings_onCaptionsChanged);

                status = m_usersettingsplugin->GetCaptions(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting Captions Values");
                status = m_usersettingsplugin->SetPreferredCaptionsLanguages("en,es");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onPreferredCaptionsLanguagesChanged);
                EXPECT_TRUE(signalled & UserSettings_onPreferredCaptionsLanguagesChanged);

                status = m_usersettingsplugin->GetPreferredCaptionsLanguages(getStringValue);
                EXPECT_EQ(getStringValue, "en,es");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting PreferredClosedCaptionService Values");
                status = m_usersettingsplugin->SetPreferredClosedCaptionService("CC3");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onPreferredClosedCaptionServiceChanged);
                EXPECT_TRUE(signalled & UserSettings_onPreferredClosedCaptionServiceChanged);

                status = m_usersettingsplugin->GetPreferredClosedCaptionService(getStringValue);
                EXPECT_EQ(getStringValue, "CC3");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting PinControl Values");
                status = m_usersettingsplugin->SetPinControl(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onPinControlChanged);
                EXPECT_TRUE(signalled & UserSettings_onPinControlChanged);

                status = m_usersettingsplugin->GetPinControl(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                string viewRes = "{\"restrictions\": [{\"scheme\": \"US_TV\", \"restrict\": [\"TV-Y7/FV\"]}, {\"scheme\": \"MPAA\", \"restrict\": []}]}";
                TEST_LOG("Setting and Getting ViewingRestrictions Values");
                status = m_usersettingsplugin->SetViewingRestrictions(viewRes);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onViewingRestrictionsChanged);
                EXPECT_TRUE(signalled & UserSettings_onViewingRestrictionsChanged);

                status = m_usersettingsplugin->GetViewingRestrictions(getStringValue);
                EXPECT_EQ(getStringValue, viewRes);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting ViewingRestrictionsWindow Values");
                status = m_usersettingsplugin->SetViewingRestrictionsWindow("ALWAYS");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onViewingRestrictionsWindowChanged);
                EXPECT_TRUE(signalled & UserSettings_onViewingRestrictionsWindowChanged);

                status = m_usersettingsplugin->GetViewingRestrictionsWindow(getStringValue);
                EXPECT_EQ(getStringValue, "ALWAYS");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting LiveWatershed Values");
                status = m_usersettingsplugin->SetLiveWatershed(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onLiveWatershedChanged);
                EXPECT_TRUE(signalled & UserSettings_onLiveWatershedChanged);

                status = m_usersettingsplugin->GetLiveWatershed(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting PlaybackWatershed Values");
                status = m_usersettingsplugin->SetPlaybackWatershed(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onPlaybackWatershedChanged);
                EXPECT_TRUE(signalled & UserSettings_onPlaybackWatershedChanged);

                status = m_usersettingsplugin->GetPlaybackWatershed(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting BlockNotRatedContent Values");
                status = m_usersettingsplugin->SetBlockNotRatedContent(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onBlockNotRatedContentChanged);
                EXPECT_TRUE(signalled & UserSettings_onBlockNotRatedContentChanged);

                status = m_usersettingsplugin->GetBlockNotRatedContent(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting PinOnPurchase Values");
                status = m_usersettingsplugin->SetPinOnPurchase(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onPinOnPurchaseChanged);
                EXPECT_TRUE(signalled & UserSettings_onPinOnPurchaseChanged);

                status = m_usersettingsplugin->GetPinOnPurchase(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting HighContrast Values");
                status = m_usersettingsplugin->SetHighContrast(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onHighContrastChanged);
                EXPECT_TRUE(signalled & UserSettings_onHighContrastChanged);

                status = m_usersettingsplugin->GetHighContrast(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }


                TEST_LOG("Setting and Getting VoiceGuidance Values");
                status = m_usersettingsplugin->SetVoiceGuidance(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onVoiceGuidanceChanged);
                EXPECT_TRUE(signalled & UserSettings_onVoiceGuidanceChanged);

                status = m_usersettingsplugin->GetVoiceGuidance(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                double rate = 9;
                TEST_LOG("Setting and Getting VoiceGuidanceRate Values");
                status = m_usersettingsplugin->SetVoiceGuidanceRate(rate);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onVoiceGuidanceRateChanged);
                EXPECT_TRUE(signalled & UserSettings_onVoiceGuidanceRateChanged);

                status = m_usersettingsplugin->GetVoiceGuidanceRate(getDoubleValue);
                EXPECT_EQ(getDoubleValue, rate);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting VoiceGuidanceHints Values");
                status = m_usersettingsplugin->SetVoiceGuidanceHints(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onVoiceGuidanceHintsChanged);
                EXPECT_TRUE(signalled & UserSettings_onVoiceGuidanceHintsChanged);

                status = m_usersettingsplugin->GetVoiceGuidanceHints(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                TEST_LOG("Setting and Getting ContentPin Values");
                status = m_usersettingsplugin->SetContentPin("1234");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT,UserSettings_onContentPinChanged);
                EXPECT_TRUE(signalled & UserSettings_onContentPinChanged);

                status = m_usersettingsplugin->GetContentPin(getStringValue);
                EXPECT_EQ(getStringValue, "1234");
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }


                TEST_LOG("Testing getMigrationState after migrating properties");

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PREFERRED_AUDIO_LANGUAGES, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::AUDIO_DESCRIPTION, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::CAPTIONS, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PREFERRED_CAPTIONS_LANGUAGES, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PREFERRED_CLOSED_CAPTION_SERVICE, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PRESENTATION_LANGUAGE, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::HIGH_CONTRAST, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PIN_CONTROL, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::VIEWING_RESTRICTIONS, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::VIEWING_RESTRICTIONS_WINDOW, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::LIVE_WATERSHED, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PLAYBACK_WATERSHED, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::BLOCK_NOT_RATED_CONTENT, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::PIN_ON_PURCHASE, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::VOICE_GUIDANCE, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::VOICE_GUIDANCE_RATE, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::VOICE_GUIDANCE_HINTS, requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationState(Exchange::IUserSettingsInspector::SettingsKey::CONTENT_PIN , requiresMigration);
                EXPECT_EQ(requiresMigration, false);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                status = m_usersettings_inspe_plugin->GetMigrationStates(states);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }

                migration_states = {};
                if (nullptr != states)
                {
                    while (states->Next(migration_states) == true)
                    {
                        EXPECT_EQ(migration_states.requiresMigration, false);
                    }
                }

                m_usersettingsplugin->Unregister(&notification);
                m_usersettingsplugin->Release();
                m_usersettings_inspe_plugin->Release();

            }
            else
            {
                TEST_LOG("m_usersettingsplugin is NULL");
            }
            m_controller_usersettings->Release();
        }
        else
        {
            TEST_LOG("m_controller_usersettings is NULL");
        }
    }
}

TEST_F(UserSettingTest, NoDBFileInPersistentstoreErrorCase)
{
    uint32_t status = Core::ERROR_GENERAL;
    bool getBoolValue = false;
    string getStringValue = "";
    Core::Sink<NotificationHandler> notification;
    uint32_t signalled = UserSettings_StateInvalid;

    if (CreateUserSettingInterfaceObjectUsingComRPCConnection() != Core::ERROR_NONE)
    {
        TEST_LOG("Invalid Client_UserSettings");
    }
    else
    {
        ASSERT_TRUE(m_controller_usersettings!= nullptr);
        if (m_controller_usersettings)
        {
            ASSERT_TRUE(m_usersettingsplugin!= nullptr);
            if (m_usersettingsplugin)
            {
                m_usersettingsplugin->AddRef();
                m_usersettingsplugin->Register(&notification);

                TEST_LOG("Setting and Getting AudioDescription Values");
                status = m_usersettingsplugin->SetAudioDescription(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onAudioDescriptionChanged);
                EXPECT_TRUE(signalled & UserSettings_onAudioDescriptionChanged);

                status = m_usersettingsplugin->GetAudioDescription(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status, Core::ERROR_NONE);

                TEST_LOG("Setting and Getting PinControl Values");
                status = m_usersettingsplugin->SetPinControl(true);
                EXPECT_EQ(status,Core::ERROR_NONE);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onPinControlChanged);
                EXPECT_TRUE(signalled & UserSettings_onPinControlChanged);

                status = m_usersettingsplugin->GetPinControl(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status, Core::ERROR_NONE);

                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
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

                TEST_LOG("Setting and Getting AudioDescription Values after DB file deletion");
                status = m_usersettingsplugin->SetAudioDescription(false);
                EXPECT_EQ(status, Core::ERROR_GENERAL);

                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onAudioDescriptionChanged);
                EXPECT_TRUE(signalled & UserSettings_onAudioDescriptionChanged);

                status = m_usersettingsplugin->GetAudioDescription(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status, Core::ERROR_NONE);

                TEST_LOG("Setting and Getting setPinControl Values after DB file deletion");
                status = m_usersettingsplugin->SetPinControl(false);
                EXPECT_EQ(status, Core::ERROR_GENERAL);

                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onPinControlChanged);
                EXPECT_TRUE(signalled & UserSettings_onPinControlChanged);

                status = m_usersettingsplugin->GetPinControl(getBoolValue);
                EXPECT_EQ(getBoolValue, true);
                EXPECT_EQ(status, Core::ERROR_NONE);

                m_usersettingsplugin->Unregister(&notification);
                m_usersettingsplugin->Release();
            }
            else
            {
                TEST_LOG("m_usersettingsplugin is NULL");
            }
            m_controller_usersettings->Release();
        }
        else
        {
            TEST_LOG("m_controller_usersettings is NULL");
        }
    }
}

TEST_F(UserSettingTest, PersistentstoreIsDeactivatedErrorCase)
{
    uint32_t status = Core::ERROR_GENERAL;
    bool getBoolValue = false;
    string getStringValue = "";
    Core::Sink<NotificationHandler> notification;
    uint32_t signalled = UserSettings_StateInvalid;

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

    if (CreateUserSettingInterfaceObjectUsingComRPCConnection() != Core::ERROR_NONE)
    {
        TEST_LOG("Invalid Client_UserSettings");
    }
    else
    {
        ASSERT_TRUE(m_controller_usersettings!= nullptr);
        if (m_controller_usersettings)
        {
            ASSERT_TRUE(m_usersettingsplugin!= nullptr);
            if (m_usersettingsplugin)
            {
                m_usersettingsplugin->AddRef();
                m_usersettingsplugin->Register(&notification);

                TEST_LOG("Setting and Getting AudioDescription Values");
                status = m_usersettingsplugin->SetAudioDescription(true);
                EXPECT_EQ(status,Core::ERROR_GENERAL);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onAudioDescriptionChanged);
                EXPECT_FALSE(signalled & UserSettings_onAudioDescriptionChanged);

                status = m_usersettingsplugin->GetAudioDescription(getBoolValue);
                EXPECT_EQ(getBoolValue, false);
                EXPECT_EQ(status, Core::ERROR_NONE);

                TEST_LOG("Setting and Getting PinControl Values");
                status = m_usersettingsplugin->SetPinControl(true);
                EXPECT_EQ(status,Core::ERROR_GENERAL);
                if (status != Core::ERROR_NONE)
                {
                    std::string errorMsg = "COM-RPC returned error " + std::to_string(status) + " (" + std::string(Core::ErrorToString(status)) + ")";
                    TEST_LOG("Err: %s", errorMsg.c_str());
                }
                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onPinControlChanged);
                EXPECT_FALSE(signalled & UserSettings_onPinControlChanged);

                status = m_usersettingsplugin->GetPinControl(getBoolValue);
                EXPECT_EQ(getBoolValue, false);
                EXPECT_EQ(status, Core::ERROR_NONE);

                m_usersettingsplugin->Unregister(&notification);
                m_usersettingsplugin->Release();
            }
            else
            {
                TEST_LOG("m_usersettingsplugin is NULL");
            }
            m_controller_usersettings->Release();
        }
        else
        {
            TEST_LOG("m_controller_usersettings is NULL");
        }
    }
}

TEST_F(UserSettingTest, PersistentstoreIsNotActivatedWhileUserSettingsActivatingErrorCase)
{
    uint32_t status = Core::ERROR_GENERAL;
    bool getBoolValue = false;
    string getStringValue = "";
    Core::Sink<NotificationHandler> notification;
    uint32_t signalled = UserSettings_StateInvalid;

    status = DeactivateService("org.rdk.UserSettings");
    EXPECT_EQ(Core::ERROR_NONE, status);
    status = DeactivateService("org.rdk.PersistentStore");
    EXPECT_EQ(Core::ERROR_NONE, status);

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

    sleep(5);
    status = ActivateService("org.rdk.UserSettings");
    EXPECT_EQ(Core::ERROR_NONE, status);

    if (CreateUserSettingInterfaceObjectUsingComRPCConnection() != Core::ERROR_NONE)
    {
        TEST_LOG("Invalid Client_UserSettings");
    }
    else
    {
        ASSERT_TRUE(m_controller_usersettings!= nullptr);
        if (m_controller_usersettings)
        {
            ASSERT_TRUE(m_usersettingsplugin!= nullptr);
            if (m_usersettingsplugin)
            {
                m_usersettingsplugin->AddRef();
                m_usersettingsplugin->Register(&notification);

                TEST_LOG("Setting and Getting AudioDescription Values");
                status = m_usersettingsplugin->SetAudioDescription(true);
                EXPECT_EQ(status,Core::ERROR_GENERAL);

                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onAudioDescriptionChanged);
                EXPECT_FALSE(signalled & UserSettings_onAudioDescriptionChanged);

                status = m_usersettingsplugin->GetAudioDescription(getBoolValue);
                EXPECT_EQ(getBoolValue, false);
                EXPECT_EQ(status, Core::ERROR_GENERAL);

                TEST_LOG("Setting and Getting PinControl Values");
                status = m_usersettingsplugin->SetPinControl(true);
                EXPECT_EQ(status,Core::ERROR_GENERAL);

                signalled = notification.WaitForRequestStatus(JSON_TIMEOUT, UserSettings_onPinControlChanged);
                EXPECT_FALSE(signalled & UserSettings_onPinControlChanged);

                status = m_usersettingsplugin->GetPinControl(getBoolValue);
                EXPECT_EQ(getBoolValue, false);
                EXPECT_EQ(status, Core::ERROR_GENERAL);

                m_usersettingsplugin->Unregister(&notification);
                m_usersettingsplugin->Release();
            }
            else
            {
                TEST_LOG("m_usersettingsplugin is NULL");
            }
            m_controller_usersettings->Release();
        }
        else
        {
            TEST_LOG("m_controller_usersettings is NULL");
        }
    }
}


