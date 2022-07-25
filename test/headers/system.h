
class systemServ {
public:
    virtual int setMode(const char* mode, const int duration) const = 0 ;
    virtual int getMode(char* mode, int* duration) const = 0 ;

};

class systemFile{
public:
    virtual bool isGzEnabledHelper(bool* enabled) const = 0;
    virtual bool setGzEnabled(bool enabled) const =0;
};

class systemCmdImpl {
    public:
        virtual ~systemCmdImpl() = default;
        virtual int system() = 0;
};
class systemCmd{
    public:
    static systemCmd& getInstance()
    {
        static systemCmd instance;
        return instance;
    }

    systemCmdImpl* impl;

    static int system(const char* command)
    {
        return getInstance().impl->system(command);
    }



};

class systemPower{
    virtual int setDevicePowerState(const char* powerState, const char* standbyReason) const =0;
    virtual int getDevicePowerState(char* powerState) const =0;
};

class systemIARM{


};
