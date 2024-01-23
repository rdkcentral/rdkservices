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

#include <interfaces/IStore2.h>
#include <libIBus.h>
#include <sysMgr.h>

namespace WPEFramework {
namespace Plugin {
    namespace Sqlite {

        template <typename ACTUALSTORE2>
        class Store2WithClockSyncType : public ACTUALSTORE2 {
        private:
            Store2WithClockSyncType(const Store2WithClockSyncType<ACTUALSTORE2>&) = delete;
            Store2WithClockSyncType<ACTUALSTORE2>& operator=(const Store2WithClockSyncType<ACTUALSTORE2>&) = delete;

            struct ClockSync {
                std::atomic_bool Synced{ false };
            };

            using ClockSyncSingleton = Core::SingletonType<ClockSync>;

            class IARMHandler {
            private:
                IARMHandler(const IARMHandler&) = delete;
                IARMHandler& operator=(const IARMHandler&) = delete;

            public:
                IARMHandler()
                {
                    auto rc = IARM_Bus_Init(__FUNCTION__);
                    if ((rc != IARM_RESULT_SUCCESS) && (rc != IARM_RESULT_INVALID_STATE)) {
                        TRACE(Trace::Error, (_T("Sqlite IARM init error %d"), rc));
                    }
                    rc = IARM_Bus_Connect();
                    if ((rc != IARM_RESULT_SUCCESS) && (rc != IARM_RESULT_INVALID_STATE)) {
                        TRACE(Trace::Error, (_T("Sqlite IARM connect error %d"), rc));
                    }

                    IARM_Bus_SYSMgr_GetSystemStates_Param_t param;
                    memset(&param, 0, sizeof(param));
                    rc = IARM_Bus_Call(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_API_GetSystemStates, &param, sizeof(param));
                    if (rc == IARM_RESULT_SUCCESS) {
                        TRACE(Trace::Information, (_T("Sqlite time source state is %d"), param.time_source.state));
                        if (param.time_source.state) {
                            ClockSyncSingleton::Instance().Synced = true;
                        }
                    } else {
                        TRACE(Trace::Error, (_T("Sqlite IARM call error %d"), rc));
                    }

                    rc = IARM_Bus_RegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, Handler);
                    if (rc != IARM_RESULT_SUCCESS) {
                        TRACE(Trace::Error, (_T("Sqlite IARM register error %d"), rc));
                    }
                }
                ~IARMHandler()
                {
                    auto rc = IARM_Bus_RemoveEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, Handler);
                    if (rc != IARM_RESULT_SUCCESS) {
                        TRACE(Trace::Error, (_T("Sqlite IARM remove error %d"), rc));
                    }
                }

            private:
                static void Handler(const char* owner, IARM_EventId_t eventId, void* data, size_t len)
                {
                    auto sysEventData = (IARM_Bus_SYSMgr_EventData_t*)data;
                    if (sysEventData->data.systemStates.stateId == IARM_BUS_SYSMGR_SYSSTATE_TIME_SOURCE) {
                        TRACE_GLOBAL(Trace::Information, (_T("Sqlite time source state changed %d"), sysEventData->data.systemStates.state));
                        ClockSyncSingleton::Instance().Synced = sysEventData->data.systemStates.state;
                    }
                }
            };

        public:
            template <typename... Args>
            Store2WithClockSyncType(Args&&... args)
                : ACTUALSTORE2(std::forward<Args>(args)...)
            {
            }
            ~Store2WithClockSyncType() override = default;

        public:
            uint32_t SetValue(const Exchange::IStore2::ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl) override
            {
                return ((ttl != 0) && !ClockSyncSingleton::Instance().Synced)
                    ? Core::ERROR_PENDING_CONDITIONS
                    : ACTUALSTORE2::SetValue(scope, ns, key, value, ttl);
            }
            uint32_t GetValue(const Exchange::IStore2::ScopeType scope, const string& ns, const string& key, string& value, uint32_t& ttl) override
            {
                string v;
                uint32_t t;
                auto result = ACTUALSTORE2::GetValue(scope, ns, key, v, t);
                if (result == Core::ERROR_NONE) {
                    if ((t != 0) && !ClockSyncSingleton::Instance().Synced) {
                        result = Core::ERROR_PENDING_CONDITIONS;
                    } else {
                        value = v;
                        ttl = t;
                    }
                } else if ((result == Core::ERROR_UNKNOWN_KEY) && !ClockSyncSingleton::Instance().Synced) {
                    result = Core::ERROR_PENDING_CONDITIONS;
                }
                return result;
            }

        private:
            IARMHandler handler;
        };

    } // namespace Sqlite
} // namespace Plugin
} // namespace WPEFramework
