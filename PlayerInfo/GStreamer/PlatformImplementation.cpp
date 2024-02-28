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

#include <gst/gst.h>

namespace WPEFramework {
namespace Plugin {

class PlayerInfoImplementation : public Exchange::IPlayerProperties {
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

                            if (padTemplate && padTemplate->direction == GST_PAD_SRC) {
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

            return (candidates);
        }

    };

private:
    using AudioIteratorImplementation = RPC::IteratorType<Exchange::IPlayerProperties::IAudioCodecIterator>;
    using VideoIteratorImplementation = RPC::IteratorType<Exchange::IPlayerProperties::IVideoCodecIterator>;

    typedef std::map<const string, const Exchange::IPlayerProperties::AudioCodec> AudioCaps;
    typedef std::map<const string, const Exchange::IPlayerProperties::VideoCodec> VideoCaps;

public:
    PlayerInfoImplementation()
#if DOLBY_SUPPORT
        : _dolbyOut(Core::ServiceAdministrator::Instance()
                        .Instantiate<Exchange::Dolby::IOutput>(Core::Library(), "DolbyOutputImplementation", ~0))
#endif
    {
        gst_init(0, nullptr);
        UpdateAudioCodecInfo();
        UpdateVideoCodecInfo();
    }

    PlayerInfoImplementation(const PlayerInfoImplementation&) = delete;
    PlayerInfoImplementation& operator= (const PlayerInfoImplementation&) = delete;
    ~PlayerInfoImplementation() override
    {
        _audioCodecs.clear();
        _videoCodecs.clear();

#if DOLBY_SUPPORT
        if(_dolbyOut != nullptr) {
            _dolbyOut->Release();
        }
#endif
    }

public:
    uint32_t AudioCodecs(Exchange::IPlayerProperties::IAudioCodecIterator*& codec) const override
    {
        codec = (Core::Service<AudioIteratorImplementation>::Create<Exchange::IPlayerProperties::IAudioCodecIterator>(_audioCodecs));
        return (codec != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
    uint32_t VideoCodecs(Exchange::IPlayerProperties::IVideoCodecIterator*& codec) const override
    {
        codec = (Core::Service<VideoIteratorImplementation>::Create<Exchange::IPlayerProperties::IVideoCodecIterator>(_videoCodecs));
        return (codec != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
    uint32_t Resolution(PlaybackResolution&) const override
    {
        return (Core::ERROR_NONE);
    }
    uint32_t IsAudioEquivalenceEnabled(bool&) const override
    {
        return (Core::ERROR_NONE);
    }

    BEGIN_INTERFACE_MAP(PlayerInfoImplementation)
    INTERFACE_ENTRY(Exchange::IPlayerProperties)
#if DOLBY_SUPPORT
    INTERFACE_AGGREGATE(Exchange::Dolby::IOutput, _dolbyOut)
#endif
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
            {"audio/x-ac3", Exchange::IPlayerProperties::AudioCodec::AUDIO_AC3},
            {"audio/x-eac3", Exchange::IPlayerProperties::AudioCodec::AUDIO_AC3_PLUS},
            {"audio/x-opus", Exchange::IPlayerProperties::AudioCodec::AUDIO_OPUS},
            {"audio/x-dts", Exchange::IPlayerProperties::AudioCodec::AUDIO_DTS},
            {"audio/x-vorbis", Exchange::IPlayerProperties::AudioCodec::AUDIO_VORBIS_OGG},
            {"audio/x-wav", Exchange::IPlayerProperties::AudioCodec::AUDIO_WAV},
        };
        if (GstUtils::GstRegistryCheckElementsForMediaTypes(audioCaps, _audioCodecs) != true) {
            TRACE(Trace::Warning, (_T("There is no Audio Codec support available")));
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
            TRACE(Trace::Warning, (_T("There is no Video Codec support available")));
        }
    }

private:
    std::list<Exchange::IPlayerProperties::AudioCodec> _audioCodecs;
    std::list<Exchange::IPlayerProperties::VideoCodec> _videoCodecs;
#if DOLBY_SUPPORT
    Exchange::Dolby::IOutput* _dolbyOut;
#endif
};

    SERVICE_REGISTRATION(PlayerInfoImplementation, 1, 0);
}
}
