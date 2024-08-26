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
                // Sift 2.0 decoration
                std::string commonSchema;
                std::string env;
                std::string productName;
                std::string productVersion; //TODO: in persistent or /system/information/entertainmentOSVersion
                std::string loggerName;
                std::string loggerVersion;
                std::string partnerId; // from authservice
                std::string xboAccountId; // from authservice ServiceAccountId
                std::string xboDeviceId; // from authservice XDeviceId
                bool activated; // from authservice IF (authservice getActivationStatus returns activation-state = activated) then TRUE
                std::string deviceModel;
                std::string deviceType;
                std::string deviceTimeZone;
                std::string deviceOsName;
                std::string deviceOsVersion;
                std::string platform;
                std::string deviceManufacturer;
                bool authenticated; // set to true if the event is to be SAT authenticated
                std::string sessionId; // TODO: should be auto generated, but for now we will use sessionId from file
                std::string proposition;
                std::string retailer;
                std::string jvAgent;
                std::string coam;
                std::string deviceSerialNumber; // /system/information serialNumber
                std::string deviceFriendlyName;
                std::string deviceMacAddress; // /system/information MACAddress
                std::string country;
                std::string region;
                std::string accountType;
                std::string accountOperator;
                std::string accountDetailType;

                // TODO: read in SiftUploader
                std::string url;
                std::string apiKey;
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