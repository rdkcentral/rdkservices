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
#include <interfaces/IDRM.h>
#include "rtkDrmUtils.h"
#define RTK_HDRSTANDARD_DolbyVision 4
#define RTK_HDCP_VERSION_1X 0
#define RTK_HDCP_VERSION_2X 1

#define RTK_TOTAL_MEM_PARAM_STR  "CmaTotal:"
#define RTK_FREE_MEM_PARAM_STR  "CmaFree:"

#ifdef REALTEK_E2
static pthread_mutex_t drmFD_lock = PTHREAD_MUTEX_INITIALIZER;
drmModeConnector *hdmiConn;
drmModeRes *res;

int openDefaultDRMDevice() {
    int drmFD = -1;
    pthread_mutex_lock(&drmFD_lock);
    if (drmFD < 0) {
        drmFD = open(DEFUALT_DRM_DEVICE, O_RDWR | O_CLOEXEC);
        // Re-check if open successfully or not
        if (drmFD < 0) {
            printf("%s:%d cannot open %s\n", __FUNCTION__, __LINE__, DEFUALT_DRM_DEVICE);
        }
    }
    pthread_mutex_unlock(&drmFD_lock);
    return drmFD;
}

int getSupportedDRMResolutions(drmModeConnector *conn, drmConnectorModes *drmResolution) {
    for (int i =0; i < conn->count_modes; i++) {
        if(!strcmp(conn->modes[i].name,"720x480i")) {
            drmResolution[i] = drmMode_480i;
        }
        else if(!strcmp(conn->modes[i].name,"720x480")) {
            drmResolution[i] = drmMode_480p;
        }
        else if(!strcmp(conn->modes[i].name,"1280x720")) {
            drmResolution[i] = drmMode_720p;
        }
        else if(!strcmp(conn->modes[i].name,"1920x1080i")) {
            drmResolution[i] = drmMode_1080i;
        }
        else if(!strcmp(conn->modes[i].name,"1920x1080")) {
            if(conn->modes[i].vrefresh == 60) {
                drmResolution[i] = drmMode_1080p;
            }
            else if(conn->modes[i].vrefresh == 24) {
                drmResolution[i] = drmMode_1080p24;
            }
            else if(conn->modes[i].vrefresh == 25) {
                drmResolution[i] = drmMode_1080p25;
            }
              else if(conn->modes[i].vrefresh == 30) {
                drmResolution[i] = drmMode_1080p30;
            }
            else if(conn->modes[i].vrefresh == 50) {
                drmResolution[i] = drmMode_1080p50;
            }
            else {
                drmResolution[i] = drmMode_Unknown;
            }
        }
        else if(!strcmp(conn->modes[i].name,"3840x2160")) {
            if(conn->modes[i].vrefresh == 24) {
                drmResolution[i] = drmMode_3840x2160p24;
            }
            else if(conn->modes[i].vrefresh == 25) {
                drmResolution[i] = drmMode_3840x2160p25;
            }
            else if(conn->modes[i].vrefresh == 30) {
                drmResolution[i] = drmMode_3840x2160p30;
            }
            else if(conn->modes[i].vrefresh == 50) {
                drmResolution[i] = drmMode_3840x2160p50;
            }
            else if(conn->modes[i].vrefresh == 60) {
                drmResolution[i] = drmMode_3840x2160p60;
            }
            else {
                drmResolution[i] = drmMode_Unknown;
            }
        }
        else if(!strcmp(conn->modes[i].name,"4096x2160")) {
		           if(conn->modes[i].vrefresh == 24) {
                drmResolution[i] = drmMode_4096x2160p24;
            }
            else if(conn->modes[i].vrefresh == 25) {
                drmResolution[i] = drmMode_4096x2160p25;
            }
            else if(conn->modes[i].vrefresh == 30) {
                drmResolution[i] = drmMode_4096x2160p30;
            }
            else if(conn->modes[i].vrefresh == 50) {
                drmResolution[i] = drmMode_4096x2160p50;
            }
            else if(conn->modes[i].vrefresh == 60) {
                drmResolution[i] = drmMode_4096x2160p60;
            }
            else {
                drmResolution[i] = drmMode_Unknown;
            }
        }
    }
   return 0;
}
int rtksysfs_get_sysfs_str(const char *path, char *valstr, int size)
{
    int fd;
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        memset(valstr, 0, size);
        read(fd, valstr, size - 1);
        valstr[strlen(valstr)] = '\0';
        close(fd);
    } else {
        printf("%s:%d unable to open file %s,err: %s\n", __FUNCTION__, __LINE__,  path, strerror(errno));
        sprintf(valstr, "%s", "fail");
        return -1;
    };
    return 0;
}

#endif

namespace WPEFramework {
namespace Plugin {

class DisplayInfoImplementation : public Exchange::IGraphicsProperties, public Exchange::IConnectionProperties {
public:
    DisplayInfoImplementation()
       : _width(0)
       , _height(0)
       , _connected(false)
       , _verticalFreq(0)
       , _hdcpprotection(HDCPProtectionType::HDCP_Unencrypted)
       , _type(HDR_OFF)
       , _totalGpuRam(0)
       , _audioPassthrough(false)
       , _adminLock()
       , _activity(*this) {

        UpdateTotalMem(_totalGpuRam);
        UpdateDisplayInfo(_connected, _width, _height, _type, _verticalFreq);
        UpdateAudioPassthrough(_audioPassthrough);
        UpdateDisplayInfoHDCP(_hdcpprotection);

        RegisterCallback();
    }

    DisplayInfoImplementation(const DisplayInfoImplementation&) = delete;
    DisplayInfoImplementation& operator= (const DisplayInfoImplementation&) = delete;
    virtual ~DisplayInfoImplementation()
    {

    }

public:
    // Graphics Properties interface
    uint64_t TotalGpuRam() const override
    {
        return _totalGpuRam;
    }
    uint64_t FreeGpuRam() const override
    {
        return GetMemInfo(RTK_FREE_MEM_PARAM_STR);
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
    HDCPProtectionType HDCPProtection() const override
    {
	    return _hdcpprotection;
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

    static uint64_t  GetMemInfo(const char * param)
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
         totalRam = GetMemInfo(RTK_TOTAL_MEM_PARAM_STR);
    }


    inline void UpdateAudioPassthrough(bool& audioPassthrough)
    {
       audioPassthrough = false;
    }

    void UpdateDisplayInfo(bool& connected, uint32_t& width, uint32_t& height, HDRType& type, uint32_t& verticalFreq)
    {
#ifdef REALTEK_E2
	    char strStatus[13] = {'\0'};

	    rtksysfs_get_sysfs_str("/sys/class/drm/card0-HDMI-A-1/status",strStatus, sizeof(strStatus));
	    if(strncmp(strStatus,"connected",9) == 0) {
		    connected = true;
	    }
	    else {
		    connected = false;
	    }
#else
	    connected = true; //Display always connected for Panel
            verticalFreq = 60;
#endif
#ifdef REALTEK_E2
	    rtkError_t ret = rtkERR_NONE;
	    bool drmInitialized = false;
	    int drmFD = -1;
	    if(!drmInitialized) {
		    bool acquiredConnector = false;
		    drmFD = openDefaultDRMDevice();
		    if (drmFD < 0) {
			    ret = rtkERR_GENERAL;
		    }
		    /* retrieve resources */
		    res = drmModeGetResources(drmFD);
		    if (!res) {
			    fprintf(stderr, "cannot retrieve DRM resources (%d): %m\n",
					    errno);
			    ret = rtkERR_GENERAL;
		    }

		    while(!acquiredConnector) {
			    for (int i = 0; i < res->count_connectors; ++i) {
				    /* get information for each connector */
				    hdmiConn = drmModeGetConnector(drmFD, res->connectors[i]);
				    if (!hdmiConn) {
					    fprintf(stderr, "cannot retrieve DRM connector %u:%u (%d): %m\n",
							    i, res->connectors[i], errno);
					    continue;
				    }
				    if (hdmiConn->connector_type == DRM_MODE_CONNECTOR_HDMIA) {  //Save connector pointer for HDMI Tx
					    acquiredConnector = true;
					    break;
				    }

				    continue;
			    }
		    }
               }
		    drmConnectorModes supportedModes[drmMode_Max] = {drmMode_Unknown};
		    getSupportedDRMResolutions(hdmiConn, supportedModes);
		    for(int i = 0; i<drmMode_Max; i++ ) {
			    switch(supportedModes[i]) {
				    case drmMode_3840x2160p24:
				    case drmMode_3840x2160p25:
				    case drmMode_3840x2160p30:
				    case drmMode_3840x2160p50:
				    case drmMode_4096x2160p24:
				    case drmMode_4096x2160p25:
				    case drmMode_4096x2160p30:
				    case drmMode_4096x2160p50:
					    height = 4096;
					    width  = 2160;
					    break;
				    case drmMode_3840x2160p60:
				    case drmMode_4096x2160p60:
					    height = 4096;
					    width  = 2160;
					    break;
				    default:
					    break;
			    }
		    }
#else
		    height = 4096;
		    width  = 2160;
#endif
		    // Read HDR status
		    type = HDR_DOLBYVISION;

		    // Read display width and height
	    }


     void UpdateDisplayInfoHDCP(HDCPProtectionType hdcpprotection) const
     {
         hdcpprotection = HDCPProtectionType::HDCP_2X;
     }

    void RegisterCallback()
    {
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
        UpdateDisplayInfo(_connected, _width, _height, _type, _verticalFreq);
        _adminLock.Unlock();

        _activity.Submit();
    }

private:
    uint32_t _width;
    uint32_t _height;
    bool _connected;
    uint32_t _verticalFreq;

    HDCPProtectionType  _hdcpprotection;
    HDRType _type;
    uint64_t _totalGpuRam;
    bool _audioPassthrough;

    std::list<IConnectionProperties::INotification*> _observers;

    mutable Core::CriticalSection _adminLock;

    Core::WorkerPool::JobType<DisplayInfoImplementation&> _activity;
};

    SERVICE_REGISTRATION(DisplayInfoImplementation, 1, 0);
}
}
