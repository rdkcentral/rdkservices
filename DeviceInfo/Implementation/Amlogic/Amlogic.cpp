#include "../../Module.h"
#include <interfaces/IDeviceIdentification.h>

#include <fstream>

namespace WPEFramework {
namespace Plugin {
    class DeviceImplementation  : public Exchange::IDeviceIdentification , public PluginHost::ISubSystem::IIdentifier {
	    static constexpr const TCHAR* ChipsetInfo= _T("T962X3");
        static constexpr const TCHAR* VERSIONFile = _T("/version.txt");

    public:
        DeviceImplementation()
        {
            UpdateChipset(_chipset);
            UpdateFirmwareVersion(_firmwareVersion);
            UpdateIdentifier();
        }

        DeviceImplementation(const DeviceImplementation&) = delete;
        DeviceImplementation& operator=(const DeviceImplementation&) = delete;

        virtual ~DeviceImplementation()
        {
            /* Nothing to do here. */
        }

    public:
        // IIdentifier interface
        uint8_t Identifier(const uint8_t length, uint8_t buffer[]) const override
        {
            uint8_t result = 0;
            if ((_identity.length())) {
                result = (_identity.length() > length ? length : _identity.length());
                ::memcpy(buffer, _identity.c_str(), result);
            } else {
                SYSLOG(Logging::Notification, (_T("Cannot determine system identity")));
            }

            return result;
        }
        string Architecture() const override
        {
            return Core::SystemInfo::Instance().Architecture();
        }
        string Chipset() const override
        {
            return _chipset;
        }
        string FirmwareVersion() const override
        {
            return _firmwareVersion;
        }

        // IDeviceIdentification interface

        Core::hresult Identification(DeviceInfo& info) const override
        {
            info.deviceID = "";
            uint8_t myBuffer[64];

            myBuffer[0] = Identifier(sizeof(myBuffer) - 1, &(myBuffer[1]));

            if (myBuffer[0] != 0) {
                info.deviceID = Core::SystemInfo::Instance().Id(myBuffer, ~0);
            }

            info.firmwareVersion = FirmwareVersion();
            info.chipset = Chipset();

            return Core::ERROR_NONE;
        }

        BEGIN_INTERFACE_MAP(DeviceImplementation)
        INTERFACE_ENTRY(PluginHost::ISubSystem::IIdentifier)
        INTERFACE_ENTRY(Exchange::IDeviceIdentification)
        END_INTERFACE_MAP

    private:
        inline void UpdateFirmwareVersion(string& firmwareVersion) const
        {
            string line;
            std::ifstream file(VERSIONFile);

            if (file.is_open()) {
                while (getline(file, line)) {
                    if (line.find("SDK_VERSION") != std::string::npos) {
                        std::size_t position = line.find('=');
                        if (position != std::string::npos) {
                            firmwareVersion.assign(line.substr(position + 1, string::npos));
                            break;
                        }
                    }
                }

                file.close();
            }
        }

        inline void UpdateChipset(string& chipset) const
        {

#ifndef DEVICE_IDENTIFICATION_CHIPSET_INFO
             chipset.assign(ChipsetInfo);
#else
             chipset.assign(DEVICE_IDENTIFICATION_CHIPSET_INFO);
#endif
        }

        inline void UpdateIdentifier()
        {
            /*
           * @TODO : Update proper code for identifier when SOC ID is made
           * available for Amlogic boards
           */
            _identity.assign("");
        }

    private:
        string _chipset;
        string _firmwareVersion;
        string _identity;
    };

    SERVICE_REGISTRATION(DeviceImplementation, 1, 0);
}
}
