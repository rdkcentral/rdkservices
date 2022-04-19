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
 
#include "LinearPlaybackControl.h"
#include "DemuxerStreamFsFCC.h"
#include "LinearConfig.h"
#define SYSTEM_SERVICES_CALLSIGN 	"SystemServices"

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(LinearPlaybackControl, 1, 0);

    /* virtual */ const string LinearPlaybackControl::Initialize(PluginHost::IShell* service)
    {
        ASSERT(_service == nullptr);

        LinearConfig::Config config;
        config.FromString(service->ConfigLine());
        _mountPoint = config.MountPoint.Value();
        _isStreamFSEnabled = config.IsStreamFSEnabled.Value();

        if (_isStreamFSEnabled) {
            // For now we only have one Nokia FCC demuxer interface with demux Id = 0
            _demuxer = std::unique_ptr<DemuxerStreamFsFCC>(new DemuxerStreamFsFCC(&config, 0));
            _trickPlayFileListener = std::unique_ptr<FileSelectListener>(new FileSelectListener(_demuxer->getTrickPlayFile(), 16,[this](const std::string &data){
                speedchangedNotify(data);
            }));
        }

        _service = service;
        _service->Register(&_notification);

        // Initialize streamfs and associated dependencies here.

        // No additional info to report in this initial/empty implementation.
        return string();
    }

    /* virtual */ void LinearPlaybackControl::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        _service->Unregister(&_notification);
       // Deinitialize streamfs and associated dependencies here.

        _service = nullptr;
    }

    void LinearPlaybackControl::StateChange(PluginHost::IShell* plugin) {
        const string callsign(plugin->Callsign());

        if (callsign == SYSTEM_SERVICES_CALLSIGN || callsign == _service->Callsign()) {
            Exchange::ISystemServices* handler(_service->QueryInterfaceByCallsign<Exchange::ISystemServices>(SYSTEM_SERVICES_CALLSIGN));
            if (plugin->State() == PluginHost::IShell::ACTIVATION && handler != nullptr) {
                handler->Register(&_notification);
            }
            else if (plugin->State() == PluginHost::IShell::DEACTIVATION && handler != nullptr) {
                handler->Unregister(&_notification);
            }
        }
    }

    void LinearPlaybackControl::StateChange(Exchange::ISystemServices::SystemServicesState state, int left_time) {
        // It is notification callback. It shouldn't blocked
        if (state == Exchange::ISystemServices::SystemServicesState::DEEP_SLEEP) {
            _job.Submit();
        }
        else if (state == Exchange::ISystemServices::SystemServicesState::ON) {
            if (_isStreamFSEnabled) {
                // For now we only have one Nokia FCC demuxer interface with demux Id = 0
                _trickPlayFileListener = std::unique_ptr<FileSelectListener>(new FileSelectListener(_demuxer->getTrickPlayFile(), 16,[this](const std::string &data){
                    speedchangedNotify(data);
                }));
            }
        }
    }

    /* virtual */ string LinearPlaybackControl::Information() const
    {
        // No additional info to report in this initial/empty implementation.
        return (string());
    }

} // namespace Plugin
} // namespace WPEFramework
