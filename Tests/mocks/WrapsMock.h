#pragma once

#include <gmock/gmock.h>

class WrapsMock {
public:
    static WrapsMock* instance{ nullptr };
    WrapsMock()
    {
        instance = this;
    }
    virtual ~WrapsMock()
    {
        instance = nullptr;
    }

    MOCK_METHOD(int, system, (const char* command));
    MOCK_METHOD(FILE*, popen, (const char* command, const char* type));
    MOCK_METHOD(void, syslog, (int pri, const char* fmt, va_list args));

    static int _system(const char* command)
    {
        return instance->system(command);
    }

    static FILE* _popen(const char* command, const char* type)
    {
        return instance->popen(command, type);
    }

    static void _syslog(int pri, const char* fmt, va_list args)
    {
        instance->syslog(pri, fmt, args);
    }
};
