#pragma once

#include <stdio.h>
#include <mntent.h>
#include <secure_wrappermock.h>

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

	static FILE* setmntent(const char* command, const char* type)
    {
        return getInstance().impl->setmntent(command, type);
    }

    static struct mntent* getmntent(FILE* pipe)
    {
        return getInstance().impl->getmntent(pipe);
    }
     static FILE *v_secure_popen(const char *direction, const char *command, va_list args)
    {
        return getInstance().impl->v_secure_popen(direction, command, args);
    }

    static int v_secure_pclose(FILE *file)
    {
        return getInstance().impl->v_secure_pclose(file);
    }
    static int v_secure_system(const char *command, va_list args)
    {
    	return getInstance().impl->v_secure_system(command,args);
    }
};
