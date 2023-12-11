#include "DeviceAudioCapabilities.h"

#include "exception.hpp"
#include "host.hpp"
#include "manager.hpp"

#include "UtilsIarm.h"

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(DeviceAudioCapabilities, 1, 0);

    DeviceAudioCapabilities::DeviceAudioCapabilities()
    {
        Utils::IARM::init();
    }

    uint32_t DeviceAudioCapabilities::SupportedAudioPorts(RPC::IStringIterator*& supportedAudioPorts) const
    {
        uint32_t result = Core::ERROR_NONE;

        std::list<string> list;

        try {
            const auto& aPorts = device::Host::getInstance().getAudioOutputPorts();
            for (size_t i = 0; i < aPorts.size(); i++) {
                list.emplace_back(aPorts.at(i).getName());
            }
        } catch (const device::Exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
            result = Core::ERROR_GENERAL;
        } catch (const std::exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
            result = Core::ERROR_GENERAL;
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        if (result == Core::ERROR_NONE) {
            supportedAudioPorts = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(list));
        }

        return result;
    }

    uint32_t DeviceAudioCapabilities::AudioCapabilities(const string& audioPort, Exchange::IDeviceAudioCapabilities::IAudioCapabilityIterator*& audioCapabilities) const
    {
        uint32_t result = Core::ERROR_NONE;

        std::list<Exchange::IDeviceAudioCapabilities::AudioCapability> list;

        int capabilities = dsAUDIOSUPPORT_NONE;

        try {
            auto strAudioPort = audioPort.empty() ? device::Host::getInstance().getDefaultAudioPortName() : audioPort;
            auto& aPort = device::Host::getInstance().getAudioOutputPort(strAudioPort);
            aPort.getAudioCapabilities(&capabilities);
        } catch (const device::Exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
            result = Core::ERROR_GENERAL;
        } catch (const std::exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
            result = Core::ERROR_GENERAL;
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        if (!capabilities)
            list.emplace_back(Exchange::IDeviceAudioCapabilities::AudioCapability::AUDIOCAPABILITY_NONE);
        if (capabilities & dsAUDIOSUPPORT_ATMOS)
            list.emplace_back(Exchange::IDeviceAudioCapabilities::AudioCapability::ATMOS);
        if (capabilities & dsAUDIOSUPPORT_DD)
            list.emplace_back(Exchange::IDeviceAudioCapabilities::AudioCapability::DD);
        if (capabilities & dsAUDIOSUPPORT_DDPLUS)
            list.emplace_back(Exchange::IDeviceAudioCapabilities::AudioCapability::DDPLUS);
        if (capabilities & dsAUDIOSUPPORT_DAD)
            list.emplace_back(Exchange::IDeviceAudioCapabilities::AudioCapability::DAD);
        if (capabilities & dsAUDIOSUPPORT_DAPv2)
            list.emplace_back(Exchange::IDeviceAudioCapabilities::AudioCapability::DAPV2);
        if (capabilities & dsAUDIOSUPPORT_MS12)
            list.emplace_back(Exchange::IDeviceAudioCapabilities::AudioCapability::MS12);

        if (result == Core::ERROR_NONE) {
            audioCapabilities = (Core::Service<RPC::IteratorType<Exchange::IDeviceAudioCapabilities::IAudioCapabilityIterator>>::Create<Exchange::IDeviceAudioCapabilities::IAudioCapabilityIterator>(list));
        }

        return result;
    }

    uint32_t DeviceAudioCapabilities::MS12Capabilities(const string& audioPort, Exchange::IDeviceAudioCapabilities::IMS12CapabilityIterator*& ms12Capabilities) const
    {
        uint32_t result = Core::ERROR_NONE;

        std::list<Exchange::IDeviceAudioCapabilities::MS12Capability> list;

        int capabilities = dsMS12SUPPORT_NONE;

        try {
            auto strAudioPort = audioPort.empty() ? device::Host::getInstance().getDefaultAudioPortName() : audioPort;
            auto& aPort = device::Host::getInstance().getAudioOutputPort(strAudioPort);
            aPort.getMS12Capabilities(&capabilities);
        } catch (const device::Exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
            result = Core::ERROR_GENERAL;
        } catch (const std::exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
            result = Core::ERROR_GENERAL;
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        if (!capabilities)
            list.emplace_back(Exchange::IDeviceAudioCapabilities::MS12Capability::MS12CAPABILITY_NONE);
        if (capabilities & dsMS12SUPPORT_DolbyVolume)
            list.emplace_back(Exchange::IDeviceAudioCapabilities::MS12Capability::DOLBYVOLUME);
        if (capabilities & dsMS12SUPPORT_InteligentEqualizer)
            list.emplace_back(Exchange::IDeviceAudioCapabilities::MS12Capability::INTELIGENTEQUALIZER);
        if (capabilities & dsMS12SUPPORT_DialogueEnhancer)
            list.emplace_back(Exchange::IDeviceAudioCapabilities::MS12Capability::DIALOGUEENHANCER);

        if (result == Core::ERROR_NONE) {
            ms12Capabilities = (Core::Service<RPC::IteratorType<Exchange::IDeviceAudioCapabilities::IMS12CapabilityIterator>>::Create<Exchange::IDeviceAudioCapabilities::IMS12CapabilityIterator>(list));
        }

        return result;
    }

    uint32_t DeviceAudioCapabilities::SupportedMS12AudioProfiles(const string& audioPort, RPC::IStringIterator*& supportedMS12AudioProfiles) const
    {
        uint32_t result = Core::ERROR_NONE;

        std::list<string> list;

        try {
            auto strAudioPort = audioPort.empty() ? device::Host::getInstance().getDefaultAudioPortName() : audioPort;
            auto& aPort = device::Host::getInstance().getAudioOutputPort(strAudioPort);
            const auto supportedProfiles = aPort.getMS12AudioProfileList();
            for (size_t i = 0; i < supportedProfiles.size(); i++) {
                list.emplace_back(supportedProfiles.at(i));
            }
        } catch (const device::Exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
            result = Core::ERROR_GENERAL;
        } catch (const std::exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
            result = Core::ERROR_GENERAL;
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        if (result == Core::ERROR_NONE) {
            supportedMS12AudioProfiles = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(list));
        }

        return result;
    }
}
}
