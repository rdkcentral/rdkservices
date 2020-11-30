#include "DeviceInfoImplementation.h"

namespace WPEFramework {
    
ENUM_CONVERSION_BEGIN(Exchange::IDeviceCapabilities::AudioOutput)
    { Exchange::IDeviceCapabilities::AudioOutput::ANALOG, _TXT("analog") },
    { Exchange::IDeviceCapabilities::AudioOutput::DIGITAL_COAX, _TXT("digital_coax") },
    { Exchange::IDeviceCapabilities::AudioOutput::HDMI_DOLBY, _TXT("hdmi_dolby") },
    { Exchange::IDeviceCapabilities::AudioOutput::HDMI_PCM, _TXT("hdmi_pcm") },
    { Exchange::IDeviceCapabilities::AudioOutput::TOSLINK, _TXT("toslink") },
ENUM_CONVERSION_END(Exchange::IDeviceCapabilities::AudioOutput)

ENUM_CONVERSION_BEGIN(Exchange::IDeviceCapabilities::VideoOutput)
    { Exchange::IDeviceCapabilities::VideoOutput::COMPOSITE, _TXT("composite") },
    { Exchange::IDeviceCapabilities::VideoOutput::HDMI_14, _TXT("hdmi_14") },
    { Exchange::IDeviceCapabilities::VideoOutput::HDMI_20, _TXT("hdmi_20") },
    { Exchange::IDeviceCapabilities::VideoOutput::HDMI_21, _TXT("hdmi_21") },
    { Exchange::IDeviceCapabilities::VideoOutput::SCART_RGB, _TXT("scart_rgb") },
ENUM_CONVERSION_END(Exchange::IDeviceCapabilities::VideoOutput)

ENUM_CONVERSION_BEGIN(Exchange::IDeviceCapabilities::OutputResolution)
    { Exchange::IDeviceCapabilities::OutputResolution::RESOLUTION_1080I, _TXT("1080i") },
    { Exchange::IDeviceCapabilities::OutputResolution::RESOLUTION_1080P, _TXT("1080p") },
    { Exchange::IDeviceCapabilities::OutputResolution::RESOLUTION_2160P30, _TXT("2160p30") },
    { Exchange::IDeviceCapabilities::OutputResolution::RESOLUTION_2160P60, _TXT("2160p60") },
    { Exchange::IDeviceCapabilities::OutputResolution::RESOLUTION_4320P30, _TXT("4320p30") },
    { Exchange::IDeviceCapabilities::OutputResolution::RESOLUTION_4320P60, _TXT("4320p60") },
    { Exchange::IDeviceCapabilities::OutputResolution::RESOLUTION_480I, _TXT("480i") },
    { Exchange::IDeviceCapabilities::OutputResolution::RESOLUTION_480P, _TXT("480p") },
    { Exchange::IDeviceCapabilities::OutputResolution::RESOLUTION_576I, _TXT("576i") },
    { Exchange::IDeviceCapabilities::OutputResolution::RESOLUTION_576P, _TXT("576p") },
    { Exchange::IDeviceCapabilities::OutputResolution::RESOLUTION_720P, _TXT("720p") },
    { Exchange::IDeviceCapabilities::OutputResolution::RESOLUTION_UNKNOWN, _TXT("unknown") },
ENUM_CONVERSION_END(Exchange::IDeviceCapabilities::OutputResolution)


namespace Plugin
{
    SERVICE_REGISTRATION(DeviceInfoImplementation, 1, 0);

    using AudioJsonArray = Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceCapabilities::AudioOutput>>;
    using VideoJsonArray = Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceCapabilities::VideoOutput>>;
    using ResolutionJsonArray = Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceCapabilities::OutputResolution>>;

    using AudioIteratorImplementation = RPC::IteratorType<Exchange::IDeviceCapabilities::IAudioOutputIterator>;
    using VideoIteratorImplementation = RPC::IteratorType<Exchange::IDeviceCapabilities::IVideoOutputIterator>;
    using ResolutionIteratorImplementation = RPC::IteratorType<Exchange::IDeviceCapabilities::IOutputResolutionIterator>;


    uint32_t DeviceInfoImplementation::Configure(const PluginHost::IShell* service)
    {
        TRACE_L1(_T("CHODZI"));
        ASSERT(service != nullptr);

        _config.FromString(service->ConfigLine());

        _supportsHdr = _config.HdrSupport.Value();
        _supportsAtmos = _config.AtmosSupport.Value();

        AudioJsonArray::Iterator audioIterator(_config.Audio.Elements());
        while (audioIterator.Next()) {
            TRACE_L1(_T("%d"), audioIterator.Current().Value());
            _audio.push_back(audioIterator.Current().Value());
        }

        VideoJsonArray::Iterator videoIterator(_config.Video.Elements());
        while (videoIterator.Next()) {
            TRACE_L1(_T("%d"), videoIterator.Current().Value());
            _video.push_back(videoIterator.Current().Value());
        }

        ResolutionJsonArray::Iterator resolutionIterator(_config.Resolution.Elements());
        while (resolutionIterator.Next()) {
            TRACE_L1(_T("%d"), resolutionIterator.Current().Value());
            _resolution.push_back(resolutionIterator.Current().Value());
        }

        TRACE_L1(_T("PARAMS"));
        TRACE_L1("HDR: %d", _supportsHdr);
        TRACE_L1("Atmos: %d", _supportsAtmos);
    }

    uint32_t DeviceInfoImplementation::SupportedAudioOutputs(IAudioOutputIterator * &res) const {
        res = Core::Service<AudioIteratorImplementation>::Create<Exchange::IDeviceCapabilities::IAudioOutputIterator>(_audio);
        return (res != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }
    uint32_t DeviceInfoImplementation::SupportedVideoOutputs(IVideoOutputIterator * &res) const {
        res = Core::Service<VideoIteratorImplementation>::Create<Exchange::IDeviceCapabilities::IVideoOutputIterator>(_video);
        return (res != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }

    uint32_t DeviceInfoImplementation::SupportedResolutions(IOutputResolutionIterator * &res) const {
        res = Core::Service<ResolutionIteratorImplementation>::Create<Exchange::IDeviceCapabilities::IOutputResolutionIterator>(_resolution);
        return (res != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }

    uint32_t DeviceInfoImplementation::SupportsHDR(bool& supportsHDR) const {
        supportsHDR = _supportsHdr;
    }
    uint32_t DeviceInfoImplementation::SupportsAtmos(bool& supportsAtmos) const {
        supportsAtmos = _supportsAtmos;
    }
}
}