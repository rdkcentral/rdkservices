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

#ifndef DEVICE_H
#define DEVICE_H

#include "string.h"
#include <pthread.h>
#include "Module.h"
#include "utils.h"
#include "tvTypes.h"
#include "tvError.h"
#include "tvLog.h"
#include "tvSettings.h"

namespace WPEFramework {
namespace Plugin {

class ControlSettingsDevice  {
    private:
        ControlSettingsDevice(const ControlSettingsDevice&) = delete;
        ControlSettingsDevice& operator=(const ControlSettingsDevice&) = delete;

    public:
        ControlSettingsDevice();
        ~ControlSettingsDevice();
        tvError_t getAspectRatio();
        tvError_t setAspectRatio();
        virtual void Initialize();
        virtual void DeInitialize();
	virtual bool isDisplayAvailable() { return false; }
	virtual tvError_t setBacklight();
	virtual tvError_t getBacklight();
	virtual tvError_t getVolume();
	virtual tvError_t setVolume();

    protected:

       std::string getErrorString (tvError_t eReturn);
};
}
}
#endif
