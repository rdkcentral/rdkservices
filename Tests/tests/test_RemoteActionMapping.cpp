/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
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
 **/

#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "FactoriesImplementation.h"
#include "RemoteActionMapping.h"

#include "IarmBusMock.h"
#include "ServiceMock.h"

using namespace WPEFramework;
using ::testing::NiceMock;

namespace
{
    const string getKeymapParams        = "{\"deviceID\":1,\"keymapType\":1}";
    const string setKeyActionMapParams  = "{\"deviceID\":1,\"keymapType\":1,"
                                          "\"keyActionMapping\":[{\"keyName\":80,\"rfKeyCode\":109,"
                                          "\"tvIRKeyCode\":[4,19,4,0,34,0,210,0,144,0,4,45,101,4,101,"
                                          "4,144,0,166,1,144,0,141,0,18,34,51,51,50,34,51,51,50,51,"
                                          "34,51,35,34,51,34,48],\"avrIRKeyCode\":[]},{\"keyName\":81,"
                                          "\"rfKeyCode\":108,\"tvIRKeyCode\":[4,19,4,0,34,0,210,0,144,"
                                          "0,4,45,101,4,101,4,144,0,166,1,144,0,141,0,18,34,51,51,50,"
                                          "34,51,51,51,51,34,51,34,34,51,34,48],\"avrIRKeyCode\":[]},"
                                          "{\"keyName\":128,\"rfKeyCode\":107,\"tvIRKeyCode\":[4,19,4,"
                                          "0,34,0,210,0,144,0,4,45,101,4,101,4,144,0,166,1,144,0,141,"
                                          "0,18,34,51,51,50,34,51,51,51,35,51,51,50,50,34,34,32],"
                                          "\"avrIRKeyCode\":[]},{\"keyName\":138,\"rfKeyCode\":65,"
                                          "\"tvIRKeyCode\":[4,17,4,0,34,0,210,0,144,0,4,45,101,4,101,"
                                          "4,144,0,166,1,144,0,141,0,18,34,51,51,50,34,51,51,50,34,"
                                          "51,51,51,51,34,34,32],\"avrIRKeyCode\":[]},{\"keyName\":139,"
                                          "\"rfKeyCode\":66,\"tvIRKeyCode\":[4,17,4,0,34,0,210,0,144,"
                                          "0,4,45,101,4,101,4,144,0,166,1,144,0,141,0,18,34,51,51,50,"
                                          "34,51,51,50,35,35,51,51,50,50,34,32],\"avrIRKeyCode\":[]},"
                                          "{\"keyName\":140,\"rfKeyCode\":67,\"tvIRKeyCode\":[4,17,4,"
                                          "0,34,0,210,0,144,0,4,45,101,4,101,4,144,0,166,1,144,0,141,"
                                          "0,18,34,51,51,50,34,51,51,50,34,35,51,51,51,50,34,32],"
                                          "\"avrIRKeyCode\":[]},{\"keyName\":208,\"rfKeyCode\":52,"
                                          "\"tvIRKeyCode\":[4,17,4,0,34,0,210,0,144,0,4,45,101,4,"
                                          "101,4,144,0,166,1,144,0,141,0,18,34,51,51,50,34,51,51,"
                                          "50,51,51,51,51,34,34,34,32],\"avrIRKeyCode\":[]}]}";
    const string clearKeyActionParams   = "{\"deviceID\":1,\"keymapType\":0,\"keyNames\":"
                                          "[0x80,0x51,0x50,0x8A,0x8B,0x8C,0xD0]}";
    const string lastUsedDeviceResponse = "{\"deviceID\":1,\"remoteType\":\"XR15-2\","
                                          "\"fiveDigitCodePresent\":false,"
                                          "\"setFiveDigitCodeSupported\":true,"
                                          "\"status_code\":0,\"success\":true}";
    const string getFullKeyActionParams = "{\"deviceID\":1,\"keymapType\":1}";
    const string getSingleActionParams  = "{\"deviceID\":1,\"keymapType\":1,\"keyName\":140}";
    const string cancelDownloadParams   = "{\"deviceID\":1}";
    const string setFiveDigitCodeParams = "{\"deviceID\":1,\"tvFiveDigitCode\":0,\"avrFiveDigitCode\":0}";
    const string getKeymapResponse      = "{\"keyNames\":[80,81,128,138,139,140,208],"
                                          "\"status_code\":0,\"success\":true}";
    const string onIRCodeLoadParams     = "{\"deviceID\":1,\"keyNames\":[208,138,139,140,128,81,80],"
                                          "\"rfKeyCodes\":[52,65,66,67,107,108,109],\"loadStatus\":0}";
    const string onFiveCodeLoadParams   = "{\"deviceID\":1,\"tvLoadStatus\":0,\"avrLoadStatus\":0}";
    const string getFullKeyMapResponse  = "{\"keyMappings\":"
                                          "[{\"keyName\":80,\"rfKeyCode\":109,\"tvIRKeyCode\":\"[]\",\"avrIRKeyCode\":\"[]\"},"
                                          "{\"keyName\":81,\"rfKeyCode\":108,\"tvIRKeyCode\":\"[]\",\"avrIRKeyCode\":\"[]\"},"
                                          "{\"keyName\":128,\"rfKeyCode\":107,\"tvIRKeyCode\":\"[]\",\"avrIRKeyCode\":\"[]\"},"
                                          "{\"keyName\":138,\"rfKeyCode\":65,\"tvIRKeyCode\":\"[]\",\"avrIRKeyCode\":\"[]\"},"
                                          "{\"keyName\":139,\"rfKeyCode\":66,\"tvIRKeyCode\":\"[]\",\"avrIRKeyCode\":\"[]\"},"
                                          "{\"keyName\":140,\"rfKeyCode\":67,\"tvIRKeyCode\":\"[]\",\"avrIRKeyCode\":\"[]\"},"
                                          "{\"keyName\":208,\"rfKeyCode\":52,\"tvIRKeyCode\":\"[]\",\"avrIRKeyCode\":\"[]\"}],"
                                          "\"status_code\":0,\"success\":true}";
    const string getSingleKeyResponse   = "{\"keyMapping\":{\"keyName\":140,\"rfKeyCode\":67,"
                                          "\"tvIRKeyCode\":\"[]\",\"avrIRKeyCode\":\"[]\"},"
                                          "\"status_code\":0,\"success\":true}";
    const string successResponse        = "{\"status_code\":0,\"success\":true}";
}

class RemoteActionMappingTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::RemoteActionMapping> plugin_;
    Core::JSONRPC::Handler&                      handler_;
    Core::JSONRPC::Connection                    connection_;
    string                                       response_;
    NiceMock<IarmBusImplMock>                    iarmBusImplMock_;

    RemoteActionMappingTest()
        : plugin_(Core::ProxyType<Plugin::RemoteActionMapping>::Create())
        , handler_(*plugin_)
        , connection_(1, 0)
    {
        IarmBus::getInstance().impl = &iarmBusImplMock_;
    }

    virtual ~RemoteActionMappingTest() override
    {
        IarmBus::getInstance().impl = nullptr;
    }
};

class RemoteActionMappingInitializedEventTest : public RemoteActionMappingTest {
protected:
    IARM_EventHandler_t               ramEventHandler_;
    NiceMock<ServiceMock>             service_;
    NiceMock<FactoriesImplementation> factoriesImplementation_;
    Core::JSONRPC::Message            message_;
    PluginHost::IDispatcher*          dispatcher_;

    RemoteActionMappingInitializedEventTest() :
        RemoteActionMappingTest()
    {
        ON_CALL(iarmBusImplMock_, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                    EXPECT_EQ(eventId, CTRLM_RCU_IARM_EVENT_RIB_ACCESS_CONTROLLER);
                    ramEventHandler_ = handler;
                    return IARM_RESULT_SUCCESS;
                }));

        EXPECT_EQ(string(""), plugin_->Initialize(nullptr));
        PluginHost::IFactories::Assign(&factoriesImplementation_);
        dispatcher_ = static_cast<PluginHost::IDispatcher*>(plugin_->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher_->Activate(&service_);
    }

    virtual ~RemoteActionMappingInitializedEventTest() override
    {
        plugin_->Deinitialize(nullptr);
        dispatcher_->Deactivate();
        dispatcher_->Release();
        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(RemoteActionMappingTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getLastUsedDeviceID")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getKeymap")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("setKeyActionMapping")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("clearKeyActionMapping")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getFullKeyActionMapping")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getSingleKeyActionMapping")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("cancelCodeDownload")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("setFiveDigitCode")));
}

::testing::AssertionResult mockCtrlmData(const char* methodName, void* arg)
{
    if (!string(methodName).compare(CTRLM_MAIN_IARM_CALL_STATUS_GET))
    {
        auto param = static_cast<ctrlm_main_iarm_call_status_t*>(arg);
        memset(param, 0, sizeof(ctrlm_main_iarm_call_status_t));

        param->network_qty      = CTRLM_MAIN_MAX_NETWORKS;
        param->networks[0].id   = 1;
        param->networks[0].type = CTRLM_NETWORK_TYPE_RF4CE;

        return testing::AssertionSuccess();
    }
    else if (!string(methodName).compare(CTRLM_MAIN_IARM_CALL_NETWORK_STATUS_GET))
    {
        auto param = static_cast<ctrlm_main_iarm_call_network_status_t*>(arg);
        memset(param, 0, sizeof(ctrlm_main_iarm_call_network_status_t));

        sprintf(param->status.rf4ce.version_hal,  "%s", "GPv2.6.3.514598");
        sprintf(param->status.rf4ce.chipset,      "%s", "GP502KXBG");

        param->status.rf4ce.ieee_address             = (uint64_t) 0x00155F00205E1789;
        param->status.rf4ce.controller_qty           = 1;
        param->status.rf4ce.pan_id                   = 25684;
        param->status.rf4ce.rf_channel_active.number = 25;
        param->status.rf4ce.short_address            = 64253;
        param->status.rf4ce.controllers[0]           = 1;

        return testing::AssertionSuccess();
    }
    else if (!string(methodName).compare(CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS))
    {
        auto param = static_cast<ctrlm_rcu_iarm_call_controller_status_t*>(arg);
        memset(param, 0, sizeof(ctrlm_rcu_iarm_call_controller_status_t));

        sprintf(param->status.type,                   "%s", "XR15-2");
        sprintf(param->status.ir_db_code_tv,          "%s", "12371");
        sprintf(param->status.ir_db_code_avr,         "%s", "31360");
        sprintf(param->status.manufacturer,           "%s", "RS");
        sprintf(param->status.chipset,                "%s", "QORVO");
        sprintf(param->status.version_software,       "%s", "2.0.1.2");
        sprintf(param->status.version_dsp,            "%s", "0.0");
        sprintf(param->status.version_keyword_model,  "%s", "0.0");
        sprintf(param->status.version_arm,            "%s", "0.0");
        sprintf(param->status.version_hardware,       "%s", "2.3.1.0");
        sprintf(param->status.version_irdb,           "%s", "4.3.2.0");
        sprintf(param->status.version_build_id,       "%s", "0.0");
        sprintf(param->status.version_dsp_build_id,   "%s", "...");
        sprintf(param->status.version_bootloader,     "%s", "0.0");
        sprintf(param->status.version_golden,         "%s", "0.0");
        sprintf(param->status.version_audio_data,     "%s", "0.0");
        sprintf(param->status.irdb_entry_id_name_tv,  "%s", "...");
        sprintf(param->status.irdb_entry_id_name_avr, "%s", "...");

        param->status.ieee_address                                         = (uint64_t) 0x00155F011C7F7359;
        param->status.short_address                                        = 0;
        param->status.time_binding                                         = 1538782229;
        param->status.binding_type                                         = CTRLM_RCU_BINDING_TYPE_SCREEN_BIND;
        param->status.validation_type                                      = CTRLM_RCU_VALIDATION_TYPE_SCREEN_BIND;
        param->status.security_type                                        = CTRLM_RCU_BINDING_SECURITY_TYPE_NORMAL;
        param->status.command_count                                        = 0;
        param->status.last_key_code                                        = CTRLM_KEY_CODE_OK;
        param->status.last_key_status                                      = CTRLM_KEY_STATUS_UP;
        param->status.link_quality_percent                                 = 0;
        param->status.link_quality                                         = 0;
        param->status.firmware_updated                                     = 1;
        param->status.has_battery                                          = 1;
        param->status.battery_level_percent                                = 60;
        param->status.battery_voltage_loaded                               = 2.61908;
        param->status.battery_voltage_unloaded                             = 0.0;
        param->status.time_last_key                                        = 1580263335;
        param->status.time_battery_update                                  = 1602879639;
        param->status.time_battery_changed                                 = 1641294389;
        param->status.battery_changed_actual_percentage                    = 64;
        param->status.battery_changed_unloaded_voltage                     = 2.729412;
        param->status.time_battery_75_percent                              = 1641294389;
        param->status.battery_75_percent_actual_percentage                 = 64;
        param->status.battery_75_percent_unloaded_voltage                  = 2.729412;
        param->status.time_battery_50_percent                              = 0;
        param->status.battery_50_percent_actual_percentage                 = 0;
        param->status.battery_50_percent_unloaded_voltage                  = 0;
        param->status.time_battery_25_percent                              = 0;
        param->status.battery_25_percent_actual_percentage                 = 0;
        param->status.battery_25_percent_unloaded_voltage                  = 0;
        param->status.time_battery_5_percent                               = 0;
        param->status.battery_5_percent_actual_percentage                  = 0;
        param->status.battery_5_percent_unloaded_voltage                   = 0;
        param->status.time_battery_0_percent                               = 0;
        param->status.battery_0_percent_actual_percentage                  = 0;
        param->status.battery_0_percent_unloaded_voltage                   = 0;
        param->status.battery_event                                        = CTRLM_RCU_BATTERY_EVENT_NONE;
        param->status.time_battery_event                                   = 0;
        param->status.ir_db_type                                           = CTRLM_RCU_IR_DB_TYPE_UEI;
        param->status.ir_db_state                                          = CTRLM_RCU_IR_DB_STATE_NO_CODES;
        param->status.voice_cmd_count_today                                = 0;
        param->status.voice_cmd_count_yesterday                            = 0;
        param->status.voice_cmd_short_today                                = 0;
        param->status.voice_cmd_short_yesterday                            = 0;
        param->status.voice_packets_sent_today                             = 0;
        param->status.voice_packets_sent_yesterday                         = 0;
        param->status.voice_packets_lost_today                             = 0;
        param->status.voice_packets_lost_yesterday                         = 0;
        param->status.voice_packet_loss_average_today                      = 0;
        param->status.voice_packet_loss_average_yesterday                  = 0;
        param->status.utterances_exceeding_packet_loss_threshold_today     = 0;
        param->status.utterances_exceeding_packet_loss_threshold_yesterday = 0;
        param->status.checkin_for_device_update                            = 1;
        param->status.ir_db_code_download_supported                        = 1;
        param->status.has_dsp                                              = 0;
        param->status.average_time_in_privacy_mode                         = 0;
        param->status.in_privacy_mode                                      = 0;
        param->status.average_snr                                          = 0;
        param->status.average_keyword_confidence                           = 0;
        param->status.total_number_of_mics_working                         = 0;
        param->status.total_number_of_speakers_working                     = 0;
        param->status.end_of_speech_initial_timeout_count                  = 0;
        param->status.end_of_speech_timeout_count                          = 0;
        param->status.time_uptime_start                                    = 1641460244;
        param->status.uptime_seconds                                       = 0;
        param->status.privacy_time_seconds                                 = 0;
        param->status.reboot_reason                                        = 0;
        param->status.reboot_voltage                                       = 0;
        param->status.reboot_assert_number                                 = 0;
        param->status.reboot_timestamp                                     = 12342;
        param->status.time_last_heartbeat                                  = 0;
        param->status.battery_voltage_large_jump_counter                   = 0;
        param->status.battery_voltage_large_decline_detected               = 0;

        return testing::AssertionSuccess();
    }
    else if (!string(methodName).compare(CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET))
    {
        auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
        param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
        return testing::AssertionSuccess();
    }
    else if (!string(methodName).compare(CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET))
    {
        auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
        param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
        return testing::AssertionSuccess();
    }
    return testing::AssertionFailure() << "Unknown method" << methodName;;
}

TEST_F(RemoteActionMappingTest, getLastUsedDeviceID)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_TRUE(mockCtrlmData(methodName, arg));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getLastUsedDeviceID"), _T("{}"), response_));
    EXPECT_EQ(response_, lastUsedDeviceResponse);
}

TEST_F(RemoteActionMappingTest, getKeymap)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getKeymap"), getKeymapParams, response_));
    EXPECT_EQ(response_, getKeymapResponse);
}

TEST_F(RemoteActionMappingTest, setKeyActionMapping)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_TRUE(mockCtrlmData(methodName, arg));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("setKeyActionMapping"), setKeyActionMapParams, response_));
    EXPECT_EQ(response_, successResponse);
}

TEST_F(RemoteActionMappingTest, clearKeyActionMapping)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_TRUE(mockCtrlmData(methodName, arg));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("clearKeyActionMapping"), clearKeyActionParams, response_));
    EXPECT_EQ(response_, successResponse);
}

TEST_F(RemoteActionMappingTest, getFullKeyActionMapping)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_TRUE(mockCtrlmData(methodName, arg));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getFullKeyActionMapping"), getFullKeyActionParams, response_));
    EXPECT_EQ(response_, getFullKeyMapResponse);
}

TEST_F(RemoteActionMappingTest, getSingleKeyActionMapping)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_TRUE(mockCtrlmData(methodName, arg));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getSingleKeyActionMapping"), getSingleActionParams, response_));
    EXPECT_EQ(response_, getSingleKeyResponse);
}

TEST_F(RemoteActionMappingTest, cancelCodeDownload)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_TRUE(mockCtrlmData(methodName, arg));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("cancelCodeDownload"), cancelDownloadParams, response_));
    EXPECT_EQ(response_, successResponse);
}

TEST_F(RemoteActionMappingTest, setFiveDigitCode)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_TRUE(mockCtrlmData(methodName, arg));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("setFiveDigitCode"), setFiveDigitCodeParams, response_));
    EXPECT_EQ(response_, successResponse);
}

TEST_F(RemoteActionMappingInitializedEventTest, onIRCodeLoad)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_TRUE(mockCtrlmData(methodName, arg));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_CALL(service_, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
	  								      "\"method\":\"org.rdk.RemoteActionMapping.onIRCodeLoad\","
                                          "\"params\":")) + onIRCodeLoadParams + "}");

                return Core::ERROR_NONE;
            }));

    auto key_slots = {MSO_RFKEY_INPUT_SELECT,
                      MSO_RFKEY_VOL_PLUS,
                      MSO_RFKEY_VOL_MINUS,
                      MSO_RFKEY_MUTE,
                      MSO_RFKEY_PWR_TOGGLE,
                      MSO_RFKEY_PWR_ON,
                      MSO_RFKEY_PWR_OFF};
    ctrlm_rcu_iarm_event_rib_entry_access_t eventData = {0};

    eventData.api_revision  = CTRLM_RCU_IARM_BUS_API_REVISION;
    eventData.network_id    = 1;
    eventData.network_type  = CTRLM_NETWORK_TYPE_RF4CE;
    eventData.controller_id = 1;
    eventData.identifier    = CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE_STATUS;
    eventData.index         = 0;
    eventData.access_type   = CTRLM_ACCESS_TYPE_READ;

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("clearKeyActionMapping"), clearKeyActionParams, response_));

    handler_.Subscribe(0, _T("onIRCodeLoad"), _T("org.rdk.RemoteActionMapping"), message_);
    ramEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_RIB_ACCESS_CONTROLLER, &eventData, sizeof(ctrlm_rcu_iarm_event_rib_entry_access_t));

    eventData.identifier = CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE;
    for (auto key : key_slots) {
        eventData.index = key;
        ramEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_RIB_ACCESS_CONTROLLER, &eventData, sizeof(ctrlm_rcu_iarm_event_rib_entry_access_t));
    }
    handler_.Unsubscribe(0, _T("onIRCodeLoad"), _T("org.rdk.RemoteActionMapping"), message_);
}

TEST_F(RemoteActionMappingInitializedEventTest, onFiveDigitCodeLoad)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(CTRLM_MAIN_IARM_BUS_NAME));
                EXPECT_TRUE(mockCtrlmData(methodName, arg));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_CALL(service_, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
										  "\"method\":\"org.rdk.RemoteActionMapping.onFiveDigitCodeLoad\","
                                          "\"params\":")) + onFiveCodeLoadParams + "}");
                return Core::ERROR_NONE;
            }));

    ctrlm_rcu_iarm_event_rib_entry_access_t eventData = {0};
    eventData.api_revision  = CTRLM_RCU_IARM_BUS_API_REVISION;
    eventData.network_id    = 1;
    eventData.network_type  = CTRLM_NETWORK_TYPE_RF4CE;
    eventData.controller_id = 1;
    eventData.identifier    = CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE_STATUS;
    eventData.index         = 0;
    eventData.access_type   = CTRLM_ACCESS_TYPE_READ;

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("setFiveDigitCode"), setFiveDigitCodeParams, response_));

    handler_.Subscribe(0, _T("onFiveDigitCodeLoad"), _T("org.rdk.RemoteActionMapping"), message_);
    ramEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_RIB_ACCESS_CONTROLLER, &eventData, sizeof(ctrlm_rcu_iarm_event_rib_entry_access_t));

    eventData.identifier  = CTRLM_RCU_RIB_ATTR_ID_CONTROLLER_IRDB_STATUS;
    eventData.access_type = CTRLM_ACCESS_TYPE_WRITE;
    ramEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_RIB_ACCESS_CONTROLLER, &eventData, sizeof(ctrlm_rcu_iarm_event_rib_entry_access_t));

    handler_.Unsubscribe(0, _T("onFiveDigitCodeLoad"), _T("org.rdk.RemoteActionMapping"), message_);
}
