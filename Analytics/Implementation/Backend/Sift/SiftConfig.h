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
#pragma once

#include "../../../Module.h"
#include "../../SystemTime/SystemTime.h"
#include <interfaces/IStore.h>
#include <interfaces/IPrivacy.h>
#include <mutex>
#include <memory>
#include <set>

namespace WPEFramework
{
    namespace Plugin
    {

        class SiftConfig
        {
        public:
            struct Attributes
            {
                bool schema2Enabled;
                // Sift 2.0 decoration
                std::string commonSchema;
                std::string env;
                std::string productName;
                std::string productVersion;
                std::string loggerName;
                std::string loggerVersion;
                std::string partnerId;
                std::string xboAccountId;
                std::string xboDeviceId;
                bool activated;
                std::string deviceModel;
                std::string deviceType;
                int32_t deviceTimeZone;
                std::string deviceOsName;
                std::string deviceOsVersion;
                std::string platform;
                std::string deviceManufacturer;
                bool authenticated;
                std::string proposition;
                std::string retailer;
                std::string jvAgent;
                std::string coam;
                std::string deviceSerialNumber;
                std::string deviceFriendlyName;
                std::string deviceMacAddress;
                std::string country;
                std::string region;
                std::string accountType;
                std::string accountOperator;
                std::string accountDetailType;

                // Sift 1.0 atributes that left
                std::string deviceAppName;
                std::string deviceAppVersion;
            };

            struct StoreConfig
            {
                std::string path;
                uint32_t eventsLimit;
            };

            struct UploaderConfig
            {
                std::string url;
                uint32_t maxRandomisationWindowTime;
                uint32_t maxEventsInPost;
                uint32_t maxRetries;
                uint32_t minRetryPeriod;
                uint32_t maxRetryPeriod;
                uint32_t exponentialPeriodicFactor;
            };

            enum PrivacyChanged
            {
                EXCLUSION_POLICY
            };

            SiftConfig(const SiftConfig &) = delete;
            SiftConfig &operator=(const SiftConfig &) = delete;

            SiftConfig(PluginHost::IShell *shell, SystemTimePtr systemTime);
            ~SiftConfig();

            bool GetAttributes(Attributes &attributes);
            bool GetStoreConfig(StoreConfig &config);
            bool GetUploaderConfig(UploaderConfig &config);
            bool GetCetList(std::list<std::string> &cetList, bool &drop, const bool useCache, const std::string &appId = "");

            void OnPrivacyChanged(PrivacyChanged what, bool value = false);

        private:
            class MonitorKeys : public Exchange::IStore::INotification {
            private:
                MonitorKeys(const MonitorKeys&) = delete;
                MonitorKeys& operator=(const MonitorKeys&) = delete;
            
            public:
                MonitorKeys() : mCallbacks() {}
                ~MonitorKeys() = default;

                typedef std::function<void(const std::string &)> Callback;

                void ValueChanged(const string &ns, const string &key, const string &value) override;
                void StorageExceeded() override;

                void RegisterCallback(const string &ns, const string &key, Callback callback);

                BEGIN_INTERFACE_MAP(MonitorKeys)
                INTERFACE_ENTRY(Exchange::IStore::INotification)
                END_INTERFACE_MAP
            private:
                std::map<std::string, std::map<std::string, Callback>> mCallbacks;
            };

            class MonitorPrivacy : public Exchange::IPrivacy::INotification {
            private:
                MonitorPrivacy(const MonitorPrivacy&) = delete;
                MonitorPrivacy& operator=(const MonitorPrivacy&) = delete;
            public:
                MonitorPrivacy(SiftConfig *parent) : mParent(*parent) {
                    ASSERT(parent != nullptr);
                }
                ~MonitorPrivacy() = default;
                // Not in use
                void OnConsentStringChanged() {}
                void OnContinueWatchingChanged(const bool allowed) {}
                void OnPersonalizedRecommendationChanged(const bool allowed) {}
                void OnProductAnalyticsChanged(const bool allowed) {}
                void OnWatchHistoryChanged(const bool allowed) {}

                void OnExclusionPolicyChanged()
                {
                    mParent.OnPrivacyChanged(EXCLUSION_POLICY);
                }

                BEGIN_INTERFACE_MAP(MonitorPrivacy)
                INTERFACE_ENTRY(Exchange::IPrivacy::INotification)
                END_INTERFACE_MAP
            private:
                SiftConfig &mParent;
            };

            struct PrivacyExclusionPolicy
            {
                std::set<std::string> dataEvents;
                std::set<std::string> entityReference;
                bool derivativePropagation;

                PrivacyExclusionPolicy()
                    : derivativePropagation(false)
                {
                }
            };

        private:
            void TriggerInitialization();
            void InitializeKeysMap();
            void Initialize();
            void ParsePluginConfig();
            uint32_t GetValueFromPersistent(const string &ns, const string &key, string &value);
            void UpdateXboValues();
            bool UpdateTimeZone();
            void OnActivationStatusChanged(const JsonObject& parameters);
            void SyncPrivacyExclusionPolicies();

            static void ActivatePlugin(PluginHost::IShell *shell, const char *callSign);
            static bool IsPluginActivated(PluginHost::IShell *shell, const char *callSign);
            static std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> GetThunderControllerClient(std::string callsign="");
            static bool GetFileRegex(const std::string &fileName, const std::string &regex, std::string &result);

            std::thread mInitializationThread;
            Core::Sink<MonitorKeys> mMonitorKeys;
            std::mutex mMutex;
            Attributes mAttributes;
            StoreConfig mStoreConfig;
            UploaderConfig mUploaderConfig;
            PluginHost::IShell *mShell;
            std::map<std::string, std::map<std::string, std::string*>> mKeysMap;
            SystemTimePtr mSystemTime;
            std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> mAuthServiceLink;
            Core::Sink<MonitorPrivacy> mMonitorPrivacy;
            std::map<std::string, std::string> mCetMap;
            bool mCetDropOnAllTags;
            std::string mCetEventType;
            std::map<std::string, bool> mPrivacySettings;
            std::map<std::string, PrivacyExclusionPolicy> mPrivacyExclusionPolicies;

        };

        typedef std::unique_ptr<SiftConfig> SiftConfigPtr;
    }
}
