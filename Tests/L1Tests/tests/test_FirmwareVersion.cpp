#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "WrapsMock.h"

#include "Implementation/FirmwareVersion.h"

#include <fstream>

using namespace WPEFramework;
using ::testing::NiceMock;

class FirmwareVersionTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::FirmwareVersion> firmwareVersion;
    Exchange::IFirmwareVersion* interface;
    WrapsImplMock *p_wrapsImplMock = nullptr;

    FirmwareVersionTest()
        : firmwareVersion(Core::ProxyType<Plugin::FirmwareVersion>::Create())
    {
        interface = static_cast<Exchange::IFirmwareVersion*>(
            firmwareVersion->QueryInterface(Exchange::IFirmwareVersion::ID));
         p_wrapsImplMock = new NiceMock<WrapsImplMock>;
         Wraps::setImpl(p_wrapsImplMock);
    }
    virtual ~FirmwareVersionTest()
    {
        if (p_wrapsImplMock != nullptr)
        {
            delete p_wrapsImplMock;
            p_wrapsImplMock = nullptr;
        }
        interface->Release();
    }

    virtual void SetUp()
    {
        ASSERT_TRUE(interface != nullptr);
    }

    virtual void TearDown()
    {
        ASSERT_TRUE(interface != nullptr);
    }
};

TEST_F(FirmwareVersionTest, Imagename)
{
    std::ofstream file("/version.txt");
    file << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG";
    file.close();

    string imagename;
    EXPECT_EQ(Core::ERROR_NONE, interface->Imagename(imagename));
    EXPECT_EQ(imagename, _T("PX051AEI_VBN_2203_sprint_20220331225312sdy_NG"));
}

TEST_F(FirmwareVersionTest, Sdk)
{
    std::ofstream file("/version.txt");
    file << "SDK_VERSION=17.3";
    file.close();

    string sdk;
    EXPECT_EQ(Core::ERROR_NONE, interface->Sdk(sdk));
    EXPECT_EQ(sdk, _T("17.3"));
}

TEST_F(FirmwareVersionTest, Mediarite)
{
    std::ofstream file("/version.txt");
    file << "MEDIARITE=8.3.53";
    file.close();

    string mediarite;
    EXPECT_EQ(Core::ERROR_NONE, interface->Mediarite(mediarite));
    EXPECT_EQ(mediarite, _T("8.3.53"));
}

TEST_F(FirmwareVersionTest, Yocto)
{
    std::ofstream file("/version.txt");
    file << "YOCTO_VERSION=dunfell";
    file.close();

    string yocto;
    EXPECT_EQ(Core::ERROR_NONE, interface->Yocto(yocto));
    EXPECT_EQ(yocto, _T("dunfell"));
}
TEST_F(FirmwareVersionTest, PdriSuccessWithVersion)
{
    FILE* tmpFile = tmpfile();
    fputs("SKXI11ADS_PDRI_PROD_20221208113233_13.0.0.0.bin\n", tmpFile);
    fseek(tmpFile, 0, SEEK_SET);

    EXPECT_CALL(*p_wrapsImplMock, v_secure_popen(::testing::StrEq("r"), ::testing::StrEq("/usr/bin/mfr_util --PDRIVersion"), ::testing::_))
        .WillOnce(::testing::Return(tmpFile));
    
    EXPECT_CALL(*p_wrapsImplMock, v_secure_pclose(tmpFile))
        .WillOnce(::testing::Return(0));
    
    string pdri;
    EXPECT_EQ(Core::ERROR_NONE, interface->Pdri(pdri));
    EXPECT_EQ(pdri, _T("SKXI11ADS_PDRI_PROD_20221208113233_13.0.0.0.bin"));
}

TEST_F(FirmwareVersionTest, PdriOutputContainsFailed)
{
    FILE* tmpFile = tmpfile();
    fputs("IARM_Bus_Call provider returned error (3) for the method mfrGetManufacturerData Call failed for mfrSERIALIZED_TYPE_PDRIVERSION: error code:3\n", tmpFile);
    fseek(tmpFile, 0, SEEK_SET);

    EXPECT_CALL(*p_wrapsImplMock, v_secure_popen(::testing::StrEq("r"), ::testing::StrEq("/usr/bin/mfr_util --PDRIVersion"), ::testing::_))
        .WillOnce(::testing::Return(tmpFile));
    
    EXPECT_CALL(*p_wrapsImplMock, v_secure_pclose(tmpFile))
        .WillOnce(::testing::Return(0));
    
    string pdri;
    EXPECT_EQ(Core::ERROR_NONE, interface->Pdri(pdri));
    EXPECT_EQ(pdri, _T(""));
}

TEST_F(FirmwareVersionTest, PdriPopenFails)
{
    EXPECT_CALL(*p_wrapsImplMock, v_secure_popen(::testing::StrEq("r"), ::testing::StrEq("/usr/bin/mfr_util --PDRIVersion"), ::testing::_))
        .WillOnce(::testing::Return(nullptr));

    string pdri;
    EXPECT_EQ(Core::ERROR_GENERAL, interface->Pdri(pdri));
}
