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

#include "Module.h"

#ifndef __WINDOWS__
#include <syslog.h>
#endif

#include <sstream>
#include <type_traits>

namespace WPEFramework {
namespace Plugin {

    class TraceTextOutput : public Trace::ITraceMedia {
    public:
        TraceTextOutput(const TraceTextOutput&) = delete;
        TraceTextOutput& operator=(const TraceTextOutput&) = delete;

        explicit TraceTextOutput(const bool abbreviated)
            : _abbreviated(abbreviated)
        {
        }
        ~TraceTextOutput() override = default;

    public:
        void Output(const char fileName[], const uint32_t lineNumber, const char /* className */ [], const Trace::ITrace* information) override
        {
            std::stringstream output;
            if( _abbreviated == true ) {
                string time(Core::Time::Now().ToTimeOnly(true));
                output << '[' << time.c_str() << "]: " << information->Data() << std::endl;
            } else {
                string time(Core::Time::Now().ToRFC1123(true));
                output << '[' << time.c_str() << "]:[" << Core::FileNameOnly(fileName) << ':' << lineNumber << "] " << information->Category()
                       << ": " << information->Data() << std::endl;
            }
            HandleTraceMessage(output.str());
        }

    private:
        virtual void HandleTraceMessage(const string& message) = 0;

    private:
        bool _abbreviated;
    };
    class TraceSyslogOutput : public TraceTextOutput {
    public:
        TraceSyslogOutput(const TraceSyslogOutput&) = delete;
        TraceSyslogOutput& operator=(const TraceSyslogOutput&) = delete;

        explicit TraceSyslogOutput(const bool abbreviated)
            : TraceTextOutput(abbreviated)
        {
        }

        ~TraceSyslogOutput() override = default;

    private:
        void HandleTraceMessage(const string& message) override {
#ifndef __WINDOWS__
            syslog(LOG_NOTICE, _T("%s"), message.c_str());
#else
            printf(_T("%s"), message.c_str());
#endif
        }
    };

    class TraceConsoleOutput : public TraceTextOutput {
    public:
        TraceConsoleOutput(const TraceConsoleOutput&) = delete;
        TraceConsoleOutput& operator=(const TraceConsoleOutput&) = delete;

        explicit TraceConsoleOutput(const bool abbreviated)
            : TraceTextOutput(abbreviated)
        {
        }
        ~TraceConsoleOutput() override = default;

    private:
        void HandleTraceMessage(const string& message) override {
            printf(_T("%s"), message.c_str());
        }
    };
    class TraceJSONOutput : public Trace::ITraceMedia {
    public:
        class Data : public Core::JSON::Container {

        public:
            Data(const Data&) = delete;
            Data& operator=(const Data&) = delete;

            Data()
                : Core::JSON::Container()
                , Time()
                , Filename()
                , Linenumber()
                , Classname()
                , Category()
                , Message()
            {
                Add(_T("time"), &Time);
                Add(_T("filename"), &Filename);
                Add(_T("linenumber"), &Linenumber);
                Add(_T("classname"), &Classname);
                Add(_T("category"), &Category);
                Add(_T("message"), &Message);
            }

            ~Data() override = default;

        public:
            Core::JSON::String Time;
            Core::JSON::String Filename;
            Core::JSON::DecUInt32 Linenumber;
            Core::JSON::String Classname;
            Core::JSON::String Category;
            Core::JSON::String Message;
        };

    public:
        TraceJSONOutput(const TraceJSONOutput&) = delete;
        TraceJSONOutput& operator=(const TraceJSONOutput&) = delete;

        enum class ExtraOutputOptions {
            ABREVIATED      = 0,
            FILENAME        = 1,
            LINENUMBER      = 3,  // selecting LINENUMBER will automatically select FILENAME
            CLASSNAME       = 4,
            CATEGORY        = 8,
            INCLUDINGDATE   = 16,
            ALL             = 31
        };

        explicit TraceJSONOutput(const ExtraOutputOptions outputoptions = ExtraOutputOptions::ALL)
            : _outputoptions(outputoptions)
        {
        }
        ~TraceJSONOutput() override = default;

    public:

        ExtraOutputOptions OutputOptions() const {
            return _outputoptions;
        }

        void OutputOptions(const ExtraOutputOptions outputoptions) {
            _outputoptions = outputoptions;
        }

        // just because I'm lazy :)
        template<typename E>
        static inline auto AsNumber(E t) -> typename std::underlying_type<E>::type {
            return static_cast<typename std::underlying_type<E>::type>(t);
        }

    public:

        void Output(const char fileName[], const uint32_t lineNumber, const char className[], const Trace::ITrace* information) override {
            ExtraOutputOptions options = _outputoptions;

            Core::ProxyType<Data> data = GetDataContainer();
            data->Clear();
            if( ( AsNumber(options) & AsNumber(ExtraOutputOptions::INCLUDINGDATE) ) != 0 ) {
                data->Time = Core::Time::Now().ToRFC1123(true);
            } else {
                data->Time = Core::Time::Now().ToTimeOnly(true);
            }

            if( ( AsNumber(options) & AsNumber(ExtraOutputOptions::FILENAME) ) != 0 ) {
                data->Filename = fileName;
                if( ( AsNumber(options) & AsNumber(ExtraOutputOptions::LINENUMBER) ) != 0 ) {
                    data->Linenumber = lineNumber;
                }
            }

            if( ( AsNumber(options) & AsNumber(ExtraOutputOptions::CLASSNAME) ) != 0 ) {
                data->Classname = className;
            }

            if( ( AsNumber(options) & AsNumber(ExtraOutputOptions::CATEGORY) ) != 0 ) {
                data->Category = information->Category();
            }

            data->Message = information->Data();

            HandleTraceMessage(data);
        }


    private:
        virtual void HandleTraceMessage(const Core::ProxyType<Data>& jsondata) = 0;
        virtual Core::ProxyType<Data> GetDataContainer() = 0;
    private:
        std::atomic<ExtraOutputOptions> _outputoptions;

    };

}
}
