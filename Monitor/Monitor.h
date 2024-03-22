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

#ifndef __MONITOR_H
#define __MONITOR_H

#include "Module.h"
#include <interfaces/IMemory.h>
#include <interfaces/json/JsonData_Monitor.h>
#include <limits>
#include <string>

static uint32_t gcd(uint32_t a, uint32_t b)
{
    return b == 0 ? a : gcd(b, a % b);
}

namespace WPEFramework {
namespace Plugin {

    class Monitor : public PluginHost::IPlugin, public PluginHost::IWeb, public PluginHost::JSONRPC {
    private:
        class RestartInfo : public Core::JSON::Container {
        public:
            RestartInfo& operator=(const RestartInfo&) = delete;

            RestartInfo()
                : Core::JSON::Container()
            {
                Add(_T("window"), &Window);
                Add(_T("limit"), &Limit);
            }
            RestartInfo(const RestartInfo& copy)
                : Core::JSON::Container()
                , Window(copy.Window)
                , Limit(copy.Limit)
            {
                Add(_T("window"), &Window);
                Add(_T("limit"), &Limit);
            }
            virtual ~RestartInfo()
            {
            }

            Core::JSON::DecUInt16 Window;
            Core::JSON::DecUInt8 Limit;
        };

    public:
        class MetaData {
        public:
            MetaData()
                : _resident()
                , _allocated()
                , _shared()
                , _process()
            {
            }
            MetaData(const MetaData& copy)
                : _resident(copy._resident)
                , _allocated(copy._allocated)
                , _shared(copy._shared)
                , _process(copy._process)
            {
            }
            ~MetaData()
            {
            }

            MetaData& operator= (const MetaData& rhs)
            {
                _resident = rhs._resident;
                _allocated = rhs._allocated;
                _shared = rhs._shared;
                _process = rhs._process;

                return (*this);
            }

        public:
            bool HasMeasurements() const {
                return ((_resident.Measurements() != 0) || (_allocated.Measurements() != 0) || (_shared.Measurements() != 0) || (_process.Measurements() != 0));
            }

            void AddMeasurements(const uint64_t resident, const uint64_t allocated, const uint64_t shared, const uint64_t process) {
                _resident.Set(resident);
                _allocated.Set(allocated);
                _shared.Set(shared);
                _process.Set(process);
            }

            void Measure(Exchange::IMemory* memInterface)
            {
                ASSERT(memInterface != nullptr);
                _resident.Set(memInterface->Resident());
                _allocated.Set(memInterface->Allocated());
                _shared.Set(memInterface->Shared());
                _process.Set(memInterface->Processes());
            }
            void Reset()
            {
                _resident.Reset();
                _allocated.Reset();
                _shared.Reset();
                _process.Reset();
            }

        public:
            inline const Core::MeasurementType<uint64_t>& Resident() const
            {
                return (_resident);
            }
            inline const Core::MeasurementType<uint64_t>& Allocated() const
            {
                return (_allocated);
            }
            inline const Core::MeasurementType<uint64_t>& Shared() const
            {
                return (_shared);
            }
            inline const Core::MeasurementType<uint8_t>& Process() const
            {
                return (_process);
            }
        private:
            Core::MeasurementType<uint64_t> _resident;
            Core::MeasurementType<uint64_t> _allocated;
            Core::MeasurementType<uint64_t> _shared;
            Core::MeasurementType<uint8_t> _process;
        };

        class Data : public Core::JSON::Container {
        public:
            class MetaData : public Core::JSON::Container {
            public:
                class Measurement : public Core::JSON::Container {
                public:
                    Measurement()
                        : Core::JSON::Container()
                    {
                        Add(_T("min"), &Min);
                        Add(_T("max"), &Max);
                        Add(_T("average"), &Average);
                        Add(_T("last"), &Last);
                    }
                    Measurement(const uint64_t min, const uint64_t max, const uint64_t average, const uint64_t last)
                        : Core::JSON::Container()
                    {
                        Add(_T("min"), &Min);
                        Add(_T("max"), &Max);
                        Add(_T("average"), &Average);
                        Add(_T("last"), &Last);

                        Min = min;
                        Max = max;
                        Average = average;
                        Last = last;
                    }
                    Measurement(const Core::MeasurementType<uint64_t>& input)
                        : Core::JSON::Container()
                    {
                        Add(_T("min"), &Min);
                        Add(_T("max"), &Max);
                        Add(_T("average"), &Average);
                        Add(_T("last"), &Last);

                        Min = input.Min();
                        Max = input.Max();
                        Average = input.Average();
                        Last = input.Last();
                    }
                    Measurement(const Core::MeasurementType<uint8_t>& input)
                        : Core::JSON::Container()
                    {
                        Add(_T("min"), &Min);
                        Add(_T("max"), &Max);
                        Add(_T("average"), &Average);
                        Add(_T("last"), &Last);

                        Min = input.Min();
                        Max = input.Max();
                        Average = input.Average();
                        Last = input.Last();
                    }
                    Measurement(const Measurement& copy)
                        : Core::JSON::Container()
                        , Min(copy.Min)
                        , Max(copy.Max)
                        , Average(copy.Average)
                        , Last(copy.Last)
                    {
                        Add(_T("min"), &Min);
                        Add(_T("max"), &Max);
                        Add(_T("average"), &Average);
                        Add(_T("last"), &Last);
                    }
                    ~Measurement()
                    {
                    }

                public:
                    Measurement& operator=(const Measurement& RHS)
                    {
                        Min = RHS.Min;
                        Max = RHS.Max;
                        Average = RHS.Average;
                        Last = RHS.Last;

                        return (*this);
                    }
                    Measurement& operator=(const Core::MeasurementType<uint64_t>& RHS)
                    {
                        Min = RHS.Min();
                        Max = RHS.Max();
                        Average = RHS.Average();
                        Last = RHS.Last();

                        return (*this);
                    }

                public:
                    Core::JSON::DecUInt64 Min;
                    Core::JSON::DecUInt64 Max;
                    Core::JSON::DecUInt64 Average;
                    Core::JSON::DecUInt64 Last;
                };

            public:
                MetaData()
                    : Core::JSON::Container()
                    , Allocated()
                    , Resident()
                    , Shared()
                    , Process()
                    , Operational()
                    , Count()
                {
                    Add(_T("allocated"), &Allocated);
                    Add(_T("resident"), &Resident);
                    Add(_T("shared"), &Shared);
                    Add(_T("process"), &Process);
                    Add(_T("operational"), &Operational);
                    Add(_T("count"), &Count);
                }
                MetaData(const Monitor::MetaData& input, const bool operational)
                    : Core::JSON::Container()
                {
                    Add(_T("allocated"), &Allocated);
                    Add(_T("resident"), &Resident);
                    Add(_T("shared"), &Shared);
                    Add(_T("process"), &Process);
                    Add(_T("operational"), &Operational);
                    Add(_T("count"), &Count);

                    Allocated = input.Allocated();
                    Resident = input.Resident();
                    Shared = input.Shared();
                    Process = input.Process();
                    Operational = operational;
                    Count = input.Allocated().Measurements();
                }
                MetaData(const MetaData& copy)
                    : Core::JSON::Container()
                    , Allocated(copy.Allocated)
                    , Resident(copy.Resident)
                    , Shared(copy.Shared)
                    , Process(copy.Process)
                    , Operational(copy.Operational)
                    , Count(copy.Count)
                {
                    Add(_T("allocated"), &Allocated);
                    Add(_T("resident"), &Resident);
                    Add(_T("shared"), &Shared);
                    Add(_T("process"), &Process);
                    Add(_T("operational"), &Operational);
                    Add(_T("count"), &Count);
                }
                ~MetaData()
                {
                }

                MetaData& operator=(const MetaData& RHS)
                {
                    Allocated = RHS.Allocated;
                    Resident = RHS.Resident;
                    Shared = RHS.Shared;
                    Process = RHS.Process;
                    Operational = RHS.Operational;
                    Count = RHS.Count;

                    return (*this);
                }

            public:
                Measurement Allocated;
                Measurement Resident;
                Measurement Shared;
                Measurement Process;
                Core::JSON::Boolean Operational;
                Core::JSON::DecUInt32 Count;
            };

        private:
            Data& operator=(const Data&);

        public:
            Data()
                : Core::JSON::Container()
                , Name()
                , Measurement()
                , Observable()
                , Restart()
            {
                Add(_T("name"), &Name);
                Add(_T("measurment"), &Measurement);
                Add(_T("observable"), &Observable);
                Add(_T("restart"), &Restart);
            }
            Data(const string& name, const Monitor::MetaData& info, const bool operational)
                : Core::JSON::Container()
                , Name()
                , Measurement(info, operational)
                , Observable()
                , Restart()
            {
                Add(_T("name"), &Name);
                Add(_T("measurment"), &Measurement);
                Add(_T("observable"), &Observable);
                Add(_T("restart"), &Restart);

                Name = name;
            }
            Data(const Data& copy)
                : Core::JSON::Container()
                , Name(copy.Name)
                , Measurement(copy.Measurement)
                , Observable(copy.Observable)
                , Restart(copy.Restart)
            {
                Add(_T("name"), &Name);
                Add(_T("measurment"), &Measurement);
                Add(_T("observable"), &Observable);
                Add(_T("restart"), &Restart);
            }
            ~Data()
            {
            }

        public:
            Core::JSON::String Name;
            MetaData Measurement;
            Core::JSON::String Observable;
            RestartInfo Restart;
        };

    private:
        Monitor(const Monitor&);
        Monitor& operator=(const Monitor&);

        class Config : public Core::JSON::Container {
        private:
            Config(const Config&);
            Config& operator=(const Config&);

        public:
            class Entry : public Core::JSON::Container {
            private:
                Entry& operator=(const Entry& RHS);

            public:
                Entry()
                    : Core::JSON::Container()
                {
                    Add(_T("callsign"), &Callsign);
                    Add(_T("memory"), &MetaData);
                    Add(_T("memorylimit"), &MetaDataLimit);
                    Add(_T("operational"), &Operational);
                    Add(_T("restart"), &Restart);
                }
                Entry(const Entry& copy)
                    : Core::JSON::Container()
                    , Callsign(copy.Callsign)
                    , MetaData(copy.MetaData)
                    , MetaDataLimit(copy.MetaDataLimit)
                    , Operational(copy.Operational)
                    , Restart(copy.Restart)
                {
                    Add(_T("callsign"), &Callsign);
                    Add(_T("memory"), &MetaData);
                    Add(_T("memorylimit"), &MetaDataLimit);
                    Add(_T("operational"), &Operational);
                    Add(_T("restart"), &Restart);
                }
                ~Entry()
                {
                }

            public:
                Core::JSON::String Callsign;
                Core::JSON::DecUInt32 MetaData;
                Core::JSON::DecUInt32 MetaDataLimit;
                Core::JSON::DecSInt32 Operational;
                RestartInfo Restart;
            };

        public:
            Config()
                : Core::JSON::Container()
            {
                Add(_T("observables"), &Observables);
            }
            ~Config()
            {
            }

        public:
            Core::JSON::ArrayType<Entry> Observables;
        };

        class MonitorObjects : public PluginHost::IPlugin::INotification {
        public:
            using Job = Core::ThreadPool::JobType<MonitorObjects>;

            class MonitorObject {
            public:
                MonitorObject() = delete;
                MonitorObject& operator=(const MonitorObject&) = delete;

                enum evaluation {
                    SUCCESFULL = 0x00,
                    NOT_OPERATIONAL = 0x01,
                    EXCEEDED_MEMORY = 0x02
                };

                typedef struct {
                    int32_t Limit;
                    int32_t WindowSeconds;
                } RestartSettings;

            public:
                MonitorObject(
                    const bool actOnOperational,
                    const uint32_t operationalInterval,
                    const uint32_t memoryInterval,
                    const uint64_t memoryThreshold,
                    const uint64_t absTime,
                    const uint16_t restartWindow,
                    const uint8_t restartLimit)
                    : _operationalInterval(operationalInterval)
                    , _memoryInterval(memoryInterval)
                    , _memoryThreshold(memoryThreshold * 1024)
                    , _operationalSlots(operationalInterval)
                    , _memorySlots(memoryInterval)
                    , _nextSlot(absTime)
                    , _restartWindow(restartWindow)
                    , _restartWindowStart()
                    , _restartCount(0)
                    , _restartLimit(restartLimit)
                    , _measurement()
                    , _operational(false)
                    , _operationalEvaluate(actOnOperational)
                    , _source(nullptr)
                    , _interval( ((operationalInterval != 0) || (_memoryInterval != 0)) ? gcd(_operationalInterval, _memoryInterval) : 0 )
                    , _active{ false }
                    , _adminLock()
                {
                    ASSERT((_operationalInterval != 0) || (_memoryInterval != 0));
                }
                ~MonitorObject()
                {
                    if (_source != nullptr) {
                        _source->Release();
                        _source = nullptr;
                    }
                }

                MonitorObject(MonitorObject&) = delete;
                MonitorObject& operator=(MonitorObject&) = delete;
                MonitorObject(MonitorObject&&) = delete;
                MonitorObject& operator=(MonitorObject&&) = delete;

            public:
                inline bool RegisterRestart(PluginHost::IShell::reason why VARIABLE_IS_NOT_USED)
                {
                    ASSERT(why == PluginHost::IShell::MEMORY_EXCEEDED || why == PluginHost::IShell::FAILURE);
                    ASSERT(HasRestartAllowed());

                    if (((_restartWindowStart.IsValid() == true) && (_restartWindowStart > Core::Time::Now())) || (_restartWindow == 0)) {
                        // It's within window.
                        _restartCount += _restartCount + 1;
                    } else {
                        _restartWindowStart = Core::Time::Now().Add(_restartWindow * 1000 /* ms */);
                        _restartCount = 0;
                    }
                    
                    bool result = ((_restartLimit == 0) || (_restartCount < _restartLimit));
                    if (result == false) {
                        _restartCount = 0;
                    }

                    return result;
                }
                inline uint8_t RestartLimit() const
                {
                    return _restartLimit;
                }
                inline uint16_t RestartWindow() const
                {
                    return _restartWindow;
                }
                inline void UpdateRestartLimits(
                    const uint16_t restartWindow,
                    const uint8_t restartLimit)
                {
                    _restartWindow = restartWindow;
                    _restartLimit = restartLimit;
                }
                inline bool HasRestartAllowed() const
                {
                    return (_operationalEvaluate);
                }
                inline uint32_t Interval() const
                {
                    return (_interval);
                }
                inline uint32_t Operational() const
                {
                    return (_operational);
                }
                inline const MetaData& Measurement() const
                {
                    Core::SafeSyncType<Core::CriticalSection> guard(_adminLock);
                    return (_measurement);
                }
                inline uint64_t TimeSlot() const
                {
                    return (_nextSlot);
                }
                inline void Reset()
                {
                    Core::SafeSyncType<Core::CriticalSection> guard(_adminLock);
                    _measurement.Reset();
                }
                inline void Retrigger(uint64_t currentSlot)
                {
                    while (_nextSlot < currentSlot) {
                        _nextSlot += _interval;
                    }
                }
                inline void Set(Exchange::IMemory* memory)
                {
                    _adminLock.Lock();
                    if (_source != nullptr) {
                        _source->Release();
                        _source = nullptr;
                    }

                    if (memory != nullptr) {
                        _source = memory;
                        _source->AddRef();
                    }
                    _adminLock.Unlock();

                    _operational = (memory != nullptr);
                }

                Core::ProxyType<const Exchange::IMemory> Source() const
                {
                    Core::ProxyType<const Exchange::IMemory> source;
                    _adminLock.Lock();
                    if (_source != nullptr) {
#ifdef USE_THUNDER_R4
                        source = Core::ProxyType<const Exchange::IMemory>(*_source, *_source);
#else
                        source = Core::ProxyType<const Exchange::IMemory>(*_source);
#endif
                    }
                    _adminLock.Unlock();
                    return source;
                }

                inline uint32_t Evaluate()
                {
                    Core::ProxyType<const Exchange::IMemory> source = Source();

                    uint32_t status(SUCCESFULL);
                    if (source.IsValid() == true) {
                        _operationalSlots -= _interval;
                        _memorySlots -= _interval;

                        if ((_operationalInterval != 0) && (_operationalSlots == 0)) {
                            _operational = source->IsOperational();
                            if (_operational == false) {
                                status |= NOT_OPERATIONAL;
                                TRACE(Trace::Error, (_T("Status not operational. %d"), __LINE__));
                            }
                            _operationalSlots = _operationalInterval;
                        }
                        if ((_memoryInterval != 0) && (_memorySlots == 0)) {

                            uint64_t resident = source->Resident();
                            uint64_t allocated = source->Allocated();
                            uint64_t shared = source->Shared();
                            uint64_t  process = source->Processes();

                            _adminLock.Lock();
                            _measurement.AddMeasurements(resident, allocated, shared, process);
                            _adminLock.Unlock();

                            if ((_memoryThreshold != 0) && (resident > _memoryThreshold)) {
                                status |= EXCEEDED_MEMORY;
                                TRACE(Trace::Error, (_T("Status MetaData Exceeded. %d"), __LINE__));
                            }
                            _memorySlots = _memoryInterval;
                        }
                    }
                    return (status);
                }

                bool IsActive() const { return _active; }
                void Active(bool active) { _active = active; }

            private:
                const uint32_t _operationalInterval; //!< Interval (s) to check the monitored processes
                const uint32_t _memoryInterval; //!<  Interval (s) for a memory measurement.
                const uint64_t _memoryThreshold; //!< MetaData threshold in bytes for all processes.
                uint32_t _operationalSlots; // does not need protection, only touched in job evaluate
                uint32_t _memorySlots; // does not need protection, only touched in job evaluate
                std::atomic<uint64_t> _nextSlot; // no ordering needed, atomic should suffice
                std::atomic<uint16_t> _restartWindow; // no ordering needed, atomic should suffice
                Core::Time _restartWindowStart; // only used in job (indirectly), no protection needed
                uint32_t _restartCount; // only used in job (indirectly), no protection needed
                std::atomic<uint8_t> _restartLimit; // no ordering needed, atomic should suffice
                MetaData _measurement;
                std::atomic<bool> _operational; // no ordering needed, atomic should suffice
                const bool _operationalEvaluate;
                Exchange::IMemory* _source;
                const uint32_t _interval; //!< The greatest possible interval to check both memory and processes.
                std::atomic<bool> _active;
                mutable Core::CriticalSection _adminLock;
            };

        public:
            MonitorObjects(const MonitorObjects&) = delete;
            MonitorObjects& operator=(const MonitorObjects&) = delete;

#ifdef __WINDOWS__
#pragma warning(disable : 4355)
#endif
            MonitorObjects(Monitor* parent)
                : _monitor()
                , _job(*this)
                , _service(nullptr)
                , _parent(*parent)
            {
            }
#ifdef __WINDOWS__
#pragma warning(default : 4355)
#endif
            ~MonitorObjects() override
            {
                ASSERT(_monitor.size() == 0);
            }

        public:
            inline uint32_t Length() const
            {
                return (static_cast<uint32_t>(_monitor.size()));
            }
            inline void Update(
                const string& observable,
                const uint16_t restartWindow,
                const uint8_t restartLimit)
            {
                MonitorObjectContainer::iterator index(_monitor.find(observable));
                if (index != _monitor.end()) {
                    index->second.UpdateRestartLimits(
                        restartWindow,
                        restartLimit);
                }
            }
            inline void Open(PluginHost::IShell* service, Core::JSON::ArrayType<Config::Entry>::Iterator& index)
            {
                ASSERT((service != nullptr) && (_service == nullptr));

                uint64_t baseTime = Core::Time::Now().Ticks();

                _service = service;
                _service->AddRef();

                while (index.Next() == true) {
                    Config::Entry& element(index.Current());
                    string callSign(element.Callsign.Value());
                    uint64_t memoryThreshold(element.MetaDataLimit.Value());
                    uint32_t interval = abs(element.Operational.Value());
                    interval = interval * 1000 * 1000; // Move from Seconds to MicroSecond
                    uint32_t memory(element.MetaData.Value() * 1000 * 1000); // Move from Seconds to MicroSeconds
                    uint16_t restartWindow = 0;
                    uint8_t restartLimit = 0;

                    if (element.Restart.IsSet()) {
                        restartWindow = element.Restart.Window;
                        restartLimit = element.Restart.Limit;
                    }
                    SYSLOG(Logging::Startup, (_T("Monitoring: %s (%d,%d)."), callSign.c_str(), (interval / 1000000), (memory / 1000000)));
                    if ((interval != 0) || (memory != 0)) {

                        _monitor.emplace(std::piecewise_construct,
                                         std::forward_as_tuple(callSign),
                                         std::forward_as_tuple(
                                            element.Operational.Value() >= 0,
                                            interval,
                                            memory,
                                            memoryThreshold,
                                            baseTime,
                                            restartWindow,
                                            restartLimit)
                                    );
                    }
                }

                _job.Submit();
            }
            inline void Close()
            {
                ASSERT(_service != nullptr);

                _job.Revoke();

                _monitor.clear();
                _service->Release();
                _service = nullptr;
            }

#ifndef USE_THUNDER_R4
            void StateChange(PluginHost::IShell* service) override
            {

                MonitorObjectContainer::iterator index(_monitor.find(service->Callsign()));

                if (index != _monitor.end()) {

                    // Only act on Activated or Deactivated...
                    PluginHost::IShell::state currentState(service->State());

                    if (currentState == PluginHost::IShell::ACTIVATED) {
                        bool is_active = index->second.IsActive();
                        index->second.Active(true);
                        if (is_active == false && std::count_if(_monitor.begin(), _monitor.end(), [](const std::pair<const string, MonitorObject>& v) {
                                return v.second.IsActive();
                            }) == 1) {

                            // A monitor which previously was stopped restarting is being activated.
                            // Moreover it's the only only which now becomes active. This means probing
                            // has to be activated as well since it was stopped at point the last observee
                            // turned inactive
                            _job.Schedule(Core::Time::Now());

                            TRACE(Trace::Information, (_T("Starting to probe as active observee appeared.")));
                        }

                        // Get the MetaData interface
                        Exchange::IMemory* memory = service->QueryInterface<Exchange::IMemory>();

                        if (memory != nullptr) {
                            index->second.Set(memory);
                            memory->Release();
                        }
                    } else if (currentState == PluginHost::IShell::DEACTIVATION) {
                        index->second.Set(nullptr);
                    } else if ((currentState == PluginHost::IShell::DEACTIVATED)) {
                        index->second.Active(false);
                        if ((index->second.HasRestartAllowed() == true) && ((service->Reason() == PluginHost::IShell::MEMORY_EXCEEDED) || (service->Reason() == PluginHost::IShell::FAILURE))) {
                            if (index->second.RegisterRestart(service->Reason()) == false) {
                                TRACE(Trace::Fatal, (_T("Giving up restarting of %s: Failed more than %d times within %d seconds."), service->Callsign().c_str(), index->second.RestartLimit(), index->second.RestartWindow()));
                                const string message("{\"callsign\": \"" + service->Callsign() + "\", \"action\": \"Restart\", \"reason\":\"" + (std::to_string(index->second.RestartLimit())).c_str() + " Attempts Failed within the restart window\"}");
                                _service->Notify(message);
                                _parent.event_action(service->Callsign(), "StoppedRestaring", std::to_string(index->second.RestartLimit()) + " attempts failed within the restart window");
                            } else {
                                const string message("{\"callsign\": \"" + service->Callsign() + "\", \"action\": \"Activate\", \"reason\": \"Automatic\" }");
                                _service->Notify(message);
                                _parent.event_action(service->Callsign(), "Activate", "Automatic");
                                TRACE(Trace::Error, (_T("Restarting %s again because we detected it misbehaved."), service->Callsign().c_str()));

                                Core::IWorkerPool::Instance().Schedule(
                                    Core::Time::Now(),
                                    PluginHost::IShell::Job::Create(
                                        service,
                                        PluginHost::IShell::ACTIVATED,
                                        PluginHost::IShell::AUTOMATIC));
                            }
                        }
                    }
                }
            }

#else
            void Activated (const string& callsign, PluginHost::IShell* service) override
            {
                MonitorObjectContainer::iterator index(_monitor.find(callsign));

                if (index != _monitor.end()) {

                    index->second.Active(true);

                    // Get the MetaData interface
                    Exchange::IMemory* memory = service->QueryInterface<Exchange::IMemory>();

                    if (memory != nullptr) {
                        index->second.Set(memory);
                        memory->Release();
                    }

                    if (_job.Submit() == true) {
                        TRACE(Trace::Information, (_T("Starting to probe as active observee appeared.")));
                    }
                }
            }
            void Deactivated (const string& callsign, PluginHost::IShell* service) override
            {
                MonitorObjectContainer::iterator index(_monitor.find(callsign));

                if (index != _monitor.end()) {

                    index->second.Set(nullptr);
                    index->second.Active(false);

                    PluginHost::IShell::reason reason = service->Reason();

                    if ((index->second.HasRestartAllowed() == true) && ((reason == PluginHost::IShell::MEMORY_EXCEEDED) || (reason == PluginHost::IShell::FAILURE))) {
                        if (index->second.RegisterRestart(reason) == false) {
                            uint8_t restartlimit = index->second.RestartLimit();
                            uint16_t restartwindow = index->second.RestartWindow();
                            TRACE(Trace::Fatal, (_T("Giving up restarting of %s: Failed more than %d times within %d seconds."), callsign.c_str(), restartlimit, restartwindow));
                            const string message("{\"callsign\": \"" + callsign + "\", \"action\": \"Restart\", \"reason\":\"" + (std::to_string(restartlimit)).c_str() + " Attempts Failed within the restart window\"}");
                            _service->Notify(message);
                            _parent.event_action(callsign, "StoppedRestaring", std::to_string(index->second.RestartLimit()) + " attempts failed within the restart window");
                        } else {
                            const string message("{\"callsign\": \"" + callsign + "\", \"action\": \"Activate\", \"reason\": \"Automatic\" }");
                            _service->Notify(message);
                            _parent.event_action(callsign, "Activate", "Automatic");
                            TRACE(Trace::Error, (_T("Restarting %s again because we detected it misbehaved."), callsign.c_str()));
                            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(service, PluginHost::IShell::ACTIVATED, PluginHost::IShell::AUTOMATIC));
                        }
                    }
                }
            }
            void Unavailable(const string&, PluginHost::IShell*) override
            {
            }
#endif
            void Snapshot(Core::JSON::ArrayType<Monitor::Data>& snapshot) const
            {
                MonitorObjectContainer::const_iterator element(_monitor.cbegin());

                // Go through the list of observations...
                while (element != _monitor.cend()) {
                    MetaData data = element->second.Measurement();
                    if (data.HasMeasurements() == true) {
                        snapshot.Add(Monitor::Data(element->first, data, element->second.Operational()));
                    }
                    element++;
                }

            }
            bool Snapshot(const string& name, Monitor::MetaData& result, bool& operational) const
            {
                bool found = false;


                MonitorObjectContainer::const_iterator index(_monitor.find(name));

                if (index != _monitor.cend()) {
                    MetaData data = index->second.Measurement();
                    if (data.HasMeasurements() == true) {
                        result = data;
                        operational = index->second.Operational();
                        found = true;
                    }
                }

                return (found);
            }

            void AddElementToResponse( Core::JSON::ArrayType<JsonData::Monitor::InfoInfo>& response, const string& callsign, const MonitorObject& object) const {
                const MetaData& metaData = object.Measurement();
                JsonData::Monitor::InfoInfo info;
                info.Observable = callsign;

                if (object.HasRestartAllowed()) {
                    info.Restart.Limit = object.RestartLimit();
                    info.Restart.Window = object.RestartWindow();
                }

                if (metaData.HasMeasurements() == true) {
                    translate(metaData.Allocated(), &info.Measurements.Allocated);
                    translate(metaData.Resident(), &info.Measurements.Resident);
                    translate(metaData.Shared(), &info.Measurements.Shared);
                    translate(metaData.Process(), &info.Measurements.Process);
                }
                info.Measurements.Operational = object.Operational();
                info.Measurements.Count = metaData.Allocated().Measurements();

                response.Add(info);
            };

            void Snapshot(const string& callsign, Core::JSON::ArrayType<JsonData::Monitor::InfoInfo>* response) const
            {

                ASSERT(response != nullptr);

                if (callsign.empty() == false) {
                    auto element = _monitor.find(callsign);
                    if (element != _monitor.end()) {
                        AddElementToResponse(*response, element->first, element->second);
                    }
                } else {
                    for (auto& element : _monitor) {
                        AddElementToResponse(*response, element.first, element.second);
                    }
                }
            }

            bool Reset(const string& name, Monitor::MetaData& result, bool& operational)
            {
                bool found = false;

                MonitorObjectContainer::iterator index(_monitor.find(name));

                if (index != _monitor.end()) {
                    result = index->second.Measurement();
                    operational = index->second.Operational();
                    index->second.Reset();
                    found = true;
                }

                return (found);
            }

            bool Reset(const string& name)
            {
                bool found = false;

                MonitorObjectContainer::iterator index(_monitor.find(name));

                if (index != _monitor.end()) {
                    index->second.Reset();
                    found = true;
                }

                return (found);
            }

            BEGIN_INTERFACE_MAP(MonitorObjects)
            INTERFACE_ENTRY(PluginHost::IPlugin::INotification)
            END_INTERFACE_MAP

        private:
            friend Core::ThreadPool::JobType<MonitorObjects&>;

            void Dispatch()
            {
                uint64_t scheduledTime(Core::Time::Now().Ticks());
                uint64_t nextSlot(static_cast<uint64_t>(~0));

                MonitorObjectContainer::iterator index(_monitor.begin());

                // Go through the list of pending observations...
                while (index != _monitor.end()) {
                    MonitorObject& info(index->second);
                    if (info.IsActive() == false) {
                        ++index;
                        continue;
                    }

                    if (info.TimeSlot() <= scheduledTime) {
                        uint32_t value(info.Evaluate());

                        if ((value & (MonitorObject::NOT_OPERATIONAL | MonitorObject::EXCEEDED_MEMORY)) != 0) {
                            PluginHost::IShell* plugin(_service->QueryInterfaceByCallsign<PluginHost::IShell>(index->first));

                            if (plugin != nullptr) {
                                Core::EnumerateType<PluginHost::IShell::reason> why(((value & MonitorObject::EXCEEDED_MEMORY) != 0) ? PluginHost::IShell::MEMORY_EXCEEDED : PluginHost::IShell::FAILURE);

                                const string message("{\"callsign\": \"" + plugin->Callsign() + "\", \"action\": \"Deactivate\", \"reason\": \"" + why.Data() + "\" }");
                                SYSLOG(Logging::Fatal, (_T("FORCED Shutdown: %s by reason: %s."), plugin->Callsign().c_str(), why.Data()));

                                _service->Notify(message);

                                _parent.event_action(plugin->Callsign(), "Deactivate", why.Data());

                                Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(plugin, PluginHost::IShell::DEACTIVATED, why.Value()));

                                plugin->Release();
                            }
                        }
                        info.Retrigger(scheduledTime);
                    }

                    if (info.TimeSlot() < nextSlot) {
                        nextSlot = info.TimeSlot();
                    }

                    index++;
                }

                if (nextSlot != static_cast<uint64_t>(~0)) {
                    if (nextSlot < Core::Time::Now().Ticks()) {
                        _job.Submit();
                    } else {
                        nextSlot += 1000 /* Add 1 ms */;
#ifdef USE_THUNDER_R4
                        _job.Reschedule(nextSlot);
#else
                        _job.Schedule(nextSlot);
#endif /* USE_THUNDER_R4 */
                    }
                } else {
                    TRACE(Trace::Information, (_T("Stopping to probe due to lack of active observees.")));
                }
            }

        private:
            template <typename T>
            void translate(const Core::MeasurementType<T>& from, JsonData::Monitor::MeasurementInfo* to) const
            {
                ASSERT(to != nullptr);
                to->Min = from.Min();
                to->Max = from.Max();
                to->Average = from.Average();
                to->Last = from.Last();
            }

        private:

            using MonitorObjectContainer = std::unordered_map<string, MonitorObject>;

            MonitorObjectContainer _monitor;
            Core::WorkerPool::JobType<MonitorObjects&> _job;
            PluginHost::IShell* _service;
            Monitor& _parent;
        };

    public:
#ifdef __WINDOWS__
#pragma warning(disable : 4355)
#endif
        Monitor()
            : _skipURL(0)
            , _monitor(this)
        {
        }
#ifdef __WINDOWS__
#pragma warning(default : 4355)
#endif
        ~Monitor() = default;

        BEGIN_INTERFACE_MAP(Monitor)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IWeb)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        END_INTERFACE_MAP

    public:
        //  IPlugin methods
        // -------------------------------------------------------------------------------------------------------

        // First time initialization. Whenever a plugin is loaded, it is offered a Service object with relevant
        // information and services for this particular plugin. The Service object contains configuration information that
        // can be used to initialize the plugin correctly. If Initialization succeeds, return nothing (empty string)
        // If there is an error, return a string describing the issue why the initialisation failed.
        // The Service object is *NOT* reference counted, lifetime ends if the plugin is deactivated.
        // The lifetime of the Service object is guaranteed till the deinitialize method is called.
        const string Initialize(PluginHost::IShell* service) override;

        // The plugin is unloaded from WPEFramework. This is call allows the module to notify clients
        // or to persist information if needed. After this call the plugin will unlink from the service path
        // and be deactivated. The Service object is the same as passed in during the Initialize.
        // After theis call, the lifetime of the Service object ends.
        void Deinitialize(PluginHost::IShell* service) override;

        // Returns an interface to a JSON struct that can be used to return specific metadata information with respect
        // to this plugin. This Metadata can be used by the MetData plugin to publish this information to the ouside world.
        string Information() const override;

        //  IWeb methods
        // -------------------------------------------------------------------------------------------------------
        // Whenever a request is received, it might carry some additional data in the body. This method allows
        // the plugin to attach a deserializable data object (ref counted) to be loaded with any potential found
        // in the body of the request.
        void Inbound(Web::Request& request) override;

        // If everything is received correctly, the request is passed on to us, through a thread from the thread pool, to
        // do our thing and to return the result in the response object. Here the actual specific module work,
        // based on a a request is handled.
        Core::ProxyType<Web::Response> Process(const Web::Request& request) override;

    private:
        uint8_t _skipURL;
        Config _config;
        Core::Sink<MonitorObjects> _monitor;

    private:
        void RegisterAll();
        void UnregisterAll();
        uint32_t endpoint_restartlimits(const JsonData::Monitor::RestartlimitsParamsData& params);
        uint32_t endpoint_resetstats(const JsonData::Monitor::ResetstatsParamsData& params, JsonData::Monitor::InfoInfo& response);
        uint32_t get_status(const string& index, Core::JSON::ArrayType<JsonData::Monitor::InfoInfo>& response) const;
        void event_action(const string& callsign, const string& action, const string& reason);
    };
}
}

#endif // __MONITOR_H
