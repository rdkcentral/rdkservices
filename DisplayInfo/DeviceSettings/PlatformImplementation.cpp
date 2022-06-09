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
#include "../DisplayInfoTracing.h"
#include "SoC_abstraction.h"

#include <interfaces/IDisplayInfo.h>

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
#include "edid-parser.hpp"
#include "UtilsIarm.h"

#include "libIBus.h"
#include "libIBusDaemon.h"
#include "dsMgr.h"

#define EDID_MAX_HORIZONTAL_SIZE 21
#define EDID_MAX_VERTICAL_SIZE   22

namespace WPEFramework {
namespace Plugin {

class DisplayInfoImplementation :
    public Exchange::IGraphicsProperties,
    public Exchange::IConnectionProperties,
    public Exchange::IHDRProperties,
    public Exchange::IDisplayProperties  {
private:
    using HdrteratorImplementation = RPC::IteratorType<Exchange::IHDRProperties::IHDRIterator>;
    using ColorimetryIteratorImplementation = RPC::IteratorType<Exchange::IDisplayProperties::IColorimetryIterator>;
public:
    DisplayInfoImplementation()
    {
        DisplayInfoImplementation::_instance = this;
        try
        {
            Utils::IARM::init();
            IARM_Result_t res;
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_PRECHANGE,ResolutionChange) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE, ResolutionChange) );

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
        IARM_Result_t res;
        IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_PRECHANGE) );
        IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE) );
        DisplayInfoImplementation::_instance = nullptr;
    }

public:
    // Graphics Properties interface
    uint32_t TotalGpuRam(uint64_t& total) const override
    {
        total = SoC_GetTotalGpuRam(); // TODO: Implement using DeviceSettings
        return (Core::ERROR_NONE);
    }
    uint32_t FreeGpuRam(uint64_t& free ) const override
    {
        free = SoC_GetFreeGpuRam(); // TODO: Implement using DeviceSettings
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

    static void ResolutionChange(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        IConnectionProperties::INotification::Source eventtype = IConnectionProperties::INotification::Source::PRE_RESOLUTION_CHANGE;
        if (strcmp(owner, IARM_BUS_DSMGR_NAME) == 0)
        {
            switch (eventId) {
                case IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE:
                    eventtype = IConnectionProperties::INotification::Source::POST_RESOLUTION_CHANGE;
                    break;
                case IARM_BUS_DSMGR_EVENT_RES_PRECHANGE:
                    eventtype = IConnectionProperties::INotification::Source::PRE_RESOLUTION_CHANGE;
            }
        }

        if(DisplayInfoImplementation::_instance)
        {
           DisplayInfoImplementation::_instance->ResolutionChangeImpl(eventtype);
        }
    }

    void ResolutionChangeImpl(IConnectionProperties::INotification::Source eventtype)
    {
        _adminLock.Lock();

        std::list<IConnectionProperties::INotification*>::const_iterator index = _observers.begin();
        while(index != _observers.end()) {
            (*index)->Updated(eventtype);
            index++;
        }

        _adminLock.Unlock();
    }

    uint32_t IsAudioPassthrough (bool& value) const override
    {
        uint32_t ret =  (Core::ERROR_NONE);
        value = false;
        try
        {
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
            device::AudioStereoMode mode = vPort.getAudioOutputPort().getStereoMode(true);
            if (mode == device::AudioStereoMode::kPassThru)
                value = true;
        }
        catch (const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
            ret = Core::ERROR_GENERAL;
        }
        return ret;
    }
    uint32_t Connected(bool& connected) const override
    {
        try
        {
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
            connected = vPort.isDisplayConnected();
        }
        catch (const device::Exception& err)
        {
           TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
           return Core::ERROR_GENERAL;
        }
        return (Core::ERROR_NONE);
    }
    uint32_t Width(uint32_t& value) const override
    {
        value = SoC_GetGraphicsWidth();
        return (Core::ERROR_NONE);
    }
    uint32_t Height(uint32_t& value) const override
    {
        value = SoC_GetGraphicsHeight();
        return (Core::ERROR_NONE);
    }
    uint32_t VerticalFreq(uint32_t& value) const override
    {
        vector<uint8_t> edidVec;
        uint32_t ret = GetEdidBytes(edidVec);
        if (ret == Core::ERROR_NONE)
        {
            uint32_t edidLen = edidVec.size();
            unsigned char* edidbytes = new unsigned char [edidLen];
            std::copy(edidVec.begin(), edidVec.end(), edidbytes);
            if (edid_parser::EDID_Verify(edidbytes, edidLen) == edid_parser::EDID_STATUS_OK)
            {
                edid_parser::edid_data_t data_ptr;
                edid_parser::EDID_Parse(edidbytes, edidLen, &data_ptr);
                value = data_ptr.res.refresh;
                TRACE(Trace::Information, (_T("Vertical frequency = %d"), value));
            }
            else
            {
                TRACE(Trace::Information, (_T("EDID Verification failed")));
                ret = Core::ERROR_GENERAL;
            }
            delete edidbytes;
        }
        else
        {
            TRACE(Trace::Information, (_T("HDMI not connected")));
            ret = Core::ERROR_GENERAL;
        }
        return ret;
   }

    uint32_t HDCPProtection(HDCPProtectionType& value) const override //get
    {
        int hdcpversion = 1;
        string portname;
        PortName(portname);
        if(!portname.empty())
        {
            try
            {
                device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(portname);
                hdcpversion = vPort.GetHdmiPreference();
                switch(static_cast<dsHdcpProtocolVersion_t>(hdcpversion))
                {
                    case dsHDCP_VERSION_1X: value = IConnectionProperties::HDCPProtectionType::HDCP_1X; break;
                    case dsHDCP_VERSION_2X: value = IConnectionProperties::HDCPProtectionType::HDCP_2X; break;
                    case dsHDCP_VERSION_MAX: value = IConnectionProperties::HDCPProtectionType::HDCP_AUTO; break;
                }
            }
            catch(const device::Exception& err)
            {
                TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
            }
        }
        else
        {
            TRACE(Trace::Information, (_T("No STB video ouptut ports connected to TV, returning HDCP as unencrypted %d"), hdcpversion));
        }
        return (Core::ERROR_NONE);
    }

    uint32_t HDCPProtection(const HDCPProtectionType value) override //set
    {
        dsHdcpProtocolVersion_t hdcpversion = dsHDCP_VERSION_MAX;
        string portname;
        PortName(portname);
        if(!portname.empty())
        {
            switch(value)
            {
                case IConnectionProperties::HDCPProtectionType::HDCP_1X : hdcpversion = dsHDCP_VERSION_1X; break;
                case IConnectionProperties::HDCPProtectionType::HDCP_2X: hdcpversion = dsHDCP_VERSION_2X; break;
                case IConnectionProperties::HDCPProtectionType::HDCP_AUTO: hdcpversion = dsHDCP_VERSION_MAX; break;
            }
            try
            {
                device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(portname);
                if(!vPort.SetHdmiPreference(hdcpversion))
                {
                    TRACE(Trace::Information, (_T("HDCPProtection: SetHdmiPreference failed")));
                }
            }
            catch(const device::Exception& err)
            {
                TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
            }
        }
        else
        {
            TRACE(Trace::Information, (_T("No STB video ouptut ports connected to TV, returning HDCP as unencrypted %d"), hdcpversion));
        }
        return (Core::ERROR_NONE);
    }

    uint32_t WidthInCentimeters(uint8_t& width /* @out */) const override
    {
        int ret = Core::ERROR_NONE;
        vector<uint8_t> edidVec;
        ret = GetEdidBytes(edidVec);
        if (Core::ERROR_NONE == ret)
        {
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            if(edidVec.size() > EDID_MAX_VERTICAL_SIZE)
            {
                width = edidVec[EDID_MAX_HORIZONTAL_SIZE];
                TRACE(Trace::Information, (_T("Width in cm = %d"), width));
            }
            else
            {
                TRACE(Trace::Information, (_T("Failed to get Display Size!")));
                ret = Core::ERROR_GENERAL;
            }
        }
        return ret;
    }

    uint32_t HeightInCentimeters(uint8_t& height /* @out */) const override
    {
        try
        {
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            ::device::VideoOutputPort vPort = ::device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
            if (vPort.isDisplayConnected())
            {
                std::vector<uint8_t> edidVec;

                vPort.getDisplay().getEDIDBytes(edidVec);

                if(edidVec.size() > EDID_MAX_VERTICAL_SIZE)
                {
                    height = edidVec[EDID_MAX_VERTICAL_SIZE];
                    TRACE(Trace::Information, (_T("Height in cm = %d"), height));
                }
                else
                {
                    TRACE(Trace::Information, (_T("Failed to get Display Size!")));
                }
            }
        }
        catch (const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        return (Core::ERROR_NONE);
    }

    uint32_t EDID (uint16_t& length /* @inout */, uint8_t data[] /* @out @length:length */) const override
    {
        vector<uint8_t> edidVec({'u','n','k','n','o','w','n' });
        int ret = Core::ERROR_NONE;
        try
        {
            vector<uint8_t> edidVec2;
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
            if (vPort.isDisplayConnected())
            {
                vPort.getDisplay().getEDIDBytes(edidVec2);
                edidVec = edidVec2;//edidVec must be "unknown" unless we successfully get to this line
            }
            else
            {
                TRACE(Trace::Information, (_T("failure: HDMI not connected!")));
                ret = Core::ERROR_GENERAL;
            }
        }
        catch (const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION0();
            ret = Core::ERROR_GENERAL;
        }
        //convert to base64
        uint16_t size = min(edidVec.size(), (size_t)numeric_limits<uint16_t>::max());
        if(edidVec.size() > (size_t)numeric_limits<uint16_t>::max())
            LOGERR("Size too large to use ToString base64 wpe api");
        int i = 0;
        for (i; i < length && i < size; i++)
        {
            data[i] = edidVec[i];
        }
        length = i;
        return ret;

    }

    uint32_t PortName (string& name /* @out */) const
    {
        try
        {
            device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
            for (size_t i = 0; i < vPorts.size(); i++)
            {
                device::VideoOutputPort &vPort = vPorts.at(i);
                if (vPort.isDisplayConnected() && ((vPort.getType() == device::VideoOutputPortType::kHDMI) \
                                                || (vPort.getType() == device::VideoOutputPortType::kInternal)))
                {
                    name = vPort.getName();
                    TRACE(Trace::Information, (_T("Connected video output port = %s"), name));
                    break;
                }
            }
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        return (Core::ERROR_NONE);
    }

    uint32_t ColorSpace(ColourSpaceType& cs /* @out */) const override
    {
        int ret = Core::ERROR_NONE;
        try
        {
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
            if (vPort.isDisplayConnected())
            {
                int _cs = vPort.getColorSpace();
                TRACE(Trace::Information, (_T("colour space = %d"), _cs ));
                switch(_cs)
                {
                    case dsDISPLAY_COLORSPACE_RGB:
                        cs = FORMAT_RGB_444; break;
                    case dsDISPLAY_COLORSPACE_YCbCr444:
                        cs = FORMAT_YCBCR_444; break;
                    case dsDISPLAY_COLORSPACE_YCbCr422:
                        cs = FORMAT_YCBCR_422; break;
                    case dsDISPLAY_COLORSPACE_YCbCr420:
                        cs = FORMAT_YCBCR_420; break;
                    case dsDISPLAY_COLORSPACE_AUTO:
                        cs = FORMAT_OTHER;break;
                    case dsDISPLAY_COLORSPACE_UNKNOWN:
                    default:
                        cs = FORMAT_UNKNOWN;
                }
            }
            else
            {
                TRACE(Trace::Error, (_T("HDMI not connected!")));
                ret = Core::ERROR_GENERAL;
            }
        }
        catch (const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("caught an exception: %d, %s"),err.getCode(), err.what()));
            ret = Core::ERROR_GENERAL;
        }
        return ret;
    }

    uint32_t FrameRate(FrameRateType& rate /* @out */) const override
    {
        rate = FRAMERATE_UNKNOWN;
        uint32_t ret =  (Core::ERROR_NONE);
        try
        {
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
            device::VideoResolution resolution = vPort.getResolution();
            device::PixelResolution pr = resolution.getPixelResolution();
            device::FrameRate fr = resolution.getFrameRate();
            if (fr == device::FrameRate::k24 ) {
                rate = FRAMERATE_24;
            } else if(fr == device::FrameRate::k25) {
                rate = FRAMERATE_25;
            } else if(fr == device::FrameRate::k30) {
                rate = FRAMERATE_30;
            } else if(fr == device::FrameRate::k60) {
                rate = FRAMERATE_60;
            } else if(fr == device::FrameRate::k23dot98) {
                rate = FRAMERATE_23_976;
            } else if(fr == device::FrameRate::k29dot97) {
                rate = FRAMERATE_29_97;
            } else if(fr == device::FrameRate::k50) {
                rate = FRAMERATE_50;
            } else if(fr == device::FrameRate::k59dot94) {
                rate = FRAMERATE_59_94;
            } else {
                rate = FRAMERATE_UNKNOWN;
            }
        }
        catch (const device::Exception& err)
        {
           TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
           ret = Core::ERROR_GENERAL;
        }
        return ret;
    }

    uint32_t ColourDepth(ColourDepthType& colour /* @out */) const override
    {
        int ret = Core::ERROR_NONE;
        try
        {
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
            if (vPort.isDisplayConnected())
            {
                int _colour = vPort.getColorDepth();
                TRACE(Trace::Information, (_T("colour depth = %d"),_colour));
                switch(_colour)
                {
                    case 8:
                        colour = COLORDEPTH_8_BIT;  break;
                    case 10:
                        colour = COLORDEPTH_10_BIT; break;
                    case 12:
                        colour = COLORDEPTH_12_BIT; break;
                    case 0:
                    default:
                        colour = COLORDEPTH_UNKNOWN;
                }
            }
            else
            {
                TRACE(Trace::Error, (_T("HDMI not connected!")));
                ret = Core::ERROR_GENERAL;
            }
        }
        catch (const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("caught an exception: %d, %s"),err.getCode(), err.what()));
            ret = Core::ERROR_GENERAL;
        }
        return ret;
    }

    uint32_t QuantizationRange(QuantizationRangeType& qr /* @out */) const override
    {
        int ret = Core::ERROR_NONE;
        try
        {
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
            if (vPort.isDisplayConnected())
            {
                int _qr = vPort.getQuantizationRange();
                TRACE(Trace::Information, (_T("quantization range = %d"),_qr));
                switch (_qr)
                {
                    case dsDISPLAY_QUANTIZATIONRANGE_LIMITED:
                        qr = QUANTIZATIONRANGE_LIMITED; break;
                    case dsDISPLAY_QUANTIZATIONRANGE_FULL:
                        qr = QUANTIZATIONRANGE_FULL; break;
                    case dsDISPLAY_QUANTIZATIONRANGE_UNKNOWN:
                    default:
                        qr = QUANTIZATIONRANGE_UNKNOWN;
                }
            }
            else
            {
                TRACE(Trace::Error, (_T("HDMI not connected!")));
                ret = Core::ERROR_GENERAL;
            }
        }
        catch (const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("caught an exception: %d, %s"),err.getCode(), err.what()));
            ret = Core::ERROR_GENERAL;
        }
        return ret;
    }

    uint32_t Colorimetry(IColorimetryIterator*& colorimetry /* @out */) const override
    {
        std::list<Exchange::IDisplayProperties::ColorimetryType> colorimetryCaps;
        vector<uint8_t> edidVec;
        uint32_t ret = GetEdidBytes(edidVec);
        if (ret == Core::ERROR_NONE)
        {
            uint32_t edidLen = edidVec.size();
            unsigned char* edidbytes = new unsigned char [edidLen];
            std::copy(edidVec.begin(), edidVec.end(), edidbytes);
            if (edid_parser::EDID_Verify(edidbytes, edidLen) == edid_parser::EDID_STATUS_OK)
            {
                edid_parser::edid_data_t data_ptr;
                edid_parser::EDID_Parse(edidbytes, edidLen, &data_ptr);
                uint32_t colorimetry_info = data_ptr.colorimetry_info;
                TRACE(Trace::Information, (_T("colorimetry = %d"),colorimetry_info));
                if (!colorimetry_info) colorimetryCaps.push_back(COLORIMETRY_UNKNOWN);
                if (colorimetry_info & edid_parser::COLORIMETRY_INFO_XVYCC601) colorimetryCaps.push_back(COLORIMETRY_XVYCC601);
                if (colorimetry_info & edid_parser::COLORIMETRY_INFO_XVYCC709) colorimetryCaps.push_back(COLORIMETRY_XVYCC709);
                if (colorimetry_info & edid_parser::COLORIMETRY_INFO_SYCC601) colorimetryCaps.push_back(COLORIMETRY_SYCC601);
                if (colorimetry_info & edid_parser::COLORIMETRY_INFO_ADOBEYCC601) colorimetryCaps.push_back(COLORIMETRY_OPYCC601);
                if (colorimetry_info & edid_parser::COLORIMETRY_INFO_ADOBERGB) colorimetryCaps.push_back(COLORIMETRY_OPRGB);
                if (colorimetry_info & edid_parser::COLORIMETRY_INFO_BT2020CL || colorimetry_info & edid_parser::COLORIMETRY_INFO_BT2020NCL) colorimetryCaps.push_back(COLORIMETRY_BT2020YCCBCBRC);
                if (colorimetry_info & edid_parser::COLORIMETRY_INFO_BT2020RGB) colorimetryCaps.push_back(COLORIMETRY_BT2020RGB_YCBCR);
                if (colorimetry_info & edid_parser::COLORIMETRY_INFO_DCI_P3) colorimetryCaps.push_back(COLORIMETRY_OTHER);
            }
            else
            {
                TRACE(Trace::Error, (_T("EDID Verification failed")));
                ret = Core::ERROR_GENERAL;
            }
            delete edidbytes;
        }
        else
        {
            TRACE(Trace::Error, (_T("HDMI not connected!")));
            ret = Core::ERROR_GENERAL;
        }
        colorimetry = Core::Service<ColorimetryIteratorImplementation>::Create<Exchange::IDisplayProperties::IColorimetryIterator>(colorimetryCaps);
        return (colorimetry != nullptr && ret == Core::ERROR_NONE ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }

    uint32_t EOTF(EotfType& eotf /* @out */) const override
    {
        int ret = Core::ERROR_NONE;
        try
        {
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
            if (vPort.isDisplayConnected())
            {
                int _eotf = vPort.getVideoEOTF();
                TRACE(Trace::Information, (_T("videoEOTF = %d"),_eotf));
                switch (_eotf)
                {
                    /* bt1886 = sdr; smpte2084 = hdr10; bt2100 = HLG*/
                    case dsHDRSTANDARD_HDR10:
                        eotf = EOTF_SMPTE_ST_2084; break;
                    case dsHDRSTANDARD_HLG:
                        eotf = EOTF_BT2100; break;
                    default:
                        eotf = EOTF_UNKNOWN;
                }
            }
            else
            {
                TRACE(Trace::Error, (_T("HDMI not connected!")));
                ret = Core::ERROR_GENERAL;
            }
        }
        catch (const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("caught an exception: %d, %s"),err.getCode(), err.what()));
            ret = Core::ERROR_GENERAL;
        }
        return ret;
    }

    // @property
    // @brief HDR formats supported by TV
    // @return HDRType: array of HDR formats
    uint32_t TVCapabilities(IHDRIterator*& type /* out */) const override
    {
        std::list<Exchange::IHDRProperties::HDRType> hdrCapabilities;

        int capabilities = static_cast<int>(dsHDRSTANDARD_NONE);
        try
        {
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
            if (vPort.isDisplayConnected()) {
                vPort.getTVHDRCapabilities(&capabilities);
            }
            else {
                TRACE(Trace::Error, (_T("getTVHDRCapabilities failure: HDMI not connected!")));
            }
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        if(!capabilities) hdrCapabilities.push_back(HDR_OFF);
        if(capabilities & dsHDRSTANDARD_HDR10) hdrCapabilities.push_back(HDR_10);
        if(capabilities & dsHDRSTANDARD_HLG) hdrCapabilities.push_back(HDR_HLG);
        if(capabilities & dsHDRSTANDARD_DolbyVision) hdrCapabilities.push_back(HDR_DOLBYVISION);
        if(capabilities & dsHDRSTANDARD_TechnicolorPrime) hdrCapabilities.push_back(HDR_TECHNICOLOR);
        if(capabilities & dsHDRSTANDARD_Invalid)hdrCapabilities.push_back(HDR_OFF);


        type = Core::Service<HdrteratorImplementation>::Create<Exchange::IHDRProperties::IHDRIterator>(hdrCapabilities);
        return (type != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
    // @property
    // @brief HDR formats supported by STB
    // @return HDRType: array of HDR formats
    uint32_t STBCapabilities(IHDRIterator*& type /* out */) const override
    {
        std::list<Exchange::IHDRProperties::HDRType> hdrCapabilities;

        int capabilities = static_cast<int>(dsHDRSTANDARD_NONE);
        try
        {
            device::VideoDevice &device = device::Host::getInstance().getVideoDevices().at(0);
            device.getHDRCapabilities(&capabilities);
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        if(!capabilities) hdrCapabilities.push_back(HDR_OFF);
        if(capabilities & dsHDRSTANDARD_HDR10) hdrCapabilities.push_back(HDR_10);
        if(capabilities & dsHDRSTANDARD_HLG) hdrCapabilities.push_back(HDR_HLG);
        if(capabilities & dsHDRSTANDARD_DolbyVision) hdrCapabilities.push_back(HDR_DOLBYVISION);
        if(capabilities & dsHDRSTANDARD_TechnicolorPrime) hdrCapabilities.push_back(HDR_TECHNICOLOR);
        if(capabilities & dsHDRSTANDARD_Invalid)hdrCapabilities.push_back(HDR_OFF);


        type = Core::Service<HdrteratorImplementation>::Create<Exchange::IHDRProperties::IHDRIterator>(hdrCapabilities);
        return (type != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
    // @property
    // @brief HDR format in use
    // @param type: HDR format
    uint32_t HDRSetting(HDRType& type /* @out */) const override
    {
        type = IHDRProperties::HDRType::HDR_OFF;
        bool isHdr = false;
        try
        {
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
            if (vPort.isDisplayConnected()) {
                isHdr = vPort.IsOutputHDR();
            }
            else
            {
                TRACE(Trace::Information, (_T("IsOutputHDR failure: HDMI not connected!")));
            }
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        TRACE(Trace::Information, (_T("Output HDR = %s"), isHdr ? "Yes" : "No"));

        type = isHdr? HDR_10 : HDR_OFF;
        return (Core::ERROR_NONE);
    }


    BEGIN_INTERFACE_MAP(DisplayInfoImplementation)
        INTERFACE_ENTRY(Exchange::IGraphicsProperties)
        INTERFACE_ENTRY(Exchange::IConnectionProperties)
        INTERFACE_ENTRY(Exchange::IHDRProperties)
        INTERFACE_ENTRY(Exchange::IDisplayProperties)
    END_INTERFACE_MAP

private:
    std::list<IConnectionProperties::INotification*> _observers;
    mutable Core::CriticalSection _adminLock;

private:
    uint32_t GetEdidBytes(vector<uint8_t> &edid) const
    {
        uint32_t ret = Core::ERROR_NONE;
        try
        {
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
            if (vPort.isDisplayConnected())
            {
                vPort.getDisplay().getEDIDBytes(edid);
            }
            else
            {
                TRACE(Trace::Error, (_T("HDMI not connected!")));
                ret = Core::ERROR_GENERAL;
            }

        }
        catch (const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("caught an exception: %d, %s"),err.getCode(), err.what()));
            ret = Core::ERROR_GENERAL;
        }

        return ret;
    }


public:
    static DisplayInfoImplementation* _instance;
};
    DisplayInfoImplementation* DisplayInfoImplementation::_instance = nullptr;
    SERVICE_REGISTRATION(DisplayInfoImplementation, 1, 0);
}
}
