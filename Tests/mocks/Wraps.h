#pragma once

#include <stdio.h>
#include <secure_wrappermock.h>


class WrapsImpl {
public:
    virtual ~WrapsImpl() = default;

    virtual int system(const char* command) = 0;
    virtual FILE* popen(const char* command, const char* type) = 0;
    virtual void syslog(int pri, const char* fmt, va_list args) = 0;
    virtual FILE *v_secure_popen(const char *direction, const char *command, va_list args) = 0;
    virtual int v_secure_pclose(FILE *) = 0;
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

    static void syslog(int pri, const char* fmt, va_list args)
    {
        getInstance().impl->syslog(pri, fmt, args);
    }
    static FILE *v_secure_popen(const char *direction, const char *command, va_list args)
    {
        return getInstance().impl->v_secure_popen(direction, command, args);
    }

    static int v_secure_pclose(FILE *file)
    {
        return getInstance().impl->v_secure_pclose(file);
    }
};
