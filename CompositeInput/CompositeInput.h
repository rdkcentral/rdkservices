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

#include "libIBus.h"

#include "Module.h"

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
        class CompositeInput : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:

            // We do not allow this plugin to be copied !!
            CompositeInput(const CompositeInput&) = delete;
            CompositeInput& operator=(const CompositeInput&) = delete;

            void InitializeIARM();
            void DeinitializeIARM();

            //Begin methods
            uint32_t getCompositeInputDevicesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t startCompositeInput(const JsonObject& parameters, JsonObject& response);
            uint32_t stopCompositeInput(const JsonObject& parameters, JsonObject& response);

            uint32_t setVideoRectangleWrapper(const JsonObject& parameters, JsonObject& response);
            //End methods

            JsonArray getCompositeInputDevices();

            bool setVideoRectangle(int x, int y, int width, int height);

            void compositeInputHotplug( int input , int connect);
            static void dsCompositeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

            void compositeInputSignalChange( int port , int signalStatus);
            static void dsCompositeSignalStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

            void compositeInputStatusChange( int port , bool isPresented);
            static void dsCompositeStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

        public:
            CompositeInput();
            virtual ~CompositeInput();
            virtual const string Initialize(PluginHost::IShell* shell) override { return {}; }
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            void terminate();

            BEGIN_INTERFACE_MAP(CompositeInput)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        public:
            static CompositeInput* _instance;
        };
	} // namespace Plugin
} // namespace WPEFramework
