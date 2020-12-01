#include "DeviceInfoImplementation.h"

namespace WPEFramework {
    
ENUM_CONVERSION_BEGIN(Exchange::IDeviceCapabilities::AudioOutput)
    { Exchange::IDeviceCapabilities::AudioOutput::AUDIO_OTHER, _TXT("other") },
    { Exchange::IDeviceCapabilities::AudioOutput::AUDIO_ANALOG, _TXT("analog") },
    { Exchange::IDeviceCapabilities::AudioOutput::AUDIO_DISPLAYPORT, _TXT("displayport") },
    { Exchange::IDeviceCapabilities::AudioOutput::AUDIO_HDMI, _TXT("hdmi") },
    { Exchange::IDeviceCapabilities::AudioOutput::AUDIO_RF_MODULATOR, _TXT("rf_modulator") },
    { Exchange::IDeviceCapabilities::AudioOutput::AUDIO_SPDIF, _TXT("spdif") },
ENUM_CONVERSION_END(Exchange::IDeviceCapabilities::AudioOutput)

ENUM_CONVERSION_BEGIN(Exchange::IDeviceCapabilities::VideoOutput)
    { Exchange::IDeviceCapabilities::VideoOutput::VIDEO_COMPONENT, _TXT("component") },
    { Exchange::IDeviceCapabilities::VideoOutput::VIDEO_COMPOSITE, _TXT("composite") },
    { Exchange::IDeviceCapabilities::VideoOutput::VIDEO_DISPLAYPORT, _TXT("displayport") },
    { Exchange::IDeviceCapabilities::VideoOutput::VIDEO_HDMI, _TXT("hdmi") },
    { Exchange::IDeviceCapabilities::VideoOutput::VIDEO_OTHER, _TXT("other") },
    { Exchange::IDeviceCapabilities::VideoOutput::VIDEO_RF_MODULATOR, _TXT("rf_modulator") },
    { Exchange::IDeviceCapabilities::VideoOutput::VIDEO_SCART_RGB, _TXT("scart_rgb") },
    { Exchange::IDeviceCapabilities::VideoOutput::VIDEO_SVIDEO, _TXT("svideo") },
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

ENUM_CONVERSION_BEGIN(Exchange::IDeviceCapabilities::CopyProtection)
    { Exchange::IDeviceCapabilities::CopyProtection::HDCP_UNAVAILABLE, _TXT("unavailable") },
    { Exchange::IDeviceCapabilities::CopyProtection::HDCP_14, _TXT("hdcp_14") },
    { Exchange::IDeviceCapabilities::CopyProtection::HDCP_20, _TXT("hdcp_20") },
    { Exchange::IDeviceCapabilities::CopyProtection::HDCP_21, _TXT("hdcp_21") },
    { Exchange::IDeviceCapabilities::CopyProtection::HDCP_22, _TXT("hdcp_22") },
ENUM_CONVERSION_END(Exchange::IDeviceCapabilities::CopyProtection)


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
        ASSERT(service != nullptr);

        _config.FromString(service->ConfigLine());

        _supportsHdr = _config.Hdr.Value();
        _supportsAtmos = _config.Atmos.Value();
        _supportsCEC = _config.Cec.Value();
        _supportedHDCP = _config.Hdcp.Value();

        AudioJsonArray::Iterator audioIterator(_config.Audio.Elements());
        while (audioIterator.Next()) {
            _audio.push_back(audioIterator.Current().Value());
        }

        VideoJsonArray::Iterator videoIterator(_config.Video.Elements());
        while (videoIterator.Next()) {
            _video.push_back(videoIterator.Current().Value());
        }

        ResolutionJsonArray::Iterator resolutionIterator(_config.Resolution.Elements());
        while (resolutionIterator.Next()) {
            _resolution.push_back(resolutionIterator.Current().Value());
        }
    }

    uint32_t DeviceInfoImplementation::AudioOutputs(IAudioOutputIterator * &res) const {
        res = Core::Service<AudioIteratorImplementation>::Create<Exchange::IDeviceCapabilities::IAudioOutputIterator>(_audio);
        return (res != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }

    uint32_t DeviceInfoImplementation::VideoOutputs(IVideoOutputIterator * &res) const {
        res = Core::Service<VideoIteratorImplementation>::Create<Exchange::IDeviceCapabilities::IVideoOutputIterator>(_video);
        return (res != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }

    uint32_t DeviceInfoImplementation::Resolutions(IOutputResolutionIterator * &res) const {
        res = Core::Service<ResolutionIteratorImplementation>::Create<Exchange::IDeviceCapabilities::IOutputResolutionIterator>(_resolution);
        return (res != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }

    uint32_t DeviceInfoImplementation::HDR(bool& supportsHDR) const {
        supportsHDR = _supportsHdr;
        return Core::ERROR_NONE;
    }
    
    uint32_t DeviceInfoImplementation::Atmos(bool& supportsAtmos) const {
        supportsAtmos = _supportsAtmos;
        return Core::ERROR_NONE;
    }

    uint32_t DeviceInfoImplementation::CEC(bool& supportsCEC) const {
        supportsCEC = _supportsCEC;
        return Core::ERROR_NONE;
    }
    uint32_t DeviceInfoImplementation::HDCP(CopyProtection& supportedHDCP) const {     
        supportedHDCP = _supportedHDCP;
        return Core::ERROR_NONE;
    }
}
}