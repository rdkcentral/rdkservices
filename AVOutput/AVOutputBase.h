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

#ifndef AVOUTPUTBASE_H
#define AVOUTPUTBASE_H

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

#define DECLARE_JSON_RPC_METHOD(method) \
	uint32_t method(const JsonObject& parameters, JsonObject& response);

namespace WPEFramework {
namespace Plugin {

    class AVOutputBase : public PluginHost::IPlugin, public PluginHost::JSONRPC {

    private:
        AVOutputBase(const AVOutputBase&) = delete;
        AVOutputBase& operator=(const AVOutputBase&) = delete;

   public:
        AVOutputBase();
        ~AVOutputBase();
    public:
        uint8_t _skipURL;
        bool isIARMConnected();
        bool IARMinit();
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        virtual void Initialize();
        virtual void Deinitialize();
        virtual void dsHdmiVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
        virtual void dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
        virtual void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
	virtual void InitializeIARM();
        virtual void DeinitializeIARM();
   };
}
}
#endif
