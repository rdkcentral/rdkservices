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
#include "../Module.h"
#include <interfaces/IPlayerInfo.h>
#include <gst/gst.h>
#include "utils.h"

#include "audioOutputPort.hpp"
#include "audioOutputPortType.hpp"
#include "videoOutputPort.hpp"
#include "manager.hpp"
#include "host.hpp"
#include "exception.hpp"

#if defined(USE_IARM)
#include "libIBus.h"
#include "libIBusDaemon.h"
#include "dsMgr.h"
#endif

namespace WPEFramework {
namespace Plugin {

class PlayerInfoImplementation : public Exchange::IPlayerProperties, public Exchange::IPlayerAtmosProperties {
private:

    class GstUtils {
    private:
        struct FeatureListDeleter {
            void operator()(GList* p) { gst_plugin_feature_list_free(p); }
        };

        struct CapsDeleter {
            void operator()(GstCaps* p) { gst_caps_unref(p); }
        };
        typedef std::unique_ptr<GList, FeatureListDeleter> FeatureList;
        typedef std::unique_ptr<GstCaps, CapsDeleter> MediaTypes;

   public:
        GstUtils() = delete;
        GstUtils(const GstUtils&) = delete;
        GstUtils& operator= (const GstUtils&) = delete;

        template <typename C, typename CodecIteratorList>
        static bool GstRegistryCheckElementsForMediaTypes(C caps, CodecIteratorList& codecIteratorList) {

            auto type = std::is_same<C, VideoCaps>::value ? GST_ELEMENT_FACTORY_TYPE_MEDIA_VIDEO : GST_ELEMENT_FACTORY_TYPE_MEDIA_AUDIO;

            FeatureList decoderFactories{gst_element_factory_list_get_elements(GST_ELEMENT_FACTORY_TYPE_DECODER | type, GST_RANK_MARGINAL)};
            FeatureList parserFactories{gst_element_factory_list_get_elements(GST_ELEMENT_FACTORY_TYPE_PARSER | type, GST_RANK_MARGINAL)};

            FeatureList elements;
            for (auto index: caps) {

                MediaTypes mediaType{gst_caps_from_string(index.first.c_str())};
                if (elements = std::move(GstUtils::GstRegistryGetElementForMediaType(decoderFactories.get(), std::move(mediaType)))) {
                    codecIteratorList.push_back(index.second);

                } else if (elements = std::move(GstUtils::GstRegistryGetElementForMediaType(parserFactories.get(), std::move(mediaType)))) {

                    for (GList* iterator = elements.get(); iterator; iterator = iterator->next) {

                        GstElementFactory* gstElementFactory = static_cast<GstElementFactory*>(iterator->data);
                        const GList* padTemplates = gst_element_factory_get_static_pad_templates(gstElementFactory);

                        for (const GList* padTemplatesIterator = padTemplates; padTemplatesIterator; padTemplatesIterator = padTemplatesIterator->next) {
                            GstStaticPadTemplate* padTemplate = static_cast<GstStaticPadTemplate*>(padTemplatesIterator->data);

                            if (padTemplate->direction == GST_PAD_SRC) {
                                MediaTypes mediaTypes{gst_static_pad_template_get_caps(padTemplate)};
                                if (GstUtils::GstRegistryGetElementForMediaType(decoderFactories.get(), std::move(mediaTypes))) {
                                    codecIteratorList.push_back(index.second);
                                }
                            }
                        }
                    }
                 }
             }

             return (codecIteratorList.size() != 0);
         }

    private:
        static inline FeatureList GstRegistryGetElementForMediaType(GList* elementsFactories, MediaTypes&& mediaTypes) {
            FeatureList candidates{gst_element_factory_list_filter(elementsFactories, mediaTypes.get(), GST_PAD_SINK, false)};

            return std::move(candidates);
        }

    };

private:
    class AudioIteratorImplementation : public Exchange::IPlayerProperties::IAudioIterator {
    public:
        AudioIteratorImplementation() = delete;
        AudioIteratorImplementation(const AudioIteratorImplementation&) = delete;
        AudioIteratorImplementation& operator= (const AudioIteratorImplementation&) = delete;

        AudioIteratorImplementation(const std::list<AudioCodec>& codecs)
            : _index(0)
            , _codecs(codecs)
        {
        }
        virtual ~AudioIteratorImplementation()
        {
            _codecs.clear();
        }

    public:
        bool IsValid() const override
        {
            return ((_index != 0) && (_index <= _codecs.size()));
        }
        bool Next() override
        {
            if (_index == 0) {
                _index = 1;
            } else if (_index <= _codecs.size()) {
                _index++;
            }
            return (IsValid());
        }
        void Reset() override
        {
            _index = 0;
        }
        AudioCodec Codec() const
        {
            ASSERT(IsValid() == true);
            std::list<AudioCodec>::const_iterator codec = std::next(_codecs.begin(), _index - 1);
            ASSERT(*codec != AudioCodec::UNDEFINED);

            return *codec;
        }

        BEGIN_INTERFACE_MAP(AudioIteratorIImplementation)
        INTERFACE_ENTRY(Exchange::IPlayerProperties::IAudioIterator)
        END_INTERFACE_MAP

    private:
        uint16_t _index;
        std::list<AudioCodec> _codecs;
    };

    class VideoIteratorImplementation : public Exchange::IPlayerProperties::IVideoIterator {
    public:
        VideoIteratorImplementation() = delete;
        VideoIteratorImplementation(const VideoIteratorImplementation&) = delete;
        VideoIteratorImplementation& operator= (const VideoIteratorImplementation&) = delete;

        VideoIteratorImplementation(const std::list<VideoCodec>& codecs)
            : _index(0)
            , _codecs(codecs)
        {
        }
        virtual ~VideoIteratorImplementation()
        {
            _codecs.clear();
        }

    public:
        bool IsValid() const override
        {
            return ((_index != 0) && (_index <= _codecs.size()));
        }
        bool Next() override
        {
            if (_index == 0) {
                _index = 1;
            } else if (_index <= _codecs.size()) {
                _index++;
            }
            return (IsValid());
        }
        void Reset() override
        {
            _index = 0;
        }
        VideoCodec Codec() const
        {
            ASSERT(IsValid() == true);
            std::list<VideoCodec>::const_iterator codec = std::next(_codecs.begin(), _index - 1);

            ASSERT(*codec != VideoCodec::UNDEFINED);

            return *codec;
        }

        BEGIN_INTERFACE_MAP(VideoIteratorIImplementation)
        INTERFACE_ENTRY(Exchange::IPlayerProperties::IVideoIterator)
        END_INTERFACE_MAP

    private:
        uint16_t _index;
        std::list<VideoCodec> _codecs;
    };

    typedef std::map<const string, const Exchange::IPlayerProperties::IAudioIterator::AudioCodec> AudioCaps;
    typedef std::map<const string, const Exchange::IPlayerProperties::IVideoIterator::VideoCodec> VideoCaps;

public:
    PlayerInfoImplementation() {
        gst_init(0, nullptr);
        UpdateAudioCodecInfo();
        UpdateVideoCodecInfo();

         try
        {
#if defined(USE_IARM)
            Utils::IARM::init();
            IARM_Result_t res;
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_AUDIO_MODE, AudioModeHandler) );
#endif
            //TODO: this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
            device::Manager::Initialize();
            TRACE(Trace::Information, (_T("device::Manager::Initialize success")));
        }
        catch(...)
        {
            TRACE(Trace::Error, (_T("device::Manager::Initialize failed")));
        }
        PlayerInfoImplementation::_instance = this;
    }

    PlayerInfoImplementation(const PlayerInfoImplementation&) = delete;
    PlayerInfoImplementation& operator= (const PlayerInfoImplementation&) = delete;
    virtual ~PlayerInfoImplementation()
    {
        IARM_Result_t res;
        IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_AUDIO_MODE) );
        _audioCodecs.clear();
        _videoCodecs.clear();
        PlayerInfoImplementation::_instance = nullptr;
    }

public:
    Exchange::IPlayerProperties::IAudioIterator* AudioCodec() const override
    {
        return (Core::Service<AudioIteratorImplementation>::Create<Exchange::IPlayerProperties::IAudioIterator>(_audioCodecs));
    }
    Exchange::IPlayerProperties::IVideoIterator* VideoCodec() const override
    {
        return (Core::Service<VideoIteratorImplementation>::Create<Exchange::IPlayerProperties::IVideoIterator>(_videoCodecs));
    }

    uint32_t Register(Exchange::IPlayerAtmosProperties::INotification* sink)
    {
        _adminLock.Lock();

        // Make sure a sink is not registered multiple times.
        ASSERT(std::find(_notificationClients.begin(), _notificationClients.end(), sink) == _notificationClients.end());

        _notificationClients.push_back(sink);
        sink->AddRef();

        _adminLock.Unlock();

        TRACE_L1("IPlayerAtmosProperties : Registered a subscriber %p", sink);
        return (Core::ERROR_NONE);
    }

    uint32_t Unregister(Exchange::IPlayerAtmosProperties::INotification* sink)
    {
        _adminLock.Lock();

        std::list<IPlayerAtmosProperties::INotification*>::iterator index(std::find(_notificationClients.begin(), _notificationClients.end(), sink));

        // Make sure you do not unregister something you did not register !!!
        ASSERT(index != _notificationClients.end());

        if (index != _notificationClients.end()) {
            (*index)->Release();
            _notificationClients.erase(index);
            TRACE_L1("IPlayerAtmosProperties : unregistered a subscriber %p", sink);
        }

        _adminLock.Unlock();
        return (Core::ERROR_NONE);
    }

    string GetSinkAtmosCapability() const override
    {
        dsATMOSCapability_t atmosCapability;
        string ret;
        try
        {
            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI0");
            if (aPort.isConnected())
            {
                aPort.getSinkDeviceAtmosCapability(atmosCapability);
            }
            else
            {
               TRACE(Trace::Error, (_T("getSinkAtmosCapability failure: HDMI0 not connected!\n")));
            }
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }

        switch (atmosCapability)
        {
            case 0: ret = "unsupported"; break;
            case 1: ret = "DDPlus"; break;
            case 2: ret = "Atmos"; break;
            default: ret = "Unknown"; break;
        }
        return ret;
    }

    string GetSoundMode() const override
    {
        string audioPort;
        string modeString;
        device::AudioStereoMode mode = device::AudioStereoMode::kStereo;  //default to stereo

        try
        {
            /* Return the sound mode of the audio ouput connected to the specified videoDisplay */
            /* Check if HDMI is connected - Return (default) Stereo Mode if not connected */

            if (device::Host::getInstance().getVideoOutputPort("HDMI0").isDisplayConnected())
            {
                audioPort = "HDMI0";
            }
            else
            {
                /*  * If HDMI is not connected
                    * Get the SPDIF if it is supported by platform
                    * If Platform does not have connected ports. Default to HDMI.
                */
                audioPort = "HDMI0";
                device::List<device::VideoOutputPort> vPorts = device::Host::getInstance().getVideoOutputPorts();
                for (size_t i = 0; i < vPorts.size(); i++)
                {
                    device::VideoOutputPort &vPort = vPorts.at(i);
                    if (vPort.isDisplayConnected())
                    {
                        audioPort = "SPDIF0";
                        break;
                    }
                }
            }

            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);

            if (aPort.isConnected())
            {
                mode = aPort.getStereoMode();

                if (aPort.getType().getId() == device::AudioOutputPortType::kHDMI)
                {
                    /* In DS5, "Surround" implies "Auto" */
                    if (aPort.getStereoAuto() || mode == device::AudioStereoMode::kSurround)
                    {
                        TRACE(Trace::Information, (_T("HDMI0 is in Auto Mode")));
                        int surroundMode = device::Host::getInstance().getVideoOutputPort("HDMI0").getDisplay().getSurroundMode();
                        if ( surroundMode & dsSURROUNDMODE_DDPLUS)
                        {
                            TRACE(Trace::Information, (_T("HDMI0 has surround DDPlus")));
                            modeString.append("AUTO (Dolby Digital Plus)");
                        }
                        else if (surroundMode & dsSURROUNDMODE_DD)
                        {
                            TRACE(Trace::Information, (_T("HDMI0 has surround DD 5.1")));
                            modeString.append("AUTO (Dolby Digital 5.1)");
                        }
                        else
                        {
                            TRACE(Trace::Information, (_T("HDMI0 does not surround")));
                            modeString.append("AUTO (Stereo)");
                        }
                    }
                    else
                        modeString.append(mode.toString());
                }
                else
                {
                    if (mode == device::AudioStereoMode::kSurround)
                        modeString.append("Surround");
                    else
                        modeString.append(mode.toString());
                }
            }
            else
            {
                /*
                * VideoDisplay is not connected. Its audio mode is unknown. Return
                * "Stereo" as safe default;
                */
                mode = device::AudioStereoMode::kStereo;
                modeString.append("AUTO (Stereo)");
            }
        }
        catch (const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
            //
            // Exception
            // "Stereo" as safe default;
            //
            mode = device::AudioStereoMode::kStereo;
            modeString += "AUTO (Stereo)";
        }

        TRACE(Trace::Information, (_T("audioPort = %s, mode = %s!"), audioPort.c_str(), modeString.c_str()));

        return modeString;
    }

    void SetAudioAtmosOutputMode(const bool enable) override
    {
        try
        {
            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI0");
            if (aPort.isConnected()) {
                aPort.setAudioAtmosOutputMode (enable);
            }
            else {
                TRACE(Trace::Error, (_T("setAudioAtmosOutputMode failure: HDMI0 not connected!\n")));
            }

        }
        catch (const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
    }

    static void AudioModeHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        switch (eventId)
        {
            case IARM_BUS_DSMGR_EVENT_AUDIO_MODE:
                {
                    IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                    string  AudioPortMode;
                    string AudioPortType;
                    switch (eventData->data.Audioport.mode)
                    {
                        case 1: AudioPortMode = "mono"; break;
                        case 2: AudioPortMode = "stereo"; break;
                        case 3: AudioPortMode = "surround"; break;
                        case 4: AudioPortMode = "passthrough"; break;
                        case 0: AudioPortMode = "unknown"; break;
                        default: AudioPortMode = "unknown"; break;
                    }

                    switch(eventData->data.Audioport.type)
                    {
                        case 0: AudioPortType = "unsupported"; break;
                        case 1: AudioPortType = "DDPlus"; break;
                        case 2: AudioPortType = "Atmos"; break;
                        default: AudioPortType = "Unknown"; break;
                    }
                    if(PlayerInfoImplementation::_instance)
                    {
                        PlayerInfoImplementation::_instance->audiomodeChanged(AudioPortMode, AudioPortType);
                    }
                }
                break;
            default:
                break;
        }
    }

    void audiomodeChanged(const string AudioPortMode, const string AudioPortType)
    {
        _adminLock.Lock();
        std::list<Exchange::IPlayerAtmosProperties::INotification*>::const_iterator index = _notificationClients.begin();
        if (index != _notificationClients.end())
        {
            (*index)->AudioModeChanged(AudioPortMode, AudioPortType);
        }
        _adminLock.Unlock();
    }

   BEGIN_INTERFACE_MAP(PlayerInfoImplementation)
        INTERFACE_ENTRY(Exchange::IPlayerProperties)
        INTERFACE_ENTRY(Exchange::IPlayerAtmosProperties)
   END_INTERFACE_MAP

private:


    void UpdateAudioCodecInfo()
    {
        AudioCaps audioCaps = {
            {"audio/mpeg, mpegversion=(int)1", Exchange::IPlayerProperties::IAudioIterator::AudioCodec::AUDIO_MPEG1},
            {"audio/mpeg, mpegversion=(int)2", Exchange::IPlayerProperties::IAudioIterator::AudioCodec::AUDIO_MPEG2},
            {"audio/mpeg, mpegversion=(int)4", Exchange::IPlayerProperties::IAudioIterator::AudioCodec::AUDIO_MPEG4},
            {"audio/mpeg, mpegversion=(int)1, layer=(int)[1, 3]", Exchange::IPlayerProperties::IAudioIterator::AudioCodec::AUDIO_MPEG3},
            {"audio/mpeg, mpegversion=(int){2, 4}", Exchange::IPlayerProperties::IAudioIterator::AudioCodec::AUDIO_AAC},
            {"audio/x-ac3", Exchange::IPlayerProperties::IAudioIterator::AudioCodec::AUDIO_AC3},
            {"audio/x-eac3", Exchange::IPlayerProperties::IAudioIterator::AudioCodec::AUDIO_AC3_PLUS},
            {"audio/x-opus", Exchange::IPlayerProperties::IAudioIterator::AudioCodec::AUDIO_OPUS},
            {"audio/x-dts", Exchange::IPlayerProperties::IAudioIterator::AudioCodec::AUDIO_DTS},
            {"audio/x-vorbis", Exchange::IPlayerProperties::IAudioIterator::AudioCodec::AUDIO_VORBIS_OGG},
            {"audio/x-wav", Exchange::IPlayerProperties::IAudioIterator::AudioCodec::AUDIO_WAV},
        };
        if (GstUtils::GstRegistryCheckElementsForMediaTypes(audioCaps, _audioCodecs) != true) {
            TRACE_L1(_T("There is no Audio Codec support available"));
        }

    }
    void UpdateVideoCodecInfo()
    {
        VideoCaps videoCaps = {
            {"video/x-h263", Exchange::IPlayerProperties::IVideoIterator::VideoCodec::VIDEO_H263},
            {"video/x-h264, profile=(string)high", Exchange::IPlayerProperties::IVideoIterator::VideoCodec::VIDEO_H264},
            {"video/x-h265", Exchange::IPlayerProperties::IVideoIterator::VideoCodec::VIDEO_H265},
            {"video/mpeg, mpegversion=(int){1,2}, systemstream=(boolean)false", Exchange::IPlayerProperties::IVideoIterator::VideoCodec::VIDEO_MPEG},
            {"video/x-vp8", Exchange::IPlayerProperties::IVideoIterator::VideoCodec::VIDEO_VP8},
            {"video/x-vp9", Exchange::IPlayerProperties::IVideoIterator::VideoCodec::VIDEO_VP9},
            {"video/x-vp10", Exchange::IPlayerProperties::IVideoIterator::VideoCodec::VIDEO_VP10}
        };
        if (GstUtils::GstRegistryCheckElementsForMediaTypes(videoCaps, _videoCodecs) != true) {
            TRACE_L1(_T("There is no Video Codec support available"));
        }
    }

private:
    mutable Core::CriticalSection _adminLock;
    std::list<Exchange::IPlayerProperties::IAudioIterator::AudioCodec> _audioCodecs;
    std::list<Exchange::IPlayerProperties::IVideoIterator::VideoCodec> _videoCodecs;
    std::list<Exchange::IPlayerAtmosProperties::INotification*> _notificationClients;
public:
    static PlayerInfoImplementation* _instance;
};
    PlayerInfoImplementation* PlayerInfoImplementation::_instance = nullptr;
    SERVICE_REGISTRATION(PlayerInfoImplementation, 1, 0);
}
}
