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
#include <MiracastController.h>

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
        class MiracastService : public PluginHost::IPlugin, public PluginHost::JSONRPC, public MiracastServiceNotifier
        {
        public:
            // constants
            static const short API_VERSION_NUMBER_MAJOR;
            static const short API_VERSION_NUMBER_MINOR;
            static const string SERVICE_NAME;

            // methods
            static const string METHOD_MIRACAST_SET_ENABLE;
            static const string METHOD_MIRACAST_GET_ENABLE;
            static const string METHOD_MIRACAST_SET_P2P_BACKEND_DISCOVERY;
            static const string METHOD_MIRACAST_CLIENT_CONNECT;
            static const string METHOD_MIRACAST_STOP_CLIENT_CONNECT;
            static const string METHOD_MIRACAST_SET_UPDATE_PLAYER_STATE;
            static const string METHOD_MIRACAST_SET_LOG_LEVEL;

#ifdef ENABLE_MIRACAST_SERVICE_TEST_NOTIFIER
            static const string METHOD_MIRACAST_TEST_NOTIFIER;
            uint32_t testNotifier(const JsonObject &parameters, JsonObject &response);
            bool m_isTestNotifierEnabled;
#endif /* ENABLE_MIRACAST_SERVICE_TEST_NOTIFIER */

            MiracastService();
            virtual ~MiracastService();
            virtual const string Initialize(PluginHost::IShell *shell) override;
            virtual void Deinitialize(PluginHost::IShell *service) override;
            virtual string Information() const override;

            virtual void onMiracastServiceClientConnectionRequest(string client_mac, string client_name) override;
            virtual void onMiracastServiceClientConnectionError(string client_mac, string client_name , eMIRACAST_SERVICE_ERR_CODE error_code ) override;
            virtual void onMiracastServiceLaunchRequest(string src_dev_ip, string src_dev_mac, string src_dev_name, string sink_dev_ip) override;

            BEGIN_INTERFACE_MAP(MiracastService)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

            PluginHost::IShell *m_CurrentService;
            static MiracastService *_instance;
            static MiracastController *m_miracast_ctrler_obj;

        private:
            bool m_isServiceInitialized;
            bool m_isServiceEnabled;
            guint m_FriendlyNameMonitorTimerID{0};
            eMIRA_SERVICE_STATES m_eService_state;
            WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> *m_SystemPluginObj = NULL;
            uint32_t setEnable(const JsonObject &parameters, JsonObject &response);
            uint32_t getEnable(const JsonObject &parameters, JsonObject &response);
            uint32_t setP2PBackendDiscovery(const JsonObject &parameters, JsonObject &response);
            uint32_t acceptClientConnection(const JsonObject &parameters, JsonObject &response);
            uint32_t stopClientConnection(const JsonObject &parameters, JsonObject &response);
            uint32_t updatePlayerState(const JsonObject &parameters, JsonObject &response);
            uint32_t setLogging(const JsonObject &parameters, JsonObject &response);

            std::string reasonDescription(eMIRACAST_SERVICE_ERR_CODE e);
            void getSystemPlugin();
            bool updateSystemFriendlyName();
            void onFriendlyNameUpdateHandler(const JsonObject &parameters);
            static gboolean monitor_friendly_name_timercallback(gpointer userdata);
            bool envGetValue(const char *key, std::string &value);

            // We do not allow this plugin to be copied !!
            MiracastService(const MiracastService &) = delete;
            MiracastService &operator=(const MiracastService &) = delete;
        };
    } // namespace Plugin
} // namespace WPEFramework
