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
        registerMethod("resetBrightness", &ControlSettingsTV::resetBrightness, this, {1});

	this->Initialize();

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

       LOGINFO("Exit numberModesSupported =%d numberSourcesSupported=%d\n",numberModesSupported,numberSourcesSupported);
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
        long long start =  getCurrentTimeMicro();
        LOGINFO("Entry start = %lld\n",start);

        std::string pqmode;
        std::string source;
        std::string format;
	std::string key;
	int sourceIndex=0,pqIndex=0,formatIndex=0;
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
        if( err == 0 ) {
            response["brightness"] = std::to_string(brightness);
	    long long end =  getCurrentTimeMicro();
            LOGINFO("Exit : Brightness Value: %d end = %lld  duration = %lld \n", brightness,end,end-start);
            returnResponse(true, "success");
        }
        else {
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
    }



    uint32_t ControlSettingsTV::setBrightness(const JsonObject& parameters, JsonObject& response)
    {
        long long start =  getCurrentTimeMicro();
        LOGINFO("Entry start = %lld\n",start);

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
	    long long end =  getCurrentTimeMicro();
            LOGINFO("Exit : setBrightness successful to value: %d end = %lld  duration = %lld \n", brightness,end,end-start);
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::resetBrightness(const JsonObject& parameters, JsonObject& response)
    {

        long long start =  getCurrentTimeMicro();
        LOGINFO("Entry start = %lld\n",start);

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

        int retval= UpdatePQParamsToCache("reset","Brightness",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_BRIGHTNESS,params);
        if(retval != 0 ) {
            LOGWARN("Failed to reset Brightness\n");
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("Brightness",formatIndex,pqIndex,sourceIndex,brightness);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,brightness);
                ret = SetBrightness(brightness);
            }
            else
                LOGINFO("%s : GetLcoalParam Failed \n",__FUNCTION__);
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            long long end =  getCurrentTimeMicro();
            LOGINFO("Exit : resetBrightness Successful to value : %d  end = %lld  duration = %lld \n",brightness,end,end-start);
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
            formatIndex = ConvertVideoFormatToHDRFormat(GetCurrentContentFormat());
        }
        else
            formatIndex = ConvertFormatStringToHDRFormat(format.c_str());

        LOGINFO("%s: Exit sourceIndex = %d pqmodeIndex = %d formatIndex = %d\n",__FUNCTION__,sourceIndex,pqmodeIndex,formatIndex);

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
        LOGINFO("Entry : %s\n",__FUNCTION__);

        string key;
        TR181_ParamData_t param={0};
       
        formatIndex=ConvertHDRFormatToContentFormat((tvhdr_type_t)formatIndex); 
        generateStorageIdentifier(key,forParam,formatIndex,pqIndex,sourceIndex);
        if(key.empty())
        {
            LOGWARN("generateStorageIdentifier failed\n");
            return -1;
        }

       tr181ErrorCode_t err=getLocalParam(rfc_caller_id, key.c_str(), &param);
       LOGINFO("%s: key %s\n",__FUNCTION__,key.c_str());
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
            LOGINFO("%s: key %s\n",__FUNCTION__,key.c_str());
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

    uint32_t ControlSettingsTV::generateStorageIdentifier(std::string &key,const char * forParam,int contentFormat, int pqmode, int source)
    {
        key+=std::string(TVSETTINGS_GENERIC_STRING_RFC_PARAM);
        key+=STRING_SOURCE+std::to_string(source)+std::string(".")+STRING_PICMODE+std::to_string(pqmode)+std::string(".")+std::string(STRING_FORMAT)+std::to_string(contentFormat)+std::string(".")+forParam;
        return tvERROR_NONE;
    }

    int ControlSettingsTV::GetSaveConfig(const char *pqmode, const char *source, const char *format,std::vector<int> &sources,std::vector<int> &picturemodes, std::vector<int> &formats)
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

     tvError_t ControlSettingsTV::UpdatePQParamToLocalCache(const char* forParam, int source, int pqmode, int format, int value,bool setNotDelete)
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

    int ControlSettingsTV::UpdatePQParamsToCache( const char *action, const char *tr181ParamName, const char *pqmode, const char *source, const char *format, tvPQParameterIndex_t pqParamIndex, int params[] )
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
                                    if(!GetLocalparam(tr181ParamName,format,mode,source,value))
                                        LOGINFO("Found param from tr181 %s pqmode : %d format:%d  source : %d value:%d\n",tr181ParamName,mode,format,source,value);
                                    else
                                        LOGINFO("Default1 value not found in tr181 %s pqmode : %d format:%d source : %d value:%d\n",tr181ParamName,mode,format,source,value);
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

    void ControlSettingsTV::GetColorTempStringFromEnum(int value, std::string &toStore)
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


}//namespace Plugin
}//namespace WPEFramework
