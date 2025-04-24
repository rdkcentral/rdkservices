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

#include "../../Module.h"
#include <interfaces/IConfiguration.h>

#include <fstream>
#include <sys/utsname.h>

namespace WPEFramework {
namespace Plugin {
    class DeviceImplementation : public PluginHost::ISubSystem::IIdentifier, public Exchange::IConfiguration {
    private:
        static uint8_t constexpr MacSize = 6;

    private:
        class Config : public Core::JSON::Container {
        public:
            Config(const Config&);
            Config& operator=(const Config&);

            Config()
                : Core::JSON::Container()
                , Interface()
            {
                Add(_T("interface"), &Interface);
            }

            ~Config() override
            {
            }

        public:
            Core::JSON::String Interface;
        };

    private:
       class AdapterObserver : public WPEFramework::Core::AdapterObserver::INotification {
        public:
            static int32_t constexpr WaitTime = 5000; //Just wait for 5 seconds

        public:
            AdapterObserver() = delete;
            AdapterObserver(const AdapterObserver&) = delete;
            AdapterObserver& operator=(const AdapterObserver&) = delete;

            AdapterObserver(const string& interface)
                : _signal(false, true)
                , _interface(interface)
                , _observer(this)
            {
                _observer.Open();
                if (_interface.empty() != true) {
                    // Check given interface has valid MAC
                    if (IsInterfaceHasValidMAC(interface) == true) {
                        _signal.SetEvent();
                    }
                } else {
                    // If interface is not given then,
                    // check any of the activated interface has valid MAC
                    if (IsAnyInterfaceHasValidMAC() == true) {
                       _signal.SetEvent();
                    }
                }
            }
            ~AdapterObserver() override
            {
                _observer.Close();
            }

        public:
            virtual void Event(const string& interface) override
            {
                if (_interface.empty() != true) {
                    // Check configured interface has valid MAC
                    if (interface == _interface) {
                         if (IsInterfaceHasValidMAC(interface) == true) {
                           _signal.SetEvent();
                         }
                    }
                } else {
                    // If interface is not configured then,
                    // check activated interface has valid MAC
                    if (IsInterfaceHasValidMAC(interface) == true) {
                       _signal.SetEvent();
                    }
                }
            }
            inline uint32_t WaitForCompletion(int32_t waitTime)
            {
                return _signal.Lock(waitTime);
            }
            const uint8_t* MACAddress()
            {
                return _MACAddressBuffer;
            }

       private:
            inline bool IsAnyInterfaceHasValidMAC()
            {
                bool valid = false;
                Core::AdapterIterator adapters;

                while ((adapters.Next() == true)) {
                    if ((valid = IsValidMAC(adapters)) == true) {
                        break;
                    }
                }
                return valid;
            }
            inline bool IsInterfaceHasValidMAC(const string& interface)
            {
                Core::AdapterIterator adapter(interface);
                return IsValidMAC(adapter);
            }
            inline bool IsValidMAC(Core::AdapterIterator adapter)
            {
                bool valid = false;
                if ((adapter.IsValid() == true) && ((valid = adapter.HasMAC()) == true)) {
                    memset(_MACAddressBuffer, 0, Core::AdapterIterator::MacSize);
                    adapter.MACAddress(_MACAddressBuffer, Core::AdapterIterator::MacSize);
                }
                return valid;
            }

        private:
            Core::Event _signal;
            string _interface;
            Core::AdapterObserver _observer;
            uint8_t _MACAddressBuffer[Core::AdapterIterator::MacSize];
        };
    public:
        DeviceImplementation() = default;
        virtual ~DeviceImplementation() = default;

        DeviceImplementation(const DeviceImplementation&) = delete;
        DeviceImplementation& operator=(const DeviceImplementation&) = delete;

    public:
        uint32_t Configure(PluginHost::IShell* service) override
        {
            if (service) {
                Config config;
                config.FromString(service->ConfigLine());

                _interface = config.Interface.Value();
                UpdateDeviceId();
            }

            return Core::ERROR_NONE;
        }

        uint8_t Identifier(const uint8_t length, uint8_t* buffer) const override
        {
            uint8_t ret = 0;
            if (_identifier.length()) {
                ret = (_identifier.length() > length ? length : _identifier.length());
                ::memcpy(buffer, _identifier.c_str(), ret);
            }
            return ret;
        }
        string Architecture() const override
        {
            return Core::SystemInfo::Instance().Architecture();
        }
        string Chipset() const override
        {
            return Core::SystemInfo::Instance().Chipset();
        }
        string FirmwareVersion() const override
        {
            return Core::SystemInfo::Instance().FirmwareVersion();
        }

    private:
        void UpdateDeviceId()
        {
            AdapterObserver observer(_interface);

            if (observer.WaitForCompletion(AdapterObserver::WaitTime) == Core::ERROR_NONE) {
                _identifier.assign(reinterpret_cast<const char*>(observer.MACAddress()), Core::AdapterIterator::MacSize);
            } else {
                TRACE(Trace::Error, (_T("There is no any valid physical interface available")));
            }
        }

    public:
        BEGIN_INTERFACE_MAP(DeviceImplementation)
        INTERFACE_ENTRY(PluginHost::ISubSystem::IIdentifier)
        INTERFACE_ENTRY(Exchange::IConfiguration)
        END_INTERFACE_MAP

    private:
        string _interface;
        string _identifier;
    };

    SERVICE_REGISTRATION(DeviceImplementation, 1, 0);

}
}
