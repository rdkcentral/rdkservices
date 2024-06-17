#pragma once

#include <string>
#include <atomic>

#include "Module.h"
#include "core/Link.h"
#include "NetworkManagerTimer.h"

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
        class Network : public PluginHost::IPlugin, public PluginHost::JSONRPC
        {
        private:
            // We do not allow this plugin to be copied !!
            Network(const Network&) = delete;
            Network& operator=(const Network&) = delete;

            void RegisterLegacyMethods();
            void UnregisterLegacyMethods();
            void subscribeToEvents(void);
            static std::string getInterfaceMapping(const std::string &interface);
            void activatePrimaryPlugin();

            /* Methods */
            uint32_t getInterfaces(const JsonObject& parameters, JsonObject& response);
            uint32_t isInterfaceEnabled(const JsonObject& parameters, JsonObject& response);
            uint32_t setInterfaceEnabled(const JsonObject& parameters, JsonObject& response);
            uint32_t getDefaultInterface(const JsonObject& parameters, JsonObject& response);
            uint32_t setDefaultInterface(const JsonObject& parameters, JsonObject& response);
            uint32_t doTrace(const JsonObject& parameters, JsonObject& response);
            uint32_t doPing(const JsonObject& parameters, JsonObject& response);
            uint32_t setIPSettings(const JsonObject& parameters, JsonObject& response);
            uint32_t getIPSettings(const JsonObject& parameters, JsonObject& response);
            uint32_t isConnectedToInternet(const JsonObject& parameters, JsonObject& response);
            uint32_t setConnectivityTestEndpoints(const JsonObject& parameters, JsonObject& response);
            uint32_t getInternetConnectionState(const JsonObject& parameters, JsonObject& response);
            uint32_t startConnectivityMonitoring(const JsonObject& parameters, JsonObject& response);
            uint32_t getCaptivePortalURI(const JsonObject& parameters, JsonObject& response);
            uint32_t stopConnectivityMonitoring(const JsonObject& parameters, JsonObject& response);
            uint32_t getPublicIP(const JsonObject& parameters, JsonObject& response);
            uint32_t setStunEndPoint(const JsonObject& parameters, JsonObject& response);
            uint32_t getStbIp(const JsonObject& parameters, JsonObject& response);
            uint32_t getSTBIPFamily(const JsonObject& parameters, JsonObject& response);

            /* Events */
            static void onInterfaceStateChange(const JsonObject& parameters);
            static void onActiveInterfaceChange(const JsonObject& parameters);
            static void onIPAddressChange(const JsonObject& parameters);
            static void onInternetStatusChange(const JsonObject& parameters);

        public:
            Network();
            ~Network();
            //Build QueryInterface implementation, specifying all possible interfaces to be returned.
            BEGIN_INTERFACE_MAP(Network)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP
            
            void ReportonInterfaceStateChange(const JsonObject& parameters);
            void ReportonActiveInterfaceChange(const JsonObject& parameters);
            void ReportonIPAddressChange(const JsonObject& parameters);
            void ReportonInternetStatusChange(const JsonObject& parameters);

            //IPlugin methods
            virtual const std::string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual std::string Information() const override;

        private:
            PluginHost::IShell* m_service;
            std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> m_networkmanager;
            string m_defaultInterface;
            NetworkManagerTimer m_timer;

            bool m_subsIfaceStateChange;
            bool m_subsActIfaceChange;
            bool m_subsIPAddrChange;
            bool m_subsInternetChange;
        };
    } // namespace Plugin
} // namespace WPEFramework
