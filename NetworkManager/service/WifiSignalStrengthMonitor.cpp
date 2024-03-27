#include <cstdio>
#include <thread>
#include <chrono>
#include <atomic>
#include "NetworkManagerLogger.h"
#include "NetworkManagerImplementation.h"
#include "WifiSignalStrengthMonitor.h"

#define BUFFER_SIZE 512
#define rssid_command "wpa_cli signal_poll"
#define ssid_command "wpa_cli status"

namespace WPEFramework
{
    namespace Plugin
    {
        static const float signalStrengthThresholdExcellent = -50.0f;
        static const float signalStrengthThresholdGood = -60.0f;
        static const float signalStrengthThresholdFair = -67.0f;
        extern NetworkManagerImplementation* _instance;

        std::string WifiSignalStrengthMonitor::retrieveValues(const char *command, const char* keyword, char *output_buffer, size_t output_buffer_size)
        {
            std::string key, value;
            std::string keystr = "";

            FILE *fp = popen(command, "r");
            if (!fp)
            {
                NMLOG_ERROR("Failed in getting output from command %s \n",command);
                return keystr;
            }

            while ((!feof(fp)) && (fgets(output_buffer, output_buffer_size, fp) != NULL))
            {
                std::istringstream mystream(output_buffer);
                if(std::getline(std::getline(mystream, key, '=') >> std::ws, value))
                    if (key == keyword) {
                        keystr = value;
                        break;
                    }
            }
            pclose(fp);

            return keystr;
        }

        void WifiSignalStrengthMonitor::getSignalData(std::string &ssid, Exchange::INetworkManager::WiFiSignalQuality &quality, std::string &strengthOut)
        {
            float signalStrengthOut = 0.0f;
            char buff[BUFFER_SIZE] = {'\0'};

            ssid = retrieveValues(ssid_command, "ssid", buff, sizeof (buff));
            if (ssid.empty())
            {
                NMLOG_ERROR("ssid is empty");
                quality = Exchange::INetworkManager::WIFI_SIGNAL_DISCONNECTED;
                return;
            }

            string signalStrength = retrieveValues(rssid_command, "RSSI", buff, sizeof (buff));
            if (!signalStrength.empty())
                signalStrengthOut = std::stof(signalStrength.c_str());
            else
                NMLOG_ERROR("signalStrength is empty");

            NMLOG_TRACE("SSID = %s Signal Strength %f db", ssid.c_str(), signalStrengthOut);
            if (signalStrengthOut == 0.0f)
            {
                strengthOut = "Disconnected";
                quality = Exchange::INetworkManager::WIFI_SIGNAL_DISCONNECTED;
            }
            else if (signalStrengthOut >= signalStrengthThresholdExcellent && signalStrengthOut < 0)
            {
                strengthOut = "Excellent";
                quality = Exchange::INetworkManager::WIFI_SIGNAL_EXCELLENT;
            }
            else if (signalStrengthOut >= signalStrengthThresholdGood && signalStrengthOut < signalStrengthThresholdExcellent)
            {
                strengthOut = "Good";
                quality = Exchange::INetworkManager::WIFI_SIGNAL_GOOD;
            }
            else if (signalStrengthOut >= signalStrengthThresholdFair && signalStrengthOut < signalStrengthThresholdGood)
            {
                strengthOut = "Fair";
                quality = Exchange::INetworkManager::WIFI_SIGNAL_FAIR;
            }
            else
            {
                strengthOut = "Weak";
                quality = Exchange::INetworkManager::WIFI_SIGNAL_WEAK;
            };
        }

        void WifiSignalStrengthMonitor::startWifiSignalStrengthMonitor(int interval)
        {
            stopThread = false;
            if (isRunning) {
                NMLOG_INFO("WifiSignalStrengthMonitor Thread is already running.");
                return;
            }
            isRunning = true;
            monitorThread = std::thread(&WifiSignalStrengthMonitor::monitorThreadFunction, this, interval);
            monitorThread.detach();
            std::thread::id threadId = monitorThread.get_id();
            NMLOG_INFO("Thread started with interval: %d seconds. Thread ID: %lu", interval);
        }

        void WifiSignalStrengthMonitor::monitorThreadFunction(int interval)
        {
            static Exchange::INetworkManager::WiFiSignalQuality oldSignalQuality = Exchange::INetworkManager::WIFI_SIGNAL_DISCONNECTED;
            NMLOG_INFO("WifiSignalStrengthMonitor thread started !");
            while (!stopThread)
            {
                string ssid = "";
                string signalStrength;
                Exchange::INetworkManager::WiFiSignalQuality newSignalQuality;
                if (_instance != nullptr)
                {
                    NMLOG_TRACE("checking WiFi signal strength");
                    getSignalData(ssid, newSignalQuality, signalStrength);
                    if(oldSignalQuality != newSignalQuality)
                    {
                        NMLOG_INFO("Notifying WiFiSignalStrengthChangedEvent ...%s", signalStrength.c_str());
                        oldSignalQuality = newSignalQuality;
                        _instance->ReportWiFiSignalStrengthChangedEvent(ssid, signalStrength, newSignalQuality);
                    }

                    if(newSignalQuality == Exchange::INetworkManager::WIFI_SIGNAL_DISCONNECTED)
                    {
                        NMLOG_WARNING("WiFiSignalStrengthChanged to disconnect - WifiSignalStrengthMonitor exiting");
                        stopThread= false;
                        break; // Let the thread exit naturally
                    }
                }
                else
                    NMLOG_FATAL("NetworkManagerImplementation pointer error !");
                // Wait for the specified interval or until notified to stop
                std::this_thread::sleep_for(std::chrono::seconds(interval));
            }
            isRunning = false;
        }
    }
}