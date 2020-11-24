#include "DeviceInfoImplementation.h"

namespace WPEFramework {
namespace Plugin {
    SERVICE_REGISTRATION(DeviceInfoImplementation, 1, 0);

    uint32_t Configure(const PluginHost::IShell* service)
    {
    }
    uint32_t DeviceInfoImplementation::SupportedResolutions(IOutputResolutionIterator*& res) const {}
    uint32_t DeviceInfoImplementation::SupportedAudioOutputs(IAudioOutputIterator*& res) const {}
    uint32_t DeviceInfoImplementation::SupportedVideoOutputs(IVideoOutputIterator*& res) const {}
    uint32_t DeviceInfoImplementation::SupportsHDR(bool& supportsHDR) const {}
    uint32_t DeviceInfoImplementation::SupportsAtmos(bool& supportsAtmos) const {}
}
}