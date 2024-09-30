/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2024 RDK Management
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

#pragma once

#include "Module.h"
#include <interfaces/Ids.h>
#include <interfaces/ISystemMode.h>
#include <interfaces/IDeviceOptimizeStateActivator.h>
#include "tracing/Logging.h"
#include <vector>
#include "UtilsString.h"

#include <com/com.h>
#include <core/core.h>
#include <plugins/plugins.h>

#define SYSTEM_MODE_COUNT 1 //Number of system mode define in Exchange::ISystemMode::SystemMode enum
#define SYSTEM_MODE_FILE "/tmp/SystemMode.txt"

namespace WPEFramework {
namespace Plugin {
    class SystemModeImplementation : public Exchange::ISystemMode{

    public:
        // We do not allow this plugin to be copied !!
        SystemModeImplementation();
        ~SystemModeImplementation() override;

        static SystemModeImplementation* instance(SystemModeImplementation *SystemModeImpl = nullptr);

        // We do not allow this plugin to be copied !!
        SystemModeImplementation(const SystemModeImplementation&) = delete;
        SystemModeImplementation& operator=(const SystemModeImplementation&) = delete;

        BEGIN_INTERFACE_MAP(SystemModeImplementation)
        INTERFACE_ENTRY(Exchange::ISystemMode)
        END_INTERFACE_MAP

    public:
	Core::hresult RequestState(const SystemMode systemMode, const State state ) override;
	Core::hresult GetState(const SystemMode systemMode , GetStateResult& successResult)const override;
	
	virtual uint32_t ClientActivated(const string& callsign ,const string& systemMode) override ;
	virtual uint32_t ClientDeactivated(const string& callsign, const string& systemMode) override ;

    private:
        mutable Core::CriticalSection _adminLock;
	std::map<const string, Exchange::IDeviceOptimizeStateActivator*> _clients;
	Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> _engine;
        Core::ProxyType<RPC::CommunicatorClient> _communicatorClient;
        PluginHost::IShell *_controller;
	std::map<Exchange::ISystemMode::SystemMode, std::string > SystemModeMap;
	std::map<Exchange::ISystemMode::State, std::string > deviceOptimizeStateMap;
	std::map<std::string , Exchange::ISystemMode::SystemMode> SystemModeInterfaceMap;
	bool stateRequested ;

    };
} // namespace Plugin
} // namespace WPEFramework
