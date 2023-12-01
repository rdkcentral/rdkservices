/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2022 Sky UK
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

#include <string>
#include "AVOutput.h"
#include "UtilsIarm.h"

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(AVOutput,1, 0);

    AVOutput::AVOutput()
    {
        LOGINFO("Entry\n");

        //Common API Registration
        LOGINFO("Exit \n");
    }

    AVOutput::~AVOutput()
    {
        LOGINFO();
    }

    const std::string AVOutput::Initialize(PluginHost::IShell* service)
    {
	LOGINFO("Entry\n");
        
	ASSERT(service != nullptr);
        _skipURL = static_cast<uint8_t>(service->WebPrefix().length());

	DEVICE_TYPE::Initialize();

	LOGINFO("Exit\n");
        return (service != nullptr ? _T("") : _T("No service."));
    }

    void AVOutput::Deinitialize(PluginHost::IShell* service)
    {
        LOGINFO();

	DEVICE_TYPE::Deinitialize();
    }

} //namespace WPEFramework

} //namespace Plugin
