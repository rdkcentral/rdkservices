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
#include "als_bl_iniparser.h"
#include "bl_table.h"

#define TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.SDR.Backlight"
#define TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.HDR.Backlight"
#define TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM  "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.AutoBacklightMode"
#define TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.DolbyVisionMode"
#define TVSETTINGS_HLGMODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.HLGMode"
#define TVSETTINGS_HDR10MODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.HDR10Mode"
#define TVSETTINGS_CONVERTERBOARD_PANELID     "0_0_00"

#define SAVE_FOR_ALL_SOURCES    (-1)
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

#define registerMethod(...) Register(__VA_ARGS__);GetHandler(2)->Register<JsonObject, JsonObject>(__VA_ARGS__)
namespace WPEFramework {
namespace Plugin {

    ControlSettingsTV::ControlSettingsTV(): PluginHost::JSONRPC(),numberModesSupported(0),pic_mode_index()
                                           ,appUsesGlobalBackLightFactor(false),source_index()
                                           ,rfc_caller_id()
    						
    {
        LOGINFO("Entry\n");
        instance = this;
	CreateHandler({ 2 });
        Register("getBacklight", &ControlSettingsTV::getBacklight, this);
        registerMethod("setBacklight", &ControlSettingsTV::setBacklight, this);
        registerMethod("resetBacklight", &ControlSettingsTV::resetBacklight, this);
        Register("getBrightness", &ControlSettingsTV::getBrightness, this);
        registerMethod("setBrightness", &ControlSettingsTV::setBrightness, this);
        registerMethod("resetBrightness", &ControlSettingsTV::resetBrightness, this);
        Register("getContrast", &ControlSettingsTV::getContrast, this);
        registerMethod("setContrast", &ControlSettingsTV::setContrast, this);
        registerMethod("resetContrast", &ControlSettingsTV::resetContrast, this);
        Register("getSharpness", &ControlSettingsTV::getSharpness, this);
        registerMethod("setSharpness", &ControlSettingsTV::setSharpness, this);
        registerMethod("resetSharpness", &ControlSettingsTV::resetSharpness, this);
        Register("getSaturation", &ControlSettingsTV::getSaturation, this);
        registerMethod("setSaturation", &ControlSettingsTV::setSaturation, this);
        registerMethod("resetSaturation", &ControlSettingsTV::resetSaturation, this);
        Register("getHue", &ControlSettingsTV::getHue, this);
        registerMethod("setHue", &ControlSettingsTV::setHue, this);
        registerMethod("resetHue", &ControlSettingsTV::resetHue, this);
        Register("getColorTemperature", &ControlSettingsTV::getColorTemperature, this);
        registerMethod("setColorTemperature", &ControlSettingsTV::setColorTemperature, this);
        registerMethod("resetColorTemperature", &ControlSettingsTV::resetColorTemperature, this);

        registerMethod("getComponentSaturation", &ControlSettingsTV::getComponentSaturation, this);
        registerMethod("setComponentSaturation", &ControlSettingsTV::setComponentSaturation, this);
        registerMethod("resetComponentSaturation", &ControlSettingsTV::resetComponentSaturation, this);
        registerMethod("getComponentHue", &ControlSettingsTV::getComponentHue, this);
        registerMethod("setComponentHue", &ControlSettingsTV::setComponentHue, this);
        registerMethod("resetComponentHue", &ControlSettingsTV::resetComponentHue, this);
        registerMethod("getComponentLuma", &ControlSettingsTV::getComponentLuma, this);
        registerMethod("setComponentLuma", &ControlSettingsTV::setComponentLuma, this);
        registerMethod("resetComponentLuma", &ControlSettingsTV::resetComponentLuma, this);
        registerMethod("getBacklightDimmingMode", &ControlSettingsTV::getBacklightDimmingMode, this);
        registerMethod("setBacklightDimmingMode", &ControlSettingsTV::setBacklightDimmingMode, this);
        registerMethod("resetBacklightDimmingMode", &ControlSettingsTV::resetBacklightDimmingMode, this);
        registerMethod("getAutoBacklightControl", &ControlSettingsTV::getAutoBacklightControl, this);
        registerMethod("setAutoBacklightControl", &ControlSettingsTV::setAutoBacklightControl, this);
        registerMethod("resetAutoBacklightControl", &ControlSettingsTV::resetAutoBacklightControl, this);

        registerMethod("getSupportedDolbyVisionModes", &ControlSettingsTV::getSupportedDolbyVisionModes, this);
        registerMethod("getDolbyVisionMode", &ControlSettingsTV::getDolbyVisionMode, this);
        registerMethod("setDolbyVisionMode", &ControlSettingsTV::setDolbyVisionMode, this);
        registerMethod("resetDolbyVisionMode", &ControlSettingsTV::resetDolbyVisionMode, this);
        registerMethod("getSupportedHLGModes", &ControlSettingsTV::getSupportedHLGModes, this);
        registerMethod("getHLGMode", &ControlSettingsTV::getHLGMode, this);
        registerMethod("setHLGMode", &ControlSettingsTV::setHLGMode, this);
        registerMethod("resetHLGMode", &ControlSettingsTV::resetHLGMode, this);
        registerMethod("getSupportedHDR10Modes", &ControlSettingsTV::getSupportedHDR10Modes, this);
	registerMethod("getHDR10Mode", &ControlSettingsTV::getHDR10Mode, this);
	registerMethod("setHDR10Mode", &ControlSettingsTV::setHDR10Mode, this);
	registerMethod("resetHDR10Mode", &ControlSettingsTV::resetHDR10Mode, this);
	
	registerMethod("getWBInfo", &ControlSettingsTV::getWBInfo, this);
	registerMethod("getWBCtrl", &ControlSettingsTV::getWBCtrl, this);
	registerMethod("setWBCtrl", &ControlSettingsTV::setWBCtrl, this);
	registerMethod("resetWBCtrl", &ControlSettingsTV::resetWBCtrl, this);

	GetHandler(2)->Register<JsonObject, JsonObject>("getBacklight", &ControlSettingsTV::getBacklight2, this);
        GetHandler(2)->Register<JsonObject, JsonObject>("getBrightness", &ControlSettingsTV::getBrightness2, this);
        GetHandler(2)->Register<JsonObject, JsonObject>("getContrast", &ControlSettingsTV::getContrast2, this);
        GetHandler(2)->Register<JsonObject, JsonObject>("getSharpness", &ControlSettingsTV::getSharpness2, this);
        GetHandler(2)->Register<JsonObject, JsonObject>("getSaturation", &ControlSettingsTV::getSaturation2, this);
        GetHandler(2)->Register<JsonObject, JsonObject>("gethue", &ControlSettingsTV::getHue2, this);
        GetHandler(2)->Register<JsonObject, JsonObject>("getColorTemperature", &ControlSettingsTV::getColorTemperature2, this);


        LOGINFO("Exit\n");
    }
    
    ControlSettingsTV :: ~ControlSettingsTV()
    {
        LOGINFO();    
    }

    void ControlSettingsTV::Initialize()
    {
       LOGINFO("Entry\n");
       std::system("echo \"Testing ControlSettingsTV dmesg [starts] - ControlSettingsTV::Initialize()\" > /dev/kmsg");
       tvError_t ret = tvERROR_NONE;

       TR181_ParamData_t param;
       memset(&param, 0, sizeof(param));
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

       std::system("echo \"Testing dmesg [starts..before SyncPQParamsToDriverCache] - ControlSettingsTV::Initialize()\" > /dev/kmsg");

       SyncPQParamsToDriverCache();

       std::system("echo \"Testing dmesg [starts..after SyncPQParamsToDriverCache] - ControlSettingsTV::Initialize()\" > /dev/kmsg");

       tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, &param);
       if ( tr181Success == err )
       {
           LOGINFO("getLocalParam for %s is %s\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, param.value);
           ret = SetTVPictureMode(param.value);

           if(ret != tvERROR_NONE) {
               LOGWARN("Picture Mode set failed: %s\n",getErrorString(ret).c_str());
           }
           else {
               LOGINFO("Picture Mode initialized successfully, value: %s\n", param.value);
           }
       }
       else
       {
           LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, getTR181ErrorString(err));
       }

       std::system("echo \"Testing dmesg [starts..before InitializeSDRHDRBacklight] - ControlSettingsTV::Initialize()\" > /dev/kmsg");

       //Initialize Backlight
       if(appUsesGlobalBackLightFactor)
       {
           if(!InitializeSDRHDRBacklight())
               LOGINFO("InitializeSDRHDRBacklight() : Success\n");
           else
               LOGWARN("InitializeSDRHDRBacklight() : Failed\n");
       }
       std::system("echo \"Testing dmesg [starts..after InitializeSDRHDRBacklight] - ControlSettingsTV::Initialize()\" > /dev/kmsg");

       tvBacklightMode_t blMode = tvBacklightMode_NONE;
       memset(&param, 0, sizeof(param));
       err = getLocalParam(rfc_caller_id, TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, &param);
       if ( tr181Success == err )
       {
           LOGINFO("getLocalParam for %s is %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, param.value);

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
               LOGWARN("Auto Backlight Mode set failed: %s\n",getErrorString(ret).c_str());
           }
           else {
               LOGINFO("Auto Backlight Mode initialized successfully, value: %s\n", param.value);
           }

        }
	else
        {
            LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
        }
        std::system("echo \"Testing dmesg [before..setDefaultAspectRatio] - ControlSettingsTV::SyncPQParamsToDriverCache()\" > /dev/kmsg");
        setDefaultAspectRatio();
        std::system("echo \"Testing dmesg [after..setDefaultAspectRatio] - ControlSettingsTV::SyncPQParamsToDriverCache()\" > /dev/kmsg");

       std::system("echo \"Testing dmesg [ends] - ControlSettingsTV::Initialize()\" > /dev/kmsg");
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

    uint32_t ControlSettingsTV::getBacklight2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        JsonObject range;
        JsonObject backlightObj;
        range["From"] = 0;
        range["To"] = 100;
        backlightObj["Range"] = range;
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
            backlightObj["Setting"] = std::to_string(backlight);
            response["Backlight"] = backlightObj;
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

    uint32_t ControlSettingsTV::getBrightness2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        int brightness = 0;
        JsonObject range;
        JsonObject brightnessObj;
        range["From"] = 0;
        range["To"] = 100;
        brightnessObj["Range"] = range;

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
            brightnessObj["Setting"] = std::to_string(brightness);
            response["Brightness"] = brightnessObj;
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
	tvError_t ret = tvERROR_NONE;

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
        
	if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with setBrightness\n");
             ret = SetBrightness(brightness);
        }
        else
            LOGINFO("Set not required for this request!!! Just Save it\n");

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

    uint32_t ControlSettingsTV::getContrast2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        int contrast = 0;
        JsonObject range;
        JsonObject contrastObj;
        range["From"] = 0;
        range["To"] = 100;
        contrastObj["Range"] = range;

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
            contrastObj["Setting"] = std::to_string(contrast);
            response["Contrast"] = contrastObj;
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

    uint32_t ControlSettingsTV::getSaturation2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        int saturation = 0;
        JsonObject range;
        JsonObject saturationObj;
        range["From"] = 0;
        range["To"] = 100;
        saturationObj["Range"] = range;

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
            saturationObj["Setting"] = std::to_string(saturation);
            response["Saturation"] = saturationObj;
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

    uint32_t ControlSettingsTV::getSharpness2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        int sharpness = 0;
        JsonObject range;
        JsonObject sharpnessObj;
        range["From"] = 0;
        range["To"] = 100;
        sharpnessObj["Range"] = range;

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
            sharpnessObj["Setting"] = std::to_string(sharpness);
            response["Sharpness"] = sharpnessObj;
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

    uint32_t ControlSettingsTV::getHue2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        int hue = 0;
        JsonObject range;
        JsonObject hueObj;
        range["From"] = 0;
        range["To"] = 100;
        hueObj["Range"] = range;

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
            hueObj["Setting"] = std::to_string(hue);
            response["Hue"] = hueObj;
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

    uint32_t ControlSettingsTV::getColorTemperature2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");

        std::string pqmode;
        std::string source;
        std::string format;
        std::string key;
        int sourceIndex=0,pqIndex=0,formatIndex=0;
        tvError_t ret = tvERROR_NONE;
        int colortemp = 0;
		JsonObject ctObj;
        JsonArray ctOptions;
        ctOptions.Add("Standard"); ctOptions.Add("Warm"); ctOptions.Add("Cold"); ctOptions.Add("User Defined");
        ctObj["Selected"] = "Standard";
        ctObj["Options"] = ctOptions;

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
                    ctObj["Selected"] = "Standard";
                    break;

                case tvColorTemp_WARM:
                    LOGINFO("Color Temp Value: Warm\n");
                    ctObj["Selected"] = "Warm";
                    break;

                case tvColorTemp_COLD:
                    LOGINFO("Color Temp Value: Cold\n");
                    ctObj["Selected"] = "Cold";
                    break;

                case tvColorTemp_USER:
                    LOGINFO("Color Temp Value: User Defined\n");
                    ctObj["Selected"] = "User Defined";
                    break;

                default:
                    LOGINFO("Color Temp Value: Standard\n");
                    ctObj["Selected"] = "Standard";
                    break;
            }
            LOGINFO("Exit : ColorTemperature Value: %d \n", colortemp);
            response["ColorTemperature"] = ctObj;
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

    uint32_t ControlSettingsTV::getSupportedDolbyVisionModes(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        pic_modes_t *dvModes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedDVModes(&dvModes,&totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            JsonArray SupportedDVModes;

            for(int count = 0;count <totalAvailable;count++ )
            {
                SupportedDVModes.Add(dvModes[count].name);
            }

            response["SupportedDVModes"] = SupportedDVModes;
            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::getDolbyVisionMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry");
        TR181_ParamData_t param;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, &param);
        if (err!= tr181Success) {
            returnResponse(false,getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            std::string s;
            s+=param.value;
            response["DolbyVisionMode"] = s;
            LOGINFO("Exit getDolbyVisionMode(): %s\n",s.c_str());
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::setDolbyVisionMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;

	value = parameters.HasLabel("DolbyVisionMode") ? parameters["DolbyVisionMode"].String() : "";
        returnIfParamNotFound(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "dolby";


        tvError_t ret = SetTVDolbyVisionMode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=GetDolbyModeIndex(value.c_str());
            int retval=UpdatePQParamsToCache("set","DolbyVisionMode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_DOLBY_MODE,params);
            if(retval != 0) {
                LOGWARN("Failed to Save DolbyVisionMode to ssm_data\n");
            }
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, value.c_str());
            }

            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::resetDolbyVisionMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
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
            format = "dolby";

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));
            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM);

                ret = SetTVDolbyVisionMode(param.value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("DV Mode set failed: %s\n",getErrorString(ret).c_str());
                }
                else {

                    LOGINFO("DV Mode initialized successfully value %s\n",param.value);
                    //Save DolbyVisionMode to ssm_data
                    int params[3]={0};
                    params[0]=GetDolbyModeIndex(param.value);
                    int retval=UpdatePQParamsToCache("reset","DolbyVisionMode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_DOLBY_MODE,params);

                    if(retval != 0) {
                        LOGWARN("Failed to Save DolbyVisionMode to ssm_data\n");
                        ret=tvERROR_GENERAL;
                    }
                }
            }
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : resetDolbyVisionMode() \n");
            returnResponse(true, "success");
        }
    }



    uint32_t ControlSettingsTV::getSupportedHDR10Modes(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        pic_modes_t *hdr10Modes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedHDR10Modes(&hdr10Modes,&totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            JsonArray SupportedHDR10Modes;

            for(int count = 0;count <totalAvailable;count++ )
            {
                SupportedHDR10Modes.Add(hdr10Modes[count].name);
            }

            response["SupportedHDR10Modes"] = SupportedHDR10Modes;
            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::setHDR10Mode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;

        value = parameters.HasLabel("DolbyVisionMode") ? parameters["DolbyVisionMode"].String() : "";
        returnIfParamNotFound(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "hdr10";

        tvError_t ret = SetTVHDR10Mode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=GetHDR10ModeIndex(value.c_str());
            int retval=UpdatePQParamsToCache("set","HDR10Mode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_HDR10_MODE,params);
            if(retval != 0) {
                LOGWARN("Failed to Save HDR10Mode to ssm_data\n");
            }
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_HDR10MODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_HDR10MODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_HDR10MODE_RFC_PARAM, value.c_str());
            }

            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }
    }

      
    uint32_t ControlSettingsTV::getHDR10Mode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        TR181_ParamData_t param;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HDR10MODE_RFC_PARAM, &param);
        if (err!= tr181Success) {
            returnResponse(false,getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            std::string s;
            s+=param.value;
            response["HDR10Mode"] = s;
            LOGINFO("Exit getHDR10Mode(): %s\n",s.c_str());
            returnResponse(true, "success");
        }

    }

    uint32_t ControlSettingsTV::resetHDR10Mode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
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
            format = "hdr10";

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_HDR10MODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_HDR10MODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_HDR10MODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));
			tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HDR10MODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_HDR10MODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", TVSETTINGS_HDR10MODE_RFC_PARAM);

                ret = SetTVHDR10Mode(param.value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("DV Mode set failed: %s\n",getErrorString(ret).c_str());
                }
                else {

                    LOGINFO("DV Mode initialized successfully value %s\n",param.value);
                    //Save HDR10Mode to ssm_data
                    int params[3]={0};
                    params[0]=GetHDR10ModeIndex(param.value);
                    int retval=UpdatePQParamsToCache("reset","HDR10Mode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_HDR10_MODE,params);

                    if(retval != 0) {
                        LOGWARN("Failed to Save HDR10Mode to ssm_data\n");
                        ret=tvERROR_GENERAL;
                    }
                }
            }
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : %s \n",__FUNCTION__);
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::getSupportedHLGModes(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        pic_modes_t *hlgModes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedHLGModes(&hlgModes,&totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            JsonArray SupportedHLGModes;

            for(int count = 0;count <totalAvailable;count++ )
            {
                SupportedHLGModes.Add(hlgModes[count].name);
            }

            response["SupportedHLGModes"] = SupportedHLGModes;
            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::getHLGMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        TR181_ParamData_t param;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HLGMODE_RFC_PARAM, &param);
        if (err!= tr181Success) {
            returnResponse(false,getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            std::string s;
            s+=param.value;
            response["HLGMode"] = s;
            LOGINFO("Exit getHLGMode(): %s\n",s.c_str());
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::setHLGMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;

        value = parameters.HasLabel("HLGMode") ? parameters["HLGMode"].String() : "";
        returnIfParamNotFound(value);

        pqmode = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        if(pqmode.empty())
            pqmode = "current";

        source = parameters.HasLabel("source") ? parameters["source"].String() : "";
        if(source.empty())
            source = "current";

        format = parameters.HasLabel("format") ? parameters["format"].String() : "";
        if(format.empty())
            format = "hlg";

        tvError_t ret = SetTVHLGMode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=GetHLGModeIndex(value.c_str());
            int retval=UpdatePQParamsToCache("set","HLGMode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_HLG_MODE,params);
            if(retval != 0) {
                LOGWARN("Failed to Save HLGMode to ssm_data\n");
            }
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_HLGMODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_HLGMODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_HLGMODE_RFC_PARAM, value.c_str());
            }

            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::resetHLGMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;
        std::string pqmode;
        std::string source;
        std::string format;
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
            format = "hlg";

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_HLGMODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_HLGMODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_HLGMODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));
			tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HLGMODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_HLGMODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", TVSETTINGS_HLGMODE_RFC_PARAM);

                ret = SetTVHLGMode(param.value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("DV Mode set failed: %s\n",getErrorString(ret).c_str());
                }
                else {

                    LOGINFO("DV Mode initialized successfully value %s\n",param.value);
                    //Save HLGMode to ssm_data
                    int params[3]={0};
                    params[0]=GetHLGModeIndex(param.value);
                    int retval=UpdatePQParamsToCache("reset","HLGMode",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_HLG_MODE,params);

                    if(retval != 0) {
                        LOGWARN("Failed to Save HLGMode to ssm_data\n");
                        ret=tvERROR_GENERAL;
                    }
                }
            }
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : %s \n",__FUNCTION__);
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::getWBInfo(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        JsonArray supportedWBSelectors;
        JsonObject supportedWBSelectorInfo;
        JsonArray colorTemp;
        JsonArray inputs;
        JsonArray supportedWBCtrls;
        JsonObject supportedWBCtrlInfo;
        JsonObject gain;
        JsonObject offset;
        tvError_t ret;
        unsigned int index = 0;
        getWBInfo_t wbInfo;
        std::vector<std::string> wbSelector;
        std::vector<std::string> wbColorTmp;
        std::vector<std::string> wbInput;

        /* HAL call for getWBInfo */
        ret = getWbInfo(&wbInfo, wbSelector, wbColorTmp, wbInput);
        if(ret != tvERROR_NONE) {
            LOGWARN("getWbInfo() Failed!!! \n");
        }
        for (index = 0; index < wbSelector.size(); index++) {
            supportedWBSelectors.Add(wbSelector[index]);
        }
        response["supportedWBSelectors"] = supportedWBSelectors;

        for (index = 0; index < wbColorTmp.size(); index++) {
            colorTemp.Add(wbColorTmp[index]);
        }

        for (index = 0; index < wbInput.size(); index++) {
            inputs.Add(wbInput[index]);
        }
        if (wbColorTmp.size() != 0) {
            supportedWBSelectorInfo[wbSelector[0].c_str()] = colorTemp;
        }
        if (wbInput.size() != 0) {
            supportedWBSelectorInfo[wbSelector[1].c_str()] = inputs;
        }
        response["supportedWBSelectorInfo"] = supportedWBSelectorInfo;

        for (index = 0; index < 2; index++) {
            supportedWBCtrls.Add(wbInfo.wbControls[index]);
        }
        response["supportedWBCtrls"] = supportedWBCtrls;
        gain["min"] = std::string(wbInfo.wbGain[0]);
        gain["max"] = std::string(wbInfo.wbGain[1]);
        supportedWBCtrlInfo[wbInfo.wbControls[0]] = gain;
        offset["min"] = std::string(wbInfo.wbOffset[0]);
        offset["max"] = std::string(wbInfo.wbOffset[1]);
        supportedWBCtrlInfo[wbInfo.wbControls[1]] = offset;
        response["supportedWBCtrlInfo"] = supportedWBCtrlInfo;
        returnResponse(true, "success");
    }

    uint32_t ControlSettingsTV::getWBCtrl(const JsonObject& parameters,JsonObject& response)
    {
        LOGINFO("Entry\n");
        JsonArray applies;
        JsonObject obj;
        std::vector<std::string> selector;
        std::vector<std::string> index;
        std::string color;
        std::string ctrl;
        int value;
        if ((parameters.HasLabel("applies")) && (parameters.HasLabel("color")) && (parameters.HasLabel("ctrl")))
        {
            color = parameters["color"].String();
            ctrl = parameters["ctrl"].String();
            applies = parameters["applies"].Array();
            for (int i = 0; i < applies.Length(); ++i)
            {
                obj = applies[i].Object();
                selector.push_back(obj["selector"].String());
                index.push_back(obj["index"].String());
            }
            std::string colorTemp;
            std::string inputSrc;
            int pos;
            std::string temp = "color temp";
            std::string input = "input";
            auto found = find(selector.begin(), selector.end(), temp);
            auto found1 = find(selector.begin(), selector.end(), input);
            if(found1 == selector.end()) {
                LOGINFO("Input source not passed");
                inputSrc = "";
            } else {
                pos = found1 - selector.begin();
                inputSrc = index.at(pos);
            }
			if(found != selector.end())
            {
                pos = found - selector.begin();
                colorTemp = index.at(pos);

                LOGINFO("input source : %s\ncolortemp : %s\ncolor : %s\nctrl : %s\n", inputSrc.c_str(), colorTemp.c_str(), color.c_str(), ctrl.c_str());
                // Call to HAL API to getWBctrl
                tvError_t ret = getWBctrl (const_cast <char *> (inputSrc.c_str()), const_cast <char *> (colorTemp.c_str()), const_cast <char *> (color.c_str()), const_cast <char *> (ctrl.c_str()), &value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("getWBCtrl failed");
                    returnResponse(false, getErrorString(ret).c_str());
                } else {
                    LOGINFO("Exit : getWBCtrl success");
                    response["value"] = value;
                    returnResponse(true, "success");
                }

            } else {
                returnResponse(false, "wrong selector value");
            }
        } else {
            returnResponse(false, "Missing Parameter");
        }
    }

    uint32_t ControlSettingsTV::setWBCtrl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        JsonArray applies;
        JsonObject obj;
        std::vector<std::string> selector;
        std::vector<std::string> index;
        std::string val;
        std::string color;
        std::string ctrl;
        int value;
        if(parameters.HasLabel("applies") && parameters.HasLabel("color") && parameters.HasLabel("ctrl") && parameters.HasLabel("value"))
        {
            color = parameters["color"].String();
            ctrl = parameters["ctrl"].String();
            val = parameters["value"].String();
            value = std::stoi(val);
            applies = parameters["applies"].Array();
            for(int i=0; i<applies.Length(); ++i)
            {
                obj = applies[i].Object();
                selector.push_back(obj["selector"].String());
                index.push_back(obj["index"].String());
            }
            std::string temp = "color temp";
            std::string input = "input";
            std::string colorTemp;
            std::string inputSrc;
            int pos;
            auto found = find(selector.begin(), selector.end(), temp);
            auto found1 = find(selector.begin(), selector.end(), input);
            if(found1 == selector.end()) {
                LOGINFO("Input source not passed");
                inputSrc = "";
            } else {
                pos = found1 - selector.begin();
                inputSrc = index.at(pos);
            }
	    if(found != selector.end())
            {
                pos = found - selector.begin();
                colorTemp = index.at(pos);
                LOGINFO("input source : %s\ncolortemp : %s\ncolor : %s\nctrl : %s\nvalue : %d\n", inputSrc.c_str(), colorTemp.c_str(), color.c_str(), ctrl.c_str(), value);
                tvError_t ret = setWBctrl (const_cast <char *> (inputSrc.c_str()),const_cast <char *> (colorTemp.c_str()), const_cast <char *> (color.c_str()), const_cast <char *> (ctrl.c_str()), value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("setWBCtrl failed");
                    returnResponse(false, getErrorString(ret).c_str());
                } else {
                    //set it to local cache
                    std::string identifier = TVSETTINGS_GENERIC_STRING_RFC_PARAM;
                    identifier+=std::string("wb.")+color+"."+ctrl;
                    tr181ErrorCode_t err = setLocalParam(rfc_caller_id, identifier.c_str(), val.c_str());
                    if ( err != tr181Success ) {
                        LOGWARN("setLocalParam for %s Failed : %s\n", identifier.c_str(), getTR181ErrorString(err));
                    }
                    else {
                        LOGINFO("setLocalParam for %s Successful, Value: %d\n", identifier.c_str(), value);
                    }
                    LOGINFO("Exit : setWBCtrl success");
                    returnResponse(true, "success");
                }
            } else {
                returnResponse(false, "color temp or input not available");
            }
        }
        else{
            returnResponse(false,"not valid parameter");
        }
    }

    uint32_t ControlSettingsTV::resetWBCtrl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tvError_t ret = tvERROR_NONE;
        string identifier=(std::string(TVSETTINGS_GENERIC_STRING_RFC_PARAM)+std::string("wb."));

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,identifier.c_str());
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n",identifier.c_str(),getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }else{
            tvDataColor_t WBValues={0};
            //get the defaults
            ret = GetCustomWBValuesFromLocalCache(WBValues);
            if(ret == tvERROR_NONE)
            {
                //set the defaults and save
                ret = SyncCustomWBValuesToDriverCache(WBValues,true);
            }
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : Successfully reset user WB Entries \n");
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
	TR181_ParamData_t param_old={0};
       
        formatIndex=ConvertHDRFormatToContentFormat((tvhdr_type_t)formatIndex); 
        generateStorageIdentifier(key,forParam,formatIndex,pqIndex,sourceIndex);
        if(key.empty())
        {
            LOGWARN("generateStorageIdentifier failed\n");
            return -1;
        }

       tr181ErrorCode_t err=getLocalParam(rfc_caller_id, key.c_str(), &param);
       LOGINFO("%s: key %s\n",__FUNCTION__,key.c_str());

       //storage Migration Code
       tr181ErrorCode_t err_old = tr181Failure;
       if( tr181Success != err )
       {
           if (strncmp(forParam,"DimmingMode",strlen(forParam))==0)
           {
               if(!GetLDIMParamsToSync(value,pqIndex))
               {
                  LOGINFO("Found Dimmingmode Dirty DimmingMode=%d\n",value);
                  return 0;
	       }
           }
           else
           {
               key.clear();
               generateStorageIdentifierDirty(key,forParam,formatIndex,pqIndex);
               err_old = getLocalParam(rfc_caller_id, key.c_str(), &param_old);
           }//storage migration ends
       }

       if ( tr181Success == err )//Fetch new tr181format values
       {
           LOGINFO("Case 1\n");
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
        else if( tr181Success == err_old )//Fetch old tr181format values
        {
            LOGINFO("Case 2 key %s\n found",key.c_str());
            if(strncmp(forParam,"ColorTemp",strlen(forParam))==0)
            {
                if (strncmp(param.value, "Standard", strlen(param_old.value))==0)
                    value=tvColorTemp_STANDARD;
                else if (strncmp(param.value, "Warm", strlen(param_old.value))==0)
                    value=tvColorTemp_WARM;
                else if (strncmp(param.value, "Cold", strlen(param_old.value))==0)
                    value=tvColorTemp_COLD;
                else if (strncmp(param.value, "User Defined", strlen(param_old.value))==0)
                    value=tvColorTemp_USER;
                else
                    value=tvColorTemp_STANDARD;

		return 0;
            }
            else
            {
                value=std::stoi(param_old.value);
                return 0;
            }
        }
        else//Fall back to default tvsettings.ini
        {
            if(cms)
            {
                value=GetCMSDefault((tvCMS_tunel_t)tunnel_type);
                return 1;/*block default cms sync and allow default values during reset*/
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

    uint32_t ControlSettingsTV::generateStorageIdentifierDirty(std::string &key,const char * forParam,uint32_t contentFormat, int pqmode)
    {
    key+=std::string(TVSETTINGS_GENERIC_STRING_RFC_PARAM);
    key+=STRING_PICMODE+std::to_string(pqmode)+std::string(".")+std::string(STRING_FORMAT)+std::to_string(contentFormat);
    CREATE_DIRTY(key)+=forParam;

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
            char *formatString = strdup(format);
            char *token = NULL;
            int count=1;
            while (token = strtok_r(formatString," ",&formatString))
            {
                formats.push_back(ConvertFormatStringToHDRFormat(token));//This function needs to be implemented
                LOGINFO("%s : Format[%d] : %s\n",__FUNCTION__,count,token);
                count++;
            }
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
            {
                err = clearLocalParam(rfc_caller_id, key.c_str());
                //Remove old format too
		key.clear();
                generateStorageIdentifierDirty(key,forParam,format,pqmode);
		tr181ErrorCode_t err_old = clearLocalParam(rfc_caller_id, key.c_str());
            }

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
                            case PQ_PARAM_BACKLIGHT:
                                ret |= SaveBacklight(source, mode,format,params[0]);
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
                             case PQ_PARAM_DOLBY_MODE:
                                 if(sync)
                                 {
                                     int value=0;
                                     if( !GetDolbyParamToSync(value) )
                                         LOGINFO("Found param from tr181 dvmode pqmode : %d format:%d value:%d\n",mode,format,value);
                                     else
                                         LOGINFO("value not found in tr181 dvmode pqmode : %d format:%d value:%d\n",mode,format,value);

                                     params[0]=value;
                                 }
                                 ret |= SaveDolbyMode(source, mode,format,params[0]);
                                 break;
                           
                              case PQ_PARAM_HDR10_MODE:
                                 if(sync){
                                      int value=0;
                                      if( !GetHDR10ParamToSync(value) )
                                          LOGINFO("Found param from tr181 hdr10mode pqmode : %d format:%d value:%d\n",mode,format,value);
                                      else
                                          LOGINFO("value not found in tr181 hdr10mode pqmode : %d format:%d value:%d\n",mode,format,value);

                                      params[0]=value;
                                 }
                                 ret |= SaveDolbyMode(source, mode,format,params[0]);
                                 break;
   
                            case PQ_PARAM_HLG_MODE:
                                if(sync){
                                    int value=0;
                                    if( !GetHLGParamToSync(value) )
                                        LOGINFO("Found param from tr181 hlgmode pqmode : %d format:%d value:%d\n",mode,format,value);
                                    else
                                        LOGINFO("value not found in tr181 hlgmode pqmode : %d format:%d value:%d\n",mode,format,value);

                                    params[0]=value;
                                }
                                ret |= SaveDolbyMode(source, mode,format,params[0]);
                                break;
                             case PQ_PARAM_ASPECT_RATIO:
                                 ret |= SaveDisplayMode(source,mode,format,params[0]);
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

    int ControlSettingsTV::GetDolbyParamToSync(int& value)
    {
        int ret=0;
        TR181_ParamData_t param;

        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            value=GetDolbyModeIndex(param.value);
        }
        else
        {
            LOGWARN("Unable to fetch %s from localstore\n",TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM);
            ret=-1;
        }
        return ret;
    }

    int ControlSettingsTV::GetHDR10ParamToSync(int& value)
    {
        int ret=0;
        TR181_ParamData_t param;

        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HDR10MODE_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            value=GetHDR10ModeIndex(param.value);
        }
        else
        {
            LOGWARN("Unable to fetch %s from localstore\n",TVSETTINGS_HDR10MODE_RFC_PARAM);
            ret=-1;
        }
        return ret;
    }

    int ControlSettingsTV::GetHLGParamToSync(int& value)
    {
        int ret=0;
        TR181_ParamData_t param;

        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HLGMODE_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            value=GetHLGModeIndex(param.value);
        }
        else
        {
            LOGWARN("Unable to fetch %s from localstore\n",TVSETTINGS_HLGMODE_RFC_PARAM);
            ret=-1;
        }
        return ret;
    }

    tvError_t ControlSettingsTV::SyncCustomWBValuesToDriverCache(tvDataColor_t WBValues,bool setDuringSync)
    {
        tvError_t ret = tvERROR_NONE;
        std::vector<std::string> allCtrls{ "gain", "offset"};
        std::vector<std::string> allColors{ "red", "green","blue"};

        for(std::string color : allColors)
        {
            for(std::string ctrl : allCtrls)
            {
                int rgbType;
                int value = GetGainOffsetValue(color.c_str(),ctrl.c_str(),WBValues,rgbType);

                if(setDuringSync)
                    ret  = SetColorTemperatureUser(rgbType,value);

                if(tvERROR_NONE == ret)
                    ret  = SaveColorTemperatureUser(rgbType,value);

                if(tvERROR_NONE!= ret)
                {
                    LOGWARN("WB Entry for %s.%s fail to save to driver\n",color.c_str(),ctrl.c_str());
                }
                else
                    LOGINFO("WB Entry for %s.%s saved to driver\n",color.c_str(),ctrl.c_str());
            }
        }
        return ret;
    }

    tvError_t ControlSettingsTV::GetCustomWBValuesFromLocalCache(tvDataColor_t &WBValues)
    {
        TR181_ParamData_t param={0};
        tvError_t ret = tvERROR_NONE;
        std::vector<std::string> allCtrls{ "gain", "offset"};
        std::vector<std::string> allColors{ "red", "green","blue"};

        LOGINFO("Entry");
        for(std::string color : allColors)
        {
            for(std::string ctrl : allCtrls)
            {
                string identifier=std::string(TVSETTINGS_GENERIC_STRING_RFC_PARAM)+std::string("wb");
                int value =0;
                tr181ErrorCode_t err;
                identifier+=color+"."+ctrl;
                err = getLocalParam(rfc_caller_id, identifier.c_str(), &param);
                if ( tr181Success == err )
                {
                    value = std::stoi(param.value);
                    LOGINFO("%s  : %d\n",identifier.c_str(),value);
                }
				else
                {
                    LOGINFO("%s  Failed trying dirty\n",identifier.c_str());
                    identifier.clear();
                    identifier+=std::string(TVSETTINGS_GENERIC_STRING_RFC_PARAM)+std::string("wb.")+color+"."+ctrl;
                    err = getLocalParam(rfc_caller_id, identifier.c_str(), &param);
                    if ( tr181Success == err )
                    {
                        value = std::stoi(param.value);
                        LOGINFO("%s  : %d\n",identifier.c_str(),value);

                    }
                    else
                    {
                        LOGWARN("Not finding entry for %s  : %s\n",identifier.c_str(),getTR181ErrorString(err));
                        ret = tvERROR_GENERAL;
                    }
                }
                
		        if(tr181Success == err )
                {
                    switch(GetWBRgbType(color.c_str(),ctrl.c_str()))
                    {
                        case R_GAIN:
                            WBValues.r_gain = value;
                            break;
                        case G_GAIN:
                            WBValues.g_gain = value;
                            break;
                        case B_GAIN:
                            WBValues.b_gain = value;
                           break;
                        case R_POST_OFFSET:
                            WBValues.r_offset = value;
                            break;
                        case G_POST_OFFSET:
                            WBValues.g_offset = value;
                            break;
                        case B_POST_OFFSET:
                            WBValues.b_offset = value;
                            break;
                    }
                }
            }
        }
        LOGINFO("r.gain %d,r.offset %d,g.gain %d,g.offset %d,b.gain %d,b.offset %d",WBValues.r_gain,WBValues.r_offset,
        WBValues.g_gain,WBValues.g_offset,WBValues.b_gain,WBValues.b_offset);
        LOGINFO("Exit");
        return ret;
    }

    int ControlSettingsTV::GetGainOffsetValue(const char* color,const char*ctrl,tvDataColor_t WBValues,int &rgbType)
    {
        int value = 0;
        rgbType = GetWBRgbType(color,ctrl);
        switch(rgbType)
        {
            case R_GAIN:
                value = WBValues.r_gain;
                break;
            case G_GAIN:
                value = WBValues.g_gain;
                break;
            case B_GAIN:
                value = WBValues.b_gain;
                break;
            case R_POST_OFFSET:
                value = WBValues.r_offset;
                break;
            case G_POST_OFFSET:
                value = WBValues.g_offset;
                break;
            case B_POST_OFFSET:
                value = WBValues.b_offset;
                break;
        }
        return value;
    }

    bool ControlSettingsTV::isSetRequired(std::string pqmode,std::string source,std::string format)
    {
        bool ret=false;
        char picMode[PIC_MODE_NAME_MAX]={0};
        std::string currentPicMode;
        std::string currentSource;
        std::string currentFormat;
        std::string space_delimiter = " ";
	
        //GetCurrent pqmode
        if(!getCurrentPictureMode(picMode))
            LOGWARN("Failed to get the current picture mode\n");
		
        currentPicMode = picMode; //Convert to string

        //GetCurrentSource
        currentSource = ConvertSourceIndexToString( GetCurrentContentFormat());
	
        //GetCurrentFormat
        currentFormat = ConvertVideoFormatToString( GetCurrentContentFormat());
	
        LOGINFO("%s : currentSource = %s,currentPicMode = %s,currentFormat = %s\n",__FUNCTION__,currentSource.c_str(),currentPicMode.c_str(),currentFormat.c_str());
	LOGINFO("%s : source = %s,PicMode = %s, format= %s\n",__FUNCTION__,source.c_str(),pqmode.c_str(),format.c_str());

        if( ( pqmode.compare("current") == 0 || (pqmode.find(currentPicMode) != std::string::npos) || pqmode.compare("all") == 0) &&
            (source.compare("current") == 0 || (source.find(currentSource) != std::string::npos)  || source.compare("all") == 0) &&
            (format.compare("current") == 0 || (format.find(currentFormat) !=  std::string::npos) || format.compare("all") == 0)  )	
            ret=true;
		
	return ret;
    }

    int ControlSettingsTV::SyncCMSParams(const char *pqParam,tvCMS_tunel_t tunnel_type)
    {
        int ret=0;
        char cms_param[BUFFER_SIZE]={0};
        int params[3]={0};

        for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
        {
            memset(&cms_param, 0, sizeof(cms_param));
            snprintf(cms_param,sizeof(cms_param),"%s.%s",pqParam,component_color[color]);
            params[0]=tunnel_type;//tunnel_type
            params[1]=color;//color_type
            params[2]=0;//value

            if(!UpdatePQParamsToCache("sync",cms_param,"all","all","all",PQ_PARAM_CMS,params))
                ret |= 0;
            else
                ret |= 1;
        }
        return ret;
    }

    tvError_t ControlSettingsTV::SyncCMSParamsToDriverCache()
    {
        int cms_enable[3]={0};
        cms_enable[0]=COLOR_STATE;//tunel_type
        cms_enable[1]=COLOR_ENABLE;//color_type
        cms_enable[2]=0;//value

        if(! UpdatePQParamsToCache("sync","cms.enable","all","all","all",PQ_PARAM_CMS,cms_enable))
            LOGINFO("CMS Enable Flag  Successfully Synced to Drive Cache\n");
        else
            LOGWARN("CMS Enable Flag Sync to cache Failed !!!\n");

        if( !SyncCMSParams("saturation",COLOR_SATURATION))
            LOGINFO("Component saturation Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Component saturation Sync to cache Failed !!!\n");

        if( !SyncCMSParams("hue",COLOR_HUE))
            LOGINFO("Component hue Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Component hue to cache Failed !!!\n");

        if( !SyncCMSParams("luma",COLOR_LUMA))
            LOGINFO("Component Luma Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Component Luma Sync to cache Failed !!!\n");

        return tvERROR_NONE;
    }

    void ControlSettingsTV::SyncWBparams(void)
    {
        tvDataColor_t WbValuesFromCache={0};
        tvDataColor_t WbValueFromDrv={0};
        tvDataColor_t WbValueAllZero={0};
        bool WbvalueFromDrvIsDefault = false;
        bool WbvalueInLocalCacheIsDefault = false;

        GetCustomWBValuesFromLocalCache(WbValuesFromCache);
        LOGINFO("cached:r.gain %d,r.offset %d,g.gain %d,g.offset %d,b.gain %d,b.offset %d",WbValuesFromCache.r_gain,WbValuesFromCache.r_offset,
        WbValuesFromCache.g_gain,WbValuesFromCache.g_offset,WbValuesFromCache.b_gain,WbValuesFromCache.b_offset);
        WbValueFromDrv = GetUSerWBValueOnInit();
        LOGINFO("cached:r.gain %d,r.offset %d,g.gain %d,g.offset %d,b.gain %d,b.offset %d",WbValueFromDrv.r_gain,WbValueFromDrv.r_offset,
        WbValueFromDrv.g_gain,WbValueFromDrv.g_offset,WbValueFromDrv.b_gain,WbValueFromDrv.b_offset);
        WbvalueFromDrvIsDefault = isWBUserDfault(WbValueFromDrv);
        WbvalueInLocalCacheIsDefault = isWBUserDfault(WbValuesFromCache);
        LOGINFO("Drv WB deafult:%s , tr181 WB default:%s\n",WbvalueFromDrvIsDefault?"Yes":"No",
            WbvalueInLocalCacheIsDefault?"Yes":"No");
        if(!WbvalueInLocalCacheIsDefault &&
            !areEqual(WbValuesFromCache,WbValueFromDrv) &&
            !areEqual(WbValuesFromCache,WbValueAllZero) )
        {
            //RDK->Driver
            LOGINFO("RDK->Driver\n");
            SyncCustomWBValuesToDriverCache(WbValuesFromCache,false);

        }
        else
        {
            LOGINFO("No need to sync WB params\n");
        }

    }

    int ControlSettingsTV::InitializeSDRHDRBacklight()
    {
        char panelId[20] = {0};
        int val=GetPanelID(panelId);
        int ret=0;

        if (val != 0)
        {
            LOGERR("Failed to read panel id!!! Set 55 panel as default\n");
            memset(panelId,0,sizeof(panelId));
            GetDefaultPanelID(panelId);
            LOGINFO("Panel ID : %s \n",panelId);
        }
        else
            LOGINFO("Read panel id ok [%s] \n", panelId);

        /* Load Default Panel ID for Converter Boards*/
        if(strncmp(panelId,TVSETTINGS_CONVERTERBOARD_PANELID,strlen(TVSETTINGS_CONVERTERBOARD_PANELID))==0)
        {
            memset(panelId,0,sizeof(panelId));
            GetDefaultPanelID(panelId);
            LOGINFO("Load 55 panel values as default panel ID : %s\n",panelId);
        }

        val=ReadBacklightFromTable(panelId);
        if(val == 0)
            LOGINFO("Backlight read success from backlight_default.ini\n");
        else
            LOGWARN("Backlight read failed from backlight_default.ini\n");

        ret=SetBacklightAtInitAndSaveForBLUsingGBF();
        if(ret == tvERROR_NONE)
        {
            LOGINFO("SetBacklightAtInitAndSaveForBLUsingGBF: success\n");
        }
        else
        {
            ret=-1;
            LOGWARN("SetBacklightFromLocalStore(): Failed\n");
        }
        return ret;
    }

    tvError_t ControlSettingsTV::SetBacklightAtInitAndSaveForBLUsingGBF(void)
    {
        tvError_t ret = tvERROR_NONE;
        TR181_ParamData_t param={0};
        int backlightSDR=0;
        int backlightHDR=0;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            backlightSDR=GetDriverEquivalentBLForCurrentFmt(std::stoi(param.value));
        }
        else
        {
            LOGERR("Default BL for SDR can't be read \n");
            ret = tvERROR_GENERAL;
        }

        if(ret == tvERROR_NONE)
        {
            memset(&param, 0, sizeof(param));
            err = getLocalParam(rfc_caller_id, TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM, &param);
            if ( tr181Success == err )
            {
                backlightHDR=GetDriverEquivalentBLForCurrentFmt(std::stoi(param.value));
            }
            else
            {
                LOGERR("Default BL for HDR can't be read \n");
                ret = tvERROR_GENERAL;
            }
        }
        if(ret == tvERROR_NONE)
        {
            int backlight = isCurrentHDRTypeIsSDR()?backlightSDR:backlightHDR;
            ret = SetBacklight(backlight);
            if(ret != tvERROR_NONE) {
                LOGERR("Failed to set backlight at init \n");
            }
            else
            {
                ret = SaveSDRHDRBacklightAtInitToDrv(backlightSDR,backlightHDR);
            }
        }
        return ret;
    }

    int ControlSettingsTV::ReadBacklightFromTable(char *panelId)
    {
        tvBacklightInfo_t  blInfo = {0};

        std::string temp_panelid=std::string(panelId);
        temp_panelid=std::string(temp_panelid.rbegin(),temp_panelid.rend());//reverse
        std::string delimiter="_";
        temp_panelid=temp_panelid.erase(0, temp_panelid.find(delimiter) + delimiter.length());//remove the first _ token
        temp_panelid=std::string(temp_panelid.rbegin(),temp_panelid.rend());  //reverse again

        LOGINFO("%s: Looking at %s / %s for BLT \n",__FUNCTION__,panelId,temp_panelid.c_str());

        try
        {
            CIniFile inFile(BACKLIGHT_FILE_NAME);

            for(int i = 0 ; i < BACKLIGHT_CURVE_MAX_INDEX; i++ )
            {
                std::string s;
                s = temp_panelid+ "_" + std::string("SDR") + ".bl_" + std::to_string(i);
                blInfo.sdrBLCurve[i] = inFile.Get<int>(s);
                LOGINFO("bl_table_sdr[%d] = %u\n", i, blInfo.sdrBLCurve[i] );
            }

            for(int j = 0 ; j < BACKLIGHT_CURVE_MAX_INDEX; j++ )
            {
                std::string s;
                s = temp_panelid+ "_" + std::string("HDR") + ".bl_" + std::to_string(j);
                blInfo.hdrBLCurve[j] = inFile.Get<int>(s);
                LOGINFO("bl_table_hdr[%d] = %u\n", j, blInfo.hdrBLCurve[j] );
            }
        }
		catch(const boost::property_tree::ptree_error &e)
        {
            LOGERR("%s: error %s::config table entry not found in ini file\n",__FUNCTION__,e.what());
            return -1;
        }

        {
            blInfo.defaultBLSDR = defaultSDR;
            blInfo.defaultBLHDR = defaultHDR;
            SetBacklightInfo(blInfo);
        }
        return 0;
    }

    tvError_t ControlSettingsTV::SaveSDRHDRBacklightAtInitToDrv(int backlightSDR,int backlightHDR)
    {
        tvError_t ret = tvERROR_NONE;

        for(int lCount=0;lCount<numberModesSupported;lCount++)
        {
            if(tvERROR_NONE != SaveBacklight(SAVE_FOR_ALL_SOURCES, pic_mode_index[lCount],(int)HDR_TYPE_SDR,backlightSDR)||
                tvERROR_NONE != SaveBacklight(SAVE_FOR_ALL_SOURCES, pic_mode_index[lCount],(int)HDR_TYPE_HDR10,backlightHDR) ||
                tvERROR_NONE != SaveBacklight(SAVE_FOR_ALL_SOURCES, pic_mode_index[lCount],(int)HDR_TYPE_HLG,backlightHDR) ||
                tvERROR_NONE != SaveBacklight(SAVE_FOR_ALL_SOURCES, pic_mode_index[lCount],(int)HDR_TYPE_DOVI,backlightHDR))
            {
                ret = tvERROR_GENERAL;
                LOGWARN("BL update failed for picmode %d\n",pic_mode_index[lCount]);
                break;
            }
        }
        return ret;
    }

    int ControlSettingsTV::GetLDIMParamsToSync(int &value,int mode)
    {
        int ret=0;
        TR181_ParamData_t param;
        char format[BUFFER_SIZE]={0};

        memset(&param, 0, sizeof(param));
        snprintf(format,sizeof(format),"%s%d.%s",TVSETTINGS_GENERIC_STRING_RFC_PARAM,mode,"DimmingMode");
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, format,&param);
        if ( tr181Success == err )
        {
            value=GetDimmingModeIndex(param.value);
        }
        else
        {
            LOGWARN("Unable to fetch %s from localstore\n",format);
            ret=-1;
        }
        return ret;
    }

    tvError_t ControlSettingsTV::SyncPQParamsToDriverCache()
    {
        int params[3]={0};

        if( !UpdatePQParamsToCache("sync","Brightness","all","all","all",PQ_PARAM_BRIGHTNESS,params))
            LOGINFO("Brightness Successfully sync to Drive Cache\n");
        else
            LOGWARN("Brightness Sync to cache Failed !!!\n");

        if( !UpdatePQParamsToCache("sync","Contrast","all","all","all",PQ_PARAM_CONTRAST,params))
            LOGINFO("Contrast Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Contrast Sync to cache Failed !!!\n");

        if( !UpdatePQParamsToCache("sync","Sharpness","all","all","all",PQ_PARAM_SHARPNESS,params))
            LOGINFO("Sharpness Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Sharpness Sync to cache Failed !!!\n");

        if( !UpdatePQParamsToCache("sync","Saturation","all","all","all",PQ_PARAM_SATURATION,params))
            LOGINFO("Saturation Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Saturation Sync to cache Failed !!!\n");

        if( !UpdatePQParamsToCache("sync","Hue","all","all","all",PQ_PARAM_HUE,params))
            LOGINFO("Hue Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Hue Sync to cache Failed !!!\n");

        if( !UpdatePQParamsToCache("sync","ColorTemp","all","all","all",PQ_PARAM_COLOR_TEMPERATURE,params))
            LOGINFO("ColorTemp Successfully Synced to Drive Cache\n");
        else
            LOGWARN("ColorTemp Sync to cache Failed !!!\n");

	if( !UpdatePQParamsToCache("sync","DolbyVisionMode","all","all","all",PQ_PARAM_DOLBY_MODE,params))
            LOGINFO("dvmode Successfully Synced to Drive Cache\n");
        else
            LOGWARN("dvmode Sync to cache Failed !!!\n");

        if( !UpdatePQParamsToCache("sync","DimmingMode","all","all","all",PQ_PARAM_LDIM,params))
            LOGINFO("dimmingmode Successfully Synced to Drive Cache\n");
        else
            LOGWARN("dimmingmode Sync to cache Failed !!!\n");

        if(appUsesGlobalBackLightFactor){
            if( !UpdatePQParamsToCache("sync","HLGMode","all","all","all",PQ_PARAM_HLG_MODE,params))
                LOGINFO("hlgmode Successfully Synced to Drive Cache\n");
            else
                LOGWARN("hlgmode Sync to cache Failed !!!\n");

            if( !UpdatePQParamsToCache("sync","HDR10Mode","all","all","all",PQ_PARAM_HDR10_MODE,params))
                LOGINFO("hdr10mode Successfully Synced to Drive Cache\n");
            else
                LOGWARN("hdr10mode Sync to cache Failed !!!\n");
        }

        if( !UpdatePQParamsToCache("sync","Backlight","all","all","all",PQ_PARAM_BACKLIGHT,params) )
            LOGINFO("Backlight Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Backlight Sync to cache Failed !!!\n");


        if(appUsesGlobalBackLightFactor)
        {
            SyncCMSParamsToDriverCache();

            SyncWBparams();
        }
        return tvERROR_NONE;
    }



}//namespace Plugin
}//namespace WPEFramework
