#pragma once

#include <gmock/gmock.h>

#include "Module.h"

class WorkerPoolImplementation : public WPEFramework::Core::WorkerPool {
private:
    class Dispatcher : public WPEFramework::Core::ThreadPool::IDispatcher {
    public:
        Dispatcher(const Dispatcher&) = delete;
        Dispatcher& operator=(const Dispatcher&) = delete;

        Dispatcher() = default;
        ~Dispatcher() override = default;

    private:
        void Initialize() override {}
        void Deinitialize() override {}
        void Dispatch(WPEFramework::Core::IDispatch* job) override
        {
            job->Dispatch();
        }
    };

public:
    WorkerPoolImplementation() = delete;
    WorkerPoolImplementation(const WorkerPoolImplementation&) = delete;
    WorkerPoolImplementation& operator=(const WorkerPoolImplementation&) = delete;

    WorkerPoolImplementation(const uint8_t threads, const uint32_t stackSize, const uint32_t queueSize)
        : WPEFramework::Core::WorkerPool(threads - 1, stackSize, queueSize, &_dispatcher)
        , _dispatcher()
    {
    }

    virtual ~WorkerPoolImplementation()
    {
        WPEFramework::Core::WorkerPool::Stop();
    }

private:
    Dispatcher _dispatcher;
};
