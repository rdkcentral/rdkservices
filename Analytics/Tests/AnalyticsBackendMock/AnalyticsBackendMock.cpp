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

#include "IAnalyticsBackend.h"
#include "UtilsLogging.h"

#include <plugins/JSONRPC.h>
#include <vector>
#include <curl/curl.h>
#include <string>
#include <list>
#include <memory>

#ifndef SERVER_URL
#define SERVER_URL "http://localhost:12345"
#endif // SERVER_URL

namespace WPEFramework {
namespace Plugin {

    #define TABLE_NAME      "events"
    #define TABLE_LIMIT     1000

    static size_t CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        ((std::string *)userp)->append((char *)contents, size * nmemb);
        return size * nmemb;
    }

    class AnalyticsBackendMock : public IAnalyticsBackend {
    public:
        AnalyticsBackendMock(): mStore(nullptr), mSysTime(nullptr) {
            LOGINFO("AnalyticsBackendMock created");
        }
        ~AnalyticsBackendMock() override = default;

        const std::string& Name() const override {
            static std::string name = "AnalyticsBackendMock";
            return name;
        }

        uint32_t Configure(PluginHost::IShell* shell, ISystemTimePtr sysTime, ILocalStorePtr store) override {
            if (store)
            {
                mStore = store;
                if (!mStore->Open("/tmp/AnalyticsStore")) {
                    LOGERR("Failed to open local store");
                    return Core::ERROR_OPENING_FAILED;
                }
                if (!mStore->CreateTable(TABLE_NAME)) {
                    LOGERR("Failed to create events table in local store");
                    return Core::ERROR_GENERAL;
                }
                if (!mStore->SetLimit(TABLE_NAME, TABLE_LIMIT)) {
                    LOGERR("Failed to set limit for events table in local store");
                    return Core::ERROR_GENERAL;
                }
            } else {
                return Core::ERROR_BAD_REQUEST;
            }

            mSysTime = sysTime;
            return Core::ERROR_NONE;
        }

        uint32_t SendEvent(const Event& event) override {
            // Mock implementation for testing purposes
            // prepare json object from event
            JsonObject eventJson;
            eventJson["eventName"] = event.eventName;
            eventJson["eventVersion"] = event.eventVersion;
            eventJson["eventSource"] = event.eventSource;
            eventJson["eventSourceVersion"] = event.eventSourceVersion;
            JsonArray cetList;
            for (const auto& cet : event.cetList) {
                cetList.Add(cet);
            }
            eventJson["cetList"] = cetList;
            eventJson["epochTimestamp"] = event.epochTimestamp;
            eventJson["appId"] = event.appId;
            eventJson["eventPayload"] = event.eventPayload;

            // If system time is available, get the timezone offset
            if (mSysTime && mSysTime->IsSystemTimeAvailable()) {
                int32_t offsetSec = 0;
                auto accuracy = mSysTime->GetTimeZoneOffset(offsetSec);
                if (accuracy == ISystemTime::FINAL) {
                    // Here you could add logic to handle the final timezone offset
                    eventJson["timeZoneOffset"] = offsetSec*1000;
                }
            }

            // Add the event to the local store
            std::string entry;
            eventJson.ToString(entry);
            if (mStore && !mStore->AddEntry(TABLE_NAME, entry)) {
                LOGERR("Failed to add event to local store");
                return Core::ERROR_GENERAL;
            }

            std::string json;

            // get all events from store, should be only one in this mock implementation
            if (mStore) {
                uint32_t startIndex = 0;
                uint32_t eventCount = 0;

                std::tie(startIndex, eventCount) = mStore->GetEntriesCount(TABLE_NAME, 0, TABLE_LIMIT);

                std::vector<std::string> eventsString;
                eventsString = mStore->GetEntries(TABLE_NAME, startIndex, eventCount);
                if (eventsString.empty()) {
                    LOGERR("No events found in local store");
                    return Core::ERROR_GENERAL;
                }

                // Clear the store after sending
                if (mStore->RemoveEntries(TABLE_NAME, 0, startIndex+eventCount)) {
                    LOGINFO("Removed %d events from local store", eventCount);
                    // Verify that the store is empty after removal
                    std::tie(startIndex, eventCount) = mStore->GetEntriesCount(TABLE_NAME, 0, TABLE_LIMIT);
                    if (eventCount != 0)
                    {
                        LOGERR("There are still %d events left in local store after removal, start %d", eventCount);
                        return Core::ERROR_GENERAL;
                    }

                } else {
                    LOGERR("Failed to remove events from local store");
                    return Core::ERROR_GENERAL;
                }

                JsonArray eventArray = JsonArray();
                for (const auto& entry : eventsString) {
                    JsonObject eventEntry;
                    if (eventEntry.FromString(entry)) {
                        eventArray.Add(eventEntry);
                    } else {
                        LOGERR("Failed to parse event entry: %s", entry.c_str());
                        return Core::ERROR_GENERAL;
                    }
                }

                eventArray.ToString(json);
            }

            if (json.empty()) {
                LOGERR("No events to send");
                return Core::ERROR_GENERAL;
            }

            // Send with curl
            CURL *curl;
            CURLcode res;
            long retHttpCode = 0;
            std::string url = SERVER_URL;
            std::string response;

            curl = curl_easy_init();
            if (!curl)
            {
                LOGERR("Failed to initialize curl");
                return Core::ERROR_GENERAL;
            }

            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.data());

            // Create a linked list of custom headers
            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");

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

            if (retHttpCode != 200 && retHttpCode != 400) {
                LOGERR("Failed to post analytics event - respcode: %ld, response: %s", retHttpCode, response.c_str());
                return Core::ERROR_GENERAL;
            }
            
            return Core::ERROR_NONE;
        }

    private:
        ILocalStorePtr mStore;
        ISystemTimePtr mSysTime;
    };
} // namespace Plugin
} // namespace WPEFramework

extern "C" {

    WPEFramework::Plugin::IAnalyticsBackend* Create()
    {
        return new WPEFramework::Plugin::AnalyticsBackendMock();
    }

}