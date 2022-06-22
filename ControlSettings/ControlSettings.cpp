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
#include "ControlSettings.h"

const char* PLUGIN_IARM_BUS_NAME = "Thunder_Plugins";

static char videoDescBuffer[VIDEO_DESCRIPTION_MAX*VIDEO_DESCRIPTION_NAME_SIZE] = {0};

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(ControlSettings,1, 0);

    ControlSettings* ControlSettings::_instance = nullptr;

    static void tvVideoFormatChangeHandler(tvVideoHDRFormat_t format, void *userData)
    {
        printf("tvVideoFormatChangeHandler format:%d \n",format);
        ControlSettings *obj = (ControlSettings *)userData;
        if(obj)obj->NotifyVideoFormatChange(format);
    }

    static void tvVideoResolutionChangeHandler(tvResolutionParam_t resolution, void *userData)
    {
        printf("tvVideoResolutionChangeHandler resolution:%d\n",resolution.resolutionValue);
        ControlSettings *obj = (ControlSettings *)userData;
        if(obj)obj->NotifyVideoResolutionChange(resolution);
    }

    static void tvVideoFrameRateChangeHandler(tvVideoFrameRate_t frameRate, void *userData)
    {
        printf("tvVideoFrameRateChangeHandler format:%d \n",frameRate);
        ControlSettings *obj = (ControlSettings *)userData;
        if(obj)obj->NotifyVideoFrameRateChange(frameRate);
    }

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


    ControlSettings::ControlSettings()
               : _skipURL(0)
               , m_currentHdmiInResoluton (dsVIDEO_PIXELRES_1920x1080)
               , m_videoZoomMode (tvDisplayMode_NORMAL)
               , m_isDisabledHdmiIn4KZoom (false)
    {
        LOGINFO("Entry\n");

	ControlSettings::_instance = this;
	InitializeIARM();

        //Common API Registration
	registerMethod("getAspectRatio", &ControlSettings::getAspectRatio, this, {1});
        registerMethod("setAspectRatio", &ControlSettings::setAspectRatio, this, {1});
        LOGINFO("Exit \n");
    }

    ControlSettings::~ControlSettings()
    {
        LOGINFO();
    }

    const std::string ControlSettings::Initialize(PluginHost::IShell* service)
    {
	LOGINFO("Entry\n");
        try {
            dsVideoPortResolution_t vidResolution;
            device::HdmiInput::getInstance().getCurrentVideoModeObj(vidResolution);
            m_currentHdmiInResoluton = vidResolution.pixelResolution;
        } catch (...){
            LOGWARN("ControlSettingsPlugins: getCurrentVideoModeObj failed");
        }
        LOGWARN("ControlSettingsPlugins: ControlSettings Initialize m_currentHdmiInResoluton:%d m_mod:%d", m_currentHdmiInResoluton, m_videoZoomMode);

        ASSERT(service != nullptr);
        _skipURL = static_cast<uint8_t>(service->WebPrefix().length());

        tvError_t ret = tvERROR_NONE;

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
        tvVideoFormatCallbackData callbackData = {this,tvVideoFormatChangeHandler};
        RegisterVideoFormatChangeCB(callbackData);

        tvVideoResolutionCallbackData RescallbackData = {this,tvVideoResolutionChangeHandler};
        RegisterVideoResolutionChangeCB(RescallbackData);

        tvVideoFrameRateCallbackData FpscallbackData = {this,tvVideoFrameRateChangeHandler};
        RegisterVideoFrameRateChangeCB(FpscallbackData);

	instance->Initialize();

	LOGINFO("Exit\n");
        return (service != nullptr ? _T("") : _T("No service."));
    }

    void ControlSettings::Deinitialize(PluginHost::IShell* service)
    {
        LOGINFO();
        tvError_t ret = tvERROR_NONE;
        ret = tvTerm();

        if(ret != tvERROR_NONE) {
            LOGERR("Platform De-Init failed");
        }
        else{
            LOGINFO("Platform De-Init successful... \n");
        }
        ControlSettings::_instance = nullptr;
        DeinitializeIARM();

	instance->Deinitialize();
    }

    void ControlSettings::InitializeIARM()
    {
#if !defined (HDMIIN_4K_ZOOM)
        if (IARMinit())
        {
            IARM_Result_t res;
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS, dsHdmiStatusEventHandler) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, dsHdmiVideoModeEventHandler) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG, dsHdmiEventHandler) );
        }
#endif
    }

    void ControlSettings::DeinitializeIARM()
    {
#if !defined (HDMIIN_4K_ZOOM)
        if (isIARMConnected())
        {
            IARM_Result_t res;
            IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS) );
            IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE) );
            IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG) );
        }
#endif
    }

    bool ControlSettings::IARMinit() {
        IARM_Result_t res;
        bool result = false;

        if (isIARMConnected()) {
            LOGINFO("ControlSettingsPlugin: IARM already connected");
            result = true;
        } else {
            res = IARM_Bus_Init(PLUGIN_IARM_BUS_NAME);
            LOGINFO("ControlSettingsPlugin: IARM_Bus_Init: %d", res);
            if (res == IARM_RESULT_SUCCESS ||
                res == IARM_RESULT_INVALID_STATE /* already inited or connected */) {

                res = IARM_Bus_Connect();
                LOGINFO("ControlSettingsPlugin: IARM_Bus_Connect: %d", res);
                if (res == IARM_RESULT_SUCCESS ||
                    res == IARM_RESULT_INVALID_STATE /* already connected or not inited */) {

                    result = isIARMConnected();
                } else {
                    LOGERR("ControlSettingsPlugin: IARM_Bus_Connect failure: %d", res);
                }
            } else {
                LOGERR("ControlSettingsPlugin: IARM_Bus_Init failure: %d", res);
            }
        }

        return result;
    }

    bool ControlSettings::isIARMConnected() {
        IARM_Result_t res;
        int isRegistered = 0;
        res = IARM_Bus_IsConnected(PLUGIN_IARM_BUS_NAME, &isRegistered);
        LOGINFO("ControlSettingsPlugin: IARM_Bus_IsConnected: %d (%d)", res, isRegistered);

        return (isRegistered == 1);
    }
   
    void ControlSettings::dsHdmiVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if(!ControlSettings::_instance)
            return;

        if (IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE == eventId)
        {
            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            int hdmi_in_port = eventData->data.hdmi_in_video_mode.port;
            dsVideoPortResolution_t resolution;
            ControlSettings::_instance->m_currentHdmiInResoluton = eventData->data.hdmi_in_video_mode.resolution.pixelResolution;
            resolution.pixelResolution =  eventData->data.hdmi_in_video_mode.resolution.pixelResolution;
            resolution.interlaced =  eventData->data.hdmi_in_video_mode.resolution.interlaced;
            resolution.frameRate =  eventData->data.hdmi_in_video_mode.resolution.frameRate;
            LOGWARN("ControlSettingsPlugins: Received IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE  event  port: %d, pixelResolution: %d, interlaced : %d, frameRate: %d \n", hdmi_in_port,resolution.pixelResolution, resolution.interlaced, resolution.frameRate);
            if (ControlSettings::_instance->m_isDisabledHdmiIn4KZoom) {
                tvError_t ret = tvERROR_NONE;
                if (ControlSettings::_instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
                    (dsVIDEO_PIXELRES_MAX == ControlSettings::_instance->m_currentHdmiInResoluton)){
                    LOGWARN("ControlSettingsPlugins: Setting %d zoom mode for below 4K", ControlSettings::_instance->m_videoZoomMode);
                    ret = SetAspectRatio((tvDisplayMode_t)ControlSettings::_instance->m_videoZoomMode);
                }else {
                    LOGWARN("ControlSettingsPlugins: Setting auto zoom mode for 4K and above");
                    ret = SetAspectRatio(tvDisplayMode_AUTO);
                }
                if (ret != tvERROR_NONE) {
                    LOGWARN("SetAspectRatio set Failed");
                }
            } else {
                LOGWARN("ControlSettingsPlugins: %s: HdmiInput is not started yet. m_isDisabledHdmiIn4KZoom: %d", __FUNCTION__, ControlSettings::_instance->m_isDisabledHdmiIn4KZoom);
            }
        }
    }

    void ControlSettings::dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if(!ControlSettings::_instance)
            return;

        if (IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS == eventId)
        {
            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            int hdmi_in_port = eventData->data.hdmi_in_status.port;
            bool hdmi_in_status = eventData->data.hdmi_in_status.isPresented;
            LOGWARN("ControlSettingsPlugins: Received IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS  event  port: %d, started: %d", hdmi_in_port,hdmi_in_status);
            if (!hdmi_in_status){
                tvError_t ret = tvERROR_NONE;
                ControlSettings::_instance->m_isDisabledHdmiIn4KZoom = false;
                LOGWARN("ControlSettingsPlugins: Hdmi streaming stopped here reapply the global zoom settings:%d here. m_isDisabledHdmiIn4KZoom: %d", ControlSettings::_instance->m_videoZoomMode, ControlSettings::_instance->m_isDisabledHdmiIn4KZoom);
                ret = SetAspectRatio((tvDisplayMode_t)ControlSettings::_instance->m_videoZoomMode);
                if (ret != tvERROR_NONE) {
                    LOGWARN("SetAspectRatio set Failed");
                }
            }else {
                ControlSettings::_instance->m_isDisabledHdmiIn4KZoom = true;
                LOGWARN("ControlSettingsPlugins: m_isDisabledHdmiIn4KZoom: %d", ControlSettings::_instance->m_isDisabledHdmiIn4KZoom);
            }
        }
    }
    
    void ControlSettings::NotifyVideoFormatChange(tvVideoHDRFormat_t format)
    {
        JsonObject response;
        response["currentVideoFormat"] = getVideoFormatTypeToString(format);
        response["supportedVideoFormat"] = getSupportedVideoFormat();
        sendNotify("videoFormatChanged", response);
    }

    uint32_t ControlSettings::getVideoFormat(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO();
        PLUGIN_Lock(Lock);
        tvVideoHDRFormat_t videoFormat;
        tvError_t ret = GetCurrentVideoFormat(&videoFormat);
        response["supportedVideoFormat"] = getSupportedVideoFormat();
        if(ret != tvERROR_NONE) {
            response["currentVideoFormat"] = "NONE";
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            response["currentVideoFormat"] = getVideoFormatTypeToString(videoFormat);
            LOGINFO(" getVideoFormat :%d   success \n",videoFormat);
            returnResponse(true, "success");
        }
    }

    void ControlSettings::NotifyVideoResolutionChange(tvResolutionParam_t resolution)
    {
        JsonObject response;
        response["currentVideoResolution"] = getVideoResolutionTypeToString(resolution);
        response["supportedVideoResolution"] = getSupportedVideoResolution();
        sendNotify("videoResolutionChanged", response);
    }

    uint32_t ControlSettings::getVideoResolution(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO();
        PLUGIN_Lock(Lock);
        tvResolutionParam_t videoResolution;
        tvError_t ret = GetVideoResolution(&videoResolution);
        response["supportedVideoResolution"] = getSupportedVideoResolution();
        if(ret != tvERROR_NONE) {
            response["currentVideoResolution"] = "NONE";
            returnResponse(false, getErrorString(ret));
        }
        else {
            response["currentVideoResolution"] = getVideoResolutionTypeToString(videoResolution);
            LOGINFO(" getVideoResolution :%d   success \n",videoResolution.resolutionValue);
            returnResponse(true, "success");
        }
    }

     void ControlSettings::NotifyVideoFrameRateChange(tvVideoFrameRate_t frameRate)
    {
        JsonObject response;
        response["currentVideoFrameRate"] = getVideoFrameRateTypeToString(frameRate);
        response["supportedVideoFrameRate"] = getSupportedVideoFrameRate();
        sendNotify("videoFrameRateChanged", response);
    }

    uint32_t ControlSettings::getVideoFrameRate(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO();
        PLUGIN_Lock(Lock);
        tvVideoFrameRate_t videoFramerate;
        tvError_t ret = GetVideoFrameRate(&videoFramerate);
        response["supportedFrameRate"] = getSupportedVideoFrameRate();
        if(ret != tvERROR_NONE) {
            response["currentVideoFrameRate"] = "NONE";
            returnResponse(false, getErrorString(ret));
        }
        else {
            response["currentVideoFrameRate"] = getVideoFrameRateTypeToString(videoFramerate);
            LOGINFO(" videoFramerate :%d   success \n",videoFramerate);
            returnResponse(true, "success");
        }
    }

    //API
    uint32_t ControlSettings::getAspectRatio(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry %s\n",__FUNCTION__);
        PLUGIN_Lock(Lock);
	tvError_t ret = tvERROR_NONE;

	if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            LOGINFO("Exit : %s\n",__FUNCTION__);
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettings::setAspectRatio(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry%s\n",__FUNCTION__);
        PLUGIN_Lock(Lock);
	tvError_t ret = tvERROR_NONE;

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            LOGINFO("Exit : %s\n",__FUNCTION__);
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettings::generateStorageIdentifier(std::string &key,const char * forParam,int contentFormat, int pqmode, int source)
    {
        key+=std::string(TVSETTINGS_GENERIC_STRING_RFC_PARAM);
        key+=STRING_SOURCE+std::to_string(source)+std::string(".")+STRING_PICMODE+std::to_string(pqmode)+std::string(".")+std::string(STRING_FORMAT)+std::to_string(contentFormat)+std::string(".")+forParam;

        return tvERROR_NONE;
    }

    int ControlSettings::GetSaveConfig(const char *pqmode, const char *source, const char *format,std::vector<int> &sources,std::vector<int> &picturemodes, std::vector<int> &formats)
    {
        LOGINFO("Entry : %s pqmode : %s source :%s format :%s\n",__FUNCTION__,pqmode,source,format);

        int ret = 0;

        //1)Check pqmode
        if( (strncmp(pqmode,"All",strlen(pqmode)) == 0) || (strncmp(pqmode,"all",strlen(pqmode)) == 0) ) 
        {
            int lCount = 0;
            for(;lCount<numberModesSupported;lCount++)
                picturemodes.push_back(pic_mode_index[lCount]);
        }
        else if( (strncmp(pqmode,"Current",strlen(pqmode)) == 0) || (strncmp(pqmode,"current",strlen(pqmode)) == 0) )
        {
            char picMode[PIC_MODE_NAME_MAX]={0};
            if(!getCurrentPictureMode(picMode))
            {
                LOGWARN("Failed to get the current picture mode\n");
                ret = -1;
            }
            else
                picturemodes.push_back(GetTVPictureModeIndex(picMode));
        }
        else
	{
            picturemodes.push_back(GetTVPictureModeIndex(pqmode));
	}

        //2)Check Source
        if( (strncmp(source,"All",strlen(source)) == 0) || (strncmp(source,"all",strlen(source)) == 0) )
        {
            int lCount = 0;
            for(;lCount<numberSourcesSupported;lCount++)
                sources.push_back(source_index[lCount]);
        }
        else if( (strncmp(source,"Current",strlen(source)) == 0) || (strncmp(source,"current",strlen(source)) == 0) )
        {
            int currentSource = 0;
            tvError_t ret = GetCurrentSource(&currentSource);
	    if(ret != tvERROR_NONE) 
            {
                LOGINFO("GetCurrentSource() Failed set source to default\n");
                currentSource = 10;
            }
                sources.push_back(currentSource);
        }
        else
	{
            sources.push_back(GetTVSourceIndex(source));
	}

         //3)check format
        unsigned int contentFormats=0;
        unsigned short numberOfSupportedFormats =  0;

        GetSupportedContentFormats(&contentFormats,&numberOfSupportedFormats);

        if( (strncmp(format,"All",strlen(format)) == 0) || (strncmp(format,"all",strlen(format)) == 0) )
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
        else if( (strncmp(format,"Current",strlen(format)) == 0) || (strncmp(format,"current",strlen(format)) == 0) )
        {
            if( HDR_TYPE_NONE == ConvertVideoFormatToHDRFormat(GetCurrentContentFormat()))
                formats.push_back(HDR_TYPE_SDR);//Save  To SDR if format is HDR_TYPE_NONE
            else
                formats.push_back(ConvertVideoFormatToHDRFormat(GetCurrentContentFormat()));
        }
        else
        {
            formats.push_back(ConvertFormatStringToHDRFormat(format));//This function needs to be implemented
        }
       
	LOGINFO("Exit : %s ret : %d\n",__FUNCTION__,ret);
        return ret;
    }


    tvError_t ControlSettings::UpdatePQParamToLocalCache(const char* forParam, int source, int pqmode, int format, int value,bool setNotDelete)
    {
        tvError_t ret = tvERROR_NONE;
        std::string key;

	LOGINFO("Entry : %s source:%d,pqmode:%d,format:%d\n",__FUNCTION__,source,pqmode,format);

        if((!strncmp(forParam,"Backlight",strlen("Backlight")))&&
            appUsesGlobalBackLightFactor)
        {
            /* Do nothing this global BLF using single values for SDR and HDR
             * stored only once per format and handled separately
             */
            return ret;
        }

        format=ConvertHDRFormatToContentFormat((tvhdr_type_t)format);
        key.clear();
        generateStorageIdentifier(key,forParam,format,pqmode,source);
        if(key.empty())
        {
            LOGWARN("generateStorageIdentifierDirty failed\n");
            ret = tvERROR_GENERAL;
        }
         else
        {
            tr181ErrorCode_t err  = tr181Success;
            if(setNotDelete)
            {
                std::string toStore = std::to_string(value);
                if (!strncmp(forParam,"ColorTemp",strlen("ColorTemp")))
                {
                    GetColorTempStringFromEnum(value, toStore);
                }
                err = setLocalParam(rfc_caller_id, key.c_str(),toStore.c_str());

            }
            else
                err = clearLocalParam(rfc_caller_id, key.c_str());

            if ( err != tr181Success ) {
                LOGWARN("%s for %s Failed : %s\n", setNotDelete?"Set":"Delete", key.c_str(), getTR181ErrorString(err));
                ret = tvERROR_GENERAL;
            }
            else {
                LOGINFO("%s for %s Successful \n", setNotDelete?"Set":"Delete",key.c_str());
            }
        }
        return ret;
    }

    int ControlSettings::UpdatePQParamsToCache( const char *action, const char *tr181ParamName, const char *pqmode, const char *source, const char *format, tvPQParameterIndex_t pqParamIndex, int params[] )
    {
        LOGINFO("Entry : %s\n",__FUNCTION__);
	std::vector<int> sources;
	std::vector<int> pictureModes;
	std::vector<int> formats;
        int ret = 0;
        bool sync = !strncmp(action,"sync",strlen("sync"));
        bool reset = !strncmp(action,"reset",strlen("reset"));
        bool set = !strncmp(action,"set",strlen("set"));

        LOGINFO("%s: Entry : set: %d pqmode : %s source :%s format :%s\n",__FUNCTION__,set,pqmode,source,format);
        ret = GetSaveConfig(pqmode, source, format, sources, pictureModes, formats);
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
                                if(reset)
                                    ret |= UpdatePQParamToLocalCache(tr181ParamName,source, mode, format,0,false);
                                if(sync || reset)
                                {
                                    int value=0;
                                    if(!GetPQParamsToSync(tr181ParamName,source,mode,format,value))
                                        LOGINFO("Found param from tr181 %s pqmode : %d format:%d value:%d\n",tr181ParamName,mode,format,value);
                                    else
                                        LOGINFO("value not found in tr181 %s pqmode : %d format:%d value:%d\n",tr181ParamName,mode,format,value);
                                  
                                    params[0]=value;
                                }
                                if(set)
                                    ret |= UpdatePQParamToLocalCache(tr181ParamName,source, mode, format, params[0],true);
                                break;
                            default:
                                break;
                        }
                        switch(pqParamIndex)
                        {
                            case PQ_PARAM_BRIGHTNESS:
                                ret |= SaveBrightness(source, mode,format,params[0]);
                                break;
                        }
                    }
                }
           }

        }
        return ret;
    }


    void ControlSettings::GetColorTempStringFromEnum(int value, std::string &toStore)
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

    int ControlSettings::GetPQParamsToSync (const char *forParam,int source,int pqmode,int format, int& value,bool cms,int tunnel_type)
    {
        int ret=0;
        TR181_ParamData_t param={0};
        string key;
        tr181ErrorCode_t err;

        format=ConvertHDRFormatToContentFormat((tvhdr_type_t)format);
        key.clear();
        generateStorageIdentifier(key,forParam,format,pqmode,source);

        err=getLocalParam(rfc_caller_id, key.c_str(), &param);
        if ( tr181Success == err )
        {
            if(strncmp(forParam,"ColorTemp",strlen(forParam))==0)
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
            }
            else
                value=std::stoi(param.value);
        }
        else
        {
            if(cms)
            {
                value=GetCMSDefault((tvCMS_tunel_t)tunnel_type);
                ret =  -1;/*block default cms sync and allow default values during reset*/
            }
            else
            {
                key.clear();
                memset(&param, 0, sizeof(param));
                key+=string(TVSETTINGS_GENERIC_STRING_RFC_PARAM);
                key+=STRING_PICMODE+std::to_string(pqmode)+string(".")+string(STRING_FORMAT)+std::to_string(format);
                key+=string(".")+forParam;
                err=getLocalParam(rfc_caller_id, key.c_str(), &param);
                if ( tr181Success == err ) {
                    value=std::stoi(param.value);
                    LOGINFO("GetPQParamsToSync : found default %d \n",value);
                }
                else
                {
                    LOGWARN("Default not found %s \n",key.c_str());
                    ret = -1;
                }
            }

        }
        return ret;
    }

} //namespace WPEFramework

} //namespace Plugin
