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
#include "utils.h"

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
        removeCharsFromString(mac, "\n\r: ");
        if (mac.empty())
            ret = FilenameFail;
        else
            filename = mac + "_Logs_" + currentDateTimeUtc("+%m-%d-%y-%I-%M%p") + ".tgz";
        return ret;
    }

    size_t ssrRead(char *data, size_t size, size_t nitems, void *userdata)
    {
        return ((std::stringstream *)userdata)->readsome(data, size * nitems);
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
        CURLcode res;
        long http_code = 0;
        std::stringstream read;
        string data = "filename=" + filename;
        read << data;
        std::stringstream write;

        curl = curl_easy_init();
        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(curl, CURLOPT_URL, C_STR(ssrUrl));
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, ssrRead);
            curl_easy_setopt(curl, CURLOPT_READDATA, (void *)&read);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)data.size());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ssrWrite);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&write);

            LOGINFO("curl request to: %s", C_STR(ssrUrl));
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
        CURLcode res;
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

} // namespace UploadLogs
} // namespace Plugin
} // namespace WPEFramework
