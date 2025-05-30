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
#include "SiftConfig.h"
#include "UtilsLogging.h"

#include "interfaces/IPrivacy.h"

#include <algorithm>
#include <cctype> 
#include <fstream>
#include <regex>

#define AUTHSERVICE_CALLSIGN "org.rdk.AuthService"
#define SYSTEM_CALLSIGN "org.rdk.System"
#define PERSISTENT_STORE_CALLSIGN "org.rdk.PersistentStore"
#define PRIVACY_CALLSIGN "org.rdk.Privacy"
#define PERSISTENT_STORE_ANALYTICS_NAMESPACE "Analytics"
#define PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE "accountProfile"
#define JSONRPC_THUNDER_TIMEOUT 20000
#define THUNDER_ACCESS_DEFAULT_VALUE "127.0.0.1:9998"
#define DEVICE_PROPERTIES_FILE "/etc/device.properties"

namespace WPEFramework
{
    namespace Plugin
    {
        static std::string sThunderSecurityToken;

        class AnalyticsSiftConfig : public Core::JSON::Container {
        private:
            AnalyticsSiftConfig(const AnalyticsSiftConfig&) = delete;
            AnalyticsSiftConfig& operator=(const AnalyticsSiftConfig&) = delete;

        public:
            class SiftConfig : public Core::JSON::Container {
            public:
                SiftConfig(const SiftConfig&) = delete;
                SiftConfig& operator=(const SiftConfig&) = delete;

                SiftConfig()
                    : Core::JSON::Container()
                    , Schema2(false)
                    , CommonSchema()
                    , Env()
                    , ProductName()
                    , LoggerName()
                    , LoggerVersion()
                    , PlatformDfl("entos:rdk")
                    , MaxRandomisationWindowTime(300)
                    , MaxEventsInPost(10)
                    , MaxRetries(10)
                    , MinRetryPeriod(1)
                    , MaxRetryPeriod(30)
                    , ExponentialPeriodicFactor(2)
                    , StorePath()
                    , EventsLimit(1000)
                    , Url()
                    , CetMap()
                    , DropOnAllTags(true)
                    , CetEventType()

                {
                    Add(_T("schema2"), &Schema2);
                    Add(_T("commonschema"), &CommonSchema);
                    Add(_T("env"), &Env);
                    Add(_T("productname"), &ProductName);
                    Add(_T("loggername"), &LoggerName);
                    Add(_T("loggerversion"), &LoggerVersion);
                    Add(_T("platformdefalut"), &PlatformDfl);
                    Add(_T("maxrandomisationwindowtime"), &MaxRandomisationWindowTime);
                    Add(_T("maxeventsinpost"), &MaxEventsInPost);
                    Add(_T("maxretries"), &MaxRetries);
                    Add(_T("minretryperiod"), &MinRetryPeriod);
                    Add(_T("maxretryperiod"), &MaxRetryPeriod);
                    Add(_T("exponentialperiodicfactor"), &ExponentialPeriodicFactor);
                    Add(_T("storepath"), &StorePath);
                    Add(_T("eventslimit"), &EventsLimit);
                    Add(_T("url"), &Url);
                    Add(_T("cetmap"), &CetMap);
                    Add(_T("cetdroponalltags"), &DropOnAllTags);
                    Add(_T("ceteventtype"), &CetEventType);
                }
                ~SiftConfig() = default;

            public:
                Core::JSON::Boolean Schema2;
                Core::JSON::String CommonSchema;
                Core::JSON::String Env;
                Core::JSON::String ProductName;
                Core::JSON::String LoggerName;
                Core::JSON::String LoggerVersion;
                Core::JSON::String PlatformDfl;
                Core::JSON::DecUInt32 MaxRandomisationWindowTime;
                Core::JSON::DecUInt32 MaxEventsInPost;
                Core::JSON::DecUInt32 MaxRetries;
                Core::JSON::DecUInt32 MinRetryPeriod;
                Core::JSON::DecUInt32 MaxRetryPeriod;
                Core::JSON::DecUInt32 ExponentialPeriodicFactor;
                Core::JSON::String StorePath;
                Core::JSON::DecUInt32 EventsLimit;
                Core::JSON::String Url;
                Core::JSON::VariantContainer CetMap;
                Core::JSON::Boolean DropOnAllTags;
                Core::JSON::String CetEventType;
            };

           
        public:
            AnalyticsSiftConfig()
                : Core::JSON::Container()
                , DeviceOsName()
                , Sift()
            {
                Add(_T("deviceosname"), &DeviceOsName);
                Add(_T("sift"), &Sift);
            }
            ~AnalyticsSiftConfig()
            {
            }

        public:
            Core::JSON::String DeviceOsName;
            SiftConfig Sift;
        };

        struct JSONRPCDirectLink
        {
        private:
            uint32_t mId{0};
            std::string mCallSign{};
#if ((THUNDER_VERSION >= 4) && (THUNDER_VERSION_MINOR == 4))
            PluginHost::ILocalDispatcher *mDispatcher{nullptr};
#else
            PluginHost::IDispatcher *mDispatcher{nullptr};
#endif

            Core::ProxyType<Core::JSONRPC::Message> Message() const
            {
                return (Core::ProxyType<Core::JSONRPC::Message>(PluginHost::IFactories::Instance().JSONRPC()));
            }

            template <typename PARAMETERS>
            bool ToMessage(PARAMETERS &parameters, Core::ProxyType<Core::JSONRPC::Message> &message) const
            {
                return ToMessage((Core::JSON::IElement *)(&parameters), message);
            }
            bool ToMessage(Core::JSON::IElement *parameters, Core::ProxyType<Core::JSONRPC::Message> &message) const
            {
                if (!parameters->IsSet())
                    return true;
                string values;
                if (!parameters->ToString(values))
                {
                    LOGERR("Failed to convert params to string");
                    return false;
                }
                if (values.empty() != true)
                {
                    message->Parameters = values;
                }
                return true;
            }
            template <typename RESPONSE>
            bool FromMessage(RESPONSE &response, const Core::ProxyType<Core::JSONRPC::Message> &message, bool isResponseString = false) const
            {
                return FromMessage((Core::JSON::IElement *)(&response), message, isResponseString);
            }
            bool FromMessage(Core::JSON::IElement *response, const Core::ProxyType<Core::JSONRPC::Message> &message, bool isResponseString = false) const
            {
                Core::OptionalType<Core::JSON::Error> error;
                if (!isResponseString && !response->FromString(message->Result.Value(), error))
                {
                    LOGERR("Failed to parse response!!! Error: %s", error.Value().Message().c_str());
                    return false;
                }
                return true;
            }

        public:
            JSONRPCDirectLink(PluginHost::IShell *service, std::string callsign)
                : mCallSign(callsign)
            {
                if (service)
#if ((THUNDER_VERSION >= 4) && (THUNDER_VERSION_MINOR == 4))
                    mDispatcher = service->QueryInterfaceByCallsign<PluginHost::ILocalDispatcher>(mCallSign);
#else
                    mDispatcher = service->QueryInterfaceByCallsign<PluginHost::IDispatcher>(mCallSign);
#endif
            }

            JSONRPCDirectLink(PluginHost::IShell *service)
                : JSONRPCDirectLink(service, "Controller")
            {
            }
            ~JSONRPCDirectLink()
            {
                if (mDispatcher)
                    mDispatcher->Release();
            }

            template <typename PARAMETERS>
            uint32_t Get(const uint32_t waitTime, const string &method, PARAMETERS &respObject)
            {
                JsonObject empty;
                return Invoke(waitTime, method, empty, respObject);
            }

            template <typename PARAMETERS>
            uint32_t Set(const uint32_t waitTime, const string &method, const PARAMETERS &sendObject)
            {
                JsonObject empty;
                return Invoke(waitTime, method, sendObject, empty);
            }

            template <typename PARAMETERS, typename RESPONSE>
            uint32_t Invoke(const uint32_t waitTime, const string &method, const PARAMETERS &parameters, RESPONSE &response, bool isResponseString = false)
            {
                if (mDispatcher == nullptr)
                {
                    LOGERR("No JSON RPC dispatcher for %s", mCallSign.c_str());
                    return Core::ERROR_GENERAL;
                }

                auto message = Message();

                message->JSONRPC = Core::JSONRPC::Message::DefaultVersion;
                message->Id = Core::JSON::DecUInt32(++mId);
                message->Designator = Core::JSON::String(mCallSign + ".1." + method);

                ToMessage(parameters, message);

                const uint32_t channelId = ~0;
#if ((THUNDER_VERSION >= 4) && (THUNDER_VERSION_MINOR == 4))
                string output = "";
                uint32_t result = Core::ERROR_BAD_REQUEST;

                if (mDispatcher != nullptr)
                {
                    PluginHost::ILocalDispatcher *localDispatcher = mDispatcher->Local();

                    ASSERT(localDispatcher != nullptr);

                    if (localDispatcher != nullptr)
                    {
                        result = mDispatcher->Invoke(channelId, message->Id.Value(), sThunderSecurityToken, message->Designator.Value(), message->Parameters.Value(), output);
                    }
                }

                if (message.IsValid() == true)
                {
                    if (result == static_cast<uint32_t>(~0))
                    {
                        message.Release();
                    }
                    else if (result == Core::ERROR_NONE)
                    {
                        if (output.empty() == true)
                        {
                            message->Result.Null(true);
                        }
                        else
                        {
                            message->Result = output;
                        }
                    }
                    else
                    {
                        message->Error.SetError(result);
                        if (output.empty() == false)
                        {
                            message->Error.Text = output;
                        }
                        LOGERR("Call failed: %s error: %s", message->Designator.Value().c_str(), message->Error.Text.Value().c_str());
                    }
                }
#elif (THUNDER_VERSION == 2)
                auto resp = mDispatcher->Invoke(sThunderSecurityToken, channelId, *message);
#else
                Core::JSONRPC::Context context(channelId, message->Id.Value(), sThunderSecurityToken);
                auto resp = mDispatcher->Invoke(context, *message);
#endif

#if ((THUNDER_VERSION == 2) || (THUNDER_VERSION >= 4) && (THUNDER_VERSION_MINOR == 2))

                if (resp->Error.IsSet())
                {
                    LOGERR("Call failed: %s error: %s", message->Designator.Value().c_str(), resp->Error.Text.Value().c_str());
                    return resp->Error.Code;
                }

                if (!FromMessage(response, resp, isResponseString))
                {
                    return Core::ERROR_GENERAL;
                }

                return Core::ERROR_NONE;
#else
                if (!FromMessage(response, message, isResponseString))
                {
                    return Core::ERROR_GENERAL;
                }
                
                return result;
#endif
            }
        };

        SiftConfig::SiftConfig(PluginHost::IShell *shell, SystemTimePtr systemTime) : mInitializationThread(),
                                                            mMonitorKeys(),
                                                            mMutex(),
                                                            mAttributes(),
                                                            mStoreConfig(),
                                                            mUploaderConfig(),
                                                            mShell(shell),
                                                            mSystemTime(systemTime),
                                                            mAuthServiceLink(nullptr),
                                                            mMonitorPrivacy(this),
                                                            mCetMap(),
                                                            mCetDropOnAllTags(true),
                                                            mPrivacySettings(),
                                                            mPrivacyExclusionPolicies()
        {
            ASSERT(shell != nullptr);
            ParsePluginConfig();
            InitializeKeysMap();
            TriggerInitialization();
        }

        SiftConfig::~SiftConfig()
        {
            LOGINFO("SiftConfig::~SiftConfig");
            mInitializationThread.join();
            // Unregister for notifications
            auto interface = mShell->QueryInterfaceByCallsign<Exchange::IStore>(PERSISTENT_STORE_CALLSIGN);
            if (interface == nullptr)
            {
                LOGERR("No IStore");
            }
            else
            {
                uint32_t result = interface->Unregister(&mMonitorKeys);
                LOGINFO("IStore status %d", result);
                interface->Release();
            }
            if (mAuthServiceLink != nullptr)
            {
                mAuthServiceLink->Unsubscribe(JSONRPC_THUNDER_TIMEOUT, _T("onActivationStatusChanged"));
            }
        }

        bool SiftConfig::GetAttributes(SiftConfig::Attributes &attributes)
        {
            UpdateXboValues();
            bool timeZoneValid = UpdateTimeZone();

            mMutex.lock();
            bool valid = false;

            // If not activated, make sure xbo values are empty and do not contain any old data
            if (!mAttributes.activated)
            {
                mAttributes.xboDeviceId.clear();
                mAttributes.xboAccountId.clear();
            }

            if (mAttributes.schema2Enabled)
            {
                //Sift 2.0 requires attributes
                bool activatedValid = mAttributes.activated ? (!mAttributes.xboDeviceId.empty()
                    && !mAttributes.xboAccountId.empty()) : true;

                // Sift2.0 platform equals proposition
                mAttributes.platform = mAttributes.proposition;

                // If productVersion empty, use OsVersion
                if (mAttributes.productVersion.empty())
                {
                    mAttributes.productVersion = mAttributes.deviceOsVersion;
                }

                valid = (!mAttributes.commonSchema.empty()
                    && !mAttributes.productName.empty()
                    && !mAttributes.productVersion.empty()
                    && !mAttributes.loggerName.empty()
                    && !mAttributes.loggerVersion.empty()
                    && !mAttributes.partnerId.empty()
                    && activatedValid
                    && !mAttributes.deviceModel.empty()
                    && !mAttributes.deviceType.empty()
                    && !mAttributes.deviceOsName.empty()
                    && !mAttributes.deviceOsVersion.empty()
                    && !mAttributes.platform.empty()
                    && !mAttributes.deviceManufacturer.empty()
                    && !mAttributes.proposition.empty()
                    && !mAttributes.deviceSerialNumber.empty()
                    && !mAttributes.deviceMacAddress.empty()
                    && timeZoneValid);

                if (valid)
                {
                    if (mAttributes.deviceType == "TV")
                    {
                        mAttributes.deviceType = "IPTV";
                    }
                    else if (mAttributes.deviceType == "IPSETTOPBOX")
                    {
                        mAttributes.deviceType = "IPSTB";
                    }
                }
            }
            else //Sift 1.0 required attributes
            {
                valid = (!mAttributes.productName.empty()
                    && !mAttributes.deviceAppName.empty()
                    && !mAttributes.deviceAppVersion.empty()
                    && !mAttributes.deviceModel.empty()
                    && !mAttributes.platform.empty()
                    && !mAttributes.deviceOsVersion.empty()
                    && !mAttributes.deviceType.empty()
                    && timeZoneValid);
            }

            if (valid)
            {
                attributes = mAttributes;
            }

            mMutex.unlock();
            return valid;
        }

        bool SiftConfig::GetStoreConfig(StoreConfig &config)
        {
            mMutex.lock();
            bool valid = !mStoreConfig.path.empty();
            if (valid)
            {
                config = mStoreConfig;
            }
            mMutex.unlock();
            return valid;
        }

        bool SiftConfig::GetUploaderConfig(UploaderConfig &config)
        {
            mMutex.lock();
            bool valid = !mUploaderConfig.url.empty();
            if (valid)
            {
                config = mUploaderConfig;
            }
            mMutex.unlock();
            return valid;
        }

        bool SiftConfig::GetCetList(std::list<std::string> &cetList, bool &drop, const bool useCache, const std::string &appId)
        {
            if (mCetMap.empty() || mCetEventType.empty())
            {
                return false;
            }

            // Prepare set of cet tags to not duplicate
            std::set<std::string> cetSet;

            //TODO: PrivacySettings should be kept in sync via notifications
            if (!useCache)
            {
                auto privacy = mShell->QueryInterfaceByCallsign<Exchange::IPrivacy>(PRIVACY_CALLSIGN);
                std::map<std::string, bool> newPrivacySettings;
                if (privacy)
                {
                    Exchange::IPrivacy::IPrivacySettingInfoIterator *settings = nullptr;
                    if (privacy->GetAllPrivacySettings(settings) == Core::ERROR_NONE)
                    {
                        if (settings != nullptr)
                        {
                            Exchange::IPrivacy::PrivacySettingInfo info;
                            while (settings->Next(info))
                            {
                                newPrivacySettings[info.settingName] = info.allowed;
                            }
                            settings->Release();
                        }
                    }
                    privacy->Release();
                }
                mPrivacySettings = newPrivacySettings;
            }

            std::unique_lock<std::mutex> lock(mMutex);
            drop = mCetDropOnAllTags;

            for (auto &tag : mCetMap)
            {
                bool excluded = false;
                if (!appId.empty() && mPrivacyExclusionPolicies.find(tag.first) != mPrivacyExclusionPolicies.end())
                {
                    auto &exclusionPolicy = mPrivacyExclusionPolicies.at(tag.first);
                    if (exclusionPolicy.dataEvents.find(mCetEventType) != exclusionPolicy.dataEvents.end())
                    {
                        if (exclusionPolicy.entityReference.find(appId) != exclusionPolicy.entityReference.end())
                        {
                            excluded = true;
                        }
                    }
                }

                // Excluded by Exclusion Policy
                if (excluded)
                {
                    LOGINFO("Cet Tag %s added by Exclusion Policy", tag.second.c_str());
                    cetSet.insert(tag.second);
                }
                else
                {
                    auto it = mPrivacySettings.find(tag.first);
                    // Excluded by Privacy Settings equal not allowed
                    if (!mPrivacySettings.empty() && (it == mPrivacySettings.end() || it->second == false))
                    {
                        LOGINFO("Cet Tag %s added by Privacy Settings", tag.second.c_str());
                        cetSet.insert(tag.second);
                    }
                    // Not excluded. Drop only on all tags excluded
                    else
                    {
                        drop = false;
                    }
                }
            }

            // Convert set to list
            cetList.clear();
            for (const auto &cet : cetSet)
            {
                cetList.push_back(cet);
            }

            return !cetList.empty();
        }

        void SiftConfig::OnPrivacyChanged(PrivacyChanged what, bool value)
        {
            if (what == PrivacyChanged::EXCLUSION_POLICY)
            {
                // Notification called from separate thread, so it is ok to sync here
                SyncPrivacyExclusionPolicies();
            }
        }

        void SiftConfig::TriggerInitialization()
        {
            mInitializationThread = std::thread(&SiftConfig::Initialize, this);
        }

        void SiftConfig::InitializeKeysMap()
        {
            //SIFT 2.0 attributes from persistent storage
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["deviceType"] = &mAttributes.deviceType;
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["modelNumber"] = &mAttributes.deviceOsVersion;
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["manufacturer"] = &mAttributes.deviceManufacturer;
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["serialNumber"] = &mAttributes.deviceSerialNumber;
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["entertainmentOSVersion"] = &mAttributes.productVersion;
            //For Sift 1.0 use default values from config
            if (mAttributes.schema2Enabled)
            {
                mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["proposition"] = &mAttributes.proposition;
            }
            mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["retailer"] = &mAttributes.retailer;
            mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["jvagent"] = &mAttributes.jvAgent;
            mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["coam"] = &mAttributes.coam;
            mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["accountType"] = &mAttributes.accountType;
            mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["operator"] = &mAttributes.accountOperator;
            mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["detailType"] = &mAttributes.accountDetailType;

            //SIFT 1.0 attributes from persistent storage
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["deviceAppName"] = &mAttributes.deviceAppName;
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["deviceAppVersion"] = &mAttributes.deviceAppVersion;

            // If Sift url empty, try to get from persistent store
            if (mUploaderConfig.url.empty())
            {
                mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["sift_url"] = &mUploaderConfig.url;
            }
        }

        void SiftConfig::ParsePluginConfig()
        {
            ASSERT(mShell != nullptr);
            std::string configLine = mShell->ConfigLine();
            Core::OptionalType<Core::JSON::Error> error;
            AnalyticsSiftConfig config;

            if (config.FromString(configLine, error) == false)
            {
                SYSLOG(Logging::ParsingError,
                       (_T("Failed to parse config line, error: '%s', config line: '%s'."),
                        (error.IsSet() ? error.Value().Message().c_str() : "Unknown"),
                        configLine.c_str())); 
            }
            else
            {
                mAttributes.schema2Enabled = config.Sift.Schema2.Value();
                mAttributes.commonSchema = config.Sift.CommonSchema.Value();
                mAttributes.env = config.Sift.Env.Value();
                mAttributes.productName = config.Sift.ProductName.Value();
                mAttributes.loggerName = config.Sift.LoggerName.Value();
                mAttributes.loggerVersion = config.Sift.LoggerVersion.Value();
                mAttributes.deviceOsName = config.DeviceOsName.Value();

                // Set a default values for platform and proposition if not available in Persistent Store
                mAttributes.platform = config.Sift.PlatformDfl.Value();
                mAttributes.proposition = config.Sift.PlatformDfl.Value();
                
                mStoreConfig.path = config.Sift.StorePath.Value();
                SYSLOG(Logging::Startup, ("Sift Store Path: %s", mStoreConfig.path.c_str()));
                mStoreConfig.eventsLimit = config.Sift.EventsLimit.Value();

                mUploaderConfig.url = config.Sift.Url.Value();
                mUploaderConfig.maxRandomisationWindowTime = config.Sift.MaxRandomisationWindowTime.Value();
                mUploaderConfig.maxEventsInPost = config.Sift.MaxEventsInPost.Value();
                mUploaderConfig.maxRetries = config.Sift.MaxRetries.Value();
                mUploaderConfig.minRetryPeriod = config.Sift.MinRetryPeriod.Value();
                mUploaderConfig.maxRetryPeriod = config.Sift.MaxRetryPeriod.Value();
                mUploaderConfig.exponentialPeriodicFactor = config.Sift.ExponentialPeriodicFactor.Value();

                SYSLOG(Logging::Startup, (_T("Parsed Analytics config: '%s', '%s', '%s', '%s', '%s', '%s', '%s'."),
                                          mAttributes.commonSchema.c_str(),
                                          mAttributes.env.c_str(),
                                          mAttributes.productName.c_str(),
                                          mAttributes.loggerName.c_str(),
                                          mAttributes.loggerVersion.c_str(),
                                          mAttributes.platform.c_str(),
                                          mAttributes.deviceOsName.c_str()
                    ));

                // Set CetMap
                auto cetMapIter = config.Sift.CetMap.Variants();

                while (cetMapIter.Next())
                {
                    if (cetMapIter.Current().Content() == WPEFramework::Core::JSON::Variant::type::STRING)
                    {
                        mCetMap[cetMapIter.Label()] = cetMapIter.Current().String();
                        SYSLOG(Logging::Startup, ("Sift CetMap: \"%s\" : \"%s\"", cetMapIter.Label(), mCetMap[cetMapIter.Label()].c_str()));
                    }
                }
                mCetDropOnAllTags = config.Sift.DropOnAllTags.Value();
                SYSLOG(Logging::Startup, ("Sift DropOnAllTags: %s", mCetDropOnAllTags ? "true" : "false"));
                mCetEventType = config.Sift.CetEventType.Value();
                SYSLOG(Logging::Startup, ("Sift CetEventType: \"%s\"", mCetEventType.c_str()));

                // Get Sift url from device properties as first choise
                if (GetFileRegex(DEVICE_PROPERTIES_FILE, R"(SIFT_URL=\S+)", mUploaderConfig.url))
                {
                    SYSLOG(Logging::Startup, ("Sift Url from device properties: %s", mUploaderConfig.url.c_str()));
                }

                // Get Sift common schema from device properties as first choise
                if (GetFileRegex(DEVICE_PROPERTIES_FILE, R"(SIFT_COMMON_SCHEMA=\S+)", mAttributes.commonSchema))
                {
                    SYSLOG(Logging::Startup, ("Sift Common Schema from device properties: %s", mAttributes.commonSchema.c_str()));
                }
                
                // Get Sift env from device properties as first choise
                if (GetFileRegex(DEVICE_PROPERTIES_FILE, R"(BUILD_TYPE=\S+)", mAttributes.env))
                {
                    SYSLOG(Logging::Startup, ("Sift Env from device properties: %s", mAttributes.env.c_str()));
                }

                // Get Sift schema from device properties as first choise
                string schema;
                if (GetFileRegex(DEVICE_PROPERTIES_FILE, R"(SIFT_SCHEMA_VER=\S+)", schema))
                {
                    if (schema == "2.0")
                    {
                        mAttributes.schema2Enabled = true;
                    }
                    else if (schema == "1.0")
                    {
                        mAttributes.schema2Enabled = false;
                    }
                    else
                    {
                        LOGERR("Invalid SIFT_SCHEMA value in %s: %s", DEVICE_PROPERTIES_FILE, schema.c_str());
                    }
                }
            }
        }

        void SiftConfig::Initialize()
        {
            // Generate jsonrpc token
            string token;
            // TODO: use interfaces and remove token
            auto security = mShell->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
            if (security != nullptr)
            {
                string payload = "http://localhost";
                if (security->CreateToken(
                        static_cast<uint16_t>(payload.length()),
                        reinterpret_cast<const uint8_t *>(payload.c_str()),
                        token) == Core::ERROR_NONE)
                {
                    LOGINFO("Got security token\n");
                    sThunderSecurityToken = token;
                }
                else
                {
                    LOGINFO("Failed to get security token\n");
                }
                security->Release();
            }
            else
            {
                LOGINFO("No security agent\n");
            }

            // Set to true if the event is to be SAT authenticated
            mMutex.lock();
            mAttributes.authenticated = false;
            mMutex.unlock();

            //Activate AuthService plugin if needed
            if (IsPluginActivated(mShell, AUTHSERVICE_CALLSIGN) == false)
            {
                ActivatePlugin(mShell, AUTHSERVICE_CALLSIGN);
            }

            // try to create AuthService jsonrpc link and register to onActivationStatusChanged event
            mAuthServiceLink = GetThunderControllerClient(AUTHSERVICE_CALLSIGN);
            if (mAuthServiceLink)
            {
                uint32_t result = mAuthServiceLink->Subscribe<JsonObject>(JSONRPC_THUNDER_TIMEOUT, _T("onActivationStatusChanged"), &SiftConfig::OnActivationStatusChanged, this);
                if (result != Core::ERROR_NONE)
                {
                    LOGERR("Failed to subscribe to onActivationStatusChanged");
                }

                JsonObject params;
                JsonObject response;

                // Get partnerId from AuthService.getDeviceId
                result = mAuthServiceLink->Invoke<JsonObject, JsonObject>(JSONRPC_THUNDER_TIMEOUT, "getDeviceId", params, response);
                if (result == Core::ERROR_NONE && response.HasLabel("partnerId"))
                {
                    mMutex.lock();
                    mAttributes.partnerId = response["partnerId"].String();
                    LOGINFO("Got partnerId %s", mAttributes.partnerId.c_str());
                    mMutex.unlock();
                }
                else
                {
                    LOGERR("Failed to get partnerId: %d", result);
                }

                // Get current activation status from AuthService.getActivationStatus
                result = mAuthServiceLink->Invoke<JsonObject, JsonObject>(JSONRPC_THUNDER_TIMEOUT, "getActivationStatus", params, response);
                if (result == Core::ERROR_NONE && response.HasLabel("status"))
                {
                    bool activated = false;
                    if (response["status"].String() == "activated")
                    {
                        activated = true;
                    }

                    mMutex.lock();
                    mAttributes.activated = activated;
                    mMutex.unlock();
                }
                else
                {
                    LOGERR("Failed to get activation status");
                }
            }
            else
            {
                LOGERR("Failed to get AuthService link");
                mMutex.lock();
                mAttributes.activated = false;
                mMutex.unlock();
            }

            //Activate System plugin if needed
            if (IsPluginActivated(mShell, SYSTEM_CALLSIGN) == false)
            {
                ActivatePlugin(mShell, SYSTEM_CALLSIGN);
            }

            // One time readout attributes
            auto systemLink = std::unique_ptr<JSONRPCDirectLink>(new JSONRPCDirectLink(mShell, SYSTEM_CALLSIGN));
            if (systemLink != nullptr)
            {
                JsonObject params;
                JsonObject response;

                // Get env from System.1.getDeviceInfo[build_type]
                uint32_t result = systemLink->Invoke<JsonObject, JsonObject>(JSONRPC_THUNDER_TIMEOUT, "getDeviceInfo", params, response);
                if (result == Core::ERROR_NONE && response.HasLabel("build_type"))
                {
                    mMutex.lock();
                    mAttributes.env = response["build_type"].String();
                    std::transform(mAttributes.env.begin(), mAttributes.env.end(), mAttributes.env.begin(),
                            [](unsigned char c){ return std::tolower(c); });
                    mMutex.unlock();
                    LOGINFO("Got env %s", mAttributes.env.c_str());
                }

                if (result == Core::ERROR_NONE && response.HasLabel("estb_mac"))
                {
                    mMutex.lock();
                    mAttributes.deviceMacAddress = response["estb_mac"].String();
                    mMutex.unlock();
                    LOGINFO("Got deviceMacAddress %s", mAttributes.deviceMacAddress.c_str());
                }

                if (result == Core::ERROR_NONE && response.HasLabel("model_number"))
                {
                    mMutex.lock();
                    mAttributes.deviceModel = response["model_number"].String();
                    mMutex.unlock();
                    LOGINFO("Got deviceModel %s", mAttributes.deviceModel.c_str());
                }

                mMutex.lock();
                if (result == Core::ERROR_NONE && mAttributes.partnerId.empty() && response.HasLabel("friendly_id"))
                {
                    mAttributes.partnerId = response["friendly_id"].String();
                    LOGINFO("Got partnerId %s", mAttributes.partnerId.c_str());
                }
                mMutex.unlock();

                // Get deviceFriendlyName from System.1.getFriendlyName[friendlyName]
                result = systemLink->Invoke<JsonObject, JsonObject>(JSONRPC_THUNDER_TIMEOUT, "getFriendlyName", params, response);
                if (result == Core::ERROR_NONE && response.HasLabel("friendlyName"))
                {
                    mMutex.lock();
                    mAttributes.deviceFriendlyName = response["friendlyName"].String();
                    mMutex.unlock();
                    LOGINFO("Got deviceFriendlyName %s", mAttributes.deviceFriendlyName.c_str());
                }

                // Get country from System.1.getTerritory[territory]
                // Get region from System.1.getTerritory[region]
                result = systemLink->Invoke<JsonObject, JsonObject>(JSONRPC_THUNDER_TIMEOUT, "getTerritory", params, response);
                if (result == Core::ERROR_NONE)
                {
                    if (response.HasLabel("territory"))
                    {
                        mMutex.lock();
                        mAttributes.country = response["territory"].String();
                        mMutex.unlock();
                        LOGINFO("Got country %s", mAttributes.country.c_str());
                    }
                    if (response.HasLabel("region"))
                    {
                        mMutex.lock();
                        mAttributes.region = response["region"].String();
                        mMutex.unlock();
                        LOGINFO("Got region %s", mAttributes.region.c_str());
                    }
                }
            }

            //Activate persistent store plugin if needed
            if (IsPluginActivated(mShell, PERSISTENT_STORE_CALLSIGN) == false)
            {
                ActivatePlugin(mShell, PERSISTENT_STORE_CALLSIGN);
            }
            
            //Prepare callbacks for persistent store keys
            for(auto& ns : mKeysMap)
            {
                for(auto& key : ns.second)
                {
                    mMonitorKeys.RegisterCallback(ns.first, key.first, [this, ns, key](const std::string& value) {
                        mMutex.lock();
                        *(mKeysMap[ns.first][key.first]) = value;
                        mMutex.unlock();
                    });
                }
            }

            // Register for notifications from persistent store
            auto interface = mShell->QueryInterfaceByCallsign<Exchange::IStore>(PERSISTENT_STORE_CALLSIGN);
            if (interface == nullptr)
            {
                LOGERR("No IStore");
            }
            else
            {
                uint32_t result = interface->Register(&mMonitorKeys);
                LOGINFO("IStore status %d", result);
                interface->Release();
            }

            //Get current values from persistent store
            for(auto& ns : mKeysMap)
            {
                for(auto& key : ns.second)
                {
                    std::string value;
                    uint32_t result = GetValueFromPersistent(ns.first, key.first, value);
                    if (result == Core::ERROR_NONE)
                    {
                        mMutex.lock();
                        *(mKeysMap[ns.first][key.first]) = value;
                        mMutex.unlock();
                    }
                }
            }

            // Activate Privacy plugin if needed
            if (IsPluginActivated(mShell, PRIVACY_CALLSIGN) == false)
            {
                ActivatePlugin(mShell, PRIVACY_CALLSIGN);
            }

            // Register to privacy events and get privacy settings
            auto privacy = mShell->QueryInterfaceByCallsign<Exchange::IPrivacy>(PRIVACY_CALLSIGN);
            if (privacy != nullptr)
            {
                privacy->Register(&mMonitorPrivacy);
                privacy->Release();
            }
            else
            {
                LOGERR("Failed to get Privacy interface");
            }

            SyncPrivacyExclusionPolicies();
        }

        uint32_t SiftConfig::GetValueFromPersistent(const string& ns, const string& key, string& value)
        {
            uint32_t result;
            auto interface = mShell->QueryInterfaceByCallsign<Exchange::IStore>(PERSISTENT_STORE_CALLSIGN);
            if (interface == nullptr)
            {
                result = Core::ERROR_UNAVAILABLE;
                LOGERR("No IStore");
            }
            else
            {
                result = interface->GetValue(ns, key, value);
                LOGINFO("IStore status %d for get %s", result, key.c_str());
                interface->Release();
            }
            return result;
        }

        void SiftConfig::UpdateXboValues()
        {
            if (mAuthServiceLink == nullptr)
            {
                return;
            }

            // get xboAccountId and xboDeviceId from AuthService if activated and empty
            mMutex.lock();
            bool activated = mAttributes.activated;
            string xboAccountId = mAttributes.xboAccountId;
            string xboDeviceId = mAttributes.xboDeviceId;
            mMutex.unlock();

            if (activated && xboAccountId.empty() )
            {
                JsonObject params;
                JsonObject response;
                // get xboAccountId from AuthService.getServiceAccountId
                uint32_t result = mAuthServiceLink->Invoke<JsonObject, JsonObject>(JSONRPC_THUNDER_TIMEOUT, "getServiceAccountId", params, response);
                if (result == Core::ERROR_NONE && response.HasLabel("serviceAccountId"))
                {
                    mMutex.lock();
                    mAttributes.xboAccountId = response["serviceAccountId"].String();
                    LOGINFO("Got xboAccountId %s", mAttributes.xboAccountId.c_str());
                    mMutex.unlock();
                }
                else
                {
                    LOGERR("Failed to get xboAccountId");
                }
            }

            if (activated && xboDeviceId.empty())
            {
                JsonObject params;
                JsonObject response;
                // get xboDeviceId from AuthService.getXDeviceId
                uint32_t result = mAuthServiceLink->Invoke<JsonObject, JsonObject>(JSONRPC_THUNDER_TIMEOUT, "getXDeviceId", params, response);
                if (result == Core::ERROR_NONE && response.HasLabel("xDeviceId"))
                {
                    mMutex.lock();
                    mAttributes.xboDeviceId = response["xDeviceId"].String();
                    LOGINFO("Got xboDeviceId %s", mAttributes.xboDeviceId.c_str());
                    mMutex.unlock();
                }
                else
                {
                    LOGERR("Failed to get xboDeviceId");
                }
            }
        }

        bool SiftConfig::UpdateTimeZone()
        {
            int32_t timeZoneOffsetSec = 0;
            SystemTime::TimeZoneAccuracy accuracy = SystemTime::TimeZoneAccuracy::ACC_UNDEFINED;

            if (mSystemTime != nullptr && mSystemTime->IsSystemTimeAvailable())
            {
                accuracy = mSystemTime->GetTimeZoneOffset(timeZoneOffsetSec);
                mMutex.lock();
                mAttributes.deviceTimeZone = timeZoneOffsetSec * 1000;
                mMutex.unlock();
            }

            if (accuracy == SystemTime::TimeZoneAccuracy::FINAL)
            {
                return true;
            }

            return false;
        }

        void SiftConfig::OnActivationStatusChanged(const JsonObject& parameters)
        {
            std::string parametersString;
            parameters.ToString(parametersString);
            LOGINFO("OnActivationStatusChanged: %s", parametersString.c_str());
            bool activated = false;
            if (parameters.HasLabel("newActivationStatus") && parameters["newActivationStatus"].String() == "activated")
            {
                activated = true;
            }

            mMutex.lock();
            mAttributes.activated = activated;
            mAttributes.xboAccountId.clear();
            mAttributes.xboDeviceId.clear();
            mMutex.unlock();
        }

        void SiftConfig::SyncPrivacyExclusionPolicies()
        {
            std::map<std::string, SiftConfig::PrivacyExclusionPolicy> newPolicies;

            auto privacy = mShell->QueryInterfaceByCallsign<Exchange::IPrivacy>(PRIVACY_CALLSIGN);
            if (privacy != nullptr)
            {
                Exchange::IPrivacy::IStringIterator *polices = nullptr;
                uint32_t result = privacy->GetExclusionPolicies(polices);
                if (result == Core::ERROR_NONE)
                {
                    if (polices != nullptr)
                    {
                        std::string policyName;
                        while (polices->Next(policyName))
                        {
                            Exchange::IPrivacy::IStringIterator *dataEvents = nullptr;
                            Exchange::IPrivacy::IStringIterator *entityReference = nullptr;
                            bool derivativePropagation = false;
                            uint32_t result = privacy->GetDataForExclusionPolicy(policyName, dataEvents, entityReference, derivativePropagation);
                            if (result != Core::ERROR_NONE)
                            {
                                LOGERR("Failed to get ExclusionPolicy %s", policyName.c_str());
                                continue;
                            }
                            PrivacyExclusionPolicy policy;
                            policy.derivativePropagation = derivativePropagation;
                            std::string dataEvent;
                            while (dataEvents && dataEvents->Next(dataEvent))
                            {
                                policy.dataEvents.insert(dataEvent);
                            }

                            std::string entityRef;
                            while (entityReference && entityReference->Next(entityRef))
                            {
                                policy.entityReference.insert(entityRef);
                            }

                            newPolicies[policyName] = policy;

                            if (dataEvents)
                            {
                                dataEvents->Release();
                            }

                            if (entityReference)
                            {
                                entityReference->Release();
                            }
                            LOGINFO("Got ExclusionPolicy %s", policyName.c_str());
                        }
                        polices->Release();
                    }
                }
                else
                {
                    LOGERR("Failed to get ExclusionPolicies");
                }
                privacy->Release();
            }
            else
            {
                LOGERR("Failed to get Privacy interface");
            }

            std::unique_lock<std::mutex> lock(mMutex);
            mPrivacyExclusionPolicies = newPolicies;
        }

        void SiftConfig::ActivatePlugin(PluginHost::IShell *shell, const char *callSign)
        {
            JsonObject joParams;
            joParams.Set("callsign", callSign);
            JsonObject joResult;

            if (!IsPluginActivated(shell, callSign))
            {
                LOGINFO("Activating %s", callSign);
                auto thunderController = std::unique_ptr<JSONRPCDirectLink>(new JSONRPCDirectLink(shell));
                if (thunderController != nullptr)
                {
                    uint32_t status = thunderController->Invoke<JsonObject, JsonObject>(2000, "activate", joParams, joResult);
                    string strParams;
                    string strResult;
                    joParams.ToString(strParams);
                    joResult.ToString(strResult);
                    LOGINFO("Called method %s, with params %s, status: %d, result: %s", "activate", strParams.c_str(), status, strResult.c_str());
                    if (status == Core::ERROR_NONE)
                    {
                        LOGINFO("%s Plugin activation status ret: %d ", callSign, status);
                    }
                }
                else
                {
                    LOGERR("Failed to create JSONRPCDirectLink");
                }
            }
        }

        bool SiftConfig::IsPluginActivated(PluginHost::IShell *shell, const char *callSign)
        {
            PluginHost::IShell::state state = PluginHost::IShell::DEACTIVATED;
            auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callSign);
            if (interface == nullptr)
            {
                LOGERR("No IShell for %s", callSign);
            }
            else
            {
                state = interface->State();
                LOGINFO("IShell state %d for callsing %s", state, callSign);
                interface->Release();
            }

            return state == PluginHost::IShell::ACTIVATED;
        }

        std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> SiftConfig::GetThunderControllerClient(std::string callsign)
        {
            std::string thunderAccessValue = THUNDER_ACCESS_DEFAULT_VALUE;
            char *thunderAccessValueEnv = getenv("THUNDER_ACCESS_VALUE");
            if (NULL != thunderAccessValueEnv)
            {
                thunderAccessValue = thunderAccessValueEnv;
            }

            std::string query = "token=" + sThunderSecurityToken;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(thunderAccessValue)));
            return std::make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>(callsign, "", false, query);
        }

        bool SiftConfig::GetFileRegex(const std::string &fileName, const std::string &regex, std::string &result)
        {
            std::regex pattern(regex);
            std::ifstream file(fileName);
            if (file) {
                string line;
                while (std::getline(file, line)) {
                    std::smatch sm;
                    if (std::regex_match(line, sm, pattern)) {
                        ASSERT(sm.size() == 2);
                        result = sm[1];
                        return true;
                    }
                }
            }

            return false;
        }

        void SiftConfig::MonitorKeys::ValueChanged(const string& ns, const string& key, const string& value)
        {
            auto it = mCallbacks.find(ns);
            if (it != mCallbacks.end())
            {
                auto it2 = it->second.find(key);
                if (it2 != it->second.end())
                {
                    LOGINFO("ValueChanged %s, %s, %s",ns.c_str(), key.c_str(), value.c_str());
                    it2->second(value);
                }
            }
        }

        void SiftConfig::MonitorKeys::StorageExceeded()
        {
        }

        void SiftConfig::MonitorKeys::RegisterCallback(const string& ns, const string& key, Callback callback)
        {
            mCallbacks[ns][key] = callback;
        }
    }
}
