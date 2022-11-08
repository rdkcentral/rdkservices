#include "WrapsMock.h"

#include <stdarg.h>
#include <syslog.h>

extern "C" int __wrap_system(const char* command)
{
    return WrapsMock::_system(command);
}

extern "C" FILE* __wrap_popen(const char* command, const char* type)
{
    return WrapsMock::_popen(command, type);
}

extern "C" void __wrap_syslog(int pri, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    WrapsMock::_syslog(pri, fmt, args);
    vsyslog(pri, fmt, args);
    va_end(args);
}
