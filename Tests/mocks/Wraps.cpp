#include <stdarg.h>
#include <syslog.h>
#include "Wraps.h"
#include <mntent.h>
#include <gmock/gmock.h>

extern "C" int __wrap_system(const char* command)
{
    return Wraps::getInstance().system(command);
}

extern "C" FILE* __wrap_popen(const char* command, const char* type)
{
    return Wraps::getInstance().popen(command, type);
}

extern "C" int __wrap_pclose(FILE* pipe)
{
    return Wraps::getInstance().pclose(pipe);
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

extern "C" FILE* __wrap_setmntent(const char* command, const char* type)
{
    return Wraps::getInstance().setmntent(command, type);
}

extern "C" struct mntent* __wrap_getmntent(FILE* pipe)
{
    return Wraps::getInstance().getmntent(pipe);
}

extern "C" FILE * __wrap_v_secure_popen(const char *direction, const char *command, ...) 
{
    va_list args;
    FILE *retFp = nullptr;
    va_start(args, command);
    retFp = Wraps::getInstance().v_secure_popen(direction, command, args);
    va_end(args);
    return retFp;
}


extern "C" int __wrap_v_secure_pclose(FILE *file)
{
    return Wraps::getInstance().v_secure_pclose(file);
}


extern "C" int __wrap_v_secure_system(const char *command, ...)
{
    int ret;
    va_list args;
    va_start(args,command);
    ret = Wraps::getInstance().v_secure_system(command, args);
    va_end(args);
    return ret;
}

WrapsImpl* Wraps::impl = nullptr;

Wraps::Wraps() {}

void Wraps::setImpl(WrapsImpl* newImpl)
{
        // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

Wraps& Wraps::getInstance()
{
   static Wraps instance;
   return instance;
}

int Wraps::system(const char* command)
{
    EXPECT_NE(impl, nullptr);
    return impl->system(command);
}

FILE* Wraps::popen(const char* command, const char* type)
{
    EXPECT_NE(impl, nullptr);
    return impl->popen(command, type);
}

int Wraps::pclose(FILE* pipe)
{
    EXPECT_NE(impl, nullptr);
    return impl->pclose(pipe);
}

void Wraps::syslog(int pri, const char* fmt, va_list args)
{
    EXPECT_NE(impl, nullptr);
    impl->syslog(pri, fmt, args);
}

FILE* Wraps::setmntent(const char* command, const char* type)
{
    EXPECT_NE(impl, nullptr);
    return impl->setmntent(command,type);
}

struct mntent* Wraps::getmntent(FILE* pipe)
{
    EXPECT_NE(impl, nullptr);
    return impl->getmntent(pipe);
}

FILE * Wraps::v_secure_popen(const char *direction, const char *command, va_list args)
{
    EXPECT_NE(impl, nullptr);
    return impl->v_secure_popen(direction, command, args);
}

int Wraps::v_secure_pclose(FILE *file)
{
    EXPECT_NE(impl, nullptr);
    return impl->v_secure_pclose(file);
}

int Wraps::v_secure_system(const char *command, va_list args)
{
    EXPECT_NE(impl, nullptr);
    return impl->v_secure_system(command,args);
}

