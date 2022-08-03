#include <gtest/gtest.h>

#include "FirmwareVersion.h"

using namespace WPEFramework;

class FirmwareVersionTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::FirmwareVersion> firmwareVersion;
    Exchange::IFirmwareVersion* interface;

    FirmwareVersionTest()
        : firmwareVersion(Core::ProxyType<Plugin::FirmwareVersion>::Create())
    {
    }
    virtual ~FirmwareVersionTest()
    {
    }

    virtual void SetUp()
    {
        interface = static_cast<Exchange::IFirmwareVersion*>(
            firmwareVersion->QueryInterface(Exchange::IFirmwareVersion::ID));
        ASSERT_TRUE(interface != nullptr);
    }

    virtual void TearDown()
    {
        ASSERT_TRUE(interface != nullptr);
        interface->Release();
    }
};

TEST_F(FirmwareVersionTest, Imagename)
{
    string imagename;

    EXPECT_EQ(Core::ERROR_NONE, interface->Imagename(imagename));
    EXPECT_EQ(imagename, _T("PX051AEI_VBN_2203_sprint_20220331225312sdy_NG"));
}

TEST_F(FirmwareVersionTest, Sdk)
{
    string sdk;

    EXPECT_EQ(Core::ERROR_NONE, interface->Sdk(sdk));
    EXPECT_EQ(sdk, _T("17.3"));
}

TEST_F(FirmwareVersionTest, Mediarite)
{
    string mediarite;

    EXPECT_EQ(Core::ERROR_NONE, interface->Mediarite(mediarite));
    EXPECT_EQ(mediarite, _T("8.3.53"));
}

TEST_F(FirmwareVersionTest, Yocto)
{
    string yocto;

    EXPECT_EQ(Core::ERROR_NONE, interface->Yocto(yocto));
    EXPECT_EQ(yocto, _T("dunfell"));
}
