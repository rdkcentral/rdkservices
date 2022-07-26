#pragma once

class ManagerImpl {
public:
    virtual ~ManagerImpl() = default;

    virtual void Initialize() = 0;
    virtual void DeInitialize() = 0;
};

namespace device {
class Manager {
public:
    static Manager& getInstance()
    {
        static Manager instance;
        return instance;
    }

    ManagerImpl* impl;

    static void Initialize()
    {
        return getInstance().impl->Initialize();
    }

    static void DeInitialize()
    {
        return getInstance().impl->DeInitialize();
    }

    static bool IsInitialized;
};
}
