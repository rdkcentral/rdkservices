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

#include "gtest/gtest.h"
#include "FactoriesImplementation.h"
#include "ControlService.h"

#include "IarmBusMock.h"
#include "ServiceMock.h"

using namespace WPEFramework;
using ::testing::NiceMock;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Test;
using ::testing::StrEq;
using ::testing::Gt;
using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;

class ControlServiceTest : public Test {
protected:
    Core::ProxyType<Plugin::ControlService> plugin_;
    Core::JSONRPC::Handler&                 handler_;
    Core::JSONRPC::Connection               connection_;
    string                                  response_;
    NiceMock<IarmBusImplMock>               iarmBusImplMock_;

    ControlServiceTest()
        : plugin_(Core::ProxyType<Plugin::ControlService>::Create())
        , handler_(*plugin_)
        , connection_(1, 0)
    {
        IarmBus::getInstance().impl = &iarmBusImplMock_;
    }

    virtual ~ControlServiceTest() override
    {
        IarmBus::getInstance().impl = nullptr;
    }
};

static AssertionResult isValidCtrlmRcuIarmEvent(IARM_EventId_t ctrlmRcuIarmEventId)
{
    switch (ctrlmRcuIarmEventId) {
        case CTRLM_RCU_IARM_EVENT_KEY_GHOST:
        case CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN:
        case CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_PRESS:
        case CTRLM_RCU_IARM_EVENT_VALIDATION_END:
        case CTRLM_RCU_IARM_EVENT_CONFIGURATION_COMPLETE:
        case CTRLM_RCU_IARM_EVENT_BATTERY_MILESTONE:
        case CTRLM_RCU_IARM_EVENT_REMOTE_REBOOT:
        case CTRLM_RCU_IARM_EVENT_RCU_REVERSE_CMD_END:
        case CTRLM_RCU_IARM_EVENT_CONTROL:
            return AssertionSuccess();
        default:
            return AssertionFailure();
    }
}

class ControlServiceInitializedEventTest : public ControlServiceTest {
protected:
    IARM_EventHandler_t               controlEventHandler_;
    NiceMock<ServiceMock>             service_;
    NiceMock<FactoriesImplementation> factoriesImplementation_;
    Core::JSONRPC::Message            message_;
    PluginHost::IDispatcher*          dispatcher_;

    ControlServiceInitializedEventTest() :
        ControlServiceTest()
    {
        EXPECT_CALL(iarmBusImplMock_, IARM_Bus_RegisterEventHandler(StrEq(IARM_BUS_IRMGR_NAME),IARM_BUS_IRMGR_EVENT_IRKEY, _))
            .WillOnce(Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    controlEventHandler_ = handler;
                    return IARM_RESULT_SUCCESS;
                }));
        EXPECT_CALL(iarmBusImplMock_, IARM_Bus_RegisterEventHandler(StrEq(CTRLM_MAIN_IARM_BUS_NAME), _, _))
            .WillRepeatedly(Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    EXPECT_TRUE(isValidCtrlmRcuIarmEvent(eventId));
                    controlEventHandler_ = handler;
                    return IARM_RESULT_SUCCESS;
                }));

        EXPECT_EQ(string(""), plugin_->Initialize(nullptr));
        PluginHost::IFactories::Assign(&factoriesImplementation_);
        dispatcher_ = static_cast<PluginHost::IDispatcher*>(plugin_->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher_->Activate(&service_);
    }

    virtual ~ControlServiceInitializedEventTest() override
    {
        plugin_->Deinitialize(nullptr);
        dispatcher_->Deactivate();
        dispatcher_->Release();
        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(ControlServiceTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getQuirks")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getAllRemoteData")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getSingleRemoteData")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getLastKeypressSource")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getLastPairedRemoteData")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("setValues")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getValues")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("startPairingMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("endPairingMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("canFindMyRemote")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("findLastUsedRemote")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("checkRf4ceChipConnectivity")));
}

TEST_F(ControlServiceTest, getQuirks)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getQuirks"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"quirks\":[\"DELIA-43686\",\"RDK-28767\",\"RDK-31263\",\"RDK-32347\"],\"success\":true}");
}

TEST_F(ControlServiceTest, getAllRemoteData)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_STATUS_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_status_t));

                param->network_qty      = CTRLM_MAIN_MAX_NETWORKS;
                param->networks[0].id   = 1;
                param->networks[0].type = CTRLM_NETWORK_TYPE_RF4CE;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_NETWORK_STATUS_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_network_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_network_status_t));

                snprintf(param->status.rf4ce.version_hal, sizeof(param->status.rf4ce.version_hal), "%s", "GPv2.6.3.514598");
                snprintf(param->status.rf4ce.chipset, sizeof(param->status.rf4ce.chipset), "%s", "GP502KXBG");

                param->status.rf4ce.ieee_address             = (uint64_t) 0x00155F00205E1789;
                param->status.rf4ce.controller_qty           = 1;
                param->status.rf4ce.pan_id                   = 25684;
                param->status.rf4ce.rf_channel_active.number = 25;
                param->status.rf4ce.short_address            = 64253;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_PAIRING_METRICS_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_pairing_metrics_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_pairing_metrics_t));

                snprintf(param->last_non_screenbind_remote_type, sizeof(param->last_non_screenbind_remote_type), "%s", "...");
                snprintf(param->last_screenbind_remote_type, sizeof(param->last_screenbind_remote_type), "%s", "...");

                param->num_screenbind_failures                = 1;
                param->last_screenbind_error_timestamp        = 1589356931;
                param->last_screenbind_error_code             = CTRLM_BIND_STATUS_NO_DISCOVERY_REQUEST;
                param->num_non_screenbind_failures            = 3;
                param->last_non_screenbind_error_timestamp    = 1589359161;
                param->last_non_screenbind_error_code         = CTRLM_BIND_STATUS_NO_PAIRING_REQUEST;
                param->last_non_screenbind_error_binding_type = 2;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_IR_REMOTE_USAGE_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_ir_remote_usage_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_ir_remote_usage_t));

                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_controller_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_rcu_iarm_call_controller_status_t));

                snprintf(param->status.type, sizeof(param->status.type), "%s", "XR15-2");
                snprintf(param->status.ir_db_code_tv, sizeof(param->status.ir_db_code_tv), "%s", "12371");
                snprintf(param->status.ir_db_code_avr, sizeof(param->status.ir_db_code_avr), "%s", "31360");
                snprintf(param->status.manufacturer, sizeof(param->status.manufacturer), "%s", "RS");
                snprintf(param->status.chipset, sizeof(param->status.chipset), "%s", "QORVO");
                snprintf(param->status.version_software, sizeof(param->status.version_software), "%s", "2.0.1.2");
                snprintf(param->status.version_hardware, sizeof(param->status.version_hardware), "%s", "2.3.1.0");
                snprintf(param->status.version_irdb, sizeof(param->status.version_irdb), "%s", "4.3.2.0");
                snprintf(param->status.irdb_entry_id_name_tv, sizeof(param->status.irdb_entry_id_name_tv), "%s", "...");
                snprintf(param->status.irdb_entry_id_name_avr, sizeof(param->status.irdb_entry_id_name_avr), "%s", "...");

                param->controller_id                                               = 1;
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
                param->status.has_battery                                          = 0;
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
                param->status.ir_db_state                                          = CTRLM_RCU_IR_DB_STATE_TV_AVR_CODES;
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
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getAllRemoteData"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"stbRf4ceMACAddress\":\"0x00155F00205E1789\","
                         "\"stbRf4ceSocMfr\":\"GP502KXBG\","
                         "\"stbHALVersion\":\"GPv2.6.3.514598\","
                         "\"stbRf4ceShortAddress\":64253,"
                         "\"stbPanId\":25684,"
                         "\"stbActiveChannel\":25,"
                         "\"stbNumPairedRemotes\":1,"
                         "\"stbNumScreenBindFailures\":1,"
                         "\"stbLastScreenBindErrorCode\":1,"
                         "\"stbLastScreenBindErrorRemoteType\":\"...\","
                         "\"stbLastScreenBindErrorTimestamp\":1589356931000,"
                         "\"stbNumOtherBindFailures\":3,"
                         "\"stbLastOtherBindErrorCode\":2,"
                         "\"stbLastOtherBindErrorRemoteType\":\"...\","
                         "\"stbLastOtherBindErrorBindType\":2,"
                         "\"stbLastOtherBindErrorTimestamp\":1589359161000,"
                         "\"bHasIrRemotePreviousDay\":false,"
                         "\"bHasIrRemoteCurrentDay\":false,"
                         "\"remoteData\":[{\"remoteId\":1,"
                         "\"remoteMACAddress\":\"0x00155F011C7F7359\","
                         "\"remoteModel\":\"XR15\","
                         "\"remoteModelVersion\":\"v2\","
                         "\"howRemoteIsPaired\":\"screen-bind\","
                         "\"pairingTimestamp\":1538782229000,"
                         "\"batteryLevelLoaded\":\"2.619080\","
                         "\"batteryLevelUnloaded\":\"0.000000\","
                         "\"batteryLevelPercentage\":60,"
                         "\"batteryLastEvent\":0,"
                         "\"batteryLastEventTimestamp\":1602879639000,"
                         "\"numVoiceCommandsPreviousDay\":0,"
                         "\"numVoiceCommandsCurrentDay\":0,"
                         "\"numVoiceShortUtterancesPreviousDay\":0,"
                         "\"numVoiceShortUtterancesCurrentDay\":0,"
                         "\"numVoicePacketsSentPreviousDay\":0,"
                         "\"numVoicePacketsSentCurrentDay\":0,"
                         "\"numVoicePacketsLostPreviousDay\":0,"
                         "\"numVoicePacketsLostCurrentDay\":0,"
                         "\"aveVoicePacketLossPreviousDay\":\"0.000000\","
                         "\"aveVoicePacketLossCurrentDay\":\"0.000000\","
                         "\"numVoiceCmdsHighLossPreviousDay\":0,"
                         "\"numVoiceCmdsHighLossCurrentDay\":0,"
                         "\"lastRebootErrorCode\":0,"
                         "\"lastRebootTimestamp\":12342000,"
                         "\"versionInfoSw\":\"2.0.1.2\","
                         "\"versionInfoHw\":\"2.3.1.0\","
                         "\"versionInfoIrdb\":\"4.3.2.0\","
                         "\"irdbType\":0,\"irdbState\":3,"
                         "\"programmedTvIRCode\":\"12371\","
                         "\"programmedAvrIRCode\":\"31360\","
                         "\"bHasRemoteBeenUpdated\":true,"
                         "\"lastCommandTimeDate\":1580263335000,"
                         "\"rf4ceRemoteSocMfr\":\"QORVO\","
                         "\"remoteMfr\":\"RS\","
                         "\"signalStrengthPercentage\":0,"
                         "\"linkQuality\":0,\"bHasCheckedIn\":true,"
                         "\"bIrdbDownloadSupported\":true,"
                         "\"securityType\":0,\"bHasBattery\":false,"
                         "\"bHasDSP\":false}],"
                         "\"status_code\":0,\"success\":true}");
}

TEST_F(ControlServiceTest, getSingleRemoteData)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_STATUS_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_status_t));

                param->network_qty      = CTRLM_MAIN_MAX_NETWORKS;
                param->networks[0].id   = 1;
                param->networks[0].type = CTRLM_NETWORK_TYPE_RF4CE;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_NETWORK_STATUS_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_network_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_network_status_t));

                param->status.rf4ce.controller_qty = 1;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_controller_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_rcu_iarm_call_controller_status_t));

                snprintf(param->status.type, sizeof(param->status.type), "%s", "XR15-2");
                snprintf(param->status.ir_db_code_tv, sizeof(param->status.ir_db_code_tv), "%s", "12371");
                snprintf(param->status.ir_db_code_avr, sizeof(param->status.ir_db_code_avr), "%s", "31360");
                snprintf(param->status.manufacturer, sizeof(param->status.manufacturer), "%s", "RS");
                snprintf(param->status.chipset, sizeof(param->status.chipset), "%s", "QORVO");
                snprintf(param->status.version_software, sizeof(param->status.version_software), "%s", "2.0.1.2");
                snprintf(param->status.version_hardware, sizeof(param->status.version_hardware), "%s", "2.3.1.0");
                snprintf(param->status.version_irdb, sizeof(param->status.version_irdb), "%s", "4.3.2.0");
                snprintf(param->status.irdb_entry_id_name_tv, sizeof(param->status.irdb_entry_id_name_tv), "%s", "...");
                snprintf(param->status.irdb_entry_id_name_avr, sizeof(param->status.irdb_entry_id_name_avr), "%s", "...");

                param->controller_id                                               = 1;
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
                param->status.has_battery                                          = 0;
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
                param->status.ir_db_state                                          = CTRLM_RCU_IR_DB_STATE_TV_AVR_CODES;
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
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getSingleRemoteData"), "{\"remoteId\":1}", response_));
    EXPECT_EQ(response_, "{\"remoteData\":{\"remoteId\":1,"
                         "\"remoteMACAddress\":\"0x00155F011C7F7359\","
                         "\"remoteModel\":\"XR15\","
                         "\"remoteModelVersion\":\"v2\","
                         "\"howRemoteIsPaired\":\"screen-bind\","
                         "\"pairingTimestamp\":1538782229000,"
                         "\"batteryLevelLoaded\":\"2.619080\","
                         "\"batteryLevelUnloaded\":\"0.000000\","
                         "\"batteryLevelPercentage\":60,"
                         "\"batteryLastEvent\":0,"
                         "\"batteryLastEventTimestamp\":1602879639000,"
                         "\"numVoiceCommandsPreviousDay\":0,"
                         "\"numVoiceCommandsCurrentDay\":0,"
                         "\"numVoiceShortUtterancesPreviousDay\":0,"
                         "\"numVoiceShortUtterancesCurrentDay\":0,"
                         "\"numVoicePacketsSentPreviousDay\":0,"
                         "\"numVoicePacketsSentCurrentDay\":0,"
                         "\"numVoicePacketsLostPreviousDay\":0,"
                         "\"numVoicePacketsLostCurrentDay\":0,"
                         "\"aveVoicePacketLossPreviousDay\":\"0.000000\","
                         "\"aveVoicePacketLossCurrentDay\":\"0.000000\","
                         "\"numVoiceCmdsHighLossPreviousDay\":0,"
                         "\"numVoiceCmdsHighLossCurrentDay\":0,"
                         "\"lastRebootErrorCode\":0,"
                         "\"lastRebootTimestamp\":12342000,"
                         "\"versionInfoSw\":\"2.0.1.2\","
                         "\"versionInfoHw\":\"2.3.1.0\","
                         "\"versionInfoIrdb\":\"4.3.2.0\","
                         "\"irdbType\":0,\"irdbState\":3,"
                         "\"programmedTvIRCode\":\"12371\","
                         "\"programmedAvrIRCode\":\"31360\","
                         "\"bHasRemoteBeenUpdated\":true,"
                         "\"lastCommandTimeDate\":1580263335000,"
                         "\"rf4ceRemoteSocMfr\":\"QORVO\","
                         "\"remoteMfr\":\"RS\","
                         "\"signalStrengthPercentage\":0,"
                         "\"linkQuality\":0,\"bHasCheckedIn\":true,"
                         "\"bIrdbDownloadSupported\":true,"
                         "\"securityType\":0,\"bHasBattery\":false,"
                         "\"bHasDSP\":false},"
                         "\"status_code\":0,\"success\":true}");
}

TEST_F(ControlServiceTest, getLastKeypressSource)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_LAST_KEY_INFO_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_last_key_info_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_last_key_info_t));

                snprintf(param->source_name, sizeof(param->source_name), "%s", "XR15-10");

                param->result               = CTRLM_IARM_CALL_RESULT_SUCCESS;
                param->controller_id        = 1;
                param->source_type          = IARM_BUS_IRMGR_KEYSRC_RF;
                param->source_key_code      = 133;
                param->timestamp            = 1598470622000;
                param->is_screen_bind_mode  = 1;
                param->remote_keypad_config = 1;

                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getLastKeypressSource"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"remoteId\":1,\"timestamp\":1598470622000,"
                         "\"sourceName\":\"XR15-10\",\"sourceType\":\"RF\","
                         "\"sourceKeyCode\":133,\"bIsScreenBindMode\":true,"
                         "\"remoteKeypadConfig\":1,\"status_code\":0,"
                         "\"success\":true}");
}

TEST_F(ControlServiceTest, getLastPairedRemoteData)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME),StrEq(CTRLM_MAIN_IARM_CALL_STATUS_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_status_t));

                param->network_qty      = CTRLM_MAIN_MAX_NETWORKS;
                param->networks[0].id   = 1;
                param->networks[0].type = CTRLM_NETWORK_TYPE_RF4CE;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_NETWORK_STATUS_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_network_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_network_status_t));

                param->status.rf4ce.controller_qty = 1;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_controller_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_rcu_iarm_call_controller_status_t));

                snprintf(param->status.type, sizeof(param->status.type), "%s", "XR15-2");
                snprintf(param->status.ir_db_code_tv, sizeof(param->status.ir_db_code_tv), "%s", "12371");
                snprintf(param->status.ir_db_code_avr, sizeof(param->status.ir_db_code_avr), "%s", "31360");
                snprintf(param->status.manufacturer, sizeof(param->status.manufacturer), "%s", "RS");
                snprintf(param->status.chipset, sizeof(param->status.chipset), "%s", "QORVO");
                snprintf(param->status.version_software, sizeof(param->status.version_software), "%s", "2.0.1.2");
                snprintf(param->status.version_hardware, sizeof(param->status.version_hardware), "%s", "2.3.1.0");
                snprintf(param->status.version_irdb, sizeof(param->status.version_irdb), "%s", "4.3.2.0");
                snprintf(param->status.irdb_entry_id_name_tv, sizeof(param->status.irdb_entry_id_name_tv), "%s", "...");
                snprintf(param->status.irdb_entry_id_name_avr, sizeof(param->status.irdb_entry_id_name_avr), "%s", "...");

                param->controller_id                                               = 1;
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
                param->status.has_battery                                          = 0;
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
                param->status.ir_db_state                                          = CTRLM_RCU_IR_DB_STATE_TV_AVR_CODES;
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
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getLastPairedRemoteData"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"remoteData\":{\"remoteId\":1,"
                         "\"remoteMACAddress\":\"0x00155F011C7F7359\","
                         "\"remoteModel\":\"XR15\","
                         "\"remoteModelVersion\":\"v2\","
                         "\"howRemoteIsPaired\":\"screen-bind\","
                         "\"pairingTimestamp\":1538782229000,"
                         "\"batteryLevelLoaded\":\"2.619080\","
                         "\"batteryLevelUnloaded\":\"0.000000\","
                         "\"batteryLevelPercentage\":60,"
                         "\"batteryLastEvent\":0,"
                         "\"batteryLastEventTimestamp\":1602879639000,"
                         "\"numVoiceCommandsPreviousDay\":0,"
                         "\"numVoiceCommandsCurrentDay\":0,"
                         "\"numVoiceShortUtterancesPreviousDay\":0,"
                         "\"numVoiceShortUtterancesCurrentDay\":0,"
                         "\"numVoicePacketsSentPreviousDay\":0,"
                         "\"numVoicePacketsSentCurrentDay\":0,"
                         "\"numVoicePacketsLostPreviousDay\":0,"
                         "\"numVoicePacketsLostCurrentDay\":0,"
                         "\"aveVoicePacketLossPreviousDay\":\"0.000000\","
                         "\"aveVoicePacketLossCurrentDay\":\"0.000000\","
                         "\"numVoiceCmdsHighLossPreviousDay\":0,"
                         "\"numVoiceCmdsHighLossCurrentDay\":0,"
                         "\"lastRebootErrorCode\":0,"
                         "\"lastRebootTimestamp\":12342000,"
                         "\"versionInfoSw\":\"2.0.1.2\","
                         "\"versionInfoHw\":\"2.3.1.0\","
                         "\"versionInfoIrdb\":\"4.3.2.0\","
                         "\"irdbType\":0,\"irdbState\":3,"
                         "\"programmedTvIRCode\":\"12371\","
                         "\"programmedAvrIRCode\":\"31360\","
                         "\"bHasRemoteBeenUpdated\":true,"
                         "\"lastCommandTimeDate\":1580263335000,"
                         "\"rf4ceRemoteSocMfr\":\"QORVO\","
                         "\"remoteMfr\":\"RS\","
                         "\"signalStrengthPercentage\":0,"
                         "\"linkQuality\":0,\"bHasCheckedIn\":true,"
                         "\"bIrdbDownloadSupported\":true,"
                         "\"securityType\":0,\"bHasBattery\":false,"
                         "\"bHasDSP\":false},"
                         "\"status_code\":0,\"success\":true}");
}

TEST_F(ControlServiceTest, setValues)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_SET_VALUES), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_control_service_settings_t*>(arg);
                EXPECT_FALSE(param->asb_enabled);
                EXPECT_FALSE(param->open_chime_enabled);
                EXPECT_TRUE(param->close_chime_enabled);
                EXPECT_TRUE(param->privacy_chime_enabled);
                EXPECT_EQ(param->conversational_mode, 6);
                EXPECT_EQ(param->chime_volume, 1);
                EXPECT_EQ(param->ir_command_repeats, 3);
                EXPECT_THAT(param->available, Gt(0));
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("setValues"),
                                _T("{\"enableASB\":false,\"enableOpenChime\":false,"
                                "\"enableCloseChime\":true,\"enablePrivacyChime\":true,"
                                "\"conversationalMode\":6,\"chimeVolume\":1,"
                                "\"irCommandRepeats\":3}"),
                                response_));
    EXPECT_EQ(response_, "{\"status_code\":0,\"success\":true}");
}

TEST_F(ControlServiceTest, getValues)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_GET_VALUES), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_control_service_settings_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_control_service_settings_t));

                param->result                = CTRLM_IARM_CALL_RESULT_SUCCESS;
                param->available             = 0;
                param->asb_supported         = 1;
                param->asb_enabled           = 0;
                param->open_chime_enabled    = 0;
                param->close_chime_enabled   = 1;
                param->privacy_chime_enabled = 1;
                param->conversational_mode   = 6;
                param->chime_volume          = CTRLM_CHIME_VOLUME_MEDIUM;
                param->ir_command_repeats    = 3;

                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getValues"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"supportsASB\":true,\"enableASB\":false,"
                         "\"enableOpenChime\":false,\"enableCloseChime\":true,"
                         "\"enablePrivacyChime\":true,\"conversationalMode\":6,"
                         "\"chimeVolume\":1,\"irCommandRepeats\":3,"
                         "\"status_code\":0,\"success\":true}");
}

TEST_F(ControlServiceTest, startPairingMode)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_STATUS_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_status_t));

                param->network_qty      = CTRLM_MAIN_MAX_NETWORKS;
                param->networks[0].id   = 1;
                param->networks[0].type = CTRLM_NETWORK_TYPE_RF4CE;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_START_PAIRING_MODE), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_control_service_pairing_mode_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_control_service_pairing_mode_t));
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("startPairingMode"), "{\"pairingMode\":0,\"restrictPairing\":0}", response_));
    EXPECT_EQ(response_, "{\"status_code\":0,\"success\":true}");
}

TEST_F(ControlServiceTest, endPairingMode)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_STATUS_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_status_t));

                param->network_qty      = CTRLM_MAIN_MAX_NETWORKS;
                param->networks[0].id   = 1;
                param->networks[0].type = CTRLM_NETWORK_TYPE_RF4CE;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_END_PAIRING_MODE), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_control_service_pairing_mode_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_control_service_pairing_mode_t));
                param->result      = CTRLM_IARM_CALL_RESULT_SUCCESS;
                param->bind_status = 1;
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("endPairingMode"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"bindStatus\":1,\"status_code\":0,\"success\":true}");
}

TEST_F(ControlServiceTest, canFindMyRemote)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_CAN_FIND_MY_REMOTE), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_control_service_can_find_my_remote_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_control_service_can_find_my_remote_t));
                param->result       = CTRLM_IARM_CALL_RESULT_SUCCESS;
                param->is_supported = 1;
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("canFindMyRemote"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"result\":true,\"success\":true}");
}

TEST_F(ControlServiceTest, findLastUsedRemote)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_REVERSE_CMD), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_rcu_reverse_cmd_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_rcu_reverse_cmd_t));
                param->result     = CTRLM_IARM_CALL_RESULT_SUCCESS;
                param->cmd_result = CTRLM_RCU_REVERSE_CMD_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("findLastUsedRemote"), "{\"timeOutPeriod\":20}", response_));
    EXPECT_EQ(response_, "{\"status_code\":0,\"success\":true}");
}

TEST_F(ControlServiceTest, checkRf4ceChipConnectivity)
{
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_STATUS_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_status_t));

                param->network_qty      = CTRLM_MAIN_MAX_NETWORKS;
                param->networks[0].id   = 1;
                param->networks[0].type = CTRLM_NETWORK_TYPE_RF4CE;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_CHIP_STATUS_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_chip_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_chip_status_t));
                param->result         = CTRLM_IARM_CALL_RESULT_SUCCESS;
                param->chip_connected = 1;
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("checkRf4ceChipConnectivity"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"rf4ceChipConnected\":1,\"success\":true}");
}

TEST_F(ControlServiceInitializedEventTest, onControl)
{
    EXPECT_CALL(service_, Submit(_, _))
        .WillOnce(Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.ControlService.onControl\","
                                          "\"params\":{\"remoteId\":-1,\"eventValue\":82,"
                                          "\"eventSource\":\"IR\",\"eventType\":\"SETUP\","
                                          "\"eventData\":\"\"}}")));
                return Core::ERROR_NONE;
            }));

    IARM_Bus_IRMgr_EventData_t eventData = {0};
    eventData.data.irkey.keyType         = KET_KEYDOWN;
    eventData.data.irkey.keyCode         = KED_SETUP;
    eventData.data.irkey.keySrc          = IARM_BUS_IRMGR_KEYSRC_IR;
    eventData.data.irkey.keySourceId     = 1;

    handler_.Subscribe(0, _T("onControl"), _T("org.rdk.ControlService"), message_);
    controlEventHandler_(IARM_BUS_IRMGR_NAME, IARM_BUS_IRMGR_EVENT_IRKEY, &eventData, sizeof(IARM_Bus_IRMgr_EventData_t));
    handler_.Unsubscribe(0, _T("onControl"), _T("org.rdk.ControlService"), message_);
}

TEST_F(ControlServiceInitializedEventTest, onXRConfigurationComplete)
{
    EXPECT_CALL(service_, Submit(_, _))
        .Times(1)
        .WillOnce(Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.ControlService.onXRConfigurationComplete\","
                                          "\"params\":{\"remoteId\":1,\"remoteType\":\"XR11\","
                                          "\"bindingType\":1,\"configurationStatus\":0}}")));
                return Core::ERROR_NONE;
            }));

    ctrlm_rcu_iarm_event_configuration_complete_t eventData = {0};
    eventData.api_revision  = CTRLM_RCU_IARM_BUS_API_REVISION;
    eventData.controller_id = 1;
    eventData.result        = CTRLM_RCU_CONFIGURATION_RESULT_SUCCESS;
    eventData.binding_type  = CTRLM_RCU_BINDING_TYPE_AUTOMATIC;
    snprintf(eventData.controller_type, sizeof(eventData.controller_type), "%s", "XR11");

    handler_.Subscribe(0, _T("onXRConfigurationComplete"), _T("org.rdk.ControlService"), message_);
    controlEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_CONFIGURATION_COMPLETE, &eventData, sizeof(ctrlm_rcu_iarm_event_configuration_complete_t));
    handler_.Unsubscribe(0, _T("onXRConfigurationComplete"), _T("org.rdk.ControlService"), message_);
}

TEST_F(ControlServiceInitializedEventTest, onXRPairingStart)
{
    EXPECT_CALL(service_, Submit(_, _))
        .Times(1)
        .WillOnce(Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.ControlService.onXRPairingStart\","
                                          "\"params\":{\"remoteId\":1,\"remoteType\":\"XR11\","
                                          "\"bindingType\":0,\"validationDigit1\":1,"
                                          "\"validationDigit2\":3,\"validationDigit3\":5}}")));
                return Core::ERROR_NONE;
            }));

    ctrlm_rcu_iarm_event_validation_begin_t eventData = {0};
    eventData.api_revision       = CTRLM_RCU_IARM_BUS_API_REVISION;
    eventData.controller_id      = 1;
    eventData.binding_type       = CTRLM_RCU_BINDING_TYPE_INTERACTIVE;
    eventData.validation_keys[0] = CTRLM_KEY_CODE_DIGIT_1;
    eventData.validation_keys[1] = CTRLM_KEY_CODE_DIGIT_3;
    eventData.validation_keys[2] = CTRLM_KEY_CODE_DIGIT_5;
    snprintf(eventData.controller_type, sizeof(eventData.controller_type), "%s", "XR11");

    handler_.Subscribe(0, _T("onXRPairingStart"), _T("org.rdk.ControlService"), message_);
    controlEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN, &eventData, sizeof(ctrlm_rcu_iarm_event_validation_begin_t));
    handler_.Unsubscribe(0, _T("onXRPairingStart"), _T("org.rdk.ControlService"), message_);
}

TEST_F(ControlServiceInitializedEventTest, onXRValidationComplete)
{
    EXPECT_CALL(service_, Submit(_, _))
        .Times(1)
        .WillOnce(Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.ControlService.onXRValidationComplete\","
                                          "\"params\":{\"remoteId\":1,\"remoteType\":\"XR11\","
                                          "\"bindingType\":1,\"validationStatus\":0}}")));
                return Core::ERROR_NONE;
            }));

    ctrlm_rcu_iarm_event_validation_end_t eventData = {0};
    eventData.api_revision  = CTRLM_RCU_IARM_BUS_API_REVISION;
    eventData.controller_id = 1;
    eventData.binding_type  = CTRLM_RCU_BINDING_TYPE_AUTOMATIC;
    eventData.result        = CTRLM_RCU_VALIDATION_RESULT_SUCCESS;
    snprintf(eventData.controller_type, sizeof(eventData.controller_type), "%s", "XR11");

    handler_.Subscribe(0, _T("onXRValidationComplete"), _T("org.rdk.ControlService"), message_);
    controlEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_END, &eventData, sizeof(ctrlm_rcu_iarm_event_validation_end_t));
    handler_.Unsubscribe(0, _T("onXRValidationComplete"), _T("org.rdk.ControlService"), message_);
}

TEST_F(ControlServiceInitializedEventTest, onXRValidationUpdate)
{
    EXPECT_CALL(service_, Submit(_, _))
        .WillOnce(Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.ControlService.onXRPairingStart\","
                                          "\"params\":{\"remoteId\":1,\"remoteType\":\"XR11\","
                                          "\"bindingType\":0,\"validationDigit1\":1,"
                                          "\"validationDigit2\":3,\"validationDigit3\":5}}")));
                return Core::ERROR_NONE;
            }))
        .WillOnce(Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.ControlService.onXRValidationUpdate\","
                                          "\"params\":{\"remoteId\":1,\"remoteType\":\"XR11\","
                                          "\"bindingType\":0,\"validationDigit1\":1}}")));
                return Core::ERROR_NONE;
            }));

    ctrlm_rcu_iarm_event_validation_begin_t beginData = {0};
    ctrlm_rcu_iarm_event_key_press_t        eventData = {0};

    beginData.api_revision       = CTRLM_RCU_IARM_BUS_API_REVISION;
    beginData.controller_id      = 1;
    beginData.binding_type       = CTRLM_RCU_BINDING_TYPE_INTERACTIVE;
    beginData.validation_keys[0] = CTRLM_KEY_CODE_DIGIT_1;
    beginData.validation_keys[1] = CTRLM_KEY_CODE_DIGIT_3;
    beginData.validation_keys[2] = CTRLM_KEY_CODE_DIGIT_5;
    snprintf(beginData.controller_type, sizeof(beginData.controller_type), "%s", "XR11");

    eventData.api_revision  = CTRLM_RCU_IARM_BUS_API_REVISION;
    eventData.controller_id = 1;
    eventData.binding_type  = CTRLM_RCU_BINDING_TYPE_INTERACTIVE;
    eventData.key_code      = CTRLM_KEY_CODE_DIGIT_1;
    eventData.key_status    = CTRLM_KEY_STATUS_DOWN;
    snprintf(eventData.controller_type, sizeof(eventData.controller_type), "%s", "XR11");

    handler_.Subscribe(0, _T("onXRPairingStart"), _T("org.rdk.ControlService"), message_);
    controlEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN, &beginData, sizeof(ctrlm_rcu_iarm_event_validation_begin_t));
    handler_.Unsubscribe(0, _T("onXRPairingStart"), _T("org.rdk.ControlService"), message_);

    handler_.Subscribe(0, _T("onXRValidationUpdate"), _T("org.rdk.ControlService"), message_);
    controlEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_PRESS, &eventData, sizeof(ctrlm_rcu_iarm_event_key_press_t));
    handler_.Unsubscribe(0, _T("onXRValidationUpdate"), _T("org.rdk.ControlService"), message_);
}
