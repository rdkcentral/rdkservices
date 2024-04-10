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

static std::map<std::string, int> supportedSourcemap;
static std::map<std::string, int> supportedPictureModemap;
static std::map<std::string, int> supportedFormatmap;
static bool m_isDalsEnabled = false;

namespace WPEFramework {
namespace Plugin {

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

    int AVOutputTV::getPictureModeIndex(std::string pqparam)
    {
        int index = -1;
        std::map<std::string, int> :: iterator it;

        for(it = supportedPictureModemap.begin(); it != supportedPictureModemap.end(); it++) {
            if (it->first == pqparam) {
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

        for(it = supportedSourcemap.begin(); it != supportedSourcemap.end(); it++) {
            if (it->first == pqparam) {
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

        for(it =  supportedFormatmap.begin(); it !=  supportedFormatmap.end(); it++) {
            if (it->first == pqparam) {
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

        for(it =  supportedFormatmap.begin(); it !=  supportedFormatmap.end(); it++) {
            if (it->first == pqparam) {
                index = it->second;
                break;
            }
        }
        return index;
    }

    int AVOutputTV::getPqParamIndex()
    {
        std::vector<std::string> localpq;
        std::vector<std::string> localformat;
        std::vector<std::string> localsource;
        std::vector<std::string> localrange;
        std::string platformsupport;
        std::vector<std::string> index;

        tvError_t ret = getParamsCaps(localrange, localpq, localformat, localsource,"VideoSource", platformsupport, index);
        if (ret == tvERROR_NONE) {
            if (localrange.size() == index.size()) {
                for (unsigned int i = 0; i< localrange.size(); i++) {
                    supportedSourcemap[localrange[i]] = stoi(index[i]);
                }
            }
        }
        else {
            LOGERR("%s: Failed to fetch the source index \n", __FUNCTION__);
            return -1;
        }
        if (!localpq.empty()) {
	    localpq.clear();
	}
        if (!localformat.empty()) {
	    localformat.clear();
	}
        if (!localsource.empty()) {
	    localsource.clear();
	}
        if (!localrange.empty()) {
	    localrange.clear();
	}
        if(!index.empty()) {
	    index.clear();
	}

        ret = getParamsCaps(localrange, localpq, localformat, localsource,"PictureMode", platformsupport, index);
        if (ret == tvERROR_NONE) {
            if (localrange.size() == index.size()) {
                for (unsigned int i = 0; i< localrange.size(); i++) {
                    supportedPictureModemap[localrange[i]] = stoi(index[i]);
                }
            }
        }
        else {
            LOGERR("%s: Failed to fetch the picture index \n", __FUNCTION__);
            return -1;
        }

        if (!localpq.empty()) {
	    localpq.clear();
	}
        if (!localformat.empty()) {
	    localformat.clear();
	}
        if (!localsource.empty()) {
	    localsource.clear();
	}
        if (!localrange.empty()) {
	    localrange.clear();
	}
        if(!index.empty()) {
	    index.clear();
	}

        ret = getParamsCaps(localrange, localpq, localformat, localsource,"VideoFormat", platformsupport, index);
        if (ret == tvERROR_NONE) {
            if (localrange.size() == index.size()) {
                for (unsigned int i = 0; i< localrange.size(); i++) {
                    supportedFormatmap[localrange[i]] = stoi(index[i]);
                }
            }
        }
        else {
            LOGERR("%s: Failed to fetch the format index \n", __FUNCTION__);
            return -1;
        }

        return 0;
    }

    int AVOutputTV::getParamIndex(string source,string pqmode,string format,int& sourceIndex,int& pqmodeIndex,int& formatIndex)
    {
        LOGINFO("Entry : %s pqmode : %s source :%s format :%s\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str());

        if( source.compare("none") == 0 || source.compare("Current") == 0 ) {
            tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;
            GetCurrentSource(&currentSource);
            sourceIndex = (int)currentSource;
        }
        else {
            sourceIndex = getSourceIndex(source);
        }
        if( pqmode.compare("none") == 0 || pqmode.compare("Current") == 0) {
            char picMode[PIC_MODE_NAME_MAX]={0};
            if(!getCurrentPictureMode(picMode)) {
                LOGERR("Failed to get the Current picture mode\n");
            }
            else {
                std::string local = picMode;
                pqmodeIndex = getPictureModeIndex(local);
            }
        }
        else {
            pqmodeIndex = getPictureModeIndex(pqmode);
        }

        if( format.compare("none") == 0 || format.compare("Current") == 0) {
            tvVideoFormatType_t currentFormat = VIDEO_FORMAT_NONE;
            GetCurrentVideoFormat(&currentFormat);
            if( VIDEO_FORMAT_NONE == currentFormat ) {
                formatIndex = VIDEO_FORMAT_SDR;
            }
            else {
                formatIndex = (int)currentFormat;
	    }
        }
        else {
            formatIndex = getFormatIndex(format);
        }

        if (sourceIndex == -1 || pqmodeIndex == -1 || formatIndex == -1) {
	    return -1;
	}
        LOGINFO("%s: Exit sourceIndex = %d pqmodeIndex = %d formatIndex = %d\n",__FUNCTION__,sourceIndex,pqmodeIndex,formatIndex);

        return 0;
    }

    int AVOutputTV::GetDolbyModeIndex(const char * dolbyMode)
    {
        int mode = 0;
        pic_modes_t *dolbyModes     ;
        unsigned short totalAvailable = 0;

        tvError_t ret = GetTVSupportedDolbyVisionModesODM(&dolbyModes,&totalAvailable);
        if(ret == tvERROR_NONE) {
            for(int count = 0;count <totalAvailable;count++) {
                if(strncasecmp(dolbyMode, dolbyModes[count].name, strlen(dolbyMode))==0) {
                    mode = dolbyModes[count].value;
                    break;
                }

            }
        }
	else {
            mode = -1;
            printf("(%s):get supported mode is failed\n", __func__);
        }

        return mode;
    }

    tvDimmingMode_t AVOutputTV::getDimmingModeIndex(std::string mode)
    {
        tvDimmingMode_t index = tvDimmingMode_MAX;

        if(mode.compare("local") == 0 ) {
            index=tvDimmingMode_Local;
	}
        else if(mode.compare("fixed") == 0 ) {
            index=tvDimmingMode_Fixed;
	}
        else if(mode.compare("global") == 0 ) {
            index=tvDimmingMode_Global;
	}
        else {
            LOGINFO("Return Default Dimmingmode:%d!!!\n",index);
	}

        return index;
    }

    bool AVOutputTV::isIncluded(const std::set<string> set1,const std::set<string> set2)
    {
        for( const auto& element : set2) {
            if(set1.find(element) == set1.end()) {
                return false;
	    }
        }
        return true;
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
        if(!getCurrentPictureMode(picMode)) {
            LOGERR("Failed to get the current picture mode\n");
	}

        currentPicMode = picMode; //Convert to string

        //GetCurrentSource
        retVal = GetCurrentSource(&sourceIndex);
        if(retVal != tvERROR_NONE) {
             LOGERR("%s : GetCurrentSource( ) Failed\n",__FUNCTION__);
             return false;
        }
        currentSource = convertSourceIndexToString(sourceIndex);

        //GetCurrentFormat
        tvVideoFormatType_t formatIndex = VIDEO_FORMAT_NONE;
        GetCurrentVideoFormat(&formatIndex);
        if ( formatIndex  == VIDEO_FORMAT_NONE) {
	    formatIndex = VIDEO_FORMAT_SDR;
	}
        currentFormat = convertVideoFormatToString(formatIndex);


        if( ( (pqmode.find(currentPicMode) != std::string::npos) || (pqmode.compare("Global") == 0)  || (pqmode.compare("Current") == 0) ||
            (pqmode.compare("none") == 0) ) &&
           ((source.find(currentSource) != std::string::npos)  || (source.compare("Global") == 0) || (source.compare("Current") == 0) ||
            (source.compare("none") == 0) ) &&
           ( (format.find(currentFormat) !=  std::string::npos) || (format.compare("Global") == 0) || (format.compare("Current") == 0) ||
             (format.compare("none") == 0) ) ) {
            ret=true;
	}

        return ret;
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

        if (ret != tvERROR_NONE) {
           LOGINFO("%s: failed to get the capability \n", __FUNCTION__);
           return -1;
        }
        else {
            if(isPlatformSupport.compare("true") != 0) {
                LOGERR("%s: platform support not available\n", __FUNCTION__);
                return -1;
            }
        }
        return 0;
    }

    void AVOutputTV::spliltCapablities( std::vector<std::string> &range,std::vector<std::string> &pqmode,std::vector<std::string> &format, std::vector<std::string> &source, std::vector<std::string> &index, std::string rangeInfo, std::string pqmodeInfo, std::string formatInfo, std::string sourceInfo, std::string indexInfo)
    {
        std::string token;
        std::stringstream rangeStream(rangeInfo);
        std::stringstream pqmodeStream(pqmodeInfo);
        std::stringstream formatStream(formatInfo);
        std::stringstream sourceStream(sourceInfo);
        std::stringstream indexStream(indexInfo);

        while( getline(rangeStream,token,',')) {
            range.push_back(token );
            token.clear();
        }

        while( getline(pqmodeStream,token,',') ) {
            pqmode.push_back(token );
            token.clear();
        }

        while( getline(formatStream,token,',')) {
            format.push_back( token );
            token.clear();
        }

        while( getline(sourceStream,token,',') ) {
            source.push_back( token );
            token.clear();
        }

        while( getline(indexStream,token,',') ) {
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

        if( ReadCapablitiesFromConfODM( rangeCapInfo, pqmodeCapInfo, formatCapInfo, sourceCapInfo,param, isPlatformSupport, indexInfo) ) {
            LOGINFO( "%s: readCapablitiesFromConf Failed !!!\n",__FUNCTION__);
            return false;
        }

        //Compare capablityInfo with Input params

        //1.convertCapablity Info to set for comparison
        spliltStringsAndConvertToSet( pqmodeCapInfo, formatCapInfo, sourceCapInfo, pqmodeCapSet, formatCapSet, sourceCapset);

        //2.convert Application Input Info to set for comparison
        spliltStringsAndConvertToSet( pqmodeInputInfo, formatInputInfo, sourceInputInfo, pqmodeInputSet, formatInputSet, sourceInputSet );

        //3.Compare Each pqmode/format/source InputInfo against CapablityInfo
        if ( isIncluded(pqmodeCapSet,pqmodeInputSet) && isIncluded(formatCapSet,formatInputSet) && isIncluded(sourceCapset,sourceInputSet) ) {
            LOGINFO("%s : Capablity Chesk passed \n", __FUNCTION__);
            return true;
        }
        else {
            LOGERR("%s : Capablity Check Failed \n", __FUNCTION__);
            return false;
        }
    }

    int AVOutputTV::parsingSetInputArgument(const JsonObject& parameters, std::string pqparam, std::string & source, std::string & pqmode, std::string & format) {

        JsonArray sourceArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;


        pqmodeArray = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].Array() : JsonArray();
        for (int i = 0; i < pqmodeArray.Length(); ++i) {
            pqmode += pqmodeArray[i].String();
            if (i != (pqmodeArray.Length() - 1) ) {
	        pqmode += ",";
            }
        }

        sourceArray = parameters.HasLabel("videoSource") ? parameters["videoSource"].Array() : JsonArray();
        for (int i = 0; i < sourceArray.Length(); ++i) {
            source += sourceArray[i].String();
            if (i != (sourceArray.Length() - 1) ) {
                source += ",";
            }
        }

        formatArray = parameters.HasLabel("videoFormat") ? parameters["videoFormat"].Array() : JsonArray();
        for (int i = 0; i < formatArray.Length(); ++i) {
            format += formatArray[i].String();
            if (i != (formatArray.Length() - 1) ) {
                format += ",";
	    }
        }

        if (source.empty()) {
	    source = "Global";
	}
        if (pqmode.empty()) {
	    pqmode = "Global";
	}
        if (format.empty()) {
	    format = "Global";
	}

        if (convertToValidInputParameter(pqparam, source, pqmode, format) != 0) {
            LOGERR("%s: Failed to convert the input paramters. \n", __FUNCTION__);
            return -1;
        }

        return 0;
    }

    int AVOutputTV::parsingGetInputArgument(const JsonObject& parameters, std::string pqparam, std::string & source, std::string & pqmode, std::string & format) {
        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";

        source = parameters.HasLabel("videoSource") ? parameters["videoSource"].String() : "";

        format = parameters.HasLabel("videoFormat") ? parameters["videoFormat"].String() : "";

        if ( (source.compare("Global") == 0) || (pqmode.compare("Global") == 0) || (format.compare("Global") == 0) ) {
            LOGERR("%s: get cannot fetch the Global inputs \n", __FUNCTION__);
            return -1;
        }

        if (source.empty()) {
	    source = "Current";
	}
        if (pqmode.empty()) {
	    pqmode = "Current";
	}
        if (format.empty()) {
	    format = "Current";
	}

        if (convertToValidInputParameter(pqparam,source, pqmode, format) != 0) {
            LOGERR("%s: Failed to convert the input paramters. \n", __FUNCTION__);
            return -1;
        }

        return 0;
    }

    void AVOutputTV::spliltStringsAndConvertToSet( std::string pqmodeInfo,std::string formatInfo,std::string sourceInfo,std::set<string> &pqmode, std::set<string> &format, std::set<string> &source)
    {
        std::string token;
        std::stringstream pqmodeStream(pqmodeInfo);
        std::stringstream formatStream(formatInfo);
        std::stringstream sourceStream(sourceInfo);

        while( getline(pqmodeStream,token,',') ) {
            pqmode.insert( token );
            token.clear();
        }

        while( getline(formatStream,token,',') ) {
            format.insert( token );
            token.clear();
        }

        while( getline(sourceStream,token,',')) {
            source.insert( token );
            token.clear();
        }
    }

    int AVOutputTV::validateIntegerInputParameter(std::string param, int inputValue)
    {

        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        tvError_t ret = getParamsCaps(range, pqmode, source, format, param);

        if (ret != tvERROR_NONE) {
            LOGERR("Failed to fetch the range capability[%s] \n", param.c_str());
            return -1;
        }

        if ( (param == "Brightness") || (param == "Contrast") ||
             (param == "Sharpness") || (param == "Saturation") ||
             (param == "Hue") || (param == "WhiteBalance") ||
            (param == "ComponentSaturation") || (param == "Backlight") ||
            (param == "ComponentHue") || (param == "ComponentLuma") || (param == "LowLatencyState") ) {
                if (inputValue < stoi(range[0]) || inputValue > stoi(range[1])) {
                    LOGERR("wrong Input value[%d]", inputValue);
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

    int AVOutputTV::validateInputParameter(std::string param, std::string inputValue)
    {

        std::vector<std::string> range;
        std::vector<std::string> pqmode;
        std::vector<std::string> source;
        std::vector<std::string> format;

        tvError_t ret = getParamsCaps(range, pqmode, source, format, param);

        if (ret != tvERROR_NONE) {
            LOGERR("Failed to fetch the range capability[%s] \n", param.c_str());
            return -1;
        }

        if ( (param == "ColorTemperature") ||
             (param == "DimmingMode") || (param == "AutoBacklightControl") ||
             (param == "DolbyVisionMode") || (param == "HDR10Mode") ||
            (param == "HLGMode") || (param == "AspectRatio") || (param == "PictureMode") ) {
            auto iter = find(range.begin(), range.end(), inputValue);

            if (iter == range.end()) {
                LOGERR("Not a valid input value[%s].\n", inputValue.c_str());
                return -1;
            }
        }
        return 0;
    }

    void AVOutputTV::LocatePQSettingsFile()
    {
        LOGINFO("Entry\n");
        char panelId[20] = {0};
        std::string PQFileName = AVOUTPUT_RFC_CALLERID;
        std::string FilePath = "/etc/rfcdefaults/";

        /* The if condition is to override the tvsettings ini file so it helps the PQ tuning process for new panels */
        if(access(AVOUTPUT_OVERRIDE_PATH, F_OK) == 0) {
            PQFileName = std::string(AVOUTPUT_RFC_CALLERID_OVERRIDE);
        }
        else {
            int val=GetPanelIDODM(panelId);
            if(val==0) {
                LOGINFO("%s : panel id read is : %s\n",__FUNCTION__,panelId);
                if(strncmp(panelId,AVOUTPUT_CONVERTERBOARD_PANELID,strlen(AVOUTPUT_CONVERTERBOARD_PANELID))!=0) {
                    PQFileName+=std::string("_")+panelId;
                    struct stat tmp_st;

                    LOGINFO("%s: Looking for %s.ini \n",__FUNCTION__,PQFileName.c_str());
                    if(stat((FilePath+PQFileName+std::string(".ini")).c_str(), &tmp_st)!=0) {
                        //fall back
                        LOGINFO("%s not available in %s Fall back to default\n",PQFileName.c_str(),FilePath.c_str());
                        PQFileName =std::string(AVOUTPUT_RFC_CALLERID);
                    }
                }
            }
            else {
                LOGINFO("%s : GetPanelID failed : %d\n",__FUNCTION__,val);
            }
        }
        strncpy(rfc_caller_id,PQFileName.c_str(),PQFileName.size());
        LOGINFO("%s : Default tvsettings file : %s\n",__FUNCTION__,rfc_caller_id);
    }

    tvError_t AVOutputTV::InitializePictureMode()
    {
        tvError_t ret = tvERROR_NONE;
        TR181_ParamData_t param;
        tvVideoSrcType_t current_source = VIDEO_SOURCE_IP;
        std::string tr181_param_name = "";
        tvVideoFormatType_t current_format = VIDEO_FORMAT_NONE;

        GetCurrentVideoFormat(&current_format);
        if ( current_format  == VIDEO_FORMAT_NONE) {
	    current_format  = VIDEO_FORMAT_SDR;
	}
        // get current source
        GetCurrentSource(&current_source);

        tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
        tr181_param_name += "."+convertSourceIndexToString(current_source)+"."+"Format."+convertVideoFormatToString(current_format)+"."+"PictureModeString";

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
        if ( tr181Success == err ) {
            std::string local = param.value;
            transform(local.begin(), local.end(), local.begin(), ::tolower);
            ret = SetTVPictureMode(local.c_str());

            if(ret != tvERROR_NONE) {
                LOGWARN("Picture Mode set failed: %s\n",getErrorString(ret).c_str());
            }
            else {
                LOGINFO("Picture Mode initialized successfully, tr181 value [%s] value: %s\n", tr181_param_name.c_str(), param.value);
            }
       }
       else {
           ret = tvERROR_GENERAL;
           LOGWARN("getLocalParam for %s Failed : %s\n", tr181_param_name.c_str(), getTR181ErrorString(err));
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
        if (pqmode == "Global") {
            std::string localSource;
            std::string localPqmode;
            std::string localFormat;
            if (FetchCapablities(pqparam, localSource, localPqmode, localFormat) == 0) {
                pqmode = localPqmode;
                //if pqmode none from capabilty then lets keep pqmode as global to fail the capabilty
            }
            else {
                LOGINFO("%s, Failed to get picturemode capability\n", __FUNCTION__);
                return -1;
            }
        }
        else if (pqmode == "Current") {
            char picMode[PIC_MODE_NAME_MAX]={0};
            if(!getCurrentPictureMode(picMode)) {
                LOGINFO("Failed to get the Current picture mode\n");
                return -1;
            }
            else {
                pqmode = picMode;
            }
        }

        if (source == "Global") {
            std::string localSource;
            std::string localPqmode;
            std::string localFormat;
            if (FetchCapablities(pqparam, localSource, localPqmode, localFormat) == 0) {
                source = localSource;
            }
            else {
                LOGINFO("%s, Failed to get source capability\n", __FUNCTION__);
                return -1;
            }
        }
        else if (source == "Current") {
            tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;
            tvError_t ret = GetCurrentSource(&currentSource);

            if(ret != tvERROR_NONE) {
                LOGWARN("%s: GetCurrentSource( ) Failed \n",__FUNCTION__);
                return -1;
            }
            source = convertSourceIndexToString(currentSource);
        }

        //convert format into valid parameter
        if (format == "Global") {
            std::string localSource;
            std::string localPqmode;
            std::string localFormat;
            if (FetchCapablities(pqparam, localSource, localPqmode, localFormat) == 0) {
                format = localFormat;
            }
            else {
                LOGINFO("%s, Failed to get format capability\n", __FUNCTION__);
                return -1;
            }
        }
        else if (format == "Current") {
            tvVideoFormatType_t formatIndex = VIDEO_FORMAT_NONE;
            GetCurrentVideoFormat(&formatIndex);
            if ( formatIndex  == VIDEO_FORMAT_NONE) {
	        formatIndex  = VIDEO_FORMAT_SDR;
            }
            format = convertVideoFormatToString(formatIndex);
        }

        LOGINFO("Exit %s source %s pqmode %s format %s \n", __FUNCTION__, source.c_str(), pqmode.c_str(), format.c_str());
        return 0;
    }

    string AVOutputTV::convertSourceIndexToString(int source)
    {
        std::string ret;
        std::map<std::string, int> :: iterator it;
        for (it = supportedSourcemap.begin(); it != supportedSourcemap.end(); it++) {
            if (it->second == source) {
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
        for (it = supportedFormatmap.begin(); it != supportedFormatmap.end(); it++) {
            if (it->second == format) {
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
        for(it = supportedPictureModemap.begin(); it != supportedPictureModemap.end(); it++) {
            if (it->second == pqmode) {
                ret = it->first;
                break;
            }
        }
        return ret;
    }

    tvContentFormatType_t AVOutputTV::ConvertFormatStringToTVContentFormat(const char *format)
    {
        tvContentFormatType_t ret = tvContentFormatType_SDR;

        if( strncmp(format,"sdr",strlen(format)) == 0 || strncmp(format,"SDR",strlen(format)) == 0 ) {
            ret = tvContentFormatType_SDR;
	}
        else if( strncmp(format,"hdr10",strlen(format)) == 0 || strncmp(format,"HDR10",strlen(format))==0 ) {
            ret = tvContentFormatType_HDR10;
	}
        else if( strncmp(format,"hlg",strlen(format)) == 0 || strncmp(format,"HLG",strlen(format)) == 0 ) {
            ret = tvContentFormatType_HLG;
	}
        else if( strncmp(format,"dolby",strlen(format)) == 0 || strncmp(format,"DOLBY",strlen(format)) == 0 ) {
            ret=tvContentFormatType_DOVI;
	}

        return ret;
    }

    tvError_t AVOutputTV::UpdateAVoutputTVParamToHAL(std::string forParam, int source, int pqmode, int format, int value,bool setNotDelete)
    {
        tvError_t ret = tvERROR_NONE;
        std::string key;

        generateStorageIdentifier(key,forParam,format,pqmode,source);
        if(key.empty()) {
            LOGERR("generateStorageIdentifierDirty failed\n");
            ret = tvERROR_GENERAL;
        }

        else {
            tr181ErrorCode_t err  = tr181Success;
            if(setNotDelete) {
                std::string toStore = std::to_string(value);
                if (forParam.compare("ColorTemp") == 0) {
                    getColorTempStringFromEnum(value, toStore);
                }
                else if(forParam.compare("DimmingMode") == 0 ) {
                    getDimmingModeStringFromEnum(value, toStore);
                }
                else if (forParam.compare("DolbyVisionMode") == 0 ) {
                    toStore = getDolbyModeStringFromEnum((tvDolbyMode_t)value);
                }
                err = setLocalParam(rfc_caller_id, key.c_str(),toStore.c_str());

            }
            else {
                err = clearLocalParam(rfc_caller_id, key.c_str());
            }

            if ( err != tr181Success ) {
                LOGERR("%s for %s Failed : %s\n", setNotDelete?"Set":"Delete", key.c_str(), getTR181ErrorString(err));
                ret = tvERROR_GENERAL;
            }
        }
        return ret;
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
        if( 0 == ret ) {
            for(int sourceType: sources) {
                tvVideoSrcType_t source = (tvVideoSrcType_t)sourceType;
                for(int mode : pictureModes) {
                    for(int formatType : formats) {
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
                                if(reset) {
                                    ret |= UpdateAVoutputTVParamToHAL(tr181ParamName,source, mode, format,0,false);
				}
                                if(sync || reset) {
                                    int value=0;
                                    if(getLocalparam(tr181ParamName,format,mode,source,value,pqParamIndex,sync)) {
					continue;
			            }
                                    params[0]=value;
                                }
                                if(set) {
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
                                 if(sync) {
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

    tvError_t AVOutputTV::SyncAvoutputTVParamsToHAL(std::string pqmode,std::string source,std::string format)
    {
        int params[3]={0};

        LOGINFO("Entry %s : pqmode : %s source : %s format : %s\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str());

        if( !UpdateAVoutputTVParam("sync","Brightness",pqmode,source,format,PQ_PARAM_BRIGHTNESS,params)) {
            LOGINFO("Brightness Successfully sync to Drive Cache\n");
	}
        else {
            LOGERR("Brightness Sync to cache Failed !!!\n");
	}
        if( !UpdateAVoutputTVParam("sync","Contrast",pqmode,source,format,PQ_PARAM_CONTRAST,params)) {
            LOGINFO("Contrast Successfully Synced to Drive Cache\n");
	}
        else {
            LOGERR("Contrast Sync to cache Failed !!!\n");
        }
        if(!UpdateAVoutputTVParam("sync","Sharpness",pqmode,source,format,PQ_PARAM_SHARPNESS,params)) {
            LOGINFO("Sharpness Successfully Synced to Drive Cache\n");
	}
        else {
            LOGERR("Sharpness Sync to cache Failed !!!\n");
        }
        if(!UpdateAVoutputTVParam("sync","Saturation",pqmode,source,format,PQ_PARAM_SATURATION,params)) {
            LOGINFO("Saturation Successfully Synced to Drive Cache\n");
	}
        else {
            LOGERR("Saturation Sync to cache Failed !!!\n");
        }
        if(!UpdateAVoutputTVParam("sync","Hue",pqmode,source,format,PQ_PARAM_HUE,params)) {
            LOGINFO("Hue Successfully Synced to Drive Cache\n");
	}
        else {
            LOGERR("Hue Sync to cache Failed !!!\n");
        }
        if(!UpdateAVoutputTVParam("sync","ColorTemp",pqmode,source,format,PQ_PARAM_COLOR_TEMPERATURE,params)) {
            LOGINFO("ColorTemp Successfully Synced to Drive Cache\n");
	}
        else {
            LOGERR("ColorTemp Sync to cache Failed !!!\n");
        }
        if(!UpdateAVoutputTVParam("sync","DolbyVisionMode",pqmode,source,"DV",PQ_PARAM_DOLBY_MODE,params)) {
            LOGINFO("dvmode Successfully Synced to Drive Cache\n");
	}
        else {
            LOGERR("dvmode Sync to cache Failed !!!\n");
	}
        if(!UpdateAVoutputTVParam("sync","DimmingMode",pqmode,source,format,PQ_PARAM_DIMMINGMODE,params)) {
            LOGINFO("dimmingmode Successfully Synced to Drive Cache\n");
	}
        else {
            LOGERR("dimmingmode Sync to cache Failed !!!\n");
        }
        if(!UpdateAVoutputTVParam("sync","Backlight",pqmode,source,format,PQ_PARAM_BACKLIGHT,params) ) {
            LOGINFO("Backlight Successfully Synced to Drive Cache\n");
	}
        else {
            LOGERR("Backlight Sync to cache Failed !!!\n");
        }
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

        if (ret == 0 ) {
            for (int source : sources) {
                tvVideoSrcType_t sourceType = (tvVideoSrcType_t)source;
                for (int format : formats) {
                    tvVideoFormatType_t formatType = (tvVideoFormatType_t)format;
                    std::string tr181_param_name = "";
                    tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
                    tr181_param_name += "."+convertSourceIndexToString(sourceType)+"."+"Format."+
                                         convertVideoFormatToString(formatType)+"."+"PictureModeString";

                    err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
                    if ( tr181Success == err ) {
                        std::string local = param.value;
                        int pqmodeindex = (int)getPictureModeIndex(local);

                        tvError_t tv_err = SaveSourcePictureMode(sourceType, formatType, pqmodeindex);
                        if (tv_err != tvERROR_NONE) {
                            LOGWARN("failed to SaveSourcePictureMode \n");
                            return -1;
                        }
                    }
                    else {
                        LOGWARN("Failed to get the getLocalParam \n");
                        return -1;
                    }
                }
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

    int AVOutputTV::getSaveConfig(std::string pqmode, std::string source, std::string format,std::vector<int> &sources,std::vector<int> &picturemodes, std::vector<int> &formats)
    {
        LOGINFO("Entry : %s pqmode : %s source :%s format :%s\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str());

        int ret = 0;

        if (getAvailableCapabilityModes(source, pqmode, format) != 0) {
            LOGERR("%s: failed to get picture/source/format mode capability \n", __FUNCTION__);
            return -1;
        }
        //pqmode
        char *modeString = strdup(pqmode.c_str());
        char *token = NULL;
        while ((token = strtok_r(modeString,",",&modeString))) {
            std::string local = token;
            picturemodes.push_back(getPictureModeIndex(local));
        }
        //source
        char *sourceString = strdup(source.c_str());
        char *sourceToken = NULL;
        while ((sourceToken = strtok_r(sourceString,",",&sourceString))) {
            std::string local = sourceToken;
            sources.push_back(getSourceIndex(local));
        }
        //3)check format
        char *formatString = strdup(format.c_str());
        char *formatToken = NULL;
        while ((formatToken = strtok_r(formatString,",",&formatString))) {
            std::string local = formatToken;
            formats.push_back(getFormatIndex(local));
        }

        LOGINFO("Exit : %s pqmode : %s source :%s format :%s ret:%d\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str(), ret);
        return ret;
    }

    int AVOutputTV::getLocalparam( std::string forParam,int formatIndex,int pqIndex,int sourceIndex,int & value, tvPQParameterIndex_t pqParamIndex,bool sync,int color )
    {
        string key;
        TR181_ParamData_t param={0};
        generateStorageIdentifier(key,forParam,formatIndex,pqIndex,sourceIndex);
        if(key.empty()) {
            LOGERR("generateStorageIdentifier failed\n");
            return -1;
        }

       tr181ErrorCode_t err=getLocalParam(rfc_caller_id, key.c_str(), &param);

       if ( tr181Success == err ) {//Fetch new tr181format values
           if( forParam.compare("ColorTemp") == 0 ) {
               if (strncmp(param.value, "Standard", strlen(param.value))==0) {
                   value=tvColorTemp_STANDARD;
	       }
               else if (strncmp(param.value, "Warm", strlen(param.value))==0) {
                   value=tvColorTemp_WARM;
	       }
               else if (strncmp(param.value, "Cold", strlen(param.value))==0) {
                   value=tvColorTemp_COLD;
	       }
               else if (strncmp(param.value, "User Defined", strlen(param.value))==0) {
                   value=tvColorTemp_USER;
	       }
               else {
                   value=tvColorTemp_STANDARD;
	       }
               return 0;
           }
           else if( forParam.compare("DimmingMode") == 0 ) {
               if (strncmp(param.value, "fixed", strlen(param.value))==0) {
                   value=tvDimmingMode_Fixed;
	       }
               else if (strncmp(param.value, "local", strlen(param.value))==0) {
                   value=tvDimmingMode_Local;
	       }
               else if (strncmp(param.value, "global", strlen(param.value))==0) {
                   value=tvDimmingMode_Global;
	       }
               return 0;
           }
           else if ( forParam.compare("DolbyVisionMode") == 0) {
               if (strncmp(param.value, "Dark", strlen(param.value)) == 0) {
                   value = tvDolbyMode_Dark;
               }
               else {
                   value = tvDolbyMode_Bright;
               }
               return 0;
           }
           else {
               value=std::stoi(param.value);
               return 0;
           }
        }
        else {// default value from DB
            if( sync ) {
                return 1;
            }
            GetDefaultPQParams(pqIndex,(tvVideoSrcType_t)sourceIndex,(tvVideoFormatType_t)formatIndex,pqParamIndex,&value);
            LOGINFO("Default value from DB : %s : %d \n",key.c_str(),value);
            return 0;
        }
    }

    tvDataComponentColor_t AVOutputTV::getComponentColorEnum(std::string colorName)
    {
        tvDataComponentColor_t CompColorEnum = tvDataColor_MAX;

        if(!colorName.compare("none")) {
            CompColorEnum = tvDataColor_NONE;
        }
        else if (!colorName.compare("red")) {
            CompColorEnum = tvDataColor_RED;
        }
        else if (!colorName.compare("green")) {
            CompColorEnum = tvDataColor_GREEN;
        }
        else if (!colorName.compare("blue")) {
            CompColorEnum = tvDataColor_BLUE;
        }
        else if (!colorName.compare("yellow")) {
            CompColorEnum = tvDataColor_YELLOW;
        }
        else if (!colorName.compare("cyan")) {
            CompColorEnum = tvDataColor_CYAN;
        }
        else if (!colorName.compare("magenta")) {
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
        if (!source.empty()) {
            sourceIndex = (tvVideoSrcType_t)getSourceIndex(source);
        }
        else {
            GetCurrentSource(&sourceIndex);
        }

        char picMode[PIC_MODE_NAME_MAX]={0};
        int pqmodeIndex = 0;
        if(!getCurrentPictureMode(picMode)) {
            LOGERR("Failed to get the Current picture mode\n");
        }
        else {
            std::string local = picMode;
            pqmodeIndex = getPictureModeIndex(local);
        }
        memset(&param, 0, sizeof(param));
        if (format == tvContentFormatType_HLG ) {
            rfc_param = AVOUTPUT_HLGMODE_RFC_PARAM;
        }
        else if (format == tvContentFormatType_DOVI) {
            rfc_param = AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM + std::to_string(sourceIndex) + "."+"DolbyVisionMode";
        }

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, rfc_param.c_str(), &param);
        if ( tr181Success != err) {
            tvError_t retVal = GetDefaultPQParams(pqmodeIndex,(tvVideoSrcType_t)sourceIndex, (tvVideoFormatType_t)ConvertHDRFormatToContentFormatODM((tvhdr_type_t)format), PQ_PARAM_DOLBY_MODE,&dolby_mode_value);
            if( retVal != tvERROR_NONE ) {
                LOGERR("%s : failed\n",__FUNCTION__);
                return ret;
            }
            s = getDolbyModeStringFromEnum((tvDolbyMode_t)dolby_mode_value);
            ret = 0;
        }
        else {
            s += param.value;
            ret = 0;
        }
        return ret;
    }

    tvError_t AVOutputTV::getParamsCaps(std::vector<std::string> &range, std::vector<std::string> &pqmode, std::vector<std::string> &source, std::vector<std::string> &format,std::string param )
    {
        tvError_t ret = tvERROR_NONE;

        std::string rangeInfo;
        std::string sourceInfo;
        std::string formatInfo;
        std::string pqmodeInfo;

        std::string platformsupport;
        std::string indexInfo;
        std::vector<std::string> localIndex;

        if( ReadCapablitiesFromConfODM( rangeInfo, pqmodeInfo, formatInfo ,sourceInfo,param, platformsupport, indexInfo)) {
            LOGERR( "%s: ReadCapablitiesFromConf Failed !!!\n",__FUNCTION__);
            return tvERROR_GENERAL;
        }
        else {
            spliltCapablities( range, pqmode, format, source, localIndex,rangeInfo, pqmodeInfo, formatInfo, sourceInfo , indexInfo);
        }

        return ret;
    }

    tvError_t AVOutputTV::getParamsCaps(std::vector<std::string> &range, std::vector<std::string> &pqmode, std::vector<std::string> &source, std::vector<std::string> &format,std::string param, std::string & isPlatformSupport, std::vector<std::string> & index)
    {
        tvError_t ret = tvERROR_NONE;

        std::string rangeInfo;
        std::string sourceInfo;
        std::string formatInfo;
        std::string pqmodeInfo;
        std::string indexInfo;

        if( ReadCapablitiesFromConfODM( rangeInfo, pqmodeInfo, formatInfo ,sourceInfo,param, isPlatformSupport, indexInfo)) {
            LOGERR( "%s: ReadCapablitiesFromConf Failed !!!\n",__FUNCTION__);
            return tvERROR_GENERAL;
        }
        else {
            spliltCapablities( range, pqmode, format, source, index,rangeInfo, pqmodeInfo, formatInfo, sourceInfo, indexInfo);
        }

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

    int AVOutputTV::getCurrentPictureMode(char *picMode)
    {
        tvError_t  ret = tvERROR_NONE;
        TR181_ParamData_t param;
        std::string tr181_param_name;
        tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;

        ret = GetCurrentSource(&currentSource);
        if(ret != tvERROR_NONE) {
            LOGERR("GetCurrentSource() Failed set source to default\n");
            return 0;
        }

        tvVideoFormatType_t current_format = VIDEO_FORMAT_NONE;
        GetCurrentVideoFormat(&current_format);
        if ( current_format  == VIDEO_FORMAT_NONE) {
	    current_format  = VIDEO_FORMAT_SDR;
        }
        tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
        tr181_param_name += "." + convertSourceIndexToString(currentSource) + "." + "Format."+convertVideoFormatToString(current_format)+"."+"PictureModeString";

        memset(&param, 0, sizeof(param));

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
        if ( err == tr181Success ) {
            strncpy(picMode, param.value, strlen(param.value)+1);
            LOGINFO("getLocalParam success, mode = %s\n", picMode);
            return 1;
        }
        else {
            LOGERR("getLocalParam failed");
            return 0;
        }
    }

    int AVOutputTV::getDolbyParamToSync(int sourceIndex, int formatIndex, int& value)
    {
        int ret=0;
        TR181_ParamData_t param;
        int pqmodeIndex = 0;
        char picMode[PIC_MODE_NAME_MAX]={0};
        if(!getCurrentPictureMode(picMode)) {
            LOGERR("Failed to get the Current picture mode\n");
        }
        else {
            std::string local = picMode;
            pqmodeIndex = getPictureModeIndex(local);
        }
        std ::string rfc_param = AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM + std::to_string(sourceIndex) + "."+"DolbyVisionMode";
        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, rfc_param.c_str(), &param);

        if ( tr181Success != err) {
            tvError_t retVal = GetDefaultPQParams(pqmodeIndex,(tvVideoSrcType_t)sourceIndex, (tvVideoFormatType_t)formatIndex,
                                                PQ_PARAM_DOLBY_MODE, &value);
            if( retVal != tvERROR_NONE ) {
                LOGERR("%s : failed\n",__FUNCTION__);
                return -1;
            }
            ret = 0;
        }
        else {
            value=GetDolbyModeIndex(param.value);
            ret = 0;
        }

        return ret;
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

    int AVOutputTV::getAvailableCapabilityModesWrapper(std::string param, std::string & outparam)
    {
        tvError_t err = tvERROR_NONE;
        std::vector<std::string> range;
        std::vector<std::string> picmodeVec;
        std::vector<std::string> sourceVec;
        std::vector<std::string> formatVec;

         err = getParamsCaps(range,picmodeVec,sourceVec,formatVec, param);
         if (err != tvERROR_NONE) {
             LOGERR("%s: failed to get [%s] capability \n", __FUNCTION__, param.c_str());
             return -1;
         }
         outparam = convertToString(range);

         return 0;
    }

    int AVOutputTV::getAvailableCapabilityModes(std::string & source, std::string & pqmode, std::string & format)
    {
        if ((pqmode.compare("none") == 0 )) {
            if (getAvailableCapabilityModesWrapper("PictureMode", pqmode) != 0) {
                LOGERR("%s: failed to get picture mode capability \n", __FUNCTION__);
                return -1;
            }
        }

        if( (source.compare("none") == 0)) {
            if (getAvailableCapabilityModesWrapper("VideoSource",source) != 0) {
                LOGERR("%s: failed to get source capability \n", __FUNCTION__);
                return -1;
            }
        }

        if( (format.compare("none") == 0) ) {
            if (getAvailableCapabilityModesWrapper("VideoFormat",format) != 0) {
                LOGERR("%s: failed to get format capability \n", __FUNCTION__);
                return -1;
            }
        }
        return 0;
    }

    int AVOutputTV::getCapabilitySource(JsonArray & rangeArray)
    {
        std::vector<string> range,pqmode,source,format;

        tvError_t ret = getParamsCaps(range,pqmode,source,format,"VideoSource");

        if(ret != tvERROR_NONE) {
            return -1;
        }
        else {
            if ((range.front()).compare("none") != 0) {
                for (unsigned int index = 0; index < range.size(); index++) {
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

        if(ret != tvERROR_NONE) {
            return -1;
        }
        else {
            if ((range.front()).compare("none") != 0) {
                rangeInfo = range;
            }
        }
        return 0;
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

    tvError_t AVOutputTV::getUserSelectedAspectRatio (tvDisplayMode_t* mode)
    {
        tvError_t ret = tvERROR_GENERAL;
#if !defined (HDMIIN_4K_ZOOM)
        LOGERR("%s:mode selected is: %d", __FUNCTION__, m_videoZoomMode);
        if (AVOutputTV::instance->m_isDisabledHdmiIn4KZoom) {
            if (!(AVOutputTV::instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
               (dsVIDEO_PIXELRES_MAX == AVOutputTV::instance->m_currentHdmiInResoluton))) {
                *mode = (tvDisplayMode_t)AVOutputTV::instance->m_videoZoomMode;
                LOGWARN("%s: Getting zoom mode %d for display, for 4K and above", __FUNCTION__, *mode);
                return tvERROR_NONE;
            }
        }
#endif
        ret = GetAspectRatio(mode);
        return ret;
    }

    void AVOutputTV::BroadcastLowLatencyModeChangeEvent(bool lowLatencyMode)
    {
           LOGINFO("Entry:%d\n",lowLatencyMode);
           JsonObject response;
           response["lowLatencyMode"] = lowLatencyMode;
           sendNotify("gameModeEvent", response);
    }

    tvError_t AVOutputTV::setAspectRatioZoomSettings(tvDisplayMode_t mode)
    {
        tvError_t ret = tvERROR_GENERAL;
        LOGERR("%s: mode selected is: %d", __FUNCTION__, m_videoZoomMode);
#if !defined (HDMIIN_4K_ZOOM)
        if (AVOutputTV::instance->m_isDisabledHdmiIn4KZoom) {
            if (AVOutputTV::instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
                (dsVIDEO_PIXELRES_MAX == m_currentHdmiInResoluton)) {
                LOGWARN("%s: Setting %d zoom mode for below 4K", __FUNCTION__, m_videoZoomMode);
#endif
                ret = SetAspectRatio(mode);
#if !defined (HDMIIN_4K_ZOOM)
            }
            else {
                LOGWARN("%s: Setting auto zoom mode for 4K and above", __FUNCTION__);
                ret = SetAspectRatio(tvDisplayMode_AUTO);
            }
        }
        else {
            LOGWARN("%s: HdmiInput is not started yet. m_isDisabledHdmiIn4KZoom: %d", __FUNCTION__, AVOutputTV::instance->m_isDisabledHdmiIn4KZoom);
            ret = SetAspectRatio((tvDisplayMode_t)m_videoZoomMode);
        }
#endif
        return ret;
    }

    tvError_t AVOutputTV::setDefaultAspectRatio(std::string pqmode,std::string  format,std::string source)
    {
        tvDisplayMode_t mode = tvDisplayMode_MAX;
        TR181_ParamData_t param;
        tvError_t ret = tvERROR_NONE;

        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, AVOUTPUT_ASPECTRATIO_RFC_PARAM, &param);
        if ( tr181Success == err ) {
            if(!std::string(param.value).compare("16:9")) {
                mode = tvDisplayMode_16x9;
            }
            else if (!std::string(param.value).compare("4:3")) {
                mode = tvDisplayMode_4x3;
            }
            else if (!std::string(param.value).compare("Full")) {
                mode = tvDisplayMode_FULL;
            }
            else if (!std::string(param.value).compare("Normal")) {
                mode = tvDisplayMode_NORMAL;
            }
            else if (!std::string(param.value).compare("TV AUTO")) {
                mode = tvDisplayMode_AUTO;
            }
            else if (!std::string(param.value).compare("TV DIRECT")) {
                mode = tvDisplayMode_DIRECT;
            }
            else if (!std::string(param.value).compare("TV NORMAL")) {
                mode = tvDisplayMode_NORMAL;
            }
            else if (!std::string(param.value).compare("TV ZOOM")) {
                mode = tvDisplayMode_ZOOM;
            }
            else if (!std::string(param.value).compare("TV 16X9 STRETCH")) {
                mode = tvDisplayMode_16x9;
            }
            else if (!std::string(param.value).compare("TV 4X3 PILLARBOX")) {
                mode = tvDisplayMode_4x3;
            }
            else {
                mode = tvDisplayMode_AUTO;
            }

            m_videoZoomMode = mode;
            tvError_t ret = setAspectRatioZoomSettings (mode);

            if(ret != tvERROR_NONE) {
                LOGERR("AspectRatio  set failed: %s\n",getErrorString(ret).c_str());
            }
            else {
                //Save DisplayMode to ssm_data
                int params[3]={0};
                params[0]=mode;
                int retval=UpdateAVoutputTVParam("set","AspectRatio",pqmode,source,format,PQ_PARAM_ASPECT_RATIO,params);

                if(retval != 0) {
                    LOGERR("Failed to Save DisplayMode to ssm_data\n");
                    ret = tvERROR_GENERAL;
                }
                LOGINFO("Aspect Ratio initialized successfully, value: %s\n", param.value);
            }

        }
        else {
            LOGERR("getLocalParam for %s Failed : %s\n", AVOUTPUT_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            ret = tvERROR_GENERAL;
        }
        return ret;
    }
}//namespace Plugin
}//namespace WPEFramework

