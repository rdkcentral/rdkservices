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

#define BUFFER_SIZE     (128)

#define registerMethod(...) Register(__VA_ARGS__);GetHandler(2)->Register<JsonObject, JsonObject>(__VA_ARGS__)


static const char *component_color[] = {
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

    ControlSettingsTV::ControlSettingsTV(): PluginHost::JSONRPC(),rfc_caller_id()
    						
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

	registerMethod("getPictureMode", &ControlSettingsTV::getPictureMode, this);
        registerMethod("setPictureMode", &ControlSettingsTV::setPictureMode, this);
	registerMethod("resetPictureMode", &ControlSettingsTV::resetPictureMode, this);
        registerMethod("getSupportedPictureModes", &ControlSettingsTV::getSupportedPictureModes, this);
	registerMethod("enableWBMode", &ControlSettingsTV::enableWBMode, this);

	registerMethod("setBacklightFade", &ControlSettingsTV::setBacklightFade, this);

	tvError_t ret = tvERROR_NONE;
        
	std::system("echo \"Testing dmesg [starts - before tvInit] - ControlSettingsTV::Initialize()\" > /dev/kmsg");
        ret = tvInit();
        std::system("echo \"Testing dmesg [starts - after tvInit] - ControlSettingsTV::Initialize()\" > /dev/kmsg");

        if(ret != tvERROR_NONE) {
            LOGERR("Platform Init failed, ret: %s \n", getErrorString(ret).c_str());

        }
        else{
            LOGINFO("Platform Init successful...\n");
            std::system("echo \"Testing dmesg [starts..before tvSD3toCriSyncInit] - ControlSettingsTV::Initialize()\" > /dev/kmsg");
            ret = tvSD3toCriSyncInit();
            std::system("echo \"Testing dmesg [starts..after tvSD3toCriSyncInit] - ControlSettingsTV::Initialize()\" > /dev/kmsg");
            if(ret != tvERROR_NONE) {
                LOGERR(" SD3 <->cri_data sync failed, ret: %s \n", getErrorString(ret).c_str());
            }
            else {
                LOGERR(" SD3 <->cri_data sync success, ret: %s \n", getErrorString(ret).c_str());
            }

        }
	std::system("echo \"Testing dmesg [starts..before SyncPQParamsToDriverCache] - ControlSettingsTV::Initialize()\" > /dev/kmsg");

        SyncPQParamsToDriverCache();

        std::system("echo \"Testing dmesg [starts..after SyncPQParamsToDriverCache] - ControlSettingsTV::Initialize()\" > /dev/kmsg");

        LOGINFO("Exit\n");
    }
    
    ControlSettingsTV :: ~ControlSettingsTV()
    {
        LOGINFO();    
    }

    void ControlSettingsTV::Initialize()
    {
       std::system("echo \"Testing ControlSettingsTV dmesg [starts] - ControlSettingsTV::Initialize()\" > /dev/kmsg");
       LOGINFO("Entry\n");
       
       tvError_t ret = tvERROR_NONE;

       TR181_ParamData_t param;
       memset(&param, 0, sizeof(param));

       LocatePQSettingsFile(rfc_caller_id);

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
        setDefaultAspectRatio();
	std::system("echo \"Testing dmesg [ends] - ControlSettingsTV::Initialize()\" > /dev/kmsg");

       LOGINFO("Exit\n" );
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
			
        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        err = GetLocalparam("Backlight",formatIndex,pqIndex,sourceIndex,backlight);
        
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

        GetParamIndex(source,pqmode,format,sourceIndex,pqIndex,formatIndex);
        err = GetLocalparam("Backlight",formatIndex,pqIndex,sourceIndex,backlight);
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
        int userScaleBacklight = 0;
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

        ret = SetBacklight(backlight,pqmode.c_str(),format.c_str(),source.c_str(),true);
        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set backlight\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
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

       // if(appUsesGlobalBackLightFactor){
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
//      }
        /* non backlight factor path */
	int retval= UpdatePQParamsToCache("reset","Backlight",pqmode.c_str(),source.c_str(),format.c_str(),PQ_PARAM_BACKLIGHT,params);
        if(retval != 0 ) {
            LOGWARN("Failed to reset Backlight\n");
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            GetParamIndex("current","current","current",sourceIndex,pqIndex,formatIndex);
            int err = GetLocalparam("Backlight",formatIndex,pqIndex,sourceIndex,backlight);
            if( err == 0 ) {
                LOGINFO("%s : GetLocalparam success format :%d source : %d format : %d value : %d\n",__FUNCTION__,formatIndex, sourceIndex, pqIndex,backlight);
                ret = SetBacklight(backlight,"current","current","current",false);
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
	tvError_t ret = tvERROR_NONE;

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

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetContrast(contrast);
        }
        else
            LOGINFO("Set not required for this request!!! Just Save it\n");

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
        tvError_t ret = tvERROR_NONE;

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

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetSaturation(saturation);
        }
        else
            LOGINFO("Set not required for this request!!! Just Save it\n");


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
        tvError_t ret = tvERROR_NONE;

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
        
        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetSharpness(sharpness);
        }
        else
            LOGINFO("Set not required for this request!!! Just Save it\n");

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
        tvError_t ret = tvERROR_NONE;

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

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetHue(hue);
        }
        else
            LOGINFO("Set not required for this request!!! Just Save it\n");


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
        tvError_t ret = tvERROR_NONE;

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

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetColorTemperature((tvColorTemp_t)colortemp);
        }
        else
            LOGINFO("Set not required for this request!!! Just Save it\n");

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
	if( err == 0 || err == 1 ) {//err value willbe 1 if cms is default.(SPECIAL case)
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
        if( err == 0 || err == 1 ) {//err value willbe 1 if cms is default.(SPECIAL case)
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
        if( err == 0 || err == 1 ) {//err value willbe 1 if cms is default.(SPECIAL case)
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
	tvError_t ret = tvERROR_NONE;

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

        if( isSetRequired(pqmode,source,format) ) {
             LOGINFO("Proceed with %s\n",__FUNCTION__);
             ret = SetTVDimmingMode(value.c_str());
        }
        else
            LOGINFO("Set not required for this request!!! Just Save it\n");


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

    uint32_t ControlSettingsTV::getSupportedPictureModes(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        pic_modes_t *pictureModes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedPictureModes(&pictureModes,&totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            JsonArray SupportedPicModes;

            for(int count = 0;count <totalAvailable;count++ )
            {
                SupportedPicModes.Add(pictureModes[count].name);
                // printf("Added Mode %s %s \n",pictureModes[count].name,SupportedPicModes[count].String().c_str());
            }

            response["SupportedPicmodes"] = SupportedPicModes;
            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::getPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        char mode[PIC_MODE_NAME_MAX]={0};
        std::string picturemode;
        tvError_t ret = GetTVPictureMode(mode);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            std::string s;
            s+=mode;
            response["pictureMode"] = s;
	    LOGINFO("Exit : getPictureMode() : %s\n",s.c_str());
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::setPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string value;

        value = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        returnIfParamNotFound(value);

        tvError_t ret = SetTVPictureMode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, value.c_str());
            }
            LOGINFO("Exit : Value : %s \n",value.c_str());
            returnResponse(true, "success");
        }
    }

    uint32_t ControlSettingsTV::resetPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        tr181ErrorCode_t err = tr181Success;

        err = clearLocalParam(rfc_caller_id, TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, getTR181ErrorString(err));
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            TR181_ParamData_t param = {0};
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM);
            err = getLocalParam(rfc_caller_id, TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, &param);
            if ( tr181Success == err )
            {
                LOGINFO("getLocalParam for %s is %s\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, param.value);
                tvError_t ret = SetTVPictureMode(param.value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("Picture Mode set failed: %s\n",getErrorString(ret).c_str());
                    returnResponse(false, getErrorString(ret).c_str());
                }
                else {
                    LOGINFO("Exit : Picture Mode reset successfully, value: %s\n", param.value);
                    returnResponse(true, "success");
                }
            }
            else {
                LOGWARN("getLocalParam for %s failed\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM);
                returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
            }
        }
    }

    uint32_t ControlSettingsTV::enableWBMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();
        std::string value;
        tvError_t ret = tvERROR_NONE;
        bool mode = 0;
        if(parameters.HasLabel("mode")) {
            value = parameters["mode"].String();
            if(value == "true") {
                mode = 1;
            } else if(value == "false") {
                mode = 0;
            }
            ret = enableWBmode(mode);
            if(ret != tvERROR_NONE) {
                LOGERR("enableWBmode failed\n");
                returnResponse(false, getErrorString(ret).c_str());
            }
            else{
                LOGINFO("enableWBmode successful... \n");
                returnResponse(true, "Success");
            }

        } else {
            returnResponse(false, "Parameter missing");
        }
    }

    uint32_t ControlSettingsTV::setBacklightFade(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        std::string from,to,duration;
        int fromValue = 0,toValue = 0,durationValue = 0;

        from = parameters.HasLabel("from") ? parameters["from"].String() : "";
        if(from.empty())
            fromValue = 100;
        else
            fromValue = std::stoi(from);

        to = parameters.HasLabel("to") ? parameters["to"].String() : "";
        if(to.empty())
            toValue = 0;
        else
            toValue = std::stoi(to);

        duration = parameters.HasLabel("duration") ? parameters["duration"].String() : "";
        if(duration.empty())
            durationValue = 0;
        else
            durationValue = std::stoi(duration);

        LOGINFO("from = %d to = %d d = %d\n" ,fromValue,toValue,durationValue);
        tvError_t ret = SetBacklightFade(fromValue,toValue,durationValue);

        if(ret != tvERROR_NONE) {
           LOGWARN("Failed to set BacklightFade \n");
           returnResponse(false, getErrorString(ret).c_str());
        }
        else {
           LOGINFO("Exit : backlightFade Success \n");
           returnResponse(true, "success");
        }
    }

}//namespace Plugin
}//namespace WPEFramework
