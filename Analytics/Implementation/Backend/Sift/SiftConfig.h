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
#include <interfaces/IStore.h>
#include <mutex>
#include <memory>

namespace WPEFramework
{
    namespace Plugin
    {

        class SiftConfig
        {
        public:
            struct Config
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
                std::string deviceTimeZone;
                std::string deviceOsName;
                std::string deviceOsVersion;
                std::string platform;
                std::string deviceManufacturer;
                bool authenticated;
                std::string sessionId;
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

                // TODO: read in SiftUploader
                std::string url;
                std::string apiKey;

                // Sift 1.0 atributes that left
                std::string deviceSoftwareVersion;
                std::string deviceAppName;
                std::string deviceAppVersion;
                std::string accountId;
                std::string deviceId;
            };

            SiftConfig(const SiftConfig &) = delete;
            SiftConfig &operator=(const SiftConfig &) = delete;

            SiftConfig(PluginHost::IShell *shell);
            ~SiftConfig();

            bool Get(Config& config);

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

        private:
            void TriggerInitialization();
            void InitializeKeysMap();
            void Initialize();
            void ParsePluginConfig();

            uint32_t GetValueFromPersistent(const string &ns, const string &key, string &value);
            void GetAuthServiceValues();
            static void ActivatePlugin(PluginHost::IShell *shell, const char *callSign);
            static bool IsPluginActivated(PluginHost::IShell *shell, const char *callSign);

            std::thread mInitializationThread;
            Core::Sink<MonitorKeys> mMonitorKeys;
            std::mutex mMutex;
            Config mConfig;  
            PluginHost::IShell *mShell;
            std::map<std::string, std::map<std::string, std::string*>> mKeysMap;
        };

        typedef std::unique_ptr<SiftConfig> SiftConfigPtr;
    }
}