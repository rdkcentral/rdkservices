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

//#include "hdmi_wrapper.h"

#define EDID_MAX_HORIZONTAL_SIZE 21
#define EDID_MAX_VERTICAL_SIZE   22
#define TOTAL_MEM_PARAM_STR  "CmaTotal:"
#define FREE_MEM_PARAM_STR  "CmaFree:"

namespace WPEFramework {
namespace Plugin {

class DisplayInfoImplementation :
    public Exchange::IGraphicsProperties,
    public Exchange::IConnectionProperties {
public:
    DisplayInfoImplementation()
        : _totalGpuRam(0)
        , _width(0)
        , _height(0)
        , _frameRate(0)
    {
        try
        {
            Utils::IARM::init();
            
			//TODO: this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
            device::Manager::Initialize();
            TRACE(Trace::Information, (_T("device::Manager::Initialize success")));

            UpdateResolution(_width, _height, _frameRate);
			//hdmi_wrap_init();
			//TRACE(Trace::Information, (_T("hdmi_wrap_init success")));
            UpdateTotalMem(_totalGpuRam);
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
        LOGINFO();
		
		//hdmi_wrap_deinit();
    }

public:
    // Graphics Properties interface
    uint64_t TotalGpuRam() const override
    {
        return _totalGpuRam;
    }
    uint64_t FreeGpuRam() const override
    {
        return GetMemInfo(FREE_MEM_PARAM_STR);
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
        return false;
    }
    bool Connected() const override
    {
		bool connected = false;
		try
        {
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
            connected = vPort.isDisplayConnected();
        }
        catch (const device::Exception& err)
        {
           TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        return connected;
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
        return _frameRate;
    }

    HDCPProtectionType HDCPProtection() const override {
        HDCPProtectionType value;
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
                    //case dsHDCP_VERSION_MAX: value = IConnectionProperties::HDCPProtectionType::HDCP_AUTO; break;
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
        return value;
    }

	HDRType Type() const override
    {
        return HDR_OFF;
    }

    uint32_t PortName (string& name /* @out */) const
    {
        LOGINFO();
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

 
    BEGIN_INTERFACE_MAP(DisplayInfoImplementation)
        INTERFACE_ENTRY(Exchange::IGraphicsProperties)
        INTERFACE_ENTRY(Exchange::IConnectionProperties)
    END_INTERFACE_MAP

private:
	std::list<IConnectionProperties::INotification*> _observers;
    mutable Core::CriticalSection _adminLock;
	uint64_t _totalGpuRam;
    uint32_t _width;
    uint32_t _height;
    uint32_t _frameRate;
    
    static uint64_t parseLine(const char * line)
    {

        string str(line);
        uint64_t val = 0;
        size_t begin = str.find_first_of("0123456789");
        size_t end = std::string::npos;

        if (std::string::npos != begin)
            end = str.find_first_not_of("0123456789", begin);

        if (std::string::npos != begin && std::string::npos != end)
        {

            str = str.substr(begin, end);
            val = strtoul(str.c_str(), NULL, 10);

        }
        else
        {
            printf("%s:%d Failed to parse value from %s", __FUNCTION__, __LINE__,line);

        }

        return val;
    }
    
	static uint64_t GetMemInfo(const char * param)
    {
        uint64_t memVal = 0;
        FILE *meminfoFile = fopen("/proc/meminfo", "r");
        if (NULL == meminfoFile)
        {
            printf("%s:%d : Failed to open /proc/meminfo:%s", __FUNCTION__, __LINE__, strerror(errno));
        }
        else
        {
            std::vector <char> buf;
            buf.resize(1024);

            while (fgets(buf.data(), buf.size(), meminfoFile))
            {
                 if ( strstr(buf.data(), param ) == buf.data())
                 {
                     memVal = parseLine(buf.data()) * 1000;
                     break;
                 }
            }

            fclose(meminfoFile);
        }
        return memVal;
   }
   void UpdateTotalMem(uint64_t& totalRam)
   {
        totalRam = GetMemInfo(TOTAL_MEM_PARAM_STR);
   }
   
   uint32_t UpdateResolution(uint32_t &w, uint32_t &h, uint32_t &rate)
   {
        uint32_t ret =  (Core::ERROR_NONE);
        try
        {
            device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
            device::VideoResolution resolution = vPort.getResolution();
            device::PixelResolution pr = resolution.getPixelResolution();
            device::FrameRate fr = resolution.getFrameRate();
            
            if(pr == device::PixelResolution::k720x480) {
                w = 720;
                h = 480;
            } else if(pr == device::PixelResolution::k720x576) {
                w = 720;
                h = 576;
            } else if(pr == device::PixelResolution::k1280x720) {
                w = 1280;
                h = 720;
            } else if(pr == device::PixelResolution::k1920x1080) {
                w = 1920;
                h = 1080;
            } else if(pr == device::PixelResolution::k3840x2160) {
                w = 3840;
                h = 2160;
            } else if(pr == device::PixelResolution::k4096x2160) {
                w = 4096;
                h = 2160;
            } else {
               ret = Core::ERROR_GENERAL;
            }
            
            if (fr == device::FrameRate::k24 ) {
                rate = 24;
            } else if(fr == device::FrameRate::k25) {
                rate = 25;
            } else if(fr == device::FrameRate::k30) {
                rate = 30;
            } else if(fr == device::FrameRate::k60) {
                rate = 60;
            } else if(fr == device::FrameRate::k23dot98) {
                rate = 23;
            } else if(fr == device::FrameRate::k29dot97) {
                rate = 29;
            } else if(fr == device::FrameRate::k50) {
                rate = 50;
            } else if(fr == device::FrameRate::k59dot94) {
                rate = 59;
            } else {
                ret = Core::ERROR_GENERAL;
            }
           
        }
        catch (const device::Exception& err)
        {
           TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
           ret = Core::ERROR_GENERAL;
        }
        
        return ret;
   }
   
    
};
    SERVICE_REGISTRATION(DisplayInfoImplementation, 1, 0);
}
}
