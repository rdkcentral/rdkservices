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

#define TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.SDR.Backlight"
#define TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.HDR.Backlight"
#define TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM  "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.AutoBacklightMode"

#define BUFFER_SIZE     (128)

const char *component_color[] = {
    [COLOR_ENABLE] = "enable",
    [COLOR_RED] = "red",
    [COLOR_GREEN] = "green",
    [COLOR_BLUE] = "blue",
    [COLOR_CYAN] = "cyan",
    [COLOR_MAGENTA] = "magenta",
    [COLOR_YELLOW] = "yellow"
};

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
        registerMethod("resetBacklight", &ControlSettingsTV::resetBacklight, this, {1});
        registerMethod("getBrightness", &ControlSettingsTV::getBrightness, this, {1});
        registerMethod("setBrightness", &ControlSettingsTV::setBrightness, this, {1});
        registerMethod("resetBrightness", &ControlSettingsTV::resetBrightness, this, {1});
        registerMethod("getContrast", &ControlSettingsTV::getContrast, this, {1});
        registerMethod("setContrast", &ControlSettingsTV::setContrast, this, {1});
        registerMethod("resetContrast", &ControlSettingsTV::resetContrast, this, {1});
        registerMethod("getSharpness", &ControlSettingsTV::getSharpness, this, {1});
        registerMethod("setSharpness", &ControlSettingsTV::setSharpness, this, {1});
        registerMethod("resetSharpness", &ControlSettingsTV::resetSharpness, this, {1});
        registerMethod("getSaturation", &ControlSettingsTV::getSaturation, this, {1});
        registerMethod("setSaturation", &ControlSettingsTV::setSaturation, this, {1});
        registerMethod("resetSaturation", &ControlSettingsTV::resetSaturation, this, {1});
        registerMethod("getHue", &ControlSettingsTV::getHue, this, {1});
        registerMethod("setHue", &ControlSettingsTV::setHue, this, {1});
        registerMethod("resetHue", &ControlSettingsTV::resetHue, this, {1});
        registerMethod("getColorTemperature", &ControlSettingsTV::getColorTemperature, this, {1});
        registerMethod("setColorTemperature", &ControlSettingsTV::setColorTemperature, this, {1});
        registerMethod("resetColorTemperature", &ControlSettingsTV::resetColorTemperature, this, {1});

        registerMethod("getComponentSaturation", &ControlSettingsTV::getComponentSaturation, this, {1});
        registerMethod("setComponentSaturation", &ControlSettingsTV::setComponentSaturation, this, {1});
        registerMethod("resetComponentSaturation", &ControlSettingsTV::resetComponentSaturation, this, {1});
        registerMethod("getComponentHue", &ControlSettingsTV::getComponentHue, this, {1});
        registerMethod("setComponentHue", &ControlSettingsTV::setComponentHue, this, {1});
        registerMethod("resetComponentHue", &ControlSettingsTV::resetComponentHue, this, {1});
        registerMethod("getComponentLuma", &ControlSettingsTV::getComponentLuma, this, {1});
        registerMethod("setComponentLuma", &ControlSettingsTV::setComponentLuma, this, {1});
        registerMethod("resetComponentLuma", &ControlSettingsTV::resetComponentLuma, this, {1});
        registerMethod("getBacklightDimmingMode", &ControlSettingsTV::getBacklightDimmingMode, this, {1});
        registerMethod("setBacklightDimmingMode", &ControlSettingsTV::setBacklightDimmingMode, this, {1});
        registerMethod("resetBacklightDimmingMode", &ControlSettingsTV::resetBacklightDimmingMode, this, {1});
        registerMethod("getAutoBacklightControl", &ControlSettingsTV::getAutoBacklightControl, this, {1});
        registerMethod("setAutoBacklightControl", &ControlSettingsTV::setAutoBacklightControl, this, {1});
        registerMethod("resetAutoBacklightControl", &ControlSettingsTV::resetAutoBacklightControl, this, {1});

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
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        int backlight = 0,err = 0;

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

	if(appUsesGlobalBackLightFactor){
            err = GetLastSetBacklightForGBF(backlight);
        }
        else {
            GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
            err = GetLocalparam("Backlight",formatIndex,pqIndex,sourceIndex,backlight);
        }
        if( err == 0 ) {
            response["backlight"] = std::to_string(backlight);
            LOGINFO("Exit : Backlight Value: %d \n", backlight);
            returnResponse(true, "success");
        }
        else {
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
    }

    uint32_t ControlSettingsTV::setBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
     
	std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int backlight = 0;
        tvError_t ret  = tvERROR_NONE;

        value = parameters.HasLabel("backlight") ? parameters["backlight"].String() : "";
        returnIfParamNotFound(value);
        backlight = std::stoi(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        if(appUsesGlobalBackLightFactor){
            int convertedBL = GetDriverEquivalentBLForCurrentFmt(backlight);// 100scale/255
            backlight = convertedBL;
        }
        ret = SetBacklight(backlight);
        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set backlight\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=backlight;
            int retval = UpdatePQParamsToCache("set","Backlight",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_BRIGHTNESS,params);
            if(retval != 0) {
                LOGWARN("Failed to Save Backlight to ssm_data\n");
            }
            if(appUsesGlobalBackLightFactor)
                saveBacklightToLocalStoreForGBF("Backlight",value.c_str());
            LOGINFO("Exit : setBacklight successful to value: %d\n",backlight);
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::resetBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,backlight=0;
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

        if(appUsesGlobalBackLightFactor){
            tr181ErrorCode_t err = clearLocalParam(rfc_caller_id, TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM);
            if ( err != tr181Success ) {
                LOGWARN("ClearLocalParam for %s Failed : %s\n", TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM, getTR181ErrorString(err));
                ret = tvERROR_GENERAL;
            }
            else {
                LOGINFO("ClearLocalParam for %s Successful\n", TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM);
            }
            err = clearLocalParam(rfc_caller_id, TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM);
            if ( err != tr181Success ) {
                LOGWARN("ClearLocalParam for %s Failed : %s\n", TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM, getTR181ErrorString(err));
                ret = tvERROR_GENERAL;
            }
        }
        /* non backlight factor path */
	int retval= UpdatePQParamsToCache("reset","Backlight",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_BRIGHTNESS,params);
        if(retval != 0 ) {
            LOGWARN("Failed to reset Brightness\n");
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("Backlight",formatIndex,pqIndex,sourceIndex,backlight);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,backlight);
                ret = SetBacklight(backlight);
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
            LOGINFO("Exit : resetBacklight Successful to value : %d \n",backlight);
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::getBrightness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

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
            LOGINFO("Exit : Brightness Value: %d \n", brightness);
            returnResponse(true, "success");
        }
        else {
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
    }



    uint32_t ControlSettingsTV::setBrightness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

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

    uint32_t ControlSettingsTV::resetBrightness(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

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
            LOGINFO("Exit : resetBrightness Successful to value : %d \n",brightness);
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::getContrast(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        int contrast = 0;

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
        int err = GetLocalparam("Contrast",formatIndex,pqIndex,sourceIndex,contrast);
        if( err == 0 ) {
            response["contrast"] = std::to_string(contrast);
            LOGINFO("Exit : Contrast Value: %d \n", contrast);
            returnResponse(true, "success");
        }
        else {
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
    }

    uint32_t ControlSettingsTV::setContrast(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int contrast = 0;

        value = parameters.HasLabel("contrast") ? parameters["contrast"].String() : "";
        returnIfParamNotFound(value);
        contrast = stoi(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        tvError_t ret = SetContrast(contrast);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set Contrast\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=contrast;
            int retval= UpdatePQParamsToCache("set","Contrast",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CONTRAST,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Contrast to ssm_data\n");
            }
            LOGINFO("Exit : setContrast successful to value: %d\n", contrast);
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::resetContrast(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,contrast=0;
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

        int retval= UpdatePQParamsToCache("reset","Contrast",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CONTRAST,params);

        if(retval != 0 ) {
            LOGWARN("Failed to reset Contrast\n");
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("Contrast",formatIndex,pqIndex,sourceIndex,contrast);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,contrast);
                ret = SetContrast(contrast);
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
            LOGINFO("Exit : resetContrast Successful to value : %d \n",contrast);
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::getSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        int saturation = 0;

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
        int err = GetLocalparam("Saturation",formatIndex,pqIndex,sourceIndex,saturation);
        if( err == 0 ) {
            response["saturation"] = std::to_string(saturation);
            LOGINFO("Exit : Saturation Value: %d \n", saturation);
            returnResponse(true, "success");
        }
        else {
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
    }

    uint32_t ControlSettingsTV::setSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int saturation = 0;

        value = parameters.HasLabel("saturation") ? parameters["saturation"].String() : "";
        returnIfParamNotFound(value);
        saturation = stoi(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        tvError_t ret = SetSaturation(saturation);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set Saturation\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=saturation;
            int retval= UpdatePQParamsToCache("set","Saturation",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_SATURATION,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Saturation to ssm_data\n");
            }
            LOGINFO("Exit : setSaturation successful to value: %d\n", saturation);
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::resetSaturation(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,saturation=0;
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

        int retval= UpdatePQParamsToCache("reset","Saturation",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_SATURATION,params);

        if(retval != 0 ) {
            LOGWARN("Failed to reset Saturation\n");
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("Saturation",formatIndex,pqIndex,sourceIndex,saturation);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,saturation);
                ret = SetSaturation(saturation);
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
            LOGINFO("Exit : resetSaturation Successful to value : %d \n",saturation);
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::getSharpness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        int sharpness = 0;

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
        int err = GetLocalparam("Sharpness",formatIndex,pqIndex,sourceIndex,sharpness);
        if( err == 0 ) {
            response["sharpness"] = std::to_string(sharpness);
            LOGINFO("Exit : Sharpness Value: %d \n", sharpness);
            returnResponse(true, "success");
        }
        else {
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
    }

    uint32_t ControlSettingsTV::setSharpness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sharpness = 0;

        value = parameters.HasLabel("sharpness") ? parameters["sharpness"].String() : "";
        returnIfParamNotFound(value);
        sharpness = stoi(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        tvError_t ret = SetSharpness(sharpness);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set Sharpness\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=sharpness;
            int retval= UpdatePQParamsToCache("set","Sharpness",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_SHARPNESS,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Sharpness to ssm_data\n");
            }
            LOGINFO("Exit : setSharpness successful to value: %d\n", sharpness);
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::resetSharpness(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,sharpness=0;
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

        int retval= UpdatePQParamsToCache("reset","Sharpness",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_SHARPNESS,params);

        if(retval != 0 ) {
            LOGWARN("Failed to reset Sharpness\n");
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("Sharpness",formatIndex,pqIndex,sourceIndex,sharpness);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,sharpness);
                ret = SetSharpness(sharpness);
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
            LOGINFO("Exit : resetSharpness Successful to value : %d \n",sharpness);
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::getHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        int hue = 0;

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
        int err = GetLocalparam("Hue",formatIndex,pqIndex,sourceIndex,hue);
        if( err == 0 ) {
            response["hue"] = std::to_string(hue);
            LOGINFO("Exit : Hue Value: %d \n", hue);
            returnResponse(true, "success");
        }
        else {
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
    }

    uint32_t ControlSettingsTV::setHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int hue = 0;

        value = parameters.HasLabel("hue") ? parameters["hue"].String() : "";
        returnIfParamNotFound(value);
        hue = stoi(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        tvError_t ret = SetHue(hue);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set Hue\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=hue;
            int retval= UpdatePQParamsToCache("set","Hue",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_HUE,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Hue to ssm_data\n");
            }
            LOGINFO("Exit : setHue successful to value: %d\n", hue);
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::resetHue(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,hue=0;
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

        int retval= UpdatePQParamsToCache("reset","Hue",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_HUE,params);

        if(retval != 0 ) {
            LOGWARN("Failed to reset Hue\n");
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("Hue",formatIndex,pqIndex,sourceIndex,hue);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,hue);
                ret = SetHue(hue);
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
            LOGINFO("Exit : resetHue Successful to value : %d \n",hue);
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::getColorTemperature(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        int colortemp = 0;

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
        int err = GetLocalparam("ColorTemp",formatIndex,pqIndex,sourceIndex,colortemp);
        if( err == 0 ) {
            switch(colortemp) {

                case tvColorTemp_STANDARD:
                    LOGINFO("Color Temp Value: Standard\n");
                    response["colorTemp"] = "Standard";
                    break;

                case tvColorTemp_WARM:
                    LOGINFO("Color Temp Value: Warm\n");
                    response["colorTemp"] = "Warm";
                    break;

                case tvColorTemp_COLD:
                    LOGINFO("Color Temp Value: Cold\n");
                    response["colorTemp"] = "Cold";
                    break;

                case tvColorTemp_USER:
                    LOGINFO("Color Temp Value: User Defined\n");
                    response["colorTemp"] = "User Defined";
                    break;

                default:
                    LOGINFO("Color Temp Value: Standard\n");
                    response["colorTemp"] = "Standard";
                    break;
            }
            LOGINFO("Exit : ColorTemperature Value: %d \n", colortemp);
            returnResponse(true, "success");
        }
        else {
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
    }

    uint32_t ControlSettingsTV::setColorTemperature(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int colortemp = 0;

        value = parameters.HasLabel("colorTemp") ? parameters["colorTemp"].String() : "";
        returnIfParamNotFound(value);
        if(!value.compare("Standard")) {
            colortemp = tvColorTemp_STANDARD;
        }
        else if (!value.compare("Warm")){
            colortemp = tvColorTemp_WARM;
        }
        else if (!value.compare("Cold")){
            colortemp = tvColorTemp_COLD;
        }
        else if (!value.compare("User Defined")){
            colortemp = tvColorTemp_USER;
        }
        else {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        tvError_t ret = SetColorTemperature((tvColorTemp_t)colortemp);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set ColorTemperature\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=colortemp;
            int retval= UpdatePQParamsToCache("set","ColorTemp",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CONTRAST,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save ColorTemperature to ssm_data\n");
            }
            LOGINFO("Exit : setColorTemperature successful to value: %d\n", colortemp);
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::resetColorTemperature(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,colortemp=0;
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

        int retval= UpdatePQParamsToCache("reset","ColorTemp",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CONTRAST,params);

        if(retval != 0 ) {
            LOGWARN("Failed to reset ColorTemperature\n");
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("ColorTemp",formatIndex,pqIndex,sourceIndex,colortemp);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,colortemp);
                ret = SetColorTemperature((tvColorTemp_t)colortemp);
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
            LOGINFO("Exit : resetColorTemperature Successful to value : %d \n",colortemp);
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::getComponentSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
	std::string cms("saturation.");
	std::string value;
	int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        tvDataComponentColor_t blSaturationColor;
        int saturation=0;
        JsonObject range;
        JsonObject saturationColorObj;
        range["From"] = 0;
        range["To"] = 100;
        saturationColorObj["Range"] = range;

	value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        blSaturationColor =  GetComponentColorEnum(value);
        if(blSaturationColor ==tvDataColor_MAX)
        {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }
        cms += value;
        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);	
	int err = GetLocalparam(cms.c_str(),formatIndex,pqIndex,sourceIndex,saturation,true,COLOR_SATURATION);
	if( err == 0 ) {
            saturationColorObj["Setting"] = std::to_string(saturation);
            response["saturation"] = saturationColorObj;
            LOGINFO("Exit : Component Saturation for color: %s Value: %d\n", value.c_str(),saturation);
            returnResponse(true, "success");
        }
        else {
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }

    }

    uint32_t ControlSettingsTV::setComponentSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int saturation = 0;
        tvDataComponentColor_t blSaturationColor;

        value = parameters.HasLabel("saturation") ? parameters["saturation"].String() : "";
        returnIfParamNotFound(value);
        saturation = stoi(value);

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(value);

	blSaturationColor =  GetComponentColorEnum(value);
        if(blSaturationColor ==tvDataColor_MAX)
        {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        tvError_t ret = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_ENABLE);
        if(ret != tvERROR_NONE) {
            LOGWARN("CMS enable failed\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else
        {
            int cms_params[3]={0};
            cms_params[0]=COLOR_STATE;
            cms_params[1]=COLOR_ENABLE;
            cms_params[2]=COMPONENT_ENABLE;
            int retval = UpdatePQParamsToCache("set","cms.enable",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CMS,cms_params);
            if(retval != 0) {
                LOGWARN("Failed to Save enableflag to ssm_data\n");
            }
            LOGINFO("CMS Enable Success to localstore\n");
        }

        ret = SetCurrentComponentSaturation(blSaturationColor, saturation);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=COLOR_SATURATION;
            params[1]=ConvertTVColorToVendorColor(blSaturationColor);
            params[2]=saturation;

            char tr181format[64]={0};
            snprintf(tr181format,sizeof(tr181format),"saturation.%s",component_color[ConvertTVColorToVendorColor(blSaturationColor)]);
            int retval=UpdatePQParamsToCache("set",tr181format,pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CMS,params);

            if(retval != 0) {
                LOGWARN("Failed to Save component saturation to ssm_data\n");
            }
            LOGINFO("Exit : SetComponentSaturation() %s : %s\n",format,value.c_str());
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::resetComponentSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,contrast=0,ret=0;
        tvError_t err = tvERROR_NONE;
	char param[BUFFER_SIZE]={0};

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";
			
        err = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_DISABLE);
        if(err == tvERROR_NONE)
        {
            int params[3]={0};
            params[0]=COLOR_STATE;
            params[1]=COLOR_ENABLE;
            ret=UpdatePQParamsToCache("reset","cms.enable",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CMS,params);
            if(ret != 0) {
                LOGWARN("Failed to Save enable flag to ssm_data\n");
            }

            for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
            {
                params[0]=COLOR_SATURATION;
                params[1]=color;
                snprintf(param,sizeof(param),"saturation.%s",component_color[color]);
                ret |= UpdatePQParamsToCache("reset",param,pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CMS,params);
                memset(&param, 0, sizeof(param));
            }

            if(ret != 0)
            {
                LOGWARN("resetComponentSaturation Failed couldn't remove from localstore error %d \n", ret);
                err=tvERROR_GENERAL;
            }
        }
		
		if(err != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(err));
        }
        else
        {
            LOGINFO("Exit : resetComponentSaturation()\n");
            returnResponse(true, "success");
        }
    }    

    uint32_t ControlSettingsTV::getComponentHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string cms("hue.");
        std::string value;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        tvDataComponentColor_t blHueColor;
        int hue=0;
        JsonObject range;
        JsonObject hueColorObj;
        range["From"] = 0;
        range["To"] = 100;
        hueColorObj["Range"] = range;

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        blHueColor =  GetComponentColorEnum(value);
        if(blHueColor ==tvDataColor_MAX)
        {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }
        cms += value;
        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam(cms.c_str(),formatIndex,pqIndex,sourceIndex,hue,true,COLOR_HUE);
        if( err == 0 ) {
            hueColorObj["Setting"] = std::to_string(hue);
            response["hue"] = hueColorObj;
            LOGINFO("Exit : Component Hue for color: %s Value: %d\n", value.c_str(),hue);
            returnResponse(true, "success");
        }
        else {
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
    }


    uint32_t ControlSettingsTV::setComponentHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int hue = 0;
        tvDataComponentColor_t blHueColor;

        value = parameters.HasLabel("hue") ? parameters["hue"].String() : "";
        returnIfParamNotFound(value);
        hue = stoi(value);

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(value);

        blHueColor =  GetComponentColorEnum(value);
        if(blHueColor ==tvDataColor_MAX)
        {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        tvError_t ret = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_ENABLE);
        if(ret != tvERROR_NONE) {
            LOGWARN("CMS enable failed\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else
        {
            int cms_params[3]={0};
            cms_params[0]=COLOR_STATE;
            cms_params[1]=COLOR_ENABLE;
            cms_params[2]=COMPONENT_ENABLE;
            int retval = UpdatePQParamsToCache("set","cms.enable",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CMS,cms_params);
            if(retval != 0) {
                LOGWARN("Failed to Save enableflag to ssm_data\n");
            }
            LOGINFO("CMS Enable Success to localstore\n");
        }

        ret = SetCurrentComponentHue(blHueColor, hue);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=COLOR_HUE;
            params[1]=ConvertTVColorToVendorColor(blHueColor);
            params[2]=hue;

            char tr181format[64]={0};
            snprintf(tr181format,sizeof(tr181format),"hue.%s",component_color[ConvertTVColorToVendorColor(blHueColor)]);
            int retval=UpdatePQParamsToCache("set",tr181format,pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CMS,params);

            if(retval != 0) {
                LOGWARN("Failed to Save component hue to ssm_data\n");
            }
            LOGINFO("Exit : SetComponentHue() %s : %s\n",format,value.c_str());
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::resetComponentHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,contrast=0,ret=0;
        tvError_t err = tvERROR_NONE;
        char param[BUFFER_SIZE]={0};

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        err = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_DISABLE);
        if(err == tvERROR_NONE)
        {
            int params[3]={0};
            params[0]=COLOR_STATE;
            params[1]=COLOR_ENABLE;
            ret=UpdatePQParamsToCache("reset","cms.enable",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CMS,params);
            if(ret != 0) {
                LOGWARN("Failed to Save enable flag to ssm_data\n");
            }

            for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
            {
                params[0]=COLOR_HUE;
                params[1]=color;
                snprintf(param,sizeof(param),"hue.%s",component_color[color]);
                ret |= UpdatePQParamsToCache("reset",param,pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CMS,params);
                memset(&param, 0, sizeof(param));
            }

            if(ret != 0)
            {
                LOGWARN("resetComponentHue Failed couldn't remove from localstore error %d \n", ret);
                err=tvERROR_GENERAL;
            }
        }

        if(err != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(err));
        }
        else
        {
            LOGINFO("Exit : resetComponentHue()\n");
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::getComponentLuma(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string cms("luma.");
        std::string value;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        tvDataComponentColor_t blLumaColor;
        int luma=0;
        JsonObject range;
        JsonObject lumaColorObj;
        range["From"] = 0;
        range["To"] = 100;
        lumaColorObj["Range"] = range;

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        blLumaColor =  GetComponentColorEnum(value);
        if(blLumaColor ==tvDataColor_MAX)
        {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }
        cms += value;
	GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        int err = GetLocalparam(cms.c_str(),formatIndex,pqIndex,sourceIndex,luma,true,COLOR_LUMA);
        if( err == 0 ) {
            lumaColorObj["Setting"] = std::to_string(luma);
            response["luma"] = lumaColorObj;
            LOGINFO("Exit : Component Luma for color: %s Value: %d\n", value.c_str(),luma);
            returnResponse(true, "success");
        }
        else {
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
    }
    
    uint32_t ControlSettingsTV::setComponentLuma(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int luma = 0;
        tvDataComponentColor_t blLumaColor;

        value = parameters.HasLabel("luma") ? parameters["luma"].String() : "";
        returnIfParamNotFound(value);
        luma = stoi(value);

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(value);

        blLumaColor =  GetComponentColorEnum(value);
        if(blLumaColor ==tvDataColor_MAX)
        {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        tvError_t ret = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_ENABLE);
        if(ret != tvERROR_NONE) {
            LOGWARN("CMS enable failed\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else
        {
            int cms_params[3]={0};
            cms_params[0]=COLOR_STATE;
            cms_params[1]=COLOR_ENABLE;
            cms_params[2]=COMPONENT_ENABLE;
            int retval = UpdatePQParamsToCache("set","cms.enable",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CMS,cms_params);
            if(retval != 0) {
                LOGWARN("Failed to Save enableflag to ssm_data\n");
            }
            LOGINFO("CMS Enable Success to localstore\n");
        }

        ret = SetCurrentComponentLuma(blLumaColor, luma);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=COLOR_LUMA;
            params[1]=ConvertTVColorToVendorColor(blLumaColor);
            params[2]=luma;

            char tr181format[64]={0};
            snprintf(tr181format,sizeof(tr181format),"luma.%s",component_color[ConvertTVColorToVendorColor(blLumaColor)]);
            int retval=UpdatePQParamsToCache("set",tr181format,pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CMS,params);

            if(retval != 0) {
                LOGWARN("Failed to Save component luma to ssm_data\n");
            }
            LOGINFO("Exit : SetComponentLuma() %s : %d\n",value.c_str(),params[2]);
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::resetComponentLuma(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int sourceIndex=0,pqIndex=0,formatIndex=0,contrast=0,ret=0;
        tvError_t err = tvERROR_NONE;
        char param[BUFFER_SIZE]={0};

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        err = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_DISABLE);
        if(err == tvERROR_NONE)
        {
            int params[3]={0};
            params[0]=COLOR_STATE;
            params[1]=COLOR_ENABLE;
            ret=UpdatePQParamsToCache("reset","cms.enable",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CMS,params);
            if(ret != 0) {
                LOGWARN("Failed to Save enable flag to ssm_data\n");
            }

            for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
            {
                params[0]=COLOR_LUMA;
                params[1]=color;
                snprintf(param,sizeof(param),"luma.%s",component_color[color]);
                ret |= UpdatePQParamsToCache("reset",param,pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_CMS,params);
                memset(&param, 0, sizeof(param));
            }

            if(ret != 0)
            {
                LOGWARN("resetComponentLuma Failed couldn't remove from localstore error %d \n", ret);
                err=tvERROR_GENERAL;
            }
        }

        if(err != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(err));
        }
        else
        {
            LOGINFO("Exit : resetComponentLuma()\n");
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::getBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
	std::string key;
	int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        int dimmingMode = 0;

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
        int err = GetLocalparam("DimmingMode",formatIndex,pqIndex,sourceIndex,dimmingMode);
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
            returnResponse(true, "success");
        }
        else {
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
    }

    uint32_t ControlSettingsTV::setBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
        int dimmingMode = 0;

        value = parameters.HasLabel("DimmingMode") ? parameters["DimmingMode"].String() : "";
        returnIfParamNotFound(value);

        dimmingMode = GetDimmingModeIndex(value.c_str());

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "current";

        tvError_t ret = SetTVDimmingMode(value.c_str());	

	if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set DimmingMode\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=dimmingMode;
            int retval= UpdatePQParamsToCache("set","DimmingMode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_LDIM,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save DimmingMode to ssm_data\n");
            }
            LOGINFO("Exit : setDimmingMode successful to value: %d\n", dimmingMode);
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::resetBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");

        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;std::string dimmingMode;
        int sourceIndex=0,pqIndex=0,formatIndex=0,dMode=0;
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

	int retval= UpdatePQParamsToCache("reset","DimmingMode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_LDIM,params);

        if(retval != 0 ) {
            LOGWARN("Failed to reset ldim\n");
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("DimmingMode",formatIndex,pqIndex,sourceIndex,dMode);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,dMode);
                GetDimmingModeStringFromEnum(dMode,dimmingMode);
                ret = SetTVDimmingMode(dimmingMode.c_str());
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
            LOGINFO("Exit : resetColorTemperature Successful to value : %s \n",dimmingMode);
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::getAutoBacklightControl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvBacklightMode_t blMode;
        JsonArray supportedBLModes;
        int blModes = tvBacklightMode_NONE;
        tvError_t ret = GetSupportedBacklightModes(&blModes);
        if(ret != tvERROR_NONE) {
            supportedBLModes.Add("none");
        }
        else {
            if(!blModes)supportedBLModes.Add("none");
            if(blModes & tvBacklightMode_MANUAL)supportedBLModes.Add("manual");
            if(blModes & tvBacklightMode_AMBIENT)supportedBLModes.Add("ambient");
            if(blModes & tvBacklightMode_ECO)supportedBLModes.Add("eco");
        }

        JsonObject backlightCtrlObj;

        ret = GetCurrentBacklightMode(&blMode);
        if(ret != tvERROR_NONE) {
            response["mode"] = "none";
            response["supportedModes"] = supportedBLModes;
            returnResponse(false, getErrorString(ret).c_str());
        }
	else {
            switch(blMode) {

                case tvBacklightMode_NONE:
                    LOGINFO("Auto Backlight Control Mode: none\n");
                    response["mode"] = "none";
                    break;

                case tvBacklightMode_MANUAL:
                    LOGINFO("Auto Backlight Control Mode: manual\n");
                    response["mode"] = "manual";
                    break;

                case tvBacklightMode_AMBIENT:
                    LOGINFO("Auto Backlight Control Mode: ambient\n");
                    response["mode"] = "ambient";
                    break;

                case tvBacklightMode_ECO:
                    LOGINFO("Auto Backlight Control Mode: eco\n");
                    response["mode"] = "eco";
                    break;

                default:
                    LOGINFO("Auto Backlight Control Mode: none\n");
                    response["mode"] = "none";
                    break;
            }

            response["supportedModes"] = supportedBLModes;
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::setAutoBacklightControl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        tvBacklightMode_t blMode = tvBacklightMode_NONE;

        value = parameters.HasLabel("mode") ? parameters["mode"].String() : "";
        returnIfParamNotFound(value);

        if(!value.compare("none")) {
            blMode = tvBacklightMode_NONE;
        }
        else if (!value.compare("manual")){
            blMode = tvBacklightMode_MANUAL;
        }
        else if (!value.compare("ambient")){
            blMode = tvBacklightMode_AMBIENT;
        }
        else if (!value.compare("eco")){
            blMode = tvBacklightMode_ECO;
        }
        else {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }


        tvError_t ret = SetCurrentBacklightMode(blMode);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, value.c_str());
            }
            LOGINFO("Exit : value :%s\n",value.c_str());
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::resetAutoBacklightControl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvError_t ret = tvERROR_NONE;

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));

            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM);

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
            returnResponse(false, getErrorString(ret));
        }
        else
        {
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

    int ControlSettingsTV::GetLocalparam(const char * forParam,int formatIndex,int pqIndex,int sourceIndex,int &value,bool cms,int tunnel_type)
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
	   else if(strncmp(forParam,"DimmingMode",strlen(forParam))==0)
           {
               if (strncmp(param.value, "fixed", strlen(param.value))==0)
                   value=tvDimmingMode_Fixed;
               else if (strncmp(param.value, "local", strlen(param.value))==0)
                   value=tvDimmingMode_Local;
               else if (strncmp(param.value, "global", strlen(param.value))==0)
                   value=tvDimmingMode_Global;
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
            if(cms)
            {
                value=GetCMSDefault((tvCMS_tunel_t)tunnel_type);
                return 0;/*block default cms sync and allow default values during reset*/
            }
	    else
            {
                key.clear();
                memset(&param, 0, sizeof(param));
                key+=std::string(TVSETTINGS_GENERIC_STRING_RFC_PARAM);
                key+=STRING_SOURCE+std::string("ALL")+std::string(".")+STRING_PICMODE+std::to_string(pqIndex)+std::string(".")+std::string(STRING_FORMAT)+std::to_string(formatIndex)+std::string(".")+forParam;
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
        else if( (strncmp(format,"Allminusdolby",strlen(format)) == 0) || (strncmp(format,"allminusdolby",strlen(format)) == 0) )
        {
            unsigned int lcount=0;
             for(;(lcount<sizeof(uint32_t)*8 && numberOfSupportedFormats);lcount++)
             {
                 tvhdr_type_t formatToStore = (tvhdr_type_t)ConvertVideoFormatToHDRFormat((tvVideoHDRFormat_t)(contentFormats&(1<<lcount)));
                 if(formatToStore!= HDR_TYPE_NONE)
                 {
                     numberOfSupportedFormats--;
                     if( formatToStore == HDR_TYPE_DOVI )
                         continue;
                     formats.push_back(formatToStore);
                 }
             }
	}
        else if( (strncmp(format,"group",strlen(format)) == 0) )
        {
            if(isCurrentHDRTypeIsSDR())
                formats.push_back(HDR_TYPE_SDR);
            else {
                formats.push_back(HDR_TYPE_HDR10);
                formats.push_back(HDR_TYPE_HLG);
                formats.push_back(HDR_TYPE_DOVI);
	    }
	}
        else if( (strncmp(format,"sdrgroup",strlen(format)) == 0) || (strncmp(format,"SDRgroup",strlen(format)) == 0) )
        {
            formats.push_back(HDR_TYPE_SDR);      
        }
        else if( (strncmp(format,"hdrgroup",strlen(format)) == 0) || (strncmp(format,"HDRgroup",strlen(format)) == 0) )
        {
            formats.push_back(HDR_TYPE_HDR10);
            formats.push_back(HDR_TYPE_HLG);
            formats.push_back(HDR_TYPE_DOVI);
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
                else if(!strncmp(forParam,"DimmingMode",strlen("DimmingMode")))
                {
                    GetDimmingModeStringFromEnum(value, toStore);
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
                            case PQ_PARAM_LDIM:
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
                                ret |= SaveColorTemperature(source, mode,format,params[0]);
                                break;
                            case PQ_PARAM_LDIM:
                                ret |= ret |= SaveDynamicBacklight(source,mode,format,params[0]);
                                break;
                            case PQ_PARAM_CMS:
                                if(reset)
                                    ret |= UpdatePQParamToLocalCache(tr181ParamName,source, mode, format,0,false);
                                if(sync || reset)
                                {
                                    int value=0;
                                    if(!GetLocalparam(tr181ParamName,mode,format,source,value,true,params[0]))
                                        LOGINFO("Found param from tr181 CMS pqmode : %d format:%d value:%d\n",mode,format,value);
                                    else{
                                        if(sync) /*block default cms sync to save tvsettings init time*/
                                            break;
                                     }
                                     params[2]=value;
                                }
                               ret |= SaveCMS(source, mode,format,params[0],params[1],params[2]);
                               if(set)
                                   ret |= UpdatePQParamToLocalCache(tr181ParamName,source,mode, format, params[2],true);
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

    void ControlSettingsTV::GetDimmingModeStringFromEnum(int value, std::string &toStore)
    {
        const char *color_temp_string[] = {
                    [tvDimmingMode_Fixed] = "fixed",
                    [tvDimmingMode_Local] = "local",
                    [tvDimmingMode_Global] = "global",
                };
        toStore.clear();
        toStore+=color_temp_string[value];
    }
    
    tvError_t ControlSettingsTV::saveBacklightToLocalStoreForGBF(const char* key, const char* value)
    {
        tvError_t ret = tvERROR_NONE;
        tr181ErrorCode_t err;
        const char * paramToSet = isCurrentHDRTypeIsSDR()?TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM:TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM;

        err = setLocalParam(rfc_caller_id, paramToSet, value);
        if ( err != tr181Success ) {
            LOGWARN("setLocalParam for %s Failed : %s\n", paramToSet, getTR181ErrorString(err));
            ret = tvERROR_GENERAL;
        }
        else {
            LOGINFO("setLocalParam %s Successful, Value: %s\n", paramToSet,value);
        }
        return ret;
    }

    int ControlSettingsTV::GetLastSetBacklightForGBF(int &backlight)
    {
        int ret = 0;
        TR181_ParamData_t param={0};

        const char * paramToRead = isCurrentHDRTypeIsSDR()?TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM:TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, paramToRead, &param);
        if ( tr181Success == err )
        {
            backlight = std::stoi(param.value);
        }
        else
        {
            LOGWARN("Reading %s fails \n",paramToRead);
            ret = 1;
        }

        return ret;
    }

    tvDataComponentColor_t ControlSettingsTV::GetComponentColorEnum(std::string colorName)
    {
        tvDataComponentColor_t CompColorEnum = tvDataColor_MAX;

        if(!colorName.compare("none")) {
            CompColorEnum = tvDataColor_NONE;
        }
        else if (!colorName.compare("red")){
            CompColorEnum = tvDataColor_RED;
        }
        else if (!colorName.compare("green")){
            CompColorEnum = tvDataColor_GREEN;
        }
        else if (!colorName.compare("blue")){
            CompColorEnum = tvDataColor_BLUE;
        }
        else if (!colorName.compare("yellow")){
            CompColorEnum = tvDataColor_YELLOW;
        }
        else if (!colorName.compare("cyan")){
            CompColorEnum = tvDataColor_CYAN;
        }
        else if (!colorName.compare("magenta")){
            CompColorEnum = tvDataColor_MAGENTA;
        }
        return CompColorEnum;
    }

    int ControlSettingsTV::GetDimmingModeIndex(const char* mode)
    {
        unsigned short index = 1;

        if(strncmp(mode,"local",strlen("local"))==0)
            index=tvDimmingMode_Local;
        else if(strncmp(mode,"fixed",strlen("fixed"))==0)
            index=tvDimmingMode_Fixed;
        else if(strncmp(mode,"global",strlen("global"))==0)
            index=tvDimmingMode_Global;
        else
            LOGWARN("Return Default Dimmingmode:%d!!!\n",index);

        return index;

    }


}//namespace Plugin
}//namespace WPEFramework
