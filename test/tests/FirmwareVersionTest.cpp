#include <gtest/gtest.h>

#include "FirmwareVersion.h"

using namespace WPEFramework;

class FirmwareVersionTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::FirmwareVersion> firmwareVersion;
    Exchange::IFirmwareVersion* interface;

    FirmwareVersionTestFixture()
    {
    }
    virtual ~FirmwareVersionTestFixture()
    {
    }

    virtual void SetUp()
    {
        firmwareVersion = Core::ProxyType<Plugin::FirmwareVersion>::Create();
        interface = static_cast<Exchange::IFirmwareVersion*>(
            firmwareVersion->QueryInterface(Exchange::IFirmwareVersion::ID));
        EXPECT_TRUE(interface != nullptr);
    }

    virtual void TearDown()
    {
        interface->Release();
    }
};

TEST_F(FirmwareVersionTestFixture, Imagename)
{
    string imagename;

    EXPECT_EQ(Core::ERROR_NONE, interface->Imagename(imagename));
    EXPECT_EQ(imagename, _T("PX051AEI_VBN_2203_sprint_20220331225312sdy_NG"));
}

TEST_F(FirmwareVersionTestFixture, Sdk)
{
    string sdk;

    EXPECT_EQ(Core::ERROR_NONE, interface->Sdk(sdk));
    EXPECT_EQ(sdk, _T("17.3"));
}

TEST_F(FirmwareVersionTestFixture, Mediarite)
{
    string mediarite;

    EXPECT_EQ(Core::ERROR_NONE, interface->Mediarite(mediarite));
    EXPECT_EQ(mediarite, _T("8.3.53"));
}

TEST_F(FirmwareVersionTestFixture, Yocto)
{
    string yocto;

    EXPECT_EQ(Core::ERROR_NONE, interface->Yocto(yocto));
    EXPECT_EQ(yocto, _T("dunfell"));
}
