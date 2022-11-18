#pragma once

#include <gmock/gmock.h>

#include "rdkshell.h"

class RdkShellApiImplMock : public RdkShell::CompControl {
public:
    RdkShellApiImplMock()
	    : RdkShell::CompControl()
    {
	    EXPECT_CALL(*this, isErmEnabled())
		    .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Return(true));

	    ON_CALL(*this, getOpacity(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [](const std::string& client, unsigned int& opacity) {
                    opacity = 100;
                    return true;
                }));
	    ON_CALL(*this, setScreenResolution(::testing::_, ::testing::_))
	    .WillByDefault(::testing::Invoke(
                [](const uint32_t width, const uint32_t height){
		      return true;
		}));
	    ON_CALL(*this, getScreenResolution(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [](uint32_t &width, uint32_t &height){
		      width = 1920;
		      height = 1080;
                      return true;
                }));
	    ON_CALL(*this, getVirtualResolution(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [](const std::string& client, uint32_t &virtualWidth, uint32_t &virtualHeight){
                      virtualWidth = 1920;
                      virtualHeight = 1080;
                      return true;
                }));
            ON_CALL(*this, setVirtualResolution(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [](const std::string& client, const uint32_t virtualWidth, const uint32_t virtualHeight){
                      return true;
                }));
	    ON_CALL(*this, getClients(::testing::_))
            .WillByDefault(::testing::Invoke(
                [](std::vector<std::string>& clients){
                      clients.push_back("org.rdk.Netflix");
                      clients.push_back("org.rdk.RDKBrowser2");
		      clients.push_back("Test2");
                      return true;
		   }));
	    ON_CALL(*this, getZOrder(::testing::_))
            .WillByDefault(::testing::Invoke(
                [](std::vector<std::string>& clients){
                      clients.push_back("org.rdk.Netflix");
                      clients.push_back("org.rdk.RDKBrowser2");
                      return true;
		  }));
	    ON_CALL(*this, getFocused(::testing::_))
		.WillByDefault(::testing::Invoke(
		 [](std::string& client){
		   return true;
		 }));

    }
    virtual ~RdkShellApiImplMock() = default;

    MOCK_METHOD(bool, setLogLevel, (const std::string level), (override));
    MOCK_METHOD(bool, isErmEnabled, (), (override));
    MOCK_METHOD(bool, scaleToFit, (const std::string& client, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height), (override));
    MOCK_METHOD(bool, ignoreKeyInputs, (bool ignore), (override));
    MOCK_METHOD(bool, moveToFront, (const std::string& client), (override));
    MOCK_METHOD(bool, moveToBack, (const std::string& client), (override));
    MOCK_METHOD(bool, getClients, (std::vector<std::string>& clients), (override));
    MOCK_METHOD(bool, moveBehind, (const std::string& client, const std::string& target), (override));
    MOCK_METHOD(bool, getOpacity, (const std::string& client, unsigned int& opacity), (override));
    MOCK_METHOD(bool, setOpacity, (const std::string& client, const unsigned int opacity), (override));
    MOCK_METHOD(bool, setFocus, (const std::string& client), (override));
    MOCK_METHOD(bool, removeKeyIntercept, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags), (override));
    MOCK_METHOD(bool, removeKeyListener, (const std::string& client, const uint32_t& keyCode, const uint32_t& flags), (override));
    MOCK_METHOD(bool, removeKeyMetadataListener, (const std::string& client), (override));
    MOCK_METHOD(bool, injectKey, (const uint32_t& keyCode, const uint32_t& flags), (override));
    MOCK_METHOD(bool, setScreenResolution, (const uint32_t width, const uint32_t height), (override));
    MOCK_METHOD(bool, getScreenResolution, (uint32_t &width, uint32_t &height), (override));
    MOCK_METHOD(bool, setVirtualResolution, (const std::string& client, const uint32_t virtualWidth, const uint32_t virtualHeight), (override));
    MOCK_METHOD(bool, getVirtualResolution, (const std::string& client, uint32_t &virtualWidth, uint32_t &virtualHeight), (override));
    MOCK_METHOD(bool, getZOrder, (std::vector<std::string>&clients), (override));
    MOCK_METHOD(bool, getVisibility, (const std::string& client, bool& visible), (override));
    MOCK_METHOD(bool, setVisibility, (const std::string& client, const bool visible), (override));
    MOCK_METHOD(bool, getHolePunch, (const std::string& client, bool& holePunch), (override));
    MOCK_METHOD(bool, setHolePunch, (const std::string& client, const bool holePunch), (override));
    MOCK_METHOD(bool, getBounds, (const std::string& client, uint32_t &x, uint32_t &y, uint32_t &width, uint32_t &height), (override));
    MOCK_METHOD(bool, getScale, (const std::string& client, double &scaleX, double &scaleY), (override));
    MOCK_METHOD(bool, setCursorSize, (uint32_t width, uint32_t height), (override));
    MOCK_METHOD(bool, getCursorSize, (uint32_t& width, uint32_t& height), (override));
    MOCK_METHOD(bool, setBounds, (const std::string& client, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height ), (override));
    MOCK_METHOD(bool, setScale, (const std::string& client, double scaleX, double scaleY), (override));
    MOCK_METHOD(bool, getFocused, (std::string& client), (override));
    MOCK_METHOD(void, enableInactivityReporting, (const bool enable), (override));
    MOCK_METHOD(bool, showCursor, (), (override));
    MOCK_METHOD(bool, hideCursor, (), (override));
    MOCK_METHOD(void, setKeyRepeatConfig, (bool enabled, int32_t initialDelay, int32_t repeatInterval), (override));
    MOCK_METHOD(bool, getBlockedAVApplications, (std::vector<std::string>& apps), (override));
    MOCK_METHOD(bool, setAVBlocked, (std::string callsign, bool blockAV), (override));
    MOCK_METHOD(bool, enableInputEvents, (const std::string& client, bool enable), (override));
    MOCK_METHOD(bool, getLogLevel, (std::string& level), (override));
    MOCK_METHOD(bool, enableKeyRepeats, (bool enable), (override));
    MOCK_METHOD(bool, getKeyRepeatsEnabled, (bool& enable), (override));
};

