#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mntent.h>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <cstdio>
#include "UserSettings.h"
#include "UserSettingsImplementation.h"
#include "ServiceMock.h"
#include "Store2Mock.h"
#include "COMLinkMock.h"
#include "WrapsMock.h"
#include "ThunderPortability.h"
#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);

using ::testing::NiceMock;
using namespace WPEFramework;

class UserSettingsTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::UserSettings> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Context connection;
    Core::JSONRPC::Message message;
    NiceMock<ServiceMock> service;
    NiceMock<COMLinkMock> comLinkMock;
    Core::ProxyType<Plugin::UserSettingsImplementation> UserSettingsImpl;
    //Exchange::IUserSettings::INotification *notification = nullptr;
    string response;
    WrapsImplMock *p_wrapsImplMock   = nullptr;
    ServiceMock  *p_serviceMock  = nullptr;
    Store2Mock  *p_store2Mock  = nullptr;
    UserSettingsTest()
        : plugin(Core::ProxyType<Plugin::UserSettings>::Create())
        , handler(*plugin)
        , connection(1,0,"")
    {
        p_serviceMock = new NiceMock <ServiceMock>;

        p_store2Mock = new NiceMock <Store2Mock>;

        p_wrapsImplMock  = new NiceMock <WrapsImplMock>;
        Wraps::setImpl(p_wrapsImplMock);

        EXPECT_CALL(service, QueryInterfaceByCallsign(::testing::_, ::testing::_))
            .WillOnce(testing::Return(p_store2Mock));

        ON_CALL(comLinkMock, Instantiate(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](const RPC::Object& object, const uint32_t waitTime, uint32_t& connectionId) {
                UserSettingsImpl = Core::ProxyType<Plugin::UserSettingsImplementation>::Create();
                return &UserSettingsImpl;
                }));

        plugin->Initialize(&service);
    }

    virtual ~UserSettingsTest() override
    {

        plugin->Deinitialize(&service);

        if (p_serviceMock != nullptr)
        {
            delete p_serviceMock;
            p_serviceMock = nullptr;
        }
        
        if (p_store2Mock != nullptr)
        {
            delete p_store2Mock;
            p_store2Mock = nullptr;
        }

        Wraps::setImpl(nullptr);
        if (p_wrapsImplMock != nullptr)
        {
            delete p_wrapsImplMock;
            p_wrapsImplMock = nullptr;
        }
    }
};

TEST_F(UserSettingsTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setAudioDescription")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getAudioDescription")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPreferredAudioLanguages")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreferredAudioLanguages")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPresentationLanguage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPresentationLanguage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setCaptions")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getCaptions")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPreferredCaptionsLanguages")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreferredCaptionsLanguages")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPreferredClosedCaptionService")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreferredClosedCaptionService")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPinControl")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPinControl")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setViewingRestrictions")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getViewingRestrictions")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setViewingRestrictionsWindow")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getViewingRestrictionsWindow")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setLiveWatershed")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getLiveWatershed")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPlaybackWatershed")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPlaybackWatershed")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setBlockNotRatedContent")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getBlockNotRatedContent")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPinOnPurchase")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPinOnPurchase")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setHighContrast")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getHighContrast")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setVoiceGuidance")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getVoiceGuidance")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setVoiceGuidanceRate")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getVoiceGuidanceRate")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setVoiceGuidanceHints")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getVoiceGuidanceHints")));
}

TEST_F(UserSettingsTest, SetAudioDescription_Failure)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setAudioDescription"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetAudioDescription_Success)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setAudioDescription"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetAudioDescription_Failure)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getAudioDescription"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetAudioDescription_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getAudioDescription"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetPreferredAudioLanguages_Failure)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setPreferredAudioLanguages"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetPreferredAudioLanguages_Success)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPreferredAudioLanguages"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPreferredAudioLanguages_Failure)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPreferredAudioLanguages"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPreferredAudioLanguages_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreferredAudioLanguages"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetPresentationLanguage_Failure)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setPresentationLanguage"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetPresentationLanguage_Success)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPresentationLanguage"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPresentationLanguage_Failure)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPresentationLanguage"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPresentationLanguage_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPresentationLanguage"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetCaptions_Failure)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setCaptions"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetCaptions_Success)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setCaptions"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetCaptions_Failure)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getCaptions"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetCaptions_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getCaptions"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetPreferredCaptionsLanguages_Failure)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setPreferredCaptionsLanguages"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetPreferredCaptionsLanguages_Success)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPreferredCaptionsLanguages"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPreferredCaptionsLanguages_Failure)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPreferredCaptionsLanguages"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPreferredCaptionsLanguages_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreferredCaptionsLanguages"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetPreferredClosedCaptionService_Failure)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setPreferredClosedCaptionService"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetPreferredClosedCaptionService_Success)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPreferredClosedCaptionService"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPreferredClosedCaptionService_Failure)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPreferredClosedCaptionService"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPreferredClosedCaptionService_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreferredClosedCaptionService"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetPinControl_Failure)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setPinControl"), _T("{}"), response));
}

TEST_F(UserSettingsTest, setPinControl_Success)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPinControl"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPinControl_Failure)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPinControl"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPinControl_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPinControl"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetViewingRestrictions_Failure)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setViewingRestrictions"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetViewingRestrictions_Success)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setViewingRestrictions"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetViewingRestrictions_Failure)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getViewingRestrictions"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetViewingRestrictions_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getViewingRestrictions"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetViewingRestrictionsWindow_Failure)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setViewingRestrictionsWindow"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetViewingRestrictionsWindow_Success)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setViewingRestrictionsWindow"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetViewingRestrictionsWindow_Failure)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getViewingRestrictionsWindow"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetViewingRestrictionsWindow_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getViewingRestrictionsWindow"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetLiveWatershed_Failure)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setLiveWatershed"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetLiveWatershed_Success)
{
	EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setLiveWatershed"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetLiveWatershed_Failure)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getLiveWatershed"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetLiveWatershed_Success)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getLiveWatershed"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetPlaybackWatershed_Failure)
{
	EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setPlaybackWatershed"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetPlaybackWatershed_Success)
{
	EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPlaybackWatershed"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPlaybackWatershed_Failure)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPlaybackWatershed"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPlaybackWatershed_Success)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPlaybackWatershed"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetBlockNotRatedContent_Failure)
{
	EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setBlockNotRatedContent"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetBlockNotRatedContent_Success)
{
	EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBlockNotRatedContent"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetBlockNotRatedContent_Failure)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getBlockNotRatedContent"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetBlockNotRatedContent_Success)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBlockNotRatedContent"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetPinOnPurchase_Failure)
{
	EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setPinOnPurchase"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetPinOnPurchase_Success)
{
	EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPinOnPurchase"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPinOnPurchase_Failure)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPinOnPurchase"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetPinOnPurchase_Success)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPinOnPurchase"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetHighContrast_Failure)
{
	EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setHighContrast"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetHighContrast_Success)
{
	EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setHighContrast"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetHighContrast_Failure)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getHighContrast"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetHighContrast_Success)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getHighContrast"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetVoiceGuidance_Failure)
{
	EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setVoiceGuidance"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetVoiceGuidance_Success)
{
	EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVoiceGuidance"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetVoiceGuidance_Failure)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getVoiceGuidance"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetVoiceGuidance_Success)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getVoiceGuidance"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetVoiceGuidanceRate_Failure)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setVoiceGuidanceRate"), _T("{\"rate\":0.1}"), response));
}

TEST_F(UserSettingsTest, SetVoiceGuidanceRate_FailureOutOfRangeMin)
{
    EXPECT_EQ(Core::ERROR_INVALID_PARAMETER, handler.Invoke(connection, _T("setVoiceGuidanceRate"), _T("{\"rate\":0.01}"), response));
}

TEST_F(UserSettingsTest, SetVoiceGuidanceRate_FailureOutOfRangeMax)
{
    EXPECT_EQ(Core::ERROR_INVALID_PARAMETER, handler.Invoke(connection, _T("setVoiceGuidanceRate"), _T("{\"rate\":11}"), response));
}

TEST_F(UserSettingsTest, SetVoiceGuidanceRate_Success)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVoiceGuidanceRate"), _T("{\"rate\":0.1}"), response));
}

TEST_F(UserSettingsTest, GetVoiceGuidanceRate_Failure)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getVoiceGuidanceRate"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetVoiceGuidanceRate_Success)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getVoiceGuidanceRate"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetVoiceGuidanceHints_Failure)
{
	EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setVoiceGuidanceHints"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetVoiceGuidanceHints_Success)
{
	EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVoiceGuidanceHints"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetVoiceGuidanceHints_Failure)
{
	EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getVoiceGuidanceHints"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetVoiceGuidanceHints_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getVoiceGuidanceHints"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetContentPin_Failure)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setContentPin"), _T("{}"), response));
}

TEST_F(UserSettingsTest, SetContentPin_Success)
{
    EXPECT_CALL(*p_store2Mock, SetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setContentPin"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetContentPin_Failure)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getContentPin"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetContentPin_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getContentPin"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetMigrationState_Failure)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_GENERAL));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMigrationState"), _T("{\"key\": \"VOICE_GUIDANCE_RATE\"}"), response));
}

TEST_F(UserSettingsTest, GetMigrationState_RequiresMigrationStateTrue)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NOT_EXIST));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMigrationState"), _T("{\"key\": \"VOICE_GUIDANCE_RATE\"}"), response));
}

TEST_F(UserSettingsTest, GetMigrationState_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMigrationState"), _T("{\"key\": \"VOICE_GUIDANCE_RATE\"}"), response));
}

TEST_F(UserSettingsTest, GetMigrationStates_Failure)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillRepeatedly(::testing::Return(Core::ERROR_GENERAL));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMigrationStates"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetMigrationStates_RequiresMigrationStateTrueForAllProperties)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillRepeatedly(::testing::Return(Core::ERROR_NOT_EXIST));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMigrationStates"), _T("{}"), response));
}

TEST_F(UserSettingsTest, GetMigrationStates_Success)
{
    EXPECT_CALL(*p_store2Mock, GetValue(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillRepeatedly(::testing::Return(Core::ERROR_NONE));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMigrationStates"), _T("{}"), response));
}

