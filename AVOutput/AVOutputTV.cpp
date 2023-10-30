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
#include "AVOutputTV.h"

#define BUFFER_SIZE     (128)

#define registerMethod(...) for (uint8_t i = 1; GetHandler(i); i++) GetHandler(i)->Register<JsonObject, JsonObject>(__VA_ARGS__)

static char videoDescBuffer[VIDEO_DESCRIPTION_MAX*VIDEO_DESCRIPTION_NAME_SIZE] = {0};
static tvBacklightInfo_t  blInfo = {0};
static int numberModesSupported = 0;
static int numberSourcesSupported = 0;
static bool filmMakerMode= false;

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

    AVOutputTV* AVOutputTV::instance = nullptr;

    static void tvVideoFormatChangeHandler(tvVideoHDRFormat_t format, void *userData)
    {
        LOGINFO("tvVideoFormatChangeHandler format:%d \n",format);
        AVOutputTV *obj = (AVOutputTV *)userData;
        if(obj)obj->NotifyVideoFormatChange(format);
    }

    static void tvFilmMakerModeChangeHandler(tvContentType_t mode, void *userData)
    {
        LOGINFO("tvFilmMakerModeChangeHandler content:%d \n",mode);
        AVOutputTV *obj = (AVOutputTV *)userData;
        if(obj)obj->NotifyFilmMakerModeChange(mode);
    }

    static void tvVideoResolutionChangeHandler(tvResolutionParam_t resolution, void *userData)
    {
        LOGINFO("tvVideoResolutionChangeHandler resolution:%d\n",resolution.resolutionValue);
        AVOutputTV *obj = (AVOutputTV *)userData;
        if(obj)obj->NotifyVideoResolutionChange(resolution);
    }

    static void tvVideoFrameRateChangeHandler(tvVideoFrameRate_t frameRate, void *userData)
    {
        LOGINFO("tvVideoFrameRateChangeHandler format:%d \n",frameRate);
        AVOutputTV *obj = (AVOutputTV *)userData;
        if(obj)obj->NotifyVideoFrameRateChange(frameRate);
    }

    static const char *getVideoContentTypeToString(tvContentType_t content)
    {
        const char *strValue = "NONE";
        switch(content) {
            case tvContentType_FMM:
                LOGINFO("Content Type: FMM\n");
                strValue = "true";
                break;
            default:
                LOGINFO("Content Type: NONE\n");
                strValue = "false";
                break;
        } 
        return strValue;
    }

    static const char *getVideoFormatTypeToString(tvVideoHDRFormat_t format)
    {
        const char *strValue = "NONE";
        switch(format) {
            case tvVideoHDRFormat_SDR:
                LOGINFO("Video Format: SDR\n");
                strValue = "SDR";
                break;
            case tvVideoHDRFormat_HDR10:
                LOGINFO("Video Format: HDR10\n");
                strValue = "HDR10";
                break;
            case tvVideoHDRFormat_HDR10PLUS:
                LOGINFO("Video Format: HDR10PLUS\n");
                strValue = "HDR10PLUS";
                break;
            case tvVideoHDRFormat_HLG:
                LOGINFO("Video Format: HLG\n");
                strValue = "HLG";
                break;
            case tvVideoHDRFormat_DV:
                LOGINFO("Video Format: DV\n");
                strValue = "DV";
                break;
            default:
                LOGINFO("Video Format:: NONE\n");
                strValue = "NONE";
                break;
        }
        return strValue;
    }

    static JsonArray getAvailableVideoFormat(void)
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
            LOGWARN("%s : Reading supported video format failed %d", __FUNCTION__, ret);
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
        LOGINFO("Video Resolution:[%s]\n", strValue.c_str());
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
            LOGWARN("%s : Reading supported video resolutions failed %d", __FUNCTION__, ret);
        }

        return supportedResolution;
    }


    static const char *getVideoFrameRateTypeToString(tvVideoFrameRate_t frameRate)
    {
       const char *strValue = "NONE";
       switch(frameRate) {
            case tvVideoFrameRate_24:
                LOGINFO("Video FrameRate: 24\n");
                strValue = "24";
                break;
            case tvVideoFrameRate_25:
                LOGINFO("Video FrameRate: 25\n");
                strValue = "25";
                break;
            case tvVideoFrameRate_30:
                LOGINFO("Video FrameRate: 30\n");
                strValue = "30";
                break;
            case tvVideoFrameRate_50:
                LOGINFO("Video FrameRate: 50\n");
                strValue = "50";
                break;
            case tvVideoFrameRate_60:
                LOGINFO("Video FrameRate: 60\n");
                strValue = "60";
                break;
            case tvVideoFrameRate_23dot98:
                LOGINFO("Video FrameRate: 23.98\n");
                strValue = "23.98";
                break;
            case tvVideoFrameRate_29dot97:
                LOGINFO("Video FrameRate: 29.97\n");
                strValue = "29.97";
                break;
            case tvVideoFrameRate_59dot94:
                LOGINFO("Video FrameRate: 59.94\n");
                strValue = "59.94";
                break;
            default:
                LOGINFO("Video FrameRate: NONE\n");
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
            LOGWARN("%s : Reading supported video frame rate failed %d", __FUNCTION__, ret);
        }

        return supportedFrameRate;
    }

    void AVOutputTV::NotifyVideoFormatChange(tvVideoHDRFormat_t format)
    {
        JsonObject response;
        response["currentVideoFormat"] = getVideoFormatTypeToString(format);
        response["supportedVideoFormat"] = getAvailableVideoFormat();
        sendNotify("videoFormatChanged", response);
    }

    void AVOutputTV::NotifyFilmMakerModeChange(tvContentType_t mode)
    {
        JsonObject response;
	std::string fmmMode;
	fmmMode = getVideoContentTypeToString(mode);
        response["filmMakerMode"] = fmmMode;

        if (fmmMode.compare("true") == 0 ) {
	    filmMakerMode = true;
	}
	else
	{
	    filmMakerMode = false;
	}
        sendNotify("filmMakerModeChanged", response);
    }

    uint32_t AVOutputTV::getFilmMakerMode(const JsonObject & parameters, JsonObject & response)
    {
	response["filmMakerMode"] = filmMakerMode;
	returnResponse(true);
    }
	
    void AVOutputTV::NotifyVideoResolutionChange(tvResolutionParam_t resolution)
    {
        JsonObject response;
        response["currentVideoResolution"] = getVideoResolutionTypeToString(resolution);
        response["supportedVideoResolution"] = getSupportedVideoResolution();
        sendNotify("videoResolutionChanged", response);
    }

    void AVOutputTV::NotifyVideoFrameRateChange(tvVideoFrameRate_t frameRate)
    {
        JsonObject response;
        response["currentVideoFrameRate"] = getVideoFrameRateTypeToString(frameRate);
        response["supportedVideoFrameRate"] = getSupportedVideoFrameRate();
        sendNotify("videoFrameRateChanged", response);
    }

    std::string AVOutputTV::getErrorString (tvError_t eReturn)
    {
        switch (eReturn)
        {
            case tvERROR_NONE:
                return "API SUCCESS";
            case tvERROR_GENERAL:
                return "API FAILED";
            case tvERROR_OPERATION_NOT_SUPPORTED:
                return "OPERATION NOT SUPPORTED ERROR";
            case tvERROR_INVALID_PARAM:
                return "INVALID PARAM ERROR";
            case tvERROR_INVALID_STATE:
                return "INVALID STATE ERROR";
         }
         return "UNKNOWN ERROR";
    }

    JsonArray AVOutputTV::getSupportedVideoSource(void) {
        JsonArray supportedSource;
        int lCount = 0;
        for(;lCount<numberSourcesSupported;lCount++) {
            supportedSource.Add(convertSourceIndexToString(source_index[lCount]));
        }

        return supportedSource;
    }

    bool AVOutputTV::isBacklightUsingGlobalBacklightFactor(void)
    {
        TR181_ParamData_t param;
        bool ret  =false;

        memset(&param, 0, sizeof(param));

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, AVOUTPUT_BACKLIGHT_CONTROL_USE_GBF_RFC_PARAM,&param);
        if ( err != tr181Success ) {
            LOGWARN("getLocalParam for %s Failed : %s\n", AVOUTPUT_BACKLIGHT_CONTROL_USE_GBF_RFC_PARAM, getTR181ErrorString(err));
        }
        else {
            if(!std::string(param.value).compare("true"))
            {
                ret = true;
            }
        }
        return ret;
    }

    AVOutputTV::AVOutputTV(): PluginHost::JSONRPC()
					    , m_currentHdmiInResoluton (dsVIDEO_PIXELRES_1920x1080)
                                            , m_videoZoomMode (tvDisplayMode_NORMAL)
                                            , m_isDisabledHdmiIn4KZoom (false)
					    , rfc_caller_id()
    {
        LOGINFO("Entry\n");
        AVOutputTV::instance = this;
	CreateHandler({ 2 });
        registerMethod("getBacklight", &AVOutputTV::getBacklight, this);
        registerMethod("setBacklight", &AVOutputTV::setBacklight, this);
        registerMethod("resetBacklight", &AVOutputTV::resetBacklight, this);
        registerMethod("getBacklightCaps", &AVOutputTV::getBacklightCaps, this);
        registerMethod("getBrightnessCaps", &AVOutputTV::getBrightnessCaps, this);
        registerMethod("getBrightness", &AVOutputTV::getBrightness, this);
        registerMethod("setBrightness", &AVOutputTV::setBrightness, this);
        registerMethod("resetBrightness", &AVOutputTV::resetBrightness, this);
        registerMethod("getContrast", &AVOutputTV::getContrast, this);
        registerMethod("setContrast", &AVOutputTV::setContrast, this);
        registerMethod("resetContrast", &AVOutputTV::resetContrast, this);
	registerMethod("getContrastCaps", &AVOutputTV::getContrastCaps, this);
        registerMethod("getSharpness", &AVOutputTV::getSharpness, this);
        registerMethod("setSharpness", &AVOutputTV::setSharpness, this);
        registerMethod("resetSharpness", &AVOutputTV::resetSharpness, this);
	registerMethod("getSharpnessCaps", &AVOutputTV::getSharpnessCaps, this);
        registerMethod("getSaturation", &AVOutputTV::getSaturation, this);
        registerMethod("setSaturation", &AVOutputTV::setSaturation, this);
        registerMethod("resetSaturation", &AVOutputTV::resetSaturation, this);
	registerMethod("getSaturationCaps", &AVOutputTV::getSaturationCaps, this);
        registerMethod("getHue", &AVOutputTV::getHue, this);
        registerMethod("setHue", &AVOutputTV::setHue, this);
        registerMethod("resetHue", &AVOutputTV::resetHue, this);
	registerMethod("getHueCaps", &AVOutputTV::getHueCaps, this);
        registerMethod("getColorTemperature", &AVOutputTV::getColorTemperature, this);
        registerMethod("setColorTemperature", &AVOutputTV::setColorTemperature, this);
        registerMethod("resetColorTemperature", &AVOutputTV::resetColorTemperature, this);
	registerMethod("getColorTemperatureCaps", &AVOutputTV::getColorTemperatureCaps, this);

        registerMethod("getComponentCaps", &AVOutputTV::getComponentCaps, this);
        registerMethod("getComponentSaturation", &AVOutputTV::getComponentSaturation, this);
        registerMethod("setComponentSaturation", &AVOutputTV::setComponentSaturation, this);
        registerMethod("resetComponentSaturation", &AVOutputTV::resetComponentSaturation, this);
        registerMethod("getComponentHue", &AVOutputTV::getComponentHue, this);
        registerMethod("setComponentHue", &AVOutputTV::setComponentHue, this);
        registerMethod("resetComponentHue", &AVOutputTV::resetComponentHue, this);
        registerMethod("getComponentLuma", &AVOutputTV::getComponentLuma, this);
        registerMethod("setComponentLuma", &AVOutputTV::setComponentLuma, this);
        registerMethod("resetComponentLuma", &AVOutputTV::resetComponentLuma, this);
        registerMethod("getBacklightDimmingMode", &AVOutputTV::getBacklightDimmingMode, this);
        registerMethod("setBacklightDimmingMode", &AVOutputTV::setBacklightDimmingMode, this);
        registerMethod("resetBacklightDimmingMode", &AVOutputTV::resetBacklightDimmingMode, this);
        registerMethod("getBacklightDimmingModeCaps", &AVOutputTV::getBacklightDimmingModeCaps, this);
        registerMethod("getAutoBacklightControl", &AVOutputTV::getAutoBacklightControl, this);
        registerMethod("setAutoBacklightControl", &AVOutputTV::setAutoBacklightControl, this);
        registerMethod("resetAutoBacklightControl", &AVOutputTV::resetAutoBacklightControl, this);
        registerMethod("getAutoBacklightControlCaps", &AVOutputTV::getAutoBacklightControlCaps, this);

        registerMethod("getSupportedDolbyVisionModes", &AVOutputTV::getSupportedDolbyVisionModes, this);
        registerMethod("getDolbyVisionMode", &AVOutputTV::getDolbyVisionMode, this);
        registerMethod("setDolbyVisionMode", &AVOutputTV::setDolbyVisionMode, this);
        registerMethod("resetDolbyVisionMode", &AVOutputTV::resetDolbyVisionMode, this);
        registerMethod("getDolbyVisionModeCaps", &AVOutputTV::getDolbyVisionModeCaps, this);
        registerMethod("getSupportedHLGModes", &AVOutputTV::getSupportedHLGModes, this);
        registerMethod("getHLGMode", &AVOutputTV::getHLGMode, this);
        registerMethod("setHLGMode", &AVOutputTV::setHLGMode, this);
        registerMethod("resetHLGMode", &AVOutputTV::resetHLGMode, this);
	registerMethod("getHLGModeCaps", &AVOutputTV::getHLGModeCaps, this);
        registerMethod("getSupportedHDR10Modes", &AVOutputTV::getSupportedHDR10Modes, this);
	registerMethod("getHDR10Mode", &AVOutputTV::getHDR10Mode, this);
	registerMethod("setHDR10Mode", &AVOutputTV::setHDR10Mode, this);
	registerMethod("resetHDR10Mode", &AVOutputTV::resetHDR10Mode, this);
	registerMethod("getHDR10ModeCaps", &AVOutputTV::getHDR10ModeCaps, this);
	registerMethod("getVideoFormat", &AVOutputTV::getVideoFormat, this);
	registerMethod("getVideoSource", &AVOutputTV::getVideoSource, this);
	registerMethod("getVideoFrameRate", &AVOutputTV::getVideoFrameRate, this);
	registerMethod("getVideoResolution", &AVOutputTV::getVideoResolution, this);
	registerMethod("getFilmMakerMode", &AVOutputTV::getFilmMakerMode, this);
	
	registerMethod("getWBInfo", &AVOutputTV::getWBInfo, this);
	registerMethod("getWBCtrl", &AVOutputTV::getWBCtrl, this);
	registerMethod("setWBCtrl", &AVOutputTV::setWBCtrl, this);
	registerMethod("resetWBCtrl", &AVOutputTV::resetWBCtrl, this);

	registerMethod("getZoomMode", &AVOutputTV::getZoomMode, this);
        registerMethod("setZoomMode", &AVOutputTV::setZoomMode, this);
        registerMethod("resetZoomMode", &AVOutputTV::resetZoomMode, this);
        registerMethod("getZoomModeCaps", &AVOutputTV::getZoomModeCaps, this);


	registerMethod("getPictureMode", &AVOutputTV::getPictureMode, this);
        registerMethod("setPictureMode", &AVOutputTV::setPictureMode, this);
	registerMethod("resetPictureMode", &AVOutputTV::resetPictureMode, this);
	registerMethod("getPictureModeCaps", &AVOutputTV::getPictureModeCaps, this);
        registerMethod("getSupportedPictureModes", &AVOutputTV::getSupportedPictureModes, this);
        registerMethod("getVideoSourceCaps", &AVOutputTV::getVideoSourceCaps, this);
        registerMethod("getVideoFormatCaps", &AVOutputTV::getVideoFormatCaps, this);
        registerMethod("getVideoFrameRateCaps", &AVOutputTV::getVideoFrameRateCaps, this);
        registerMethod("getVideoResolutionCaps", &AVOutputTV::getVideoResolutionCaps, this);
	registerMethod("enableWBMode", &AVOutputTV::enableWBMode, this);

	registerMethod("setBacklightFade", &AVOutputTV::setBacklightFade, this);

	registerMethod("getLowLatencyState", &AVOutputTV::getLowLatencyState, this);
        registerMethod("setLowLatencyState", &AVOutputTV::setLowLatencyState, this);
        registerMethod("resetLowLatencyState", &AVOutputTV::resetLowLatencyState, this);
        registerMethod("getLowLatencyStateCaps", &AVOutputTV::getLowLatencyStateCaps, this);

        LOGINFO("Exit\n");
    }
    
    AVOutputTV :: ~AVOutputTV()
    {
        LOGINFO();    
    }

    void AVOutputTV::Initialize()
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
            LOGWARN("AVOutputPlugins: getCurrentVideoModeObj failed");
        }
        LOGWARN("AVOutputPlugins: AVOutput Initialize m_currentHdmiInResoluton:%d m_mod:%d", m_currentHdmiInResoluton, m_videoZoomMode);

        ret = tvInit();

        if(ret != tvERROR_NONE) {
            LOGERR("Platform Init failed, ret: %s \n", getErrorString(ret).c_str());
	} else {
            LOGINFO("Platform Init successful...\n");
            ret = tvSD3toCriSyncInit();
            if(ret != tvERROR_NONE) {
                LOGERR(" SD3 <->cri_data sync failed, ret: %s \n", getErrorString(ret).c_str());
            }
            else {
                LOGERR(" SD3 <->cri_data sync success, ret: %s \n", getErrorString(ret).c_str());
            }
        }

        tvVideoFormatCallbackData callbackData = {this,tvVideoFormatChangeHandler};
        RegisterVideoFormatChangeCB(callbackData);

        tvVideoContentCallbackData ConcallbackData = {this,tvFilmMakerModeChangeHandler};
        RegisterVideoContentChangeCB(ConcallbackData);

        tvVideoResolutionCallbackData RescallbackData = {this,tvVideoResolutionChangeHandler};
        RegisterVideoResolutionChangeCB(RescallbackData);

        tvVideoFrameRateCallbackData FpscallbackData = {this,tvVideoFrameRateChangeHandler};
        RegisterVideoFrameRateChangeCB(FpscallbackData);

	appUsesGlobalBackLightFactor = isBacklightUsingGlobalBacklightFactor();

        LocatePQSettingsFile();

	GetAllSupportedPicModeIndex(pic_mode_index);

        GetAllSupportedSourceIndex(source_index);

	//Get number of pqmode supported
        numberModesSupported=GetNumberOfModesupported();

	//Get number of source supported
	numberSourcesSupported = GetNumberOfSourcesSupported( );

        SyncPQParamsToDriverCache("global","global","global");

        // source format specific sync to ssm data
        SyncSourceFormatPicModeToCache("current", "global", "global");//Tamil--need to move it to SyncPQPAramsToDriverCache

       if(appUsesGlobalBackLightFactor)
       {
           if(!InitializeSDRHDRBacklight())
               LOGINFO("%s:InitializeSDRHDRBacklight() : Success\n",__FUNCTION__);
           else
               LOGWARN("%s:InitializeSDRHDRBacklight() : Failed\n",__FUNCTION__);
       }	

        // As we have source to picture mode mapping, get current source and
        // setting those picture mode
        int current_source = 0;
        int current_format = getContentFormatIndex(GetCurrentContentFormat());
        std::string tr181_param_name = "";
        // get current source
        GetCurrentSource(&current_source);

       tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
       tr181_param_name += "."+std::to_string(current_source)+"."+"Format."+std::to_string(current_format)+"."+"PictureModeString";

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
       err = getLocalParam(rfc_caller_id, AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM, &param);
       if ( tr181Success == err )
       {
           LOGINFO("getLocalParam for %s is %s\n", AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM, param.value);

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
            LOGWARN("getLocalParam for %s Failed : %s\n", AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
        }
        setDefaultAspectRatio();

        LOGINFO("Exit\n" );
    }

    void AVOutputTV::Deinitialize()
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
    void AVOutputTV::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        LOGINFO("Entry %s\n",__FUNCTION__);
        LOGINFO("Exit %s\n",__FUNCTION__);
    }

    void AVOutputTV::dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if(!AVOutputTV::instance)
            return;

        if (IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS == eventId)
        {
            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            int hdmi_in_port = eventData->data.hdmi_in_status.port;
            bool hdmi_in_status = eventData->data.hdmi_in_status.isPresented;
            LOGWARN("AVOutputPlugins: Received IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS  event  port: %d, started: %d", hdmi_in_port,hdmi_in_status);
            if (!hdmi_in_status){
                tvError_t ret = tvERROR_NONE;
                AVOutputTV::instance->m_isDisabledHdmiIn4KZoom = false;
                LOGWARN("AVOutputPlugins: Hdmi streaming stopped here reapply the global zoom settings:%d here. m_isDisabledHdmiIn4KZoom: %d", AVOutputTV::instance->m_videoZoomMode, AVOutputTV::instance->m_isDisabledHdmiIn4KZoom);
                ret = SetAspectRatio((tvDisplayMode_t)AVOutputTV::instance->m_videoZoomMode);
                if (ret != tvERROR_NONE) {
                    LOGWARN("SetAspectRatio set Failed");
                }
            }else {
                AVOutputTV::instance->m_isDisabledHdmiIn4KZoom = true;
                LOGWARN("AVOutputPlugins: m_isDisabledHdmiIn4KZoom: %d", AVOutputTV::instance->m_isDisabledHdmiIn4KZoom);
            }
        }
    }

    void AVOutputTV::dsHdmiVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if(!AVOutputTV::instance)
            return;

        if (IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE == eventId)
        {
            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            int hdmi_in_port = eventData->data.hdmi_in_video_mode.port;
            dsVideoPortResolution_t resolution;
            AVOutputTV::instance->m_currentHdmiInResoluton = eventData->data.hdmi_in_video_mode.resolution.pixelResolution;
            resolution.pixelResolution =  eventData->data.hdmi_in_video_mode.resolution.pixelResolution;
            resolution.interlaced =  eventData->data.hdmi_in_video_mode.resolution.interlaced;
            resolution.frameRate =  eventData->data.hdmi_in_video_mode.resolution.frameRate;
            LOGWARN("AVOutputPlugins: Received IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE  event  port: %d, pixelResolution: %d, interlaced : %d, frameRate: %d \n", hdmi_in_port,resolution.pixelResolution, resolution.interlaced, resolution.frameRate);
            if (AVOutputTV::instance->m_isDisabledHdmiIn4KZoom) {
                tvError_t ret = tvERROR_NONE;
                if (AVOutputTV::instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
                    (dsVIDEO_PIXELRES_MAX == AVOutputTV::instance->m_currentHdmiInResoluton)){
                    LOGWARN("AVOutputPlugins: Setting %d zoom mode for below 4K", AVOutputTV::instance->m_videoZoomMode);
                    ret = SetAspectRatio((tvDisplayMode_t)AVOutputTV::instance->m_videoZoomMode);
                }else {
                    LOGWARN("AVOutputPlugins: Setting auto zoom mode for 4K and above");
                    ret = SetAspectRatio(tvDisplayMode_AUTO);
                }
                if (ret != tvERROR_NONE) {
                    LOGWARN("SetAspectRatio set Failed");
                }
            } else {
                LOGWARN("AVOutputPlugins: %s: HdmiInput is not started yet. m_isDisabledHdmiIn4KZoom: %d", __FUNCTION__, AVOutputTV::instance->m_isDisabledHdmiIn4KZoom);
            }
        }
    }
    
    uint32_t AVOutputTV::getZoomModeCaps(const JsonObject& parameters, JsonObject& response)
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

        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"AspectRatio");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
		else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["options"]=rangeArray;

            if (pqmode.front().compare("none") != 0) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::setZoomMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        tvDisplayMode_t mode = tvDisplayMode_16x9;
        std::string pqmode;
        std::string source;
        std::string format;


        value = parameters.HasLabel("zoomMode") ? parameters["zoomMode"].String() : "";
        returnIfParamNotFound(parameters,"zoomMode");

        if (parsingSetInputArgument(parameters, "AspectRatio",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

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
            int retval=updatePQParamsToCache("set","AspectRatio",pqmode,source,format,PQ_PARAM_ASPECT_RATIO,params);;

            if(retval != 0) {
                LOGWARN("Failed to Save DisplayMode to ssm_data\n");
            }

            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, AVOUTPUT_ASPECTRATIO_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, value.c_str());
            }
            LOGINFO("Exit : SetAspectRatio2() value : %s\n",value.c_str());
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getZoomMode(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        tvDisplayMode_t mode;

        tvError_t ret = getUserSelectedAspectRatio (&mode);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            switch(mode) {

                case tvDisplayMode_16x9:
                    LOGINFO("Aspect Ratio: TV 16X9 STRETCH\n");
                    response["zoomMode"] = "TV 16X9 STRETCH";
                    break;

                case tvDisplayMode_4x3:
                    LOGINFO("Aspect Ratio: TV 4X3 PILLARBOX\n");
                    response["zoomMode"] = "TV 4X3 PILLARBOX";
                    break;

                case tvDisplayMode_NORMAL:
                    LOGINFO("Aspect Ratio: TV Normal\n");
                    response["zoomMode"] = "TV NORMAL";
                    break;

                case tvDisplayMode_AUTO:
                    LOGINFO("Aspect Ratio: TV AUTO\n");
                    response["zoomMode"] = "TV AUTO";
                    break;

                case tvDisplayMode_DIRECT:
                    LOGINFO("Aspect Ratio: TV DIRECT\n");
                    response["zoomMode"] = "TV DIRECT";
                    break;

                case tvDisplayMode_ZOOM:
                    LOGINFO("Aspect Ratio: TV ZOOM\n");
                    response["zoomMode"] = "TV ZOOM";
                    break;

                default:
                    LOGINFO("Aspect Ratio: TV AUTO\n");
                    response["zoomMode"] = "TV AUTO";
                    break;
            }
            returnResponse(true);
        }
    }

    tvError_t AVOutputTV::setAspectRatioZoomSettings(tvDisplayMode_t mode)
    {
        tvError_t ret = tvERROR_GENERAL;
        LOGERR("tvmgrplugin: %s mode selected is: %d", __FUNCTION__, m_videoZoomMode);
#if !defined (HDMIIN_4K_ZOOM)
        if (AVOutputTV::instance->m_isDisabledHdmiIn4KZoom) {
            if (AVOutputTV::instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
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
            LOGWARN("tvmgrplugin: %s: HdmiInput is not started yet. m_isDisabledHdmiIn4KZoom: %d", __FUNCTION__, AVOutputTV::instance->m_isDisabledHdmiIn4KZoom);
            ret = SetAspectRatio((tvDisplayMode_t)m_videoZoomMode);
        }
#endif
        return ret;
    }

    tvError_t AVOutputTV::getUserSelectedAspectRatio (tvDisplayMode_t* mode)
    {
        tvError_t ret = tvERROR_GENERAL;
#if !defined (HDMIIN_4K_ZOOM)
        LOGERR("controlsettingsplugin: %s mode selected is: %d", __FUNCTION__, m_videoZoomMode);
        if (AVOutputTV::instance->m_isDisabledHdmiIn4KZoom) {
            if (!(AVOutputTV::instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
               (dsVIDEO_PIXELRES_MAX == AVOutputTV::instance->m_currentHdmiInResoluton))){
                *mode = (tvDisplayMode_t)AVOutputTV::instance->m_videoZoomMode;
                LOGWARN("tvmgrplugin: %s: Getting zoom mode %d for display, for 4K and above", __FUNCTION__, *mode);
                return tvERROR_NONE;
            }
        }
#endif
        ret = GetAspectRatio(mode);
        return ret;
    }

    uint32_t AVOutputTV::resetZoomMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "AspectRatio",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

	if( !isCapablityCheckPassed( pqmode, source, format, "AspectRatio" )) {
            LOGERR("%s: CapablityCheck failed for AspectRatio\n", __FUNCTION__);
            returnResponse(false);
        }

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,AVOUTPUT_ASPECTRATIO_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM);
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

    tvError_t AVOutputTV::setDefaultAspectRatio(std::string pqmode,std::string  format,std::string source)
    {
        tvDisplayMode_t mode = tvDisplayMode_MAX;
        TR181_ParamData_t param;
        tvError_t ret = tvERROR_NONE;

        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, AVOUTPUT_ASPECTRATIO_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            LOGINFO("getLocalParam for %s is %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, param.value);

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
                int retval=updatePQParamsToCache("set","AspectRatio",pqmode,source,format,PQ_PARAM_ASPECT_RATIO,params);

                if(retval != 0) {
                    LOGWARN("Failed to Save DisplayMode to ssm_data\n");
                }
                LOGINFO("Aspect Ratio initialized successfully, value: %s\n", param.value);
            }

        }
	else
        {
            LOGWARN("getLocalParam for %s Failed : %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            ret = tvERROR_GENERAL;
        }
        return ret;
    }

    uint32_t AVOutputTV::getVideoFormat(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        //PLUGIN_Lock(Lock);
        tvVideoHDRFormat_t videoFormat;
        tvError_t ret = GetCurrentVideoFormat(&videoFormat);
        //response["supportedVideoFormat"] = getSupportedVideoFormat();
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

    uint32_t AVOutputTV::getVideoResolution(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        //PLUGIN_Lock(Lock);
        tvResolutionParam_t videoResolution;
        tvError_t ret = GetVideoResolution(&videoResolution);
        //response["supportedVideoResolution"] = getSupportedVideoResolution();
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

    uint32_t AVOutputTV::getVideoFrameRate(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvVideoFrameRate_t videoFramerate;
        tvError_t ret = GetVideoFrameRate(&videoFramerate);
        //response["supportedFrameRate"] = getSupportedVideoFrameRate();
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

    uint32_t AVOutputTV::getBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        
	int sourceIndex=0,pqIndex=0,formatIndex=0;
        int backlight = 0,err = 0;

        if (parsingGetInputArgument(parameters, "Backlight",source, pqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
        
        if (isPlatformSupport("Backlight") != 0) returnResponse(false);
#if 0
        if( !isCapablityCheckPassed( pqmode, source, format, "Backlight" )) {
            LOGERR("%s: CapablityCheck failed for Backlight\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        err = getLocalparam("Backlight",formatIndex,pqIndex,sourceIndex,backlight, PQ_PARAM_BACKLIGHT);
        if( err == 0 ) {
            response["backlight"] = backlight;
            LOGINFO("Exit : Backlight Value: %d \n", backlight);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t AVOutputTV::setBacklight(const JsonObject& parameters, JsonObject& response)
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

        if (parsingSetInputArgument(parameters, "Backlight",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("Backlight") != 0 ) returnResponse(false);

	if( !isCapablityCheckPassed( pqmode, source, format, "Backlight" )) {
            LOGERR("%s: CapablityCheck failed for Backlight\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) { //Needs rework
            LOGINFO("Proceed with setBacklight\n");
            ret = SetBacklight(backlight);
        }

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set Backlight\n");
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=backlight;
            int retval= updatePQParamsToCache("set","Backlight",pqmode,source,format,PQ_PARAM_BACKLIGHT,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Backlight to ssm_data\n");
            }
            LOGINFO("Exit : setBacklight successful to value: %d\n", backlight);
            returnResponse(true);
        }

    }
    uint32_t AVOutputTV::resetBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,backlight=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "Backlight",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("Backlight") != 0) returnResponse(false);

        if( !isCapablityCheckPassed( pqmode, source, format, "Backlight" )) {
            LOGERR("%s: CapablityCheck failed for Backlight\n", __FUNCTION__);
            returnResponse(false);
        }

	int retval= updatePQParamsToCache("reset","Backlight",pqmode,source,format,PQ_PARAM_BACKLIGHT,params);
        if(retval != 0 )
	{
            LOGWARN("Failed to reset Backlight\n");
            returnResponse(false);
        }
        else
	{
	    if (isSetRequired(pqmode,source,format))
	    {
                getParamIndex("current","current", "current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("Backlight",formatIndex,pqIndex,sourceIndex,backlight, PQ_PARAM_BACKLIGHT);
                if( err == 0 )
		{
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,backlight);
                    ret = SetBacklight(backlight);
                }
                else
		{
                    LOGINFO("%s : GetLocalParam Failed \n",__FUNCTION__);
	        }
	    }
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

    uint32_t AVOutputTV::getBacklightCaps(const JsonObject& parameters, JsonObject& response)
     {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;
  
        std::string isPlatformSupport;

	JsonObject rangeObj;
        //JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"Backlight", isPlatformSupport);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
	    response["platformSupport"] = (isPlatformSupport.compare("true") == 0)  ? true : false;

            rangeObj["from"] = stoi(range[0]);
            rangeObj["to"] = stoi(range[1]);
            response["rangeInfo"]=rangeObj;

            if ((pqmode.front()).compare("none") != 0) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getBrightness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int brightness = 0;

        if (parsingGetInputArgument(parameters, "Brightness",source, pqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
#if 0
        if( !isCapablityCheckPassed( pqmode, source, format, "Brightness" )) {
            LOGERR("%s: CapablityCheck failed for Brightness\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Brightness",formatIndex,pqIndex,sourceIndex,brightness, PQ_PARAM_BRIGHTNESS);
        if( err == 0 ) {
            response["brightness"] = brightness;
            LOGINFO("Exit : Brightness Value: %d \n", brightness);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t AVOutputTV::setBrightness(const JsonObject& parameters, JsonObject& response)
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

        if (parsingSetInputArgument(parameters, "Brightness",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

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
            int retval= updatePQParamsToCache("set","Brightness",pqmode,source,format,PQ_PARAM_BRIGHTNESS,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Brightness to ssm_data\n");
            }
            LOGINFO("Exit : setBrightness successful to value: %d\n", brightness);
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::resetBrightness(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,brightness=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "Brightness",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

	if( !isCapablityCheckPassed( pqmode, source, format, "Brightness" )) {
            LOGERR("%s: CapablityCheck failed for Brightness\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updatePQParamsToCache("reset","Brightness",pqmode,source,format,PQ_PARAM_BRIGHTNESS,params);
        if(retval != 0 )
        {
            LOGWARN("Failed to reset Brightness\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("current","current", "current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("Brightness",formatIndex,pqIndex,sourceIndex,brightness, PQ_PARAM_BRIGHTNESS);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,brightness);
                    ret = SetBrightness(brightness);
                }
                else
                {
                    LOGINFO("%s : GetLocalParam Failed \n",__FUNCTION__);
                }
            }
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

    uint32_t AVOutputTV::getBrightnessCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
	std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;
        
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;
        JsonObject rangeObj;

	unsigned int index = 0;

	tvError_t ret = getParamsCaps(range,pqmode,source,format,"Brightness");

	if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            rangeObj["from"] = stoi(range[0]);
            rangeObj["to"] = stoi(range[1]);
            response["rangeInfo"]=rangeObj;

            if ((pqmode.front()).compare("none") != 0) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
	        response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
	}
    }

    uint32_t AVOutputTV::getContrast(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int contrast = 0;

        if (parsingGetInputArgument(parameters, "Contrast",source, pqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
#if 0
        if( !isCapablityCheckPassed( pqmode, source, format, "Contrast" )) {
            LOGERR("%s: CapablityCheck failed for Contrast\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Contrast",formatIndex,pqIndex,sourceIndex,contrast, PQ_PARAM_CONTRAST);
        if( err == 0 ) {
            response["contrast"] = contrast;
            LOGINFO("Exit : Contrast Value: %d \n", contrast);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t AVOutputTV::setContrast(const JsonObject& parameters, JsonObject& response)
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

        if (parsingSetInputArgument(parameters, "Contrast",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

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
            int retval= updatePQParamsToCache("set","Contrast",pqmode,source,format,PQ_PARAM_CONTRAST,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Contrast to ssm_data\n");
            }
            LOGINFO("Exit : setContrast successful to value: %d\n", contrast);
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::resetContrast(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,contrast=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "Contrast",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

	if( !isCapablityCheckPassed( pqmode, source, format, "Contrast" )) {
            LOGERR("%s: CapablityCheck failed for Contrast\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updatePQParamsToCache("reset","Contrast",pqmode,source,format,PQ_PARAM_CONTRAST,params);

        if(retval != 0 )
        {
            LOGWARN("Failed to reset Contrast\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("current","current", "current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("Contrast",formatIndex,pqIndex,sourceIndex,contrast, PQ_PARAM_CONTRAST);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,contrast);
                    ret = SetContrast(contrast);
                }
                else
                {
                    LOGINFO("%s : GetLocalParam Failed \n",__FUNCTION__);
                }
            }
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

    uint32_t AVOutputTV::getContrastCaps(const JsonObject& parameters, JsonObject& response)
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

	JsonObject rangeObj;
        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"Contrast");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            rangeObj["from"] = stoi(range[0]);
            rangeObj["to"] = stoi(range[1]);
            response["rangeInfo"]=rangeObj;
            
            if ((pqmode.front()).compare("none") != 0) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int saturation = 0;

        if (parsingGetInputArgument(parameters, "Saturation",source, pqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
#if 0
        if( !isCapablityCheckPassed( pqmode, source, format, "Saturation" )) {
            LOGERR("%s: CapablityCheck failed for Saturation\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Saturation",formatIndex,pqIndex,sourceIndex,saturation, PQ_PARAM_SATURATION);
        if( err == 0 ) {
            response["saturation"] = saturation;
            LOGINFO("Exit : Saturation Value: %d \n", saturation);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t AVOutputTV::setSaturation(const JsonObject& parameters, JsonObject& response)
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

        if (parsingSetInputArgument(parameters, "Saturation",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }
 
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
            int retval= updatePQParamsToCache("set","Saturation",pqmode,source,format,PQ_PARAM_SATURATION,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Saturation to ssm_data\n");
            }
            LOGINFO("Exit : setSaturation successful to value: %d\n", saturation);
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::resetSaturation(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,saturation=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "Saturation",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "Saturation" )) {
            LOGERR("%s: CapablityCheck failed for Saturation\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updatePQParamsToCache("reset","Saturation",pqmode,source,format,PQ_PARAM_SATURATION,params);

        if(retval != 0 )
        {
            LOGWARN("Failed to reset Saturation\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("current","current", "current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("Saturation",formatIndex,pqIndex,sourceIndex, saturation, PQ_PARAM_SATURATION);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,saturation);
                    ret = SetSaturation(saturation);
                }
                else
                {
                    LOGINFO("%s : GetLocalParam Failed \n",__FUNCTION__);
                }
            }
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

    uint32_t AVOutputTV::getSaturationCaps(const JsonObject& parameters, JsonObject& response)
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

        unsigned int index = 0;
        JsonObject rangeObj;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"Saturation");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            rangeObj["from"] = stoi(range[0]);
            rangeObj["to"] = stoi(range[1]);
            response["rangeInfo"]=rangeObj;

            
	    if ((pqmode.front()).compare("none") != 0) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
	    if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
	    if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getSharpness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int sharpness = 0;

        if (parsingGetInputArgument(parameters, "Sharpness",source, pqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
#if 0
        if( !isCapablityCheckPassed( pqmode, source, format, "Sharpness" )) {
            LOGERR("%s: CapablityCheck failed for Sharpness\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Sharpness",formatIndex,pqIndex,sourceIndex,sharpness, PQ_PARAM_SHARPNESS);
        if( err == 0 ) {
            response["sharpness"] = sharpness;
            LOGINFO("Exit : Sharpness Value: %d \n", sharpness);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t AVOutputTV::setSharpness(const JsonObject& parameters, JsonObject& response)
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

        if (parsingSetInputArgument(parameters, "Sharpness",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }
 
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
            int retval= updatePQParamsToCache("set","Sharpness",pqmode,source,format,PQ_PARAM_SHARPNESS,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Sharpness to ssm_data\n");
            }
            LOGINFO("Exit : setSharpness successful to value: %d\n", sharpness);
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::resetSharpness(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,sharpness=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "Sharpness",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "Sharpness" )) {
            LOGERR("%s: CapablityCheck failed for Sharpness\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updatePQParamsToCache("reset","Sharpness",pqmode,source,format,PQ_PARAM_SHARPNESS,params);

        if(retval != 0 )
        {
            LOGWARN("Failed to reset Sharpness\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("current","current", "current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("Sharpness",formatIndex,pqIndex,sourceIndex, sharpness, PQ_PARAM_SHARPNESS);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,sharpness);
                    ret = SetSharpness(sharpness);
                }
                else
                {
                    LOGINFO("%s : GetLocalParam Failed \n",__FUNCTION__);
                }
            }
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

    uint32_t AVOutputTV::getSharpnessCaps(const JsonObject& parameters, JsonObject& response)
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

        JsonObject rangeObj;
        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"Sharpness");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            rangeObj["from"] = stoi(range[0]);
            rangeObj["to"] = stoi(range[1]);
            response["rangeInfo"]=rangeObj;
 
	    if ((pqmode.front()).compare("none") != 0) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
	    if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
	    if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int hue = 0;

        if (parsingGetInputArgument(parameters, "Hue",source, pqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
#if 0
        if( !isCapablityCheckPassed( pqmode, source, format, "Hue" )) {
            LOGERR("%s: CapablityCheck failed for Hue\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Hue",formatIndex,pqIndex,sourceIndex,hue, PQ_PARAM_HUE);
        if( err == 0 ) {
            response["hue"] = hue;
            LOGINFO("Exit : Hue Value: %d \n", hue);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t AVOutputTV::setHue(const JsonObject& parameters, JsonObject& response)
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

        if (parsingSetInputArgument(parameters, "Hue",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

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
            int retval= updatePQParamsToCache("set","Hue",pqmode,source,format,PQ_PARAM_HUE,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Hue to ssm_data\n");
            }
            LOGINFO("Exit : setHue successful to value: %d\n", hue);
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::resetHue(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,hue=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "Hue",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

	if( !isCapablityCheckPassed( pqmode, source, format, "Hue" )) {
            LOGERR("%s: CapablityCheck failed for Hue\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updatePQParamsToCache("reset","Hue",pqmode,source,format,PQ_PARAM_HUE,params);

        if(retval != 0 )
        {
            LOGWARN("Failed to reset Hue\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("current","current", "current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("Hue",formatIndex,pqIndex,sourceIndex, hue, PQ_PARAM_HUE);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,hue);
                    ret = SetHue(hue);
                }
                else
                {
                    LOGINFO("%s : GetLocalParam Failed \n",__FUNCTION__);
                }
            }
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

    uint32_t AVOutputTV::getHueCaps(const JsonObject& parameters, JsonObject& response)
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

        JsonObject rangeObj;
        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"Hue");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            rangeObj["from"] = stoi(range[0]);
            rangeObj["to"] = stoi(range[1]);
            response["rangeInfo"]=rangeObj; 

            if ((pqmode.front()).compare("none") != 0) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getColorTemperature(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int colortemp = 0;

        if (parsingGetInputArgument(parameters, "ColorTemperature",source, pqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
#if 0
        if( !isCapablityCheckPassed( pqmode, source, format, "ColorTemperature" )) {
            LOGERR("%s: CapablityCheck failed for Hue\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("ColorTemp",formatIndex,pqIndex,sourceIndex,colortemp,PQ_PARAM_COLOR_TEMPERATURE);
        if( err == 0 ) {
            switch(colortemp) {

                case tvColorTemp_STANDARD:
                    LOGINFO("Color Temp Value: Standard\n");
                    response["colorTemperature"] = "Standard";
                    break;

                case tvColorTemp_WARM:
                    LOGINFO("Color Temp Value: Warm\n");
                    response["colorTemperature"] = "Warm";
                    break;

                case tvColorTemp_COLD:
                    LOGINFO("Color Temp Value: Cold\n");
                    response["colorTemperature"] = "Cold";
                    break;

                case tvColorTemp_USER:
                    LOGINFO("Color Temp Value: User Defined\n");
                    response["colorTemperature"] = "User Defined";
                    break;

                default:
                    LOGINFO("Color Temp Value: Standard\n");
                    response["colorTemperature"] = "Standard";
                    break;
            }
            LOGINFO("Exit : ColorTemperature Value: %d \n", colortemp);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t AVOutputTV::setColorTemperature(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int colortemp = 0;
        tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("colorTemperature") ? parameters["colorTemperature"].String() : "";
        returnIfParamNotFound(parameters,"colorTemperature");
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

        if (parsingSetInputArgument(parameters, "ColorTemperature",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

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
            int retval= updatePQParamsToCache("set","ColorTemp",pqmode,source,format,PQ_PARAM_COLOR_TEMPERATURE,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save ColorTemperature to ssm_data\n");
            }
            LOGINFO("Exit : setColorTemperature successful to value: %d\n", colortemp);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::resetColorTemperature(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,colortemp=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "ColorTemperature",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "ColorTemperature" )) {
            LOGERR("%s: CapablityCheck failed for colorTemperature\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updatePQParamsToCache("reset","ColorTemp",pqmode,source,format,PQ_PARAM_COLOR_TEMPERATURE,params);

        if(retval != 0 )
        {
            LOGWARN("Failed to reset ColorTemperature\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("current","current", "current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("ColorTemp",formatIndex,pqIndex,sourceIndex, colortemp, PQ_PARAM_COLOR_TEMPERATURE);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex, colortemp);
                    ret = SetColorTemperature((tvColorTemp_t)colortemp);
                }
                else
                {
                    LOGINFO("%s : GetLocalParam Failed \n",__FUNCTION__);
                }
            }
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

    uint32_t AVOutputTV::getColorTemperatureCaps(const JsonObject& parameters, JsonObject& response)
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

        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"ColorTemperature");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
		else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["options"]=rangeArray;

            if (((pqmode.front()).compare("none") != 0)) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getComponentSaturation(const JsonObject& parameters, JsonObject& response)
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

	value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(parameters,"color");

        if (parsingGetInputArgument(parameters, "ComponentSaturation",source, pqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
#if 0
        if( !isCapablityCheckPassed( pqmode, source, format, "ComponentSaturation" )) {
            LOGERR("%s: CapablityCheck failed for ComponentSaturation\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        blSaturationColor =  getComponentColorEnum(value);
        if(blSaturationColor ==tvDataColor_MAX)
        {
            returnResponse(false);
        }
        cms += value;

        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

	int err = getLocalparam(cms.c_str(),formatIndex,pqIndex,sourceIndex,saturation,
			        PQ_PARAM_COMPONENT_SATURATION);
	if( err == 0 || err == 1 ) {//err value willbe 1 if cms is default.(SPECIAL case)
            response["saturation"] = saturation;
            LOGINFO("Exit : Component Saturation for color: %s Value: %d\n", value.c_str(),saturation);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }

    }

    uint32_t AVOutputTV::setComponentSaturation(const JsonObject& parameters, JsonObject& response)
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

	blSaturationColor =  getComponentColorEnum(value);
        if(blSaturationColor ==tvDataColor_MAX)
        {
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "ComponentSaturation",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }
 
        if( !isCapablityCheckPassed( pqmode, source, format, "ComponentSaturation" )) {
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
            int retval = updatePQParamsToCache("set","cms.enable",pqmode,source,format,PQ_PARAM_COMPONENT_SATURATION,cms_params);
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
            int retval=updatePQParamsToCache("set",tr181format,pqmode,source,format,PQ_PARAM_COMPONENT_SATURATION,params);

            if(retval != 0) {
                LOGWARN("Failed to Save component saturation to ssm_data\n");
            }
            LOGINFO("Exit : SetComponentSaturation() %s : %s\n",format.c_str(),value.c_str());
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::resetComponentSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t err = tvERROR_NONE;
	char param[BUFFER_SIZE]={0};

        if (parsingSetInputArgument(parameters, "ComponentSaturation",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "ComponentSaturation" )) {
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
            int ret=updatePQParamsToCache("reset","cms.enable",pqmode,source,format,PQ_PARAM_COMPONENT_SATURATION,params);
            if(ret != 0) {
                LOGWARN("Failed to Save enable flag to ssm_data\n");
            }

            for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
            {
                params[0]=COLOR_SATURATION;
                params[1]=color;
                snprintf(param,sizeof(param),"saturation.%s",component_color[color]);
                ret |= updatePQParamsToCache("reset",param,pqmode,source,format,PQ_PARAM_COMPONENT_SATURATION,params);
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


    uint32_t AVOutputTV::getComponentHue(const JsonObject& parameters, JsonObject& response)
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

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(parameters,"color");

        if (parsingGetInputArgument(parameters, "ComponentHue",source, pqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
#if 0
        if( !isCapablityCheckPassed( pqmode, source, format, "ComponentHue" )) {
            LOGERR("%s: CapablityCheck failed for ComponentHue\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        blHueColor =  getComponentColorEnum(value);
        if(blHueColor ==tvDataColor_MAX)
        {
            returnResponse(false);
        }
        cms += value;

        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam(cms.c_str(),formatIndex,pqIndex,sourceIndex,hue,PQ_PARAM_COMPONENT_HUE );
        if( err == 0 || err == 1 ) {//err value willbe 1 if cms is default.(SPECIAL case)
            response["hue"] = hue;
            LOGINFO("Exit : Component Hue for color: %s Value: %d\n", value.c_str(),hue);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }


    uint32_t AVOutputTV::setComponentHue(const JsonObject& parameters, JsonObject& response)
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

        blHueColor =  getComponentColorEnum(value);
        if(blHueColor ==tvDataColor_MAX)
        {
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "ComponentHue",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

	if( !isCapablityCheckPassed( pqmode, source, format, "ComponentHue" )) {
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
            int retval = updatePQParamsToCache("set","cms.enable",pqmode,source,format,PQ_PARAM_COMPONENT_HUE,cms_params);
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
            int retval=updatePQParamsToCache("set",tr181format,pqmode,source,format,PQ_PARAM_COMPONENT_HUE,params);

            if(retval != 0) {
                LOGWARN("Failed to Save component hue to ssm_data\n");
            }
            LOGINFO("Exit : SetComponentHue() %s : %s\n",format.c_str(),value.c_str());
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::resetComponentHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t err = tvERROR_NONE;
        char param[BUFFER_SIZE]={0};

        if (parsingSetInputArgument(parameters, "ComponentHue",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

	if( !isCapablityCheckPassed( pqmode, source, format, "ComponentHue" )) {
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
            int ret=updatePQParamsToCache("reset","cms.enable",pqmode,source,format,PQ_PARAM_COMPONENT_HUE,params);
            if(ret != 0) {
                LOGWARN("Failed to Save enable flag to ssm_data\n");
            }

            for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
            {
                params[0]=COLOR_HUE;
                params[1]=color;
                snprintf(param,sizeof(param),"hue.%s",component_color[color]);
                ret |= updatePQParamsToCache("reset",param,pqmode,source,format,PQ_PARAM_COMPONENT_HUE,params);
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

    uint32_t AVOutputTV::getComponentLuma(const JsonObject& parameters, JsonObject& response)
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

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(parameters,"color");

        if (parsingGetInputArgument(parameters, "ComponentLuma",source, pqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
#if 0
        if( !isCapablityCheckPassed( pqmode, source, format, "ComponentLuma" )) {
            LOGERR("%s: CapablityCheck failed for ComponentLuma\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        blLumaColor =  getComponentColorEnum(value);
        if(blLumaColor ==tvDataColor_MAX)
        {
            returnResponse(false);
        }
        cms += value;

        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam(cms.c_str(),formatIndex,pqIndex,sourceIndex,luma,PQ_PARAM_COMPONENT_LUMA);
        if( err == 0 || err == 1 ) {//err value willbe 1 if cms is default.(SPECIAL case)
            response["luma"] = luma;
            LOGINFO("Exit : Component Luma for color: %s Value: %d\n", value.c_str(),luma);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }
    
    uint32_t AVOutputTV::setComponentLuma(const JsonObject& parameters, JsonObject& response)
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

        blLumaColor =  getComponentColorEnum(value);
        if(blLumaColor ==tvDataColor_MAX)
        {
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "ComponentLuma",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "ComponentLuma" )) {
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
            int retval = updatePQParamsToCache("set","cms.enable",pqmode,source,format,PQ_PARAM_COMPONENT_LUMA,cms_params);
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
            int retval=updatePQParamsToCache("set",tr181format,pqmode,source,format,PQ_PARAM_COMPONENT_LUMA,params);

            if(retval != 0) {
                LOGWARN("Failed to Save component luma to ssm_data\n");
            }
            LOGINFO("Exit : SetComponentLuma() %s : %d\n",value.c_str(),params[2]);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::resetComponentLuma(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t err = tvERROR_NONE;
        char param[BUFFER_SIZE]={0};

        if (parsingSetInputArgument(parameters, "ComponentHue",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "ComponentHue" )) {
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
            int ret=updatePQParamsToCache("reset","cms.enable",pqmode,source,format,PQ_PARAM_COMPONENT_LUMA,params);
            if(ret != 0) {
                LOGWARN("Failed to Save enable flag to ssm_data\n");
            }

            for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
            {
                params[0]=COLOR_LUMA;
                params[1]=color;
                snprintf(param,sizeof(param),"luma.%s",component_color[color]);
                ret |= updatePQParamsToCache("reset",param,pqmode,source,format,PQ_PARAM_COMPONENT_LUMA,params);
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

    uint32_t AVOutputTV::getComponentCaps(const JsonObject& parameters, JsonObject& response)
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

        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"CMS");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(stoi(range[index]));

            response["rangeInfo"]=rangeArray;

            if (((pqmode.front()).compare("none") != 0)) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if (((source.front()).compare("none") != 0)) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if (((format.front()).compare("none") != 0)) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
	std::string key;
	int sourceIndex=0,pqIndex=0,formatIndex=0;
        int dimmingMode = 0;

        if (parsingGetInputArgument(parameters, "DimmingMode",source, pqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
#if 0
        if( !isCapablityCheckPassed( pqmode, source, format, "DimmingMode" )) {
            LOGERR("%s: CapablityCheck failed for DimmingMode\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }


        int err = getLocalparam("DimmingMode",formatIndex,pqIndex,sourceIndex,dimmingMode, PQ_PARAM_DIMMINGMODE);
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

    uint32_t AVOutputTV::setBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
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

        dimmingMode = getDimmingModeIndex(value);

        if (parsingSetInputArgument(parameters, "DimmingMode",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

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
            int retval= updatePQParamsToCache("set","DimmingMode",pqmode,source,format,PQ_PARAM_DIMMINGMODE,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save DimmingMode to ssm_data\n");
            }

            //Update DimmingLevel to ssmdata
	    retval = saveLocalDimmingLevelToDriverCache("set",pqmode,source,format,params);
            if( retval != 0 ) {
                LOGWARN("Failed to Save DimmingLevel to ssmdata\n");
            }

            LOGINFO("Exit : setDimmingMode successful to value: %d\n", dimmingMode);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::resetBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;std::string dimmingMode;
        int sourceIndex=0,pqIndex=0,formatIndex=0,dMode=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "DimmingMode",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "DimmingMode" )) {
            LOGERR("%s: CapablityCheck failed for DimmingMode\n", __FUNCTION__);
            returnResponse(false);
        }

	int retval= updatePQParamsToCache("reset","DimmingMode",pqmode,source,format,PQ_PARAM_DIMMINGMODE,params);

        if(retval != 0 )
        {
            LOGWARN("Failed to reset ldim\n");
            returnResponse(false);
        }

        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("current","current", "current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("DimmingMode",formatIndex,pqIndex,sourceIndex, dMode, PQ_PARAM_DIMMINGMODE);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex, dMode);
                    getDimmingModeStringFromEnum(dMode,dimmingMode);
                    ret = SetTVDimmingMode(dimmingMode.c_str());
                }
                else
                {
                    LOGINFO("%s : GetLocalParam Failed \n",__FUNCTION__);
                }
            }
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            //Update DimmingLevel to ssmdata
	    params[0] = dMode;
            retval = saveLocalDimmingLevelToDriverCache("reset",pqmode,source,format,params);
            if( retval != 0 ) {
                LOGWARN("Failed to Save DimmingLevel to ssmdata\n");
            }

            LOGINFO("Exit : resetBacklightDimmingMode Successful to value : %s \n",dimmingMode.c_str());
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getBacklightDimmingModeCaps(const JsonObject& parameters, JsonObject& response)
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

        unsigned int index = 0;

        tvError_t ret = getParamsCaps(supportedDimmingMode,pqmode,source,format,"DimmingMode");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < supportedDimmingMode.size(); index++)
                supportedDimmingModeArray.Add(supportedDimmingMode[index]);

            response["options"]=supportedDimmingModeArray;
             
            if (((pqmode.front()).compare("none") != 0)) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getAutoBacklightControl(const JsonObject& parameters, JsonObject& response)
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

    uint32_t AVOutputTV::setAutoBacklightControl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        tvBacklightMode_t blMode = tvBacklightMode_NONE;
        std::string pqmode;
        std::string source;
        std::string format;

        value = parameters.HasLabel("mode") ? parameters["mode"].String() : "";
        returnIfParamNotFound(parameters,"mode");

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

        if (parsingSetInputArgument(parameters, "AutoBacklightControl",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
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
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM, value.c_str());
            }
            LOGINFO("Exit : value :%s\n",value.c_str());
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::resetAutoBacklightControl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvError_t ret = tvERROR_NONE;
	std::string value;
        std::string pqmode;
        std::string source;
        std::string format;

        if (parsingSetInputArgument(parameters, "AutoBacklightControl",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

	if( !isCapablityCheckPassed( pqmode, source, format, "AutoBacklightControl" )) {
            LOGERR("%s: CapablityCheck failed for AutoBacklightControl\n", __FUNCTION__);
            returnResponse(false);
        }

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));

            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM);

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

    uint32_t AVOutputTV::getAutoBacklightControlCaps(const JsonObject& parameters, JsonObject& response)
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

        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"AutoBacklightControl");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["options"]=rangeArray;

            if ((pqmode.front()).compare("none") != 0) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getSupportedDolbyVisionModes(const JsonObject& parameters, JsonObject& response)
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

            response["supportedDVModes"] = SupportedDVModes;
            LOGINFO("Exit\n");
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::getDolbyVisionMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::string pqmode;
        std::string source;
        std::string format;
        int dolbyMode = 0;
        int err = 0;

        int sourceIndex=0,pqIndex=0,formatIndex=0;
         
        if (parsingGetInputArgument(parameters, "DolbyVisionMode",source, pqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("DolbyVisionMode") != 0) returnResponse(false);
#if 0
        if( !isCapablityCheckPassed( pqmode, source, format, "DolbyVisionMode" )) {
            LOGERR("%s: CapablityCheck failed for DolbyVisionMode\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        // Since it is dolby vision mode, to should get only for dolby vision format
        format = "dv";

        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        err = getLocalparam("DolbyVisionMode",formatIndex,pqIndex,sourceIndex,dolbyMode, PQ_PARAM_DOLBY_MODE);
        if( err == 0 ) {
            response["dolbyVisionMode"] = getDolbyModeStringFromEnum((tvDolbyMode_t)dolbyMode);
            LOGINFO("Exit : DolbyVisionMode Value: %d \n", dolbyMode);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }

    }

    uint32_t AVOutputTV::setDolbyVisionMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t ret  = tvERROR_NONE;

        value = parameters.HasLabel("dolbyVisionMode") ? parameters["dolbyVisionMode"].String() : "";
        returnIfParamNotFound(parameters,"dolbyVisionMode");

        if (parsingSetInputArgument(parameters, "DolbyVisionMode",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("DolbyVisionMode") != 0) returnResponse(false);

        if( !isCapablityCheckPassed( pqmode, source, format, "DolbyVisionMode" )) {
            LOGERR("%s: CapablityCheck failed for DolbyVisionMode\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired("current",source,"dv") ) {
            LOGINFO("Proceed with setDolbyVisionMode\n\n");
            ret = SetTVDolbyVisionMode(value.c_str());
        }

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set DolbyVisionMode\n\n");
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=GetDolbyModeIndex(value.c_str());
            format = "dv";
            int retval= updatePQParamsToCache("set","DolbyVisionMode",pqmode,source,format,PQ_PARAM_DOLBY_MODE,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Dolbyvision mode\n");
            }
            LOGINFO("Exit : Dolbyvision successful to value: %s\n", value.c_str());
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::resetDolbyVisionMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,dolbyMode=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "DolbyVisionMode",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("DolbyVisionMode") != 0) returnResponse(false);

        if( !isCapablityCheckPassed( pqmode, source, format, "DolbyVisionMode" )) {
            LOGERR("%s: CapablityCheck failed for DolbyVisionMode\n", __FUNCTION__);
            returnResponse(false);
        }
        
        format = "dv";
        int retval= updatePQParamsToCache("reset","DolbyVisionMode",pqmode,source,format,PQ_PARAM_DOLBY_MODE,params);
        if(retval != 0 )
        {
            LOGWARN("Failed to reset DolbyVisionMode\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired("current",source,format))
            {
                getParamIndex("current","current", format,sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("DolbyVisionMode",formatIndex,pqIndex,sourceIndex, dolbyMode, PQ_PARAM_DOLBY_MODE);
                if( err == 0 )
                {
                    std::string dolbyModeValue = getDolbyModeStringFromEnum((tvDolbyMode_t)dolbyMode);
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d dolbyvalue : %s\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex, dolbyModeValue.c_str());
                    ret = SetTVDolbyVisionMode(dolbyModeValue.c_str());
                }
                else
                {
                    LOGINFO("%s : GetLocalParam Failed \n",__FUNCTION__);
                }
            }
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
            LOGINFO("Exit : resetDolbyMode Successful to value : %d \n",dolbyMode);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getDolbyVisionModeCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;
        std::string isPlatformSupport;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"DolbyVisionMode", isPlatformSupport);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
	   
	    response["platformSupport"] = (isPlatformSupport.compare("true") == 0 ) ? true : false;

            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["options"]=rangeArray;
            if ((pqmode.front()).compare("none") != 0) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getSupportedHDR10Modes(const JsonObject& parameters, JsonObject& response)
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

            response["supportedHDR10Modes"] = SupportedHDR10Modes;
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::setHDR10Mode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;

        value = parameters.HasLabel("HDR10Mode") ? parameters["HDR10Mode"].String() : "";
        returnIfParamNotFound(parameters,"HDR10Mode");

	    tvError_t ret = SetTVHDR10Mode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=getHDR10ModeIndex(value.c_str());
            int retval=updatePQParamsToCache("set","HDR10Mode", "all","all","hdr10",PQ_PARAM_HDR10_MODE,params);
            if(retval != 0) {
                LOGWARN("Failed to Save HDR10Mode to ssm_data\n");
            }
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, AVOUTPUT_HDR10MODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", AVOUTPUT_HDR10MODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", AVOUTPUT_HDR10MODE_RFC_PARAM, value.c_str());
            }

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

      
    uint32_t AVOutputTV::getHDR10Mode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string hdr10;

        if ( -1 == getDolbyParams(tvContentFormatType_HDR10, hdr10)) {
            returnResponse(false);
        }
        else {
            response["hdr10Mode"] = hdr10;
            LOGINFO("Exit getHDR10Mode(): %s\n",hdr10.c_str());
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::resetHDR10Mode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        tvError_t ret = tvERROR_NONE;

	if( !isCapablityCheckPassed( "global", "global", "hdr10", "HDR10Mode" )) {
            LOGERR("%s: CapablityCheck failed for HDR10Mode\n", __FUNCTION__);
            returnResponse(false);
        }

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,AVOUTPUT_HDR10MODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", AVOUTPUT_HDR10MODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", AVOUTPUT_HDR10MODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));
			tr181ErrorCode_t err = getLocalParam(rfc_caller_id, AVOUTPUT_HDR10MODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", AVOUTPUT_HDR10MODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", AVOUTPUT_HDR10MODE_RFC_PARAM);

                ret = SetTVHDR10Mode(param.value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("DV Mode set failed: %s\n",getErrorString(ret).c_str());
                }
                else {

                    LOGINFO("DV Mode initialized successfully value %s\n",param.value);
                    //Save HDR10Mode to ssm_data
                    int params[3]={0};
                    params[0]=getHDR10ModeIndex(param.value);
                    int retval=updatePQParamsToCache("reset","HDR10Mode","all","all","hdr10",PQ_PARAM_HDR10_MODE,params);

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

    uint32_t AVOutputTV::getHDR10ModeCaps(const JsonObject& parameters, JsonObject& response)
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

        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"HDR10Mode");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {

            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["HDR10ModeInfo"]=rangeArray;
            if ((pqmode.front()).compare("none") != 0) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }
    
    uint32_t AVOutputTV::getSupportedHLGModes(const JsonObject& parameters, JsonObject& response)
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

            response["supportedHLGModes"] = SupportedHLGModes;
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getHLGMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string hlg;

        if ( -1 == getDolbyParams(tvContentFormatType_HLG, hlg)) {
            returnResponse(false);
        }
        else {
            response["hlgMode"] = hlg;
            LOGINFO("Exit getHLGMode(): %s\n",hlg.c_str());
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::setHLGMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;

        value = parameters.HasLabel("HLGMode") ? parameters["HLGMode"].String() : "";
        returnIfParamNotFound(parameters,"HLGMode");

	    tvError_t ret = SetTVHLGMode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=getHLGModeIndex(value.c_str());
            int retval=updatePQParamsToCache("set","HLGMode","global","global","hlg",PQ_PARAM_HLG_MODE,params);
            if(retval != 0) {
                LOGWARN("Failed to Save HLGMode to ssm_data\n");
            }
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, AVOUTPUT_HLGMODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", AVOUTPUT_HLGMODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", AVOUTPUT_HLGMODE_RFC_PARAM, value.c_str());
            }

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::resetHLGMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        tvError_t ret = tvERROR_NONE;

	if( !isCapablityCheckPassed( "global", "global", "hlg", "HLGMode" )) {
            LOGERR("%s: CapablityCheck failed for HLGMode\n", __FUNCTION__);
            returnResponse(false);
        }

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,AVOUTPUT_HLGMODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", AVOUTPUT_HLGMODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", AVOUTPUT_HLGMODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));
			tr181ErrorCode_t err = getLocalParam(rfc_caller_id, AVOUTPUT_HLGMODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", AVOUTPUT_HLGMODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", AVOUTPUT_HLGMODE_RFC_PARAM);

                ret = SetTVHLGMode(param.value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("DV Mode set failed: %s\n",getErrorString(ret).c_str());
                }
                else {

                    LOGINFO("DV Mode initialized successfully value %s\n",param.value);
                    //Save HLGMode to ssm_data
                    int params[3]={0};
                    params[0]=getHLGModeIndex(param.value);
                    int retval=updatePQParamsToCache("reset","HLGMode","global","global","hlg",PQ_PARAM_HLG_MODE,params);

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

    uint32_t AVOutputTV::getHLGModeCaps(const JsonObject& parameters, JsonObject& response)
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

        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"HLGMode");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {

            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["HLG10ModeInfo"]=rangeArray;
            if ((pqmode.front()).compare("none") != 0) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getWBInfo(const JsonObject& parameters, JsonObject& response)
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

    uint32_t AVOutputTV::getWBCtrl(const JsonObject& parameters,JsonObject& response)
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

    uint32_t AVOutputTV::setWBCtrl(const JsonObject& parameters, JsonObject& response)
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

        if (parsingSetInputArgument(parameters, "WhiteBalance",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

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
                    std::string identifier = AVOUTPUT_GENERIC_STRING_RFC_PARAM;
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

    uint32_t AVOutputTV::resetWBCtrl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvError_t ret = tvERROR_NONE;
        
	string identifier=(std::string(AVOUTPUT_GENERIC_STRING_RFC_PARAM)+std::string("sdr")+std::string(".wb."));

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,identifier.c_str());
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n",identifier.c_str(),getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }else{
            identifier=(std::string(AVOUTPUT_GENERIC_STRING_RFC_PARAM)+std::string("hdr")+std::string(".wb."));
            if ( err != tr181Success ) {
                LOGWARN("clearLocalParam for %s Failed : %s\n",identifier.c_str(),getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
        }
        if( ret == tvERROR_NONE )
        {
            if( tvERROR_NONE != SyncWBFromLocalCache( ) )
            {
                LOGWARN("%s: Sync WB Failed!!! \n",__FUNCTION__);
                ret  = tvERROR_GENERAL;
            }
            else
                LOGINFO("%s: Sync WB Success!!! \n",__FUNCTION__);
            }
            else
            returnResponse(false);

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

    uint32_t AVOutputTV::getSupportedPictureModes(const JsonObject& parameters, JsonObject& response)
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

            response["supportedPictureModes"] = SupportedPicModes;
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getVideoSourceCaps(const JsonObject& parameters, JsonObject& response) {

        JsonArray rangeArray;

        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"VideoSource");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            if ((range.front()).compare("none") != 0) {
                for (unsigned int index = 0; index < range.size(); index++) {
                    rangeArray.Add(range[index]);
                }
                response["options"]=rangeArray;
            }
        }
        LOGINFO("Exit\n");
        returnResponse(true);
    }

    uint32_t AVOutputTV::getVideoFormatCaps(const JsonObject& parameters, JsonObject& response) {

        JsonArray rangeArray;

        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"VideoFormat");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            if ((range.front()).compare("none") != 0) {
                for (unsigned int index = 0; index < range.size(); index++) {
                    rangeArray.Add(range[index]);
                }
                response["options"]=rangeArray;
            }
        }
        LOGINFO("Exit\n");
        returnResponse(true);
    }

    uint32_t AVOutputTV::getVideoFrameRateCaps(const JsonObject& parameters, JsonObject& response) {
            LOGINFO("Entry\n");
            response["options"] = getSupportedVideoFrameRate();
            returnResponse(true);
    }

    uint32_t AVOutputTV::getVideoResolutionCaps(const JsonObject& parameters, JsonObject& response) {
            LOGINFO("Entry\n");
            response["options"] = getSupportedVideoResolution();
            returnResponse(true);
    }

    tvContentFormatType_t AVOutputTV::ConvertFormatStringToTVContentFormat(const char *format)
    {
        tvContentFormatType_t ret = tvContentFormatType_SDR;

        if( strncmp(format,"sdr",strlen(format)) == 0 || strncmp(format,"SDR",strlen(format)) == 0 )
            ret = tvContentFormatType_SDR;
        else if( strncmp(format,"hdr10",strlen(format)) == 0 || strncmp(format,"HDR10",strlen(format))==0 )
            ret = tvContentFormatType_HDR10;
        else if( strncmp(format,"hlg",strlen(format)) == 0 || strncmp(format,"HLG",strlen(format)) == 0 )
            ret = tvContentFormatType_HLG;
        else if( strncmp(format,"dolby",strlen(format)) == 0 || strncmp(format,"DOLBY",strlen(format)) == 0 )
            ret=tvContentFormatType_DOVI;

        return ret;
    }
    
    uint32_t AVOutputTV::getPictureModeCaps(const JsonObject& parameters, JsonObject& response) {

        JsonArray sourceArray;
        JsonArray formatArray;
        JsonArray rangeArray;
        
	std::vector<std::string> range;
	std::vector<std::string> source;
	std::vector<std::string> pqmode;
	std::vector<std::string> format;

	unsigned int index = 0;
        tvError_t ret = getParamsCaps(range,pqmode,source,format,"PictureMode");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
      
            if ((range.front()).compare("none") != 0) {
                for (index = 0; index < range.size(); index++) {
                    rangeArray.Add(range[index]);
                }
                response["options"]=rangeArray;
            }
 
            if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }
    uint32_t AVOutputTV::getPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string picturemode;
        std::string source;
        std::string format;
        std::string dummyPqmode;
        int current_source = 0;
        int current_format = 0;
        int pqIndex = 0;
        std::string tr181_param_name;
        TR181_ParamData_t param = {0};
        tr181ErrorCode_t err = tr181Success;

        if (parsingGetInputArgument(parameters, "PictureMode",source, dummyPqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
#if 0
        if( !isCapablityCheckPassed( dummyPqmode, source, format, "PictureMode" )) {
            LOGERR("%s: CapablityCheck failed for PictureMode\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        if (getParamIndex(source,dummyPqmode,format,current_source,pqIndex,current_format) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        current_format = ConvertHDRFormatToContentFormat((tvhdr_type_t)current_format);

        LOGINFO("current format index[%d] \n", current_format);
        tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
        tr181_param_name += "." + std::to_string(current_source) + "." + "Format."+std::to_string(current_format)+"."+"PictureModeString";
        LOGINFO("tr181 command[%s]", tr181_param_name.c_str());
         err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);

         if ( tr181Success != err ) {
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

    uint32_t AVOutputTV::setPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string source;
        std::string format;
        std::string dummyPqmode;
        char prevmode[PIC_MODE_NAME_MAX]={0};
        GetTVPictureMode(prevmode);

        tvError_t ret = tvERROR_NONE;
        value = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        returnIfParamNotFound(parameters,"pictureMode");

        // As only source need to validate, so pqmode and formate passing as currrent
        if (parsingSetInputArgument(parameters, "PictureMode",source, dummyPqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( dummyPqmode, source,format, "PictureMode" )) {
            LOGERR("%s: CapablityCheck failed for PictureMode\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired("current",source,format) ) {
            LOGINFO("Proceed with SetTVPictureMode\n");
            ret = SetTVPictureMode(value.c_str());
         }
        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
	    std::vector<int> pq_mode_vec;
	    std::vector<int> source_vec;
	    std::vector<int> format_vec;

	    getSaveConfig("current", source.c_str(), format.c_str(), source_vec, pq_mode_vec, format_vec);
            
	    for (unsigned int x = 0; x < source_vec.size(); x++ ) {
                for (unsigned int y = 0; y < format_vec.size(); y++ ) {

                    std::string tr181_param_name = "";
                    tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
                    // framing Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.Source.source_index[x].Format.format_index[x].PictureModeString.value
                    tr181_param_name += "."+std::to_string(source_vec[x])+"."+"Format."+
			              std::to_string(ConvertHDRFormatToContentFormat((tvhdr_type_t)format_vec[y]))+"."+"PictureModeString";
                    tr181ErrorCode_t err = setLocalParam(rfc_caller_id, tr181_param_name.c_str(), value.c_str());
                    if ( err != tr181Success ) {
                        LOGWARN("setLocalParam for %s Failed : %s\n", AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM, getTR181ErrorString(err));
                    }
                    else {
                        LOGINFO("setLocalParam for %s Successful, Value: %s\n", AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM, value.c_str());
		        int pqmodeindex = (int)GetTVPictureModeIndex(value.c_str());
                        SaveSourcePictureMode(source_vec[x], format_vec[y], pqmodeindex);
		    }
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

    uint32_t AVOutputTV::resetPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tr181ErrorCode_t err = tr181Success;
        TR181_ParamData_t param = {0};
        
        std::vector<int> pq_mode_vec;
        std::vector<int> source_vec;
        std::vector<int> format_vec;
        std::string source;
        std::string dummyPqmode;
        std::string format;

        // As only source need to validate, so pqmode and formate passing as currrent
        if (parsingSetInputArgument(parameters, "PictureMode",source, dummyPqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( dummyPqmode, source,format, "PictureMode" )) {
            LOGERR("%s: CapablityCheck failed for PictureMode\n", __FUNCTION__);
            returnResponse(false);
        }

        getSaveConfig("current", source, format, source_vec, pq_mode_vec, format_vec);
 
        for (int source : source_vec) {
            for (int format : format_vec) {

                std::string tr181_param_name = "";
                tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
	        tr181_param_name += "."+std::to_string(source)+"."+"Format."+
			           std::to_string(ConvertHDRFormatToContentFormat((tvhdr_type_t)format))+"."+"PictureModeString";

       	        err = clearLocalParam(rfc_caller_id, tr181_param_name.c_str());
                if ( err != tr181Success ) {
                    LOGWARN("clearLocalParam for %s Failed : %s\n", tr181_param_name.c_str(), getTR181ErrorString(err));
                    returnResponse(false);
                }
                else {
                    LOGINFO("clearLocalParam for %s Successful\n", tr181_param_name.c_str());
                    err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
                    if ( tr181Success == err )
                    {
                        LOGINFO("getLocalParam for %s is %s\n", AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM, param.value);
                        //get curren source and if matches save for that alone
		        int current_source = 0;
                        int current_format = 0;
		        GetCurrentSource(&current_source);

                        if( HDR_TYPE_NONE == ConvertVideoFormatToHDRFormat(GetCurrentContentFormat())) {
                            current_format = (int)HDR_TYPE_SDR;
                        } else {
                            current_format = (int)ConvertVideoFormatToHDRFormat(GetCurrentContentFormat());
                        }

		        if (current_source == source && current_format == format ) {
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
                        LOGWARN("getLocalParam for %s failed\n", AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
                        returnResponse(false);
                    }
                }

                int pqmodeindex = (int)GetTVPictureModeIndex(param.value);
                SaveSourcePictureMode(source, format, pqmodeindex);
	    }
        }
	returnResponse(true;)
    }

    uint32_t AVOutputTV::enableWBMode(const JsonObject& parameters, JsonObject& response)
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

    uint32_t AVOutputTV::setBacklightFade(const JsonObject& parameters, JsonObject& response)
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

    uint32_t AVOutputTV::setLowLatencyState(const JsonObject& parameters, JsonObject& response)
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

        if (parsingSetInputArgument(parameters, "LowLatencyState",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed(pqmode, source, format, "LowLatencyState" )) {
            LOGERR("%s: CapablityCheck failed for LowLatencyState\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with setLowLatencyState\n");
             ret = SetLowLatencyState( lowLatencyIndex );
        }
	else
        {
            LOGINFO("%s: Set not required for this request!!! Just Save it\n",__FUNCTION__);
        }

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to setLowLatency\n");
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=lowLatencyIndex;
            int retval= updatePQParamsToCache("set","LowLatencyState",pqmode,source,format,PQ_PARAM_LOWLATENCY_STATE,params);
            if(retval != 0 ) {
                LOGWARN("Failed to SaveLowLatency to ssm_data\n");
            }
            LOGINFO("Exit : setLowLatency successful to value: %d\n", lowLatencyIndex);
            returnResponse(true);
        }
    }
	
    uint32_t AVOutputTV::getLowLatencyState(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        int lowlatencystate = 0;

        if (parsingGetInputArgument(parameters, "LowLatencyState",source, pqmode, format) != 0)
        {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
#if 0
        if( !isCapablityCheckPassed( pqmode, source, format, "LowLatencyState" )) {
            LOGERR("%s: CapablityCheck failed for LowLatencyState\n", __FUNCTION__);
            returnResponse(false);
        }
#endif
        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("LowLatencyState",formatIndex,pqIndex,sourceIndex,lowlatencystate, PQ_PARAM_LOWLATENCY_STATE);
        if( err == 0 ) {
            response["lowLatencyState"] = std::to_string(lowlatencystate);
            LOGINFO("Exit : LowLatencyState Value: %d \n", lowlatencystate);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t AVOutputTV::resetLowLatencyState(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,lowlatencystate=0;
        int params[3]={0};
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "LowLatencyState",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "LowLatencyState" )) {
            LOGERR("%s: CapablityCheck failed for LowLatencyState\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updatePQParamsToCache("reset","LowLatencyState",pqmode,source,format,PQ_PARAM_LOWLATENCY_STATE,params);
        if(retval != 0 )
        {
            LOGWARN("Failed to clear Lowlatency from ssmdata and localstore\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("current","current", "current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("LowLatencyState",formatIndex,pqIndex,sourceIndex, lowlatencystate, PQ_PARAM_LOWLATENCY_STATE);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex, lowlatencystate);
                    ret = SetLowLatencyState(lowlatencystate);
                }
                else
                {
                    LOGINFO("%s : GetLocalParam Failed \n",__FUNCTION__);
                }
            }
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

    uint32_t AVOutputTV::getLowLatencyStateCaps(const JsonObject& parameters, JsonObject& response)
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

        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"LowLatencyState");

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(stoi(range[index]));

            response["LowLatencyInfo"]=rangeArray;
            if ((pqmode.front()).compare("none") != 0) {
                for (index = 0; index < pqmode.size(); index++) {
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) {
                for (index = 0; index < source.size(); index++) {
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) {
                for (index = 0; index < format.size(); index++) {
                    formatArray.Add(format[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }
  
    uint32_t AVOutputTV::getVideoSource(const JsonObject& parameters,JsonObject& response)
    {
        LOGINFO("Entry\n");
        //PLUGIN_Lock(Lock);
        int currentSource = 0;

        tvError_t ret = GetCurrentSource(&currentSource);
        //response["supportedVideoSource"] = getSupportedVideoSource();
        if(ret != tvERROR_NONE) {
            response["currentVideoSource"] = "NONE";
            returnResponse(false);
        }
        else {
            response["currentVideoSource"] = convertSourceIndexToString(currentSource);
            LOGINFO("Exit: getVideoSource :%d   success \n", currentSource);
            returnResponse(true);
        }
    }
 
//Helper Function

    void AVOutputTV::LocatePQSettingsFile()
    {
        LOGINFO("Entry\n");
        char panelId[20] = {0};
        std::string PQFileName = AVOUTPUT_RFC_CALLERID;
        std::string FilePath = "/etc/rfcdefaults/";

        /* The if condition is to override the tvsettings ini file so it helps the PQ tuning process for new panels */
        if(access(AVOUTPUT_OVERRIDE_PATH, F_OK) == 0){
            PQFileName = std::string(AVOUTPUT_RFC_CALLERID_OVERRIDE);
        }else{
            int val=GetPanelID(panelId);
            if(val==0)
            {
                LOGINFO("%s : panel id read is : %s\n",__FUNCTION__,panelId);
                if(strncmp(panelId,AVOUTPUT_CONVERTERBOARD_PANELID,strlen(AVOUTPUT_CONVERTERBOARD_PANELID))!=0)
                {
                    PQFileName+=std::string("_")+panelId;
                    struct stat tmp_st;

                    LOGINFO("%s: Looking for %s.ini \n",__FUNCTION__,PQFileName.c_str());
                    if(stat((FilePath+PQFileName+std::string(".ini")).c_str(), &tmp_st)!=0)
                    {
                        //fall back
                        LOGINFO("%s not available in %s Fall back to default\n",PQFileName.c_str(),FilePath.c_str());
                        PQFileName =std::string(AVOUTPUT_RFC_CALLERID);
                    }
                }
            }
            else{
                LOGINFO("%s : GetPanelID failed : %d\n",__FUNCTION__,val);
            }
        }
        strncpy(rfc_caller_id,PQFileName.c_str(),PQFileName.size());
        LOGINFO("%s : Default tvsettings file : %s\n",__FUNCTION__,rfc_caller_id);
    }

    tvContentFormatType_t AVOutputTV::getContentFormatIndex(tvVideoHDRFormat_t formatToConvert)
    {
        /* default to SDR always*/
        tvContentFormatType_t ret = tvContentFormatType_NONE;
        switch(formatToConvert)
        {
            case tvVideoHDRFormat_HLG:
                ret = tvContentFormatType_HLG;
                break;

            case tvVideoHDRFormat_HDR10:
                ret = tvContentFormatType_HDR10;
                break;

            case tvVideoHDRFormat_HDR10PLUS:
                ret =  tvContentFormatType_HDR10PLUS;
                break;

            case tvVideoHDRFormat_DV:
                ret = tvContentFormatType_DOVI;
                break;

            case tvVideoHDRFormat_SDR:
            case tvVideoHDRFormat_NONE:
            default:
                ret  = tvContentFormatType_SDR;
                break;
        }
        return ret;
    }

    tvError_t AVOutputTV::SyncPQParamsToDriverCache(std::string pqmode,std::string source,std::string format)
    {
        int params[3]={0};

        LOGINFO("Entry %s : pqmode : %s source : %s format : %s\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str());

        if( !updatePQParamsToCache("sync","Brightness",pqmode,source,format,PQ_PARAM_BRIGHTNESS,params))
            LOGINFO("Brightness Successfully sync to Drive Cache\n");
        else
            LOGINFO("Brightness Sync to cache Failed !!!\n");

        if( !updatePQParamsToCache("sync","Contrast",pqmode,source,format,PQ_PARAM_CONTRAST,params))
            LOGINFO("Contrast Successfully Synced to Drive Cache\n");
        else
            LOGINFO("Contrast Sync to cache Failed !!!\n");

        if( !updatePQParamsToCache("sync","Sharpness",pqmode,source,format,PQ_PARAM_SHARPNESS,params))
            LOGINFO("Sharpness Successfully Synced to Drive Cache\n");
        else
            LOGINFO("Sharpness Sync to cache Failed !!!\n");

        if( !updatePQParamsToCache("sync","Saturation",pqmode,source,format,PQ_PARAM_SATURATION,params))
            LOGINFO("Saturation Successfully Synced to Drive Cache\n");
        else
            LOGINFO("Saturation Sync to cache Failed !!!\n");

	if( !updatePQParamsToCache("sync","Hue",pqmode,source,format,PQ_PARAM_HUE,params))
            LOGINFO("Hue Successfully Synced to Drive Cache\n");
        else
            LOGINFO("Hue Sync to cache Failed !!!\n");

        if( !updatePQParamsToCache("sync","ColorTemp",pqmode,source,format,PQ_PARAM_COLOR_TEMPERATURE,params))
            LOGINFO("ColorTemp Successfully Synced to Drive Cache\n");
        else
            LOGINFO("ColorTemp Sync to cache Failed !!!\n");

        if( !updatePQParamsToCache("sync","DolbyVisionMode",pqmode,source,"dv",PQ_PARAM_DOLBY_MODE,params))
            LOGINFO("dvmode Successfully Synced to Drive Cache\n");
        else
            LOGINFO("dvmode Sync to cache Failed !!!\n");

        if( !updatePQParamsToCache("sync","DimmingMode",pqmode,source,format,PQ_PARAM_DIMMINGMODE,params))
            LOGINFO("dimmingmode Successfully Synced to Drive Cache\n");
        else
            LOGINFO("dimmingmode Sync to cache Failed !!!\n");

        if(appUsesGlobalBackLightFactor){
            if( !updatePQParamsToCache("sync","HLGMode",pqmode,source,"hlg",PQ_PARAM_HLG_MODE,params))
                LOGINFO("hlgmode Successfully Synced to Drive Cache\n");
            else
                LOGINFO("hlgmode Sync to cache Failed !!!\n");

            if( !updatePQParamsToCache("sync","HDR10Mode",pqmode,source,"hdr10",PQ_PARAM_HDR10_MODE,params))
                LOGINFO("hdr10mode Successfully Synced to Drive Cache\n");
            else
                LOGINFO("hdr10mode Sync to cache Failed !!!\n");
        }

	if( !updatePQParamsToCache("sync","Backlight",pqmode,source,format,PQ_PARAM_BACKLIGHT,params) )
            LOGINFO("Backlight Successfully Synced to Drive Cache\n");
        else
            LOGINFO("Backlight Sync to cache Failed !!!\n");

        if(appUsesGlobalBackLightFactor)
        {
            syncCMSParamsToDriverCache(pqmode,source,format);

            SyncWBparams();
        }
        LOGINFO("Exit %s : pqmode : %s source : %s format : %s\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str());
        return tvERROR_NONE;
    }

    int AVOutputTV::SyncSourceFormatPicModeToCache(std::string pqmode, std::string source, std::string format)
    {
        std::vector<int> sources;
        std::vector<int> pictureModes;
        std::vector<int> formats;
        tr181ErrorCode_t err = tr181Success;
        TR181_ParamData_t param = {0};
        int ret = 0;

        ret = getSaveConfig(pqmode, source, format, sources, pictureModes, formats);

        if (ret == 0 )
        {
            for (int source : sources) 
            {
                for (int format : formats)
                {
                    std::string tr181_param_name = "";
                    tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
                    tr181_param_name += "."+std::to_string(source)+"."+"Format."+
                                         std::to_string(ConvertHDRFormatToContentFormat((tvhdr_type_t)format))+"."+"PictureModeString";

                    err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
                    if ( tr181Success == err ) 
                    {
                        int pqmodeindex = (int)GetTVPictureModeIndex(param.value);

                        tvError_t tv_err = SaveSourcePictureMode(source, format, pqmodeindex);
                        if (tv_err != tvERROR_NONE)
                        {
                            LOGWARN("failed to SaveSourcePictureMode \n");
                            return -1;
                        }
                    } 
                    else
                    {
                        LOGWARN("Failed to get the getLocalParam \n");
                        return -1;
                    }
                }
            }
        }
        return ret;
   }

   int AVOutputTV::InitializeSDRHDRBacklight(void)
    {
        char panelId[20] = {0};
        int val=GetPanelID(panelId);
        int ret=0;

        if (val != 0)
        {
            LOGINFO("Failed to read panel id!!! Set 55 panel as default\n");
            memset(panelId,0,sizeof(panelId));
            GetDefaultPanelID(panelId);
            LOGINFO("Panel ID : %s \n",panelId);
        }
        else
            LOGINFO("Read panel id ok [%s] \n", panelId);

        /* Load Default Panel ID for Converter Boards*/
        if(strncmp(panelId,AVOUTPUT_CONVERTERBOARD_PANELID,strlen(AVOUTPUT_CONVERTERBOARD_PANELID))==0)
        {
            memset(panelId,0,sizeof(panelId));
            GetDefaultPanelID(panelId);
            LOGINFO("Load 55 panel values as default panel ID : %s\n",panelId);
        }

        val=ReadBacklightFromTable(panelId);
        if(val == 0)
            LOGINFO("Backlight read success from backlight_default.ini\n");
        else
            LOGWARN("Backlight read failed from backlight_default.ini\n");

        return ret;
    }

    void AVOutputTV::convertParamToLowerCase(std::string &source, std::string &pqmode, std::string &format)
    {
        LOGINFO("%s : Entry Source : %s, pqmode : %s, format: %s \n",__FUNCTION__,source.c_str(),pqmode.c_str(), format.c_str());
        transform(source.begin(), source.end(), source.begin(), ::tolower);
        transform(pqmode.begin(), pqmode.end(), pqmode.begin(), ::tolower);
        transform(format.begin(), format.end(), format.begin(), ::tolower);

        LOGINFO("%s : Exit Source : %s, pqmode : %s, format: %s \n",__FUNCTION__,source.c_str(),pqmode.c_str(), format.c_str());
    }

    std::string AVOutputTV::convertToString(std::vector<std::string> vec_strings)
    {
        std::string result = std::accumulate(vec_strings.begin(), vec_strings.end(), std::string(),
            [](const std::string& a, const std::string& b) -> std::string {
                return a.empty() ? b : a + "," + b;
            });
        return result;
    }

    int AVOutputTV::convertToValidInputParameter(std::string pqparam, std::string & source, std::string & pqmode, std::string & format) 
    {

        LOGINFO("Entry %s source %s pqmode %s format %s \n", __FUNCTION__, source.c_str(), pqmode.c_str(), format.c_str());

        convertParamToLowerCase(source, pqmode, format);
        //std::vector<std::string> temp_vec;
        //std::string temp_string;

        // converting pq to valid paramter format
        if (pqmode == "global")
        {
            std::string localSource;
            std::string localPqmode;
            std::string localFormat;
            if (FetchCapablities(pqparam, localSource, localPqmode, localFormat) == 0) {
                pqmode = localPqmode;
		//if pqmode none from capabilty then lets keep pqmode as global to fail the capabilty
            }
            else
            {
                LOGINFO("%s, Failed to get picturemode capability\n", __FUNCTION__);
                return -1;
            }
#if 0
            pic_modes_t *availableModes;
            unsigned short num_pqmodes = 0;
            GetTVSupportedPictureModes(&availableModes, &num_pqmodes);

            for(int count = 0;count < num_pqmodes; count++)
            {
                temp_string.clear();
                temp_string += availableModes[count].name;
                LOGINFO("ALL:%s \n", temp_string.c_str());
                temp_vec.push_back(temp_string);
            }
            if (temp_vec.size() != 0) {
                pqmode = convertToString(temp_vec);
            }
            if (temp_vec.size() != 0 ) temp_vec.clear();
            if (!temp_string.empty()) temp_string.clear();
#endif
            
        }
        else if (pqmode == "current")
        {
            char picMode[PIC_MODE_NAME_MAX]={0};
            if(!getCurrentPictureMode(picMode)) 
	    {
                LOGINFO("Failed to get the current picture mode\n");
                return -1;
            } 
	    else 
            {
                pqmode = picMode;
                LOGINFO("current PQmode :%s \n", pqmode.c_str());
            }
        }
	//else if (pqmode.empty()) {
	//    pqmode = "none";
	//}

        if (source == "global")
        {
#if 0
            pic_modes_t *availableSources;
            unsigned short num_source = 0;
            GetTVSupportedSources(&availableSources, &num_source);
            for (int count = 0; count < num_source; count++)
            {
                temp_string.clear();
                temp_string +=  availableSources[count].name;
                LOGINFO("ALL source:%s \n", temp_string.c_str());
                temp_vec.push_back(temp_string);
            }
            if (temp_vec.size() != 0) {
                source = convertToString(temp_vec);
            }
            if (temp_vec.size() != 0 ) temp_vec.clear();
            if (!temp_string.empty()) temp_string.clear();
#endif
            std::string localSource;
            std::string localPqmode;
            std::string localFormat;
            if (FetchCapablities(pqparam, localSource, localPqmode, localFormat) == 0) {
                source = localSource;
            }
            else
            {
                LOGINFO("%s, Failed to get source capability\n", __FUNCTION__);
                return -1;
            }
        } 
        else if (source == "current") 
        {
            int currentSource = 0;
            tvError_t ret = GetCurrentSource(&currentSource);
            
	    if(ret != tvERROR_NONE)
                LOGWARN("%s: GetCurrentSource( ) Failed \n",__FUNCTION__);
          
            source = convertSourceIndexToString(currentSource);
            LOGINFO("current source:%s \n", source.c_str());
        }
        //else if (source.empty()) {
	//    source = "none";
	//}	

        //convert format into valid parameter
        if (format == "global")
        {
#if 0
            char * formatVal;
            unsigned short num_format = 0;
            tvError_t ret = GetTVSupportedFormats(&formatVal, &num_format);
            if (ret == tvERROR_NONE)
            {
                for (int count = 0; count < num_format; count++)
                {
                    temp_string.clear();
                    temp_string += formatVal+(count*FORMAT_NAME_SIZE);
                    LOGINFO("ALL format:%s \n", temp_string.c_str());
                    if (temp_string != "none") {
                        temp_vec.push_back(temp_string);
                    }
                }
            }
            else
            {
                LOGINFO("Failed to read the format. \n");
                return -1;
            }
            if (temp_vec.size() != 0) {
                format = convertToString(temp_vec);
            }
            if (temp_vec.size() != 0 ) temp_vec.clear();
            if (!temp_string.empty()) temp_string.clear();
#endif
            std::string localSource;
            std::string localPqmode;
            std::string localFormat;
            if (FetchCapablities(pqparam, localSource, localPqmode, localFormat) == 0) {
                format = localFormat;
            }
            else
            {
                LOGINFO("%s, Failed to get format capability\n", __FUNCTION__);
                return -1;
            }
        }
	else if (format == "current") 
	{
          format = convertVideoFormatToString( GetCurrentContentFormat());
          LOGINFO("current:%s \n", format.c_str());
        }
	//else if (format.empty()) {
	//    format = "none";
	//}

        LOGINFO("Exit %s source %s pqmode %s format %s \n", __FUNCTION__, source.c_str(), pqmode.c_str(), format.c_str());
        return 0;
    }

    tvError_t AVOutputTV::getParamsCaps(std::vector<std::string> &range
                , std::vector<std::string> &pqmode, std::vector<std::string> &source, std::vector<std::string> &format,std::string param )
    {
        tvError_t ret = tvERROR_NONE;
    
        std::string rangeInfo;
        std::string sourceInfo;
        std::string formatInfo;
        std::string pqmodeInfo;
   
        std::string platformsupport;

        if( ReadCapablitiesFromConf( rangeInfo, pqmodeInfo, formatInfo ,sourceInfo,param, platformsupport))
        {
            LOGERR( "%s: ReadCapablitiesFromConf Failed !!!\n",__FUNCTION__);
            return tvERROR_GENERAL;
        }
        else
        {
            LOGINFO("%s : range : %s pqmode : %s source : %s format : %s param : %s \n",__FUNCTION__,rangeInfo.c_str(), pqmodeInfo.c_str(), sourceInfo.c_str(), formatInfo.c_str(),param.c_str() );
            spliltCapablities( range, pqmode, format, source, rangeInfo, pqmodeInfo, formatInfo, sourceInfo );
        }
    
        return ret;
    }

    tvError_t AVOutputTV::getParamsCaps(std::vector<std::string> &range
                , std::vector<std::string> &pqmode, std::vector<std::string> &source, std::vector<std::string> &format,std::string param,
                 std::string & isPlatformSupport)
    {
        tvError_t ret = tvERROR_NONE;

        std::string rangeInfo;
        std::string sourceInfo;
        std::string formatInfo;
        std::string pqmodeInfo;

        if( ReadCapablitiesFromConf( rangeInfo, pqmodeInfo, formatInfo ,sourceInfo,param, isPlatformSupport))
        {
            LOGERR( "%s: ReadCapablitiesFromConf Failed !!!\n",__FUNCTION__);
            return tvERROR_GENERAL;
        }
        else
        {
            LOGINFO("%s : range : %s pqmode : %s source : %s format : %s param : %s \n",__FUNCTION__,rangeInfo.c_str(), pqmodeInfo.c_str(),
 sourceInfo.c_str(), formatInfo.c_str(),param.c_str() );
            spliltCapablities( range, pqmode, format, source, rangeInfo, pqmodeInfo, formatInfo, sourceInfo );
        }

        return ret;
    }

    void AVOutputTV::spliltCapablities( std::vector<std::string> &range,std::vector<std::string> &pqmode,std::vector<std::string> &format,std::vector<std::string> &source, std::string rangeInfo, std::string pqmodeInfo, std::string formatInfo, std::string sourceInfo )
    {
        std::string token;
        std::stringstream rangeStream(rangeInfo);
        std::stringstream pqmodeStream(pqmodeInfo);
        std::stringstream formatStream(formatInfo);
        std::stringstream sourceStream(sourceInfo);
    
        while( getline(rangeStream,token,','))
        {
            range.push_back(token );
            token.clear();
        }
    
        while( getline(pqmodeStream,token,',') )
        {
            pqmode.push_back(token );
            token.clear();
        }
    
        while( getline(formatStream,token,','))
        {
            format.push_back( token );
            token.clear();
        }
    
        while( getline(sourceStream,token,',') )
        {
            source.push_back( token );
            token.clear();
        }
    
        LOGINFO("All the vectors \n");
        LOGINFO("Range :\n");
    
        for( std::string param : range )
            LOGINFO("%s ", param.c_str() );
    
        for( std::string param : pqmode )
            LOGINFO("%s ", param.c_str() );
    
        for( std::string param : format )
            LOGINFO("%s ", param.c_str() );
    
        for( std::string param : source )
            LOGINFO("%s ",param.c_str() );
    
        LOGINFO("\n");
    }

    bool AVOutputTV::isCapablityCheckPassed( std::string  pqmodeInputInfo,std::string sourceInputInfo,std::string formatInputInfo,std::string param )
    {

        std::string rangeCapInfo;
        std::string sourceCapInfo;
        std::string formatCapInfo;
        std::string pqmodeCapInfo;
	std::string isPlatformSupport;

        std::set<string> pqmodeCapSet;
        std::set<string> formatCapSet;
        std::set<string> sourceCapset;
        std::set<string> pqmodeInputSet;
        std::set<string> formatInputSet;
        std::set<string> sourceInputSet;
        
        if( ReadCapablitiesFromConf( rangeCapInfo, pqmodeCapInfo, formatCapInfo, sourceCapInfo,param, isPlatformSupport) )
        {
            LOGINFO( "%s: readCapablitiesFromConf Failed !!!\n",__FUNCTION__);
            return false;
        }

        //Compare capablityInfo with Input params

        //1.convertCapablity Info to set for comparison
        spliltStringsAndConvertToSet( pqmodeCapInfo, formatCapInfo, sourceCapInfo, pqmodeCapSet, formatCapSet, sourceCapset);

        //2.convert Application Input Info to set for comparison
        spliltStringsAndConvertToSet( pqmodeInputInfo, formatInputInfo, sourceInputInfo, pqmodeInputSet, formatInputSet, sourceInputSet );

        //3.Compare Each pqmode/format/source InputInfo against CapablityInfo
        if ( isIncluded(pqmodeCapSet,pqmodeInputSet) && isIncluded(formatCapSet,formatInputSet) && isIncluded(sourceCapset,sourceInputSet) )
        {
            LOGINFO("%s : Capablity Chesk passed \n", __FUNCTION__);
            return true;
        }
        else
        {
            LOGERR("%s : Capablity Check Failed \n", __FUNCTION__);
            return false;
        }
    }

    int AVOutputTV::updatePQParamsToCache( std::string action, std::string tr181ParamName, std::string pqmode, std::string source, std::string format, tvPQParameterIndex_t pqParamIndex, int params[] )
    {
        LOGINFO("Entry : %s\n",__FUNCTION__);
        std::vector<int> sources;
        std::vector<int> pictureModes;
        std::vector<int> formats;
        int ret = 0;
        bool sync = !(action.compare("sync"));
        bool reset = !(action.compare("reset"));
        bool set = !(action.compare("set"));

        LOGINFO("%s: Entry param : %s Action : %s pqmode : %s source :%s format :%s\n",__FUNCTION__,tr181ParamName.c_str(),action.c_str(),pqmode.c_str(),source.c_str(),format.c_str() );
        ret = getSaveConfig(pqmode, source, format, sources, pictureModes, formats);
        if( 0 == ret )
        {
            for(int source: sources)
            {
                for(int mode : pictureModes)
                {
                    for(int format : formats)
                    {
                        switch(pqParamIndex)
                        {
                            case PQ_PARAM_BRIGHTNESS:
                            case PQ_PARAM_CONTRAST:
                            case PQ_PARAM_BACKLIGHT:
                            case PQ_PARAM_SATURATION:
                            case PQ_PARAM_SHARPNESS:
                            case PQ_PARAM_HUE:
                            case PQ_PARAM_COLOR_TEMPERATURE:
                            case PQ_PARAM_DIMMINGMODE:
                            case PQ_PARAM_LOWLATENCY_STATE:
                            case PQ_PARAM_DOLBY_MODE:
                                if(reset)
                                    ret |= updatePQParamToLocalCache(tr181ParamName,source, mode, format,0,false);
                                if(sync || reset)
                                {
                                    int value=0;
                                    if(!getLocalparam(tr181ParamName,format,mode,source,value,pqParamIndex,sync))
                                        LOGINFO("Found param  %s pqmode : %d format:%d  source : %d value:%d\n",tr181ParamName.c_str(),mode,format,source,value);
                                    else
                                    {
                                        LOGINFO("Value not found in ini %s pqmode : %d format:%d source : %d value:%d\n",tr181ParamName.c_str(),mode,format,source,value);
                                        continue;
                                    }
                                    params[0]=value;
                                }
                                if(set)
                                {
                                    //SpecialCase to store UserScale backlight in localstore only products using global BLF
                                    if(( tr181ParamName.compare("Backlight") == 0) &&  appUsesGlobalBackLightFactor)
                                        ret |= updatePQParamToLocalCache(tr181ParamName,source, mode, format, params[1],true);
                                    else
                                        ret |= updatePQParamToLocalCache(tr181ParamName,source, mode, format, params[0],true);
                                }
                                break;
                            default:
                                break;
                        }

            	    switch(pqParamIndex)
                        {
                            case PQ_PARAM_BRIGHTNESS:
                                ret |= SaveBrightness(source, mode,format,params[0]);
                                break;
                            case PQ_PARAM_CONTRAST:
                                ret |= SaveContrast(source, mode,format,params[0]);
                                break;
                            case PQ_PARAM_SHARPNESS:
                                ret |= SaveSharpness(source, mode,format,params[0]);
                                break;
                            case PQ_PARAM_HUE:
                                ret |= SaveHue(source, mode,format,params[0]);
                                break;
                            case PQ_PARAM_SATURATION:
                                ret |= SaveSaturation(source, mode,format,params[0]);
                                break;
                            case PQ_PARAM_COLOR_TEMPERATURE:
                                ret |= SaveColorTemperature(source, mode,format,params[0]);
                                break;
                            case PQ_PARAM_BACKLIGHT:
                                if( (sync || reset) && appUsesGlobalBackLightFactor )
                                    convertUserScaleBacklightToDriverScale(format,params);

                                ret |= SaveBacklight(source, mode,format,params[0]);
                                break;
                            case PQ_PARAM_DIMMINGMODE:
                                ret |= SaveDynamicBacklight(source,mode,format,params[0]);
                                break;
                            case PQ_PARAM_LOWLATENCY_STATE:
                                ret |= SaveLowLatencyState(source, mode,format,params[0]);
                                break;
                            case PQ_PARAM_COMPONENT_HUE:
                            case PQ_PARAM_COMPONENT_SATURATION:
                            case PQ_PARAM_COMPONENT_LUMA:
                                if(reset)
                                    ret |= updatePQParamToLocalCache(tr181ParamName,source, mode, format,0,false);
                                if(sync || reset)
                                {
                                    int value=0;
                                    if(!getLocalparam(tr181ParamName,format,mode,source,value,pqParamIndex,sync,params[1]))
                                        LOGINFO("Found param from tr181 CMS pqmode : %d format:%d value:%d\n",mode,format,value);
                                    else{
                                        if(sync) /*block default cms sync to save tvsettings init time*/
                                            continue;
                                    }
                                    params[2]=value;
                                }
                                ret |= SaveCMS(source, mode,format,params[0],params[1],params[2]);
                                if(set)
                                    ret |= updatePQParamToLocalCache(tr181ParamName,source,mode, format, params[2],true);
                                break;
                            case PQ_PARAM_DOLBY_MODE:
                                 ret |= SaveDolbyMode(source, mode,format,params[0]);
                                 break;
                             case PQ_PARAM_HDR10_MODE:
                                 if(sync){
                                      int value=0;
                                      if( !getHDR10ParamToSync(value) )
                                          LOGINFO("Found param hdr10mode pqmode : %d format:%d value:%d\n",mode,format,value);
                                      else
                                          LOGERR("value not found  hdr10mode pqmode : %d format:%d value:%d\n",mode,format,value);

                                      params[0]=value;
                                 }
                                 ret |= SaveDolbyMode(source, mode,format,params[0]);
                                 break;

                             case PQ_PARAM_HLG_MODE:
                                 if(sync){
                                    int value=0;
                                    if( !getHLGParamToSync(value) )
                                        LOGINFO("Found param  hlgmode pqmode : %d format:%d value:%d\n",mode,format,value);
                                    else
                                        LOGERR("value not found hlgmode pqmode : %d format:%d value:%d\n",mode,format,value);

                                    params[0]=value;
                                 }
                                 ret |= SaveDolbyMode(source, mode,format,params[0]);
                                 break;
                             case PQ_PARAM_ASPECT_RATIO:
                                 ret |= SaveDisplayMode(source,mode,format,params[0]);
                                 break;
                             case PQ_PARAM_LOCALDIMMING_LEVEL:
			     {
                                 int dimmingLevel=0,edidLevel=0;
                                 if(sync)
                                 {
                                     int value=0;
                                     if(!getLocalparam(tr181ParamName,format,mode,source,value,pqParamIndex,sync))
                                         LOGINFO("Found param from tr181 ldim pqmode : %d format:%d value:%d \n",mode,format,value);
                                     else
                                         LOGERR("value not found in tr181 ldim pqmode : %d format:%d value:%d \n",mode,format,value);

                                     params[0]=value;
                                 }
                                 GetLDIMAndEDIDLevel(params[0],format,&dimmingLevel,&edidLevel,mode);
                                 ret |= SaveLocalDimmingLevel(source, mode,format,dimmingLevel);
                                 break;
			     }
			     case PQ_PARAM_CMS:
			     case PQ_PARAM_LDIM:
			     default:
				 break;
                        }
                    }
                }
           }

        }
        return ret;
    }

    tvError_t AVOutputTV::updatePQParamToLocalCache(std::string forParam, int source, int pqmode, int format, int value,bool setNotDelete)
    {
        tvError_t ret = tvERROR_NONE;
        std::string key;

        LOGINFO("Entry : %s source:%d,pqmode:%d,format:%d\n",__FUNCTION__,source,pqmode,format);

        format=ConvertHDRFormatToContentFormat((tvhdr_type_t)format);
        key.clear();
        generateStorageIdentifier(key,forParam,format,pqmode,source);
        if(key.empty())
        {
            LOGERR("generateStorageIdentifierDirty failed\n");
            ret = tvERROR_GENERAL;
        }
        else
        {
            tr181ErrorCode_t err  = tr181Success;
            if(setNotDelete)
            {
                std::string toStore = std::to_string(value);
                if (forParam.compare("ColorTemp") == 0)
                {
                    getColorTempStringFromEnum(value, toStore);
                }
                else if(forParam.compare("DimmingMode") == 0 )
                {
                    getDimmingModeStringFromEnum(value, toStore);
                }
                else if (forParam.compare("DolbyVisionMode") == 0 )
                {
                    toStore = getDolbyModeStringFromEnum((tvDolbyMode_t)value); 
                }
                err = setLocalParam(rfc_caller_id, key.c_str(),toStore.c_str());

            }
            else
            {
                err = clearLocalParam(rfc_caller_id, key.c_str());
            }

            if ( err != tr181Success ) {
                LOGERR("%s for %s Failed : %s\n", setNotDelete?"Set":"Delete", key.c_str(), getTR181ErrorString(err));
                ret = tvERROR_GENERAL;
            }
            else {
                LOGINFO("%s for %s Successful \n", setNotDelete?"Set":"Delete",key.c_str());
            }
        }
        return ret;
    }

    int AVOutputTV::getSaveConfig(std::string pqmode, std::string source, std::string format,std::vector<int> &sources,std::vector<int> &picturemodes, std::vector<int> &formats)
    {
        LOGINFO("Entry : %s pqmode : %s source :%s format :%s\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str());

        int ret = 0;

        if (getAvailableCapabilityModes(source, pqmode, format) != 0)
        {
            LOGERR("%s: failed to get picture/source/format mode capability \n", __FUNCTION__);
            return -1;
        }
        //1)Check pqmode
        //if( (pqmode.compare("global") == 0) || (pqmode.compare("none") == 0 ) )
        //{
        //    int lCount = 0;
       //     for(;lCount<numberModesSupported;lCount++)
       //         picturemodes.push_back(pic_mode_index[lCount]);
        //}
        //else
        //{
            char *modeString = strdup(pqmode.c_str());
            char *token = NULL;
            while ((token = strtok_r(modeString,",",&modeString)))
            {
                picturemodes.push_back(GetTVPictureModeIndex(token));
                LOGINFO("%s : PQmode: %s\n",__FUNCTION__,token);
            }
        //}

        //2)Check Source
        //if( (source.compare("global") == 0) || (source.compare("none") == 0) )
        //{
        //    int lCount = 0;
        //    for(;lCount<numberSourcesSupported;lCount++)
        //        sources.push_back(source_index[lCount]);
        //}
        //else
        //{
            char *sourceString = strdup(source.c_str());
            char *sourceToken = NULL;
            while ((sourceToken = strtok_r(sourceString,",",&sourceString)))
            {
                sources.push_back(GetTVSourceIndex(sourceToken));
                LOGINFO("%s : Source %s\n",__FUNCTION__,sourceToken);
            }
        //}

        //3)check format
#if 0
        unsigned int contentFormats=0;
        unsigned short numberOfSupportedFormats =  0;

        GetSupportedContentFormats(&contentFormats,&numberOfSupportedFormats);

        if( (format.compare("global") == 0) || (format.compare("none") == 0) )
        {
             unsigned int lcount=0;
             for(;(lcount<sizeof(uint32_t)*8 && numberOfSupportedFormats);lcount++)
             {
                 tvhdr_type_t formatToStore = (tvhdr_type_t)ConvertVideoFormatToHDRFormat((tvVideoHDRFormat_t)(contentFormats&(1<<lcount)));
                 if(formatToStore!= HDR_TYPE_NONE)
                 {
                     numberOfSupportedFormats--;
                     formats.push_back(formatToStore);
                 }
             }
        }
        else
        {
#endif
            char *formatString = strdup(format.c_str());
            char *formatToken = NULL;
            while ((formatToken = strtok_r(formatString,",",&formatString)))
            {
                formats.push_back(ConvertFormatStringToHDRFormat(formatToken));
                LOGINFO("%s : Format: %s\n",__FUNCTION__,formatToken);
            }
        //}

        LOGINFO("Exit : %s pqmode : %s source :%s format :%s ret:%d\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str(), ret);
        return ret;
    }

    uint32_t AVOutputTV::generateStorageIdentifier(std::string &key, std::string forParam,int contentFormat, int pqmode, int source)
    {
        key+=std::string(AVOUTPUT_GENERIC_STRING_RFC_PARAM);
        key+=STRING_SOURCE+std::to_string(source)+std::string(".")+STRING_PICMODE+std::to_string(pqmode)+std::string(".")+std::string(STRING_FORMAT)+std::to_string(contentFormat)+std::string(".")+forParam;
        return tvERROR_NONE;
    }

    uint32_t AVOutputTV::generateStorageIdentifierDirty(std::string &key, std::string forParam,uint32_t contentFormat, int pqmode)
    {
        key+=std::string(AVOUTPUT_GENERIC_STRING_RFC_PARAM);
        key+=STRING_PICMODE+std::to_string(pqmode)+std::string(".")+std::string(STRING_FORMAT)+std::to_string(contentFormat);
        CREATE_DIRTY(key)+=forParam;

        return tvERROR_NONE;
    }

    int AVOutputTV::getLocalparam( std::string forParam,int formatIndex,int pqIndex,int sourceIndex,int & value,
		                   tvPQParameterIndex_t pqParamIndex,bool sync,int color )
    {
        string key;
        //int value = 0;
        TR181_ParamData_t param={0};
        int formatVendorIndex = formatIndex;

        formatIndex=ConvertHDRFormatToContentFormat((tvhdr_type_t)formatIndex);
        generateStorageIdentifier(key,forParam,formatIndex,pqIndex,sourceIndex);
        if(key.empty())
        {
            LOGERR("generateStorageIdentifier failed\n");
            return -1;
        }

       tr181ErrorCode_t err=getLocalParam(rfc_caller_id, key.c_str(), &param);

       if ( tr181Success == err )//Fetch new tr181format values
       {
           if( forParam.compare("ColorTemp") == 0 )
           {
               if (strncmp(param.value, "Standard", strlen(param.value))==0)
                   value=tvColorTemp_STANDARD;
               else if (strncmp(param.value, "Warm", strlen(param.value))==0)
                   value=tvColorTemp_WARM;
               else if (strncmp(param.value, "Cold", strlen(param.value))==0)
                   value=tvColorTemp_COLD;
               else if (strncmp(param.value, "User Defined", strlen(param.value))==0)
                   value=tvColorTemp_USER;
               else
                   value=tvColorTemp_STANDARD;
               return 0;
           }
           else if( forParam.compare("DimmingMode") == 0 )
           {
               if (strncmp(param.value, "fixed", strlen(param.value))==0)
                   value=tvDimmingMode_Fixed;
               else if (strncmp(param.value, "local", strlen(param.value))==0)
                   value=tvDimmingMode_Local;
               else if (strncmp(param.value, "global", strlen(param.value))==0)
                   value=tvDimmingMode_Global;
               return 0;
           }
	   else if ( forParam.compare("DolbyVisionMode") == 0)
	   {
	       if (strncmp(param.value, "dark", strlen(param.value)) == 0) {
	           value = tvDolbyMode_Dark; 
               }
	       else {
	           value = tvDolbyMode_Bright;
	       }
	       return 0;
	   }
           else
           {
               value=std::stoi(param.value);
               return 0;
           }
        }
        else // default value from DB
        {
            if( sync )
            {
                LOGINFO("Function %s: pqmode : %d format : %d source : %d \n", __FUNCTION__, pqIndex,formatIndex,sourceIndex);
                return 1;
            }
            GetDefaultParams(pqIndex,sourceIndex,formatVendorIndex,pqParamIndex,&value);
            LOGINFO("Default value from DB : %s : %d \n",key.c_str(),value);
            return 0;
        }
    }

    bool AVOutputTV::isSetRequired(std::string pqmode,std::string source,std::string format)
    {
        bool ret=false;
        char picMode[PIC_MODE_NAME_MAX]={0};
	tvError_t retVal = tvERROR_NONE;
	int sourceIndex = 0;
        std::string currentPicMode;
        std::string currentSource;
        std::string currentFormat;

        //GetCurrent pqmode
        if(!getCurrentPictureMode(picMode))
            LOGERR("Failed to get the current picture mode\n");

        currentPicMode = picMode; //Convert to string

        //GetCurrentSource
	retVal = GetCurrentSource(&sourceIndex);
	if(retVal != tvERROR_NONE) 
        {
             LOGERR("%s : GetCurrentSource( ) Failed\n",__FUNCTION__);
             return false;
        }
        currentSource = convertSourceIndexToString(sourceIndex);

        //GetCurrentFormat
        currentFormat = convertVideoFormatToString( GetCurrentContentFormat());

        LOGINFO("%s : currentSource = %s,currentPicMode = %s,currentFormat = %s\n",__FUNCTION__,currentSource.c_str(),currentPicMode.c_str(),currentFormat.c_str());
        LOGINFO("%s : source = %s,PicMode = %s, format= %s\n",__FUNCTION__,source.c_str(),pqmode.c_str(),format.c_str());

        if( ( (pqmode.find(currentPicMode) != std::string::npos) || (pqmode.compare("global") == 0)  || (pqmode.compare("current") == 0) ||
            (pqmode.compare("none") == 0) ) &&
           ((source.find(currentSource) != std::string::npos)  || (source.compare("global") == 0) || (source.compare("current") == 0) ||
            (source.compare("none") == 0) ) &&
           ( (format.find(currentFormat) !=  std::string::npos) || (format.compare("global") == 0) || (format.compare("current") == 0) ||
             (format.compare("none") == 0) ) )
            ret=true;

        return ret;
    }

    int AVOutputTV::getParamIndex(string source,string pqmode,string format,int& sourceIndex,int& pqmodeIndex,int& formatIndex)
    {
	LOGINFO("Entry : %s pqmode : %s source :%s format :%s\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str());

        if( source.compare("none") == 0 || source.compare("current") == 0 )
        {
            GetCurrentSource(&sourceIndex);
        }
        else
        {
            sourceIndex = GetTVSourceIndex(source.c_str());
        }
        if( pqmode.compare("none") == 0 || pqmode.compare("current") == 0)
        {
            char picMode[PIC_MODE_NAME_MAX]={0};
            if(!getCurrentPictureMode(picMode))
                LOGERR("Failed to get the current picture mode\n");
            else
                pqmodeIndex = GetTVPictureModeIndex(picMode);
        }
        else
        { 
            pqmodeIndex = GetTVPictureModeIndex(pqmode.c_str());
        }

        if( format.compare("none") == 0 || format.compare("current") == 0)
        {
            formatIndex = ConvertVideoFormatToHDRFormat(GetCurrentContentFormat());
            if( HDR_TYPE_NONE == formatIndex )
                formatIndex = HDR_TYPE_SDR; //Consider format invalid as SDR
        }
        else
        {
            formatIndex = ConvertFormatStringToHDRFormat(format.c_str());
        }
        LOGINFO("%s: Exit sourceIndex = %d pqmodeIndex = %d formatIndex = %d\n",__FUNCTION__,sourceIndex,pqmodeIndex,formatIndex);

        if (sourceIndex == tvSource_MAX || pqmodeIndex == tvPictureMode_MAX || formatIndex == HDR_TYPE_MAX) return -1;

	return 0;
    }

    tvDataComponentColor_t AVOutputTV::getComponentColorEnum(std::string colorName)
    {
        tvDataComponentColor_t CompColorEnum = tvDataColor_MAX;

        if(!colorName.compare("none")) {
            CompColorEnum = tvDataColor_NONE;
        }
        else if (!colorName.compare("red")){
            CompColorEnum = tvDataColor_RED;
        }
        else if (!colorName.compare("green")){
            CompColorEnum = tvDataColor_GREEN;
        }
        else if (!colorName.compare("blue")){
            CompColorEnum = tvDataColor_BLUE;
        }
        else if (!colorName.compare("yellow")){
            CompColorEnum = tvDataColor_YELLOW;
        }
        else if (!colorName.compare("cyan")){
            CompColorEnum = tvDataColor_CYAN;
        }
        else if (!colorName.compare("magenta")){
            CompColorEnum = tvDataColor_MAGENTA;
        }
        return CompColorEnum;
    }

    int AVOutputTV::getDolbyParams(tvContentFormatType_t format, std::string &s, std::string source) 
    {
        int ret = -1;
        TR181_ParamData_t param;
	std::string rfc_param = AVOUTPUT_HDR10MODE_RFC_PARAM;
        int dolby_mode_value = 0;
        int sourceIndex = 0;
        /*Since dolby vision is source specific, we should for check for specific source*/
        if (!source.empty()) {
            sourceIndex = GetTVSourceIndex(source.c_str());
	} else {
            GetCurrentSource(&sourceIndex);
        }
        memset(&param, 0, sizeof(param));
        if (format == tvContentFormatType_HLG ) {
            rfc_param = AVOUTPUT_HLGMODE_RFC_PARAM;
        } else if (format == tvContentFormatType_DOVI) {
            rfc_param = AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM + std::to_string(sourceIndex) + "."+"DolbyVisionMode";
        }

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, rfc_param.c_str(), &param);
        if ( tr181Success != err) {
            tvError_t retVal = GetDefaultParams(GetCurrentPQIndex(),sourceIndex,ConvertHDRFormatToContentFormat((tvhdr_type_t)format), PQ_PARAM_DOLBY_MODE,&dolby_mode_value);
	    if( retVal != tvERROR_NONE )
            {
                LOGERR("%s : failed\n",__FUNCTION__);
		return ret;
            }
            s = getDolbyModeStringFromEnum((tvDolbyMode_t)dolby_mode_value);
            ret = 0;
        } 
	else 
	{
            s += param.value;
            ret = 0;
        }
        return ret;
    }

    int AVOutputTV::getDimmingModeIndex(std::string mode)
    {
        unsigned short index = 1;

        if(mode.compare("local") == 0 )
            index=tvDimmingMode_Local;
        else if(mode.compare("fixed") == 0 )
            index=tvDimmingMode_Fixed;
        else if(mode.compare("global") == 0 )
            index=tvDimmingMode_Global;
        else
            LOGINFO("Return Default Dimmingmode:%d!!!\n",index);

        return index;
    }

    int AVOutputTV::saveLocalDimmingLevelToDriverCache(std::string action,std::string pqmode, std::string source, std::string format,int params[] )
    {
        int ret = 0;
    
        if( appUsesGlobalBackLightFactor )
            ret = updatePQParamsToCache(action,"LocalDimmingLevel",pqmode,source,format,PQ_PARAM_LOCALDIMMING_LEVEL,params);
        else
            LOGINFO("%s : Not Required !!!\n",__FUNCTION__);
    
        return ret;
    }

    void AVOutputTV::getDimmingModeStringFromEnum(int value, std::string &toStore)
    {
        const char *color_temp_string[] = {
                    [tvDimmingMode_Fixed] = "fixed",
                    [tvDimmingMode_Local] = "local",
                    [tvDimmingMode_Global] = "global",
                };
        toStore.clear();
        toStore+=color_temp_string[value];
    }

    void AVOutputTV::getColorTempStringFromEnum(int value, std::string &toStore)
    {
        const char *color_temp_string[] = {
                    [tvColorTemp_STANDARD] = "Standard",
                    [tvColorTemp_WARM] = "Warm",
                    [tvColorTemp_COLD] = "Cold",
                    [tvColorTemp_USER] = "User Defined"
                };
        toStore.clear();
        toStore+=color_temp_string[value];
    }

    int AVOutputTV::ReadBacklightFromTable(char *panelId)
    {
        std::string temp_panelid=std::string(panelId);
        temp_panelid=std::string(temp_panelid.rbegin(),temp_panelid.rend());//reverse
        std::string delimiter="_";
        temp_panelid=temp_panelid.erase(0, temp_panelid.find(delimiter) + delimiter.length());//remove the first _ token
        temp_panelid=std::string(temp_panelid.rbegin(),temp_panelid.rend());  //reverse again

        LOGINFO("%s: Looking at %s / %s for BLT \n",__FUNCTION__,panelId,temp_panelid.c_str());

        try
        {
            CIniFile inFile(BACKLIGHT_FILE_NAME);

            for(int i = 0 ; i < BACKLIGHT_CURVE_MAX_INDEX; i++ )
            {
                std::string s;
                s = temp_panelid+ "_" + std::string("SDR") + ".bl_" + std::to_string(i);
                blInfo.sdrBLCurve[i] = inFile.Get<int>(s);
                LOGINFO("bl_table_sdr[%d] = %u\n", i, blInfo.sdrBLCurve[i] );
            }

            for(int j = 0 ; j < BACKLIGHT_CURVE_MAX_INDEX; j++ )
            {
                std::string s;
                s = temp_panelid+ "_" + std::string("HDR") + ".bl_" + std::to_string(j);
                blInfo.hdrBLCurve[j] = inFile.Get<int>(s);
                LOGINFO("bl_table_hdr[%d] = %u\n", j, blInfo.hdrBLCurve[j] );
            }
        }
	catch(const boost::property_tree::ptree_error &e)
        {
            LOGERR("%s: error %s::config table entry not found in ini file\n",__FUNCTION__,e.what());
            return -1;
        }
        {
            blInfo.defaultBLSDR = defaultSDR;
            blInfo.defaultBLHDR = defaultHDR;
            SetBacklightInfo(&blInfo);
        }
        return 0;
    }

    int AVOutputTV::getCurrentPictureMode(char *picMode)
    {
        tvError_t  ret = tvERROR_NONE;
        TR181_ParamData_t param;
        std::string tr181_param_name;
        int currentSource = 0;
        int current_format = 0;

        ret = GetCurrentSource(&currentSource);
        if(ret != tvERROR_NONE)
        {
            LOGERR("GetCurrentSource() Failed set source to default\n");
	    return 0;
        }

        current_format = getContentFormatIndex(GetCurrentContentFormat());
        if ( current_format  == tvVideoHDRFormat_NONE) current_format  = tvVideoHDRFormat_SDR;

        tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
        tr181_param_name += "." + std::to_string(currentSource) + "." + "Format."+std::to_string(current_format)+"."+"PictureModeString";

        memset(&param, 0, sizeof(param));

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
        if ( err == tr181Success ) 
        {
            strncpy(picMode, param.value, strlen(param.value)+1);
            LOGINFO("getLocalParam success, mode = %s\n", picMode);
            return 1;
        }
        else 
	{
            LOGERR("getLocalParam failed");
            return 0;
        }
    }

    int AVOutputTV::syncCMSParams(std::string pqParam,tvCMS_tunel_t tunnel_type,std::string pqmode, std::string source, std::string format)
    {
        int ret=0;
	std::string cms_param;
        int params[3]={0};
        tvPQParameterIndex_t index;

        for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
        {
            cms_param = pqParam + ".";
	    cms_param += component_color[color];
            params[0]=tunnel_type;//tunnel_type
            params[1]=color;//color_type
            params[2]=0;//value

            if( COLOR_SATURATION == tunnel_type )
                index = PQ_PARAM_COMPONENT_SATURATION;
            else if(COLOR_HUE == tunnel_type )
                index = PQ_PARAM_COMPONENT_HUE;
            else
                index = PQ_PARAM_COMPONENT_LUMA;


            if(!updatePQParamsToCache("sync",cms_param,pqmode,source,format,index,params))
                ret |= 0;
            else
                ret |= 1;
        }
        return ret;
    }

    tvError_t AVOutputTV::syncCMSParamsToDriverCache(std::string pqmode, std::string source, std::string format)
    {
        int cms_enable[3]={0};
        cms_enable[0]=COLOR_STATE;//tunel_type
        cms_enable[1]=COLOR_ENABLE;//color_type
        cms_enable[2]=0;//value

        if(! updatePQParamsToCache("sync","cms.enable",pqmode,source,format,PQ_PARAM_SATURATION,cms_enable))
            LOGINFO("CMS Enable Flag  Successfully Synced to Drive Cache\n");
        else
            LOGERR("CMS Enable Flag Sync to cache Failed !!!\n");

        if( !syncCMSParams("saturation",COLOR_SATURATION,pqmode,source,format))
            LOGINFO("Component saturation Successfully Synced to Drive Cache\n");
        else
            LOGERR("Component saturation Sync to cache Failed !!!\n");

        if( !syncCMSParams("hue",COLOR_HUE,pqmode,source,format))
            LOGINFO("Component hue Successfully Synced to Drive Cache\n");
        else
            LOGERR("Component hue to cache Failed !!!\n");

        if( !syncCMSParams("luma",COLOR_LUMA,pqmode,source,format))
            LOGINFO("Component Luma Successfully Synced to Drive Cache\n");
        else
            LOGERR("Component Luma Sync to cache Failed !!!\n");

        return tvERROR_NONE;
    }

    void AVOutputTV::SyncWBparams(void)
    {

        if ( tvERROR_NONE != CheckWBMigration( ) )
            LOGWARN("%s: Migration Failed \n",__FUNCTION__);

        if( tvERROR_NONE != SyncWBFromLocalCache( ) )
            LOGWARN("%s: Sync WB Failed!!! \n",__FUNCTION__);
        else
            LOGINFO("%s: Sync WB Success!!! \n",__FUNCTION__);
    }

    tvError_t AVOutputTV::CheckWBMigration()
    {
        TR181_ParamData_t param={0};
        tvError_t ret = tvERROR_NONE;
        std::vector<std::string> allCtrls{ "gain", "offset"};
        std::vector<std::string> allColors{ "red", "green","blue"};

        LOGINFO("Entry");
        for(std::string color : allColors)
        {
            for(std::string ctrl : allCtrls)
            {
                std::string identifier=std::string(AVOUTPUT_GENERIC_STRING_RFC_PARAM)+std::string("wb");
                tr181ErrorCode_t err;
                identifier+=std::string(STRING_DIRTY)+color+"."+ctrl;
                err = getLocalParam(rfc_caller_id, identifier.c_str(), &param);
                if ( tr181Success == err )
                {
                    //set SDR
                    string key = std::string(AVOUTPUT_GENERIC_STRING_RFC_PARAM)+std::string("sdr.wb.")+color+"."+ctrl;
                    tr181ErrorCode_t err = setLocalParam(rfc_caller_id, key.c_str(), param.value);
                    if ( err != tr181Success )
                        LOGWARN("setLocalParam for %s Failed : %s\n", key.c_str(), getTR181ErrorString(err));
                    else
                        LOGINFO("setLocalParam for %s Successful, Value: %s\n", key.c_str(), param.value);

                    key.clear();
		    //set HDR
                    key = std::string(AVOUTPUT_GENERIC_STRING_RFC_PARAM)+std::string("hdr.wb.")+color+"."+ctrl;
                    err = setLocalParam(rfc_caller_id, key.c_str(), param.value);
                    if ( err != tr181Success )
                        LOGWARN("setLocalParam for %s Failed : %s\n", key.c_str(), getTR181ErrorString(err));
                    else
                        LOGINFO("setLocalParam for %s Successful, Value: %s\n", key.c_str(), param.value);

                    key.clear();

                    //clear old dirty values
                    err = clearLocalParam(rfc_caller_id,identifier.c_str());
                    if ( err != tr181Success ) {
                        LOGWARN("clearLocalParam for %s Failed : %s\n",identifier.c_str(),getTR181ErrorString(err));
                        ret  = tvERROR_GENERAL;
                    }

                    LOGINFO(" Migration completed \n");
                }
                else
                    LOGINFO("%s No old dirty values in local store - Migration not required\n",identifier.c_str());
            }
        }
        LOGINFO("Exit\n");
        return ret;
    }
   
    tvError_t AVOutputTV::SyncWBFromLocalCache( )
    {
        TR181_ParamData_t param={0};
        tvError_t ret = tvERROR_NONE;
        std::vector<std::string> allCtrls{ "gain", "offset"};
        std::vector<std::string> allColors{ "red", "green","blue"};
        std::vector<std::string> formats{ "sdr","hdr"};

        LOGINFO("Entry");
        for(std::string color : allColors)
        {
            for(std::string ctrl : allCtrls)
            {
                for(std::string format : formats)
                {
                    string identifier=std::string(AVOUTPUT_GENERIC_STRING_RFC_PARAM)+format+std::string(".wb.");
                    int value =0;
                    tr181ErrorCode_t err;
                    identifier+=color+"."+ctrl;
                    err = getLocalParam(rfc_caller_id, identifier.c_str(), &param);
                    if ( tr181Success == err )
                    {
                        value = std::stoi(param.value);
                        LOGINFO("%s  : %d\n",identifier.c_str(),value);
                        int rgbType = GetWBRgbType(color.c_str(),ctrl.c_str());
                        ret  = SaveColorTemperatureUser(rgbType,value,(strcmp(format.c_str(),"sdr") == 0)?0:1);
                        if(tvERROR_NONE!= ret)
                            LOGWARN("WB Entry for %s.%s.%s fail to save to driver\n",format.c_str(),color.c_str(),ctrl.c_str());
                        else
                            LOGINFO("WB Entry for %s.%s.%s=%d saved to driver\n",format.c_str(),color.c_str(),ctrl.c_str(),value);
                    }
		    else
                    {
                        LOGINFO("%s  Failed trying dirty\n",identifier.c_str());
                        ret = tvERROR_GENERAL;
                    }
                }
            }
        }
        return ret;
    }

    std::string AVOutputTV::convertSourceIndexToString(int sourceIndex)
    {
        std::string source="ip";//default
    
        switch(sourceIndex)
        {
            case 1:
                source="composite";
                break;
            case 5:
                source="hdmi1";
                break;
            case 6:
                source="hdmi2";
                break;
            case 7:
                source="hdmi3";
                break;
            case 10:
		source="ip";
		break;
	    case 11:
		source="tuner";
		break;
            default:
                break;
        }
        return source;
    }

    std::string AVOutputTV::convertVideoFormatToString( int formatIndex )
    {
        std::string format="sdr";//default
    
        switch(formatIndex)
        {
            case tvVideoHDRFormat_HLG:
                format="hlg";
                break;
            case tvVideoHDRFormat_HDR10:
                format="hdr10";
                break;
            case tvVideoHDRFormat_DV:
                format="dv";
                break;
            case tvVideoHDRFormat_HDR10PLUS:
                format="hdr10plus";
                break;
            case tvVideoHDRFormat_SDR:
            case tvVideoHDRFormat_NONE:
            default:
                format="sdr";
                break;
        }
        return format;
    }

    bool AVOutputTV::isIncluded(const std::set<string> set1,const std::set<string> set2)
    {
        for( const auto& element : set2){
            if(set1.find(element) == set1.end())
                return false;
        }
        return true;
    }

    void AVOutputTV::convertUserScaleBacklightToDriverScale(int format,int * params)
    {
        //Check the format if SDR use SDR array else use HDR array.
        int * arrayToCheck=( HDR_TYPE_SDR == format )?blInfo.sdrBLCurve:blInfo.hdrBLCurve;
        int backlightInCurve = arrayToCheck[params[0]];
        params[0] = (backlightInCurve*100)/BACKLIGHT_RAW_VALUE_MAX;
    }
    
    int AVOutputTV::getDolbyParamToSync(int sourceIndex, int formatIndex, int& value)
    {
        int ret=0;
        TR181_ParamData_t param;
        std ::string rfc_param = AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM + std::to_string(sourceIndex) + "."+"DolbyVisionMode";
        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, rfc_param.c_str(), &param);

        if ( tr181Success != err) {
            tvError_t retVal = GetDefaultParams(GetCurrentPQIndex(),sourceIndex, formatIndex, PQ_PARAM_DOLBY_MODE, &value);
            if( retVal != tvERROR_NONE )
            {
                LOGERR("%s : failed\n",__FUNCTION__);
                return -1;
            }
            ret = 0;
        }
        else
        {
            value=GetDolbyModeIndex(param.value);
            ret = 0;
        }

        return ret;
    }     

    int AVOutputTV::getHDR10ParamToSync(int& value)
    {
        int ret=0;
        TR181_ParamData_t param;

        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, AVOUTPUT_HDR10MODE_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            value=getHDR10ModeIndex(param.value);
        }
        else
        {
            LOGERR("Unable to fetch %s from localstore\n",AVOUTPUT_HDR10MODE_RFC_PARAM);
            ret=-1;
        }
        return ret;
    }

    int AVOutputTV::getHLGParamToSync(int& value)
    {
        int ret=0;
        TR181_ParamData_t param;

        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, AVOUTPUT_HLGMODE_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            value=getHLGModeIndex(param.value);
        }
        else
        {
            LOGERR("Unable to fetch %s from localstore\n",AVOUTPUT_HLGMODE_RFC_PARAM);
            ret=-1;
        }
        return ret;
    }

    int AVOutputTV::getHDR10ModeIndex(const char * hdr10Mode)
    {
        int mode = 0;
        pic_modes_t *hdr10Modes;
        unsigned short totalAvailable = 0;
    
        tvError_t ret = GetTVSupportedHDR10Modes(&hdr10Modes,&totalAvailable);
        if(ret == tvERROR_NONE) {
            for(int count = 0;count <totalAvailable;count++ )
            {
                if(strncmp(hdr10Mode, hdr10Modes[count].name, strlen(hdr10Mode))==0){
                    mode = hdr10Modes[count].value;
                    break;
                }
    
            }
        }else{
            mode = -1;
            LOGERR("(%s):get supported mode is failed\n", __func__);
        }
    
        return mode;
    }

    int AVOutputTV::getHLGModeIndex(const char * hlgMode)
    {
        int mode = 0;
        pic_modes_t *hlgModes;
        unsigned short totalAvailable = 0;
    
        tvError_t ret = GetTVSupportedHLGModes(&hlgModes,&totalAvailable);
        if(ret == tvERROR_NONE) {
            for(int count = 0;count <totalAvailable;count++ )
            {
                if(strncmp(hlgMode, hlgModes[count].name, strlen(hlgMode))==0){
                    mode = hlgModes[count].value;
                    break;
                }
    
            }
        }else{
            mode = -1;
            LOGERR("(%s):get supported mode is failed\n", __func__);
        }
    
        return mode;
    }

    void AVOutputTV::spliltStringsAndConvertToSet( std::string pqmodeInfo,std::string formatInfo,std::string sourceInfo,std::set<string> &pqmode, std::set<string> &format, std::set<string> &source)
    {
        std::string token;
        std::stringstream pqmodeStream(pqmodeInfo);
        std::stringstream formatStream(formatInfo);
        std::stringstream sourceStream(sourceInfo);
    
        while( getline(pqmodeStream,token,',') )
        {
            pqmode.insert( token );
            token.clear();
        }
    
        while( getline(formatStream,token,',') )
        {
            format.insert( token );
            token.clear();
        }
    
        while( getline(sourceStream,token,','))
        {
            source.insert( token );
            token.clear();
        }
    
        LOGINFO("All the Sets \n");
        LOGINFO("Range :\n");
    
        for( std::string param : pqmode )
            LOGINFO("%s \n", param.c_str());
    
        for( std::string param : format )
            LOGINFO("%s \n", param.c_str());
    
        for( std::string param : source )
            LOGINFO("%s \n",param.c_str());
    
        LOGINFO("\n");
    }

    std::string AVOutputTV::getDolbyModeStringFromEnum( tvDolbyMode_t mode)

    {
            std::string value;
            switch(mode)
            {
                case tvDolbyMode_Dark:
                case tvHDR10Mode_Dark:
                case tvHLGMode_Dark:
                        value = "dark";
                        break;
                case tvDolbyMode_Bright:
                case tvHDR10Mode_Bright:
                case tvHLGMode_Bright:
                        value = "bright";
                        break;
                default:
                        break;
            }
            return value;
    }

    int AVOutputTV::parsingSetInputArgument(const JsonObject& parameters, std::string pqparam, std::string & source,
                                             std::string & pqmode, std::string & format) {

        JsonArray sourceArray;
	JsonArray pqmodeArray;
	JsonArray formatArray;
		

	pqmodeArray = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].Array() : JsonArray();
        for (int i = 0; i < pqmodeArray.Length(); ++i) {
            pqmode += pqmodeArray[i].String();
            if (i != (pqmodeArray.Length() - 1) ) pqmode += ",";
	}
		
	sourceArray = parameters.HasLabel("videoSource") ? parameters["videoSource"].Array() : JsonArray();
        for (int i = 0; i < sourceArray.Length(); ++i) {
            source += sourceArray[i].String();
            if (i != (sourceArray.Length() - 1) ) source += ",";
	}

	formatArray = parameters.HasLabel("videoFormat") ? parameters["videoFormat"].Array() : JsonArray();
        for (int i = 0; i < formatArray.Length(); ++i) {
	    format += formatArray[i].String();
            if (i != (formatArray.Length() - 1) ) format += ",";
	}

	LOGINFO("%s source:[%s] pqmode[%s] format[%s]", __FUNCTION__,source.c_str(), pqmode.c_str(), format.c_str());

        if (source.empty()) source = "global";
        if (pqmode.empty()) pqmode = "global";
        if (format.empty()) format = "global";

        if (convertToValidInputParameter(pqparam, source, pqmode, format) != 0) {
            LOGERR("%s: Failed to convert the input paramters. \n", __FUNCTION__);
            return -1;
        }

	return 0;
    }

    int AVOutputTV::parsingGetInputArgument(const JsonObject& parameters, std::string pqparam,
                                         std::string & source, std::string & pqmode, std::string & format) {
        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";

        source = parameters.HasLabel("videoSource") ? parameters["videoSource"].String() : "";

        format = parameters.HasLabel("videoFormat") ? parameters["videoFormat"].String() : "";

        LOGINFO("%s source:[%s] pqmode[%s] format[%s]", __FUNCTION__,source.c_str(), pqmode.c_str(), format.c_str());

	if ( (source.compare("global") == 0) || (pqmode.compare("global") == 0) || (format.compare("global") == 0) )
	{
	    LOGERR("%s: get cannot fetch the global inputs \n", __FUNCTION__);
	    return -1;
	}

        if (source.empty()) source = "current";
        if (pqmode.empty()) pqmode = "current";
        if (format.empty()) format = "current";

        if (convertToValidInputParameter(pqparam,source, pqmode, format) != 0) {
            LOGERR("%s: Failed to convert the input paramters. \n", __FUNCTION__);
            return -1;
        }

        return 0;
    }

    int AVOutputTV::isPlatformSupport(std::string pqparam)
    {
        std::vector<std::string> range;
        std::vector<std::string> sourceVec;
        std::vector<std::string> pqmodeVec;
        std::vector<std::string> formatVec;
        std::string isPlatformSupport;

        tvError_t ret = getParamsCaps(range, pqmodeVec, sourceVec, formatVec, pqparam, isPlatformSupport);

        if (ret != tvERROR_NONE) {
           LOGINFO("%s: failed to get the capability \n", __FUNCTION__);
           return -1;
        }
        else
	{
	    if(isPlatformSupport.compare("true") != 0) {
	        LOGERR("%s: platform support not available\n", __FUNCTION__);
		return -1;
            }
        }
	return 0;
    }

    int AVOutputTV::FetchCapablities(string pqparam, string & source, string & pqmode, string & format) {

        std::vector<std::string> range;
        std::vector<std::string> sourceVec;
        std::vector<std::string> pqmodeVec;
        std::vector<std::string> formatVec;

       tvError_t ret = tvERROR_NONE;
       
       ret = getParamsCaps(range, pqmodeVec, sourceVec, formatVec, pqparam);

       if (ret != tvERROR_NONE) {
           LOGINFO("%s: failed to get the capability \n", __FUNCTION__);
           return -1;
       }

       if (sourceVec.size() != 0) {
            source = convertToString(sourceVec);
       }

       if (pqmodeVec.size() != 0) {
            pqmode = convertToString(pqmodeVec);
       }

       if (formatVec.size() != 0) {
            format = convertToString(formatVec);
       }
       return 0;
    }

    int AVOutputTV::getAvailableCapabilityModesWrapper(std::string & source, std::string & pqmode, std::string & format) {
        tvError_t err = tvERROR_NONE;
        std::vector<std::string> range;
        std::vector<std::string> picmodeVec;
        std::vector<std::string> sourceVec;
        std::vector<std::string> formatVec;

         err = getParamsCaps(range,picmodeVec,sourceVec,formatVec,"PictureMode");
         if (err != tvERROR_NONE) {
             LOGERR("%s: failed to get picture mode capability \n", __FUNCTION__);
             return -1;
         }
         pqmode = convertToString(range);

         if(!range.empty()) range.clear();
         err = getParamsCaps(range,picmodeVec,sourceVec,formatVec,"VideoSource");
         if (err != tvERROR_NONE) {
             LOGERR("%s: failed to get picture mode capability \n", __FUNCTION__);
             return -1;
         }
         source = convertToString(range);

         if(!range.empty()) range.clear();
         err = getParamsCaps(range,picmodeVec,sourceVec,formatVec,"VideoFormat");
         if (err != tvERROR_NONE) {
             LOGERR("%s: failed to get picture mode capability \n", __FUNCTION__);
             return -1;
         }
         format = convertToString(range);
	 return 0;
    }

    int AVOutputTV::getAvailableCapabilityModes(std::string & source, std::string & pqmode, std::string & format)
    {
        std::string localSource;
        std::string localPqmode;
        std::string localFormat;

        if ((pqmode.compare("none") == 0 ))
        {
            if (getAvailableCapabilityModesWrapper(localSource, pqmode, localFormat) != 0)
            {
                LOGERR("%s: failed to get picture mode capability \n", __FUNCTION__);
                return -1;
            }
        }

        if( (source.compare("none") == 0))
        {
            if (getAvailableCapabilityModesWrapper(source, localPqmode, localFormat) != 0) {
                LOGERR("%s: failed to get source mode capability \n", __FUNCTION__);
                return -1;
            }
        }

        if( (format.compare("none") == 0) )
        {
            if (getAvailableCapabilityModesWrapper(localSource, localPqmode, format) != 0) {
                LOGERR("%s: failed to get source mode capability \n", __FUNCTION__);
                return -1;
            }
        }
        return 0;
    }

}//namespace Plugin
}//namespace WPEFramework
