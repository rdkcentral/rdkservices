#include <stdarg.h>
#include <syslog.h>
#include "Wraps.h"
#include <unistd.h>

extern "C" int __wrap_system(const char* command)
{
    return Wraps::getInstance().system(command);
}

extern "C" FILE* __wrap_popen(const char* command, const char* type)
{
    return Wraps::getInstance().popen(command, type);
}

//This function will be called for syslog() in the code (added -Wl,-wrap,syslog)
extern  "C" void __wrap_syslog(int pri, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Wraps::getInstance().syslog(pri, fmt, args);
    vsyslog(pri, fmt, args);
    va_end(args);
}

extern "C" FILE * __wrap_v_secure_popen(const char *direction, const char *command, ...)
{
    va_list args;
    FILE *retFp = nullptr;
    va_start(args, command);
    Wraps::getInstance().v_secure_popen(direction, command, args);
    retFp = v_secure_popen(direction, command, &args);
    va_end(args);
    return retFp;
}

extern "C" int __wrap_v_secure_pclose(FILE *file)
{
    return Wraps::getInstance().v_secure_pclose(file);
}

extern "C" int __wrap_unlink(const char* filePath)
{
    return Wraps::getInstance().unlink(filePath);
}