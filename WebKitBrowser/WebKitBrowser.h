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

#ifndef __BROWSER_H
#define __BROWSER_H

#include "Module.h"
#include <interfaces/IBrowser.h>
#include <interfaces/IApplication.h>
#include <interfaces/IMemory.h>
#include <interfaces/json/JsonData_Browser.h>
#include <interfaces/json/JsonData_WebKitBrowser.h>
#include <interfaces/json/JsonData_StateControl.h>
#include <interfaces/json/JWebBrowser.h>
#include <interfaces/json/JBrowserResources.h>
#include <interfaces/json/JBrowserSecurity.h>

namespace WPEFramework {

namespace WebKitBrowser {
    // An implementation file needs to implement this method to return an operational browser, wherever that would be :-)
    Exchange::IMemory* MemoryObserver(const RPC::IRemoteConnection* connection);
}

namespace Plugin {

    class WebKitBrowser : public PluginHost::IPlugin, public PluginHost::IWeb, public PluginHost::JSONRPC {
    private:
        WebKitBrowser(const WebKitBrowser&) = delete;
        WebKitBrowser& operator=(const WebKitBrowser&) = delete;

        class Notification : public RPC::IRemoteConnection::INotification,
                             public PluginHost::IStateControl::INotification,
                             public Exchange::IWebBrowser::INotification {
        private:
            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

        public:
            explicit Notification(WebKitBrowser* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }
            ~Notification() override
            {
            }

        public:
            void LoadFinished(const string& URL, int32_t code) override
            {
                _parent.LoadFinished(URL, code);
            }
            void LoadFailed(const string& URL) override
            {
                _parent.LoadFailed(URL);
            }
            void URLChange(const string& URL, bool loaded) override
            {
                _parent.URLChange(URL, loaded);
            }
            void VisibilityChange(const bool hidden) override
            {
                _parent.VisibilityChange(hidden);
            }
            void PageClosure() override
            {
                _parent.PageClosure();
            }
            void BridgeQuery(const string& message) override
            {
                _parent.BridgeQuery(message);
            }
            void StateChange(const PluginHost::IStateControl::state state) override
            {
                _parent.StateChange(state);
            }
            void Activated(RPC::IRemoteConnection* /* connection */) override
            {
            }
            void Deactivated(RPC::IRemoteConnection* connection) override
            {
                _parent.Deactivated(connection);
            }

            BEGIN_INTERFACE_MAP(Notification)
            INTERFACE_ENTRY(Exchange::IWebBrowser::INotification)
            INTERFACE_ENTRY(PluginHost::IStateControl::INotification)
            INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

        private:
            WebKitBrowser& _parent;
        };

    public:
        class Data : public Core::JSON::Container {
        private:
            Data(const Data&) = delete;
            Data& operator=(const Data&) = delete;

        public:
            Data()
                : Core::JSON::Container()
                , URL()
                , FPS()
                , Suspended(false)
                , Hidden(false)
                , Path()
            {
                Add(_T("url"), &URL);
                Add(_T("fps"), &FPS);
                Add(_T("suspended"), &Suspended);
                Add(_T("hidden"), &Hidden);
                Add(_T("path"), &Path);
            }
            ~Data()
            {
            }

        public:
            Core::JSON::String URL;
            Core::JSON::DecUInt32 FPS;
            Core::JSON::Boolean Suspended;
            Core::JSON::Boolean Hidden;
            Core::JSON::String Path;
        };

    public:
        WebKitBrowser()
            : _skipURL(0)
            , _service(nullptr)
            , _browser(nullptr)
            , _browserResources(nullptr)
            , _browserSecurity(nullptr)
            , _memory(nullptr)
            , _application(nullptr)
            , _notification(this)
            , _jsonBodyDataFactory(2)
        {
        }

        ~WebKitBrowser() override
        {
        }

        inline static bool EnvironmentOverride(const bool configFlag)
        {
            bool result = configFlag;

            if (result == false) {
                string value;
                Core::SystemInfo::GetEnvironment(_T("WPE_ENVIRONMENT_OVERRIDE"), value);
                result = (value == "1");
            }
            return (result);
        }

    public:
        BEGIN_INTERFACE_MAP(WebKitBrowser)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IWeb)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(PluginHost::IStateControl, _browser)
        INTERFACE_AGGREGATE(Exchange::IBrowser, _browser)
        INTERFACE_AGGREGATE(Exchange::IApplication, _application)
        INTERFACE_AGGREGATE(Exchange::IWebBrowser, _browser)
        INTERFACE_AGGREGATE(Exchange::IBrowserResources, _browser)
        INTERFACE_AGGREGATE(Exchange::IBrowserSecurity, _browser)
        INTERFACE_AGGREGATE(Exchange::IMemory, _memory)
        END_INTERFACE_MAP

    public:
        //  IPlugin methods
        // -------------------------------------------------------------------------------------------------------

        // First time initialization. Whenever a plugin is loaded, it is offered a Service object with relevant
        // information and services for this particular plugin. The Service object contains configuration information that
        // can be used to initialize the plugin correctly. If Initialization succeeds, return nothing (empty string)
        // If there is an error, return a string describing the issue why the initialisation failed.
        // The Service object is *NOT* reference counted, lifetime ends if the plugin is deactivated.
        // The lifetime of the Service object is guaranteed till the deinitialize method is called.
        const string Initialize(PluginHost::IShell* service) override;

        // The plugin is unloaded from WPEFramework. This is call allows the module to notify clients
        // or to persist information if needed. After this call the plugin will unlink from the service path
        // and be deactivated. The Service object is the same as passed in during the Initialize.
        // After theis call, the lifetime of the Service object ends.
        void Deinitialize(PluginHost::IShell* service) override;

        // Returns an interface to a JSON struct that can be used to return specific metadata information with respect
        // to this plugin. This Metadata can be used by the MetData plugin to publish this information to the ouside world.
        string Information() const override;

        //  IWeb methods
        // -------------------------------------------------------------------------------------------------------
        void Inbound(Web::Request& request) override;
        Core::ProxyType<Web::Response> Process(const Web::Request& request) override;

    private:
        void Deactivated(RPC::IRemoteConnection* connection);
        void LoadFinished(const string& URL, int32_t code);
        void LoadFailed(const string& URL);
        void URLChange(const string& URL, bool loaded);
        void VisibilityChange(const bool hidden);
        void PageClosure();
        void BridgeQuery(const string& message);
        void StateChange(const PluginHost::IStateControl::state state);
        uint32_t DeleteDir(const string& path);

        // JsonRpc
        void RegisterAll();
        void UnregisterAll();
        uint32_t get_state(Core::JSON::EnumType<JsonData::StateControl::StateType>& response) const; // StateControl
        uint32_t set_state(const Core::JSON::EnumType<JsonData::StateControl::StateType>& param); // StateControl
        uint32_t endpoint_delete(const JsonData::Browser::DeleteParamsData& params);
        uint32_t get_languages(Core::JSON::ArrayType<Core::JSON::String>& response) const;
        uint32_t set_languages(const Core::JSON::ArrayType<Core::JSON::String>& param);
        uint32_t get_headers(Core::JSON::ArrayType<JsonData::WebKitBrowser::HeadersData>& response) const;
        uint32_t set_headers(const Core::JSON::ArrayType<JsonData::WebKitBrowser::HeadersData>& param);
        void event_bridgequery(const string& message);
        void event_statechange(const bool& suspended); // StateControl

    private:
        uint8_t _skipURL;
        uint32_t _connectionId;
        PluginHost::IShell* _service;
        Exchange::IWebBrowser* _browser;
        Exchange::IBrowserResources* _browserResources;
        Exchange::IBrowserSecurity* _browserSecurity;
        Exchange::IMemory* _memory;
        Exchange::IApplication* _application;
        Core::Sink<Notification> _notification;
        Core::ProxyPoolType<Web::JSONBodyType<WebKitBrowser::Data>> _jsonBodyDataFactory;
        string _persistentStoragePath;
    };
}
}

#endif // __BROWSER_H
