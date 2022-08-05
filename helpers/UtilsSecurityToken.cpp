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

#include "UtilsSecurityToken.h"

#include <mutex>
#include <curl/curl.h>
#include <securityagent/SecurityTokenUtil.h>

#include "UtilsCStr.h"
#include "UtilsLogging.h"
#include "UtilsgetRFCConfig.h"

#define MAX_STRING_LENGTH 2048

#define SERVER_DETAILS  "127.0.0.1:9998"

using namespace WPEFramework;
using namespace std;
using namespace WPEFramework;

void Utils::SecurityToken::getSecurityToken(std::string& token)
{
    static std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);

    if(m_sThunderSecurityChecked)
    {
        token = m_sToken;
        return;
    }

    // Thunder Security is enabled by Default.
    bool thunderSecurityRFCEnabled = true;
    RFC_ParamData_t param;
    if (getRFCConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.ThunderSecurity.Enable", param))
    {
        if (param.type == WDMP_BOOLEAN && (strncasecmp(param.value,"false",5) == 0))
        {
            thunderSecurityRFCEnabled = false;
        }
    }
    std::cout << "Thunder Security RFC enabled: " << thunderSecurityRFCEnabled << std::endl;
    if(!isThunderSecurityConfigured() || !thunderSecurityRFCEnabled)
    {
        m_sThunderSecurityChecked = true;
        std::cout << "Thunder Security is not enabled. Not getting token\n";
        return;
    }
    m_sThunderSecurityChecked = true;
    unsigned char buffer[MAX_STRING_LENGTH] = {0};

    int ret = GetSecurityToken(MAX_STRING_LENGTH,buffer);
    if(ret < 0)
    {
        std::cout << "Error in getting token\n";
    }
    else
    {
        std::cout << "retrieved token successfully\n";
        token = (char*)buffer;
        m_sToken = token;
    }
}

static size_t writeCurlResponse(void *ptr, size_t size, size_t nmemb, string stream)
{
    size_t realsize = size * nmemb;
    string temp(static_cast<const char*>(ptr), realsize);
    stream.append(temp);
    return realsize;
}

bool Utils::SecurityToken::isThunderSecurityConfigured()
{
    bool configured = false;
    long http_code = 0;
    std::string jsonResp;
    CURL *curl_handle = NULL;
    CURLcode res = CURLE_OK;
    curl_handle = curl_easy_init();
    string serialNumber = "";
    string url = "http://127.0.0.1:9998/Service/Controller/Configuration/Controller";
    if (curl_handle &&
        !curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str()) &&
        !curl_easy_setopt(curl_handle, CURLOPT_HTTPGET,1) &&
        !curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1) && //when redirected, follow the redirections
        !curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeCurlResponse) &&
        !curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &jsonResp)) {

        res = curl_easy_perform(curl_handle);
        if(curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code) != CURLE_OK)
        {
            std::cout << "curl_easy_getinfo failed\n";
        }
        std::cout << "Thunder Controller Configuration ret: " << res << " http response code: " << http_code << std::endl;
        curl_easy_cleanup(curl_handle);
    }
    else
    {
        std::cout << "Could not perform curl to read Thunder Controller Configuration\n";
    }
    if ((res == CURLE_OK) && (http_code == 200))
    {
        //check for "Security" in response
        JsonObject responseJson = JsonObject(jsonResp);
        if (responseJson.HasLabel("subsystems"))
        {
            const JsonArray subsystemList = responseJson["subsystems"].Array();
            for (int i=0; i<subsystemList.Length(); i++)
            {
                string subsystem = subsystemList[i].String();
                if (subsystem == "Security")
                {
                    configured = true;
                    break;
                }
            }
        }
    }
    return configured;
}

// Thunder plugins communication
std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > Utils::getThunderControllerClient(std::string callsign)
{
    string token;
    Utils::SecurityToken::getSecurityToken(token);
    string query = "token=" + token;

    Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));
    static std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> > thunderClient = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >(callsign.c_str(), "",false,query);
    return thunderClient;
}

void Utils::activatePlugin(const char* callSign)
{
    JsonObject joParams;
    joParams.Set("callsign",callSign);
    JsonObject joResult;

    if(!isPluginActivated(callSign))
    {
        LOGINFO("Activating %s", callSign);
        uint32_t status = getThunderControllerClient()->Invoke<JsonObject, JsonObject>(2000, "activate", joParams, joResult);
        string strParams;
        string strResult;
        joParams.ToString(strParams);
        joResult.ToString(strResult);
        LOGINFO("Called method %s, with params %s, status: %d, result: %s"
                , "activate"
                , C_STR(strParams)
                , status
                , C_STR(strResult));
        if (status == Core::ERROR_NONE)
        {
            LOGINFO("%s Plugin activation status ret: %d ", callSign, status);
        }
    }
}

bool Utils::isPluginActivated(const char* callSign)
{
    string method = "status@" + string(callSign);
    Core::JSON::ArrayType<PluginHost::MetaData::Service> joResult;
    uint32_t status = getThunderControllerClient()->Get<Core::JSON::ArrayType<PluginHost::MetaData::Service> >(2000, method.c_str(),joResult);
    bool pluginActivated = false;
    if (status == Core::ERROR_NONE)
    {
        LOGINFO("Getting status for callSign %s, result: %s", callSign, joResult[0].JSONState.Data().c_str());
        pluginActivated = joResult[0].JSONState == PluginHost::IShell::ACTIVATED;
    }
    else
    {
        LOGWARN("Getting status for callSign %s, status: %d", callSign, status);
    }

    if(!pluginActivated){
        LOGWARN("Plugin %s is not active", callSign);
    } else {
        LOGINFO("Plugin %s is active ", callSign);
    }
    return pluginActivated;
}

std::string Utils::SecurityToken::m_sToken = "";
bool Utils::SecurityToken::m_sThunderSecurityChecked = false;
