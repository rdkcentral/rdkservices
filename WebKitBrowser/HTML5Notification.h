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
 
#ifndef __HTML5NOTIFICATION_H
#define __HTML5NOTIFICATION_H

#include <tracing/tracing.h>

using namespace WPEFramework;

class HTML5Notification {
private:
    HTML5Notification() = delete;
    HTML5Notification(const HTML5Notification& a_Copy) = delete;
    HTML5Notification& operator=(const HTML5Notification& a_RHS) = delete;

public:
    HTML5Notification(const TCHAR formatter[], ...)
    {
        va_list ap;
        va_start(ap, formatter);
        Core::Format(_text, formatter, ap);
        va_end(ap);
    }
    explicit HTML5Notification(const string& text)
        : _text(Core::ToString(text))
    {
    }
    ~HTML5Notification()
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

#endif // __HTML5NOTIFICATION_H
