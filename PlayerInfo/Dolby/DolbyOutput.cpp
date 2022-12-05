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
#include "Dolby.h"

namespace WPEFramework
{
    namespace Plugin
    {

        class DolbyOutputImplementation : public Exchange::Dolby::IOutput
        {
        public:
            uint32_t Register(Exchange::Dolby::IOutput::INotification* notification) override
            {
                _adminLock.Lock();

                // Make sure a sink is not registered multiple times.
                ASSERT(std::find(_observers.begin(), _observers.end(), notification) == _observers.end());

                _observers.push_back(notification);
                notification->AddRef();

                _adminLock.Unlock();

                return (Core::ERROR_NONE);
            }
            uint32_t Unregister(Exchange::Dolby::IOutput::INotification* notification) override
            {
                _adminLock.Lock();

                std::list<Exchange::Dolby::IOutput::INotification*>::iterator index(std::find(_observers.begin(), _observers.end(), notification));

                // Make sure you do not unregister something you did not register !!!
                ASSERT(index != _observers.end());

                if (index != _observers.end()) {
                    (*index)->Release();
                    _observers.erase(index);
                }

                _adminLock.Unlock();

                return (Core::ERROR_NONE);
            }

            uint32_t AtmosMetadata(bool& supported) const override
            {
                return (Core::ERROR_UNAVAILABLE);
            }

            uint32_t SoundMode(Exchange::Dolby::IOutput::SoundModes& mode) const override
            {
                return (Core::ERROR_UNAVAILABLE);
            }

            uint32_t EnableAtmosOutput(const bool& enable) override
            {
                return (Core::ERROR_UNAVAILABLE);
            }

            uint32_t Mode(const Exchange::Dolby::IOutput::Type& mode) override
            {
                return set_audio_output_type(mode);
            }

            uint32_t Mode(Exchange::Dolby::IOutput::Type& mode) const override
            {
                return get_audio_output_type(&mode);
            }

            BEGIN_INTERFACE_MAP(DolbyOutputImplementation)
                INTERFACE_ENTRY(Exchange::Dolby::IOutput)
            END_INTERFACE_MAP

        private:
            std::list<Exchange::Dolby::IOutput::INotification*> _observers;
            mutable Core::CriticalSection _adminLock;
        };

        SERVICE_REGISTRATION(DolbyOutputImplementation, 1, 0);

    } // namespace Plugin
} // namespace WPEFramework
