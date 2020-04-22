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
 
#include "../Module.h"
#include <interfaces/IDisplayInfo.h>

#include <bcm_host.h>
#include <fstream>

namespace WPEFramework {
namespace Plugin {

class DisplayInfoImplementation : public Exchange::IGraphicsProperties, public Exchange::IConnectionProperties {

public:
    DisplayInfoImplementation()
        : _width(0)
        , _height(0)
        , _connected(false)
        , _totalGpuRam(0)
        , _audioPassthrough(false)
        , _adminLock()
        , _activity(*this) {

        bcm_host_init();

        UpdateTotalGpuRam(_totalGpuRam);

        UpdateDisplayInfo(_connected, _width, _height, _audioPassthrough);
        RegisterDisplayCallback();
    }

    DisplayInfoImplementation(const DisplayInfoImplementation&) = delete;
    DisplayInfoImplementation& operator= (const DisplayInfoImplementation&) = delete;
    virtual ~DisplayInfoImplementation()
    {
        bcm_host_deinit();
    }

public:
    // Graphics Properties interface
    uint64_t TotalGpuRam() const override
    {
        return _totalGpuRam;
    }
    uint64_t FreeGpuRam() const override
    {
        uint64_t result;
        Command("get_mem reloc ", result);
        return (result);
    }

    // Connection Properties interface
    uint32_t Register(INotification* notification) override
    {
        _adminLock.Lock();

        // Make sure a sink is not registered multiple times.
        ASSERT(std::find(_observers.begin(), _observers.end(), notification) == _observers.end());

        _observers.push_back(notification);
        notification->AddRef();

        _adminLock.Unlock();

        return (Core::ERROR_NONE);
    }
    uint32_t Unregister(INotification* notification) override
    {
        _adminLock.Lock();

        std::list<IConnectionProperties::INotification*>::iterator index(std::find(_observers.begin(), _observers.end(), notification));

        // Make sure you do not unregister something you did not register !!!
        ASSERT(index != _observers.end());

        if (index != _observers.end()) {
            (*index)->Release();
            _observers.erase(index);
        }

        _adminLock.Unlock();

        return (Core::ERROR_NONE);
    }

    bool IsAudioPassthrough () const override
    {
        return _audioPassthrough;
    }
    bool Connected() const override
    {
        return _connected;
    }
    uint32_t Width() const override
    {
        return _width;
    }
    uint32_t Height() const override
    {
        return _height;
    }
    // HDCP support is not used for RPI now, it is always settings as DISPMANX_PROTECTION_NONE
    HDCPProtectionType HDCPProtection() const override {
        return HDCPProtectionType::HDCP_Unencrypted;
    }
    HDRType Type() const override
    {
        return HDR_OFF;
    }
    void Dispatch() const
    {
        _adminLock.Lock();

        std::list<IConnectionProperties::INotification*>::const_iterator index = _observers.begin();

        if (index != _observers.end()) {
            (*index)->Updated();
        }

        _adminLock.Unlock();
    }

    BEGIN_INTERFACE_MAP(DisplayInfoImplementation)
        INTERFACE_ENTRY(Exchange::IGraphicsProperties)
        INTERFACE_ENTRY(Exchange::IConnectionProperties)
    END_INTERFACE_MAP

private:
    inline void UpdateTotalGpuRam(uint64_t& totalRam) const
    {
        Command("get_mem reloc_total ", totalRam);
    }
    void Command(const char request[], string& value) const
    {
        char buffer[512];

        // Reset the string
        buffer[0] = '\0';

        // Most VC API calls are guarded but we want to be sure anyway
        _adminLock.Lock();

        int VARIABLE_IS_NOT_USED status = vc_gencmd(buffer, sizeof(buffer), &request[0]);
        assert((status == 0) && "Error: vc_gencmd failed.\n");

        _adminLock.Unlock();

        // Make sure it is null-terminated
        buffer[sizeof(buffer) - 1] = '\0';

        // We do not need the stuff that is before the '=', we know what we requested :-)
        char* equal = strchr(buffer, '=');
        if (equal != nullptr) {
            equal++;
        }
        else {
            equal = buffer;
        }

        // Create string from buffer.
        Core::ToString(equal, value);
    }
    template<typename VALUE>
    void Command(const char request[], VALUE& result) const
    {
        string response;

        Command(request, response);

        const char* start = response.c_str();
        const char* index = start;

        // move the index to the unit inidicatuion type
        while (::isdigit(*index) || (*index == ' ') || (*index == '.') || (*index == ',')) {
            index++;
        }

        result = WPEFramework::Core::NumberType<VALUE>(WPEFramework::Core::TextFragment(start, (index - start))).Value();

        // Convert into bytes, if necessary.
        if ( (*index == 'M') && (index[1] == '\0') ) {
            // Multiply with MB
            result *= (1024 * 1024);
        }
        else if ( (*index == 'K') && (index[1] == '\0') ) {
            // Multiply with KB
            result *= 1024;
        }
    }

    inline void UpdateDisplayInfo(bool& connected, uint32_t& width, uint32_t& height, bool& audioPassthrough) const
    {
        TV_DISPLAY_STATE_T tvState;
        if (vc_tv_get_display_state(&tvState) == 0) {

            if (tvState.display.hdmi.width && tvState.display.hdmi.height) {
                width = tvState.display.hdmi.width;
                height = tvState.display.hdmi.height;
            }
            if ((tvState.state & VC_HDMI_ATTACHED) || ((tvState.state & VC_SDTV_ATTACHED) && ((tvState.state & VC_SDTV_NTSC) || (tvState.state & VC_SDTV_PAL))))  {
                connected = true;
            } else {
                connected = false;
            }
            if (tvState.state & VC_HDMI_HDMI) {
                audioPassthrough = true;
            } else {
                audioPassthrough = false;
            }
        }
    }
    inline void RegisterDisplayCallback()
    {
        vc_tv_register_callback(&DisplayCallback, reinterpret_cast<void*>(this));
    }
    static void DisplayCallback(void *cbData, uint32_t reason, uint32_t, uint32_t)
    {
        DisplayInfoImplementation* platform = static_cast<DisplayInfoImplementation*>(cbData);
        ASSERT(platform != nullptr);

        if (platform != nullptr) {
            switch (reason) {
            case VC_HDMI_UNPLUGGED:
            case VC_SDTV_UNPLUGGED:
            case VC_HDMI_ATTACHED:
            case VC_SDTV_ATTACHED: {
                platform->UpdateDisplayInfo();
                break;
            }
            default: {
                // Ignore all other reasons
                break;
            }
            }
        }
    }
    void UpdateDisplayInfo()
    {
        _adminLock.Lock();
        UpdateDisplayInfo(_connected, _width, _height, _audioPassthrough);
        _adminLock.Unlock();

        _activity.Submit();
    }

private:
    uint32_t _width;
    uint32_t _height;
    bool _connected;
    uint64_t _totalGpuRam;
    bool _audioPassthrough;

    std::list<IConnectionProperties::INotification*> _observers;

    mutable Core::CriticalSection _adminLock;

    Core::WorkerPool::JobType< DisplayInfoImplementation& > _activity;
};

    SERVICE_REGISTRATION(DisplayInfoImplementation, 1, 0);
}
}
