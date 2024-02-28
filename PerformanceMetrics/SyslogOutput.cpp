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

#include <utility>
#include <array>
#include <fstream>
#include <string>

#include <sys/sysinfo.h>

#include "UtilsTelemetry.h"

namespace WPEFramework {
namespace Plugin {

class SysLogOuput : public PerformanceMetrics::IBrowserMetricsLogger {
private:

    static constexpr char webProcessName[] = _T("WPEWebProcess");

    class URLLoadedMetrics {
    public:

        static constexpr uint8_t nbrAvgCpuMeasurement = 3;

        using AverageCPULoadArray = std::array<uint64_t, nbrAvgCpuMeasurement>;

    public:

        URLLoadedMetrics() 
        : _total(0)
        , _free(0)
        , _swapped(0)
        , _uptime(0)
        , _avgload()
        , _rssmemprocess(0)
        , _statmline()
        , _startload_ms(0)
        , _idletime_s(0)
        , _cold(false)
        {
        }
        ~URLLoadedMetrics() = default;

        URLLoadedMetrics(const URLLoadedMetrics& copy)
        : _total(copy._total)
        , _free(copy._free)
        , _swapped(copy._swapped)
        , _uptime(copy._uptime)
        , _avgload(copy._avgload)
        , _rssmemprocess(copy._rssmemprocess)
        , _statmline(copy._statmline)
        , _startload_ms(copy._startload_ms)
        , _idletime_s(copy._idletime_s)
        , _cold(copy._cold)
        { 
        }

        URLLoadedMetrics& operator=(const URLLoadedMetrics& copy) 
        {
            if (this != &copy) {
                _total = copy._total;
                _free = copy._free;
                _swapped = copy._swapped;
                _uptime = copy._uptime;
                _avgload = copy._avgload;
                _rssmemprocess = copy._rssmemprocess;
                _statmline = copy._statmline;
                _startload_ms = copy._startload_ms;
                _idletime_s = copy._idletime_s;
                _cold=copy._cold;
            }

            return *this;
        }

        URLLoadedMetrics(URLLoadedMetrics&& orig) 
        : _total(orig._total)
        , _free(orig._free)
        , _swapped(orig._swapped)
        , _uptime(orig._uptime)
        , _avgload(orig._avgload)
        , _rssmemprocess(orig._rssmemprocess)
        , _statmline(std::move(orig._statmline))
        , _startload_ms(orig._startload_ms)
        , _idletime_s(orig._idletime_s)
        , _cold(orig._cold)
        {
        }

        URLLoadedMetrics& operator=(URLLoadedMetrics&& orig) 
        {
            _total = orig._total;
            _free = orig._free;
            _swapped = orig._swapped;
            _uptime = orig._uptime;
            _avgload = orig._avgload;
            _rssmemprocess = orig._rssmemprocess;
            _statmline = std::move(orig._statmline);
            _startload_ms = orig._startload_ms;
            _idletime_s = orig._idletime_s;
            _cold = orig._cold;

            return *this;
        }

        uint64_t Total() const { return _total; }
        void Total(const uint64_t total) { _total = total; }

        uint64_t Free() const { return _free; }
        void Free(const uint64_t free) { _free = free; }

        uint64_t Swapped() const { return _swapped; }
        void Swapped(const uint64_t swapped) { _swapped = swapped; }

        uint64_t Uptime() const { return _uptime; }
        void Uptime(const uint64_t uptime) { _uptime = uptime; }

        const AverageCPULoadArray& AverageLoad() const { return _avgload; }
        AverageCPULoadArray& AverageLoad() { return _avgload; }

        uint64_t RSSMemProcess() const { return _rssmemprocess; }
        void RSSMemProcess(const uint64_t rssmemprocess) { _rssmemprocess = rssmemprocess; }

        const string& StatmLine() const { return _statmline; }
        void StatmLine(const string& statmline) { _statmline = statmline; }

        Core::Time::microsecondsfromepoch StartLoad() const { return _startload_ms; }
        void StartLoad(const Core::Time::microsecondsfromepoch startload) { _startload_ms = startload; }

        uint64_t IdleTime() const { return _idletime_s; }
        void IdleTime(const uint64_t idletime) { _idletime_s = idletime; }

        bool IsColdLaunch() const { return _cold; }
        void SetColdLaunch(bool mode) { _cold= mode; }

    private:
        uint64_t _total;
        uint64_t _free;
        uint64_t _swapped;
        uint64_t _uptime;
        AverageCPULoadArray _avgload;
        uint64_t _rssmemprocess;
        string _statmline;
        Core::Time::microsecondsfromepoch _startload_ms;
        uint32_t _idletime_s;
        bool _cold;
    };

    public:

        enum class ModeType {
            WARM,
            COLD
        };

    private:

    class MetricsAsJson : public Core::JSON::Container {
        public:

            MetricsAsJson()
                : Core::JSON::Container()
                , Mode(ModeType::WARM)
                , AppType()
                , MemoryTotal()
                , MemoryFree()
                , MemorySwapped()
                , Uptime()
                , LoadAvarage()
                , NbrProcessors()
                , ProcessRSS()
                , ProcessPID()
                , Appname()
                , StatmLine()
                , IdleTime()
                , LaunchTime()
                , LoadSuccess()
                , NbrLoaded()
                , Callsign()
            {
                Add(_T("LaunchState"), &Mode);
                Add(_T("AppType"), &AppType);
                Add(_T("MemTotal"), &MemoryTotal);
                Add(_T("MemFree"), &MemoryFree);
                Add(_T("MemSwapped"), &MemorySwapped);
                Add(_T("Uptime"), &Uptime);
                Add(_T("LoadAvg"), &LoadAvarage);
                Add(_T("NProc"), &NbrProcessors);
                Add(_T("ProcessRSS"), &ProcessRSS);
                Add(_T("ProcessPID"), &ProcessPID);
                Add(_T("AppName"), &Appname);
                Add(_T("webProcessStatmLine"), &StatmLine);
                Add(_T("webProcessIdleTime"), &IdleTime);
                Add(_T("LaunchTime"), &LaunchTime);
                Add(_T("AppLoadSuccess"), &LoadSuccess);
                Add(_T("webPageLoadNum"), &NbrLoaded);
                Add(_T("CallSign"), &Callsign);
            }
            ~MetricsAsJson() override = default;

            MetricsAsJson(const MetricsAsJson&) = delete;
            MetricsAsJson& operator=(const MetricsAsJson&) = delete;

        public:
            Core::JSON::EnumType<ModeType> Mode;
            Core::JSON::String AppType;
            Core::JSON::DecUInt64 MemoryTotal;
            Core::JSON::DecUInt64 MemoryFree;
            Core::JSON::DecUInt64 MemorySwapped;
            Core::JSON::DecUInt64 Uptime;
            Core::JSON::String LoadAvarage;
            Core::JSON::DecUInt32 NbrProcessors;
            Core::JSON::DecUInt64 ProcessRSS;
            Core::JSON::DecUInt32 ProcessPID;
            Core::JSON::String Appname;
            Core::JSON::String StatmLine;
            Core::JSON::DecUInt32 IdleTime;
            Core::JSON::DecUInt64 LaunchTime;
            Core::JSON::DecUInt8 LoadSuccess; // kept backwards compatibility, use 0 and 1 instead of bool
            Core::JSON::DecUInt32 NbrLoaded;
            Core::JSON::String Callsign;
    };

public:
    SysLogOuput(const SysLogOuput&) = delete;
    SysLogOuput& operator=(const SysLogOuput&) = delete;

    SysLogOuput() 
        : PerformanceMetrics::IBrowserMetricsLogger()
        , _callsign()
        , _service(nullptr)
        , _memory(nullptr)
        , _processmemory(nullptr)
        , _cold(true)
        , _urloadmetrics()
        , _timeIdleFirstStart(0)
        , _timePluginStart(0)
        , _adminLock()
        , _lastURL()
        , _didLogLaunchMetrics(false)
        , _lastLoggedApp()
        {
            Utils::Telemetry::init();
        }
    ~SysLogOuput() override
    {
        ASSERT(_memory == nullptr);
        ASSERT(_processmemory == nullptr);
    }

    void Enable(PluginHost::IShell& service, const string& callsign) override 
    {
        ASSERT(_memory == nullptr);
        ASSERT(_processmemory == nullptr);

        _callsign = callsign;
        _service = &service;
        _service->AddRef();
        _memory = service.QueryInterface<Exchange::IMemory>();
        if (_memory != nullptr) {
            Exchange::IMemoryExtended* extended = _memory->QueryInterface<Exchange::IMemoryExtended>();
            if (extended != nullptr) {
                Exchange::IMemoryExtended::IStringIterator* iterator = nullptr;
                if ((extended->Processes(iterator) == Core::ERROR_NONE ) && ( iterator != nullptr)) {
                    string processname;
                    while (iterator->Next(processname) == true) {
                        if (processname == webProcessName) {
                            VARIABLE_IS_NOT_USED uint32_t result =  extended->Process(webProcessName, _processmemory);
                            ASSERT( ( result == Core::ERROR_NONE ) && (_processmemory != nullptr ));
                            break;
                        }
                    }
                    iterator->Release();
                }
                extended->Release();
            }
        }
    }

    void Disable() override 
    {
        if (_service != nullptr) {
            _service->Release();
            _service = nullptr;
        }
        if (_memory != nullptr) {
            _memory->Release();
            _memory = nullptr;
        }
        if (_processmemory != nullptr) {
            _processmemory->Release();
            _processmemory = nullptr;
        }
    }

    void Activated() 
    {
        _timeIdleFirstStart = Core::Time::Now().Ticks();
        _timePluginStart = Core::Time::Now().Ticks();
    }

    void Deactivated(const uint32_t) override 
    {
        PluginHost::IShell::reason reason = _service->Reason();
        if (reason == PluginHost::IShell::FAILURE || reason == PluginHost::IShell::MEMORY_EXCEEDED) {
            SYSLOG(Logging::Notification, (_T("Browser::Deactivated ( \"URL\": %s , \"Reason\": %d )"), getHostName(_lastURL).c_str(), reason));
            string eventName("BrowserDeactivation_accum");
            string eventValue;
            JsonArray array;

            array.Add(getHostName(_lastURL).c_str());
            array.Add(reason);
            array.ToString(eventValue);
            Utils::Telemetry::sendMessage((char *)eventName.c_str(), (char *)eventValue.c_str());
        }
    }

    void Resumed() override
    {
    }

    void Suspended() override
    {
        _timeIdleFirstStart = Core::Time::Now().Ticks();
    }

    string getHostName(string _URL){
        std::size_t startIdx = _URL.find("://");
        if (startIdx == std::string::npos) {
            return _URL;
        }
        else {
            startIdx += 3; // skip "://"
            size_t endIdx = _URL.find("/",startIdx);
            if (endIdx == std::string::npos) {
                return _URL.substr(startIdx);
            }
            else {
                return _URL.substr(startIdx, endIdx - startIdx);
            }
        }
    }

    void LoadFinished(const string& URL, const int32_t, const bool success, const uint32_t totalsuccess, const uint32_t totalfailed) override 
    {
        if (URL != startURL) {
            _adminLock.Lock();
            URLLoadedMetrics metrics(_urloadmetrics);
            _adminLock.Unlock();
                        
            uint64_t urllaunchtime_ms = ( ( Core::Time::Now().Ticks() - metrics.StartLoad() ) / Core::Time::TicksPerMillisecond);

            if (strcmp(getHostName(URL).c_str(), _lastLoggedApp.c_str())) {
                _didLogLaunchMetrics = false;
            }

            OutputLoadFinishedMetrics(metrics, getHostName(URL), urllaunchtime_ms, success, totalsuccess + totalfailed);

            _timeIdleFirstStart = 0; // we only measure on first non about:blank url handling
        }
    }

    void URLChange(const string& URL, const bool) override 
    {
        if (URL != startURL) {
            URLLoadedMetrics metrics;
            Core::SystemInfo::MemorySnapshot snapshot = Core::SystemInfo::Instance().TakeMemorySnapshot();
            metrics.Total(snapshot.Total());
            metrics.Free(snapshot.Free());
            metrics.Swapped(snapshot.SwapTotal()-snapshot.SwapFree());
            metrics.Uptime(Core::SystemInfo::Instance().GetUpTime());
            metrics.AverageLoad()[0] = Core::SystemInfo::Instance().GetCpuLoadAvg()[0];
            metrics.AverageLoad()[1] = Core::SystemInfo::Instance().GetCpuLoadAvg()[1];
            metrics.AverageLoad()[2] = Core::SystemInfo::Instance().GetCpuLoadAvg()[2];

            uint64_t resident = 0;
            if (_processmemory != nullptr) {
                resident = _processmemory->Resident();
                uint32_t pid = _processmemory->Identifier();
                if (pid != 0) {
                    metrics.StatmLine(GetProcessStatmLine(pid));
                }
            } else if (_memory != nullptr) {
                resident = _memory->Resident();
            }

            metrics.RSSMemProcess(resident);
            metrics.StartLoad(Core::Time::Now().Ticks());
            metrics.IdleTime((Core::Time::Now().Ticks() - _timeIdleFirstStart) / Core::Time::MicroSecondsPerSecond );

            uint64_t timeLaunched = 0;
            timeLaunched = (Core::Time::Now().Ticks() - _timePluginStart) / Core::Time::MicroSecondsPerSecond;
            if (timeLaunched < 2) {
                metrics.SetColdLaunch(true);
            }

            _adminLock.Lock();
            _urloadmetrics = std::move(metrics);
            _adminLock.Unlock();
            _lastURL = URL;
        }
    }

    void VisibilityChange(const bool) override 
    {
    }
    void PageClosure() override 
    {
    }
private:
    void OutputLoadFinishedMetrics(const URLLoadedMetrics& urloadedmetrics, 
                                   const string& URL, 
                                   const uint64_t urllaunchtime_ms,
                                   const bool success, 
                                   const uint32_t totalloaded) 
    {
        if (_didLogLaunchMetrics) {
            return;
        }

        MetricsAsJson output;

        output.Mode = ( urloadedmetrics.IsColdLaunch() == true ? ModeType::COLD : ModeType::WARM );
        output.AppType = _T("Web");
        output.MemoryTotal = urloadedmetrics.Total()/(1024);
        output.MemoryFree = urloadedmetrics.Free()/(1024);
        output.MemorySwapped = urloadedmetrics.Swapped()/(1024);
        output.Uptime = urloadedmetrics.Uptime();

        static const float LA_SCALE = static_cast<float>(1 << SI_LOAD_SHIFT);
        output.LoadAvarage = std::to_string(urloadedmetrics.AverageLoad()[0] / LA_SCALE).substr(0,4) + " " +
                             std::to_string(urloadedmetrics.AverageLoad()[1] / LA_SCALE).substr(0,4) + " " +
                             std::to_string(urloadedmetrics.AverageLoad()[2] / LA_SCALE).substr(0,4);

        static const long NPROC_ONLN = sysconf(_SC_NPROCESSORS_ONLN);
        output.NbrProcessors = NPROC_ONLN;

        output.ProcessRSS = urloadedmetrics.RSSMemProcess();

        uint32_t pid = 0;
        if (_processmemory != nullptr) {
            pid = _processmemory->Identifier();
        }
        output.ProcessPID = pid;

        output.Appname = URL;
        output.StatmLine = urloadedmetrics.StatmLine();
        output.IdleTime = urloadedmetrics.IdleTime();
        output.LaunchTime = urllaunchtime_ms;
        output.LoadSuccess = success;
        output.NbrLoaded = totalloaded;
        output.Callsign = _callsign;

        string outputstring;
        output.ToString(outputstring);
        JsonArray array;
        JsonObject const metrics(outputstring);
        JsonObject::Iterator it = metrics.Variants();
        while (it.Next()) {
            array.Add(it.Current().String().c_str());
        }
        string eventValue;
        array.ToString(eventValue);

        string eventName("LaunchMetrics_accum");

        Utils::Telemetry::sendMessage((char *)eventName.c_str(), (char *)eventValue.c_str());

        SYSLOG(Logging::Notification, (_T( "%s Launch Metrics: %s "), _callsign.c_str(), outputstring.c_str()));
        _didLogLaunchMetrics = true;
        _lastLoggedApp = URL;
    }

    string GetProcessStatmLine(const uint32_t pid) 
    {
        string statmLine;
        std::string procPath = std::string("/proc/") + std::to_string(pid) + "/statm";
        std::ifstream statmStream(procPath);
        if (statmStream.is_open() ) {
            std::getline(statmStream, statmLine);
            statmStream.close();
        }

        return  statmLine;
    }

private:
    string _callsign;
    PluginHost::IShell* _service;
    Exchange::IMemory* _memory;
    Exchange::IProcessMemory* _processmemory;
    bool _cold;
    URLLoadedMetrics _urloadmetrics;
    Core::Time::microsecondsfromepoch _timeIdleFirstStart;
    Core::Time::microsecondsfromepoch _timePluginStart;
    mutable Core::CriticalSection _adminLock;
    string _lastURL;
    bool _didLogLaunchMetrics;
    string _lastLoggedApp;
};

constexpr char SysLogOuput::webProcessName[];

template<class LOGGERINTERFACE>
std::unique_ptr<LOGGERINTERFACE> PerformanceMetrics::LoggerFactory() {
    return std::unique_ptr<LOGGERINTERFACE>(new SysLogOuput());
}

template std::unique_ptr<PerformanceMetrics::IBasicMetricsLogger> PerformanceMetrics::LoggerFactory<PerformanceMetrics::IBasicMetricsLogger>();                
template std::unique_ptr<PerformanceMetrics::IStateMetricsLogger> PerformanceMetrics::LoggerFactory<PerformanceMetrics::IStateMetricsLogger>();                
template std::unique_ptr<PerformanceMetrics::IBrowserMetricsLogger> PerformanceMetrics::LoggerFactory<PerformanceMetrics::IBrowserMetricsLogger>();                

}

ENUM_CONVERSION_BEGIN(WPEFramework::Plugin::SysLogOuput::ModeType)

    { Plugin::SysLogOuput::ModeType::COLD, _TXT("Cold") },
    { Plugin::SysLogOuput::ModeType::WARM, _TXT("Warm") },

ENUM_CONVERSION_END(Plugin::SysLogOuput::ModeType);

}

