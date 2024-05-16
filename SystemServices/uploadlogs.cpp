/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
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

#include "uploadlogs.h"

#include <curl/curl.h>
#include <sstream>
#include <map>

#include "SystemServicesHelper.h"

#include "rfcapi.h"

#include "UtilsCStr.h"
#include "UtilsLogging.h"
#include "UtilscRunScript.h"
#include "UtilsfileExists.h"

#define TR181_MTLS_LOGUPLOAD "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.MTLS.mTlsLogUpload.Enable"
#define TR181_LOGUPLOAD_BEF_DEEPSLEEP "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.LogUploadBeforeDeepSleep.Enable"

namespace WPEFramework
{
namespace Plugin
{
namespace UploadLogs
{
bool checkXpkiMtlsBasedLogUpload(){
    if ( Utils::fileExists("/usr/bin/rdkssacli") &&
            Utils::fileExists("/opt/certs/devicecert_1.pk12") ){
        return true;
    }
    else {
        return false;
    }
}

bool checkmTlsLogUploadFlag(){
    bool ret=false;
    RFC_ParamData_t param;
    WDMP_STATUS wdmpStatus = getRFCParameter(const_cast<char *>("SystemServices"),TR181_MTLS_LOGUPLOAD, &param);
    if (wdmpStatus == WDMP_SUCCESS || wdmpStatus == WDMP_ERR_DEFAULT_VALUE){
        if( param.type == WDMP_BOOLEAN ){
            if(strncasecmp(param.value,"true",4) == 0 ){
                ret=true;
            }
        }
    }
    LOGINFO(" mTlsLogUpload.Enable = %s , call value %d ", (ret == true)?"true":"false", wdmpStatus);
    return ret;
}

bool checkLogUploadBeforeDeepSleepFlag(){
    bool ret=false;
    RFC_ParamData_t param;
    WDMP_STATUS wdmpStatus = getRFCParameter(const_cast<char *>("SystemServices"),TR181_LOGUPLOAD_BEF_DEEPSLEEP, &param);
    if (wdmpStatus == WDMP_SUCCESS || wdmpStatus == WDMP_ERR_DEFAULT_VALUE){
        if( param.type == WDMP_BOOLEAN ){
            if(strncasecmp(param.value,"true",4) == 0 ){
                ret=true;
            }
        }
    }
    LOGINFO(" mTlsLogUpload.Enable = %s , call value %d ", (ret == true)?"true":"false", wdmpStatus);
    return ret;
}


bool getDCMconfigDetails(string &upload_protocol,string &httplink, string &uploadCheck){

    string dcminfo;

    if (!getFileContent(TMP_DCM_SETTINGS,dcminfo)) {
        return false;
    }
    if (dcminfo.length() < 1){
        return false;
    }

    smatch match;
    string temp;
    if (regex_search(dcminfo, match, regex("LogUploadSettings:UploadRepository:uploadProtocol=([^\\n]+)"))
            &&  match.size() > 1) temp = trim(match[1]);
    if (temp.size() > 0) upload_protocol = temp;

    if (regex_search(dcminfo, match, regex("LogUploadSettings:UploadRepository:URL=([^\\n]+)"))
            &&  match.size() > 1) temp = trim(match[1]);
    if (temp.size() > 0) httplink = temp;

    if (regex_search(dcminfo, match, regex("LogUploadSettings:UploadOnReboot=([^\\n]+)"))
            &&  match.size() > 1) temp = trim(match[1]);
    if (temp.size() > 0) uploadCheck = temp;

    return true;
}

std::int32_t getUploadLogParameters(string &tftp_server, string &upload_protocol, string &upload_httplink)
{
    string build_type;
    string httplink;
    string uploadcheck;
    string dcmFile, force_mtls;
    string calledFromPlugin="1";
    bool mTlsLogUpload = false;

    mTlsLogUpload = checkmTlsLogUploadFlag();

    if ( !parseConfigFile(DEVICE_PROPERTIES,"BUILD_TYPE",build_type) ){
        LOGINFO("Failed to get BUILD Type\n");
        return E_NOK;
    }

    if ( ( "prod" != build_type ) && ( Utils::fileExists(OPT_DCM_PROPERTIES) )){
        dcmFile=OPT_DCM_PROPERTIES;
    }
    else{
        dcmFile=ETC_DCM_PROPERTIES;
    }

    if ( !parseConfigFile(dcmFile.c_str(),"LOG_SERVER",tftp_server) ){
        LOGINFO("Failed to get Log server\n");
        return E_NOK;
    }

    if (parseConfigFile(DEVICE_PROPERTIES,"FORCE_MTLS",force_mtls) ){
        if ( "true" == force_mtls ){
            mTlsLogUpload = true;
        }
    }

    if ( !getDCMconfigDetails(upload_protocol,httplink,uploadcheck)){
        LOGINFO("Failed to get DCM configDetails\n");
        return E_NOK;
    }

   upload_httplink = httplink;

   if ( mTlsLogUpload || checkXpkiMtlsBasedLogUpload() ){
       //Sky endpoint dont use /secure extension
       if( "true" != force_mtls ){
        //append secure with the url
        upload_httplink=regex_replace(httplink,regex("cgi-bin"),"secure/cgi-bin");
       }
   }

   return E_OK;
}


/* Call uploadSTBLogs.sh direclty by preparing the url
 * and other details
 */

std::int32_t LogUploadBeforeDeepSleep()
{
    /* Check the RFC flag */
    if( !checkLogUploadBeforeDeepSleepFlag()){
        LOGINFO("LogUploadBeforeDeepSleep RFC is False.\n");
        return E_NOK;
    }

    if ( !Utils::fileExists("/lib/rdk/uploadSTBLogs.sh") ){
        return E_NOK;
    }

    string tftp_server;
    string upload_protocol;
    string upload_httplink;
    string calledFromPlugin="1";

    if (E_NOK == getUploadLogParameters(tftp_server, upload_protocol, upload_httplink))
        return E_NOK;

   string cmd;
   cmd = "nice -n 19 /bin/busybox sh /lib/rdk/uploadSTBLogs.sh " + tftp_server + " 0 1 0 " + upload_protocol + " " + upload_httplink + " " + calledFromPlugin + " & " + " \0";
   int exec_status = system(cmd.c_str());
   LOGINFO("CMD %s [exec_status:%d]",cmd.c_str(), exec_status);

   return E_OK;
}

pid_t logUploadAsync(void)
{
    if ( !Utils::fileExists("/lib/rdk/uploadSTBLogs.sh") ){
        return -1;
    }

    string tftp_server;
    string upload_protocol;
    string upload_httplink;

    if (E_NOK == getUploadLogParameters(tftp_server, upload_protocol, upload_httplink))
        return -1;

    const char *argArray[] = {
        "/bin/sh",
        "/lib/rdk/uploadSTBLogs.sh",
        tftp_server.c_str(),
        "0", //FLAG,
        "1", //DCM_FLAG,
        "0", //UploadOnReboot,
        upload_protocol.c_str(),
        upload_httplink.c_str(), 
        "1",
        0
    };

    pid_t pid  = fork();

    if (-1 == pid)
    {
        LOGERR("Fork failed for %s", argArray[2]);
    }
    else if (0 == pid)
    {
        if (execve(argArray[0], (char **)argArray, environ) == -1)
        {
            LOGERR("Execve failed: %s", strerror(errno));
            _Exit(127);
        }
    }

    LOGINFO("Started %d process with %s", pid, argArray[1]);

    return pid;
}


} // namespace UploadLogs
} // namespace Plugin
} // namespace WPEFramework
