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
#include "../ExtendedDisplayIdentification.h"

#include <interfaces/IDisplayInfo.h>
#include <interfaces/IConfiguration.h>

#include <bcm_host.h>
#include <fstream>

namespace WPEFramework {
namespace Plugin {

class DisplayInfoImplementation : public Exchange::IHDRProperties,
                                  public Exchange::IGraphicsProperties,
                                  public Exchange::IConnectionProperties,
                                  public Exchange::IDisplayProperties,
                                  public Exchange::IConfiguration {

public:
    DisplayInfoImplementation(const DisplayInfoImplementation&) = delete;
    DisplayInfoImplementation& operator= (const DisplayInfoImplementation&) = delete;

    DisplayInfoImplementation()
        : _width(0)
        , _height(0)
        , _connected(false)
        , _totalGpuRam(0)
        , _audioPassthrough(false)
        , _EDID()
        , _value(HDCP_Unencrypted)
        , _observers()
        , _adminLock()
        , _activity(*this) {

        bcm_host_init();

        UpdateTotalGpuRam(_totalGpuRam);

        Dispatch();

        vc_tv_register_callback(&DisplayCallback, reinterpret_cast<void*>(this));
    }
    ~DisplayInfoImplementation() override
    {
        bcm_host_deinit();
    }

    uint32_t Configure(PluginHost::IShell* framework) override
    {
        Config config;
        config.FromString(framework->ConfigLine());
        _value = config.hdcpLevel.Value();

        return Core::ERROR_NONE;
    }

public:
    // Graphics Properties interface
    uint32_t TotalGpuRam(uint64_t& total) const override
    {
        total = _totalGpuRam;
        return (Core::ERROR_NONE);
    }
    uint32_t FreeGpuRam(uint64_t& free) const override
    {
        uint64_t result;
        Command("get_mem reloc ", result);
        free = (result);
        return (Core::ERROR_NONE);
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
    uint32_t IsAudioPassthrough (bool& passthru) const override
    {
        passthru = _audioPassthrough;
        return (Core::ERROR_NONE);
    }
    uint32_t Connected(bool& isconnected) const override
    {
        isconnected = _connected;
        return (Core::ERROR_NONE);
    }
    uint32_t Width(uint32_t& width) const override
    {
        width = _width;
        return (Core::ERROR_NONE);
    }
    uint32_t Height(uint32_t& height) const override
    {
        height = _height;
        return (Core::ERROR_NONE);
    }
    uint32_t VerticalFreq(uint32_t& vf) const override
    {
        vf = ~0;
        return (Core::ERROR_NONE);
    }
    uint32_t HDCPProtection(HDCPProtectionType& value) const override
    {
        value = _value;
        return (Core::ERROR_NONE);
    }
    uint32_t HDCPProtection(const HDCPProtectionType value) override
    {
        _value = value;
        return (Core::ERROR_NONE);
    }
    uint32_t EDID (uint16_t& length, uint8_t data[]) const override
    {
        length = _EDID.Raw(length, data);
        return (Core::ERROR_NONE);
    }
    uint32_t WidthInCentimeters(uint8_t& width) const override
    {
        width = _EDID.WidthInCentimeters();
        return width ? (Core::ERROR_NONE) : Core::ERROR_UNAVAILABLE;
    }
    uint32_t HeightInCentimeters(uint8_t& height) const override
    {
        height = _EDID.HeightInCentimeters();
        return height ? (Core::ERROR_NONE) : Core::ERROR_UNAVAILABLE;
    }
    uint32_t PortName(string&) const override
    {
        return (Core::ERROR_UNAVAILABLE);
    }
    uint32_t TVCapabilities(IHDRIterator*&) const override
    {
        return (Core::ERROR_UNAVAILABLE);
    }
    uint32_t STBCapabilities(IHDRIterator*&) const override
    {
        return (Core::ERROR_UNAVAILABLE);
    }
    uint32_t HDRSetting(HDRType& type) const override
    {
        type = HDR_OFF;
        return (Core::ERROR_NONE);
    }

    void Dispatch()
    {
        TV_DISPLAY_STATE_T tvState;
        if (vc_tv_get_display_state(&tvState) == 0) {

            if (tvState.display.hdmi.width && tvState.display.hdmi.height) {
                _width = tvState.display.hdmi.width;
                _height = tvState.display.hdmi.height;
            }
            if ((tvState.state & VC_HDMI_ATTACHED) || ((tvState.state & VC_SDTV_ATTACHED) && ((tvState.state & VC_SDTV_NTSC) || (tvState.state & VC_SDTV_PAL))))  {
                _connected = true;
            } else {
                _connected = false;
            }
            if (tvState.state & VC_HDMI_HDMI) {
                _audioPassthrough = true;
            } else {
                _audioPassthrough = false;
            }
        }

        TRACE(Trace::Information, (_T("Display event; resolution: %dx%d; state: %s; audio: %s passthrough"), 
                _width, _height, 
                (_connected) ? "connected" : "disconnected",
                (_audioPassthrough) ? "is" : "is not" )
        );

        _adminLock.Lock();

        if (_connected == true) {
            RetrieveEDID(_EDID, -1);
        }
        else {
            _EDID.Clear();
        }

        for (auto const& index : _observers) {
            index->Updated(Exchange::IConnectionProperties::INotification::Source::HDMI_CHANGE);
        }

        _adminLock.Unlock();
    }

    uint32_t ColorSpace(ColourSpaceType&) const override
    {
        return (Core::ERROR_UNAVAILABLE);
    }

    uint32_t FrameRate(FrameRateType&) const override
    {
        return (Core::ERROR_UNAVAILABLE);
    }

    uint32_t ColourDepth(ColourDepthType&) const override
    {
        return (Core::ERROR_UNAVAILABLE);
    }

    uint32_t Colorimetry(IColorimetryIterator*&) const override
    {
        return (Core::ERROR_UNAVAILABLE);
    }

    uint32_t QuantizationRange(QuantizationRangeType&) const override
    {
        return (Core::ERROR_UNAVAILABLE);
    }

    uint32_t EOTF(EotfType&) const override
    {
        return (Core::ERROR_UNAVAILABLE);
    }

    BEGIN_INTERFACE_MAP(DisplayInfoImplementation)
        INTERFACE_ENTRY(Exchange::IHDRProperties)
        INTERFACE_ENTRY(Exchange::IGraphicsProperties)
        INTERFACE_ENTRY(Exchange::IConnectionProperties)
        INTERFACE_ENTRY(Exchange::IDisplayProperties)
        INTERFACE_ENTRY(Exchange::IConfiguration)
    END_INTERFACE_MAP

private:

    class Config : public Core::JSON::Container {
        public:
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

            Config()
                : Core::JSON::Container()
                , hdcpLevel(Exchange::IConnectionProperties::HDCPProtectionType::HDCP_Unencrypted)
            {
                Add(_T("hdcplevel"), &hdcpLevel);
            }

            Core::JSON::EnumType<Exchange::IConnectionProperties::HDCPProtectionType> hdcpLevel;
    };

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
    static void DisplayCallback(void *cbData, uint32_t reason, uint32_t, uint32_t)
    {
        DisplayInfoImplementation* platform = static_cast<DisplayInfoImplementation*>(cbData);
        ASSERT(platform != nullptr);

        if (platform != nullptr) {
            switch (reason) {
            case VC_HDMI_UNPLUGGED:
            case VC_SDTV_UNPLUGGED:
            case VC_HDMI_ATTACHED:
            case VC_SDTV_ATTACHED:
            case VC_HDMI_DVI:
            case VC_HDMI_HDMI:
            case VC_HDMI_HDCP_UNAUTH:
            case VC_HDMI_HDCP_AUTH:
            case VC_HDMI_HDCP_KEY_DOWNLOAD:
            case VC_HDMI_HDCP_SRM_DOWNLOAD:
                platform->_activity.Submit();
                break;
            default: {
                // Ignore all other reasons
                break;
            }
            }
        }
    }

    void RetrieveEDID(ExtendedDisplayIdentification& info, int displayId = -1) {
        int size;
        uint8_t  index = 0;

        do {
            if (displayId != -1) {
                #ifdef RPI4 // or higer
                size = vc_tv_hdmi_ddc_read_id(displayId, index * info.Length(), info.Length(), info.Segment(index));
                #endif
            }
            else {
                uint8_t* buffer = info.Segment(index);
                size = vc_tv_hdmi_ddc_read(index * info.Length(), info.Length(), buffer);
            }

            index++;

        } while ( (index < info.Segments()) && (size == info.Length()) );

        TRACE(Trace::Information, (_T("EDID, Read %d segments [%d]"), index, size));
    }

private:
    uint32_t _width;
    uint32_t _height;
    bool _connected;
    uint64_t _totalGpuRam;
    bool _audioPassthrough;
    ExtendedDisplayIdentification _EDID;
    HDCPProtectionType _value;

    std::list<IConnectionProperties::INotification*> _observers;

    mutable Core::CriticalSection _adminLock;

    Core::WorkerPool::JobType< DisplayInfoImplementation& > _activity;
};

    SERVICE_REGISTRATION(DisplayInfoImplementation, 1, 0);
}
}
