#pragma once

#include "list.hpp"

namespace device {
class SleepMode;
class SleepModeImpl {
public:
    virtual ~SleepModeImpl() = default;
    virtual List<SleepMode> getSleepModes() = 0;
    virtual const std::string& toString() const = 0;
};

class SleepMode {
public:
    SleepModeImpl* impl;

    static SleepMode& getInstance()
    {
        static SleepMode instance;
        return instance;
    }

    static SleepMode& getInstance(int)
    {
        return getInstance();
    }

    static SleepMode& getInstance(const std::string&)
    {
        return getInstance();
    }

    List<SleepMode> getSleepModes()
    {
        return impl->getSleepModes();
    }

    const std::string& toString() const
    {
        return impl->toString();
    }
};
}
