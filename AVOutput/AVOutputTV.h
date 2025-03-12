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
#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "tvTypes.h"
#include "tvSettings.h"
#include <pthread.h>
#include "Module.h"
#include "tvError.h"
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
#define STRING_COMPONENT    "Component."
#define STRING_COLOR    "Color."
#define STRING_CONTROL   "Control."
#define STRING_COLORTEMPERATURE   "ColorTemperature."
#define CREATE_DIRTY(__X__) (__X__+=STRING_DIRTY)
#define CAPABLITY_FILE_NAME    "pq_capabilities.ini"


class CIniFile
{
	std::string m_path;
	std::string opt_path;
	boost::property_tree::ptree m_data;

public:
	CIniFile(const std::string & filename, const std::string & filepath = "/etc/" )
	{
		opt_path = "/opt/panel/";
		m_path = filepath;
		m_path.append(filename);
		opt_path.append(filename);

		if(!boost::filesystem::exists( opt_path)) {
			std::cout << "AVOutput : Using " << m_path <<std::endl;
			boost::property_tree::ini_parser::read_ini(m_path, m_data);
		}
		else {
			std::cout << "AVOutput : Using " << opt_path << std::endl;
			boost::property_tree::ini_parser::read_ini(opt_path, m_data);
	        }
	}

	~CIniFile()
	{
	}

	template <typename T>
	T Get(const std::string & key)
	{
		return m_data.get<T>(key);
	}

	template <typename T>
	void Set(const std::string & key, const T & value){
		//TODO DD: Not required currently
		//m_data.put(key, value);
	}
};

namespace WPEFramework {
namespace Plugin {

typedef struct
{
    std::string range;
    std::string pqmode;
    std::string format;
    std::string source;
    std::string isPlatformSupport;
    std::string index;
    std::string color;
    std::string component;
    std::string colorTemperature;
    std::string control;
}capDetails_t;

typedef struct
{
    std::vector<std::string> rangeVector;
    std::vector<std::string> pqmodeVector;
    std::vector<std::string> formatVector;
    std::vector<std::string> sourceVector;
    std::vector<std::string> isPlatformSupportVector;
    std::vector<std::string> indexVector;
    std::vector<std::string> colorVector;
    std::vector<std::string> componentVector;
    std::vector<std::string> colorTempVector;
    std::vector<std::string> controlVector;
}capVectors_t;


typedef struct
{
    std::vector<int> rangeValues;
    std::vector<int> pqmodeValues;
    std::vector<int> formatValues;
    std::vector<int> sourceValues;
    std::vector<int> isPlatformSupportValues;
    std::vector<int> indexValues;
    std::vector<int> colorValues;
    std::vector<int> componentValues;
    std::vector<int> colorTempValues;
    std::vector<int> controlValues;
}valueVectors_t;

typedef struct
{
    uint8_t sourceIndex;
    uint8_t pqmodeIndex;
    uint8_t formatIndex;
    uint8_t colorIndex;
    uint8_t componentIndex;
    uint8_t colorTempIndex;
    uint8_t controlIndex;
}paramIndex_t;


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
		DECLARE_JSON_RPC_METHOD(getDolbyVisionMode)
		DECLARE_JSON_RPC_METHOD(getSupportedPictureModes )
		DECLARE_JSON_RPC_METHOD(getPictureMode )
		DECLARE_JSON_RPC_METHOD(getVideoFormat)
		DECLARE_JSON_RPC_METHOD(getVideoSource)
		DECLARE_JSON_RPC_METHOD(getVideoFrameRate)
		DECLARE_JSON_RPC_METHOD(getVideoResolution)
		DECLARE_JSON_RPC_METHOD(getLowLatencyState)
		DECLARE_JSON_RPC_METHOD(getZoomMode)
		DECLARE_JSON_RPC_METHOD(getVideoContentType)
		DECLARE_JSON_RPC_METHOD(getCMS)
		DECLARE_JSON_RPC_METHOD(getHDRMode)
		DECLARE_JSON_RPC_METHOD(get2PointWB)
		DECLARE_JSON_RPC_METHOD(getAutoBacklightMode)


		/*Get Capability API's*/
		DECLARE_JSON_RPC_METHOD(getBacklightCaps)
		DECLARE_JSON_RPC_METHOD(getBrightnessCaps)
		DECLARE_JSON_RPC_METHOD(getContrastCaps)
		DECLARE_JSON_RPC_METHOD(getSharpnessCaps)
		DECLARE_JSON_RPC_METHOD(getSaturationCaps)
		DECLARE_JSON_RPC_METHOD(getHueCaps)
		DECLARE_JSON_RPC_METHOD(getColorTemperatureCaps)
		DECLARE_JSON_RPC_METHOD(getBacklightDimmingModeCaps )
		DECLARE_JSON_RPC_METHOD(getDolbyVisionModeCaps )
		DECLARE_JSON_RPC_METHOD(getPictureModeCaps)
		DECLARE_JSON_RPC_METHOD(getVideoFormatCaps)
		DECLARE_JSON_RPC_METHOD(getVideoSourceCaps)
		DECLARE_JSON_RPC_METHOD(getVideoFrameRateCaps)
		DECLARE_JSON_RPC_METHOD(getVideoResolutionCaps)
		DECLARE_JSON_RPC_METHOD(getLowLatencyStateCaps)
		DECLARE_JSON_RPC_METHOD(getZoomModeCaps)
		DECLARE_JSON_RPC_METHOD(getCMSCaps)
		DECLARE_JSON_RPC_METHOD(get2PointWBCaps)
		DECLARE_JSON_RPC_METHOD(getHDRModeCaps)
		DECLARE_JSON_RPC_METHOD(getAutoBacklightModeCaps)

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
		DECLARE_JSON_RPC_METHOD(setPictureMode )
		DECLARE_JSON_RPC_METHOD(setLowLatencyState)
		DECLARE_JSON_RPC_METHOD(setZoomMode)
		DECLARE_JSON_RPC_METHOD(setWBCtrl )
		DECLARE_JSON_RPC_METHOD(setHDRMode )
		DECLARE_JSON_RPC_METHOD(setCMS )
		DECLARE_JSON_RPC_METHOD(set2PointWB )
 		DECLARE_JSON_RPC_METHOD(signalFilmMakerMode)
		DECLARE_JSON_RPC_METHOD(setAutoBacklightMode)

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
		DECLARE_JSON_RPC_METHOD(resetLowLatencyState)
		DECLARE_JSON_RPC_METHOD(resetZoomMode)
		DECLARE_JSON_RPC_METHOD(resetHDRMode)
		DECLARE_JSON_RPC_METHOD(resetCMS)
		DECLARE_JSON_RPC_METHOD(reset2PointWB)
		DECLARE_JSON_RPC_METHOD(resetAutoBacklightMode)

    private:

		
		int getPictureModeIndex(std::string pqmode);
		int getSourceIndex(std::string source);
		int getFormatIndex(std::string format);		
		int getPqParamIndex();
		int getParamIndex(std::string param, capDetails_t& paramInfo, paramIndex_t& indexInfo);
		int getDolbyModeIndex(const char * dolbyMode);
		int getHDRModeIndex(const std::string HDRMode, const std::string format,tvDolbyMode_t &value);
		tvDimmingMode_t getDimmingModeIndex(string mode);
		
		bool isIncluded(const std::set<string> set1,const std::set<string> set2);
		bool isSetRequired(std::string pqmode,std::string source,std::string format);
		int isPlatformSupport(std::string pqparam);
		
	
		bool isCapablityCheckPassed( std::string param, capDetails_t inputInfo );
		int parsingSetInputArgument(const JsonObject& parameters, std::string pqparam,capDetails_t& paramInfo);
		int parsingGetInputArgument(const JsonObject& parameters, std::string pqparam, capDetails_t& info);
		void spliltCapablities( capVectors_t& vectorInfo, capDetails_t stringInfo);
		void spliltStringsAndConvertToSet( std::string pqmodeInfo,std::string formatInfo,std::string sourceInfo,std::set<string> &pqmode, std::set<string> &format, std::set<string> &source);
		int validateIntegerInputParameter(std::string param, int inputValue);
		int fetchCapablities(string pqparam, capDetails_t& info);
		int validateInputParameter(std::string param, std::string inputValue);
		int validateWBParameter(std::string param,std::string control,int inputValue);
		int validateCMSParameter(std::string component,int inputValue);

                /* AVoutput ini file default entries */
		void locatePQSettingsFile(void);
		/* Intialise the last set picture mode at bootup */
		tvError_t initializePictureMode();		
		

		std::string convertToString(std::vector<std::string> vec_strings);
		void convertParamToLowerCase(std::string &source, std::string &pqmode, std::string &format);
		int convertToValidInputParameter(std::string pqparam, capDetails_t& info);
		string convertSourceIndexToString(int source);
		string convertVideoFormatToString(int format);
		string convertPictureIndexToString(int pqmode);
		//std::string convertSourceIndexToString(int sourceIndex);
		//std::string convertVideoFormatToString( int formatIndex );
		void convertUserScaleBacklightToDriverScale(int format,int * params);
	
	        /* Update TR181 with new values when app calls set/reset calls */	
		tvError_t updateAVoutputTVParamToHAL(std::string forParam, paramIndex_t indexInfo, int value,bool setNotDelete);
		/* updatePQParamsToCache will call updatePQParamToLocalCache for writing to TR181.
		 * it will call TVSettings HAL for setting/saving the value
		 * Will be called whenever the application invokes set/reset call
		 */
		int updateAVoutputTVParam( std::string action, std::string tr181ParamName, capDetails_t info, tvPQParameterIndex_t pqParamIndex, int level );

		/* Every bootup this function is called to sync TR181 to TVSettings HAL for saving the value */
		tvError_t syncAvoutputTVParamsToHAL(std::string pqmode, std::string source, std::string format);
		/* Every Bootup this function is called to sync TR181 to TVSettings HAL for saving the picture mode assiocation to source */
		int syncAvoutputTVPQModeParamsToHAL(std::string pqmode, std::string source, std::string format);
		void syncCMSParams( );
		void syncWBParams( );
		
		uint32_t generateStorageIdentifier(std::string &key, std::string forParam,paramIndex_t info);
		uint32_t generateStorageIdentifierCMS(std::string &key, std::string forParam, paramIndex_t info);
		uint32_t generateStorageIdentifierWB(std::string &key, std::string forParam, paramIndex_t info);
		uint32_t generateStorageIdentifierDirty(std::string &key, std::string forParam,uint32_t contentFormat, int pqmode);

		std::string getErrorString (tvError_t eReturn);

		/* Get function to query TR181 entries or pq capability.ini file*/
		int getSaveConfig(std::string param, capDetails_t capInfo, valueVectors_t &values);
		int getLocalparam( std::string forParam,paramIndex_t indexInfo,int & value,tvPQParameterIndex_t pqParamIndex,bool sync=false);
		
		tvDataComponentColor_t getComponentColorEnum(std::string colorName);
		tvError_t getParamsCaps(std::string param, capVectors_t &vecInfo);
		int GetPanelID(char *panelid);
		int ReadCapablitiesFromConf(std::string param, capDetails_t& info);
		void getDimmingModeStringFromEnum(int value, std::string &toStore);
		void getColorTempStringFromEnum(int value, std::string &toStore);
		int getCurrentPictureMode(char *picMode);
		int getDolbyParamToSync(int sourceIndex, int formatIndex, int& value);
		tvDolbyMode_t GetDolbyVisionEnumFromModeString(const char* modeString);
		std::string getDolbyModeStringFromEnum( tvDolbyMode_t mode);
		JsonArray getSupportedVideoSource(void);
		int getAvailableCapabilityModesWrapper(std::string param, std::string & outparam);
		int getAvailableCapabilityModes( capDetails_t& info );
		int getCapabilitySource(JsonArray &rangeArray);
		int getRangeCapability(std::string param, std::vector<std::string> & rangeInfo);
		void getDynamicAutoLatencyConfig();
		tvError_t getUserSelectedAspectRatio (tvDisplayMode_t* mode);
		std::string  getColorTemperatureStringFromEnum(tvColorTemp_t value);
		std::string getCMSColorStringFromEnum(tvDataComponentColor_t value);
		std::string getCMSComponentStringFromEnum(tvComponentType_t value);
		std::string getWBControlStringFromEnum(tvWBControl_t value);
		int getCMSColorEnumFromString(std::string color,tvDataComponentColor_t &value);
		int getCMSComponentEnumFromString(std::string component, tvComponentType_t& value);
		std::string getWBColorStringFromEnum(tvWBColor_t value);
		int getWBColorEnumFromString(std::string color,tvWBColor_t& value);
		int getWBControlEnumFromString(std::string color,tvWBControl_t& value);
		int getColorTempEnumFromString(std::string color, tvColorTemp_t& value);

		bool checkCMSColorAndComponentCapability(const std::string capValue, const std::string inputValue);
		int convertCMSParamToPQEnum(const std::string component, const std::string color,tvPQParameterIndex_t& value);
		int convertWBParamToPQEnum(const std::string control, const std::string color,tvPQParameterIndex_t& value);
		int convertWBParamToRGBEnum(const std::string color,const std::string control,tvRGBType_t &value);

		void broadcastLowLatencyModeChangeEvent(bool lowLatencyMode);
		tvError_t setAspectRatioZoomSettings(tvDisplayMode_t mode);
		tvError_t setDefaultAspectRatio(std::string pqmode="none",std::string format="none",std::string source="none");

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
