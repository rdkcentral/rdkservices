/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 Metrological
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
#include <interfaces/IConfiguration.h>
#include <interfaces/IFireboltPrivacy.h>

namespace WPEFramework {

namespace Plugin
{
    class FireboltPrivacyImplementation : public Exchange::IFireboltPrivacy,
                                         public Exchange::IConfiguration {

    private:
        class AdapterObserver : public WPEFramework::Core::AdapterObserver::INotification {
        public:
            AdapterObserver() = delete;
            AdapterObserver(const AdapterObserver&) = delete;
            AdapterObserver& operator=(const AdapterObserver&) = delete;

PUSH_WARNING(DISABLE_WARNING_THIS_IN_MEMBER_INITIALIZER_LIST)
            AdapterObserver(FireboltPrivacyImplementation& parent)
                : _parent(parent)
                , _adminLock()
                , _reporting()
                , _job(*this)
            {
            }
POP_WARNING()
            ~AdapterObserver() override = default;

        public:
            void Open()
            {
            }
            void Close()
            {

                _adminLock.Lock();

                _reporting.clear();

                _adminLock.Unlock();

                _job.Revoke();
            }
 
            virtual void Event(const string& interface) override
            {

                _adminLock.Lock();

                SYSLOG(Logging::Notification, (_T("Event: %s"), interface));
#if 0
                if (std::find(_reporting.begin(), _reporting.end(), interface) == _reporting.end()) {
                    // We need to add this interface, it is currently not present.
                    _reporting.push_back(interface);

                    _job.Submit();
                }
#endif

                _adminLock.Unlock();
            }
            void Dispatch()
            {
                // Yippie a yee, we have an interface notification:
                _adminLock.Lock();

#if 0
                while (_reporting.size() != 0) {
                    const string interfaceName(_reporting.front());
                    _reporting.pop_front();
                    _adminLock.Unlock();

                    _parent.Activity(interfaceName);

                    _adminLock.Lock();
                }
#endif
                _adminLock.Unlock();
            }

        private:
            FireboltPrivacyImplementation& _parent;
            Core::CriticalSection _adminLock;
            std::list<string> _reporting;
            Core::WorkerPool::JobType<AdapterObserver&> _job;
        };

        class Config : public Core::JSON::Container {
        public:
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

            Config()
                : Core::JSON::Container()
                , DeviceManifest("/etc/firebolt-device-manifest.json")
            {
                Add(_T("devicemanifest"), &DeviceManifest);
            }
            ~Config() override = default;

        public:
            Core::JSON::String DeviceManifest;
        };


    public:
        FireboltPrivacyImplementation(const FireboltPrivacyImplementation&) = delete;
        FireboltPrivacyImplementation& operator= (const FireboltPrivacyImplementation&) = delete;
        FireboltPrivacyImplementation()
            : _adminLock()
            , _config()
            , _deviceManifest()
            , _persistentStoragePath()
            , _allowResumePoints(false)
        {
        }
        ~FireboltPrivacyImplementation() override
        {
            // Stop observing.
            _deviceManifest.clear();
            if (_service) {
                _service->Release();
                _service = nullptr;
            }
        }

        uint32_t Register(Exchange::IFireboltPrivacy::INotification* notification) override
        {
            ASSERT(notification);
            _adminLock.Lock();
            notification->AddRef();
            _notifications.push_back(notification);
            _adminLock.Unlock();

            return Core::ERROR_NONE;
        }

        uint32_t Unregister(Exchange::IFireboltPrivacy::INotification* notification) override
        {
            ASSERT(notification);
            _adminLock.Lock();
            auto item = std::find(_notifications.begin(), _notifications.end(), notification);
            ASSERT(item != _notifications.end());
            _notifications.erase(item);
            (*item)->Release();
            _adminLock.Unlock();

            return Core::ERROR_NONE;
        }
        uint32_t AllowResumePoints(bool& allowResumePoints /* @out */) const override {
            _adminLock.Lock();
            allowResumePoints = _allowResumePoints;
            _adminLock.Unlock();
	    return 0;
        }
        uint32_t SetAllowResumePoints(const bool& allowResumePoints ) override {
            _adminLock.Lock();
            _allowResumePoints = allowResumePoints;
	    for(auto notification: _notifications){
	        notification->OnAllowResumePointsChanged(allowResumePoints);
	    }
            _adminLock.Unlock();
	    return 0;
        }
        uint32_t Configure(PluginHost::IShell* service)  override {
	    ASSERT( service != nullptr);
            SYSLOG(Logging::Notification, (_T("service Config: %s"), service->ConfigLine()));
	    return 0;
        }

        BEGIN_INTERFACE_MAP(FireboltPrivacyImplementation)
        INTERFACE_ENTRY(Exchange::IFireboltPrivacy)
        INTERFACE_ENTRY(Exchange::IConfiguration)
        END_INTERFACE_MAP

    private:
        mutable Core::CriticalSection _adminLock;
        Config _config;
        string _deviceManifest;
        string _persistentStoragePath;
        bool _allowResumePoints;
        PluginHost::IShell* _service;
        std::vector<Exchange::IFireboltPrivacy::INotification*> _notifications;
    };

    SERVICE_REGISTRATION(FireboltPrivacyImplementation, 1, 0)
} // namespace Plugin
} // namespace WPEFramework

