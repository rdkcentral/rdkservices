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
#include <interfaces/IDisplayInfo.h>
#include <interfaces/json/JsonData_DisplayInfo.h>

namespace WPEFramework {
namespace Plugin {

    class DisplayInfo : public PluginHost::IPlugin, public PluginHost::IWeb, public PluginHost::JSONRPC {
    private:private:
        class Notification : protected Exchange::IConnectionProperties::INotification, Exchange::IDisplayProperties::INotification {
        private:
            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

        public:
            explicit Notification(DisplayInfo* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }
            virtual ~Notification()
            {
            }

            void Initialize(Exchange::IConnectionProperties* client)
            {
                ASSERT(client != nullptr);
                _client = client;
                _client->AddRef();
                _client->Register(this);
            }

            void Initialize(Exchange::IDisplayProperties* client)
            {
                ASSERT(client != nullptr);
                _clientDisplay = client;
                _clientDisplay->AddRef();
                _clientDisplay->Register(this);
            }

            void Deinitialize()
            {
                ASSERT(_client != nullptr);
                if (_client != nullptr) {
                    _client->Unregister(this);
                    _client->Release();
                    _client = nullptr;
                }

                ASSERT(_clientDisplay != nullptr);
                if (_clientDisplay != nullptr) {
                    _clientDisplay->Unregister(this);
                    _clientDisplay->Release();
                    _clientDisplay = nullptr;
                }
            }
            void Updated() override
            {
                _parent.Updated();
            }

            void ResolutionPreChange()
            {
                _parent.resolutionPreChanged();
            }

            void ResolutionChanged(const int32_t& height, const int32_t& width, const string& videoOutputPort, const string& resolution) override
            {
                _parent.resolutionchanged(height, width, videoOutputPort, resolution);
            }

            BEGIN_INTERFACE_MAP(Notification)
            INTERFACE_ENTRY(Exchange::IConnectionProperties::INotification)
            INTERFACE_ENTRY(Exchange::IDisplayProperties::INotification)
            END_INTERFACE_MAP

        private:
            DisplayInfo& _parent;
            Exchange::IConnectionProperties* _client;
            Exchange::IDisplayProperties* _clientDisplay;
        };

    public:
        DisplayInfo(const DisplayInfo&) = delete;
        DisplayInfo& operator=(const DisplayInfo&) = delete;

        DisplayInfo()
            : _skipURL(0)
            , _connectionId(0)
            , _graphicsProperties(nullptr)
            , _connectionProperties(nullptr)
            , _displayProperties(nullptr)
            , _notification(this)
        {
            RegisterAll();
        }

        virtual ~DisplayInfo()
        {
            UnregisterAll();
        }

        BEGIN_INTERFACE_MAP(DisplayInfo)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IWeb)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        END_INTERFACE_MAP

    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        virtual const string Initialize(PluginHost::IShell* service) override;
        virtual void Deinitialize(PluginHost::IShell* service) override;
        virtual string Information() const override;

        //   IWeb methods
        // -------------------------------------------------------------------------------------------------------
        virtual void Inbound(Web::Request& request) override;
        virtual Core::ProxyType<Web::Response> Process(const Web::Request& request) override;

        void Updated()
        {
            event_updated();
        }

        void resolutionPreChanged()
        {
            resolutionPreChangedJrpc();
        }

        void resolutionchanged(const int32_t& height, const int32_t& width, const string& videoOutputPort, const string& resolution)
        {
            resolutionChangedJrpc(height, width, videoOutputPort, resolution);
        }

    private:
        // JsonRpc
        void RegisterAll();
        void UnregisterAll();
        uint32_t get_displayinfo(JsonData::DisplayInfo::DisplayinfoData&) const;
        uint32_t getCurrentResolutionJrpc(JsonData::DisplayInfo::VideoPlaybackResolutionData&) const;
        uint32_t getConnectedVideoDisplayJrpc(JsonData::DisplayInfo::VideoOutputPortNameData&) const;
        uint32_t getTVHDRCapabilitiesJrpc(JsonData::DisplayInfo::TvHdrCapabilitesData&) const;
        uint32_t getSettopHDRSupportJrpc(JsonData::DisplayInfo::StbHdrCapabilitiesData&) const;
        uint32_t IsOutputHDRJrpc(JsonData::DisplayInfo::OutputHdrStatusData&) const;
        uint32_t getHdmiPreferencesJrpc(JsonData::DisplayInfo::HdmiPreferencesData&) const;
        uint32_t setHdmiPreferencesJrpc(const JsonData::DisplayInfo::HdmiPreferencesData&);
        uint32_t isAudioEquivalenceEnabledJrpc(JsonData::DisplayInfo::AudioEquivalenceStatusData&) const;
        uint32_t readEDIDJrpc(JsonData::DisplayInfo::EDIDData&) const;

        void event_updated();
        void resolutionPreChangedJrpc();
        void resolutionChangedJrpc(const int32_t& height, const int32_t& width, const string& videoOutputPort, const string& resolution);

        void Info(JsonData::DisplayInfo::DisplayinfoData&) const;
        void getCurrentResolution(JsonData::DisplayInfo::VideoPlaybackResolutionData&) const;
        void getConnectedVideoDisplay(JsonData::DisplayInfo::VideoOutputPortNameData&) const;
        void getTVHDRCapabilities(JsonData::DisplayInfo::TvHdrCapabilitesData&) const;
        void getSettopHDRSupport(JsonData::DisplayInfo::StbHdrCapabilitiesData&) const;
        void IsOutputHDR(JsonData::DisplayInfo::OutputHdrStatusData&) const;
        void getHdmiPreferences(JsonData::DisplayInfo::HdmiPreferencesData&) const;
        void setHdmiPreferences(const JsonData::DisplayInfo::HdmiPreferencesData&);
        void isAudioEquivalenceEnabled(JsonData::DisplayInfo::AudioEquivalenceStatusData &) const;
        void readEDID(JsonData::DisplayInfo::EDIDData &) const;

    private:
        uint8_t _skipURL;
        uint32_t _connectionId;
        Exchange::IGraphicsProperties* _graphicsProperties;
        Exchange::IConnectionProperties* _connectionProperties;
        Exchange::IDisplayProperties* _displayProperties;
        Core::Sink<Notification> _notification;
    };

} // namespace Plugin
} // namespace WPEFramework
