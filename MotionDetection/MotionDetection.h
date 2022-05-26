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

#include <chrono>
#include "Module.h"
#include "motionDetector.h"

namespace WPEFramework {

    namespace Plugin {

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
        class MotionDetection : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:
            typedef Core::JSON::String JString;
            typedef Core::JSON::ArrayType<JString> JStringArray;
            typedef Core::JSON::Boolean JBool;

            // We do not allow this plugin to be copied !!
            MotionDetection(const MotionDetection&) = delete;
            MotionDetection& operator=(const MotionDetection&) = delete;

            //Begin methods
            uint32_t getMotionDetectors(const JsonObject& parameters, JsonObject& response);
            uint32_t arm(const JsonObject& parameters, JsonObject& response);
            uint32_t disarm(const JsonObject& parameters, JsonObject& response);
            uint32_t isarmed(const JsonObject& parameters, JsonObject& response);
            uint32_t setNoMotionPeriod(const JsonObject& parameters, JsonObject& response);
            uint32_t getNoMotionPeriod(const JsonObject& parameters, JsonObject& response);
            uint32_t setSensitivity(const JsonObject& parameters, JsonObject& response);
            uint32_t getSensitivity(const JsonObject& parameters, JsonObject& response);
            uint32_t getLastMotionEventElapsedTime(const JsonObject& parameters, JsonObject& response);
            uint32_t setMotionEventsActivePeriod(const JsonObject& parameters, JsonObject& response);
            uint32_t getMotionEventsActivePeriod(const JsonObject& parameters, JsonObject& response);
            //End methods

        public:
            MotionDetection();
            virtual ~MotionDetection();

            //Build QueryInterface implementation, specifying all possible interfaces to be returned.
            BEGIN_INTERFACE_MAP(MotionDetection)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

            //IPlugin methods
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            //Begin events
            void onMotionEvent(const string& index, const string& eventType);
            //End events
        public:
            static MotionDetection* _instance;

        private:
            std::chrono::system_clock::time_point m_lastEventTime;
        };
	} // namespace Plugin
} // namespace WPEFramework
