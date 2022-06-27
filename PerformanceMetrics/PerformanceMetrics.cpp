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
 
#include "PerformanceMetrics.h"

namespace WPEFramework {
namespace Plugin {

    namespace {

        static Metadata<PerformanceMetrics> metadata(
            // Version
            1, 0, 0,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {} 
        );
    }

    const string PerformanceMetrics::Initialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);

        Config config;
        config.FromString(service->ConfigLine());

        string result;

        if( ( config.ObservableCallsign.IsSet() == true ) && (config.ObservableClassname.IsSet() == true) ) {
            result = _T("Both callsign and classname set to observe for metrics");
        }
        else if( ( config.ObservableCallsign.IsSet() == true ) && ( config.ObservableCallsign.Value().empty() == false ) ) {
            _handler.reset(new CallsignPerfMetricsHandler(config.ObservableCallsign.Value()));
        }
        else if( ( config.ObservableClassname.IsSet() == true ) && ( config.ObservableClassname.Value().empty() == false ) ) {
            _handler.reset(new ClassnamePerfMetricsHandler(config.ObservableClassname.Value()));
        } else {
            result = _T("No callsign or classname set to observe for metrics");
        }

        if( result.empty() == true ) {
            ASSERT(_handler);
            _handler->Initialize();
            service->Register(&_notification);
        } else {
            Deinitialize(service);
        }

        return result;
    }

    void PerformanceMetrics::Deinitialize(PluginHost::IShell* service)
    {
        if( _handler ) {
            service->Unregister(&_notification);
            // as we do this after the unregister the call to Deinitialize should be threadsafe, no more notifications can be received
            // if the deactivate of the observable did not happen we must clean up here
            _handler->Deinitialize();
            _handler.reset();
        }
    }

    string PerformanceMetrics::Information() const
    {
        return (_T(""));
    }

    void PerformanceMetrics::PluginActivated(PluginHost::IShell& service) 
    {
        ASSERT(_handler);

        _handler->Activated(service);
    }

    void PerformanceMetrics::PluginDeactivated(PluginHost::IShell& service) 
    {
        ASSERT(_handler);

        _handler->Deactivated(service);
    }

    void PerformanceMetrics::CallsignPerfMetricsHandler::CreateObservable(PluginHost::IShell& service)
    {
        Exchange::IWebBrowser* webbrowser = service.QueryInterface<Exchange::IWebBrowser>();
        PluginHost::IStateControl* statecontrol = service.QueryInterface<PluginHost::IStateControl>();

        if( webbrowser != nullptr ) {
            TRACE(Trace::Information, (_T("Start oberserving %s as webbrowser"), Callsign().c_str()) );
            _observable = Core::ProxyType<WebBrowserObservable<>>::Create(*this, service, *webbrowser, statecontrol);
            webbrowser->Release();
        } else {
            Exchange::IBrowser* browser = service.QueryInterface<Exchange::IBrowser>();

            if( browser != nullptr ) {
                TRACE(Trace::Information, (_T("Start oberserving %s as browser"), Callsign().c_str()) );
                _observable = Core::ProxyType<BrowserObservable<>>::Create(*this, service, *browser, statecontrol);
                browser->Release();
            }
            else if( statecontrol != nullptr ) {
                TRACE(Trace::Information, (_T("Start oberserving %s as statecontrol"), Callsign().c_str()) );
                _observable = Core::ProxyType<StateObservable<>>::Create(*this, service, statecontrol);
            } else {
                TRACE(Trace::Information, (_T("Start oberserving %s as basic"), Callsign().c_str()) );
                _observable = Core::ProxyType<BasicObservable<>>::Create(*this, service);
            }
        }

        ASSERT(_observable.IsValid() == true);
        _observable->Enable();

        if( statecontrol != nullptr ) {
            statecontrol->Release();
        }
    }

    constexpr char PerformanceMetrics::IBrowserMetricsLogger::startURL[];

}
}
