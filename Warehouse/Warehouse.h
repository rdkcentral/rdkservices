/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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

#pragma once

#include <thread>

#include "Module.h"
#include "utils.h"
#include "UtilsThreadRAII.h"
#include "libIARM.h"

namespace WPEFramework {

    namespace Plugin {

#ifdef HAS_FRONT_PANEL
        class Warehouse;
        class LedInfo
        {
        private:
            LedInfo() = delete;
            LedInfo& operator=(const LedInfo& RHS) = delete;

        public:
            LedInfo(Warehouse* wh)
            : m_warehouse(wh)
            {
            }
            LedInfo(const LedInfo& copy)
            : m_warehouse(copy.m_warehouse)
            {
            }
            ~LedInfo() {}

            inline bool operator==(const LedInfo& RHS) const
            {
                return(m_warehouse == RHS.m_warehouse);
            }

        public:
            uint64_t Timed(const uint64_t scheduledTime);

        private:
            Warehouse* m_warehouse;
        };
#endif

        // This is a server for a JSONRPC communication channel. 
        // For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
        // By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
        // This realization of this interface implements, by default, the following methods on this plugin
        // - exists
        // - register
        // - unregister
        // Any other methood to be handled by this plugin  can be added can be added by using the
        // templated methods Register on the PluginHost::JSONRPC class.
        // As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
        // this class exposes a public method called, Notify(), using this methods, all subscribed clients
        // will receive a JSONRPC message as a notification, in case this method is called.
        class Warehouse : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:

            // We do not allow this plugin to be copied !!
            Warehouse(const Warehouse&) = delete;
            Warehouse& operator=(const Warehouse&) = delete;

            void resetDevice();
            void resetDevice(bool suppressReboot, const string& resetType = string());
            std::vector<std::string>  getAllowedCNameTails();
            void setFrontPanelState(int state, JsonObject& response);
            void internalReset(JsonObject& response);
            void lightReset(JsonObject& response);
            void isClean(int age, JsonObject& response);
            bool executeHardwareTest() const;
            bool getHardwareTestResults(string& testResults) const;

            //Begin methods
            uint32_t resetDeviceWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getDeviceInfoWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setFrontPanelStateWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t internalResetWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t lightResetWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t isCleanWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t executeHardwareTestWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getHardwareTestResultsWrapper(const JsonObject& parameters, JsonObject& response);
            //End methods

	    static void dsWareHouseOpnStatusChanged(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
        public:
            Warehouse();
            virtual ~Warehouse();
            //IPlugin methods
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            void onSetFrontPanelStateTimer();

            BEGIN_INTERFACE_MAP(Warehouse)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        private:
            void InitializeIARM();
            void DeinitializeIARM();

            void getDeviceInfo(JsonObject &params);

            Utils::ThreadRAII m_resetThread;

#ifdef HAS_FRONT_PANEL
            Core::TimerType<LedInfo> m_ledTimer;
            LedInfo m_ledInfo;
            int m_ledTimerIteration;
            int m_ledState;
#endif
        public:
            static Warehouse* _instance;
        };
	} // namespace Plugin
} // namespace WPEFramework
