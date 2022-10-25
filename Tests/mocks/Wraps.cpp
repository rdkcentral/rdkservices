#include <stdarg.h>
#include "Wraps.h"

extern "C" int __wrap_system(const char* command)
{
    return Wraps::getInstance().system(command);
}

extern "C" FILE* __wrap_popen(const char* command, const char* type)
{
    return Wraps::getInstance().popen(command, type);
}

extern "C" FILE* __real_syslog(int pri, const char *fmt, ...);
//This function will be called for syslog() in the code (added -Wl,-wrap,syslog)
extern  "C" void __wrap_syslog(int pri, const char *fmt, ...)
{
    char strFmt[256];
    va_list args;
    va_start(args, fmt);
    vsprintf(strFmt, fmt, args);
    va_end(args);

    Wraps::getInstance().syslog(pri, strFmt);
    __real_syslog(pri, fmt);
}
