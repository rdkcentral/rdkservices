/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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
 */

#include <gtest/gtest.h>

#include "CountingLock.h"

#include "WorkerPoolImplementation.h"

using namespace WPEFramework;

namespace {
Plugin::CountingLock Lock;

Core::Event Job1Lock(false, true);
Core::Event Job1WillUnlock(false, true);
Core::Event Job2Lock(false, true);
Core::Event Job2WillUnlock(false, true);
Core::Event Job3TryLock(false, true);
Core::Event Job3Lock(false, true);

struct Job1 {
    void Dispatch()
    {
        Plugin::CountingLockSync lockSync(Lock);
        Job1Lock.SetEvent();
        EXPECT_EQ(Core::ERROR_NONE, Job2Lock.Lock()); // wait Job2 lock
        EXPECT_EQ(Core::ERROR_NONE, Job3TryLock.Lock()); // wait Job3 start trying
        EXPECT_EQ(Core::ERROR_TIMEDOUT, Job3Lock.Lock(100)); // expect no Job3 lock in 100 ms
        Job1WillUnlock.SetEvent();
    }
};

struct Job2 {
    void Dispatch()
    {
        Plugin::CountingLockSync lockSync(Lock);
        Job2Lock.SetEvent();
        EXPECT_EQ(Core::ERROR_NONE, Job1Lock.Lock()); // wait Job1 lock
        EXPECT_EQ(Core::ERROR_NONE, Job3TryLock.Lock()); // wait Job3 start trying
        EXPECT_EQ(Core::ERROR_TIMEDOUT, Job3Lock.Lock(100)); // expect no Job3 lock in 100 ms
        Job2WillUnlock.SetEvent();
    }
};

struct Job3 {
    void Dispatch()
    {
        EXPECT_EQ(Core::ERROR_NONE, Job1Lock.Lock()); // wait Job1 Lock
        EXPECT_EQ(Core::ERROR_NONE, Job2Lock.Lock()); // wait Job2 Lock
        Job3TryLock.SetEvent();
        Plugin::CountingLockSync lockSync(Lock, 0);
        Job3Lock.SetEvent();
    }
};
}

class CountingLockTest : public ::testing::Test {
protected:
    Core::ProxyType<WorkerPoolImplementation> workerPool;

    CountingLockTest()
        : workerPool(Core::ProxyType<WorkerPoolImplementation>::Create(
            5, Core::Thread::DefaultStackSize(), 16))
    {
        Core::IWorkerPool::Assign(&(*workerPool));
        workerPool->Run();
    }
    virtual ~CountingLockTest()
    {
        Core::IWorkerPool::Assign(nullptr);
        workerPool.Release();
    }
};

TEST_F(CountingLockTest, countingLockTest)
{
    Job1 job1;
    Job2 job2;
    Job3 job3;

    auto job1Activity = Core::ProxyType<Core::WorkerPool::JobType<Job1&>>::Create(job1);
    auto job2Activity = Core::ProxyType<Core::WorkerPool::JobType<Job2&>>::Create(job2);
    auto job3Activity = Core::ProxyType<Core::WorkerPool::JobType<Job3&>>::Create(job3);

    job1Activity->Submit();
    job2Activity->Submit();
    job3Activity->Submit();

    EXPECT_EQ(Core::ERROR_NONE, Job3Lock.Lock());
    EXPECT_EQ(Core::ERROR_NONE, Job1WillUnlock.Lock());
    EXPECT_EQ(Core::ERROR_NONE, Job2WillUnlock.Lock());

    job1Activity->Revoke();
    job2Activity->Revoke();
    job3Activity->Revoke();

    job1Activity.Release();
    job2Activity.Release();
    job3Activity.Release();
}
