#include <gtest/gtest.h>

#include "UsbAccess.h"

#include "UdevMock.h"
#include "WrapsMock.h"

using namespace WPEFramework;

class UsbAccessTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::UsbAccess> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    UsbAccessTest()
        : plugin(Core::ProxyType<Plugin::UsbAccess>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
};

TEST_F(UsbAccessTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFileList")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("createLink")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("clearLink")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getLinks")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getAvailableFirmwareFiles")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMounted")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("updateFirmware")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("ArchiveLogs")));
}

TEST_F(UsbAccessTest, UpdateFirmware)
{
    UdevImplMock   *p_udevImplMock = nullptr ;
    WrapsImplMock  *p_wrapsImplMock   = nullptr ;

    p_wrapsImplMock  = new testing::NiceMock <WrapsImplMock>;
    Wraps::setImpl(p_wrapsImplMock);

    p_udevImplMock  = new testing::NiceMock <UdevImplMock>;
    Udev::setImpl(p_udevImplMock);

    EXPECT_CALL(*p_wrapsImplMock, system(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/userInitiatedFWDnld.sh usb '/tmp;reboot;' 'my.bin' 0 >> /opt/logs/swupdate.log &")));

                return 0;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updateFirmware"), _T("{\"fileName\":\"/tmp;reboot;/my.bin\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("updateFirmware"), _T("{\"fileName\":\"/tmp\';reboot;/my.bin\"}"), response));

    Udev::setImpl(nullptr);
    if (p_udevImplMock != nullptr)
    {
        delete p_udevImplMock;
        p_udevImplMock = nullptr;
    }

    Wraps::setImpl(nullptr);
    if (p_wrapsImplMock != nullptr)
    {
        delete p_wrapsImplMock;
        p_wrapsImplMock = nullptr;
    }
}
