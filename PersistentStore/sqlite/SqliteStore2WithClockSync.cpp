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

#pragma once

#include "SqliteStore2WithClockSync.h"

#include "libIARM.h"
#include "sysMgr.h"

namespace WPEFramework {
namespace Plugin {

    SqliteStore2WithClockSync::IARMHandler::IARMHandler()
        : clockSynced(false)
    {
        auto rc = IARM_Bus_Init(__FUNCTION__);
        if (rc != IARM_RESULT_SUCCESS && rc != IARM_RESULT_INVALID_STATE) {
            TRACE(Trace::Error, (_T("IARM_Bus_Init returned %d"), rc));
        }
        rc = IARM_Bus_Connect();
        if (rc != IARM_RESULT_SUCCESS && rc != IARM_RESULT_INVALID_STATE) {
            TRACE(Trace::Error, (_T("IARM_Bus_Connect returned %d"), rc));
        }

        IARM_Bus_SYSMgr_GetSystemStates_Param_t param;
        memset(&param, 0, sizeof(param));
        rc = IARM_Bus_Call(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_API_GetSystemStates, &param, sizeof(param));
        if (rc == IARM_RESULT_SUCCESS) {
            TRACE(Trace::Information, (_T("time_source.state %d"), param.time_source.state));
            if (param.time_source.state) {
                clockSynced = true;
            }
        } else {
            TRACE(Trace::Error, (_T("IARM_Bus_Call returned %d"), rc));
        }

        rc = IARM_Bus_RegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, onEvent);
        if (rc != IARM_RESULT_SUCCESS) {
            TRACE(Trace::Error, (_T("IARM_Bus_RegisterEventHandler returned %d"), rc));
        }
    }

    SqliteStore2WithClockSync::IARMHandler::~IARMHandler()
    {
        auto rc = IARM_Bus_RemoveEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, onEvent);
        if (rc != IARM_RESULT_SUCCESS) {
            TRACE(Trace::Error, (_T("IARM_Bus_RemoveEventHandler returned %d"), rc));
        }
    }

    void SqliteStore2WithClockSync::IARMHandler::onEvent(const char* owner, IARM_EventId_t eventId, void* data, size_t len)
    {
        auto sysEventData = (IARM_Bus_SYSMgr_EventData_t*)data;
        if (sysEventData->data.systemStates.stateId == IARM_BUS_SYSMGR_SYSSTATE_TIME_SOURCE) {
            TRACE(Trace::Information, (_T("time_source.state %d"), sysEventData->data.systemStates.state));
            clockSynced = sysEventData->data.systemStates.state;
        }
    }

} // namespace Plugin
} // namespace WPEFramework
