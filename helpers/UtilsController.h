#pragma once

#include <mutex>
#include <curl/curl.h>

#ifndef DISABLE_SECURITY_TOKEN
#include <securityagent/SecurityTokenUtil.h>
#endif

// std
#include <string>

#define MAX_STRING_LENGTH 2048

#define SERVER_DETAILS  "127.0.0.1:9998"

using namespace WPEFramework;
using namespace std;

namespace Utils
{
    struct SecurityToken
    {
        static void getSecurityToken(std::string &token)
        {
            static std::string sToken = "";
            static bool sThunderSecurityChecked = false;

            static std::mutex mtx;
            std::unique_lock<std::mutex> lock(mtx);

            if (sThunderSecurityChecked)
            {
                token = sToken;
                return;
            }

            sThunderSecurityChecked = true;

#ifdef DISABLE_SECURITY_TOKEN
            token = sToken;
#else
            if (!isThunderSecurityConfigured())
            {
                LOGINFO("Thunder Security is not enabled. Not getting token");
                token = sToken;
                return;
            }

            unsigned char buffer[MAX_STRING_LENGTH] = {0};
            int ret = GetSecurityToken(MAX_STRING_LENGTH, buffer);
            if (ret < 0)
            {
                LOGERR("Error in getting token");
            }
            else
            {
                LOGINFO("Retrieved token successfully");
                token = (char *)buffer;
                sToken = token;
            }
#endif
        }

#ifndef DISABLE_SECURITY_TOKEN
        static size_t writeCurlResponse(void *ptr, size_t size, size_t nmemb, string stream)
        {
            size_t realsize = size * nmemb;
            string temp(static_cast<const char *>(ptr), realsize);
            stream.append(temp);
            return realsize;
        }

        static bool isThunderSecurityConfigured()
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
                !curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1) &&
                !curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1) && // when redirected, follow the redirections
                !curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeCurlResponse) &&
                !curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &jsonResp))
            {

                res = curl_easy_perform(curl_handle);
                if (curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code) != CURLE_OK)
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
                // check for "Security" in response
                JsonObject responseJson = JsonObject(jsonResp);
                if (responseJson.HasLabel("subsystems"))
                {
                    const JsonArray subsystemList = responseJson["subsystems"].Array();
                    for (int i = 0; i < subsystemList.Length(); i++)
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
#endif
  
    };

    // Thunder Plugin Communication
    std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> getThunderControllerClient(std::string callsign="")
    {

        string token;
        Utils::SecurityToken::getSecurityToken(token);
        string query = "token=" + token;

        Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(SERVER_DETAILS)));
        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> thunderClient = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>(callsign.c_str(), "", false, query);

        return thunderClient;
    }

#ifndef USE_THUNDER_R4
    class Job : public Core::IDispatchType<void>
#else
    class Job : public Core::IDispatch
#endif /* USE_THUNDER_R4 */
    {
    public:
        Job(std::function<void()> work)
            : _work(work)
        {
        }
        void Dispatch() override
        {
            _work();
        }

    private:
        std::function<void()> _work;
    };

    uint32_t getServiceState(PluginHost::IShell *shell, const string &callsign, PluginHost::IShell::state &state)
    {
        uint32_t result;
        auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
        if (interface == nullptr)
        {
            result = Core::ERROR_UNAVAILABLE;
            std::cout << "no IShell for " << callsign << std::endl;
        }
        else
        {
            result = Core::ERROR_NONE;
            state = interface->State();
            std::cout << "IShell state " << state << " for " << callsign << std::endl;
            interface->Release();
        }
        return result;
    }

    uint32_t activatePlugin(PluginHost::IShell *shell, const string &callsign)
    {
        uint32_t result = Core::ERROR_ASYNC_FAILED;
        Core::Event event(false, true);

#ifndef USE_THUNDER_R4
        Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatchType<void>>(Core::ProxyType<Job>::Create([&]()
                                                                                                                     {
#else
        Core::IWorkerPool::Instance().Submit(Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create([&]()
                                                                                                           {
#endif /* USE_THUNDER_R4 */
                    auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                    if (interface == nullptr) {
                        result = Core::ERROR_UNAVAILABLE;
                        std::cout << "no IShell for " << callsign << std::endl;
                    } else {
                        result = interface->Activate(PluginHost::IShell::reason::REQUESTED);
                        std::cout << "IShell activate status " << result << " for " << callsign << std::endl;
                        interface->Release();
                    }
                    event.SetEvent(); })));

        event.Lock();
        return result;
    }

}
