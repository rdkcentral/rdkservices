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
#include "Device.h"

namespace WPEFramework {
namespace Plugin {

    Device::Device() 
    {
       LOGINFO(); 
    }
    
    void Device::Initialize()
    {
        LOGINFO();
    }
    void Device::DeInitialize()
    {
        LOGINFO();
    }

    void Device::getAspectRatio()
    {
        LOGINFO("Base Entry : %s\n",__FUNCTION__);
	LOGINFO("Base Exit  : %s\n",__FUNCTION__);
    }

    void Device::setAspectRatio()
    {
        LOGINFO("Base Entry : %s\n",__FUNCTION__);
        LOGINFO("Base Exit  : %s\n",__FUNCTION__);
    }

    void Device::getBacklight()
    {
        LOGINFO("Base Entry : %s\n",__FUNCTION__);
        LOGINFO("Base Exit  : %s\n",__FUNCTION__);
    }

    void Device::setBacklight()
    {
        LOGINFO("Base Entry : %s\n",__FUNCTION__);
        LOGINFO("Base Exit  : %s\n",__FUNCTION__);
    }

   }
}
