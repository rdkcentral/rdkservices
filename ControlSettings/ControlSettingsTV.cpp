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

namespace WPEFramework {
namespace Plugin {

    ControlSettingsTV::ControlSettingsTV(): AbstractPlugin(3)
					   ,numberModesSupported(0),pic_mode_index()
                                           ,appUsesGlobalBackLightFactor(false),source_index()
                                           ,rfc_caller_id()
    						
    {
        LOGINFO("Entry\n");
        instance = this;
        registerMethod("getBacklight", &ControlSettingsTV::getBacklight, this, {1});
        registerMethod("setBacklight", &ControlSettingsTV::setBacklight, this, {1});
        registerMethod("getBrightness", &ControlSettingsTV::getBrightness, this, {1});
        registerMethod("setBrightness", &ControlSettingsTV::setBrightness, this, {1});
        LOGINFO("Exit\n");
    }
    
    ControlSettingsTV :: ~ControlSettingsTV()
    {
        LOGINFO();    
    }

    void ControlSettingsTV::Initialize()
    {
       LOGINFO("Entry\n");
       //Space for Device specific Init Sequence
       //Get number of pqmode supported
       numberModesSupported=GetNumberOfModesupported();

       //UpdatePicModeIndex
       GetAllSupportedPicModeIndex(pic_mode_index);

       //Get number of pqmode supported
       numberSourcesSupported=GetNumberOfSourceSupported();

       //UpdatePicModeIndex
       GetAllSupportedSourceIndex(source_index);


       LocatePQSettingsFile(rfc_caller_id);

       appUsesGlobalBackLightFactor = isBacklightUsingGlobalBacklightFactor();

       LOGINFO("Exit\n");
    }

    void ControlSettingsTV::Deinitialize()
    {
       LOGINFO("Entry\n");
       LOGINFO("Exit\n");
    }

    //Event
    void ControlSettingsTV::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        LOGINFO("Entry %s\n",__FUNCTION__);
        LOGINFO("Exit %s\n",__FUNCTION__);
    }

    uint32_t ControlSettingsTV::getBacklight(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry %s\n",__FUNCTION__);
       // PLUGIN_Lock(Lock);
        tvError_t ret = tvERROR_NONE;

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            LOGINFO("Exit : %s\n",__FUNCTION__);
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::setBacklight(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
      //  PLUGIN_Lock(Lock);
        tvError_t ret = tvERROR_NONE;

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            LOGINFO("Exit : %s\n",__FUNCTION__);
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::getBrightness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
    //    PLUGIN_Lock(Lock);

        std::string pqmode;
        std::string source;
        std::string format;
	std::string key;
	int sourceIndex=0,pqIndex=0,formatIndex=0;
	TR181_ParamData_t param={0};
        tvError_t ret = tvERROR_NONE;
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
        int err = GetLocalparam("Brightness",formatIndex,pqIndex,sourceIndex,brightness);
        if( err = 0 ) {
            brightness = std::stoi(param.value);
            response["brightness"] = std::to_string(brightness);
            LOGINFO("Exit : Brightness Value: %d\n", brightness);
            returnResponse(true, "success");
        }
        else {
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
    }



    uint32_t ControlSettingsTV::setBrightness(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
       // PLUGIN_Lock(Lock);

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int brightness = 0;

        value = parameters.HasLabel("brightness") ? parameters["brightness"].String() : "";
        returnIfParamNotFound(value);
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

        tvError_t ret = SetBrightness(brightness);

	if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set Brightness\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=brightness;
            int retval= UpdatePQParamsToCache("set","Brightness",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_BRIGHTNESS,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Brightness to ssm_data\n");
            }
            LOGINFO("Exit : setBrightness successful to value: %d\n", brightness);
            returnResponse(true, "success");
        }

    }

    bool ControlSettingsTV::isBacklightUsingGlobalBacklightFactor(void)
    {
        TR181_ParamData_t param;
        bool ret  =false;

        memset(&param, 0, sizeof(param));

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_BACKLIGHT_CONTROL_USE_GBF_RFC_PARAM,&param);
        if ( err != tr181Success ) {
            LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_BACKLIGHT_CONTROL_USE_GBF_RFC_PARAM, getTR181ErrorString(err));
        }
        else {
            if(!std::string(param.value).compare("true"))
            {
                ret = true;
            }
        }
        return ret;
    }

    int ControlSettingsTV::getCurrentPictureMode(char *picMode)
    {

        TR181_ParamData_t param;
        memset(&param, 0, sizeof(param));

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, &param);
        if ( err == tr181Success ) {
            strncpy(picMode, param.value, strlen(param.value)+1);
            LOGINFO("getLocalParam success, mode = %s\n", picMode);
            return 1;
        }
        else {
            LOGWARN("getLocalParam failed");
            return 0;
        }
    }

    void ControlSettingsTV::GetParamIndex(string source,string pqmode,string format,int& sourceIndex,int& pqmodeIndex,int& formatIndex)
    {
        LOGINFO("Entry\n");

        if( (strncmp(source.c_str(),"current",strlen(source.c_str())) == 0) || (strncmp(source.c_str(),"Current",strlen(source.c_str())) == 0) )
            GetCurrentSource(&sourceIndex);
        else
            sourceIndex = GetTVSourceIndex(source.c_str());

        if( (strncmp(pqmode.c_str(),"current",strlen(pqmode.c_str())) == 0) || (strncmp(pqmode.c_str(),"Current",strlen(pqmode.c_str())) == 0) )
        {
            char picMode[PIC_MODE_NAME_MAX]={0};
            if(!getCurrentPictureMode(picMode))
                LOGWARN("Failed to get the current picture mode\n");
            else
                pqmodeIndex = GetTVPictureModeIndex(picMode);
        }
        else
            pqmodeIndex = GetTVPictureModeIndex(pqmode.c_str());

        if( (strncmp(format.c_str(),"current",strlen(format.c_str())) == 0) || (strncmp(format.c_str(),"Current",strlen(format.c_str())) == 0) )
        {
            formatIndex = getContentFormatIndex(GetCurrentContentFormat());
        }
        else
            formatIndex = ConvertHDRFormatToContentFormat((tvhdr_type_t)ConvertFormatStringToHDRFormat(format.c_str()));

    }

    int ControlSettingsTV::getContentFormatIndex(tvVideoHDRFormat_t formatToConvert)
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

    int ControlSettingsTV::GetLocalparam(const char * forParam,int formatIndex,int pqIndex,int sourceIndex,int &value)
    {
        string key;
        TR181_ParamData_t param={0};

        generateStorageIdentifier(key,forParam,formatIndex,pqIndex,sourceIndex);
        if(key.empty())
        {
            LOGWARN("generateStorageIdentifier failed\n");
            return -1;
        }

       tr181ErrorCode_t err=getLocalParam(rfc_caller_id, key.c_str(), &param);
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
               return 0;
           }
           else
           {
               value=std::stoi(param.value);
	       return 0;
	   }
        }
        else
        {
            key.clear();
            memset(&param, 0, sizeof(param));
            key+=std::string(TVSETTINGS_GENERIC_STRING_RFC_PARAM);
            key+=STRING_SOURCE+std::string("ALL")+std::string(".")+STRING_PICMODE+std::to_string(pqIndex)+std::string(".")+std::string(STRING_FORMAT)                 +std::to_string(formatIndex)+std::string(".")+forParam;
            err=getLocalParam(rfc_caller_id, key.c_str(), &param);
            if ( tr181Success == err ) {
                value=std::stoi(param.value);
                LOGINFO("GetPQParamsToSync : found default %d \n",value);
                return 0;
            }
            else
            {
                LOGWARN("Default not found %s \n",key.c_str());
                return -1;
            }
        }
    }
}//namespace Plugin
}//namespace WPEFramework
