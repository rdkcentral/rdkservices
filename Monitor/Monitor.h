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
                , _operational(false)
            {
            }
            MetaData(const MetaData& copy)
                : _resident(copy._resident)
                , _allocated(copy._allocated)
                , _shared(copy._shared)
                , _process(copy._process)
                , _operational(copy._operational)
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
                _operational = rhs._operational;

                return (*this);
            }

        public:
            void Measure(Exchange::IMemory* memInterface)
            {
                _resident.Set(memInterface->Resident());
                _allocated.Set(memInterface->Allocated());
                _shared.Set(memInterface->Shared());
                _process.Set(memInterface->Processes());
            }
            void Operational(const bool operational)
            {
                _operational = operational;
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
            inline bool Operational() const
            {
                return (_operational);
            }

        private:
            Core::MeasurementType<uint64_t> _resident;
            Core::MeasurementType<uint64_t> _allocated;
            Core::MeasurementType<uint64_t> _shared;
            Core::MeasurementType<uint8_t> _process;
            bool _operational;
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
                MetaData(const Monitor::MetaData& input)
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
                    Operational = input.Operational();
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
                MetaData& operator=(const Monitor::MetaData& RHS)
                {
                    Allocated = RHS.Allocated();
                    Resident = RHS.Resident();
                    Shared = RHS.Shared();
                    Process = RHS.Process();
                    Operational = RHS.Operational();
                    Count = RHS.Allocated().Measurements();

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
            Data(const string& name, const Monitor::MetaData& info)
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

                Name = name;
                Measurement = info;
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
                    , _operationalEvaluate(actOnOperational)
                    , _source(nullptr)
                    , _active{ false }
                {
                    ASSERT((_operationalInterval != 0) || (_memoryInterval != 0));
                    _interval = gcd(_operationalInterval, _memoryInterval);
                }
                MonitorObject(const MonitorObject& copy)
                    : _operationalInterval(copy._operationalInterval)
                    , _memoryInterval(copy._memoryInterval)
                    , _memoryThreshold(copy._memoryThreshold)
                    , _operationalSlots(copy._operationalSlots)
                    , _memorySlots(copy._memorySlots)
                    , _nextSlot(copy._nextSlot)
                    , _restartWindow(copy._restartWindow)
                    , _restartWindowStart(copy._restartWindowStart)
                    , _restartCount(copy._restartCount)
                    , _restartLimit(copy._restartLimit)
                    , _measurement(copy._measurement)
                    , _operationalEvaluate(copy._operationalEvaluate)
                    , _source(copy._source)
                    , _interval(copy._interval)
                    , _active{ copy._active }
                {
                    if (_source != nullptr) {
                        _source->AddRef();
                    }
                }
                ~MonitorObject()
                {
                    if (_source != nullptr) {
                        _source->Release();
                        _source = nullptr;
                    }
                }

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
                inline uint8_t RestartLimit()
                {
                    return _restartLimit;
                }
                inline uint16_t RestartWindow()
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
                inline const MetaData& Measurement() const
                {
                    return (_measurement);
                }
                inline bool HasMeasurement() const
                {
                    return (((_measurement.Allocated().Min() == Core::NumberType<uint64_t>::Max()) && 
                    (_measurement.Allocated().Max() == Core::NumberType<uint64_t>::Min())) ? false : true);
                }
                inline uint64_t TimeSlot() const
                {
                    return (_nextSlot);
                }
                inline void Reset()
                {
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
                    if (_source != nullptr) {
                        _source->Release();
                        _source = nullptr;
                    }

                    if (memory != nullptr) {
                        _source = memory;
                        _source->AddRef();
                    }

                    _measurement.Operational(_source != nullptr);
                }
                inline uint32_t Evaluate()
                {
                    uint32_t status(SUCCESFULL);
                    if (_source != nullptr) {
                        _operationalSlots -= _interval;
                        _memorySlots -= _interval;

                        if ((_operationalInterval != 0) && (_operationalSlots == 0)) {
                            bool operational = _source->IsOperational();
                            _measurement.Operational(operational);
                            if (operational == false) {
                                status |= NOT_OPERATIONAL;
                                TRACE(Trace::Error, (_T("Status not operational. %d"), __LINE__));
                            }
                            _operationalSlots = _operationalInterval;
                        }
                        if ((_memoryInterval != 0) && (_memorySlots == 0)) {
                            _measurement.Measure(_source);

                            if ((_memoryThreshold != 0) && (_measurement.Resident().Last() > _memoryThreshold)) {
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
                uint32_t _operationalSlots;
                uint32_t _memorySlots;
                uint64_t _nextSlot;
                uint16_t _restartWindow;
                Core::Time _restartWindowStart;
                uint32_t _restartCount;
                uint8_t _restartLimit;
                MetaData _measurement;
                bool _operationalEvaluate;
                Exchange::IMemory* _source;
                uint32_t _interval; //!< The greatest possible interval to check both memory and processes.
                bool _active;
            };

        public:
            MonitorObjects(const MonitorObjects&) = delete;
            MonitorObjects& operator=(const MonitorObjects&) = delete;

#ifdef __WINDOWS__
#pragma warning(disable : 4355)
#endif
            MonitorObjects(Monitor* parent)
                : _adminLock()
                , _monitor()
                , _job(*this)
                , _service(nullptr)
                , _parent(*parent)
            {
            }
#ifdef __WINDOWS__
#pragma warning(default : 4355)
#endif
            virtual ~MonitorObjects()
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
                _adminLock.Lock();

                std::map<string, MonitorObject>::iterator index(_monitor.find(observable));
                if (index != _monitor.end()) {
                    index->second.UpdateRestartLimits(
                        restartWindow,
                        restartLimit);
                }

                _adminLock.Unlock();
            }
            inline void Open(PluginHost::IShell* service, Core::JSON::ArrayType<Config::Entry>::Iterator& index)
            {
                ASSERT((service != nullptr) && (_service == nullptr));

                uint64_t baseTime = Core::Time::Now().Ticks();

                _service = service;
                _service->AddRef();

                _adminLock.Lock();

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
                        _monitor.insert(
                            std::pair<string, MonitorObject>(callSign, MonitorObject(
                                element.Operational.Value() >= 0, 
                                interval, 
                                memory, 
                                memoryThreshold, 
                                baseTime, 
                                restartWindow, 
                                restartLimit)));
                    }
                }

                _adminLock.Unlock();

                _job.Submit();
            }
            inline void Close()
            {
                ASSERT(_service != nullptr);

                _job.Revoke();

                _adminLock.Lock();
                _monitor.clear();
                _adminLock.Unlock();
                _service->Release();
                _service = nullptr;
            }
            virtual void StateChange(PluginHost::IShell* service)
            {
                _adminLock.Lock();

                std::map<string, MonitorObject>::iterator index(_monitor.find(service->Callsign()));

                if (index != _monitor.end()) {

                    // Only act on Activated or Deactivated...
                    PluginHost::IShell::state currentState(service->State());

                    if (currentState == PluginHost::IShell::ACTIVATED) {
                        bool is_active = index->second.IsActive();
                        index->second.Active(true);
                        if (is_active == false && std::count_if(_monitor.begin(), _monitor.end(), [](const std::pair<string, MonitorObject>& v) {
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

                _adminLock.Unlock();
            }
            void Snapshot(Core::JSON::ArrayType<Monitor::Data>& snapshot)
            {
                _adminLock.Lock();

                std::map<string, MonitorObject>::iterator element(_monitor.begin());

                // Go through the list of observations...
                while (element != _monitor.end()) {
                    if (element->second.HasMeasurement() == true) {
                        snapshot.Add(Monitor::Data(element->first, element->second.Measurement()));
                    }
                    element++;
                }

                _adminLock.Unlock();
            }
            bool Snapshot(const string& name, Monitor::MetaData& result)
            {
                bool found = false;

                _adminLock.Lock();

                std::map<string, MonitorObject>::iterator index(_monitor.find(name));

                if (index != _monitor.end()) {
                    if (index->second.HasMeasurement() == true) {
                        result = index->second.Measurement();
                        found = true;
                    }
                }

                _adminLock.Unlock();

                return (found);
            }

            void Snapshot(const string& callsign, Core::JSON::ArrayType<JsonData::Monitor::InfoInfo>* response)
            {
                _adminLock.Lock();

                auto AddElement = [this, &response](const string& callsign, MonitorObject& object) {
                    const MetaData& metaData = object.Measurement();
                    JsonData::Monitor::InfoInfo info;
                    info.Observable = callsign;

                    if (object.HasRestartAllowed()) {
                        info.Restart.Limit = object.RestartLimit();
                        info.Restart.Window = object.RestartWindow();
                    }

                    if (object.HasMeasurement()) {
                        translate(metaData.Allocated(), &info.Measurements.Allocated);
                        translate(metaData.Resident(), &info.Measurements.Resident);
                        translate(metaData.Shared(), &info.Measurements.Shared);
                        translate(metaData.Process(), &info.Measurements.Process);
                    }
                    info.Measurements.Operational = metaData.Operational();
                    info.Measurements.Count = metaData.Allocated().Measurements();

                    response->Add(info);
                };

                if (callsign.empty() == false) {
                    auto element = _monitor.find(callsign);
                    if (element != _monitor.end()) {
                        AddElement(element->first, element->second);
                    }
                } else {
                    for (auto& element : _monitor) {
                        AddElement(element.first, element.second);
                    }
                }

                _adminLock.Unlock();
            }

            bool Reset(const string& name, Monitor::MetaData& result)
            {
                bool found = false;

                _adminLock.Lock();

                std::map<string, MonitorObject>::iterator index(_monitor.find(name));

                if (index != _monitor.end()) {
                    result = index->second.Measurement();
                    index->second.Reset();
                    found = true;
                }

                _adminLock.Unlock();

                return (found);
            }

            bool Reset(const string& name)
            {
                bool found = false;

                _adminLock.Lock();

                std::map<string, MonitorObject>::iterator index(_monitor.find(name));

                if (index != _monitor.end()) {
                    index->second.Reset();
                    found = true;
                }

                _adminLock.Unlock();

                return (found);
            }

            BEGIN_INTERFACE_MAP(MonitorObjects)
            INTERFACE_ENTRY(PluginHost::IPlugin::INotification)
            END_INTERFACE_MAP

        private:
            friend Core::ThreadPool::JobType<MonitorObjects&>;

            // Dispatch can be run in an unlocked state as the destruction of the observer list
            // is always done if the thread that calls the Dispatch is blocked (paused)
            void Dispatch()
            {
                uint64_t scheduledTime(Core::Time::Now().Ticks());
                uint64_t nextSlot(static_cast<uint64_t>(~0));

                std::map<string, MonitorObject>::iterator index(_monitor.begin());

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
                                SYSLOG(Trace::Fatal, (_T("FORCED Shutdown: %s by reason: %s."), plugin->Callsign().c_str(), why.Data()));

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
                        _job.Schedule(nextSlot);
                    }
                } else {
                    TRACE(Trace::Information, (_T("Stopping to probe due to lack of active observees.")));
                }
            }

        private:
            template <typename T>
            void translate(const Core::MeasurementType<T>& from, JsonData::Monitor::MeasurementInfo* to)
            {
                to->Min = from.Min();
                to->Max = from.Max();
                to->Average = from.Average();
                to->Last = from.Last();
            }

            Core::CriticalSection _adminLock;
            std::map<string, MonitorObject> _monitor;
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
            , _monitor(Core::Service<MonitorObjects>::Create<MonitorObjects>(this))
        {
            RegisterAll();
        }
#ifdef __WINDOWS__
#pragma warning(default : 4355)
#endif
        virtual ~Monitor()
        {
            UnregisterAll();
            _monitor->Release();
        }

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
        virtual const string Initialize(PluginHost::IShell* service);

        // The plugin is unloaded from WPEFramework. This is call allows the module to notify clients
        // or to persist information if needed. After this call the plugin will unlink from the service path
        // and be deactivated. The Service object is the same as passed in during the Initialize.
        // After theis call, the lifetime of the Service object ends.
        virtual void Deinitialize(PluginHost::IShell* service);

        // Returns an interface to a JSON struct that can be used to return specific metadata information with respect
        // to this plugin. This Metadata can be used by the MetData plugin to publish this information to the ouside world.
        virtual string Information() const;

        //  IWeb methods
        // -------------------------------------------------------------------------------------------------------
        // Whenever a request is received, it might carry some additional data in the body. This method allows
        // the plugin to attach a deserializable data object (ref counted) to be loaded with any potential found
        // in the body of the request.
        virtual void Inbound(Web::Request& request);

        // If everything is received correctly, the request is passed on to us, through a thread from the thread pool, to
        // do our thing and to return the result in the response object. Here the actual specific module work,
        // based on a a request is handled.
        virtual Core::ProxyType<Web::Response> Process(const Web::Request& request);

    private:
        bool Activated(const string& className, const string& callSign, IPlugin* plugin);
        bool Deactivated(const string& className, const string& callSign, IPlugin* plugin);

    private:
        uint8_t _skipURL;
        Config _config;
        MonitorObjects* _monitor;

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
