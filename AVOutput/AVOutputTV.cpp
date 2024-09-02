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
static bool m_isDalsEnabled = false;

namespace WPEFramework {
namespace Plugin {

    AVOutputTV* AVOutputTV::instance = nullptr;

    static void tvVideoFormatChangeHandler(tvVideoFormatType_t format, void *userData)
    {
        LOGINFO("tvVideoFormatChangeHandler format:%d \n",format);
        AVOutputTV *obj = (AVOutputTV *)userData;
        if(obj) {
	    obj->NotifyVideoFormatChange(format);
	}
    }

    static void tvFilmMakerModeChangeHandler(tvContentType_t mode, void *userData)
    {
        LOGINFO("tvFilmMakerModeChangeHandler content:%d \n",mode);
        AVOutputTV *obj = (AVOutputTV *)userData;
        if(obj) {
	    obj->NotifyFilmMakerModeChange(mode);
	}
    }

    static void tvVideoResolutionChangeHandler(tvResolutionParam_t resolution, void *userData)
    {
        LOGINFO("tvVideoResolutionChangeHandler resolution:%d\n",resolution.resolutionValue);
        AVOutputTV *obj = (AVOutputTV *)userData;
        if(obj) {
	    obj->NotifyVideoResolutionChange(resolution);
	}
    }

    static void tvVideoFrameRateChangeHandler(tvVideoFrameRate_t frameRate, void *userData)
    {
        LOGINFO("tvVideoFrameRateChangeHandler format:%d \n",frameRate);
        AVOutputTV *obj = (AVOutputTV *)userData;
        if(obj) {
	    obj->NotifyVideoFrameRateChange(frameRate);
	}
    }

    static bool getVideoContentTypeToString(tvContentType_t content)
    {
        bool fmmMode = false;
        switch(content) {
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
        switch(format) {
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
        if ( resolution.resolutionValue != tvVideoResolution_NONE ) {
            strValue = std::to_string(resolution.frameWidth) + "*" + std::to_string(resolution.frameHeight) + interlaceValue;
        }
        LOGINFO("Video Resolution:[%s]\n", strValue.c_str());
        return strValue;
    }

    static const char *getVideoFrameRateTypeToString(tvVideoFrameRate_t frameRate)
    {
       const char *strValue = "NONE";
       switch(frameRate) {
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

        if (getCapabilitySource(rangeArray) == 0) {
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
        if(!AVOutputTV::instance) {
	    return;
	}

	if (IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS == eventId) {
            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            int hdmi_in_port = eventData->data.hdmi_in_status.port;
            bool hdmi_in_status = eventData->data.hdmi_in_status.isPresented;
            LOGWARN("AVOutputPlugins: Received IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS  event	port: %d, started: %d", hdmi_in_port,hdmi_in_status);
	    if (!hdmi_in_status) {
	        tvError_t ret = tvERROR_NONE;
		AVOutputTV::instance->m_isDisabledHdmiIn4KZoom = false;
	        LOGWARN("AVOutputPlugins: Hdmi streaming stopped here reapply the global zoom settings:%d here. m_isDisabledHdmiIn4KZoom: %d", AVOutputTV::instance->m_videoZoomMode, AVOutputTV::instance->m_isDisabledHdmiIn4KZoom);
		ret = SetAspectRatio((tvDisplayMode_t)AVOutputTV::instance->m_videoZoomMode);
		if (ret != tvERROR_NONE) {
		    LOGWARN("SetAspectRatio set Failed");
		}
	    }
	    else {
	        AVOutputTV::instance->m_isDisabledHdmiIn4KZoom = true;
                LOGWARN("AVOutputPlugins: m_isDisabledHdmiIn4KZoom: %d", AVOutputTV::instance->m_isDisabledHdmiIn4KZoom);
            }
	}
    }
	
    void AVOutputTV::dsHdmiVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if(!AVOutputTV::instance) {
	    return;
	}

	if (IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE == eventId) {
	    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
	    int hdmi_in_port = eventData->data.hdmi_in_video_mode.port;
	    dsVideoPortResolution_t resolution;
	    AVOutputTV::instance->m_currentHdmiInResoluton = eventData->data.hdmi_in_video_mode.resolution.pixelResolution;
	    resolution.pixelResolution =  eventData->data.hdmi_in_video_mode.resolution.pixelResolution;
	    resolution.interlaced =  eventData->data.hdmi_in_video_mode.resolution.interlaced;
	    resolution.frameRate =  eventData->data.hdmi_in_video_mode.resolution.frameRate;
	    LOGWARN("AVOutputPlugins: Received IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE	event  port: %d, pixelResolution: %d, interlaced : %d, frameRate: %d \n", hdmi_in_port,resolution.pixelResolution, resolution.interlaced, resolution.frameRate);
	    if (AVOutputTV::instance->m_isDisabledHdmiIn4KZoom) {
                tvError_t ret = tvERROR_NONE;
		if (AVOutputTV::instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
				 (dsVIDEO_PIXELRES_MAX == AVOutputTV::instance->m_currentHdmiInResoluton)) {
		    LOGWARN("AVOutputPlugins: Setting %d zoom mode for below 4K", AVOutputTV::instance->m_videoZoomMode);
		    ret = SetAspectRatio((tvDisplayMode_t)AVOutputTV::instance->m_videoZoomMode);
		}
	        else {
		    LOGWARN("AVOutputPlugins: Setting auto zoom mode for 4K and above");
		    ret = SetAspectRatio(tvDisplayMode_AUTO);
	        }
		if (ret != tvERROR_NONE) {
		    LOGWARN("SetAspectRatio set Failed");
		}
	    } 
	    else {
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

        registerMethod("getCMS", &AVOutputTV::getCMS, this);
        registerMethod("setCMS", &AVOutputTV::setCMS, this);
        registerMethod("resetCMS", &AVOutputTV::resetCMS, this);
        registerMethod("getCMSCaps", &AVOutputTV::getCMSCaps, this);

        registerMethod("get2PointWB", &AVOutputTV::get2PointWB, this);
        registerMethod("set2PointWB", &AVOutputTV::set2PointWB, this);
        registerMethod("reset2PointWB", &AVOutputTV::reset2PointWB, this);
        registerMethod("get2PointWBCaps", &AVOutputTV::get2PointWBCaps, this);

        registerMethod("getHDRMode", &AVOutputTV::getHDRMode, this);
        registerMethod("setHDRMode", &AVOutputTV::setHDRMode, this);
        registerMethod("resetHDRMode", &AVOutputTV::resetHDRMode, this);
        registerMethod("getHDRModeCaps", &AVOutputTV::getHDRModeCaps, this);

        registerMethod("getAutoBacklightMode", &AVOutputTV::getAutoBacklightMode, this);
        registerMethod("setAutoBacklightMode", &AVOutputTV::setAutoBacklightMode, this);
        registerMethod("resetAutoBacklightMode", &AVOutputTV::resetAutoBacklightMode, this);
        registerMethod("getAutoBacklightModeCaps", &AVOutputTV::getAutoBacklightModeCaps, this);

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
       
        if(ret != tvERROR_NONE) {
            LOGERR("Platform Init failed, ret: %s \n", getErrorString(ret).c_str());
	} 
	else {
            LOGINFO("Platform Init successful...\n");
            ret = TvSyncCalibrationInfoODM();
            if(ret != tvERROR_NONE) {
                LOGERR(" SD3 <->cri_data sync failed, ret: %s \n", getErrorString(ret).c_str());
            }
            else {
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

        locatePQSettingsFile();

        // Get Index from PQ capabailites
        if (getPqParamIndex() != 0) {
            LOGWARN("Failed to get the supported index from capability \n");
        }

        syncAvoutputTVParamsToHAL("none","none","none");
	
        setDefaultAspectRatio();

        // source format specific sync to ssm data
        syncAvoutputTVPQModeParamsToHAL("Current", "none", "none");

        // As we have source to picture mode mapping, get current source and
        // setting those picture mode
        initializePictureMode();

        LOGINFO("Exit\n" );
    }

    void AVOutputTV::Deinitialize()
    {
       LOGINFO("Entry\n");

       tvError_t ret = tvERROR_NONE;
       ret = TvTerm();

       if(ret != tvERROR_NONE) {
           LOGERR("Platform De-Init failed");
       }
       else {
           LOGINFO("Platform De-Init successful... \n");
       }

       LOGINFO("Exit\n");
    }

    uint32_t AVOutputTV::getZoomModeCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps("AspectRatio",info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
	    else {
            for (index = 0; index < info.rangeVector.size(); index++) {
                rangeArray.Add(info.rangeVector[index]);
	        }

            response["options"]=rangeArray;

            if (info.pqmodeVector.front().compare("none") != 0) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
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
        capDetails_t inputInfo;


        value = parameters.HasLabel("zoomMode") ? parameters["zoomMode"].String() : "";
        returnIfParamNotFound(parameters,"zoomMode");

        if (validateInputParameter("AspectRatio",value) != 0) {
            LOGERR("%s: Range validation failed for AspectRatio\n", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters,"AspectRatio",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

	    if( !isCapablityCheckPassed( "AspectRatio",inputInfo )) {
            LOGERR("%s: CapablityCheck failed for AspectRatio\n", __FUNCTION__);
            returnResponse(false);
        }

        if(!value.compare("TV 16X9 STRETCH")) {
            mode = tvDisplayMode_16x9;
        }
        else if (!value.compare("TV 4X3 PILLARBOX")) {
            mode = tvDisplayMode_4x3;
        }
        else if (!value.compare("TV NORMAL")) {
            mode = tvDisplayMode_NORMAL;
        }
        else if (!value.compare("TV DIRECT")) {
            mode = tvDisplayMode_DIRECT;
        }
        else if (!value.compare("TV AUTO")) {
            mode = tvDisplayMode_AUTO;
        }
        else if (!value.compare("TV ZOOM")) {
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
            int retval=updateAVoutputTVParam("set","AspectRatio",inputInfo,PQ_PARAM_ASPECT_RATIO,mode);

            if(retval != 0) {
                LOGERR("Failed to Save DisplayMode to ssm_data\n");
		        returnResponse(false);
            }

            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, AVOUTPUT_ASPECTRATIO_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGERR("setLocalParam for %s Failed : %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
                returnResponse(false);
            }
            else {
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

    uint32_t AVOutputTV::resetZoomMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        capDetails_t inputInfo;
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "AspectRatio",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "AspectRatio",inputInfo )) {
            LOGERR("%s: CapablityCheck failed for AspectRatio\n", __FUNCTION__);
            returnResponse(false);
        }

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,AVOUTPUT_ASPECTRATIO_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGERR("clearLocalParam for %s Failed : %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            ret = setDefaultAspectRatio(inputInfo.pqmode,inputInfo.source,inputInfo.format);
        }
        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            LOGINFO("Exit : resetDefaultAspectRatio()\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getVideoFormat(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvVideoFormatType_t videoFormat;
        tvError_t ret = GetCurrentVideoFormat(&videoFormat);
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
        tvResolutionParam_t videoResolution;
        tvError_t ret = GetCurrentVideoResolution(&videoResolution);
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
        tvError_t ret = GetCurrentVideoFrameRate(&videoFramerate);
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

        capDetails_t inputInfo;
        std::string key;
        paramIndex_t indexInfo;
        int backlight = 0,err = 0;

        if (parsingGetInputArgument(parameters, "Backlight",inputInfo) != 0) {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("Backlight") != 0) {
	        returnResponse(false);
        }

        if (getParamIndex("Backlight", inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        err = getLocalparam("Backlight",indexInfo,backlight, PQ_PARAM_BACKLIGHT);
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
        capDetails_t inputInfo;
        int backlight = 0;
        tvError_t ret  = tvERROR_NONE;

        value = parameters.HasLabel("backlight") ? parameters["backlight"].String() : "";
        returnIfParamNotFound(parameters,"backlight");
        backlight = std::stoi(value);

        if (validateIntegerInputParameter("Backlight",backlight) != 0) {
            LOGERR("Failed in Backlight range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters,"Backlight",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("Backlight") != 0 ) {
	        returnResponse(false);
	    }

        if( !isCapablityCheckPassed( "Backlight" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for Backlight\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
            LOGINFO("Proceed with setBacklight\n");
            ret = SetBacklight(backlight);
        }

        if(ret != tvERROR_NONE) {
            LOGERR("Failed to set Backlight\n");
            returnResponse(false);
        }
        else {
            int retval= updateAVoutputTVParam("set","Backlight",inputInfo,PQ_PARAM_BACKLIGHT,backlight);
            if(retval != 0 ) {
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
        capDetails_t inputInfo;
        int backlight=0;
        paramIndex_t indexInfo;
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "Backlight",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("Backlight") != 0) {
            returnResponse(false);
	    }

        if( !isCapablityCheckPassed( "Backlight",inputInfo )) {
            LOGERR("%s: CapablityCheck failed for Backlight\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updateAVoutputTVParam("reset","Backlight",inputInfo,PQ_PARAM_BACKLIGHT,params);
        if(retval != 0 ) {
            LOGERR("Failed to reset Backlight\n");
            returnResponse(false);
        }
        else {
            if (isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format)) {
                inputInfo.pqmode = "Current";
                inputInfo.source = "Current";
                inputInfo.format = "Current";
                getParamIndex("Backlight", inputInfo,indexInfo);
                int err = getLocalparam("Backlight",indexInfo,backlight, PQ_PARAM_BACKLIGHT);
                if( err == 0 ) {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,indexInfo.formatIndex, indexInfo.sourceIndex, indexInfo.pqmodeIndex,backlight);
                    ret = SetBacklight(backlight);
                }
                else {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
                    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            LOGINFO("Exit : resetBacklight Successful to value : %d \n",backlight);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getBacklightCaps(const JsonObject& parameters, JsonObject& response)
     {
        LOGINFO("Entry");
        capVectors_t vectorInfo;
        JsonObject rangeObj;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps("Backlight", vectorInfo );

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            response["platformSupport"] = (vectorInfo.isPlatformSupportVector[0].compare("true") == 0)  ? true : false;

            rangeObj["from"] = std::stoi(vectorInfo.rangeVector[0]);
            rangeObj["to"] = std::stoi(vectorInfo.rangeVector[1]);
            response["rangeInfo"]=rangeObj;

            if ((vectorInfo.pqmodeVector.front()).compare("none") != 0) {
                for (index = 0; index < vectorInfo.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(vectorInfo.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((vectorInfo.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < vectorInfo.sourceVector.size(); index++) {
                    sourceArray.Add(vectorInfo.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((vectorInfo.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < vectorInfo.formatVector.size(); index++) {
                    formatArray.Add(vectorInfo.formatVector[index]);
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int brightness = 0;

        if (parsingGetInputArgument(parameters, "Brightness",inputInfo) != 0) {
            LOGERR("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (getParamIndex("Brightness", inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Brightness",indexInfo,brightness, PQ_PARAM_BRIGHTNESS);
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
        capDetails_t inputInfo;
        int brightness = 0;
        tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("brightness") ? parameters["brightness"].String() : "";
        returnIfParamNotFound(parameters,"brightness");
        brightness = stoi(value);

        if (validateIntegerInputParameter("Brightness",brightness) != 0) {
            LOGERR("Failed in Brightness range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "Brightness",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "Brightness",inputInfo )) {
            LOGERR("%s: CapablityCheck failed for Brightness\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
             LOGINFO("Proceed with %s \n",__FUNCTION__);
             ret = SetBrightness(brightness);
        }

        if(ret != tvERROR_NONE) {
            LOGERR("Failed to set Brightness\n");
            returnResponse(false);
        }
        else {
            int retval= updateAVoutputTVParam("set","Brightness",inputInfo,PQ_PARAM_BRIGHTNESS,brightness);
            if(retval != 0 ) {
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
        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int brightness=0;
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "Brightness",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "Brightness",inputInfo )) {
            LOGERR("%s: CapablityCheck failed for Brightness\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updateAVoutputTVParam("reset","Brightness",inputInfo,PQ_PARAM_BRIGHTNESS,brightness);
        if(retval != 0 ) {
            LOGWARN("Failed to reset Brightness\n");
            returnResponse(false);
        }
        else {
            if (isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format)) {
                inputInfo.pqmode = "Current";
                inputInfo.source = "Current";
                inputInfo.format = "Current";
                getParamIndex("Brightness", inputInfo,indexInfo);
                int err = getLocalparam("Brightness",indexInfo,brightness, PQ_PARAM_BRIGHTNESS);
                if( err == 0 ) {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,indexInfo.formatIndex, indexInfo.sourceIndex, indexInfo.pqmodeIndex,brightness);
                    ret = SetBrightness(brightness);
                }
                else {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
                    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            LOGINFO("Exit : resetBrightness Successful to value : %d \n",brightness);
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::getBrightnessCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;
        JsonObject rangeObj;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps("Brightness",info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            rangeObj["from"] = stoi(info.rangeVector[0]);
            rangeObj["to"] = stoi(info.rangeVector[1]);
            response["rangeInfo"]=rangeObj;

            if ((info.pqmodeVector.front()).compare("none") != 0) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int contrast = 0;

        if (parsingGetInputArgument(parameters, "Contrast",inputInfo) != 0) {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (getParamIndex("Contrast",inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Contrast",indexInfo,contrast, PQ_PARAM_CONTRAST);
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

        capDetails_t inputInfo;
        int contrast = 0;
        tvError_t ret = tvERROR_NONE;
        std::string value;

        value = parameters.HasLabel("contrast") ? parameters["contrast"].String() : "";
        returnIfParamNotFound(parameters,"contrast");
        contrast = std::stoi(value);

        if (validateIntegerInputParameter("Contrast", contrast) != 0) {
            LOGERR("Failed in contrast range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "Contrast",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "Contrast" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for Contrast\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
             LOGINFO("Proceed with %s \n",__FUNCTION__);
             ret = SetContrast(contrast);
        }

        if(ret != tvERROR_NONE) {
            LOGERR("Failed to set Contrast\n");
            returnResponse(false);
        }
        else {
            int retval= updateAVoutputTVParam("set","Contrast",inputInfo,PQ_PARAM_CONTRAST,contrast);
            if(retval != 0 ) {
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int contrast=0;
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "Contrast",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "Contrast" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for Contrast\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updateAVoutputTVParam("reset","Contrast",inputInfo,PQ_PARAM_CONTRAST,contrast);

        if(retval != 0 ) {
            LOGWARN("Failed to reset Contrast\n");
            returnResponse(false);
        }
        else {
            if (isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format)) {
                inputInfo.pqmode = "Current";
                inputInfo.source = "Current";
                inputInfo.format = "Current";
                getParamIndex("Contrast", inputInfo,indexInfo);
                int err = getLocalparam("Contrast",indexInfo,contrast, PQ_PARAM_CONTRAST);
                if( err == 0 ) {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,indexInfo.formatIndex, indexInfo.sourceIndex, indexInfo.pqmodeIndex,contrast);
                    ret = SetContrast(contrast);
                }
                else {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
                    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            LOGINFO("Exit : resetContrast Successful to value : %d \n",contrast);
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::getContrastCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject rangeObj;
        unsigned int index = 0;

        tvError_t ret = getParamsCaps("Contrast",info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            rangeObj["from"] = stoi(info.rangeVector[0]);
            rangeObj["to"] = stoi(info.rangeVector[1]);
            response["rangeInfo"]=rangeObj;

            if ((info.pqmodeVector.front()).compare("none") != 0) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int saturation = 0;

        if (parsingGetInputArgument(parameters, "Saturation",inputInfo) != 0) {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (getParamIndex("Saturation", inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Saturation",indexInfo,saturation, PQ_PARAM_SATURATION);
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

        capDetails_t inputInfo;
        std::string value;
        int saturation = 0;
        tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("saturation") ? parameters["saturation"].String() : "";
        returnIfParamNotFound(parameters,"saturation");
        saturation = std::stoi(value);

        if (validateIntegerInputParameter("Saturation",saturation) != 0) {
            LOGERR("Failed in saturation range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "Saturation",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "Saturation" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for Saturation\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetSaturation(saturation);
        }

        if(ret != tvERROR_NONE) {
            LOGERR("Failed to set Saturation\n");
            returnResponse(false);
        }
        else {
            int retval= updateAVoutputTVParam("set","Saturation",inputInfo,PQ_PARAM_SATURATION,saturation);
            if(retval != 0 ) {
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int saturation=0;
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "Saturation", inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "Saturation", inputInfo )) {
            LOGERR("%s: CapablityCheck failed for Saturation\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updateAVoutputTVParam("reset","Saturation",inputInfo,PQ_PARAM_SATURATION,saturation);

        if(retval != 0 ) {
            LOGERR("Failed to reset Saturation\n");
            returnResponse(false);
        }
        else {
            if (isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format)) {
                inputInfo.pqmode = "Current";
                inputInfo.source = "Current";
                inputInfo.format = "Current";
                getParamIndex("Saturation",inputInfo,indexInfo);
                int err = getLocalparam("Saturation",indexInfo, saturation, PQ_PARAM_SATURATION);
                if( err == 0 ) {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,indexInfo.formatIndex, indexInfo.sourceIndex, indexInfo.pqmodeIndex,saturation);
                    ret = SetSaturation(saturation);
                }
                else {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
                    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            LOGINFO("Exit : resetSaturation Successful to value : %d \n",saturation);
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::getSaturationCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        unsigned int index = 0;
        JsonObject rangeObj;

        tvError_t ret = getParamsCaps("Saturation",info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            rangeObj["from"] = stoi(info.rangeVector[0]);
            rangeObj["to"] = stoi(info.rangeVector[1]);
            response["rangeInfo"]=rangeObj;


            if ((info.pqmodeVector.front()).compare("none") != 0) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int sharpness = 0;

        if (parsingGetInputArgument(parameters, "Sharpness",inputInfo) != 0) {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (getParamIndex("Sharpness",inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Sharpness",indexInfo,sharpness, PQ_PARAM_SHARPNESS);
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

        capDetails_t inputInfo;
        int sharpness = 0;
        tvError_t ret = tvERROR_NONE;
        std::string value;

        value = parameters.HasLabel("sharpness") ? parameters["sharpness"].String() : "";
        returnIfParamNotFound(parameters,"sharpness");
        sharpness = std::stoi(value);

        if (validateIntegerInputParameter("Sharpness",sharpness) != 0) {
            LOGERR("Failed in sharpness range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "Sharpness", inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "Sharpness", inputInfo  )) {
            LOGERR("%s: CapablityCheck failed for Sharpness\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetSharpness(sharpness);
        }

        if(ret != tvERROR_NONE) {
            LOGERR("Failed to set Sharpness\n");
            returnResponse(false);
        }
        else {
            int retval= updateAVoutputTVParam("set","Sharpness",inputInfo,PQ_PARAM_SHARPNESS,sharpness);
            if(retval != 0 ) {
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int sharpness=0;
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "Sharpness",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "Sharpness" , inputInfo)) {
            LOGERR("%s: CapablityCheck failed for Sharpness\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updateAVoutputTVParam("reset","Sharpness", inputInfo,PQ_PARAM_SHARPNESS,sharpness);

        if(retval != 0 ) {
            LOGERR("Failed to reset Sharpness\n");
            returnResponse(false);
        }
        else {
            if (isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format)) {
                inputInfo.pqmode = "Current";
                inputInfo.source = "Current";
                inputInfo.format = "Current";
                getParamIndex("Sharpness",inputInfo,indexInfo);
                int err = getLocalparam("Sharpness",indexInfo, sharpness, PQ_PARAM_SHARPNESS);
                if( err == 0 ) {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,indexInfo.formatIndex, indexInfo.sourceIndex, indexInfo.pqmodeIndex,sharpness);
                    ret = SetSharpness(sharpness);
                }
                else {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
                    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            LOGINFO("Exit : resetSharpness Successful to value : %d \n",sharpness);
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::getSharpnessCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject rangeObj;
        unsigned int index = 0;

        tvError_t ret = getParamsCaps("Sharpness",info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            rangeObj["from"] = stoi(info.rangeVector[0]);
            rangeObj["to"] = stoi(info.rangeVector[1]);
            response["rangeInfo"]=rangeObj;

            if ((info.pqmodeVector.front()).compare("none") != 0) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int hue = 0;

        if (parsingGetInputArgument(parameters, "Hue", inputInfo) != 0) {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (getParamIndex("Hue",inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("Hue",indexInfo,hue, PQ_PARAM_HUE);
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

        capDetails_t inputInfo;
        int hue = 0;
        tvError_t ret = tvERROR_NONE;
        std::string value;

        value = parameters.HasLabel("hue") ? parameters["hue"].String() : "";
        returnIfParamNotFound(parameters,"hue");
        hue = std::stoi(value);

        if (validateIntegerInputParameter("Hue",hue) != 0) {
            LOGERR("Failed in hue range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "Hue",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "Hue", inputInfo )) {
            LOGERR("%s: CapablityCheck failed for Hue\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetHue(hue);
        }

        if(ret != tvERROR_NONE) {
            LOGERR("Failed to set Hue\n");
            returnResponse(false);
        }
        else {
            int retval= updateAVoutputTVParam("set","Hue",inputInfo,PQ_PARAM_HUE,hue);
            if(retval != 0 ) {
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int hue=0;
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "Hue",inputInfo)!= 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "Hue" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for Hue\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updateAVoutputTVParam("reset","Hue", inputInfo,PQ_PARAM_HUE,hue);

        if(retval != 0 ) {
            LOGERR("Failed to reset Hue\n");
            returnResponse(false);
        }
        else {
            if (isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format)) {
                inputInfo.pqmode = "Current";
                inputInfo.source = "Current";
                inputInfo.format = "Current";
                getParamIndex("Hue",inputInfo,indexInfo);
                int err = getLocalparam("Hue",indexInfo, hue, PQ_PARAM_HUE);
                if( err == 0 ) {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,indexInfo.formatIndex, indexInfo.sourceIndex, indexInfo.pqmodeIndex,hue);
                    ret = SetHue(hue);
                }
                else {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
                    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            LOGINFO("Exit : resetHue Successful to value : %d \n",hue);
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::getHueCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        JsonObject rangeObj;
        unsigned int index = 0;

        tvError_t ret = getParamsCaps("Hue",info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            rangeObj["from"] = stoi(info.rangeVector[0]);
            rangeObj["to"] = stoi(info.rangeVector[1]);
            response["rangeInfo"]=rangeObj;

            if ((info.pqmodeVector.front()).compare("none") != 0) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int colortemp = 0;

        if (parsingGetInputArgument(parameters, "ColorTemperature", inputInfo) != 0) {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (getParamIndex("ColorTemperature",inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("ColorTemp",indexInfo,colortemp,PQ_PARAM_COLOR_TEMPERATURE);
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

        capDetails_t inputInfo;
        std::string value;
        tvColorTemp_t colortemp = tvColorTemp_MAX;
        tvError_t ret = tvERROR_NONE;

        value = parameters.HasLabel("colorTemperature") ? parameters["colorTemperature"].String() : "";
        returnIfParamNotFound(parameters,"colorTemperature");
        if(!value.compare("Standard")) {
            colortemp = tvColorTemp_STANDARD;
        }
        else if (!value.compare("Warm")) {
            colortemp = tvColorTemp_WARM;
        }
        else if (!value.compare("Cold")) {
            colortemp = tvColorTemp_COLD;
        }
        else if (!value.compare("UserDefined")) {
            colortemp = tvColorTemp_USER;
        }
        else {
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "ColorTemperature",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "ColorTemperature", inputInfo )) {
            LOGERR("%s: CapablityCheck failed for colorTemperature\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetColorTemperature((tvColorTemp_t)colortemp);
        }

        if(ret != tvERROR_NONE) {
            LOGERR("Failed to set ColorTemperature\n");
            returnResponse(false);
        }
        else {
            int retval= updateAVoutputTVParam("set","ColorTemp", inputInfo,PQ_PARAM_COLOR_TEMPERATURE,(int)colortemp);
            if(retval != 0 ) {
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int colortemp=0;
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "ColorTemperature", inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "ColorTemperature", inputInfo )) {
            LOGERR("%s: CapablityCheck failed for colorTemperature\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updateAVoutputTVParam("reset","ColorTemp", inputInfo,PQ_PARAM_COLOR_TEMPERATURE,colortemp);

        if(retval != 0 ) {
            LOGERR("Failed to reset ColorTemperature\n");
            returnResponse(false);
        }
        else {
            if (isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format)) {
                inputInfo.pqmode = "Current";
                inputInfo.source = "Current";
                inputInfo.format = "Current";
                getParamIndex("ColorTemperature",inputInfo,indexInfo);
                int err = getLocalparam("ColorTemp",indexInfo, colortemp, PQ_PARAM_COLOR_TEMPERATURE);
                if( err == 0 ) {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,indexInfo.formatIndex, indexInfo.sourceIndex, indexInfo.pqmodeIndex, colortemp);
                    ret = SetColorTemperature((tvColorTemp_t)colortemp);
                }
                else {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
                    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            LOGINFO("Exit : resetColorTemperature Successful to value : %d \n",colortemp);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getColorTemperatureCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps("ColorTemperature",info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            for (index = 0; index < info.rangeVector.size(); index++) {
                rangeArray.Add(info.rangeVector[index]);
            }

            response["options"]=rangeArray;

            if (((info.pqmodeVector.front()).compare("none") != 0)) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int dimmingMode = 0;

        if (parsingGetInputArgument(parameters, "DimmingMode", inputInfo) != 0) {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (getParamIndex("DimmingMode",inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }


        int err = getLocalparam("DimmingMode",indexInfo,dimmingMode, PQ_PARAM_DIMMINGMODE);
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

        capDetails_t inputInfo;
        int dimmingMode = 0;
        tvError_t ret = tvERROR_NONE;
        std::string value;

        value = parameters.HasLabel("DimmingMode") ? parameters["DimmingMode"].String() : "";
        returnIfParamNotFound(parameters,"DimmingMode");

        if (validateInputParameter("DimmingMode",value) != 0) {
            LOGERR("%s: Range validation failed for DimmingMode\n", __FUNCTION__);
            returnResponse(false);
        }
        dimmingMode = getDimmingModeIndex(value);

        if (parsingSetInputArgument(parameters, "DimmingMode",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "DimmingMode" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for DimmingMode\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetTVDimmingMode(value.c_str());
        }

        if(ret != tvERROR_NONE) {
            LOGERR("Failed to set DimmingMode\n");
            returnResponse(false);
        }
        else {
            int retval= updateAVoutputTVParam("set","DimmingMode",inputInfo,PQ_PARAM_DIMMINGMODE,(int)dimmingMode);
            if(retval != 0 ) {
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        std::string dimmingMode;
        int dMode=0;
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "DimmingMode", inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "DimmingMode" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for DimmingMode\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updateAVoutputTVParam("reset","DimmingMode", inputInfo,PQ_PARAM_DIMMINGMODE,dMode);

        if(retval != 0 ) {
            LOGERR("Failed to reset ldim\n");
            returnResponse(false);
        }

        else {
            if (isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format)) {
                inputInfo.pqmode = "Current";
                inputInfo.source = "Current";
                inputInfo.format = "Current";
                getParamIndex("DimmingMode",inputInfo,indexInfo);
                int err = getLocalparam("DimmingMode",indexInfo, dMode, PQ_PARAM_DIMMINGMODE);
                if( err == 0 ) {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,indexInfo.formatIndex, indexInfo.sourceIndex, indexInfo.pqmodeIndex, dMode);
                    getDimmingModeStringFromEnum(dMode,dimmingMode);
                    ret = SetTVDimmingMode(dimmingMode.c_str());
                }
                else {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
                    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            LOGINFO("Exit : resetBacklightDimmingMode Successful to value : %s \n",dimmingMode.c_str());
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getBacklightDimmingModeCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray supportedDimmingModeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps("DimmingMode",info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            for (index = 0; index < info.rangeVector.size(); index++) {
                supportedDimmingModeArray.Add(info.rangeVector[index]);
            }

            response["options"]=supportedDimmingModeArray;

            if (((info.pqmodeVector.front()).compare("none") != 0)) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
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
        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            JsonArray SupportedDVModes;

            for(int count = 0;count <totalAvailable;count++ ) {
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
        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int dolbyMode = 0;
        int err = 0;

        if (parsingGetInputArgument(parameters, "DolbyVisionMode",inputInfo) != 0) {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("DolbyVisionMode") != 0) {
	    returnResponse(false);
	}


        if (getParamIndex("DolbyVisionMode",inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        err = getLocalparam("DolbyVisionMode",indexInfo,dolbyMode, PQ_PARAM_DOLBY_MODE);
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

        capDetails_t inputInfo;
        std::string value;
        tvError_t ret  = tvERROR_NONE;

        value = parameters.HasLabel("dolbyVisionMode") ? parameters["dolbyVisionMode"].String() : "";
        returnIfParamNotFound(parameters,"dolbyVisionMode");

        if (parsingSetInputArgument(parameters, "DolbyVisionMode",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("DolbyVisionMode") != 0) {
            returnResponse(false);
        }

        if (validateInputParameter("DolbyVisionMode",value) != 0) {
            LOGERR("%s: Range validation failed for DolbyVisionMode\n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "DolbyVisionMode" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for DolbyVisionMode\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired("Current",inputInfo.source,"DV") ) {
            LOGINFO("Proceed with setDolbyVisionMode\n\n");
            ret = SetTVDolbyVisionModeODM(value.c_str());
        }

        if(ret != tvERROR_NONE) {
            LOGERR("Failed to set DolbyVisionMode\n\n");
            returnResponse(false);
        }
        else {
            int retval= updateAVoutputTVParam("set","DolbyVisionMode",inputInfo,PQ_PARAM_DOLBY_MODE,getDolbyModeIndex(value.c_str()));
            if(retval != 0 ) {
                LOGERR("Failed to Save Dolbyvision mode\n");
                returnResponse(false);
            }
            LOGINFO("Exit : Dolbyvision successful to value: %s\n", value.c_str());
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::resetDolbyVisionMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int dolbyMode=0;
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "DolbyVisionMode",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("DolbyVisionMode") != 0) {
	    returnResponse(false);
	}

        if( !isCapablityCheckPassed( "DolbyVisionMode" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for DolbyVisionMode\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updateAVoutputTVParam("reset","DolbyVisionMode",inputInfo,PQ_PARAM_DOLBY_MODE,dolbyMode);
        if(retval != 0 ) {
            LOGERR("Failed to reset DolbyVisionMode\n");
            returnResponse(false);
        }
        else {
            if (isSetRequired("Current",inputInfo.source,inputInfo.format)) {
                inputInfo.source = "Current";
                inputInfo.pqmode = "Current";
                inputInfo.format = "DV";
                getParamIndex("DolbyVisionMode",inputInfo,indexInfo);
                int err = getLocalparam("DolbyVisionMode",indexInfo, dolbyMode, PQ_PARAM_DOLBY_MODE);
                if( err == 0 ) {
                    std::string dolbyModeValue = getDolbyModeStringFromEnum((tvDolbyMode_t)dolbyMode);
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d dolbyvalue : %s\n",__FUNCTION__,indexInfo.formatIndex, indexInfo.sourceIndex, indexInfo.pqmodeIndex, dolbyModeValue.c_str());
                    ret = SetTVDolbyVisionModeODM(dolbyModeValue.c_str());
                }
                else {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
                    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            LOGINFO("Exit : resetDolbyMode Successful to value : %d \n",dolbyMode);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getDolbyVisionModeCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps("DolbyVisionMode", info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {

            response["platformSupport"] = (info.isPlatformSupportVector[0].compare("true") == 0 ) ? true : false;

            for (index = 0; index < info.rangeVector.size(); index++) {
                rangeArray.Add(info.rangeVector[index]);
            }

            response["options"]=rangeArray;
            if ((info.pqmodeVector.front()).compare("none") != 0) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
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
        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            JsonArray SupportedPicModes;

            for(int count = 0;count <totalAvailable;count++ ) {
                SupportedPicModes.Add(pictureModes[count].name);
                // printf("Added Mode %s %s \n",pictureModes[count].name,SupportedPicModes[count].String().c_str());
            }

            response["supportedPictureModes"] = SupportedPicModes;
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getVideoSourceCaps(const JsonObject& parameters, JsonObject& response)
    {

        JsonArray rangeArray;

        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        if (getCapabilitySource(rangeArray) != 0) {
            returnResponse(false);
        }
        response["options"]=rangeArray;
        LOGINFO("Exit\n");
        returnResponse(true);
    }

    uint32_t AVOutputTV::getVideoFormatCaps(const JsonObject& parameters, JsonObject& response)
    {

        JsonArray rangeArray;

        capVectors_t info;

        tvError_t ret = getParamsCaps("VideoFormat",info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            if ((info.rangeVector.front()).compare("none") != 0) {
                for (unsigned int index = 0; index < info.rangeVector.size(); index++) {
                    rangeArray.Add(info.rangeVector[index]);
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

        if ( getRangeCapability("VideoFrameRate", rangeInfo) != 0 ) {
            returnResponse(false);
        }

        for (unsigned int index = 0; index < rangeInfo.size(); index++) {
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

        capVectors_t info;

        unsigned int index = 0;
        tvError_t ret = getParamsCaps("PictureMode",info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {

            if ((info.rangeVector.front()).compare("none") != 0) {
                for (index = 0; index < info.rangeVector.size(); index++) {
                    rangeArray.Add(info.rangeVector[index]);
                }
                response["options"]=rangeArray;
            }

            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
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
        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        std::string tr181_param_name;
        TR181_ParamData_t param = {0};
        tr181ErrorCode_t err = tr181Success;

        if (parsingGetInputArgument(parameters, "PictureMode",inputInfo) != 0) {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (getParamIndex("PictureMode",inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
        tr181_param_name += "." + convertSourceIndexToString(indexInfo.sourceIndex) + "." + "Format."+convertVideoFormatToString(indexInfo.formatIndex)+"."+"PictureModeString";
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
        capDetails_t inputInfo;
        char prevmode[PIC_MODE_NAME_MAX]={0};
        std::string value;
        GetTVPictureMode(prevmode);

        tvError_t ret = tvERROR_NONE;
        value = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        returnIfParamNotFound(parameters,"pictureMode");

        // As only source need to validate, so pqmode and formate passing as currrent
        if (parsingSetInputArgument(parameters, "PictureMode",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if (validateInputParameter("PictureMode",value) != 0) {
            LOGERR("%s: Range validation failed for PictureMode\n", __FUNCTION__);
            returnResponse(false);
        }
        if( !isCapablityCheckPassed( "PictureMode" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for PictureMode\n", __FUNCTION__);
            returnResponse(false);
        }

        std::string local = value;
        transform(local.begin(), local.end(), local.begin(), ::tolower);
        if( isSetRequired("Current",inputInfo.source,inputInfo.format) ) {
            LOGINFO("Proceed with SetTVPictureMode\n");
            ret = SetTVPictureMode(local.c_str());
         }
        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            valueVectors_t values;
            inputInfo.pqmode = "Current";

            getSaveConfig("PictureMode" ,inputInfo, values);

            for (int sourceType : values.sourceValues) {
                tvVideoSrcType_t source = (tvVideoSrcType_t)sourceType;
                for (int formatType : values.formatValues) {
                    tvVideoFormatType_t format = (tvVideoFormatType_t)formatType;
                    std::string tr181_param_name = "";
                    tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
                    // framing Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.Source.source_index[x].Format.format_index[x].PictureModeString.value
                    tr181_param_name += "."+convertSourceIndexToString(source)+"."+"Format."+
                                      convertVideoFormatToString(format)+"."+"PictureModeString";
                    tr181ErrorCode_t err = setLocalParam(rfc_caller_id, tr181_param_name.c_str(), value.c_str());
                    if ( err != tr181Success ) {
                        LOGERR("setLocalParam for %s Failed : %s\n", AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM, getTR181ErrorString(err));
                        returnResponse(false);
                    }
                    else {
                        LOGINFO("setLocalParam for %s Successful, Value: %s\n", AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM, value.c_str());
                        int pqmodeindex = (int)getPictureModeIndex(value);
                        SaveSourcePictureMode(source, format, pqmodeindex);
                    }
                }
            }

            //Filmmaker mode telemetry
            if(!strncmp(value.c_str(),"filmmaker",strlen(value.c_str())) && strncmp(prevmode,"filmmaker",strlen(prevmode))) {
                LOGINFO("%s mode has been enabled",value.c_str());
	    }
            else if(!strncmp(prevmode,"filmmaker",strlen(prevmode)) && strncmp(value.c_str(),"filmmaker",strlen(value.c_str()))) {
                LOGINFO("%s mode has been disabled",prevmode);
	    }

            LOGINFO("Broadcasting the low latency change event \n");

            if(m_isDalsEnabled) {
                //GameModebroadcast
                if(!strncmp(value.c_str(),"game",strlen(value.c_str())) && strncmp(prevmode,"game",strlen(prevmode))) {
                    broadcastLowLatencyModeChangeEvent(1);
		}
                else if(!strncmp(prevmode,"game",strlen(prevmode)) && strncmp(value.c_str(),"game",strlen(value.c_str()))) {
                    broadcastLowLatencyModeChangeEvent(0);
		}
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

        valueVectors_t values;
        capDetails_t inputInfo;

        // As only source need to validate, so pqmode and formate passing as currrent
        if (parsingSetInputArgument(parameters, "PictureMode",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "PictureMode",inputInfo )) {
            LOGERR("%s: CapablityCheck failed for PictureMode\n", __FUNCTION__);
            returnResponse(false);
        }
        inputInfo.pqmode = "Current";
        getSaveConfig("PictureMode", inputInfo, values);

        for (int source : values.sourceValues) {
            tvVideoSrcType_t sourceType = (tvVideoSrcType_t)source;
            for (int format : values.formatValues) {
                tvVideoFormatType_t formatType = (tvVideoFormatType_t)format;
                std::string tr181_param_name = "";
                tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
                tr181_param_name += "."+convertSourceIndexToString(sourceType)+"."+"Format."+
                                   convertVideoFormatToString(formatType)+"."+"PictureModeString";

                err = clearLocalParam(rfc_caller_id, tr181_param_name.c_str());
                if ( err != tr181Success ) {
                    LOGWARN("clearLocalParam for %s Failed : %s\n", tr181_param_name.c_str(), getTR181ErrorString(err));
                    returnResponse(false);
                }
                else {
                    err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
                    if ( tr181Success == err ) {
                        //get curren source and if matches save for that alone
                        tvVideoSrcType_t current_source = VIDEO_SOURCE_IP;
                        GetCurrentSource(&current_source);

                        tvVideoFormatType_t current_format = VIDEO_FORMAT_NONE;
                        GetCurrentVideoFormat(&current_format);
                        if( current_format == VIDEO_FORMAT_NONE) {
                            current_format = VIDEO_FORMAT_SDR;
                        }

                        //as hal using lower across converting to lower
                        std::string setparam = param.value;
                        transform(setparam.begin(), setparam.end(), setparam.begin(), ::tolower);

                        if (current_source == sourceType && current_format == formatType) {

                            tvError_t ret = SetTVPictureMode(setparam.c_str());
                            if(ret != tvERROR_NONE) {
                                LOGWARN("Picture Mode set failed: %s\n",getErrorString(ret).c_str());
                                returnResponse(false);
                            }
                            else {
                                LOGINFO("Exit : Picture Mode reset successfully, value: %s\n", param.value);
                            }
                        }
                        int pqmodeindex = (int)getPictureModeIndex(param.value);
                        SaveSourcePictureMode(sourceType, formatType, pqmodeindex);
                    }
                    else {
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

        if(strncmp(value.c_str(),"ON",strlen(value.c_str())) == 0) {
                NotifyFilmMakerModeChange(tvContentType_FMM);
                LOGINFO(" enabling Film makermode \n");
        }
        else {
                LOGINFO(" disabling Film makermode \n");
                NotifyFilmMakerModeChange(tvContentType_NONE);
        }
        returnResponse(true);
    }

    uint32_t AVOutputTV::setLowLatencyState(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        capDetails_t inputInfo;
        int lowLatencyIndex = 0,prevLowLatencyIndex = 0;
        tvError_t ret = tvERROR_NONE;

        ret = GetLowLatencyState(&prevLowLatencyIndex);
        if(ret != tvERROR_NONE) {
            LOGERR("Get previous low latency state failed\n");
            returnResponse(false);
        }

        value = parameters.HasLabel("LowLatencyState") ? parameters["LowLatencyState"].String() : "";
        returnIfParamNotFound(parameters,"LowLatencyState");
        lowLatencyIndex = std::stoi(value);

        if (validateIntegerInputParameter("LowLatencyState",lowLatencyIndex) != 0) {
            LOGERR("Failed in Brightness range validation:%s", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters, "LowLatencyState",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "LowLatencyState" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for LowLatencyState\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updateAVoutputTVParam("set","LowLatencyState",inputInfo,PQ_PARAM_LOWLATENCY_STATE,lowLatencyIndex);
        if(retval != 0 ) {
            LOGERR("Failed to SaveLowLatency to ssm_data\n");
            returnResponse(false);
        } else {

            if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
             LOGINFO("Proceed with setLowLatencyState\n");
             ret = SetLowLatencyState( lowLatencyIndex );
            }

            if(ret != tvERROR_NONE) {
                LOGERR("Failed to set low latency. Fallback to previous state %d\n", prevLowLatencyIndex);
                retval=updateAVoutputTVParam("set","LowLatencyState",inputInfo,PQ_PARAM_LOWLATENCY_STATE,prevLowLatencyIndex);
                if(retval != 0 ){
                    LOGERR("Fallback to previous low latency state %d failed.\n", prevLowLatencyIndex);
                }
                returnResponse(false);
            }

            LOGINFO("Exit : setLowLatency successful to value: %d\n", lowLatencyIndex);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getLowLatencyState(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int lowlatencystate = 0;

        if (parsingGetInputArgument(parameters, "LowLatencyState",inputInfo) != 0) {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }
        if (getParamIndex("LowLatencyState",inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("LowLatencyState", indexInfo ,lowlatencystate, PQ_PARAM_LOWLATENCY_STATE);
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

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int lowlatencystate=0;
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "LowLatencyState", inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "LowLatencyState" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for LowLatencyState\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updateAVoutputTVParam("reset","LowLatencyState", inputInfo,PQ_PARAM_LOWLATENCY_STATE,lowlatencystate);
        if(retval != 0 ) {
            LOGERR("Failed to clear Lowlatency from ssmdata and localstore\n");
            returnResponse(false);
        }
        else {
            if (isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format)) {
                inputInfo.pqmode = "Current";
                inputInfo.source = "Current";
                inputInfo.format = "Current";
                getParamIndex("LowLatencyState",inputInfo, indexInfo);
                int err = getLocalparam("LowLatencyState",indexInfo, lowlatencystate, PQ_PARAM_LOWLATENCY_STATE);
                if( err == 0 ) {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,indexInfo.formatIndex, indexInfo.sourceIndex, indexInfo.pqmodeIndex, lowlatencystate);
                    ret = SetLowLatencyState(lowlatencystate);
                }
                else {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
                    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            LOGINFO("Exit : resetLowLatency Successful to value : %d \n",lowlatencystate);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getLowLatencyStateCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps("LowLatencyState", info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            for (index = 0; index < info.rangeVector.size(); index++) {
                rangeArray.Add(stoi(info.rangeVector[index]));
	    }

            response["LowLatencyInfo"]=rangeArray;
            if ((info.pqmodeVector.front()).compare("none") != 0) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getCMS(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int level = 0;
        tvPQParameterIndex_t tvPQEnum;

        inputInfo.color = parameters.HasLabel("color") ? parameters["color"].String() : "";
	    inputInfo.component = parameters.HasLabel("component") ? parameters["component"].String() : "";
        
        if( inputInfo.color.empty() || inputInfo.component.empty() ) {
	        LOGERR("%s : Color/Component param not found!!!\n",__FUNCTION__);
            returnResponse(false);
        }


        if (parsingGetInputArgument(parameters, "CMS", inputInfo) != 0) {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (getParamIndex("CMS",inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        if ( convertCMSParamToPQEnum(inputInfo.component,inputInfo.color,tvPQEnum) != 0 ) {
            LOGINFO("%s: Component/Color Param Not Found \n",__FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("CMS",indexInfo,level,tvPQEnum);
        if( err == 0 ) {
            response["level"] = level;
            LOGINFO("Exit : params Value: %d \n", level);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t AVOutputTV::setCMS(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        capDetails_t inputInfo;
        int level = 0,retVal = 0;
        tvPQParameterIndex_t tvPQEnum;
        tvComponentType_t compEnum;
        tvDataComponentColor_t colorEnum=tvDataColor_NONE;
        std::string color,component;
        tvError_t ret = tvERROR_NONE;
        std::string value;

        inputInfo.color = parameters.HasLabel("color") ? parameters["color"].String() : "";
	    inputInfo.component = parameters.HasLabel("component") ? parameters["component"].String() : "";

        if( inputInfo.color.empty() || inputInfo.component.empty() ) {
	        LOGERR("%s : Color/Component param not found!!!\n",__FUNCTION__);
            returnResponse(false);
        }

        value = parameters.HasLabel("level") ? parameters["level"].String() : "";
        returnIfParamNotFound(parameters,"level");
        level = std::stoi(value);

        if (validateCMSParameter(inputInfo.component,level) != 0) {
            LOGERR("%s: CMS Failed in range validation", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters,"CMS",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "CMS",inputInfo )) {
            LOGERR("%s: CapablityCheck failed for CMS\n", __FUNCTION__);
            returnResponse(false);
        }

        if ( convertCMSParamToPQEnum(inputInfo.component,inputInfo.color,tvPQEnum) != 0 ) {
            LOGERR("%s: %s/%s Param Not Found \n",__FUNCTION__,inputInfo.component.c_str(),inputInfo.color.c_str());
            returnResponse(false);
        }    

 /*       retVal = getCMSComponentEnumFromString(inputInfo.component,compEnum);
        if( retVal == -1) {
            LOGERR("%s: Invalid Component : %s\n",__FUNCTION__,inputInfo.component.c_str());
            returnResponse(false);
        }

        retVal = getCMSColorEnumFromString(inputInfo.color,colorEnum);
        if( retVal == -1) {
            LOGERR("%s: Invalid Color : %s\n",__FUNCTION__,inputInfo.color.c_str());
            returnResponse(false);
        }*/

        if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
            LOGINFO("Proceed with %s\n",__FUNCTION__);
            tvError_t ret = SetCMSState(true);
            if(ret != tvERROR_NONE) {
                LOGWARN("CMS enable failed\n");
                returnResponse(false);
            }
            
            if(inputInfo.component.compare("Saturation") == 0)
                ret = SetCurrentComponentSaturation(colorEnum, level);
            else if(inputInfo.component.compare("Hue") == 0 )
                ret = SetCurrentComponentHue(colorEnum,level);
            else if( inputInfo.component.compare("Luma") == 0 )
                ret = SetCurrentComponentLuma(colorEnum,level);
        
        }      

        if(ret != tvERROR_NONE) {
            LOGERR("Failed to set CMS\n");
            returnResponse(false);
        }
        else  {
            std::string cmsParam;                   
            cmsParam = inputInfo.color+"."+inputInfo.component;
            
            retVal= updateAVoutputTVParam("set","CMS",inputInfo,tvPQEnum,level);
            if(retVal != 0 ) {
                LOGERR("%s : Failed to Save CMS %s/%s(%s) to ssm_data\n",__FUNCTION__,inputInfo.component.c_str(),inputInfo.color.c_str(),cmsParam.c_str());
		        returnResponse(false);
            }
            LOGINFO("Exit : setCMS %s/%s successful to value: %d\n", inputInfo.component.c_str(),inputInfo.color.c_str(),level);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::resetCMS(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        capDetails_t inputInfo;
        int retVal = 0;
        std::string color,component;
        tvError_t ret = tvERROR_NONE;

        inputInfo.color = parameters.HasLabel("color") ? parameters["color"].String() : "";
	    inputInfo.component = parameters.HasLabel("component") ? parameters["component"].String() : "";

        if (parsingSetInputArgument(parameters,"CMS",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "CMS" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for CMS\n", __FUNCTION__);
            returnResponse(false);
        }
/*
        if ( convertCMSParamToEnum(inputInfo.component,inputInfo.color,tvPQEnum) != 0 ) {
            LOGERR("%s: %s/%s Param Not Found \n",__FUNCTION__,inputInfo.component.c_str(),inputInfo.color.c_str());
            returnResponse(false);
        }    

        retVal = getCMSComponentEnumFromString(inputInfo.component,compEnum);
        if( ret == -1) {
            LOGERR("%s: Invalid Component : %s\n",__FUNCTION__,inputInfo.component.c_str());
            returnResponse(false);
        }

        retVal = getCMSColorEnumFromString(inputInfo.color,colorEnum);
        if( ret == -1) {
            LOGERR("%s: Invalid Color : %s\n",__FUNCTION__,inputInfo.color.c_str());
            returnResponse(false);
        } */

        if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
            LOGINFO("Proceed with %s\n",__FUNCTION__);
            tvError_t ret = SetCMSState(false);
            if(ret != tvERROR_NONE) {
                LOGWARN("CMS disable failed\n");
                returnResponse(false);
            } 
        }      

        if(ret != tvERROR_NONE) {
            LOGERR("%s : Failed to setCMSState\n",__FUNCTION__);
            returnResponse(false);
        }
        else  {
            int cms = 0;
            retVal= updateAVoutputTVParam("reset","CMS",inputInfo,PQ_PARAM_CMS_SATURATION_RED,cms);
            if(retVal != 0 ) {
                LOGERR("%s : Failed to Save CMS %s/%s to ssm_data\n",__FUNCTION__,inputInfo.component.c_str(),inputInfo.color.c_str() );
		        returnResponse(false);
            }
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getCMSCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;
        JsonArray colorArray;
        JsonArray componentArray;

        JsonObject componentSaturationRangeInfo;
        JsonObject componentHueRangeInfo;
        JsonObject componentLumaRangeInfo;
        unsigned int index = 0;

        tvError_t ret = getParamsCaps("CMS",info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {

            response["platformSupport"] = (info.isPlatformSupportVector[0].compare("true") == 0)  ? true : false;

            componentSaturationRangeInfo["from"] = stoi(info.rangeVector[0]);
            componentSaturationRangeInfo["to"]   = stoi(info.rangeVector[1]);
            response["componentSaturationRangeInfo"]=componentSaturationRangeInfo;
            
            componentHueRangeInfo["from"] = stoi(info.rangeVector[2]);
            componentHueRangeInfo["to"]   = stoi(info.rangeVector[3]);
            response["componentHueRangeInfo"]=componentHueRangeInfo;

            componentLumaRangeInfo["from"] = stoi(info.rangeVector[4]);
            componentLumaRangeInfo["to"]   = stoi(info.rangeVector[5]);
            response["componentLumaRangeInfo"]=componentLumaRangeInfo;


            if ((info.pqmodeVector.front()).compare("none") != 0) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }

            if ((info.colorVector.front()).compare("none") != 0) {
                for (index = 0; index < info.colorVector.size(); index++) {
                    colorArray.Add(info.colorVector[index]);
                }
                response["colorInfo"]=colorArray;
            }

            if ((info.componentVector.front()).compare("none") != 0) {
                for (index = 0; index < info.componentVector.size(); index++) {
                    componentArray.Add(info.componentVector[index]);
                }
                response["componentInfo"]=componentArray;
            }

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getHDRMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capDetails_t inputInfo;
        int dolbyMode = 0;
        int err = 0;
        paramIndex_t indexInfo;

        if (parsingGetInputArgument(parameters, "HDRMode", inputInfo) != 0) {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("HDRMode") != 0) {
	        returnResponse(false);
        }

        if (getParamIndex("HDRMode",inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        err = getLocalparam("HDRMode", indexInfo,dolbyMode, PQ_PARAM_HDR_MODE);
        if( err == 0 ) {
            response["hdrMode"] = getDolbyModeStringFromEnum((tvDolbyMode_t)dolbyMode);
            LOGINFO("Exit : hdrMode Value: %d \n", dolbyMode);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }

    }

    uint32_t AVOutputTV::setHDRMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        capDetails_t inputInfo;
        tvError_t ret  = tvERROR_NONE;
        std::string value;

        value = parameters.HasLabel("HDRMode") ? parameters["HDRMode"].String() : "";
        returnIfParamNotFound(parameters,"HDRMode");

        if (parsingSetInputArgument(parameters, "HDRMode", inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("HDRMode") != 0) {
	        returnResponse(false);
        }

        if (validateInputParameter("HDRMode",value) != 0) {
            LOGERR("%s: Range validation failed for hdrMode\n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "hdrMode", inputInfo )) {
            LOGERR("%s: CapablityCheck failed for hdrMode\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
            LOGINFO("Proceed with HDRMode\n\n");
            ret = SetTVDolbyVisionModeODM(value.c_str());
        }

        if(ret != tvERROR_NONE) {
            LOGERR("Failed to set HDRMode\n\n");
            returnResponse(false);
        }
        else {
            int retval= updateAVoutputTVParam("set","HDRMode",inputInfo,PQ_PARAM_HDR_MODE,getDolbyModeIndex(value.c_str()));
            if(retval != 0 ) {
                LOGERR("Failed to Save hdrMode mode\n");
                returnResponse(false);
            }
            LOGINFO("Exit : hdrMode successful to value: %s\n", value.c_str());
            returnResponse(true);
        }

    }

    uint32_t AVOutputTV::resetHDRMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int dolbyMode=0;
        tvError_t ret = tvERROR_NONE;

        if (parsingSetInputArgument(parameters, "HDRMode", inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("HDRMode") != 0) {
	        returnResponse(false);
	    }

        if( !isCapablityCheckPassed( "HDRMode" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for HDRMode\n", __FUNCTION__);
            returnResponse(false);
        }

        int retval= updateAVoutputTVParam("reset","HDRMode",inputInfo,PQ_PARAM_HDR_MODE,dolbyMode);
        if(retval != 0 ) {
            LOGERR("Failed to reset HDRMode\n");
            returnResponse(false);
        }
        else {
            if (isSetRequired( inputInfo.pqmode,inputInfo.source,inputInfo.format)) {
                getParamIndex( "HDRMode", inputInfo,indexInfo);
                int err = getLocalparam("HDRMode", indexInfo, dolbyMode, PQ_PARAM_HDR_MODE);
                if( err == 0 ) {
                    std::string dolbyModeValue = getDolbyModeStringFromEnum((tvDolbyMode_t)dolbyMode);
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d dolbyvalue : %s\n",__FUNCTION__,indexInfo.formatIndex, indexInfo.sourceIndex, indexInfo.pqmodeIndex, dolbyModeValue.c_str());
                    ret = SetTVDolbyVisionModeODM(dolbyModeValue.c_str());
                }
                else {
                    LOGERR("%s : GetLocalParam Failed \n",__FUNCTION__);
                    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            LOGINFO("Exit : resetHDRMode Successful to value : %d \n",dolbyMode);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getHDRModeCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps("HDRMode", info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {

            response["platformSupport"] = (info.isPlatformSupportVector[0].compare("true") == 0 ) ? true : false;

            for (index = 0; index < info.rangeVector.size(); index++) {
                rangeArray.Add(info.rangeVector[index]);
            }

            response["options"]=rangeArray;
            if ((info.pqmodeVector.front()).compare("none") != 0) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::get2PointWB(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int level = 0;
        tvPQParameterIndex_t tvPQEnum;

        inputInfo.color = parameters.HasLabel("color") ? parameters["color"].String() : "";
	    inputInfo.control = parameters.HasLabel("control") ? parameters["control"].String() : "";
        inputInfo.colorTemperature = parameters.HasLabel("colorTemperature") ? parameters["colorTemperature"].String() : "";

        if( inputInfo.color.empty() || inputInfo.control.empty() || inputInfo.colorTemperature.empty() ) {
	        LOGERR("%s : Color/Control/ColorTemp param not found!!!\n",__FUNCTION__);
            returnResponse(false);
        }

        if (parsingGetInputArgument(parameters, "WhiteBalance", inputInfo) != 0) {
            LOGINFO("%s: Failed to parse argument\n", __FUNCTION__);
            returnResponse(false);
        }

        if (getParamIndex("WhiteBalance", inputInfo,indexInfo) == -1) {
            LOGERR("%s: getParamIndex failed to get \n", __FUNCTION__);
            returnResponse(false);
        }

        if ( convertWBParamToPQEnum(inputInfo.control,inputInfo.color,tvPQEnum) != 0 ) {
            LOGINFO("%s: Control/Color Param Not Found \n",__FUNCTION__);
            returnResponse(false);
        }

        int err = getLocalparam("WhiteBalance",indexInfo,level, tvPQEnum);
        if( err == 0 ) {
            response["level"] = level;
            LOGINFO("Exit : params Value: %d \n", level);
            returnResponse(true);
        }
        else {
            returnResponse(false);
        }
    }

    uint32_t AVOutputTV::set2PointWB(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        capDetails_t inputInfo;
        int level = 0;
        tvPQParameterIndex_t tvPQEnum;
        tvColorTempSourceOffset_t colorTempSourceEnum;
        tvColorTemp_t colorTempEnum;
        int retVal = 0;
        std::string color,control,value;
        tvError_t ret = tvERROR_NONE;

        inputInfo.color = parameters.HasLabel("color") ? parameters["color"].String() : "";
	    inputInfo.control = parameters.HasLabel("control") ? parameters["control"].String() : "";
        inputInfo.colorTemperature = parameters.HasLabel("colorTemperature") ? parameters["colorTemperature"].String() : "";

        if( inputInfo.color.empty() || inputInfo.control.empty() || inputInfo.colorTemperature.empty() ) {
	        LOGERR("%s : Color/Control/ColorTemp param not found!!!\n",__FUNCTION__);
            returnResponse(false);
        }

        value = parameters.HasLabel("level") ? parameters["level"].String() : "";
        returnIfParamNotFound(parameters,"level");
        level = std::stoi(value);

        if (validateWBParameter("WhiteBalance",inputInfo.control,level) != 0) {
            LOGERR("%s: CMS Failed in range validation", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters,"WhiteBalance",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "WhiteBalance",inputInfo )) {
            LOGERR("%s: CapablityCheck failed for WhiteBalance\n", __FUNCTION__);
            returnResponse(false);
        }

        if ( convertWBParamToPQEnum(inputInfo.control,inputInfo.color,tvPQEnum) != 0 ) {
            LOGERR("%s: %s/%s Param Not Found \n",__FUNCTION__,inputInfo.component.c_str(),inputInfo.color.c_str());
            returnResponse(false);
        }    

        retVal = getColorTempEnumFromString(inputInfo.colorTemperature,colorTempEnum);
        if( ret == -1) {
            LOGERR("%s: Invalid ColorTemp : %s\n",__FUNCTION__,inputInfo.colorTemperature.c_str());
            returnResponse(false);
        }

        if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
            LOGINFO("Proceed with %s\n",__FUNCTION__);

            if ( -1 == convertToSourceOffsetEnum(inputInfo.source, colorTempSourceEnum) )
            {
                LOGERR("%s: Invalid SourceOffset : %s\n",__FUNCTION__,inputInfo.source.c_str());
                returnResponse(false);
            }
            
            LOGINFO("colorTempEnum : %d level : %d,colorTempSourceEnum:%d\n ",colorTempEnum,level,colorTempSourceEnum);
            switch(tvPQEnum)
            {
                case PQ_PARAM_WB_RED_GAIN:
                    ret = SetColorTemp_Rgain_onSource(colorTempEnum,level,colorTempSourceEnum,0);
                    break;
                case PQ_PARAM_WB_BLUE_GAIN:
                    ret = SetColorTemp_Bgain_onSource(colorTempEnum,level,colorTempSourceEnum,0);
                    break;
                case PQ_PARAM_WB_GREEN_GAIN:
                    ret = SetColorTemp_Ggain_onSource(colorTempEnum,level,colorTempSourceEnum,0);
                    break;
                case PQ_PARAM_WB_RED_OFFSET:
                    ret = SetColorTemp_Rgain_onSource(colorTempEnum,level,colorTempSourceEnum,0);
                    break;
                case PQ_PARAM_WB_BLUE_OFFSET:
                    ret = SetColorTemp_Bgain_onSource(colorTempEnum,level,colorTempSourceEnum,0);
                    break;
                case PQ_PARAM_WB_GREEN_OFFSET:
                    ret = SetColorTemp_Bgain_onSource(colorTempEnum,level,colorTempSourceEnum,0);
                    break;
                default:
                    LOGERR("%s : wrong color/control values\n",__FUNCTION__);
                    returnResponse(false);
                    break;
            }
        }      

        if(ret != tvERROR_NONE) {
            LOGERR("%s: Failed to set WhiteBalance\n",__FUNCTION__);
            returnResponse(false);
        }
        else  {
            retVal= updateAVoutputTVParam("set","WhiteBalance",inputInfo,tvPQEnum,level);
            if(retVal != 0 ) {
                LOGERR("%s : Failed to Save WB %s/%s : %d to ssm_data\n",__FUNCTION__,inputInfo.control.c_str(),inputInfo.color.c_str(),level);
		        returnResponse(false);
            }
            LOGINFO("Exit : set2PointWB %s/%s successful to value: %d\n", inputInfo.control.c_str(),inputInfo.color.c_str(),level);
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::reset2PointWB(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        capDetails_t inputInfo;
        tvPQParameterIndex_t tvPQEnum;
        tvColorTemp_t colorTempEnum;
        int retVal = 0;
        int level = 0;
        std::string color,control;
        inputInfo.color = parameters.HasLabel("color") ? parameters["color"].String() : "";
	    inputInfo.control = parameters.HasLabel("control") ? parameters["control"].String() : "";
        inputInfo.colorTemperature = parameters.HasLabel("colorTemperature") ? parameters["colorTemperature"].String() : "";


        if (parsingSetInputArgument(parameters,"WhiteBalance",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "WhiteBalance",inputInfo )) {
            LOGERR("%s: CapablityCheck failed for WhiteBalance\n", __FUNCTION__);
            returnResponse(false);
        }

        for( int colorTempIndex = tvColorTemp_STANDARD;colorTempIndex <= tvColorTemp_USER; colorTempIndex++)
        {
            for( int colorIndex= tvWB_COLOR_RED; colorIndex < tvWB_COLOR_MAX; colorIndex++)
            {
                for(int controlIndex = tvWB_CONTROL_GAIN;controlIndex < tvWB_CONTROL_MAX;controlIndex++)
                {
                    inputInfo.control = getWBControlStringFromEnum((tvWBControl_t)controlIndex);
                    inputInfo.color   = getWBColorStringFromEnum((tvWBColor_t)colorIndex);
                    inputInfo.colorTemperature = getColorTemperatureStringFromEnum((tvColorTemp_t)colorTempIndex);

                    if ( convertWBParamToPQEnum(inputInfo.control,inputInfo.color,tvPQEnum) != 0 ) {
                        LOGERR("%s: %s/%s Param Not Found \n",__FUNCTION__,inputInfo.control.c_str(),inputInfo.color.c_str());
                        returnResponse(false);
                    }    

                    retVal |= getColorTempEnumFromString(inputInfo.colorTemperature,colorTempEnum);
                    if( retVal == -1) {
                        LOGERR("%s: Invalid ColorTemp : %s\n",__FUNCTION__,inputInfo.colorTemperature.c_str());
                        returnResponse(false);
                    }

                    retVal |= updateAVoutputTVParam("reset","WhiteBalance",inputInfo,tvPQEnum,level);
                }
            }
        } 

        if( retVal != 0 ) {
            LOGWARN("Failed to reset WhiteBalance\n");
            returnResponse(false);
        }
        else {        
            LOGINFO("Exit : reset2PointWB successful \n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::get2PointWBCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;
        JsonArray colorArray;
        JsonArray componentArray;

        JsonObject gainInfo;
        JsonObject offsetInfo;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps("CMS",info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            response["platformSupport"] = (info.isPlatformSupportVector[0].compare("true") == 0)  ? true : false;

            gainInfo["from"] = stoi(info.rangeVector[0]);
            gainInfo["to"]   = stoi(info.rangeVector[1]);
            response["gainInfo"]=gainInfo;

            offsetInfo["from"] = stoi(info.rangeVector[0]);
            offsetInfo["to"]   = stoi(info.rangeVector[1]);
            response["offsetInfo"]=offsetInfo;
            
            

            if ((info.pqmodeVector.front()).compare("none") != 0) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }

            if ((info.colorVector.front()).compare("none") != 0) {
                for (index = 0; index < info.colorVector.size(); index++) {
                    colorArray.Add(info.colorVector[index]);
                }
                response["colorInfo"]=colorArray;
            }

            if ((info.componentVector.front()).compare("none") != 0) {
                for (index = 0; index < info.componentVector.size(); index++) {
                    componentArray.Add(info.componentVector[index]);
                }
                response["componentInfo"]=componentArray;
            }

            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getAutoBacklightModeCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        capVectors_t info;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps("AutoBacklightMode",info);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
	    else {

            response["platformSupport"] = (info.isPlatformSupportVector[0].compare("true") == 0 ) ? true : false;

            for (index = 0; index < info.rangeVector.size(); index++) {
                rangeArray.Add(info.rangeVector[index]);
	        }

            response["options"]=rangeArray;

            if (info.pqmodeVector.front().compare("none") != 0) {
                for (index = 0; index < info.pqmodeVector.size(); index++) {
                    pqmodeArray.Add(info.pqmodeVector[index]);
                }
                response["pictureModeInfo"]=pqmodeArray;
            }
            if ((info.sourceVector.front()).compare("none") != 0) {
                for (index = 0; index < info.sourceVector.size(); index++) {
                    sourceArray.Add(info.sourceVector[index]);
                }
                response["videoSourceInfo"]=sourceArray;
            }
            if ((info.formatVector.front()).compare("none") != 0) {
                for (index = 0; index < info.formatVector.size(); index++) {
                    formatArray.Add(info.formatVector[index]);
                }
                response["videoFormatInfo"]=formatArray;
            }
            LOGINFO("Exit\n");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::setAutoBacklightMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        tvBacklightMode_t mode = tvBacklightMode_AMBIENT;
        capDetails_t inputInfo;


        value = parameters.HasLabel("mode") ? parameters["mode"].String() : "";
        returnIfParamNotFound(parameters,"mode");

        if (validateInputParameter("AutoBacklightMode",value) != 0) {
            LOGERR("%s: Range validation failed for AspectRatio\n", __FUNCTION__);
            returnResponse(false);
        }

        if (parsingSetInputArgument(parameters,"AutoBacklightMode",inputInfo) != 0) {
            LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
            returnResponse(false);
        }

	    if( !isCapablityCheckPassed( "AutoBacklightMode",inputInfo )) {
            LOGERR("%s: CapablityCheck failed for AspectRatio\n", __FUNCTION__);
            returnResponse(false);
        }

        if(!value.compare("Manual")) {
            mode = tvBacklightMode_MANUAL;
        }
        else if (!value.compare("Ambient")) {
            mode = tvBacklightMode_AMBIENT;
        }
        else {
            returnResponse(false);
        }
        
        tvError_t ret = SetCurrentBacklightMode (mode);

        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            //Save AutoBacklightMode to localstore

            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGERR("setLocalParam for %s Failed : %s\n", AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
                returnResponse(false);
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM, value.c_str());
            }
            LOGINFO("Exit : SetAutoBacklightMode() value : %s\n",value.c_str());
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::getAutoBacklightMode(const JsonObject& parameters, JsonObject& response)
    {

        TR181_ParamData_t param;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM, &param);
        if (err!= tr181Success) {
            returnResponse(false);
        }
        else {
            std::string s;
            s+=param.value;
            response["mode"] = s;
            LOGINFO("Exit getAutoBacklightMode(): %s\n",s.c_str());
            returnResponse(true);
        }
    
    }

    uint32_t AVOutputTV::resetAutoBacklightMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        tvError_t ret = tvERROR_NONE;

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

    uint32_t AVOutputTV::getVideoSource(const JsonObject& parameters,JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;

        tvError_t ret = GetCurrentSource(&currentSource);
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


    uint32_t AVOutputTV::getVideoContentType(const JsonObject & parameters, JsonObject & response)
    {
        JsonArray rangeArray;

        response["currentFilmMakerMode"] = filmMakerMode;

        if (getCapabilitySource(rangeArray) == 0) {
            response["currentFilmMakerModeSources"] = rangeArray;
        }

        returnResponse(true);
    }

    void AVOutputTV::InitializeIARM()
    {
        AVOutputBase::InitializeIARM();
#if !defined (HDMIIN_4K_ZOOM)
        if (Utils::IARM::init()) {
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

}//namespace Plugin
}//namespace WPEFramework
//}
