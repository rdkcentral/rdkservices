#pragma once

#include <gmock/gmock.h>
#include "readprocMock.h"

class readprocImplMock : public readprocImpl {
public:
    virtual ~readprocImplMock() = default;

    MOCK_METHOD(PROCTAB*, openproc, (int flags), (override));
    MOCK_METHOD(void, closeproc, (PROCTAB* PT), (override));
    MOCK_METHOD(proc_t*, readproc, (PROCTAB *__restrict const PT, proc_t *__restrict p), (override));
};
