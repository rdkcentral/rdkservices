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
#include <mutex>
#ifdef ENABLE_ERM
#include <essos-resmgr.h>
#endif

#include "Module.h"

#include "utils.h"
#include "AbstractPlugin.h"

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
        class DeviceDiagnostics : public AbstractPlugin {
        private:

            // We do not allow this plugin to be copied !!
            DeviceDiagnostics(const DeviceDiagnostics&) = delete;
            DeviceDiagnostics& operator=(const DeviceDiagnostics&) = delete;

            //Begin methods
            uint32_t getConfigurationWrapper(const JsonObject& parameters, JsonObject& response);
            //End methods

            int getConfiguration(const std::string& postData, JsonObject& response);
            uint32_t getAVDecoderStatus(const JsonObject& parameters, JsonObject& response);
            int getMostActiveDecoderStatus();
            void onDecoderStatusChange(int status);
#ifdef ENABLE_ERM
            static void *AVPollThread(void *arg);
#endif

        private:
#ifdef ENABLE_ERM
            std::thread m_AVPollThread;
            std::mutex m_AVDecoderStatusLock;
            EssRMgr* m_EssRMgr;
            int m_pollThreadRun;
#endif

        public:
            DeviceDiagnostics();
            virtual ~DeviceDiagnostics();
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual const string Initialize(PluginHost::IShell* service) override;

        public:
            static DeviceDiagnostics* _instance;

        };
	} // namespace Plugin
} // namespace WPEFramework


