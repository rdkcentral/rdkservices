/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#pragma once

#include <thread>

#include "Module.h"
#include "UtilsThreadRAII.h"

#include "btmgr.h" //TODO: can we move it to the module? Required by notifyEventWrapper()

namespace WPEFramework {
    namespace Plugin {

        // This is a server for a JSONRPC communication channel.
        // For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
        // By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
        // This realization of this interface implements, by default, the following methods on this plugin
        // - exists
        // - register
        // - unregister
        // Any other methood to be handled by this plugin  can be added can be added by using the
        // templated methods Register on the PluginHost::JSONRPC class.
        // As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
        // this class exposes a public method called, Notify(), using this methods, all subscribed clients
        // will receive a JSONRPC message as a notification, in case this method is called.
        class Bluetooth;
        class DiscoveryTimer
        {
        private:
            DiscoveryTimer() = delete;
            DiscoveryTimer& operator=(const DiscoveryTimer& RHS) = delete;

        public:
            DiscoveryTimer(Bluetooth* bt): m_bt(bt){}
            DiscoveryTimer(const DiscoveryTimer& copy): m_bt(copy.m_bt){}
            ~DiscoveryTimer() {}

            inline bool operator==(const DiscoveryTimer& RHS) const
            {
                return(m_bt == RHS.m_bt);
            }

        public:
            uint64_t Timed(const uint64_t scheduledTime);

        private:
            Bluetooth* m_bt;
        };

        class Bluetooth : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:

            // We do not allow this plugin to be copied !!
            Bluetooth(const Bluetooth&) = delete;
            Bluetooth& operator=(const Bluetooth&) = delete;

            // Registered methods begin
            // Note: `JsonObject& parameters` corresponds to `params` in JSON RPC call
            uint32_t getApiVersionNumber(const JsonObject& parameters, JsonObject& response);
            uint32_t startScanWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t stopScanWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t isDiscoverableWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setDiscoverableWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getDiscoveredDevicesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getPairedDevicesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getConnectedDevicesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t connectWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t disconnectWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setAudioStreamWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t pairWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t unpairWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t enableWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t disableWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getNameWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setNameWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t sendAudioPlaybackCommandWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setEventResponseWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getDeviceInfoWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getMediaTrackInfoWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getDeviceVolumeMuteInfoWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setDeviceVolumeMuteInfoWrapper(const JsonObject& parameters, JsonObject& response);
            // Registered methods end

        private: /*internal methods*/
            void getStatusSupport(string& status);
            bool isAdapterDiscoverable();
            string startDeviceDiscovery(int timeout, const string &discProfile = "LOUDSPEAKER, HEADPHONES, WEARABLE HEADSET, HIFI AUDIO DEVICE");
            bool stopDeviceDiscovery();
            void startDiscoveryTimer(int msec);
            void stopDiscoveryTimer();
            void onDiscoveryTimer();
            JsonArray getDiscoveredDevices();
            JsonArray getPairedDevices();
            JsonArray getConnectedDevices();
            bool setDeviceConnection(long long int deviceID, const string &enable, const string &deviceType = "UNKNOWN DEVICE");
            bool setAudioStream(long long int deviceID, const string &audioStreamName);
            bool setDevicePairing(long long int deviceID, bool pair);
            bool setBluetoothEnabled(const string &enabled);
            bool setBluetoothDiscoverable(bool enabled, int timeout);
            bool getBluetoothProperties(JsonObject* rp);
            bool setBluetoothProperties(const JsonObject& properties);
            bool setAudioControlCommand(long long int  deviceID, const string &audioCtrlCmd);
            bool setEventResponse(long long int  deviceID, const string &eventType, const string &respValue);
            JsonObject getDeviceInfo(long long int deviceID);
            JsonObject getMediaTrackInfo(long long int deviceID);
            bool setDeviceVolumeMuteProperties(long long int  deviceID, const string &deviceProfile, unsigned char ui8volume, unsigned char mute);
            JsonObject getDeviceVolumeMuteProperties(long long int  deviceID, const string &deviceProfile);
            BTRMGR_DeviceOperationType_t btmgrDeviceOperationTypeFromString(const string &deviceProfile);


        public:
            static const string SERVICE_NAME;
            static const string METHOD_START_SCAN;
            static const string METHOD_STOP_SCAN;
            static const string METHOD_IS_DISCOVERABLE;
            static const string METHOD_GET_DISCOVERED_DEVICES;
            static const string METHOD_GET_PAIRED_DEVICES;
            static const string METHOD_GET_CONNECTED_DEVICES;
            static const string METHOD_CONNECT;
            static const string METHOD_DISCONNECT;
            static const string METHOD_SET_AUDIO_STREAM;
            static const string METHOD_PAIR;
            static const string METHOD_UNPAIR;
            static const string METHOD_ENABLE;
            static const string METHOD_DISABLE;
            static const string METHOD_SET_DISCOVERABLE;
            static const string METHOD_GET_NAME;
            static const string METHOD_SET_NAME;
            static const string METHOD_SET_AUDIO_PLAYBACK_COMMAND;
            static const string METHOD_SET_EVENT_RESPONSE;
            static const string METHOD_GET_DEVICE_INFO;
            static const string METHOD_GET_AUDIO_INFO;
            static const string METHOD_GET_API_VERSION_NUMBER;
            static const string METHOD_GET_DEVICE_VOLUME_MUTE_INFO;
            static const string METHOD_SET_DEVICE_VOLUME_MUTE_INFO;
            static const string EVT_STATUS_CHANGED;
            static const string EVT_PAIRING_REQUEST;
            static const string EVT_REQUEST_FAILED;
            static const string EVT_CONNECTION_REQUEST;

            static const string EVT_PLAYBACK_REQUEST;
            static const string EVT_PLAYBACK_STARTED;
            static const string EVT_PLAYBACK_PAUSED;
            static const string EVT_PLAYBACK_RESUMED;
            static const string EVT_PLAYBACK_STOPPED;
            static const string EVT_PLAYBACK_ENDED;

            static const string EVT_PLAYBACK_POSITION;
            static const string EVT_PLAYBACK_NEW_TRACK;
            static const string EVT_DEVICE_FOUND;
            static const string EVT_DEVICE_LOST_OR_OUT_OF_RANGE;
            static const string EVT_DEVICE_DISCOVERY_UPDATE;
            static const string EVT_DEVICE_MEDIA_STATUS;

            Bluetooth();
            virtual ~Bluetooth();
            virtual const string Initialize(PluginHost::IShell* shell) override { return {}; }
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override;

            BEGIN_INTERFACE_MAP(Bluetooth)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        public:
            static Bluetooth* _instance;
            void notifyEventWrapper (BTRMGR_EventMessage_t eventMsg);

        private:
            static const string STATUS_NO_BLUETOOTH_HARDWARE;
            static const string STATUS_SOFTWARE_DISABLED;
            static const string STATUS_AVAILABLE;
            static const string ENABLE_CONNECT;
            static const string ENABLE_DISCONNECT;
            static const string ENABLE_BLUETOOTH_ENABLED;
            static const string ENABLE_BLUETOOTH_DISABLED;
            static const string ENABLE_BLUETOOTH_INPUT_ENABLED;

            static const string ENABLE_PRIMARY_AUDIO;
            static const string ENABLE_AUXILIARY_AUDIO;
            static const string STATUS_HARDWARE_AVAILABLE;
            static const string STATUS_HARDWARE_DISABLED;
            static const string STATUS_SOFTWARE_ENABLED;
            static const string STATUS_SOFTWARE_INPUT_ENABLED;
            static const string STATUS_PAIRING_CHANGE;
            static const string STATUS_CONNECTION_CHANGE;
            static const string STATUS_DISCOVERY_STARTED;
            static const string STATUS_DISCOVERY_COMPLETED;
            static const string STATUS_PAIRING_FAILED;
            static const string STATUS_CONNECTION_FAILED;

            static const string CMD_AUDIO_CTRL_PLAY;
            static const string CMD_AUDIO_CTRL_STOP;
            static const string CMD_AUDIO_CTRL_PAUSE;
            static const string CMD_AUDIO_CTRL_RESUME;
            static const string CMD_AUDIO_CTRL_SKIP_NEXT;
            static const string CMD_AUDIO_CTRL_SKIP_PREV;
            static const string CMD_AUDIO_CTRL_RESTART;
            static const string CMD_AUDIO_CTRL_VOLUME_UP;
            static const string CMD_AUDIO_CTRL_VOLUME_DOWN;
            static const string CMD_AUDIO_CTRL_MUTE;
            static const string CMD_AUDIO_CTRL_UNMUTE;
            static const string CMD_AUDIO_CTRL_UNKNOWN;

            uint32_t m_apiVersionNumber;
            // Assuming that there will be only one threaded call at a time (which is the case for Bluetooth)
            // Otherwise we might need a thread for each async command for better performance
            Utils::ThreadRAII m_executionThread;
            bool m_discoveryRunning;
            DiscoveryTimer m_discoveryTimer;
            friend class DiscoveryTimer;
        };
	} // Plugin
} // WPEFramework
