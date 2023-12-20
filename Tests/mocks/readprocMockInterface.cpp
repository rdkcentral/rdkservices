/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2023 Synamedia
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#include "readprocMockInterface.h"
#include <gmock/gmock.h>

readprocImpl* ProcImpl::impl = nullptr;

ProcImpl::ProcImpl() {}

void ProcImpl::setImpl(readprocImpl* newImpl)
{
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

PROCTAB* ProcImpl::openproc(int flags, ... /* pid_t*|uid_t*|dev_t*|char* [, int n] */ )
{
    EXPECT_NE(impl, nullptr);
    return impl->openproc(flags);
}

void ProcImpl::closeproc(PROCTAB* PT)
{
    EXPECT_NE(impl, nullptr);
    return impl->closeproc(PT);
}

proc_t* ProcImpl::readproc(PROCTAB *__restrict const PT, proc_t *__restrict p)
{
    EXPECT_NE(impl, nullptr);
    return impl->readproc(PT,p);
}

PROCTAB* (*openproc)(int, ...) = &ProcImpl::openproc;
void (*closeproc)(PROCTAB*) = &ProcImpl::closeproc;
proc_t* (*readproc)(PROCTAB*, proc_t*) = &ProcImpl::readproc;
