#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <thread>
#include <string>
#include <map>
#include <NetworkManager.h>
#include <libnm/NetworkManager.h>
#include "Module.h"

#include "NetworkManagerLogger.h"
#include "NetworkManagerGnomeUtils.h"
#include "NetworkManagerImplementation.h"
#include "INetworkManager.h"

namespace WPEFramework
{
    namespace Plugin
    {
       uint8_t nmUtils::wifiSecurityModeFromAp(guint32 flags, guint32 wpaFlags, guint32 rsnFlags)
       {
            uint8_t security = Exchange::INetworkManager::WIFI_SECURITY_NONE;
            if ((flags == NM_802_11_AP_FLAGS_NONE) && (wpaFlags == NM_802_11_AP_SEC_NONE) && (rsnFlags == NM_802_11_AP_SEC_NONE))
            {
                security = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_NONE;
            }
            else if( (flags & NM_802_11_AP_FLAGS_PRIVACY) && ((wpaFlags & NM_802_11_AP_SEC_PAIR_WEP40) || (rsnFlags & NM_802_11_AP_SEC_PAIR_WEP40)) )
            {
                security = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WEP_64;
            }
            else if( (flags & NM_802_11_AP_FLAGS_PRIVACY) && ((wpaFlags & NM_802_11_AP_SEC_PAIR_WEP104) || (rsnFlags & NM_802_11_AP_SEC_PAIR_WEP104)) )
            {
                security = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WEP_128;
            }
            else if((wpaFlags & NM_802_11_AP_SEC_PAIR_TKIP) || (rsnFlags & NM_802_11_AP_SEC_PAIR_TKIP))
            {
                security = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_PSK_TKIP;
            }
            else if((wpaFlags & NM_802_11_AP_SEC_PAIR_CCMP) || (rsnFlags & NM_802_11_AP_SEC_PAIR_CCMP))
            {
                security = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_PSK_AES;
            }
            else if ((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_PSK) && (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X))
            {
                security = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_WPA2_ENTERPRISE;
            }
            else if(rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_PSK)
            {
                security = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_WPA2_PSK;
            }
            else if((wpaFlags & NM_802_11_AP_SEC_GROUP_CCMP) || (rsnFlags & NM_802_11_AP_SEC_GROUP_CCMP))
            {
                security = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA2_PSK_AES;
            }
            else if((wpaFlags & NM_802_11_AP_SEC_GROUP_TKIP) || (rsnFlags & NM_802_11_AP_SEC_GROUP_TKIP))
            {
                security = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA2_PSK_TKIP;
            }
            else if((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE) || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM))
            {
                security = Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA3_SAE;
            }
            else
                NMLOG_WARNING("security mode not defined (flag: %d, wpaFlags: %d, rsnFlags: %d)", flags, wpaFlags, rsnFlags);
            return security;
       }

        std::string nmUtils::getSecurityModeString(guint32 flags, guint32 wpaFlags, guint32 rsnFlags)
        {
            switch(flags)
            {
                case NM_802_11_AP_FLAGS_NONE:
                    NMLOG_TRACE("ap type : point has no special capabilities");
                    break;
                case NM_802_11_AP_FLAGS_PRIVACY:
                    NMLOG_TRACE("ap type : access point requires authentication and encryption");
                    break;
                case NM_802_11_AP_FLAGS_WPS:
                    NMLOG_TRACE("ap type : access point supports some WPS method");
                    break;
                case NM_802_11_AP_FLAGS_WPS_PBC:
                    NMLOG_TRACE("ap type : access point supports push-button WPS");
                    break;
                case NM_802_11_AP_FLAGS_WPS_PIN:
                    NMLOG_TRACE("ap type : access point supports PIN-based WPS");
                    break;
                default:
                    NMLOG_ERROR("ap type : 802.11 flags unknown!");
            }

            std::string securityStr;

            if (!(flags & NM_802_11_AP_FLAGS_PRIVACY) && (wpaFlags != NM_802_11_AP_SEC_NONE) && (rsnFlags != NM_802_11_AP_SEC_NONE))
                securityStr += ("Encrypted: ");

            if ((flags & NM_802_11_AP_FLAGS_PRIVACY) && (wpaFlags == NM_802_11_AP_SEC_NONE)
                && (rsnFlags == NM_802_11_AP_SEC_NONE))
                securityStr += ("WEP ");
            if (wpaFlags != NM_802_11_AP_SEC_NONE)
                securityStr += ("WPA ");
            if ((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_PSK)
                || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
                securityStr += ("WPA2 ");
            }
            if (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_SAE) {
                securityStr += ("WPA3 ");
            }
            if ((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE)
                || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM)) {
                securityStr += ("OWE ");
            }
            if ((wpaFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)
                || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
                securityStr += ("802.1X ");
            }

            if (securityStr.empty())
            {
                securityStr = "None";
                return securityStr;
            }

            uint32_t flags[2] = { wpaFlags, rsnFlags };
            securityStr += "[ WPA Flags: ";
            
            for (int i = 0; i < 2; ++i)
            {
                if (flags[i] & NM_802_11_AP_SEC_PAIR_WEP40)
                    securityStr += "pair_wep40 ";
                if (flags[i] & NM_802_11_AP_SEC_PAIR_WEP104)
                    securityStr += "pair_wep104 ";
                if (flags[i] & NM_802_11_AP_SEC_PAIR_TKIP)
                    securityStr += "pair_tkip ";
                if (flags[i] & NM_802_11_AP_SEC_PAIR_CCMP)
                    securityStr += "pair_ccmp ";
                if (flags[i] & NM_802_11_AP_SEC_GROUP_WEP40)
                    securityStr += "group_wep40 ";
                if (flags[i] & NM_802_11_AP_SEC_GROUP_WEP104)
                    securityStr += "group_wep104 ";
                if (flags[i] & NM_802_11_AP_SEC_GROUP_TKIP)
                    securityStr += "group_tkip ";
                if (flags[i] & NM_802_11_AP_SEC_GROUP_CCMP)
                    securityStr += "group_ccmp ";
                if (flags[i] & NM_802_11_AP_SEC_KEY_MGMT_PSK)
                    securityStr += "psk ";
                if (flags[i] & NM_802_11_AP_SEC_KEY_MGMT_802_1X)
                    securityStr += "802.1X ";
                if (flags[i] & NM_802_11_AP_SEC_KEY_MGMT_SAE)
                    securityStr += "sae ";
                if (flags[i] & NM_802_11_AP_SEC_KEY_MGMT_OWE)
                    securityStr += "owe ";
                if (flags[i] & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM)
                    securityStr += "owe_transition_mode ";
                if (flags[i] & NM_802_11_AP_SEC_KEY_MGMT_EAP_SUITE_B_192)
                    securityStr += "wpa-eap-suite-b-192 ";
                
                if (i == 0) {
                    securityStr += "] [ RSN Flags: ";
                }
            }
            securityStr +="]";
            return securityStr;
        }

       std::string nmUtils::wifiFrequencyFromAp(guint32 apFreq)
       {
            std:string freq;
            if (apFreq >= 2400 && apFreq < 5000)
                freq = "2.4";
            else if (apFreq >= 5000 && apFreq < 6000)
                freq = "5";
            else if (apFreq >= 6000)
                freq = "6";
            else
                freq = "Not available";

            return freq;
       }

       JsonObject nmUtils::apToJsonObject(NMAccessPoint *ap)
       {
            GError *error = NULL;
            GBytes *ssid = NULL;
            int strength = 0;
            std::string freq;
            int security;
            guint32 flags, wpaFlags, rsnFlags, apFreq;
            JsonObject ssidObj;
            if(ap == nullptr)
                return ssidObj;
            ssid = nm_access_point_get_ssid(ap);
            if (ssid)
            {
                char *ssidStr = nullptr;
                ssidStr = nm_utils_ssid_to_utf8((const guint8*)g_bytes_get_data(ssid, NULL), g_bytes_get_size(ssid));
                string ssidString(ssidStr);
                ssidObj["ssid"] = ssidString;
            }
            else
                ssidObj["ssid"] = "---"; // hidden ssid TODO modify
            strength = nm_access_point_get_strength(ap);
            apFreq   = nm_access_point_get_frequency(ap);
            flags    = nm_access_point_get_flags(ap);
            wpaFlags = nm_access_point_get_wpa_flags(ap);
            rsnFlags = nm_access_point_get_rsn_flags(ap);
            freq = nmUtils::wifiFrequencyFromAp(apFreq);
            security = nmUtils::wifiSecurityModeFromAp(flags, wpaFlags, rsnFlags);

            ssidObj["security"] = security;
            ssidObj["signalStrength"] = strength;
            ssidObj["frequency"] = freq;

            return ssidObj;
       }

        void nmUtils::printActiveSSIDsOnly(NMDeviceWifi *wifiDevice)
        {
            if(!NM_IS_DEVICE_WIFI(wifiDevice))
            {
                NMLOG_ERROR("Not a wifi object ");
                return;
            }
            const GPtrArray *accessPointsArray = nm_device_wifi_get_access_points(wifiDevice);
            for (guint i = 0; i < accessPointsArray->len; i++)
            {
                NMAccessPoint *ap = NULL;
                GBytes *ssidGByte = NULL;
                std::string ssid;

                ap = (NMAccessPoint*)accessPointsArray->pdata[i];
                ssidGByte = nm_access_point_get_ssid(ap);
                if(ssidGByte)
                {
                    char* ssidStr = NULL;
                    gsize len;
                    const guint8 *ssidData = static_cast<const guint8 *>(g_bytes_get_data(ssidGByte, &len));
                    ssidStr = nm_utils_ssid_to_utf8(ssidData, len);
                    if(ssidStr != NULL) {
                        std::string ssidTmp(ssidStr, len);
                        ssid = ssidTmp;
                    }
                    else
                        ssid = "---";
                }
                else
                    ssid = "---";
            
                NMLOG_INFO("ssid: %s", ssid.c_str());
            }
        }


        uint32_t nmUtils::GetInterfacesName(string &wifiInterface, string &ethernetInterface) {
            string line;
            uint32_t rc = Core::ERROR_GENERAL;

            ifstream file("/etc/device.properties");
            if (!file.is_open()) {
                NMLOG_WARNING("/etc/device.properties opening file Error ");
                return rc;
            }

            while (std::getline(file, line)) {
                // Remove newline character if present
                if (!line.empty() && line.back() == '\n') {
                    line.pop_back();
                }

                istringstream iss(line);
                string token;
                getline(iss, token, '=');

                if (token == "WIFI_INTERFACE") {
                    std::getline(iss, wifiInterface, '=');
                } else if (token == "ETHERNET_INTERFACE") {
                    std::getline(iss, ethernetInterface, '=');
                }
            }
            file.close();
            return Core::ERROR_NONE;
        }

    }   // Plugin
}   // WPEFramework
