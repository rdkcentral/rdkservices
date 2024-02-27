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
namespace
{
    const string DEFAULT_SSR_URL = "https://ssr.ccp.xcal.tv/cgi-bin/rdkb_snmp.cgi";

    err_t getFilename(string& filename)
    {
        err_t ret = OK;
        string mac = Utils::cRunScript(". /lib/rdk/utils.sh && getMacAddressOnly");
        removeCharsFromString(mac, "\n\r ");
        if (mac.empty())
            ret = FilenameFail;
        else
            filename = mac + "_Logs_" + currentDateTimeUtc("%m-%d-%y-%I-%M%p") + ".tgz";
        return ret;
    }

    size_t ssrWrite(char *data, size_t size, size_t nitems, void *userdata)
    {
        size_t len = size * nitems;
        ((std::stringstream *)userdata)->write(data, len);
        return len;
    }

    err_t acquireUploadUrl(const string& ssrUrl, const string& filename, string& uploadUrl)
    {
        err_t ret = OK;

        CURL *curl;
        CURLcode res = CURLE_UPLOAD_FAILED;
        long http_code = 0;
        string data = "filename=" + filename;
        std::stringstream write;

        curl = curl_easy_init();
        if (curl)
        {
//            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(curl, CURLOPT_URL, C_STR(ssrUrl));
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ssrWrite);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&write);

            LOGINFO("curl request to: %s with data: %s", C_STR(ssrUrl), data.c_str());
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
                LOGERR("curl_easy_perform() failed: %s", curl_easy_strerror(res));

            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            LOGINFO("curl response code: %ld", http_code);

            curl_easy_cleanup(curl);
        }

        if (res != CURLE_OK || http_code != 200)
            ret = SsrFail;
        else
        {
            uploadUrl = write.str();
            LOGINFO("curl response: %s", C_STR(uploadUrl));
        }

        return ret;
    }

    err_t archiveLogs(const string& filename, string& path)
    {
        err_t ret = OK;

        string tmp = "/tmp/" + filename;
        string cmd = "tar -C /opt/logs -zcf " + tmp + " ./";

        Utils::cRunScript(C_STR(cmd));
        if (!Utils::fileExists(C_STR(tmp)))
            ret = TarFail;
        else
            path = tmp;

        return ret;
    }

    size_t uploadRead(void *data, size_t size, size_t nitems, void *userdata)
    {
        return fread(data, size, nitems, (FILE *)userdata);
    }

    err_t uploadLogs(const string& path, const string& uploadUrl)
    {
        err_t ret = OK;

        CURL *curl;
        CURLcode res = CURLE_UPLOAD_FAILED;
        long http_code = 0;
        FILE *fd;
        struct stat file_info;
        stat(C_STR(path), &file_info);
        fd = fopen(C_STR(path), "rb");

        curl = curl_easy_init();
        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, uploadRead);
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(curl, CURLOPT_PUT, 1L);
            curl_easy_setopt(curl, CURLOPT_URL, C_STR(uploadUrl));
            curl_easy_setopt(curl, CURLOPT_READDATA, fd);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60L);

            LOGINFO("curl request to: %s", C_STR(uploadUrl));
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
                LOGERR("curl_easy_perform() failed: %s", curl_easy_strerror(res));

            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            LOGINFO("curl response code: %ld", http_code);

            curl_easy_cleanup(curl);
        }
        fclose(fd);

        if (res != CURLE_OK || http_code != 200)
            ret = UploadFail;

        return ret;
    }
} // namespace

// similar to /lib/rdk/UploadLogsNow.sh
err_t upload(const std::string& ssrUrl)
{
    err_t ret = OK;

    string ssr = ssrUrl;
    if (ssr.empty())
        ssr = DEFAULT_SSR_URL;
    if (ssr.rfind("https://", 0) != 0)
        ret = BadUrl;

    string filename;
    if (ret == OK)
    {
        LOGINFO("ssr: %s", C_STR(ssr));
        ret = getFilename(filename);
    }

    string uploadUrl;
    if (ret == OK)
    {
        LOGINFO("filename: %s", C_STR(filename));
        ret = acquireUploadUrl(ssr, filename, uploadUrl);
    }

    string path;
    if (ret == OK)
    {
        LOGINFO("uploadUrl: %s", C_STR(uploadUrl));
        ret = archiveLogs(filename, path);
    }

    if (ret == OK)
    {
        LOGINFO("path: %s", C_STR(path));
        ret = uploadLogs(path, uploadUrl);

        int removeStatus = remove(C_STR(path));
        LOGINFO("remove %s exit code %d", C_STR(path), removeStatus);
    }

    return ret;
}

std::string errToText(err_t err)
{
    static std::map<err_t, string> _map =
    {
            {OK, "OK"},
            {BadUrl, "invalid or insecure input url"},
            {FilenameFail, "can't generate logs filename"},
            {SsrFail, "ssr fail"},
            {TarFail, "tar fail"},
            {UploadFail, "upload fail"},
    };

    auto it = _map.find(err);
    if (it != _map.end())
        return it->second;
    return "";
}

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
            mTlsLogUpload = "true";
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
