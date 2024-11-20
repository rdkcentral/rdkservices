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

#define CAPABLITY_FILE_NAME    "pq_capabilities.ini"

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
        switch(formatToConvert) {
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

    int AVOutputTV::getPqParamIndex()
    {
     
        capVectors_t info;

        tvError_t ret = getParamsCaps("VideoSource", info);
        if (ret == tvERROR_NONE) {
            if (info.rangeVector.size() == info.indexVector.size()) {
                for (unsigned int i = 0; i< info.rangeVector.size(); i++) {
                    supportedSourcemap[info.rangeVector[i]] = stoi(info.indexVector[i]);
                }
            }
        }
        else {
            LOGERR("%s: Failed to fetch the source index \n", __FUNCTION__);
            return -1;
        }
      
        info.pqmodeVector.clear();
        info.sourceVector.clear();
        info.formatVector.clear();
	info.indexVector.clear();
	info.rangeVector.clear();

        ret = getParamsCaps("PictureMode", info);
        if (ret == tvERROR_NONE) {
            if (info.rangeVector.size() == info.indexVector.size()) {
                for (unsigned int i = 0; i< info.rangeVector.size(); i++) {
                    supportedPictureModemap[info.rangeVector[i]] = stoi(info.indexVector[i]);
                }
            }
        }
        else {
            LOGERR("%s: Failed to fetch the picture index \n", __FUNCTION__);
            return -1;
        }
        
        info.pqmodeVector.clear();
        info.sourceVector.clear();
        info.formatVector.clear();
	info.indexVector.clear();
        info.rangeVector.clear();

        ret = getParamsCaps( "VideoFormat", info);
        if (ret == tvERROR_NONE) {
            if ( info.rangeVector.size() == info.indexVector.size()) {
                for (unsigned int i = 0; i< info.rangeVector.size(); i++) {
                    supportedFormatmap[info.rangeVector[i]] = stoi(info.indexVector[i]);
                }
            }
        }
        else {
            LOGERR("%s: Failed to fetch the format index \n", __FUNCTION__);
            return -1;
        }

        return 0;
    }

    int AVOutputTV::getParamIndex(std::string param, capDetails_t& paramInfo, paramIndex_t& indexInfo)
    {
        LOGINFO("Entry : %s  param : %s pqmode : %s source :%s format :%s\n",__FUNCTION__,param.c_str(),paramInfo.pqmode.c_str(),paramInfo.source.c_str(),paramInfo.format.c_str());

        if( paramInfo.source.compare("none") == 0 || paramInfo.source.compare("Current") == 0 ) {
            tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;
            GetCurrentSource(&currentSource);
            indexInfo.sourceIndex = (int)currentSource;
        }
        else {
            indexInfo.sourceIndex = getSourceIndex(paramInfo.source);
        }
        if( paramInfo.pqmode.compare("none") == 0 || paramInfo.pqmode.compare("Current") == 0) {
            char picMode[PIC_MODE_NAME_MAX]={0};
            if(!getCurrentPictureMode(picMode)) {
                LOGERR("Failed to get the Current picture mode\n");
            }
            else {
                std::string local = picMode;
                indexInfo.pqmodeIndex = getPictureModeIndex(local);
            }
        }
        else {
            indexInfo.pqmodeIndex = getPictureModeIndex(paramInfo.pqmode);
        }

        if( paramInfo.format.compare("none") == 0 || paramInfo.format.compare("Current") == 0) {
            tvVideoFormatType_t currentFormat = VIDEO_FORMAT_NONE;
            GetCurrentVideoFormat(&currentFormat);
            if( VIDEO_FORMAT_NONE == currentFormat ) {
		        indexInfo.formatIndex = VIDEO_FORMAT_SDR;
            }
            else {
                indexInfo.formatIndex = (int)currentFormat;
	    }
        }
        else {
            indexInfo.formatIndex = getFormatIndex(paramInfo.format);
        }

        if(param == "CMS")
        {
            tvDataComponentColor_t level = tvDataColor_NONE;
            if ( getCMSColorEnumFromString(paramInfo.color,level ) == -1 ) {
                LOGERR("%s : GetColorEnumFromString Failed!!! ",__FUNCTION__);
                return -1;
            }

            indexInfo.colorIndex = level;

            tvComponentType_t componentLevel;
            if ( getCMSComponentEnumFromString(paramInfo.component,componentLevel ) == -1 ) {
                LOGERR("%s : GetComponentEnumFromString Failed!!! ",__FUNCTION__);
                return -1;
            }

            indexInfo.componentIndex = componentLevel;

            LOGINFO("%s colorIndex : %d , componentIndex : %d\n",__FUNCTION__,indexInfo.colorIndex, indexInfo.componentIndex);
        }

        if(param == "WhiteBalance")
        {
            tvWBColor_t level;
            if ( getWBColorEnumFromString(paramInfo.color,level ) == -1 ) {
                LOGERR("%s : GetColorEnumFromString Failed!!! ",__FUNCTION__);
                return -1;
            }

            indexInfo.colorIndex = level;

            tvWBControl_t controlLevel;
            if ( getWBControlEnumFromString(paramInfo.control,controlLevel ) == -1 ) {
                LOGERR("%s : GetComponentEnumFromString Failed!!! ",__FUNCTION__);
                return -1;
            }

            indexInfo.controlIndex = controlLevel;

            /*tvColorTemp_t colorTemp;
            if ( getColorTempEnumFromString(paramInfo.colorTemperature,colorTemp ) == -1 ) {
                LOGERR("%s : GetComponentEnumFromString Failed!!! ",__FUNCTION__);
                return -1;
            }

            indexInfo.colorTempIndex = colorTemp; */

            LOGINFO("%s colorIndex : %d , controlIndex : %d \n",__FUNCTION__,indexInfo.colorIndex, indexInfo.controlIndex);

        }

        if (indexInfo.sourceIndex == -1 || indexInfo.pqmodeIndex == -1 || indexInfo.formatIndex == -1) {
	        return -1;
    	}
        LOGINFO("%s: Exit sourceIndex = %d pqmodeIndex = %d formatIndex = %d\n",__FUNCTION__,indexInfo.sourceIndex,indexInfo.pqmodeIndex,indexInfo.formatIndex);

        return 0;
    }

    int AVOutputTV::getDolbyModeIndex(const char * dolbyMode)
    {
        int mode = 0;
        pic_modes_t *dolbyModes     ;
        unsigned short totalAvailable = 0;

        tvError_t ret = GetTVSupportedDolbyVisionModesODM(&dolbyModes,&totalAvailable);
        if(ret == tvERROR_NONE) {
            for(int count = 0;count <totalAvailable;count++ ) {
                if(strncasecmp(dolbyMode, dolbyModes[count].name, strlen(dolbyMode))==0) {
                    mode = dolbyModes[count].value;
                    break;
                }

            }
        } else {
            mode = -1;
            printf("(%s):get supported mode is failed\n", __func__);
        }

        return mode;
    }

    int AVOutputTV::getHDRModeIndex(const std::string HDRMode, const std::string format,tvDolbyMode_t &value) {
        // Create a map to associate format-mode pairs with enum values
        int ret = 0;
        static const std::unordered_map<std::string, tvDolbyMode_t> hdrModeIndexMap = {
	    {"DVDark", tvDolbyMode_Dark},
            {"DVBright", tvDolbyMode_Bright},
            {"DVGame", tvDolbyMode_Game},
            {"HDR10Dark", tvHDR10Mode_Dark},
            {"HDR10Bright", tvHDR10Mode_Bright},
            {"HDR10Game", tvHDR10Mode_Game},
            {"HLGDark", tvHLGMode_Dark},
            {"HLGBright", tvHLGMode_Bright},
            {"HLGGame", tvHLGMode_Game}
        };

        // Create the key by concatenating the format and HDRMode
        std::string key = format+HDRMode;

        // Look up the key in the map
        auto it = hdrModeIndexMap.find(key);
        if (it != hdrModeIndexMap.end()) {
            value = it->second;
            ret = 0;
        } else {
            LOGERR("%s : Invalid format/mode\n",__FUNCTION__);
            ret = -1;
        }
        return ret;
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
        capVectors_t vectorInfo;

        tvError_t ret = getParamsCaps(pqparam,vectorInfo);

        if (ret != tvERROR_NONE) {
           LOGINFO("%s: failed to get the capability \n", __FUNCTION__);
           return -1;
        }
        else {
            if(vectorInfo.isPlatformSupportVector[0].compare("true") != 0) {
                LOGERR("%s: platform support not available\n", __FUNCTION__);
                return -1;
            }
        }
        return 0;
    }

    void AVOutputTV::spliltCapablities( capVectors_t& vectorInfo, capDetails_t stringInfo)
    {
        std::vector<std::pair<std::stringstream, std::vector<std::string>&>> streamVector;

         // Initializing the streamVector with stringstreams and corresponding vectors
        streamVector.push_back({std::stringstream(stringInfo.range), vectorInfo.rangeVector});
        streamVector.push_back({std::stringstream(stringInfo.pqmode), vectorInfo.pqmodeVector});
        streamVector.push_back({std::stringstream(stringInfo.format), vectorInfo.formatVector});
        streamVector.push_back({std::stringstream(stringInfo.source), vectorInfo.sourceVector});
        streamVector.push_back({std::stringstream(stringInfo.isPlatformSupport), vectorInfo.isPlatformSupportVector});
        streamVector.push_back({std::stringstream(stringInfo.index), vectorInfo.indexVector});
        streamVector.push_back({std::stringstream(stringInfo.color), vectorInfo.colorVector});
        streamVector.push_back({std::stringstream(stringInfo.component), vectorInfo.componentVector});
        streamVector.push_back({std::stringstream(stringInfo.colorTemperature), vectorInfo.colorTempVector});

        for (auto& pair : streamVector) {
            std::stringstream& ss = pair.first;
            std::vector<std::string>& vec = pair.second;
    
            std::string token;
            while (getline(ss, token, ',')) {
                vec.push_back(token);
            }
        }
    }
    
    bool AVOutputTV::isCapablityCheckPassed( std::string param, capDetails_t inputInfo )
    {

        capDetails_t paramInfo;

        std::set<string> pqmodeCapSet;
        std::set<string> formatCapSet;
        std::set<string> sourceCapset;
        std::set<string> pqmodeInputSet;
        std::set<string> formatInputSet;
        std::set<string> sourceInputSet;

        if( ReadCapablitiesFromConfODM( param, paramInfo ) != 0 ) {
            LOGINFO( "%s: readCapablitiesFromConf Failed !!!\n",__FUNCTION__);
            return false;
        }

        if( param == "CMS")
        {
            if ( ( paramInfo.color.find(inputInfo.color) == std::string::npos ) || ( paramInfo.component.find(inputInfo.component) == std::string::npos) )
                return false;
        }
        else if( param == "WhiteBalance")
        {
            if ( ( paramInfo.color.find(inputInfo.color) == std::string::npos ) || ( paramInfo.control.find(inputInfo.control) == std::string::npos) )
                return false;
        }
        //Compare capablityInfo with Input params

        //1.convertCapablity Info to set for comparison
        spliltStringsAndConvertToSet( paramInfo.pqmode, paramInfo.format, paramInfo.source, pqmodeCapSet, formatCapSet, sourceCapset);

        //2.convert Application Input Info to set for comparison
        spliltStringsAndConvertToSet( inputInfo.pqmode, inputInfo.format, inputInfo.source, pqmodeInputSet, formatInputSet, sourceInputSet );

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

    int AVOutputTV::parsingSetInputArgument(const JsonObject& parameters, std::string pqparam,capDetails_t& paramInfo) {

        JsonArray sourceArray;
        JsonArray pqmodeArray;
        JsonArray formatArray;


        pqmodeArray = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].Array() : JsonArray();
        for (int i = 0; i < pqmodeArray.Length(); ++i) {
            paramInfo.pqmode += pqmodeArray[i].String();
            if (i != (pqmodeArray.Length() - 1) ) {
		        paramInfo.pqmode += ",";
            }
        }

        sourceArray = parameters.HasLabel("videoSource") ? parameters["videoSource"].Array() : JsonArray();
        for (int i = 0; i < sourceArray.Length(); ++i) {
            paramInfo.source += sourceArray[i].String();
            if (i != (sourceArray.Length() - 1) ) {
		        paramInfo.source += ",";
	        }
        }

        formatArray = parameters.HasLabel("videoFormat") ? parameters["videoFormat"].Array() : JsonArray();
        for (int i = 0; i < formatArray.Length(); ++i) {
            paramInfo.format += formatArray[i].String();
            if (i != (formatArray.Length() - 1) ) {
		        paramInfo.format += ",";
            }
        }

	    if (paramInfo.source.empty()) {
	        paramInfo.source = "Global";
	    }
        if (paramInfo.pqmode.empty()) {
	        paramInfo.pqmode = "Global";
	    }
        if (paramInfo.format.empty()) {
	        paramInfo.format = "Global";
	    }

        if( pqparam.compare("WhiteBalance") == 0 )
        {
            if ( paramInfo.color.empty() )
              paramInfo.color = "Global";

            if ( paramInfo.control.empty() )
              paramInfo.control = "Global";

            if ( paramInfo.colorTemperature.empty() )
              paramInfo.colorTemperature = "Global";
        }

        if( pqparam.compare("CMS") == 0 )
        {
            if ( paramInfo.color.empty() )
              paramInfo.color = "Global";

            if ( paramInfo.component.empty() )
              paramInfo.component = "Global";
        }

        if (convertToValidInputParameter(pqparam, paramInfo) != 0) {
            LOGERR("%s: Failed to convert the input paramters. \n", __FUNCTION__);
            return -1;
        }

        return 0;
    }

    int AVOutputTV::parsingGetInputArgument(const JsonObject& parameters, std::string pqparam, capDetails_t& info)
    {                                     
        info.pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";

        info.source = parameters.HasLabel("videoSource") ? parameters["videoSource"].String() : "";

        info.format = parameters.HasLabel("videoFormat") ? parameters["videoFormat"].String() : "";

        if ( (info.source.compare("Global") == 0) || (info.pqmode.compare("Global") == 0) || (info.format.compare("Global") == 0) ) {
            LOGERR("%s: get cannot fetch the Global inputs \n", __FUNCTION__);
            return -1;
        }

        if (info.source.empty()) {
	    info.source = "Current";
	    }
        if (info.pqmode.empty()) {
	    info.pqmode = "Current";
	    }
        if (info.format.empty()) {
	    info.format = "Current";
	    }

        if (convertToValidInputParameter(pqparam,info) != 0) {
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
        capVectors_t info;
        tvError_t ret = getParamsCaps(param, info);

        if (ret != tvERROR_NONE) {
            LOGERR("Failed to fetch the range capability[%s] \n", param.c_str());
            return -1;
        }

        if ( (param == "Brightness") || (param == "Contrast") ||
             (param == "Sharpness") || (param == "Saturation") ||
             (param == "Hue") || (param == "WhiteBalance") ||
            (param == "CMS") || (param == "Backlight") ||
            (param == "WhiteBalance") || (param == "LowLatencyState") ) {
            if (inputValue < stoi(info.rangeVector[0]) || inputValue > std::stoi(info.rangeVector[1])) {
                LOGERR("wrong Input value[%d]", inputValue);
                return -1;
            }
        }
        return 0;
    }

    int AVOutputTV::fetchCapablities(string pqparam, capDetails_t& info) {

       capVectors_t vectorInfo;

       tvError_t ret = tvERROR_NONE;

       ret = getParamsCaps(pqparam, vectorInfo);

       if (ret != tvERROR_NONE) {
           LOGINFO("%s: failed to get the capability \n", __FUNCTION__);
           return -1;
       }

       if (vectorInfo.sourceVector.size() != 0) {
            info.source = convertToString(vectorInfo.sourceVector);
       }

       if (vectorInfo.pqmodeVector.size() != 0) {
            info.pqmode = convertToString(vectorInfo.pqmodeVector);
       }

       if (vectorInfo.formatVector.size() != 0) {
            info.format = convertToString(vectorInfo.formatVector);
       }

       if (vectorInfo.colorVector.size() != 0) {
            info.color = convertToString(vectorInfo.colorVector);
       }

       if (vectorInfo.componentVector.size() != 0) {
            info.component = convertToString(vectorInfo.componentVector);
       }

       if (vectorInfo.controlVector.size() != 0) {
            info.control = convertToString(vectorInfo.controlVector);
       }

       if (vectorInfo.colorTempVector.size() != 0) {
            info.colorTemperature = convertToString(vectorInfo.colorTempVector);
       }

       return 0;
    }

    int AVOutputTV::validateInputParameter(std::string param, std::string inputValue)
    {

        capVectors_t info;

        tvError_t ret = getParamsCaps( param, info);

        if (ret != tvERROR_NONE) {
            LOGERR("Failed to fetch the range capability[%s] \n", param.c_str());
            return -1;
        }

        if ( (param == "ColorTemperature") ||
             (param == "DimmingMode") || (param == "AutoBacklightMode") ||
             (param == "DolbyVisionMode") || (param == "HDR10Mode") ||
            (param == "HLGMode") || (param == "AspectRatio") || (param == "PictureMode")  ) {
            auto iter = find(info.rangeVector.begin(), info.rangeVector.end(), inputValue);

            if (iter == info.rangeVector.end()) {
                LOGERR("Not a valid input value[%s].\n", inputValue.c_str());
                return -1;
            }
        }
        return 0;
    }

    void AVOutputTV::locatePQSettingsFile()
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
        rfc_caller_id[sizeof(rfc_caller_id) - 1] = '\0';
        LOGINFO("%s : Default tvsettings file : %s\n",__FUNCTION__,rfc_caller_id);
    }

    tvError_t AVOutputTV::initializePictureMode()
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
            ret = SetTVPictureMode(param.value);

            if(ret != tvERROR_NONE) {
                LOGWARN("Picture Mode set failed: %s\n",getErrorString(ret).c_str());
            }
            else {
                LOGINFO("Picture Mode initialized successfully, tr181 value [%s] value: %s\n", tr181_param_name.c_str(),
                        param.value);
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

    int AVOutputTV::convertToValidInputParameter(std::string pqparam, capDetails_t& info)
    {

        LOGINFO("Entry %s source %s pqmode %s format %s \n", __FUNCTION__, info.source.c_str(), info.pqmode.c_str(), info.format.c_str());

        capDetails_t localInfo;
        if (fetchCapablities(pqparam, localInfo) != 0) {
            LOGINFO("%s, Failed to get capability fo %s\n", __FUNCTION__,pqparam.c_str());
            return -1;
        }
        
        // converting pq to valid paramter format
        if (info.pqmode == "Global") {
            info.pqmode = localInfo.pqmode;
        }
        else if (info.pqmode == "Current") {
            char picMode[PIC_MODE_NAME_MAX]={0};
            if(!getCurrentPictureMode(picMode)) {
                LOGINFO("Failed to get the Current picture mode\n");
                return -1;
            }
            else {
                info.pqmode = picMode;
            }
        }

        if (info.source == "Global") {
            info.source = localInfo.source;
        }
        else if (info.source == "Current") {
            tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;
            tvError_t ret = GetCurrentSource(&currentSource);

            if(ret != tvERROR_NONE) {
                LOGWARN("%s: GetCurrentSource( ) Failed \n",__FUNCTION__);
                return -1;
            }
            info.source = convertSourceIndexToString(currentSource);
        }

        //convert format into valid parameter
        if (info.format == "Global") {
            info.format = localInfo.format;
        }
        else if (info.format == "Current") {
            tvVideoFormatType_t formatIndex = VIDEO_FORMAT_NONE;
            GetCurrentVideoFormat(&formatIndex);
            if ( formatIndex  == VIDEO_FORMAT_NONE) {
                formatIndex  = VIDEO_FORMAT_SDR;
	        }
            info.format = convertVideoFormatToString(formatIndex);
        }

        //convert WB and CMS params
        if( pqparam.compare("WhiteBalance") == 0 )
        {
            if( info.control.compare("Global") == 0 )
            {
                info.control = localInfo.control;       
            }
            
            if( info.color.compare("Global") == 0 )
            {
                info.color = localInfo.color;       
            }

            if( info.colorTemperature.compare("Global") == 0 )
            {
                info.colorTemperature= localInfo.colorTemperature;     
            }

            LOGINFO("%s : control : %s color : %s colorTemp : %s \n",__FUNCTION__,info.control.c_str(),info.color.c_str(),info.colorTemperature.c_str());
            
        }

        if( pqparam.compare("CMS") == 0 )
        {
            if( info.component.compare("Global") == 0 )
            {
                info.component = localInfo.component;       
            }
            
            if( info.color.compare("Global") == 0 )
            {
                info.color = localInfo.color;       
            }

            LOGINFO("%s : component : %s color : %s \n",__FUNCTION__,info.component.c_str(),info.color.c_str());
        }

        LOGINFO("Exit %s source %s pqmode %s format %s \n", __FUNCTION__, info.source.c_str(), info.pqmode.c_str(), info.format.c_str());
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

    tvContentFormatType_t AVOutputTV::convertFormatStringToTVContentFormat(const char *format)
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

    tvError_t AVOutputTV::updateAVoutputTVParamToHAL(std::string forParam, paramIndex_t indexInfo, int value,bool setNotDelete)
    {
        tvError_t ret = tvERROR_NONE;
        std::string key;

        if( forParam.compare("CMS") == 0 )
            generateStorageIdentifierCMS(key,forParam,indexInfo);
        else if( forParam.compare("WhiteBalance") == 0 ) 
            generateStorageIdentifierWB(key,forParam,indexInfo);
        else 
            generateStorageIdentifier(key,forParam,indexInfo);

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
                else if (forParam.compare("DolbyVisionMode") == 0 || forParam.compare("HDRMode") == 0 ) {
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

    int AVOutputTV::updateAVoutputTVParam( std::string action, std::string tr181ParamName, capDetails_t info, tvPQParameterIndex_t pqParamIndex, int level )
    {
        LOGINFO("Entry : %s\n",__FUNCTION__);
        valueVectors_t values;
        paramIndex_t paramIndex;
        std::vector<int> sources;
        std::vector<int> pictureModes;
        std::vector<int> formats;
        int ret = 0;
        bool sync = !(action.compare("sync"));
        bool reset = !(action.compare("reset"));
        bool set = !(action.compare("set"));

        LOGINFO("%s: Entry param : %s Action : %s pqmode : %s source :%s format :%s color:%s component:%s control:%s\n",__FUNCTION__,tr181ParamName.c_str(),action.c_str(),info.pqmode.c_str(),info.source.c_str(),info.format.c_str(),info.color.c_str(),info.component.c_str(),info.control.c_str() );
        ret = getSaveConfig(tr181ParamName,info, values);
        if( 0 == ret ) {
            for( int sourceType: values.sourceValues ) {
                paramIndex.sourceIndex = sourceType;
                for( int modeType : values.pqmodeValues ) {
                    paramIndex.pqmodeIndex = modeType;
                    for( int formatType : values.formatValues ) {
                        paramIndex.formatIndex = formatType;
                        switch(pqParamIndex) {
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
                            case PQ_PARAM_HDR_MODE:
                                if(reset) {
                                    ret |= updateAVoutputTVParamToHAL(tr181ParamName,paramIndex,0,false);
		                		}
                                if(sync || reset) {
                                    int value=0;
                                    if(getLocalparam(tr181ParamName,paramIndex,value,pqParamIndex,sync)) {
					                    continue;
			                        }
                                    level=value;
                                }
                                if(set) {
                                    ret |= updateAVoutputTVParamToHAL(tr181ParamName,paramIndex,level,true);
                                }
                                break;
                            default:
                                break;
                        }
                        switch(pqParamIndex) {
                            case PQ_PARAM_BRIGHTNESS:
                                ret |= SaveBrightness((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,level);
                                break;
                            case PQ_PARAM_CONTRAST:
                                ret |= SaveContrast((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,level);
                                break;
                            case PQ_PARAM_SHARPNESS:
                                ret |= SaveSharpness((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,level);
                                break;
                            case PQ_PARAM_HUE:
                                ret |= SaveHue((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,level);
                                break;
                            case PQ_PARAM_SATURATION:
                                ret |= SaveSaturation((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,level);
                                break;
                            case PQ_PARAM_COLOR_TEMPERATURE:
                                ret |= SaveColorTemperature((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,(tvColorTemp_t)level);
                                break;
                            case PQ_PARAM_BACKLIGHT:
                                ret |= SaveBacklight((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,level);
                                break;
                            case PQ_PARAM_DIMMINGMODE:
                                ret |= SaveTVDimmingMode((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,(tvDimmingMode_t)level);
                                break;
                            case PQ_PARAM_LOWLATENCY_STATE:
                                ret |= SaveLowLatencyState((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,level);
                                break;
                            case PQ_PARAM_DOLBY_MODE:
                            case PQ_PARAM_HDR_MODE:
                                 ret |= SaveTVDolbyVisionMode((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,(tvDolbyMode_t)level);
                                 break;

                            case PQ_PARAM_ASPECT_RATIO:
                                 ret |= SaveAspectRatio((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,(tvDisplayMode_t)level);
                                 break;
                             
                            case PQ_PARAM_CMS_SATURATION_RED:                
                            case PQ_PARAM_CMS_SATURATION_BLUE:        
                            case PQ_PARAM_CMS_SATURATION_GREEN:          
                            case PQ_PARAM_CMS_SATURATION_YELLOW:         
                            case PQ_PARAM_CMS_SATURATION_CYAN:         
                            case PQ_PARAM_CMS_SATURATION_MAGENTA:        
                            case PQ_PARAM_CMS_HUE_RED:        
                            case PQ_PARAM_CMS_HUE_BLUE:                  
                            case PQ_PARAM_CMS_HUE_GREEN:                 
                            case PQ_PARAM_CMS_HUE_YELLOW:                
                            case PQ_PARAM_CMS_HUE_CYAN:                
                            case PQ_PARAM_CMS_HUE_MAGENTA:               
                            case PQ_PARAM_CMS_LUMA_RED:               
                            case PQ_PARAM_CMS_LUMA_BLUE:                 
                            case PQ_PARAM_CMS_LUMA_GREEN:
                            case PQ_PARAM_CMS_LUMA_YELLOW:               
                            case PQ_PARAM_CMS_LUMA_CYAN:               
                            case PQ_PARAM_CMS_LUMA_MAGENTA:
                            {
                                for( int componentType : values.componentValues ) {
                                    paramIndex.componentIndex = componentType;
                                    for( int colorType : values.colorValues ) {
                                        paramIndex.colorIndex = colorType;                     
                                        if(reset) {
                                            ret |= updateAVoutputTVParamToHAL(tr181ParamName,paramIndex,0,false);
		                		        }
                                        if(sync || reset) {
                                            int value=0;
                                            tvPQParameterIndex_t pqIndex;
                                            if ( convertCMSParamToPQEnum(getCMSComponentStringFromEnum((tvComponentType_t)paramIndex.componentIndex),getCMSColorStringFromEnum((tvDataComponentColor_t)paramIndex.colorIndex),pqIndex) != 0 )
                                            {
                                                LOGERR("%s:convertCMSParamToPQEnum failed color : %d component : %d \n",__FUNCTION__,paramIndex.colorIndex,paramIndex.componentIndex);
                                                return -1;
                                            }
                                            if(getLocalparam(tr181ParamName,paramIndex,value,pqIndex,sync)) {
					                            continue;
			                                }
                                            else {
                                                if(sync)
                                                    ret |= SaveCMS((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,COMP_NONE,tvDataColor_NONE,COMPONENT_ENABLE);
                                                else
                                                    ret |= SaveCMS((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,COMP_NONE,tvDataColor_NONE,COMPONENT_DISABLE);
                                            }
                                            level=value;
                                        }
                                        ret |= SaveCMS((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,(tvComponentType_t)paramIndex.componentIndex,(tvDataComponentColor_t)paramIndex.colorIndex,level);

                                        if(set) {
                                            ret |= updateAVoutputTVParamToHAL(tr181ParamName,paramIndex,level,true);
                                        }
                                    }
                                }
                                break;
                            }
                            case PQ_PARAM_WB_RED_GAIN:
                            case PQ_PARAM_WB_GREEN_GAIN:
                            case PQ_PARAM_WB_BLUE_GAIN:
                            case PQ_PARAM_WB_RED_OFFSET:
                            case PQ_PARAM_WB_GREEN_OFFSET:
                            case PQ_PARAM_WB_BLUE_OFFSET:
                            {
                               for( int colorType : values.colorValues ) {
                                   paramIndex.colorIndex = colorType;
                                   for( int controlType : values.controlValues ) {
                                        paramIndex.controlIndex = controlType;
                                        if(reset) {
                                            ret |= updateAVoutputTVParamToHAL(tr181ParamName,paramIndex,0,false);
		                	            }
                                        if(sync || reset) {
                                            int value=0;
                                            if(getLocalparam(tr181ParamName,paramIndex,value,pqParamIndex,sync)) {
					                            continue;
			                                }
                                            level=value;
                                        }
                                         /* tvRGBType_t rgbIndex;
                                            if ( convertWBParamToRGBEnum(getWBColorStringFromEnum((tvWBColor_t)(paramIndex.colorIndex)),getWBControlStringFromEnum((tvWBControl_t)(paramIndex.controlIndex)),rgbIndex) != 0 )
                                            {
                                                LOGERR("%s:convertWBParamToRGBEnum failed Color : %d Control : %d  \n",__FUNCTION__,paramIndex.colorIndex,paramIndex.controlIndex);
                                                return -1;  
                                            }*/
                                        ret |= SaveCustom2PointWhiteBalance((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,(tvWBColor_t)paramIndex.colorIndex,(tvWBControl_t)paramIndex.controlIndex,level);

                                        if(set) {
                                            ret |= updateAVoutputTVParamToHAL(tr181ParamName,paramIndex,level,true);
                                        }   
                                }
                            }
                                break;
                            }
                            case PQ_PARAM_LOCALDIMMING_LEVEL:
                             {
                                if(sync) {
                                    int value=0;
                                    getLocalparam(tr181ParamName,paramIndex,value,pqParamIndex,sync);
                                    level=value;
                                }
                                ret |= SaveTVDimmingMode((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,(tvDimmingMode_t)level);
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

    tvError_t AVOutputTV::syncAvoutputTVParamsToHAL(std::string pqmode,std::string source,std::string format)
    {
        int level={0};
        capDetails_t info;
        info.pqmode = pqmode;
        info.source = source;
        info.format = format;

        LOGINFO("Entry %s : pqmode : %s source : %s format : %s\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str());

        if( !updateAVoutputTVParam("sync","Brightness",info,PQ_PARAM_BRIGHTNESS,level)) {
            LOGINFO("Brightness Successfully sync to Drive Cache\n");
	    }
        else {
            LOGERR("Brightness Sync to cache Failed !!!\n");
	    }

        if( !updateAVoutputTVParam("sync","Contrast",info,PQ_PARAM_CONTRAST,level)) {
            LOGINFO("Contrast Successfully Synced to Drive Cache\n");
	    }
        else {
            LOGERR("Contrast Sync to cache Failed !!!\n");
        }

        if( !updateAVoutputTVParam("sync","Sharpness",info,PQ_PARAM_SHARPNESS,level)) {
            LOGINFO("Sharpness Successfully Synced to Drive Cache\n");
        }
        else {
            LOGERR("Sharpness Sync to cache Failed !!!\n");
        }

        if( !updateAVoutputTVParam("sync","Saturation",info,PQ_PARAM_SATURATION,level)) {
            LOGINFO("Saturation Successfully Synced to Drive Cache\n");
        }
        else {
            LOGERR("Saturation Sync to cache Failed !!!\n");
        }

        if( !updateAVoutputTVParam("sync","Hue",info,PQ_PARAM_HUE,level)) {
            LOGINFO("Hue Successfully Synced to Drive Cache\n");
        }
        else {
            LOGERR("Hue Sync to cache Failed !!!\n");
        }

        if( !updateAVoutputTVParam("sync","ColorTemp",info,PQ_PARAM_COLOR_TEMPERATURE,level)) {
            LOGINFO("ColorTemp Successfully Synced to Drive Cache\n");
        }
        else {
            LOGERR("ColorTemp Sync to cache Failed !!!\n");
        }
        
        if( !updateAVoutputTVParam("sync","HDRMode",info,PQ_PARAM_HDR_MODE,level)) {
            LOGINFO("dvmode Successfully Synced to Drive Cache\n");
        }
        else {
            LOGERR("dvmode Sync to cache Failed !!!\n");
        }

        if( !updateAVoutputTVParam("sync","DimmingMode",info,PQ_PARAM_DIMMINGMODE,level)) {
            LOGINFO("dimmingmode Successfully Synced to Drive Cache\n");
        }
        else {
            LOGERR("dimmingmode Sync to cache Failed !!!\n");
        }

        if( !updateAVoutputTVParam("sync","Backlight",info,PQ_PARAM_BACKLIGHT,level) ) {
            LOGINFO("Backlight Successfully Synced to Drive Cache\n");
        }
        else {
            LOGERR("Backlight Sync to cache Failed !!!\n");
        }

        syncCMSParams(); //sync CMS 
        
        syncWBParams();

        info.format = "DV";

        if( !updateAVoutputTVParam("sync","DolbyVisionMode",info,PQ_PARAM_DOLBY_MODE,level)) {
            LOGINFO("dvmode Successfully Synced to Drive Cache\n");
        }
        else {
            LOGERR("dvmode Sync to cache Failed !!!\n");
        }

        LOGINFO("Exit %s : pqmode : %s source : %s format : %s\n",__FUNCTION__,pqmode.c_str(),source.c_str(),format.c_str());
        return tvERROR_NONE;
    }

    int AVOutputTV::syncAvoutputTVPQModeParamsToHAL(std::string pqmode, std::string source, std::string format)
    {
        capDetails_t inputInfo;
        valueVectors_t valueVectors;
        tr181ErrorCode_t err = tr181Success;
        TR181_ParamData_t param = {0};
        int ret = 0;

        inputInfo.pqmode = pqmode;
        inputInfo.source = source;
        inputInfo.format = format;

        ret = getSaveConfig("PictureMode", inputInfo, valueVectors);

        if (ret == 0 ) {
            for (int source : valueVectors.sourceValues ) {
                tvVideoSrcType_t sourceType = (tvVideoSrcType_t)source;
                for (int format : valueVectors.formatValues ) {
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

    uint32_t AVOutputTV::generateStorageIdentifier(std::string &key, std::string forParam, paramIndex_t info)
    {
        key+=std::string(AVOUTPUT_GENERIC_STRING_RFC_PARAM);
        key+=STRING_SOURCE+convertSourceIndexToString(info.sourceIndex)+std::string(".")+STRING_PICMODE+convertPictureIndexToString(info.pqmodeIndex)+std::string(".")+std::string(STRING_FORMAT)+convertVideoFormatToString(info.formatIndex)+std::string(".")+forParam;
        return tvERROR_NONE;
    }

    uint32_t AVOutputTV::generateStorageIdentifierCMS(std::string &key, std::string forParam, paramIndex_t info)
    {
        key+=std::string(AVOUTPUT_GENERIC_STRING_RFC_PARAM);
        key+=STRING_SOURCE+convertSourceIndexToString(info.sourceIndex)+std::string(".")+STRING_PICMODE+convertPictureIndexToString(info.pqmodeIndex)+std::string(".")+std::string(STRING_FORMAT)+convertVideoFormatToString(info.formatIndex)+std::string(".")+STRING_COLOR+getCMSColorStringFromEnum((tvDataComponentColor_t)info.colorIndex)+std::string(".")+STRING_COMPONENT+getCMSComponentStringFromEnum((tvComponentType_t)info.componentIndex)+std::string(".")+forParam;
        return tvERROR_NONE;
    }

    uint32_t AVOutputTV::generateStorageIdentifierWB(std::string &key, std::string forParam, paramIndex_t info)
    {
        key+=std::string(AVOUTPUT_GENERIC_STRING_RFC_PARAM);
        key+=STRING_SOURCE+convertSourceIndexToString(info.sourceIndex)+std::string(".")+STRING_PICMODE+convertPictureIndexToString(info.pqmodeIndex)+std::string(".")+std::string(STRING_FORMAT)+convertVideoFormatToString(info.formatIndex)+std::string(".")+STRING_COLOR+getWBColorStringFromEnum((tvWBColor_t)info.colorIndex)+std::string(".")+STRING_CONTROL+getWBControlStringFromEnum((tvWBControl_t)info.controlIndex)+std::string(".")+forParam;
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
        switch (eReturn) {
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

    int AVOutputTV::getSaveConfig(std::string param, capDetails_t capInfo, valueVectors_t &values)
    {
        LOGINFO("Entry : %s pqmode : %s source :%s format :%s component : %s color : %s control:%s\n",__FUNCTION__,capInfo.pqmode.c_str(),capInfo.source.c_str(),capInfo.format.c_str(),capInfo.component.c_str(),capInfo.color.c_str(),capInfo.control.c_str());

        int ret = 0;

        if (getAvailableCapabilityModes(capInfo) != 0) {
            LOGERR("%s: failed to get picture/source/format mode capability \n", __FUNCTION__);
            return -1;
        }
        //pqmode
        char *modeString = strdup(capInfo.pqmode.c_str());
        char *token = NULL;
        while ((token = strtok_r(modeString,",",&modeString))) {
            std::string local = token;
            values.pqmodeValues.push_back(getPictureModeIndex(local));
        }
        //source
        char *sourceString = strdup(capInfo.source.c_str());
        char *sourceToken = NULL;
        while ((sourceToken = strtok_r(sourceString,",",&sourceString))) {
            std::string local = sourceToken;
            if( local == "All") continue;
            values.sourceValues.push_back(getSourceIndex(local));
        }
        //3)check format
        char *formatString = strdup(capInfo.format.c_str());
        char *formatToken = NULL;
        while ((formatToken = strtok_r(formatString,",",&formatString))) {
            std::string local = formatToken;
            values.formatValues.push_back(getFormatIndex(local));
        }

        if( param.compare("CMS") == 0 )
        {
            //Check Color
            char *colorString = strdup(capInfo.color.c_str());
            char *colorToken = NULL;
            while ((colorToken = strtok_r(colorString,",",&colorString))) {
                std::string local = colorToken;
                tvDataComponentColor_t level = tvDataColor_NONE;
                if ( getCMSColorEnumFromString(local,level ) == -1 ) {
                    LOGERR("%s : GetColorEnumFromString Failed!!! ",__FUNCTION__);
                    return -1;
                }
                values.colorValues.push_back(level);
            }

            //Check Component
            char *componentString = strdup(capInfo.component.c_str());
            char *componentToken = NULL;
            while ((componentToken = strtok_r(componentString,",",&componentString))) {
                std::string local = componentToken;
                tvComponentType_t level;
                if ( getCMSComponentEnumFromString(local,level ) == -1 ) {
                    LOGERR("%s : GetComponentEnumFromString Failed!!! ",__FUNCTION__);
                    return -1;
                }
                values.componentValues.push_back(level);
            }
        }

        if( param.compare("WhiteBalance") == 0 )
        {
            //Check Color
            char *colorString = strdup(capInfo.color.c_str());
            char *colorToken = NULL;
            while ((colorToken = strtok_r(colorString,",",&colorString))) {
                std::string local = colorToken;
                tvWBColor_t level=tvWB_COLOR_RED;
                if ( getWBColorEnumFromString(local,level ) == -1 ) {
                    LOGERR("%s : GetWBColorEnumFromString Failed!!! ",__FUNCTION__);
                    return -1;
                }
                values.colorValues.push_back(level);
            }

            //Check Control
            char *controlString = strdup(capInfo.control.c_str());
            char *controlToken = NULL;
            while ((controlToken = strtok_r(controlString,",",&controlString))) {
                std::string local = controlToken;
                tvWBControl_t level=tvWB_CONTROL_GAIN;;
                if ( getWBControlEnumFromString(local,level ) == -1 ) {
                    LOGERR("%s : GetWBControlEnumFromString Failed!!! ",__FUNCTION__);
                    return -1;
                }
                values.controlValues.push_back(level);
            }
            
            /*
            //Check Color Temp
            char *colorTempString = strdup(capInfo.colorTemperature.c_str());
            char *colorTempToken = NULL;
            while ((colorTempToken = strtok_r(colorTempString,",",&colorTempString))) {
                std::string local = colorTempToken;
                tvColorTemp_t level;
                if ( getColorTempEnumFromString(local,level ) == -1 ) {
                    LOGERR("%s : GetColorTempEnumFromString Failed!!! ",__FUNCTION__);
                    return -1;
                }
                values.colorTempValues.push_back(level);
            }*/
        }

        LOGINFO("Exit : %s pqmode : %s source :%s format :%s ret:%d\n",__FUNCTION__,capInfo.pqmode.c_str(),capInfo.source.c_str(),capInfo.format.c_str(), ret);
        return ret;
    }

    int AVOutputTV::getLocalparam( std::string forParam,paramIndex_t indexInfo,int & value,tvPQParameterIndex_t pqParamIndex,bool sync)
    {
        string key;
        TR181_ParamData_t param={0};
        
	    if( forParam.compare("CMS") == 0 ) {
            generateStorageIdentifierCMS(key,forParam,indexInfo);
        } else if( forParam.compare("WhiteBalance") == 0 ) {
            generateStorageIdentifierWB(key,forParam,indexInfo);
        } else {
            generateStorageIdentifier(key,forParam,indexInfo);
        }

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
                else if (strncmp(param.value, "UserDefined", strlen(param.value))==0) {
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
	       else if(strncmp(param.value, "Game", strlen(param.value)) == 0) {
                   value = tvDolbyMode_Game;
               }
               else {
                   value = tvDolbyMode_Bright;
               }
               return 0;
           }
	       else if ( forParam.compare("HDRMode") == 0) {
               if (strncmp(param.value, "Dark", strlen(param.value)) == 0 && key.find("DV") != std::string::npos ) {
                   value = tvDolbyMode_Dark;
               }
               else if(strncmp(param.value, "Bright", strlen(param.value)) == 0 && key.find("DV") != std::string::npos ) {
                   value = tvDolbyMode_Game;
               }
	           else if(strncmp(param.value, "Dark", strlen(param.value)) == 0 && key.find("HDR10") != std::string::npos ) {
                   value = tvHDR10Mode_Dark;
               }
	           else if(strncmp(param.value, "Bright", strlen(param.value)) == 0 && key.find("HDR10") != std::string::npos ) {
                   value = tvHDR10Mode_Bright;
               }
	           else if(strncmp(param.value, "Dark", strlen(param.value)) == 0 && key.find("HLG") != std::string::npos ) {
                   value = tvHLGMode_Dark;
               }
               else if(strncmp(param.value, "Bright", strlen(param.value)) == 0 && key.find("HLG") != std::string::npos ) {
                   value = tvHLGMode_Bright;
               }
               else {
                   value = tvDolbyMode_Game;
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
            GetDefaultPQParams(indexInfo.pqmodeIndex,(tvVideoSrcType_t)indexInfo.sourceIndex,(tvVideoFormatType_t)indexInfo.formatIndex,pqParamIndex,&value);
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

    /*int AVOutputTV::getDolbyParams(tvContentFormatType_t format, std::string &s, std::string source)
    {
        int ret = -1;
        TR181_ParamData_t param;
        std::string rfc_param = AVOUTPUT_HDR10MODE_RFC_PARAM;
        int dolby_mode_value = 0;
        tvVideoSrcType_t sourceIndex = VIDEO_SOURCE_IP;
        //Since dolby vision is source specific, we should for check for specific source
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
            tvError_t retVal = GetDefaultPQParams(pqmodeIndex,(tvVideoSrcType_t)sourceIndex,
                                                 (tvVideoFormatType_t)ConvertHDRFormatToContentFormatODM((tvhdr_type_t)format),
                                                 PQ_PARAM_DOLBY_MODE,&dolby_mode_value);
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
    }*/

    tvError_t AVOutputTV::getParamsCaps(std::string param, capVectors_t &vecInfo)
    {
        tvError_t ret = tvERROR_NONE;
        capDetails_t stringInfo;
    
        if( ReadCapablitiesFromConfODM( param, stringInfo) != 0 )
        {
            LOGERR( "%s: ReadCapablitiesFromConf Failed !!!\n",__FUNCTION__);
            return tvERROR_GENERAL;
        }
        else
        {
            spliltCapablities( vecInfo, stringInfo);
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
                    [tvColorTemp_USER] = "UserDefined"
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
            picMode[strlen(param.value)] = '\0';
            LOGINFO("getLocalParam success, mode = %s\n", picMode);
            return 1;
        }
        else {
            LOGERR("getLocalParam failed %s\n",tr181_param_name.c_str());
            return 0;
        }
    }

    /*int AVOutputTV::getDolbyParamToSync(int sourceIndex, int formatIndex, int& value)
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
            value=getDolbyModeIndex(param.value);
            ret = 0;
        }

        return ret;
    }*/

    std::string AVOutputTV::getDolbyModeStringFromEnum( tvDolbyMode_t mode)

    {
            std::string value;
            switch(mode) {
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
	        case tvDolbyMode_Game:
                case tvHDR10Mode_Game:
                case tvHLGMode_Game:
                    value = "Game";
                    break;
                default:
                    break;
            }

            return value;
    }

    int AVOutputTV::getAvailableCapabilityModesWrapper(std::string param, std::string & outparam)
    {
        tvError_t err = tvERROR_NONE;
        capVectors_t info;

         err = getParamsCaps(param,info);
         if (err != tvERROR_NONE) {
             LOGERR("%s: failed to get [%s] capability \n", __FUNCTION__, param.c_str());
             return -1;
         }
         outparam = convertToString(info.rangeVector);

         return 0;
    }

    int AVOutputTV::getAvailableCapabilityModes( capDetails_t &info)
    {
        if ((info.pqmode.compare("none") == 0 )) {
            if (getAvailableCapabilityModesWrapper("PictureMode", info.pqmode) != 0) {
                LOGERR("%s: failed to get picture mode capability \n", __FUNCTION__);
                return -1;
            }
        }

        if( (info.source.compare("none") == 0)) {
            if (getAvailableCapabilityModesWrapper("VideoSource",info.source) != 0) {
                LOGERR("%s: failed to get source capability \n", __FUNCTION__);
                return -1;
            }
        }

        if( (info.format.compare("none") == 0) ) {
            if (getAvailableCapabilityModesWrapper("VideoFormat",info.format) != 0) {
                LOGERR("%s: failed to get format capability \n", __FUNCTION__);
                return -1;
            }
        }
        return 0;
    }

    int AVOutputTV::getCapabilitySource(JsonArray & rangeArray)
    {
        capVectors_t info;

        tvError_t ret = getParamsCaps("VideoSource",info);

        if(ret != tvERROR_NONE) {
            return -1;
        }
        else {
            if ((info.rangeVector.front()).compare("none") != 0) {
                for (unsigned int index = 0; index < info.rangeVector.size(); index++) {
                    rangeArray.Add(info.rangeVector[index]);
                }
            }
        }
        return 0;
    }

    int AVOutputTV::getRangeCapability(std::string param, std::vector<std::string> & rangeInfo)
    {
        capVectors_t info;

        tvError_t ret = getParamsCaps(param,info);

        if(ret != tvERROR_NONE) {
            return -1;
        }
        else {
            if ((info.rangeVector.front()).compare("none") != 0) {
                rangeInfo = info.rangeVector;
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

    void AVOutputTV::broadcastLowLatencyModeChangeEvent(bool lowLatencyMode)
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
        capDetails_t inputInfo;
        
        inputInfo.pqmode = pqmode;
        inputInfo.source = source;
        inputInfo.format = format;

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
                int retval=updateAVoutputTVParam("set","ZoomMode",inputInfo,PQ_PARAM_ASPECT_RATIO,mode);

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

    int AVOutputTV::getCMSComponentEnumFromString(std::string component, tvComponentType_t& value)
    {
        int ret = 0;
	
        if( component.compare("Luma") == 0 )
            value = COMP_LUMA;
	    else if( component.compare("Saturation") == 0 )
	        value =  COMP_SATURATION;
        else if( component.compare("Hue") == 0 )
	        value = COMP_HUE;
        else
	        ret = -1;
		
	    return ret;
    }

    int AVOutputTV::getCMSColorEnumFromString(std::string color,tvDataComponentColor_t& value)
    {
        int ret = 0;
	
        if( color.compare("Red") == 0 )
	        value = tvDataColor_RED;
	    else if( color.compare("Green") == 0 )
	        value = tvDataColor_GREEN;
        else if( color.compare("Blue") == 0 )
	        value = tvDataColor_BLUE;
        else if( color.compare("Yellow") == 0)
	        value = tvDataColor_YELLOW;
        else if( color.compare("Cyan") == 0)
	        value = tvDataColor_CYAN;
        else if( color.compare("Magenta") == 0)
	        value = tvDataColor_MAGENTA;
        else
	        ret = -1;
	
	    return ret;
    }

    int AVOutputTV::getColorTempEnumFromString(std::string color, tvColorTemp_t& value)
    {
        int ret = 0;
	
        if( color.compare("Standard") == 0 )
            value = tvColorTemp_STANDARD;
	    else if( color.compare("Warm") == 0 )
	        value =  tvColorTemp_WARM;
        else if( color.compare("Cold") == 0 )
	        value = tvColorTemp_COLD;
        else if( color.compare("UserDefined") == 0 )
            value =tvColorTemp_USER;
        else
	        ret = -1;
		
	    return ret;
    }

    int AVOutputTV::convertToSourceOffsetEnum(std::string source, tvColorTempSourceOffset_t& value)
    {
        int ret = 0;

        if( source.compare("Composite1") == 0 )
	        value = AV_OFFSET;
	    else if( source.compare("IP") == 0 )
	        value = TV_OFFSET;
        else if( source.compare("HDMI1") == 0  || source.compare("HDMI2") == 0 || source.compare("HDMI3") == 0 || source.compare("Tuner") == 0)
	        value = HDMI_OFFSET;
        else
	        ret = -1;
	
	    return ret;
    }

    void AVOutputTV::syncCMSParams( )
    {
        int level = 0;
        std::string cmsParam;
	    tvPQParameterIndex_t tvPQEnum;
        capDetails_t inputInfo;
        tvDataComponentColor_t colors[] = {tvDataColor_RED,tvDataColor_GREEN,tvDataColor_BLUE,tvDataColor_YELLOW,tvDataColor_CYAN,tvDataColor_MAGENTA};
        
        inputInfo.pqmode = "none";
        inputInfo.source = "none";
        inputInfo.format = "none";

        for ( int component = COMP_HUE; component < COMP_MAX;component++) {
            for(int count = 0;count < (int)(sizeof(colors)/sizeof(colors[0])); ++count) {

		        tvDataComponentColor_t color = colors[count];
			
			    std::string componentString = getCMSComponentStringFromEnum((tvComponentType_t)component);
			    std::string colorString = getCMSColorStringFromEnum((tvDataComponentColor_t)color);
			    cmsParam = componentString+"."+colorString;
			
			    if ( convertCMSParamToPQEnum(componentString,colorString,tvPQEnum) != 0 ) {
                    LOGINFO("%s: %s/%s Param Not Found \n",__FUNCTION__,componentString.c_str(),componentString.c_str());
                    continue;
                }

                inputInfo.color = colorString;
                inputInfo.component = componentString;
			    if( !updateAVoutputTVParam("sync","CMS", inputInfo,tvPQEnum,level))
                   LOGINFO("CMS Successfully Synced to Drive Cache\n");
                else
                   LOGERR("CMS Sync to cache Failed !!!\n");
		    }
	    }
    }

    void AVOutputTV::syncWBParams( )
    {
        int level = 0;
        tvPQParameterIndex_t tvPQEnum;
        capDetails_t inputInfo;
	
        inputInfo.pqmode = "none";
        inputInfo.source = "none";
        inputInfo.format = "none";
	
        for( int colorIndex= tvWB_COLOR_RED; colorIndex < tvWB_COLOR_MAX; colorIndex++) {
           for(int controlIndex = tvWB_CONTROL_GAIN;controlIndex < tvWB_CONTROL_MAX;controlIndex++) {
                inputInfo.control = getWBControlStringFromEnum((tvWBControl_t)controlIndex);
                inputInfo.color   = getWBColorStringFromEnum((tvWBColor_t)colorIndex);

                if ( convertWBParamToPQEnum(inputInfo.control,inputInfo.color,tvPQEnum) != 0 ) {
                    LOGERR("%s: %s/%s Param Not Found \n",__FUNCTION__,inputInfo.control.c_str(),inputInfo.color.c_str());
                }    
                updateAVoutputTVParam("sync","WhiteBalance",inputInfo,tvPQEnum,level);
            }
        }
	}
    

    int AVOutputTV:: convertCMSParamToPQEnum(const std::string component, const std::string color,tvPQParameterIndex_t& value) {
    // Create a map to associate color-component pairs with enum values
	    int ret = 0;
        static const std::unordered_map<std::string, tvPQParameterIndex_t> colorComponentMap = {
            {"SaturationRed", PQ_PARAM_CMS_SATURATION_RED},
            {"SaturationGreen", PQ_PARAM_CMS_SATURATION_GREEN},
            {"SaturationBlue", PQ_PARAM_CMS_SATURATION_BLUE},
            {"SaturationCyan", PQ_PARAM_CMS_SATURATION_CYAN},
            {"SaturationMagenta", PQ_PARAM_CMS_SATURATION_MAGENTA},
            {"SaturationYellow", PQ_PARAM_CMS_SATURATION_YELLOW},
            {"HueRed", PQ_PARAM_CMS_HUE_RED},
            {"HueGreen", PQ_PARAM_CMS_HUE_GREEN},
            {"HueBlue", PQ_PARAM_CMS_HUE_BLUE},
            {"HueCyan", PQ_PARAM_CMS_HUE_CYAN},
            {"HueMagenta", PQ_PARAM_CMS_HUE_MAGENTA},
            {"HueYellow", PQ_PARAM_CMS_HUE_YELLOW},
            {"LumaRed", PQ_PARAM_CMS_LUMA_RED},
            {"LumaGreen", PQ_PARAM_CMS_LUMA_GREEN},
            {"LumaBlue", PQ_PARAM_CMS_LUMA_BLUE},
            {"LumaCyan", PQ_PARAM_CMS_LUMA_CYAN},
            {"LumaMagenta", PQ_PARAM_CMS_LUMA_MAGENTA},
            {"LumaYellow", PQ_PARAM_CMS_LUMA_YELLOW}
        };

        // Create the key by concatenating the component and color
        std::string key = component + color;

        // Look up the key in the map
        auto it = colorComponentMap.find(key);
        if (it != colorComponentMap.end()) {
            value = it->second;
		    ret = 0;
        } else {
	        LOGERR("%s : Invalid color/component\n",__FUNCTION__);
            ret = -1;
        }
	    return ret;
    }

    int AVOutputTV:: convertWBParamToRGBEnum(const std::string color,std::string control,tvRGBType_t &value)
    {
        // Create a map to associate color-ntrol pairs with enum values
	    int ret = 0;
        static const std::unordered_map<std::string, tvRGBType_t> colorControlMap = {
            {"RedGain", R_GAIN},
            {"GreenGain", G_GAIN},
            {"BlueGain", B_GAIN},
            {"RedOffset", R_POST_OFFSET},
            {"GreenOffset", R_POST_OFFSET},
            {"BlueOffset", R_POST_OFFSET}
        };

        // Create the key by concatenating the color and control
        std::string key = color + control;

        // Look up the key in the map
        auto it = colorControlMap.find(key);
        if (it != colorControlMap.end()) {
            value = it->second;
		    ret = 0;
        } else {
	        LOGERR("%s : Invalid color/control\n",__FUNCTION__);
            ret = -1;
        }
	    return ret; 
    }

    int AVOutputTV:: convertWBParamToPQEnum(const std::string control, const std::string color,tvPQParameterIndex_t& value) {
    // Create a map to associate color-component pairs with enum values
	    int ret = 0;
        static const std::unordered_map<std::string, tvPQParameterIndex_t> colorControlMap = {
            {"RedGain", PQ_PARAM_WB_RED_GAIN},
            {"RedOffset", PQ_PARAM_WB_RED_OFFSET},
            {"GreenGain", PQ_PARAM_WB_GREEN_GAIN},
            {"GreenOffset", PQ_PARAM_WB_GREEN_OFFSET},
            {"BlueGain", PQ_PARAM_WB_BLUE_GAIN},
            {"BlueOffset", PQ_PARAM_WB_BLUE_OFFSET},
        };

        // Create the key by concatenating the component and color
        std::string key = color+control;

        // Look up the key in the map
        auto it = colorControlMap.find(key);
        if (it != colorControlMap.end()) {
            value = it->second;
		    ret = 0;
        } else {
	        LOGERR("%s : Invalid color/control\n",__FUNCTION__);
            ret = -1;
        }
	    return ret;
    }

    std::string AVOutputTV::getCMSColorStringFromEnum(tvDataComponentColor_t value)
    {
        switch(value)
        {
            case tvDataColor_RED: return "Red";
            case tvDataColor_GREEN: return "Green";
            case tvDataColor_BLUE: return "Blue";
		    case tvDataColor_YELLOW: return "Yellow";
		    case tvDataColor_CYAN: return "Cyan";
		    case tvDataColor_MAGENTA: return "Magenta";
            default : return "Max";
        }
    }

    std::string AVOutputTV::getCMSComponentStringFromEnum(tvComponentType_t value) {
        switch(value) {
            case COMP_HUE: return "Hue";
            case COMP_SATURATION: return "Saturation";
            case COMP_LUMA: return "Luma";
            default : return "Max";
        }
    }

    std::string AVOutputTV::getWBColorStringFromEnum(tvWBColor_t value) {
        switch(value) {
            case tvWB_COLOR_RED: return "Red";
            case tvWB_COLOR_GREEN: return "Green";
            case tvWB_COLOR_BLUE: return "Blue";
            default : return "Max";
        }
    }

    std::string AVOutputTV::getWBControlStringFromEnum(tvWBControl_t value) {
        switch(value)
        {
            case tvWB_CONTROL_GAIN: return "Gain";
            case tvWB_CONTROL_OFFSET: return "Offset";
            default: return "Max";
        }
    }

    int AVOutputTV::getWBColorEnumFromString(std::string color,tvWBColor_t& value) {
        int ret = 0;
	
        if( color.compare("Red") == 0 )
	        value = tvWB_COLOR_RED;
	    else if( color.compare("Green") == 0 )
	        value = tvWB_COLOR_GREEN;
        else if( color.compare("Blue") == 0 )
	        value = tvWB_COLOR_BLUE;
        else
	        ret = -1;
	
	    return ret;
    }

    int AVOutputTV::getWBControlEnumFromString(std::string color,tvWBControl_t& value) {
        int ret = 0;
	
        if( color.compare("Gain") == 0 )
	        value = tvWB_CONTROL_GAIN;
	    else if( color.compare("Offset") == 0 )
	        value = tvWB_CONTROL_OFFSET;
        else
	        ret = -1;
	
	    return ret;
    }

    std::string  AVOutputTV::getColorTemperatureStringFromEnum(tvColorTemp_t value) {
        switch(value)  {
            case tvColorTemp_STANDARD: return "Standard";
            case tvColorTemp_WARM: return "Warm";
            case tvColorTemp_COLD: return "Cold";
		    case tvColorTemp_USER : return "UserDefined";
            default : return "Max";
        }
    }

    int AVOutputTV:: validateCMSParameter(std::string component,int inputValue)
    {
        capVectors_t info;
        tvError_t ret = getParamsCaps("CMS", info);

        LOGINFO("%s : component : %s inputValue : %d\n",__FUNCTION__,component.c_str(),inputValue);

        if (ret != tvERROR_NONE) {
            LOGERR("Failed to fetch the range capability \n");
            return -1;
        }
	
	    if( component == "Saturation" ) {
	        if (inputValue < stoi(info.rangeVector[0]) || inputValue > std::stoi(info.rangeVector[1])) {
                LOGERR("wrong Input value[%d] for %s\n", inputValue,component.c_str());
                return -1;
		    }
	    } else if ( component == "Hue" ) {
	        if (inputValue < stoi(info.rangeVector[2]) || inputValue > std::stoi(info.rangeVector[3])) {
                LOGERR("wrong Input value[%d] for %s\n", inputValue,component.c_str());
                return -1;
		    }
    	} else if ( component == "Luma" ) {
	        if (inputValue < stoi(info.rangeVector[4]) || inputValue > std::stoi(info.rangeVector[5])) {
                LOGERR("wrong Input value[%d] for %s\n", inputValue,component.c_str());
                return -1;
		    }
	    }
        return 0;
    }

    int AVOutputTV:: validateWBParameter(std::string param,std::string control,int inputValue)
    {
        capVectors_t info;
        tvError_t ret = getParamsCaps(param, info);

        if (ret != tvERROR_NONE) {
            LOGERR("Failed to fetch the range capability[%s] \n", param.c_str());
            return -1;
        }
	
	    if( param == "Gain" ) {
	        if (inputValue < stoi(info.rangeVector[0]) || inputValue > std::stoi(info.rangeVector[1])) {
                LOGERR("wrong Input value[%d] for %s\n", inputValue,control.c_str());
                return -1;
		    }
	    } else if ( param == "Offset" ) {
	        if (inputValue < stoi(info.rangeVector[2]) || inputValue > std::stoi(info.rangeVector[3])) {
                LOGERR("wrong Input value[%d] for %s\n", inputValue,control.c_str());
                return -1;
		    }
        }
        return 0;
    }

    int AVOutputTV::ReadCapablitiesFromConfODM(std::string param, capDetails_t& info)
    {
        int ret = 0;

        /*Consider User WhiteBalance as CustomWhiteBalance
        To avoid clash with Factory WhiteBalance Calibration capablities*/

        if ( param == "WhiteBalance")
            param = "CustomWhiteBalance";

        try {
            CIniFile inFile(CAPABLITY_FILE_NAME);
            std::string configString;

            if(param == "CMS")
            {
                configString = param + ".color";
                info.color = inFile.Get<std::string>(configString);

                configString = param + ".component";
                info.component = inFile.Get<std::string>(configString);
            }

            if(param == "CustomWhiteBalance")
            {
                configString = param + ".color";
                info.color = inFile.Get<std::string>(configString);

                configString = param + ".control";
                info.control = inFile.Get<std::string>(configString);

            }

            if ((param == "DolbyVisionMode") || (param == "Backlight") || (param == "CMS") || (param == "CustomWhiteBalance") || (param == "HDRMode") ) {
                configString = param + ".platformsupport";
                info.isPlatformSupport = inFile.Get<std::string>(configString);
                printf(" platformsupport : %s\n",info.isPlatformSupport.c_str() );
            }

            if ( (param == "ColorTemperature") || (param == "DimmingMode") ||
                 ( param == "AutoBacklightControl") || (param == "DolbyVisionMode") ||
                 (param == "HDR10Mode") || (param == "HLGMode") || (param == "AspectRatio") ||
                 (param == "PictureMode") || (param == "VideoSource") || (param == "VideoFormat") ||
                 (param == "VideoFrameRate") || (param == "HDRMode") ) {
                configString =  param + ".range";
                info.range = inFile.Get<std::string>(configString);
                printf(" String Range info : %s\n",info.range.c_str() );
            } else if ( (param == "CMS" ))  {
                configString.clear();
                configString = param + ".range_Saturation_from";
                info.range = inFile.Get<std::string>(configString);
                configString = param + ".range_Saturation_to";
                info.range += ","+inFile.Get<std::string>(configString);

                configString = param + ".range_Hue_from";
                info.range += ","+inFile.Get<std::string>(configString);
                configString = param + ".range_Hue_to";
                info.range += ","+inFile.Get<std::string>(configString);

                configString = param + ".range_Luma_from";
                info.range += ","+inFile.Get<std::string>(configString);
                configString = param + ".range_Luma_to";
                info.range += ","+inFile.Get<std::string>(configString);
            } else if ( (param == "CustomWhiteBalance")) {
                configString = param + ".range_Gain_from";
                info.range = inFile.Get<std::string>(configString);
                configString = param + ".range_Gain_to";
                info.range += ","+inFile.Get<std::string>(configString);

                configString = param + ".range_Offset_from";
                info.range += ","+inFile.Get<std::string>(configString);
                configString = param + ".range_Offset_to";
                info.range += ","+inFile.Get<std::string>(configString);
            } else {
                configString = param + ".range_from";
                info.range = inFile.Get<std::string>(configString);
                configString = param + ".range_to";
                info.range += ","+inFile.Get<std::string>(configString);
                printf(" Integer Range Info : %s\n",info.range.c_str() );
            }

            if ((param == "VideoSource") || (param == "PictureMode") || (param == "VideoFormat") ) {
                configString.clear();
                configString = param + ".index";
                info.index = inFile.Get<std::string>(configString);
                printf("Index value %s\n", info.index.c_str());
            }

            configString.clear();
            configString = param + ".pqmode";
            info.pqmode = inFile.Get<std::string>(configString);
            configString = param + ".format";
            info.format = inFile.Get<std::string>(configString);
            configString = param + ".source";
            info.source = inFile.Get<std::string>(configString);
            ret = 0;
        }
        catch(const boost::property_tree::ptree_error &e) {
            printf("%s: error %s::config table entry not found in ini file\n",__FUNCTION__,e.what());
            ret = -1;
        }
        return ret;
   }

} //namespace Plugin
} //namespace WPEFramework
