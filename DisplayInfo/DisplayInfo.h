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
#include <interfaces/json/JsonData_DisplayInfo.h>
#include <interfaces/json/JGraphicsProperties.h>
#include <interfaces/json/JConnectionProperties.h>
#include <interfaces/json/JHDRProperties.h>
#include <interfaces/json/JDisplayProperties.h>
#include <interfaces/IConfiguration.h>

namespace WPEFramework {
namespace Plugin {

    class DisplayInfo : public PluginHost::IPlugin, public PluginHost::IWeb, public PluginHost::JSONRPC {
    private:
        class Notification : public Exchange::IConnectionProperties::INotification, public RPC::IRemoteConnection::INotification {
        public:
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

        public:
            explicit Notification(DisplayInfo* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }
            ~Notification() override = default;

            void Updated(const Exchange::IConnectionProperties::INotification::Source event) override
            {
                Exchange::JConnectionProperties::Event::Updated(_parent, event);
            }

            void Activated(RPC::IRemoteConnection* connection VARIABLE_IS_NOT_USED) override {
            }

            void Deactivated(RPC::IRemoteConnection* connection) override
            {
                _parent.Deactivated(connection);
            }

            BEGIN_INTERFACE_MAP(Notification)
            INTERFACE_ENTRY(Exchange::IConnectionProperties::INotification)
            INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

        private:
            DisplayInfo& _parent;
        };

    public:
        DisplayInfo(const DisplayInfo&) = delete;
        DisplayInfo& operator=(const DisplayInfo&) = delete;

        DisplayInfo()
            : _skipURL(0)
            , _connectionId(0)
            , _graphicsProperties(nullptr)
            , _connectionProperties(nullptr)
            , _hdrProperties(nullptr)
            , _displayProperties(nullptr)
            , _notification(this)
            , _service(nullptr)
        {
        }

        ~DisplayInfo() override = default;

        BEGIN_INTERFACE_MAP(DisplayInfo)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IWeb)
        INTERFACE_AGGREGATE(Exchange::IGraphicsProperties, _graphicsProperties)
        INTERFACE_AGGREGATE(Exchange::IConnectionProperties, _connectionProperties)
        INTERFACE_AGGREGATE(Exchange::IHDRProperties, _hdrProperties)
        INTERFACE_AGGREGATE(Exchange::IDisplayProperties, _displayProperties)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        END_INTERFACE_MAP

    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

        //   IWeb methods
        // -------------------------------------------------------------------------------------------------------
        void Inbound(Web::Request& request) override;
        Core::ProxyType<Web::Response> Process(const Web::Request& request) override;

    private:

        void Info(JsonData::DisplayInfo::DisplayinfoData&) const;
        void Deactivated(RPC::IRemoteConnection* connection);

    private:
        uint8_t _skipURL;
        uint32_t _connectionId;
        Exchange::IGraphicsProperties* _graphicsProperties;
        Exchange::IConnectionProperties* _connectionProperties;
        Exchange::IHDRProperties* _hdrProperties;
        Exchange::IDisplayProperties* _displayProperties;
        Core::Sink<Notification> _notification;
        PluginHost::IShell* _service;
    };

} // namespace Plugin
} // namespace WPEFramework
