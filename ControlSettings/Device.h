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

class Device  {
    private:
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

    public:
        Device();
        ~Device();
        void getAspectRatio();
        void setAspectRatio();
	virtual void getBacklight();
        virtual void setBacklight();
        virtual void Initialize();
        virtual void DeInitialize();

    protected:

       std::string getErrorString (tvError_t eReturn);
};
}
}
#endif
