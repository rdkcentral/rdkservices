#include "SiftConfig.h"
#include "UtilsLogging.h"

#include <algorithm>
#include <cctype> 

#define AUTHSERVICE_CALLSIGN "org.rdk.AuthService"
#define SYSTEM_CALLSIGN "org.rdk.System"
#define PERSISTENT_STORE_CALLSIGN "org.rdk.PersistentStore"
#define PERSISTENT_STORE_ANALYTICS_NAMESPACE "Analytics"
#define PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE "accountProfile"
#define JSONRPC_THUNDER_TIMEOUT 20000

#include <fstream>

namespace WPEFramework
{
    namespace Plugin
    {
        static std::string sThunderSecurityToken;

        class AnalyticsConfig : public Core::JSON::Container {
        private:
            AnalyticsConfig(const AnalyticsConfig&) = delete;
            AnalyticsConfig& operator=(const AnalyticsConfig&) = delete;

        public:
            class SiftConfig : public Core::JSON::Container {
            public:
                SiftConfig(const SiftConfig&) = delete;
                SiftConfig& operator=(const SiftConfig&) = delete;

                SiftConfig()
                    : Core::JSON::Container()
                    , CommonSchema()
                    , Env()
                    , ProductName()
                    , LoggerName()
                    , LoggerVersion()
                {
                    Add(_T("commonschema"), &CommonSchema);
                    Add(_T("env"), &Env);
                    Add(_T("productname"), &ProductName);
                    Add(_T("loggername"), &LoggerName);
                    Add(_T("loggerversion"), &LoggerVersion);
                }
                ~SiftConfig() = default;

            public:
                Core::JSON::String CommonSchema;
                Core::JSON::String Env;
                Core::JSON::String ProductName;
                Core::JSON::String LoggerName;
                Core::JSON::String LoggerVersion;
            };

           
        public:
            AnalyticsConfig()
                : Core::JSON::Container()
                , DeviceOsName()
                , Sift()
            {
                Add(_T("deviceosname"), &DeviceOsName);
                Add(_T("sift"), &Sift);
            }
            ~AnalyticsConfig()
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

        SiftConfig::SiftConfig(PluginHost::IShell *shell) : mInitializationThread(),
                                                            mMonitorKeys(),
                                                            mMutex(),
                                                            mConfig(),
                                                            mShell(shell)
        {
            ASSERT(shell != nullptr);
            InitializeKeysMap();
            ParsePluginConfig();
            TriggerInitialization();
        }

        SiftConfig::~SiftConfig()
        {
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
        }

        bool SiftConfig::Get(SiftConfig::Config &config)
        {
            //Read /tmp/sift_session evrytime to get the latest session id
            std::ifstream sessionFile("/tmp/sift_session");
            if (sessionFile.is_open())
            {
                std::getline(sessionFile, mConfig.sessionId);
                sessionFile.close();
            }

            // Get latest values from AuthService
            GetAuthServiceValues();

            mMutex.lock();

            bool activatedValid = mConfig.activated? 
                (!mConfig.xboDeviceId.empty() && !mConfig.xboAccountId.empty()) : true; 

            //Sift 2.0 required fields
            bool valid = ( !mConfig.commonSchema.empty()
                && !mConfig.productName.empty()
                && !mConfig.productVersion.empty()
                && !mConfig.loggerName.empty()
                && !mConfig.loggerVersion.empty()
                && !mConfig.partnerId.empty()
                && activatedValid
                && !mConfig.deviceModel.empty()
                && !mConfig.deviceType.empty()
                && !mConfig.deviceTimeZone.empty()
                && !mConfig.deviceOsName.empty()
                && !mConfig.deviceOsVersion.empty()
                && !mConfig.platform.empty()
                && !mConfig.deviceManufacturer.empty()
                && !mConfig.sessionId.empty()
                && !mConfig.proposition.empty()
                && !mConfig.deviceSerialNumber.empty()
                && !mConfig.deviceMacAddress.empty() );

            LOGINFO( " commonSchema: %s,"
            " productName: %s,"
            " productVersion: %s,"
            " loggerName: %s,"
            " loggerVersion: %s,"
            " partnerId: %s,"
            " activatedValid %d,"
            " deviceModel: %s,"
            " deviceType: %s,"
            " deviceTimeZone: %s,"
            " deviceOsName: %s,"
            " deviceOsVersion: %s,"
            " platform: %s,"
            " deviceManufacturer: %s,"
            " sessionId: %s,"
            " proposition: %s,"
            " deviceSerialNumber: %s,"
            " deviceMacAddress: %s,", 
             mConfig.commonSchema.c_str(),
             mConfig.productName.c_str(),
             mConfig.productVersion.c_str(),
             mConfig.loggerName.c_str(),
             mConfig.loggerVersion.c_str(),
             mConfig.partnerId.c_str(),
             activatedValid,
             mConfig.deviceModel.c_str(),
             mConfig.deviceType.c_str(),
             mConfig.deviceTimeZone.c_str(),
             mConfig.deviceOsName.c_str(),
             mConfig.deviceOsVersion.c_str(),
             mConfig.platform.c_str(),
             mConfig.deviceManufacturer.c_str(),
             mConfig.sessionId.c_str(),
             mConfig.proposition.c_str(),
             mConfig.deviceSerialNumber.c_str(),
             mConfig.deviceMacAddress.c_str());

            if (valid)
            {
                if (mConfig.deviceType == "TV")
                {
                    mConfig.deviceType = "IPTV";
                }
                else if (mConfig.deviceType == "IPSETTOPBOX")
                {
                    mConfig.deviceType = "IPSTB";
                }
                config = mConfig;
            }
            mMutex.unlock();
            return valid;
        }

        void SiftConfig::TriggerInitialization()
        {
            mInitializationThread = std::thread(&SiftConfig::Initialize, this);
            mInitializationThread.detach();
        }

        void SiftConfig::InitializeKeysMap()
        {
            //Based on SIFT 2.0 properties
            //Device info based on SIFT 2.0 properties
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["deviceHardwareModel"] = &mConfig.deviceModel;
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["deviceType"] = &mConfig.deviceType;
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["devicePlatform"] = &mConfig.platform;//TODO: in ripple equal to 'proposition'
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["modelNumber"] = &mConfig.deviceOsVersion;
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["manufacturer"] = &mConfig.deviceManufacturer;
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["serialNumber"] = &mConfig.deviceSerialNumber;
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["macAddress"] = &mConfig.deviceMacAddress;
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["entertainmentOSVersion"] = &mConfig.productVersion;
            mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["proposition"] = &mConfig.proposition;
            mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["retailer"] = &mConfig.retailer;
            mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["jvagent"] = &mConfig.jvAgent;
            mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["coam"] = &mConfig.coam;
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["country"] = &mConfig.country;//TODO
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["region"] = &mConfig.region;//TODO
            mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["accountType"] = &mConfig.accountType;
            mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["operator"] = &mConfig.accountOperator;
            mKeysMap[PERSISTENT_STORE_ACCOUNT_PROFILE_NAMESPACE]["detailType"] = &mConfig.accountDetailType;

            //TODO: Values provided by AS but should be provided by RDK
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["deviceTimeZone"] = &mConfig.deviceTimeZone;



            //TODO: Sift cloud configuration - move to plugin config? (at least url)
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["sift_url"] = &mConfig.url;
            mKeysMap[PERSISTENT_STORE_ANALYTICS_NAMESPACE]["sift_xapikey"] = &mConfig.apiKey; 
        }

        void SiftConfig::ParsePluginConfig()
        {
            ASSERT(mShell != nullptr);
            std::string configLine = mShell->ConfigLine();
            Core::OptionalType<Core::JSON::Error> error;
            AnalyticsConfig config;

            if (config.FromString(configLine, error) == false)
            {
                SYSLOG(Logging::ParsingError,
                       (_T("Failed to parse config line, error: '%s', config line: '%s'."),
                        (error.IsSet() ? error.Value().Message().c_str() : "Unknown"),
                        configLine.c_str())); 
            }
            else
            {
                mConfig.commonSchema = config.Sift.CommonSchema.Value();
                mConfig.env = config.Sift.Env.Value();
                mConfig.productName = "entos-immerse";//config.Sift.ProductName.Value();
                mConfig.loggerName = config.Sift.LoggerName.Value();
                mConfig.loggerVersion = config.Sift.LoggerVersion.Value();
                mConfig.deviceOsName = "rdk";//config.DeviceOsName.Value();
                SYSLOG(Logging::Startup, (_T("Parsed config: '%s', '%s', '%s', '%s', '%s', '%s'."),
                                          mConfig.commonSchema.c_str(),
                                          mConfig.env.c_str(),
                                          mConfig.productName.c_str(),
                                          mConfig.loggerName.c_str(),
                                          mConfig.loggerVersion.c_str(),
                                          mConfig.deviceOsName.c_str()
                    ));
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
            mConfig.authenticated = false;
            mMutex.unlock();

            //Activate AuthService plugin if needed
            if (IsPluginActivated(mShell, AUTHSERVICE_CALLSIGN) == false)
            {
                ActivatePlugin(mShell, AUTHSERVICE_CALLSIGN);
            }
 
            GetAuthServiceValues();

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
                    mConfig.env = response["build_type"].String();
                    std::transform(mConfig.env.begin(), mConfig.env.end(), mConfig.env.begin(),
                            [](unsigned char c){ return std::tolower(c); });
                    mMutex.unlock();
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

        void SiftConfig::GetAuthServiceValues()
        {
            auto authServiceLink = std::unique_ptr<JSONRPCDirectLink>(new JSONRPCDirectLink(mShell, AUTHSERVICE_CALLSIGN));
            if (authServiceLink == nullptr)
            {
                LOGERR("Failed to create JSONRPCDirectLink");
                return;
            }

            JsonObject params;
            JsonObject response;

            uint32_t result = authServiceLink->Invoke<JsonObject, JsonObject>(JSONRPC_THUNDER_TIMEOUT, "getDeviceId", params, response);
            if (result == Core::ERROR_NONE && response.HasLabel("partnerId"))
            {
                mMutex.lock();
                mConfig.partnerId = response["partnerId"].String();
                LOGINFO("Got partnerId %s", mConfig.partnerId.c_str());
                mMutex.unlock();
            }

            // get activation status from AuthService.getActivationStatus
            result = authServiceLink->Invoke<JsonObject, JsonObject>(JSONRPC_THUNDER_TIMEOUT, "getActivationStatus", params, response);
            if (result == Core::ERROR_NONE && response.HasLabel("status") && response["status"].String() == "activated")
            {
                LOGINFO("Device is activated");
                // get xboAccountId from AuthService.getServiceAccountId
                result = authServiceLink->Invoke<JsonObject, JsonObject>(JSONRPC_THUNDER_TIMEOUT, "getServiceAccountId", params, response);
                if (result == Core::ERROR_NONE && response.HasLabel("serviceAccountId"))
                {
                    mMutex.lock();
                    mConfig.xboAccountId = response["serviceAccountId"].String();
                    mMutex.unlock();
                    LOGINFO("Got xboAccountId %s", mConfig.xboAccountId.c_str());
                }

                // get xboDeviceId from AuthService.getXDeviceId
                result = authServiceLink->Invoke<JsonObject, JsonObject>(JSONRPC_THUNDER_TIMEOUT, "getXDeviceId", params, response);
                if (result == Core::ERROR_NONE && response.HasLabel("xDeviceId"))
                {
                    mMutex.lock();
                    mConfig.xboDeviceId = response["xDeviceId"].String();
                    mMutex.unlock();
                    LOGINFO("Got xboDeviceId %s", mConfig.xboDeviceId.c_str());
                }

                mMutex.lock();
                mConfig.activated = true;
                mMutex.unlock();
            }
            else
            {
                mMutex.lock();
                mConfig.activated = false;
                mMutex.unlock();
            }
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
            std::string callsign = PERSISTENT_STORE_CALLSIGN;
            auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
            if (interface == nullptr)
            {
                LOGERR("No IShell for %s", callsign.c_str());
            }
            else
            {
                state = interface->State();
                LOGINFO("IShell state %d for callsing %s", state, callsign.c_str());
                interface->Release();
            }

            return state == PluginHost::IShell::ACTIVATED;
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
