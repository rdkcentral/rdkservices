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

class SystemInfo : public WPEFramework::PluginHost::ISubSystem {
private:
    SystemInfo(const SystemInfo&) = delete;
    SystemInfo& operator=(const SystemInfo&) = delete;

public:
    SystemInfo()
        : _flags(0)
    {
    }
    virtual ~SystemInfo() = default;

public:
    virtual void Register(WPEFramework::PluginHost::ISubSystem::INotification* notification) override {}
    virtual void Unregister(WPEFramework::PluginHost::ISubSystem::INotification* notification) override {}

    string SecurityCallsign() const { return (string()); }

    virtual string BuildTreeHash() const override { return (string()); }

    virtual void Set(const subsystem type, WPEFramework::Core::IUnknown* information) override
    {
        _subsystems.emplace(type, information);

        if (type >= NEGATIVE_START) {
            _flags &= ~(1 << (type - NEGATIVE_START));
        } else {
            _flags |= (1 << type);
        }
    }
    virtual const WPEFramework::Core::IUnknown* Get(const subsystem type) const override
    {
        const WPEFramework::Core::IUnknown* result(nullptr);

        auto it = _subsystems.find(type);
        if (it != _subsystems.end()) {
            result = it->second;
        }

        return result;
    }
    virtual bool IsActive(const subsystem type) const override
    {
        return ((type < END_LIST) && ((_flags & (1 << type)) != 0));
    };

    BEGIN_INTERFACE_MAP(SystemInfo)
    INTERFACE_ENTRY(WPEFramework::PluginHost::ISubSystem)
    END_INTERFACE_MAP

private:
    std::map<subsystem, WPEFramework::Core::IUnknown*> _subsystems;
    uint32_t _flags;
};
