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

        registerMethod("getBacklightCapsV2", &AVOutputTV::getBacklightCapsV2, this);
        registerMethod("getBrightnessCapsV2", &AVOutputTV::getBrightnessCapsV2, this);
        registerMethod("getContrastCapsV2", &AVOutputTV::getContrastCapsV2, this);
        registerMethod("getSharpnessCapsV2", &AVOutputTV::getSharpnessCapsV2, this);
        registerMethod("getSaturationCapsV2", &AVOutputTV::getSaturationCapsV2, this);
        registerMethod("getHueCapsV2", &AVOutputTV::getHueCapsV2, this);
        registerMethod("getLowLatencyStateCapsV2", &AVOutputTV::getLowLatencyStateCapsV2, this);
        registerMethod("getColorTemperatureCapsV2", &AVOutputTV::getColorTemperatureCapsV2, this);
        registerMethod("getBacklightDimmingModeCapsV2", &AVOutputTV::getBacklightDimmingModeCapsV2, this);
        registerMethod("getZoomModeCapsV2", &AVOutputTV::getZoomModeCapsV2, this);
        registerMethod("getDolbyVisionCalibrationCaps", &AVOutputTV::getDolbyVisionCalibrationCaps, this);
        registerMethod("getPictureModeCapsV2", &AVOutputTV::getPictureModeCapsV2, this);
        registerMethod("getAutoBacklightModeCapsV2", &AVOutputTV::getAutoBacklightModeCapsV2, this);
        registerMethod("getCMSCapsV2", &AVOutputTV::getCMSCapsV2, this);
        registerMethod("get2PointWBCapsV2", &AVOutputTV::get2PointWBCapsV2, this);
        registerMethod("getSDRGammaCaps", &AVOutputTV::getSDRGammaCaps, this);

        registerMethod("getPrecisionDetailCaps", &AVOutputTV::getPrecisionDetailCaps, this);
        registerMethod("getPrecisionDetail", &AVOutputTV::getPrecisionDetail, this);
        registerMethod("setPrecisionDetail", &AVOutputTV::setPrecisionDetail, this);
        registerMethod("resetPrecisionDetail", &AVOutputTV::resetPrecisionDetail, this);

        registerMethod("getLocalContrastEnhancementCaps", &AVOutputTV::getLocalContrastEnhancementCaps, this);
        registerMethod("getLocalContrastEnhancement", &AVOutputTV::getLocalContrastEnhancement, this);
        registerMethod("setLocalContrastEnhancement", &AVOutputTV::setLocalContrastEnhancement, this);
        registerMethod("resetLocalContrastEnhancement", &AVOutputTV::resetLocalContrastEnhancement, this);

        registerMethod("getMPEGNoiseReductionCaps", &AVOutputTV::getMPEGNoiseReductionCaps, this);
        registerMethod("getMPEGNoiseReduction", &AVOutputTV::getMPEGNoiseReduction, this);
        registerMethod("setMPEGNoiseReduction", &AVOutputTV::setMPEGNoiseReduction, this);
        registerMethod("resetMPEGNoiseReduction", &AVOutputTV::resetMPEGNoiseReduction, this);

        registerMethod("getDigitalNoiseReductionCaps", &AVOutputTV::getDigitalNoiseReductionCaps, this);
        registerMethod("getDigitalNoiseReduction", &AVOutputTV::getDigitalNoiseReduction, this);
        registerMethod("setDigitalNoiseReduction", &AVOutputTV::setDigitalNoiseReduction, this);
        registerMethod("resetDigitalNoiseReduction", &AVOutputTV::resetDigitalNoiseReduction, this);

        registerMethod("getMEMCCaps", &AVOutputTV::getMEMCCaps, this);
        registerMethod("getMEMC", &AVOutputTV::getMEMC, this);
        registerMethod("setMEMC", &AVOutputTV::setMEMC, this);
        registerMethod("resetMEMC", &AVOutputTV::resetMEMC, this);

        registerMethod("getAISuperResolutionCaps", &AVOutputTV::getAISuperResolutionCaps, this);
        registerMethod("getAISuperResolution", &AVOutputTV::getAISuperResolution, this);
        registerMethod("setAISuperResolution", &AVOutputTV::setAISuperResolution, this);
        registerMethod("resetAISuperResolution", &AVOutputTV::resetAISuperResolution, this);

        registerMethod("getMultiPointWBCaps", &AVOutputTV::getMultiPointWBCaps, this);

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

    // Shared zoom mode mappings
    static const std::unordered_map<tvDisplayMode_t, std::string> zoomModeReverseMap = {
        {tvDisplayMode_16x9,     "TV 16X9 STRETCH"},
        {tvDisplayMode_4x3,      "TV 4X3 PILLARBOX"},
        {tvDisplayMode_NORMAL,   "TV NORMAL"},
        {tvDisplayMode_DIRECT,   "TV DIRECT"},
        {tvDisplayMode_AUTO,     "TV AUTO"},
        {tvDisplayMode_ZOOM,     "TV ZOOM"},
        {tvDisplayMode_FULL,     "TV FULL"}
    };
    static const std::unordered_map<std::string, tvDisplayMode_t> zoomModeMap = {
        {"TV 16X9 STRETCH", tvDisplayMode_16x9},
        {"TV 4X3 PILLARBOX", tvDisplayMode_4x3},
        {"TV NORMAL",        tvDisplayMode_NORMAL},
        {"TV DIRECT",        tvDisplayMode_DIRECT},
        {"TV AUTO",          tvDisplayMode_AUTO},
        {"TV ZOOM",          tvDisplayMode_ZOOM},
        {"TV FULL",          tvDisplayMode_FULL}
    };
    static const std::unordered_map<int, std::string> dimmingModeReverseMap = {
        { tvDimmingMode_Fixed, "Fixed" },
        { tvDimmingMode_Local, "Local" },
        { tvDimmingMode_Global, "Global" }
    };
    static const std::unordered_map<std::string, int> dimmingModeMap = {
        { "Fixed", tvDimmingMode_Fixed },
        { "Local", tvDimmingMode_Local },
        { "Global", tvDimmingMode_Global }
    };

    bool AVOutputTV::getPQParamFromContext(const JsonObject& parameters,
        const std::string& paramName,
        tvPQParameterIndex_t paramType,
        int& outValue)
    {
        tvConfigContext_t validContext = getValidContextFromGetParameters(parameters, paramName);
        if ((validContext.videoSrcType == VIDEO_SOURCE_ALL &&
             validContext.videoFormatType == VIDEO_FORMAT_NONE &&
             validContext.pq_mode == PQ_MODE_INVALID))
        {
            LOGWARN("No Valid context for get %s", paramName.c_str());
            return false;
        }

        paramIndex_t indexInfo
        {
            .sourceIndex = static_cast<uint8_t>(validContext.videoSrcType),
            .pqmodeIndex = static_cast<uint8_t>(validContext.pq_mode),
            .formatIndex = static_cast<uint8_t>(validContext.videoFormatType)
        };

        int value = 0;
        tvError_t err = static_cast<tvError_t>(getLocalparam(paramName.c_str(), indexInfo, value, paramType));
        if (err == tvERROR_NONE) {
            outValue = value;
            return true;
        }

        LOGERR("getLocalparam failed for %s with error code %d", paramName.c_str(), err);
        return false;
    }

    bool AVOutputTV::getEnumPQParamString(
        const JsonObject& parameters,
        const std::string& paramName,
        tvPQParameterIndex_t pqType,
        const std::unordered_map<int, std::string>& enumToStrMap,
        std::string& outStr)
    {
        LOGINFO("getEnumPQParamString Entry for %s\n", paramName.c_str());

        tvConfigContext_t validContext = getValidContextFromGetParameters(parameters, paramName);
        if ((validContext.videoSrcType == VIDEO_SOURCE_ALL &&
            validContext.videoFormatType == VIDEO_FORMAT_NONE &&
            validContext.pq_mode == PQ_MODE_INVALID))
        {
            LOGWARN("No valid context for get %s", paramName.c_str());
            return false;
        }

        paramIndex_t indexInfo {
            .sourceIndex = static_cast<uint8_t>(validContext.videoSrcType),
            .pqmodeIndex = static_cast<uint8_t>(validContext.pq_mode),
            .formatIndex = static_cast<uint8_t>(validContext.videoFormatType)
        };

        int paramValue = 0;
        int err = getLocalparam(paramName, indexInfo, paramValue, pqType);
        if (err != 0) {
            LOGERR("Failed to get %s from localparam", paramName.c_str());
            return false;
        }

        auto it = enumToStrMap.find(paramValue);
        if (it != enumToStrMap.end()) {
            outStr = it->second;
            LOGINFO("%s = %s", paramName.c_str(), outStr.c_str());
            return true;
        } else {
            LOGERR("Enum value %d not found in map for %s", paramValue, paramName.c_str());
            return false;
        }
    }

    bool AVOutputTV::setEnumPQParam(const JsonObject& parameters,
        const std::string& inputKey,
        const std::string& paramName,
        const std::unordered_map<std::string, int>& valueMap,
        tvPQParameterIndex_t paramType,
        std::function<tvError_t(int)> halSetter)
    {
        if (!parameters.HasLabel(inputKey.c_str())) {
        LOGERR("Missing input field: %s", inputKey.c_str());
        return false;
        }

        std::string value = parameters[inputKey.c_str()].String();
        auto it = valueMap.find(value);
        if (it == valueMap.end()) {
        LOGERR("Invalid value '%s' for parameter: %s", value.c_str(), inputKey.c_str());
        return false;
        }

        int intVal = it->second;

        // Only call HAL for current system context
        if (isSetRequiredForParam(parameters, paramName)) {
            LOGINFO("Calling HAL for %s = %s intVal %d", paramName.c_str(), value.c_str(), intVal);
            tvError_t ret = halSetter(intVal);
            if (ret != tvERROR_NONE) {
            LOGERR("HAL setter failed for %s", paramName.c_str());
            return false;
            }
        }

        // Persist the parameter contextually
        int result = updateAVoutputTVParamV2("set", paramName, parameters, paramType, intVal);
        if (result != 0) {
        LOGERR("Persistence failed for %s", paramName.c_str());
        return false;
        }

        LOGINFO("setEnumPQParam successful: %s = %s", paramName.c_str(), value.c_str());
        return true;
    }

    bool AVOutputTV::setIntPQParam(const JsonObject& parameters, const std::string& paramName,
        tvPQParameterIndex_t pqType, tvSetFunction halSetter, int maxCap)
    {
        LOGINFO("Entry: %s\n", paramName.c_str());
        int paramValue = 0;
        tvError_t ret = tvERROR_NONE;
        std::string value = "";
        std::string lowerParamName = paramName;
        std::transform(lowerParamName.begin(), lowerParamName.end(), lowerParamName.begin(), ::tolower);

        if (!parameters.HasLabel(lowerParamName.c_str())) {
        LOGERR("%s: Missing parameter: %s", __FUNCTION__, lowerParamName.c_str());
        return false;
        }

        value = parameters[lowerParamName.c_str()].String();

        try {
        paramValue = std::stoi(value);
        } catch (const std::exception& e) {
        LOGERR("Invalid %s value: %s. Exception: %s", paramName.c_str(), value.c_str(), e.what());
        return false;
        }

        if (paramValue < 0 || paramValue > maxCap) {
        LOGERR("Input value %d is out of range (0 - %d) for %s", paramValue, maxCap, paramName.c_str());
        return false;
        }

        if (isSetRequiredForParam(parameters, paramName)) {
            LOGINFO("Proceed with set%s\n", paramName.c_str());
            ret = halSetter(paramValue);
            if (ret != tvERROR_NONE){
                LOGERR("Failed to set %s\n", paramName.c_str());
                return false;
            }
        }

        int retval = updateAVoutputTVParamV2("set", paramName, parameters, pqType, paramValue);
        if (retval != 0) {
        LOGERR("Failed to Save %s to ssm_data. retval: %d\n", paramName.c_str(), retval);
        return false;
        }

        LOGINFO("Exit: set%s successful to value: %d\n", paramName.c_str(), paramValue);
        return true;
    }

    uint32_t AVOutputTV::getPQCapabilityWithContext(
        const std::function<tvError_t(tvContextCaps_t**, int*)>& getCapsFunc,
        const JsonObject& parameters,
        JsonObject& response)
    {
        int max_value = 0;
        tvContextCaps_t* context_caps = nullptr;

        // Call the HAL function
        tvError_t result = getCapsFunc(&context_caps, &max_value);
        LOGWARN("AVOutputPlugins: %s: result: %d", __FUNCTION__, result);

        if (result != tvERROR_NONE) {
            returnResponse(false);
        }

        response["platformSupport"] = true;

        if (max_value > 0) {
            JsonObject rangeInfo;
            rangeInfo["from"] = 0;
            rangeInfo["to"] = max_value;
            response["rangeInfo"] = rangeInfo;
        }

        response["context"] = parseContextCaps(context_caps);

        returnResponse(true);
    }


    JsonObject AVOutputTV::parseContextCaps(tvContextCaps_t* context_caps) {
        JsonObject contextObj;
        if (context_caps && context_caps->num_contexts > 0) {
            for (size_t i = 0; i < context_caps->num_contexts; ++i) {
                int pqMode = context_caps->contexts[i].pq_mode;
                int videoFormat = context_caps->contexts[i].videoFormatType;
                int videoSource = context_caps->contexts[i].videoSrcType;

                auto pqModeIt = AVOutputTV::pqModeMap.find(pqMode);
                auto videoFormatIt = AVOutputTV::videoFormatMap.find(videoFormat);
                auto videoSrcIt = AVOutputTV::videoSrcMap.find(videoSource);

                if (pqModeIt != AVOutputTV::pqModeMap.end() &&
                    videoFormatIt != AVOutputTV::videoFormatMap.end() &&
                    videoSrcIt != AVOutputTV::videoSrcMap.end()) {

                    const char* pqModeStr = pqModeIt->second.c_str();
                    const char* videoFormatStr = videoFormatIt->second.c_str();
                    const char* videoSrcStr = videoSrcIt->second.c_str();

                    if (!contextObj.HasLabel(pqModeStr)) {
                        contextObj[pqModeStr] = JsonObject();
                    }
                    JsonObject pqModeObj = contextObj[pqModeStr].Object();

                    if (!pqModeObj.HasLabel(videoFormatStr)) {
                        pqModeObj[videoFormatStr] = JsonArray();
                    }
                    JsonArray formatArray = pqModeObj[videoFormatStr].Array();
                    // **Manually check for existence before adding**
                    bool exists = false;
                    for (size_t j = 0; j < formatArray.Length(); ++j) {
                        if (strcmp(formatArray[j].String().c_str(), videoSrcStr) == 0) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        formatArray.Add(videoSrcStr);
                    }
                    // Update objects
                    pqModeObj[videoFormatStr] = formatArray;
                    contextObj[pqModeStr] = pqModeObj;
                }
            }
        }
        return contextObj;
    }

    uint32_t AVOutputTV::getBacklightCapsV2(const JsonObject& parameters, JsonObject& response) {
        return getPQCapabilityWithContext([this]( tvContextCaps_t** context_caps, int* max_backlight) {
#if HAL_NOT_READY
        return this->GetBacklightCaps(max_backlight, context_caps);
#else
        return GetBacklightCaps(max_backlight, context_caps);
#endif
        }, parameters, response);
    }

    uint32_t AVOutputTV::getBrightnessCapsV2(const JsonObject& parameters, JsonObject& response) {
        return getPQCapabilityWithContext([this]( tvContextCaps_t** context_caps, int* max_brightness) {
#if HAL_NOT_READY
        return this->GetBrightnessCaps(max_brightness, context_caps);
#else
        return GetBrightnessCaps(max_brightness, context_caps);
#endif
        },
        parameters, response);
    }

    uint32_t AVOutputTV::getContrastCapsV2(const JsonObject& parameters, JsonObject& response) {
        return getPQCapabilityWithContext([this](tvContextCaps_t** context_caps, int* max_contrast) {
#if HAL_NOT_READY
        return this->GetContrastCaps(max_contrast, context_caps);
#else
        return GetContrastCaps(max_contrast, context_caps);
#endif
        },
        parameters, response);
    }

    uint32_t AVOutputTV::getSharpnessCapsV2(const JsonObject& parameters, JsonObject& response) {
        return getPQCapabilityWithContext([this](tvContextCaps_t** context_caps, int* max_sharpness) {
#if HAL_NOT_READY
        return this->GetSharpnessCaps(max_sharpness, context_caps);
#else
        return GetSharpnessCaps(max_sharpness, context_caps);
#endif
        },
        parameters, response);
    }

    uint32_t AVOutputTV::getSaturationCapsV2(const JsonObject& parameters, JsonObject& response) {
        return getPQCapabilityWithContext([this](tvContextCaps_t** context_caps, int* max_saturation) {
#if HAL_NOT_READY
        return this->GetSaturationCaps(max_saturation, context_caps);
#else
        return GetSaturationCaps(max_saturation, context_caps);
#endif
        },
        parameters, response);
    }

    uint32_t AVOutputTV::getHueCapsV2(const JsonObject& parameters, JsonObject& response) {
        return getPQCapabilityWithContext([this]( tvContextCaps_t** context_caps, int* max_hue) {
#if HAL_NOT_READY
        return this->GetHueCaps(max_hue, context_caps);
#else
        return GetHueCaps(max_hue, context_caps);
#endif
        },
        parameters, response);
    }

    uint32_t AVOutputTV::getPrecisionDetailCaps(const JsonObject& parameters, JsonObject& response) {
        return getPQCapabilityWithContext([this](tvContextCaps_t** context_caps, int* max_precision) {
#if HAL_NOT_READY
        return this->GetPrecisionDetailCaps(max_precision, context_caps);
#else
        return GetPrecisionDetailCaps(max_precision, context_caps);
#endif
        },
        parameters, response);
    }

    uint32_t AVOutputTV::getLocalContrastEnhancementCaps(const JsonObject& parameters, JsonObject& response) {
        return getPQCapabilityWithContext([this](tvContextCaps_t** context_caps, int* max_val) {
#if HAL_NOT_READY
        return this->GetLocalContrastEnhancementCaps(max_val, context_caps);
#else
        return GetLocalContrastEnhancementCaps(max_val, context_caps);
#endif
        },
        parameters, response);
    }

    uint32_t AVOutputTV::getMPEGNoiseReductionCaps(const JsonObject& parameters, JsonObject& response) {
        return getPQCapabilityWithContext([this](tvContextCaps_t** context_caps, int* max_val) {
#if HAL_NOT_READY
        return this->GetMPEGNoiseReductionCaps(max_val, context_caps);
#else
        return GetMPEGNoiseReductionCaps(max_val, context_caps);
#endif
        },
        parameters, response);
    }

    uint32_t AVOutputTV::getDigitalNoiseReductionCaps(const JsonObject& parameters, JsonObject& response) {
        return getPQCapabilityWithContext([this](tvContextCaps_t** context_caps, int* max_val) {
#if HAL_NOT_READY
        return this->GetDigitalNoiseReductionCaps(max_val, context_caps);
#else
        return GetDigitalNoiseReductionCaps(max_val, context_caps);
#endif
        },
        parameters, response);
    }

    uint32_t AVOutputTV::getAISuperResolutionCaps(const JsonObject& parameters, JsonObject& response) {
        return getPQCapabilityWithContext([this](tvContextCaps_t** context_caps, int* max_val) {
#if HAL_NOT_READY
        return this->GetAISuperResolutionCaps(max_val, context_caps);
#else
        return GetAISuperResolutionCaps(max_val, context_caps);
#endif
        },
        parameters, response);
    }

    uint32_t AVOutputTV::getMultiPointWBCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        int num_hal_matrix_points = 0;
        int rgb_min = 0, rgb_max = 0;
        int num_ui_matrix_points = 0;
        double* ui_matrix_positions = nullptr;
        tvContextCaps_t* context_caps = nullptr;

        tvError_t ret = GetMultiPointWBCaps(
            &num_hal_matrix_points,
            &rgb_min,
            &rgb_max,
            &num_ui_matrix_points,
            &ui_matrix_positions,
            &context_caps
        );

        if (ret != tvERROR_NONE) {
            LOGWARN("GetMultiPointWBCaps failed: %s", getErrorString(ret).c_str());
            returnResponse(false);
        }
        response["platformSupport"] = true;

        response["numHalMatrixPoints"] = num_hal_matrix_points;
        response["rgbMin"] = rgb_min;
        response["rgbMax"] = rgb_max;
        response["numUiMatrixPoints"] = num_ui_matrix_points;

        // Add UI matrix positions
        JsonArray uiPosArray;
        for (int i = 0; i < num_ui_matrix_points; ++i) {
            uiPosArray.Add(ui_matrix_positions[i]);
        }
        response["uiMatrixPositions"] = uiPosArray;
        response["context"] = parseContextCaps(context_caps);
#if HAL_NOT_READY
        // TODO:: Review cleanup once HAL is available, as memory will be allocated in HAL.
        delete[] ui_matrix_positions;
#endif
        LOGINFO("Exit\n");
        returnResponse(true);
    }

    uint32_t AVOutputTV::getMEMCCaps(const JsonObject& parameters, JsonObject& response) {
        return getPQCapabilityWithContext([this](tvContextCaps_t** context_caps, int* max_val) {
#if HAL_NOT_READY
            return this->GetMEMCCaps(max_val, context_caps);
#else
            return GetMEMCCaps(max_val, context_caps);
#endif
        },
        parameters, response);
    }

    uint32_t AVOutputTV::getLowLatencyStateCapsV2(const JsonObject& parameters, JsonObject& response) {
        return getPQCapabilityWithContext([this](tvContextCaps_t** context_caps, int* max_latency) {
#if HAL_NOT_READY
        return this->GetLowLatencyStateCaps(max_latency, context_caps);
#else
        return GetLowLatencyStateCaps(max_latency, context_caps);
#endif
        },
        parameters, response);
    }

    // Forward lookup: string → enum
    const std::unordered_map<std::string, int> colorTempMap = {
        {"Standard",            tvColorTemp_STANDARD},
        {"Warm",                tvColorTemp_WARM},
        {"Cold",                tvColorTemp_COLD},
        {"UserDefined",         tvColorTemp_USER},
        {"Supercold",           tvColorTemp_SUPERCOLD},
        {"BoostStandard",       tvColorTemp_BOOST_STANDARD},
        {"BoostWarm",           tvColorTemp_BOOST_WARM},
        {"BoostCold",           tvColorTemp_BOOST_COLD},
        {"BoostUserDefined",    tvColorTemp_BOOST_USER},
        {"BoostSupercold",      tvColorTemp_BOOST_SUPERCOLD}
    };

    // Reverse lookup: enum → string
    const std::unordered_map<int, std::string> colorTempReverseMap = {
        {tvColorTemp_STANDARD,          "Standard"},
        {tvColorTemp_WARM,              "Warm"},
        {tvColorTemp_COLD,              "Cold"},
        {tvColorTemp_USER,              "UserDefined"},
        {tvColorTemp_SUPERCOLD,         "Supercold"},
        {tvColorTemp_BOOST_STANDARD,    "BoostStandard"},
        {tvColorTemp_BOOST_WARM,        "BoostWarm"},
        {tvColorTemp_BOOST_COLD,        "BoostCold"},
        {tvColorTemp_BOOST_USER,        "BoostUserDefined"},
        {tvColorTemp_BOOST_SUPERCOLD,   "BoostSupercold"}
    };

    uint32_t AVOutputTV::getColorTemperatureCapsV2(const JsonObject& parameters, JsonObject& response) {
        tvColorTemp_t* color_temp = nullptr;
        size_t num_color_temp = 0;
        tvContextCaps_t* context_caps = nullptr;

        tvError_t err = GetColorTemperatureCaps(&color_temp, &num_color_temp, &context_caps);
        if (err != tvERROR_NONE) {
            return err;
        }

        response["platformSupport"] = true;

        JsonArray optionsArray;
        for (size_t i = 0; i < num_color_temp; ++i) {
            auto it = colorTempReverseMap.find(color_temp[i]);
            if (it != colorTempReverseMap.end()) {
                optionsArray.Add(it->second);
            }
        }
        response["options"] = optionsArray;
        response["context"] = parseContextCaps(context_caps);

    #if HAL_NOT_READY
        free(color_temp);
    #endif

        returnResponse(true);
    }

    uint32_t AVOutputTV::getSDRGammaCaps(const JsonObject& parameters, JsonObject& response)
    {
        tvSdrGamma_t* sdr_gamma = nullptr;
        size_t num_sdr_gamma = 0;
        tvContextCaps_t* context_caps = nullptr;

        tvError_t err = GetSdrGammaCaps(&sdr_gamma, &num_sdr_gamma, &context_caps);
        if (err != tvERROR_NONE) {
            return err;
        }

        response["platformSupport"] = true;

        JsonArray optionsArray;
        for (size_t i = 0; i < num_sdr_gamma; ++i) {
            switch (sdr_gamma[i]) {
                case tvSdrGamma_1_8:     optionsArray.Add("1.8"); break;
                case tvSdrGamma_1_9:     optionsArray.Add("1.9"); break;
                case tvSdrGamma_2_0:     optionsArray.Add("2.0"); break;
                case tvSdrGamma_2_1:     optionsArray.Add("2.1"); break;
                case tvSdrGamma_2_2:     optionsArray.Add("2.2"); break;
                case tvSdrGamma_2_3:     optionsArray.Add("2.3"); break;
                case tvSdrGamma_2_4:     optionsArray.Add("2.4"); break;
                case tvSdrGamma_BT_1886: optionsArray.Add("BT.1886"); break;
                default: break;
            }
        }
        response["options"] = optionsArray;

        response["context"] = parseContextCaps(context_caps);

    #if HAL_NOT_READY
        free(sdr_gamma);
    #endif

        returnResponse(true);
    }

    uint32_t AVOutputTV::getBacklightDimmingModeCapsV2(const JsonObject& parameters, JsonObject& response)
    {
        tvDimmingMode_t* dimming_mode = nullptr;
        size_t num_dimming_mode = 0;
        tvContextCaps_t* context_caps = nullptr;

        tvError_t err = GetTVDimmingModeCaps(&dimming_mode, &num_dimming_mode, &context_caps);
        if (err != tvERROR_NONE) {
            return err;
        }

        response["platformSupport"] = true;

        JsonArray optionsArray;
        for (size_t i = 0; i < num_dimming_mode; ++i) {
            auto it = dimmingModeReverseMap.find(dimming_mode[i]);
            if (it != dimmingModeReverseMap.end()) {
                optionsArray.Add(it->second);
            }
        }
        response["options"] = optionsArray;

        response["context"] = parseContextCaps(context_caps);

    #if HAL_NOT_READY
        free(dimming_mode);
    #endif

        returnResponse(true);
    }

    uint32_t AVOutputTV::getZoomModeCapsV2(const JsonObject& parameters, JsonObject& response)
    {
        response["platformSupport"] = true;

        JsonArray optionsArray;
        for (size_t i = 0; i < m_numAspectRatio; ++i) {
            auto it = zoomModeReverseMap.find(m_aspectRatio[i]);
            if (it != zoomModeReverseMap.end()) {
                optionsArray.Add(it->second);
            }
        }
        response["options"] = optionsArray;

        response["context"] = parseContextCaps(m_aspectRatioCaps);

    #if HAL_NOT_READY
        free(m_aspectRatio);
    #endif

        returnResponse(true);
    }

    uint32_t AVOutputTV::getPictureModeCapsV2(const JsonObject& parameters, JsonObject& response)
    {
        response["platformSupport"] = true;

        JsonArray optionsArray;
        for (size_t i = 0; i < m_numPictureModes; ++i) {
            auto it = pqModeMap.find(m_pictureModes[i]);
            if (it != pqModeMap.end()) {
                optionsArray.Add(it->second);
            }
        }
        response["options"] = optionsArray;

        response["context"] = parseContextCaps(m_pictureModeCaps);

    #if HAL_NOT_READY
        free(m_pictureModes);
    #endif

        returnResponse(true);
    }

    uint32_t AVOutputTV::getAutoBacklightModeCapsV2(const JsonObject& parameters, JsonObject& response)
    {
        response["platformSupport"] = true;

        JsonArray optionsArray;
        for (size_t i = 0; i < m_numBacklightModes; ++i) {
            switch (m_backlightModes[i]) {
                case tvBacklightMode_MANUAL:
                    optionsArray.Add("Manual");
                    break;
                case tvBacklightMode_AMBIENT:
                    optionsArray.Add("Ambient");
                    break;
                case tvBacklightMode_ECO:
                    optionsArray.Add("Eco");
                    break;
                default:
                    LOGINFO("Unknown backlightMode option\n");
                    break;
            }
        }
        response["options"] = optionsArray;

        response["context"] = parseContextCaps(m_backlightModeCaps);

    #if HAL_NOT_READY
        // TODO: Review cleanup once HAL is available, as memory will be allocated in HAL.
        free(m_backlightModes);
    #endif

        returnResponse(true);
    }

    uint32_t AVOutputTV::getDolbyVisionCalibrationCaps(const JsonObject& parameters, JsonObject& response)
    {
        tvDVCalibrationSettings_t* min_values = nullptr;
        tvDVCalibrationSettings_t* max_values = nullptr;
        tvContextCaps_t* context_caps = nullptr;

        if (GetDVCalibrationCaps(&min_values, &max_values, &context_caps) != tvERROR_NONE) {
            returnResponse(false);
        }

        // Set platform support
        response["platformSupport"] = true;

        // Add all range fields (flattened as per expected JSON)
        response["rangeTmax"] = JsonObject({{"from", min_values->Tmax}, {"to", max_values->Tmax}});
        response["rangeTmin"] = JsonObject({{"from", min_values->Tmin}, {"to", max_values->Tmin}});
        response["rangeTgamma"] = JsonObject({{"from", min_values->Tgamma}, {"to", max_values->Tgamma}});
        response["rangeRx"] = JsonObject({{"from", min_values->Rx}, {"to", max_values->Rx}});
        response["rangeRy"] = JsonObject({{"from", min_values->Ry}, {"to", max_values->Ry}});
        response["rangeGx"] = JsonObject({{"from", min_values->Gx}, {"to", max_values->Gx}});
        response["rangeGy"] = JsonObject({{"from", min_values->Gy}, {"to", max_values->Gy}});
        response["rangeBx"] = JsonObject({{"from", min_values->Bx}, {"to", max_values->Bx}});
        response["rangeBy"] = JsonObject({{"from", min_values->By}, {"to", max_values->By}});
        response["rangeWx"] = JsonObject({{"from", min_values->Wx}, {"to", max_values->Wx}});
        response["rangeWy"] = JsonObject({{"from", min_values->Wy}, {"to", max_values->Wy}});

        // Add context list
        response["context"] = parseContextCaps(context_caps);

        // Indicate success
        response["success"] = true;

#if HAL_NOT_READY
        // TODO: Clean up when HAL handles memory
        delete min_values;
        delete max_values;
#endif

        returnResponse(true);
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
        if(m_aspectRatioStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            std::string value = parameters.HasLabel("zoomMode") ? parameters["zoomMode"].String() : "";
            returnIfParamNotFound(parameters, "zoomMode");

            auto it = zoomModeMap.find(value);
            if (it == zoomModeMap.end()) {
                LOGERR("Invalid zoom mode: %s. Not in supported options.", value.c_str());
                returnResponse(false);
            }
            tvDisplayMode_t mode = it->second;
            tvError_t ret = setAspectRatioZoomSettings(mode);
            if (ret != tvERROR_NONE) {
                returnResponse(false);
            }
            else
            {
                // Save DisplayMode to local store and ssm_data
                int retval = updateAVoutputTVParamV2("set", "AspectRatio", parameters, PQ_PARAM_ASPECT_RATIO, mode);
                if (retval != 0) {
                    LOGERR("Failed to Save DisplayMode to ssm_data\n");
                    returnResponse(false);
                }
                tr181ErrorCode_t err = setLocalParam(rfc_caller_id, AVOUTPUT_ASPECTRATIO_RFC_PARAM, value.c_str());
                if (err != tr181Success) {
                    LOGERR("setLocalParam for %s Failed : %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
                    returnResponse(false);
                } else {
                    LOGINFO("setLocalParam for %s Successful, Value: %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, value.c_str());
                }
                LOGINFO("Exit : SetAspectRatio() value : %s\n", value.c_str());
                returnResponse(true);
            }
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

    uint32_t AVOutputTV::resetPrecisionDetail(const JsonObject& parameters, JsonObject& response)
    {
    #if HAL_NOT_READY
        bool success = false;
    #else
        bool success = resetPQParamToDefault(parameters, "PrecisionDetail",
                                            PQ_PARAM_PRECISION_DETAIL, SetPrecisionDetail);
    #endif
        returnResponse(success);
    }

    uint32_t AVOutputTV::resetLocalContrastEnhancement(const JsonObject& parameters, JsonObject& response)
    {
    #if HAL_NOT_READY
        bool success = false;
    #else
        #if ENABLE_PQ_PARAM
        bool success = resetPQParamToDefault(parameters, "LocalContrastEnhancement",
                                            PQ_PARAM_LOCAL_CONTRAST_ENHANCEMENT, SetLocalContrastEnhancement);
        #else
        bool success = true;
        #endif
    #endif
        returnResponse(success);
    }

    uint32_t AVOutputTV::resetMPEGNoiseReduction(const JsonObject& parameters, JsonObject& response)
    {
    #if HAL_NOT_READY
        bool success = false;
    #else
        #if ENABLE_PQ_PARAM
        bool success = resetPQParamToDefault(parameters, "MPEGNoiseReduction",
                                            PQ_PARAM_MPEG_NOISE_REDUCTION, SetMPEGNoiseReduction);
        #else
        bool success = true;
        #endif
    #endif
        returnResponse(success);
    }

    uint32_t AVOutputTV::resetDigitalNoiseReduction(const JsonObject& parameters, JsonObject& response)
    {
    #if HAL_NOT_READY
        bool success = false;
    #else
        #if ENABLE_PQ_PARAM
        bool success = resetPQParamToDefault(parameters, "DigitalNoiseReduction",
                                            PQ_PARAM_DIGITAL_NOISE_REDUCTION, SetDigitalNoiseReduction);
        #else
        bool success = true;
        #endif

    #endif
        returnResponse(success);
    }

    uint32_t AVOutputTV::resetMEMC(const JsonObject& parameters, JsonObject& response)
    {
    #if HAL_NOT_READY
        bool success = false;
    #else
        bool success = resetPQParamToDefault(parameters, "MEMC",
                                            PQ_PARAM_MEMC, SetMEMC);
    #endif
        returnResponse(success);
    }

    uint32_t AVOutputTV::resetAISuperResolution(const JsonObject& parameters, JsonObject& response)
    {
#if HAL_NOT_READY
        bool success= false;
#else
        bool success= resetPQParamToDefault(parameters,"AISuperResolution",
                                        PQ_PARAM_AI_SUPER_RESOLUTION, SetAISuperResolution);
#endif
        returnResponse(success);
    }

    uint32_t AVOutputTV::getPrecisionDetail(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        int precisionDetail = 0;
        bool success = getPQParamFromContext(parameters,
            "PrecisionDetail",
            PQ_PARAM_PRECISION_DETAIL,
            precisionDetail);
        if (success) {
            response["precisionDetail"] = precisionDetail;
        }
        returnResponse(success);
    }

    uint32_t AVOutputTV::getLocalContrastEnhancement(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        int localContraseEnhancement = 0;
        bool success = getPQParamFromContext(parameters,
            "LocalContrastEnhancement",
            PQ_PARAM_LOCAL_CONTRAST_ENHANCEMENT,
            localContraseEnhancement);
        if (success) {
            response["localContrastEnhancement"] = localContraseEnhancement;
        }
        returnResponse(success);
    }

    uint32_t AVOutputTV::getMPEGNoiseReduction(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        int MPEGNoiseReduction = 0;
        bool success = getPQParamFromContext(parameters,
            "MPEGNoiseReduction",
            PQ_PARAM_MPEG_NOISE_REDUCTION,
            MPEGNoiseReduction);
        if (success) {
            response["mpegNoiseReduction"] = MPEGNoiseReduction;
        }
        returnResponse(success);
    }

    uint32_t AVOutputTV::getDigitalNoiseReduction(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        int digitalNoiseReduction = 0;
        bool success = getPQParamFromContext(parameters,
            "DigitalNoiseReduction",
            PQ_PARAM_DIGITAL_NOISE_REDUCTION,
            digitalNoiseReduction);
        if (success) {
            response["digitalNoiseReduction"] = digitalNoiseReduction;
        }
        returnResponse(success);
    }

    uint32_t AVOutputTV::getMEMC(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        int MEMC = 0;
        bool success = getPQParamFromContext(parameters,
            "MEMC",
            PQ_PARAM_MEMC,
            MEMC);
        if (success) {
            response["memc"] = MEMC;
        }
        returnResponse(success);
    }

    uint32_t AVOutputTV::getAISuperResolution(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        int aiSuperResolution = 0;
        bool success = getPQParamFromContext(parameters,
            "AISuperResolution",
            PQ_PARAM_AI_SUPER_RESOLUTION,
            aiSuperResolution);
        if (success) {
            response["aiSuperResolution"] = aiSuperResolution;
        }
        returnResponse(success);
    }

    uint32_t AVOutputTV::setContextPQParam(const JsonObject& parameters, JsonObject& response,
                                       const std::string& inputParamName,
                                       const std::string& tr181ParamName,
                                       int maxAllowedValue,
                                       tvPQParameterIndex_t pqParamType,
                                       std::function<tvError_t(tvVideoSrcType_t, tvPQModeIndex_t, tvVideoFormatType_t, int)> halSetter)
    {
        LOGINFO("Entry");

        if (!parameters.HasLabel(inputParamName.c_str())) {
            LOGERR("Missing parameter: %s", inputParamName.c_str());
            returnResponse(false);
        }

        std::string valueStr = parameters[inputParamName.c_str()].String();
        int value = std::stoi(valueStr);

        if (value < 0 || value > maxAllowedValue) {
            LOGERR("Input value %d is out of range for %s", value, inputParamName.c_str());
            returnResponse(false);
        }

        // Get current context
        tvVideoSrcType_t currentSrc = VIDEO_SOURCE_IP;
        tvVideoFormatType_t currentFmt = VIDEO_FORMAT_SDR;
        tvPQModeIndex_t currentPQMode = PQ_MODE_STANDARD;

        GetCurrentVideoSource(&currentSrc);
        GetCurrentVideoFormat(&currentFmt);
        if (currentFmt == VIDEO_FORMAT_NONE)
            currentFmt = VIDEO_FORMAT_SDR;

        char picMode[PIC_MODE_NAME_MAX] = {0};
        if (getCurrentPictureMode(picMode))
        {
            auto it = pqModeReverseMap.find(picMode);
            if (it != pqModeReverseMap.end())
            {
                currentPQMode = static_cast<tvPQModeIndex_t>(it->second);
            }
            else
            {
                LOGERR("Unknown picture mode");
            }
        }
        else
        {
            LOGERR("Failed to get current picture mode");
        }

        LOGINFO("currentPQMode: %d, currentFmt: %d, currentSrc: %d", currentPQMode, currentFmt, currentSrc);

        if (isSetRequiredForParam(parameters, tr181ParamName)) {
    #if HAL_NOT_READY
    #else
            tvError_t ret = halSetter(currentSrc, currentPQMode, currentFmt, value);
            if (ret != tvERROR_NONE) {
                LOGERR("HAL setter failed for %s", inputParamName.c_str());
                returnResponse(false);
            }
    #endif
        }

        // Persist
        int retval = updateAVoutputTVParamV2("set", tr181ParamName, parameters, pqParamType, value);
        if (retval != 0) {
            LOGERR("Failed to save %s to ssm_data", inputParamName.c_str());
            returnResponse(false);
        }

        LOGINFO("Exit: %s set successfully to %d", inputParamName.c_str(), value);
        returnResponse(true);
    }

    uint32_t AVOutputTV::setAISuperResolution(const JsonObject& parameters, JsonObject& response)
    {
        return setContextPQParam(
            parameters, response,
            "aiSuperResolution",
            "AISuperResolution",
            m_maxAISuperResolution,
            PQ_PARAM_AI_SUPER_RESOLUTION,
            [](tvVideoSrcType_t src, tvPQModeIndex_t mode, tvVideoFormatType_t fmt, int val) {
                return SetAISuperResolution(src, mode, fmt, val);
            }
        );
    }

    uint32_t AVOutputTV::setMEMC(const JsonObject& parameters, JsonObject& response)
    {
        return setContextPQParam(
            parameters, response,
            "memc", "MEMC",
            m_maxMEMC,
            PQ_PARAM_MEMC,
            [](tvVideoSrcType_t src, tvPQModeIndex_t mode, tvVideoFormatType_t fmt, int val) {
                return SetMEMC(src, mode, fmt, val);
            }
        );
    }

    uint32_t AVOutputTV::setPrecisionDetail(const JsonObject& parameters, JsonObject& response)
    {
        return setContextPQParam(
            parameters, response,
            "precisionDetail", "PrecisionDetail",
            m_maxPrecisionDetail,
            PQ_PARAM_PRECISION_DETAIL,
            [](tvVideoSrcType_t src, tvPQModeIndex_t mode, tvVideoFormatType_t fmt, int val) {
                return SetPrecisionDetail(src, mode, fmt, val);
            }
        );
    }

    uint32_t AVOutputTV::setLocalContrastEnhancement(const JsonObject& parameters, JsonObject& response)
    {
#if ENABLE_PQ_PARAM
        return setContextPQParam(
            parameters, response,
            "localContrastEnhancement", "LocalContrastEnhancement",
            m_maxLocalContrastEnhancement,
            PQ_PARAM_LOCAL_CONTRAST_ENHANCEMENT,
            [](tvVideoSrcType_t src, tvPQModeIndex_t mode, tvVideoFormatType_t fmt, int val) {
                return SetLocalContrastEnhancement(src, mode, fmt, val);
            }
        );
#else
    returnResponse(true);
#endif
    }

    uint32_t AVOutputTV::setMPEGNoiseReduction(const JsonObject& parameters, JsonObject& response)
    {
#if ENABLE_PQ_PARAM
        return setContextPQParam(
            parameters, response,
            "mpegNoiseReduction", "MPEGNoiseReduction",
            m_maxMPEGNoiseReduction,
            PQ_PARAM_MPEG_NOISE_REDUCTION,
            [](tvVideoSrcType_t src, tvPQModeIndex_t mode, tvVideoFormatType_t fmt, int val) {
                return SetMPEGNoiseReduction(src, mode, fmt, val);
            }
        );
#else
    returnResponse(true);
#endif
    }

    uint32_t AVOutputTV::setDigitalNoiseReduction(const JsonObject& parameters, JsonObject& response)
    {
#if ENABLE_PQ_PARAM
        return setContextPQParam(
            parameters, response,
            "digitalNoiseReduction", "DigitalNoiseReduction",
            m_maxDigitalNoiseReduction,
            PQ_PARAM_DIGITAL_NOISE_REDUCTION,
            [](tvVideoSrcType_t src, tvPQModeIndex_t mode, tvVideoFormatType_t fmt, int val) {
                return SetDigitalNoiseReduction(src, mode, fmt, val);
            }
        );
#else
    returnResponse(true);
#endif
    }

    uint32_t AVOutputTV::getBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        if(m_backlightStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            int backlight = 0;
            bool success = getPQParamFromContext(parameters,
                "Backlight",
                PQ_PARAM_BACKLIGHT,
                backlight);
            if (success) {
                response["backlight"] = backlight;
            }
            returnResponse(success);

        }
    }

    uint32_t AVOutputTV::setBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if(m_backlightStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            bool success = setIntPQParam(parameters, "Backlight", PQ_PARAM_BACKLIGHT, SetBacklight, m_maxBacklight);
            returnResponse(success);
        }

    }
    bool AVOutputTV::resetEnumPQParamToDefault(
        const JsonObject& parameters,
        const std::string& paramName,
        tvPQParameterIndex_t pqIndex,
        const std::unordered_map<int, std::string>& valueMap,
        std::function<tvError_t(int, const std::unordered_map<int, std::string>&)> halSetter)
    {
        LOGINFO("Entry: %s\n", paramName.c_str());

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int intVal = 0;
        tvError_t ret = tvERROR_NONE;

        // Step 1: Save reset state using V2 persistence
        LOGINFO("Updating AVOutputTVParamV2 for: %s\n", paramName.c_str());
        int retval = updateAVoutputTVParamV2("reset", paramName, parameters, pqIndex, intVal);
        if (retval != 0) {
            LOGERR("Failed to reset %s via updateAVoutputTVParamV2. retval: %d\n", paramName.c_str(), retval);
            return false;
        }

        // Step 2: Apply value from persisted config to HAL if needed
        if (isSetRequiredForParam(parameters, paramName)) {
            inputInfo.pqmode = "Current";
            inputInfo.source = "Current";
            inputInfo.format = "Current";

            if (getParamIndex(paramName, inputInfo, indexInfo) == 0 &&
                getLocalparam(paramName, indexInfo, intVal, pqIndex) == 0)
            {
                LOGINFO("%s: getLocalparam success for %s [format=%d, source=%d, mode=%d] → value=%d\n",
                    __FUNCTION__, paramName.c_str(), indexInfo.formatIndex,
                    indexInfo.sourceIndex, indexInfo.pqmodeIndex, intVal);

                if (valueMap.find(intVal) == valueMap.end()) {
                    LOGERR("%s: Invalid enum value %d for %s\n", __FUNCTION__, intVal, paramName.c_str());
                    return false;
                }

                ret = halSetter(intVal, valueMap);
                if (ret != tvERROR_NONE) {
                    LOGERR("%s: HAL setter failed for value %d\n", paramName.c_str(), intVal);
                    return false;
                }
            }
            else {
                LOGERR("%s: Failed to get local param for %s\n", __FUNCTION__, paramName.c_str());
                return false;
            }
        }

        LOGINFO("Exit: resetEnumPQParamToDefault for %s successful (value: %d)\n", paramName.c_str(), intVal);
        return true;
    }

    bool AVOutputTV::resetPQParamToDefault(const JsonObject& parameters,
        const std::string& paramName,
        tvPQParameterIndex_t pqIndex,
        tvSetFunctionV2 halSetter)
    {
        LOGINFO("Entry: %s\n", paramName.c_str());

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int level = 0;
        tvError_t ret = tvERROR_NONE;

        // Save reset state using V2 path
        LOGINFO("Updating AVOutputTVParamV2 for: %s\n", paramName.c_str());
        int retval = updateAVoutputTVParamV2("reset", paramName, parameters, pqIndex, level);
        if (retval != 0)
        {
            LOGERR("Failed to update %s via updateAVoutputTVParamV2. retval: %d\n", paramName.c_str(), retval);
            return false;
        }

        // If update succeeded, apply value from local config to HAL
        if (isSetRequiredForParam(parameters, paramName))
        {
            inputInfo.pqmode = "Current";
            inputInfo.source = "Current";
            inputInfo.format = "Current";

            if (getParamIndex(paramName, inputInfo, indexInfo) == 0 &&
                getLocalparam(paramName, indexInfo, level, pqIndex) == 0)
            {
                LOGINFO("%s: getLocalparam success for %s: format=%d, source=%d, mode=%d, value=%d\n",
                        __FUNCTION__, paramName.c_str(), indexInfo.formatIndex,
                        indexInfo.sourceIndex, indexInfo.pqmodeIndex, level);
                if (halSetter) {
                    ret = halSetter(
                        static_cast<tvVideoSrcType_t>(indexInfo.sourceIndex),
                        static_cast<tvPQModeIndex_t>(indexInfo.pqmodeIndex),
                        static_cast<tvVideoFormatType_t>(indexInfo.formatIndex),
                        level);
                    LOGINFO("%s halSetter return value: %d\n", paramName.c_str(), ret);
                } else {
                    LOGERR("halSetter is null for %s\n", paramName.c_str());
                    return false;
                }
            }
            else
            {
                LOGERR("%s: Failed to get local param for %s\n", __FUNCTION__, paramName.c_str());
                return false;
            }
        }

        LOGINFO("Exit: reset%s successful to value: %d\n", paramName.c_str(), level);
        return true;
    }

    bool AVOutputTV::resetPQParamToDefault(const JsonObject& parameters,
        const std::string& paramName,
        tvPQParameterIndex_t pqIndex,
        tvSetFunction halSetter)
    {
        LOGINFO("Entry: %s\n", paramName.c_str());

        capDetails_t inputInfo;
        paramIndex_t indexInfo;
        int level = 0;
        tvError_t ret = tvERROR_NONE;

        // Save reset state using V2 path
        LOGINFO("Updating AVOutputTVParamV2 for: %s\n", paramName.c_str());
        int retval = updateAVoutputTVParamV2("reset", paramName, parameters, pqIndex, level);
        if (retval != 0)
        {
            LOGERR("Failed to update %s via updateAVoutputTVParamV2. retval: %d\n", paramName.c_str(), retval);
            return false;
        }

        // If update succeeded, apply value from local config to HAL
        if (isSetRequiredForParam(parameters, paramName))
        {
            inputInfo.pqmode = "Current";
            inputInfo.source = "Current";
            inputInfo.format = "Current";

            if (getParamIndex(paramName, inputInfo, indexInfo) == 0 &&
                getLocalparam(paramName, indexInfo, level, pqIndex) == 0)
            {
                LOGINFO("%s: getLocalparam success for %s: format=%d, source=%d, mode=%d, value=%d\n",
                        __FUNCTION__, paramName.c_str(), indexInfo.formatIndex,
                        indexInfo.sourceIndex, indexInfo.pqmodeIndex, level);
                ret = halSetter(level);
                LOGINFO("%s halSetter return value: %d\n", paramName.c_str(), ret);
            }
            else
            {
                LOGERR("%s: Failed to get local param for %s\n", __FUNCTION__, paramName.c_str());
                return false;
            }
        }

        LOGINFO("Exit: reset%s successful to value: %d\n", paramName.c_str(), level);
        return true;
    }

    uint32_t AVOutputTV::resetBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if(m_backlightStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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

            int retval= updateAVoutputTVParam("reset","Backlight",inputInfo,PQ_PARAM_BACKLIGHT,backlight);
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
        else
        {
            bool success= resetPQParamToDefault(parameters, "Backlight", PQ_PARAM_BACKLIGHT, SetBacklight);
            returnResponse(success);
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
        if(m_brightnessStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            int brightness = 0;
            bool success = getPQParamFromContext(parameters,
                "Brightness",
                PQ_PARAM_BRIGHTNESS,
                brightness);
            if (success) {
                response["brightness"] = brightness;
            }
            returnResponse(success);
        }
    }

    uint32_t AVOutputTV::setBrightness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if(m_brightnessStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            bool success = setIntPQParam(parameters, "Brightness", PQ_PARAM_BRIGHTNESS, SetBrightness, m_maxBrightness);
            returnResponse(success);
        }
    }


    uint32_t AVOutputTV::resetBrightness(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        if(m_brightnessStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            bool success = resetPQParamToDefault(parameters, "Brightness", PQ_PARAM_BRIGHTNESS, SetBrightness);
            returnResponse(success);
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
        if(m_contrastStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            int contrast = 0;
            bool success = getPQParamFromContext(parameters,
                "Contrast",
                PQ_PARAM_CONTRAST,
                contrast);
            if (success) {
                response["contrast"] = contrast;
            }
            returnResponse(success);

        }
    }

    uint32_t AVOutputTV::setContrast(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if(m_contrastStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            bool success = setIntPQParam(parameters, "Contrast", PQ_PARAM_CONTRAST, SetContrast, m_maxContrast);
            returnResponse(success);
        }
    }

    uint32_t AVOutputTV::resetContrast(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        if(m_contrastStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            bool success= resetPQParamToDefault(parameters, "Contrast", PQ_PARAM_CONTRAST, SetContrast);
            returnResponse(success);

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
        if(m_saturationStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            int saturation = 0;
            bool success = getPQParamFromContext(parameters,
                "Saturation",
                PQ_PARAM_SATURATION,
                saturation);
            if (success) {
                response["saturation"] = saturation;
            }
            returnResponse(success);

        }
    }

    uint32_t AVOutputTV::setSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if(m_saturationStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            bool success = setIntPQParam(parameters, "Saturation", PQ_PARAM_SATURATION, SetSaturation, m_maxSaturation);
            returnResponse(success);
        }
    }

    uint32_t AVOutputTV::resetSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if(m_saturationStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            bool success= resetPQParamToDefault(parameters, "Saturation", PQ_PARAM_SATURATION, SetSaturation);
            returnResponse(success);

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
        if(m_sharpnessStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            int sharpness = 0;
            bool success = getPQParamFromContext(parameters,
                "Sharpness",
                PQ_PARAM_SHARPNESS,
                sharpness);
            if (success) {
                response["sharpness"] = sharpness;
            }
            returnResponse(success);

        }
    }

    uint32_t AVOutputTV::setSharpness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if(m_sharpnessStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            bool success = setIntPQParam(parameters, "Sharpness", PQ_PARAM_SHARPNESS, SetSharpness, m_maxSharpness);
            returnResponse(success);
        }
    }

    uint32_t AVOutputTV::resetSharpness(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        if(m_sharpnessStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            bool success= resetPQParamToDefault(parameters, "Sharpness", PQ_PARAM_SHARPNESS, SetSharpness);
            returnResponse(success);

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
        if(m_hueStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            int hue = 0;
            bool success = getPQParamFromContext(parameters,
                "Hue",
                PQ_PARAM_HUE,
                hue);
            if (success) {
                response["hue"] = hue;
            }
            returnResponse(success);

        }
    }

    uint32_t AVOutputTV::setHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if(m_hueStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            bool success = setIntPQParam(parameters, "Hue", PQ_PARAM_HUE, SetHue, m_maxHue);
            returnResponse(success);
        }
    }

    uint32_t AVOutputTV::resetHue(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        if(m_hueStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            bool success= resetPQParamToDefault(parameters, "Hue", PQ_PARAM_HUE, SetHue);
            returnResponse(success);

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
        if(m_colorTempStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
                        response["colorTemperature"] = "UserDefined";
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
        else
        {
            std::string outMode;
            if (getEnumPQParamString(parameters, "ColorTemp",
                 PQ_PARAM_COLOR_TEMPERATURE, colorTempReverseMap, outMode)) {
                response["colorTemperature"] = outMode;
                returnResponse(true);
            } else {
                returnResponse(false);
            }

        }
    }

    uint32_t AVOutputTV::setColorTemperature(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if(m_colorTempStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            bool success = setEnumPQParam(
                parameters,
                "colorTemperature",
                "ColorTemp",
                colorTempMap,
                PQ_PARAM_COLOR_TEMPERATURE,
                [](int val) {
                    return SetColorTemperature(static_cast<tvColorTemp_t>(val));
                });

            if (!success) {
                LOGERR("setColorTemperature failed");
                returnResponse(false);
            }

            LOGINFO("setColorTemperature: Success");
            returnResponse(true);
        }
    }

    uint32_t AVOutputTV::resetColorTemperature(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        if(m_colorTempStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            bool success = resetEnumPQParamToDefault(
            parameters,
            "ColorTemp",
            PQ_PARAM_COLOR_TEMPERATURE,
            colorTempReverseMap,
            [](int val, const std::unordered_map<int, std::string>&) {
                return SetColorTemperature(static_cast<tvColorTemp_t>(val));
            });

            returnResponse(success);
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
        if(m_dimmingModeStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
                        response["dimmingMode"] = "Fixed";
                        break;

                    case tvDimmingMode_Local:
                        LOGINFO("DimmingMode Value: Local\n");
                        response["dimmingMode"] = "Local";
                        break;

                    case tvDimmingMode_Global:
                        LOGINFO("DimmingMode Value: Global\n");
                        response["dimmingMode"] = "Global";
                        break;

                }
                LOGINFO("Exit : DimmingMode Value: %d \n", dimmingMode);
                returnResponse(true);
            }
            else {
                returnResponse(false);
            }
        }
        else
        {
            std::string mode;
            if (getEnumPQParamString(parameters, "DimmingMode",
                 PQ_PARAM_DIMMINGMODE, dimmingModeReverseMap, mode)) {
                response["dimmingMode"] = mode;
                returnResponse(true);
            } else {
                returnResponse(false);
            }
        }
    }

    uint32_t AVOutputTV::setBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if(m_dimmingModeStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {

            capDetails_t inputInfo;
            int dimmingMode = 0;
            tvError_t ret = tvERROR_NONE;
            std::string value;

            value = parameters.HasLabel("dimmingMode") ? parameters["dimmingMode"].String() : "";
            returnIfParamNotFound(parameters,"dimmingMode");

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
        else
        {
            int dimmingMode = 0;
            tvError_t ret = tvERROR_NONE;
            std::string value;

            value = parameters.HasLabel("dimmingMode") ? parameters["dimmingMode"].String() : "";
            returnIfParamNotFound(parameters,"dimmingMode");

            dimmingMode = getDimmingModeIndex(value);
            if (dimmingMode < 0 || dimmingMode > tvDimmingMode_MAX) {
                LOGERR("Input value %d is out of range (0 - %d) for DimmingMode", dimmingMode, tvDimmingMode_MAX);
                returnResponse(false);
            }
            if( isSetRequiredForParam(parameters, "DimmingMode" ) ) {
                LOGINFO("Proceed with %s\n",__FUNCTION__);
                ret = SetTVDimmingMode(value.c_str());
            }
            if(ret != tvERROR_NONE) {
                LOGERR("Failed to set DimmingMode\n");
                returnResponse(false);
            }
            else
            {
                // Update the TV parameter
                int retval = updateAVoutputTVParamV2("set", "DimmingMode", parameters, PQ_PARAM_DIMMINGMODE, (int)dimmingMode);
                if (retval != 0) {
                    LOGERR("Failed to Save DimmingMode to ssm_data. retval: %d \n", retval);
                    returnResponse(false);
                }
                LOGINFO("Exit : setDimmingMode successful to value: %d \n", dimmingMode);
                returnResponse(true);
            }
        }
    }

    uint32_t AVOutputTV::resetBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if(m_dimmingModeStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {

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
        else
        {
            bool success = resetEnumPQParamToDefault(
                            parameters,
                            "DimmingMode",
                            PQ_PARAM_DIMMINGMODE,
                            dimmingModeReverseMap,
                            [](int val, const std::unordered_map<int, std::string>& enumMap) -> tvError_t {
                                auto it = enumMap.find(val);
                                if (it != enumMap.end()) {
                                    return SetTVDimmingMode(it->second.c_str());
                                } else {
                                    LOGERR("Invalid enum value: %d for DimmingMode\n", val);
                                    return tvERROR_GENERAL;
                                }
                            });

            returnResponse(success);

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
        tvDolbyMode_t dvModes[tvMode_Max];
        tvDolbyMode_t *dvModesPtr[tvMode_Max]={0};
        unsigned short totalAvailable = 0;
        for (int i = 0; i < tvMode_Max; i++)
        {
            dvModesPtr[i] = &dvModes[i];
        }
        // Set an initial value to indicate the mode type
        dvModes[0] = tvDolbyMode_Dark;

        tvError_t ret = GetTVSupportedDolbyVisionModes(dvModesPtr, &totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false);
        }
        else {
            JsonArray SupportedDVModes;

            for(int count = 0;count <totalAvailable;count++ ) {
                SupportedDVModes.Add(getDolbyModeStringFromEnum(dvModes[count]));
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
            ret = SetTVDolbyVisionMode(GetDolbyVisionEnumFromModeString(value.c_str()));
        }

        if(ret != tvERROR_NONE) {
            LOGERR("Failed to set DolbyVisionMode\n\n");
            returnResponse(false);
        }
        else {
            inputInfo.format = "DV"; //Update only for DV format
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
        inputInfo.format = "DV"; //Update only for DV format
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

                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d dolbyvalue : %d\n",__FUNCTION__,indexInfo.formatIndex, indexInfo.sourceIndex, indexInfo.pqmodeIndex, dolbyMode);
                    ret = SetTVDolbyVisionMode((tvDolbyMode_t)dolbyMode);
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
        pic_modes_t pictureModes[PIC_MODES_SUPPORTED_MAX];
        pic_modes_t *pictureModesPtr[PIC_MODES_SUPPORTED_MAX]={0};
        unsigned short totalAvailable = 0;
        for (int i = 0; i < PIC_MODES_SUPPORTED_MAX; i++)
        {
            pictureModesPtr[i] = &pictureModes[i];
        }
        tvError_t ret = GetTVSupportedPictureModes(pictureModesPtr,&totalAvailable);
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

    bool AVOutputTV::getPictureModeV2(const JsonObject& parameters, std::string& outMode)
    {
        LOGINFO("Entry");

        tvVideoSrcType_t source = VIDEO_SOURCE_IP;
        tvVideoFormatType_t format = VIDEO_FORMAT_SDR;

        // Parse videoSource
        if (!parameters.HasLabel("videoSource") || parameters["videoSource"].String() == "Current") {
            GetCurrentVideoSource(&source);
        } else {
            std::string srcStr = parameters["videoSource"].String();
            if (videoSrcReverseMap.count(srcStr)) {
                source = static_cast<tvVideoSrcType_t>(videoSrcReverseMap.at(srcStr));
            } else {
                LOGERR("Invalid videoSource: %s", srcStr.c_str());
                return false;
            }
        }

        // Parse videoFormat
        if (!parameters.HasLabel("videoFormat") || parameters["videoFormat"].String() == "Current") {
            GetCurrentVideoFormat(&format);
            if (format == VIDEO_FORMAT_NONE) format = VIDEO_FORMAT_SDR;
        } else {
            std::string fmtStr = parameters["videoFormat"].String();
            if (videoFormatReverseMap.count(fmtStr)) {
                format = static_cast<tvVideoFormatType_t>(videoFormatReverseMap.at(fmtStr));
            } else {
                LOGERR("Invalid videoFormat: %s", fmtStr.c_str());
                return false;
            }
        }

        // Directly use TR-181 to fetch active picture mode
        std::string tr181_param_name = std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM) +
            "." + convertSourceIndexToStringV2(source) +
            ".Format." + convertVideoFormatToStringV2(format) +
            ".PictureModeString";

        LOGINFO("TR181 Param Name = %s", tr181_param_name.c_str());

        TR181_ParamData_t param = {0};
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
        if (err != tr181Success) {
            LOGERR("getLocalParam failed: %d", err);
            return false;
        }

        outMode = param.value;
        LOGINFO("Exit: PictureMode = %s", outMode.c_str());
        return true;
    }

    uint32_t AVOutputTV::getPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string pictureModeStr;
        if (m_pictureModeStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
            capDetails_t inputInfo;
            paramIndex_t indexInfo;
            TR181_ParamData_t param = {0};

            if (parsingGetInputArgument(parameters, "PictureMode", inputInfo) != 0) {
                LOGERR("%s: Failed to parse input argument", __FUNCTION__);
                returnResponse(false);
            }

            if (getParamIndex("PictureMode", inputInfo, indexInfo) == -1) {
                LOGERR("%s: getParamIndex failed", __FUNCTION__);
                returnResponse(false);
            }

            std::string tr181_param_name = std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM) +
                "." + convertSourceIndexToString(indexInfo.sourceIndex) +
                ".Format." + convertVideoFormatToString(indexInfo.formatIndex) +
                ".PictureModeString";

            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
            if (err != tr181Success) {
                returnResponse(false);
            }

            pictureModeStr = param.value;
        }
        else
        {
            if (!getPictureModeV2(parameters, pictureModeStr)) {
                returnResponse(false);
            }
        }
        response["pictureMode"] = pictureModeStr;
        LOGINFO("Exit: getPictureMode() : %s", pictureModeStr.c_str());
        returnResponse(true);
    }

    bool AVOutputTV::setPictureModeV2(const JsonObject& parameters)
    {
        LOGINFO("Entry %s", __FUNCTION__);

        if (!parameters.HasLabel("pictureMode")) {
            LOGERR("Missing 'pictureMode' in parameters.");
            return false;
        }

        std::string mode = parameters["pictureMode"].String();

        // Validate against m_pictureModes
        int modeIndex = -1;
        for (size_t i = 0; i < m_numPictureModes; ++i) {
            auto it = pqModeMap.find(m_pictureModes[i]);
            if (it != pqModeMap.end()) {
                if (it->second == mode) {
                    modeIndex = static_cast<int>(i);
                    LOGINFO("Matched pictureMode '%s' at index %d", mode.c_str(), modeIndex);
                    break;
                }
            } else {
                LOGERR("pqModeMap does not contain m_pictureModes[%zu] = %d", i, m_pictureModes[i]);
            }
        }

        if (modeIndex == -1) {
            LOGERR("Invalid pictureMode: %s", mode.c_str());
            return false;
        }

        // Extract videoSource
        std::vector<std::string> sources;
        if (parameters.HasLabel("videoSource")) {
            const JsonArray& sourceParam = parameters["videoSource"].Array();
            for (uint32_t i = 0; i < sourceParam.Length(); ++i) {
                std::string source = sourceParam[i].Value();
                if (!source.empty()) {
                    sources.push_back(source);
                }
            }
        } else {
            sources.push_back("Global");
            LOGINFO("videoSource not provided, defaulting to 'Global'");
        }

        // Extract videoFormat
        std::vector<std::string> formats;
        if (parameters.HasLabel("videoFormat")) {
            const JsonArray& formatParam = parameters["videoFormat"].Array();
            for (uint32_t i = 0; i < formatParam.Length(); ++i) {
                std::string format = formatParam[i].Value();
                if (!format.empty()) {
                    formats.push_back(format);
                }
            }
        } else {
            formats.push_back("Global");
            LOGINFO("videoFormat not provided, defaulting to 'Global'");
        }

        // Expand 'Global' sources
        if (std::find(sources.begin(), sources.end(), "Global") != sources.end()) {
            std::unordered_set<std::string> sourceSet;
            for (size_t j = 0; j < m_pictureModeCaps->num_contexts; ++j) {
                if (m_pictureModeCaps->contexts[j].pq_mode == m_pictureModes[modeIndex]) {
                    std::string srcStr = convertSourceIndexToStringV2(m_pictureModeCaps->contexts[j].videoSrcType);
                    sourceSet.insert(srcStr);
                }
            }
            sources.insert(sources.end(), sourceSet.begin(), sourceSet.end());
        }

        // Expand 'Global' formats
        if (std::find(formats.begin(), formats.end(), "Global") != formats.end()) {
            std::unordered_set<std::string> formatSet;
            for (size_t j = 0; j < m_pictureModeCaps->num_contexts; ++j) {
                if (m_pictureModeCaps->contexts[j].pq_mode == m_pictureModes[modeIndex]) {
                    std::string fmtStr = convertVideoFormatToStringV2(m_pictureModeCaps->contexts[j].videoFormatType);
                    formatSet.insert(fmtStr);
                }
            }
            formats.insert(formats.end(), formatSet.begin(), formatSet.end());
        }

        // Get current context
        tvVideoSrcType_t currentSrc = VIDEO_SOURCE_IP;
        tvVideoFormatType_t currentFmt = VIDEO_FORMAT_SDR;
        GetCurrentVideoSource(&currentSrc);
        GetCurrentVideoFormat(&currentFmt);
        if (currentFmt == VIDEO_FORMAT_NONE)
            currentFmt = VIDEO_FORMAT_SDR;

        LOGINFO("Current video source: %s, format: %s",
            convertSourceIndexToStringV2(currentSrc).c_str(),
            convertVideoFormatToStringV2(currentFmt).c_str());

        bool contextHandled = false;

        // Iterate through contexts and apply mode
        for (size_t i = 0; i < m_pictureModeCaps->num_contexts; ++i) {
            const tvConfigContext_t& ctx = m_pictureModeCaps->contexts[i];

            if (ctx.pq_mode != m_pictureModes[modeIndex])
                continue;

            if (!isValidFormat(formats, ctx.videoFormatType))
                continue;

            if (!isValidSource(sources, ctx.videoSrcType))
                continue;

            std::string srcStr = convertSourceIndexToStringV2(ctx.videoSrcType);
            std::string fmtStr = convertVideoFormatToStringV2(ctx.videoFormatType);

            if (ctx.videoSrcType == currentSrc && ctx.videoFormatType == currentFmt) {
                if (SetTVPictureMode(mode.c_str()) != tvERROR_NONE) {
                    LOGERR("SetTVPictureMode failed for mode: %s", mode.c_str());
                    continue;
                }
            }
//TODO:: Revisit this logic. Have to revert if HAL call fails.
            std::string tr181Param = std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM) + "." +
                srcStr + ".Format." + fmtStr + ".PictureModeString";

            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, tr181Param.c_str(), mode.c_str());
            if (err != tr181Success) {
                LOGERR("setLocalParam failed: %s => %s", tr181Param.c_str(), getTR181ErrorString(err));
                continue;
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM, mode.c_str());
                int pqmodeindex = (int)getPictureModeIndex(mode);
                SaveSourcePictureMode(ctx.videoSrcType, ctx.videoFormatType, pqmodeindex);
            }

            contextHandled = true;
        }

        if (!contextHandled) {
            LOGERR("No valid context found to apply pictureMode: %s", mode.c_str());
            return false;
        }

        LOGINFO("Exit %s: PictureMode '%s' applied successfully.", __FUNCTION__, mode.c_str());
        return true;
    }


    uint32_t AVOutputTV::setPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if (m_pictureModeStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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

            if( isSetRequired("Current",inputInfo.source,inputInfo.format) ) {
                LOGINFO("Proceed with SetTVPictureMode\n");
                ret = SetTVPictureMode(value.c_str());
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
        else {
            bool success = false;
            try {
                success = setPictureModeV2(parameters);
            } catch (const std::exception& e) {
                LOGERR("Exception in setPictureModeV2: %s", e.what());
            } catch (...) {
                LOGERR("Unknown exception in setPictureModeV2");
            }
            returnResponse(success);
        }
    }
    bool AVOutputTV::resetPictureModeV2(const JsonObject& parameters)
    {
        LOGINFO("Entry %s\n", __FUNCTION__);

        auto extractList = [](const JsonObject& params, const std::string& key) -> std::vector<std::string> {
            std::vector<std::string> result;
            if (params.HasLabel(key.c_str())) {
                const JsonArray& array = params[key.c_str()].Array();
                for (uint32_t i = 0; i < array.Length(); ++i) {
                    result.push_back(array[i].Value());
                }
            } else {
                result.push_back("Global");
            }
            return result;
        };

        std::vector<std::string> sources = extractList(parameters, "videoSource");
        std::vector<std::string> formats = extractList(parameters, "videoFormat");

        auto expandGlobal = [](std::vector<std::string>& vec, const std::unordered_set<std::string>& fullSet) {
            if (std::find(vec.begin(), vec.end(), "Global") != vec.end()) {
                vec.erase(std::remove(vec.begin(), vec.end(), "Global"), vec.end());
                vec.insert(vec.end(), fullSet.begin(), fullSet.end());
            }
            std::unordered_set<std::string> unique(vec.begin(), vec.end());
            vec.assign(unique.begin(), unique.end());
        };

        // Expand "Global" values
        std::unordered_set<std::string> allSources, allFormats;
        for (size_t j = 0; j < m_pictureModeCaps->num_contexts; ++j) {
            allSources.insert(convertSourceIndexToStringV2(m_pictureModeCaps->contexts[j].videoSrcType));
            allFormats.insert(convertVideoFormatToStringV2(m_pictureModeCaps->contexts[j].videoFormatType));
        }
        expandGlobal(sources, allSources);
        expandGlobal(formats, allFormats);

        // Get current source & format
        tvVideoSrcType_t currentSrc = VIDEO_SOURCE_IP;
        tvVideoFormatType_t currentFmt = VIDEO_FORMAT_SDR;
        GetCurrentVideoSource(&currentSrc);
        GetCurrentVideoFormat(&currentFmt);
        if (currentFmt == VIDEO_FORMAT_NONE)
            currentFmt = VIDEO_FORMAT_SDR;

        bool contextHandled = false;

        for (size_t i = 0; i < m_pictureModeCaps->num_contexts; ++i) {
            const tvConfigContext_t& ctx = m_pictureModeCaps->contexts[i];

            if (!isValidSource(sources, ctx.videoSrcType) || !isValidFormat(formats, ctx.videoFormatType))
                continue;

            std::string tr181Param = std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM) + "." +
                convertSourceIndexToStringV2(ctx.videoSrcType) + ".Format." +
                convertVideoFormatToStringV2(ctx.videoFormatType) + ".PictureModeString";

            // Clear override
            tr181ErrorCode_t err = clearLocalParam(rfc_caller_id, tr181Param.c_str());
            if (err != tr181Success) {
                LOGERR("clearLocalParam failed for %s: %s", tr181Param.c_str(), getTR181ErrorString(err));
                continue;
            }

            // Read saved TR-181 value
            TR181_ParamData_t param = {0};
            err = getLocalParam(rfc_caller_id, tr181Param.c_str(), &param);
            if (err != tr181Success || strlen(param.value) == 0) {
                LOGWARN("getLocalParam failed or empty for %s", tr181Param.c_str());
                continue;
            }

            // Apply to hardware if current context matches
            if (ctx.videoSrcType == currentSrc && ctx.videoFormatType == currentFmt) {

                tvError_t ret = SetTVPictureMode(param.value);
                if (ret != tvERROR_NONE) {
                    LOGERR("SetTVPictureMode failed for %s", param.value);
                    continue;
                }
            }

            // Save to internal config
            int pqmodeIndex = static_cast<int>(getPictureModeIndex(param.value));
            SaveSourcePictureMode(ctx.videoSrcType, ctx.videoFormatType, pqmodeIndex);
            contextHandled = true;
        }

        if (!contextHandled) {
            LOGERR("No valid pictureMode context matched to reset.\n");
            return false;
        }

        LOGINFO("resetPictureModeV2: Exit - PictureMode reset successfully.\n");
        return true;
    }

    uint32_t AVOutputTV::resetPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if (m_pictureModeStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
                            GetCurrentVideoSource(&current_source);

                            tvVideoFormatType_t current_format = VIDEO_FORMAT_NONE;
                            GetCurrentVideoFormat(&current_format);
                            if( current_format == VIDEO_FORMAT_NONE) {
                                current_format = VIDEO_FORMAT_SDR;
                            }

                            if (current_source == sourceType && current_format == formatType) {

                                tvError_t ret = SetTVPictureMode(param.value);
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
            returnResponse(true);
        }
        else
        {
            bool success = resetPictureModeV2(parameters);
            returnResponse(success);
        }
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
        if(m_lowLatencyStateStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            std::string value;
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
            if (lowLatencyIndex < 0 || lowLatencyIndex > m_maxlowLatencyState) {
                LOGERR("Input value %d is out of range (0 - %d) for LowLatencyState", lowLatencyIndex, m_maxlowLatencyState);
                returnResponse(false);
            }

            int retval= updateAVoutputTVParamV2("set","LowLatencyState",parameters,PQ_PARAM_LOWLATENCY_STATE,lowLatencyIndex);
            if(retval != 0 ) {
                LOGERR("Failed to SaveLowLatency to ssm_data\n");
                returnResponse(false);
            }
            else
            {
                if(isSetRequiredForParam(parameters, "LowLatencyState"))
                {
                    LOGINFO("Proceed with setLowLatencyState\n");
                    ret = SetLowLatencyState( lowLatencyIndex );
                }
                if(ret != tvERROR_NONE) {
                    LOGERR("Failed to set low latency. Fallback to previous state %d\n", prevLowLatencyIndex);
                    retval=updateAVoutputTVParamV2("set","LowLatencyState",parameters,PQ_PARAM_LOWLATENCY_STATE, prevLowLatencyIndex);
                    if(retval != 0 ){
                        LOGERR("Fallback to previous low latency state %d failed.\n", prevLowLatencyIndex);
                    }
                    returnResponse(false);
                }

                LOGINFO("Exit : setLowLatency successful to value: %d\n", lowLatencyIndex);
                returnResponse(true);
            }
        }
    }

    uint32_t AVOutputTV::getLowLatencyState(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        if(m_lowLatencyStateStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
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
        else
        {
            int lowlatencystate = 0;
            if (getPQParamFromContext(parameters, "LowLatencyState", PQ_PARAM_LOWLATENCY_STATE, lowlatencystate)) {
                response["lowLatencyState"] = std::to_string(lowlatencystate);
                LOGINFO("Exit : LowLatencyState Value: %d", lowlatencystate);
                returnResponse(true);
            } else {
                LOGERR("Failed to get LowLatencyState");
                returnResponse(false);
            }
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

        if (isPlatformSupport("CMS") != 0) {
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

        if (isPlatformSupport("CMS") != 0) {
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

        retVal = getCMSColorEnumFromString(inputInfo.color,colorEnum);
        if( retVal == -1) {
            LOGERR("%s: Invalid Color : %s\n",__FUNCTION__,inputInfo.color.c_str());
            returnResponse(false);
        }

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
        JsonArray sourceArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray colorArray;
        JsonArray componentArray;

        if (isPlatformSupport("CMS") != 0) {
            returnResponse(false);
        }

        pqmodeArray = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].Array() : JsonArray();
        for (int i = 0; i < pqmodeArray.Length(); ++i) {
            inputInfo.pqmode += pqmodeArray[i].String();
            if (i != (pqmodeArray.Length() - 1) ) {
                inputInfo.pqmode += ",";
            }
        }

        sourceArray = parameters.HasLabel("videoSource") ? parameters["videoSource"].Array() : JsonArray();
        for (int i = 0; i < sourceArray.Length(); ++i) {
            inputInfo.source += sourceArray[i].String();
            if (i != (sourceArray.Length() - 1) ) {
                inputInfo.source += ",";
	        }
        }

        formatArray = parameters.HasLabel("videoFormat") ? parameters["videoFormat"].Array() : JsonArray();
        for (int i = 0; i < formatArray.Length(); ++i) {
            inputInfo.format += formatArray[i].String();
            if (i != (formatArray.Length() - 1) ) {
                inputInfo.format += ",";
            }
        }
        colorArray = parameters.HasLabel("color") ? parameters["color"].Array() : JsonArray();
        for (int i = 0; i < colorArray.Length(); ++i) {
            inputInfo.color += colorArray[i].String();
            if (i != (colorArray.Length() - 1) ) {
                inputInfo.color += ",";
            }
        }
        componentArray = parameters.HasLabel("component") ? parameters["component"].Array() : JsonArray();
        for (int i = 0; i < componentArray.Length(); ++i) {
            inputInfo.component += componentArray[i].String();
            if (i != (componentArray.Length() - 1) ) {
                inputInfo.component += ",";
            }
        }
        if (inputInfo.source.empty()) {
            inputInfo.source = "Global";
	    }
        if (inputInfo.pqmode.empty()) {
            inputInfo.pqmode = "Global";
	    }
        if (inputInfo.format.empty()) {
            inputInfo.format = "Global";
	    }
        if (inputInfo.color.empty()) {
            inputInfo.color = "Global";
	    }
        if (inputInfo.component.empty()) {
            inputInfo.component = "Global";
	    }

        if (convertToValidInputParameter("CMS", inputInfo) != 0) {
            LOGERR("%s: Failed to convert the input paramters. \n", __FUNCTION__);
            returnResponse(false);
        }

        if( !isCapablityCheckPassed( "CMS" , inputInfo )) {
            LOGERR("%s: CapablityCheck failed for CMS\n", __FUNCTION__);
            returnResponse(false);
        }

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

    uint32_t AVOutputTV::getCMSCapsV2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry: getCMSCapsV2");

        int max_hue = 0, max_saturation = 0, max_luma = 0;
        tvDataComponentColor_t* colorArray = nullptr;
        tvComponentType_t* componentArray = nullptr;
        size_t num_color = 0, num_component = 0;
        tvContextCaps_t* context_caps = nullptr;

        tvError_t ret = GetCMSCaps(&max_hue, &max_saturation, &max_luma,
                                &colorArray, &componentArray,
                                &num_color, &num_component, &context_caps);

        if (ret != tvERROR_NONE) {
            LOGERR("GetCMSCaps failed with error: %d", ret);
            returnResponse(false);
        }
        response["platformSupport"] = true;

        // Range Info
        JsonObject rangeHue, rangeSaturation, rangeLuma;
        rangeHue["from"] = 0;
        rangeHue["to"] = max_hue;
        rangeSaturation["from"] = 0;
        rangeSaturation["to"] = max_saturation;
        rangeLuma["from"] = 0;
        rangeLuma["to"] = max_luma;

        response["rangeHue"] = rangeHue;
        response["rangeSaturation"] = rangeSaturation;
        response["rangeLuma"] = rangeLuma;

        // Color Info
        JsonArray colorJson;
        for (size_t i = 0; i < num_color; ++i) {
            colorJson.Add(getCMSColorStringFromEnum(colorArray[i]));
        }
        response["color"] = colorJson;

        // Component Info
        JsonArray componentJson;
        for (size_t i = 0; i < num_component; ++i) {
            componentJson.Add(getCMSComponentStringFromEnum(componentArray[i]));
        }
        response["component"] = componentJson;
        response["context"] = parseContextCaps(context_caps);
#if HAL_NOT_READY
        // Clean up dynamic memory
        delete[] colorArray;
        delete[] componentArray;
#endif
        LOGINFO("Exit: getCMSCapsV2");
        returnResponse(true);
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

        err = getLocalparam("HDRMode", indexInfo,dolbyMode, PQ_PARAM_DOLBY_MODE);
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
        tvDolbyMode_t index;
        capDetails_t inputInfo;
        tvError_t ret  = tvERROR_NONE;
        std::string value;
	    int retval = 0;

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

        if( !isCapablityCheckPassed( "HDRMode", inputInfo )) {
            LOGERR("%s: CapablityCheck failed for hdrMode\n", __FUNCTION__);
            returnResponse(false);
        }

        if( isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format) ) {
            LOGINFO("Proceed with HDRMode\n\n");
            retval = getHDRModeIndex(value,inputInfo.format,index);
            if( retval != 0 )
            {
                LOGERR("Failed to getHDRMode index\n");
                returnResponse(false);
            }
            ret = SetTVDolbyVisionMode(index);
        }

        if(ret != tvERROR_NONE) {
            LOGERR("Failed to set HDRMode\n\n");
            returnResponse(false);
        }
        else {
            retval= updateAVoutputTVParam("set","HDRMode",inputInfo,PQ_PARAM_DOLBY_MODE,(int)index);
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

        int retval= updateAVoutputTVParam("reset","HDRMode",inputInfo,PQ_PARAM_DOLBY_MODE,dolbyMode);
        if(retval != 0 ) {
            LOGERR("Failed to reset HDRMode\n");
            returnResponse(false);
        }
        else {
            if (isSetRequired( inputInfo.pqmode,inputInfo.source,inputInfo.format)) {
                getParamIndex( "HDRMode", inputInfo,indexInfo);
                int err = getLocalparam("HDRMode", indexInfo, dolbyMode, PQ_PARAM_DOLBY_MODE);
                if( err == 0 ) {
                    LOGINFO("%s : getLocalparam success format :%d source : %d format : %d dolbyvalue : %d\n",__FUNCTION__,indexInfo.formatIndex, indexInfo.sourceIndex, indexInfo.pqmodeIndex, dolbyMode);
                    ret = SetTVDolbyVisionMode((tvDolbyMode_t)dolbyMode);
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

        if( inputInfo.color.empty() || inputInfo.control.empty() ) {
            LOGERR("%s : Color/Control param not found!!!\n",__FUNCTION__);
            returnResponse(false);
        }

        if (isPlatformSupport("WhiteBalance") != 0) {
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
        int retVal = 0;
        std::string color,control,value;
        tvError_t ret = tvERROR_NONE;

        inputInfo.color = parameters.HasLabel("color") ? parameters["color"].String() : "";
        inputInfo.control = parameters.HasLabel("control") ? parameters["control"].String() : "";

        if (isPlatformSupport("WhiteBalance") != 0) {
            returnResponse(false);
        }

        if( inputInfo.color.empty() || inputInfo.control.empty()  ) {
            LOGERR("%s : Color/Control param not found!!!\n",__FUNCTION__);
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

        if( (isSetRequired(inputInfo.pqmode,inputInfo.source,inputInfo.format))) {
            LOGINFO("Proceed with %s\n",__FUNCTION__);

            tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;
            tvError_t ret = GetCurrentVideoSource(&currentSource);

            if(ret != tvERROR_NONE) {
                LOGWARN("%s: GetCurrentVideoSource( ) Failed \n",__FUNCTION__);
                return -1;
            }
    
            tvWBColor_t colorLevel;
            if ( getWBColorEnumFromString(inputInfo.color,colorLevel ) == -1 ) {
                LOGERR("%s : GetColorEnumFromString Failed!!! ",__FUNCTION__);
                return -1;
            }
	
            tvWBControl_t controlLevel;
            if ( getWBControlEnumFromString(inputInfo.control,controlLevel ) == -1 ) {
                LOGERR("%s : GetComponentEnumFromString Failed!!! ",__FUNCTION__);
                return -1;
            }
             
            ret = SetCustom2PointWhiteBalance(colorLevel,controlLevel,level);
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
        int retVal = 0;
        int level = 0;
        std::string color,control;
        inputInfo.color = parameters.HasLabel("color") ? parameters["color"].String() : "";
        inputInfo.control = parameters.HasLabel("control") ? parameters["control"].String() : "";

        if (isPlatformSupport("WhiteBalance") != 0) {
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

        for( int colorIndex= tvWB_COLOR_RED; colorIndex < tvWB_COLOR_MAX; colorIndex++)  {
            for(int controlIndex = tvWB_CONTROL_GAIN;controlIndex < tvWB_CONTROL_MAX;controlIndex++) {
                inputInfo.control = getWBControlStringFromEnum((tvWBControl_t)controlIndex);
                inputInfo.color   = getWBColorStringFromEnum((tvWBColor_t)colorIndex);
                if ( convertWBParamToPQEnum(inputInfo.control,inputInfo.color,tvPQEnum) != 0 ) {
                    LOGERR("%s: %s/%s Param Not Found \n",__FUNCTION__,inputInfo.control.c_str(),inputInfo.color.c_str());
                    returnResponse(false);
                }    

                retVal |= updateAVoutputTVParam("reset","WhiteBalance",inputInfo,tvPQEnum,level);
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

    uint32_t AVOutputTV::get2PointWBCapsV2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry: get2PointWBCapsV2");

        int min_gain = 0, min_offset = 0, max_gain = 0, max_offset = 0;
        tvWBColor_t* colorArray = nullptr;
        tvWBControl_t* controlArray = nullptr;
        size_t num_color = 0, num_control = 0;
        tvContextCaps_t* context_caps = nullptr;

        tvError_t ret = GetCustom2PointWhiteBalanceCaps(&min_gain, &min_offset, &max_gain, &max_offset,
                                                        &colorArray, &controlArray,
                                                        &num_color, &num_control, &context_caps);

        if (ret != tvERROR_NONE) {
            LOGERR("GetCustom2PointWhiteBalanceCaps failed with error: %d", ret);
            returnResponse(false);
        }

        response["platformSupport"] = true;

        // Range Info
        JsonObject rangeGain, rangeOffset;
        rangeGain["from"] = min_gain;
        rangeGain["to"] = max_gain;
        rangeOffset["from"] = min_offset;
        rangeOffset["to"] = max_offset;

        response["rangeGain"] = rangeGain;
        response["rangeOffset"] = rangeOffset;

        // Control Info
        JsonArray controlJson;
        for (size_t i = 0; i < num_control; ++i) {
            controlJson.Add(getWBControlStringFromEnum(controlArray[i]));
        }
        response["control"] = controlJson;

        // Color Info
        JsonArray colorJson;
        for (size_t i = 0; i < num_color; ++i) {
            colorJson.Add(getWBColorStringFromEnum(colorArray[i]));
        }
        response["color"] = colorJson;
        response["context"] = parseContextCaps(context_caps);

#if HAL_NOT_READY
        delete[] colorArray;
        delete[] controlArray;
#endif

        LOGINFO("Exit: get2PointWBCapsV2");
        returnResponse(true);
    }


    uint32_t AVOutputTV::get2PointWBCaps(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        capVectors_t info;

        JsonArray rangeArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;
        JsonArray sourceArray;
        JsonArray colorArray;
        JsonArray controlArray;

        JsonObject gainInfo;
        JsonObject offsetInfo;

        unsigned int index = 0;

        tvError_t ret = getParamsCaps("WhiteBalance",info);

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

            if ((info.controlVector.front()).compare("none") != 0) {
                for (index = 0; index < info.controlVector.size(); index++) {
                    controlArray.Add(info.controlVector[index]);
                }
                response["controlInfo"]=controlArray;
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
        if(m_backlightModeStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
            std::string value;
            tvBacklightMode_t mode = tvBacklightMode_AMBIENT;
            capDetails_t inputInfo;

            value = parameters.HasLabel("mode") ? parameters["mode"].String() : "";
            returnIfParamNotFound(parameters,"mode");

            if (validateInputParameter("AutoBacklightMode",value) != 0) {
                LOGERR("%s: Range validation failed for AutoBacklightMode\n", __FUNCTION__);
                returnResponse(false);
            }

            if (isPlatformSupport("AutoBacklightMode") != 0) {
                returnResponse(false);
            }

            if (parsingSetInputArgument(parameters,"AutoBacklightMode",inputInfo) != 0) {
                LOGERR("%s: Failed to parse the input arguments \n", __FUNCTION__);
                returnResponse(false);
            }

            if( !isCapablityCheckPassed( "AutoBacklightMode",inputInfo )) {
                LOGERR("%s: CapablityCheck failed for AutoBacklightMode\n", __FUNCTION__);
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
        else
        {
            bool success = false;
            success =  setEnumPQParam(
                parameters,
                "mode",
                "BacklightMode",
                backlightModeReverseMap,
                PQ_PARAM_BACKLIGHT_MODE,
                [](int val) {
                    return SetCurrentBacklightMode(static_cast<tvBacklightMode_t>(val));
                });

            returnResponse(success);
        }
    }

    uint32_t AVOutputTV::getAutoBacklightMode(const JsonObject& parameters, JsonObject& response)
    {
        if(m_backlightModeStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
            TR181_ParamData_t param;

            if (isPlatformSupport("AutoBacklightMode") != 0) {
                returnResponse(false);
            }

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
        else
        {
            std::string mode;
            if (getEnumPQParamString(parameters, "BacklightMode",
                 PQ_PARAM_BACKLIGHT_MODE, backlightModeMap, mode)) {
                response["mode"] = mode;
                returnResponse(true);
            } else {
                returnResponse(false);
            }
        }
    
    }

    uint32_t AVOutputTV::resetAutoBacklightMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        if(m_backlightModeStatus == tvERROR_OPERATION_NOT_SUPPORTED)
        {
            tvError_t ret = tvERROR_NONE;

            if (isPlatformSupport("AutoBacklightMode") != 0) {
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
                    tvBacklightMode_t blMode = tvBacklightMode_NONE;

                    if(!std::string(param.value).compare("none")) {
                        blMode = tvBacklightMode_NONE;
                    }
                    else if (!std::string(param.value).compare("Manual")){
                        blMode = tvBacklightMode_MANUAL;
                    }
                    else if (!std::string(param.value).compare("Ambient")){
                        blMode = tvBacklightMode_AMBIENT;
                    }
                    else if (!std::string(param.value).compare("Eco")){
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
        else
        {
            bool success = resetEnumPQParamToDefault(
                    parameters,
                    "BacklightMode",
                    PQ_PARAM_BACKLIGHT_MODE,
                    backlightModeMap,
                    [](int value, const std::unordered_map<int, std::string>&) -> tvError_t {
                        return SetCurrentBacklightMode(static_cast<tvBacklightMode_t>(value));
                    });
            returnResponse(success);
        }
    }    

    uint32_t AVOutputTV::getVideoSource(const JsonObject& parameters,JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;

        tvError_t ret = GetCurrentVideoSource(&currentSource);
        if(ret != tvERROR_NONE) {
            response["currentVideoSource"] = "NONE";
            returnResponse(false);
        }
        else {
            response["currentVideoSource"] = convertSourceIndexToStringV2(currentSource);
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
