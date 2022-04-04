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
#include "../DisplayInfoTracing.h"

#include <nexus_config.h>
#include <nexus_platform.h>
#include <nxclient.h>
#include <nexus_core_utils.h>

#if ( (NEXUS_PLATFORM_VERSION_MAJOR > 18) ||  \
      ( (NEXUS_PLATFORM_VERSION_MAJOR == 18) && (NEXUS_PLATFORM_VERSION_MINOR >= 2) ) \
    )

#define NEXUS_HDCPVERSION_SUPPORTED
#define NEXUS_HDR_SUPPORTED

#endif


namespace WPEFramework {
namespace Plugin {

class DisplayInfoImplementation : public Exchange::IGraphicsProperties, public Exchange::IConnectionProperties {
public:
    DisplayInfoImplementation()
       : _width(0)
       , _height(0)
       , _verticalFreq(0)
       , _connected(false)
       , _hdcpprotection(HDCPProtectionType::HDCP_Unencrypted)
       , _type(HDR_OFF)
       , _totalGpuRam(0)
       , _audioPassthrough(false)
       , _adminLock()
       , _activity(*this) {

        VARIABLE_IS_NOT_USED NEXUS_Error rc = NxClient_Join(NULL);
        ASSERT(!rc);
#if defined(DISPLAYINFO_BCM_VERSION_MAJOR) && (DISPLAYINFO_BCM_VERSION_MAJOR > 16)
        NxClient_UnregisterAcknowledgeStandby(NxClient_RegisterAcknowledgeStandby());
#endif
        NEXUS_Platform_GetConfiguration(&_platformConfig);

        UpdateTotalGpuRam(_totalGpuRam);

        NexusHdmiOutput hdmihandle;
        if( hdmihandle ) {
            UpdateDisplayInfoConnected(hdmihandle, _connected);
            UpdateDisplayInfoVerticalFrequency(hdmihandle, _verticalFreq);
            UpdateDisplayInfoHDCP(hdmihandle, _hdcpprotection);
        }
        UpdateDisplayInfoDisplayStatus(_width, _height, _type);

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
    uint32_t VerticalFreq() const override
    {
        return _verticalFreq;
    }
    HDRType Type() const override
    {
        return _type;
    }
    HDCPProtectionType HDCPProtection() const override {
        return _hdcpprotection;
    }
    void Dispatch() const
    {
        _adminLock.Lock();

        std::list<IConnectionProperties::INotification*>::const_iterator index = _observers.begin();

        while(index != _observers.end()) {
            (*index)->Updated();
            index++;
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

    static string NEXUSHdmiOutputHdcpStateToString(const NEXUS_HdmiOutputHdcpState state) {

        struct HdmiOutputHdcpStateStrings {
            NEXUS_HdmiOutputHdcpState state;
            const char* strValue;
        };

        static const HdmiOutputHdcpStateStrings StateToStringTable[] = { 
                                                {NEXUS_HdmiOutputHdcpState_eUnpowered, _T("Unpowered")},
                                                {NEXUS_HdmiOutputHdcpState_eUnauthenticated, _T("Unauthenticated")},
                                                {NEXUS_HdmiOutputHdcpState_eWaitForValidVideo, _T("WaitForValidVideo")},
                                                {NEXUS_HdmiOutputHdcpState_eInitializedAuthentication, _T("InitializedAuthentication")},
                                                {NEXUS_HdmiOutputHdcpState_eWaitForReceiverAuthentication, _T("WaitForReceiverAuthentication")},
                                                {NEXUS_HdmiOutputHdcpState_eReceiverR0Ready, _T("ReceiverR0Ready")},
                                                {NEXUS_HdmiOutputHdcpState_eR0LinkFailure, _T("R0LinkFailure")},
                                                {NEXUS_HdmiOutputHdcpState_eReceiverAuthenticated, _T("ReceiverAuthenticated")},
                                                {NEXUS_HdmiOutputHdcpState_eWaitForRepeaterReady, _T("WaitForRepeaterReady")},
                                                {NEXUS_HdmiOutputHdcpState_eCheckForRepeaterReady, _T("CheckForRepeaterReady")},
                                                {NEXUS_HdmiOutputHdcpState_eRepeaterReady, _T("RepeaterReady")},
                                                {NEXUS_HdmiOutputHdcpState_eLinkAuthenticated, _T("LinkAuthenticated")},
                                                {NEXUS_HdmiOutputHdcpState_eEncryptionEnabled, _T("EncryptionEnabled")},
                                                {NEXUS_HdmiOutputHdcpState_eRepeaterAuthenticationFailure, _T("RepeaterAuthenticationFailure")},
                                                {NEXUS_HdmiOutputHdcpState_eRiLinkIntegrityFailure, _T("RiLinkIntegrityFailure")},
                                                {NEXUS_HdmiOutputHdcpState_ePjLinkIntegrityFailure, _T("PjLinkIntegrityFailure")},
                                              };
        static const HdmiOutputHdcpStateStrings* end = {StateToStringTable + (sizeof(StateToStringTable)/sizeof(HdmiOutputHdcpStateStrings))};
        const HdmiOutputHdcpStateStrings * it = std::find_if(StateToStringTable, end,
                                                           [state](const HdmiOutputHdcpStateStrings& item){
                                                                return item.state == state;
                                                            });

        string result;

        if( it != end ) {
            result = it->strValue;
        } else {
            result = _T("Unknown(") + Core::NumberType<std::underlying_type<NEXUS_HdmiOutputHdcpState>::type>(state).Text() + _T(")");
        }

        return result;
    }

    static string NEXUSHdmiOutputHdcpErrorToString(const NEXUS_HdmiOutputHdcpError error) {

        struct HdmiOutputHdcpErrorStrings {
            NEXUS_HdmiOutputHdcpError error;
            const char* strValue;
        };

        static const HdmiOutputHdcpErrorStrings ErrorToStringTable[] = { 
                                                {NEXUS_HdmiOutputHdcpError_eSuccess, _T("Success")},
                                                {NEXUS_HdmiOutputHdcpError_eRxBksvError, _T("RxBksvError")},
                                                {NEXUS_HdmiOutputHdcpError_eRxBksvRevoked, _T("RxBksvRevoked")},
                                                {NEXUS_HdmiOutputHdcpError_eRxBksvI2cReadError, _T("RxBksvI2cReadError")},
                                                {NEXUS_HdmiOutputHdcpError_eTxAksvError, _T("TxAksvError")},
                                                {NEXUS_HdmiOutputHdcpError_eTxAksvI2cWriteError, _T("TxAksvI2cWriteError")},
                                                {NEXUS_HdmiOutputHdcpError_eReceiverAuthenticationError, _T("ReceiverAuthenticationError")},
                                                {NEXUS_HdmiOutputHdcpError_eRepeaterAuthenticationError, _T("RepeaterAuthenticationError")},
                                                {NEXUS_HdmiOutputHdcpError_eRxDevicesExceeded, _T("RxDevicesExceeded")},
                                                {NEXUS_HdmiOutputHdcpError_eRepeaterDepthExceeded, _T("RepeaterDepthExceeded")},
                                                {NEXUS_HdmiOutputHdcpError_eRepeaterFifoNotReady, _T("RepeaterFifoNotReady")},
                                                {NEXUS_HdmiOutputHdcpError_eRepeaterDeviceCount0, _T("RepeaterDeviceCount")},
                                                {NEXUS_HdmiOutputHdcpError_eRepeaterLinkFailure, _T("RepeaterLinkFailure")},
                                                {NEXUS_HdmiOutputHdcpError_eLinkRiFailure, _T("LinkRiFailure")},
                                                {NEXUS_HdmiOutputHdcpError_eLinkPjFailure, _T("LinkPjFailure")},
                                                {NEXUS_HdmiOutputHdcpError_eFifoUnderflow, _T("FifoUnderflow")},
                                                {NEXUS_HdmiOutputHdcpError_eFifoOverflow, _T("FifoOverflow")},
                                                {NEXUS_HdmiOutputHdcpError_eMultipleAnRequest, _T("MultipleAnRequest")},
                                              };
        static const HdmiOutputHdcpErrorStrings* end = {ErrorToStringTable + (sizeof(ErrorToStringTable)/sizeof(HdmiOutputHdcpErrorStrings))};
        const HdmiOutputHdcpErrorStrings * it = std::find_if(ErrorToStringTable, end,
                                                           [error](const HdmiOutputHdcpErrorStrings& item){
                                                                return item.error == error;
                                                            });

        string result;

        if( it != end ) {
            result = it->strValue;
        } else {
            result = _T("Unknown(") + Core::NumberType<std::underlying_type<NEXUS_HdmiOutputHdcpError>::type>(error).Text() + _T(")");
        }

        return result;
    }

#ifdef NEXUS_HDCPVERSION_SUPPORTED

    static string NEXUSHdcpVersionToString(const NEXUS_HdcpVersion version) {
        string result;
        if( version == NEXUS_HdcpVersion_e1x ) {
            result = _T("1x");
        }
        else if( version == NEXUS_HdcpVersion_e2x ) {
            result = _T("2x");
        }
        else {
            result = _T("Unknown(") + Core::NumberType<std::underlying_type<NEXUS_HdcpVersion>::type>(version).Text() + _T(")");
        }
        return result;
    }

#endif

    class NexusHdmiOutput {
        public:
        NexusHdmiOutput(const NexusHdmiOutput&) = delete;
        NexusHdmiOutput& operator=(const NexusHdmiOutput&) = delete;

        NexusHdmiOutput() : _hdmiOutput(nullptr) {

            _hdmiOutput = NEXUS_HdmiOutput_Open(NEXUS_ALIAS_ID + 0, NULL);

            if( _hdmiOutput == nullptr ) {
                TRACE(Trace::Error, (_T("Error opening Nexus HDMI ouput")));
            }
        }

        ~NexusHdmiOutput() {
            if( _hdmiOutput != nullptr ) {
                NEXUS_HdmiOutput_Close(_hdmiOutput);
            }
        }

        operator bool() const {
            return (_hdmiOutput != nullptr);
        }

        operator NEXUS_HdmiOutputHandle() const { 
            return _hdmiOutput; 
        }

        private:
            NEXUS_HdmiOutputHandle _hdmiOutput;
    };

    void UpdateDisplayInfoConnected(const NEXUS_HdmiOutputHandle& hdmiOutput, bool& connected) const
    {
        NEXUS_HdmiOutputStatus status;
        NEXUS_Error rc = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status);
        if (rc == NEXUS_SUCCESS) {
            connected = status.connected;
        }
    }

    void UpdateDisplayInfoVerticalFrequency(const NEXUS_HdmiOutputHandle& hdmiOutput, uint32_t& verticalFreq) const
    {
        NEXUS_HdmiOutputStatus status;
        NEXUS_Error rc = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status);
        if (rc == NEXUS_SUCCESS) {
            NEXUS_VideoFormat videoFormat = status.preferredVideoFormat;
            NEXUS_VideoFormatInfo videoFormatInfo;
            NEXUS_VideoFormat_GetInfo(videoFormat, &videoFormatInfo);

            // TODO: do we need vertical freq as float, or is nearest uint enough?
            verticalFreq = videoFormatInfo.verticalFreq + 50 / 100; // vertical frequency is stored multiplied by 100
        }
    }

    void UpdateDisplayInfoDisplayStatus(uint32_t& width, uint32_t& height, HDRType& type) const
    {
        NxClient_DisplaySettings displaySettings;
        NxClient_GetDisplaySettings(&displaySettings);
#ifdef NEXUS_HDR_SUPPORTED
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
            default:
                break;
            }
#elif defined(DISPLAYINFO_BCM_VERSION_MAJOR) && (DISPLAYINFO_BCM_VERSION_MAJOR > 16)
        switch  (displaySettings.hdmiPreferences.drmInfoFrame.eotf) {
        case NEXUS_VideoEotf_eHdr10: {
            type = HDR_10;
            break;
        }
        default:
            break;
        }
#endif

        // Read display width and height
        NEXUS_DisplayCapabilities capabilities;
        NEXUS_GetDisplayCapabilities(&capabilities);
        width = capabilities.display[0].graphics.width;
        height = capabilities.display[0].graphics.height;
    }

    void UpdateDisplayInfoHDCP(const NEXUS_HdmiOutputHandle& hdmiOutput, HDCPProtectionType& hdcpprotection) const
    {
        // Check HDCP version
        NEXUS_HdmiOutputHdcpStatus hdcpStatus;
        NEXUS_Error rc = NEXUS_HdmiOutput_GetHdcpStatus(hdmiOutput, &hdcpStatus);

        if (rc  == NEXUS_SUCCESS) {
             TRACE(Trace::Information, (_T(" HDCP Error=[%s]")
                                        _T(" TransmittingEncrypted=[%s]")
                                        _T(" HDCP2.2Features=[%s]")
#ifdef NEXUS_HDCPVERSION_SUPPORTED
                                        _T(" SelectedHDCPVersion=[%s]")
#endif
                                        , NEXUSHdmiOutputHdcpErrorToString(hdcpStatus.hdcpError).c_str()
                                        , hdcpStatus.transmittingEncrypted ? _T("true") : _T("false")
                                        , hdcpStatus.hdcp2_2Features ? _T("true") : _T("false")
#ifdef NEXUS_HDCPVERSION_SUPPORTED
                                        , NEXUSHdcpVersionToString(hdcpStatus.selectedHdcpVersion).c_str()
#endif
                                    ) );

            TRACE(HDCPDetailedInfo, 
                                       (_T("HDCP State=[%s]")
                                        _T(" ReadyForEncryption=[%s]")
                                        _T(" HDCP1.1Features=[%s]")
#ifdef NEXUS_HDCPVERSION_SUPPORTED
                                        _T(" MaxHDCPVersion=[%s]")
#endif
                                        , NEXUSHdmiOutputHdcpStateToString(hdcpStatus.hdcpState).c_str()
                                        , hdcpStatus.linkReadyForEncryption ? _T("true") : _T("false")
                                        , hdcpStatus.hdcp1_1Features ? _T("true") : _T("false")
#ifdef NEXUS_HDCPVERSION_SUPPORTED
                                        , NEXUSHdcpVersionToString(hdcpStatus.rxMaxHdcpVersion).c_str()
#endif
                                    ) );


            if(  hdcpStatus.transmittingEncrypted == false ) {
                hdcpprotection = HDCPProtectionType::HDCP_Unencrypted;
            }  else {

#ifdef NEXUS_HDCPVERSION_SUPPORTED
                if (hdcpStatus.selectedHdcpVersion == NEXUS_HdcpVersion_e2x) {
#else
                if (hdcpStatus.hdcp2_2Features == true) {
#endif
                    hdcpprotection = HDCPProtectionType::HDCP_2X;

                } else {
                    hdcpprotection = HDCPProtectionType::HDCP_1X;
                }
            }
        } else {
            TRACE(Trace::Error, (_T("Error retrieving HDCP status")));
        }
    }

    enum class CallbackType : int { HotPlug, DisplaySettings, HDCP };

    void RegisterCallback()
    {
        NxClient_CallbackThreadSettings settings;
        NxClient_GetDefaultCallbackThreadSettings(&settings);

        settings.hdmiOutputHotplug.callback = Callback;
        settings.hdmiOutputHotplug.context = reinterpret_cast<void*>(this);
        settings.hdmiOutputHotplug.param = static_cast<int>(CallbackType::HotPlug);

        settings.displaySettingsChanged.callback = Callback;
        settings.displaySettingsChanged.context = reinterpret_cast<void*>(this);
        settings.displaySettingsChanged.param = static_cast<int>(CallbackType::DisplaySettings);

        settings.hdmiOutputHdcpChanged.callback = Callback;
        settings.hdmiOutputHdcpChanged.context = reinterpret_cast<void*>(this);
        settings.hdmiOutputHdcpChanged.param = static_cast<int>(CallbackType::HDCP);

        if (NxClient_StartCallbackThread(&settings) != NEXUS_SUCCESS) {
            TRACE_L1(_T("Error in starting nexus callback thread"));
        }
    }
    
    static void Callback(void *cbData, int param)
    {
        DisplayInfoImplementation* platform = static_cast<DisplayInfoImplementation*>(cbData);

        ASSERT(platform != nullptr);

        if( platform != nullptr ) {
            platform->UpdateDisplayInfo(static_cast<CallbackType>(param));
        }
    }

    void UpdateDisplayInfo(const CallbackType callbacktype)
    {
        switch ( callbacktype ) {
        case CallbackType::HotPlug : { 
            NexusHdmiOutput hdmihandle;
            if( hdmihandle ) {
                _adminLock.Lock();
                UpdateDisplayInfoConnected(hdmihandle, _connected);
                _adminLock.Unlock();
            }
            break;
        }
        case CallbackType::DisplaySettings : {  // DiplaySettings Changed
            _adminLock.Lock();
            UpdateDisplayInfoDisplayStatus(_width, _height, _type);
            NexusHdmiOutput hdmihandle;
            if( hdmihandle ) {
                UpdateDisplayInfoVerticalFrequency(hdmihandle, _verticalFreq);
            }
            _adminLock.Unlock();
            break;
        }
        case CallbackType::HDCP : {  // HDCP settings changed
            NexusHdmiOutput hdmihandle;
            if( hdmihandle ) {
                _adminLock.Lock();
                UpdateDisplayInfoHDCP(hdmihandle, _hdcpprotection);
                _adminLock.Unlock();
            }
            break;
        }
        default:
            break;
        }
        _activity.Submit();
    }

private:
    uint32_t _width;
    uint32_t _height;
    uint32_t _verticalFreq;
    bool _connected;

    HDCPProtectionType _hdcpprotection;
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
