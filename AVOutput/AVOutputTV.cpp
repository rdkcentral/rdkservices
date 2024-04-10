/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2024 Sky UK
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
#include "UtilsIarm.h"
#include "rfcapi.h"

#define BUFFER_SIZE     (128)

#define registerMethod(...) for (uint8_t i = 1; GetHandler(i); i++) GetHandler(i)->Register<JsonObject, JsonObject>(__VA_ARGS__)

static bool filmMakerMode= false;
static std::map<std::string, int> supportedSourcemap;
static std::map<std::string, int> supportedPictureModemap;
static std::map<std::string, int> supportedFormatmap;
static bool m_isDalsEnabled = false;

namespace WPEFramework {
namespace Plugin {

    AVOutputTV* AVOutputTV::instance = nullptr;

    static void tvVideoFormatChangeHandler(tvVideoFormatType_t format, void *userData)
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

    static bool getVideoContentTypeToString(tvContentType_t content)
    {
        bool fmmMode = false;
        switch(content) 
	{
            case tvContentType_FMM:
                LOGINFO("Content Type: FMM\n");
                fmmMode = true;
                break;
            default:
                LOGINFO("Content Type: NONE\n");
                fmmMode = false;
                break;
        } 
        return fmmMode;
    }

    static const char *getVideoFormatTypeToString(tvVideoFormatType_t format)
    {
        const char *strValue = "NONE";
        switch(format) 
	{
            case VIDEO_FORMAT_SDR:
                strValue = "SDR";
                break;
            case VIDEO_FORMAT_HDR10:
                strValue = "HDR10";
                break;
            case VIDEO_FORMAT_HDR10PLUS:
                strValue = "HDR10PLUS";
                break;
            case VIDEO_FORMAT_HLG:
                strValue = "HLG";
                break;
            case VIDEO_FORMAT_DV:
                strValue = "DV";
                break;
            default:
                strValue = "NONE";
                break;
        }
	LOGINFO("Video Format:%s\n", strValue);
        return strValue;
    }

    static std::string getVideoResolutionTypeToString(tvResolutionParam_t resolution)
    {
        std::string strValue = "NONE";
        std::string interlaceValue = (resolution.isInterlaced) ? "i" : "p";
        if ( resolution.resolutionValue != tvVideoResolution_NONE ) 
	{
            strValue = std::to_string(resolution.frameWidth) + "*" + std::to_string(resolution.frameHeight) + interlaceValue;
        }
        LOGINFO("Video Resolution:[%s]\n", strValue.c_str());
        return strValue;
    }

    static const char *getVideoFrameRateTypeToString(tvVideoFrameRate_t frameRate)
    {
       const char *strValue = "NONE";
       switch(frameRate) 
       {
            case tvVideoFrameRate_24:
                strValue = "24";
                break;
            case tvVideoFrameRate_25:
                strValue = "25";
                break;
            case tvVideoFrameRate_30:
                strValue = "30";
                break;
            case tvVideoFrameRate_50:
                strValue = "50";
                break;
            case tvVideoFrameRate_60:
                strValue = "60";
                break;
            case tvVideoFrameRate_23dot98:
                strValue = "23.98";
                break;
            case tvVideoFrameRate_29dot97:
                strValue = "29.97";
                break;
            case tvVideoFrameRate_59dot94:
                strValue = "59.94";
                break;
            default:
                strValue = "NONE";
                break;

        }
        LOGINFO("Video FrameRate: %s\n",strValue);
        return strValue;
    }

    void AVOutputTV::NotifyVideoFormatChange(tvVideoFormatType_t format)
    {
        JsonObject response;
        response["currentVideoFormat"] = getVideoFormatTypeToString(format);
        sendNotify("onVideoFormatChanged", response);
    }

    void AVOutputTV::NotifyFilmMakerModeChange(tvContentType_t mode)
    {
        JsonObject response;
        JsonArray rangeArray;
	bool fmmMode;
	fmmMode = getVideoContentTypeToString(mode);
        response["filmMakerMode"] = fmmMode;

        if (getCapabilitySource(rangeArray) == 0)
        {
            response["filmMakerModeSources"] = rangeArray;
        }
        // cache for latest fmm mode
		filmMakerMode = fmmMode;
        sendNotify("onVideoContentChanged", response);
    }

    void AVOutputTV::NotifyVideoResolutionChange(tvResolutionParam_t resolution)
    {
        JsonObject response;
        response["currentVideoResolution"] = getVideoResolutionTypeToString(resolution);
        sendNotify("onVideoResolutionChanged", response);
    }

    void AVOutputTV::NotifyVideoFrameRateChange(tvVideoFrameRate_t frameRate)
    {
        JsonObject response;
        response["currentVideoFrameRate"] = getVideoFrameRateTypeToString(frameRate);
        sendNotify("onVideoFrameRateChanged", response);
    }

	//Event
    void AVOutputTV::dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if(!AVOutputTV::instance)
	    return;

	 if (IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS == eventId)
	 {
             IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
             int hdmi_in_port = eventData->data.hdmi_in_status.port;
             bool hdmi_in_status = eventData->data.hdmi_in_status.isPresented;
             LOGWARN("AVOutputPlugins: Received IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS  event	port: %d, started: %d", hdmi_in_port,hdmi_in_status);
	     if (!hdmi_in_status)
	     {
	         tvError_t ret = tvERROR_NONE;
		 AVOutputTV::instance->m_isDisabledHdmiIn4KZoom = false;
	         LOGWARN("AVOutputPlugins: Hdmi streaming stopped here reapply the global zoom settings:%d here. m_isDisabledHdmiIn4KZoom: %d", AVOutputTV::instance->m_videoZoomMode, AVOutputTV::instance->m_isDisabledHdmiIn4KZoom);
		 ret = SetAspectRatio((tvDisplayMode_t)AVOutputTV::instance->m_videoZoomMode);
		 if (ret != tvERROR_NONE) 
	         {
		     LOGWARN("SetAspectRatio set Failed");
		 }
	     }
	     else 
	     {
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
	    LOGWARN("AVOutputPlugins: Received IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE	event  port: %d, pixelResolution: %d, interlaced : %d, frameRate: %d \n", hdmi_in_port,resolution.pixelResolution, resolution.interlaced, resolution.frameRate);
	    if (AVOutputTV::instance->m_isDisabledHdmiIn4KZoom) 
	    {
                tvError_t ret = tvERROR_NONE;
		if (AVOutputTV::instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
				 (dsVIDEO_PIXELRES_MAX == AVOutputTV::instance->m_currentHdmiInResoluton))
	        {
		    LOGWARN("AVOutputPlugins: Setting %d zoom mode for below 4K", AVOutputTV::instance->m_videoZoomMode);
		    ret = SetAspectRatio((tvDisplayMode_t)AVOutputTV::instance->m_videoZoomMode);
		}
	        else 
	        {
		    LOGWARN("AVOutputPlugins: Setting auto zoom mode for 4K and above");
		    ret = SetAspectRatio(tvDisplayMode_AUTO);
	        }
		if (ret != tvERROR_NONE) 
	        {
		    LOGWARN("SetAspectRatio set Failed");
		}
	    } 
	    else 
	    {
	        LOGWARN("AVOutputPlugins: %s: HdmiInput is not started yet. m_isDisabledHdmiIn4KZoom: %d", __FUNCTION__, AVOutputTV::instance->m_isDisabledHdmiIn4KZoom);
	    }
        }
    }

    AVOutputTV::AVOutputTV(): m_currentHdmiInResoluton (dsVIDEO_PIXELRES_1920x1080)
                            , m_videoZoomMode (tvDisplayMode_NORMAL)
                            , m_isDisabledHdmiIn4KZoom (false)
	                    , rfc_caller_id()
    {
        LOGINFO("CTOR\n");
        AVOutputTV::instance = this;

		InitializeIARM();

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

		registerMethod("getBacklightDimmingMode", &AVOutputTV::getBacklightDimmingMode, this);
		registerMethod("setBacklightDimmingMode", &AVOutputTV::setBacklightDimmingMode, this);
		registerMethod("resetBacklightDimmingMode", &AVOutputTV::resetBacklightDimmingMode, this);
		registerMethod("getBacklightDimmingModeCaps", &AVOutputTV::getBacklightDimmingModeCaps, this);

		registerMethod("getSupportedDolbyVisionModes", &AVOutputTV::getSupportedDolbyVisionModes, this);
		registerMethod("getDolbyVisionMode", &AVOutputTV::getDolbyVisionMode, this);
		registerMethod("setDolbyVisionMode", &AVOutputTV::setDolbyVisionMode, this);
		registerMethod("resetDolbyVisionMode", &AVOutputTV::resetDolbyVisionMode, this);
		registerMethod("getDolbyVisionModeCaps", &AVOutputTV::getDolbyVisionModeCaps, this);
		registerMethod("getVideoFormat", &AVOutputTV::getVideoFormat, this);
		registerMethod("getVideoSource", &AVOutputTV::getVideoSource, this);
		registerMethod("getVideoFrameRate", &AVOutputTV::getVideoFrameRate, this);
		registerMethod("getVideoResolution", &AVOutputTV::getVideoResolution, this);
		registerMethod("getVideoContentType", &AVOutputTV::getVideoContentType, this);

		registerMethod("getZoomMode", &AVOutputTV::getZoomMode, this);
		registerMethod("setZoomMode", &AVOutputTV::setZoomMode, this);
		registerMethod("resetZoomMode", &AVOutputTV::resetZoomMode, this);
		registerMethod("getZoomModeCaps", &AVOutputTV::getZoomModeCaps, this);


		registerMethod("getPictureMode", &AVOutputTV::getPictureMode, this);
		registerMethod("setPictureMode", &AVOutputTV::setPictureMode, this);
		registerMethod("signalFilmMakerMode", &AVOutputTV::signalFilmMakerMode, this);
		registerMethod("resetPictureMode", &AVOutputTV::resetPictureMode, this);
		registerMethod("getPictureModeCaps", &AVOutputTV::getPictureModeCaps, this);
		registerMethod("getSupportedPictureModes", &AVOutputTV::getSupportedPictureModes, this);
		registerMethod("getVideoSourceCaps", &AVOutputTV::getVideoSourceCaps, this);
		registerMethod("getVideoFormatCaps", &AVOutputTV::getVideoFormatCaps, this);
		registerMethod("getVideoFrameRateCaps", &AVOutputTV::getVideoFrameRateCaps, this);
		registerMethod("getVideoResolutionCaps", &AVOutputTV::getVideoResolutionCaps, this);

		registerMethod("getLowLatencyState", &AVOutputTV::getLowLatencyState, this);
		registerMethod("setLowLatencyState", &AVOutputTV::setLowLatencyState, this);
		registerMethod("resetLowLatencyState", &AVOutputTV::resetLowLatencyState, this);
		registerMethod("getLowLatencyStateCaps", &AVOutputTV::getLowLatencyStateCaps, this);

        LOGINFO("Exit\n");
    }
    
    AVOutputTV :: ~AVOutputTV()
    {
        DeinitializeIARM();	
    }

    void AVOutputTV::Initialize()
    {
        LOGINFO("Entry\n");
       
        tvError_t ret = tvERROR_NONE;

        TR181_ParamData_t param;
        memset(&param, 0, sizeof(param));
        
        getDynamicAutoLatencyConfig();

        try {
            dsVideoPortResolution_t vidResolution;
            device::HdmiInput::getInstance().getCurrentVideoModeObj(vidResolution);
            m_currentHdmiInResoluton = vidResolution.pixelResolution;
        } 
	catch (...)
	{
            LOGWARN("AVOutputPlugins: getCurrentVideoModeObj failed");
        }
        LOGWARN("AVOutputPlugins: AVOutput Initialize m_currentHdmiInResoluton:%d m_mod:%d", m_currentHdmiInResoluton, m_videoZoomMode);

        ret = TvInit();
       
        if(ret != tvERROR_NONE) 
	{
            LOGERR("Platform Init failed, ret: %s \n", getErrorString(ret).c_str());
	} 
	else 
	{
            LOGINFO("Platform Init successful...\n");
            ret = TvSyncCalibrationInfoODM();
            if(ret != tvERROR_NONE) 
	    {
                LOGERR(" SD3 <->cri_data sync failed, ret: %s \n", getErrorString(ret).c_str());
            }
            else 
	    {
                LOGERR(" SD3 <->cri_data sync success, ret: %s \n", getErrorString(ret).c_str());
            }
        }

        tvVideoFormatCallbackData callbackData = {this,tvVideoFormatChangeHandler};
        ret = RegisterVideoFormatChangeCB(&callbackData);
        if(ret != tvERROR_NONE) {
            LOGWARN("RegisterVideoFormatChangeCB failed");
        }

        tvVideoContentCallbackData ConcallbackData = {this,tvFilmMakerModeChangeHandler};
        ret = RegisterVideoContentChangeCB(&ConcallbackData);
        if(ret != tvERROR_NONE) {
            LOGWARN("RegisterVideoContentChangeCB failed");
        }

        tvVideoResolutionCallbackData RescallbackData = {this,tvVideoResolutionChangeHandler};
        ret = RegisterVideoResolutionChangeCB(&RescallbackData);
        if(ret != tvERROR_NONE) {
            LOGWARN("RegisterVideoResolutionChangeCB failed");
        }

        tvVideoFrameRateCallbackData FpscallbackData = {this,tvVideoFrameRateChangeHandler};
        ret = RegisterVideoFrameRateChangeCB(&FpscallbackData);
        if(ret != tvERROR_NONE) {
            LOGWARN("RegisterVideoFrameRateChangeCB failed");
        }

        LocatePQSettingsFile();

	// Get Index from PQ capabailites
	if (getPqParamIndex() != 0)
        {
            LOGWARN("Failed to get the supported index from capability \n");
        }

        SyncAvoutputTVParamsToHAL("none","none","none");
	
        setDefaultAspectRatio();

        // source format specific sync to ssm data
        SyncAvoutputTVPQModeParamsToHAL("Current", "none", "none");

        // As we have source to picture mode mapping, get current source and
        // setting those picture mode
       InitializePictureMode();

        LOGINFO("Exit\n" );
    }

    void AVOutputTV::Deinitialize()
    {
       LOGINFO("Entry\n");

       tvError_t ret = tvERROR_NONE;
       ret = TvTerm();

       if(ret != tvERROR_NONE) 
       {
           LOGERR("Platform De-Init failed");
       }
       else
       {
           LOGINFO("Platform De-Init successful... \n");
       }

       LOGINFO("Exit\n");
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

        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
	else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["options"]=rangeArray;

            if (pqmode.front().compare("none") != 0) 
	    {
                for (index = 0; index < pqmode.size(); index++) 
		{
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) 
	    {
                for (index = 0; index < source.size(); index++) 
		{
                    sourceArray.Add(source[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((format.front()).compare("none") != 0) 
	    {
                for (index = 0; index < format.size(); index++) 
		{
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

        if (validateInputParameter("AspectRatio",value) != 0) 
	{
            LOGERR("%s: Range validation failed for AspectRatio\n", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "AspectRatio",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

	if( !isCapablityCheckPassed( pqmode, source, format, "AspectRatio" )) 
	{
            LOGERR("%s: CapablityCheck failed for AspectRatio\n", __FUNCTION__);
            returnResponse(false);
        }

        if(!value.compare("TV 16X9 STRETCH")) 
	{
            mode = tvDisplayMode_16x9;
        }
        else if (!value.compare("TV 4X3 PILLARBOX"))
	{
            mode = tvDisplayMode_4x3;
        }
        else if (!value.compare("TV NORMAL"))
	{
            mode = tvDisplayMode_NORMAL;
        }
        else if (!value.compare("TV DIRECT"))
	{
            mode = tvDisplayMode_DIRECT;
        }
        else if (!value.compare("TV AUTO"))
	{
            mode = tvDisplayMode_AUTO;
        }
        else if (!value.compare("TV ZOOM"))
	{
            mode = tvDisplayMode_ZOOM;
        }
        else 
	{
            returnResponse(false);
        }
        m_videoZoomMode = mode;
        tvError_t ret = setAspectRatioZoomSettings (mode);

        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else 
	{
            //Save DisplayMode to localstore and ssm_data
            int params[3]={0};
            params[0]=mode;
            int retval=UpdateAVoutputTVParam("set","AspectRatio",pqmode,source,format,PQ_PARAM_ASPECT_RATIO,params);;

            if(retval != 0) 
	    {
                LOGERR("Failed to Save DisplayMode to ssm_data\n");
		returnResponse(false);
            }

            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, AVOUTPUT_ASPECTRATIO_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) 
	    {
                LOGERR("setLocalParam for %s Failed : %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
		returnResponse(false);
            }
            else 
	    {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, value.c_str());
            }
            LOGINFO("Exit : SetAspectRatio() value : %s\n",value.c_str());
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getZoomMode(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        tvDisplayMode_t mode;

        tvError_t ret = getUserSelectedAspectRatio (&mode);

        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else 
	{
            switch(mode) 
	    {
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

	if( !isCapablityCheckPassed( pqmode, source, format, "AspectRatio" )) 
	{
            LOGERR("%s: CapablityCheck failed for AspectRatio\n", __FUNCTION__);
            returnResponse(false);
        }

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,AVOUTPUT_ASPECTRATIO_RFC_PARAM);
        if ( err != tr181Success ) 
	{
            LOGERR("clearLocalParam for %s Failed : %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else 
	{
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

}//namespace Plugin
}//namespace WPEFramework
//}
