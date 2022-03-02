#include "Utils.h"

#include <curl/curl.h>
#include <iostream>
#include <mutex>
#include <plugins/plugins.h>
#include <string>

namespace Utils {

struct RFC_ParamData_t {
    std::string type;
    std::string value;
};

#define WDMP_BOOLEAN "WDMP_BOOLEAN"
#define MAX_STRING_LENGTH 255

int GetSecurityToken(unsigned short, unsigned char*) {
    return 1;
}

bool getRFCConfig(char* paramName, RFC_ParamData_t& paramOutput) {
    paramOutput.type = WDMP_BOOLEAN;
    paramOutput.value = "true";

    return true;
}

template<typename CharType> inline CharType toASCIIUpper(CharType c)
{
    return c & ~((c >= 'a' && c <= 'z') << 5);
}

int strncasecmp(const char* s1, const char* s2, size_t len)
{
    while (len > 0 && toASCIIUpper(*s1) == toASCIIUpper(*s2)) {
        if (*s1 == '\0')
            return 0;
        s1++;
        s2++;
        len--;
    }

    if (!len)
        return 0;

    return toASCIIUpper(*s1) - toASCIIUpper(*s2);
}

void SecurityToken::getSecurityToken(std::string& token)
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
        if (param.type == WDMP_BOOLEAN && (strncasecmp(param.value.data(),"false",5) == 0))
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

static size_t writeCurlResponse(void *ptr, size_t size, size_t nmemb, std::string stream)
{
    size_t realsize = size * nmemb;
    std::string temp(static_cast<const char*>(ptr), realsize);
    stream.append(temp);
    return realsize;
}

bool SecurityToken::isThunderSecurityConfigured()
{
    bool configured = false;
    long http_code = 0;
    std::string jsonResp;
    CURL *curl_handle = NULL;
    CURLcode res = CURLE_OK;
    curl_handle = curl_easy_init();
    std::string serialNumber = "";
    std::string url = "http://127.0.0.1:9998/Service/Controller/Configuration/Controller";
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
                std::string subsystem = subsystemList[i].String();
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

} // namespace Utils