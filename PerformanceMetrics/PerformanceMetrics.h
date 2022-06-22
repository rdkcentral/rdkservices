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
#include <interfaces/IMemory.h>
#include <interfaces/IBrowser.h>

#include <memory>

namespace WPEFramework {
namespace Plugin {

    class PerformanceMetrics : public PluginHost::IPlugin {

    private:
        class Config : public Core::JSON::Container {
        public:
            Config(const Config&);
            Config& operator=(const Config&);

        public:
            Config()
                : Core::JSON::Container()
                , ObservableCallsign()
                , ObservableClassname()
            {
                Add(_T("callsign"), &ObservableCallsign);
                Add(_T("classname"), &ObservableClassname);
            }

        public:
            Core::JSON::String ObservableCallsign;
            Core::JSON::String ObservableClassname;
        };

        class Notification : public PluginHost::IPlugin::INotification {
        public:
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

            explicit Notification(PerformanceMetrics& parent)
                : PluginHost::IPlugin::INotification()
                , _parent(parent)
            {
            }
            ~Notification() override = default;

            void Activated (const string&, PluginHost::IShell* service) override
            {
                ASSERT(service != nullptr);

                _parent.PluginActivated(*service);
            }
            void Deactivated (const string&, PluginHost::IShell* service) override
            {
                ASSERT(service != nullptr);

                _parent.PluginDeactivated(*service);
            }
            void Unavailable(const string&, PluginHost::IShell*) override
            {
            }

            BEGIN_INTERFACE_MAP(Notification)
            INTERFACE_ENTRY(PluginHost::IPlugin::INotification)
            END_INTERFACE_MAP

        private:
            PerformanceMetrics& _parent;
        };

    private:

        template<class LOGGERINTERFACE>
        static std::unique_ptr<LOGGERINTERFACE> LoggerFactory();

    public:

        class IBasicMetricsLogger {
        public:
            virtual ~IBasicMetricsLogger() = default;

            virtual void Enable(PluginHost::IShell& service, const string& callsign) = 0;
            virtual void Disable() = 0;

            virtual void Activated()  = 0;
            virtual void Deactivated(const uint32_t uptime_s)  = 0;
        };

        struct IStateMetricsLogger : public IBasicMetricsLogger {
        public:

            virtual void Resumed() = 0;
            virtual void Suspended() = 0;
        };

        struct IBrowserMetricsLogger : public IStateMetricsLogger {
        public:
            static constexpr char startURL[] = _T("about:blank");

            virtual void LoadFinished(const string& URL, const int32_t httpstatus, const bool success, const uint32_t totalsuccess, const uint32_t totalfailed) = 0;
            virtual void URLChange(const string& URL, const bool loaded) = 0;
            virtual void VisibilityChange(const bool hidden) = 0;
            virtual void PageClosure() = 0;
        };

    private:
        struct IObservable {
            virtual ~IObservable() = default;

            // do not call this Initialize and Deinitialize :)
            virtual void Enable() = 0;
            virtual void Disable() = 0;

            virtual void Activated(PluginHost::IShell&) = 0;
            virtual void Deactivated(PluginHost::IShell&) = 0;
        };

    private:

        struct IPerfMetricsHandler {
            virtual ~IPerfMetricsHandler() = default;

            virtual void Initialize() = 0;
            virtual void Deinitialize() = 0;

            virtual void Activated(PluginHost::IShell&) = 0;
            virtual void Deactivated(PluginHost::IShell&) = 0;
        };

        class CallsignPerfMetricsHandler : public IPerfMetricsHandler
        {
        public:
            CallsignPerfMetricsHandler(const string& callsign) 
                : IPerfMetricsHandler()
                , _callsign(callsign)
                , _observable()
            {
            }

            ~CallsignPerfMetricsHandler() override 
            {
                ASSERT(_observable.IsValid() == false);
            }

            CallsignPerfMetricsHandler(const CallsignPerfMetricsHandler&) = delete;
            CallsignPerfMetricsHandler& operator=(const CallsignPerfMetricsHandler&) = delete;

            const string& Callsign() const 
            {
                return _callsign;
            }

            void Initialize() override
            {
                ASSERT(_observable.IsValid() == false);
            }

            void Deinitialize() override
            {
                if( _observable.IsValid() == true ) {
                    _observable->Disable();
                    VARIABLE_IS_NOT_USED uint32_t result =_observable.Release(); 
                    ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);
                    ASSERT(_observable.IsValid() == false);
                }
            }

            void Activated(PluginHost::IShell& service) override
            {
                if( service.Callsign() == _callsign ) {
                    ASSERT(_observable.IsValid() == false);

                    CreateObservable(service);
                    _observable->Activated(service);
                }
            }
            
            void Deactivated(PluginHost::IShell& service) override
            {
                if( service.Callsign() == _callsign ) {
                    ASSERT(_observable.IsValid() == true);

                    _observable->Deactivated(service);
                    _observable->Disable();
                    VARIABLE_IS_NOT_USED uint32_t result =_observable.Release(); 
                    ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);
                    ASSERT(_observable.IsValid() == false);            
                }
            }

        private:
            void CreateObservable(PluginHost::IShell& service);

        private:
            string _callsign;
            Core::ProxyType<IObservable> _observable;
        };

        class ClassnamePerfMetricsHandler : public IPerfMetricsHandler
        {
        public:
            ClassnamePerfMetricsHandler(const string& classname) 
                : IPerfMetricsHandler()
                , _classname(classname)
                , _observers()
                , _adminLock()
            {
            }

            ~ClassnamePerfMetricsHandler() override 
            {
                ASSERT(_observers.empty() == true);
            }

            ClassnamePerfMetricsHandler(const ClassnamePerfMetricsHandler&) = delete;
            ClassnamePerfMetricsHandler& operator=(const ClassnamePerfMetricsHandler&) = delete;

            const string& Classname() const 
            {
                return _classname;
            }

            void Initialize() override
            {
                ASSERT(_observers.empty() == true);
            }

            void Deinitialize() override
            {
                // no lock needed, no notification are possible here.
                for( auto& observer : _observers ) {
                    observer.second.Deinitialize();
                }
                _observers.clear();
            }

            void Activated(PluginHost::IShell& service) override
            {
                if( service.ClassName() == Classname() ) {
                    _adminLock.Lock();
                    auto result =_observers.emplace(std::piecewise_construct,
                                       std::forward_as_tuple(service.Callsign()),
                                       std::forward_as_tuple(service.Callsign()));
                    ASSERT( ( result.second == true ) && ( result.first != _observers.end() ) );
                    result.first->second.Initialize();
                    result.first->second.Activated(service);
                    _adminLock.Unlock();
                }
            }
            void Deactivated(PluginHost::IShell& service) override
            {
                if( service.ClassName() == Classname() ) {
                    _adminLock.Lock();
                    auto it =_observers.find(service.Callsign());
                    if( it != _observers.end() ) {
                        it->second.Deactivated(service);
                        it->second.Deinitialize();
                        _observers.erase(it);
                    }
                    _adminLock.Unlock();
                }
            }

        private:
            using OberserverMap = std::unordered_map<string, CallsignPerfMetricsHandler>;

            string _classname;
            OberserverMap _observers;
            mutable Core::CriticalSection _adminLock;
        };

    private:

        // we cannot make the Logger a static and get it via Instance or something similar as the Plugin (and therefore the library) 
        // might be used multipe times fot different callsigns and the they would share the same Logger instance (and Logger state)
        template<class LOGGERINTERFACE>
        class LoggerProxy {
        public:
            LoggerProxy() : _logger(LoggerFactory<LOGGERINTERFACE>()) {}
            ~LoggerProxy() = default;

            LOGGERINTERFACE& Logger() 
            { 
                ASSERT(_logger.get() != nullptr);
                return *_logger; 
            }
            const LOGGERINTERFACE& Logger() const
            { 
                ASSERT(_logger.get() != nullptr);
                return *_logger; 
            }

        private:
            std::unique_ptr<LOGGERINTERFACE> _logger; 
        };

        template<class LOGGERINTERFACE = IBasicMetricsLogger>
        class BasicObservable : public IObservable, protected LoggerProxy<LOGGERINTERFACE> {
        protected:
            // make Logger accessable here, so we don't have to put this-> in front of it everywhere
            using LoggerProxy<LOGGERINTERFACE>::Logger;

        public:
            BasicObservable(const BasicObservable&) = delete;
            BasicObservable& operator=(const BasicObservable&) = delete;

            BasicObservable(CallsignPerfMetricsHandler& parent, PluginHost::IShell& service) 
            : IObservable()
            , LoggerProxy<LOGGERINTERFACE>()
            , _parent(parent)
            , _activatetime(0)
            , _service(&service)
            {
                _service->AddRef();
            }
            ~BasicObservable() override 
            {
                ASSERT(_service == nullptr);
            }

            // do not call this Initialize and Deinitialize :)
            void Enable() override
            {
                ASSERT(_service != nullptr);
                ASSERT(_service->Callsign() == Parent().Callsign());

                Logger().Enable(*_service, Parent().Callsign());
            }
            void Disable() override
            {
                ASSERT(_service != nullptr);

                Logger().Disable();

                _service->Release();
                _service = nullptr;
            }

            void Activated(PluginHost::IShell&) override
            { 
                _activatetime = Core::Time::Now().Ticks();
                Logger().Activated();
            }

            void Deactivated(PluginHost::IShell&) override
            {
                Logger().Deactivated( Uptime() );
            }

            uint64_t ActivateTime() const 
            {
                return _activatetime;
            }

            uint32_t Uptime() const 
            {
                return ( Core::Time::Now().Ticks() - ActivateTime() ) / Core::Time::MicroSecondsPerSecond;
            }

            CallsignPerfMetricsHandler& Parent() const
            {
                return _parent;
            }

            PluginHost::IShell* Service () const {
                return _service;
            }

        private: 
            CallsignPerfMetricsHandler& _parent;
            uint64_t _activatetime;
            PluginHost::IShell* _service;
        };

        template<class LOGGERINTERFACE = IStateMetricsLogger>
        class StateObservable : public BasicObservable<LOGGERINTERFACE>, public PluginHost::IStateControl::INotification {
        private:
            using Base = BasicObservable<LOGGERINTERFACE>;

        protected:
            // make Logger accessable here, so we don't have to put this-> in front of it everywhere
            using LoggerProxy<LOGGERINTERFACE>::Logger;

        public:
            StateObservable(const StateObservable&) = delete;
            StateObservable& operator=(const StateObservable&) = delete;

            StateObservable(CallsignPerfMetricsHandler& parent, PluginHost::IShell& service, PluginHost::IStateControl* statecontrol) 
            : Base(parent, service)
            , PluginHost::IStateControl::INotification()
            , _statecontrol(statecontrol)
            {
                // not very likely but it could happen that we have a Browser without StatControl
                if(_statecontrol != nullptr) {
                    TRACE(Trace::Information, (_T("Observable supports Statecontrol")) );
                    _statecontrol->AddRef();
                }
            }

            ~StateObservable() override
            {
                ASSERT(_statecontrol == nullptr);
            }

            void Enable() override
            {
                Base::Enable();

                if(_statecontrol != nullptr) {
                    
                    _statecontrol->Register(this);
                }
            }

            void Disable() override
            {
                Cleanup();
                Base::Disable();
            }

            void StateChange(const PluginHost::IStateControl::state state) override 
            {
                if( state == PluginHost::IStateControl::state::RESUMED ) {
                    Logger().Resumed();
                } else if( state == PluginHost::IStateControl::state::SUSPENDED ) {
                    Logger().Suspended();
                }
            }

            BEGIN_INTERFACE_MAP(StateObservable)
                INTERFACE_ENTRY(PluginHost::IStateControl::INotification)
            END_INTERFACE_MAP

        private:
            void Cleanup() 
            {
                if(_statecontrol != nullptr) {
                    _statecontrol->Unregister(this);
                    _statecontrol->Release();
                    _statecontrol = nullptr;
                }
            }

        private:
            PluginHost::IStateControl* _statecontrol;
        };

        template<class LOGGERINTERFACE = IBrowserMetricsLogger>
        class BrowserObservable : public StateObservable<LOGGERINTERFACE>, public Exchange::IBrowser::INotification {
        private:
            using Base = StateObservable<LOGGERINTERFACE>;

        protected:
            // make Logger accessable here, so we don't have to put this-> in front of it everywhere
            using LoggerProxy<LOGGERINTERFACE>::Logger;

        public:
            BrowserObservable(const BrowserObservable&) = delete;
            BrowserObservable& operator=(const BrowserObservable&) = delete;

            BrowserObservable(CallsignPerfMetricsHandler& parent, 
                              PluginHost::IShell& service, 
                              Exchange::IBrowser& browser,
                              PluginHost::IStateControl* statecontrol) 
            : Base(parent, service, statecontrol)
            , Exchange::IBrowser::INotification()
            , _browser(&browser)
            , _nbrloaded(0)
            {
                _browser->AddRef();
            }

            ~BrowserObservable() override 
            {
                 ASSERT(_browser = nullptr);
            }

            void Enable() override
            {
                 ASSERT(_browser != nullptr);

                Base::Enable();
                _browser->Register(this);
                _nbrloaded = 0;
            }

            void Disable() override
            {
                ASSERT(_browser != nullptr);
                _browser->Unregister(this);
                _browser->Release();
                _browser = nullptr;

                Base::Disable();
            }

            BEGIN_INTERFACE_MAP(BrowserObservable)
                INTERFACE_ENTRY(PluginHost::IStateControl::INotification)
                INTERFACE_ENTRY(Exchange::IBrowser::INotification)
            END_INTERFACE_MAP

            void LoadFinished(const string& URL) override 
            {
                if( URL != IBrowserMetricsLogger::startURL ) {
                    ++_nbrloaded;
                }
                Logger().LoadFinished(URL, 0, true, _nbrloaded, 0);
            }
            void URLChanged(const string& URL) override
            {
                Logger().URLChange(URL, false);
            }
            void Hidden(const bool hidden) override
            {
                Logger().VisibilityChange(hidden);
            }
            void Closure() override
            {
                Logger().PageClosure();
            }

        private:
            Exchange::IBrowser* _browser;
            uint32_t _nbrloaded;
        };

        template<class LOGGERINTERFACE = IBrowserMetricsLogger>
        class WebBrowserObservable : public StateObservable<LOGGERINTERFACE>, public Exchange::IWebBrowser::INotification {
        private:
            using Base = StateObservable<LOGGERINTERFACE>;

        protected:
            // make Logger accessable here, so we don't have to out this-> in front of it everywhere
            using LoggerProxy<LOGGERINTERFACE>::Logger;
        public:
            WebBrowserObservable(const WebBrowserObservable&) = delete;
            WebBrowserObservable& operator=(const WebBrowserObservable&) = delete;

            WebBrowserObservable(CallsignPerfMetricsHandler& parent, 
                                 PluginHost::IShell& service, 
                                 Exchange::IWebBrowser& browser, 
                                 PluginHost::IStateControl* statecontrol) 
            : Base(parent, service, statecontrol)
            , Exchange::IWebBrowser::INotification()
            , _browser(&browser)
            , _nbrloadedsuccess(0)
            , _nbrloadedfailed(0)
            {
                _browser->AddRef();
            }

            ~WebBrowserObservable() override 
            {
                ASSERT(_browser == nullptr);
            }

            void Enable() override
            {
                ASSERT(_browser != nullptr);

                Base::Enable();
                _browser->Register(this);
                _nbrloadedsuccess = 0;
                _nbrloadedfailed = 0;
            }

            void Disable() override
            {
                ASSERT(_browser != nullptr);

                _browser->Unregister(this);
                _browser->Release();
                _browser = nullptr;

                Base::Disable();
            }

            BEGIN_INTERFACE_MAP(BrowserObservable)
                INTERFACE_ENTRY(PluginHost::IStateControl::INotification)
                INTERFACE_ENTRY(Exchange::IWebBrowser::INotification)
            END_INTERFACE_MAP

            void LoadFinished(const string& URL, const int32_t httpstatus) override
            {
                if( URL != IBrowserMetricsLogger::startURL ) {
                    ++_nbrloadedsuccess;
                }
                Logger().LoadFinished(URL, httpstatus, true, _nbrloadedsuccess, _nbrloadedfailed);
            }
            void LoadFailed(const string& URL) override
            {
                if( URL != IBrowserMetricsLogger::startURL ) {
                    ++_nbrloadedfailed;
                }
                Logger().LoadFinished(URL, 0, false, _nbrloadedsuccess, _nbrloadedfailed);
            }
            void URLChange(const string& URL, const bool loaded) override
            {
                Logger().URLChange(URL, loaded);
            }
            void VisibilityChange(const bool hidden) override
            {
                Logger().VisibilityChange(hidden);
            }
            void PageClosure() override
            {
                Logger().PageClosure();
            }
            void BridgeQuery(const string&) override
            {
            }

        private:
            Exchange::IWebBrowser* _browser;
            uint32_t _nbrloadedsuccess;
            uint32_t _nbrloadedfailed;
        };

    public:

PUSH_WARNING(DISABLE_WARNING_THIS_IN_MEMBER_INITIALIZER_LIST)
        PerformanceMetrics()
        : PluginHost::IPlugin()
        , _notification(*this)
        , _handler()
        {
        }
POP_WARNING()
        ~PerformanceMetrics() override = default;

        PerformanceMetrics(const PerformanceMetrics&) = delete;
        PerformanceMetrics& operator=(const PerformanceMetrics&) = delete;

        BEGIN_INTERFACE_MAP(PerformanceMetrics)
        INTERFACE_ENTRY(PluginHost::IPlugin)
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

    private:

        void PluginActivated(PluginHost::IShell& service);
        void PluginDeactivated(PluginHost::IShell& service);

    private:
        Core::Sink<Notification> _notification;
        std::unique_ptr<IPerfMetricsHandler> _handler;
    };

}
}



