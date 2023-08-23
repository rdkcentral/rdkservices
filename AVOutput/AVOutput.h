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

#ifndef AVOUTPUT_H
#define AVOUTPUT_H

#include <string>
#include "libIARM.h"
#include "libIBusDaemon.h"
#include "libIBus.h"
#include "iarmUtil.h"

#include <iostream>

#include <pthread.h>
#include "Module.h"

#include <sys/stat.h>
#include <vector>

//Default AVOutputSTB
#ifndef DEVICE_TYPE
#define DEVICE_TYPE AVOutputSTB
#include "AVOutputSTB.h"
#else
#include "AVOutputTV.h"
#endif

namespace WPEFramework {
namespace Plugin {

    class AVOutput : public DEVICE_TYPE {

    private:
        AVOutput(const AVOutput&) = delete;
        AVOutput& operator=(const AVOutput&) = delete;

   public:
        AVOutput();
        ~AVOutput();

    private:
        uint8_t _skipURL;
	void InitializeIARM();
        void DeinitializeIARM();
        bool isIARMConnected();
        bool IARMinit();

    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        const std::string Initialize(PluginHost::IShell* service);
        void Deinitialize(PluginHost::IShell* service);
        virtual string Information() const override { return {}; }
	virtual void AddRef() const { }
	virtual uint32_t Release() const {return 0; }
        BEGIN_INTERFACE_MAP(AVOutput)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        END_INTERFACE_MAP
   };
}
}
#endif
