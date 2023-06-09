#pragma once

#include <stdio.h>

class WrapsImpl {
public:
    virtual ~WrapsImpl() = default;

    virtual int system(const char* command) = 0;
    virtual FILE* popen(const char* command, const char* type) = 0;
    virtual int pclose(FILE *pipe) = 0;
    virtual void syslog(int pri, const char* fmt, va_list args) = 0;
    virtual FILE* fopen(const char* filename, const char* mode) = 0;
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

    static FILE* popen(const char* command, const char* type)
    {
        return getInstance().impl->popen(command, type);
    }

    static int pclose(FILE* pipe)
    {
        return getInstance().impl->pclose(pipe);
    }

    static void syslog(int pri, const char* fmt, va_list args)
    {
        getInstance().impl->syslog(pri, fmt, args);
    }

    static FILE* fopen(const char* filename, const char* mode)
    {
       return getInstance().impl->fopen(filename,mode);
    }
};
