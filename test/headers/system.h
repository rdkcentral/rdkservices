#pragma once

class systemServ {
public:
    virtual ~systemServ() = default;
    virtual bool isGzEnabledHelper(bool* enabled) const = 0;
    virtual bool setGzEnabled(bool enabled) const =0;

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

};

class systemPower{
    virtual int setDevicePowerState(const char* powerState, const char* standbyReason) const =0;
    virtual int getDevicePowerState(char* powerState) const =0;
};

