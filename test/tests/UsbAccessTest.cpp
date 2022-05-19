#include <gtest/gtest.h>

#include "UsbAccess.h"

#include "UdevMock.h"
#include "WrapsMock.h"

using namespace WPEFramework;

class UsbAccessTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::UsbAccess> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Handler& handlerV2;
    Core::JSONRPC::Connection connection;
    UdevImplMock udevImplMock;
    WrapsImplMock wrapsImplMock;
    string response;

    UsbAccessTestFixture()
        : plugin(Core::ProxyType<Plugin::UsbAccess>::Create())
        , handler(*(plugin))
        , handlerV2(*(plugin->GetHandler(2)))
        , connection(1, 0)
    {
    }
    virtual ~UsbAccessTestFixture()
    {
    }

    virtual void SetUp()
    {
        Udev::getInstance().impl = &udevImplMock;
        Wraps::getInstance().impl = &wrapsImplMock;
    }

    virtual void TearDown()
    {
        Udev::getInstance().impl = nullptr;
        Wraps::getInstance().impl = nullptr;
    }
};

TEST_F(UsbAccessTestFixture, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFileList")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("createLink")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("clearLink")));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getFileList")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("createLink")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("clearLink")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getAvailableFirmwareFiles")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getMounted")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("updateFirmware")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("ArchiveLogs")));
}

TEST_F(UsbAccessTestFixture, UpdateFirmware)
{
    EXPECT_CALL(wrapsImplMock, system(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/userInitiatedFWDnld.sh usb '/tmp;reboot;' 'my.bin' 0 >> /opt/logs/swupdate.log &")));

                return 0;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("updateFirmware"), _T("{\"fileName\":\"/tmp;reboot;/my.bin\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("updateFirmware"), _T("{\"fileName\":\"/tmp\';reboot;/my.bin\"}"), response));
    EXPECT_EQ(response, string("{\"error\":\"invalid filename\",\"success\":false}"));
}
