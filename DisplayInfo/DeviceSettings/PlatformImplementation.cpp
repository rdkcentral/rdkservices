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
#include "utils.h"

#include "libIBus.h"
#include "libIBusDaemon.h"
#include "dsMgr.h"

namespace WPEFramework {
namespace Plugin {

class DisplayInfoImplementation :
    public Exchange::IGraphicsProperties,
    public Exchange::IConnectionProperties,
    public Exchange::IHDRProperties  {
private:
    using HdrteratorImplementation = RPC::IteratorType<Exchange::IHDRProperties::IHDRIterator>;
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
    uint64_t TotalGpuRam() const override
    {
        return 0; // TODO: Implement using DeviceSettings
    }
    uint64_t FreeGpuRam() const override
    {
        return 0; // TODO: Implement using DeviceSettings
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
        IConnectionProperties::INotification::Source eventtype;
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
            (*index)->Updated(IConnectionProperties::INotification::Source::POST_RESOLUTION_CHANGE);
            index++;
        }

        _adminLock.Unlock();
    }

    uint32_t IsAudioPassthrough (bool& value) const override
    {
        value = false; // TODO: Implement using DeviceSettings
        return (Core::ERROR_NONE);
    }
    uint32_t Connected(bool& connected) const override
    {
        connected = false; // TODO: Implement using DeviceSettings (or use HDCP Profile plugin for this)
        return (Core::ERROR_NONE);
    }
    uint32_t Width(uint32_t& value) const override
    {
        value = 0; // TODO: Implement using DeviceSettings
        return (Core::ERROR_NONE);
    }
    uint32_t Height(uint32_t& value) const override
    {
        value = 0; // TODO: Implement using DeviceSettings
        return (Core::ERROR_NONE);
    }
    uint32_t VerticalFreq(uint32_t& value) const override
    {
        value = 0; // TODO: Implement using DeviceSettings
        return (Core::ERROR_NONE);
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
                    case dsHDCP_VERSION_2X:
                    case dsHDCP_VERSION_MAX: value = IConnectionProperties::HDCPProtectionType::HDCP_2X; break;
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

    uint32_t HDCPProtection(const HDCPProtectionType& value) override //set
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
                default: hdcpversion = dsHDCP_VERSION_MAX;
            }
            try
            {
                device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(portname);
                if(!vPort.SetHdmiPreference(hdcpversion))
                {
                    TRACE(Trace::Information, (_T("HDCPProtection: SetHdmiPreference failed")));
                    LOGERR("SetHdmiPreference failed");
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

    uint32_t EDID (uint16_t& length /* @inout */, uint8_t data[] /* @out @length:length */) const override
    {
        vector<uint8_t> edidVec({'u','n','k','n','o','w','n' });
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
                LOGWARN("failure: HDMI0 not connected!");
            }
        }
        catch (const device::Exception& err)
        {
            LOG_DEVICE_EXCEPTION0();
        }
        //convert to base64
        uint16_t size = min(edidVec.size(), (size_t)numeric_limits<uint16_t>::max());
        if(edidVec.size() > (size_t)numeric_limits<uint16_t>::max())
            LOGERR("Size too large to use ToString base64 wpe api");
        string edidbase64;
        // Align input string size to multiple of 3
        int paddingSize = 0;
        for (; paddingSize < (3-size%3);paddingSize++)
        {
            edidVec.push_back(0x00);
        }
        size += paddingSize;
        int i = 0;

        for (i; i < length && i < size; i++)
        {
            data[i] = edidVec[i];
        }
        length = i;
        return (Core::ERROR_NONE);

    }

    uint32_t PortName (string& name /* @out */) const
    {
        try
        {
            device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
            for (size_t i = 0; i < vPorts.size(); i++)
            {
                device::VideoOutputPort &vPort = vPorts.at(i);
                if (vPort.isDisplayConnected())
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

    // @property
    // @brief HDR formats supported by TV
    // @return HDRType: array of HDR formats
    uint32_t TVCapabilities(IHDRIterator*& type /* out */) const override
    {
        std::list<Exchange::IHDRProperties::HDRType> hdrCapabilities;

        int capabilities = static_cast<int>(dsHDRSTANDARD_NONE);
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
    uint32_t HDRSetting(HDRType& type /* @out */) const override {
        type = IHDRProperties::HDRType::HDR_OFF;
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

        type = isHdr? HDR_10 : HDR_OFF;
        return (Core::ERROR_NONE);
    }


    BEGIN_INTERFACE_MAP(DisplayInfoImplementation)
        INTERFACE_ENTRY(Exchange::IGraphicsProperties)
        INTERFACE_ENTRY(Exchange::IConnectionProperties)
        INTERFACE_ENTRY(Exchange::IHDRProperties)
    END_INTERFACE_MAP

private:
    std::list<IConnectionProperties::INotification*> _observers;
    mutable Core::CriticalSection _adminLock;

public:
    static DisplayInfoImplementation* _instance;
};
    DisplayInfoImplementation* DisplayInfoImplementation::_instance = nullptr;
    SERVICE_REGISTRATION(DisplayInfoImplementation, 1, 0);
}
}
