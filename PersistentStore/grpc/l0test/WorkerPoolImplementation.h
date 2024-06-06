#pragma once

#include "../../Module.h"

class WorkerPoolImplementation : public WPEFramework::Core::WorkerPool, public WPEFramework::Core::ThreadPool::ICallback {
private:
    class Dispatcher : public WPEFramework::Core::ThreadPool::IDispatcher {
    public:
        Dispatcher(const Dispatcher&) = delete;
        Dispatcher& operator=(const Dispatcher&) = delete;

        Dispatcher() = default;
        ~Dispatcher() override = default;

    private:
        void Initialize() override
        {
        }
        void Deinitialize() override
        {
        }
        void Dispatch(WPEFramework::Core::IDispatch* job) override
        {
            job->Dispatch();
        }
    };

public:
    WorkerPoolImplementation() = delete;
    WorkerPoolImplementation(const WorkerPoolImplementation&) = delete;
    WorkerPoolImplementation& operator=(const WorkerPoolImplementation&) = delete;

    WorkerPoolImplementation(const uint32_t stackSize)
        : WPEFramework::Core::WorkerPool(4 /*threadCount*/, stackSize, 32 /*queueSize*/, &_dispatch, this)
        , _dispatch()
    {
        Run();
    }
    ~WorkerPoolImplementation() override
    {
        Stop();
    }
    void Idle() override
    {
    }

private:
    Dispatcher _dispatch;
};
