/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2024 Sky UK
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
#include "AVOutputBase.h"
#include "UtilsIarm.h"

const char* PLUGIN_IARM_BUS_NAME = "Thunder_Plugins";

namespace WPEFramework {
namespace Plugin {

    AVOutputBase::AVOutputBase()
               : _skipURL(0)
    {
        LOGINFO("CTOR\n");
    }

    AVOutputBase::~AVOutputBase()
    {
    }

    void AVOutputBase::Initialize()
    {
	LOGINFO("AVOutputBase Initialize\n");
        
    }

    void AVOutputBase::Deinitialize()
    {
	LOGINFO("AVOutputBase Deinitialize\n");
    }
    void AVOutputBase::InitializeIARM()
    {
        LOGINFO("AVOutputBase InitializeIARM \n");
    }

    void AVOutputBase::DeinitializeIARM()
    {
        LOGINFO("AVOutputBase De-InitializeIARM \n");
    }

} //namespace WPEFramework

} //namespace Plugin
