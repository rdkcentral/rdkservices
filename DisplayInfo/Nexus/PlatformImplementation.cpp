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

#include <nexus_config.h>
#include <nexus_platform.h>
#include <nxclient.h>

namespace WPEFramework {
namespace Plugin {

class DisplayInfoImplementation : public Exchange::IGraphicsProperties, public Exchange::IConnectionProperties {
public:
    DisplayInfoImplementation()
       : _width(0)
       , _height(0)
       , _connected(false)
       , _major(0)
       , _minor(0)
       , _type(HDR_OFF)
       , _totalGpuRam(0)
       , _audioPassthrough(false)
       , _adminLock()
       , _activity(*this) {

        NEXUS_Error rc = NxClient_Join(NULL);
        ASSERT(!rc);
        NxClient_UnregisterAcknowledgeStandby(NxClient_RegisterAcknowledgeStandby());
        NEXUS_Platform_GetConfiguration(&_platformConfig);

        UpdateTotalGpuRam(_totalGpuRam);

        UpdateDisplayInfo(_connected, _width, _height, _major, _minor, _type);
        UpdateAudioPassthrough(_audioPassthrough);

        RegisterCallback();
    }

    DisplayInfoImplementation(const DisplayInfoImplementation&) = delete;
    DisplayInfoImplementation& operator= (const DisplayInfoImplementation&) = delete;
    virtual ~DisplayInfoImplementation()
    {
        NxClient_StopCallbackThread();
        NxClient_Uninit();
    }

public:
    // Graphics Properties interface
    uint64_t TotalGpuRam() const override
    {
        return _totalGpuRam;
    }
    uint64_t FreeGpuRam() const override
    {
        uint64_t freeRam = 0;
        NEXUS_MemoryStatus status;

        NEXUS_Error rc = NEXUS_UNKNOWN;
#if NEXUS_MEMC0_GRAPHICS_HEAP
        if (_platformConfig.heap[NEXUS_MEMC0_GRAPHICS_HEAP]) {
            rc = NEXUS_Heap_GetStatus(_platformConfig.heap[NEXUS_MEMC0_GRAPHICS_HEAP], &status);
            if (rc == NEXUS_SUCCESS) {
                freeRam = static_cast<uint64_t>(status.free);
            }
        }
#endif
#if NEXUS_MEMC1_GRAPHICS_HEAP
        if (_platformConfig.heap[NEXUS_MEMC1_GRAPHICS_HEAP]) {
            rc = NEXUS_Heap_GetStatus(_platformConfig.heap[NEXUS_MEMC1_GRAPHICS_HEAP], &status);
            if (rc == NEXUS_SUCCESS) {
                freeRam += static_cast<uint64_t>(status.free);
            }
        }
#endif
#if NEXUS_MEMC2_GRAPHICS_HEAP
        if (_platformConfig.heap[NEXUS_MEMC2_GRAPHICS_HEAP]) {
            rc = NEXUS_Heap_GetStatus(_platformConfig.heap[NEXUS_MEMC2_GRAPHICS_HEAP], &status);
            if (rc == NEXUS_SUCCESS) {
                freeRam  += static_cast<uint64_t>(status.free);
            }
        }
#endif
        return (freeRam);
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
    uint8_t HDCPMajor() const override
    {
        return _major;
    }
    uint8_t HDCPMinor() const override
    {
        return _minor;
    }
    HDRType Type() const override
    {
        return _type;
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
        NEXUS_MemoryStatus status;
        NEXUS_Error rc = NEXUS_UNKNOWN;

#if NEXUS_MEMC0_GRAPHICS_HEAP
        if (_platformConfig.heap[NEXUS_MEMC0_GRAPHICS_HEAP]) {
            rc = NEXUS_Heap_GetStatus(_platformConfig.heap[NEXUS_MEMC0_GRAPHICS_HEAP], &status);
            if (rc == NEXUS_SUCCESS) {
                totalRam = static_cast<uint64_t>(status.size);
            }
        }
#endif
#if NEXUS_MEMC1_GRAPHICS_HEAP
        if (_platformConfig.heap[NEXUS_MEMC1_GRAPHICS_HEAP]) {
            rc = NEXUS_Heap_GetStatus(_platformConfig.heap[NEXUS_MEMC1_GRAPHICS_HEAP], &status);
            if (rc == NEXUS_SUCCESS) {
                totalRam += static_cast<uint64_t>(status.size);
            }
        }
#endif
#if NEXUS_MEMC2_GRAPHICS_HEAP
        if (_platformConfig.heap[NEXUS_MEMC2_GRAPHICS_HEAP]) {
            rc = NEXUS_Heap_GetStatus(_platformConfig.heap[NEXUS_MEMC2_GRAPHICS_HEAP], &status);
            if (rc == NEXUS_SUCCESS) {
                totalRam += static_cast<uint64_t>(status.size);
            }
        }
#endif
    }

    inline void UpdateAudioPassthrough(bool& audioPassthrough)
    {
        NxClient_AudioStatus status;
        NEXUS_Error rc = NEXUS_UNKNOWN;
        rc = NxClient_GetAudioStatus(&status);
        if (rc == NEXUS_SUCCESS) {
            if ((status.hdmi.outputMode != NxClient_AudioOutputMode_eNone) && (status.hdmi.outputMode < NxClient_AudioOutputMode_eMax)) {
                audioPassthrough = true;
            }
        }
    }
    inline void UpdateDisplayInfo(bool& connected, uint32_t& width, uint32_t& height, uint8_t& major, uint8_t& minor, HDRType type) const
    {
        NEXUS_Error rc = NEXUS_SUCCESS;

        NEXUS_HdmiOutputHandle hdmiOutput;
        hdmiOutput = NEXUS_HdmiOutput_Open(NEXUS_ALIAS_ID + 0, NULL);
        if (hdmiOutput) {
            NEXUS_HdmiOutputStatus status;
            rc = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status);
            if (rc == NEXUS_SUCCESS) {
                connected = status.connected;
            }

            NxClient_DisplaySettings displaySettings;
            NxClient_GetDisplaySettings(&displaySettings);
#if defined(DISPLAYINFO_BCM_VERSION_MAJOR) && (DISPLAYINFO_BCM_VERSION_MAJOR > 18)
            // Read HDR status
            switch (displaySettings.hdmiPreferences.dynamicRangeMode) {
            case NEXUS_VideoDynamicRangeMode_eHdr10: {
                type = HDR_10;
                break;
            }
            case NEXUS_VideoDynamicRangeMode_eHdr10Plus: {
                type = HDR_10PLUS;
                break;
            }
#else
            switch  (displaySettings.hdmiPreferences.drmInfoFrame.eotf) {
            case NEXUS_VideoEotf_eHdr10: {
                type = HDR_10;
                break;
            }
#endif
            default:
                break;
            }


            // Check HDCP version
            NEXUS_HdmiOutputHdcpStatus hdcpStatus;
            rc = NEXUS_HdmiOutput_GetHdcpStatus(hdmiOutput, &hdcpStatus);

            if (rc  == NEXUS_SUCCESS) {
#if defined(DISPLAYINFO_BCM_VERSION_MAJOR) && (DISPLAYINFO_BCM_VERSION_MAJOR >= 18)
                if (hdcpStatus.selectedHdcpVersion == NEXUS_HdcpVersion_e2x) {
#else
                if (hdcpStatus.hdcp2_2Features == true) {
#endif
                    major = 2;
                    minor = 2;
                } else {
                    major = 1;
                    minor = 1;
                }
            } else {
                major = 0;
                minor = 0;
            }
        }

        // Read display width and height
        NEXUS_DisplayCapabilities capabilities;
        NEXUS_GetDisplayCapabilities(&capabilities);
        width = capabilities.display[0].graphics.width;
        height = capabilities.display[0].graphics.height;
    }
    void RegisterCallback()
    {
        NxClient_CallbackThreadSettings settings;
        NxClient_GetDefaultCallbackThreadSettings(&settings);

        settings.hdmiOutputHotplug.callback = Callback;
        settings.hdmiOutputHotplug.context = reinterpret_cast<void*>(this);
        settings.hdmiOutputHotplug.param = 0;

        settings.displaySettingsChanged.callback = Callback;
        settings.displaySettingsChanged.context = reinterpret_cast<void*>(this);
        settings.displaySettingsChanged.param = 1;

        if (NxClient_StartCallbackThread(&settings) != NEXUS_SUCCESS) {
            TRACE_L1(_T("Error in starting nexus callback thread"));
        }
    }
    static void Callback(void *cbData, int param)
    {
        DisplayInfoImplementation* platform = static_cast<DisplayInfoImplementation*>(cbData);

        switch (param) {
        case 0:
        case 1: {
            platform->UpdateDisplayInfo();
            break;
        }
        default:
            break;
        }
    }
    void UpdateDisplayInfo()
    {
        _adminLock.Lock();
        UpdateDisplayInfo(_connected, _width, _height, _major, _minor, _type);
        _adminLock.Unlock();

        _activity.Submit();
    }

private:
    uint32_t _width;
    uint32_t _height;
    bool _connected;

    uint8_t _major;
    uint8_t _minor;
    HDRType _type;

    uint64_t _totalGpuRam;
    bool _audioPassthrough;

    std::list<IConnectionProperties::INotification*> _observers;

    NEXUS_PlatformConfiguration _platformConfig;

    mutable Core::CriticalSection _adminLock;

    Core::WorkerPool::JobType<DisplayInfoImplementation&> _activity;
};

    SERVICE_REGISTRATION(DisplayInfoImplementation, 1, 0);
}
}
