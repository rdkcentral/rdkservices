#pragma once

class WrapsImpl {
public:
    virtual ~WrapsImpl() = default;

    virtual int system(const char* command) = 0;
};

class Wraps {
public:
    static Wraps& getInstance()
    {
        static Wraps instance;
        return instance;
    }

    WrapsImpl* impl;

    static int system(const char* command)
    {
        return getInstance().impl->system(command);
    }
};
