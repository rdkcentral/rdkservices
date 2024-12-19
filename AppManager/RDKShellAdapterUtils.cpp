/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
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


#include "RDKShellAdapterUtils.h"

using namespace std;

namespace WPEFramework {
    namespace Plugin {
    namespace AppManagerV1 {

        uint32_t cloneService(PluginHost::IShell* shell, const string& basecallsign, const string& newcallsign)
        {
            uint32_t result = Core::ERROR_ASYNC_FAILED;
            Core::Event event(false, true);
#ifndef USE_THUNDER_R4
            Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatchType<void>>(Core::ProxyType<Job>::Create([&]() {
#else
            Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create([&]() {
#endif /* USE_THUNDER_R4 */
                auto interface = shell->QueryInterfaceByCallsign<PluginHost::IController>("");
                if (interface == nullptr) {
                    result = Core::ERROR_UNAVAILABLE;
                    std::cout << "no IController" << std::endl;
                } else {
                    result = interface->Clone(basecallsign, newcallsign);
                    std::cout << "IController clone status " << result << std::endl;
                    interface->Release();
                }
                event.SetEvent();
            })));
            event.Lock();
            return result;
        }

        uint32_t getConfig(PluginHost::IShell* shell, const string& callsign, string& config)
        {
            uint32_t result;
            auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
            if (interface == nullptr) {
                result = Core::ERROR_UNAVAILABLE;
                std::cout << "no IShell for " << callsign << std::endl;
            } else {
                result = Core::ERROR_NONE;
                config = interface->ConfigLine();
                std::cout << "IShell config " << config << " for " << callsign << std::endl;
                interface->Release();
            }
            return result;
        }

        uint32_t setConfig(PluginHost::IShell* shell, const string& callsign, const string& config)
        {
            uint32_t result;
            auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
            if (interface == nullptr) {
                result = Core::ERROR_UNAVAILABLE;
                std::cout << "no IShell for " << callsign << std::endl;
            } else {
                result = interface->ConfigLine(config);
                std::cout << "IShell config status " << result << " for " << callsign << std::endl;
                interface->Release();
            }
            return result;
        }

        uint32_t getServiceState(PluginHost::IShell* shell, const string& callsign, PluginHost::IShell::state& state)
        {
            uint32_t result;
            auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
            if (interface == nullptr) {
                result = Core::ERROR_UNAVAILABLE;
                std::cout << "no IShell for " << callsign << std::endl;
            } else {
                result = Core::ERROR_NONE;
                state = interface->State();
                std::cout << "IShell state " << state << " for " << callsign << std::endl;
                interface->Release();
            }
            return result;
        }

        uint32_t activate(PluginHost::IShell* shell, const string& callsign)
        {
            uint32_t result = Core::ERROR_ASYNC_FAILED;
            Core::Event event(false, true);
#ifndef USE_THUNDER_R4
            Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatchType<void>>(Core::ProxyType<Job>::Create([&]() {
#else
            Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create([&]() {
#endif /* USE_THUNDER_R4 */
                auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                if (interface == nullptr) {
                    result = Core::ERROR_UNAVAILABLE;
                    std::cout << "no IShell for " << callsign << std::endl;
                } else {
                    result = interface->Activate(PluginHost::IShell::reason::REQUESTED);
                    std::cout << "IShell activate status " << result << " for " << callsign << std::endl;
                    interface->Release();
                }
                event.SetEvent();
            })));
            event.Lock();
            return result;
        }

        uint32_t deactivate(PluginHost::IShell* shell, const string& callsign)
        {
            uint32_t result = Core::ERROR_ASYNC_FAILED;
            Core::Event event(false, true);
#ifndef USE_THUNDER_R4
            Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatchType<void>>(Core::ProxyType<Job>::Create([&]() {
#else
            Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create([&]() {
#endif /* USE_THUNDER_R4 */
                auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                if (interface == nullptr) {
                    result = Core::ERROR_UNAVAILABLE;
                    std::cout << "no IShell for " << callsign << std::endl;
                } else {
                    result = interface->Deactivate(PluginHost::IShell::reason::REQUESTED);
                    std::cout << "IShell deactivate status " << result << " for " << callsign << std::endl;
                    interface->Release();
                }
                event.SetEvent();
            })));
            event.Lock();
            return result;
        }

        Job::Job(std::function<void()> work): _work(work)
	{
	}

	void Job::Dispatch()
	{
            _work();
        }	

        } // namespace AppManager
    } // namespace Plugin
} // namespace WPEFramework
