#include "NetworkManagerImplementation.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace std;

namespace WPEFramework
{
    namespace Plugin
    {
        const float signalStrengthThresholdExcellent = -50.0f;
        const float signalStrengthThresholdGood = -60.0f;
        const float signalStrengthThresholdFair = -67.0f;
        NetworkManagerImplementation* _instance = nullptr;

        void NetworkManagerImplementation::platform_init()
        {
            LOG_ENTRY_FUNCTION();

            ::_instance = this;

            // TODO
        }

        uint32_t NetworkManagerImplementation::GetAvailableInterfaces (Exchange::INetworkManager::IInterfaceDetailsIterator*& interfacesItr/* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;

            return rc;
        }

        /* @brief Get the active Interface used for external world communication */
        uint32_t NetworkManagerImplementation::GetPrimaryInterface (string& interface /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }

        /* @brief Set the active Interface used for external world communication */
        uint32_t NetworkManagerImplementation::SetPrimaryInterface (const string& interface/* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::SetInterfaceEnabled (const string& interface/* @in */, const bool& isEnabled /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;

            return rc;
        }

        /* @brief Get IP Address Of the Interface */
        uint32_t NetworkManagerImplementation::GetIPSettings(const string& interface /* @in */, const string& ipversion /* @in */, IPAddressInfo& result /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;

            return rc;
        }

        /* @brief Set IP Address Of the Interface */
        uint32_t NetworkManagerImplementation::SetIPSettings(const string& interface /* @in */, const string &ipversion /* @in */, const IPAddressInfo& address /* @in */)
        {
            uint32_t rc = Core::ERROR_NONE;

            return rc;
        }

        uint32_t NetworkManagerImplementation::StartWiFiScan(const WiFiFrequency frequency /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::StopWiFiScan(void)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetKnownSSIDs(IStringIterator*& ssids /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::AddToKnownSSIDs(const WiFiConnectTo& ssid /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::RemoveKnownSSID(const string& ssid /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::WiFiConnect(const WiFiConnectTo& ssid /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::WiFiDisconnect(void)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetConnectedSSID(WiFiSSIDInfo&  ssidInfo /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetWiFiSignalStrength(string& ssid /* @out */, string& signalStrength /* @out */, WiFiSignalQuality& quality /* @out */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::StartWPS(const WiFiWPS& method /* @in */, const string& wps_pin /* @in */)
        {
            LOG_ENTRY_FUNCTION();
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }

        uint32_t NetworkManagerImplementation::StopWPS(void)
        {
            uint32_t rc = Core::ERROR_NONE;
            return rc;
        }
    }
}
