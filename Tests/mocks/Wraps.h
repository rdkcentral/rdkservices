#pragma once

#include <stdio.h>
#include <mntent.h>
#include "secure_wrappermock.h"

class WrapsImpl {
public:
    virtual ~WrapsImpl() = default;

    virtual int system(const char* command) = 0;
    virtual FILE* popen(const char* command, const char* type) = 0;
    virtual int pclose(FILE *pipe) = 0;
    virtual void syslog(int pri, const char* fmt, va_list args) = 0;
    virtual FILE* setmntent(const char* command, const char* type) = 0;
    virtual struct mntent* getmntent(FILE *pipe) = 0;
    virtual FILE *v_secure_popen(const char *direction, const char *command, va_list args) = 0;
    virtual int v_secure_pclose(FILE *) = 0;
    virtual int v_secure_system(const char *command, va_list args) =0;
    virtual ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) = 0;
};

class Wraps {
protected:
   static WrapsImpl* impl;

public:
    Wraps();
    Wraps(const Wraps &obj) = delete;
    static void setImpl(WrapsImpl* newImpl);
    static Wraps& getInstance();

    static int system(const char* command);

    static FILE* popen(const char* command, const char* type);

    static int pclose(FILE* pipe);

    static void syslog(int pri, const char* fmt, va_list args);

    static FILE* setmntent(const char* command, const char* type);

    static struct mntent* getmntent(FILE* pipe);

    static FILE *v_secure_popen(const char *direction, const char *command, va_list args);

    static int v_secure_pclose(FILE *file);

    static int v_secure_system(const char *command, va_list args);

    ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
};
