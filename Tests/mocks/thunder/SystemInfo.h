#pragma once

#include <gmock/gmock.h>

#include "Module.h"

class SystemInfo : public WPEFramework::PluginHost::ISubSystem {
private:
    SystemInfo(const SystemInfo&) = delete;
    SystemInfo& operator=(const SystemInfo&) = delete;

public:
    SystemInfo()
        : _flags(0)
    {
        // Defaults:
        ON_CALL(*this, Set(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const subsystem type, WPEFramework::Core::IUnknown* information) {
                    _subsystems.emplace(type, information);

                    if (type >= NEGATIVE_START) {
                        _flags &= ~(1 << (type - NEGATIVE_START));
                    } else {
                        _flags |= (1 << type);
                    }
                }));
        ON_CALL(*this, Get(::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const subsystem type) -> const WPEFramework::Core::IUnknown* {
                    const WPEFramework::Core::IUnknown* result(nullptr);

                    auto it = _subsystems.find(type);
                    if (it != _subsystems.end()) {
                        result = it->second;
                    }

                    return result;
                }));
        ON_CALL(*this, IsActive(::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const subsystem type) -> bool {
                    return ((type < END_LIST) && ((_flags & (1 << type)) != 0));
                }));
    }
    virtual ~SystemInfo() = default;

public:
    MOCK_METHOD(void, Register, (WPEFramework::PluginHost::ISubSystem::INotification * notification), (override));
    MOCK_METHOD(void, Unregister, (WPEFramework::PluginHost::ISubSystem::INotification * notification), (override));
    MOCK_METHOD(string, BuildTreeHash, (), (const, override));
    MOCK_METHOD(void, Set, (const subsystem type, WPEFramework::Core::IUnknown* information), (override));
    MOCK_METHOD(const WPEFramework::Core::IUnknown*, Get, (const subsystem type), (const, override));
    MOCK_METHOD(bool, IsActive, (const subsystem type), (const, override));
    MOCK_METHOD(string, Version, (), (const, override)); //KKK1

    BEGIN_INTERFACE_MAP(SystemInfo)
    INTERFACE_ENTRY(WPEFramework::PluginHost::ISubSystem)
    END_INTERFACE_MAP

private:
    std::map<subsystem, WPEFramework::Core::IUnknown*> _subsystems;
    uint32_t _flags;
};
