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

#include "host.hpp"
#include "exception.hpp"
#include "videoOutputPort.hpp"
#include "videoOutputPortType.hpp"
#include "videoOutputPortConfig.hpp"
#include "videoResolution.hpp"
#include "audioOutputPort.hpp"
#include "audioOutputPortType.hpp"
#include "audioOutputPortConfig.hpp"
#include "manager.hpp"
#include "utils.h"

#if defined(USE_IARM)
#include "libIBus.h"
#include "libIBusDaemon.h"
#include "dsMgr.h"
#endif

#include <string.h>
#include <vector>

namespace WPEFramework {
namespace Plugin {

class DisplayInfoImplementation : public Exchange::IGraphicsProperties, public Exchange::IConnectionProperties, public Exchange::IDisplayProperties {
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
        DisplayInfoImplementation::_instance = this;
        try
        {
#if defined(USE_IARM)
            Utils::IARM::init();
            IARM_Result_t res;
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_PRECHANGE,ResolutionPreChange) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE, ResolutionPostChange) );
#endif
            //TODO: this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
            device::Manager::Initialize();
            TRACE(Trace::Information, (_T("device::Manager::Initialize success")));
        }
        catch(...)
        {
           TRACE(Trace::Error, (_T("device::Manager::Initialize failed")));
        }
    }

    DisplayInfoImplementation(const DisplayInfoImplementation&) = delete;
    DisplayInfoImplementation& operator= (const DisplayInfoImplementation&) = delete;
    virtual ~DisplayInfoImplementation()
    {
        NxClient_StopCallbackThread();
        NxClient_Uninit();
#if defined(USE_IARM)
        IARM_Result_t res;
        IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_PRECHANGE) );
        IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE) );
#endif
        DisplayInfoImplementation::_instance = nullptr;
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
    uint32_t Register(Exchange::IConnectionProperties::INotification* notification) override
    {
        _adminLock.Lock();

        // Make sure a sink is not registered multiple times.
        ASSERT(std::find(_observers.begin(), _observers.end(), notification) == _observers.end());

        _observers.push_back(notification);
        notification->AddRef();

        _adminLock.Unlock();

        return (Core::ERROR_NONE);
    }
    uint32_t Unregister(Exchange::IConnectionProperties::INotification* notification) override
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

    void Dispatch() const
    {
        _adminLock.Lock();

        std::list<IConnectionProperties::INotification*>::const_iterator index = _observers.begin();

        if (index != _observers.end()) {
            (*index)->Updated();
        }

        _adminLock.Unlock();
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


    uint32_t Register(Exchange::IDisplayProperties::INotification* sink)
    {
        _adminLock.Lock();

        // Make sure a sink is not registered multiple times.
        ASSERT(std::find(_notificationClients.begin(), _notificationClients.end(), sink) == _notificationClients.end());

        _notificationClients.push_back(sink);
        sink->AddRef();

        _adminLock.Unlock();

        TRACE_L1("IDisplayProperties : Registered a subscriber %p", sink);
        return (Core::ERROR_NONE);
    }

    uint32_t Unregister(Exchange::IDisplayProperties::INotification* sink)
    {
        _adminLock.Lock();

        std::list<IDisplayProperties::INotification*>::iterator index(std::find(_notificationClients.begin(), _notificationClients.end(), sink));

        // Make sure you do not unregister something you did not register !!!
        ASSERT(index != _notificationClients.end());

        if (index != _notificationClients.end()) {
            (*index)->Release();
            _notificationClients.erase(index);
            TRACE_L1("IDisplayProperties : unregistered a subscriber %p", sink);
        }

        _adminLock.Unlock();
        return (Core::ERROR_NONE);
    }

    static void ResolutionPreChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if(DisplayInfoImplementation::_instance)
        {
           DisplayInfoImplementation::_instance->ResolutionPreChangeImpl();
        }
    }

    void ResolutionPreChangeImpl()
    {
        _adminLock.Lock();

        std::list<Exchange::IDisplayProperties::INotification*>::const_iterator index = _notificationClients.begin();

        if (index != _notificationClients.end()) {
            (*index)->ResolutionPreChange();
        }

        _adminLock.Unlock();
    }

    static void ResolutionPostChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        int dw = 1280;
        int dh = 720;

        if (strcmp(owner, IARM_BUS_DSMGR_NAME) == 0)
        {
            switch (eventId) {
                case IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE:
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    dw = eventData->data.resn.width;
                    dh = eventData->data.resn.height;
                    break;
            }
        }

        if(DisplayInfoImplementation::_instance)
        {
           DisplayInfoImplementation::_instance->ResolutionPostChangeImpl(dh, dw);
        }
    }

    void ResolutionPostChangeImpl(int dh, int dw)
    {
        TRACE(Trace::Information, (_T("ResolutionPostChangeImpl width = %d, heght = %d"), dw, dh));
        string connectedDisplay = getConnectedVideoDisplay();
        string resolution = getCurrentResolution();

        _adminLock.Lock();

        std::list<Exchange::IDisplayProperties::INotification*>::const_iterator index = _notificationClients.begin();

        if (index != _notificationClients.end()) {
            (*index)->ResolutionChanged(dh, dw, connectedDisplay, resolution);
        }
        _adminLock.Unlock();
    }

    string getCurrentResolution() const override
    {
        string currentResolution = "0";
        try
        {
            device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
            currentResolution = vPort.getResolution().getName();
            TRACE(Trace::Information, (_T("Current video playback resolution = %s"), currentResolution));
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        return currentResolution;
    }

    string getConnectedVideoDisplay() const override
    {
        string displayName;
        try
        {
            device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
            for (size_t i = 0; i < vPorts.size(); i++)
            {
                device::VideoOutputPort &vPort = vPorts.at(i);
                if (vPort.isDisplayConnected())
                {
                    string portName = vPort.getName();
                    displayName = displayName.empty()?  portName : ", " + portName;
                }
            }
            TRACE(Trace::Information, (_T("Connected Video Display = %s"), displayName));
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        return displayName;
    }

    void getTvHdrCapabilities(string& hdrCapabilities, int& capabilities) const override
    {
        capabilities = static_cast<int>(dsHDRSTANDARD_NONE);
        try
        {
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
            if (vPort.isDisplayConnected()) {
                vPort.getTVHDRCapabilities(&capabilities);
            }
            else {
                TRACE(Trace::Error, (_T("getTVHDRCapabilities failure: HDMI0 not connected!")));
            }
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        if(!capabilities)hdrCapabilities.append("none,");
        if(capabilities & dsHDRSTANDARD_HDR10)hdrCapabilities.append("HDR10,");
        if(capabilities & dsHDRSTANDARD_HLG)hdrCapabilities.append("HLG,");
        if(capabilities & dsHDRSTANDARD_DolbyVision)hdrCapabilities.append("DolbyVision,");
        if(capabilities & dsHDRSTANDARD_TechnicolorPrime)hdrCapabilities.append("TechnicolorPrime,");
        if(capabilities & dsHDRSTANDARD_Invalid)hdrCapabilities.append("Invalid,");

        TRACE(Trace::Information, (_T("TV HDR capabilities = %s"), hdrCapabilities.c_str()));
    }

    void getStbHdrCapabilities(string& hdrCapabilities, int& capabilities) const override
    {
        capabilities = static_cast<int>(dsHDRSTANDARD_NONE);
        try
        {
            device::VideoDevice &device = device::Host::getInstance().getVideoDevices().at(0);
            device.getHDRCapabilities(&capabilities);
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }

        if(!capabilities)hdrCapabilities.append("none,");
        if(capabilities & dsHDRSTANDARD_HDR10)hdrCapabilities.append("HDR10,");
        if(capabilities & dsHDRSTANDARD_HLG)hdrCapabilities.append("HLG,");
        if(capabilities & dsHDRSTANDARD_DolbyVision)hdrCapabilities.append("DolbyVision,");
        if(capabilities & dsHDRSTANDARD_TechnicolorPrime)hdrCapabilities.append("TechnicolorPrime,");
        if(capabilities & dsHDRSTANDARD_Invalid)hdrCapabilities.append("Invalid,");

        TRACE(Trace::Information, (_T("STB HDR capabilities = %s"), hdrCapabilities.c_str()));
    }

    bool isOutputHDR() const override
    {
        bool isHdr = false;
        try
        {
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
            if (vPort.isDisplayConnected()) {
                isHdr = vPort.IsOutputHDR();
            }
            else
            {
                TRACE(Trace::Information, (_T("IsOutputHDR failure: HDMI0 not connected!")));
            }
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        TRACE(Trace::Information, (_T("Output HDR = %s"), isHdr ? "Yes" : "No"));
        return isHdr;
    }

    int getHdmiPreferences() const override
    {
        int hdcpversion = 0;
        try
        {
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
            if (vPort.isDisplayConnected()) {
                hdcpversion = vPort.GetHdmiPreference();
            }
            else
            {
                TRACE(Trace::Information, (_T("getHdmiPreferences failure: HDMI0 not connected!")));
            }
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        TRACE(Trace::Information, (_T("getHdmiPreferences failure: HDCP protocol in use = %d"), hdcpversion));
        return hdcpversion;
    }

    void setHdmiPreferences(const int HdcpProtocol) override
    {
        dsHdcpProtocolVersion_t hdcpCurrentProtocol = static_cast<dsHdcpProtocolVersion_t>(HdcpProtocol);
        try
        {
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
            if (vPort.isDisplayConnected())
            {
                vPort.SetHdmiPreference(hdcpCurrentProtocol);
            }
            else
            {
                TRACE(Trace::Warning, (_T("setHdmiPreferences failure: HDMI0 not connected!")));
            }
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
    }

    bool isAudioEquivalenceEnabled() const override
    {
        bool isEnbaled = false;
        try
        {
            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI0");
            if (aPort.isConnected()) {
                isEnbaled = aPort.GetLEConfig();
            }
            else
            {
                TRACE(Trace::Information, (_T("isAudioEquivalenceEnabled failure: HDMI0 not connected!")));
            }
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        TRACE(Trace::Information, (_T("Audio Equivalence = %d"), isEnbaled? "Enabled":"Disabled"));
        return isEnbaled;
    }

    string readEDID() const override
    {
        std::vector<uint8_t> edidVec({'u','n','k','n','o','w','n' });
        try
        {
            vector<uint8_t> edidVec2;
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
            if (vPort.isDisplayConnected())
            {
                vPort.getDisplay().getEDIDBytes(edidVec2);
                edidVec = edidVec2;//edidVec must be "unknown" unless we successfully get to this line
            }
            else
            {
                TRACE(Trace::Warning, (_T("readEDID failure: HDMI0 not connected!")));
            }
        }
        catch (const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        //convert to base64
        uint16_t size = min(edidVec.size(), (size_t)numeric_limits<uint16_t>::max());
        if(edidVec.size() > (size_t)numeric_limits<uint16_t>::max())
            TRACE(Trace::Error, (_T("Size too large to use ToString base64 wpe api")));
        string edidbase64;
        Core::ToString((uint8_t*)&edidVec[0], size, false, edidbase64);
        return edidbase64;
    }


    BEGIN_INTERFACE_MAP(DisplayInfoImplementation)
        INTERFACE_ENTRY(Exchange::IGraphicsProperties)
        INTERFACE_ENTRY(Exchange::IDisplayProperties)
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
    std::list<Exchange::IDisplayProperties::INotification*> _notificationClients;

    NEXUS_PlatformConfiguration _platformConfig;

    mutable Core::CriticalSection _adminLock;

    Core::WorkerPool::JobType<DisplayInfoImplementation&> _activity;
    public:
    static DisplayInfoImplementation* _instance;
};
    DisplayInfoImplementation* DisplayInfoImplementation::_instance = nullptr;
    SERVICE_REGISTRATION(DisplayInfoImplementation, 1, 0);
}
}
