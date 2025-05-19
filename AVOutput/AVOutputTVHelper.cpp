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
            GetCurrentVideoSource(&currentSource);
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
        tvDolbyMode_t dolbyModes[tvMode_Max];
        tvDolbyMode_t *dolbyModesPtr[tvMode_Max]={0};
        unsigned short totalAvailable = 0;

        for (int i = 0; i < tvMode_Max; i++)
        {
            dolbyModesPtr[i] = &dolbyModes[i];
        }

        // Set an initial value to indicate the mode type
        dolbyModes[0] = tvDolbyMode_Dark;

        tvError_t ret = GetTVSupportedDolbyVisionModes(dolbyModesPtr, &totalAvailable);
        if (ret == tvERROR_NONE) {
            for (int count = 0; count < totalAvailable; count++) {
		        if(strncasecmp(dolbyMode, getDolbyModeStringFromEnum(dolbyModes[count]).c_str(), strlen(dolbyMode))==0) {
                    mode = dolbyModes[count];
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

        //GetCurrentVideoSource
        retVal = GetCurrentVideoSource(&sourceIndex);
        if(retVal != tvERROR_NONE) {
            LOGERR("%s : GetCurrentVideoSource( ) Failed\n",__FUNCTION__);
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
	streamVector.push_back({std::stringstream(stringInfo.control), vectorInfo.controlVector});

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


        if( ReadCapablitiesFromConf( param, paramInfo ) != 0 ) {
            LOGINFO( "%s: readCapablitiesFromConf Failed !!!\n",__FUNCTION__);
            return false;
        }

        if( param == "CMS")
        {
            // Check color
            if (! checkCMSColorAndComponentCapability(paramInfo.color, inputInfo.color)) {
                LOGINFO( "%s:CMS color Capablity Failed CapColor : %s inputColor : %s!!!\n",__FUNCTION__,paramInfo.color.c_str(), inputInfo.color.c_str());
                return false;
            }

            // Check component
            if (! checkCMSColorAndComponentCapability(paramInfo.component, inputInfo.component)) {
                LOGINFO( "%s:CMS component Capablity capComponent : %s inputComponent : %s Failed!!!.\n",__FUNCTION__,paramInfo.component.c_str(), inputInfo.component.c_str());
                return false;
            }
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
            int val=GetPanelID(panelId);
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
        GetCurrentVideoSource(&current_source);

        tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
        tr181_param_name += "."+convertSourceIndexToStringV2(current_source)+"."+"Format."+convertVideoFormatToStringV2(current_format)+"."+"PictureModeString";
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
            tvError_t ret = GetCurrentVideoSource(&currentSource);

            if(ret != tvERROR_NONE) {
                LOGWARN("%s: GetCurrentVideoSource( ) Failed \n",__FUNCTION__);
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

    tvError_t AVOutputTV::updateAVoutputTVParamToHALV2(std::string forParam, paramIndex_t indexInfo, int value, bool setNotDelete)
    {
        tvError_t ret = tvERROR_NONE;
        std::string key;

        // Generate storage key based on parameter type
        if (forParam == "CMS")
            generateStorageIdentifierCMSV2(key, forParam, indexInfo);
        else if (forParam == "WhiteBalance")
            generateStorageIdentifierWBV2(key, forParam, indexInfo);
        else
            generateStorageIdentifierV2(key, forParam, indexInfo);

        if (key.empty()) {
            LOGERR("%s generateStorageIdentifier failed\n", __FUNCTION__);
            return tvERROR_GENERAL;
        }

        tr181ErrorCode_t err = tr181Success;

        if (setNotDelete) {
            std::string toStore = std::to_string(value);

            // Map parameters to their string transformation logic (if applicable)
            std::map<std::string, std::function<void(int, std::string&)>> fnMap = {
                {"ColorTemp", [this](int v, std::string& s) { getColorTempStringFromEnum(v, s); }},
                {"DimmingMode", [this](int v, std::string& s) { getDimmingModeStringFromEnum(v, s); }},
                {"AspectRatio", [this](int v, std::string& s) { getDisplayModeStringFromEnum(v, s); }},
                {"AutoBacklightMode", [this](int v, std::string& s) { getBacklightModeStringFromEnum(v, s); }}
            };

            // If there's a custom string conversion for this parameter, apply it
            auto it = fnMap.find(forParam);
            if (it != fnMap.end()) {
                it->second(value, toStore);
            }
            // Set the value using TR-181
            err = setLocalParam(rfc_caller_id, key.c_str(), toStore.c_str());
        }
        else
        {
            // Delete the value using TR-181
            err = clearLocalParam(rfc_caller_id, key.c_str());
        }

        if (err != tr181Success) {
            LOGERR("%s: %s for %s Failed : %s\n",__FUNCTION__, setNotDelete ? "Set" : "Delete", key.c_str(), getTR181ErrorString(err));
            ret = tvERROR_GENERAL;
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
            LOGERR("%s generateStorageIdentifierDirty failed\n", __FUNCTION__);
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
            else
            {
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
                            case PQ_PARAM_WB_GAIN_RED:
                            case PQ_PARAM_WB_GAIN_GREEN:
                            case PQ_PARAM_WB_GAIN_BLUE:
                            case PQ_PARAM_WB_OFFSET_RED:
                            case PQ_PARAM_WB_OFFSET_GREEN:
                            case PQ_PARAM_WB_OFFSET_BLUE:
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

    tvError_t AVOutputTV::syncAvoutputTVParamsToHAL(std::string pqmode, std::string source, std::string format)
    {
        int level = {0};
        capDetails_t info;
        info.pqmode = pqmode;
        info.source = source;
        info.format = format;

        JsonObject paramJson;
        paramJson["pictureMode"] = info.pqmode;
        paramJson["videoSource"] = info.source;
        paramJson["videoFormat"] = info.format;
        LOGINFO("Entry %s : pqmode : %s source : %s format : %s\n", __FUNCTION__, pqmode.c_str(), source.c_str(), format.c_str());

        // Brightness
        m_brightnessStatus = GetBrightnessCaps(&m_maxBrightness, &m_brightnessCaps);
        LOGINFO("GetBrightnessCaps returned status: %d, max: %d", m_brightnessStatus, m_maxBrightness);
        if (m_brightnessStatus == tvERROR_OPERATION_NOT_SUPPORTED) {
            updateAVoutputTVParam("sync", "Brightness", info, PQ_PARAM_BRIGHTNESS, level);
        } else {
            updateAVoutputTVParamV2("sync", "Brightness", paramJson, PQ_PARAM_BRIGHTNESS,level);
        }

        // Contrast
        m_contrastStatus = GetContrastCaps(&m_maxContrast, &m_contrastCaps);
        LOGINFO("GetContrastCaps returned status: %d, max: %d", m_contrastStatus, m_maxContrast);
        if (m_contrastStatus == tvERROR_OPERATION_NOT_SUPPORTED) {
            updateAVoutputTVParam("sync", "Contrast", info, PQ_PARAM_CONTRAST, level);
        } else {
            updateAVoutputTVParamV2("sync", "Contrast", paramJson, PQ_PARAM_CONTRAST,level);
        }

        // Sharpness
        m_sharpnessStatus = GetSharpnessCaps(&m_maxSharpness, &m_sharpnessCaps);
        LOGINFO("GetSharpnessCaps returned status: %d, max: %d", m_sharpnessStatus, m_maxSharpness);
        if (m_sharpnessStatus == tvERROR_OPERATION_NOT_SUPPORTED) {
            updateAVoutputTVParam("sync", "Sharpness", info, PQ_PARAM_SHARPNESS, level);
        } else {
            updateAVoutputTVParamV2("sync", "Sharpness", paramJson, PQ_PARAM_SHARPNESS, level);
        }

        // Saturation
        m_saturationStatus = GetSaturationCaps(&m_maxSaturation, &m_saturationCaps);
        LOGINFO("GetSaturationCaps returned status: %d, max: %d", m_saturationStatus, m_maxSaturation);
        if (m_saturationStatus == tvERROR_OPERATION_NOT_SUPPORTED) {
            updateAVoutputTVParam("sync", "Saturation", info, PQ_PARAM_SATURATION, level);
        } else {
            updateAVoutputTVParamV2("sync", "Saturation", paramJson, PQ_PARAM_SATURATION,level);
        }

        // Hue
        m_hueStatus = GetHueCaps(&m_maxHue, &m_hueCaps);
        LOGINFO("GetHueCaps returned status: %d, max: %d", m_hueStatus, m_maxHue);
        if (m_hueStatus == tvERROR_OPERATION_NOT_SUPPORTED) {
            updateAVoutputTVParam("sync", "Hue", info, PQ_PARAM_HUE, level);
        } else {
            updateAVoutputTVParamV2("sync", "Hue", paramJson, PQ_PARAM_HUE, level);
        }

        // ColorTemperature
        m_colorTempStatus = GetColorTemperatureCaps(&m_colortemp, &m_numColortemp, &m_colortempCaps);
        LOGINFO("GetColorTemperatureCaps returned status: %d, numColortemp: %d", m_colorTempStatus, m_numColortemp);
        if (m_colorTempStatus == tvERROR_OPERATION_NOT_SUPPORTED) {
            updateAVoutputTVParam("sync", "ColorTemp", info, PQ_PARAM_COLOR_TEMPERATURE, level);
        } else {
            updateAVoutputTVParamV2("sync", "ColorTemp", paramJson, PQ_PARAM_COLOR_TEMPERATURE,level);
        }

        // HDRMode
        updateAVoutputTVParam("sync", "HDRMode", info, PQ_PARAM_DOLBY_MODE, level);

        // DimmingMode
        m_dimmingModeStatus = GetTVDimmingModeCaps(&m_dimmingModes, &m_numdimmingModes, &m_dimmingModeCaps);
        LOGINFO("GetTVDimmingModeCaps returned status: %d, numdimmingModes: %d", m_dimmingModeStatus, m_numdimmingModes);
        if (m_dimmingModeStatus == tvERROR_OPERATION_NOT_SUPPORTED) {
            updateAVoutputTVParam("sync", "DimmingMode", info, PQ_PARAM_DIMMINGMODE, level);
        } else {
            updateAVoutputTVParamV2("sync", "DimmingMode", paramJson, PQ_PARAM_DIMMINGMODE,level);
        }

        // Backlight
        LOGINFO("Calling GetBacklightCaps...");
        m_backlightStatus = GetBacklightCaps(&m_maxBacklight, &m_backlightCaps);
        LOGINFO("GetBacklightCaps returned status: %d, maxBacklight: %d", m_backlightStatus, m_maxBacklight);
#if DEBUG
        if (m_backlightCaps)
        {
            LOGINFO("Backlight caps pointer is valid. Num contexts: %zu", m_backlightCaps->num_contexts);
            for (size_t i = 0; i < m_backlightCaps->num_contexts; ++i) {
                const auto& context = m_backlightCaps->contexts[i];
                std::string pqModeStr = AVOutputTV::pqModeMap.count(context.pq_mode) ?
                                        AVOutputTV::pqModeMap.at(context.pq_mode) : "Unknown";
                std::string formatStr = AVOutputTV::videoFormatMap.count(context.videoFormatType) ?
                                        AVOutputTV::videoFormatMap.at(context.videoFormatType) : "Unknown";
                std::string srcStr = AVOutputTV::videoSrcMap.count(context.videoSrcType) ?
                                     AVOutputTV::videoSrcMap.at(context.videoSrcType) : "Unknown";
                LOGINFO("Context[%zu]: PQMode = %s (%d), Format = %s (%d), Source = %s (%d)",
                        i, pqModeStr.c_str(), context.pq_mode,
                        formatStr.c_str(), context.videoFormatType,
                        srcStr.c_str(), context.videoSrcType);
            }
        } else {
            LOGWARN("Backlight caps pointer is null.");
        }
#endif
        if (m_backlightStatus == tvERROR_OPERATION_NOT_SUPPORTED) {
            updateAVoutputTVParam("sync", "Backlight", info, PQ_PARAM_BACKLIGHT, level);
        } else {
            updateAVoutputTVParamV2("sync", "Backlight", paramJson, PQ_PARAM_BACKLIGHT, level);
        }

        //Ambient Bakclight Mode
        m_backlightModeStatus = GetBacklightModeCaps(&m_backlightModes, &m_numBacklightModes, &m_backlightModeCaps);
        //AspectRatio
        m_aspectRatioStatus = GetAspectRatioCaps(&m_aspectRatio, &m_numAspectRatio, &m_aspectRatioCaps);
        //LowLatencyState
        m_lowLatencyStateStatus = GetLowLatencyStateCaps(&m_maxlowLatencyState, &m_lowLatencyStateCaps);
        // PrecisionDetail
        m_precisionDetailStatus = GetPrecisionDetailCaps(&m_maxPrecisionDetail, &m_precisionDetailCaps);
        //PictureMode
        m_pictureModeStatus = GetTVPictureModeCaps(&m_pictureModes, &m_numPictureModes, &m_pictureModeCaps);

        // LocalContrastEnhancement
        m_localContrastEnhancementStatus = GetLocalContrastEnhancementCaps(&m_maxLocalContrastEnhancement, &m_localContrastEnhancementCaps);
        if (m_localContrastEnhancementStatus == tvERROR_NONE) {
            updateAVoutputTVParamV2("sync", "LocalContrastEnhancement", paramJson, PQ_PARAM_LOCAL_CONTRAST_ENHANCEMENT, level);
        }

        // MPEGNoiseReduction
        m_MPEGNoiseReductionStatus = GetMPEGNoiseReductionCaps(&m_maxMPEGNoiseReduction, &m_MPEGNoiseReductionCaps);
        if (m_MPEGNoiseReductionStatus == tvERROR_NONE) {
            updateAVoutputTVParamV2("sync", "MPEGNoiseReduction", paramJson, PQ_PARAM_MPEG_NOISE_REDUCTION, level);
        }

        // DigitalNoiseReduction
        m_digitalNoiseReductionStatus = GetDigitalNoiseReductionCaps(&m_maxDigitalNoiseReduction, &m_digitalNoiseReductionCaps);
        if (m_digitalNoiseReductionStatus == tvERROR_NONE) {
            updateAVoutputTVParamV2("sync", "DigitalNoiseReduction", paramJson, PQ_PARAM_DIGITAL_NOISE_REDUCTION, level);
        }

        // AISuperResolution
        m_AISuperResolutionStatus = GetAISuperResolutionCaps(&m_maxAISuperResolution, &m_AISuperResolutionCaps);
        if (m_AISuperResolutionStatus == tvERROR_NONE) {
            updateAVoutputTVParamV2("sync", "AISuperResolution", paramJson, PQ_PARAM_AI_SUPER_RESOLUTION,level);
        }

        // MEMC
        m_MEMCStatus = GetMEMCCaps(&m_maxMEMC, &m_MEMCCaps);
        if (m_MEMCStatus == tvERROR_NONE) {
            updateAVoutputTVParamV2("sync", "MEMC", paramJson, PQ_PARAM_MEMC, level);
        }
        // Sync CMS and WB
        syncCMSParams();
        syncWBParams();

        // Dolby Vision Mode
        info.format = "DV"; // Sync only for Dolby
        updateAVoutputTVParam("sync", "DolbyVisionMode", info, PQ_PARAM_DOLBY_MODE, level);

        LOGINFO("Exit %s : pqmode : %s source : %s format : %s\n", __FUNCTION__, pqmode.c_str(), source.c_str(), format.c_str());
        return tvERROR_NONE;
    }
    int AVOutputTV::syncAvoutputTVPQModeParamsToHALV2(std::string pqmode, std::string source, std::string format)
    {
        tr181ErrorCode_t err = tr181Success;
        TR181_ParamData_t param = {0};
        bool contextSynced = false;

        // Treat "none" as "Global"
        if (source == "none")
            source = "Global";
        if (format == "none")
            format = "Global";

        // Handle "Current" source/format substitution
        if (source == "Current" || format == "Current") {
            tvVideoSrcType_t currentSrc = VIDEO_SOURCE_IP;
            tvVideoFormatType_t currentFmt = VIDEO_FORMAT_SDR;
            GetCurrentVideoSource(&currentSrc);
            GetCurrentVideoFormat(&currentFmt);
            if (currentFmt == VIDEO_FORMAT_NONE)
                currentFmt = VIDEO_FORMAT_SDR;

            if (source == "Current")
                source = convertSourceIndexToStringV2(currentSrc);
            if (format == "Current")
                format = convertVideoFormatToStringV2(currentFmt);
        }
        if (m_pictureModeStatus == tvERROR_NONE)
        {
            for (size_t i = 0; i < m_pictureModeCaps->num_contexts; ++i) {
                const tvConfigContext_t& ctx = m_pictureModeCaps->contexts[i];

                std::string sourceStr = convertSourceIndexToStringV2(ctx.videoSrcType);
                std::string formatStr = convertVideoFormatToStringV2(ctx.videoFormatType);

                // Filter by provided source/format
                if (source != "Global" && source != sourceStr)
                    continue;
                if (format != "Global" && format != formatStr)
                    continue;

                std::string tr181Param = std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM) +
                                        "." + sourceStr + ".Format." + formatStr + ".PictureModeString";

                err = getLocalParam(rfc_caller_id, tr181Param.c_str(), &param);
                if (err != tr181Success) {
                    LOGWARN("Failed to getLocalParam for %s\n", tr181Param.c_str());
                    continue;
                }

                std::string modeStr = param.value;
                int modeIndex = -1;
                for (size_t i = 0; i < m_numPictureModes; ++i) {
                    if (pqModeMap.at(m_pictureModes[i]) == modeStr) {
                        modeIndex = static_cast<int>(i);
                        break;
                    }
                }
#if DEBUG
                LOGINFO("Got mode string from TR181: %s -> index=%d", modeStr.c_str(), modeIndex);
#endif
                tvError_t tv_err = SaveSourcePictureMode(ctx.videoSrcType, ctx.videoFormatType, modeIndex);
                if (tv_err != tvERROR_NONE) {
                    LOGWARN("Failed SaveSourcePictureMode for %s / %s\n", sourceStr.c_str(), formatStr.c_str());
                    continue;
                }

                contextSynced = true;
            }

            if (!contextSynced) {
                LOGWARN("No matching context synced for pqmode=%s source=%s format=%s\n",
                        pqmode.c_str(), source.c_str(), format.c_str());
                return -1;
            }
            return 0;
        }
        return -1;
    }

    int AVOutputTV::syncAvoutputTVPQModeParamsToHAL(std::string pqmode, std::string source, std::string format)
    {
        if (m_pictureModeStatus == tvERROR_OPERATION_NOT_SUPPORTED)
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
        else
        {
            return syncAvoutputTVPQModeParamsToHALV2(pqmode,source,format);
        }
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

    tvError_t AVOutputTV::getParamsCaps(std::string param, capVectors_t &vecInfo)
    {
        tvError_t ret = tvERROR_NONE;
        capDetails_t stringInfo;
    
        if( ReadCapablitiesFromConf( param, stringInfo) != 0 )
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

    int AVOutputTV::GetPanelID(char *panelId)
    {
        if (panelId == NULL) {
            printf("Invalid buffer provided for panel ID\n");
            return -1;
        }

        const char *command = "/usr/bin/panelIDConfig -i";
        FILE *fp;

        // Execute the binary
        fp = popen(command, "r");
        if (fp == NULL) {
            printf("Failed to execute command: %s\n", command);
            return -1;
        }

        // Read the panel ID from the binary's output
        if (fgets(panelId, 20, fp) != NULL) {
            size_t len = strlen(panelId);
            if (len > 0 && panelId[len - 1] == '\n') {
                panelId[len - 1] = '\0';
            }
        } else {
            printf("Failed to read panel ID from panelIDConfig binary\n");
            pclose(fp);
            return -1;
        }

        pclose(fp);
        return 0;
    }

    int AVOutputTV::ConvertHDRFormatToContentFormat(tvhdr_type_t hdrFormat)
    {
        int ret=tvContentFormatType_SDR;
        switch(hdrFormat)
        {
            case HDR_TYPE_SDR:
                ret=tvContentFormatType_SDR;
                break;
            case HDR_TYPE_HDR10:
                ret=tvContentFormatType_HDR10;
                break;
            case HDR_TYPE_HDR10PLUS:
                ret=tvContentFormatType_HDR10PLUS;
                break;
            case HDR_TYPE_DOVI:
                ret=tvContentFormatType_DOVI;
                break;
            case HDR_TYPE_HLG:
                ret=tvContentFormatType_HLG;
                break;
            default:
                break;
        }
        return ret;
    }

    void AVOutputTV::getDimmingModeStringFromEnum(int value, std::string &toStore)
    {
        const char *dimmingmode_string[] = {
                    [tvDimmingMode_Fixed] = "fixed",
                    [tvDimmingMode_Local] = "local",
                    [tvDimmingMode_Global] = "global",
                };
        toStore.clear();
        toStore+=dimmingmode_string[value];
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
    void AVOutputTV::getDisplayModeStringFromEnum(int value, std::string &toStore)
    {
        static const char* display_mode_string[] = {
            [tvDisplayMode_4x3]    = "TV 4X3 PILLARBOX",
            [tvDisplayMode_16x9]   = "TV 16X9 STRETCH",
            [tvDisplayMode_FULL]   = "TV FULL",
            [tvDisplayMode_NORMAL] = "TV NORMAL",
            [tvDisplayMode_AUTO]   = "TV AUTO",
            [tvDisplayMode_DIRECT] = "TV DIRECT",
            [tvDisplayMode_ZOOM]   = "TV ZOOM"
        };

        toStore.clear();
        if (value >= 0 && value < tvDisplayMode_MAX && display_mode_string[value]) {
            toStore += display_mode_string[value];
        } else {
            toStore += "TV AUTO";
        }
    }

    void AVOutputTV::getBacklightModeStringFromEnum(int value, std::string& toStore)
    {
        toStore.clear();
        switch (static_cast<tvBacklightMode_t>(value)) {
            case tvBacklightMode_MANUAL:
                toStore = "Manual";
                break;
            case tvBacklightMode_AMBIENT:
                toStore = "Ambient";
                break;
            case tvBacklightMode_ECO:
                toStore = "Eco";
                break;
            default:
                toStore = "Unknown";
                break;
        }
    }

    int AVOutputTV::getCurrentPictureMode(char *picMode)
    {
        tvError_t  ret = tvERROR_NONE;
        TR181_ParamData_t param;
        std::string tr181_param_name;
        tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;

        ret = GetCurrentVideoSource(&currentSource);
        if(ret != tvERROR_NONE) {
            LOGERR("GetCurrentVideoSource() Failed set source to default\n");
            return 0;
        }

        tvVideoFormatType_t current_format = VIDEO_FORMAT_NONE;
        GetCurrentVideoFormat(&current_format);
        if ( current_format  == VIDEO_FORMAT_NONE) {
	    current_format  = VIDEO_FORMAT_SDR;
	}

        tr181_param_name += std::string(AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM);
        tr181_param_name += "." + convertSourceIndexToStringV2(currentSource) + "." + "Format."+convertVideoFormatToStringV2(current_format)+"."+"PictureModeString";

        memset(&param, 0, sizeof(param));

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, tr181_param_name.c_str(), &param);
        if ( err == tr181Success ) {
            strncpy(picMode, param.value, strlen(param.value)+1);
            //LOGINFO("getLocalParam success, mode = %s\n", picMode);
            return 1;
        }
        else {
            LOGERR("getLocalParam failed %s\n",tr181_param_name.c_str());
            return 0;
        }
    }
    
    tvDolbyMode_t AVOutputTV::GetDolbyVisionEnumFromModeString(const char* modeString)
    {
        if (strcmp(modeString, "Invalid") == 0) {
            return tvDolbyMode_Invalid;
        } else if (strcmp(modeString, "Dark") == 0) {
            return tvDolbyMode_Dark;
        } else if (strcmp(modeString, "Bright") == 0) {
            return tvDolbyMode_Bright;
        } else if (strcmp(modeString, "Game") == 0) {
            return tvDolbyMode_Game;
        }
        return tvDolbyMode_Invalid; // Default case for invalid input
    }

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
        LOGINFO("%s:mode selected is: %d", __FUNCTION__, m_videoZoomMode);
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
            {"GreenOffset", G_POST_OFFSET},
            {"BlueOffset", B_POST_OFFSET}
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
            {"RedGain", PQ_PARAM_WB_GAIN_RED},
            {"RedOffset", PQ_PARAM_WB_OFFSET_RED},
            {"GreenGain", PQ_PARAM_WB_GAIN_GREEN},
            {"GreenOffset", PQ_PARAM_WB_OFFSET_GREEN},
            {"BlueGain", PQ_PARAM_WB_GAIN_BLUE},
            {"BlueOffset", PQ_PARAM_WB_OFFSET_BLUE},
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
            default : return "Standard";
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
	
        if( control == "Gain" ) {
            if (inputValue < stoi(info.rangeVector[0]) || inputValue > std::stoi(info.rangeVector[1])) {
                LOGERR("wrong Input value[%d] for %s\n", inputValue,control.c_str());
                return -1;
            }
        } else if ( control == "Offset" ) {
            if (inputValue < stoi(info.rangeVector[2]) || inputValue > std::stoi(info.rangeVector[3])) {
                LOGERR("wrong Input value[%d] for %s\n", inputValue,control.c_str());
                return -1;
	    }
        }
        return 0;
    }
//JSON Based V2 Helpers
    const std::map<int, std::string> AVOutputTV::pqModeMap = {
        {PQ_MODE_SPORTS, "Sports"},
        {PQ_MODE_THEATER, "Theater"},
        {PQ_MODE_GAME, "Game"},
        {PQ_MODE_IQ, "IQ"},
        {PQ_MODE_DARK, "Dark"},
        {PQ_MODE_BRIGHT, "Bright"},
        {PQ_MODE_AIPQ, "AI PQ"},
        {PQ_MODE_STANDARD, "Standard"},
        {PQ_MODE_VIVID, "Vivid"},
        {PQ_MODE_ENERGY_SAVING, "EnergySaving"},
        {PQ_MODE_CUSTOM, "Custom"}
    };

    const std::map<int, std::string> AVOutputTV::videoFormatMap = {
        {VIDEO_FORMAT_NONE, "None"},
        {VIDEO_FORMAT_SDR, "SDR"},
        {VIDEO_FORMAT_HDR10, "HDR10"},
        {VIDEO_FORMAT_HDR10PLUS, "HDR10Plus"},
        {VIDEO_FORMAT_DV, "DV"},
        {VIDEO_FORMAT_HLG, "HLG"}
    };

    const std::map<int, std::string> AVOutputTV::videoSrcMap = {
        {VIDEO_SOURCE_COMPOSITE1, "Composite"},
        {VIDEO_SOURCE_HDMI1, "HDMI1"},
        {VIDEO_SOURCE_HDMI2, "HDMI2"},
        {VIDEO_SOURCE_HDMI3, "HDMI3"},
        {VIDEO_SOURCE_HDMI4, "HDMI4"},
        {VIDEO_SOURCE_IP, "IP"},
        {VIDEO_SOURCE_TUNER, "Tuner"}
    };
    const std::unordered_map<std::string, tvBacklightMode_t> AVOutputTV::backlightModeMap = {
        { "Manual",  tvBacklightMode_MANUAL },
        { "Ambient", tvBacklightMode_AMBIENT },
        { "Eco",     tvBacklightMode_ECO }
    };

    const std::map<std::string, int> AVOutputTV::pqModeReverseMap = []{
        std::map<std::string, int> m;
        for (const auto& pair : AVOutputTV::pqModeMap) m[pair.second] = pair.first;
        return m;
    }();

    const std::map<std::string, int> AVOutputTV::videoFormatReverseMap = []{
        std::map<std::string, int> m;
        for (const auto& pair : AVOutputTV::videoFormatMap) m[pair.second] = pair.first;
        return m;
    }();

    const std::map<std::string, int> AVOutputTV::videoSrcReverseMap = []{
        std::map<std::string, int> m;
        for (const auto& pair : AVOutputTV::videoSrcMap) m[pair.second] = pair.first;
        return m;
    }();

    std::string AVOutputTV::convertSourceIndexToStringV2(int source) {
        auto it = videoSrcMap.find(source);
        return (it != videoSrcMap.end()) ? it->second : "";
    }

    std::string AVOutputTV::convertVideoFormatToStringV2(int format) {
        auto it = videoFormatMap.find(format);
        return (it != videoFormatMap.end()) ? it->second : "";
    }

    std::string AVOutputTV::convertPictureIndexToStringV2(int pqmode) {
        auto it = pqModeMap.find(pqmode);
        return (it != pqModeMap.end()) ? it->second : "";
    }
    uint32_t AVOutputTV::generateStorageIdentifierV2(std::string &key, std::string forParam, paramIndex_t info)
    {
        key += AVOUTPUT_GENERIC_STRING_RFC_PARAM;
        key += STRING_SOURCE + convertSourceIndexToStringV2(info.sourceIndex) + "." +
            STRING_PICMODE + convertPictureIndexToStringV2(info.pqmodeIndex) + "." +
            STRING_FORMAT + convertVideoFormatToStringV2(info.formatIndex) + "." +
            forParam;
        return tvERROR_NONE;
    }

    bool AVOutputTV::isValidSource(const std::vector<std::string>& sourceArray, tvVideoSrcType_t sourceIndex)
    {
        // If "Current" is passed, match the current source
        if (std::find(sourceArray.begin(), sourceArray.end(), "Current") != sourceArray.end()) {
            tvVideoSrcType_t currentSource = VIDEO_SOURCE_IP;
            GetCurrentVideoSource(&currentSource);
            return (sourceIndex == currentSource);
        }

        // Match against specific source strings
        const std::string srcStr = convertSourceIndexToStringV2(sourceIndex);
        return std::find(sourceArray.begin(), sourceArray.end(), srcStr) != sourceArray.end();
    }

    bool AVOutputTV::isValidFormat(const std::vector<std::string>& formatArray, tvVideoFormatType_t formatIndex)
    {
        // If "Current" is passed, match the current format
        if (std::find(formatArray.begin(), formatArray.end(), "Current") != formatArray.end()) {
            tvVideoFormatType_t currentFormat = VIDEO_FORMAT_NONE;
            GetCurrentVideoFormat(&currentFormat);
            return (formatIndex == currentFormat);
        }

        // Match against specific format strings
        const std::string fmtStr = convertVideoFormatToStringV2(formatIndex);
        return std::find(formatArray.begin(), formatArray.end(), fmtStr) != formatArray.end();
    }

    tvConfigContext_t AVOutputTV::getValidContextFromGetParameters(const JsonObject& parameters, const std::string& paramName)
    {
        tvConfigContext_t validContext = {PQ_MODE_INVALID, VIDEO_FORMAT_NONE, VIDEO_SOURCE_ALL};
        // Picture Mode
        std::string pictureModeStr;
        //"Current", empty string, or missing key as a cue to fetch system values
        if (!parameters.HasLabel("pictureMode") ||
            (pictureModeStr = parameters["pictureMode"].String()).empty() ||
            pictureModeStr == "Current")
        {
            char picMode[PIC_MODE_NAME_MAX] = {0};
            getCurrentPictureMode(picMode);
            std::string pictureModeStr(picMode);
            LOGINFO("Current Picture Mode: %s", picMode);
            validContext.pq_mode = static_cast<tvPQModeIndex_t>(
                pqModeReverseMap.count(pictureModeStr) ? pqModeReverseMap.at(pictureModeStr) : PQ_MODE_INVALID
            );
        }
        else
        {
            validContext.pq_mode = static_cast<tvPQModeIndex_t>(
                pqModeReverseMap.count(pictureModeStr) ? pqModeReverseMap.at(pictureModeStr) : PQ_MODE_INVALID
            );
        }
        // Video Format
        std::string videoFormatStr;
        if (!parameters.HasLabel("videoFormat") ||
            (videoFormatStr = parameters["videoFormat"].String()).empty() ||
            videoFormatStr == "Current")
        {
            GetCurrentVideoFormat(&validContext.videoFormatType);
        }
        else
        {
            validContext.videoFormatType = static_cast<tvVideoFormatType_t>(
                videoFormatReverseMap.count(videoFormatStr) ? videoFormatReverseMap.at(videoFormatStr) : VIDEO_FORMAT_NONE
            );
        }
        // Video Source
        std::string videoSourceStr;
        if (!parameters.HasLabel("videoSource") ||
            (videoSourceStr = parameters["videoSource"].String()).empty() ||
            videoSourceStr == "Current")
        {
            GetCurrentVideoSource(&validContext.videoSrcType);
        }
        else
        {
            validContext.videoSrcType = static_cast<tvVideoSrcType_t>(
                videoSrcReverseMap.count(videoSourceStr) ? videoSrcReverseMap.at(videoSourceStr) : VIDEO_SOURCE_ALL
            );
        }
        tvContextCaps_t* caps = getCapsForParam(paramName);
        LOGINFO("Looking for context: PQMode=%d, Format=%d, Source=%d",
            validContext.pq_mode, validContext.videoFormatType, validContext.videoSrcType);
        // Match context if caps exist
        if (caps && caps->num_contexts > 0) {
            for (size_t i = 0; i < caps->num_contexts; ++i) {
                const tvConfigContext_t& available = caps->contexts[i];
#if DEBUG
                LOGINFO("Context[%zu]: PQMode=%d, Format=%d, Source=%d", i,
                    available.pq_mode, available.videoFormatType, available.videoSrcType);
#endif
                if (available.videoSrcType == validContext.videoSrcType &&
                    available.videoFormatType == validContext.videoFormatType &&
                    available.pq_mode == validContext.pq_mode) {
                    return available; // valid context found
                }
            }
        }
        else
        {
            LOGWARN("No valid context found for %s with provided parameters", paramName.c_str());
            validContext = {PQ_MODE_INVALID, VIDEO_FORMAT_NONE, VIDEO_SOURCE_ALL};
        }
        return validContext;
    }

    bool AVOutputTV::isGlobalParam(const JsonArray& arr) {
        return (arr.Length() == 0) ||
            (arr.Length() == 1 && (
                arr[0].String() == "Global" || arr[0].String() == "none"));
    }

    // Modular helper functions
    std::vector<tvPQModeIndex_t> AVOutputTV::extractPQModes(const JsonObject& parameters)
    {
        std::vector<tvPQModeIndex_t> pqModes;
        if (!parameters.HasLabel("pictureMode"))
        {
            return pqModes;
        }
        JsonArray pqmodeArray = parameters["pictureMode"].Array();
        for (uint32_t i = 0; i < pqmodeArray.Length(); ++i) {
            std::string modeStr = pqmodeArray[i].String();
            if (modeStr == "Current") {
                char picMode[PIC_MODE_NAME_MAX] = {0};
                if (getCurrentPictureMode(picMode)) {
                    for (const auto& entry : pqModeMap) {
                        if (entry.second == picMode) {
                            pqModes.push_back(static_cast<tvPQModeIndex_t>(entry.first));
                        }
                    }
                }
            } else {
                for (const auto& entry : pqModeMap) {
                    if (entry.second == modeStr) {
                        pqModes.push_back(static_cast<tvPQModeIndex_t>(entry.first));
                    }
                }
            }
        }
        return pqModes;
    }

    std::vector<tvVideoSrcType_t> AVOutputTV::extractVideoSources(const JsonObject& parameters)
    {
        std::vector<tvVideoSrcType_t> sources;
        if (!parameters.HasLabel("videoSource"))
        {
            return sources;
        }
        JsonArray sourceArray = parameters["videoSource"].Array();
        for (uint32_t i = 0; i < sourceArray.Length(); ++i) {
            std::string srcStr = sourceArray[i].String();
            if (srcStr == "Current") {
                tvVideoSrcType_t sourceIndex = VIDEO_SOURCE_IP;
                if (GetCurrentVideoSource(&sourceIndex) == tvERROR_NONE) {
                    sources.push_back(sourceIndex);
                }
            } else {
                for (const auto& entry : videoSrcMap) {
                    if (entry.second == srcStr) {
                        sources.push_back(static_cast<tvVideoSrcType_t>(entry.first));
                    }
                }
            }
        }
        return sources;
    }

    std::vector<tvVideoFormatType_t> AVOutputTV::extractVideoFormats(const JsonObject& parameters)
    {
        std::vector<tvVideoFormatType_t> formats;
        if (!parameters.HasLabel("videoFormat"))
        {
            return formats;
        }
        JsonArray formatArray = parameters["videoFormat"].Array();
        for (uint32_t i = 0; i < formatArray.Length(); ++i) {
            std::string fmtStr = formatArray[i].String();
            if (fmtStr == "Current") {
                tvVideoFormatType_t formatIndex = VIDEO_FORMAT_NONE;
                GetCurrentVideoFormat(&formatIndex);
                if (formatIndex == VIDEO_FORMAT_NONE) {
                    formatIndex = VIDEO_FORMAT_SDR;
                }
                formats.push_back(formatIndex);
            } else {
                for (const auto& entry : videoFormatMap) {
                    if (entry.second == fmtStr) {
                        formats.push_back(static_cast<tvVideoFormatType_t>(entry.first));
                    }
                }
            }
        }
        return formats;
    }
    JsonArray AVOutputTV::getJsonArrayIfArray(const JsonObject& obj, const std::string& key) {
        return (obj.HasLabel(key.c_str()) && obj[key.c_str()].Content() == JsonValue::type::ARRAY)
            ? obj[key.c_str()].Array()
            : JsonArray(); // returns empty array
    }

    tvContextCaps_t* AVOutputTV::getCapsForParam(const std::string& paramName)
    {
        tvContextCaps_t* caps = nullptr;
        if (paramName == "Backlight") caps = m_backlightCaps;
        else if (paramName == "Brightness") caps = m_brightnessCaps;
        else if (paramName == "Contrast") caps = m_contrastCaps;
        else if (paramName == "Sharpness") caps = m_sharpnessCaps;
        else if (paramName == "Saturation") caps = m_saturationCaps;
        else if (paramName == "Hue") caps = m_hueCaps;
        else if (paramName == "ColorTemp") caps = m_colortempCaps;
        else if (paramName == "DimmingMode") caps = m_dimmingModeCaps;
        else if (paramName == "PictureMode") caps = m_pictureModeCaps;
        else if (paramName == "AspectRatio") caps = m_aspectRatioCaps;
        else if (paramName == "LowLatencyState") caps = m_lowLatencyStateCaps;
        else if (paramName == "PrecisionDetail") caps = m_precisionDetailCaps;
        else if (paramName == "LocalContrastEnhancement") caps = m_localContrastEnhancementCaps;
        else if (paramName == "MPEGNoiseReduction") caps = m_MPEGNoiseReductionCaps;
        else if (paramName == "DigitalNoiseReduction") caps = m_digitalNoiseReductionCaps;
        else if (paramName == "AISuperResolution") caps = m_AISuperResolutionCaps;
        else if (paramName == "MEMC") caps = m_MEMCCaps;
        else if (paramName == "AutoBacklightMode") caps = m_backlightModeCaps;
        else {
            LOGERR("Unknown tr181ParamName: %s", paramName.c_str());
            return nullptr;
        }
        // Fallback to global pictureModeCaps if cap is empty
        if (!caps || caps->num_contexts == 0)
        caps = m_pictureModeCaps;

        return caps;
    }
    std::string AVOutputTV::getCurrentPictureModeAsString() {
        char picMode[PIC_MODE_NAME_MAX] = {0};
        if (!getCurrentPictureMode(picMode)) {
            LOGERR("Failed to get current picture mode");
            return "";
        }
        return picMode;
    }

    std::string AVOutputTV::getCurrentVideoSourceAsString() {
        tvVideoSrcType_t sourceIndex = VIDEO_SOURCE_IP;
        if (GetCurrentVideoSource(&sourceIndex) != tvERROR_NONE) {
            LOGERR("GetCurrentVideoSource failed");
            return "";
        }
        return convertSourceIndexToStringV2(sourceIndex);
    }

    std::string AVOutputTV::getCurrentVideoFormatAsString() {
        tvVideoFormatType_t formatIndex = VIDEO_FORMAT_NONE;
        if (GetCurrentVideoFormat(&formatIndex) != tvERROR_NONE || formatIndex == VIDEO_FORMAT_NONE) {
            formatIndex = VIDEO_FORMAT_SDR;
        }
        return convertVideoFormatToStringV2(formatIndex);
    }

    bool AVOutputTV::isSetRequiredForParam(const JsonObject& parameters)
    {
        // Get current state once
        const std::string curPicMode = getCurrentPictureModeAsString();
        const std::string curSource  = getCurrentVideoSourceAsString();
        const std::string curFormat  = getCurrentVideoFormatAsString();

        // Helper to resolve a parameter to a list of effective values
        auto resolveParam = [&](const std::string& label, const std::string& currentValue) -> std::vector<std::string> {
            std::vector<std::string> result;

            if (!parameters.HasLabel(label.c_str()))
                return { currentValue };

            const auto& array = parameters[label.c_str()].Array();
            if (array.Length() == 0)
                return { currentValue };

            for (uint16_t i = 0; i < array.Length(); ++i) {
                const std::string val = array[i].String();
                if (val == "Current" || val == "Global" || val == "none") {
                    return { currentValue };
                } else {
                    result.push_back(val);
                }
            }
            return result;
        };

        // Resolve all
        const auto resolvedPicModes = resolveParam("pictureMode", curPicMode);
        const auto resolvedFormats  = resolveParam("videoFormat", curFormat);
        const auto resolvedSources  = resolveParam("videoSource", curSource);

        // Check if current combination exists in resolved sets
        for (const auto& pm : resolvedPicModes) {
            if (pm != curPicMode) continue;

            for (const auto& fmt : resolvedFormats) {
                if (fmt != curFormat) continue;

                for (const auto& src : resolvedSources) {
                    if (src == curSource) {
                        // Log the matched combination
                        LOGINFO("isSetRequiredForParam: matched combination - pictureMode: %s, videoFormat: %s, videoSource: %s",
                                pm.c_str(), fmt.c_str(), src.c_str());
                        return true;
                    }
                }
            }
        }

        return false;
    }

    std::vector<tvConfigContext_t> AVOutputTV::getValidContextsFromParameters(const JsonObject& parameters, const std::string& tr181ParamName)
    {
        std::vector<tvConfigContext_t> validContexts;
        tvContextCaps_t* caps = getCapsForParam(tr181ParamName);

        JsonArray pqmodeArray = getJsonArrayIfArray(parameters, "pictureMode");
        JsonArray sourceArray = getJsonArrayIfArray(parameters, "videoSource");
        JsonArray formatArray = getJsonArrayIfArray(parameters, "videoFormat");

        std::vector<tvPQModeIndex_t> pqModes = extractPQModes(parameters);
        std::vector<tvVideoSrcType_t> sources = extractVideoSources(parameters);
        std::vector<tvVideoFormatType_t> formats = extractVideoFormats(parameters);
        // "Global" indicates that the setting applies to all valid entries from the capability list
        if (isGlobalParam(pqmodeArray)) {
            if (caps != nullptr && caps->contexts != nullptr) {
                for (size_t i = 0; i < caps->num_contexts; ++i) {
                    pqModes.push_back(caps->contexts[i].pq_mode);
                }
            } else {
                LOGWARN("Caps or contexts is null while assigning global pqModes.");
            }
        }
        if (isGlobalParam(sourceArray)) {
            if (caps != nullptr && caps->contexts != nullptr) {
                for (size_t i = 0; i < caps->num_contexts; ++i) {
                    sources.push_back(caps->contexts[i].videoSrcType);
                }
            } else {
                LOGWARN("Caps or contexts is null while assigning global sources.");
            }
        }
        if (isGlobalParam(formatArray)) {
            if (caps != nullptr && caps->contexts != nullptr) {
                for (size_t i = 0; i < caps->num_contexts; ++i) {
                    formats.push_back(caps->contexts[i].videoFormatType);
                }
            } else {
                LOGWARN("Caps or contexts is null while assigning global formats.");
            }
        }

        if (pqModes.empty() || sources.empty() || formats.empty()) {
            LOGWARN("One or more parameter vectors are empty: PQModes[%zu], Sources[%zu], Formats[%zu]",
                    pqModes.size(), sources.size(), formats.size());
            return validContexts;
        }

        std::set<std::tuple<int, int, int>> seenContexts;

        for (const auto& pq : pqModes) {
            for (const auto& fmt : formats) {
                for (const auto& src : sources) {
                    tvConfigContext_t testCtx = { pq, fmt, src };
                    for (size_t i = 0; i < caps->num_contexts; ++i) {
                        const tvConfigContext_t& available = caps->contexts[i];
                        if (available.pq_mode == testCtx.pq_mode &&
                            available.videoFormatType == testCtx.videoFormatType &&
                            available.videoSrcType == testCtx.videoSrcType) {
                            std::tuple<int, int, int> key = std::make_tuple(pq, fmt, src);
                            if (seenContexts.find(key) == seenContexts.end()) {
                                validContexts.push_back(testCtx);
                                seenContexts.insert(key);
                            }
                        }
                    }
                }
            }
        }
    // std::tie(...) creates a tuple of values from struct a and b.
    // The < operator for tuples performs lexicographic comparison, like dictionary order:
    // It compares the first elements;If they're equal, it compares the second;If still equal, it compares the third.
        std::sort(validContexts.begin(), validContexts.end(), [](const tvConfigContext_t& a, const tvConfigContext_t& b) {
            return std::tie(a.pq_mode, a.videoFormatType, a.videoSrcType) <
                std::tie(b.pq_mode, b.videoFormatType, b.videoSrcType);
        });
        return validContexts;
    }

    int AVOutputTV::updateAVoutputTVParamV2(std::string action, std::string tr181ParamName,
        const JsonObject& parameters,
        tvPQParameterIndex_t pqParamIndex,int level)
    {
#if DEBUG
        LOGINFO("Entry %s: Action: %s, Param: %s, Level: %d", __FUNCTION__, action.c_str(), tr181ParamName.c_str(), level);
#endif
        int ret = 0;
        const bool isSet = (action == "set");
        const bool isReset = (action == "reset");
        const bool isSync = (action == "sync");

        std::vector<tvConfigContext_t> validContexts = getValidContextsFromParameters(parameters, tr181ParamName);
#if DEBUG
        for (const auto& ctx : validContexts) {

            std::string pqStr = pqModeMap.count(ctx.pq_mode) ? pqModeMap.at(ctx.pq_mode) : std::to_string(ctx.pq_mode);
            std::string fmtStr = videoFormatMap.count(ctx.videoFormatType) ? videoFormatMap.at(ctx.videoFormatType) : std::to_string(ctx.videoFormatType);
            std::string srcStr = videoSrcMap.count(ctx.videoSrcType) ? videoSrcMap.at(ctx.videoSrcType) : std::to_string(ctx.videoSrcType);
            LOGINFO("Valid Context - PQMode: %s, Format: %s, Source: %s", pqStr.c_str(), fmtStr.c_str(), srcStr.c_str());
        }
#endif
        if (validContexts.empty()) {
            LOGWARN("%s: No valid contexts found for parameters", __FUNCTION__);
            return (int)tvERROR_GENERAL;
        }

        for (const auto& ctx : validContexts)
        {
            paramIndex_t paramIndex {
            .sourceIndex = static_cast<uint8_t>(ctx.videoSrcType),
            .pqmodeIndex = static_cast<uint8_t>(ctx.pq_mode),
            .formatIndex = static_cast<uint8_t>(ctx.videoFormatType)
            };
            std::string pqStr = pqModeMap.count(ctx.pq_mode) ? pqModeMap.at(ctx.pq_mode) : std::to_string(ctx.pq_mode);
            std::string fmtStr = videoFormatMap.count(ctx.videoFormatType) ? videoFormatMap.at(ctx.videoFormatType) : std::to_string(ctx.videoFormatType);
            std::string srcStr = videoSrcMap.count(ctx.videoSrcType) ? videoSrcMap.at(ctx.videoSrcType) : std::to_string(ctx.videoSrcType);

            if (isSet)
            {
                ret |= updateAVoutputTVParamToHALV2(tr181ParamName, paramIndex, level, true);
            }
            else
            {
                if (isReset)
                {
                    ret |= updateAVoutputTVParamToHALV2(tr181ParamName, paramIndex, level, false);
                }
                if(getLocalparam(tr181ParamName,paramIndex,level,pqParamIndex,isSync))
                {
                    continue;
                }
            }
            switch (pqParamIndex)
            {
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
                    ret |= SaveTVDolbyVisionMode((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,(tvDolbyMode_t)level);
                    break;
                case PQ_PARAM_ASPECT_RATIO:
                    ret |= SaveAspectRatio((tvVideoSrcType_t)paramIndex.sourceIndex, paramIndex.pqmodeIndex,(tvVideoFormatType_t)paramIndex.formatIndex,(tvDisplayMode_t)level);
                    break;
                    case PQ_PARAM_PRECISION_DETAIL:
            #if HAL_NOT_READY
            #else
                    ret |= SetPrecisionDetail((tvVideoSrcType_t)paramIndex.sourceIndex,
                                            (tvPQModeIndex_t)paramIndex.pqmodeIndex,
                                            (tvVideoFormatType_t)paramIndex.formatIndex,
                                            level);
            #endif
                    break;

                case PQ_PARAM_LOCAL_CONTRAST_ENHANCEMENT:
            #if HAL_NOT_READY
            #else
                #if ENABLE_PQ_PARAM
                    ret |= SetLocalContrastEnhancement((tvVideoSrcType_t)paramIndex.sourceIndex,
                                                    (tvPQModeIndex_t)paramIndex.pqmodeIndex,
                                                    (tvVideoFormatType_t)paramIndex.formatIndex,
                                                    level);
                #endif
            #endif
                    break;

                case PQ_PARAM_MPEG_NOISE_REDUCTION:
            #if HAL_NOT_READY
            #else
                #if ENABLE_PQ_PARAM
                    ret |= SetMPEGNoiseReduction((tvVideoSrcType_t)paramIndex.sourceIndex,
                                                (tvPQModeIndex_t)paramIndex.pqmodeIndex,
                                                (tvVideoFormatType_t)paramIndex.formatIndex,
                                                level);
                #endif
            #endif
                    break;

                case PQ_PARAM_DIGITAL_NOISE_REDUCTION:
            #if HAL_NOT_READY
            #else
                #if ENABLE_PQ_PARAM
                    ret |= SetDigitalNoiseReduction((tvVideoSrcType_t)paramIndex.sourceIndex,
                                                    (tvPQModeIndex_t)paramIndex.pqmodeIndex,
                                                    (tvVideoFormatType_t)paramIndex.formatIndex,
                                                    level);
                #endif
            #endif
                    break;

                case PQ_PARAM_AI_SUPER_RESOLUTION:
            #if HAL_NOT_READY
            #else
                    ret |= SetAISuperResolution((tvVideoSrcType_t)paramIndex.sourceIndex,
                                                (tvPQModeIndex_t)paramIndex.pqmodeIndex,
                                                (tvVideoFormatType_t)paramIndex.formatIndex,
                                                level);
            #endif
                    break;

                case PQ_PARAM_MEMC:
            #if HAL_NOT_READY
            #else
                    ret |= SetMEMC((tvVideoSrcType_t)paramIndex.sourceIndex,
                                (tvPQModeIndex_t)paramIndex.pqmodeIndex,
                                (tvVideoFormatType_t)paramIndex.formatIndex,
                                level);
            #endif
                    break;
                //case PQ_PARAM_AUTO_BACKLIGHT_MODE:
                   // break;
                case PQ_PARAM_HDR10_MODE:
                case PQ_PARAM_HLG_MODE:
                case PQ_PARAM_LDIM:
                case PQ_PARAM_LOCALDIMMING_LEVEL:
                case PQ_PARAM_CMS:
                case PQ_PARAM_CMS_STATE:
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
                case PQ_PARAM_WB_GAIN_RED:
                case PQ_PARAM_WB_GAIN_GREEN:
                case PQ_PARAM_WB_GAIN_BLUE:
                case PQ_PARAM_WB_OFFSET_RED:
                case PQ_PARAM_WB_OFFSET_GREEN:
                case PQ_PARAM_WB_OFFSET_BLUE:
                    // TODO: Add implementation
                    break;

                default:
                    // Prevent compiler warning for unhandled enums
                    LOGWARN("Unhandled PQ parameter index: %d", pqParamIndex);
                    break;
            }
        }
        LOGINFO("Exit: %s, Return Value: %d", __FUNCTION__, ret);
        return ret;
    }

#if HAL_NOT_READY

tvError_t AVOutputTV::ReadJsonFile(JsonObject& root) {
    std::ifstream file(CAPABLITY_FILE_NAMEV2);
    if (!file.is_open()) {
        LOGWARN("AVOutputPlugins: %s: Unable to open file", __FUNCTION__);
        return tvERROR_GENERAL;
    }

    std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (!root.FromString(jsonStr)) {
        LOGWARN("AVOutputPlugins: %s: JSON parsing failed", __FUNCTION__);
        return tvERROR_GENERAL;
    }

    return tvERROR_NONE;
}

tvError_t AVOutputTV::ExtractRangeInfo(const JsonObject& data, int* max_value) {
    if (!data.HasLabel("rangeInfo")) {
        LOGWARN("AVOutputPlugins: %s: 'rangeInfo' not available", __FUNCTION__);
        return tvERROR_NONE;
    }

    JsonObject rangeInfo = data["rangeInfo"].Object();
    if (rangeInfo.HasLabel("to")) {
        if (!max_value) {
            LOGWARN("AVOutputPlugins: %s: NULL input param max_value", __FUNCTION__);
            return tvERROR_INVALID_PARAM;
        }
        *max_value = rangeInfo["to"].Number();
        return tvERROR_NONE;
    }

    LOGWARN("AVOutputPlugins: %s: Invalid 'rangeInfo' format", __FUNCTION__);
    return tvERROR_GENERAL;
}

tvError_t AVOutputTV::ExtractContextCaps(const JsonObject& data, tvContextCaps_t** context_caps) {
    if (!context_caps) {
        LOGWARN("AVOutputPlugins: %s: NULL input param", __FUNCTION__);
        return tvERROR_INVALID_PARAM;
    }

    if (!data.HasLabel("context")) {
        LOGWARN("AVOutputPlugins: %s: 'context' missing", __FUNCTION__);
        return tvERROR_GENERAL;
    }

    JsonObject context = data["context"].Object();
    if (!context.IsSet()) {
        LOGWARN("AVOutputPlugins: %s: Context is not set", __FUNCTION__);
        return tvERROR_GENERAL;
    }

    std::vector<tvConfigContext_t> contexts = ParseContextCaps(context);
    *context_caps = AllocateContextCaps(contexts);
    if (!*context_caps) {
        LOGWARN("AVOutputPlugins: %s: Memory allocation failed", __FUNCTION__);
        return tvERROR_GENERAL;
    }

    return tvERROR_NONE;
}
template<typename EnumType>
bool LookupEnum(const std::string& str, const std::map<int, std::string>& map, EnumType& outEnum) {
    for (const auto& entry : map) {
        if (entry.second == str) {
            outEnum = static_cast<EnumType>(entry.first);
            return true;
        }
    }
    return false;
}

std::vector<tvConfigContext_t> AVOutputTV::ParseContextCaps(const JsonObject& context)
{
    std::vector<tvConfigContext_t> contexts;
    std::set<std::tuple<tvPQModeIndex_t, tvVideoFormatType_t, tvVideoSrcType_t>> seen;

    WPEFramework::Core::JSON::VariantContainer::Iterator modeIterator = context.Variants();
    while (modeIterator.Next()) {
        std::string modeStr = modeIterator.Label();

        tvPQModeIndex_t modeEnum;
        if (!LookupEnum(modeStr, pqModeMap, modeEnum)) continue;

        const auto& modeValue = context[modeStr.c_str()];
        if (!modeValue.IsSet() || modeValue.Content() != WPEFramework::Core::JSON::Variant::type::OBJECT) continue;

        JsonObject formatMap = modeValue.Object();
        WPEFramework::Core::JSON::VariantContainer::Iterator formatIterator = formatMap.Variants();
        while (formatIterator.Next()) {
            std::string formatStr = formatIterator.Label();

            tvVideoFormatType_t fmtEnum;
            if (!LookupEnum(formatStr, videoFormatMap, fmtEnum)) continue;

            const auto& formatValue = formatMap[formatStr.c_str()];
            if (!formatValue.IsSet() || formatValue.Content() != WPEFramework::Core::JSON::Variant::type::ARRAY) continue;

            JsonArray sources = formatValue.Array();
            for (uint32_t i = 0; i < sources.Length(); ++i) {
                std::string srcStr = sources[i].String();

                tvVideoSrcType_t srcEnum;
                if (!LookupEnum(srcStr, videoSrcMap, srcEnum)) continue;

                auto triplet = std::make_tuple(modeEnum, fmtEnum, srcEnum);
                if (seen.find(triplet) == seen.end()) {
                    contexts.push_back({modeEnum, fmtEnum, srcEnum});
                    seen.insert(triplet);
                }
            }
        }
    }

    return contexts;
}

tvContextCaps_t* AVOutputTV::AllocateContextCaps(const std::vector<tvConfigContext_t>& contexts) {
    tvContextCaps_t* context_caps = new (std::nothrow) tvContextCaps_t;
    if (!context_caps) {
        return nullptr;
    }

    context_caps->num_contexts = contexts.size();
    context_caps->contexts = contexts.empty() ? nullptr : new (std::nothrow) tvConfigContext_t[contexts.size()];

    if (!contexts.empty() && !context_caps->contexts) {
        delete context_caps;
        return nullptr;
    }

    if (!contexts.empty()) {
        std::copy(contexts.begin(), contexts.end(), context_caps->contexts);
    }
    return context_caps;
}

tvError_t AVOutputTV::GetCaps(const std::string& key, int* max_value, tvContextCaps_t** context_caps) {
    LOGINFO("Entry\n");
    JsonObject root;
    if (ReadJsonFile(root) != tvERROR_NONE) {
        return tvERROR_GENERAL;
    }

    if (!root.HasLabel(key.c_str())) {
        LOGWARN("AVOutputPlugins: %s: Missing '%s' label", __FUNCTION__, key.c_str());
        return tvERROR_GENERAL;
    }

    JsonObject data = root[key.c_str()].Object();
    if (!data.HasLabel("platformSupport") || !data["platformSupport"].Boolean()) {
        LOGWARN("AVOutputPlugins: %s: Platform support is false", __FUNCTION__);
        return tvERROR_OPERATION_NOT_SUPPORTED;
    }

    if (ExtractRangeInfo(data, max_value) != tvERROR_NONE) {
        return tvERROR_GENERAL;
    }

    if (ExtractContextCaps(data, context_caps) != tvERROR_NONE) {
        return tvERROR_GENERAL;
    }

    return tvERROR_NONE;
}

tvError_t AVOutputTV::GetBacklightCaps(int* max_backlight, tvContextCaps_t** context_caps) {
    return GetCaps("Backlight", max_backlight, context_caps);
}

tvError_t AVOutputTV::GetBrightnessCaps(int* max_brightness, tvContextCaps_t** context_caps) {
    return GetCaps("Brightness", max_brightness, context_caps);
}

tvError_t AVOutputTV::GetContrastCaps(int* max_contrast, tvContextCaps_t** context_caps) {
    return GetCaps("Contrast", max_contrast, context_caps);
}

tvError_t AVOutputTV::GetSharpnessCaps(int* max_sharpness, tvContextCaps_t** context_caps) {
    return GetCaps("Sharpness", max_sharpness, context_caps);
}

tvError_t AVOutputTV::GetSaturationCaps(int* max_saturation, tvContextCaps_t** context_caps) {
    return GetCaps("Saturation", max_saturation, context_caps);
}

tvError_t AVOutputTV::GetHueCaps(int* max_hue, tvContextCaps_t** context_caps) {
    return GetCaps("Hue", max_hue, context_caps);
}

tvError_t AVOutputTV::GetLowLatencyStateCaps(int* max_latency, tvContextCaps_t ** context_caps){
    return GetCaps("LowLatencyState", max_latency, context_caps);
}

// PrecisionDetail
tvError_t AVOutputTV::GetPrecisionDetailCaps(int * maxPrecision, tvContextCaps_t ** context_caps) {
    return GetCaps("PrecisionDetail", maxPrecision, context_caps);
}

// LocalContrastEnhancement
tvError_t AVOutputTV::GetLocalContrastEnhancementCaps(int * maxLocalContrastEnhancement, tvContextCaps_t ** context_caps) {
    return GetCaps("LocalContrastEnhancement", maxLocalContrastEnhancement, context_caps);
}

// MPEGNoiseReduction
tvError_t AVOutputTV::GetMPEGNoiseReductionCaps(int * maxMPEGNoiseReduction, tvContextCaps_t ** context_caps) {
    return GetCaps("MPEGNoiseReduction", maxMPEGNoiseReduction, context_caps);
}

// DigitalNoiseReduction
tvError_t AVOutputTV::GetDigitalNoiseReductionCaps(int * maxDigitalNoiseReduction, tvContextCaps_t ** context_caps) {
    return GetCaps("DigitalNoiseReduction", maxDigitalNoiseReduction, context_caps);
}

// AISuperResolution
tvError_t AVOutputTV::GetAISuperResolutionCaps(int * maxAISuperResolution, tvContextCaps_t ** context_caps) {
    return GetCaps("AISuperResolution", maxAISuperResolution, context_caps);
}

// MEMC
tvError_t AVOutputTV::GetMEMCCaps(int * maxMEMC, tvContextCaps_t ** context_caps) {
    return GetCaps("MEMC", maxMEMC, context_caps);
}

tvError_t AVOutputTV::GetMultiPointWBCaps(int* num_hal_matrix_points,
    int* rgb_min,
    int* rgb_max,
    int* num_ui_matrix_points,
    double** ui_matrix_positions,
    tvContextCaps_t** context_caps)
{
    if (!num_hal_matrix_points || !rgb_min || !rgb_max ||
    !num_ui_matrix_points || !ui_matrix_positions || !context_caps)
    return tvERROR_INVALID_PARAM;

    JsonObject root;
    if (ReadJsonFile(root) != tvERROR_NONE)
    return tvERROR_GENERAL;

    const std::string key = "MultiPointWB";
    if (!root.HasLabel(key.c_str()))
    return tvERROR_OPERATION_NOT_SUPPORTED;

    JsonObject data = root[key.c_str()].Object();

    if (!data.HasLabel("platformSupport") || !data["platformSupport"].Boolean())
    return tvERROR_OPERATION_NOT_SUPPORTED;

    // Extract matrix points
    if (!data.HasLabel("points"))
    return tvERROR_INVALID_PARAM;
    *num_hal_matrix_points = data["points"].Number();

    // Extract range info
    if (!data.HasLabel("rangeInfo"))
    return tvERROR_INVALID_PARAM;

    JsonObject range = data["rangeInfo"].Object();
    if (!range.HasLabel("from") || !range.HasLabel("to"))
    return tvERROR_INVALID_PARAM;

    *rgb_min = range["from"].Number();
    *rgb_max = range["to"].Number();

    // Allocate UI matrix points (same count for now)
    *num_ui_matrix_points = *num_hal_matrix_points;
    *ui_matrix_positions = new double[*num_ui_matrix_points];
    if (!(*ui_matrix_positions))
    return tvERROR_GENERAL;

    for (int i = 0; i < *num_ui_matrix_points; ++i)
    (*ui_matrix_positions)[i] = static_cast<double>(i) / (*num_ui_matrix_points - 1);

    if (ExtractContextCaps(data, context_caps) != tvERROR_NONE)
        return tvERROR_GENERAL;

    return tvERROR_NONE;
}


tvError_t AVOutputTV::GetColorTemperatureCaps(tvColorTemp_t** color_temp, size_t* num_color_temp, tvContextCaps_t** context_caps) {
    LOGINFO("Entry\n");
    JsonObject root;
    if (ReadJsonFile(root) != tvERROR_NONE) {
        return tvERROR_GENERAL;
    }

    std::string key = "ColorTemperature";
    if (!root.HasLabel(key.c_str())) {
        LOGWARN("AVOutputPlugins: %s: Missing '%s' label", __FUNCTION__, key.c_str());
        return tvERROR_GENERAL;
    }

    JsonObject data = root[key.c_str()].Object();
    if (!data.HasLabel("platformSupport") || !data["platformSupport"].Boolean()) {
        LOGWARN("AVOutputPlugins: %s: Platform support is false", __FUNCTION__);
        return tvERROR_OPERATION_NOT_SUPPORTED;
    }

    JsonObject rangeInfo = data["rangeInfo"].Object();
    JsonArray optionsArray = rangeInfo["options"].Array();

    *num_color_temp = optionsArray.Length();
    *color_temp = static_cast<tvColorTemp_t*>(malloc(*num_color_temp * sizeof(tvColorTemp_t)));
    if (!(*color_temp)) {
        return tvERROR_GENERAL;
    }

    for (size_t i = 0; i < *num_color_temp; ++i) {
        std::string tempStr = optionsArray[i].String();
        if (tempStr == "Standard") (*color_temp)[i] = tvColorTemp_STANDARD;
        else if (tempStr == "Warm") (*color_temp)[i] = tvColorTemp_WARM;
        else if (tempStr == "Cold") (*color_temp)[i] = tvColorTemp_COLD;
        else if (tempStr == "UserDefined") (*color_temp)[i] = tvColorTemp_USER;
        else if (tempStr == "Supercold") (*color_temp)[i] = tvColorTemp_SUPERCOLD;
        else if (tempStr == "BoostStandard") (*color_temp)[i] = tvColorTemp_BOOST_STANDARD;
        else if (tempStr == "BoostWarm") (*color_temp)[i] = tvColorTemp_BOOST_WARM;
        else if (tempStr == "BoostCold") (*color_temp)[i] = tvColorTemp_BOOST_COLD;
        else if (tempStr == "BoostUserDefined") (*color_temp)[i] = tvColorTemp_BOOST_USER;
        else if (tempStr == "BoostSupercold") (*color_temp)[i] = tvColorTemp_BOOST_SUPERCOLD;
        else (*color_temp)[i] = tvColorTemp_STANDARD;
    }

    if (ExtractContextCaps(data, context_caps) != tvERROR_NONE) {
        free(*color_temp);
        return tvERROR_GENERAL;
    }

    return tvERROR_NONE;
}


tvError_t AVOutputTV::GetSdrGammaCaps(tvSdrGamma_t** sdr_gamma, size_t* num_sdr_gamma, tvContextCaps_t** context_caps) {
    LOGINFO("Entry\n");
    JsonObject root;
    if (ReadJsonFile(root) != tvERROR_NONE) {
        return tvERROR_GENERAL;
    }

    std::string key = "SDRGamma";
    if (!root.HasLabel(key.c_str())) {
        LOGWARN("AVOutputPlugins: %s: Missing '%s' label", __FUNCTION__, key.c_str());
        return tvERROR_GENERAL;
    }

    JsonObject data = root[key.c_str()].Object();
    if (!data.HasLabel("platformSupport") || !data["platformSupport"].Boolean()) {
        LOGWARN("AVOutputPlugins: %s: Platform support is false", __FUNCTION__);
        return tvERROR_OPERATION_NOT_SUPPORTED;
    }

    JsonObject rangeInfo = data["rangeInfo"].Object();
    JsonArray optionsArray = rangeInfo["options"].Array();

    *num_sdr_gamma = optionsArray.Length();
    *sdr_gamma = static_cast<tvSdrGamma_t*>(malloc(*num_sdr_gamma * sizeof(tvSdrGamma_t)));
    if (!(*sdr_gamma)) {
        return tvERROR_GENERAL;
    }

    for (size_t i = 0; i < *num_sdr_gamma; ++i) {
        std::string gammaStr = optionsArray[i].String();
        if (gammaStr == "1.8") (*sdr_gamma)[i] = tvSdrGamma_1_8;
        else if (gammaStr == "1.9") (*sdr_gamma)[i] = tvSdrGamma_1_9;
        else if (gammaStr == "2.0") (*sdr_gamma)[i] = tvSdrGamma_2_0;
        else if (gammaStr == "2.1") (*sdr_gamma)[i] = tvSdrGamma_2_1;
        else if (gammaStr == "2.2") (*sdr_gamma)[i] = tvSdrGamma_2_2;
        else if (gammaStr == "2.3") (*sdr_gamma)[i] = tvSdrGamma_2_3;
        else if (gammaStr == "2.4") (*sdr_gamma)[i] = tvSdrGamma_2_4;
        else if (gammaStr == "BT.1886") (*sdr_gamma)[i] = tvSdrGamma_BT_1886;
        else (*sdr_gamma)[i] = tvSdrGamma_INVALID;
    }

    if (ExtractContextCaps(data, context_caps) != tvERROR_NONE) {
        free(*sdr_gamma);
        return tvERROR_GENERAL;
    }

    return tvERROR_NONE;
}
tvError_t AVOutputTV::GetTVDimmingModeCaps(tvDimmingMode_t** dimming_mode, size_t* num_dimming_mode, tvContextCaps_t** context_caps){
    LOGINFO("Entry\n");
    JsonObject root;
    if (ReadJsonFile(root) != tvERROR_NONE) {
        return tvERROR_GENERAL;
    }
    std::string key = "DimmingMode";
    if (!root.HasLabel(key.c_str())) {
        LOGWARN("AVOutputPlugins: %s: Missing '%s' label", __FUNCTION__, key.c_str());
        return tvERROR_GENERAL;
    }
    JsonObject data = root[key.c_str()].Object();
    if (!data.HasLabel("platformSupport") || !data["platformSupport"].Boolean()) {
        LOGWARN("AVOutputPlugins: %s: Platform support is false", __FUNCTION__);
        return tvERROR_OPERATION_NOT_SUPPORTED;
    }

    JsonObject rangeInfo = data["rangeInfo"].Object();
    JsonArray optionsArray = rangeInfo["options"].Array();
    *num_dimming_mode = optionsArray.Length();
    *dimming_mode = static_cast<tvDimmingMode_t*>(malloc(*num_dimming_mode * sizeof(tvDimmingMode_t)));
    if (!(*dimming_mode)) {
        return tvERROR_GENERAL;
    }

    for (size_t i = 0; i < *num_dimming_mode; ++i) {
        std::string modeStr = optionsArray[i].String();
        if (modeStr == "Fixed") (*dimming_mode)[i] = tvDimmingMode_Fixed;
        else if (modeStr == "Local") (*dimming_mode)[i] = tvDimmingMode_Local;
        else if (modeStr == "Global") (*dimming_mode)[i] = tvDimmingMode_Global;
        else (*dimming_mode)[i] = tvDimmingMode_MAX;
    }

    if (ExtractContextCaps(data, context_caps) != tvERROR_NONE) {
        free(*dimming_mode);
        return tvERROR_GENERAL;
    }

    return tvERROR_NONE;

}

tvError_t AVOutputTV::GetAspectRatioCaps(tvDisplayMode_t** aspect_ratio, size_t* num_aspect_ratio, tvContextCaps_t** context_caps) {
    LOGINFO("Entry\n");
    JsonObject root;
    if (ReadJsonFile(root) != tvERROR_NONE) {
        return tvERROR_GENERAL;
    }

    std::string key = "AspectRatio";
    if (!root.HasLabel(key.c_str())) {
        LOGWARN("AVOutputPlugins: %s: Missing '%s' label", __FUNCTION__, key.c_str());
        return tvERROR_GENERAL;
    }

    JsonObject data = root[key.c_str()].Object();
    if (!data.HasLabel("platformSupport") || !data["platformSupport"].Boolean()) {
        LOGWARN("AVOutputPlugins: %s: Platform support is false", __FUNCTION__);
        return tvERROR_OPERATION_NOT_SUPPORTED;
    }

    JsonObject rangeInfo = data["rangeInfo"].Object();
    JsonArray optionsArray = rangeInfo["options"].Array();

    *num_aspect_ratio = optionsArray.Length();
    *aspect_ratio = static_cast<tvDisplayMode_t*>(malloc(*num_aspect_ratio * sizeof(tvDisplayMode_t)));
    if (!(*aspect_ratio)) {
        return tvERROR_GENERAL;
    }

    for (size_t i = 0; i < *num_aspect_ratio; ++i) {
        std::string aspectStr = optionsArray[i].String();
        if (aspectStr == "TV AUTO") (*aspect_ratio)[i] = tvDisplayMode_AUTO;
        else if (aspectStr == "TV DIRECT") (*aspect_ratio)[i] = tvDisplayMode_DIRECT;
        else if (aspectStr == "TV FULL") (*aspect_ratio)[i] = tvDisplayMode_FULL;
        else if (aspectStr == "TV NORMAL") (*aspect_ratio)[i] = tvDisplayMode_NORMAL;
        else if (aspectStr == "TV 16X9 STRETCH") (*aspect_ratio)[i] = tvDisplayMode_16x9;
        else if (aspectStr == "TV 4X3 PILLARBOX") (*aspect_ratio)[i] = tvDisplayMode_4x3;
        else if (aspectStr == "TV ZOOM") (*aspect_ratio)[i] = tvDisplayMode_ZOOM;
        else (*aspect_ratio)[i] = tvDisplayMode_MAX;
    }

    if (ExtractContextCaps(data, context_caps) != tvERROR_NONE) {
        free(*aspect_ratio);
        return tvERROR_GENERAL;
    }

    return tvERROR_NONE;
}

tvError_t AVOutputTV::GetTVPictureModeCaps(tvPQModeIndex_t** mode, size_t* num_pic_modes, tvContextCaps_t** context_caps) {
    LOGINFO("Entry\n");
    JsonObject root;
    if (ReadJsonFile(root) != tvERROR_NONE) {
        return tvERROR_GENERAL;
    }

    std::string key = "PictureMode";
    if (!root.HasLabel(key.c_str())) {
        LOGWARN("AVOutputPlugins: %s: Missing '%s' label", __FUNCTION__, key.c_str());
        return tvERROR_GENERAL;
    }

    JsonObject data = root[key.c_str()].Object();
    if (!data.HasLabel("platformSupport") || !data["platformSupport"].Boolean()) {
        LOGWARN("AVOutputPlugins: %s: Platform support is false", __FUNCTION__);
        return tvERROR_OPERATION_NOT_SUPPORTED;
    }

    JsonObject rangeInfo = data["rangeInfo"].Object();
    JsonArray optionsArray = rangeInfo["options"].Array();

    *num_pic_modes = optionsArray.Length();
    *mode = static_cast<tvPQModeIndex_t*>(malloc(*num_pic_modes * sizeof(tvPQModeIndex_t)));
    if (!(*mode)) {
        return tvERROR_GENERAL;
    }

    for (size_t i = 0; i < *num_pic_modes; ++i) {
        std::string modeStr = optionsArray[i].String();

        if (modeStr == "Standard") (*mode)[i] = PQ_MODE_STANDARD;
        else if (modeStr == "Vivid") (*mode)[i] = PQ_MODE_VIVID;
        else if (modeStr == "EnergySaving" || modeStr == "Energy Saving") (*mode)[i] = PQ_MODE_ENERGY_SAVING;
        else if (modeStr == "Theater") (*mode)[i] = PQ_MODE_THEATER;
        else if (modeStr == "Game") (*mode)[i] = PQ_MODE_GAME;
        else if (modeStr == "Sports") (*mode)[i] = PQ_MODE_SPORTS;
        else if (modeStr == "AI PQ") (*mode)[i] = PQ_MODE_AIPQ;
        else if (modeStr == "Dark") (*mode)[i] = PQ_MODE_DARK;
        else if (modeStr == "Bright") (*mode)[i] = PQ_MODE_BRIGHT;
        else if (modeStr == "IQ") (*mode)[i] = PQ_MODE_IQ;
        else (*mode)[i] = PQ_MODE_INVALID;
    }

    if (ExtractContextCaps(data, context_caps) != tvERROR_NONE) {
        free(*mode);
        return tvERROR_GENERAL;
    }

    return tvERROR_NONE;
}
tvError_t AVOutputTV::GetBacklightModeCaps(tvBacklightMode_t** backlight_mode, size_t* num_backlight_mode, tvContextCaps_t** context_caps)
{
    LOGINFO("Entry\n");

    JsonObject root;
    if (ReadJsonFile(root) != tvERROR_NONE) {
        return tvERROR_GENERAL;
    }

    std::string key = "BacklightMode";
    if (!root.HasLabel(key.c_str())) {
        LOGWARN("AVOutputPlugins: %s: Missing '%s' label", __FUNCTION__, key.c_str());
        return tvERROR_GENERAL;
    }

    JsonObject data = root[key.c_str()].Object();
    if (!data.HasLabel("platformSupport") || !data["platformSupport"].Boolean()) {
        LOGWARN("AVOutputPlugins: %s: Platform support is false", __FUNCTION__);
        return tvERROR_OPERATION_NOT_SUPPORTED;
    }

    JsonObject rangeInfo = data["rangeInfo"].Object();
    JsonArray optionsArray = rangeInfo["options"].Array();

    *num_backlight_mode = optionsArray.Length();
    *backlight_mode = static_cast<tvBacklightMode_t*>(malloc(*num_backlight_mode * sizeof(tvBacklightMode_t)));
    if (!(*backlight_mode)) {
        return tvERROR_GENERAL;
    }

    for (size_t i = 0; i < *num_backlight_mode; ++i) {
        std::string modeStr = optionsArray[i].String();
        auto it = backlightModeMap.find(modeStr);
        if (it != backlightModeMap.end()) {
            (*backlight_mode)[i] = it->second;
        } else {
            (*backlight_mode)[i] = tvBacklightMode_INVALID;
        }
    }

    if (ExtractContextCaps(data, context_caps) != tvERROR_NONE) {
        free(*backlight_mode);
        return tvERROR_GENERAL;
    }

    return tvERROR_NONE;
}


/*
tvError_t GetColorTemperatureCaps(char*** options, size_t* options_count, tvContextCaps_t** context_caps);
Since C does not have std::vector, we use char*** options (a pointer to an array of strings).
size_t* options_count is used to store the number of elements in the array.

char** options = NULL;
size_t options_count = 0;
tvContextCaps_t* context_caps = NULL;

tvError_t status = GetColorTemperatureCaps(&options, &options_count, &context_caps);
if (status == tvERROR_NONE) {
    for (size_t i = 0; i < options_count; i++) {
        printf("Option: %s\n", options[i]);
    }
}
*/

tvError_t AVOutputTV::GetDVCalibrationCaps(tvDVCalibrationSettings_t **min_values, tvDVCalibrationSettings_t **max_values, tvContextCaps_t **context_caps) {
    JsonObject root;
    if (ReadJsonFile(root) != tvERROR_NONE) {
        return tvERROR_GENERAL;
    }
    if (!root.HasLabel("DolbyVisionCalibration")) {
        LOGWARN("AVOutputPlugins: %s: Missing 'DolbyVisionCalibration' label", __FUNCTION__);
        return tvERROR_GENERAL;
    }

    JsonObject data = root["DolbyVisionCalibration"].Object();
    *min_values = new tvDVCalibrationSettings_t();
    *max_values = new tvDVCalibrationSettings_t();

    std::map<std::string, double tvDVCalibrationSettings_t::*> keyMap = {
        {"Tmax", &tvDVCalibrationSettings_t::Tmax},
        {"Tmin", &tvDVCalibrationSettings_t::Tmin},
        {"Tgamma", &tvDVCalibrationSettings_t::Tgamma},
        {"Rx", &tvDVCalibrationSettings_t::Rx},
        {"Ry", &tvDVCalibrationSettings_t::Ry},
        {"Gx", &tvDVCalibrationSettings_t::Gx},
        {"Gy", &tvDVCalibrationSettings_t::Gy},
        {"Bx", &tvDVCalibrationSettings_t::Bx},
        {"By", &tvDVCalibrationSettings_t::By},
        {"Wx", &tvDVCalibrationSettings_t::Wx},
        {"Wy", &tvDVCalibrationSettings_t::Wy}
    };

    for (auto it = keyMap.begin(); it != keyMap.end(); ++it) {
        const std::string& key = it->first;
        double tvDVCalibrationSettings_t::*member = it->second;
        std::string minKey = "range" + key;
        if (data.HasLabel(minKey.c_str())) {
            JsonObject range = data[minKey.c_str()].Object();
            (*min_values)->*member = range["from"].Number();
            (*max_values)->*member = range["to"].Number();
        }
    }

    if (ExtractContextCaps(data, context_caps) != tvERROR_NONE) {
        return tvERROR_GENERAL;
    }
    return tvERROR_NONE;
}

tvError_t AVOutputTV::GetCMSCaps(int* max_hue,
    int* max_saturation,
    int* max_luma,
    tvDataComponentColor_t** color,
    tvComponentType_t** component,
    size_t* num_color,
    size_t* num_component,
    tvContextCaps_t** context_caps)
{
    if (!max_hue || !max_saturation || !max_luma || !color || !component || !num_color || !num_component || !context_caps) {
    return tvERROR_INVALID_PARAM;
    }

    JsonObject root;
    if (ReadJsonFile(root) != tvERROR_NONE) {
    return tvERROR_GENERAL;
    }

    const char* key = "CMS";
    if (!root.HasLabel(key)) {
    return tvERROR_OPERATION_NOT_SUPPORTED;
    }

    JsonObject cms = root[key].Object();

    if (!cms.HasLabel("platformSupport") || !cms["platformSupport"].Boolean()) {
    return tvERROR_OPERATION_NOT_SUPPORTED;
    }

    // Extract ranges
    *max_hue = cms.HasLabel("rangeHue") ? cms["rangeHue"].Object()["to"].Number() : 0;
    *max_saturation = cms.HasLabel("rangeSaturation") ? cms["rangeSaturation"].Object()["to"].Number() : 0;
    *max_luma = cms.HasLabel("rangeLuma") ? cms["rangeLuma"].Object()["to"].Number() : 0;

    // Extract colors
    const JsonArray& colorArray = cms["color"].Array();
    *num_color = colorArray.Length();
    *color = new tvDataComponentColor_t[*num_color];
    for (size_t i = 0; i < *num_color; ++i) {
        std::string colorStr = colorArray[i].String();
        if (getCMSColorEnumFromString(colorStr, (*color)[i]) != 0) {
        delete[] *color;
        *color = nullptr;
        return tvERROR_INVALID_PARAM;
        }
    }

    // Extract components
    const JsonArray& compArray = cms["component"].Array();
    *num_component = compArray.Length();
    *component = new tvComponentType_t[*num_component];
    for (size_t i = 0; i < *num_component; ++i) {
        std::string compStr = compArray[i].String();
        if (getCMSComponentEnumFromString(compStr, (*component)[i]) != 0) {
        delete[] *color;
        delete[] *component;
        *color = nullptr;
        *component = nullptr;
        return tvERROR_INVALID_PARAM;
        }
    }

    // Extract context capabilities
    if (ExtractContextCaps(cms, context_caps) != tvERROR_NONE) {
        delete[] *color;
        delete[] *component;
        *color = nullptr;
        *component = nullptr;
        return tvERROR_GENERAL;
    }

    return tvERROR_NONE;
}

tvError_t AVOutputTV::GetCustom2PointWhiteBalanceCaps(int* min_gain, int* min_offset,
    int* max_gain, int* max_offset,
    tvWBColor_t** color,
    tvWBControl_t** control,
    size_t* num_color, size_t* num_control,
    tvContextCaps_t** context_caps)
{
    if (!min_gain || !min_offset || !max_gain || !max_offset ||
    !color || !control || !num_color || !num_control || !context_caps)
    {
        LOGERR("Invalid input pointers");
        return tvERROR_INVALID_PARAM;
    }

    JsonObject root;
    if (ReadJsonFile(root) != tvERROR_NONE) {
        LOGERR("Failed to read JSON capabilities");
        return tvERROR_GENERAL;
    }

    const char* key = "Custom2PointWhiteBalance";
    if (!root.HasLabel(key)) {
        LOGERR("Missing key: %s", key);
        return tvERROR_OPERATION_NOT_SUPPORTED;
    }

    JsonObject section = root[key].Object();

    if (!section.HasLabel("platformSupport") || !section["platformSupport"].Boolean()) {
        return tvERROR_OPERATION_NOT_SUPPORTED;
    }

    // Parse rangeGain and rangeOffset
    *min_gain  = section["rangeGain"].Object()["from"].Number();
    *max_gain  = section["rangeGain"].Object()["to"].Number();
    *min_offset = section["rangeOffset"].Object()["from"].Number();
    *max_offset = section["rangeOffset"].Object()["to"].Number();

    // Parse control array
    JsonArray controlArray = section["control"].Array();
    *num_control = controlArray.Length();
    *control = new tvWBControl_t[*num_control];
    for (size_t i = 0; i < *num_control; ++i) {
        std::string ctrlStr = controlArray[i].String();
        if (getWBControlEnumFromString(ctrlStr, (*control)[i]) != 0) {
            LOGERR("Invalid control: %s", ctrlStr.c_str());
            delete[] *control;
            *control = nullptr;
            return tvERROR_INVALID_PARAM;
        }
    }

    // Parse color array
    JsonArray colorArray = section["color"].Array();
    *num_color = colorArray.Length();
    *color = new tvWBColor_t[*num_color];
    for (size_t i = 0; i < *num_color; ++i) {
        std::string colStr = colorArray[i].String();
        if (getWBColorEnumFromString(colStr, (*color)[i]) != 0) {
            LOGERR("Invalid color: %s", colStr.c_str());
            delete[] *color;
            delete[] *control;
            *color = nullptr;
            *control = nullptr;
            return tvERROR_INVALID_PARAM;
        }
    }

    // Parse contextCaps
    if (ExtractContextCaps(section, context_caps) != tvERROR_NONE) {
        delete[] *color;
        delete[] *control;
        *color = nullptr;
        *control = nullptr;
        return tvERROR_GENERAL;
    }

    return tvERROR_NONE;
}

#endif

	int AVOutputTV::ReadCapablitiesFromConf(std::string param, capDetails_t& info)
    {
        int ret = 0;

        /*Consider User WhiteBalance as CustomWhiteBalance
        To avoid clash with Factory WhiteBalance Calibration capablities*/

        if ( param == "WhiteBalance") {
            param = "CustomWhiteBalance";
        } else if ( param == "AutoBacklightMode") {
            param = "BacklightControl";
        }

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

            if ((param == "DolbyVisionMode") || (param == "Backlight") || (param == "CMS") || (param == "CustomWhiteBalance") || (param == "HDRMode") || (param == "BacklightControl")) {
                configString = param + ".platformsupport";
                info.isPlatformSupport = inFile.Get<std::string>(configString);
                printf(" platformsupport : %s\n",info.isPlatformSupport.c_str() );
            }

            if ( (param == "ColorTemperature") || (param == "DimmingMode") ||
                 ( param == "BacklightControl") || (param == "DolbyVisionMode") ||
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

   bool AVOutputTV::checkCMSColorAndComponentCapability(const std::string capValue, const std::string inputValue) {
        // Parse capValue into a set
        std::set<std::string> capSet;
        std::istringstream capStream(capValue);
        std::string token;

        while (std::getline(capStream, token, ',')) {
            capSet.insert(token);
        }

        // Parse inputValue and check if each item exists in the set
        std::istringstream inputStream(inputValue);
        while (std::getline(inputStream, token, ',')) {
            if (capSet.find(token) == capSet.end()) {
                return false;
            }
        }
        return true;
    }

} //namespace Plugin
} //namespace WPEFramework
