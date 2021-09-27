#pragma once

#include "Module.h"
#include <interfaces/IDeviceInfo.h>

namespace WPEFramework {
namespace Plugin {
    class DeviceInfoImplementation : public Exchange::IDeviceCapabilities , public Exchange::IDeviceMetadata{
    private:
        //CONFIG
        class Config : public Core::JSON::Container {
        public:
            Config()
                : Core::JSON::Container()
                , Hdr(false)
                , Atmos(false)
                , Cec(false)
                , Hdcp(CopyProtection::HDCP_UNAVAILABLE)
                , Audio()
                , Video()
                , Resolution()
                , ModelName()
                , ModelYear()
                , FriendlyName()
                , SystemIntegratorName()
                , PlatformName()
            {
                Add(_T("hdr"), &Hdr);
                Add(_T("atmos"), &Atmos);
                Add(_T("cec"), &Cec);
                Add(_T("hdcp"), &Hdcp);
                Add(_T("audio"), &Audio);
                Add(_T("video"), &Video);
                Add(_T("resolution"), &Resolution);
                Add(_T("modelName"), &ModelName);
                Add(_T("modelYear"), &ModelYear);
                Add(_T("friendlyName"), &FriendlyName);
                Add(_T("systemIntegratorName"), &SystemIntegratorName);
                Add(_T("platformName"), &PlatformName);
            }
            ~Config() = default;

            Core::JSON::Boolean Hdr;
            Core::JSON::Boolean Atmos;
            Core::JSON::Boolean Cec;
            Core::JSON::EnumType<Exchange::IDeviceCapabilities::CopyProtection> Hdcp;
            Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceCapabilities::AudioOutput>> Audio;
            Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceCapabilities::VideoOutput>> Video;
            Core::JSON::ArrayType<Core::JSON::EnumType<Exchange::IDeviceCapabilities::OutputResolution>> Resolution;
            Core::JSON::String ModelName;
            Core::JSON::DecUInt16 ModelYear;
            Core::JSON::String FriendlyName;
            Core::JSON::String SystemIntegratorName;
            Core::JSON::String PlatformName;

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
            , _supportsCEC(false)
            , _supportedHDCP(CopyProtection::HDCP_UNAVAILABLE)
            , _audio()
            , _video()
            , _resolution()
            , _modelName()
            , _modelYear()
            , _friendlyName()
            , _systemIntegratorName()
            , _platformName()
        {
        }
        ~DeviceInfoImplementation() {}

        BEGIN_INTERFACE_MAP(DeviceInfoImplementation)
        INTERFACE_ENTRY(Exchange::IDeviceCapabilities)
        INTERFACE_ENTRY(Exchange::IDeviceMetadata)
        END_INTERFACE_MAP

        uint32_t Configure(const PluginHost::IShell* service) override;

        uint32_t AudioOutputs(IAudioOutputIterator*& res) const override;
        uint32_t VideoOutputs(IVideoOutputIterator*& res) const override;
        uint32_t Resolutions(IOutputResolutionIterator*& res) const override;
        uint32_t HDR(bool& supportsHDR) const override;
        uint32_t Atmos(bool& supportsAtmos) const override;
        uint32_t CEC(bool& supportsCEC) const override;
        uint32_t HDCP(CopyProtection& supportedHDCP) const override;
        uint32_t ModelName(string& value) const override ; 
        uint32_t ModelYear(uint16_t& value) const override; 
        uint32_t FriendlyName(string& value) const override; 
        uint32_t SystemIntegratorName(string& value) const override; 
        uint32_t PlatformName(string& value) const override; 

    private:
        Config _config;
        bool _supportsHdr;
        bool _supportsAtmos;
        bool _supportsCEC;
        CopyProtection _supportedHDCP;
        std::list<IDeviceCapabilities::AudioOutput> _audio;
        std::list<IDeviceCapabilities::VideoOutput> _video;
        std::list<IDeviceCapabilities::OutputResolution> _resolution;
        string _modelName;
        uint16_t _modelYear;
        string _friendlyName;
        string _systemIntegratorName;
        string _platformName;
    };
} //namespace WPEFramework
} //namespace Plugin
