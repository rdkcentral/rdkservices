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

#include <mutex>

#include "Module.h"


#include "tptimer.h"

namespace WPEFramework {

    namespace Plugin {

        enum TimerState {
            INITIAL,
            RUNNING,
            SUSPENDED,
            CANCELED,
            EXPIRED,
        };

        enum TimerMode {
            GENERIC,
            SLEEP,
            WAKE,
        };

        struct TimerItem {
            TimerState state;
            double interval;
            TimerMode mode;
            double repeatInterval;
            double remindBefore;
            std::chrono::system_clock::time_point lastExpired;
            std::chrono::system_clock::time_point lastExpiryReminder;
            bool reminderSent;
        };

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
        class Timer : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:

            // We do not allow this plugin to be copied !!
            Timer(const Timer&) = delete;
            Timer& operator=(const Timer&) = delete;

            //Begin methods
            uint32_t startTimerWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t cancelWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t suspendWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t resumeWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getTimerStatusWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getTimersWrapper(const JsonObject& parameters, JsonObject& response);
            //End methods

            //Begin events
            void sendTimerExpired(int timerId);
            void sendTimerExpiryReminder(int timerId);
            //End events

            void checkTimers();

            void startTimer(int timerId);
            bool cancelTimer(int timerId);
            bool suspendTimer(int timerId);

            void onTimerCallback();
            void getTimerStatus(int timerId, JsonObject& output, bool writeTimerId = false);

        protected:
            void InitializeIARM();
            void DeinitializeIARM();

        public:
            Timer();
            virtual ~Timer();
            virtual const string Initialize(PluginHost::IShell* shell) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            BEGIN_INTERFACE_MAP(Timer)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        public:
            static Timer* _instance;
        private:
            TpTimer m_timer;
            std::vector <TimerItem> m_timerItems;
            std::list <int> m_runningItems;
            std::mutex m_callMutex;
        };
	} // namespace Plugin
} // namespace WPEFramework
