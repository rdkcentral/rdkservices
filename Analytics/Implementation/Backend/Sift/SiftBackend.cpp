/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */
#include "SiftBackend.h"
#include "UtilsLogging.h"

#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <memory>

#include <curl/curl.h>

namespace WPEFramework
{
    namespace Plugin
    {

        const uint32_t POPULATE_CONFIG_TIMEOUT_MS = 3000;

        SiftBackend::SiftBackend()
            : mQueueMutex()
            , mQueueCondition()
            , mActionQueue()
            , mShell(nullptr)
            , mConfigPtr(nullptr)

        {
            mThread = std::thread(&SiftBackend::ActionLoop, this);
            mThread.detach();
        }

        SiftBackend::~SiftBackend()
        {
            Action action = {ACTION_TYPE_SHUTDOWN, nullptr};
            {
                std::lock_guard<std::mutex> lock(mQueueMutex);
                mActionQueue.push(action);
            }
            mQueueCondition.notify_one();
        }

        /* virtual */ uint32_t SiftBackend::Configure(PluginHost::IShell *shell)
        {
            ASSERT(shell != nullptr);
            std::unique_lock<std::mutex> lock(mQueueMutex);
            mShell = shell;
            mConfigPtr = std::unique_ptr<SiftConfig>(new SiftConfig(shell));
            return Core::ERROR_NONE;
        }

        /* virtual */ uint32_t SiftBackend::SendEvent(const Event &event)
        {
            Action action;
            action.type = ACTION_TYPE_SEND_EVENT;
            action.payload = std::make_shared<Event>(event);
            std::unique_lock<std::mutex> lock(mQueueMutex);
            mActionQueue.push(action);
            lock.unlock();
            mQueueCondition.notify_one();
            return Core::ERROR_NONE;
        }

        void SiftBackend::ActionLoop()
        {
            std::unique_lock<std::mutex> lock(mQueueMutex);
            bool configValid = false;
            while (true)
            {
                std::chrono::milliseconds queueTimeout(std::chrono::milliseconds::max());
                SiftConfig::Config config;

                if (!configValid) 
                {
                    queueTimeout = std::chrono::milliseconds(POPULATE_CONFIG_TIMEOUT_MS);
                }

                if (queueTimeout == std::chrono::milliseconds::max())
                {
                    mQueueCondition.wait(lock, [this] { return !mActionQueue.empty(); });
                }
                else
                {
                    mQueueCondition.wait_for(lock, queueTimeout, [this] { return !mActionQueue.empty(); });
                }

                Action action = {ACTION_TYPE_UNDEF, nullptr};

                if (mActionQueue.empty())
                {
                    action = {ACTION_TYPE_POPULATE_CONFIG, nullptr};
                }
                else
                {
                    action = mActionQueue.front();
                    mActionQueue.pop();
                }

                //Always get the most recent config
                if (mConfigPtr != nullptr && mConfigPtr->Get(config))
                {
                    configValid = true;
                }
                else
                {
                    configValid = false;
                }
                lock.unlock();

                LOGINFO("Action %d", action.type);

                switch (action.type)
                {
                case ACTION_TYPE_POPULATE_CONFIG:
                    if (configValid)
                    {
                        // Try to send the events from the queue
                        while (!mEventQueue.empty() && SendEventInternal(mEventQueue.front(), config))
                        {
                            mEventQueue.pop();
                        }

                        if (!mEventQueue.empty())
                        {
                            LOGERR("Failed to send all events from queue");
                            configValid = false;
                        }
                    }
                    break;
                case ACTION_TYPE_SEND_EVENT:
                    if (configValid)
                    {
                        SendEventInternal(*action.payload, config);
                    }
                    else
                    {
                        mEventQueue.push(*action.payload);
                    }
                    break;
                case ACTION_TYPE_SHUTDOWN:
                    return;
                default:
                    break;
                }

                lock.lock();
            }
        }

        bool SiftBackend::SendEventInternal(const Event &event, const SiftConfig::Config &config)
        {
            JsonObject eventJson = JsonObject();
            if (config.schema2Enabled)
            {
                // Sift 2.0 schema
                eventJson["common_schema"] = config.commonSchema;
                if (!config.env.empty())
                {
                    eventJson["env"] = config.env;
                }
                eventJson["product_name"] = config.productName;
                eventJson["product_version"] = config.productVersion;
                eventJson["event_schema"] = config.productName + "/" + event.eventName + "/" + event.eventVersion;
                eventJson["event_name"] = event.eventName;
                eventJson["timestamp"] = event.epochTimestamp;
                eventJson["event_id"] = GenerateRandomUUID();
                eventJson["event_source"] = event.eventSource;
                eventJson["event_source_version"] = event.eventSourceVersion;
                if (!event.cetList.empty())
                {
                    JsonArray cetList = JsonArray();
                    for (const std::string &cet : event.cetList)
                    {
                        cetList.Add(cet);
                    }
                    eventJson["cet_list"] = cetList;
                }
                eventJson["logger_name"] = config.loggerName;
                eventJson["logger_version"] = config.loggerVersion;
                eventJson["partner_id"] = config.partnerId;
                if (config.activated)
                {
                    eventJson["xbo_account_id"] = config.xboAccountId;
                    eventJson["xbo_device_id"] = config.xboDeviceId;
                    eventJson["activated"] = config.activated;
                }
                eventJson["device_model"] = config.deviceModel;
                eventJson["device_type"] = config.deviceType;
                eventJson["device_timezone"] = std::stoi(config.deviceTimeZone);
                eventJson["device_os_name"] = config.deviceOsName;
                eventJson["device_os_version"] = config.deviceOsVersion;
                eventJson["platform"] = config.platform;
                eventJson["device_manufacturer"] = config.deviceManufacturer;
                eventJson["authenticated"] = config.authenticated;
                eventJson["session_id"] = config.sessionId;
                eventJson["proposition"] = config.proposition;
                if (!config.retailer.empty())
                {
                    eventJson["retailer"] = config.retailer;
                }
                if (!config.jvAgent.empty())
                {
                    eventJson["jv_agent"] = config.jvAgent;
                }
                if (!config.coam.empty())
                {
                    eventJson["coam"] = JsonValue(config.coam == "true");
                }
                eventJson["device_serial_number"] = config.deviceSerialNumber;
                if (!config.deviceFriendlyName.empty())
                {
                    eventJson["device_friendly_name"] = config.deviceFriendlyName;
                }
                if (!config.deviceMacAddress.empty())
                {
                    eventJson["device_mac_address"] = config.deviceMacAddress;
                }
                if (!config.country.empty())
                {
                    eventJson["country"] = config.country;
                }
                if (!config.region.empty())
                {
                    eventJson["region"] = config.region;
                }
                if (!config.accountType.empty())
                {
                    eventJson["account_type"] = config.accountType;
                }
                if (!config.accountOperator.empty())
                {
                    eventJson["operator"] = config.accountOperator;
                }
                if (!config.accountDetailType.empty())
                {
                    eventJson["account_detail_type"] = config.accountDetailType;
                }

                eventJson["event_payload"] = JsonObject(event.eventPayload);
            }
            else
            {
                //Sift 1.0
                eventJson["event_name"] = event.eventName;
                eventJson["event_schema"] = config.productName + "/" + event.eventName + "/" + event.eventVersion;
                eventJson["event_payload"] = JsonObject(event.eventPayload);
                eventJson["session_id"] = config.sessionId;
                eventJson["event_id"] = GenerateRandomUUID();

                if (!config.accountId.empty() && !config.deviceId.empty() && !config.partnerId.empty())
                {
                    eventJson["account_id"] = config.accountId;
                    eventJson["device_id"] = config.deviceId;
                    eventJson["partner_id"] = config.partnerId;
                }
                else
                {
                    std::cout << "Sift: Account ID, Device ID, or Partner ID is empty for: " << event.eventName << std::endl;
                }

                eventJson["app_name"] = config.deviceAppName;
                eventJson["app_ver"] = config.deviceAppVersion;
                eventJson["device_model"] = config.deviceModel;
                eventJson["device_timezone"] = std::stoi(config.deviceTimeZone);
                eventJson["platform"] = config.platform;
                eventJson["os_ver"] = config.deviceSoftwareVersion;
                eventJson["device_language"] = ""; // Empty for now
                eventJson["timestamp"] = event.epochTimestamp;
                eventJson["device_type"] = config.deviceType;
            }

            // TODO: push to persistent queue instead of sending array
            JsonArray eventArray = JsonArray();
            eventArray.Add(eventJson);

            std::string json;
            eventArray.ToString(json);
            LOGINFO("Sending event: %s", json.c_str());

            // Upload the event to Sift
            uint32_t httpCode = PostJson(config.url, config.apiKey, json);
            if (httpCode == 400)
            {
                LOGINFO("Backend refused data, skipping: %s, HTTP Code: %d", event.eventName.c_str(), httpCode);
                return true;
            }
            if (httpCode == 200)
            {
                LOGINFO("Backend accepted data: %s, HTTP Code: %d", event.eventName.c_str(), httpCode);
                return true;
            }
            LOGERR("Backend failed to accept data: %s, HTTP COde: %d", event.eventName.c_str(), httpCode);
            return false;
        }

        uint8_t SiftBackend::GenerateRandomCharacter()
        {
            static std::random_device randomDevice;
            static std::mt19937 randomNumberGenerator(randomDevice());
            static std::uniform_int_distribution<> distribution(0, 255);

            return static_cast<uint8_t>(distribution(randomNumberGenerator));
        }

        std::string SiftBackend::GenerateRandomOctetString(uint32_t numOctets)
        {
            std::stringstream ss;
            for (uint32_t index = 0; index < numOctets; index++)
            {
                ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(GenerateRandomCharacter());
            }

            return ss.str();
        }

        std::string SiftBackend::GenerateRandomUUID()
        {
            std::stringstream randomUUIDStream;

            // The UUID format is xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx (8-4-4-4-12) where each x is a hexadecimal digit.
            // M indicates the version of the UUID (4 in this case) and the 2 most significant bits of N should be 10 (binary) indicating
            // that it is variant-1 (RFC 4122).

            randomUUIDStream << GenerateRandomOctetString(4);
            randomUUIDStream << ("-" + GenerateRandomOctetString(2));
            randomUUIDStream << std::hex << "-" << static_cast<uint32_t>(0x40 + (GenerateRandomCharacter() & 0x0Fu)) << GenerateRandomOctetString(1);
            randomUUIDStream << std::hex << "-" << static_cast<uint32_t>(0x80 + (GenerateRandomCharacter() & 0x3Fu)) << GenerateRandomOctetString(1);
            randomUUIDStream << ("-" + GenerateRandomOctetString(6));

            return randomUUIDStream.str();
        }

        static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }

        uint32_t SiftBackend::PostJson(const std::string &url, const std::string &apiKey, const std::string &json)
        {
            CURL *curl;
            CURLcode res;
            uint32_t retHttpCode = 0;
            std::string response;

            if (url.empty() || apiKey.empty() || json.empty())
            {
                LOGERR("Invalid parameters for postJson");
                return retHttpCode;
            }

            curl = curl_easy_init();
            if (!curl)
            {
                LOGERR("Failed to initialize curl");
                return retHttpCode;
            }

            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.data());

            // Create a linked list of custom headers
            struct curl_slist *headers = NULL;
            std::string keyHeader("X-Api-Key: " + apiKey);
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, keyHeader.data());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            // Perform the request, res will get the return code
            res = curl_easy_perform(curl);

            // Check for errors
            if (res != CURLE_OK)
            {
                LOGERR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
            }
            else
            {
                LOGINFO("Response: %s", response.c_str());
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retHttpCode);
            }

            // Clean up the header list
            curl_slist_free_all(headers);

            // Clean up curl session
            curl_easy_cleanup(curl);

            return retHttpCode;
        }
    }
}