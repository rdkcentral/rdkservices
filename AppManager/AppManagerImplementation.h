/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2024 RDK Management
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
#include <interfaces/Ids.h>
#include <interfaces/IAppManager.h>
#include "EventListener.h"
#include "WebSocket.h"
#include "RDKShellAdapter.h"
#include <map>

//#include "tracing/Logging.h"

//#include <com/com.h>
//#include <core/core.h>
//#include <plugins/plugins.h>

#define APPMANAGER_NAMESPACE "AppManager"

namespace WPEFramework {
namespace Plugin {

    using namespace AppManagerV1;

    class AppManagerImplementation : public Exchange::IAppManager, public EventListener {

    public:
        AppManagerImplementation();
        ~AppManagerImplementation() override;

        // We do not allow this plugin to be copied !!
        AppManagerImplementation(const AppManagerImplementation&) = delete;
        AppManagerImplementation& operator=(const AppManagerImplementation&) = delete;

        BEGIN_INTERFACE_MAP(AppManagerImplementation)
        INTERFACE_ENTRY(Exchange::IAppManager)
        END_INTERFACE_MAP

    public:

        enum EventNames {
          APPSTATECHANGED
        };
        class EXTERNAL Job : public Core::IDispatch {
        protected:
             Job(AppManagerImplementation *appManagerImplementation, EventNames event, JsonObject &params)
                : mAppManagerImplementation(appManagerImplementation)
                , _event(event)
                , _params(params) {
                if (mAppManagerImplementation != nullptr) {
                    mAppManagerImplementation->AddRef();
                }
            }

       public:
            Job() = delete;
            Job(const Job&) = delete;
            Job& operator=(const Job&) = delete;
            ~Job() {
                if (mAppManagerImplementation != nullptr) {
                    mAppManagerImplementation->Release();
                }
            }

       public:
            static Core::ProxyType<Core::IDispatch> Create(AppManagerImplementation *appManagerImplementation, EventNames event, JsonObject params) {
#ifndef USE_THUNDER_R4
                return (Core::proxy_cast<Core::IDispatch>(Core::ProxyType<Job>::Create(appManagerImplementation, event, params)));
#else
                return (Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create(appManagerImplementation, event, params)));
#endif
            }

            virtual void Dispatch() {
                mAppManagerImplementation->Dispatch(_event, _params);
            }
        private:
            AppManagerImplementation *mAppManagerImplementation;
            const EventNames _event;
            const JsonObject _params;
        };

    public:
        virtual Core::hresult Initialize(PluginHost::IShell* service) override;
        virtual void Deinitialize(PluginHost::IShell* service) override;
        virtual uint32_t Register(Exchange::IAppManager::INotification *notification ) override ;
        virtual uint32_t Unregister(Exchange::IAppManager::INotification *notification ) override ;
        virtual uint32_t LaunchApp(const string& appId /* @in */, const string& intent /* @in */, const string& launchArgs /* @in */, bool& success /* @out */) override;
        virtual uint32_t CloseApp(const string& appId /* @in */, bool& success /* @out */) override;
        virtual uint32_t TerminateApp(const string& appId /* @in */, bool& success /* @out */) override;
        virtual uint32_t GetLoadedApps(string& appData /* @out */, bool& success/* @out */) override;
        virtual uint32_t SendIntent(const string& appId /* @in */, const string& intent /* @in */, bool& success /* @out */) override;
        virtual uint32_t PreloadApp(const string& appId /* @in */, const string& launchArgs /* @in */, bool& success /* @out */) override;
	virtual void onAppStateChanged(std::string client, Exchange::IAppManager::LifecycleState newState, Exchange::IAppManager::LifecycleState oldState) override;

    private:
        mutable Core::CriticalSection _adminLock;
        std::list<Exchange::IAppManager::INotification*> mAppManagerNotification;
        RDKShellAdapter* mRDKShellAdapter;
        WebSocketEndPoint* mWSEndPoint;
        int mRippleConnectionId;
        string mFireboltEndpoint;

        void dispatchEvent(EventNames, const JsonObject &params);
        void Dispatch(EventNames event, const JsonObject params);
        friend class Job;
    };
} // namespace Plugin
} // namespace WPEFramework
