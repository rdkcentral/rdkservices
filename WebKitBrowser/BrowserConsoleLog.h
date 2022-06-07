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
 
#ifndef __BROWSERCONSOLELOG_H
#define __BROWSERCONSOLELOG_H

#include <tracing/tracing.h>
#ifndef WEBKIT_GLIB_API
#include "InjectedBundle/Utils.h"
#endif
using namespace WPEFramework;

class BrowserConsoleLog {
private:
    BrowserConsoleLog() = delete;
    BrowserConsoleLog(const BrowserConsoleLog& a_Copy) = delete;
    BrowserConsoleLog& operator=(const BrowserConsoleLog& a_RHS) = delete;

public:
#ifdef WEBKIT_GLIB_API
    BrowserConsoleLog(const string& prefix, const string& message, const uint64_t line, const uint64_t column)
    {
        _text = '[' + prefix + "][" + Core::NumberType<uint64_t>(line).Text() + ',' + Core::NumberType<uint64_t>(column).Text() + ']' + message;
        const uint16_t maxStringLength = Trace::TRACINGBUFFERSIZE - 1;
        if (_text.length() > maxStringLength) {
            _text = _text.substr(0, maxStringLength);
        }
    }
#else
    BrowserConsoleLog(const string& prefix, const WKStringRef message, const uint64_t line, const uint64_t column)
    { 
        _text = '[' + prefix + "][" + Core::NumberType<uint64_t>(line).Text() + ',' + Core::NumberType<uint64_t>(column).Text() + ']' + WebKit::Utils::WKStringToString(message);
        const uint16_t maxStringLength = Trace::TRACINGBUFFERSIZE - 1;
        if (_text.length() > maxStringLength) {
            _text = _text.substr(0, maxStringLength);
        }
    }
#endif
    ~BrowserConsoleLog()
    {
    }

public:
    inline const char* Data() const
    {
        return (_text.c_str());
    }
    inline uint16_t Length() const
    {
        return (static_cast<uint16_t>(_text.length()));
    }

private:
    std::string _text;
};

#endif // __BROWSERCONSOLELOG_H
