/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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
 */

#include <gtest/gtest.h>
#include "SystemServices.h"
#include "RfcApiMock.h"
using namespace WPEFramework;
using namespace std;

namespace {
const string iarmName = _T("Thunder_Plugins");
}

class SystemServicesTest : public::testing::Test
{
    protected:
    Core::ProxyType<Plugin::SystemServices> systemplugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Handler& handlerV2;
    Core::JSONRPC::Connection connection;
    RfcApiImplMock rfcApiImplMock;
    string response;
    
private:
    /* data */
public:
    SystemServicesTest()
    :systemplugin(Core::ProxyType<Plugin::SystemServices>::Create())
    ,handler(*systemplugin)
    ,handlerV2(*(systemplugin->GetHandler(2)))
    ,connection(1,0)
    {
	 RfcApi::getInstance().impl = &rfcApiImplMock;
    }
   
    virtual void SetUp()
    {
	}

    virtual void TearDown()
    {
    }

    ~SystemServicesTest()
    {
	RfcApi::getInstance().impl = nullptr;
    }
};


TEST_F(SystemServicesTest, RegisteredMethods)
{
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("requestSystemUptime")));
	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("fireFirmwarePendingReboot")));
	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("setFirmwareAutoReboot")));
	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("setFirmwareRebootDelay")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("updateFirmware")));
	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getLastFirmwareFailureReason")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDownloadedFirmwareInfo")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFirmwareDownloadPercent")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFirmwareUpdateState")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setTimeZoneDST")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTimeZoneDST")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setTerritory")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTerritory")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("hasRebootBeenRequested")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreviousRebootInfo")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreviousRebootInfo2")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreviousRebootReason")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMilestones")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isOptOutTelemetry")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setOptOutTelemetry")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSystemVersions")));
}

TEST_F(SystemServicesTest, SystemUptime)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("requestSystemUptime"), _T("{}"), response));

    size_t pos1 = response.find(",");

    string result = response.substr(pos1+1, response.length()-pos1-2);
    EXPECT_EQ(result,string("\"success\":true"));
}
    

TEST_F(SystemServicesTest, PendingReboot)
{
	EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(strcmp(pcCallerID, "thunderapi"), 0);
                EXPECT_EQ(strcmp(pcParameterName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.fwDelayReboot"), 0);
		return WDMP_SUCCESS;
	}));
	
	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("fireFirmwarePendingReboot"), _T("{}"),response));
	EXPECT_EQ(response,string("{\"success\":true}"));
}


TEST_F(SystemServicesTest, AutoReboot)
{
        EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(strcmp(pcCallerID, "thunderapi"), 0);
                EXPECT_EQ(strcmp(pcParameterName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.Enable"), 0);
                return WDMP_SUCCESS;
        }));

        EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("setFirmwareAutoReboot"), _T("{\"enable\":true}"),response));
        EXPECT_EQ(response,string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, RebootDelay)
{
        EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(strcmp(pcCallerID, "thunderapi"), 0);
                EXPECT_EQ(strcmp(pcParameterName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.fwDelayReboot"), 0);
                return WDMP_SUCCESS;
        }));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("setFirmwareRebootDelay"), _T("{\"delaySeconds\":10}"),response));
    EXPECT_EQ(response,string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, Firmware)
{
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updateFirmware"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"success\":true}"));
	
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("getLastFirmwareFailureReason"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"failReason\":\"None\",\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDownloadedFirmwareInfo"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"currentFWVersion\":\"AX013AN_5.6p4s1_VBN_sey\",\"downloadedFWVersion\":\"\",\"downloadedFWLocation\":\"\",\"isRebootDeferred\":false,\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareDownloadPercent"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"downloadPercent\":-1,\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateState"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"firmwareUpdateState\":0,\"success\":true}"));
}

TEST_F(SystemServicesTest, Timezone)
{

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTimeZoneDST"), _T("{\"timeZone\":\"America/New_York\"}"),response));
	EXPECT_EQ(response,string("{\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimeZoneDST"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"timeZone\":\"America\\/New_York\",\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTimeZoneDST"), _T("{\"timeZone\":\"America/Costa_Rica\"}"),response));
    EXPECT_EQ(response,string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimeZoneDST"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"timeZone\":\"America\\/Costa_Rica\",\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("getTimeZones"), _T("{}"),response));

	size_t pos = response.length() - 15 ;
    string timeZones = response.substr(pos,response.length());

    EXPECT_EQ(timeZones,string("\"success\":true}"));
}


TEST_F(SystemServicesTest, InvalidTerritory)
{
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"US\",\"region\":\"US-NYC\"}"),response));
    EXPECT_EQ(response,string("{\"error\":{\"message\":\"Invalid territory\"},\"success\":false}"));

	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"USA\",\"region\":\"U-NYC\"}"),response));
	EXPECT_EQ(response,string("{\"error\":{\"message\":\"Invalid region\"},\"success\":false}"));

	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"US@\",\"region\":\"US-NYC\"}"),response));
    EXPECT_EQ(response,string("{\"error\":{\"message\":\"Invalid territory\"},\"success\":false}"));

	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"USA\",\"region\":\"US-N$C\"}"),response));
    EXPECT_EQ(response,string("{\"error\":{\"message\":\"Invalid region\"},\"success\":false}"));

	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"US12\",\"region\":\"US-NYC\"}"),response));
    EXPECT_EQ(response,string("{\"error\":{\"message\":\"Invalid territory\"},\"success\":false}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTerritory"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"territory\":\"\",\"region\":\"\",\"success\":true}"));

}

TEST_F(SystemServicesTest, ValidTerritory)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"USA\",\"region\":\"US-NYC\"}"),response));
    EXPECT_EQ(response,string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTerritory"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"territory\":\"USA\",\"region\":\"US-NYC\",\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"GBR\",\"region\":\"GB-EGL\"}"),response));
    EXPECT_EQ(response,string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTerritory"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"territory\":\"GBR\",\"region\":\"GB-EGL\",\"success\":true}"));

}


TEST_F(SystemServicesTest, reboot)
{
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("hasRebootBeenRequested"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"rebootRequested\":false,\"success\":true}"));
	
}

TEST_F(SystemServicesTest, rebootReason)
{
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreviousRebootInfo"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"timeStamp\":\"18.08.2022_09:51.38\",\"reason\":\"Triggered from SystemServices! MAINTENANCE_REBOOT\",\"source\":\"Unknown\",\"customReason\":\"MAINTENANCE_REBOOT\",\"otherReason\":\"MAINTENANCE_REBOOT\",\"success\":true}"));
	
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreviousRebootInfo2"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"rebootInfo\":{\"timestamp\":\"Thu Aug 18 13:51:39 UTC 2022\",\"source\":\"HAL_CDL_notify_mgr_event\",\"reason\":\"OPS_TRIGGERED\",\"customReason\":\"Unknown\",\"lastHardPowerReset\":\"Thu Aug 18 13:51:39 UTC 2022\"},\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreviousRebootReason"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"reason\":\"OPS_TRIGGERED\",\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMilestones"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"milestones\":[\"RDK_STARTED:35755\"],\"deprecated\":true,\"success\":true}"));


}

TEST_F(SystemServicesTest, Telemetry)
{

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isOptOutTelemetry"), _T("{}"),response));
	EXPECT_EQ(response,string("{\"Opt-Out\":false,\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setOptOutTelemetry"), _T("{\"Opt-Out\":true}"),response));
    EXPECT_EQ(response,string("{\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isOptOutTelemetry"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"Opt-Out\":true,\"success\":true}"));

}

TEST_F(SystemServicesTest, SystemVersions)
{
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSystemVersions"), _T("{}"),response));
    EXPECT_EQ(response,string("{\"stbVersion\":\"AX013AN_5.6p4s1_VBN_sey\",\"receiverVersion\":\"5.6.0.4\",\"stbTimestamp\":\"Fri 05 Aug 2022 16:14:54 AP UTC\",\"success\":true}"));
}
