#pragma once

#include "Module.h"
#include <interfaces/IDeviceInfo.h>

namespace WPEFramework {
namespace Plugin {
    class DeviceInfoImplementation : public Exchange::IDeviceCapabilities {
    private:
        //CONFIG
        class Config : public Core::JSON::Container {
        public:
            Config()
                : Core::JSON::Container()
                , Audio()
                , Video()
                , Resolution()
                , HdrSupport(false)
                , AtmosSupport(false)
            {
                Add(_T("audio"), &Audio);
                Add(_T("video"), &Video);
                Add(_T("resolution"), &Resolution);
                Add(_T("supports_hdr"), &HdrSupport);
                Add(_T("supports_atmos"), &AtmosSupport);
            }
            ~Config() = default;

            Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceCapabilities::AudioOutput>> Audio;
            Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceCapabilities::VideoOutput>> Video;
            Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceCapabilities::OutputResolution>> Resolution;
            Core::JSON::Boolean HdrSupport;
            Core::JSON::Boolean AtmosSupport;

        private:
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

        }; // CONFIG

    public:
        DeviceInfoImplementation(const DeviceInfoImplementation&) = delete;
        DeviceInfoImplementation& operator=(const DeviceInfoImplementation&) = delete;
        DeviceInfoImplementation()
            : _config()
            , _supportsHdr(false)
            , _supportsAtmos(false)
            , _audio()
            , _video()
            , _resolution()
        {
        }
        ~DeviceInfoImplementation() {}

        BEGIN_INTERFACE_MAP(DeviceInfoImplementation)
        INTERFACE_ENTRY(Exchange::IDeviceCapabilities)
        END_INTERFACE_MAP

        uint32_t Configure(const PluginHost::IShell* service) override;

        uint32_t SupportedAudioOutputs(IAudioOutputIterator*& res) const override;
        uint32_t SupportedVideoOutputs(IVideoOutputIterator*& res) const override;
        uint32_t SupportedResolutions(IOutputResolutionIterator*& res) const override;
        uint32_t SupportsHDR(bool& supportsHDR) const override;
        uint32_t SupportsAtmos(bool& supportsAtmos) const override;

    private:
        Config _config;
        bool _supportsHdr;
        bool _supportsAtmos;
        std::list<IDeviceCapabilities::AudioOutput> _audio;
        std::list<IDeviceCapabilities::VideoOutput> _video;
        std::list<IDeviceCapabilities::OutputResolution> _resolution;
    };
} //namespace WPEFramework
} //namespace Plugin