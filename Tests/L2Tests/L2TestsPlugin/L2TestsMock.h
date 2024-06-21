/* If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2023 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

#pragma once

#include <websocket/JSONRPCLink.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "UtilsJsonRpc.h"

#include "IarmBusMock.h"
#include "RfcApiMock.h"
#include "UdevMock.h"
#include "WrapsMock.h"
#include "RBusMock.h"
#include "TelemetryMock.h"
#include "VideoOutputPortConfigMock.h"
#include "VideoOutputPortMock.h"
#include "HostMock.h"
#include "ManagerMock.h"

using ::testing::NiceMock;
using namespace WPEFramework;

class L2TestMocks : public ::testing::Test {
protected:
        RfcApiImplMock   *p_rfcApiImplMock = nullptr ;
        RfcApi           *p_rfcApi         = nullptr ;
        IarmBusImplMock  *p_iarmBusImplMock = nullptr ;
        WrapsImplMock    *p_wrapsImplMock = nullptr ;
        UdevImplMock     *p_udevImplMock = nullptr ;
        HostImplMock     *p_hostImplMock = nullptr ;
        VideoOutputPortConfigImplMock *p_videoOutputPortConfigImplMock = nullptr ;
        ManagerImplMock  *p_managerImplMock = nullptr ;
        VideoOutputPortMock *p_videoOutputPortMock = nullptr;
        RBusApiImplMock *p_rBusApiImplMock = nullptr;
        TelemetryApiImplMock   *p_telemetryApiImplMock = nullptr ;

        std::string thunder_address;

        L2TestMocks();
        virtual ~L2TestMocks();

       /**
         * @brief Invoke a service method
         *
         * @param[in] callsign Service callsign
         * @param[in] method Method name
         * @param[in] params Method parameters
         * @param[out] results Method results
         * @return Zero (Core::ERROR_NONE) on succes or another value on error
         */
        uint32_t InvokeServiceMethod(const char *callsign, const char *method, JsonObject &params, JsonObject &results);

        /**
         * @brief Activate a service plugin
         *
         * @param[in] callsign Service callsign
         * @return Zero (Core::ERROR_NONE) on succes or another value on error
         */
        uint32_t ActivateService(const char *callsign);

        /**
         * @brief Deactivate a service plugin
         *
         * @param[in] callsign Service callsign
         * @return Zero (Core::ERROR_NONE) on succes or another value on error
         */
        uint32_t DeactivateService(const char *callsign);

};

