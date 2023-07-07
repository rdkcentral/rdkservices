/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
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
 **/

#pragma once

#include <iostream>
#include <mutex>

#include "tptimer.h"
#include "Module.h"
#include "RtNotifier.h"
#include "libIBus.h"
#include "libIBusDaemon.h"
#include "pwrMgr.h"
#include "XCastCommon.h"

using namespace std;

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
class XCast : public PluginHost::IPlugin, public PluginHost::JSONRPC, public RtNotifier {
private:
    
    // We do not allow this plugin to be copied !!
    XCast(const XCast&) = delete;
    XCast& operator=(const XCast&) = delete;
    
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
    //End methods
    
    //Begin events
    
    //End events
public:
    XCast();
    virtual ~XCast();
    //Build QueryInterface implementation, specifying all possible interfaces to be returned.
    BEGIN_INTERFACE_MAP(XCast)
    INTERFACE_ENTRY(PluginHost::IPlugin)
    INTERFACE_ENTRY(PluginHost::IDispatcher)
    END_INTERFACE_MAP
    //IPlugin methods
    virtual const string Initialize(PluginHost::IShell* service) override;
    virtual void Deinitialize(PluginHost::IShell* service) override;
    virtual string Information() const override;
    
    virtual void onRtServiceDisconnected(void) override;
    virtual void onXcastApplicationLaunchRequest(string appName, string parameter) override;
    virtual void onXcastApplicationLaunchRequestWithLaunchParam (string appName,
                    string strPayLoad, string strQuery, string strAddDataUrl) override;
    virtual void onXcastApplicationStopRequest(string appName, string appID) override;
    virtual void onXcastApplicationHideRequest(string appName, string appID) override;
    virtual void onXcastApplicationResumeRequest(string appName, string appID) override;
    virtual void onXcastApplicationStateRequest(string appName, string appID) override;
private:
    /**
     * Whether Cast service is enabled by RFC
     */
    static bool isCastEnabled;
    static bool m_xcastEnable;
    static IARM_Bus_PWRMgr_PowerState_t m_powerState;
    uint32_t m_apiVersionNumber;
    bool m_isDynamicRegistrationsRequired;
    mutex m_appConfigMutex;
    WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> * m_SystemPluginObj = NULL;
    PluginHost::IShell *m_CurrentService;
    std::vector<DynamicAppConfig*> m_appConfigCache;
    static string m_friendlyName;
    static bool m_standbyBehavior;
    static bool m_enableStatus;
    //Timer related variables and functions
    TpTimer m_locateCastTimer;
    const void InitializeIARM();
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
    static bool checkRFCServiceStatus();
    static void powerModeChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
    static void threadPowerModeChangeEvent(void);
};
} // namespace Plugin
} // namespace WPEFramework
