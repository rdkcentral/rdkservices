/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#pragma once

#include "tags.h"
#include <wpe/webkit.h>

namespace Testing {

class TestRunner {
public:
    static TestRunner* Instance();

    virtual bool EnsureInitialized(WebKitWebView* parent, const char* extensionDir) = 0;
    virtual bool handleUserMessage(WebKitUserMessage *message) = 0;

protected:
    TestRunner() = default;
    TestRunner(const TestRunner&) = delete;
};

} // namespace Testing
