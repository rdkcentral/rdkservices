/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
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

#pragma once

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

    ~WorkerPoolImplementation()
    {
        WPEFramework::Core::WorkerPool::Stop();
    }

private:
    Dispatcher _dispatcher;
};
