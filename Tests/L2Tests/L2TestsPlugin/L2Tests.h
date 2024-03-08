/* If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2023 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

#pragma once

#include "Module.h"
#include <core/core.h>

#include <gtest/gtest.h>

namespace WPEFramework {
namespace Plugin {
    class L2Tests : public PluginHost::IPlugin, public PluginHost::JSONRPC { 

    public:
        BEGIN_INTERFACE_MAP(L2Tests)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        END_INTERFACE_MAP
        L2Tests();
        ~L2Tests() override;

        // IPlugin methods
        virtual const string Initialize(PluginHost::IShell* service) override;
        virtual void Deinitialize(PluginHost::IShell* service) override;
        virtual string Information() const override { return {}; }


    private:

          L2Tests(const L2Tests&) = delete;
          L2Tests& operator=(const L2Tests&) = delete;
        // JsonRpc methods
        uint32_t PerformL2Tests(const JsonObject& parameters, JsonObject& response);
    };
  } //namespace Plugin
} //namespace WPEFramework
