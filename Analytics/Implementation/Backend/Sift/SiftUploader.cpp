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

#include "SiftUploader.h"
#include "UtilsLogging.h"
#include "../../../Module.h"

#include <vector>
#include <curl/curl.h>
#include <random>

namespace WPEFramework
{
    namespace Plugin
    {
        SiftUploader::SiftUploader(SiftStorePtr storePtr,
                                   const std::string &url,
                                   const uint32_t &maxRandomisationWindowTime,
                                   const uint32_t &maxEventsInPost,
                                   const uint32_t &maxRetries,
                                   const uint32_t &minRetryPeriod,
                                   const uint32_t &maxRetryPeriod,
                                   const uint32_t &exponentialPeriodicFactor)
            : mStorePtr(storePtr)
            , mUrl(url)
            , mMaxRandomisationWindowTime(maxRandomisationWindowTime)
            , mMaxEventsInPost(maxEventsInPost)
            , mMaxRetries(maxRetries)
            , mMinRetryPeriod(minRetryPeriod)
            , mMaxRetryPeriod(maxRetryPeriod)
            , mExponentialPeriodicFactor(exponentialPeriodicFactor)
            , mUploaderState(UploaderState::RANDOMISATION_WINDOW_WAIT_STATE)
            , mStop(false)
            , mCurrentRetryCount(0)
            , mEventStartIndex(0)
        {
            mThread = std::thread(&SiftUploader::Run, this);
        }

        SiftUploader::~SiftUploader()
        {
            LOGINFO("SiftUploader::~SiftUploader");
            {
                std::lock_guard<std::mutex> lock(mMutex);
                mStop = true;
            }
            mCondition.notify_one();
            mThread.join();
        }

        void SiftUploader::setDeviceInfoRequiredFields(const std::string &accountId, const std::string &deviceId, const std::string &partnerId)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mAccountId = accountId;
            mDeviceId = deviceId;
            mPartnerId = partnerId;
        }

        void SiftUploader::Run()
        {
            while (true)
            {
                static std::vector<std::string> collectedEvents;

                switch (mUploaderState)
                {
                case SiftUploader::UploaderState::RANDOMISATION_WINDOW_WAIT_STATE:
                {
                    std::unique_lock<std::mutex> lock( mMutex );
                    if (mStop)
                    {
                        break;
                    }
                    mCondition.wait_for(lock, std::chrono::seconds(RandomisationWindowTimeGenerator()),
                             [this] () { return mStop; } );

                    mUploaderState = SiftUploader::UploaderState::COLLECT_ANALYTICS;
                }
                break;

                case SiftUploader::UploaderState::COLLECT_ANALYTICS:
                {
                    bool eventsCollected = false;

                    if (CollectEventsFromAnalyticsStore(mMaxEventsInPost))
                    {
                        collectedEvents.clear(); // ensure previous events are cleared

                        if (!mEvents.empty())
                        {   
                            collectedEvents = mEvents;
                            eventsCollected = true;
                        }
                        else
                        {
                            LOGERR("No collected events to be got");
                        }
                    }

                    if (eventsCollected)
                    {
                        LOGINFO("Successfully collected events from analytics Store");
                        mUploaderState = UploaderState::POST_ANALYTICS;
                        // Falling through to the POST_ANALYTICS case as we have successfully collected events
                    }
                    else
                    {
                        // No events to be sent, so go back to the randomisation window wait state
                        mUploaderState = UploaderState::RANDOMISATION_WINDOW_WAIT_STATE;
                        break;
                    }
                }
                    // NO BREAK HERE, FALLING THROUGH TO THE NEXT CASE (WHICH SHOULD BE POST_ANALYTICS)

                case SiftUploader::UploaderState::POST_ANALYTICS:
                {
                    std::string jsonEventPayload = ComposeJSONEventArrayToBeUploaded(collectedEvents);

                    std::string resp;

                    long respcode;

                    LOGINFO("Posting analytics events: %s", jsonEventPayload.c_str());

                    do
                    {
                        respcode = PostJson(mUrl, jsonEventPayload, resp);
                    } while ((respcode != 200) && (respcode != 400) && PerformWaitIfRetryNeeded());

                    if ((respcode == 200) || (respcode == 400))
                    {
                        if (respcode == 400)
                        {
                            LOGWARN("Received a 400 response - deleting the events as the end point refuses them");
                        }

                        if (!mEvents.empty() && mStorePtr->RemoveEvents(mEventStartIndex, mEventStartIndex + mEvents.size() - 1))
                        {
                            LOGINFO("Collected events successfully deleted");
                        }
                        else
                        {
                            LOGERR("No collected events to be deleted");
                        }
                    }
                    else
                    {
                        LOGERR("Failed to post analytics event - respcode: %ld, response: %s", respcode, resp.c_str());
                    }

                    if (!resp.empty())
                    {
                        validateResponse(resp, collectedEvents);
                    }

                    mUploaderState = UploaderState::RANDOMISATION_WINDOW_WAIT_STATE;
                }
                break;

                default:
                {
                    LOGERR("Unhandled state: %d", static_cast<int>(mUploaderState));
                }
                break;
                }

                std::unique_lock<std::mutex> lock( mMutex );
                if (mStop)
                {
                    LOGINFO("SiftUploader Run exit");
                    return;
                }
            }
        }

        bool SiftUploader::PerformWaitIfRetryNeeded()
        {
            bool retry = false;

            if (mCurrentRetryCount >= mMaxRetries)
            {
                mCurrentRetryCount = 0;
            }
            else
            {
                static auto retryTime = mMinRetryPeriod;

                if (retryTime >= mMaxRetryPeriod)
                {
                    retryTime = mMinRetryPeriod;
                }

                LOGINFO("Failed posting retry wait time:  %d seconds, with retries completed: %d", retryTime, mCurrentRetryCount);

                std::unique_lock<std::mutex> lock( mMutex );
                if (mStop)
                {
                    // return immediately if stop is set
                    return false;
                }
                mCondition.wait_for(lock, std::chrono::seconds(retryTime),
                             [this] () { return mStop; } );

                if (retryTime < mMaxRetryPeriod)
                {
                    retryTime *= mExponentialPeriodicFactor;
                }

                ++mCurrentRetryCount;

                retry = !mStop;
            }

            return retry;
        }

        uint32_t SiftUploader::RandomisationWindowTimeGenerator() const
        {
            uint32_t max = mMaxRandomisationWindowTime;
            uint32_t min = 0;

            srand(time(nullptr));

            return (((rand() % (max + 1 - min)) + min));
        }

        bool SiftUploader::CollectEventsFromAnalyticsStore(uint32_t count)
        {
            bool success = false;

            uint32_t startIndex{};
            uint32_t eventCount{};

            std::tie(startIndex, eventCount) = mStorePtr->GetEventCount();

            // if count is specified in the call, then only those number of events are desired even if more events are available
            if ((count > 0) && (eventCount > count))
            {
                eventCount = count;
            }

            if (eventCount > 0)
            {
                mEvents = mStorePtr->GetEvents(startIndex, eventCount);

                if (!mEvents.empty())
                {
                    LOGINFO("Successfully got %zu events from analytics store", mEvents.size());
                    mEventStartIndex = startIndex;
                    success = true;
                }
                else
                {
                    LOGINFO("Got no events from the analytics store");
                }
            }

            return success;
        }

        std::string SiftUploader::ComposeJSONEventArrayToBeUploaded(const std::vector<std::string> &events) const
        {
            std::string output;

            auto validateEventLambda = [](const JsonObject &event)
            {
                // Just perform some basic sanity to check if the event is valid. If event_id is present,
                // the event is bound to be valid (since all the other attributes are populated together).
                // Otherwise, it is invalid/malformed and can be dropped since it would be rejected by the backend anyway
                return (event.HasLabel("event_id") && event["event_id"].Content() == WPEFramework::Core::JSON::Variant::type::STRING && !event["event_id"].String().empty());
            };

            // check if there are any events in the first place, if not just return empty string
            if (!events.empty())
            {
                JsonArray eventArray = JsonArray();

                for (const auto &event : events)
                {
                    JsonObject root(event);
                    if (validateEventLambda(root))
                    {
                        updateEventDeviceInfoIfRequired(root);
                        eventArray.Add(root);
                    }
                    else
                    {
                        LOGWARN("Dropping an invalid/malformed event since it would be rejected by the backend anyway");
                    }
                }

                eventArray.ToString(output);
            }
            return output;
        }

        void SiftUploader::updateEventDeviceInfoIfRequired(JsonObject &event) const
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if (!mAccountId.empty() && !mDeviceId.empty() && !mPartnerId.empty()
                && (!event.HasLabel("account_id") || !event.HasLabel("device_id") || !event.HasLabel("partner_id")))
            {
                event["account_id"] = mAccountId;
                event["device_id"] = mDeviceId;
                event["partner_id"] = mPartnerId;
            }
        }

        void SiftUploader::validateResponse(const std::string &response, const std::vector<std::string> &events) const
        {
            JsonObject responseJson(response);

            if (!responseJson.HasLabel("Events") && responseJson["Events"].Content() != WPEFramework::Core::JSON::Variant::type::ARRAY)
            {
                LOGERR("Response does not contain Events array");
                return;
            }

            JsonArray eventsArray(responseJson["Events"].Array());

            // go over events event_id and find out which ones were rejected
            for (const auto &event : events)
            {
                JsonObject eventJson(event);
                const std::string &eventId = eventJson["event_id"].String();

                bool found = false;
                for (int i = 0; i < eventsArray.Length(); i++)
                {
                    JsonObject responseEvent(eventsArray[i].Object());
                    if (responseEvent.HasLabel("EventId"))
                    {
                        if (responseEvent["EventId"].String() == eventId)
                        {
                            found = true;
                            if (responseEvent.HasLabel("Status") && responseEvent["Status"].String() != "valid")
                            {
                                LOGERR("Event was rejected by the backend: %s", event.c_str());
                                if (responseEvent.HasLabel("Errors"))
                                {
                                    std::string errors;
                                    responseEvent.ToString(errors);
                                    LOGERR("Backend response for rejected event: %s", errors.c_str());
                                }
                            }
                            break;
                        }
                    }
                }

                if (!found)
                {
                    LOGERR("Event Id '%s'  was not found in the response", eventId.c_str());
                }
            }
        }

        static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }

        long SiftUploader::PostJson(const std::string &url, const std::string &json, std::string &response)
        {
            CURL *curl;
            CURLcode res;
            long retHttpCode = 0;

            if (url.empty() || json.empty())
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

            return retHttpCode;
        }

    }
}