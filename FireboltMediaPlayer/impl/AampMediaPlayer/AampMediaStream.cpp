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

#include "AampMediaStream.h"

#include "UtilsLogging.h"

// Use macros to avoid unnecessary repetition of code. 
#define ADD_SETTER_INT(name, fn)                                                                                       \
    m_setters[#name] = [] (PlayerInstanceAAMP* aamp, string const& label, Variant const& value) {                      \
        int setting;                                                                                                   \
        if (!extractSetting(label, value, setting))                                                                    \
            return false;                                                                                              \
        LOGINFO("Invoking PlayerInstanceAAMP::%s(%d)", #fn, setting);                                                  \
        aamp->fn(setting);                                                                                             \
        return true;                                                                                                   \
    };
#define ADD_SETTER_FLOAT(name, fn)                                                                                     \
    m_setters[#name] = [] (PlayerInstanceAAMP* aamp, string const& label, Variant const& value) {                      \
        double setting;                                                                                                \
        if (!extractSetting(label, value, setting))                                                                    \
            return false;                                                                                              \
        LOGINFO("Invoking PlayerInstanceAAMP::%s(%lf)", #fn, setting);                                                 \
        aamp->fn(setting);                                                                                             \
        return true;                                                                                                   \
    };
#define ADD_SETTER_BOOLEAN(name, fn)                                                                                   \
    m_setters[#name] = [] (PlayerInstanceAAMP* aamp, string const& label, Variant const& value) {                      \
        bool setting;                                                                                                  \
        if (!extractSetting(label, value, setting))                                                                    \
            return false;                                                                                              \
        LOGINFO("Invoking PlayerInstanceAAMP::%s(%s)", #fn, setting ? "true" : "false");                               \
        aamp->fn(setting);                                                                                             \
        return true;                                                                                                   \
    };
#define ADD_SETTER_STRING(name, fn)                                                                                    \
    m_setters[#name] = [] (PlayerInstanceAAMP* aamp, string const& label, Variant const& value) {                      \
        string setting;                                                                                                \
        if (!extractSetting(label, value, setting))                                                                    \
            return false;                                                                                              \
        LOGINFO("Invoking PlayerInstanceAAMP::%s(\"%s\")", #fn, setting.c_str());                                      \
        aamp->fn(setting.c_str());                                                                                     \
        return true;                                                                                                   \
    };

/*
 * The value is a string or null. If empty string or null then use C NULL string. Some aamp functions interpret the 
 * NULL string as a special case.
 * 
 * A null value in a JSON document appears to be an empty string in WPEFramework's "Variant" type.
 *
 */
#define ADD_SETTER_STRINGNULL(name, fn)                                                                                \
    m_setters[#name] = [] (PlayerInstanceAAMP* aamp, string const& label, Variant const& value) {                      \
        char const* rawSetting = NULL;                                                                                 \
        string stdSetting;                                                                                             \
        if (value.Content() != Variant::type::EMPTY) {                                                                 \
            if (!extractSetting(label, value, stdSetting))                                                             \
                return false;                                                                                          \
            if (!stdSetting.empty())                                                                                   \
                rawSetting = stdSetting.c_str();                                                                       \
        }                                                                                                              \
        if (rawSetting == NULL)                                                                                        \
            LOGINFO("Invoking PlayerInstanceAAMP::%s(NULL)", #fn);                                                     \
        else                                                                                                           \
            LOGINFO("Invoking PlayerInstanceAAMP::%s(\"%s\")", #fn, rawSetting);                                       \
        aamp->fn(rawSetting);                                                                                          \
        return true;                                                                                                   \
    };
#define ADD_SETTER_LICENSESERVERURL(name, licenseType)                                                                 \
    m_setters[#name] = [] (PlayerInstanceAAMP* aamp, string const& label, Variant const& value) {                      \
        string setting;                                                                                                \
        if (!extractSetting(label, value, setting))                                                                    \
            return false;                                                                                              \
        LOGINFO("Invoking PlayerInstanceAAMP::SetLicenseServerURL(\"%s\", %s)", setting.c_str(), #licenseType);        \
        aamp->SetLicenseServerURL(setting.c_str(), licenseType);                                                       \
        return true;                                                                                                   \
    };
#define ADD_SETTER_UNSUPPORTED(name)                                                                                   \
    m_setters[#name] = [] (PlayerInstanceAAMP*, string const& label, Variant const&) {                                 \
        LOGWARN("The configuration setting '%s' is currently unsupported", label.c_str());                             \
        return false;                                                                                                  \
    };

namespace {
    using namespace WPEFramework;
    using namespace WPEFramework::Core::JSON;

    /**
     * @brief Support for (generic) main configuration and DRM settings used by AAMP.
     * 
     * Derived classes are singletons and specify the settings the actual settings and support special cases.
     * 
     */
    class Settings
    {
    public:
        /**
         * @brief Apply a configuration setting to AAMP.
         * 
         * @param aamp  The instance of AAMP to apply to.
         * @param name  The name of the setting.
         * @param value The value to apply as a JSON Variant.
         * @return      Whether the setting was successfully applied.
         * 
         */
        bool apply(PlayerInstanceAAMP* aamp, string const& name, Variant const& value)
        {
            // Do we have a setting with this name?
            SetterMap::const_iterator it = m_setters.find(name);
            if (it == m_setters.end())
            {
                LOGERR("Settings::apply - failed to find setting '%s'", name.c_str());
                return false;
            }

            // Use associated lambda to set the value
            return (*it).second(aamp, name, value);
        }

        /**
         * @brief Extract Boolean value from Variant.
         * 
         * @param name              The name of the setting for this value.
         * @param variantValue      The variant value from JSON.
         * @param[out] booleanValue The extracted value. 
         * @return                  Whether suitable value could be extracted.
         * 
         */
        static bool extractSetting(string const& name, Variant const& variantValue, bool& booleanValue)
        {
            switch (variantValue.Content())
            {
                case Variant::type::BOOLEAN:
                booleanValue = variantValue.Boolean();
                return true;

                case Variant::type::NUMBER:
                {
                    // Use double as it maybe a floating point number
                    double const tmp = variantValue.Number();
                    if ((tmp != 0.0) && (tmp != 1.0)) {
                        LOGERR("Settings::extraSetting - '%s' Boolean setting is numeric but not zero or unity", name.c_str());
                        return false;
                    } else {
                        booleanValue = (tmp != 0.0);
                        return true;
                    }
                }
            
                case Variant::type::STRING:
                {
                    string tmp = variantValue.String();
                    std::transform(tmp.begin(), tmp.end(), tmp.begin(), tolower);
                    if ((tmp != "true") && (tmp != "false")) {
                        LOGERR("Settings::extraSetting - '%s' Boolean setting is string but not 'true'' or 'false''", name.c_str());
                        return false;
                    } else {
                        booleanValue = (tmp == "true");
                        return true;
                    }
                }

                default:
                LOGERR("Settings::extraSetting - '%s' setting is not Boolean, numeric or string", name.c_str());
                return false;
            }
        }

        /**
         * @brief Extract integer value from Variant.
         * 
         * @param name          The name of the setting for this value.
         * @param variantValue  The variant value from JSON.
         * @param[out] intValue The extracted value. 
         * @return              Whether suitable value could be extracted.
         * 
         */
        static bool extractSetting(string const& name, Variant const& variantValue, int& intValue)
        {
            switch (variantValue.Content())
            {
                case Variant::type::NUMBER:
                intValue = variantValue.Number();
                return true;
            
                case Variant::type::STRING:
                intValue = strtol(variantValue.String().c_str(), NULL, 10);
                return true;

                default:
                LOGERR("Settings::extraSetting - '%s' setting is not numeric or string", name.c_str());
                return false;
            }
        }

        /**
         * @brief Extract floating point value from Variant.
         * 
         * @param name            The name of the setting for this value.
         * @param variantValue    The variant value from JSON.
         * @param[out] floatValue The extracted value. 
         * @return                Whether suitable value could be extracted.
         * 
         */
        static bool extractSetting(string const& name, Variant const& variantValue, double& floatValue)
        {
            switch (variantValue.Content())
            {
                case Variant::type::NUMBER:
                floatValue = variantValue.Number();
                return true;
            
                case Variant::type::STRING:
                floatValue = strtod(variantValue.String().c_str(), NULL);
                return true;

                default:
                LOGERR("Settings::extraSetting - '%s' setting is not numeric or string", name.c_str());
                return false;
            }
        }

        /**
         * @brief Extract string value from Variant.
         * 
         * @param name             The name of the setting for this value.
         * @param variantValue     The variant value from JSON.
         * @param[out] stringValue The extracted value. 
         * @return                 Whether suitable value could be extracted.
         * 
         */
        static bool extractSetting(string const& name, Variant const& variantValue, string& stringValue)
        {
            if (variantValue.Content() == Variant::type::STRING) {
                stringValue = variantValue.String();
                return true;
            } else {
                LOGERR("Settings::extraSetting - '%s' setting is not a string", name.c_str());
                return false;
            }
        }

        // No assignment and virtual destructor; abstract so no construction possible
        Settings& operator=(Settings const&) = delete;
        virtual ~Settings() = default;

    protected:
        /**
         * @brief Populate for main configuration or DRM.
         *
         */
        virtual void populate() = 0;

        using SetterFunction = std::function<bool(PlayerInstanceAAMP* aamp, string const& name, Variant const& value)>;
        using SetterMap = std::map<string, SetterFunction>; 

        SetterMap m_setters;
    };

    /**
     * @brief DRM related settings.
     *
     */
    class DRMSettings: public Settings 
    {
    public:
        /**
         * @brief Get the DRM settings.
         *
         * @return DRM settings.
         */
        static DRMSettings& getInstance()
        {
            static DRMSettings instance;
            if (instance.m_setters.empty())
                instance.populate();
            return instance;
        }

        using Settings::apply;

        /**
         * @brief Apply multiple configuration settings to AAMP.
         *
         * Only used for DRM settings as main configuration settings are more complicated.
         *
         * @param aamp   The instance of aamp to apply to.
         * @param config A JSON object containing labels and values which are the settings.
         * @return       True if any setting could be applied.
         *
         */
        bool apply(PlayerInstanceAAMP* aamp, JsonObject const& config)
        {
            string const idLabel("id");
            JsonObject::Iterator it = config.Variants();
            bool success = false;
            while (it.Next())
            {
                string const label = it.Label();

                // Ignore 'id' as it's not a configuration setting
                if (label == idLabel)
                    continue;

                if (apply(aamp, label, it.Current()))
                    success = true;
            }

            return success;
        }

    protected:
        /**
        * @brief Populate the supported settings.
        * 
        */
        virtual void populate()
        {
            ADD_SETTER_LICENSESERVERURL(com.microsoft.playready, eDRM_PlayReady)
            ADD_SETTER_LICENSESERVERURL(com.widevine.alpha, eDRM_WideVine)
            ADD_SETTER_LICENSESERVERURL(org.w3.clearkey, eDRM_ClearKey)
            m_setters["preferredKeysystem"] = [] (PlayerInstanceAAMP* aamp, string const& label, Variant const& value) {
                string setting;
                if (!extractSetting(label, value, setting))
                    return false;

                if (setting == "com.microsoft.playready") {
                    LOGINFO("Invoking PlayerInstanceAAMP::SetPreferredDRM(eDRM_PlayReady)");
                    aamp->SetPreferredDRM(eDRM_PlayReady);
                } else if (setting == "com.widevine.alpha") {
                    LOGINFO("Invoking PlayerInstanceAAMP::SetPreferredDRM(eDRM_WideVine)");
                    aamp->SetPreferredDRM(eDRM_WideVine);
                } else {
                    LOGERR("the '%s' setting has unsupported value '%s'", label.c_str(), setting.c_str());
                    return false;
                }

                return true;
            };
        }

    private:
        DRMSettings() = default;
        DRMSettings(DRMSettings const&) = delete;
    };

    /**
     * @brief Configuration related settings.
     *
     */
    class ConfigurationSettings: public Settings 
    {
    public:
        /**
         * @brief Get the configuration settings.
         *
         * @return Configuration settings.
         */
        static ConfigurationSettings& getInstance()
        {
            static ConfigurationSettings instance;
            if (instance.m_setters.empty())
                instance.populate();
            return instance;
        }

    protected:
        /**
        * @brief Populate the supported settings.
        * 
        */
        virtual void populate()
        {
            ADD_SETTER_INT(initialBitrate, SetInitialBitrate)
            ADD_SETTER_INT(initialBitrate4K, SetInitialBitrate4K)
            m_setters["offset"] = [] (PlayerInstanceAAMP* aamp, string const& label, Variant const& value) {
                int setting;
                if (!extractSetting(label, value, setting))
                    return false;
                
                // More of an action rather than a setting but this is what UVE-JS does
                LOGINFO("Invoking PlayerInstanceAAMP::Seek(%d)", setting);
                aamp->Seek(setting);
                return true;
            };
            ADD_SETTER_FLOAT(networkTimeout, SetNetworkTimeout)
            ADD_SETTER_FLOAT(manifestTimeout, SetManifestTimeout)
            ADD_SETTER_FLOAT(playlistTimeout, SetPlaylistTimeout)
            ADD_SETTER_INT(downloadBuffer, SetDownloadBufferSize)
            ADD_SETTER_INT(minBitrate, SetMinimumBitrate)
            ADD_SETTER_INT(maxBitrate, SetMaximumBitrate)
            ADD_SETTER_STRINGNULL(preferredAudioLanguage, SetPreferredLanguages)
            ADD_SETTER_UNSUPPORTED(timeShiftBufferLength);
            ADD_SETTER_BOOLEAN(stereoOnly, SetStereoOnlyPlayback)
            ADD_SETTER_INT(liveOffset, SetLiveOffset)
            ADD_SETTER_BOOLEAN(bulkTimedMetadata, SetBulkTimedMetaReport)
            ADD_SETTER_STRING(networkProxy, SetNetworkProxy)
            ADD_SETTER_STRING(licenseProxy, SetLicenseReqProxy)
            ADD_SETTER_INT(downloadStallTimeout, SetDownloadStallTimeout)
            ADD_SETTER_INT(downloadStartTimeout, SetDownloadStartTimeout)
            ADD_SETTER_STRING(preferredSubtitleLanguage, SetPreferredSubtitleLanguage)
            ADD_SETTER_BOOLEAN(parallelPlaylistDownload, SetParallelPlaylistDL)
            ADD_SETTER_BOOLEAN(parallelPlaylistRefresh, SetParallelPlaylistRefresh)
            ADD_SETTER_BOOLEAN(useAverageBandwidth, SetAvgBWForABR)
            ADD_SETTER_INT(preCachePlaylistTime, SetPreCacheTimeWindow)
            m_setters["progressReportingInterval"] = [] (PlayerInstanceAAMP* aamp, string const& label, Variant const& value) {
                int setting;
                if (!extractSetting(label, value, setting))
                    return false;

                // Interface is in seconds but AAMP expects milliseonds
                setting *= 1000;
                LOGINFO("Invoking PlayerInstanceAAMP::SetReportInterval(%d)", setting);
                aamp->SetReportInterval(setting);
                return true;
            };
            ADD_SETTER_BOOLEAN(useRetuneForUnpairedDiscontinuity, SetRetuneForUnpairedDiscontinuity)
            ADD_SETTER_INT(drmDecryptFailThreshold, SetSegmentDecryptFailCount)
            ADD_SETTER_INT(initialBuffer, SetInitialBufferDuration)
            ADD_SETTER_BOOLEAN(useMatchingBaseUrl, SetMatchingBaseUrlConfig)
            ADD_SETTER_INT(initFragmentRetryCount, SetInitFragTimeoutRetryCount)
            ADD_SETTER_BOOLEAN(nativeCCRendering, SetNativeCCRendering)
            // langCodePreference and descriptiveTrackName - special handling
            ADD_SETTER_STRING(authToken, SetSessionToken);
            ADD_SETTER_BOOLEAN(useRetuneForGstInternalError, SetRetuneForGSTInternalError)
            ADD_SETTER_BOOLEAN(reportVideoPTS, SetReportVideoPTS)
            ADD_SETTER_BOOLEAN(propagateUriParameters, SetPropagateUriParameters)
            ADD_SETTER_BOOLEAN(enableSeekableRange, EnableSeekableRange) 
            ADD_SETTER_INT(maxPlaylistCacheSize, SetMaxPlaylistCacheSize)
            ADD_SETTER_BOOLEAN(setLicenseCaching, SetLicenseCaching)
            ADD_SETTER_BOOLEAN(persistBitrateOverSeek, PersistBitRateOverSeek)
            ADD_SETTER_BOOLEAN(sslVerifyPeer, SetSslVerifyPeerConfig)
            ADD_SETTER_INT(livePauseBehavior, SetPausedBehavior)
            ADD_SETTER_BOOLEAN(limitResolution, SetOutputResolutionCheck)

            // The following are undocumented but supported by UVE-JS
            ADD_SETTER_UNSUPPORTED(playbackBuffer)
            ADD_SETTER_STRINGNULL(preferredAudioRendition, SetPreferredRenditions)
            ADD_SETTER_STRINGNULL(preferredAudioCodec, SetPreferredCodec)
            m_setters["drmConfig"] = [] (PlayerInstanceAAMP* aamp, string const& label, Variant const& value) {
                if (value.Content() != Variant::type::OBJECT) {
                    LOGERR("Settings::populate - '%s' setting is not a JSON object", label.c_str());
                    return false;
                }

                // Apply all these DRM settings
                return DRMSettings::getInstance().apply(aamp, value.Object());
            };
            ADD_SETTER_BOOLEAN(asyncTune, SetAsyncTuneConfig)
            ADD_SETTER_BOOLEAN(useWesterosSink, SetWesterosSinkConfig)
            ADD_SETTER_BOOLEAN(useNewABR, SetNewABRConfig)
            ADD_SETTER_INT(fragmentRetryLimit, SetRampDownLimit)
            ADD_SETTER_INT(initRampdownLimit, SetInitRampdownLimit)
            ADD_SETTER_INT(segmentInjectFailThreshold, SetSegmentInjectFailCount)
            ADD_SETTER_BOOLEAN(useNewAdBreaker, SetNewAdBreakerConfig)
            ADD_SETTER_INT(ceaFormat, SetCEAFormat)
            ADD_SETTER_INT(tuneEventConfig, SetTuneEventConfig)
            ADD_SETTER_INT(onTuneRate, SetRate)
            ADD_SETTER_BOOLEAN(useAbsoluteTimeline, SetUseAbsoluteTimeline)
        }

    private:
        ConfigurationSettings() = default;
        ConfigurationSettings(ConfigurationSettings const&) = delete;
    };
}

namespace WPEFramework {
    namespace Plugin {


        AampMediaStream::AampMediaStream()
        : _adminLock()
        , _notificationRelease() //Lock
        , _notification(nullptr)
        , _aampPlayer(nullptr)
        , _aampEventListener(nullptr)
        , _aampGstPlayerMainLoop(nullptr)
        {
            gst_init(0, nullptr);
            _aampPlayer = new PlayerInstanceAAMP();
            if(_aampPlayer == nullptr)
            {
                return;
            }

            _aampEventListener = new AampEventListener(*this);
            if(_aampEventListener == nullptr)
            {
                return;
            }
            _aampPlayer->RegisterEvents(_aampEventListener);
            _aampPlayer->SetReportInterval(1000 /* ms */);

            _aampGstPlayerMainLoop = g_main_loop_new(nullptr, false);

            // Run thread with _aampGstPlayerMainLoop
            Run();
        }

        AampMediaStream::~AampMediaStream()
        {
            _adminLock.Lock();
            if(!_aampPlayer)
            {
                return;
            }

             _notificationRelease.Lock();
            if (_notification != nullptr) {
                _notification->Release();
                _notification = nullptr;
            }
            _notificationRelease.Unlock();
            _adminLock.Unlock();
            _aampPlayer->Stop();
            Block();
            _adminLock.Lock();

            if(_aampGstPlayerMainLoop)
            {
                g_main_loop_quit(_aampGstPlayerMainLoop);
            }

            _adminLock.Unlock();
            Wait(Thread::BLOCKED | Thread::STOPPED, Core::infinite);

            _adminLock.Lock();
            _aampPlayer->Stop();
            _aampPlayer->RegisterEvents(nullptr);
            delete _aampEventListener;
            _aampEventListener = nullptr;
            delete _aampPlayer;
            _aampPlayer = nullptr;
            _adminLock.Unlock();
        }

        uint32_t AampMediaStream::Load(const string& url, bool autoPlay)
        {
            LOGINFO("Load with url=%s, autoPlay=%d", url.c_str(), autoPlay);
            _adminLock.Lock();

            ASSERT(_aampPlayer != nullptr);
            _aampPlayer->Tune(url.c_str(), autoPlay);

            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::SetRate(int32_t rate)
        {
            LOGINFO("SetRate with rate=%d", rate);
            double dRate = rate;
            dRate /= 100;

            _adminLock.Lock();
            ASSERT(_aampPlayer != nullptr);
            _aampPlayer->SetRate(dRate);
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::SeekTo(int32_t positionSec)
        {
            LOGINFO("SetPosition with pos=%d sec", positionSec);
            _adminLock.Lock();
            ASSERT(_aampPlayer != nullptr);
            _aampPlayer->Seek(static_cast<double>(positionSec));
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::Stop()
        {
            LOGINFO("Stop");
            _adminLock.Lock();
            ASSERT(_aampPlayer != nullptr);
            _aampPlayer->Stop();
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::InitConfig(const string& configurationJson)
        {
            LOGINFO("InitConfig with config=%s", configurationJson.c_str());
            _adminLock.Lock();

            int langCodePreferenceValue = -1;
            bool descriptiveTrackNameValue = false;
            bool enableVideoRectangleValue = false;
            bool enableVideoRectangleSet = false;

            // Iterate through the configuration settings
            string const idLabel("id");
            string const langCodePreferenceLabel("langCodePreference");
            string const descriptiveTrackNameLabel("descriptiveTrackName");
            string const enableVideoRectangleLabel("enableVideoRectangle");
            JsonObject const config(configurationJson);
            JsonObject::Iterator it = config.Variants();
            while (it.Next())
            {
                string const label = it.Label();

                // Ignore 'id' as it's not a configuration setting
                if (label == idLabel)
                    continue;

                // The settings 'langCodePreference' and 'descriptiveTrackName' are used together so don't apply immediately
                if (label == langCodePreferenceLabel)
                    (void)Settings::extractSetting(langCodePreferenceLabel, it.Current(), langCodePreferenceValue);
                else if (label == descriptiveTrackNameLabel)
                    (void)Settings::extractSetting(descriptiveTrackNameLabel, it.Current(), descriptiveTrackNameValue);
                else if (label == enableVideoRectangleLabel)
                    enableVideoRectangleSet = Settings::extractSetting(descriptiveTrackNameLabel, it.Current(), enableVideoRectangleValue);
                else
                    ConfigurationSettings::getInstance().apply(_aampPlayer, label, it.Current());
            }

            if (langCodePreferenceValue != -1) {
                LOGINFO("Invoking PlayerInstanceAAMP::SetLanguagueFormat(%d, %s)", langCodePreferenceValue, descriptiveTrackNameValue ? "true" : "false");
                _aampPlayer->SetLanguageFormat(LangCodePreference(langCodePreferenceValue), descriptiveTrackNameValue);
            }

            // UVE-JS delays this to the end so do the same
            if (enableVideoRectangleSet) {
                LOGINFO("Invoking PlayerInstanceAAMP::EnableVideoRectangle(%s)", enableVideoRectangleValue ? "true" : "false");
                _aampPlayer->EnableVideoRectangle(enableVideoRectangleValue);
            }

            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::InitDRMConfig(const string& configurationJson)
        {
            LOGINFO("InitDRMConfig with config=%s", configurationJson.c_str());
            _adminLock.Lock();

            JsonObject const config(configurationJson);
            DRMSettings::getInstance().apply(_aampPlayer, config);

            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::Register(Exchange::IMediaPlayer::IMediaStream::INotification* notification)
        {
            LOGINFO();
            _adminLock.Lock();

            _notificationRelease.Lock();
            if (_notification != nullptr) {
                _notification->Release();
            }
            _notificationRelease.Unlock();

            if (notification != nullptr) {
                notification->AddRef();
            }
            _notification = notification;
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        uint32_t AampMediaStream::Unregister(Exchange::IMediaPlayer::IMediaStream::INotification* notification)
        {
            LOGINFO();
            _adminLock.Lock();

            if (_notification != nullptr
                    && _notification == notification) {
                _notificationRelease.Lock();
                _notification->Release();
                _notificationRelease.Unlock();
                _notification = nullptr;
            }
            _adminLock.Unlock();
            return Core::ERROR_NONE;
        }

        void AampMediaStream::SendEvent(const string& eventName, const string& parameters)
        {
            LOGINFO("eventName=%s, parameters=%s", eventName.c_str(), parameters.c_str());
            _adminLock.Lock();
            if(!_notification)
            {
                LOGERR("SendEvent: notification callback is null");
                _adminLock.Unlock();
                return;
            }
            // deep copy
            string eventForNotification = eventName.c_str();
            string parametersForNotification = parameters.c_str();
            _adminLock.Unlock();

            _notificationRelease.Lock();
            _notification->Event(eventForNotification, parametersForNotification);
            _notificationRelease.Unlock();
        }

        // Thread overrides
        uint32_t AampMediaStream::Worker()
        {
            if (_aampGstPlayerMainLoop) {
                g_main_loop_run(_aampGstPlayerMainLoop); // blocks
                g_main_loop_unref(_aampGstPlayerMainLoop);
                _aampGstPlayerMainLoop = nullptr;
            }
            return WPEFramework::Core::infinite;
        }

    }
}
