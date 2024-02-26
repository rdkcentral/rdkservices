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
        class Config : public Core::JSON::Container {
        public:
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

            Config()
                : Core::JSON::Container()
                , devicemanifest("")
                , fireboltconfigpath("")
            {
                Add(_T("devicemanifest"), &devicemanifest);
                Add(_T("fireboltconfigpath"), &fireboltconfigpath);
            }
            ~Config() override = default;
        public:
            static std::string GetValue(const Core::JSON::String& jsonValue) { return jsonValue.IsSet() ? jsonValue.Value() : ""; }

        public:
            Core::JSON::String devicemanifest;
            // This path is specific to firebolt where it has
            // config that are needed to be shared between Ripple and other FireboltThunderPlugins
            // In prod it points to /etc/
            // and in dev it points to /opt/
            Core::JSON::String fireboltconfigpath;
        };


    public:
        FireboltPrivacyImplementation(const FireboltPrivacyImplementation&) = delete;
        FireboltPrivacyImplementation& operator= (const FireboltPrivacyImplementation&) = delete;
        FireboltPrivacyImplementation()
            : _adminLock()
            , _config()
            , _deviceManifest()
            , _allowResumePoints(false)
        {
        }
        ~FireboltPrivacyImplementation() override
        {
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
            SYSLOG(Logging::Notification, (_T("service Config: %s"), service->ConfigLine().c_str()));
            _config.FromString(service->ConfigLine());
            _deviceManifest = Config::GetValue(_config.fireboltconfigpath) + Config::GetValue(_config.devicemanifest);
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
        bool _allowResumePoints;
        PluginHost::IShell* _service;
        std::vector<Exchange::IFireboltPrivacy::INotification*> _notifications;
    };

    SERVICE_REGISTRATION(FireboltPrivacyImplementation, 1, 0)
} // namespace Plugin
} // namespace WPEFramework

