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
        RegisterVideoFormatChangeCB(callbackData);

        tvVideoContentCallbackData ConcallbackData = {this,tvFilmMakerModeChangeHandler};
        RegisterVideoContentChangeCB(ConcallbackData);

        tvVideoResolutionCallbackData RescallbackData = {this,tvVideoResolutionChangeHandler};
        RegisterVideoResolutionChangeCB(RescallbackData);

        tvVideoFrameRateCallbackData FpscallbackData = {this,tvVideoFrameRateChangeHandler};
        RegisterVideoFrameRateChangeCB(FpscallbackData);

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

    tvError_t AVOutputTV::setDefaultAspectRatio(std::string pqmode,std::string  format,std::string source)
    {
        tvDisplayMode_t mode = tvDisplayMode_MAX;
        TR181_ParamData_t param;
        tvError_t ret = tvERROR_NONE;

        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, AVOUTPUT_ASPECTRATIO_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            if(!std::string(param.value).compare("16:9")) 
	    {
                mode = tvDisplayMode_16x9;
            }
            else if (!std::string(param.value).compare("4:3"))
	    {
                mode = tvDisplayMode_4x3;
            }
            else if (!std::string(param.value).compare("Full"))
	    {
                mode = tvDisplayMode_FULL;
            }
            else if (!std::string(param.value).compare("Normal"))
	    {
                mode = tvDisplayMode_NORMAL;
            }
            else if (!std::string(param.value).compare("TV AUTO"))
	    {
                mode = tvDisplayMode_AUTO;
            }
            else if (!std::string(param.value).compare("TV DIRECT"))
	    {
                mode = tvDisplayMode_DIRECT;
            }
	    else if (!std::string(param.value).compare("TV NORMAL"))
	    {
                mode = tvDisplayMode_NORMAL;
            }
            else if (!std::string(param.value).compare("TV ZOOM"))
	    {
                mode = tvDisplayMode_ZOOM;
            }
            else if (!std::string(param.value).compare("TV 16X9 STRETCH"))
	    {
                mode = tvDisplayMode_16x9;
            }
            else if (!std::string(param.value).compare("TV 4X3 PILLARBOX"))
	    {
                mode = tvDisplayMode_4x3;
            }
            else 
	    {
                mode = tvDisplayMode_AUTO;
            }

            m_videoZoomMode = mode;
            tvError_t ret = setAspectRatioZoomSettings (mode);

            if(ret != tvERROR_NONE) 
	    {
                LOGERR("AspectRatio  set failed: %s\n",getErrorString(ret).c_str());
            }
            else 
	    {
                //Save DisplayMode to ssm_data
                int params[3]={0};
                params[0]=mode;
                int retval=UpdateAVoutputTVParam("set","AspectRatio",pqmode,source,format,PQ_PARAM_ASPECT_RATIO,params);

                if(retval != 0) 
		{
                    LOGERR("Failed to Save DisplayMode to ssm_data\n");
		    ret = tvERROR_GENERAL;
                }
                LOGINFO("Aspect Ratio initialized successfully, value: %s\n", param.value);
            }

        }
	else
        {
            LOGERR("getLocalParam for %s Failed : %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            ret = tvERROR_GENERAL;
        }
        return ret;
    }

    uint32_t AVOutputTV::getVideoFormat(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvVideoFormatType_t videoFormat;
        tvError_t ret = GetCurrentVideoFormat(&videoFormat);
        if(ret != tvERROR_NONE) 
	{
            response["currentVideoFormat"] = "NONE";
            returnResponse(false);
        }
        else 
	{
            response["currentVideoFormat"] = getVideoFormatTypeToString(videoFormat);
            LOGINFO("Exit: getVideoFormat :%d   success \n",videoFormat);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getVideoResolution(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvResolutionParam_t videoResolution;
        tvError_t ret = GetCurrentVideoResolution(&videoResolution);
        if(ret != tvERROR_NONE) 
	{
            response["currentVideoResolution"] = "NONE";
            returnResponse(false);
        }
        else 
	{
            response["currentVideoResolution"] = getVideoResolutionTypeToString(videoResolution);
            LOGINFO("Exit: getVideoResolution :%d   success \n",videoResolution.resolutionValue);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getVideoFrameRate(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvVideoFrameRate_t videoFramerate;
        tvError_t ret = GetCurrentVideoFrameRate(&videoFramerate);
        if(ret != tvERROR_NONE) 
	{
            response["currentVideoFrameRate"] = "NONE";
            returnResponse(false);
        }
        else 
	{
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

        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) 
	{
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        err = getLocalparam("Backlight",formatIndex,pqIndex,sourceIndex,backlight, PQ_PARAM_BACKLIGHT);
        if( err == 0 ) 
	{
            response["backlight"] = backlight;
            LOGINFO("Exit : Backlight Value: %d \n", backlight);
            returnResponse(true);
        }
        else 
	{
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

        if (validateIntegerInputParameter("Backlight",backlight) != 0)
        {
            LOGERR("Failed in Backlight range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "Backlight",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("Backlight") != 0 ) returnResponse(false);

	if( !isCapablityCheckPassed( pqmode, source, format, "Backlight" )) 
	{
            LOGERR("%s: CapablityCheck failed for Backlight\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) 
	{
            LOGINFO("Proceed with setBacklight\n");
            ret = SetBacklight(backlight);
        }

        if(ret != tvERROR_NONE) 
	{
            LOGERR("Failed to set Backlight\n");
            returnResponse(false);
        }
        else 
	{
            int params[3]={0};
            params[0]=backlight;
            int retval= UpdateAVoutputTVParam("set","Backlight",pqmode,source,format,PQ_PARAM_BACKLIGHT,params);
            if(retval != 0 ) 
	    {
                LOGERR("Failed to Save Backlight to ssm_data\n");
		returnResponse(false);
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

        if( !isCapablityCheckPassed( pqmode, source, format, "Backlight" )) 
	{
            LOGERR("%s: CapablityCheck failed for Backlight\n", __FUNCTION__);
            returnResponse(false);
        }

	int retval= UpdateAVoutputTVParam("reset","Backlight",pqmode,source,format,PQ_PARAM_BACKLIGHT,params);
        if(retval != 0 )
	{
            LOGERR("Failed to reset Backlight\n");
            returnResponse(false);
        }
        else
	{
	    if (isSetRequired(pqmode,source,format))
	    {
                getParamIndex("Current","Current", "Current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("Backlight",formatIndex,pqIndex,sourceIndex,backlight, PQ_PARAM_BACKLIGHT);
                if( err == 0 )
		{
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,backlight);
                    ret = SetBacklight(backlight);
                }
                else
		{
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
		    ret = tvERROR_GENERAL;
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
        std::vector<std::string> indexInfo;

	JsonObject rangeObj;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"Backlight", isPlatformSupport, indexInfo);

        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else
        {
	    response["platformSupport"] = (isPlatformSupport.compare("true") == 0)  ? true : false;

            rangeObj["from"] = stoi(range[0]);
            rangeObj["to"] = stoi(range[1]);
            response["rangeInfo"]=rangeObj;

            if ((pqmode.front()).compare("none") != 0) {
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
            LOGERR("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) 
	{
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Brightness",formatIndex,pqIndex,sourceIndex,brightness, PQ_PARAM_BRIGHTNESS);
        if( err == 0 ) 
	{
            response["brightness"] = brightness;
            LOGINFO("Exit : Brightness Value: %d \n", brightness);
            returnResponse(true);
        }
        else 
	{
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

        if (validateIntegerInputParameter("Brightness",brightness) != 0)
        {
            LOGERR("Failed in Brightness range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "Brightness",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "Brightness" )) 
	{
            LOGERR("%s: CapablityCheck failed for Brightness\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) 
	{
             LOGINFO("Proceed with %s \n",__FUNCTION__);
             ret = SetBrightness(brightness);
        }

	if(ret != tvERROR_NONE) 
	{
            LOGERR("Failed to set Brightness\n");
            returnResponse(false);
        }
        else 
	{
            int params[3]={0};
            params[0]=brightness;
            int retval= UpdateAVoutputTVParam("set","Brightness",pqmode,source,format,PQ_PARAM_BRIGHTNESS,params);
            if(retval != 0 ) 
	    {
                LOGERR("Failed to Save Brightness to ssm_data\n");
		returnResponse(false);
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

	if( !isCapablityCheckPassed( pqmode, source, format, "Brightness" )) 
	{
            LOGERR("%s: CapablityCheck failed for Brightness\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= UpdateAVoutputTVParam("reset","Brightness",pqmode,source,format,PQ_PARAM_BRIGHTNESS,params);
        if(retval != 0 )
        {
            LOGWARN("Failed to reset Brightness\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("Current","Current", "Current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("Brightness",formatIndex,pqIndex,sourceIndex,brightness, PQ_PARAM_BRIGHTNESS);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,brightness);
                    ret = SetBrightness(brightness);
                }
                else
                {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
		    ret = tvERROR_GENERAL;
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

	if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else
        {
            rangeObj["from"] = stoi(range[0]);
            rangeObj["to"] = stoi(range[1]);
            response["rangeInfo"]=rangeObj;

            if ((pqmode.front()).compare("none") != 0) 
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

        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) 
	{
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Contrast",formatIndex,pqIndex,sourceIndex,contrast, PQ_PARAM_CONTRAST);
        if( err == 0 ) 
	{
            response["contrast"] = contrast;
            LOGINFO("Exit : Contrast Value: %d \n", contrast);
            returnResponse(true);
        }
        else 
	{
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

        if (validateIntegerInputParameter("Contrast", contrast) != 0)
        {
            LOGERR("Failed in contrast range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "Contrast",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "Contrast" )) 
	{
            LOGERR("%s: CapablityCheck failed for Contrast\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) 
	{
             LOGINFO("Proceed with %s \n",__FUNCTION__);
             ret = SetContrast(contrast);
        }

        if(ret != tvERROR_NONE) 
	{
            LOGERR("Failed to set Contrast\n");
            returnResponse(false);
        }
        else 
	{
            int params[3]={0};
            params[0]=contrast;
            int retval= UpdateAVoutputTVParam("set","Contrast",pqmode,source,format,PQ_PARAM_CONTRAST,params);
            if(retval != 0 ) 
	    {
                LOGERR("Failed to Save Contrast to ssm_data\n");
		returnResponse(false);
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

	if( !isCapablityCheckPassed( pqmode, source, format, "Contrast" )) 
	{
            LOGERR("%s: CapablityCheck failed for Contrast\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= UpdateAVoutputTVParam("reset","Contrast",pqmode,source,format,PQ_PARAM_CONTRAST,params);

        if(retval != 0 )
        {
            LOGWARN("Failed to reset Contrast\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("Current","Current", "Current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("Contrast",formatIndex,pqIndex,sourceIndex,contrast, PQ_PARAM_CONTRAST);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,contrast);
                    ret = SetContrast(contrast);
                }
                else
                {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
		    ret = tvERROR_GENERAL;
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

        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else
        {
            rangeObj["from"] = stoi(range[0]);
            rangeObj["to"] = stoi(range[1]);
            response["rangeInfo"]=rangeObj;
            
            if ((pqmode.front()).compare("none") != 0) 
	    {
                for (index = 0; index < pqmode.size(); index++) 
		{
                    pqmodeArray.Add(pqmode[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((source.front()).compare("none") != 0) {
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

        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) 
	{
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Saturation",formatIndex,pqIndex,sourceIndex,saturation, PQ_PARAM_SATURATION);
        if( err == 0 ) 
	{
            response["saturation"] = saturation;
            LOGINFO("Exit : Saturation Value: %d \n", saturation);
            returnResponse(true);
        }
        else 
	{
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

        if (validateIntegerInputParameter("Saturation",saturation) != 0)
        {
            LOGERR("Failed in saturation range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "Saturation",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }
 
        if( !isCapablityCheckPassed( pqmode, source, format, "Saturation" )) 
	{
            LOGERR("%s: CapablityCheck failed for Saturation\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) 
	{
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetSaturation(saturation);
        }

        if(ret != tvERROR_NONE) 
	{
            LOGERR("Failed to set Saturation\n");
            returnResponse(false);
        }
        else 
	{
            int params[3]={0};
            params[0]=saturation;
            int retval= UpdateAVoutputTVParam("set","Saturation",pqmode,source,format,PQ_PARAM_SATURATION,params);
            if(retval != 0 ) 
	    {
                LOGERR("Failed to Save Saturation to ssm_data\n");
		returnResponse(false);
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

        if( !isCapablityCheckPassed( pqmode, source, format, "Saturation" )) 
	{
            LOGERR("%s: CapablityCheck failed for Saturation\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= UpdateAVoutputTVParam("reset","Saturation",pqmode,source,format,PQ_PARAM_SATURATION,params);

        if(retval != 0 )
        {
            LOGERR("Failed to reset Saturation\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("Current","Current", "Current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("Saturation",formatIndex,pqIndex,sourceIndex, saturation, PQ_PARAM_SATURATION);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,saturation);
                    ret = SetSaturation(saturation);
                }
                else
                {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
		    ret = tvERROR_GENERAL;
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

        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else
        {
            rangeObj["from"] = stoi(range[0]);
            rangeObj["to"] = stoi(range[1]);
            response["rangeInfo"]=rangeObj;

            
	    if ((pqmode.front()).compare("none") != 0) 
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

        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) 
	{
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Sharpness",formatIndex,pqIndex,sourceIndex,sharpness, PQ_PARAM_SHARPNESS);
        if( err == 0 ) 
	{
            response["sharpness"] = sharpness;
            LOGINFO("Exit : Sharpness Value: %d \n", sharpness);
            returnResponse(true);
        }
        else 
	{
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

        if (validateIntegerInputParameter("Sharpness",sharpness) != 0)
        {
            LOGERR("Failed in sharpness range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "Sharpness",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }
 
        if( !isCapablityCheckPassed( pqmode, source, format, "Sharpness" )) 
	{
            LOGERR("%s: CapablityCheck failed for Sharpness\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) 
	{
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetSharpness(sharpness);
        }

        if(ret != tvERROR_NONE) 
	{
            LOGERR("Failed to set Sharpness\n");
            returnResponse(false);
        }
        else 
	{
            int params[3]={0};
            params[0]=sharpness;
            int retval= UpdateAVoutputTVParam("set","Sharpness",pqmode,source,format,PQ_PARAM_SHARPNESS,params);
            if(retval != 0 ) 
	    {
                LOGERR("Failed to Save Sharpness to ssm_data\n");
		returnResponse(false);
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

        if( !isCapablityCheckPassed( pqmode, source, format, "Sharpness" )) 
	{
            LOGERR("%s: CapablityCheck failed for Sharpness\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= UpdateAVoutputTVParam("reset","Sharpness",pqmode,source,format,PQ_PARAM_SHARPNESS,params);

        if(retval != 0 )
        {
            LOGERR("Failed to reset Sharpness\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("Current","Current", "Current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("Sharpness",formatIndex,pqIndex,sourceIndex, sharpness, PQ_PARAM_SHARPNESS);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,sharpness);
                    ret = SetSharpness(sharpness);
                }
                else
                {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
		    ret = tvERROR_GENERAL;
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

        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else
        {
            rangeObj["from"] = stoi(range[0]);
            rangeObj["to"] = stoi(range[1]);
            response["rangeInfo"]=rangeObj;
 
	    if ((pqmode.front()).compare("none") != 0) 
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

        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) 
	{
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Hue",formatIndex,pqIndex,sourceIndex,hue, PQ_PARAM_HUE);
        if( err == 0 ) 
	{
            response["hue"] = hue;
            LOGINFO("Exit : Hue Value: %d \n", hue);
            returnResponse(true);
        }
        else 
	{
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

        if (validateIntegerInputParameter("Hue",hue) != 0)
        {
            LOGERR("Failed in hue range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "Hue",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "Hue" )) 
	{
            LOGERR("%s: CapablityCheck failed for Hue\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) 
	{
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetHue(hue);
        }

        if(ret != tvERROR_NONE) 
	{
            LOGERR("Failed to set Hue\n");
            returnResponse(false);
        }
        else 
	{
            int params[3]={0};
            params[0]=hue;
            int retval= UpdateAVoutputTVParam("set","Hue",pqmode,source,format,PQ_PARAM_HUE,params);
            if(retval != 0 ) 
	    {
                LOGERR("Failed to Save Hue to ssm_data\n");
		returnResponse(false);
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

	if( !isCapablityCheckPassed( pqmode, source, format, "Hue" )) 
	{
            LOGERR("%s: CapablityCheck failed for Hue\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= UpdateAVoutputTVParam("reset","Hue",pqmode,source,format,PQ_PARAM_HUE,params);

        if(retval != 0 )
        {
            LOGERR("Failed to reset Hue\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("Current","Current", "Current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("Hue",formatIndex,pqIndex,sourceIndex, hue, PQ_PARAM_HUE);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,hue);
                    ret = SetHue(hue);
                }
                else
                {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
		    ret = tvERROR_GENERAL;
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

        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else
        {
            rangeObj["from"] = stoi(range[0]);
            rangeObj["to"] = stoi(range[1]);
            response["rangeInfo"]=rangeObj; 

            if ((pqmode.front()).compare("none") != 0) 
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

        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) 
	{
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("ColorTemp",formatIndex,pqIndex,sourceIndex,colortemp,PQ_PARAM_COLOR_TEMPERATURE);
        if( err == 0 ) 
	{
            switch(colortemp) 
	    {
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
        tvColorTemp_t colortemp = tvColorTemp_MAX;
        tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("colorTemperature") ? parameters["colorTemperature"].String() : "";
        returnIfParamNotFound(parameters,"colorTemperature");
        if(!value.compare("Standard")) 
	{
            colortemp = tvColorTemp_STANDARD;
        }
        else if (!value.compare("Warm"))
	{
            colortemp = tvColorTemp_WARM;
        }
        else if (!value.compare("Cold"))
	{
            colortemp = tvColorTemp_COLD;
        }
        else if (!value.compare("User Defined"))
	{
            colortemp = tvColorTemp_USER;
        }
        else 
	{
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "ColorTemperature",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "ColorTemperature" )) 
	{
            LOGERR("%s: CapablityCheck failed for colorTemperature\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) 
	{
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetColorTemperature((tvColorTemp_t)colortemp);
        }

        if(ret != tvERROR_NONE) 
	{
            LOGERR("Failed to set ColorTemperature\n");
            returnResponse(false);
        }
        else {
            int params[3]={0};
            params[0]=(int)colortemp;
            int retval= UpdateAVoutputTVParam("set","ColorTemp",pqmode,source,format,PQ_PARAM_COLOR_TEMPERATURE,params);
            if(retval != 0 ) 
	    {
                LOGERR("Failed to Save ColorTemperature to ssm_data\n");
		returnResponse(false);
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

        if( !isCapablityCheckPassed( pqmode, source, format, "ColorTemperature" )) 
	{
            LOGERR("%s: CapablityCheck failed for colorTemperature\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= UpdateAVoutputTVParam("reset","ColorTemp",pqmode,source,format,PQ_PARAM_COLOR_TEMPERATURE,params);

        if(retval != 0 )
        {
            LOGERR("Failed to reset ColorTemperature\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("Current","Current", "Current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("ColorTemp",formatIndex,pqIndex,sourceIndex, colortemp, PQ_PARAM_COLOR_TEMPERATURE);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex, colortemp);
                    ret = SetColorTemperature((tvColorTemp_t)colortemp);
                }
                else
                {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
		    ret = tvERROR_GENERAL;
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

            if (((pqmode.front()).compare("none") != 0)) 
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

        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) 
	{
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }


        int err = getLocalparam("DimmingMode",formatIndex,pqIndex,sourceIndex,dimmingMode, PQ_PARAM_DIMMINGMODE);
        if( err == 0 ) 
	{
            switch(dimmingMode) 
	    {
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
        else 
	{
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
        
        if (validateInputParameter("DimmingMode",value) != 0)
        {
            LOGERR("%s: Range validation failed for DimmingMode\n", __FUNCTION__);
            returnResponse(false);
        }
        dimmingMode = getDimmingModeIndex(value);

        if (parsingSetInputArgument(parameters, "DimmingMode",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "DimmingMode" )) 
	{
            LOGERR("%s: CapablityCheck failed for DimmingMode\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) 
	{
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetTVDimmingMode(value.c_str());
        }

	if(ret != tvERROR_NONE) 
	{
            LOGERR("Failed to set DimmingMode\n");
            returnResponse(false);
        }
        else 
	{
            int params[3]={0};
            params[0]=(int)dimmingMode;
            int retval= UpdateAVoutputTVParam("set","DimmingMode",pqmode,source,format,PQ_PARAM_DIMMINGMODE,params);
            if(retval != 0 ) 
	    {
                LOGERR("Failed to Save DimmingMode to ssm_data\n");
                returnResponse(false);
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

        if( !isCapablityCheckPassed( pqmode, source, format, "DimmingMode" )) 
	{
            LOGERR("%s: CapablityCheck failed for DimmingMode\n", __FUNCTION__);
            returnResponse(false);
        }

	int retval= UpdateAVoutputTVParam("reset","DimmingMode",pqmode,source,format,PQ_PARAM_DIMMINGMODE,params);

        if(retval != 0 )
        {
            LOGERR("Failed to reset ldim\n");
            returnResponse(false);
        }

        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("Current","Current", "Current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("DimmingMode",formatIndex,pqIndex,sourceIndex, dMode, PQ_PARAM_DIMMINGMODE);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex, dMode);
                    getDimmingModeStringFromEnum(dMode,dimmingMode);
                    ret = SetTVDimmingMode(dimmingMode.c_str());
                }
                else
                {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
		    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false);
        }
        else
        {
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

        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < supportedDimmingMode.size(); index++)
                supportedDimmingModeArray.Add(supportedDimmingMode[index]);

            response["options"]=supportedDimmingModeArray;
             
            if (((pqmode.front()).compare("none") != 0)) 
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

    uint32_t AVOutputTV::getSupportedDolbyVisionModes(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        pic_modes_t *dvModes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedDolbyVisionModesODM(&dvModes,&totalAvailable);
        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else 
	{
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

        // Since it is dolby vision mode, to should get only for dolby vision format
        format = "DV";

        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) 
	{
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        err = getLocalparam("DolbyVisionMode",formatIndex,pqIndex,sourceIndex,dolbyMode, PQ_PARAM_DOLBY_MODE);
        if( err == 0 ) 
	{
            response["dolbyVisionMode"] = getDolbyModeStringFromEnum((tvDolbyMode_t)dolbyMode);
            LOGINFO("Exit : DolbyVisionMode Value: %d \n", dolbyMode);
            returnResponse(true);
        }
        else 
	{
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

        if (validateInputParameter("DolbyVisionMode",value) != 0) 
	{
            LOGERR("%s: Range validation failed for DolbyVisionMode\n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( pqmode, source, format, "DolbyVisionMode" )) 
	{
            LOGERR("%s: CapablityCheck failed for DolbyVisionMode\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired("Current",source,"DV") ) 
	{
            LOGINFO("Proceed with setDolbyVisionMode\n\n");
            ret = SetTVDolbyVisionModeODM(value.c_str());
        }

        if(ret != tvERROR_NONE) 
	{
            LOGERR("Failed to set DolbyVisionMode\n\n");
            returnResponse(false);
        }
        else 
	{
            int params[3]={0};
            params[0]=GetDolbyModeIndex(value.c_str());
            format = "DV";
            int retval= UpdateAVoutputTVParam("set","DolbyVisionMode",pqmode,source,format,PQ_PARAM_DOLBY_MODE,params);
            if(retval != 0 ) 
	    {
                LOGERR("Failed to Save Dolbyvision mode\n");
		returnResponse(false);
            }
            LOGINFO("Exit : Dolbyvision successful to value: %s\n", value.c_str());
            returnResponse(true);
        }

    }

    int AVOutputTV::GetDolbyModeIndex(const char * dolbyMode)
    {
        int mode = 0;
        pic_modes_t *dolbyModes     ;
        unsigned short totalAvailable = 0;

        tvError_t ret = GetTVSupportedDolbyVisionModesODM(&dolbyModes,&totalAvailable);
        if(ret == tvERROR_NONE) {
            for(int count = 0;count <totalAvailable;count++ )
            {
                if(strncasecmp(dolbyMode, dolbyModes[count].name, strlen(dolbyMode))==0){
                    mode = dolbyModes[count].value;
                    break;
                }

            }
        }else{
            mode = -1;
            printf("(%s):get supported mode is failed\n", __func__);
        }

        return mode;
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

        if( !isCapablityCheckPassed( pqmode, source, format, "DolbyVisionMode" )) 
	{
            LOGERR("%s: CapablityCheck failed for DolbyVisionMode\n", __FUNCTION__);
            returnResponse(false);
        }
        
        format = "DV";
        int retval= UpdateAVoutputTVParam("reset","DolbyVisionMode",pqmode,source,format,PQ_PARAM_DOLBY_MODE,params);
        if(retval != 0 )
        {
            LOGERR("Failed to reset DolbyVisionMode\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired("Current",source,format))
            {
                getParamIndex("Current","Current", format,sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("DolbyVisionMode",formatIndex,pqIndex,sourceIndex, dolbyMode, PQ_PARAM_DOLBY_MODE);
                if( err == 0 )
                {
                    std::string dolbyModeValue = getDolbyModeStringFromEnum((tvDolbyMode_t)dolbyMode);
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d dolbyvalue : %s\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex, dolbyModeValue.c_str());
                    ret = SetTVDolbyVisionModeODM(dolbyModeValue.c_str());
                }
                else
                {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
		    ret = tvERROR_GENERAL;
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
        std::vector<std::string> indexInfo;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"DolbyVisionMode", isPlatformSupport, indexInfo);

        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else
        {
	   
	    response["platformSupport"] = (isPlatformSupport.compare("true") == 0 ) ? true : false;

            for (index = 0; index < range.size(); index++)
                rangeArray.Add(range[index]);

            response["options"]=rangeArray;
            if ((pqmode.front()).compare("none") != 0) 
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

    uint32_t AVOutputTV::getSupportedPictureModes(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        pic_modes_t *pictureModes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedPictureModes(&pictureModes,&totalAvailable);
        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else 
	{
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

    int AVOutputTV::getCapabilitySource(JsonArray & rangeArray)
    {
	std::vector<string> range,pqmode,source,format;

	tvError_t ret = getParamsCaps(range,pqmode,source,format,"VideoSource");

        if(ret != tvERROR_NONE) 
	{
            return -1;
        }
        else
        {
            if ((range.front()).compare("none") != 0) {
                for (unsigned int index = 0; index < range.size(); index++) 
		{
                    rangeArray.Add(range[index]);
                }
            }
        }
        return 0;
    }

    int AVOutputTV::getRangeCapability(std::string param, std::vector<std::string> & rangeInfo)
    {
        std::vector<string> range,pqmode,source,format;

        tvError_t ret = getParamsCaps(range,pqmode,source,format, param);

        if(ret != tvERROR_NONE) 
	{
            return -1;
        }
        else
        {
            if ((range.front()).compare("none") != 0) 
	    {
                rangeInfo = range;
            }
        }
        return 0;
    }

    uint32_t AVOutputTV::getVideoSourceCaps(const JsonObject& parameters, JsonObject& response) 
    {

        JsonArray rangeArray;

        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        if (getCapabilitySource(rangeArray) != 0)
        {
            returnResponse(false);
        }
        response["options"]=rangeArray;
        LOGINFO("Exit\n");
        returnResponse(true);
    }

    uint32_t AVOutputTV::getVideoFormatCaps(const JsonObject& parameters, JsonObject& response) 
    {

        JsonArray rangeArray;

        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"VideoFormat");

        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else
        {
            if ((range.front()).compare("none") != 0) 
	    {
                for (unsigned int index = 0; index < range.size(); index++) 
		{
                    rangeArray.Add(range[index]);
                }
                response["options"]=rangeArray;
            }
        }
        LOGINFO("Exit\n");
        returnResponse(true);
    }

    uint32_t AVOutputTV::getVideoFrameRateCaps(const JsonObject& parameters, JsonObject& response) 
    {
        LOGINFO("Entry\n");
        
        std::vector<std::string> rangeInfo;
        JsonArray rangeArray;
       
        if ( getRangeCapability("VideoFrameRate", rangeInfo) != 0 ) 
	{
            returnResponse(false);
        } 
        
        for (unsigned int index = 0; index < rangeInfo.size(); index++) 
	{
            rangeArray.Add(std::stof(rangeInfo[index]));
        }

        response["videoFrameRates"] = rangeArray;
        returnResponse(true);
    }

    uint32_t AVOutputTV::getVideoResolutionCaps(const JsonObject& parameters, JsonObject& response) 
    {
            LOGINFO("Entry\n");
            response["maxResolution"] = "4096*2160p";
            returnResponse(true);
    }

    uint32_t AVOutputTV::getPictureModeCaps(const JsonObject& parameters, JsonObject& response) 
    {

        JsonArray sourceArray;
        JsonArray formatArray;
        JsonArray rangeArray;
        
	std::vector<std::string> range;
	std::vector<std::string> source;
	std::vector<std::string> pqmode;
	std::vector<std::string> format;

	unsigned int index = 0;
        tvError_t ret = getParamsCaps(range,pqmode,source,format,"PictureMode");

        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else
        {
      
            if ((range.front()).compare("none") != 0) 
	    {
                for (index = 0; index < range.size(); index++) 
		{
                    rangeArray.Add(range[index]);
                }
                response["options"]=rangeArray;
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

        if (getParamIndex(source,dummyPqmode,format,current_source,pqIndex,current_format) == -1) 
	{
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
        tr181_param_name += "." + convertSourceIndexToString(current_source) + "." + "Format."+convertVideoFormatToString(current_format)+"."+"PictureModeString";
         err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);

         if ( tr181Success != err ) 
	 {
            returnResponse(false);
        }
        else 
	{
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

        if (validateInputParameter("PictureMode",value) != 0) 
	{
            LOGERR("%s: Range validation failed for PictureMode\n", __FUNCTION__);
            returnResponse(false);
        }
        if( !isCapablityCheckPassed( dummyPqmode, source,format, "PictureMode" )) 
	{
            LOGERR("%s: CapablityCheck failed for PictureMode\n", __FUNCTION__);
            returnResponse(false);
        }

	std::string local = value;
	transform(local.begin(), local.end(), local.begin(), ::tolower);
        if( isSetRequired("Current",source,format) ) 
	{
            LOGINFO("Proceed with SetTVPictureMode\n");
            ret = SetTVPictureMode(local.c_str());
         }
        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else 
	{
	    std::vector<int> pq_mode_vec;
	    std::vector<int> source_vec;
	    std::vector<int> format_vec;

	    getSaveConfig("Current", source.c_str(), format.c_str(), source_vec, pq_mode_vec, format_vec);
            
	    for (int sourceType : source_vec) 
	    {
                tvVideoSrcType_t source = (tvVideoSrcType_t)sourceType;
                for (int formatType : format_vec) 
		{
                    tvVideoFormatType_t format = (tvVideoFormatType_t)formatType;
                    std::string tr181_param_name = "";
                    tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
                    // framing Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.Source.source_index[x].Format.format_index[x].PictureModeString.value
                    tr181_param_name += "."+convertSourceIndexToString(source)+"."+"Format."+
			              convertVideoFormatToString(format)+"."+"PictureModeString";
                    tr181ErrorCode_t err = setLocalParam(rfc_caller_id, tr181_param_name.c_str(), value.c_str());
                    if ( err != tr181Success ) 
		    {
                        LOGERR("setLocalParam for %s Failed : %s\n", AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM, getTR181ErrorString(err));
			returnResponse(false);
                    }
                    else 
		    {
                        LOGINFO("setLocalParam for %s Successful, Value: %s\n", AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM, value.c_str());
		        int pqmodeindex = (int)getPictureModeIndex(value);
                        SaveSourcePictureMode(source, format, pqmodeindex);
		    }
                }
            }

	    //Filmmaker mode telemetry
            if(!strncmp(value.c_str(),"filmmaker",strlen(value.c_str())) && strncmp(prevmode,"filmmaker",strlen(prevmode)))
                LOGINFO("%s mode has been enabled",value.c_str());
            else if(!strncmp(prevmode,"filmmaker",strlen(prevmode)) && strncmp(value.c_str(),"filmmaker",strlen(value.c_str())))
                LOGINFO("%s mode has been disabled",prevmode);

            LOGINFO("Broadcasting the low latency change event \n");

	    if(m_isDalsEnabled)
            {
                //GameModebroadcast
                if(!strncmp(value.c_str(),"game",strlen(value.c_str())) && strncmp(prevmode,"game",strlen(prevmode)))
                    BroadcastLowLatencyModeChangeEvent(1);
                else if(!strncmp(prevmode,"game",strlen(prevmode)) && strncmp(value.c_str(),"game",strlen(value.c_str())))
                    BroadcastLowLatencyModeChangeEvent(0);
	    }

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

        if( !isCapablityCheckPassed( dummyPqmode, source,format, "PictureMode" )) 
	{
            LOGERR("%s: CapablityCheck failed for PictureMode\n", __FUNCTION__);
            returnResponse(false);
        }

        getSaveConfig("Current", source, format, source_vec, pq_mode_vec, format_vec);
 
        for (int source : source_vec) 
	{
            tvVideoSrcType_t sourceType = (tvVideoSrcType_t)source;
            for (int format : format_vec) 
	    {
                tvVideoFormatType_t formatType = (tvVideoFormatType_t)format;
                std::string tr181_param_name = "";
                tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
	        tr181_param_name += "."+convertSourceIndexToString(sourceType)+"."+"Format."+
			           convertVideoFormatToString(formatType)+"."+"PictureModeString";

       	        err = clearLocalParam(rfc_caller_id, tr181_param_name.c_str());
                if ( err != tr181Success ) 
		{
                    LOGWARN("clearLocalParam for %s Failed : %s\n", tr181_param_name.c_str(), getTR181ErrorString(err));
                    returnResponse(false);
                }
                else 
		{
                    err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
                    if ( tr181Success == err )
                    {
                        //get curren source and if matches save for that alone
		        tvVideoSrcType_t current_source = VIDEO_SOURCE_IP;
		        GetCurrentSource(&current_source);

			tvVideoFormatType_t current_format = VIDEO_FORMAT_NONE;
			GetCurrentVideoFormat(&current_format);
                        if( current_format == VIDEO_FORMAT_NONE) 
			{
                            current_format = VIDEO_FORMAT_SDR;
                        } 

			//as hal using lower across converting to lower
			std::string setparam = param.value;
			transform(setparam.begin(), setparam.end(), setparam.begin(), ::tolower);

		        if (current_source == sourceType && current_format == formatType) 
			{

                            tvError_t ret = SetTVPictureMode(setparam.c_str());
                            if(ret != tvERROR_NONE) 
			    {
                                LOGWARN("Picture Mode set failed: %s\n",getErrorString(ret).c_str());
                                returnResponse(false);
                            }
                            else 
			    {
                                LOGINFO("Exit : Picture Mode reset successfully, value: %s\n", param.value);
                            }
		        }
                        int pqmodeindex = (int)getPictureModeIndex(param.value);
                        SaveSourcePictureMode(sourceType, formatType, pqmodeindex);
                    }
                    else 
		    {
                        LOGWARN("getLocalParam for %s failed\n", AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
                        returnResponse(false);
                    }
                }
	    }
        }
	returnResponse(true)
    }

    uint32_t AVOutputTV::signalFilmMakerMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        char prevmode[PIC_MODE_NAME_MAX]={0};
        GetTVPictureMode(prevmode);

        value = parameters.HasLabel("signalFilmMakerMode") ? parameters["signalFilmMakerMode"].String() : "";
        returnIfParamNotFound(parameters, "signalFilmMakerMode");
	
	if(strncmp(value.c_str(),"ON",strlen(value.c_str())) == 0)
	{
		NotifyFilmMakerModeChange(tvContentType_FMM);
		LOGINFO(" enabling Film makermode \n");
	}
	else
	{
		LOGINFO(" disabling Film makermode \n");
		NotifyFilmMakerModeChange(tvContentType_NONE);
	}
	returnResponse(true);
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

        if (validateIntegerInputParameter("LowLatencyState",lowLatencyIndex) != 0)
        {
            LOGERR("Failed in Brightness range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "LowLatencyState",source, pqmode, format) != 0)
        {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed(pqmode, source, format, "LowLatencyState" )) 
	{
            LOGERR("%s: CapablityCheck failed for LowLatencyState\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(pqmode,source,format) ) 
	{
             LOGINFO("Proceed with setLowLatencyState\n");
             ret = SetLowLatencyState( lowLatencyIndex );
        }

        if(ret != tvERROR_NONE) 
	{
            LOGERR("Failed to setLowLatency\n");
            returnResponse(false);
        }
        else 
	{
            int params[3]={0};
            params[0]=lowLatencyIndex;
            int retval= UpdateAVoutputTVParam("set","LowLatencyState",pqmode,source,format,PQ_PARAM_LOWLATENCY_STATE,params);
            if(retval != 0 ) 
	    {
                LOGERR("Failed to SaveLowLatency to ssm_data\n");
		returnResponse(false);
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
        if (getParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex) == -1) 
	{
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("LowLatencyState",formatIndex,pqIndex,sourceIndex,lowlatencystate, PQ_PARAM_LOWLATENCY_STATE);
        if( err == 0 ) 
	{
            response["lowLatencyState"] = std::to_string(lowlatencystate);
            LOGINFO("Exit : LowLatencyState Value: %d \n", lowlatencystate);
            returnResponse(true);
        }
        else 
	{
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

        if( !isCapablityCheckPassed( pqmode, source, format, "LowLatencyState" )) 
	{
            LOGERR("%s: CapablityCheck failed for LowLatencyState\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= UpdateAVoutputTVParam("reset","LowLatencyState",pqmode,source,format,PQ_PARAM_LOWLATENCY_STATE,params);
        if(retval != 0 )
        {
            LOGERR("Failed to clear Lowlatency from ssmdata and localstore\n");
            returnResponse(false);
        }
        else
        {
            if (isSetRequired(pqmode,source,format))
            {
                getParamIndex("Current","Current", "Current",sourceIndex,pqIndex,formatIndex);
                int err = getLocalparam("LowLatencyState",formatIndex,pqIndex,sourceIndex, lowlatencystate, PQ_PARAM_LOWLATENCY_STATE);
                if( err == 0 )
                {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex, lowlatencystate);
                    ret = SetLowLatencyState(lowlatencystate);
                }
                else
                {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
		    ret = tvERROR_GENERAL;
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

        if(ret != tvERROR_NONE) 
	{
            returnResponse(false);
        }
        else
        {
            for (index = 0; index < range.size(); index++)
                rangeArray.Add(stoi(range[index]));

            response["LowLatencyInfo"]=rangeArray;
            if ((pqmode.front()).compare("none") != 0) 
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
  
    uint32_t AVOutputTV::getVideoSource(const JsonObject& parameters,JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;

        tvError_t ret = GetCurrentSource(&currentSource);
        if(ret != tvERROR_NONE) 
	{
            response["currentVideoSource"] = "NONE";
            returnResponse(false);
        }
        else 
	{
            response["currentVideoSource"] = convertSourceIndexToString(currentSource);
            LOGINFO("Exit: getVideoSource :%d   success \n", currentSource);
            returnResponse(true);
        }
    }


	uint32_t AVOutputTV::getVideoContentType(const JsonObject & parameters, JsonObject & response)
    {
        JsonArray rangeArray;

		response["currentFilmMakerMode"] = filmMakerMode;

        if (getCapabilitySource(rangeArray) == 0)
        {
            response["currentFilmMakerModeSources"] = rangeArray;
        }

		returnResponse(true);
    }

	/*********************************************************************************************************/
//Helper Function

    void AVOutputTV::LocatePQSettingsFile()
    {
        LOGINFO("Entry\n");
        char panelId[20] = {0};
        std::string PQFileName = AVOUTPUT_RFC_CALLERID;
        std::string FilePath = "/etc/rfcdefaults/";

        /* The if condition is to override the tvsettings ini file so it helps the PQ tuning process for new panels */
        if(access(AVOUTPUT_OVERRIDE_PATH, F_OK) == 0)
	{
            PQFileName = std::string(AVOUTPUT_RFC_CALLERID_OVERRIDE);
        }
	else
	{
            int val=GetPanelIDODM(panelId);
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
            else
	    {
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

    tvError_t AVOutputTV::SyncAvoutputTVParamsToHAL(std::string pqmode,std::string source,std::string format)
    {
        int params[3]={0};

        LOGINFO("Entry %s : pqmode : %s source : %s format : %s\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str());

        if( !UpdateAVoutputTVParam("sync","Brightness",pqmode,source,format,PQ_PARAM_BRIGHTNESS,params))
            LOGINFO("Brightness Successfully sync to Drive Cache\n");
        else
            LOGERR("Brightness Sync to cache Failed !!!\n");

        if( !UpdateAVoutputTVParam("sync","Contrast",pqmode,source,format,PQ_PARAM_CONTRAST,params))
            LOGINFO("Contrast Successfully Synced to Drive Cache\n");
        else
            LOGERR("Contrast Sync to cache Failed !!!\n");

        if( !UpdateAVoutputTVParam("sync","Sharpness",pqmode,source,format,PQ_PARAM_SHARPNESS,params))
            LOGINFO("Sharpness Successfully Synced to Drive Cache\n");
        else
            LOGERR("Sharpness Sync to cache Failed !!!\n");

        if( !UpdateAVoutputTVParam("sync","Saturation",pqmode,source,format,PQ_PARAM_SATURATION,params))
            LOGINFO("Saturation Successfully Synced to Drive Cache\n");
        else
            LOGERR("Saturation Sync to cache Failed !!!\n");

	if( !UpdateAVoutputTVParam("sync","Hue",pqmode,source,format,PQ_PARAM_HUE,params))
            LOGINFO("Hue Successfully Synced to Drive Cache\n");
        else
            LOGERR("Hue Sync to cache Failed !!!\n");

        if( !UpdateAVoutputTVParam("sync","ColorTemp",pqmode,source,format,PQ_PARAM_COLOR_TEMPERATURE,params))
            LOGINFO("ColorTemp Successfully Synced to Drive Cache\n");
        else
            LOGERR("ColorTemp Sync to cache Failed !!!\n");

        if( !UpdateAVoutputTVParam("sync","DolbyVisionMode",pqmode,source,"DV",PQ_PARAM_DOLBY_MODE,params))
            LOGINFO("dvmode Successfully Synced to Drive Cache\n");
        else
            LOGERR("dvmode Sync to cache Failed !!!\n");

        if( !UpdateAVoutputTVParam("sync","DimmingMode",pqmode,source,format,PQ_PARAM_DIMMINGMODE,params))
            LOGINFO("dimmingmode Successfully Synced to Drive Cache\n");
        else
            LOGERR("dimmingmode Sync to cache Failed !!!\n");

	if( !UpdateAVoutputTVParam("sync","Backlight",pqmode,source,format,PQ_PARAM_BACKLIGHT,params) )
            LOGINFO("Backlight Successfully Synced to Drive Cache\n");
        else
            LOGERR("Backlight Sync to cache Failed !!!\n");

        LOGINFO("Exit %s : pqmode : %s source : %s format : %s\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str());
        return tvERROR_NONE;
    }

    int AVOutputTV::SyncAvoutputTVPQModeParamsToHAL(std::string pqmode, std::string source, std::string format)
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
                tvVideoSrcType_t sourceType = (tvVideoSrcType_t)source;
                for (int format : formats)
                {
                    tvVideoFormatType_t formatType = (tvVideoFormatType_t)format;
                    std::string tr181_param_name = "";
                    tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
                    tr181_param_name += "."+convertSourceIndexToString(sourceType)+"."+"Format."+
                                         convertVideoFormatToString(formatType)+"."+"PictureModeString";

                    err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
                    if ( tr181Success == err ) 
                    {
			std::string local = param.value;
                        int pqmodeindex = (int)getPictureModeIndex(local);

                        tvError_t tv_err = SaveSourcePictureMode(sourceType, formatType, pqmodeindex);
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

        // converting pq to valid paramter format
        if (pqmode == "Global")
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
        }
        else if (pqmode == "Current")
        {
            char picMode[PIC_MODE_NAME_MAX]={0};
            if(!getCurrentPictureMode(picMode)) 
	    {
                LOGINFO("Failed to get the Current picture mode\n");
                return -1;
            } 
	    else 
            {
                pqmode = picMode;
            }
        }

        if (source == "Global")
        {
            std::string localSource;
            std::string localPqmode;
            std::string localFormat;
            if (FetchCapablities(pqparam, localSource, localPqmode, localFormat) == 0) 
	    {
                source = localSource;
            }
            else
            {
                LOGINFO("%s, Failed to get source capability\n", __FUNCTION__);
                return -1;
            }
        } 
        else if (source == "Current") 
        {
            tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;
            tvError_t ret = GetCurrentSource(&currentSource);
            
	    if(ret != tvERROR_NONE)
	    {
                LOGWARN("%s: GetCurrentSource( ) Failed \n",__FUNCTION__);
		return -1;
            }  
            source = convertSourceIndexToString(currentSource);
        }

        //convert format into valid parameter
        if (format == "Global")
        {
            std::string localSource;
            std::string localPqmode;
            std::string localFormat;
            if (FetchCapablities(pqparam, localSource, localPqmode, localFormat) == 0) 
	    {
                format = localFormat;
            }
            else
            {
                LOGINFO("%s, Failed to get format capability\n", __FUNCTION__);
                return -1;
            }
        }
	else if (format == "Current") 
	{
	  tvVideoFormatType_t formatIndex = VIDEO_FORMAT_NONE;
	  GetCurrentVideoFormat(&formatIndex);
	  if ( formatIndex  == VIDEO_FORMAT_NONE) formatIndex  = VIDEO_FORMAT_SDR;
          format = convertVideoFormatToString(formatIndex);
        }

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
        std::string indexInfo;
        std::vector<std::string> localIndex;

        if( ReadCapablitiesFromConfODM( rangeInfo, pqmodeInfo, formatInfo ,sourceInfo,param, platformsupport, indexInfo))
        {
            LOGERR( "%s: ReadCapablitiesFromConf Failed !!!\n",__FUNCTION__);
            return tvERROR_GENERAL;
        }
        else
        {
            spliltCapablities( range, pqmode, format, source, localIndex,rangeInfo, pqmodeInfo, formatInfo, sourceInfo , indexInfo);
        }
    
        return ret;
    }

    tvError_t AVOutputTV::getParamsCaps(std::vector<std::string> &range
                , std::vector<std::string> &pqmode, std::vector<std::string> &source, std::vector<std::string> &format,std::string param,
                 std::string & isPlatformSupport, std::vector<std::string> & index)
    {
        tvError_t ret = tvERROR_NONE;

        std::string rangeInfo;
        std::string sourceInfo;
        std::string formatInfo;
        std::string pqmodeInfo;
        std::string indexInfo;

        if( ReadCapablitiesFromConfODM( rangeInfo, pqmodeInfo, formatInfo ,sourceInfo,param, isPlatformSupport, indexInfo))
        {
            LOGERR( "%s: ReadCapablitiesFromConf Failed !!!\n",__FUNCTION__);
            return tvERROR_GENERAL;
        }
        else
        {
            spliltCapablities( range, pqmode, format, source, index,rangeInfo, pqmodeInfo, formatInfo, sourceInfo, indexInfo);
        }

        return ret;
    }

    void AVOutputTV::spliltCapablities( std::vector<std::string> &range,std::vector<std::string> &pqmode,std::vector<std::string> &format,
                                        std::vector<std::string> &source, std::vector<std::string> &index, std::string rangeInfo,
                                        std::string pqmodeInfo, std::string formatInfo, std::string sourceInfo, std::string indexInfo)
    {
        std::string token;
        std::stringstream rangeStream(rangeInfo);
        std::stringstream pqmodeStream(pqmodeInfo);
        std::stringstream formatStream(formatInfo);
        std::stringstream sourceStream(sourceInfo);
        std::stringstream indexStream(indexInfo);
    
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
    
        while( getline(indexStream,token,',') )
        {
            index.push_back( token );
            token.clear();
        }
    }

    bool AVOutputTV::isCapablityCheckPassed( std::string  pqmodeInputInfo,std::string sourceInputInfo,std::string formatInputInfo,std::string param )
    {

        std::string rangeCapInfo;
        std::string sourceCapInfo;
        std::string formatCapInfo;
        std::string pqmodeCapInfo;
	std::string isPlatformSupport;
	std::string indexInfo;

        std::set<string> pqmodeCapSet;
        std::set<string> formatCapSet;
        std::set<string> sourceCapset;
        std::set<string> pqmodeInputSet;
        std::set<string> formatInputSet;
        std::set<string> sourceInputSet;
        
        if( ReadCapablitiesFromConfODM( rangeCapInfo, pqmodeCapInfo, formatCapInfo, sourceCapInfo,param, isPlatformSupport, indexInfo) )
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

    int AVOutputTV::UpdateAVoutputTVParam( std::string action, std::string tr181ParamName, std::string pqmode, std::string source, std::string format, tvPQParameterIndex_t pqParamIndex, int params[] )
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
            for(int sourceType: sources)
            {
                tvVideoSrcType_t source = (tvVideoSrcType_t)sourceType;
                for(int mode : pictureModes)
                {
                    for(int formatType : formats)
                    {
                        tvVideoFormatType_t format = (tvVideoFormatType_t)formatType;
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
                                    ret |= UpdateAVoutputTVParamToHAL(tr181ParamName,source, mode, format,0,false);
                                if(sync || reset)
                                {
                                    int value=0;
                                    if(getLocalparam(tr181ParamName,format,mode,source,value,pqParamIndex,sync)) continue;
                                    params[0]=value;
                                }
                                if(set)
                                {
                                    ret |= UpdateAVoutputTVParamToHAL(tr181ParamName,source, mode, format, params[0],true);
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
                                ret |= SaveColorTemperature(source, mode,format,(tvColorTemp_t)params[0]);
                                break;
                            case PQ_PARAM_BACKLIGHT:
                                ret |= SaveBacklight(source, mode,format,params[0]);
                                break;
                            case PQ_PARAM_DIMMINGMODE:
                                ret |= SaveTVDimmingMode(source,mode,format,(tvDimmingMode_t)params[0]);
                                break;
                            case PQ_PARAM_LOWLATENCY_STATE:
                                ret |= SaveLowLatencyState(source, mode,format,params[0]);
                                break;
                            case PQ_PARAM_DOLBY_MODE:
                                 ret |= SaveTVDolbyVisionMode(source, mode,format,(tvDolbyMode_t)params[0]);
                                 break;

                             case PQ_PARAM_ASPECT_RATIO:
                                 ret |= SaveAspectRatio(source,mode,format,(tvDisplayMode_t)params[0]);
                                 break;
                             case PQ_PARAM_LOCALDIMMING_LEVEL:
			     {
                                 if(sync)
                                 {
                                     int value=0;
                                     getLocalparam(tr181ParamName,format,mode,source,value,pqParamIndex,sync);
                                     params[0]=value;
                                 }
                                 ret |= SaveTVDimmingMode(source, mode,format,(tvDimmingMode_t)params[0]);
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

    tvError_t AVOutputTV::UpdateAVoutputTVParamToHAL(std::string forParam, int source, int pqmode, int format, int value,bool setNotDelete)
    {
        tvError_t ret = tvERROR_NONE;
        std::string key;

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

            if ( err != tr181Success ) 
	    {
                LOGERR("%s for %s Failed : %s\n", setNotDelete?"Set":"Delete", key.c_str(), getTR181ErrorString(err));
                ret = tvERROR_GENERAL;
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
	//pqmode
        char *modeString = strdup(pqmode.c_str());
        char *token = NULL;
        while ((token = strtok_r(modeString,",",&modeString)))
        {
	    std::string local = token;
            picturemodes.push_back(getPictureModeIndex(local));
        }
        //source
        char *sourceString = strdup(source.c_str());
        char *sourceToken = NULL;
        while ((sourceToken = strtok_r(sourceString,",",&sourceString)))
        {
            std::string local = sourceToken;
            sources.push_back(getSourceIndex(local));
        }
        //3)check format
        char *formatString = strdup(format.c_str());
        char *formatToken = NULL;
        while ((formatToken = strtok_r(formatString,",",&formatString)))
        {
            std::string local = formatToken;
            formats.push_back(getFormatIndex(local));
        }

        LOGINFO("Exit : %s pqmode : %s source :%s format :%s ret:%d\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str(), ret);
        return ret;
    }

    string AVOutputTV::convertSourceIndexToString(int source)
    {
        std::string ret;
	std::map<std::string, int> :: iterator it;
	for (it = supportedSourcemap.begin(); it != supportedSourcemap.end(); it++)
        {
            if (it->second == source)
            {
                ret = it->first;
                break;
            }
        }
        return ret;
    }

    string AVOutputTV::convertVideoFormatToString(int format)
    {
        std::string ret;
        std::map<std::string, int> :: iterator it;
	for (it = supportedFormatmap.begin(); it != supportedFormatmap.end(); it++)
        {
            if (it->second == format)
            {
                ret = it->first;
                break;
            }
        }
        return ret;
    }

    string AVOutputTV::convertPictureIndexToString(int pqmode)
    {
        std::string ret;
        std::map<std::string, int> :: iterator it;
	for(it = supportedPictureModemap.begin(); it != supportedPictureModemap.end(); it++)
        {
            if (it->second == pqmode)
            {
                ret = it->first;
                break;
            }
        }
        return ret;
    }

    uint32_t AVOutputTV::generateStorageIdentifier(std::string &key, std::string forParam,int contentFormat, int pqmode, int source)
    {
        key+=std::string(AVOUTPUT_GENERIC_STRING_RFC_PARAM);
        key+=STRING_SOURCE+convertSourceIndexToString(source)+std::string(".")+STRING_PICMODE+convertPictureIndexToString(pqmode)+std::string(".")+std::string(STRING_FORMAT)+convertVideoFormatToString(contentFormat)+std::string(".")+forParam;
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
        TR181_ParamData_t param={0};
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
	       if (strncmp(param.value, "Dark", strlen(param.value)) == 0) {
	           value = tvDolbyMode_Dark; 
               }
	       else 
	       {
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
                return 1;
            }
            GetDefaultPQParams(pqIndex,(tvVideoSrcType_t)sourceIndex,(tvVideoFormatType_t)formatIndex,pqParamIndex,&value);
            LOGINFO("Default value from DB : %s : %d \n",key.c_str(),value);
            return 0;
        }
    }

    bool AVOutputTV::isSetRequired(std::string pqmode,std::string source,std::string format)
    {
        bool ret=false;
        char picMode[PIC_MODE_NAME_MAX]={0};
	tvError_t retVal = tvERROR_NONE;
	tvVideoSrcType_t sourceIndex = VIDEO_SOURCE_IP;
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
	tvVideoFormatType_t formatIndex = VIDEO_FORMAT_NONE;
	GetCurrentVideoFormat(&formatIndex);
	if ( formatIndex  == VIDEO_FORMAT_NONE) formatIndex = VIDEO_FORMAT_SDR;
        currentFormat = convertVideoFormatToString(formatIndex);


        if( ( (pqmode.find(currentPicMode) != std::string::npos) || (pqmode.compare("Global") == 0)  || (pqmode.compare("Current") == 0) ||
            (pqmode.compare("none") == 0) ) &&
           ((source.find(currentSource) != std::string::npos)  || (source.compare("Global") == 0) || (source.compare("Current") == 0) ||
            (source.compare("none") == 0) ) &&
           ( (format.find(currentFormat) !=  std::string::npos) || (format.compare("Global") == 0) || (format.compare("Current") == 0) ||
             (format.compare("none") == 0) ) )
            ret=true;

        return ret;
    }

    int AVOutputTV::getParamIndex(string source,string pqmode,string format,int& sourceIndex,int& pqmodeIndex,int& formatIndex)
    {
	LOGINFO("Entry : %s pqmode : %s source :%s format :%s\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str());

        if( source.compare("none") == 0 || source.compare("Current") == 0 )
        {
	    tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;
            GetCurrentSource(&currentSource);
	    sourceIndex = (int)currentSource;
        }
        else
        {
            sourceIndex = getSourceIndex(source);
        }
        if( pqmode.compare("none") == 0 || pqmode.compare("Current") == 0)
        {
            char picMode[PIC_MODE_NAME_MAX]={0};
            if(!getCurrentPictureMode(picMode))
	    {
                LOGERR("Failed to get the Current picture mode\n");
	    }
            else
	    {
		std::string local = picMode;
                pqmodeIndex = getPictureModeIndex(local);
	    }
        }
        else
        {
            pqmodeIndex = getPictureModeIndex(pqmode);
        }

        if( format.compare("none") == 0 || format.compare("Current") == 0)
        {
	    tvVideoFormatType_t currentFormat = VIDEO_FORMAT_NONE;
            GetCurrentVideoFormat(&currentFormat);
	    if( VIDEO_FORMAT_NONE == currentFormat )
                formatIndex = VIDEO_FORMAT_SDR;
	    else
	        formatIndex = (int)currentFormat;
        }
        else
        {
            formatIndex = getFormatIndex(format);
        }

        if (sourceIndex == -1 || pqmodeIndex == -1 || formatIndex == -1) return -1;
        LOGINFO("%s: Exit sourceIndex = %d pqmodeIndex = %d formatIndex = %d\n",__FUNCTION__,sourceIndex,pqmodeIndex,formatIndex);

	return 0;
    }

    tvDataComponentColor_t AVOutputTV::getComponentColorEnum(std::string colorName)
    {
        tvDataComponentColor_t CompColorEnum = tvDataColor_MAX;

        if(!colorName.compare("none")) 
	{
            CompColorEnum = tvDataColor_NONE;
        }
        else if (!colorName.compare("red"))
	{
            CompColorEnum = tvDataColor_RED;
        }
        else if (!colorName.compare("green"))
	{
            CompColorEnum = tvDataColor_GREEN;
        }
        else if (!colorName.compare("blue"))
	{
            CompColorEnum = tvDataColor_BLUE;
        }
        else if (!colorName.compare("yellow"))
	{
            CompColorEnum = tvDataColor_YELLOW;
        }
        else if (!colorName.compare("cyan"))
	{
            CompColorEnum = tvDataColor_CYAN;
        }
        else if (!colorName.compare("magenta"))
	{
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
        tvVideoSrcType_t sourceIndex = VIDEO_SOURCE_IP;
        /*Since dolby vision is source specific, we should for check for specific source*/
        if (!source.empty()) 
	{
            sourceIndex = (tvVideoSrcType_t)getSourceIndex(source);
	} 
	else 
	{
            GetCurrentSource(&sourceIndex);
        }

        char picMode[PIC_MODE_NAME_MAX]={0};
        int pqmodeIndex = 0;
        if(!getCurrentPictureMode(picMode))
        {
            LOGERR("Failed to get the Current picture mode\n");
        }
        else
        {
            std::string local = picMode;
            pqmodeIndex = getPictureModeIndex(local);
        }
        memset(&param, 0, sizeof(param));
        if (format == tvContentFormatType_HLG ) 
	{
            rfc_param = AVOUTPUT_HLGMODE_RFC_PARAM;
        } 
	else if (format == tvContentFormatType_DOVI) 
	{
            rfc_param = AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM + std::to_string(sourceIndex) + "."+"DolbyVisionMode";
        }

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, rfc_param.c_str(), &param);
        if ( tr181Success != err) 
	{
            tvError_t retVal = GetDefaultPQParams(pqmodeIndex,(tvVideoSrcType_t)sourceIndex,
			                         (tvVideoFormatType_t)ConvertHDRFormatToContentFormatODM((tvhdr_type_t)format),
						 PQ_PARAM_DOLBY_MODE,&dolby_mode_value);
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

    tvDimmingMode_t AVOutputTV::getDimmingModeIndex(std::string mode)
    {
        tvDimmingMode_t index = tvDimmingMode_MAX;

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

    int AVOutputTV::getCurrentPictureMode(char *picMode)
    {
        tvError_t  ret = tvERROR_NONE;
        TR181_ParamData_t param;
        std::string tr181_param_name;
        tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;

        ret = GetCurrentSource(&currentSource);
        if(ret != tvERROR_NONE)
        {
            LOGERR("GetCurrentSource() Failed set source to default\n");
	    return 0;
        }

	tvVideoFormatType_t current_format = VIDEO_FORMAT_NONE;
        GetCurrentVideoFormat(&current_format);
        if ( current_format  == VIDEO_FORMAT_NONE) current_format  = VIDEO_FORMAT_SDR;

        tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
        tr181_param_name += "." + convertSourceIndexToString(currentSource) + "." + "Format."+convertVideoFormatToString(current_format)+"."+"PictureModeString";

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

    bool AVOutputTV::isIncluded(const std::set<string> set1,const std::set<string> set2)
    {
        for( const auto& element : set2){
            if(set1.find(element) == set1.end())
                return false;
        }
        return true;
    }

    int AVOutputTV::getDolbyParamToSync(int sourceIndex, int formatIndex, int& value)
    {
        int ret=0;
        TR181_ParamData_t param;
        int pqmodeIndex = 0;
        char picMode[PIC_MODE_NAME_MAX]={0};
        if(!getCurrentPictureMode(picMode))
        {
            LOGERR("Failed to get the Current picture mode\n");
        }
        else
        {
            std::string local = picMode;
            pqmodeIndex = getPictureModeIndex(local);
        }
        std ::string rfc_param = AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM + std::to_string(sourceIndex) + "."+"DolbyVisionMode";
        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, rfc_param.c_str(), &param);

        if ( tr181Success != err) 
	{
            tvError_t retVal = GetDefaultPQParams(pqmodeIndex,(tvVideoSrcType_t)sourceIndex, (tvVideoFormatType_t)formatIndex,
			                        PQ_PARAM_DOLBY_MODE, &value);
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
    }

    std::string AVOutputTV::getDolbyModeStringFromEnum( tvDolbyMode_t mode)

    {
            std::string value;
            switch(mode)
            {
                case tvDolbyMode_Dark:
                case tvHDR10Mode_Dark:
                case tvHLGMode_Dark:
                        value = "Dark";
                        break;
                case tvDolbyMode_Bright:
                case tvHDR10Mode_Bright:
                case tvHLGMode_Bright:
                        value = "Bright";
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
        for (int i = 0; i < pqmodeArray.Length(); ++i) 
	{
            pqmode += pqmodeArray[i].String();
            if (i != (pqmodeArray.Length() - 1) ) pqmode += ",";
	}
		
	sourceArray = parameters.HasLabel("videoSource") ? parameters["videoSource"].Array() : JsonArray();
        for (int i = 0; i < sourceArray.Length(); ++i) 
	{
            source += sourceArray[i].String();
            if (i != (sourceArray.Length() - 1) ) source += ",";
	}

	formatArray = parameters.HasLabel("videoFormat") ? parameters["videoFormat"].Array() : JsonArray();
        for (int i = 0; i < formatArray.Length(); ++i) 
	{
	    format += formatArray[i].String();
            if (i != (formatArray.Length() - 1) ) format += ",";
	}

        if (source.empty()) source = "Global";
        if (pqmode.empty()) pqmode = "Global";
        if (format.empty()) format = "Global";

        if (convertToValidInputParameter(pqparam, source, pqmode, format) != 0) 
	{
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

	if ( (source.compare("Global") == 0) || (pqmode.compare("Global") == 0) || (format.compare("Global") == 0) )
	{
	    LOGERR("%s: get cannot fetch the Global inputs \n", __FUNCTION__);
	    return -1;
	}

        if (source.empty()) source = "Current";
        if (pqmode.empty()) pqmode = "Current";
        if (format.empty()) format = "Current";

        if (convertToValidInputParameter(pqparam,source, pqmode, format) != 0) 
	{
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
        std::vector<std::string> index;

        tvError_t ret = getParamsCaps(range, pqmodeVec, sourceVec, formatVec, pqparam, isPlatformSupport, index);

        if (ret != tvERROR_NONE) 
	{
           LOGINFO("%s: failed to get the capability \n", __FUNCTION__);
           return -1;
        }
        else
	{
	    if(isPlatformSupport.compare("true") != 0) 
	    {
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

       if (ret != tvERROR_NONE) 
       {
           LOGINFO("%s: failed to get the capability \n", __FUNCTION__);
           return -1;
       }

       if (sourceVec.size() != 0) 
       {
            source = convertToString(sourceVec);
       }

       if (pqmodeVec.size() != 0) 
       {
            pqmode = convertToString(pqmodeVec);
       }

       if (formatVec.size() != 0) 
       {
            format = convertToString(formatVec);
       }

       return 0;
    }

    int AVOutputTV::getAvailableCapabilityModesWrapper(std::string param, std::string & outparam)
    {
        tvError_t err = tvERROR_NONE;
        std::vector<std::string> range;
        std::vector<std::string> picmodeVec;
        std::vector<std::string> sourceVec;
        std::vector<std::string> formatVec;

         err = getParamsCaps(range,picmodeVec,sourceVec,formatVec, param);
         if (err != tvERROR_NONE) 
	 {
             LOGERR("%s: failed to get [%s] capability \n", __FUNCTION__, param.c_str());
             return -1;
         }
         outparam = convertToString(range);

	 return 0;
    }

    int AVOutputTV::getAvailableCapabilityModes(std::string & source, std::string & pqmode, std::string & format)
    {
        if ((pqmode.compare("none") == 0 ))
        {
            if (getAvailableCapabilityModesWrapper("PictureMode", pqmode) != 0)
            {
                LOGERR("%s: failed to get picture mode capability \n", __FUNCTION__);
                return -1;
            }
        }

        if( (source.compare("none") == 0))
        {
            if (getAvailableCapabilityModesWrapper("VideoSource",source) != 0) 
	    {
                LOGERR("%s: failed to get source capability \n", __FUNCTION__);
                return -1;
            }
        }

        if( (format.compare("none") == 0) )
        {
            if (getAvailableCapabilityModesWrapper("VideoFormat",format) != 0) 
	    {
                LOGERR("%s: failed to get format capability \n", __FUNCTION__);
                return -1;
            }
        }
        return 0;
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

	    int AVOutputTV::validateInputParameter(std::string param, std::string inputValue) 
    {

        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        tvError_t ret = getParamsCaps(range, pqmode, source, format, param);

        if (ret != tvERROR_NONE) 
	{
            LOGERR("Failed to fetch the range capability[%s] \n", param.c_str());
            return -1;
        }

        if ( (param == "ColorTemperature") ||
             (param == "DimmingMode") || (param == "AutoBacklightControl") ||
             (param == "DolbyVisionMode") || (param == "HDR10Mode") ||
            (param == "HLGMode") || (param == "AspectRatio") || (param == "PictureMode") )
        {
	    auto iter = find(range.begin(), range.end(), inputValue);

            if (iter == range.end()) 
	    {
		LOGERR("Not a valid input value[%s].\n", inputValue.c_str());
                return -1;
            }
        }
        return 0;
    }

    int AVOutputTV::validateIntegerInputParameter(std::string param, int inputValue) 
    {

        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        tvError_t ret = getParamsCaps(range, pqmode, source, format, param);

        if (ret != tvERROR_NONE) 
	{
            LOGERR("Failed to fetch the range capability[%s] \n", param.c_str());
            return -1;
        }

        if ( (param == "Brightness") || (param == "Contrast") ||
             (param == "Sharpness") || (param == "Saturation") ||
             (param == "Hue") || (param == "WhiteBalance") ||
            (param == "ComponentSaturation") || (param == "Backlight") ||
            (param == "ComponentHue") || (param == "ComponentLuma") || (param == "LowLatencyState") )
        {
            if (inputValue < stoi(range[0]) || inputValue > stoi(range[1]))
            {
                LOGERR("wrong Input value[%d]", inputValue);
                return -1;
            }
        }
        return 0;
    }

    void AVOutputTV::BroadcastLowLatencyModeChangeEvent(bool lowLatencyMode)
    {
           LOGINFO("Entry:%d\n",lowLatencyMode);
     	   JsonObject response;
    	   response["lowLatencyMode"] = lowLatencyMode;
      	   sendNotify("gameModeEvent", response);
    }

	int AVOutputTV::getPqParamIndex()
    {
        std::vector<std::string> localpq;
        std::vector<std::string> localformat;
        std::vector<std::string> localsource;
        std::vector<std::string> localrange;
        std::string platformsupport;
        std::vector<std::string> index;

        tvError_t ret = getParamsCaps(localrange, localpq, localformat, localsource,
                                          "VideoSource", platformsupport, index);
        if (ret == tvERROR_NONE)
        {
            if (localrange.size() == index.size())
            {
                for (unsigned int i = 0; i< localrange.size(); i++)
                {
                    supportedSourcemap[localrange[i]] = stoi(index[i]);
                }
            }
        }
        else
        {
            LOGERR("%s: Failed to fetch the source index \n", __FUNCTION__);
            return -1;
        }
       
        if (!localpq.empty()) localpq.clear();
        if (!localformat.empty()) localformat.clear();
        if (!localsource.empty()) localsource.clear();
        if (!localrange.empty()) localrange.clear();
        if(!index.empty()) index.clear();

        ret = getParamsCaps(localrange, localpq, localformat, localsource,
                                      "PictureMode", platformsupport, index);
        if (ret == tvERROR_NONE)
        {
            if (localrange.size() == index.size())
            {
                for (unsigned int i = 0; i< localrange.size(); i++)
                {
                    supportedPictureModemap[localrange[i]] = stoi(index[i]);
                }
            }
        }
        else
        {
            LOGERR("%s: Failed to fetch the picture index \n", __FUNCTION__);
            return -1;
        }

        if (!localpq.empty()) localpq.clear();
        if (!localformat.empty()) localformat.clear();
        if (!localsource.empty()) localsource.clear();
        if (!localrange.empty()) localrange.clear();
        if(!index.empty()) index.clear();

        ret = getParamsCaps(localrange, localpq, localformat, localsource,
                                      "VideoFormat", platformsupport, index);
        if (ret == tvERROR_NONE)
        {
            if (localrange.size() == index.size())
            {
                for (unsigned int i = 0; i< localrange.size(); i++)
                {
                    supportedFormatmap[localrange[i]] = stoi(index[i]);
                }
            }
        }
        else
        {
            LOGERR("%s: Failed to fetch the format index \n", __FUNCTION__);
            return -1;
        }

        return 0;
    }

    int AVOutputTV::getPictureModeIndex(std::string pqparam)
    {
	int index = -1;
        std::map<std::string, int> :: iterator it;

		for(it = supportedPictureModemap.begin(); it != supportedPictureModemap.end(); it++)
		{
		    if (it->first == pqparam)
		    {
		        index = it->second;
			break;
		    }
		}
		return index;
    }

    int AVOutputTV::getSourceIndex(std::string pqparam)
    {
		int index = -1;
        std::map<std::string, int> :: iterator it;

		for(it = supportedSourcemap.begin(); it != supportedSourcemap.end(); it++)
        {
            if (it->first == pqparam)
            {
                index = it->second;
                break;
            }
        }
        return index;
    }

    int AVOutputTV::getFormatIndex(std::string pqparam)
    {
        int index = -1;
        std::map<std::string, int> :: iterator it;

		for(it =  supportedFormatmap.begin(); it !=  supportedFormatmap.end(); it++)
        {
            if (it->first == pqparam)
            {
                index = it->second;
                break;
            }
        }
        return index;
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

	void AVOutputTV::getDynamicAutoLatencyConfig()
    {
        RFC_ParamData_t param = {0};
        WDMP_STATUS status = getRFCParameter(AVOUTPUT_RFC_CALLERID, AVOUTPUT_DALS_RFC_PARAM, &param);
        LOGINFO("RFC value for DALS - %s", param.value);
        if(WDMP_SUCCESS == status && param.type == WDMP_BOOLEAN && (strncasecmp(param.value,"true",4) == 0)) {
         m_isDalsEnabled = true;
         LOGINFO("Value of m_isDalsEnabled is %d", m_isDalsEnabled);
        }
        else {
         LOGINFO("Failed to fetch RFC or DALS is disabled");
        }
    }


	void AVOutputTV::InitializeIARM()
	{
		AVOutputBase::InitializeIARM();
#if !defined (HDMIIN_4K_ZOOM)
		if (Utils::IARM::init())
		{
			IARM_Result_t res;
			IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS, dsHdmiStatusEventHandler) );
			IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, dsHdmiVideoModeEventHandler) );
		}
#endif
	}

	void AVOutputTV::DeinitializeIARM()
	{
		AVOutputBase::DeinitializeIARM();
#if !defined (HDMIIN_4K_ZOOM)
		if (Utils::IARM::isConnected())
		{
			IARM_Result_t res;
			IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS, dsHdmiStatusEventHandler) );
			IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, dsHdmiVideoModeEventHandler) );
		}
#endif
	}

    tvError_t AVOutputTV::InitializePictureMode()
    {
        tvError_t ret = tvERROR_NONE;
	TR181_ParamData_t param;
        tvVideoSrcType_t current_source = VIDEO_SOURCE_IP;
        std::string tr181_param_name = "";
	tvVideoFormatType_t current_format = VIDEO_FORMAT_NONE;

        GetCurrentVideoFormat(&current_format);
	if ( current_format  == VIDEO_FORMAT_NONE) current_format  = VIDEO_FORMAT_SDR;
        // get current source
        GetCurrentSource(&current_source);

       tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
       tr181_param_name += "."+convertSourceIndexToString(current_source)+"."+"Format."+convertVideoFormatToString(current_format)+"."+"PictureModeString";

       tr181ErrorCode_t err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
       if ( tr181Success == err )
       {
	   std::string local = param.value;
           transform(local.begin(), local.end(), local.begin(), ::tolower);
           ret = SetTVPictureMode(local.c_str());

           if(ret != tvERROR_NONE) 
	   {
               LOGWARN("Picture Mode set failed: %s\n",getErrorString(ret).c_str());
           }
           else 
	   {
               LOGINFO("Picture Mode initialized successfully, tr181 value [%s] value: %s\n", tr181_param_name.c_str(),
			param.value);
           }
       }
       else
       {
           ret = tvERROR_GENERAL;
           LOGWARN("getLocalParam for %s Failed : %s\n", tr181_param_name.c_str(), getTR181ErrorString(err));
       }

       return ret;
    }

	tvError_t AVOutputTV::setAspectRatioZoomSettings(tvDisplayMode_t mode)
    {
        tvError_t ret = tvERROR_GENERAL;
        LOGERR("%s: mode selected is: %d", __FUNCTION__, m_videoZoomMode);
#if !defined (HDMIIN_4K_ZOOM)
        if (AVOutputTV::instance->m_isDisabledHdmiIn4KZoom) 
	{
            if (AVOutputTV::instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
                (dsVIDEO_PIXELRES_MAX == m_currentHdmiInResoluton))
	    {
                LOGWARN("%s: Setting %d zoom mode for below 4K", __FUNCTION__, m_videoZoomMode);
#endif
                ret = SetAspectRatio(mode);
#if !defined (HDMIIN_4K_ZOOM)
            }
	    else 
	    {
                LOGWARN("%s: Setting auto zoom mode for 4K and above", __FUNCTION__);
                ret = SetAspectRatio(tvDisplayMode_AUTO);
            }
        } 
	else 
	{
            LOGWARN("%s: HdmiInput is not started yet. m_isDisabledHdmiIn4KZoom: %d", __FUNCTION__, AVOutputTV::instance->m_isDisabledHdmiIn4KZoom);
            ret = SetAspectRatio((tvDisplayMode_t)m_videoZoomMode);
        }
#endif
        return ret;
    }

    tvError_t AVOutputTV::getUserSelectedAspectRatio (tvDisplayMode_t* mode)
    {
        tvError_t ret = tvERROR_GENERAL;
#if !defined (HDMIIN_4K_ZOOM)
        LOGERR("%s:mode selected is: %d", __FUNCTION__, m_videoZoomMode);
        if (AVOutputTV::instance->m_isDisabledHdmiIn4KZoom) 
	{
            if (!(AVOutputTV::instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
               (dsVIDEO_PIXELRES_MAX == AVOutputTV::instance->m_currentHdmiInResoluton)))
	    {
                *mode = (tvDisplayMode_t)AVOutputTV::instance->m_videoZoomMode;
                LOGWARN("%s: Getting zoom mode %d for display, for 4K and above", __FUNCTION__, *mode);
                return tvERROR_NONE;
            }
        }
#endif
        ret = GetAspectRatio(mode);
        return ret;
    }

}//namespace Plugin
}//namespace WPEFramework
//}
