#pragma once

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#define PROC_FILLSTATUS      0x0020 // read status
#define PROC_FILLMEM         0x0001 // read statm
#define PROC_FILLSTAT        0x0040 // read stat

typedef struct proc_t {
// 1st 16 bytes
int
	tid,		// (special)       task id, the POSIX thread ID (see also: tgid)
	ppid;		// stat,status     pid of parent process
char
	cmd[16];	// stat,status     basename of executable file in call to exec(2)

} proc_t;

typedef struct PROCTAB {
DIR*	procfs;
} PROCTAB;

class readprocImpl {
public:
    virtual ~readprocImpl() = default;

    virtual PROCTAB* openproc(int flags) = 0;
    virtual void closeproc(PROCTAB* PT) = 0;
    virtual proc_t* readproc(PROCTAB *__restrict const PT, proc_t *__restrict p) = 0;
 
};

class ProcImpl {
public:
    static ProcImpl& getInstance()
    {
        static ProcImpl instance;
        return instance;
    }

    readprocImpl* impl;
    static PROCTAB* openproc(int flags, ... /* pid_t*|uid_t*|dev_t*|char* [, int n] */ )
    {
        return getInstance().impl->openproc(flags);
    }

    static void closeproc(PROCTAB* PT)
    {
        return getInstance().impl->closeproc(PT);
    }

    static proc_t* readproc(PROCTAB *__restrict const PT, proc_t *__restrict p)
    {
        return getInstance().impl->readproc(PT,p);
    }

};

constexpr auto openproc = &ProcImpl::openproc;
constexpr auto closeproc = &ProcImpl::closeproc;
constexpr auto readproc = &ProcImpl::readproc;




