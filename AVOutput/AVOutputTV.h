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

#ifndef AVOutputTV_H
#define AVOutputTV_H

#include "string.h"
#include <set>

#include "tvTypes.h"
#include "tvSettings.h"
#include <pthread.h>
#include "Module.h"
#include "tvError.h"
#include "tvTypes.h"
#include "tr181api.h"
#include "AVOutputBase.h"
#include "libIARM.h"
#include "libIBusDaemon.h"
#include "libIBus.h"
#include "iarmUtil.h"
#include "UtilsLogging.h"
#include "UtilsJsonRpc.h"
#include "dsError.h"
#include "dsMgr.h"
#include "hdmiIn.hpp"
#include <numeric>

//Macro
#define RFC_BUFF_MAX 100
#define BACKLIGHT_RAW_VALUE_MAX    (255)
#define AVOUTPUT_RFC_CALLERID        "AVOutput"
#define AVOUTPUT_RFC_CALLERID_OVERRIDE        "../../opt/panel/AVOutput"
#define AVOUTPUT_OVERRIDE_PATH       "/opt/panel/AVOutput.ini"
#define AVOUTPUT_CONVERTERBOARD_PANELID     "0_0_00"
#define AVOUTPUT_GENERIC_STRING_RFC_PARAM    "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput."
#define AVOUTPUT_AUTO_BACKLIGHT_MODE_RFC_PARAM  "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.AutoBacklightMode"
#define AVOUTPUT_DOLBYVISIONMODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.DolbyVisionMode"
#define AVOUTPUT_HLGMODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.HLGMode"
#define AVOUTPUT_HDR10MODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.HDR10Mode"
#define AVOUTPUT_DIMMING_MODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.DimmingMode"
#define AVOUTPUT_PICTUREMODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.PictureMode"
#define AVOUTPUT_PICTUREMODE_STRING_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.PictureModeString"
#define AVOUTPUT_ASPECTRATIO_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.ZoomMode"
#define AVOUTPUT_SOURCE_PICTUREMODE_STRING_RFC_PARAM "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.AVOutput.Source"
#define AVOUTPUT_DALS_RFC_PARAM "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AVOutput.DynamicAutoLatency"

#define STRING_DIRTY  ".Dirty."
#define STRING_PICMODE  "PicMode."
#define STRING_FORMAT  "Format."
#define STRING_DEFAULT  "Default"
#define STRING_SOURCE    "Source."
#define CREATE_DIRTY(__X__) (__X__+=STRING_DIRTY)

namespace WPEFramework {
namespace Plugin {

//class AVOutputTV : public PluginHost::IPlugin, public PluginHost::JSONRPC {
class AVOutputTV : public AVOutputBase {
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
		DECLARE_JSON_RPC_METHOD(getBacklightDimmingMode )
		DECLARE_JSON_RPC_METHOD(getSupportedDolbyVisionModes )
		DECLARE_JSON_RPC_METHOD(getSupportedPictureModes )
		DECLARE_JSON_RPC_METHOD(getVideoSourceCaps)
		DECLARE_JSON_RPC_METHOD(getVideoFormatCaps)
		DECLARE_JSON_RPC_METHOD(getVideoFrameRateCaps)
		DECLARE_JSON_RPC_METHOD(getVideoResolutionCaps)
		DECLARE_JSON_RPC_METHOD(getPictureMode )
		DECLARE_JSON_RPC_METHOD(getVideoFormat)
		DECLARE_JSON_RPC_METHOD(getVideoSource)
		DECLARE_JSON_RPC_METHOD(getVideoFrameRate)
		DECLARE_JSON_RPC_METHOD(getVideoResolution)
		DECLARE_JSON_RPC_METHOD(getLowLatencyState)
		DECLARE_JSON_RPC_METHOD(getDolbyVisionMode)
		DECLARE_JSON_RPC_METHOD(getZoomMode)
		DECLARE_JSON_RPC_METHOD(getVideoContentType)


		/*Get Capability API's*/
		DECLARE_JSON_RPC_METHOD(getBacklightCaps)
		DECLARE_JSON_RPC_METHOD(getContrastCaps)
		DECLARE_JSON_RPC_METHOD(getBrightnessCaps)
		DECLARE_JSON_RPC_METHOD(getSharpnessCaps)
		DECLARE_JSON_RPC_METHOD(getSaturationCaps)
		DECLARE_JSON_RPC_METHOD(getHueCaps)
		DECLARE_JSON_RPC_METHOD(getColorTemperatureCaps)
		DECLARE_JSON_RPC_METHOD(getBacklightDimmingModeCaps )
		DECLARE_JSON_RPC_METHOD(getDolbyVisionModeCaps )
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
		DECLARE_JSON_RPC_METHOD(setBacklightDimmingMode )
		DECLARE_JSON_RPC_METHOD(setDolbyVisionMode )
		DECLARE_JSON_RPC_METHOD(setWBCtrl )
		DECLARE_JSON_RPC_METHOD(setPictureMode )
		DECLARE_JSON_RPC_METHOD(signalFilmMakerMode)
		DECLARE_JSON_RPC_METHOD(setZoomMode)
		DECLARE_JSON_RPC_METHOD(setLowLatencyState)
		/*Reset API's*/
		DECLARE_JSON_RPC_METHOD(resetBacklight)
		DECLARE_JSON_RPC_METHOD(resetBrightness )
		DECLARE_JSON_RPC_METHOD(resetContrast )
		DECLARE_JSON_RPC_METHOD(resetSharpness )
		DECLARE_JSON_RPC_METHOD(resetSaturation )
		DECLARE_JSON_RPC_METHOD(resetHue )
		DECLARE_JSON_RPC_METHOD(resetColorTemperature )
		DECLARE_JSON_RPC_METHOD(resetBacklightDimmingMode )
		DECLARE_JSON_RPC_METHOD(resetDolbyVisionMode )
		DECLARE_JSON_RPC_METHOD(resetPictureMode )
		DECLARE_JSON_RPC_METHOD(resetZoomMode)
		DECLARE_JSON_RPC_METHOD(resetLowLatencyState)

    private:

		
		tvContentFormatType_t getContentFormatIndex(tvVideoHDRFormat_t formatToConvert);
		int getPictureModeIndex(std::string pqmode);
		int getSourceIndex(std::string source);
		int getFormatIndex(std::string format);		
		int getPqParamIndex();
		int getParamIndex(string source,string pqmode,string format,int& sourceIndex,int& pqmodeIndex,int& formatIndex);
		int GetDolbyModeIndex(const char * dolbyMode);
		tvDimmingMode_t getDimmingModeIndex(string mode);
		
		bool isIncluded(const std::set<string> set1,const std::set<string> set2);
		bool isSetRequired(std::string pqmode,std::string source,std::string format);
		int isPlatformSupport(std::string pqparam);
		
		void spliltCapablities( std::vector<std::string> &range,std::vector<std::string> &pqmode,std::vector<std::string> &format,std::vector<std::string> &source, std::vector<string> &index,std::string rangeInfo, std::string pqmodeInfo, std::string formatInfo, std::string sourceInfo, std::string indexInfo);
		bool isCapablityCheckPassed( std::string pqmodeInputInfo,std::string sourceInputInfo,std::string formatInputInfo,std::string param );
		int parsingSetInputArgument(const JsonObject& parameters, std::string pqparam,std::string & source, std::string & pqmode, std::string & format);
		int parsingGetInputArgument(const JsonObject& parameters, std::string pqparam,std::string & source, std::string & pqmode, std::string & format);
		void spliltStringsAndConvertToSet( std::string pqmodeInfo,std::string formatInfo,std::string sourceInfo,std::set<string> &pqmode, std::set<string> &format, std::set<string> &source);
		int validateIntegerInputParameter(std::string param, int inputValue);
		int FetchCapablities(string pqparam, string & source, string & pqmode, string & format);
		int validateInputParameter(std::string param, std::string inputValue);

                /* AVoutput ini file default entries */
		void LocatePQSettingsFile(void);
		/* Intialise the last set picture mode at bootup */
		tvError_t InitializePictureMode();		
		

		std::string convertToString(std::vector<std::string> vec_strings);
		void convertParamToLowerCase(std::string &source, std::string &pqmode, std::string &format);
		int convertToValidInputParameter(std::string pqparam, std::string & source, std::string & pqmode, std::string & format);
		string convertSourceIndexToString(int source);
		string convertVideoFormatToString(int format);
		string convertPictureIndexToString(int pqmode);
		tvContentFormatType_t ConvertFormatStringToTVContentFormat(const char *format);
		//std::string convertSourceIndexToString(int sourceIndex);
		//std::string convertVideoFormatToString( int formatIndex );
		void convertUserScaleBacklightToDriverScale(int format,int * params);
	
	        /* Update TR181 with new values when app calls set/reset calls */	
		tvError_t UpdateAVoutputTVParamToHAL(std::string forParam, int source, int pqmode, int format, int value,bool setNotDelete);
		/* updatePQParamsToCache will call updatePQParamToLocalCache for writing to TR181.
		 * it will call TVSettings HAL for setting/saving the value
		 * Will be called whenever the application invokes set/reset call
		 */
		int UpdateAVoutputTVParam( std::string action, std::string tr181ParamName, std::string pqmode, std::string source, std::string format, tvPQParameterIndex_t pqParamIndex, int params[] );

		/* Every bootup this function is called to sync TR181 to TVSettings HAL for saving the value */
		tvError_t SyncAvoutputTVParamsToHAL(std::string pqmode, std::string source, std::string format);
		/* Every Bootup this function is called to sync TR181 to TVSettings HAL for saving the picture mode assiocation to source */
		int SyncAvoutputTVPQModeParamsToHAL(std::string pqmode, std::string source, std::string format);
		
		uint32_t generateStorageIdentifier(std::string &key, std::string forParam,int contentFormat, int pqmode, int source);
		uint32_t generateStorageIdentifierDirty(std::string &key, std::string forParam,uint32_t contentFormat, int pqmode);

		std::string getErrorString (tvError_t eReturn);

		/* Get function to query TR181 entries or pq capability.ini file*/
		int getSaveConfig(std::string pqmode, std::string source, std::string format,std::vector<int> &sources,std::vector<int> &picturemodes, std::vector<int> &formats);
		int getLocalparam(std::string forParam,int formatIndex,int pqIndex,int sourceIndex,int &value,
		  tvPQParameterIndex_t pqParamIndex ,bool cms=false,int tunnel_type=0);
		tvDataComponentColor_t getComponentColorEnum(std::string colorName);
		int getDolbyParams(tvContentFormatType_t format, std::string &s, std::string source = "");
		tvError_t getParamsCaps(std::vector<std::string> &range, std::vector<std::string> &pqmode, std::vector<std::string> &source, std::vector<std::string> &format,std::string param );
		tvError_t getParamsCaps(std::vector<std::string> &range, std::vector<std::string> &pqmode, std::vector<std::string> &source,
		                        std::vector<std::string> &format,std::string param , std::string & isPlatformSupport,
				std::vector<std::string> & index);
		void getDimmingModeStringFromEnum(int value, std::string &toStore);
		void getColorTempStringFromEnum(int value, std::string &toStore);
		int getCurrentPictureMode(char *picMode);
		int getDolbyParamToSync(int sourceIndex, int formatIndex, int& value);
		std::string getDolbyModeStringFromEnum( tvDolbyMode_t mode);
		JsonArray getSupportedVideoSource(void);
		int getAvailableCapabilityModesWrapper(std::string param, std::string & outparam);
		int getAvailableCapabilityModes(std::string & source, std::string & pqmode, std::string & format);
		int getCapabilitySource(JsonArray &rangeArray);
		int getRangeCapability(std::string param, std::vector<std::string> & rangeInfo);
		void getDynamicAutoLatencyConfig();
		tvError_t getUserSelectedAspectRatio (tvDisplayMode_t* mode);

		void BroadcastLowLatencyModeChangeEvent(bool lowLatencyMode);

		tvError_t setAspectRatioZoomSettings(tvDisplayMode_t mode);
		tvError_t setDefaultAspectRatio(std::string pqmode="all",std::string format="all",std::string source="all");

	public:
		int m_currentHdmiInResoluton;
		int m_videoZoomMode;
		bool m_isDisabledHdmiIn4KZoom;
		char rfc_caller_id[RFC_BUFF_MAX];
		bool appUsesGlobalBackLightFactor;
		int pic_mode_index[PIC_MODES_SUPPORTED_MAX];
		
		AVOutputTV();
		~AVOutputTV();

		static AVOutputTV *instance;
		static AVOutputTV* getInstance() { return instance; }

		void NotifyVideoFormatChange(tvVideoFormatType_t format);
		void NotifyFilmMakerModeChange(tvContentType_t mode);
		void NotifyVideoResolutionChange(tvResolutionParam_t resolution);
		void NotifyVideoFrameRateChange(tvVideoFrameRate_t frameRate);
		
		//override API
		static void dsHdmiVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
		static void dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
		static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
		
		void Initialize();
		void Deinitialize();
		void InitializeIARM();
		void DeinitializeIARM();
};

}//namespace Plugin
}//namespace WPEFramework
#endif
