#pragma once

class systemServ {
public:
    virtual ~systemServ() = default;
    virtual bool isGzEnabledHelper(bool* enabled) = 0;
    virtual bool setGzEnabled(bool enabled) = 0;
    virtual int setDevicePowerState(const char* powerState, const char* standbyReason) = 0;
    virtual int getDevicePowerState(char* powerState)  = 0;
};

class systemServImpl {

    public:
    static systemServImpl& getInstance()
    {
        static  systemServImpl instance;
        return instance;
    }

    systemServ* impl;

    static bool isGzEnabledHelper(bool* enabled)
    {
        return getInstance().impl->isGzEnabledHelper(enabled);
    }

    static bool setGzEnabled(bool enabled)
    {
        return getInstance().impl->setGzEnabled(enabled);
    }

    static int setDevicePowerState(const char* powerState, const char* standbyReason)
    {
        return getInstance().impl->setDevicePowerState(powerState,standbyReason);
    }

    static in getDevicePowerState(char* powerState)
    {
        return getInstance().impl->getDevicePowerState(powerState);
    }

};


