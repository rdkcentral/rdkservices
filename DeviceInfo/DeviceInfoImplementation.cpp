#include "DeviceInfoImplementation.h"

namespace WPEFramework {
namespace Plugin {
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

    uint32_t DeviceInfoImplementation::AudioOutputs(IAudioOutputIterator*& res) const
    {
        res = Core::Service<AudioIteratorImplementation>::Create<Exchange::IDeviceCapabilities::IAudioOutputIterator>(_audio);
        return (res != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }

    uint32_t DeviceInfoImplementation::VideoOutputs(IVideoOutputIterator*& res) const
    {
        res = Core::Service<VideoIteratorImplementation>::Create<Exchange::IDeviceCapabilities::IVideoOutputIterator>(_video);
        return (res != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }

    uint32_t DeviceInfoImplementation::Resolutions(IOutputResolutionIterator*& res) const
    {
        res = Core::Service<ResolutionIteratorImplementation>::Create<Exchange::IDeviceCapabilities::IOutputResolutionIterator>(_resolution);
        return (res != nullptr ? Core::ERROR_NONE : Core::ERROR_GENERAL);
    }

    uint32_t DeviceInfoImplementation::HDR(bool& supportsHDR) const
    {
        supportsHDR = _supportsHdr;
        return Core::ERROR_NONE;
    }

    uint32_t DeviceInfoImplementation::Atmos(bool& supportsAtmos) const
    {
        supportsAtmos = _supportsAtmos;
        return Core::ERROR_NONE;
    }

    uint32_t DeviceInfoImplementation::CEC(bool& supportsCEC) const
    {
        supportsCEC = _supportsCEC;
        return Core::ERROR_NONE;
    }
    uint32_t DeviceInfoImplementation::HDCP(CopyProtection& supportedHDCP) const
    {
        supportedHDCP = _supportedHDCP;
        return Core::ERROR_NONE;
    }
}
}