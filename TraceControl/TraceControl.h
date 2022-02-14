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
#include <interfaces/json/JsonData_TraceControl.h>

namespace WPEFramework {

namespace Plugin {

    class TraceControl : public PluginHost::IPlugin, public PluginHost::IWeb, public PluginHost::JSONRPC {

    public:
        enum state {
            ENABLED,
            DISABLED,
            TRISTATED
        };

    private:
        TraceControl(const TraceControl&) = delete;
        TraceControl& operator=(const TraceControl&) = delete;

        class Observer : public Core::Thread, public RPC::IRemoteConnection::INotification {
        private:
            Observer() = delete;
            Observer(const Observer&) = delete;
            Observer& operator=(const Observer&) = delete;

        public:
            class Source : public Core::CyclicBuffer {
            private:
                Source() = delete;
                Source(const Source&) = delete;
                Source& operator=(const Source&) = delete;

                class LocalIterator : public Trace::ITraceIterator, public Trace::ITraceController {
                private:
                    LocalIterator(const LocalIterator&) = delete;
                    LocalIterator& operator=(const LocalIterator&) = delete;

                public:
                    LocalIterator()
                        : _index(Trace::TraceUnit::Instance().GetCategories())
                    {
                    }
                    virtual ~LocalIterator()
                    {
                    }

                public:
                    virtual void AddRef() const
                    {
                    }
                    virtual uint32_t Release() const
                    {
                        return (Core::ERROR_NONE);
                    }

                    BEGIN_INTERFACE_MAP(LocalIterator)
                    INTERFACE_ENTRY(Trace::ITraceIterator)
                    INTERFACE_ENTRY(Trace::ITraceController)
                    END_INTERFACE_MAP

                public:
                    virtual void Reset()
                    {
                        _index = Trace::TraceUnit::Instance().GetCategories();
                    }
                    virtual bool Info(bool& enabled, string& module, string& category) const
                    {
                        bool result = _index.Next();

                        if (result == true) {
                            module = _index->Module();
                            category = _index->Category();
                            enabled = _index->Enabled();
                        }
                        return (result);
                    }
                    virtual void Enable(const bool enabled, const string& module, const string& category)
                    {
                        Trace::TraceUnit::Instance().SetCategories(
                            enabled,
                            (module.empty() == false ? module.c_str() : nullptr),
                            (category.empty() == false ? category.c_str() : nullptr));
                    }

                private:
                    mutable Trace::TraceUnit::Iterator _index;
                };

                static string SourceName(const string& pathName, RPC::IRemoteConnection* connection);

            public:
                enum state {
                    EMPTY,
                    LOADED,
                    FAILURE
                };

            public:
                Source(const string& tracePath, RPC::IRemoteConnection* connection)
                    : Core::CyclicBuffer(SourceName(tracePath, connection), Core::File::USER_WRITE|Core::File::USER_READ|Core::File::SHAREABLE, 0, true)
                    , _iterator(connection == nullptr ? &_localIterator : nullptr)
                    , _control(connection == nullptr ? &_localIterator : nullptr)
                    , _connection(connection)
                    , _module(0)
                    , _category(0)
                    , _classname(0)
                    , _information()
                    , _state(EMPTY)
                {
                    if (_connection != nullptr) {
                        TRACE(Trace::Information, (_T("Constructing TraceControl::Source (%d)"), connection->Id()));
                        _connection->AddRef();
                    }
                }
                ~Source()
                {
                    if (_connection != nullptr) {
                        TRACE(Trace::Information, (_T("Destructing TraceControl::Source (%d)"), _connection->Id()));

                        Relinquish();
                        _connection->Release();
                        _connection = nullptr;
                    }
                }

            public:
                uint32_t Id() const
                {
                    return (_connection != nullptr ? _connection->Id() : 0);
                }
                void Relinquish()
                {
                    if (_connection != nullptr) {
                        if (_iterator != nullptr) {
                            _iterator->Release();
                            _iterator = nullptr;
                        }
                        if (_control != nullptr) {
                            _control->Release();
                            _control = nullptr;
                        }
                    }
                }
                void Reset()
                {
                    // Lazy creation, get the interface, if we want to iterate over the trace categories.
                    if ((_iterator == nullptr) && (_connection != nullptr)) {
                        _iterator = _connection->Aquire<Trace::ITraceIterator>(Core::infinite, _T("TraceIterator"), static_cast<uint32_t>(~0));
                    }

                    if (_iterator != nullptr) {
                        _iterator->Reset();
                    }
                }
                bool Info(bool& enabled, string& module, string& category)
                {
                    bool result = false;

                    // Lazy creation, get the interface, if we want to iterate over the trace categories.
                    if ((_iterator == nullptr) && (_connection != nullptr)) {
                        _iterator = _connection->Aquire<Trace::ITraceIterator>(Core::infinite, _T("TraceIterator"), static_cast<uint32_t>(~0));
                    }

                    if (_iterator != nullptr) {
                        result = _iterator->Info(enabled, module, category);
                    }

                    return (result);
                }
                void Set(const bool enabled, const string& module, const string& category)
                {
                    if (_control == nullptr) {
                        _control = _connection->Aquire<Trace::ITraceController>(Core::infinite, _T("TraceController"), static_cast<uint32_t>(~0));
                    }

                    if (_control != nullptr) {
                        _control->Enable(enabled, module, category);
                    }
                }

                state Load()
                {
                    uint32_t length;
                    bool available = Core::CyclicBuffer::IsValid();

                    if (available == false) {
                        available = Core::CyclicBuffer::Validate();
                    }

                    // Traces will be commited in one go, First reserve, then write. So if there is a length (2 bytes)
                    // The full trace has to be available as well.
                    if ((available == true) && (_state == EMPTY) && ((length = Read(_traceBuffer, sizeof(_traceBuffer))) != 0)) {

                        if (length < 2) {
                            // Didn't even get enough data to read entry size. This is impossible, fallback to failure.
                            TRACE(Trace::Error, (_T("Inconsistent trace dump. Need to flush. %d"), length));
                            _state = FAILURE;
                        } else {
                            // TODO: This is platform dependend, needs to ba agnostic to the platform.
                            uint16_t requiredLength = (_traceBuffer[1] << 8) | _traceBuffer[0];

                            if (requiredLength != length) {
                                // Something went wrong, didn't read a full entry.
                                _state = FAILURE;
                            } else {
                                // length(2 bytes) - clock ticks (8 bytes) - line number (4 bytes) - file/module/category/className

                                // Keep track of location in buffer.
                                uint32_t offset = /* length */ 2 /* clock */ + 8 /* Skip line number */ + 4;

                                // Skip file name.
                                offset += static_cast<uint32_t>(strlen(reinterpret_cast<char*>(_traceBuffer + offset)) + 1);

                                // Get module offset.
                                _module = offset;
                                offset += static_cast<uint32_t>(strlen(reinterpret_cast<char*>(_traceBuffer + _module)) + 1);

                                // Get category offset.
                                _category = offset;
                                offset += static_cast<uint32_t>(strlen(reinterpret_cast<char*>(_traceBuffer + _category)) + 1);

                                // Get class name offset.
                                _classname = offset;
                                offset += static_cast<uint32_t>(strlen(reinterpret_cast<char*>(_traceBuffer + _classname)) + 1);

                                ASSERT(length >= offset);

                                // Rest of entry is information.
                                _information = offset;
                                _length = requiredLength - offset;
                                _traceBuffer[requiredLength] = '\0';

                                // Entries are read in whole, so we are done.
                                _state = LOADED;
                            }
                        }
                    }
                    return _state;
                }
                inline state State() const
                {
                    return (_state);
                }
                inline uint64_t Timestamp() const
                {
                    uint64_t stamp;
                    ::memcpy(&stamp, &(_traceBuffer[2]), sizeof(uint64_t));
                    return (stamp);
                }
                inline uint32_t LineNumber() const
                {
                    uint32_t linenumber;
                    ::memcpy(&linenumber, &(_traceBuffer[10]), sizeof(uint32_t));
                    return (linenumber);
                }
                inline const char* FileName() const
                {
                    return reinterpret_cast<const char*>(&_traceBuffer[14]);
                }
                inline const char* Module() const
                {
                    return reinterpret_cast<const char*>(&_traceBuffer[_module]);
                }
                inline const char* Category() const
                {
                    return reinterpret_cast<const char*>(&_traceBuffer[_category]);
                }
                inline const char* ClassName() const
                {
                    return reinterpret_cast<const char*>(&_traceBuffer[_classname]);
                }
                inline const char* Information() const
                {
                    return reinterpret_cast<const char*>(&_traceBuffer[_information]);
                }
                inline uint16_t Length() const
                {
                    return (_length);
                }
                void Flush()
                {
                    _state = EMPTY;
                    Core::CyclicBuffer::Flush();
                }
                void Clear()
                {
                    _state = EMPTY;
                }

            private:
                virtual uint32_t GetReadSize(Core::CyclicBuffer::Cursor& cursor) override
                {
                    // Just read one entry.
                    uint16_t entrySize = 0;
                    cursor.Peek(entrySize);
                    return entrySize;
                }

                Trace::ITraceIterator* _iterator;
                Trace::ITraceController* _control;
                RPC::IRemoteConnection* _connection;
                uint16_t _module;
                uint16_t _category;
                uint16_t _classname;
                uint16_t _information;
                uint16_t _length;
                state _state;
                uint8_t _traceBuffer[Trace::CyclicBufferSize];
                static LocalIterator _localIterator;
            };

            class ModuleIterator {
            public:
                class CategoryInfo {
                public:
                    CategoryInfo()
                        : _category()
                        , _state(TRISTATED)
                    {
                    }
                    CategoryInfo(const std::string category, const bool enabled)
                        : _category(category)
                        , _state(enabled ? ENABLED : DISABLED)
                    {
                    }
                    CategoryInfo(const CategoryInfo& copy)
                        : _category(copy._category)
                        , _state(copy._state)
                    {
                    }
                    ~CategoryInfo()
                    {
                    }

                    inline CategoryInfo& operator=(const CategoryInfo& rhs)
                    {
                        _category = rhs._category;
                        _state = rhs._state;

                        return (*this);
                    }
                    inline bool operator==(const string& rhs) const
                    {
                        return (_category == rhs);
                    }
                    inline bool operator!=(const string& rhs) const
                    {
                        return !(operator==(rhs));
                    }

                public:
                    inline state State() const
                    {
                        return (_state);
                    }
                    inline void State(const state value)
                    {
                        _state = value;
                    }
                    inline const string& Category() const
                    {
                        return (_category);
                    }

                private:
                    string _category;
                    state _state;
                };

                typedef std::list<CategoryInfo> CategoryList;
                typedef std::map<std::string, CategoryList> ModuleMap;
                typedef Core::IteratorType<const CategoryList, const CategoryInfo&, CategoryList::const_iterator> CategoryListIterator;
                typedef Core::IteratorMapType<const ModuleMap, const CategoryList&, const std::string&, ModuleMap::const_iterator> ModuleMapIterator;

            public:
                class CategoryIterator {
                public:
                    CategoryIterator()
                        : _categories(nullptr)
                        , _iterator()
                    {
                    }
                    CategoryIterator(const CategoryIterator& copy)
                        : _categories(copy._categories)
                        , _iterator()
                    {
                        if (_categories != nullptr) {
                            _iterator = CategoryListIterator(*_categories);
                        }
                    }
                    CategoryIterator(const CategoryList& categories)
                        : _categories(&categories)
                        , _iterator(*_categories)
                    {
                    }
                    ~CategoryIterator()
                    {
                    }

                    CategoryIterator& operator=(const CategoryIterator& rhs)
                    {
                        _categories = rhs._categories;

                        if (_categories != nullptr) {
                            _iterator = CategoryListIterator(*_categories);
                        } else {
                            _iterator = CategoryListIterator();
                        }

                        return (*this);
                    }

                public:
                    inline bool IsValid() const
                    {
                        return (_iterator.IsValid());
                    }
                    inline void Reset()
                    {
                        _iterator.Reset(0);
                    }
                    inline bool Next()
                    {
                        return (_iterator.Next());
                    }
                    inline const std::string& Category() const
                    {
                        ASSERT(_iterator.IsValid());

                        return (_iterator.operator->().Category());
                    }
                    inline state State() const
                    {
                        ASSERT(_iterator.IsValid());

                        return (_iterator.operator->().State());
                    }

                private:
                    const CategoryList* _categories;
                    CategoryListIterator _iterator;
                };

            public:
                ModuleIterator()
                    : _modules()
                    , _iterator()
                {
                }
                ModuleIterator(const ModuleIterator& copy)
                    : _modules(copy._modules)
                    , _iterator(_modules)
                {
                }
                ModuleIterator(const std::map<const uint32_t, Source*>& buffers)
                    : _modules()
                {
                    std::map<const uint32_t, Source*>::const_iterator index(buffers.begin());

                    while (index != buffers.end()) {
                        bool enabled;
                        string category;
                        string module;

                        Source* modules(index->second);
                        modules->Reset();

                        while (modules->Info(enabled, module, category)) {
                            ModuleMap::iterator index(_modules.find(module));

                            if (index == _modules.end()) {
                                // First attempt, no need to validate if the category exists...
                                _modules[module].push_back(CategoryInfo(category, enabled));
                            } else {
                                CategoryList::iterator selected(std::find(index->second.begin(), index->second.end(), category));

                                if (selected == index->second.end()) {
                                    _modules[module].push_back(CategoryInfo(category, enabled));
                                } else if (((enabled == true) && (selected->State() != ENABLED)) || ((enabled == false) && (selected->State() != DISABLED))) {
                                    selected->State(TRISTATED);
                                }
                            }
                        }
                        index++;
                    }

                    _iterator = ModuleMapIterator(_modules);
                }
                ~ModuleIterator()
                {
                }

                ModuleIterator& operator=(const ModuleIterator& rhs)
                {
                    _modules = rhs._modules;
                    _iterator = ModuleMapIterator(_modules);

                    return (*this);
                }

            public:
                inline bool IsValid() const
                {
                    return (_iterator.IsValid());
                }
                inline void Reset()
                {
                    _iterator.Reset(0);
                }
                inline bool Next()
                {
                    return (_iterator.Next());
                }
                inline std::string Module() const
                {
                    ASSERT(_iterator.IsValid());

                    return (_iterator.Key());
                }
                inline CategoryIterator Categories() const
                {
                    return (CategoryIterator(_iterator.operator*()));
                }

            private:
                ModuleMap _modules;
                ModuleMapIterator _iterator;
            };

        public:
            Observer(TraceControl& parent)
                : Thread(Core::Thread::DefaultStackSize(), _T("TraceWorker"))
                , _buffers()
                , _traceControl(Trace::TraceUnit::Instance())
                , _parent(parent)
                , _refcount(0)
            {
            }
            ~Observer()
            {
                ASSERT(_refcount == 0);
                ASSERT(_buffers.size() == 0);
                _traceControl.Relinquish();
                Wait(Thread::BLOCKED | Thread::STOPPED | Thread::STOPPING, Core::infinite);
            }

        public:
            void Reevaluate()
            {
                _traceControl.Announce();
            }
            void Start() 
            {
                _buffers.insert(std::pair<const uint32_t, Source*>(0, new Source(_parent.TracePath(), nullptr)));
                _traceControl.Announce();
                Thread::Run();
            }
            void Stop()
            {
                Block();

                _traceControl.Announce();

                Wait(Thread::BLOCKED | Thread::STOPPED | Thread::STOPPING, Core::infinite);

                _adminLock.Lock();

                while (_buffers.size() != 0) {
                    delete _buffers.begin()->second;

                    _buffers.erase(_buffers.begin());
                }

                _adminLock.Unlock();
            }
            virtual void Activated(RPC::IRemoteConnection* connection)
            {
                _adminLock.Lock();

                ASSERT(_buffers.find(connection->Id()) == _buffers.end());

                // By definition, get the buffer file from WPEFramework (local source)
                _buffers.insert(std::pair<const uint32_t, Source*>(connection->Id(), new Source(_parent.TracePath(), connection)));
                _traceControl.Announce();

                _adminLock.Unlock();
            }
            virtual void Deactivated(RPC::IRemoteConnection* connection)
            {
                _adminLock.Lock();

                std::map<const uint32_t, Source*>::iterator index(_buffers.find(connection->Id()));

                if (index != _buffers.end()) {
                    delete (index->second);
                    _buffers.erase(index);
                }

                _adminLock.Unlock();
            }

            void Set(const bool enabled, const std::string& module, const std::string& category)
            {
                _adminLock.Lock();

                std::map<const uint32_t, Source*>::iterator index(_buffers.begin());

                while (index != _buffers.end()) {
                    index->second->Set(enabled, module, category);
                    index++;
                }

                _adminLock.Unlock();
            }

            void Relinquish()
            {
                _adminLock.Lock();

                std::map<const uint32_t, Source*>::iterator index(_buffers.begin());

                while (index != _buffers.end()) {
                    index->second->Relinquish();
                    index++;
                }

                _adminLock.Unlock();
            }

            inline ModuleIterator Modules() const
            {
                return (ModuleIterator(_buffers));
            }

        private:
            BEGIN_INTERFACE_MAP(Observer)
            INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

            virtual void AddRef() const
            {
                Core::InterlockedIncrement(_refcount);
            }
            virtual uint32_t Release() const
            {
                Core::InterlockedDecrement(_refcount);

                return (Core::ERROR_NONE);
            }
            virtual uint32_t Worker()
            {
                
                while ((IsRunning() == true) && (_traceControl.Wait(Core::infinite) == Core::ERROR_NONE)) {
                    // Before we start we reset the flag, if new info is coming in, we will get a retrigger flag.
                    _traceControl.Acknowledge();

                    Source* selected;

                    uint64_t timeStamp;

                    do {
                        selected = nullptr;

                        _adminLock.Lock();

                        timeStamp = static_cast<uint64_t>(~0);

                        std::map<const uint32_t, Source*>::iterator index(_buffers.begin());

                        while (index != _buffers.end()) {

                            Source::state state(index->second->Load());

                            if (state == Source::LOADED) {
                                if (index->second->Timestamp() < timeStamp) {
                                    timeStamp = index->second->Timestamp();
                                    selected = index->second;
                                }
                            } else if (state == Source::FAILURE) {
                                // Oops this requires recovery, so let's flush
                                index->second->Flush();
                            }

                            index++;
                        }

                        if (selected != nullptr) {

                            // Oke, output this entry
                            _parent.Dispatch(*selected);

                            // Ready to load a new one..
                            selected->Clear();
                        } else if (timeStamp != static_cast<uint64_t>(~0)) {
                            // Looks like we are waiting for a message to be completed.
                            // Give up our slice, so the producer, can produce.
                            ::SleepMs(0);
                        }

                        _adminLock.Unlock();

                    } while ((IsRunning() == true) && (timeStamp != static_cast<uint64_t>(~0)));
                }

                return (Core::infinite);
            }

        private:
            Core::CriticalSection _adminLock;
            std::map<const uint32_t, Source*> _buffers;
            Trace::TraceUnit& _traceControl;
            TraceControl& _parent;
            mutable uint32_t _refcount;
        };

        class InformationWrapper : public Trace::ITrace {
        private:
            InformationWrapper() = delete;
            InformationWrapper(const InformationWrapper& copy) = delete;
            InformationWrapper& operator=(const InformationWrapper&) = delete;

        public:
            InformationWrapper(const TraceControl::Observer::Source& information)
                : _info(information)
            {
            }
            ~InformationWrapper()
            {
            }

        public:
            virtual const char* Category() const
            {
                return (_info.Category());
            }
            virtual const char* Module() const
            {
                return (_info.Module());
            }
            virtual const char* Data() const
            {
                return (_info.Information());
            }
            virtual uint16_t Length() const
            {
                return (_info.Length());
            }

        private:
            const TraceControl::Observer::Source& _info;
        };

    public:
        class NetworkNode : public Core::JSON::Container {
        public:
            NetworkNode()
                : Core::JSON::Container()
                , Port(2200)
                , Binding("0.0.0.0")
            {
                Add(_T("port"), &Port);
                Add(_T("binding"), &Binding);
            }
            NetworkNode(const NetworkNode& copy)
                : Core::JSON::Container()
                , Port(copy.Port)
                , Binding(copy.Binding)
            {
                Add(_T("port"), &Port);
                Add(_T("binding"), &Binding);
            }
            ~NetworkNode()
            {
            }

            NetworkNode& operator=(const NetworkNode& RHS)
            {
                Port = RHS.Port;
                Binding = RHS.Binding;

                return (*this);
            }

        public:
            Core::JSON::DecUInt16 Port;
            Core::JSON::String Binding;
        };
        class Config : public Core::JSON::Container {
        private:
            Config(const Config&);
            Config& operator=(const Config&);

        public:
            Config()
                : Core::JSON::Container()
                , Console(false)
                , SysLog(true)
                , Abbreviated(true)
                , Remote()
            {
                Add(_T("console"), &Console);
                Add(_T("syslog"), &SysLog);
                Add(_T("abbreviated"), &Abbreviated);
                Add(_T("remote"), &Remote);
            }
            ~Config()
            {
            }

        public:
            Core::JSON::Boolean Console;
            Core::JSON::Boolean SysLog;
            Core::JSON::Boolean Abbreviated;
            NetworkNode Remote;
        };
        class Data : public Core::JSON::Container {
        public:
            class StatusParam final : public Core::JSON::Container {
            public:
                StatusParam()
                    : Core::JSON::Container()
                    , Module()
                    , Category()
                {
                    Add(_T("module"), &Module);
                    Add(_T("category"), &Category);
                }

                StatusParam(const StatusParam&) = delete;
                StatusParam& operator=(const StatusParam&) = delete;

            public:
                Core::JSON::String Module; // Module name
                Core::JSON::String Category; // Category name
            }; // class StatusDataParam

            class Trace : public Core::JSON::Container {
            private:
                Trace& operator=(const Trace&);

            public:
                Trace()
                    : Core::JSON::Container()
                {
                    Add(_T("module"), &Module);
                    Add(_T("category"), &Category);
                    Add(_T("state"), &State);
                }
                Trace(const string& moduleName, const string& categoryName, const state currentState)
                    : Core::JSON::Container()
                {
                    Add(_T("module"), &Module);
                    Add(_T("category"), &Category);
                    Add(_T("state"), &State);

                    Module = moduleName;
                    Category = categoryName;
                    State = currentState;
                }
                Trace(const Trace& copy)
                    : Core::JSON::Container()
                    , Module(copy.Module)
                    , Category(copy.Category)
                    , State(copy.State)
                {
                    Add(_T("module"), &Module);
                    Add(_T("category"), &Category);
                    Add(_T("state"), &State);
                }
                ~Trace()
                {
                }

            public:
                Core::JSON::String Module;
                Core::JSON::String Category;
                Core::JSON::EnumType<state> State;
            };

        private:
            Data(const Data&);
            Data& operator=(const Data&);

        public:
            Data()
                : Core::JSON::Container()
            {
                Add(_T("console"), &Console);
                Add(_T("remote"), &Remote);
                Add(_T("settings"), &Settings);
            }
            ~Data()
            {
            }

        public:
            Core::JSON::Boolean Console;
            NetworkNode Remote;
            Core::JSON::ArrayType<Trace> Settings;
        };

    public:
#ifdef __WINDOWS__
#pragma warning(disable : 4355)
#endif
        TraceControl()
            : _skipURL(0)
            , _service(nullptr)
            , _outputs()
            , _tracePath()
            , _observer(*this)
        {
            RegisterAll();
        }
#ifdef __WINDOWS__
#pragma warning(default : 4355)
#endif
        virtual ~TraceControl()
        {
            UnregisterAll();
        }

        BEGIN_INTERFACE_MAP(TraceControl)
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
        void Dispatch(Observer::Source& information);

        void RegisterAll();
        void UnregisterAll();
        JsonData::TraceControl::StateType TranslateState(TraceControl::state state);
        uint32_t endpoint_status(const JsonData::TraceControl::StatusParamsData& params, JsonData::TraceControl::StatusResultData& response);
        uint32_t endpoint_set(const JsonData::TraceControl::TraceInfo& params);
        inline const string& TracePath() const 
        {
            return (_tracePath);
        }

    private:
        uint8_t _skipURL;
        PluginHost::IShell* _service;
        Config _config;
        std::list<Trace::ITraceMedia*> _outputs;
        string _tracePath;
        Observer _observer;
    };
}
}
