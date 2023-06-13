/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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
#include "DemuxerStreamFsFCC.h"
#include "IDemuxer.h"
#include "DemuxerStreamFsFCC.h"
#include "FileSelectListener.h"
#include <interfaces/json/JsonData_LinearPlaybackControl.h>

namespace WPEFramework {
namespace Plugin {

    using endpoint_func = uint32_t(IDemuxer*);

    class LinearPlaybackControl
        : public PluginHost::IPlugin
        , public PluginHost::JSONRPC {

    public:
        LinearPlaybackControl(const LinearPlaybackControl&) = delete;
        LinearPlaybackControl& operator=(const LinearPlaybackControl&) = delete;

#ifdef __WINDOWS__
#pragma warning(disable : 4355)
#endif
        LinearPlaybackControl()
            : _skipURL(0)
            , _service(nullptr)
        {
            RegisterAll();
        }
#ifdef __WINDOWS__
#pragma warning(default : 4355)
#endif
        virtual ~LinearPlaybackControl()
        {
            UnregisterAll();
        }

    public:
        BEGIN_INTERFACE_MAP(LinearPlaybackControl)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
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

    private:
        void RegisterAll();
        void UnregisterAll();
        uint32_t endpoint_set_channel(const string& demuxerId, const JsonData::LinearPlaybackControl::ChannelData& params);
        uint32_t endpoint_get_channel(const string& demuxerId, JsonData::LinearPlaybackControl::ChannelData& params) const;
        uint32_t endpoint_set_seek(const string& demuxerId, const JsonData::LinearPlaybackControl::SeekData& params);
        uint32_t endpoint_get_seek(const string& demuxerId, JsonData::LinearPlaybackControl::SeekData& params) const;
        uint32_t endpoint_set_trickplay(const string& demuxerId, const JsonData::LinearPlaybackControl::TrickplayData& params);
        uint32_t endpoint_get_trickplay(const string& demuxerId, JsonData::LinearPlaybackControl::TrickplayData& params) const;
        uint32_t endpoint_get_status(const string& demuxerId, JsonData::LinearPlaybackControl::StatusData& params) const;
        uint32_t endpoint_set_tracing(const JsonData::LinearPlaybackControl::TracingData& params);
        uint32_t endpoint_get_tracing(JsonData::LinearPlaybackControl::TracingData& params) const;

        // Get the concrete demuxer implementation and invoke a lambda function for interacting with the actual interface.
        // 0-255 reserved for Nokia FCC
        // QAM range is TBD.
        uint32_t callDemuxer(const string& demuxerId, const std::function<endpoint_func>& func) const;

        void speedchangedNotify(const std::string &data);

    private:
        uint32_t _skipURL;
        PluginHost::IShell* _service;
        std::string _mountPoint;
        bool _isStreamFSEnabled;
        std::unique_ptr<DemuxerStreamFsFCC> _demuxer;
        std::unique_ptr<FileSelectListener> _trickPlayFileListener;
};
} //namespace Plugin
} //namespace WPEFramework
