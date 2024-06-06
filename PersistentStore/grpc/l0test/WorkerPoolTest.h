#pragma once

#include "../../Module.h"
#include <gmock/gmock.h>

using ::testing::Test;

class WorkerPoolTest : public Test {
protected:
    WPEFramework::Core::ProxyType<WorkerPoolImplementation> workerPool;
    WorkerPoolTest()
        : workerPool(WPEFramework::Core::ProxyType<WorkerPoolImplementation>::Create(
              WPEFramework::Core::Thread::DefaultStackSize()))
    {
        WPEFramework::Core::IWorkerPool::Assign(&(*workerPool));
    }
    ~WorkerPoolTest() override
    {
        WPEFramework::Core::IWorkerPool::Assign(nullptr);
    }
};
