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

#include "source/WorkerPoolImplementation.h"

using namespace WPEFramework;

namespace {
const unsigned int nTime = 1000;

Plugin::CountingLock Lock;

Core::Event Job1AcquiredLock(0, 1);
Core::Event Job1Complete(0, 1);
Core::Event Job2AcquiredLock(0, 1);
Core::Event Job2Complete(0, 1);
Core::Event Job3WantsToAcquireLock(0, 1);
Core::Event Job3AcquiredLock(0, 1);
Core::Event Job3Complete(0, 1);

struct Job1 {
    void Dispatch()
    {
        // Job1 locks and expects Job3 to fail lock.

        Plugin::CountingLockSync lockSync(Lock);

        Job1AcquiredLock.SetEvent();

        if ((Job3WantsToAcquireLock.Lock(nTime) == Core::ERROR_NONE) && (Job3AcquiredLock.Lock(nTime) == Core::ERROR_TIMEDOUT)) {
            Job1Complete.SetEvent();
        }
    }
};

struct Job2 {
    void Dispatch()
    {
        // Job2 locks and expects Job3 to fail lock.

        Plugin::CountingLockSync lockSync(Lock);

        Job2AcquiredLock.SetEvent();

        if ((Job3WantsToAcquireLock.Lock(nTime) == Core::ERROR_NONE) && (Job3AcquiredLock.Lock(nTime) == Core::ERROR_TIMEDOUT)) {
            Job2Complete.SetEvent();
        }
    }
};

struct Job3 {
    void Dispatch()
    {
        // Job3 locks exclusive after Job1 and Job2 lock.

        if ((Job1AcquiredLock.Lock(nTime) == Core::ERROR_NONE) && (Job2AcquiredLock.Lock(nTime) == Core::ERROR_NONE)) {

            Job3WantsToAcquireLock.SetEvent();

            Plugin::CountingLockSync lockSync(Lock, 0);

            Job3AcquiredLock.SetEvent();

            if (Job1Complete.IsSet() && Job2Complete.IsSet()) {
                Job3Complete.SetEvent();
            }
        }
    }
};
}

class CountingLockTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<WorkerPoolImplementation> workerPool;

    CountingLockTestFixture()
        : workerPool(Core::ProxyType<WorkerPoolImplementation>::Create(
            5, Core::Thread::DefaultStackSize(), 16))
    {
    }
    virtual ~CountingLockTestFixture()
    {
    }

    virtual void SetUp()
    {
        Core::IWorkerPool::Assign(&(*workerPool));

        workerPool->Run();
    }

    virtual void TearDown()
    {
        Core::IWorkerPool::Assign(nullptr);
        workerPool.Release();
    }
};

TEST_F(CountingLockTestFixture, countingLockTest)
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

    EXPECT_EQ(Core::ERROR_NONE, Job3Complete.Lock(5 * nTime));
    EXPECT_TRUE(Job1AcquiredLock.IsSet());
    EXPECT_TRUE(Job2AcquiredLock.IsSet());
    EXPECT_TRUE(Job3WantsToAcquireLock.IsSet());
    EXPECT_TRUE(Job3AcquiredLock.IsSet());
    EXPECT_TRUE(Job1Complete.IsSet());
    EXPECT_TRUE(Job2Complete.IsSet());
    EXPECT_TRUE(Job3Complete.IsSet());

    job1Activity->Revoke();
    job2Activity->Revoke();
    job3Activity->Revoke();

    job1Activity.Release();
    job2Activity.Release();
    job3Activity.Release();
}
