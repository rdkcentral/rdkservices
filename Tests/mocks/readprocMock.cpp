#include "readprocMock.h"
#include "readprocMockInterface.h"

readprocImplMock* impl = new readprocImplMock;


PROCTAB* openproc(int flags, ... /* pid_t*|uid_t*|dev_t*|char* [, int n] */ )
{
    return impl->openproc(flags);
}

void closeproc(PROCTAB* PT)
{
    return impl->closeproc(PT);
}

proc_t* readproc(PROCTAB *__restrict const PT, proc_t *__restrict p)
{
    return impl->readproc(PT,p);
}

