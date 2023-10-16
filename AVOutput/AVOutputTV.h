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

#ifndef AVOutputTV_H
#define AVOutputTV_H

#include "string.h"
#include <set>

#include "tvTypes.h"
#include "tvLog.h"
#include "tvSettings.h"
#include <pthread.h>
#include "Module.h"
#include "tvError.h"
#include "tvTypes.h"
#include "tr181api.h"
#include "AVOutputCommon.h"
#include "libIARM.h"
#include "libIBusDaemon.h"
#include "libIBus.h"
#include "iarmUtil.h"
#include "UtilsLogging.h"
#include "UtilsJsonRpc.h"
#include "dsError.h"
#include "dsMgr.h"
#include "hdmiIn.hpp"
#include "als_bl_iniparser.h"
#include "bl_table.h"
#include <numeric>

//Macro
#define RFC_BUFF_MAX 100
#define BACKLIGHT_RAW_VALUE_MAX    (255)
#define AVOUTPUT_RFC_CALLERID        "AVOutput"
#define AVOUTPUT_RFC_CALLERID_OVERRIDE        "../../opt/panel/tvsettings"
#define AVOUTPUT_OVERRIDE_PATH       "/opt/panel/tvsettings.ini"
#define AVOUTPUT_CONVERTERBOARD_PANELID     "0_0_00"
#define AVOUTPUT_GENERIC_STRING_RFC_PARAM    "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput."
#define AVOUTPUT_BACKLIGHT_SDR_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.SDR.Backlight"
#define AVOUTPUT_BACKLIGHT_HDR_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.HDR.Backlight"
#define AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM  "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.AutoBacklightMode"
#define AVOUTPUT_DOLBYVISIONMODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.DolbyVisionMode"
#define AVOUTPUT_HLGMODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.HLGMode"
#define AVOUTPUT_HDR10MODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.HDR10Mode"
#define AVOUTPUT_DIMMING_MODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.DimmingMode"
#define AVOUTPUT_PICTUREMODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.PictureMode"
#define AVOUTPUT_PICTUREMODE_STRING_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.PictureModeString"
#define AVOUTPUT_ASPECTRATIO_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.AspectRatio"
#define AVOUTPUT_BACKLIGHT_CONTROL_USE_GBF_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.UseGBFForBacklightControl"
#define AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.Source"

#define STRING_DIRTY  ".Dirty."
#define STRING_PICMODE  "PicMode."
#define STRING_FORMAT  "Format."
#define STRING_DEFAULT  "Default"
#define STRING_SOURCE    "Source."
#define CREATE_DIRTY(__X__) (__X__+=STRING_DIRTY)

namespace WPEFramework {
namespace Plugin {

class AVOutputTV : public PluginHost::IPlugin, public PluginHost::JSONRPC {
    private:
        AVOutputTV(const AVOutputTV&) = delete;
        AVOutputTV& operator=(const AVOutputTV&) = delete;
    public:
	/*Get API's*/
	DECLARE_JSON_RPC_METHOD(getBacklight)
        DECLARE_JSON_RPC_METHOD(getBrightness )
        DECLARE_JSON_RPC_METHOD(getContrast )
        DECLARE_JSON_RPC_METHOD(getSharpness )
        DECLARE_JSON_RPC_METHOD(getSaturation )
        DECLARE_JSON_RPC_METHOD(getHue )
        DECLARE_JSON_RPC_METHOD(getColorTemperature )
        DECLARE_JSON_RPC_METHOD(getComponentHue )
        DECLARE_JSON_RPC_METHOD(getComponentLuma )
        DECLARE_JSON_RPC_METHOD(getComponentSaturation )
        DECLARE_JSON_RPC_METHOD(getBacklightDimmingMode )
        DECLARE_JSON_RPC_METHOD(getAutoBacklightControl )
        DECLARE_JSON_RPC_METHOD(getSupportedDolbyVisionModes )
        DECLARE_JSON_RPC_METHOD(getSupportedHLGModes )
        DECLARE_JSON_RPC_METHOD(getHLGMode )
        DECLARE_JSON_RPC_METHOD(getSupportedHDR10Modes )
        DECLARE_JSON_RPC_METHOD(getHDR10Mode )
        DECLARE_JSON_RPC_METHOD(getWBInfo )
        DECLARE_JSON_RPC_METHOD(getWBCtrl )
        DECLARE_JSON_RPC_METHOD(getSupportedPictureModes )
        DECLARE_JSON_RPC_METHOD(getSupportedVideoSources)
        DECLARE_JSON_RPC_METHOD(getSupportedVideoFormats)
        DECLARE_JSON_RPC_METHOD(getSupportedVideoFrameRates)
        DECLARE_JSON_RPC_METHOD(getSupportedVideoResolutions)
        DECLARE_JSON_RPC_METHOD(getPictureMode )
        DECLARE_JSON_RPC_METHOD(getVideoFormat)
        DECLARE_JSON_RPC_METHOD(getVideoSource)
        DECLARE_JSON_RPC_METHOD(getVideoFrameRate)
        DECLARE_JSON_RPC_METHOD(getVideoResolution)
	DECLARE_JSON_RPC_METHOD(getLowLatencyState)
        DECLARE_JSON_RPC_METHOD(getDolbyVisionMode)
        DECLARE_JSON_RPC_METHOD(getZoomMode)


	/*Get Capability API's*/
        DECLARE_JSON_RPC_METHOD(getBacklightCaps)
	DECLARE_JSON_RPC_METHOD(getContrastCaps)
	DECLARE_JSON_RPC_METHOD(getBrightnessCaps)
	DECLARE_JSON_RPC_METHOD(getSharpnessCaps)
	DECLARE_JSON_RPC_METHOD(getSaturationCaps)
	DECLARE_JSON_RPC_METHOD(getHueCaps)
	DECLARE_JSON_RPC_METHOD(getColorTemperatureCaps)
	DECLARE_JSON_RPC_METHOD(getComponentCaps )
	DECLARE_JSON_RPC_METHOD(getBacklightDimmingModeCaps )
	DECLARE_JSON_RPC_METHOD(getAutoBacklightControlCaps )
        DECLARE_JSON_RPC_METHOD(getDolbyVisionModeCaps )
        DECLARE_JSON_RPC_METHOD(getHDR10ModeCaps )
        DECLARE_JSON_RPC_METHOD(getHLGModeCaps )
        DECLARE_JSON_RPC_METHOD(getWBCaps )
        DECLARE_JSON_RPC_METHOD(getZoomModeCaps)
        DECLARE_JSON_RPC_METHOD(getLowLatencyStateCaps)
        DECLARE_JSON_RPC_METHOD(getPictureModeCaps)

	/*Set API's*/
        DECLARE_JSON_RPC_METHOD(setBacklight)
        DECLARE_JSON_RPC_METHOD(setBrightness)
        DECLARE_JSON_RPC_METHOD(setContrast )
        DECLARE_JSON_RPC_METHOD(setSharpness )
        DECLARE_JSON_RPC_METHOD(setSaturation )
        DECLARE_JSON_RPC_METHOD(setHue )
        DECLARE_JSON_RPC_METHOD(setColorTemperature )
        DECLARE_JSON_RPC_METHOD(setComponentSaturation )
        DECLARE_JSON_RPC_METHOD(setComponentHue )
        DECLARE_JSON_RPC_METHOD(setComponentLuma )
        DECLARE_JSON_RPC_METHOD(setBacklightDimmingMode )
        DECLARE_JSON_RPC_METHOD(setAutoBacklightControl )
        DECLARE_JSON_RPC_METHOD(setDolbyVisionMode )
        DECLARE_JSON_RPC_METHOD(setHLGMode )
        DECLARE_JSON_RPC_METHOD(setHDR10Mode )
        DECLARE_JSON_RPC_METHOD(setWBCtrl )
        DECLARE_JSON_RPC_METHOD(setPictureMode )
        DECLARE_JSON_RPC_METHOD(setBacklightFade )
        DECLARE_JSON_RPC_METHOD(setZoomMode)
        DECLARE_JSON_RPC_METHOD(setLowLatencyState)
        DECLARE_JSON_RPC_METHOD(enableWBMode )
	/*Reset API's*/
        DECLARE_JSON_RPC_METHOD(resetBacklight)
        DECLARE_JSON_RPC_METHOD(resetBrightness )
        DECLARE_JSON_RPC_METHOD(resetContrast )
        DECLARE_JSON_RPC_METHOD(resetSharpness )
        DECLARE_JSON_RPC_METHOD(resetSaturation )
        DECLARE_JSON_RPC_METHOD(resetHue )
        DECLARE_JSON_RPC_METHOD(resetColorTemperature )
        DECLARE_JSON_RPC_METHOD(resetComponentSaturation )
        DECLARE_JSON_RPC_METHOD(resetComponentHue )
        DECLARE_JSON_RPC_METHOD(resetComponentLuma )
        DECLARE_JSON_RPC_METHOD(resetBacklightDimmingMode )
        DECLARE_JSON_RPC_METHOD(resetAutoBacklightControl )
        DECLARE_JSON_RPC_METHOD(resetDolbyVisionMode )
        DECLARE_JSON_RPC_METHOD(resetHDR10Mode )
        DECLARE_JSON_RPC_METHOD(resetHLGMode )
        DECLARE_JSON_RPC_METHOD(resetWBCtrl )
        DECLARE_JSON_RPC_METHOD(resetPictureMode )
        DECLARE_JSON_RPC_METHOD(resetZoomMode)
        DECLARE_JSON_RPC_METHOD(resetLowLatencyState)

    private:
        std::string getErrorString (tvError_t eReturn);
	bool isBacklightUsingGlobalBacklightFactor(void);
	void LocatePQSettingsFile(void);
	int InitializeSDRHDRBacklight(void);
	tvContentFormatType_t getContentFormatIndex(tvVideoHDRFormat_t formatToConvert);
	void convertParamToLowerCase(std::string &source, std::string &pqmode, std::string &format);
        int convertToValidInputParameter(std::string pqparam, std::string & source, std::string & pqmode, std::string & format);
	tvError_t updatePQParamToLocalCache(std::string forParam, int source, int pqmode, int format, int value,bool setNotDelete);
        int updatePQParamsToCache( std::string action, std::string tr181ParamName, std::string pqmode, std::string source, std::string format, tvPQParameterIndex_t pqParamIndex, int params[] );
        void spliltCapablities( std::vector<std::string> &range,std::vector<std::string> &pqmode,std::vector<std::string> &format,std::vector<std::string> &source, std::string rangeInfo, std::string pqmodeInfo, std::string formatInfo, std::string sourceInfo );
	bool isCapablityCheckPassed( std::string pqmodeInputInfo,std::string sourceInputInfo,std::string formatInputInfo,std::string param );
        uint32_t generateStorageIdentifier(std::string &key, std::string forParam,int contentFormat, int pqmode, int source);
        uint32_t generateStorageIdentifierDirty(std::string &key, std::string forParam,uint32_t contentFormat, int pqmode);
	int getSaveConfig(std::string pqmode, std::string source, std::string format,std::vector<int> &sources,std::vector<int> &picturemodes, std::vector<int> &formats);
	int getLocalparam(std::string forParam,int formatIndex,int pqIndex,int sourceIndex,int &value,
		  tvPQParameterIndex_t pqParamIndex ,bool cms=false,int tunnel_type=0);
	tvError_t SyncPQParamsToDriverCache(std::string pqmode, std::string source, std::string format);
	int SyncSourceFormatPicModeToCache(std::string pqmode, std::string source, std::string format);
	bool isSetRequired(std::string pqmode,std::string source,std::string format);
	int getParamIndex(string source,string pqmode,string format,int& sourceIndex,int& pqmodeIndex,int& formatIndex);
	tvDataComponentColor_t getComponentColorEnum(std::string colorName);
	int getDolbyParams(tvContentFormatType_t format, std::string &s, std::string source = "");
	tvError_t getParamsCaps(std::vector<std::string> &range, std::vector<std::string> &pqmode, std::vector<std::string> &source, std::vector<std::string> &format,std::string param );
	int getDimmingModeIndex(string mode);
	int saveLocalDimmingLevelToDriverCache(std::string action,std::string pqmode, std::string source, std::string format,int params[] );
        void getDimmingModeStringFromEnum(int value, std::string &toStore);
	void getColorTempStringFromEnum(int value, std::string &toStore);
	int ReadBacklightFromTable(char *panelId);
	int syncCMSParams(std::string pqParam,tvCMS_tunel_t tunnel_type,std::string pqmode, std::string source, std::string format);
	tvError_t syncCMSParamsToDriverCache(std::string pqmode, std::string source, std::string format);
	int getCurrentPictureMode(char *picMode);
	std::string convertSourceIndexToString(int sourceIndex);
	std::string convertVideoFormatToString( int formatIndex );
	bool isIncluded(const std::set<string> set1,const std::set<string> set2);
	void convertUserScaleBacklightToDriverScale(int format,int * params);
	int getDolbyParamToSync(int sourceIndex, int formatIndex, int& value);
	int getHDR10ParamToSync(int& value);
	int getHLGParamToSync( int& value);
	int getHDR10ModeIndex(const char * hdr10Mode);
        int getHLGModeIndex(const char * hlgMode);
        void spliltStringsAndConvertToSet( std::string pqmodeInfo,std::string formatInfo,std::string sourceInfo,std::set<string> &pqmode, std::set<string> &format, std::set<string> &source);
	std::string getDolbyModeStringFromEnum( tvDolbyMode_t mode);
	void SyncWBparams(void);
        tvError_t  SyncWBFromLocalCache( );
        tvError_t CheckWBMigration();
        JsonArray getSupportedVideoSource(void);
        int parsingSetInputArgument(const JsonObject& parameters, std::string pqparam,std::string & source, std::string & pqmode, std::string & format);
        int parsingGetInputArgument(const JsonObject& parameters, std::string pqparam,std::string & source, std::string & pqmode, std::string & format);
        std::string convertToString(std::vector<std::string> vec_strings);
        int FetchCapablities(string pqparam, string & source, string & pqmode, string & format);

    public:
        int m_currentHdmiInResoluton;
        int m_videoZoomMode;
        bool m_isDisabledHdmiIn4KZoom;
        char rfc_caller_id[RFC_BUFF_MAX];
	bool appUsesGlobalBackLightFactor;
	int pic_mode_index[PIC_MODES_SUPPORTED_MAX];
	int source_index[SOURCES_SUPPORTED_MAX];
        AVOutputTV();
        ~AVOutputTV();
        void Initialize();
        void Deinitialize();

        static AVOutputTV *instance;
	static AVOutputTV* getInstance() { return instance; }
	static void dsHdmiVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
        static void dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
        static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
	tvError_t setAspectRatioZoomSettings(tvDisplayMode_t mode);
        tvError_t getUserSelectedAspectRatio (tvDisplayMode_t* mode);
        tvError_t setDefaultAspectRatio(std::string pqmode="all",std::string format="all",std::string source="all");
	tvContentFormatType_t ConvertFormatStringToTVContentFormat(const char *format);

	void NotifyVideoFormatChange(tvVideoHDRFormat_t format);
        void NotifyVideoContentChange(tvContentType_t mode);
        void NotifyVideoResolutionChange(tvResolutionParam_t resolution);
        void NotifyVideoFrameRateChange(tvVideoFrameRate_t frameRate);
};

}//namespace Plugin
}//namespace WPEFramework
#endif
