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

#ifndef ControlSettingsSTB_H
#define ControlSettingsSTB_H

#include "string.h"
#include "ControlSettingsDevice.h"

namespace WPEFramework {
namespace Plugin {

class ControlSettingsSTB : public ControlSettingsDevice {
    private:
        ControlSettingsSTB(const ControlSettingsSTB&) = delete;
        ControlSettingsSTB& operator=(const ControlSettingsSTB&) = delete;

    public:
	ControlSettingsSTB();
	~ControlSettingsSTB();
        void Initialize();
        void DeInitialize();
        tvError_t getVolume();
        tvError_t setVolume();
        bool isDisplayAvailable() { return false; }
};

}//namespace Plugin
}//namespace WPEFramework
#endif
