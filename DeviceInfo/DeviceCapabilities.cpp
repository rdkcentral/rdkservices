#include "DeviceCapabilities.h"

#include <fstream>
#include <regex>

#include "rfcapi.h"

#include "exception.hpp"
#include "host.hpp"
#include "manager.hpp"
#include "videoOutputPortConfig.hpp"

#include "UtilsIarm.h"

namespace WPEFramework {
namespace Plugin {

    namespace {
        constexpr auto* kDeviceProperties = _T("/etc/device.properties");
        constexpr auto* kAuthserviceConf = _T("/etc/authService.conf");
        constexpr auto* kSerialNumberFile = _T("/proc/device-tree/serial-number");
        constexpr auto* kPartnerIdFile = _T("/opt/www/authService/partnerId3.dat");
        constexpr auto* kRfcPartnerId = _T("Device.DeviceInfo.X_RDKCENTRAL-COM_Syndication.PartnerId");
        constexpr auto* kRfcModelName = _T("Device.DeviceInfo.ModelName");
        constexpr auto* kRfcSerialNumber = _T("Device.DeviceInfo.SerialNumber");
        constexpr auto* kDefaultAudioPort = _T("HDMI0");
    }

    SERVICE_REGISTRATION(DeviceCapabilities, 1, 0);

    DeviceCapabilities::DeviceCapabilities()
    {
        // Make sure DS is Initialized

        // Not obvious but otherwise DS seg faults
        Utils::IARM::init();

        try {
            // Also not obvious but otherwise DS fails
            device::Manager::Initialize();
        } catch (const device::Exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
        } catch (const std::exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
        } catch (...) {
        }
    }

    DeviceCapabilities::~DeviceCapabilities()
    {
        // If other services continue using DS this probably shouldn't be called

        /*try {
            device::Manager::DeInitialize();
        } catch (const device::Exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
        } catch (const std::exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
        } catch (...) {
        }*/
    }

    uint32_t DeviceCapabilities::SerialNumber(string& serialNumber) const
    {
        uint32_t result = Core::ERROR_GENERAL;

        RFC_ParamData_t param;

        auto status = getRFCParameter(nullptr, kRfcSerialNumber, &param);

        if (status == WDMP_SUCCESS) {
            serialNumber = param.value;
            result = Core::ERROR_NONE;
        } else {
            std::ifstream file(kSerialNumberFile);

            if (file) {
                string line;
                if (std::getline(file, line)) {
                    serialNumber = line;
                    result = Core::ERROR_NONE;
                }
            }
        }

        return result;
    }

    uint32_t DeviceCapabilities::Sku(string& sku) const
    {
        uint32_t result = Core::ERROR_GENERAL;

        RFC_ParamData_t param;

        auto status = getRFCParameter(nullptr, kRfcModelName, &param);

        if (status == WDMP_SUCCESS) {
            sku = param.value;
            result = Core::ERROR_NONE;
        } else {
            std::ifstream file(kDeviceProperties);

            if (file) {
                string line;
                while (std::getline(file, line)) {
                    if (line.rfind(_T("MODEL_NUM"), 0) == 0) {
                        sku = line.substr(line.find('=') + 1);
                        result = Core::ERROR_NONE;

                        break;
                    }
                }
            }
        }

        return result;
    }

    uint32_t DeviceCapabilities::Make(string& make) const
    {
        uint32_t result = Core::ERROR_GENERAL;

        std::ifstream file(kDeviceProperties);

        if (file) {
            string line;
            while (std::getline(file, line)) {
                if (line.rfind(_T("MFG_NAME"), 0) == 0) {
                    make = line.substr(line.find('=') + 1);
                    result = Core::ERROR_NONE;

                    break;
                }
            }
        }

        return result;
    }

    uint32_t DeviceCapabilities::Model(string& model) const
    {
        uint32_t result = Core::ERROR_GENERAL;

        std::ifstream file(kDeviceProperties);

        if (file) {
            string line;
            while (std::getline(file, line)) {
                if (line.rfind(_T("FRIENDLY_ID"), 0) == 0) {
                    // trim quotes

                    model = std::regex_replace(line, std::regex(_T("^\\w+=(?:\")?([^\"\\n]+)(?:\")?$")), _T("$1"));
                    result = Core::ERROR_NONE;

                    break;
                }
            }
        }

        return result;
    }

    uint32_t DeviceCapabilities::DeviceType(string& deviceType) const
    {
        uint32_t result = Core::ERROR_GENERAL;

        std::ifstream file(kAuthserviceConf);

        if (file) {
            string line;
            while (std::getline(file, line)) {
                if (line.rfind(_T("deviceType"), 0) == 0) {
                    deviceType = line.substr(line.find('=') + 1);
                    result = Core::ERROR_NONE;

                    break;
                }
            }
        }

        return result;
    }

    uint32_t DeviceCapabilities::DistributorId(string& distributorId) const
    {
        uint32_t result = Core::ERROR_GENERAL;

        RFC_ParamData_t param;

        auto status = getRFCParameter(nullptr, kRfcPartnerId, &param);

        if (status == WDMP_SUCCESS) {
            distributorId = param.value;
            result = Core::ERROR_NONE;
        } else {
            std::ifstream file(kPartnerIdFile);

            if (file) {
                string line;
                if (std::getline(file, line)) {
                    distributorId = line;
                    result = Core::ERROR_NONE;
                }
            }
        }

        return result;
    }

    uint32_t DeviceCapabilities::SupportedAudioPorts(RPC::IStringIterator*& supportedAudioPorts) const
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

    uint32_t DeviceCapabilities::SupportedVideoDisplays(RPC::IStringIterator*& supportedVideoDisplays) const
    {
        uint32_t result = Core::ERROR_NONE;

        std::list<string> list;

        try {
            const auto& vPorts = device::Host::getInstance().getVideoOutputPorts();
            for (size_t i = 0; i < vPorts.size(); i++) {
                list.emplace_back(vPorts.at(i).getName());
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
            supportedVideoDisplays = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(list));
        }

        return result;
    }

    uint32_t DeviceCapabilities::HostEDID(string& edid) const
    {
        uint32_t result = Core::ERROR_NONE;

        vector<uint8_t> edidVec({ 'u', 'n', 'k', 'n', 'o', 'w', 'n' });
        try {
            vector<unsigned char> edidVec2;
            device::Host::getInstance().getHostEDID(edidVec2);
            edidVec = edidVec2;
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
            // convert to base64

            uint16_t size = min(edidVec.size(), (size_t)numeric_limits<uint16_t>::max());
            if (edidVec.size() > (size_t)numeric_limits<uint16_t>::max()) {
                result = Core::ERROR_GENERAL;
            } else {
                string base64String;
                Core::ToString((uint8_t*)&edidVec[0], size, true, base64String);
                edid = base64String;
            }
        }

        return result;
    }

    uint32_t DeviceCapabilities::DefaultResolution(const string& videoDisplay, string& defaultResolution) const
    {
        uint32_t result = Core::ERROR_NONE;

        try {
            auto strVideoPort = videoDisplay.empty() ? device::Host::getInstance().getDefaultVideoPortName() : videoDisplay;
            auto& vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort);
            defaultResolution = vPort.getDefaultResolution().getName();
        } catch (const device::Exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
            result = Core::ERROR_GENERAL;
        } catch (const std::exception& e) {
            TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
            result = Core::ERROR_GENERAL;
        } catch (...) {
            result = Core::ERROR_GENERAL;
        }

        return result;
    }

    uint32_t DeviceCapabilities::SupportedResolutions(const string& videoDisplay, RPC::IStringIterator*& supportedResolutions) const
    {
        uint32_t result = Core::ERROR_NONE;

        std::list<string> list;

        try {
            auto strVideoPort = videoDisplay.empty() ? device::Host::getInstance().getDefaultVideoPortName() : videoDisplay;
            auto& vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort);
            const auto resolutions = device::VideoOutputPortConfig::getInstance().getPortType(vPort.getType().getId()).getSupportedResolutions();
            for (size_t i = 0; i < resolutions.size(); i++) {
                list.emplace_back(resolutions.at(i).getName());
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
            supportedResolutions = (Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(list));
        }

        return result;
    }

    uint32_t DeviceCapabilities::SupportedHdcp(const string& videoDisplay, Exchange::IDeviceCapabilities::CopyProtection& supportedHDCPVersion) const
    {
        uint32_t result = Core::ERROR_NONE;

        try {
            auto strVideoPort = videoDisplay.empty() ? device::Host::getInstance().getDefaultVideoPortName() : videoDisplay;
            auto& vPort = device::VideoOutputPortConfig::getInstance().getPort(strVideoPort);
            switch (vPort.getHDCPProtocol()) {
            case dsHDCP_VERSION_2X:
                supportedHDCPVersion = Exchange::IDeviceCapabilities::CopyProtection::HDCP_22;
                break;
            case dsHDCP_VERSION_1X:
                supportedHDCPVersion = Exchange::IDeviceCapabilities::CopyProtection::HDCP_14;
                break;
            default:
                result = Core::ERROR_GENERAL;
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

        return result;
    }

    uint32_t DeviceCapabilities::AudioCapabilities(const string& audioPort, Exchange::IDeviceCapabilities::IAudioCapabilityIterator*& audioCapabilities) const
    {
        uint32_t result = Core::ERROR_NONE;

        std::list<Exchange::IDeviceCapabilities::AudioCapability> list;

        int capabilities = dsAUDIOSUPPORT_NONE;

        try {
            auto strAudioPort = audioPort.empty() ? string(kDefaultAudioPort) : audioPort;
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
            list.emplace_back(Exchange::IDeviceCapabilities::AudioCapability::AUDIOCAPABILITY_NONE);
        if (capabilities & dsAUDIOSUPPORT_ATMOS)
            list.emplace_back(Exchange::IDeviceCapabilities::AudioCapability::ATMOS);
        if (capabilities & dsAUDIOSUPPORT_DD)
            list.emplace_back(Exchange::IDeviceCapabilities::AudioCapability::DD);
        if (capabilities & dsAUDIOSUPPORT_DDPLUS)
            list.emplace_back(Exchange::IDeviceCapabilities::AudioCapability::DDPLUS);
        if (capabilities & dsAUDIOSUPPORT_DAD)
            list.emplace_back(Exchange::IDeviceCapabilities::AudioCapability::DAD);
        if (capabilities & dsAUDIOSUPPORT_DAPv2)
            list.emplace_back(Exchange::IDeviceCapabilities::AudioCapability::DAPV2);
        if (capabilities & dsAUDIOSUPPORT_MS12)
            list.emplace_back(Exchange::IDeviceCapabilities::AudioCapability::MS12);

        if (result == Core::ERROR_NONE) {
            audioCapabilities = (Core::Service<RPC::IteratorType<Exchange::IDeviceCapabilities::IAudioCapabilityIterator>>::Create<Exchange::IDeviceCapabilities::IAudioCapabilityIterator>(list));
        }

        return result;
    }

    uint32_t DeviceCapabilities::MS12Capabilities(const string& audioPort, Exchange::IDeviceCapabilities::IMS12CapabilityIterator*& ms12Capabilities) const
    {
        uint32_t result = Core::ERROR_NONE;

        std::list<Exchange::IDeviceCapabilities::MS12Capability> list;

        int capabilities = dsMS12SUPPORT_NONE;

        try {
            auto strAudioPort = audioPort.empty() ? string(kDefaultAudioPort) : audioPort;
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
            list.emplace_back(Exchange::IDeviceCapabilities::MS12Capability::MS12CAPABILITY_NONE);
        if (capabilities & dsMS12SUPPORT_DolbyVolume)
            list.emplace_back(Exchange::IDeviceCapabilities::MS12Capability::DOLBYVOLUME);
        if (capabilities & dsMS12SUPPORT_InteligentEqualizer)
            list.emplace_back(Exchange::IDeviceCapabilities::MS12Capability::INTELIGENTEQUALIZER);
        if (capabilities & dsMS12SUPPORT_DialogueEnhancer)
            list.emplace_back(Exchange::IDeviceCapabilities::MS12Capability::DIALOGUEENHANCER);

        if (result == Core::ERROR_NONE) {
            ms12Capabilities = (Core::Service<RPC::IteratorType<Exchange::IDeviceCapabilities::IMS12CapabilityIterator>>::Create<Exchange::IDeviceCapabilities::IMS12CapabilityIterator>(list));
        }

        return result;
    }

    uint32_t DeviceCapabilities::SupportedMS12AudioProfiles(const string& audioPort, RPC::IStringIterator*& supportedMS12AudioProfiles) const
    {
        uint32_t result = Core::ERROR_NONE;

        std::list<string> list;

        try {
            auto strAudioPort = audioPort.empty() ? string(kDefaultAudioPort) : audioPort;
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

} // namespace Plugin
} // namespace WPEFramework
