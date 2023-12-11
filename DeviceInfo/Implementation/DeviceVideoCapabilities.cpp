#include "DeviceVideoCapabilities.h"

#include "exception.hpp"
#include "host.hpp"
#include "manager.hpp"
#include "videoOutputPortConfig.hpp"

#include "UtilsIarm.h"

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(DeviceVideoCapabilities, 1, 0);

    DeviceVideoCapabilities::DeviceVideoCapabilities()
    {
        Utils::IARM::init();
    }

    uint32_t DeviceVideoCapabilities::SupportedVideoDisplays(RPC::IStringIterator*& supportedVideoDisplays) const
    {
        uint32_t result = Core::ERROR_NONE;

        std::list<string> list;

        try {
            const auto& vPorts = device::Host::getInstance().getVideoOutputPorts();
            for (size_t i = 0; i < vPorts.size(); i++) {

                /**
                 * There's N:1 relation between VideoOutputPort and AudioOutputPort.
                 * When there are multiple Audio Ports on the Video Port,
                 * there are multiple VideoOutputPort-s as well.
                 * Those VideoOutputPort-s are the same except holding a different Audio Port id.
                 * As a result, a list of Video Ports has multiple Video Ports
                 * that represent the same Video Port, but different Audio Port.
                 * A list of VideoOutputPort-s returned from DS
                 * needs to be filtered by name.
                 */

                auto name = vPorts.at(i).getName();
                if (std::find(list.begin(), list.end(), name) != list.end())
                    continue;

                list.emplace_back(name);
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

    uint32_t DeviceVideoCapabilities::HostEDID(string& edid) const
    {
        uint32_t result = Core::ERROR_NONE;

        std::vector<uint8_t> edidVec({ 'u', 'n', 'k', 'n', 'o', 'w', 'n' });
        try {
            std::vector<unsigned char> edidVec2;
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

            if (edidVec.size() > (size_t)std::numeric_limits<uint16_t>::max()) {
                result = Core::ERROR_GENERAL;
            } else {
                string base64String;
                Core::ToString((uint8_t*)&edidVec[0], edidVec.size(), true, base64String);
                edid = base64String;
            }
        }

        return result;
    }

    uint32_t DeviceVideoCapabilities::DefaultResolution(const string& videoDisplay, string& defaultResolution) const
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

    uint32_t DeviceVideoCapabilities::SupportedResolutions(const string& videoDisplay, RPC::IStringIterator*& supportedResolutions) const
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

    uint32_t DeviceVideoCapabilities::SupportedHdcp(const string& videoDisplay, Exchange::IDeviceVideoCapabilities::CopyProtection& supportedHDCPVersion) const
    {
        uint32_t result = Core::ERROR_NONE;

        try {
            auto strVideoPort = videoDisplay.empty() ? device::Host::getInstance().getDefaultVideoPortName() : videoDisplay;
            auto& vPort = device::VideoOutputPortConfig::getInstance().getPort(strVideoPort);
            switch (vPort.getHDCPProtocol()) {
            case dsHDCP_VERSION_2X:
                supportedHDCPVersion = Exchange::IDeviceVideoCapabilities::CopyProtection::HDCP_22;
                break;
            case dsHDCP_VERSION_1X:
                supportedHDCPVersion = Exchange::IDeviceVideoCapabilities::CopyProtection::HDCP_14;
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
}
}
