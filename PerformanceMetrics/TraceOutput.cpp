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
 
#include "Module.h"
#include "PerformanceMetrics.h"

namespace WPEFramework {

namespace Trace {

// Trace class for internal information of the PluginHost
class EXTERNAL Metric {
private:
    Metric() = delete;
    Metric(const Metric& a_Copy) = delete;
    Metric& operator=(const Metric& a_RHS) = delete;

public:
    Metric(const TCHAR formatter[], ...)
    {
        va_list ap;
        va_start(ap, formatter);
        Core::Format(_text, formatter, ap);
        va_end(ap);
    }
    explicit Metric(const string& text)
        : _text(Core::ToString(text))
    {
    }
    ~Metric()
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

}

namespace Plugin {

class MetricsTraceOuput : public PerformanceMetrics::IStateMetricsLogger {
public:
    MetricsTraceOuput(const MetricsTraceOuput&) = delete;
    MetricsTraceOuput& operator=(const MetricsTraceOuput&) = delete;

    MetricsTraceOuput() 
        : PerformanceMetrics::IStateMetricsLogger()
        , _callsign()
        , _memory(nullptr)
        {
        }
    ~MetricsTraceOuput() override
    {
        ASSERT(_memory == nullptr);
    }

    void Enable(PluginHost::IShell& service, const string& callsign) override 
    {
        ASSERT(_memory == nullptr);

        _callsign = callsign;
        _memory = service.QueryInterface<Exchange::IMemory>();
    }

    void Disable() override 
    {
        if(_memory != nullptr) {
            _memory->Release();
            _memory = nullptr;
        }
    }

    void Activated() 
    {
        if( _memory != nullptr ) {
            TRACE(Trace::Metric, (_T("Plugin %s activated, RSS: %llu"), _callsign.c_str(), _memory->Resident()));
        }
    }

    void Deactivated(const uint32_t uptime) override 
    {
        TRACE(Trace::Metric, (_T("Plugin %s deactivated, uptime(s): %u"), _callsign.c_str(), (uptime/1000)));
    }

    void Resumed() override
    {
        if( _memory != nullptr ) {
            TRACE(Trace::Metric, (_T("Plugin %s resumed, RSS: %llu"), _callsign.c_str(), _memory->Resident()));
        }
    }

    void Suspended() override
    {
        if( _memory != nullptr ) {
            TRACE(Trace::Metric, (_T("Plugin %s suspended, RSS: %llu"), _callsign.c_str(), _memory->Resident()));
        }
    }

private:
    string _callsign;
    Exchange::IMemory* _memory;
};

class MetricsTraceOuputBrowser : public PerformanceMetrics::IBrowserMetricsLogger {
public:
    MetricsTraceOuputBrowser(const MetricsTraceOuputBrowser&) = delete;
    MetricsTraceOuputBrowser& operator=(const MetricsTraceOuputBrowser&) = delete;

    MetricsTraceOuputBrowser() 
        : PerformanceMetrics::IBrowserMetricsLogger()
        , _callsign()
        , _memory(nullptr)
        {
        }
    ~MetricsTraceOuputBrowser() override
    {
        ASSERT(_memory == nullptr);
    }

    void Enable(PluginHost::IShell& service, const string& callsign) override 
    {
        _callsign = callsign;
        _memory = service.QueryInterface<Exchange::IMemory>();
    }

    void Disable() override 
    {
        if(_memory != nullptr) {
            _memory->Release();
            _memory = nullptr;
        }
    }

    void Activated() 
    {
    }

    void Deactivated(const uint32_t uptime) override 
    {
        TRACE(Trace::Metric, (_T("Plugin %s deactivated, uptime(s): %u"), _callsign.c_str(), (uptime/1000)));
    }

    void Resumed() override
    {
    }

    void Suspended() override
    {
        uint64_t rss = 0;
        if( _memory != nullptr ) {
            rss = _memory->Resident();
        }
        TRACE(Trace::Metric, (_T("Browser %s suspended, RSS: %llu"), _callsign.c_str(), rss));
    }

    void LoadFinished(const string& URL, const int32_t, const bool success, const uint32_t totalsuccess, const uint32_t totalfailed) override 
    {
        if( ( URL != startURL ) ) {

            uint64_t rss = 0;
            if( _memory != nullptr ) {
                rss = _memory->Resident();
            }
            TRACE(Trace::Metric, (_T("Browser %s page loaded [%s] %s, total success[%u], total failure[%u], RSS: %llu"), 
                                        _callsign.c_str(), 
                                        URL.c_str(),
                                        (success == true ? _T("successfully") : _T("unsuccessfully")),
                                        totalsuccess,
                                        totalfailed,
                                        rss));
        }
    }

    void URLChange(const string&, const bool) override 
    {
    }

    void VisibilityChange(const bool) override 
    {
    }
    void PageClosure() override 
    {
    }

private:
    string _callsign;
    Exchange::IMemory* _memory;
};

template<class LOGGERINTERFACE>
std::unique_ptr<LOGGERINTERFACE> PerformanceMetrics::LoggerFactory() {
    return std::unique_ptr<LOGGERINTERFACE>(new MetricsTraceOuput());
}

template<>
std::unique_ptr<PerformanceMetrics::IBrowserMetricsLogger> PerformanceMetrics::LoggerFactory<PerformanceMetrics::IBrowserMetricsLogger>() {
    return std::unique_ptr<PerformanceMetrics::IBrowserMetricsLogger>(new MetricsTraceOuputBrowser());
}

template std::unique_ptr<PerformanceMetrics::IBasicMetricsLogger> PerformanceMetrics::LoggerFactory<PerformanceMetrics::IBasicMetricsLogger>();                
template std::unique_ptr<PerformanceMetrics::IStateMetricsLogger> PerformanceMetrics::LoggerFactory<PerformanceMetrics::IStateMetricsLogger>();                
template std::unique_ptr<PerformanceMetrics::IBrowserMetricsLogger> PerformanceMetrics::LoggerFactory<PerformanceMetrics::IBrowserMetricsLogger>();                

}
}
