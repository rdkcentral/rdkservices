#include <gtest/gtest.h>

#include "RDKShell.h"


using namespace WPEFramework;

class RDKShellTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::RDKShell> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;
    CompositorControllerMock compositorControllerMock;

    RDKShellTest()
        : plugin(Core::ProxyType<Plugin::RDKShell>::Create())
        , handler(*(plugin))
        , connection(1, 0)
        {
        }
        virtual ~RDKShellTest() = default;
};



TEST_F(RDKShellTest, RegisteredMethods){
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("addAnimation")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("addKeyIntercept")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("addKeyIntercepts")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("addKeyListener")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("addKeyMetadataListener")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("exitAgingMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("createDisplay")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("destroy")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("enableInactivityReporting")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("enableKeyRepeats")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("enableLogsFlushing")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("enableVirtualDisplay")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("generateKey")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getAvailableTypes")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getBounds")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getClients")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getCursorSize")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getHolePunch")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getKeyRepeatsEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getLastWakeupKey")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getLogsFlushingEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getOpacity")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getScale")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getScreenResolution")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getScreenshot")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getState")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSystemMemory")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSystemResourceInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getVirtualDisplayEnabled")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getVirtualResolution")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getVisibility")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getZOrder")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getGraphicsFrameRate")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("hideAllClients")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("hideCursor")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("hideFullScreenImage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("hideSplashLogo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("ignoreKeyInputs")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("injectKey")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("kill")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("launch")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("launchApplication")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("launchResidentApp")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("moveBehind")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("moveToBack")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("moveToFront")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("removeAnimation")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("removeKeyIntercept")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("removeKeyListener")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("removeKeyMetadataListener")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("resetInactivityTime")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("resumeApplication")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("scaleToFit")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setBounds")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setCursorSize")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setFocus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setHolePunch")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setInactivityInterval")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setLogLevel")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setMemoryMonitor")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setOpacity")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setScale")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setScreenResolution")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setTopmost")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setVirtualResolution")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setVisibility")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setGraphicsFrameRate")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("showCursor")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("showFullScreenImage")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("showSplashLogo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("showWatermark")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("suspend")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("suspendApplication")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("keyRepeatConfig")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setAVBlocked")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getBlockedAVApplications")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("addEasterEggs")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("removeEasterEggs")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("enableEasterEggs")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getEasterEggs")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("launchFactoryApp")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("launchFactoryAppShortcut")));
    }

TEST_F(RDKShellTest, visibility)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVisibility"), _T("{\"client\": \"org.rdk.Netflix\","
                                                                                             "\"callsign\": \"org.rdk.Netflix\","
                                                                                             "\"visible\": true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(RDKShellTest, getSystemMemory)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSystemMemory"), _T("{}"), response));
}

TEST_F(RDKShellTest, Bounds)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setBounds"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getBounds"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBounds"), _T("{\"client\": \"org.rdk.Netflix\","
                                                                                             "\"callsign\": \"org.rdk.Netflix\"}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBounds"), _T("{\"client\": \"org.rdk.Netflix\","
                                                                                             "\"callsign\": \"org.rdk.Netflix\",\"x\":10,\"y\":15,\"w\":1000,\"h\":1000}"),response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getBounds"), _T("{\"client\": \"org.rdk.Netflix\","
                                                                                             "\"callsign\": \"org.rdk.Netflix\"}"), response));
}

TEST_F(RDKShellTest, CursorSize)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setCursorSize"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getCursorSize"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setCursorSize"), _T("{\"width\":255,\"height\":255}"),response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getCursorSize"), _T("{}"), response));
}

TEST_F(RDKShellTest, showandhideCursor)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("showCursor"), _T("{}"),response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("hideCursor"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
}

TEST_F(RDKShellTest, launchResidentApp)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("launchResidentApp"), _T("{}"), response));
}


TEST_F(RDKShellTest, keyRepeat)
{
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("enableKeyRepeats"), _T("{}"), response));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enableKeyRepeats"), _T("{\"enable\": \"true\"}"), response));
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getKeyRepeatsEnabled"), _T("{}"), response));

}
