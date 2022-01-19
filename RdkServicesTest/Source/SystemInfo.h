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

#pragma once

#include "Module.h"

#include "Config.h"

namespace RdkServicesTest {

class SystemInfo: public WPEFramework::PluginHost::ISubSystem {
private:
    SystemInfo(const SystemInfo&) = delete;
    SystemInfo& operator=(const SystemInfo&) = delete;

public:
    SystemInfo()
            :_internet(nullptr), _location(nullptr) { }
    virtual ~SystemInfo() = default;

public:
    virtual void Register(WPEFramework::PluginHost::ISubSystem::INotification* notification) override { }
    virtual void Unregister(WPEFramework::PluginHost::ISubSystem::INotification* notification) override { }

    string SecurityCallsign() const { return (string()); }

    virtual string BuildTreeHash() const override { return (string()); }

    virtual void Set(const subsystem type, WPEFramework::Core::IUnknown* information) override
    {
        switch (type) {
        case INTERNET: {
            WPEFramework::PluginHost::ISubSystem::IInternet* info = (information!=nullptr ? information
                    ->QueryInterface<WPEFramework::PluginHost::ISubSystem::IInternet>() : nullptr);

            _internet = info;

            break;
        }
        case LOCATION: {
            WPEFramework::PluginHost::ISubSystem::ILocation* info = (information!=nullptr ? information
                    ->QueryInterface<WPEFramework::PluginHost::ISubSystem::ILocation>() : nullptr);

            _location = info;

            break;
        }

        default: {
            ASSERT(false && "Unknown Event");
        }
        }
    }
    virtual const WPEFramework::Core::IUnknown* Get(const subsystem type) const override
    {
        const WPEFramework::Core::IUnknown* result(nullptr);

        switch (type) {
        case INTERNET: {
            result = _internet;
            break;
        }
        case LOCATION: {
            result = _location;
            break;
        }
        default: {
            ASSERT(false && "Unknown Event");
        }
        }

        return result;
    }
    virtual bool IsActive(const subsystem type) const override
    {
        return (true);
    };

    virtual void AddRef() const { }
    virtual uint32_t Release() const { return (0); }

    BEGIN_INTERFACE_MAP(SystemInfo)
    INTERFACE_ENTRY(WPEFramework::PluginHost::ISubSystem)
    END_INTERFACE_MAP

private:
    WPEFramework::PluginHost::ISubSystem::IInternet* _internet;
    WPEFramework::PluginHost::ISubSystem::ILocation* _location;
};

} // namespace RdkServicesTest
