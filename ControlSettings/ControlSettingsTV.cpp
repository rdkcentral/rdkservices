/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2022 Sky UK
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

#include <string>
#include "ControlSettingsTV.h"

#define BUFFER_SIZE     (128)
#define TVSETTINGS_SOURCE_PICTUREMODE_STRING_RFC_PARAM "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.ControlSettings.Source"

#define registerMethod(...) Register(__VA_ARGS__);GetHandler(2)->Register<JsonObject, JsonObject>(__VA_ARGS__)

static char videoDescBuffer[VIDEO_DESCRIPTION_MAX*VIDEO_DESCRIPTION_NAME_SIZE] = {0};

static const char *component_color[] = {
    [COLOR_ENABLE] = "enable",
    [COLOR_RED] = "red",
    [COLOR_GREEN] = "green",
    [COLOR_BLUE] = "blue",
    [COLOR_CYAN] = "cyan",
    [COLOR_MAGENTA] = "magenta",
    [COLOR_YELLOW] = "yellow"
};

namespace WPEFramework {
namespace Plugin {

    ControlSettingsTV* ControlSettingsTV::instance = nullptr;

    static JsonArray getSupportedVideoFormat(void)
    {
        JsonArray supportedHdrFormat;
        tvError_t ret = tvERROR_NONE;
        char buffer[EXPECTED_FILE_SIZE] = {0};
        unsigned short totalVideoFormats = 0;

        ret = ReadAllModeConfigfile(GENERIC_MODE_CONFIG_FILE,  buffer, "videoformat");
        if(ret == tvERROR_NONE){
           splitstringsfrombuffer(buffer, videoDescBuffer, &totalVideoFormats);

            for(int index=0; index<totalVideoFormats; index++){
                supportedHdrFormat.Add(videoDescBuffer+(index*VIDEO_DESCRIPTION_NAME_SIZE));
            }
        }else{
            printf("%s : Reading supported video format failed %d", __FUNCTION__, ret);
        }

        return supportedHdrFormat;
    }

    static std::string getVideoResolutionTypeToString(tvResolutionParam_t resolution)
    {
        std::string strValue = "NONE";
        std::string interlaceValue = (resolution.isInterlaced) ? "i" : "p";
        if ( resolution.resolutionValue != tvVideoResolution_NONE ) {
            strValue = std::to_string(resolution.frameWidth) + "*" + std::to_string(resolution.frameHeight) + interlaceValue;
        }
        printf("Video Resolution:[%s]\n", strValue.c_str());
        return strValue;
    }

    static JsonArray getSupportedVideoResolution(void)
    {
        JsonArray supportedResolution;
        tvError_t ret = tvERROR_NONE;
        char buffer[EXPECTED_FILE_SIZE] = {0};
        unsigned short totalVideoResolutions = 0;

        ret = ReadAllModeConfigfile(GENERIC_MODE_CONFIG_FILE,  buffer, "videoresolution");
        if(ret == tvERROR_NONE){
            splitstringsfrombuffer(buffer, videoDescBuffer, &totalVideoResolutions);

            for(int index=0; index < totalVideoResolutions; index++){
                supportedResolution.Add(videoDescBuffer+(index*VIDEO_DESCRIPTION_NAME_SIZE));
            }
        }else{
            printf("%s : Reading supported video resolutions failed %d", __FUNCTION__, ret);
        }

        return supportedResolution;
    }

    static const char *getVideoFormatTypeToString(tvVideoHDRFormat_t format)
    {
        const char *strValue = "NONE";
        switch(format) {
            case tvVideoHDRFormat_SDR:
                printf("Video Format: SDR\n");
                strValue = "SDR";
                break;
            case tvVideoHDRFormat_HDR10:
                printf("Video Format: HDR10\n");
                strValue = "HDR10";
                break;
            case tvVideoHDRFormat_HDR10PLUS:
                printf("Video Format: HDR10PLUS\n");
                strValue = "HDR10PLUS";
                break;
            case tvVideoHDRFormat_HLG:
                printf("Video Format: HLG\n");
                strValue = "HLG";
                break;
            case tvVideoHDRFormat_DV:
                printf("Video Format: DV\n");
                strValue = "DV";
                break;
            default:
                printf("Video Format:: NONE\n");
                strValue = "NONE";
                break;
        }
        return strValue;
    }

    static const char *getVideoFrameRateTypeToString(tvVideoFrameRate_t frameRate)
   {
       const char *strValue = "NONE";
       switch(frameRate) {
            case tvVideoFrameRate_24:
                printf("Video FrameRate: 24\n");
                strValue = "24";
                break;
            case tvVideoFrameRate_25:
                printf("Video FrameRate: 25\n");
                strValue = "25";
                break;
            case tvVideoFrameRate_30:
                printf("Video FrameRate: 30\n");
                strValue = "30";
                break;
            case tvVideoFrameRate_50:
                printf("Video FrameRate: 50\n");
                strValue = "50";
                break;
            case tvVideoFrameRate_60:
                printf("Video FrameRate: 60\n");
                strValue = "60";
                break;
            case tvVideoFrameRate_23dot98:
                printf("Video FrameRate: 23.98\n");
                strValue = "23.98";
                break;
            case tvVideoFrameRate_29dot97:
                printf("Video FrameRate: 29.97\n");
                strValue = "29.97";
                break;
            case tvVideoFrameRate_59dot94:
                printf("Video FrameRate: 59.94\n");
                strValue = "59.94";
                break;
            default:
                printf("Video FrameRate: NONE\n");
                strValue = "NONE";
                break;

        }
        return strValue;
    }

    static JsonArray getSupportedVideoFrameRate(void)
    {
        JsonArray supportedFrameRate;
        tvError_t ret = tvERROR_NONE;
        char buffer[EXPECTED_FILE_SIZE] = {0};
        unsigned short totalFrameRates = 0;

        ret = ReadAllModeConfigfile(GENERIC_MODE_CONFIG_FILE,  buffer, "videoframerate");
        if(ret == tvERROR_NONE){
            splitstringsfrombuffer(buffer, videoDescBuffer, &totalFrameRates);

            for(int index=0; index<totalFrameRates; index++){
                supportedFrameRate.Add(videoDescBuffer+(index*VIDEO_DESCRIPTION_NAME_SIZE));
            }
        }else{
            printf("%s : Reading supported video frame rate failed %d", __FUNCTION__, ret);
        }

        return supportedFrameRate;
    }

    ControlSettingsTV::ControlSettingsTV(): PluginHost::JSONRPC()
					    , m_currentHdmiInResoluton (dsVIDEO_PIXELRES_1920x1080)
                                            , m_videoZoomMode (tvDisplayMode_NORMAL)
                                            , m_isDisabledHdmiIn4KZoom (false)
					    , rfc_caller_id()
    {
        LOGINFO("Entry\n");
        ControlSettingsTV::instance = this;
	CreateHandler({ 2 });
        Register("getBacklight", &ControlSettingsTV::getBacklight, this);
        registerMethod("setBacklight", &ControlSettingsTV::setBacklight, this);
        registerMethod("resetBacklight", &ControlSettingsTV::resetBacklight, this);
        registerMethod("getBacklightCaps", &ControlSettingsTV::getBacklightCaps, this);
        registerMethod("getBrightnessCaps", &ControlSettingsTV::getBrightnessCaps, this);
        Register("getBrightness", &ControlSettingsTV::getBrightness, this);
        registerMethod("setBrightness", &ControlSettingsTV::setBrightness, this);
        registerMethod("resetBrightness", &ControlSettingsTV::resetBrightness, this);
        Register("getContrast", &ControlSettingsTV::getContrast, this);
        registerMethod("setContrast", &ControlSettingsTV::setContrast, this);
        registerMethod("resetContrast", &ControlSettingsTV::resetContrast, this);
	registerMethod("getContrastCaps", &ControlSettingsTV::getContrastCaps, this);
        Register("getSharpness", &ControlSettingsTV::getSharpness, this);
        registerMethod("setSharpness", &ControlSettingsTV::setSharpness, this);
        registerMethod("resetSharpness", &ControlSettingsTV::resetSharpness, this);
	registerMethod("getSharpnessCaps", &ControlSettingsTV::getSharpnessCaps, this);
        Register("getSaturation", &ControlSettingsTV::getSaturation, this);
        registerMethod("setSaturation", &ControlSettingsTV::setSaturation, this);
        registerMethod("resetSaturation", &ControlSettingsTV::resetSaturation, this);
	registerMethod("getSaturationCaps", &ControlSettingsTV::getSaturationCaps, this);
        Register("getHue", &ControlSettingsTV::getHue, this);
        registerMethod("setHue", &ControlSettingsTV::setHue, this);
        registerMethod("resetHue", &ControlSettingsTV::resetHue, this);
	registerMethod("gethueCaps", &ControlSettingsTV::getHueCaps, this);
        Register("getColorTemperature", &ControlSettingsTV::getColorTemperature, this);
        registerMethod("setColorTemperature", &ControlSettingsTV::setColorTemperature, this);
        registerMethod("resetColorTemperature", &ControlSettingsTV::resetColorTemperature, this);
	registerMethod("getColorTemperatureCaps", &ControlSettingsTV::getColorTemperatureCaps, this);

        registerMethod("getComponentCaps", &ControlSettingsTV::getComponentCaps, this);
        registerMethod("getComponentSaturation", &ControlSettingsTV::getComponentSaturation, this);
        registerMethod("setComponentSaturation", &ControlSettingsTV::setComponentSaturation, this);
        registerMethod("resetComponentSaturation", &ControlSettingsTV::resetComponentSaturation, this);
        registerMethod("getComponentHue", &ControlSettingsTV::getComponentHue, this);
        registerMethod("setComponentHue", &ControlSettingsTV::setComponentHue, this);
        registerMethod("resetComponentHue", &ControlSettingsTV::resetComponentHue, this);
        registerMethod("getComponentLuma", &ControlSettingsTV::getComponentLuma, this);
        registerMethod("setComponentLuma", &ControlSettingsTV::setComponentLuma, this);
        registerMethod("resetComponentLuma", &ControlSettingsTV::resetComponentLuma, this);
        registerMethod("getBacklightDimmingMode", &ControlSettingsTV::getBacklightDimmingMode, this);
        registerMethod("setBacklightDimmingMode", &ControlSettingsTV::setBacklightDimmingMode, this);
        registerMethod("resetBacklightDimmingMode", &ControlSettingsTV::resetBacklightDimmingMode, this);
        registerMethod("getDimmingModeCaps", &ControlSettingsTV::getDimmingModeCaps, this);
        registerMethod("getAutoBacklightControl", &ControlSettingsTV::getAutoBacklightControl, this);
        registerMethod("setAutoBacklightControl", &ControlSettingsTV::setAutoBacklightControl, this);
        registerMethod("resetAutoBacklightControl", &ControlSettingsTV::resetAutoBacklightControl, this);
        registerMethod("getAutoBacklightControlCaps", &ControlSettingsTV::getAutoBacklightControlCaps, this);

        registerMethod("getSupportedDolbyVisionModes", &ControlSettingsTV::getSupportedDolbyVisionModes, this);
        registerMethod("getDolbyVisionMode", &ControlSettingsTV::getDolbyVisionMode, this);
        registerMethod("setDolbyVisionMode", &ControlSettingsTV::setDolbyVisionMode, this);
        registerMethod("resetDolbyVisionMode", &ControlSettingsTV::resetDolbyVisionMode, this);
        registerMethod("getDolbyVisionModeCaps", &ControlSettingsTV::getDolbyVisionModeCaps, this);
        registerMethod("getSupportedHLGModes", &ControlSettingsTV::getSupportedHLGModes, this);
        registerMethod("getHLGMode", &ControlSettingsTV::getHLGMode, this);
        registerMethod("setHLGMode", &ControlSettingsTV::setHLGMode, this);
        registerMethod("resetHLGMode", &ControlSettingsTV::resetHLGMode, this);
	registerMethod("getHLGModeCaps", &ControlSettingsTV::getHLGModeCaps, this);
        registerMethod("getSupportedHDR10Modes", &ControlSettingsTV::getSupportedHDR10Modes, this);
	registerMethod("getHDR10Mode", &ControlSettingsTV::getHDR10Mode, this);
	registerMethod("setHDR10Mode", &ControlSettingsTV::setHDR10Mode, this);
	registerMethod("resetHDR10Mode", &ControlSettingsTV::resetHDR10Mode, this);
	registerMethod("getHDR10ModeCaps", &ControlSettingsTV::getHDR10ModeCaps, this);
	
	registerMethod("getWBInfo", &ControlSettingsTV::getWBInfo, this);
	registerMethod("getWBCtrl", &ControlSettingsTV::getWBCtrl, this);
	registerMethod("setWBCtrl", &ControlSettingsTV::setWBCtrl, this);
	registerMethod("resetWBCtrl", &ControlSettingsTV::resetWBCtrl, this);

	registerMethod("getAspectRatio2", &ControlSettingsTV::getAspectRatio2, this);
        registerMethod("setAspectRatio2", &ControlSettingsTV::setAspectRatio2, this);
        registerMethod("resetAspectRatio", &ControlSettingsTV::resetAspectRatio, this);
        registerMethod("getAspectRatioCaps", &ControlSettingsTV::getAspectRatioCaps, this);

	GetHandler(2)->Register<JsonObject, JsonObject>("getBacklight", &ControlSettingsTV::getBacklight2, this);
        GetHandler(2)->Register<JsonObject, JsonObject>("getBrightness", &ControlSettingsTV::getBrightness2, this);
        GetHandler(2)->Register<JsonObject, JsonObject>("getContrast", &ControlSettingsTV::getContrast2, this);
        GetHandler(2)->Register<JsonObject, JsonObject>("getSharpness", &ControlSettingsTV::getSharpness2, this);
        GetHandler(2)->Register<JsonObject, JsonObject>("getSaturation", &ControlSettingsTV::getSaturation2, this);
        GetHandler(2)->Register<JsonObject, JsonObject>("gethue", &ControlSettingsTV::getHue2, this);
        GetHandler(2)->Register<JsonObject, JsonObject>("getColorTemperature", &ControlSettingsTV::getColorTemperature2, this);

	registerMethod("getPictureMode", &ControlSettingsTV::getPictureMode, this);
        registerMethod("setPictureMode", &ControlSettingsTV::setPictureMode, this);
	registerMethod("resetPictureMode", &ControlSettingsTV::resetPictureMode, this);
        registerMethod("getSupportedPictureModes", &ControlSettingsTV::getSupportedPictureModes, this);
	registerMethod("enableWBMode", &ControlSettingsTV::enableWBMode, this);

	registerMethod("setBacklightFade", &ControlSettingsTV::setBacklightFade, this);

	registerMethod("getLowLatencyState", &ControlSettingsTV::getLowLatencyState, this);
        registerMethod("setLowLatencyState", &ControlSettingsTV::setLowLatencyState, this);
        registerMethod("resetLowLatencyState", &ControlSettingsTV::resetLowLatencyState, this);
        registerMethod("getLowLatencyStateCaps", &ControlSettingsTV::getLowLatencyStateCaps, this);

        LOGINFO("Exit\n");
    }
    
    ControlSettingsTV :: ~ControlSettingsTV()
    {
        LOGINFO();    
    }

    void ControlSettingsTV::Initialize()
    {
       LOGINFO("Entry\n");
       
       tvError_t ret = tvERROR_NONE;

       TR181_ParamData_t param;
       memset(&param, 0, sizeof(param));

       try {
            dsVideoPortResolution_t vidResolution;
            device::HdmiInput::getInstance().getCurrentVideoModeObj(vidResolution);
            m_currentHdmiInResoluton = vidResolution.pixelResolution;
        } catch (...){
            LOGWARN("ControlSettingsPlugins: getCurrentVideoModeObj failed");
        }
        LOGWARN("ControlSettingsPlugins: ControlSettings Initialize m_currentHdmiInResoluton:%d m_mod:%d", m_currentHdmiInResoluton, m_videoZoomMode);

        ret = tvInit();

        if(ret != tvERROR_NONE) {
            LOGERR("Platform Init failed, ret: %s \n", getErrorString(ret).c_str());

        }
        else{
            LOGINFO("Platform Init successful...\n");
            ret = tvSD3toCriSyncInit();
            if(ret != tvERROR_NONE) {
                LOGERR(" SD3 <->cri_data sync failed, ret: %s \n", getErrorString(ret).c_str());
            }
            else {
                LOGERR(" SD3 <->cri_data sync success, ret: %s \n", getErrorString(ret).c_str());
            }

        }

       LocatePQSettingsFile(rfc_caller_id);

       SyncPQParamsToDriverCache("current","current","current");

       std::thread syncThread = std::thread(StartSync);
       syncThread.detach();

       // As we have source to picture mode mapping, get current source and
       // setting those picture mode
       int current_source = 0; 
       std::string tr181_param_name = "";
       GetCurrentSource(&current_source);
       tr181_param_name += std::string(TVSETTINGS_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
       tr181_param_name += "."+std::to_string(current_source)+"."+"PictureModeString"; 
       tr181ErrorCode_t err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
       if ( tr181Success == err )
       {
           LOGINFO("getLocalParam for %s is %s\n", tr181_param_name.c_str(), param.value);
           ret = SetTVPictureMode(param.value);

           if(ret != tvERROR_NONE) {
               LOGWARN("Picture Mode set failed: %s\n",getErrorString(ret).c_str());
           }
           else {
               LOGINFO("Picture Mode initialized successfully, value: %s\n", param.value);
           }
       }
       else
       {
           LOGWARN("getLocalParam for %s Failed : %s\n", tr181_param_name.c_str(), getTR181ErrorString(err));
       }

       tvBacklightMode_t blMode = tvBacklightMode_NONE;
       memset(&param, 0, sizeof(param));
       err = getLocalParam(rfc_caller_id, TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, &param);
       if ( tr181Success == err )
       {
           LOGINFO("getLocalParam for %s is %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, param.value);

           if(!std::string(param.value).compare("none")) {
               blMode = tvBacklightMode_NONE;
           }
           else if (!std::string(param.value).compare("manual")){
               blMode = tvBacklightMode_MANUAL;
           }
           else if (!std::string(param.value).compare("ambient")){
               blMode = tvBacklightMode_AMBIENT;
           }
           else if (!std::string(param.value).compare("eco")){
               blMode = tvBacklightMode_ECO;
           }
           else {
               blMode = tvBacklightMode_NONE;
           }
           ret = SetCurrentBacklightMode(blMode);

           if(ret != tvERROR_NONE) {
               LOGWARN("Auto Backlight Mode set failed: %s\n",getErrorString(ret).c_str());
           }
           else {
               LOGINFO("Auto Backlight Mode initialized successfully, value: %s\n", param.value);
           }

        }
	else
        {
            LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
        }
        setDefaultAspectRatio();

        LOGINFO("Exit\n" );
    }

    void ControlSettingsTV::Deinitialize()
    {
       LOGINFO("Entry\n");

       tvError_t ret = tvERROR_NONE;
       ret = tvTerm();

       if(ret != tvERROR_NONE) {
           LOGERR("Platform De-Init failed");
       }
       else{
           LOGINFO("Platform De-Init successful... \n");
       }

       LOGINFO("Exit\n");
    }

    //Event
    void ControlSettingsTV::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        LOGINFO("Entry %s\n",__FUNCTION__);
        LOGINFO("Exit %s\n",__FUNCTION__);
    }

    void ControlSettingsTV::dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if(!ControlSettingsTV::instance)
            return;

        if (IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS == eventId)
        {
            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            int hdmi_in_port = eventData->data.hdmi_in_status.port;
            bool hdmi_in_status = eventData->data.hdmi_in_status.isPresented;
            LOGWARN("ControlSettingsPlugins: Received IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS  event  port: %d, started: %d", hdmi_in_port,hdmi_in_status);
            if (!hdmi_in_status){
                tvError_t ret = tvERROR_NONE;
                ControlSettingsTV::instance->m_isDisabledHdmiIn4KZoom = false;
                LOGWARN("ControlSettingsPlugins: Hdmi streaming stopped here reapply the global zoom settings:%d here. m_isDisabledHdmiIn4KZoom: %d", ControlSettingsTV::instance->m_videoZoomMode, ControlSettingsTV::instance->m_isDisabledHdmiIn4KZoom);
                ret = SetAspectRatio((tvDisplayMode_t)ControlSettingsTV::instance->m_videoZoomMode);
                if (ret != tvERROR_NONE) {
                    LOGWARN("SetAspectRatio set Failed");
                }
            }else {
                ControlSettingsTV::instance->m_isDisabledHdmiIn4KZoom = true;
                LOGWARN("ControlSettingsPlugins: m_isDisabledHdmiIn4KZoom: %d", ControlSettingsTV::instance->m_isDisabledHdmiIn4KZoom);
            }
        }
    }

    void ControlSettingsTV::dsHdmiVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if(!ControlSettingsTV::instance)
            return;

        if (IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE == eventId)
        {
            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            int hdmi_in_port = eventData->data.hdmi_in_video_mode.port;
            dsVideoPortResolution_t resolution;
            ControlSettingsTV::instance->m_currentHdmiInResoluton = eventData->data.hdmi_in_video_mode.resolution.pixelResolution;
            resolution.pixelResolution =  eventData->data.hdmi_in_video_mode.resolution.pixelResolution;
            resolution.interlaced =  eventData->data.hdmi_in_video_mode.resolution.interlaced;
            resolution.frameRate =  eventData->data.hdmi_in_video_mode.resolution.frameRate;
            LOGWARN("ControlSettingsPlugins: Received IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE  event  port: %d, pixelResolution: %d, interlaced : %d, frameRate: %d \n", hdmi_in_port,resolution.pixelResolution, resolution.interlaced, resolution.frameRate);
            if (ControlSettingsTV::instance->m_isDisabledHdmiIn4KZoom) {
                tvError_t ret = tvERROR_NONE;
                if (ControlSettingsTV::instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
                    (dsVIDEO_PIXELRES_MAX == ControlSettingsTV::instance->m_currentHdmiInResoluton)){
                    LOGWARN("ControlSettingsPlugins: Setting %d zoom mode for below 4K", ControlSettingsTV::instance->m_videoZoomMode);
                    ret = SetAspectRatio((tvDisplayMode_t)ControlSettingsTV::instance->m_videoZoomMode);
                }else {
                    LOGWARN("ControlSettingsPlugins: Setting auto zoom mode for 4K and above");
                    ret = SetAspectRatio(tvDisplayMode_AUTO);
                }
                if (ret != tvERROR_NONE) {
                    LOGWARN("SetAspectRatio set Failed");
                }
            } else {
                LOGWARN("ControlSettingsPlugins: %s: HdmiInput is not started yet. m_isDisabledHdmiIn4KZoom: %d", __FUNCTION__, ControlSettingsTV::instance->m_isDisabledHdmiIn4KZoom);
            }
        }
    }
    
    uint32_t ControlSettingsTV::getAspectRatioCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject AspectRatioInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(range,pqmode,source,format,"AspectRatio");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
		else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["AspectRatioInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::setAspectRatio2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        tvDisplayMode_t mode = tvDisplayMode_16x9;
        std::string pqmode;
        std::string source;
        std::string format;


        value = parameters.HasLabel("aspectRatio") ? parameters["aspectRatio"].String() : "";
        returnIfParamNotFound(parameters,"aspectRatio");

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

	if( !isCapablityCheckPassed( pqmode, source, format, "AspectRatio" )) {
            LOGERR("%s: CapablityCheck failed for AspectRatio\n", __FUNCTION__);
            returnResponse(false);
        }

        if(!value.compare("TV 16X9 STRETCH")) {
            mode = tvDisplayMode_16x9;
        }
        else if (!value.compare("TV 4X3 PILLARBOX")){
            mode = tvDisplayMode_4x3;
        }
        else if (!value.compare("TV NORMAL")){
            mode = tvDisplayMode_NORMAL;
        }
        else if (!value.compare("TV DIRECT")){
            mode = tvDisplayMode_DIRECT;
        }
        else if (!value.compare("TV AUTO")){
            mode = tvDisplayMode_AUTO;
        }
        else if (!value.compare("TV ZOOM")){
            mode = tvDisplayMode_ZOOM;
        }
        else {
            returnResponse(false);
        }
        m_videoZoomMode = mode;
        tvError_t ret = setAspectRatioZoomSettings (mode);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            //Save DisplayMode to localstore and ssm_data
            int params[3]={0};
            params[0]=mode;
            int retval=UpdatePQParamsToCache("set","AspectRatio",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_ASPECT_RATIO,params);;

            if(retval != 0) {
                LOGWARN("Failed to Save DisplayMode to ssm_data\n");
            }

            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_ASPECTRATIO_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM, value.c_str());
            }
            LOGINFO("Exit : SetAspectRatio2() value : %s\n",value.c_str());
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getAspectRatio2(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        tvDisplayMode_t mode;
        JsonObject dispModeObj;
        JsonArray dispOptions ;
        dispOptions.Add("TV AUTO"); dispOptions.Add("TV DIRECT"); dispOptions.Add("TV NORMAL");
        dispOptions.Add("TV 16X9 STRETCH"); dispOptions.Add("TV 4X3 PILLARBOX"); dispOptions.Add("TV ZOOM");
        dispModeObj["Selected"] = "TV AUTO";
        dispModeObj["Options"] = dispOptions;

        tvError_t ret = getUserSelectedAspectRatio (&mode);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            switch(mode) {

                case tvDisplayMode_16x9:
                    LOGINFO("Aspect Ratio: TV 16X9 STRETCH\n");
                    dispModeObj["Selected"] = "TV 16X9 STRETCH";
                    break;

                case tvDisplayMode_4x3:
                    LOGINFO("Aspect Ratio: TV 4X3 PILLARBOX\n");
                    dispModeObj["Selected"] = "TV 4X3 PILLARBOX";
                    break;

                case tvDisplayMode_NORMAL:
                    LOGINFO("Aspect Ratio: TV Normal\n");
                    dispModeObj["Selected"] = "TV NORMAL";
                    break;

                case tvDisplayMode_AUTO:
                    LOGINFO("Aspect Ratio: TV AUTO\n");
                    dispModeObj["Selected"] = "TV AUTO";
                    break;

                case tvDisplayMode_DIRECT:
                    LOGINFO("Aspect Ratio: TV DIRECT\n");
                    dispModeObj["Selected"] = "TV DIRECT";
                    break;

                case tvDisplayMode_ZOOM:
                    LOGINFO("Aspect Ratio: TV ZOOM\n");
                    dispModeObj["Selected"] = "TV ZOOM";
                    break;

                default:
                    LOGINFO("Aspect Ratio: TV AUTO\n");
                    break;
            }
            response["AspectRatio"] = dispModeObj;
            returnResponse(true);
        }
    }

    tvError_t ControlSettingsTV::setAspectRatioZoomSettings(tvDisplayMode_t mode)
    {
        tvError_t ret = tvERROR_GENERAL;
        LOGERR("tvmgrplugin: %s mode selected is: %d", __FUNCTION__, m_videoZoomMode);
#if !defined (HDMIIN_4K_ZOOM)
        if (ControlSettingsTV::instance->m_isDisabledHdmiIn4KZoom) {
            if (ControlSettingsTV::instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
                (dsVIDEO_PIXELRES_MAX == m_currentHdmiInResoluton)){
                LOGWARN("tvmgrplugin: %s: Setting %d zoom mode for below 4K", __FUNCTION__, m_videoZoomMode);
#endif
                ret = SetAspectRatio(mode);
#if !defined (HDMIIN_4K_ZOOM)
            }else {
                LOGWARN("tvmgrplugin: %s: Setting auto zoom mode for 4K and above", __FUNCTION__);
                ret = SetAspectRatio(tvDisplayMode_AUTO);
            }
        } else {
            LOGWARN("tvmgrplugin: %s: HdmiInput is not started yet. m_isDisabledHdmiIn4KZoom: %d", __FUNCTION__, ControlSettingsTV::instance->m_isDisabledHdmiIn4KZoom);
            ret = SetAspectRatio((tvDisplayMode_t)m_videoZoomMode);
        }
#endif
        return ret;
    }

    tvError_t ControlSettingsTV::getUserSelectedAspectRatio (tvDisplayMode_t* mode)
    {
        tvError_t ret = tvERROR_GENERAL;
#if !defined (HDMIIN_4K_ZOOM)
        LOGERR("controlsettingsplugin: %s mode selected is: %d", __FUNCTION__, m_videoZoomMode);
        if (ControlSettingsTV::instance->m_isDisabledHdmiIn4KZoom) {
            if (!(ControlSettingsTV::instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
               (dsVIDEO_PIXELRES_MAX == ControlSettingsTV::instance->m_currentHdmiInResoluton))){
                *mode = (tvDisplayMode_t)ControlSettingsTV::instance->m_videoZoomMode;
                LOGWARN("tvmgrplugin: %s: Getting zoom mode %d for display, for 4K and above", __FUNCTION__, *mode);
                return tvERROR_NONE;
            }
        }
#endif
        ret = GetAspectRatio(mode);
        return ret;
    }

    uint32_t ControlSettingsTV::resetAspectRatio(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

	if( !isCapablityCheckPassed( pqmode, source, format, "AspectRatio" )) {
            LOGERR("%s: CapablityCheck failed for AspectRatio\n", __FUNCTION__);
            returnResponse(false);
        }

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_ASPECTRATIO_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM);
            ret = setDefaultAspectRatio(pqmode,source,format);
        }
	if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetDefaultAspectRatio()\n");
            returnResponse(true);
        }
    }

    tvError_t ControlSettingsTV::setDefaultAspectRatio(std::string pqmode,std::string  format,std::string source)
    {
        tvDisplayMode_t mode = tvDisplayMode_MAX;
        TR181_ParamData_t param;
        tvError_t ret = tvERROR_NONE;

        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_ASPECTRATIO_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            LOGINFO("getLocalParam for %s is %s\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM, param.value);

            if(!std::string(param.value).compare("16:9")) {
                mode = tvDisplayMode_16x9;
            }
            else if (!std::string(param.value).compare("4:3")){
                mode = tvDisplayMode_4x3;
            }
            else if (!std::string(param.value).compare("Full")){
                mode = tvDisplayMode_FULL;
            }
            else if (!std::string(param.value).compare("Normal")){
                mode = tvDisplayMode_NORMAL;
            }
            else if (!std::string(param.value).compare("TV AUTO")){
                mode = tvDisplayMode_AUTO;
            }
            else if (!std::string(param.value).compare("TV DIRECT")){
                mode = tvDisplayMode_DIRECT;
            }
	    else if (!std::string(param.value).compare("TV NORMAL")){
                mode = tvDisplayMode_NORMAL;
            }
            else if (!std::string(param.value).compare("TV ZOOM")){
                mode = tvDisplayMode_ZOOM;
            }
            else if (!std::string(param.value).compare("TV 16X9 STRETCH")){
                mode = tvDisplayMode_16x9;
            }
            else if (!std::string(param.value).compare("TV 4X3 PILLARBOX")){
                mode = tvDisplayMode_4x3;
            }
            else {
                mode = tvDisplayMode_AUTO;
            }

            m_videoZoomMode = mode;
            tvError_t ret = setAspectRatioZoomSettings (mode);

            if(ret != tvERROR_NONE) {
                LOGWARN("AspectRatio  set failed: %s\n",getErrorString(ret).c_str());
            }
            else {
                //Save DisplayMode to ssm_data
                int params[3]={0};
                params[0]=mode;
                int retval=UpdatePQParamsToCache("set","AspectRatio",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_ASPECT_RATIO,params);

                if(retval != 0) {
                    LOGWARN("Failed to Save DisplayMode to ssm_data\n");
                }
                LOGINFO("Aspect Ratio initialized successfully, value: %s\n", param.value);
            }

        }
	else
        {
            LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            ret = tvERROR_GENERAL;
        }
        return ret;
    }

    uint32_t ControlSettingsTV::getVideoFormat(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        //PLUGIN_Lock(Lock);
        tvVideoHDRFormat_t videoFormat;
        tvError_t ret = GetCurrentVideoFormat(&videoFormat);
        response["supportedVideoFormat"] = getSupportedVideoFormat();
        if(ret != tvERROR_NONE) {
            response["currentVideoFormat"] = "NONE";
            returnResponse(false);
        }
        else {
            response["currentVideoFormat"] = getVideoFormatTypeToString(videoFormat);
            LOGINFO("Exit: getVideoFormat :%d   success \n",videoFormat);
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getVideoResolution(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        //PLUGIN_Lock(Lock);
        tvResolutionParam_t videoResolution;
        tvError_t ret = GetVideoResolution(&videoResolution);
        response["supportedVideoResolution"] = getSupportedVideoResolution();
        if(ret != tvERROR_NONE) {
            response["currentVideoResolution"] = "NONE";
            returnResponse(false);
        }
        else {
            response["currentVideoResolution"] = getVideoResolutionTypeToString(videoResolution);
            LOGINFO("Exit: getVideoResolution :%d   success \n",videoResolution.resolutionValue);
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getVideoFrameRate(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvVideoFrameRate_t videoFramerate;
        tvError_t ret = GetVideoFrameRate(&videoFramerate);
        response["supportedFrameRate"] = getSupportedVideoFrameRate();
        if(ret != tvERROR_NONE) {
            response["currentVideoFrameRate"] = "NONE";
            returnResponse(false);
        }
        else {
            response["currentVideoFrameRate"] = getVideoFrameRateTypeToString(videoFramerate);
            LOGINFO("Exit: videoFramerate :%d   success \n",videoFramerate);
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int backlight = 0,err = 0;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";
			
        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        err = GetLocalparam("Backlight",formatIndex,pqIndex,sourceIndex,backlight,PQ_PARAM_BACKLIGHT);
        
        if( err == 0 ) {
            response["backlight"] = std::to_string(backlight);
            LOGINFO("Exit : Backlight Value: %d \n", backlight);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::getBacklight2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        JsonObject range;
        JsonObject backlightObj;
        range["From"] = 0;
        range["To"] = 100;
        backlightObj["Range"] = range;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int backlight = 0,err = 0;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        err = GetLocalparam("Backlight",formatIndex,pqIndex,sourceIndex,backlight, PQ_PARAM_BACKLIGHT);
        if( err == 0 ) {
            backlightObj["Setting"] = std::to_string(backlight);
            response["Backlight"] = backlightObj;
            LOGINFO("Exit : Backlight Value: %d \n", backlight);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::setBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
     
	std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int backlight = 0;
        tvError_t ret  = tvERROR_NONE;

        value = parameters.HasLabel("backlight") ? parameters["backlight"].String() : "";
        returnIfParamNotFound(parameters,"backlight");
        backlight = std::stoi(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

	if( !isCapablityCheckPassed( pqmode, source, format, "Backlight" )) {
            LOGERR("%s: CapablityCheck failed for Backlight\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) { //Needs rework
            LOGINFO("Proceed with setBacklight\n");
            ret = SetBacklight(backlight,pqmode.c_str(),format.c_str(),source.c_str(),true);
        }

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set backlight\n");
            returnResponse(false);
        }
        else {
            LOGINFO("Exit : setBacklight successful to value: %d\n",backlight);
            returnResponse(true);
        }

    }

    uint32_t ControlSettingsTV::resetBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,backlight=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        if( !isCapablityCheckPassed( pqmode, source, format, "Backlight" )) {
            LOGERR("%s: CapablityCheck failed for Backlight\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) { //Needs rework
            LOGINFO("Proceed with setBacklight\n");
            ret = SetBacklight(backlight,pqmode.c_str(),format.c_str(),source.c_str(),true);
        }

       // if(appUsesGlobalBackLightFactor){
            tr181ErrorCode_t err = clearLocalParam(rfc_caller_id, TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM);
            if ( err != tr181Success ) {
                LOGWARN("ClearLocalParam for %s Failed : %s\n", TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM, getTR181ErrorString(err));
                ret = tvERROR_GENERAL;
            }
            else {
                LOGINFO("ClearLocalParam for %s Successful\n", TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM);
            }
            err = clearLocalParam(rfc_caller_id, TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM);
            if ( err != tr181Success ) {
                LOGWARN("ClearLocalParam for %s Failed : %s\n", TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM, getTR181ErrorString(err));
                ret = tvERROR_GENERAL;
            }
//      }
        /* non backlight factor path */
	int retval= UpdatePQParamsToCache("reset","Backlight",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_BACKLIGHT,params);
        if(retval != 0 ) {
            LOGWARN("Failed to reset Backlight\n");
            returnResponse(false);
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("Backlight",formatIndex,pqIndex,sourceIndex,backlight, PQ_PARAM_BACKLIGHT);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,backlight);
                if( isSetRequired(pqmode,source,format) ) { //Needs rework
                    LOGINFO("Proceed with setBacklight\n");
                    ret = SetBacklight(backlight,pqmode.c_str(),format.c_str(),source.c_str(),false);
                }
            }
            else
                LOGINFO("%s : GetLcoalParam Failed \n",__FUNCTION__);
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetBacklight Successful to value : %d \n",backlight);
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getBacklightCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject rangeInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(range,pqmode,source,format,"Backlight");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["rangeInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getBrightness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
	std::string key;
	int sourceIndex=0,pqIndex=0,formatIndex=0;
        int brightness = 0;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("Brightness",formatIndex,pqIndex,sourceIndex,brightness,PQ_PARAM_BRIGHTNESS,false);
        if( err == 0 ) {
            response["brightness"] = std::to_string(brightness);
            LOGINFO("Exit : Brightness Value: %d \n", brightness);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::getBrightness2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int brightness = 0;
        JsonObject range;
        JsonObject brightnessObj;
        range["From"] = 0;
        range["To"] = 100;
        brightnessObj["Range"] = range;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("Brightness",formatIndex,pqIndex,sourceIndex,brightness, PQ_PARAM_BRIGHTNESS);
        if( err == 0 ) {
            brightnessObj["Setting"] = std::to_string(brightness);
            response["Brightness"] = brightnessObj;
            LOGINFO("Exit : Brightness Value: %d \n", brightness);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::setBrightness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int brightness = 0;
	tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("brightness") ? parameters["brightness"].String() : "";
        returnIfParamNotFound(parameters,"brightness");
        brightness = stoi(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";
        
        if( !isCapablityCheckPassed( pqmode, source, format, "Brightness" )) {
            LOGERR("%s: CapablityCheck failed for Brightness\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s \n",__FUNCTION__);
             ret = SetBrightness(brightness);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

	if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set Brightness\n");
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=brightness;
            int retval= UpdatePQParamsToCache("set","Brightness",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_BRIGHTNESS,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Brightness to ssm_data\n");
            }
            LOGINFO("Exit : setBrightness successful to value: %d\n", brightness);
            returnResponse(true);
        }

    }

    uint32_t ControlSettingsTV::resetBrightness(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,brightness=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

	if( !isCapablityCheckPassed( pqmode, source, format, "Brightness" )) {
            LOGERR("%s: CapablityCheck failed for Brightness\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s \n",__FUNCTION__);
             ret = SetBrightness(brightness);
        }
	else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        int retval= UpdatePQParamsToCache("reset","Brightness",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_BRIGHTNESS,params);
        if(retval != 0 ) {
            LOGWARN("Failed to reset Brightness\n");
            returnResponse(false);
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("Brightness",formatIndex,pqIndex,sourceIndex,brightness, PQ_PARAM_BRIGHTNESS);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,brightness);
                ret = SetBrightness(brightness);
            }
            else
                LOGINFO("%s : GetLcoalParam Failed \n",__FUNCTION__);
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetBrightness Successful to value : %d \n",brightness);
            returnResponse(true);
        }

    }

    uint32_t ControlSettingsTV::getBrightnessCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
	std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;
        
        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

	JsonObject rangeInfo;
	JsonObject pqmodeInfo;
	JsonObject sourceInfo;
	JsonObject formatInfo;

	unsigned int index = 0;

	tvError_t ret = GetParamsCaps(range,pqmode,source,format,"Brightness");

	if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["rangeInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

	    response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
	}
    }

    uint32_t ControlSettingsTV::getContrast(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int contrast = 0;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("Contrast",formatIndex,pqIndex,sourceIndex,contrast,PQ_PARAM_CONTRAST);
        if( err == 0 ) {
            response["contrast"] = std::to_string(contrast);
            LOGINFO("Exit : Contrast Value: %d \n", contrast);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::getContrast2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int contrast = 0;
        JsonObject range;
        JsonObject contrastObj;
        range["From"] = 0;
        range["To"] = 100;
        contrastObj["Range"] = range;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("Contrast",formatIndex,pqIndex,sourceIndex,contrast, PQ_PARAM_CONTRAST);
        if( err == 0 ) {
            contrastObj["Setting"] = std::to_string(contrast);
            response["Contrast"] = contrastObj;
            LOGINFO("Exit : Contrast Value: %d \n", contrast);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::setContrast(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int contrast = 0;
	tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("contrast") ? parameters["contrast"].String() : "";
        returnIfParamNotFound(parameters,"contrast");
        contrast = stoi(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";
        
        if( !isCapablityCheckPassed( pqmode, source, format, "Contrast" )) {
            LOGERR("%s: CapablityCheck failed for Contrast\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s \n",__FUNCTION__);
             ret = SetContrast(contrast);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set Contrast\n");
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=contrast;
            int retval= UpdatePQParamsToCache("set","Contrast",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CONTRAST,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Contrast to ssm_data\n");
            }
            LOGINFO("Exit : setContrast successful to value: %d\n", contrast);
            returnResponse(true);
        }

    }

    uint32_t ControlSettingsTV::resetContrast(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,contrast=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

	if( !isCapablityCheckPassed( pqmode, source, format, "Contrast" )) {
            LOGERR("%s: CapablityCheck failed for Contrast\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= UpdatePQParamsToCache("reset","Contrast",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CONTRAST,params);

        if(retval != 0 ) {
            LOGWARN("Failed to reset Contrast\n");
            returnResponse(false);
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("Contrast",formatIndex,pqIndex,sourceIndex,contrast,PQ_PARAM_CONTRAST);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,contrast);
                if( isSetRequired(pqmode,source,format) ) {
                    LOGINFO("Proceed with %s \n",__FUNCTION__);
                    ret = SetContrast(contrast);
                }
                else
                    LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);
            }
            else
                LOGINFO("%s : GetLcoalParam Failed \n",__FUNCTION__);
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetContrast Successful to value : %d \n",contrast);
            returnResponse(true);
        }

    }

    uint32_t ControlSettingsTV::getContrastCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject rangeInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(range,pqmode,source,format,"Contrast");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["rangeInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int saturation = 0;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("Saturation",formatIndex,pqIndex,sourceIndex,saturation, PQ_PARAM_SATURATION);
        if( err == 0 ) {
            response["saturation"] = std::to_string(saturation);
            LOGINFO("Exit : Saturation Value: %d \n", saturation);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::getSaturation2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int saturation = 0;
        JsonObject range;
        JsonObject saturationObj;
        range["From"] = 0;
        range["To"] = 100;
        saturationObj["Range"] = range;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("Saturation",formatIndex,pqIndex,sourceIndex,saturation, PQ_PARAM_SATURATION);
        if( err == 0 ) {
            saturationObj["Setting"] = std::to_string(saturation);
            response["Saturation"] = saturationObj;
            LOGINFO("Exit : Saturation Value: %d \n", saturation);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::setSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int saturation = 0;
        tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("saturation") ? parameters["saturation"].String() : "";
        returnIfParamNotFound(parameters,"saturation");
        saturation = stoi(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";
       
        if( !isCapablityCheckPassed( pqmode, source, format, "Saturation" )) {
            LOGERR("%s: CapablityCheck failed for Saturation\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetSaturation(saturation);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);


        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set Saturation\n");
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=saturation;
            int retval= UpdatePQParamsToCache("set","Saturation",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_SATURATION,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Saturation to ssm_data\n");
            }
            LOGINFO("Exit : setSaturation successful to value: %d\n", saturation);
            returnResponse(true);
        }

    }

    uint32_t ControlSettingsTV::resetSaturation(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,saturation=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        if( !isCapablityCheckPassed( pqmode, source, format, "Saturation" )) {
            LOGERR("%s: CapablityCheck failed for Saturation\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetSaturation(saturation);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        int retval= UpdatePQParamsToCache("reset","Saturation",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_SATURATION,params);

        if(retval != 0 ) {
            LOGWARN("Failed to reset Saturation\n");
            returnResponse(false);
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("Saturation",formatIndex,pqIndex,sourceIndex,saturation,PQ_PARAM_SATURATION);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,saturation);
                if( isSetRequired(pqmode,source,format) ) {
                    LOGINFO("Proceed with %s\n",__FUNCTION__);
                    ret = SetSaturation(saturation);
                }
                else
                    LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

            }
            else
                LOGINFO("%s : GetLcoalParam Failed \n",__FUNCTION__);
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetSaturation Successful to value : %d \n",saturation);
            returnResponse(true);
        }

    }

    uint32_t ControlSettingsTV::getSaturationCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject rangeInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(range,pqmode,source,format,"Saturation");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["rangeInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getSharpness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int sharpness = 0;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("Sharpness",formatIndex,pqIndex,sourceIndex,sharpness, PQ_PARAM_SHARPNESS);
        if( err == 0 ) {
            response["sharpness"] = std::to_string(sharpness);
            LOGINFO("Exit : Sharpness Value: %d \n", sharpness);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::getSharpness2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int sharpness = 0;
        JsonObject range;
        JsonObject sharpnessObj;
        range["From"] = 0;
        range["To"] = 100;
        sharpnessObj["Range"] = range;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("Sharpness",formatIndex,pqIndex,sourceIndex,sharpness, PQ_PARAM_SHARPNESS);
        if( err == 0 ) {
            sharpnessObj["Setting"] = std::to_string(sharpness);
            response["Sharpness"] = sharpnessObj;
            LOGINFO("Exit : Sharpness Value: %d \n", sharpness);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::setSharpness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sharpness = 0;
        tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("sharpness") ? parameters["sharpness"].String() : "";
        returnIfParamNotFound(parameters,"sharpness");
        sharpness = stoi(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";
    
        if( !isCapablityCheckPassed( pqmode, source, format, "Sharpness" )) {
            LOGERR("%s: CapablityCheck failed for Sharpness\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetSharpness(sharpness);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set Sharpness\n");
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=sharpness;
            int retval= UpdatePQParamsToCache("set","Sharpness",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_SHARPNESS,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Sharpness to ssm_data\n");
            }
            LOGINFO("Exit : setSharpness successful to value: %d\n", sharpness);
            returnResponse(true);
        }

    }

    uint32_t ControlSettingsTV::resetSharpness(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,sharpness=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        if( !isCapablityCheckPassed( pqmode, source, format, "Sharpness" )) {
            LOGERR("%s: CapablityCheck failed for Sharpness\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= UpdatePQParamsToCache("reset","Sharpness",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_SHARPNESS,params);

        if(retval != 0 ) {
            LOGWARN("Failed to reset Sharpness\n");
            returnResponse(false);
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("Sharpness",formatIndex,pqIndex,sourceIndex,sharpness, PQ_PARAM_SHARPNESS);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,sharpness);
                if( isSetRequired(pqmode,source,format) ) {
                    LOGINFO("Proceed with %s\n",__FUNCTION__);
                    ret = SetSharpness(sharpness);
                }
                else
                    LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);
            }
            else
                LOGINFO("%s : GetLcoalParam Failed \n",__FUNCTION__);
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetSharpness Successful to value : %d \n",sharpness);
            returnResponse(true);
        }

    }

    uint32_t ControlSettingsTV::getSharpnessCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject rangeInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(range,pqmode,source,format,"Sharpness");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["rangeInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int hue = 0;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("Hue",formatIndex,pqIndex,sourceIndex,hue, PQ_PARAM_HUE);
        if( err == 0 ) {
            response["hue"] = std::to_string(hue);
            LOGINFO("Exit : Hue Value: %d \n", hue);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::getHue2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int hue = 0;
        JsonObject range;
        JsonObject hueObj;
        range["From"] = 0;
        range["To"] = 100;
        hueObj["Range"] = range;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("Hue",formatIndex,pqIndex,sourceIndex,hue, PQ_PARAM_HUE);
        if( err == 0 ) {
            hueObj["Setting"] = std::to_string(hue);
            response["Hue"] = hueObj;
            LOGINFO("Exit : Hue Value: %d \n", hue);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::setHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int hue = 0;
        tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("hue") ? parameters["hue"].String() : "";
        returnIfParamNotFound(parameters,"hue");
        hue = stoi(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        if( !isCapablityCheckPassed( pqmode, source, format, "Hue" )) {
            LOGERR("%s: CapablityCheck failed for Hue\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetHue(hue);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);


        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set Hue\n");
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=hue;
            int retval= UpdatePQParamsToCache("set","Hue",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_HUE,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Hue to ssm_data\n");
            }
            LOGINFO("Exit : setHue successful to value: %d\n", hue);
            returnResponse(true);
        }

    }

    uint32_t ControlSettingsTV::resetHue(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,hue=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

	if( !isCapablityCheckPassed( pqmode, source, format, "Hue" )) {
            LOGERR("%s: CapablityCheck failed for Hue\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= UpdatePQParamsToCache("reset","Hue",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_HUE,params);

        if(retval != 0 ) {
            LOGWARN("Failed to reset Hue\n");
            returnResponse(false);
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("Hue",formatIndex,pqIndex,sourceIndex,hue,PQ_PARAM_HUE);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,hue);
                if( isSetRequired(pqmode,source,format) ) {
                    LOGINFO("Proceed with %s\n",__FUNCTION__);
                    ret = SetHue(hue);
                }
                else
                    LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);
            }
            else
                LOGINFO("%s : GetLcoalParam Failed \n",__FUNCTION__);
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetHue Successful to value : %d \n",hue);
            returnResponse(true);
        }

    }

    uint32_t ControlSettingsTV::getHueCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject rangeInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(range,pqmode,source,format,"Hue");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["rangeInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getColorTemperature(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int colortemp = 0;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("ColorTemp",formatIndex,pqIndex,sourceIndex,colortemp, PQ_PARAM_COLOR_TEMPERATURE);
        if( err == 0 ) {
            switch(colortemp) {

                case tvColorTemp_STANDARD:
                    LOGINFO("Color Temp Value: Standard\n");
                    response["colorTemp"] = "Standard";
                    break;

                case tvColorTemp_WARM:
                    LOGINFO("Color Temp Value: Warm\n");
                    response["colorTemp"] = "Warm";
                    break;

                case tvColorTemp_COLD:
                    LOGINFO("Color Temp Value: Cold\n");
                    response["colorTemp"] = "Cold";
                    break;

                case tvColorTemp_USER:
                    LOGINFO("Color Temp Value: User Defined\n");
                    response["colorTemp"] = "User Defined";
                    break;

                default:
                    LOGINFO("Color Temp Value: Standard\n");
                    response["colorTemp"] = "Standard";
                    break;
            }
            LOGINFO("Exit : ColorTemperature Value: %d \n", colortemp);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::getColorTemperature2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int colortemp = 0;
		JsonObject ctObj;
        JsonArray ctOptions;
        ctOptions.Add("Standard"); ctOptions.Add("Warm"); ctOptions.Add("Cold"); ctOptions.Add("User Defined");
        ctObj["Selected"] = "Standard";
        ctObj["Options"] = ctOptions;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("ColorTemp",formatIndex,pqIndex,sourceIndex,colortemp,PQ_PARAM_COLOR_TEMPERATURE);
        if( err == 0 ) {
            switch(colortemp) {

                case tvColorTemp_STANDARD:
                    LOGINFO("Color Temp Value: Standard\n");
                    ctObj["Selected"] = "Standard";
                    break;

                case tvColorTemp_WARM:
                    LOGINFO("Color Temp Value: Warm\n");
                    ctObj["Selected"] = "Warm";
                    break;

                case tvColorTemp_COLD:
                    LOGINFO("Color Temp Value: Cold\n");
                    ctObj["Selected"] = "Cold";
                    break;

                case tvColorTemp_USER:
                    LOGINFO("Color Temp Value: User Defined\n");
                    ctObj["Selected"] = "User Defined";
                    break;

                default:
                    LOGINFO("Color Temp Value: Standard\n");
                    ctObj["Selected"] = "Standard";
                    break;
            }
            LOGINFO("Exit : ColorTemperature Value: %d \n", colortemp);
            response["ColorTemperature"] = ctObj;
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::setColorTemperature(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int colortemp = 0;
        tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("colorTemp") ? parameters["colorTemp"].String() : "";
        returnIfParamNotFound(parameters,"colorTemp");
        if(!value.compare("Standard")) {
            colortemp = tvColorTemp_STANDARD;
        }
        else if (!value.compare("Warm")){
            colortemp = tvColorTemp_WARM;
        }
        else if (!value.compare("Cold")){
            colortemp = tvColorTemp_COLD;
        }
        else if (!value.compare("User Defined")){
            colortemp = tvColorTemp_USER;
        }
        else {
            returnResponse(false);
        }

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        if( !isCapablityCheckPassed( pqmode, source, format, "ColorTemperature" )) {
            LOGERR("%s: CapablityCheck failed for colorTemperature\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetColorTemperature((tvColorTemp_t)colortemp);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set ColorTemperature\n");
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=colortemp;
            int retval= UpdatePQParamsToCache("set","ColorTemp",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_COLOR_TEMPERATURE,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save ColorTemperature to ssm_data\n");
            }
            LOGINFO("Exit : setColorTemperature successful to value: %d\n", colortemp);
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::resetColorTemperature(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,colortemp=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        if( !isCapablityCheckPassed( pqmode, source, format, "ColorTemperature" )) {
            LOGERR("%s: CapablityCheck failed for colorTemperature\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= UpdatePQParamsToCache("reset","ColorTemp",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_COLOR_TEMPERATURE,params);

        if(retval != 0 ) {
            LOGWARN("Failed to reset ColorTemperature\n");
            returnResponse(false);
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("ColorTemp",formatIndex,pqIndex,sourceIndex,colortemp,PQ_PARAM_COLOR_TEMPERATURE);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,colortemp);
                if( isSetRequired(pqmode,source,format) ) {
                    LOGINFO("Proceed with %s\n",__FUNCTION__);
                    ret = SetColorTemperature((tvColorTemp_t)colortemp);
                }
                else
                    LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

            }
            else
                LOGINFO("%s : GetLcoalParam Failed \n",__FUNCTION__);
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetColorTemperature Successful to value : %d \n",colortemp);
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getColorTemperatureCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject rangeInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(range,pqmode,source,format,"ColorTemperature");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
		else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["rangeInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getComponentSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
	std::string cms("saturation.");
	std::string value;
	int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvDataComponentColor_t blSaturationColor;
        int saturation=0;
        JsonObject range;
        JsonObject saturationColorObj;
        range["From"] = 0;
        range["To"] = 100;
        saturationColorObj["Range"] = range;

	value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(parameters,"color");

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        blSaturationColor =  GetComponentColorEnum(value);
        if(blSaturationColor ==tvDataColor_MAX)
        {
            returnResponse(false);
        }
        cms += value;
        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);

	int err = GetLocalparam(cms.c_str(),formatIndex,pqIndex,sourceIndex,saturation,
			        PQ_PARAM_COMPONENT_SATURATION);
	if( err == 0 || err == 1 ) {//err value willbe 1 if cms is default.(SPECIAL case)
            saturationColorObj["Setting"] = std::to_string(saturation);
            response["saturation"] = saturationColorObj;
            LOGINFO("Exit : Component Saturation for color: %s Value: %d\n", value.c_str(),saturation);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }

    }

    uint32_t ControlSettingsTV::setComponentSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int saturation = 0;
        tvDataComponentColor_t blSaturationColor;
        tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("saturation") ? parameters["saturation"].String() : "";
        returnIfParamNotFound(parameters,"saturation");
        saturation = stoi(value);

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(parameters,"color");

	blSaturationColor =  GetComponentColorEnum(value);
        if(blSaturationColor ==tvDataColor_MAX)
        {
            returnResponse(false);
        }

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";
        
        if( !isCapablityCheckPassed( pqmode, source, format, "Component" )) {
            LOGERR("%s: CapablityCheck failed for ComponentSaturation\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_ENABLE);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        if(ret != tvERROR_NONE) {
            LOGWARN("CMS enable failed\n");
            returnResponse(false);
        }
        else
        {
            int cms_params[3]={0};
            cms_params[0]=COLOR_STATE;
            cms_params[1]=COLOR_ENABLE;
            cms_params[2]=COMPONENT_ENABLE;
            int retval = UpdatePQParamsToCache("set","cms.enable",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_COMPONENT_SATURATION,cms_params);
            if(retval != 0) {
                LOGWARN("Failed to Save enableflag to ssm_data\n");
            }
            LOGINFO("CMS Enable Success to localstore\n");
        }
        
	if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetCurrentComponentSaturation(blSaturationColor, saturation);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=COLOR_SATURATION;
            params[1]=ConvertTVColorToVendorColor(blSaturationColor);
            params[2]=saturation;

            char tr181format[64]={0};
            snprintf(tr181format,sizeof(tr181format),"saturation.%s",component_color[ConvertTVColorToVendorColor(blSaturationColor)]);
            int retval=UpdatePQParamsToCache("set",tr181format,pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_COMPONENT_SATURATION,params);

            if(retval != 0) {
                LOGWARN("Failed to Save component saturation to ssm_data\n");
            }
            LOGINFO("Exit : SetComponentSaturation() %s : %s\n",format.c_str(),value.c_str());
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::resetComponentSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t err = tvERROR_NONE;
	char param[BUFFER_SIZE]={0};

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";
	
        if( !isCapablityCheckPassed( pqmode, source, format, "Component" )) {
            LOGERR("%s: CapablityCheck failed for ComponentSaturation\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
              err = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_DISABLE);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);


        if(err == tvERROR_NONE)
        {
            int params[3]={0};
            params[0]=COLOR_STATE;
            params[1]=COLOR_ENABLE;
            int ret=UpdatePQParamsToCache("reset","cms.enable",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_COMPONENT_SATURATION,params);
            if(ret != 0) {
                LOGWARN("Failed to Save enable flag to ssm_data\n");
            }

            for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
            {
                params[0]=COLOR_SATURATION;
                params[1]=color;
                snprintf(param,sizeof(param),"saturation.%s",component_color[color]);
                ret |= UpdatePQParamsToCache("reset",param,pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_COMPONENT_SATURATION,params);
                memset(&param, 0, sizeof(param));
            }

            if(ret != 0)
            {
                LOGWARN("resetComponentSaturation Failed couldn't remove from localstore error %d \n", ret);
                err=tvERROR_GENERAL;
            }
        }
		
	if(err != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetComponentSaturation()\n");
            returnResponse(true);
        }
    }


    uint32_t ControlSettingsTV::getComponentHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string cms("hue.");
        std::string value;
        tvDataComponentColor_t blHueColor;
	int sourceIndex=0,pqIndex=0,formatIndex=0;
        int hue=0;
        JsonObject range;
        JsonObject hueColorObj;
        range["From"] = 0;
        range["To"] = 100;
        hueColorObj["Range"] = range;

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(parameters,"color");

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        blHueColor =  GetComponentColorEnum(value);
        if(blHueColor ==tvDataColor_MAX)
        {
            returnResponse(false);
        }
        cms += value;
        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam(cms.c_str(),formatIndex,pqIndex,sourceIndex,hue,PQ_PARAM_COMPONENT_HUE );
        if( err == 0 || err == 1 ) {//err value willbe 1 if cms is default.(SPECIAL case)
            hueColorObj["Setting"] = std::to_string(hue);
            response["hue"] = hueColorObj;
            LOGINFO("Exit : Component Hue for color: %s Value: %d\n", value.c_str(),hue);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }


    uint32_t ControlSettingsTV::setComponentHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int hue = 0;
        tvDataComponentColor_t blHueColor;
        tvError_t ret = tvERROR_NONE;
        value = parameters.HasLabel("hue") ? parameters["hue"].String() : "";
        returnIfParamNotFound(parameters,"hue");
        hue = stoi(value);

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(parameters,"color");

        blHueColor =  GetComponentColorEnum(value);
        if(blHueColor ==tvDataColor_MAX)
        {
            returnResponse(false);
        }

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

	if( !isCapablityCheckPassed( pqmode, source, format, "Component" )) {
            LOGERR("%s: CapablityCheck failed for ComponentHue\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_ENABLE);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        if(ret != tvERROR_NONE) {
            LOGWARN("CMS enable failed\n");
            returnResponse(false);
        }
        else
        {
            int cms_params[3]={0};
            cms_params[0]=COLOR_STATE;
            cms_params[1]=COLOR_ENABLE;
            cms_params[2]=COMPONENT_ENABLE;
            int retval = UpdatePQParamsToCache("set","cms.enable",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_COMPONENT_HUE,cms_params);
            if(retval != 0) {
                LOGWARN("Failed to Save enableflag to ssm_data\n");
            }
            LOGINFO("CMS Enable Success to localstore\n");
        }
        
	if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetCurrentComponentHue(blHueColor, hue);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=COLOR_HUE;
            params[1]=ConvertTVColorToVendorColor(blHueColor);
            params[2]=hue;

            char tr181format[64]={0};
            snprintf(tr181format,sizeof(tr181format),"hue.%s",component_color[ConvertTVColorToVendorColor(blHueColor)]);
            int retval=UpdatePQParamsToCache("set",tr181format,pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_COMPONENT_HUE,params);

            if(retval != 0) {
                LOGWARN("Failed to Save component hue to ssm_data\n");
            }
            LOGINFO("Exit : SetComponentHue() %s : %s\n",format.c_str(),value.c_str());
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::resetComponentHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t err = tvERROR_NONE;
        char param[BUFFER_SIZE]={0};

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

	if( !isCapablityCheckPassed( pqmode, source, format, "Component" )) {
            LOGERR("%s: CapablityCheck failed for ComponentHue\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             err = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_DISABLE);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        if(err == tvERROR_NONE)
        {
            int params[3]={0};
            params[0]=COLOR_STATE;
            params[1]=COLOR_ENABLE;
            int ret=UpdatePQParamsToCache("reset","cms.enable",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_COMPONENT_HUE,params);
            if(ret != 0) {
                LOGWARN("Failed to Save enable flag to ssm_data\n");
            }

            for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
            {
                params[0]=COLOR_HUE;
                params[1]=color;
                snprintf(param,sizeof(param),"hue.%s",component_color[color]);
                ret |= UpdatePQParamsToCache("reset",param,pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CMS,params);
                memset(&param, 0, sizeof(param));
            }

            if(ret != 0)
            {
                LOGWARN("resetComponentHue Failed couldn't remove from localstore error %d \n", ret);
                err=tvERROR_GENERAL;
            }
        }

        if(err != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetComponentHue()\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getComponentLuma(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string cms("luma.");
        std::string value;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvDataComponentColor_t blLumaColor;
        int luma=0;
        JsonObject range;
        JsonObject lumaColorObj;
        range["From"] = 0;
        range["To"] = 100;
        lumaColorObj["Range"] = range;

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(parameters,"color");

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        blLumaColor =  GetComponentColorEnum(value);
        if(blLumaColor ==tvDataColor_MAX)
        {
            returnResponse(false);
        }
        cms += value;
	GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam(cms.c_str(),formatIndex,pqIndex,sourceIndex,luma,PQ_PARAM_COMPONENT_LUMA);
        if( err == 0 || err == 1 ) {//err value willbe 1 if cms is default.(SPECIAL case)
            lumaColorObj["Setting"] = std::to_string(luma);
            response["luma"] = lumaColorObj;
            LOGINFO("Exit : Component Luma for color: %s Value: %d\n", value.c_str(),luma);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }
    
    uint32_t ControlSettingsTV::setComponentLuma(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int luma = 0;
        tvDataComponentColor_t blLumaColor;
        tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("luma") ? parameters["luma"].String() : "";
        returnIfParamNotFound(parameters,"luma");
        luma = stoi(value);

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(parameters,"color");

        blLumaColor =  GetComponentColorEnum(value);
        if(blLumaColor ==tvDataColor_MAX)
        {
            returnResponse(false);
        }

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        if( !isCapablityCheckPassed( pqmode, source, format, "Component" )) {
            LOGERR("%s: CapablityCheck failed for ComponentLuma\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_ENABLE);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        if(ret != tvERROR_NONE) {
            LOGWARN("CMS enable failed\n");
            returnResponse(false);
        }
        else
        {
            int cms_params[3]={0};
            cms_params[0]=COLOR_STATE;
            cms_params[1]=COLOR_ENABLE;
            cms_params[2]=COMPONENT_ENABLE;
            int retval = UpdatePQParamsToCache("set","cms.enable",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_COMPONENT_LUMA,cms_params);
            if(retval != 0) {
                LOGWARN("Failed to Save enableflag to ssm_data\n");
            }
            LOGINFO("CMS Enable Success to localstore\n");
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetCurrentComponentLuma(blLumaColor, luma);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=COLOR_LUMA;
            params[1]=ConvertTVColorToVendorColor(blLumaColor);
            params[2]=luma;

            char tr181format[64]={0};
            snprintf(tr181format,sizeof(tr181format),"luma.%s",component_color[ConvertTVColorToVendorColor(blLumaColor)]);
            int retval=UpdatePQParamsToCache("set",tr181format,pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_COMPONENT_LUMA,params);

            if(retval != 0) {
                LOGWARN("Failed to Save component luma to ssm_data\n");
            }
            LOGINFO("Exit : SetComponentLuma() %s : %d\n",value.c_str(),params[2]);
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::resetComponentLuma(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t err = tvERROR_NONE;
        char param[BUFFER_SIZE]={0};

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        if( !isCapablityCheckPassed( pqmode, source, format, "Component" )) {
            LOGERR("%s: CapablityCheck failed for ComponentHue\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             err = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_DISABLE);
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        if(err == tvERROR_NONE)
        {
            int params[3]={0};
            params[0]=COLOR_STATE;
            params[1]=COLOR_ENABLE;
            int ret=UpdatePQParamsToCache("reset","cms.enable",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_COMPONENT_LUMA,params);
            if(ret != 0) {
                LOGWARN("Failed to Save enable flag to ssm_data\n");
            }

            for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
            {
                params[0]=COLOR_LUMA;
                params[1]=color;
                snprintf(param,sizeof(param),"luma.%s",component_color[color]);
                ret |= UpdatePQParamsToCache("reset",param,pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_COMPONENT_LUMA,params);
                memset(&param, 0, sizeof(param));
            }

            if(ret != 0)
            {
                LOGWARN("resetComponentLuma Failed couldn't remove from localstore error %d \n", ret);
                err=tvERROR_GENERAL;
            }
        }

        if(err != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetComponentLuma()\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getComponentCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject rangeInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(range,pqmode,source,format,"CMS");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["rangeInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
	std::string key;
	int sourceIndex=0,pqIndex=0,formatIndex=0;
        int dimmingMode = 0;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("DimmingMode",formatIndex,pqIndex,sourceIndex,dimmingMode, PQ_PARAM_LDIM);
        if( err == 0 ) {
            switch(dimmingMode) {
                case tvDimmingMode_Fixed:
                    LOGINFO("DimmingMode Value: Fixed\n");
                    response["DimmingMode"] = "fixed";
                    break;

		case tvDimmingMode_Local:
                    LOGINFO("DimmingMode Value: Local\n");
                    response["DimmingMode"] = "local";
                    break;

                case tvDimmingMode_Global:
                    LOGINFO("DimmingMode Value: Global\n");
                    response["DimmingMode"] = "global";
                    break;
                
            }
            LOGINFO("Exit : DimmingMode Value: %d \n", dimmingMode);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::setBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int dimmingMode = 0;
	tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("DimmingMode") ? parameters["DimmingMode"].String() : "";
        returnIfParamNotFound(parameters,"DimmingMode");

        dimmingMode = GetDimmingModeIndex(value.c_str());

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        if( !isCapablityCheckPassed( pqmode, source, format, "DimmingMode" )) {
            LOGERR("%s: CapablityCheck failed for DimmingMode\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetTVDimmingMode(value.c_str());
        }
        else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);


	if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set DimmingMode\n");
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=dimmingMode;
            int retval= UpdatePQParamsToCache("set","DimmingMode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_LDIM,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save DimmingMode to ssm_data\n");
            }

            //Update DimmingLevel to ssmdata
	    retval = SaveLocalDimmingLevelToDriverCache("set",pqmode.c_str(),source.c_str(),format.c_str(),params);
            if( retval != 0 ) {
                LOGWARN("Failed to Save DimmingLevel to ssmdata\n");
            }

            LOGINFO("Exit : setDimmingMode successful to value: %d\n", dimmingMode);
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::resetBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;std::string dimmingMode;
        int sourceIndex=0,pqIndex=0,formatIndex=0,dMode=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        if( !isCapablityCheckPassed( pqmode, source, format, "DimmingMode" )) {
            LOGERR("%s: CapablityCheck failed for DimmingMode\n", __FUNCTION__);
            returnResponse(false);
        }

	int retval= UpdatePQParamsToCache("reset","DimmingMode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_LDIM,params);

        if(retval != 0 ) {
            LOGWARN("Failed to reset ldim\n");
            returnResponse(false);
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("DimmingMode",formatIndex,pqIndex,sourceIndex,dMode, PQ_PARAM_DIMMINGMODE);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,dMode);
                if( isSetRequired(pqmode,source,format) ) {
                    LOGINFO("Proceed with %s\n",__FUNCTION__);
		    GetDimmingModeStringFromEnum(dMode,dimmingMode);
                    ret = SetTVDimmingMode(dimmingMode.c_str());
                }
                else
                    LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);
            }
            else
                LOGINFO("%s : GetLcoalParam Failed \n",__FUNCTION__);
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            //Update DimmingLevel to ssmdata
	    params[0] = dMode;
            retval = SaveLocalDimmingLevelToDriverCache("reset",pqmode.c_str(),source.c_str(),format.c_str(),params);
            if( retval != 0 ) {
                LOGWARN("Failed to Save DimmingLevel to ssmdata\n");
            }

            LOGINFO("Exit : resetBacklightDimmingMode Successful to value : %s \n",dimmingMode.c_str());
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getDimmingModeCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> supportedDimmingMode;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray supportedDimmingModeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject supportedDimmingModeInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(supportedDimmingMode,pqmode,source,format,"DimmingMode");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < supportedDimmingMode.size(); index++)
                supportedDimmingModeArray.Add(supportedDimmingMode[index]);

            response["supportedDimmingModeInfo"]=supportedDimmingModeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getAutoBacklightControl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvBacklightMode_t blMode;
        JsonArray supportedBLModes;
        int blModes = tvBacklightMode_NONE;
        tvError_t ret = GetSupportedBacklightModes(&blModes);
        if(ret != tvERROR_NONE) {
            supportedBLModes.Add("none");
        }
        else {
            if(!blModes)supportedBLModes.Add("none");
            if(blModes & tvBacklightMode_MANUAL)supportedBLModes.Add("manual");
            if(blModes & tvBacklightMode_AMBIENT)supportedBLModes.Add("ambient");
            if(blModes & tvBacklightMode_ECO)supportedBLModes.Add("eco");
        }

        JsonObject backlightCtrlObj;

        ret = GetCurrentBacklightMode(&blMode);
        if(ret != tvERROR_NONE) {
            response["mode"] = "none";
            response["supportedModes"] = supportedBLModes;
            returnResponse(false);
        }
	else {
            switch(blMode) {

                case tvBacklightMode_NONE:
                    LOGINFO("Auto Backlight Control Mode: none\n");
                    response["mode"] = "none";
                    break;

                case tvBacklightMode_MANUAL:
                    LOGINFO("Auto Backlight Control Mode: manual\n");
                    response["mode"] = "manual";
                    break;

                case tvBacklightMode_AMBIENT:
                    LOGINFO("Auto Backlight Control Mode: ambient\n");
                    response["mode"] = "ambient";
                    break;

                case tvBacklightMode_ECO:
                    LOGINFO("Auto Backlight Control Mode: eco\n");
                    response["mode"] = "eco";
                    break;

                default:
                    LOGINFO("Auto Backlight Control Mode: none\n");
                    response["mode"] = "none";
                    break;
            }

            response["supportedModes"] = supportedBLModes;
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::setAutoBacklightControl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        tvBacklightMode_t blMode = tvBacklightMode_NONE;
        std::string pqmode;
        std::string source;
        std::string format;

        value = parameters.HasLabel("mode") ? parameters["mode"].String() : "";
        returnIfParamNotFound(parameters,"mode");

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "global";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "global";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "global";

        if(!value.compare("none")) {
            blMode = tvBacklightMode_NONE;
        }
        else if (!value.compare("manual")){
            blMode = tvBacklightMode_MANUAL;
        }
        else if (!value.compare("ambient")){
            blMode = tvBacklightMode_AMBIENT;
        }
        else if (!value.compare("eco")){
            blMode = tvBacklightMode_ECO;
        }
        else {
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "AutoBacklightControl" )) {
            LOGERR("%s: CapablityCheck failed for AutoBacklightControl\n", __FUNCTION__);
            returnResponse(false);
        }

        tvError_t ret = SetCurrentBacklightMode(blMode);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, value.c_str());
            }
            LOGINFO("Exit : value :%s\n",value.c_str());
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::resetAutoBacklightControl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvError_t ret = tvERROR_NONE;
	std::string value;
        std::string pqmode;
        std::string source;
        std::string format;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "global";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "global";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "global";

	if( !isCapablityCheckPassed( pqmode, source, format, "AutoBacklightControl" )) {
            LOGERR("%s: CapablityCheck failed for AutoBacklightControl\n", __FUNCTION__);
            returnResponse(false);
        }

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));

            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM);

                tvBacklightMode_t blMode = tvBacklightMode_NONE;

                if(!std::string(param.value).compare("none")) {
                    blMode = tvBacklightMode_NONE;
                }
                else if (!std::string(param.value).compare("manual")){
                    blMode = tvBacklightMode_MANUAL;
                }
                else if (!std::string(param.value).compare("ambient")){
                    blMode = tvBacklightMode_AMBIENT;
                }
                else if (!std::string(param.value).compare("eco")){
                    blMode = tvBacklightMode_ECO;
                }
                else {
                    blMode = tvBacklightMode_NONE;
                }
                ret = SetCurrentBacklightMode(blMode);
                if(ret != tvERROR_NONE) {
                    LOGWARN("Autobacklight Mode set failed: %s\n",getErrorString(ret).c_str());
                }
                else {
                    LOGINFO("Exit : Autobacklight Mode set successfully, value: %s\n", param.value);
                }
            }
        }
	if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getAutoBacklightControlCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject AutoBacklightControlInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(range,pqmode,source,format,"AutoBacklightControl");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["AutoBacklightControlInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getSupportedDolbyVisionModes(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        pic_modes_t *dvModes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedDVModes(&dvModes,&totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            JsonArray SupportedDVModes;

            for(int count = 0;count <totalAvailable;count++ )
            {
                SupportedDVModes.Add(dvModes[count].name);
            }

            response["SupportedDVModes"] = SupportedDVModes;
            LOGINFO("Exit\n");
            returnResponse(true);
        }

    }

    uint32_t ControlSettingsTV::getDolbyVisionMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        TR181_ParamData_t param;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, &param);
        if (err!= tr181Success) {
            returnResponse(false);
        }
        else {
            std::string s;
            s+=param.value;
            response["DolbyVisionMode"] = s;
            LOGINFO("Exit getDolbyVisionMode(): %s\n",s.c_str());
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::setDolbyVisionMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;

	value = parameters.HasLabel("DolbyVisionMode") ? parameters["DolbyVisionMode"].String() : "";
        returnIfParamNotFound(parameters,"DolbyVisionMode");

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "dolby";

        if( !isCapablityCheckPassed( pqmode, source, format, "DolbyVisionMode" )) {
            LOGERR("%s: CapablityCheck failed for DolbyVisionMode\n", __FUNCTION__);
            returnResponse(false);
        }

        tvError_t ret = SetTVDolbyVisionMode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=GetDolbyModeIndex(value.c_str());
            int retval=UpdatePQParamsToCache("set","DolbyVisionMode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_DOLBY_MODE,params);
            if(retval != 0) {
                LOGWARN("Failed to Save DolbyVisionMode to ssm_data\n");
            }
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, value.c_str());
            }

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::resetDolbyVisionMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "dolby";

	if( !isCapablityCheckPassed( pqmode, source, format, "DolbyVisionMode" )) {
            LOGERR("%s: CapablityCheck failed for DolbyVisionMode\n", __FUNCTION__);
            returnResponse(false);
        }

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));
            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM);

                ret = SetTVDolbyVisionMode(param.value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("DV Mode set failed: %s\n",getErrorString(ret).c_str());
                }
                else {

                    LOGINFO("DV Mode initialized successfully value %s\n",param.value);
                    //Save DolbyVisionMode to ssm_data
                    int params[3]={0};
                    params[0]=GetDolbyModeIndex(param.value);
                    int retval=UpdatePQParamsToCache("reset","DolbyVisionMode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_DOLBY_MODE,params);

                    if(retval != 0) {
                        LOGWARN("Failed to Save DolbyVisionMode to ssm_data\n");
                        ret=tvERROR_GENERAL;
                    }
                }
            }
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetDolbyVisionMode() \n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getDolbyVisionModeCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject DolbyVisionModeInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(range,pqmode,source,format,"DolbyVisionMode");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["DolbyVisionModeInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getSupportedHDR10Modes(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        pic_modes_t *hdr10Modes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedHDR10Modes(&hdr10Modes,&totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            JsonArray SupportedHDR10Modes;

            for(int count = 0;count <totalAvailable;count++ )
            {
                SupportedHDR10Modes.Add(hdr10Modes[count].name);
            }

            response["SupportedHDR10Modes"] = SupportedHDR10Modes;
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::setHDR10Mode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;

        value = parameters.HasLabel("HDR10Mode") ? parameters["HDR10Mode"].String() : "";
        returnIfParamNotFound(parameters,"HDR10Mode");

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "hdr10";
        
	if( !isCapablityCheckPassed( pqmode, source, format, "HDR10Mode" )) {
            LOGERR("%s: CapablityCheck failed for HDR10Mode\n", __FUNCTION__);
            returnResponse(false);
        }

        tvError_t ret = SetTVHDR10Mode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=GetHDR10ModeIndex(value.c_str());
            int retval=UpdatePQParamsToCache("set","HDR10Mode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_HDR10_MODE,params);
            if(retval != 0) {
                LOGWARN("Failed to Save HDR10Mode to ssm_data\n");
            }
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_HDR10MODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_HDR10MODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_HDR10MODE_RFC_PARAM, value.c_str());
            }

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

      
    uint32_t ControlSettingsTV::getHDR10Mode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        TR181_ParamData_t param;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HDR10MODE_RFC_PARAM, &param);
        if (err!= tr181Success) {
            returnResponse(false);
        }
        else {
            std::string s;
            s+=param.value;
            response["HDR10Mode"] = s;
            LOGINFO("Exit getHDR10Mode(): %s\n",s.c_str());
            returnResponse(true);
        }

    }

    uint32_t ControlSettingsTV::resetHDR10Mode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "hdr10";

	if( !isCapablityCheckPassed( pqmode, source, format, "HDR10Mode" )) {
            LOGERR("%s: CapablityCheck failed for HDR10Mode\n", __FUNCTION__);
            returnResponse(false);
        }

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_HDR10MODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_HDR10MODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_HDR10MODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));
			tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HDR10MODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_HDR10MODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", TVSETTINGS_HDR10MODE_RFC_PARAM);

                ret = SetTVHDR10Mode(param.value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("DV Mode set failed: %s\n",getErrorString(ret).c_str());
                }
                else {

                    LOGINFO("DV Mode initialized successfully value %s\n",param.value);
                    //Save HDR10Mode to ssm_data
                    int params[3]={0};
                    params[0]=GetHDR10ModeIndex(param.value);
                    int retval=UpdatePQParamsToCache("reset","HDR10Mode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_HDR10_MODE,params);

                    if(retval != 0) {
                        LOGWARN("Failed to Save HDR10Mode to ssm_data\n");
                        ret=tvERROR_GENERAL;
                    }
                }
            }
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : %s \n",__FUNCTION__);
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getHDR10ModeCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject HDR10ModeInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(range,pqmode,source,format,"HDR10Mode");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["HDR10ModeInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }
    
    uint32_t ControlSettingsTV::getSupportedHLGModes(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        pic_modes_t *hlgModes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedHLGModes(&hlgModes,&totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            JsonArray SupportedHLGModes;

            for(int count = 0;count <totalAvailable;count++ )
            {
                SupportedHLGModes.Add(hlgModes[count].name);
            }

            response["SupportedHLGModes"] = SupportedHLGModes;
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getHLGMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        TR181_ParamData_t param;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HLGMODE_RFC_PARAM, &param);
        if (err!= tr181Success) {
            returnResponse(false);
        }
        else {
            std::string s;
            s+=param.value;
            response["HLGMode"] = s;
            LOGINFO("Exit getHLGMode(): %s\n",s.c_str());
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::setHLGMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;

        value = parameters.HasLabel("HLGMode") ? parameters["HLGMode"].String() : "";
        returnIfParamNotFound(parameters,"HLGMode");

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "hlg";

	if( !isCapablityCheckPassed( pqmode, source, format, "HLGMode" )) {
            LOGERR("%s: CapablityCheck failed for HLGMode\n", __FUNCTION__);
            returnResponse(false);
        }

        tvError_t ret = SetTVHLGMode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=GetHLGModeIndex(value.c_str());
            int retval=UpdatePQParamsToCache("set","HLGMode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_HLG_MODE,params);
            if(retval != 0) {
                LOGWARN("Failed to Save HLGMode to ssm_data\n");
            }
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_HLGMODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_HLGMODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_HLGMODE_RFC_PARAM, value.c_str());
            }

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::resetHLGMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "hlg";

	if( !isCapablityCheckPassed( pqmode, source, format, "HLGMode" )) {
            LOGERR("%s: CapablityCheck failed for HLGMode\n", __FUNCTION__);
            returnResponse(false);
        }

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_HLGMODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_HLGMODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_HLGMODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));
			tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HLGMODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_HLGMODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", TVSETTINGS_HLGMODE_RFC_PARAM);

                ret = SetTVHLGMode(param.value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("DV Mode set failed: %s\n",getErrorString(ret).c_str());
                }
                else {

                    LOGINFO("DV Mode initialized successfully value %s\n",param.value);
                    //Save HLGMode to ssm_data
                    int params[3]={0};
                    params[0]=GetHLGModeIndex(param.value);
                    int retval=UpdatePQParamsToCache("reset","HLGMode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_HLG_MODE,params);

                    if(retval != 0) {
                        LOGWARN("Failed to Save HLGMode to ssm_data\n");
                        ret=tvERROR_GENERAL;
                    }
                }
            }
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : %s \n",__FUNCTION__);
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getHLGModeCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject HLG10ModeInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(range,pqmode,source,format,"HLGMode");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["HLG10ModeInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getWBInfo(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        JsonArray supportedWBSelectors;
        JsonObject supportedWBSelectorInfo;
        JsonArray colorTemp;
        JsonArray inputs;
        JsonArray supportedWBCtrls;
        JsonObject supportedWBCtrlInfo;
        JsonObject gain;
        JsonObject offset;
        tvError_t ret;
        unsigned int index = 0;
        getWBInfo_t wbInfo;
        std::vector<std::string> wbSelector;
        std::vector<std::string> wbColorTmp;
        std::vector<std::string> wbInput;

        /* HAL call for getWBInfo */
        ret = getWbInfo(&wbInfo, wbSelector, wbColorTmp, wbInput);
        if(ret != tvERROR_NONE) {
            LOGWARN("getWbInfo() Failed!!! \n");
        }
        for (index = 0; index < wbSelector.size(); index++) {
            supportedWBSelectors.Add(wbSelector[index]);
        }
        response["supportedWBSelectors"] = supportedWBSelectors;

        for (index = 0; index < wbColorTmp.size(); index++) {
            colorTemp.Add(wbColorTmp[index]);
        }

        for (index = 0; index < wbInput.size(); index++) {
            inputs.Add(wbInput[index]);
        }
        if (wbColorTmp.size() != 0) {
            supportedWBSelectorInfo[wbSelector[0].c_str()] = colorTemp;
        }
        if (wbInput.size() != 0) {
            supportedWBSelectorInfo[wbSelector[1].c_str()] = inputs;
        }
        response["supportedWBSelectorInfo"] = supportedWBSelectorInfo;

        for (index = 0; index < 2; index++) {
            supportedWBCtrls.Add(wbInfo.wbControls[index]);
        }
        response["supportedWBCtrls"] = supportedWBCtrls;
        gain["min"] = std::string(wbInfo.wbGain[0]);
        gain["max"] = std::string(wbInfo.wbGain[1]);
        supportedWBCtrlInfo[wbInfo.wbControls[0]] = gain;
        offset["min"] = std::string(wbInfo.wbOffset[0]);
        offset["max"] = std::string(wbInfo.wbOffset[1]);
        supportedWBCtrlInfo[wbInfo.wbControls[1]] = offset;
        response["supportedWBCtrlInfo"] = supportedWBCtrlInfo;
        returnResponse(true);
    }

    uint32_t ControlSettingsTV::getWBCtrl(const JsonObject& parameters,JsonObject& response)
    {
        LOGINFO("Entry\n");
        JsonArray applies;
        JsonObject obj;
        std::vector<std::string> selector;
        std::vector<std::string> index;
        std::string color;
        std::string ctrl;
        int value;
        if ((parameters.HasLabel("applies")) && (parameters.HasLabel("color")) && (parameters.HasLabel("ctrl")))
        {
            color = parameters["color"].String();
            ctrl = parameters["ctrl"].String();
            applies = parameters["applies"].Array();
            for (int i = 0; i < applies.Length(); ++i)
            {
                obj = applies[i].Object();
                selector.push_back(obj["selector"].String());
                index.push_back(obj["index"].String());
            }
            std::string colorTemp;
            std::string inputSrc;
            int pos;
            std::string temp = "color temp";
            std::string input = "input";
            auto found = find(selector.begin(), selector.end(), temp);
            auto found1 = find(selector.begin(), selector.end(), input);
            if(found1 == selector.end()) {
                LOGINFO("Input source not passed");
                inputSrc = "";
            } else {
                pos = found1 - selector.begin();
                inputSrc = index.at(pos);
            }
			if(found != selector.end())
            {
                pos = found - selector.begin();
                colorTemp = index.at(pos);

                LOGINFO("input source : %s\ncolortemp : %s\ncolor : %s\nctrl : %s\n", inputSrc.c_str(), colorTemp.c_str(), color.c_str(), ctrl.c_str());
                // Call to HAL API to getWBctrl
                tvError_t ret = getWBctrl (const_cast <char *> (inputSrc.c_str()), const_cast <char *> (colorTemp.c_str()), const_cast <char *> (color.c_str()), const_cast <char *> (ctrl.c_str()), &value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("getWBCtrl failed");
                    returnResponse(false);
                } else {
                    LOGINFO("Exit : getWBCtrl success");
                    response["value"] = value;
                    returnResponse(true);
                }

            } else {
                returnResponse(false);
            }
        } else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::setWBCtrl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        JsonArray applies;
        JsonObject obj;
        std::vector<std::string> selector;
        std::vector<std::string> index;
        std::string val;
        std::string color;
        std::string ctrl;
        int value;
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "global";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "global";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "global";

        if( !isCapablityCheckPassed( pqmode, source, format, "WhiteBalance" )) {
            LOGERR("%s: CapablityCheck failed for WhiteBalance\n", __FUNCTION__);
            returnResponse(false);
        }

        if(parameters.HasLabel("applies") && parameters.HasLabel("color") && parameters.HasLabel("ctrl") && parameters.HasLabel("value"))
        {
            color = parameters["color"].String();
            ctrl = parameters["ctrl"].String();
            val = parameters["value"].String();
            value = std::stoi(val);
            applies = parameters["applies"].Array();
            for(int i=0; i<applies.Length(); ++i)
            {
                obj = applies[i].Object();
                selector.push_back(obj["selector"].String());
                index.push_back(obj["index"].String());
            }
            std::string temp = "color temp";
            std::string input = "input";
            std::string colorTemp;
            std::string inputSrc;
            int pos;
            auto found = find(selector.begin(), selector.end(), temp);
            auto found1 = find(selector.begin(), selector.end(), input);
            if(found1 == selector.end()) {
                LOGINFO("Input source not passed");
                inputSrc = "";
            } else {
                pos = found1 - selector.begin();
                inputSrc = index.at(pos);
            }
	    if(found != selector.end())
            {
                pos = found - selector.begin();
                colorTemp = index.at(pos);
                LOGINFO("input source : %s\ncolortemp : %s\ncolor : %s\nctrl : %s\nvalue : %d\n", inputSrc.c_str(), colorTemp.c_str(), color.c_str(), ctrl.c_str(), value);
                ret = setWBctrl (const_cast <char *> (inputSrc.c_str()),const_cast <char *> (colorTemp.c_str()), const_cast <char *> (color.c_str()), const_cast <char *> (ctrl.c_str()), value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("setWBCtrl failed");
                    returnResponse(false);
                } else {
                    //set it to local cache
                    std::string identifier = TVSETTINGS_GENERIC_STRING_RFC_PARAM;
                    identifier+=std::string("wb")+std::string(STRING_DIRTY)+color+"."+ctrl;
                    tr181ErrorCode_t err = setLocalParam(rfc_caller_id, identifier.c_str(), val.c_str());
                    if ( err != tr181Success ) {
                        LOGWARN("setLocalParam for %s Failed : %s\n", identifier.c_str(), getTR181ErrorString(err));
                    }
                    else {
                        LOGINFO("setLocalParam for %s Successful, Value: %d\n", identifier.c_str(), value);
                    }
                    LOGINFO("Exit : setWBCtrl success");
                    returnResponse(true);
                }
            } else {
                returnResponse(false);
            }
        }
        else{
            LOGINFO("Not a  valid parameter\n");
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::resetWBCtrl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvError_t ret = tvERROR_NONE;

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "global";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "global";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "global";

        if( !isCapablityCheckPassed( pqmode, source, format, "WhiteBalance" )) {
            LOGERR("%s: CapablityCheck failed for WhiteBalance\n", __FUNCTION__);
            returnResponse(false);
        }

        string identifier=(std::string(TVSETTINGS_GENERIC_STRING_RFC_PARAM)+std::string("wb."));

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,identifier.c_str());
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n",identifier.c_str(),getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }else{
            tvDataColor_t WBValues={0};
            //get the defaults
            ret = GetCustomWBValuesFromLocalCache(WBValues);
            if(ret == tvERROR_NONE)
            {
                //set the defaults and save
                ret = SyncCustomWBValuesToDriverCache(WBValues,true);
            }
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : Successfully reset user WB Entries \n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getSupportedPictureModes(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        pic_modes_t *pictureModes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedPictureModes(&pictureModes,&totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            JsonArray SupportedPicModes;

            for(int count = 0;count <totalAvailable;count++ )
            {
                SupportedPicModes.Add(pictureModes[count].name);
                // printf("Added Mode %s %s \n",pictureModes[count].name,SupportedPicModes[count].String().c_str());
            }

            response["SupportedPicmodes"] = SupportedPicModes;
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        char mode[PIC_MODE_NAME_MAX]={0};
        std::string picturemode;
        std::string source;
        int current_source = 0;
        std::string tr181_param_name;
        TR181_ParamData_t param = {0};

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";
  
        if (source = "current") {
            GetCurrentSource(&current_source);
        } else {
               current_source = GetTVSourceIndex(source.c_str());
        }

        tr181_param_name += std::string(TVSETTINGS_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
        tr181_param_name += "." + std::to_string(current_source) + "." + "PictureModeString";
         err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
        
         if ( tr181Success != err ) 
            returnResponse(false);
        }
        else {
            std::string s;
            s+=param.value;
            response["pictureMode"] = s;
	    LOGINFO("Exit : getPictureMode() : %s\n",s.c_str());
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::setPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string source;
        char prevmode[PIC_MODE_NAME_MAX]={0};
        GetTVPictureMode(prevmode);

        value = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        returnIfParamNotFound(parameters,"pictureMode");

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "all";

        tvError_t ret = SetTVPictureMode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            int source_index[SOURCES_SUPPORTED_MAX]={0};
	    int numberofsource = 0;
	    if (source == "all") {
                GetAllSupportedSourceIndex(source_index);
                numberofsource = sizeof(source_index)/sizeof(source_index[0]);
	    } else {
		    source_index[0] = GetTVSourceIndex(source.c_str());
		    numberofsource = 1;
	    }
            std::string tr181_param_name = "";

            tr181_param_name += std::string(TVSETTINGS_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
            for (int x = 0; x < numberofsource; x++ ) {

                // framing Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.ControlSettings.Source.source_index[x].PictureModeString.value
                tr181_param_name += "."+std::to_string(source_index[x])+"."+"PictureModeString";
                tr181ErrorCode_t err = setLocalParam(rfc_caller_id, tr181_param_name.c_str(), value.c_str());
                if ( err != tr181Success ) {
                    LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_SOURCE_PICTUREMODE_STRING_RFC_PARAM, getTR181ErrorString(err));
                }
                else {
                    LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_SOURCE_PICTUREMODE_STRING_RFC_PARAM, value.c_str());
		    //pqmodeindex = getPQmodeindex(value(strin))
		    int pqmodeindex = (int)GetTVPictureModeIndex(param.value);
                    SaveSourcePictureMode(pqmodeindex, source_index[x]);
		}
            }

	    //Filmmaker mode telemetry
            if(!strncmp(value.c_str(),"filmmaker",strlen(value.c_str())) && strncmp(prevmode,"filmmaker",strlen(prevmode)))
                LOGINFO("%s mode has been enabled",value.c_str());
            else if(!strncmp(prevmode,"filmmaker",strlen(prevmode)) && strncmp(value.c_str(),"filmmaker",strlen(value.c_str())))
                LOGINFO("%s mode has been disabled",prevmode);

            LOGINFO("Exit : Value : %s \n",value.c_str());
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::resetPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tr181ErrorCode_t err = tr181Success;
        
	int source_index[SOURCES_SUPPORTED_MAX]={0};
        int numberofsource = 0;
        GetAllSupportedSourceIndex(source_index);
        numberofsource = sizeof(source_index)/sizeof(source_index[0]);
        std::string tr181_param_name = "";
        tr181_param_name += std::string(TVSETTINGS_SOURCE_PICTUREMODE_STRING_RFC_PARAM);

        for (int x = 0; x < numberofsource; x++ ) {
	    tr181_param_name += "."+std::to_string(source_index[x])+"."+"PictureModeString";
       	    err = clearLocalParam(rfc_caller_id, tr181_param_name.c_str());
            if ( err != tr181Success ) {
                LOGWARN("clearLocalParam for %s Failed : %s\n", tr181_param_name.c_str(), getTR181ErrorString(err));
                returnResponse(false);
            }
            else {
                TR181_ParamData_t param = {0};
                LOGINFO("clearLocalParam for %s Successful\n", tr181_param_name.c_str());
                err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
                if ( tr181Success == err )
                {
                    LOGINFO("getLocalParam for %s is %s\n", TVSETTINGS_SOURCE_PICTUREMODE_STRING_RFC_PARAM, param.value);
                    //get curren source and if matches save for that alone
		    int current_source = 0;
		    GetCurrentSource(&current_source);
		    if (current_source == source_index[x]) {
                        tvError_t ret = SetTVPictureMode(param.value);
                        if(ret != tvERROR_NONE) {
                            LOGWARN("Picture Mode set failed: %s\n",getErrorString(ret).c_str());
                            returnResponse(false);
                        }
                        else {
                            LOGINFO("Exit : Picture Mode reset successfully, value: %s\n", param.value);
                        }
		    }
                }
                else {
                    LOGWARN("getLocalParam for %s failed\n", TVSETTINGS_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
                    returnResponse(false);
                }
            }
            int pqmodeindex = (int)GetTVPictureModeIndex((param.value).c_str());
            SaveSourcePictureMode(pqmodeindex, source_index[x]);
	}
	returnResponse(true;)
    }

    uint32_t ControlSettingsTV::enableWBMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();
        std::string value;
        tvError_t ret = tvERROR_NONE;
        bool mode = 0;
        if(parameters.HasLabel("mode")) {
            value = parameters["mode"].String();
            if(value == "true") {
                mode = 1;
            } else if(value == "false") {
                mode = 0;
            }
            ret = enableWBmode(mode);
            if(ret != tvERROR_NONE) {
                LOGERR("enableWBmode failed\n");
                returnResponse(false);
            }
            else{
                LOGINFO("enableWBmode successful... \n");
                returnResponse(true);
            }

        } else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::setBacklightFade(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string from,to,duration;
        int fromValue = 0,toValue = 0,durationValue = 0;

        from = parameters.HasLabel("from") ? parameters["from"].String() : "";
        if(from.empty())
            fromValue = 100;
        else
            fromValue = std::stoi(from);

        to = parameters.HasLabel("to") ? parameters["to"].String() : "";
        if(to.empty())
            toValue = 0;
        else
            toValue = std::stoi(to);

        duration = parameters.HasLabel("duration") ? parameters["duration"].String() : "";
        if(duration.empty())
            durationValue = 0;
        else
            durationValue = std::stoi(duration);

        LOGINFO("from = %d to = %d d = %d\n" ,fromValue,toValue,durationValue);
        tvError_t ret = SetBacklightFade(fromValue,toValue,durationValue);

        if(ret != tvERROR_NONE) {
           LOGWARN("Failed to set BacklightFade \n");
           returnResponse(false);
        }
        else {
           LOGINFO("Exit : backlightFade Success \n");
           returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::setLowLatencyState(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int lowLatencyIndex = 0;
        tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("LowLatencyState") ? parameters["LowLatencyState"].String() : "";
        returnIfParamNotFound(parameters,"LowLatencyState");
        lowLatencyIndex = stoi(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with setLowLatencyState\n");
             ret = SetLowLatencyState( lowLatencyIndex );
        }
		else
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to setLowLatency\n");
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=lowLatencyIndex;
            int retval= UpdatePQParamsToCache("set","LowLatencyState",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_LOWLATENCY_STATE,params);
            if(retval != 0 ) {
                LOGWARN("Failed to SaveLowLatency to ssm_data\n");
            }
            LOGINFO("Exit : setLowLatency successful to value: %d\n", lowLatencyIndex);
            returnResponse(true);
        }
    }
	
    uint32_t ControlSettingsTV::getLowLatencyState(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int lowlatencystate = 0;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam("Brightness",formatIndex,pqIndex,sourceIndex,lowlatencystate,PQ_PARAM_BRIGHTNESS);
        if( err == 0 ) {
            response["LowLatencyState"] = std::to_string(lowlatencystate);
            LOGINFO("Exit : LowLatencyState Value: %d \n", lowlatencystate);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t ControlSettingsTV::resetLowLatencyState(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,lowlatencystate=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        int retval= UpdatePQParamsToCache("reset","LowLatencyState",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_LOWLATENCY_STATE,params);
        if(retval != 0 ) {
            LOGWARN("Failed to clear Lowlatency from ssmdata and localstore\n");
            returnResponse(false);
        }
		else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("LowLatencyState",formatIndex,pqIndex,sourceIndex,lowlatencystate,PQ_PARAM_LOWLATENCY_STATE);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,lowlatencystate);
                ret = SetLowLatencyState(lowlatencystate);
            }
            else
                LOGINFO("%s : GetLcoalParam Failed \n",__FUNCTION__);
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetLowLatency Successful to value : %d \n",lowlatencystate);
            returnResponse(true);
        }
    }

    uint32_t ControlSettingsTV::getLowLatencyStateCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject LowLatencyInfo;
        JsonObject pqmodeInfo;
        JsonObject sourceInfo;
        JsonObject formatInfo;

        unsigned int index = 0;

        tvError_t ret = GetParamsCaps(range,pqmode,source,format,"LowLatencyState");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["LowLatencyInfo"]=rangeArray;

            for (index = 0; index < pqmode.size(); index++)
                pqmodeArray.Add(pqmode[index]);

            response["pqmodeInfo"]=pqmodeArray;

            for (index = 0; index < source.size(); index++)
                sourceArray.Add(source[index]);

            response["sourceInfo"]=sourceArray;

            for (index = 0; index < format.size(); index++)
                formatArray.Add(format[index]);

            response["formatInfo"]=formatArray;

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

}//namespace Plugin
}//namespace WPEFramework
