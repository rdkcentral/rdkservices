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
#include <interfaces/IDolby.h>
#include "host.hpp"
#include "exception.hpp"
#include "audioOutputPortType.hpp"
#include "audioOutputPortConfig.hpp"
#include "audioOutputPort.hpp"
#include "utils.h"

#include <gst/gst.h>

#include "libIBus.h"
#include "libIBusDaemon.h"
#include "dsMgr.h"

namespace WPEFramework {
namespace Plugin {

class PlayerInfoImplementation : public Exchange::IPlayerProperties, public Exchange::Dolby::IOutput
{
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
    using AudioIteratorImplementation = RPC::IteratorType<Exchange::IPlayerProperties::IAudioCodecIterator>;
    using VideoIteratorImplementation = RPC::IteratorType<Exchange::IPlayerProperties::IVideoCodecIterator>;
    typedef std::map<const string, const Exchange::IPlayerProperties::AudioCodec> AudioCaps;
    typedef std::map<const string, const Exchange::IPlayerProperties::VideoCodec> VideoCaps;

public:
    PlayerInfoImplementation()
    {
        gst_init(0, nullptr);
        UpdateAudioCodecInfo();
        UpdateVideoCodecInfo();
        Utils::IARM::init();
        IARM_Result_t res;
        IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_AUDIO_MODE, AudioModeHandler) );
        PlayerInfoImplementation::_instance = this;
    }

    PlayerInfoImplementation(const PlayerInfoImplementation&) = delete;
    PlayerInfoImplementation& operator= (const PlayerInfoImplementation&) = delete;
    ~PlayerInfoImplementation() override
    {
        _audioCodecs.clear();
        _videoCodecs.clear();
        IARM_Result_t res;
        IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_AUDIO_MODE) );
        PlayerInfoImplementation::_instance = nullptr;
    }

public:
    uint32_t AudioCodecs(Exchange::IPlayerProperties::IAudioCodecIterator*& iterator) const override
    {
        iterator = Core::Service<AudioIteratorImplementation>::Create<Exchange::IPlayerProperties::IAudioCodecIterator>(_audioCodecs);
        return (iterator != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
    uint32_t VideoCodecs(Exchange::IPlayerProperties::IVideoCodecIterator*& iterator) const override
    {
        iterator = Core::Service<VideoIteratorImplementation>::Create<Exchange::IPlayerProperties::IVideoCodecIterator>(_videoCodecs);
        return (iterator != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }

    uint32_t Resolution(PlaybackResolution& res /* @out */) const override
    {
        res = RESOLUTION_UNKNOWN;

        string currentResolution = "0";
        try
        {
            std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
            device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
            currentResolution = vPort.getResolution().getName();
            TRACE(Trace::Information, (_T("Current video playback resolution = %s"), currentResolution));
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }

        if (_resolutions.find(currentResolution) != _resolutions.end())
        {
            res = _resolutions.find(currentResolution)->second;
        }
        else
        {
            //couldn't find the resolution returned from platform.
            //It could be the case that the framerate in the retruned
            //resolution isn't part of the map.
            //So, try to return the base resolution at least by ignoring
            //the framerate
            string baseRes = currentResolution.substr(0, currentResolution.size()-2);
            if (_resolutions.find(baseRes) != _resolutions.end())
            {
                res = _resolutions.find(baseRes)->second;
            }
        }

        return (Core::ERROR_NONE);
    }

    uint32_t IsAudioEquivalenceEnabled(bool& isEnbaled /* @out */) const override
    {
        isEnbaled = false;
        try
        {
            if (device::Host::getInstance().isHDMIOutPortPresent())
            {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI0");
                if (aPort.isConnected()) {
                    isEnbaled = aPort.GetLEConfig();
                    LOGINFO("IsAudioEquivalenceEnabled = %s", isEnbaled? "Enabled":"Disabled");
                }
                else
                {
                    TRACE(Trace::Information, (_T("IsAudioEquivalenceEnabled failure: HDMI0 not connected!")));

                    LOGERR("IsAudioEquivalenceEnabled failure: HDMI0 not connected!");
                }
            }
            else {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("SPEAKER0");
                isEnbaled = aPort.GetLEConfig();
                LOGINFO("IsAudioEquivalenceEnabled = %s", isEnbaled? "Enabled":"Disabled");
            }
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        TRACE(Trace::Information, (_T("Audio Equivalence = %d"), isEnbaled? "Enabled":"Disabled"));


        return (Core::ERROR_NONE);
    }

    uint32_t Register(Exchange::Dolby::IOutput::INotification* notification) override
    {
        _adminLock.Lock();

        // Make sure a sink is not registered multiple times.
        ASSERT(std::find(_observers.begin(), _observers.end(), notification) == _observers.end());

        _observers.push_back(notification);
        notification->AddRef();

        _adminLock.Unlock();

        return (Core::ERROR_NONE);
    }

    uint32_t Unregister(Exchange::Dolby::IOutput::INotification* notification) override
    {
        _adminLock.Lock();

        std::list<Exchange::Dolby::IOutput::INotification*>::iterator index(std::find(_observers.begin(), _observers.end(), notification));

        // Make sure you do not unregister something you did not register !!!
        ASSERT(index != _observers.end());

        if (index != _observers.end()) {
            (*index)->Release();
            _observers.erase(index);
        }

        _adminLock.Unlock();

        return (Core::ERROR_NONE);
    }

    static void AudioModeHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if(PlayerInfoImplementation::_instance)
        {
            dsAudioStereoMode_t amode = dsAUDIO_STEREO_UNKNOWN;
            Exchange::Dolby::IOutput::SoundModes mode = UNKNOWN;
            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            amode = static_cast<dsAudioStereoMode_t>(eventData->data.Audioport.mode);
            if (amode == device::AudioStereoMode::kSurround) mode = SURROUND;
            else if(amode == device::AudioStereoMode::kStereo) mode = STEREO;
            else if(amode == device::AudioStereoMode::kMono) mode = MONO;
            else if(amode == device::AudioStereoMode::kPassThru) mode = PASSTHRU;
            else mode = UNKNOWN;
            PlayerInfoImplementation::_instance->audiomodeChanged(mode, true);
        }
    }

    void audiomodeChanged(Exchange::Dolby::IOutput::SoundModes mode, bool enable)
    {
        _adminLock.Lock();

        std::list<Exchange::Dolby::IOutput::INotification*>::const_iterator index = _observers.begin();

        while(index != _observers.end()) {
            (*index)->AudioModeChanged(mode, enable);
            index++;
        }

        _adminLock.Unlock();

    }

    uint32_t Mode(const Exchange::Dolby::IOutput::Type& mode) override
    {
        return (Core::ERROR_GENERAL);
    }

    uint32_t Mode(Exchange::Dolby::IOutput::Type& mode) const override
    {
        return (Core::ERROR_GENERAL);
    }


    uint32_t AtmosMetadata(bool& supported /* @out */) const override
    {
        dsATMOSCapability_t atmosCapability = dsAUDIO_ATMOS_NOTSUPPORTED;
        supported = false;
        string audioPort = "HDMI0"; //default to HDMI
        try
        {
            /*  Check if the device has an HDMI_ARC out. If ARC is connected, then SPEAKERS and SPDIF are disabled.
                So, check the atmos capability of the HDMI_ARC first*/
            device::List<device::AudioOutputPort> aPorts = device::Host::getInstance().getAudioOutputPorts();
            for (size_t i = 0; i < aPorts.size(); i++)
            {
                device::AudioOutputPort &aPort = aPorts.at(i);
                if(aPort.getName().find("HDMI_ARC") != std::string::npos)
                {
                    //the platform supports HDMI_ARC. Get the sound mode of the ARC port
                    audioPort = "HDMI_ARC0";
                    break;
                }
            }
            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
            if (aPort.isConnected())
            {
                aPort.getSinkDeviceAtmosCapability(atmosCapability);
            }
            else
            {
                TRACE(Trace::Error, (_T("getSinkAtmosCapability failure: neither HDMI0 nor HDMI_ARC connected!\n")));
                device::Host::getInstance().getSinkDeviceAtmosCapability(atmosCapability); //gets host device-sink's atmos caps (For TV panel, device Sink is itself)
            }
        }
        catch(const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }

        if(atmosCapability == dsAUDIO_ATMOS_ATMOSMETADATA) supported = true;
        return (Core::ERROR_NONE);
    }

    uint32_t SoundMode(Exchange::Dolby::IOutput::SoundModes& mode /* @out */) const override
    {
        /* For implementation details, please refer to Flow diagram attached in RDKTV-10066*/

        string audioPort;
        if (device::Host::getInstance().isHDMIOutPortPresent())
            audioPort = "HDMI0"; //this device has an HDMI out port. This is an STB device
        else
            audioPort = "SPEAKER0"; // This device is likely to be TV. Default audio outport are speakers.

        device::AudioStereoMode soundmode = device::AudioStereoMode::kStereo;
        mode = UNKNOWN;

        try
        {
            /* Check if the device has an HDMI_ARC out. If ARC is connected, then speakers and SPDIF are disabled
               So, return the SoundMode of HDMI_ARC*/
            device::List<device::AudioOutputPort> aPorts = device::Host::getInstance().getAudioOutputPorts();
            for (size_t i = 0; i < aPorts.size(); i++)
            {
                device::AudioOutputPort &aPort = aPorts.at(i);
                /* Does this device have an SPDIF port and is it connected? If so, lets set the AudioPort to that, as SPDIF has precedence
                   over the deafault audio output port. But keep searching if the device has an HDMI_ARC connected, as ARC is highest precedence*/
                if (aPort.getName().find("SPDIF") != std::string::npos && device::Host::getInstance().getAudioOutputPort("SPDIF0").isConnected())
                {
                    audioPort = "SPDIF0";
                }
                /* Does this device support HDMI_ARC output and is the port connected? If yes, then that the audio port whose sound mode we'll return */
                if(aPort.getName().find("HDMI_ARC") != std::string::npos && device::Host::getInstance().getAudioOutputPort("HDMI_ARC0").isConnected())
                {
                    //the platform supports HDMI_ARC. Get the sound mode of the ARC port
                    LOGINFO(" HDMI ARC port detected on platform");
                    audioPort = "HDMI_ARC0";
                    break;
                }
            }

            /*When we reach here, we have determined the audio output port correctly. Now, check the sound mode on that port */
            device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
            if (aPort.isConnected())
            {
                soundmode = aPort.getStereoMode();
                if (soundmode == device::AudioStereoMode::kSurround) mode = SURROUND;
                else if(soundmode == device::AudioStereoMode::kStereo) mode = STEREO;
                else if(soundmode == device::AudioStereoMode::kMono) mode = MONO;
                else if(soundmode == device::AudioStereoMode::kPassThru) mode = PASSTHRU;
                else mode = UNKNOWN;

                /* Auto mode applicable for HDMI Arc and SPDIF */
                if((aPort.getType().getId() == device::AudioOutputPortType::kARC || aPort.getType().getId() == device::AudioOutputPortType::kSPDIF)
                        && aPort.getStereoAuto())
                {
                    mode = SOUNDMODE_AUTO;
                }
            }
        }
        catch (const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }

        return (Core::ERROR_NONE);
    }

    uint32_t EnableAtmosOutput(const bool& enable /* @in */)
    {
        try
        {
            if (device::Host::getInstance().isHDMIOutPortPresent()) {
                device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort("HDMI0");
                if (aPort.isConnected()) {
                    aPort.setAudioAtmosOutputMode(enable);
                }
                else
                {
                    TRACE(Trace::Error, (_T("setAudioAtmosOutputMode failure: HDMI0 not connected!\n")));
                }
            }
            else {
                device::Host::getInstance().setAudioAtmosOutputMode(enable);
            }
        }
        catch (const device::Exception& err)
        {
            TRACE(Trace::Error, (_T("Exception during DeviceSetting library call. code = %d message = %s"), err.getCode(), err.what()));
        }
        return (Core::ERROR_NONE);
    }

    BEGIN_INTERFACE_MAP(PlayerInfoImplementation)
    INTERFACE_ENTRY(Exchange::IPlayerProperties)
    INTERFACE_ENTRY(Exchange::Dolby::IOutput)
    END_INTERFACE_MAP

private:


    void UpdateAudioCodecInfo()
    {
        AudioCaps audioCaps = {
            {"audio/mpeg, mpegversion=(int)1", Exchange::IPlayerProperties::AudioCodec::AUDIO_MPEG1},
            {"audio/mpeg, mpegversion=(int)2", Exchange::IPlayerProperties::AudioCodec::AUDIO_MPEG2},
            {"audio/mpeg, mpegversion=(int)4", Exchange::IPlayerProperties::AudioCodec::AUDIO_MPEG4},
            {"audio/mpeg, mpegversion=(int)1, layer=(int)[1, 3]", Exchange::IPlayerProperties::AudioCodec::AUDIO_MPEG3},
            {"audio/mpeg, mpegversion=(int){2, 4}", Exchange::IPlayerProperties::AudioCodec::AUDIO_AAC},
            {"audio/x-ac3", Exchange::IPlayerProperties::AUDIO_AC3},
            {"audio/x-eac3", Exchange::IPlayerProperties::AUDIO_AC3_PLUS},
            {"audio/x-opus", Exchange::IPlayerProperties::AUDIO_OPUS},
            {"audio/x-dts", Exchange::IPlayerProperties::AUDIO_DTS},
            {"audio/x-vorbis", Exchange::IPlayerProperties::AUDIO_VORBIS_OGG},
            {"audio/x-wav", Exchange::IPlayerProperties::AUDIO_WAV},
        };
        if (GstUtils::GstRegistryCheckElementsForMediaTypes(audioCaps, _audioCodecs) != true) {
            TRACE_L1(_T("There is no Audio Codec support available"));
        }

    }
    void UpdateVideoCodecInfo()
    {
        VideoCaps videoCaps = {
            {"video/x-h263", Exchange::IPlayerProperties::VideoCodec::VIDEO_H263},
            {"video/x-h264, profile=(string)high", Exchange::IPlayerProperties::VideoCodec::VIDEO_H264},
            {"video/x-h265", Exchange::IPlayerProperties::VideoCodec::VIDEO_H265},
            {"video/mpeg, mpegversion=(int){1,2}, systemstream=(boolean)false", Exchange::IPlayerProperties::VideoCodec::VIDEO_MPEG},
            {"video/x-vp8", Exchange::IPlayerProperties::VideoCodec::VIDEO_VP8},
            {"video/x-vp9", Exchange::IPlayerProperties::VideoCodec::VIDEO_VP9},
            {"video/x-vp10", Exchange::IPlayerProperties::VideoCodec::VIDEO_VP10}
        };
        if (GstUtils::GstRegistryCheckElementsForMediaTypes(videoCaps, _videoCodecs) != true) {
            TRACE_L1(_T("There is no Video Codec support available"));
        }
    }

private:
    std::list<Exchange::IPlayerProperties::AudioCodec> _audioCodecs;
    std::list<Exchange::IPlayerProperties::VideoCodec> _videoCodecs;
    std::map<string, Exchange::IPlayerProperties::PlaybackResolution> _resolutions =
    {
        {"480i24", RESOLUTION_480I24},
        {"480i25", RESOLUTION_480I25},
        {"480i30", RESOLUTION_480I30},
        {"480i50", RESOLUTION_480I50},
        {"480i", RESOLUTION_480I},
        {"480p24", RESOLUTION_480P24},
        {"480p25", RESOLUTION_480P25},
        {"480p30", RESOLUTION_480P30},
        {"480p50", RESOLUTION_480P50},
        {"480p", RESOLUTION_480P},
        {"576i24", RESOLUTION_576I24},
        {"576i25", RESOLUTION_576I25},
        {"576i30", RESOLUTION_576I30},
        {"576i50", RESOLUTION_576I50},
        {"576i", RESOLUTION_576I},
        {"576p24", RESOLUTION_576P24},
        {"576p25", RESOLUTION_576P25},
        {"576p30", RESOLUTION_576P30},
        {"576p50", RESOLUTION_576P50},
        {"576p", RESOLUTION_576P},
        {"720p24", RESOLUTION_720P24},
        {"720p25", RESOLUTION_720P25},
        {"720p30", RESOLUTION_720P30},
        {"720p50", RESOLUTION_720P50},
        {"720p", RESOLUTION_720P},
        {"1080i24", RESOLUTION_1080I24},
        {"1080i25", RESOLUTION_1080I25},
        {"1080i30", RESOLUTION_1080I30},
        {"1080i50", RESOLUTION_1080I50},
        {"1080i", RESOLUTION_1080I},
        {"1080p24", RESOLUTION_1080P24},
        {"1080p25", RESOLUTION_1080P25},
        {"1080p30", RESOLUTION_1080P30},
        {"1080p50", RESOLUTION_1080P50},
        {"1080p60", RESOLUTION_1080P},
        {"1080p", RESOLUTION_1080P},
        {"2160p24", RESOLUTION_2160P24},
        {"2160p25", RESOLUTION_2160P25},
        {"2160p50", RESOLUTION_2160P50},
        {"2160p30", RESOLUTION_2160P30},
        {"2160p60", RESOLUTION_2160P60},
        {"2160p", RESOLUTION_2160P}
    };
    std::list<Exchange::Dolby::IOutput::INotification*> _observers;
    mutable Core::CriticalSection _adminLock;
public:
    static PlayerInfoImplementation* _instance;
};
    PlayerInfoImplementation* PlayerInfoImplementation::_instance = nullptr;
    SERVICE_REGISTRATION(PlayerInfoImplementation, 1, 0);
}
}
