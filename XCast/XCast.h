/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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

/**
 * @file XCast.h
 * @brief Thunder Plugin based Implementation for TTS service API's (RDK-27957).
 */

/**
  @mainpage XCast

  <b>XCast</b> XCast Thunder Service provides APIs for the arbitrators
  * (ex: Native application such as Cobalt) to use TTS resource.
  */

#pragma once

#include "Module.h"
#include <interfaces/IXCast.h>
#include "tracing/Logging.h"
#include "tptimer.h"
#include "RtNotifier.h"
#include "libIBus.h"
#include "libIBusDaemon.h"
#include "pwrMgr.h"
#include "XCastCommon.h"
#include <mutex>
#include <map>
#include "UtilsLogging.h"

namespace WPEFramework {
namespace Plugin {
    class XCast: public PluginHost::IPlugin, public PluginHost::JSONRPC {
    public:
        class Notification : public RPC::IRemoteConnection::INotification,
                             public Exchange::IXCast::INotification {
            private:
                Notification() = delete;
                Notification(const Notification&) = delete;
                Notification& operator=(const Notification&) = delete;

            public:
                explicit Notification(XCast* parent)
                    : _parent(*parent) {
                    ASSERT(parent != nullptr);
                }

                virtual ~Notification() {
                }

            public:
                virtual void onApplicationLaunchRequestWithLaunchParam(const string& appName, const string& strPayLoad, const string& strQuery, const string& strAddDataUrl) override;
                virtual void onApplicationLaunchRequest(const string& appName, const string& parameter) override;
				virtual void onApplicationStopRequest(const string& appName, const string& appID) override;
				virtual void onApplicationHideRequest(const string& appName, const string& appID) override;
				virtual void onApplicationStateRequest(const string& appName, const string& appID) override;
				virtual void onApplicationResumeRequest(const string& appName, const string& appID) override;
                virtual void Activated(RPC::IRemoteConnection* /* connection */) final
                {
                    LOGINFO("XCast::Notification::Activated - %p", this);
                }

                virtual void Deactivated(RPC::IRemoteConnection* connection) final
                {
                    LOGINFO("XCast::Notification::Deactivated - %p", this);
                    _parent.Deactivated(connection);
                }

                BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(Exchange::IXCast::INotification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                END_INTERFACE_MAP

            private:
                XCast& _parent;
        };

        BEGIN_INTERFACE_MAP(XCast)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(Exchange::IXCast, _xcast)
        END_INTERFACE_MAP

    public:
        XCast();
        virtual ~XCast();
        virtual const string Initialize(PluginHost::IShell* service) override;
        virtual void Deinitialize(PluginHost::IShell* service) override;
        virtual string Information() const override { return {}; }

    private:
        // We do not allow this plugin to be copied !!
        XCast(const XCast&) = delete;
        XCast& operator=(const XCast&) = delete;

        void RegisterAll();

        void Deactivated(RPC::IRemoteConnection* connection);
        //Begin methods
        uint32_t getApiVersionNumber(const JsonObject& parameters, JsonObject& response);
        uint32_t applicationStateChanged(const JsonObject& parameters, JsonObject& response);
        uint32_t setEnabled(const JsonObject& parameters, JsonObject& response);
        uint32_t getEnabled(const JsonObject& parameters, JsonObject& response);
        uint32_t setStandbyBehavior(const JsonObject& parameters, JsonObject& response);
        uint32_t getStandbyBehavior(const JsonObject& parameters, JsonObject& response);
        uint32_t setFriendlyName(const JsonObject& parameters, JsonObject& response);
        uint32_t getFriendlyName(const JsonObject& parameters, JsonObject& response);
        uint32_t registerApplications(const JsonObject& parameters, JsonObject& response);
        uint32_t unregisterApplications(const JsonObject& parameters, JsonObject& response);
        uint32_t getProtocolVersion(const JsonObject& parameters, JsonObject& response);
        uint32_t registerApplicationsInternal(std::vector<DynamicAppConfig*> appConfigEntries);
        //End methods
        /**
         * Whether Cast service is enabled by RFC
         */
        static bool m_xcastEnable;
        static IARM_Bus_PWRMgr_PowerState_t m_powerState;
        bool m_isDynamicRegistrationsRequired;
        mutex m_appConfigMutex;
        WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> * m_SystemPluginObj = NULL;
        std::vector<DynamicAppConfig*> m_appConfigCache;
        static string m_friendlyName;
        static bool m_standbyBehavior;
        //Timer related variables and functions
        TpTimer m_locateCastTimer;
        void InitializeIARM();
        void DeinitializeIARM();
        //Internal methods
        void onLocateCastTimer();
        void getUrlFromAppLaunchParams (const char *app_name, const char *payload, const char *query_string, const char *additional_data_url, char *url);
        bool getEntryFromAppLaunchParamList (const char* appName, DynamicAppConfig& retAppConfig);
        void dumpDynamicAppConfigCache(string strListName, std::vector<DynamicAppConfig*> appConfigList);
        bool deleteFromDynamicAppCache(JsonArray applications);
        bool deleteFromDynamicAppCache(vector<string>& appsToDelete);
        void updateDynamicAppCache(JsonArray applications);
        void getSystemPlugin();
        int updateSystemFriendlyName();
        void onFriendlyNameUpdateHandler(const JsonObject& parameters);

        /**
         * Check whether the xdial service is allowed in this device.
         */
        static void powerModeChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
        static void threadPowerModeChangeEvent(void);
    private:
        uint8_t _skipURL{};
        uint32_t _connectionId{};
        PluginHost::IShell* _service{};
        Exchange::IXCast* _xcast{};
        Core::Sink<Notification> _notification;
        uint32_t m_apiVersionNumber;
        friend class Notification;
    };

} // namespace Plugin
} // namespace WPEFramework
