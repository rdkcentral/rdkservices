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

#include <vector>

#include "gtest/gtest.h"
#include "FactoriesImplementation.h"
#include "RemoteActionMapping.h"

#include "IarmBusMock.h"
#include "ServiceMock.h"

using namespace WPEFramework;
using ::testing::NiceMock;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Test;
using ::testing::StrEq;
using ::testing::Contains;
using ::testing::AnyOfArray;

class RemoteActionMappingTest : public Test {
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

class RemoteActionMappingKeyActionMapTest : public RemoteActionMappingTest {
protected:
    std::vector<uint8_t> actionMapKeys_{0x4D, 0x6A, 0x6D, 0x4E, 0x69,
                                        0x6C, 0x40, 0x68, 0x6B, 0x43,
                                        0x42, 0x41, 0x34};
    std::vector<uint8_t> actionMapData_{0x00, 0x01, 0x02, 0x03, 0x04,
                                        0x05, 0x06, 0x07, 0x08, 0x09,
                                        0x0A, 0x21, 0x31, 0x4C, 0x8C,
                                        0xC0};

    //AssertionResult testkeyActionMapData(int key, char *data);

    RemoteActionMappingKeyActionMapTest() :
        RemoteActionMappingTest()
    {
    }

    virtual ~RemoteActionMappingKeyActionMapTest() override
    {
    }
};

class RemoteActionMappingFiveDigitCodeTest : public RemoteActionMappingTest {
protected:
    char testFiveDigitCodeData_[CTRLM_RCU_MAX_RIB_ATTRIBUTE_SIZE] = {0};

    RemoteActionMappingFiveDigitCodeTest() :
        RemoteActionMappingTest()
    {
        testFiveDigitCodeData_[0] |= (XRC_RIB_TARGET_STATUS_FLAGS_TV_CODE_PRESENT_BIT | XRC_RIB_TARGET_STATUS_FLAGS_AVR_CODE_PRESENT_BIT);
        for (int i = 1; i < 6; i++)
            testFiveDigitCodeData_[i] |= (0x30 + i);
        for (int i = 7; i < 12; i++)
            testFiveDigitCodeData_[i] |= (0x2F + i);
        testFiveDigitCodeData_[11] = '0';
    }

    virtual ~RemoteActionMappingFiveDigitCodeTest() override
    {
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
        EXPECT_CALL(iarmBusImplMock_, IARM_Bus_RegisterEventHandler(StrEq(CTRLM_MAIN_IARM_BUS_NAME), CTRLM_RCU_IARM_EVENT_RIB_ACCESS_CONTROLLER, _))
            .WillOnce(Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
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

TEST_F(RemoteActionMappingTest, getLastUsedDeviceID)
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

                for (int i = 0; i < CTRLM_MAIN_MAX_BOUND_CONTROLLERS; i++) {
                    param->status.rf4ce.controllers[i] = i+1;
                }
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_controller_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_rcu_iarm_call_controller_status_t));

                snprintf(param->status.type, sizeof(param->status.type), "%s", "XR15-2");
                param->status.time_last_key                 = 1580263335;
                param->status.ir_db_code_download_supported = 1;
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getLastUsedDeviceID"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"deviceID\":1,\"remoteType\":\"XR15-2\","
                         "\"fiveDigitCodePresent\":false,"
                         "\"setFiveDigitCodeSupported\":true,"
                         "\"status_code\":0,\"success\":true}");
}

TEST_F(RemoteActionMappingTest, getKeymap)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getKeymap"), "{\"deviceID\":1,\"keymapType\":1}", response_));
    EXPECT_EQ(response_, "{\"keyNames\":[80,81,128,138,139,140,208],"
                         "\"status_code\":0,\"success\":true}");
}

TEST_F(RemoteActionMappingKeyActionMapTest, setKeyActionMapping)
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

                for (int i = 0; i < CTRLM_MAIN_MAX_BOUND_CONTROLLERS; i++) {
                    param->status.rf4ce.controllers[i] = i+1;
                }
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_controller_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_rcu_iarm_call_controller_status_t));
                param->status.ir_db_state = CTRLM_RCU_IR_DB_STATE_NO_CODES;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
                EXPECT_EQ(param->network_id,    1);
                EXPECT_EQ(param->controller_id, 1);
                EXPECT_EQ(param->attribute_id,  CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE_STATUS);
                EXPECT_EQ(param->length,        1);
                param->data[0] = XRC_RIB_IRRFSTATUS_DOWNLOAD_IRDB_BIT | XRC_RIB_IRRFSTATUS_FORCE_IRDB_BIT;
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET), _, _))
        .WillByDefault(Invoke(
            [&](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
                EXPECT_EQ(param->network_id,    1);
                EXPECT_EQ(param->controller_id, 1);
                EXPECT_EQ(param->attribute_id,  CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE);
                EXPECT_THAT(param->attribute_index, AnyOfArray(actionMapKeys_));
                EXPECT_THAT(param->data, Contains(AnyOfArray(actionMapData_)));
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("setKeyActionMapping"),
                                _T("{\"deviceID\":1,\"keymapType\":1,"
                                "\"keyActionMapping\":["
                                "{\"keyName\":80,\"rfKeyCode\":109,\"tvIRKeyCode\":[0,1,2,3,4,5,6,7,8,9],\"avrIRKeyCode\":[]},"
                                "{\"keyName\":81,\"rfKeyCode\":108,\"tvIRKeyCode\":[0,1,2,3,4,5,6,7,8,9],\"avrIRKeyCode\":[]},"
                                "{\"keyName\":128,\"rfKeyCode\":107,\"tvIRKeyCode\":[0,1,2,3,4,5,6,7,8,9],\"avrIRKeyCode\":[]},"
                                "{\"keyName\":138,\"rfKeyCode\":65,\"tvIRKeyCode\":[0,1,2,3,4,5,6,7,8,9],\"avrIRKeyCode\":[]},"
                                "{\"keyName\":139,\"rfKeyCode\":66,\"tvIRKeyCode\":[0,1,2,3,4,5,6,7,8,9],\"avrIRKeyCode\":[]},"
                                "{\"keyName\":140,\"rfKeyCode\":67,\"tvIRKeyCode\":[0,1,2,3,4,5,6,7,8,9],\"avrIRKeyCode\":[]},"
                                "{\"keyName\":208,\"rfKeyCode\":52,\"tvIRKeyCode\":[0,1,2,3,4,5,6,7,8,9],\"avrIRKeyCode\":[]}]}"),
                                response_));
    EXPECT_EQ(response_, "{\"status_code\":0,\"success\":true}");
}

TEST_F(RemoteActionMappingTest, clearKeyActionMapping)
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
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_controller_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_rcu_iarm_call_controller_status_t));
                param->status.ir_db_state   = CTRLM_RCU_IR_DB_STATE_NO_CODES;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
                EXPECT_EQ(param->network_id,      1);
                EXPECT_EQ(param->controller_id,   1);
                EXPECT_EQ(param->attribute_id,    CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE);
                EXPECT_EQ(param->length,          CONTROLMGR_MAX_IR_DATA_SIZE + 1 + 2);
                EXPECT_EQ(param->data[0],         (char) (MSO_RIB_IRRFDB_PERMANENT_BIT | MSO_RIB_IRRFDB_DEFAULT_BIT));
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
                EXPECT_EQ(param->network_id,      1);
                EXPECT_EQ(param->controller_id,   1);
                EXPECT_EQ(param->attribute_id,    CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE_STATUS);
                EXPECT_EQ(param->length,          1);
                param->data[0] = (XRC_RIB_IRRFSTATUS_DOWNLOAD_IRDB_BIT | XRC_RIB_IRRFSTATUS_FORCE_IRDB_BIT);
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("clearKeyActionMapping"),
                                _T("{\"deviceID\":1,\"keymapType\":0,\"keyNames\":"
                                "[0x80,0x51,0x50,0x8A,0x8B,0x8C,0xD0]}"),
                                response_));
    EXPECT_EQ(response_, "{\"status_code\":0,\"success\":true}");
}

TEST_F(RemoteActionMappingKeyActionMapTest, getFullKeyActionMapping)
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
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET), _, _))
        .WillByDefault(Invoke(
            [&](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
                EXPECT_EQ(param->network_id,    1);
                EXPECT_EQ(param->controller_id, 1);
                EXPECT_EQ(param->attribute_id,  CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE);
                EXPECT_EQ(param->length,        CTRLM_RCU_MAX_RIB_ATTRIBUTE_SIZE);
                EXPECT_THAT(param->attribute_index, AnyOfArray(actionMapKeys_));
                param->data[0] = MSO_RIB_IRRFDB_IRSPECIFIED_BIT;
                param->data[1] = 0xFF;
                param->data[2] = 10;
                for (int i = 3; i < 13; i++) {
                    param->data[i] = i-3;
                }
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getFullKeyActionMapping"), "{\"deviceID\":1,\"keymapType\":1}", response_));
    EXPECT_EQ(response_, "{\"keyMappings\":["
                         "{\"keyName\":80,\"rfKeyCode\":109,\"tvIRKeyCode\":\"[0,1,2,3,4,5,6,7,8,9]\",\"avrIRKeyCode\":\"[]\"},"
                         "{\"keyName\":81,\"rfKeyCode\":108,\"tvIRKeyCode\":\"[0,1,2,3,4,5,6,7,8,9]\",\"avrIRKeyCode\":\"[]\"},"
                         "{\"keyName\":128,\"rfKeyCode\":107,\"tvIRKeyCode\":\"[0,1,2,3,4,5,6,7,8,9]\",\"avrIRKeyCode\":\"[]\"},"
                         "{\"keyName\":138,\"rfKeyCode\":65,\"tvIRKeyCode\":\"[0,1,2,3,4,5,6,7,8,9]\",\"avrIRKeyCode\":\"[]\"},"
                         "{\"keyName\":139,\"rfKeyCode\":66,\"tvIRKeyCode\":\"[0,1,2,3,4,5,6,7,8,9]\",\"avrIRKeyCode\":\"[]\"},"
                         "{\"keyName\":140,\"rfKeyCode\":67,\"tvIRKeyCode\":\"[0,1,2,3,4,5,6,7,8,9]\",\"avrIRKeyCode\":\"[]\"},"
                         "{\"keyName\":208,\"rfKeyCode\":52,\"tvIRKeyCode\":\"[0,1,2,3,4,5,6,7,8,9]\",\"avrIRKeyCode\":\"[]\"}],"
                         "\"status_code\":0,\"success\":true}");
}

TEST_F(RemoteActionMappingKeyActionMapTest, getSingleKeyActionMapping)
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
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET), _, _))
        .WillByDefault(Invoke(
            [&](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
                EXPECT_EQ(param->network_id,      1);
                EXPECT_EQ(param->controller_id,   1);
                EXPECT_EQ(param->attribute_id,    CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE);
                EXPECT_THAT(param->attribute_index, AnyOfArray(actionMapKeys_));
                EXPECT_EQ(param->length,          CTRLM_RCU_MAX_RIB_ATTRIBUTE_SIZE);
                param->data[0] = MSO_RIB_IRRFDB_IRSPECIFIED_BIT;
                param->data[1] = 0xFF;
                param->data[2] = 10;
                for (int i = 3; i < 13; i++) {
                    param->data[i] = i-3;
                }
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("getSingleKeyActionMapping"),
                                "{\"deviceID\":1,\"keymapType\":1,\"keyName\":140}",
                                response_));
    EXPECT_EQ(response_, "{\"keyMapping\":{\"keyName\":140,\"rfKeyCode\":67,"
                         "\"tvIRKeyCode\":\"[0,1,2,3,4,5,6,7,8,9]\",\"avrIRKeyCode\":\"[]\"},"
                         "\"status_code\":0,\"success\":true}");
}

TEST_F(RemoteActionMappingTest, cancelCodeDownload)
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
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
                EXPECT_EQ(param->network_id,      1);
                EXPECT_EQ(param->controller_id,   1);
                EXPECT_EQ(param->attribute_id,    CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE_STATUS);
                EXPECT_EQ(param->attribute_index, 0);
                EXPECT_EQ(param->length,          1);
                param->data[0] = XRC_RIB_IRRFSTATUS_DONT_DOWNLOAD_IRDB_BIT;
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("cancelCodeDownload"), "{\"deviceID\":1}", response_));
    EXPECT_EQ(response_, "{\"status_code\":0,\"success\":true}");
}

TEST_F(RemoteActionMappingFiveDigitCodeTest, setFiveDigitCode)
{
    EXPECT_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_MAIN_IARM_CALL_STATUS_GET), _, _))
        .WillRepeatedly(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_main_iarm_call_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_main_iarm_call_status_t));

                param->network_qty      = CTRLM_MAIN_MAX_NETWORKS;
                param->networks[0].id   = 1;
                param->networks[0].type = CTRLM_NETWORK_TYPE_RF4CE;
                return IARM_RESULT_SUCCESS;
            }));
    EXPECT_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET), _, _))
        .WillOnce(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
                EXPECT_EQ(param->network_id,      1);
                EXPECT_EQ(param->controller_id,   1);
                EXPECT_EQ(param->attribute_id,    CTRLM_RCU_RIB_ATTR_ID_TARGET_IRDB_STATUS);
                EXPECT_EQ(param->attribute_index, 0);
                EXPECT_EQ(param->length,          CTRLM_RCU_RIB_ATTR_LEN_TARGET_IRDB_STATUS);
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }))
        .WillOnce(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
                EXPECT_EQ(param->network_id,      1);
                EXPECT_EQ(param->controller_id,   1);
                EXPECT_EQ(param->attribute_id,    CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE_STATUS);
                EXPECT_EQ(param->attribute_index, 0);
                EXPECT_EQ(param->length,          1);
                param->data[0] = (XRC_RIB_IRRFSTATUS_DONT_DOWNLOAD_IRDB_BIT |
                                  XRC_RIB_IRRFSTATUS_DOWNLOAD_TV_5DCODE_BIT |
                                  XRC_RIB_IRRFSTATUS_DOWNLOAD_AVR_5DCODE_BIT);
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));
    EXPECT_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET), _, _))
        .WillOnce(Invoke(
            [&](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
                EXPECT_EQ(param->network_id,      1);
                EXPECT_EQ(param->controller_id,   1);
                EXPECT_EQ(param->attribute_id,    CTRLM_RCU_RIB_ATTR_ID_TARGET_IRDB_STATUS);
                EXPECT_EQ(param->attribute_index, 0);
                EXPECT_EQ(param->length,          CTRLM_RCU_RIB_ATTR_LEN_TARGET_IRDB_STATUS);
                for (int i = 0; i < CTRLM_RCU_RIB_ATTR_LEN_TARGET_IRDB_STATUS; i++) {
                    EXPECT_EQ(param->data[i], testFiveDigitCodeData_[i]) << "Index: " << i;
                }
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("setFiveDigitCode"),
                                "{\"deviceID\":1,\"tvFiveDigitCode\":12345,\"avrFiveDigitCode\":67890}",
                                response_));
    EXPECT_EQ(response_, "{\"status_code\":0,\"success\":true}");
}

TEST_F(RemoteActionMappingInitializedEventTest, onIRCodeLoad)
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
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_controller_status_t*>(arg);
                memset(param, 0, sizeof(ctrlm_rcu_iarm_call_controller_status_t));
                param->status.ir_db_state   = CTRLM_RCU_IR_DB_STATE_NO_CODES;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));
    EXPECT_CALL(service_, Submit(_, _))
        .WillOnce(Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.RemoteActionMapping.onIRCodeLoad\","
                                          "\"params\":{\"deviceID\":1,\"keyNames\":[208,138,139,140,128,81,80],"
                                          "\"rfKeyCodes\":[52,65,66,67,107,108,109],\"loadStatus\":0}}")));
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

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("clearKeyActionMapping"),
                                _T("{\"deviceID\":1,\"keymapType\":0,\"keyNames\":"
                                "[0x80,0x51,0x50,0x8A,0x8B,0x8C,0xD0]}"),
                                response_));

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
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));
    ON_CALL(iarmBusImplMock_, IARM_Bus_Call(StrEq(CTRLM_MAIN_IARM_BUS_NAME), StrEq(CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET), _, _))
        .WillByDefault(Invoke(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                auto param = static_cast<ctrlm_rcu_iarm_call_rib_request_t*>(arg);
                param->result = CTRLM_IARM_CALL_RESULT_SUCCESS;
                return IARM_RESULT_SUCCESS;
            }));
    EXPECT_CALL(service_, Submit(_, _))
        .WillOnce(Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.RemoteActionMapping.onFiveDigitCodeLoad\","
                                          "\"params\":{\"deviceID\":1,\"tvLoadStatus\":0,\"avrLoadStatus\":0}}")));
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

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_,
                                _T("setFiveDigitCode"),
                                "{\"deviceID\":1,\"tvFiveDigitCode\":0,\"avrFiveDigitCode\":0}",
                                response_));

    handler_.Subscribe(0, _T("onFiveDigitCodeLoad"), _T("org.rdk.RemoteActionMapping"), message_);
    ramEventHandler_(CTRLM_MAIN_IARM_BUS_NAME,CTRLM_RCU_IARM_EVENT_RIB_ACCESS_CONTROLLER,&eventData, sizeof(ctrlm_rcu_iarm_event_rib_entry_access_t));

    eventData.identifier  = CTRLM_RCU_RIB_ATTR_ID_CONTROLLER_IRDB_STATUS;
    eventData.access_type = CTRLM_ACCESS_TYPE_WRITE;
    ramEventHandler_(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_RIB_ACCESS_CONTROLLER, &eventData, sizeof(ctrlm_rcu_iarm_event_rib_entry_access_t));

    handler_.Unsubscribe(0, _T("onFiveDigitCodeLoad"), _T("org.rdk.RemoteActionMapping"), message_);
}
