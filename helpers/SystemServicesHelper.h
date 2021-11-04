/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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
**/

#ifndef __SYSTEM_SERVICE_HELPER_H__
#define __SYSTEM_SERVICE_HELPER_H__

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <regex>
#include <time.h>
#include <curl/curl.h>

#include "utils.h"

/* Status-keeper files */

#if defined (PLATFORM_BROADCOM) || defined (PLATFORM_BROADCOM_REF) || defined (PLATFORM_REALTEK) || defined (PLATFORM_AMLOGIC)
#define SYSTEM_SERVICE_REBOOT_INFO_FILE             "/opt/secure/reboot/reboot.info"
#define SYSTEM_SERVICE_PREVIOUS_REBOOT_INFO_FILE    "/opt/secure/reboot/previousreboot.info"
#define SYSTEM_SERVICE_HARD_POWER_INFO_FILE         "/opt/secure/reboot/hardpower.info"
#else
#define SYSTEM_SERVICE_REBOOT_INFO_FILE             "/opt/persistent/reboot.info"
#define SYSTEM_SERVICE_PREVIOUS_REBOOT_INFO_FILE    "/opt/persistent/previousreboot.info"
#define SYSTEM_SERVICE_HARD_POWER_INFO_FILE         "/opt/persistent/hardpower.info"
#endif



#define SYSTEM_UP_TIME_FILE                     "/proc/uptime"
#define MOCA_FILE                               "/opt/enablemoca"
#define VERSION_FILE_NAME                       "/version.txt"
#define SYSTEM_SERVICE_SETTINGS_FILE            "/opt/system_service_settings.conf"
#define SYSTEM_SERVICE_TEMP_FILE                "/tmp/system_service_temp.conf"
#define FWDNLDSTATUS_FILE_NAME                  "/opt/fwdnldstatus.txt"
#define REBOOT_INFO_LOG_FILE                    "/opt/logs/rebootInfo.log"
#define STANDBY_REASON_FILE                     "/opt/standbyReason.txt"
#define TMP_SERIAL_NUMBER_FILE                  "/tmp/.STB_SER_NO"
#define WAREHOUSE_MODE_FILE                     "/opt/warehouse_mode_active"
#define PREVIOUS_REBOOT_INFO2_ONE_CALL_FILE     "/tmp/previousRebootInfoOneCall"
#define MILESTONES_LOG_FILE                     "/opt/logs/rdk_milestones.log"
#define RECEIVER_STANDBY_PREFS                  "/tmp/retainConnection"
#define TZ_REGEX                                "^[0-9a-zA-Z/-+_]*$"
#define PREVIOUS_KEYPRESS_INFO_FILE             "/opt/persistent/previouskeypress.info"
#define XCONF_OVERRIDE_FILE						"/opt/swupdate.conf"
#define	URL_XCONF								"http://xconf.xcal.tv/xconf/swu/stb"
#define TZ_FILE									"/opt/persistent/timeZoneDST"
#define DEVICE_PROPERTIES                       "/etc/device.properties"
#define OPT_DCM_PROPERTIES                      "/opt/dcm.properties"
#define ETC_DCM_PROPERTIES                      "/etc/dcm.properties"
#define TMP_DCM_SETTINGS                        "/tmp/DCMSettings.conf"


#define MODE_TIMER_UPDATE_INTERVAL	1000
#define CURL_BUFFER_SIZE	(64 * 1024) /* 256kB */

#define IARM_BUS_PWRMGR_NAME					"PWRMgr"  /*!< Power manager IARM bus name */
#define IARM_BUS_PWRMGR_API_SetDeepSleepTimeOut	"SetDeepSleepTimeOut" /*!< Sets the timeout for deep sleep*/

#define MODE_NORMAL     "NORMAL"
#define MODE_EAS        "EAS"
#define MODE_WAREHOUSE  "WAREHOUSE"

#define CAT_DWNLDPROGRESSFILE_AND_GET_INFO "cat /opt/curl_progress | tr -s '\r' '\n' | tail -n 1 | sed 's/^ *//g' | sed '/^[^M/G]*$/d' | tr -s ' ' | cut -d ' ' -f3"

enum eRetval { E_NOK = -1,
    E_OK };

enum SysSrv_ErrorCode {
    SysSrv_OK = 0,
    SysSrv_MethodNotFound,
    SysSrv_MissingKeyValues,
    SysSrv_UnSupportedFormat,
    SysSrv_FileNotPresent,
    SysSrv_FileAccessFailed,
    SysSrv_FileContentUnsupported,
    SysSrv_Unexpected,
    SysSrv_SupportNotAvailable,
    SysSrv_LibcurlError,
    SysSrv_DynamicMemoryAllocationFailed,
    SysSrv_ManufacturerDataReadFailed,
    SysSrv_KeyNotFound
};

enum FirmwareUpdateState {
    FirmwareUpdateStateUninitialized = 0,
    FirmwareUpdateStateRequesting,
    FirmwareUpdateStateDownloading,
    FirmwareUpdateStateFailed,
    FirmwareUpdateStateDownloadComplete,
    FirmwareUpdateStateValidationComplete,
    FirmwareUpdateStatePreparingReboot,
    FirmwareUpdateStateNoUpgradeNeeded
};

const string GZ_STATUS = "/opt/gzenabled";

/* Used as CURL Data buffer */
struct write_result {
    char *data;
    int pos;
};

/* Forward Declaration */
uint32_t enableXREConnectionRetentionHelper(bool);

/* Helper Functions */
using namespace std;

/**
  * @brief : To map the error code with matching error message.
  * @param1[in] : error code of type SysSrv_ErrorCode.
  * @return : string; error message.
  */
std::string getErrorDescription(int errCode);

std::string dirnameOf(const std::string& fname);

/***
 * @brief	: Used to check if directory exists
 * @param1[in]	: Complete file name with path
 * @param2[in]	: Destination string to be filled with file contents
 * @return	: <bool>; TRUE if operation success; else FALSE.
 */
bool dirExists(std::string fname);

/***
 * @brief	: Used to read file contents into a string
 * @param1[in]	: Complete file name with path
 * @param2[in]	: Destination string to be filled with file contents
 * @return	: <bool>; TRUE if operation success; else FALSE.
 */
bool readFromFile(const char* filename, string &content);

namespace WPEFramework {
    namespace Plugin {
        /***
         * @brief   : Used to construct response with module error status.
         * @param1[in]  : Error Code
         * @param2[out]: "response" JSON Object which is returned by the API
         with updated module error status.
         */
        void populateResponseWithError(int errorCode, JsonObject& response);
        string caseInsensitive(string str7);
        string ltrim(const string& s);
        string rtrim(const string& s);
        string trim(const string& s);
        string getModel();
        string convertCase(string str);
        bool convert(string str3,string firm);
    } //namespace Plugin
} //namespace WPEFramework

/***
 * @brief  : Used to construct JSON response from Vector.
 * @param1[in] : Destination JSON response buffer
 * @param2[in] : JSON "Key"
 * @param3[in] : Source Vector.
 * @return : <bool>; TRUE if operation success; else FALSE.
 */
void setJSONResponseArray(JsonObject& response, const char* key,
        const vector<string>& items);

/***
 * @brief	: Used to read file contents into a string
 * @param1[in]	: Complete file name with path
 * @param2[out]	: Destination string object filled with file contents
 * @return	: <bool>; TRUE if operation success; else FALSE.
 */
bool getFileContent(std::string fileName, std::string& fileContent);

/***
 * @brief  : Used to read file contents into a vector
 * @param1[in] : Complete file name with path
 * @param2[in] : Destination vector buffer to be filled with file contents
 * @return : <bool>; TRUE if operation success; else FALSE.
 */
bool getFileContent(std::string fileName, std::vector<std::string> & vecOfStrs);

/***
 * @brief	: Used to search for files in the given directory
 * @param1[in]	: Directory on which the search has to be performed
 * @param2[in]	: Filter for the search command
 * @return	: <vector<std::string>>; Vector of file names.
 */
std::vector<std::string> searchAndGetFilesList(std::string path, std::string filter);

/***
 * @brief  : compare two C string case insensitively
 * @param1[in] : c string 1
 * @param2[in] : c string 2
 * @return : <bool>; 0 is strings are same, some number if strings are different.
 */
int strcicmp(char const *a, char const *b);

/**
 * @brief	: To find case insensitive substring in a given string.
 * @param1[in]	: Haystack buffer
 * @param2[in]	: Needle to be searched in Haystack
 * @return	: <bool> ; TRUE if match found; else FALSE.
 */
bool findCaseInsensitive(std::string data, std::string toSearch, size_t pos = 0);

/***
 * @brief	: To retrieve Xconf version of URL to override
 * @param1[out]	: bFileExists - Returns true if /opt/swupdate.conf is present
 * @return	: string
 */
string getXconfOverrideUrl(bool& bFileExists);

/***
 * @brief	: To retrieve TimeZone
 * @return	: string; TimeZone
 */
string getTimeZoneDSTHelper(void);

/***
 * @brief		: To retrieve system time in requested format
 * @param1[in]	: requested format conversion info
 * @return		: string;
 */
string currentDateTimeUtc(const char *fmt);

/***
 * @brief	: To construct url encoded from string passed
 * @param1[in]	: string; url to be encoded
 * @return		: string; encoded url
 */
std::string url_encode(std::string urlIn);

/***
 * @brief   : To construct url encoded from string passed
 * @param1[in]  : CURL *; poinetr to curl init handle
 * @param2[in]  : string; url to be encoded
 * @return      : string; encoded url
 */
std::string urlEncodeField(CURL *curl_handle, std::string &data);

/***
 * @brief	: To retrieve model details
 * @return	: string
 */
std::string getModel(void);

/**
  * @brief : CURL write handle call back.
  * @reference :
  */
size_t writeCurlResponse(void *ptr, size_t size, size_t nmemb, std::string stream);

/***
 * @brief  : extract mac address value to each key
 * @param1[in] : the entire string from which makc addresses can be extracted
 * @param2[in] : Key to each mac address
 * @param2[out]: mac address value to each key
 */
void findMacInString(std::string totalStr, std::string macId, std::string& mac);

std::string stringTodate(char *pBuffer);

/**
 * @brief Used to used to remove characters from string.
 * @param1[in] : The string that has to striped of the given characters
 * @param2[in] : char pointer too character arrya that contains all the
 * 				 characters to be removed
 * @param2[out]: String striped of given characters
 */
void removeCharsFromString(string &str, const char* charsToRemove);

/**
 * @brief Used to parse and get value of a particular key from config file
 * seperated by '=' (eg: abc=xyz, where abc is key and xyz is value).
 * @param1[in] : Filename(with absolute path) of the confiuration file
 * @param2[in] : Key which we need to find the value
 * @param3[in] : An empty buffer where the value will be updated
 *
 * @param2[out]: Boolean value either true/false
 */

bool parseConfigFile(const char* filename, string findkey, string &value);

/**
 * @brief Curl write request callback handler.
 */
size_t curl_write(void *ptr, size_t size, size_t nmemb, void *stream);

enum FwFailReason
{
    FwFailReasonNone = 0,
    FwFailReasonNotFound,
    FwFailReasonNetworkFailure,
    FwFailReasonServerUnreachable,
    FwFailReasonCorruptDownloadFile,
    FwFailReasonFailureInFlashWrite,
    FwFailReasonUpgradeFailedAfterFlashWrite,
};

static const std::map<FwFailReason, string> FwFailReasonToText =
        {
                {FwFailReasonNone, "None"},
                {FwFailReasonNotFound, "Not found"},
                {FwFailReasonNetworkFailure, "Network failure"},
                {FwFailReasonServerUnreachable, "Server unreachable"},
                {FwFailReasonCorruptDownloadFile, "Corrupt download file"},
                {FwFailReasonFailureInFlashWrite, "Failure in flash write"},
                {FwFailReasonUpgradeFailedAfterFlashWrite, "Upgrade failed after flash write"},
        };

static const std::map<string, FwFailReason> FwFailReasonFromText =
        {
                {"ESTB Download Failure", FwFailReasonServerUnreachable},
                {"Image Download Failed - Unable to connect", FwFailReasonNetworkFailure},
                {"Image Download Failed - Server not Found", FwFailReasonNotFound},
                {"Image Download Failed - Error response from server", FwFailReasonServerUnreachable},
                {"Image Download Failed - Unknown", FwFailReasonServerUnreachable},
                {"Image download failed from server", FwFailReasonServerUnreachable}, // firmwareDwnld.sh only
                {"RCDL Upgrade Failed", FwFailReasonFailureInFlashWrite},
                {"ECM trigger failed", FwFailReasonFailureInFlashWrite},
                {"Failed in flash write", FwFailReasonFailureInFlashWrite},
                {"Flashing failed", FwFailReasonFailureInFlashWrite}, // userInitiatedFWDnld.sh only
                {"Versions Match", FwFailReasonNone}, // XConf
                {"Cloud FW Version is empty", FwFailReasonNone}, // XConf
                {"Cloud FW Version is invalid", FwFailReasonNone}, // XConf
                {"Invalid Request", FwFailReasonNone}, // XConf
                {"Network Communication Error", FwFailReasonNone}, // XConf
                {"Previous Upgrade In Progress", FwFailReasonNone}, // firmwareDwnld.sh only
                {"Empty image name from CDL server", FwFailReasonNone}, // firmwareDwnld.sh only
                {"Upgrade failed after flash write", FwFailReasonUpgradeFailedAfterFlashWrite},
        };

#endif /* __SYSTEM_SERVICE_HELPER_H__ */

