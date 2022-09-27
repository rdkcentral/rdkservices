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

#include <interfaces/json/JsonData_LinearPlaybackControl.h>
#include "Module.h"
#include "DemuxerStreamFsFCC.h"

namespace WPEFramework {
namespace Plugin {

class LinearConfig {
public:
    enum DemuxerType {
        DEMUX_NOKIA_FCC,
        DEMUX_QAM
    };

    class Config : public WPEFramework::Core::JSON::Container {
    private:
        Config(const Config &) = delete;

        Config &operator=(const Config &) = delete;

    public:
        Config()
                : MountPoint() {
            Add(_T("mountpoint"), &MountPoint);
            Add(_T("streamfs_enabled"), &IsStreamFSEnabled);
        }

        ~Config() {
        }

    public:
        WPEFramework::Core::JSON::String MountPoint;
        WPEFramework::Core::JSON::Boolean IsStreamFSEnabled;
    };
};

} //namespace LinearConfig
} //namespace Plugin
