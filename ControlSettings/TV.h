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

#ifndef TV_H
#define TV_H
#include "string.h"
#include "Device.h"

namespace WPEFramework {
namespace Plugin {

class TV : public Device {
    private:
        TV(const TV&) = delete;
        TV& operator=(const TV&) = delete;

    public:
        TV();
        ~TV();
        void getBacklight();
        void setBacklight();
        void Initialize();
        void DeInitialize();
	bool isTvSupportEnabled() { return true; }
};

}//namespace Plugin
}//namespace WPEFramework
#endif
