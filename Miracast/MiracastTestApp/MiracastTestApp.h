/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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
#include <vector>

#include "Module.h"
#include <securityagent/SecurityTokenUtil.h>
#include <MiracastController.h>
#include "libIARM.h"

using std::vector;
namespace WPEFramework
{
    namespace Plugin
    {
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
        class MiracastTestApp : public PluginHost::IPlugin, public PluginHost::JSONRPC, public MiracastTestAppNotifier
        {
        public:
            // constants
            static const short API_VERSION_NUMBER_MAJOR;
            static const short API_VERSION_NUMBER_MINOR;
            static const string SERVICE_NAME;

            // methods
            static const string METHOD_MIRACAST_SET_ENABLE;
            static const string METHOD_MIRACAST_GET_ENABLE;
            static const string METHOD_MIRACAST_CLIENT_CONNECT;
            static const string METHOD_MIRACAST_STOP_CLIENT_CONNECT;
            static const string METHOD_MIRACAST_SET_VIDEO_FORMATS;
            static const string METHOD_MIRACAST_SET_AUDIO_FORMATS;
            static const string METHOD_MIRACAST_TEST_NOTIFIER;

            MiracastTestApp();
            virtual ~MiracastTestApp();
            virtual const string Initialize(PluginHost::IShell *shell) override;
            virtual void Deinitialize(PluginHost::IShell *service) override;
            virtual string Information() const override;

            virtual void onMiracastTestAppClientConnectionRequest(string client_mac, string client_name) override;
            virtual void onMiracastTestAppClientStopRequest(string client_mac, string client_name) override;
            virtual void onMiracastTestAppClientConnectionStarted(string client_mac, string client_name) override;
            virtual void onMiracastTestAppClientConnectionError(string client_mac, string client_name) override;

            BEGIN_INTERFACE_MAP(MiracastTestApp)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

            PluginHost::IShell *m_CurrentService;
            static MiracastTestApp *_instance;
            static MiracastController *m_miracast_ctrler_obj;

        private:
            bool m_isServiceInitialized;
            bool m_isServiceEnabled;
            WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> *m_SystemPluginObj = NULL;
            uint32_t setEnable(const JsonObject &parameters, JsonObject &response);
            uint32_t getEnable(const JsonObject &parameters, JsonObject &response);
            uint32_t acceptClientConnection(const JsonObject &parameters, JsonObject &response);
            uint32_t stopClientConnection(const JsonObject &parameters, JsonObject &response);
            uint32_t setVideoFormats(const JsonObject &parameters, JsonObject &response);
            uint32_t setAudioFormats(const JsonObject &parameters, JsonObject &response);
            uint32_t testNotifier(const JsonObject &parameters, JsonObject &response);
            void getSystemPlugin();
            int updateSystemFriendlyName();
            void onFriendlyNameUpdateHandler(const JsonObject& parameters);

            // We do not allow this plugin to be copied !!
            MiracastTestApp(const MiracastTestApp &) = delete;
            MiracastTestApp &operator=(const MiracastTestApp &) = delete;
        };
    } // namespace Plugin
} // namespace WPEFramework
