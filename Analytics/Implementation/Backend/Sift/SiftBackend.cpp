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
            , mStorePtr(nullptr)
            , mUploaderPtr(nullptr)
            , mSessionId()
        {
            mThread = std::thread(&SiftBackend::ActionLoop, this);
        }

        SiftBackend::~SiftBackend()
        {
            LOGINFO("SiftBackend::~SiftBackend");
            Action action = {ACTION_TYPE_SHUTDOWN, nullptr};
            {
                std::lock_guard<std::mutex> lock(mQueueMutex);
                mActionQueue.push(action);
            }
            mQueueCondition.notify_one();
            mThread.join();
        }

        /* virtual */ uint32_t SiftBackend::Configure(PluginHost::IShell *shell, SystemTimePtr sysTime)
        {
            ASSERT(shell != nullptr);
            std::unique_lock<std::mutex> lock(mQueueMutex);
            mShell = shell;
            mConfigPtr = std::unique_ptr<SiftConfig>(new SiftConfig(shell, sysTime));
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

            SiftConfig::StoreConfig storeConfig;
            SiftConfig::UploaderConfig uploaderConfig;

            while (true)
            {
                std::chrono::milliseconds queueTimeout(std::chrono::milliseconds::max());
                SiftConfig::Attributes attributes;

                if (!configValid) 
                {
                    queueTimeout = std::chrono::milliseconds(POPULATE_CONFIG_TIMEOUT_MS);
                }

                if (mActionQueue.empty())
                {
                    if (queueTimeout == std::chrono::milliseconds::max())
                    {
                        mQueueCondition.wait(lock, [this]
                                             { return !mActionQueue.empty(); });
                    }
                    else
                    {
                        mQueueCondition.wait_for(lock, queueTimeout, [this]
                                                 { return !mActionQueue.empty(); });
                    }
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

                //Always get the most recent attributes
                bool attributesValid = false;
                bool storeConfigValid = (mStorePtr != nullptr);
                bool uploaderConfigValid = (mUploaderPtr != nullptr);
                if (mConfigPtr != nullptr && mConfigPtr->GetAttributes(attributes))
                {
                    attributesValid = true;
                }

                if (mConfigPtr != nullptr)
                {
                    if (mStorePtr == nullptr && mConfigPtr->GetStoreConfig(storeConfig))
                    {
                        mStorePtr = std::make_shared<SiftStore>(storeConfig.path,
                            storeConfig.eventsLimit);
                        if (mStorePtr != nullptr)
                        {
                            storeConfigValid = true;
                        }
                    }

                    if (mUploaderPtr == nullptr && mStorePtr != nullptr && mConfigPtr->GetUploaderConfig(uploaderConfig))
                    {
                        mUploaderPtr = std::unique_ptr<SiftUploader>(new SiftUploader(mStorePtr,
                            uploaderConfig.url,
                            uploaderConfig.maxRandomisationWindowTime,
                            uploaderConfig.maxEventsInPost,
                            uploaderConfig.maxRetries,
                            uploaderConfig.minRetryPeriod,
                            uploaderConfig.maxRetryPeriod,
                            uploaderConfig.exponentialPeriodicFactor));
                        if (mUploaderPtr != nullptr)
                        {
                            uploaderConfigValid = true;
                        }
                    }
                }

                if (attributesValid && storeConfigValid && uploaderConfigValid)
                {
                    configValid = true;
                    // For Sift 1.0 update uploader with auth values if avaliable
                    // So they will be added to the events if missing
                    if (!attributes.schema2Enabled && !attributes.xboAccountId.empty() && !attributes.xboDeviceId.empty() && !attributes.partnerId.empty())
                    {
                        mUploaderPtr->setDeviceInfoRequiredFields(attributes.xboAccountId, attributes.xboDeviceId, attributes.partnerId);
                    }
                }
                else
                {
                    configValid = false;
                }

                lock.unlock();

                switch (action.type)
                {
                case ACTION_TYPE_POPULATE_CONFIG:
                    if (configValid)
                    {
                        // Try to send the events from the queue
                        while (!mEventQueue.empty() && SendEventInternal(mEventQueue.front(), attributes))
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
                        SendEventInternal(*action.payload, attributes);
                    }
                    else
                    {
                        mEventQueue.push(*action.payload);
                    }
                    break;
                case ACTION_TYPE_SHUTDOWN:
                    LOGINFO("Shutting down SiftBackend");
                    return;
                default:
                    break;
                }

                lock.lock();
            }
        }

        bool SiftBackend::SendEventInternal(const Event &event, const SiftConfig::Attributes &attributes)
        {
            if (mSessionId.empty())
            {
                mSessionId = GenerateRandomUUID();
            }

            JsonObject eventJson = JsonObject();
            if (attributes.schema2Enabled)
            {
                // Sift 2.0 schema
                eventJson["common_schema"] = attributes.commonSchema;
                if (!attributes.env.empty())
                {
                    eventJson["env"] = attributes.env;
                }
                eventJson["product_name"] = attributes.productName;
                eventJson["product_version"] = attributes.productVersion;
                eventJson["event_schema"] = attributes.productName + "/" + event.eventName + "/" + event.eventVersion;
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
                eventJson["logger_name"] = attributes.loggerName;
                eventJson["logger_version"] = attributes.loggerVersion;
                eventJson["partner_id"] = attributes.partnerId;
                if (attributes.activated)
                {
                    eventJson["xbo_account_id"] = attributes.xboAccountId;
                    eventJson["xbo_device_id"] = attributes.xboDeviceId;
                    eventJson["activated"] = attributes.activated;
                }
                eventJson["device_model"] = attributes.deviceModel;
                eventJson["device_type"] = attributes.deviceType;
                eventJson["device_timezone"] = attributes.deviceTimeZone;
                eventJson["device_os_name"] = attributes.deviceOsName;
                eventJson["device_os_version"] = attributes.deviceOsVersion;
                eventJson["platform"] = attributes.platform;
                eventJson["device_manufacturer"] = attributes.deviceManufacturer;
                eventJson["authenticated"] = attributes.authenticated;
                eventJson["session_id"] = mSessionId;
                eventJson["proposition"] = attributes.proposition;
                if (!attributes.retailer.empty())
                {
                    eventJson["retailer"] = attributes.retailer;
                }
                if (!attributes.jvAgent.empty())
                {
                    eventJson["jv_agent"] = attributes.jvAgent;
                }
                if (!attributes.coam.empty())
                {
                    eventJson["coam"] = JsonValue(attributes.coam == "true");
                }
                eventJson["device_serial_number"] = attributes.deviceSerialNumber;
                if (!attributes.deviceFriendlyName.empty())
                {
                    eventJson["device_friendly_name"] = attributes.deviceFriendlyName;
                }
                if (!attributes.deviceMacAddress.empty())
                {
                    eventJson["device_mac_address"] = attributes.deviceMacAddress;
                }
                if (!attributes.country.empty())
                {
                    eventJson["country"] = attributes.country;
                }
                if (!attributes.region.empty())
                {
                    eventJson["region"] = attributes.region;
                }
                if (!attributes.accountType.empty())
                {
                    eventJson["account_type"] = attributes.accountType;
                }
                if (!attributes.accountOperator.empty())
                {
                    eventJson["operator"] = attributes.accountOperator;
                }
                if (!attributes.accountDetailType.empty())
                {
                    eventJson["account_detail_type"] = attributes.accountDetailType;
                }

                eventJson["event_payload"] = JsonObject(event.eventPayload);
            }
            else
            {
                //Sift 1.0
                eventJson["event_name"] = event.eventName;
                eventJson["event_schema"] = attributes.productName + "/" + event.eventName + "/" + event.eventVersion;
                eventJson["event_payload"] = JsonObject(event.eventPayload);
                eventJson["session_id"] = mSessionId;
                eventJson["event_id"] = GenerateRandomUUID();

                if (!attributes.xboAccountId.empty() && !attributes.xboDeviceId.empty() && !attributes.partnerId.empty())
                {
                    eventJson["account_id"] = attributes.xboAccountId;
                    eventJson["device_id"] = attributes.xboDeviceId;
                    eventJson["partner_id"] = attributes.partnerId;
                }
                else
                {
                    LOGWARN(" Account ID, Device ID, or Partner ID is empty for: %s", event.eventName.c_str());
                }

                eventJson["app_name"] = attributes.deviceAppName;
                eventJson["app_ver"] = attributes.deviceAppVersion;
                eventJson["device_model"] = attributes.deviceModel;
                eventJson["device_timezone"] = attributes.deviceTimeZone;
                eventJson["platform"] = attributes.platform;
                eventJson["os_ver"] = attributes.deviceOsVersion;
                eventJson["device_language"] = ""; // Empty for now
                eventJson["timestamp"] = event.epochTimestamp;
                eventJson["device_type"] = attributes.deviceType;
            }

            std::string json;
            eventJson.ToString(json);

            if (mStorePtr != nullptr
                && mStorePtr->PostEvent(json))
            {
                LOGINFO("Event %s sent to store", event.eventName.c_str());
                return true;
            }

            LOGERR("Failed to send event %s to store", event.eventName.c_str());

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

    }
}