#include "../../Module.h"
#include <interfaces/IDeviceInfo.h>

#include "exception.hpp"
#include "host.hpp"
#include "manager.hpp"
#include "videoOutputPortConfig.hpp"

#include "UtilsIarm.h"

namespace WPEFramework {
namespace Plugin {
    class DeviceVideoCapabilities : public Exchange::IDeviceVideoCapabilities {
    private:
        DeviceVideoCapabilities(const DeviceVideoCapabilities&) = delete;
        DeviceVideoCapabilities& operator=(const DeviceVideoCapabilities&) = delete;

    public:
        DeviceVideoCapabilities()
        {
            Utils::IARM::init();

            try {
                device::Manager::Initialize();
            } catch (const device::Exception& e) {
                TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
            } catch (const std::exception& e) {
                TRACE(Trace::Fatal, (_T("Exception caught %s"), e.what()));
            } catch (...) {
            }
        }

        ~DeviceVideoCapabilities() override;

        BEGIN_INTERFACE_MAP(DeviceVideoCapabilities)
        INTERFACE_ENTRY(Exchange::IDeviceVideoCapabilities)
        END_INTERFACE_MAP

    private:
        // IDeviceVideoCapabilities interface
        uint32_t SupportedVideoDisplays(RPC::IStringIterator*& supportedVideoDisplays) const override
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

        uint32_t HostEDID(string& edid) const override
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

                if (edidVec.size() > (size_t)numeric_limits<uint16_t>::max()) {
                    result = Core::ERROR_GENERAL;
                } else {
                    string base64String;
                    Core::ToString((uint8_t*)&edidVec[0], edidVec.size(), true, base64String);
                    edid = base64String;
                }
            }

            return result;
        }

        uint32_t DefaultResolution(const string& videoDisplay, string& defaultResolution) const override
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

        uint32_t SupportedResolutions(const string& videoDisplay, RPC::IStringIterator*& supportedResolutions) const override
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

        uint32_t SupportedHdcp(const string& videoDisplay, Exchange::IDeviceVideoCapabilities::CopyProtection& supportedHDCPVersion) const override
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
    };

    SERVICE_REGISTRATION(DeviceVideoCapabilities, 1, 0);
}
}
