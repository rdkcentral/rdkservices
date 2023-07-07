/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright Synamedia, All rights reserved
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
**/

#include "UnifiedCASManagement.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {

    namespace Plugin {

        UnifiedCASManagement* UnifiedCASManagement::_instance;

        UnifiedCASManagement::UnifiedCASManagement() {
            _instance = this;
            m_RTPlayer = std::make_shared<RTPlayer>(this);
	    m_sessionId = UNDEFINED_SESSION_ID;
            RegisterAll();
        }

        UnifiedCASManagement::~UnifiedCASManagement() {
            UnregisterAll();
            UnifiedCASManagement::_instance = nullptr;
        }

        const string UnifiedCASManagement::Initialize(PluginHost::IShell * /* service */)
        {
           // AVInput::_instance = this;
           // InitializeIARM();

            return (string());
        }

        void UnifiedCASManagement::Deinitialize(PluginHost::IShell * /* service */)
        {
           // DeinitializeIARM();
            //AVInput::_instance = nullptr;
            UnifiedCASManagement::_instance = nullptr;
        }

        string UnifiedCASManagement::Information() const
        {
            return (string());
        }
    } // namespace
} // namespace
