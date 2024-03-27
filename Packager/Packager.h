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

#pragma once

#include "Module.h"
#include <interfaces/IPackager.h>

namespace WPEFramework {
namespace Plugin {
namespace {
    constexpr auto* kInstallMethodName = _T("install");
    constexpr auto* kSynchronizeMethodName = _T("synchronize");
}

    class Packager : public PluginHost::IPlugin,
                     public PluginHost::IWeb,
                     public PluginHost::JSONRPC {
    public:
        struct Params : public Core::JSON::Container {
            Params& operator=(const Params& other) = delete;
            Params() {
                Add(_T("package"), &Package);
                Add(_T("architecture"), &Architecture);
                Add(_T("version"), &Version);
            }
            Params(const Params& other)
                : Package(other.Package)
                , Architecture(other.Architecture)
                , Version(other.Version) {
                Add(_T("package"), &Package);
                Add(_T("architecture"), &Architecture);
                Add(_T("version"), &Version);
            }
            Core::JSON::String Package;
            Core::JSON::String Architecture;
            Core::JSON::String Version;
        };

        // We do not allow this plugin to be copied !!
        Packager(const Packager&) = delete;
        Packager& operator=(const Packager&) = delete;

        Packager()
            : _skipURL(0)
            , _connectionId(0)
            , _service(nullptr)
            , _implementation(nullptr)
            , _notification(this)
        {
        }

        ~Packager() override = default;

        BEGIN_INTERFACE_MAP(Packager)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IWeb)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_AGGREGATE(Exchange::IPackager, _implementation)
        END_INTERFACE_MAP

        //   IPlugin methods
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

        //	IWeb methods
        void Inbound(Web::Request& request) override;
        Core::ProxyType<Web::Response> Process(const Web::Request& request) override;

    private:
        class Notification : public RPC::IRemoteConnection::INotification {
        public:
            explicit Notification(Packager* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }

            ~Notification() override = default;
            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

            void Activated(RPC::IRemoteConnection*) override
            {
            }

            void Deactivated(RPC::IRemoteConnection* connection) override
            {
                _parent.Deactivated(connection);
            }

            BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

        private:
            Packager& _parent;
        };

        void Deactivated(RPC::IRemoteConnection* connection);

        uint8_t _skipURL;
        uint32_t _connectionId;
        PluginHost::IShell* _service;
        Exchange::IPackager* _implementation;
        Core::Sink<Notification> _notification;
    };

}  // namespace Plugin
}  // namespace WPEFramework
