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

#include <fstream>

#include "Bluetooth.h"

#include "UtilsUnused.h"
#include "UtilsCStr.h"
#include "UtilsIarm.h"
#include "UtilsString.h"
#include "UtilsJsonRpc.h"

#include <stdlib.h>

// IMPLEMENTATION NOTE
//
// Bluetooth Settings API in Thunder follows the schema proposed by Metrological what differs from the underlying
// Bluetooth Manager API in RDK, which the plugin calls.
// As a result, the exposed (registered) methods are implemented as wrappers that follow the Metrological notation. The wrappers then call the private methods
// with RDK naming schema  which, in turn, call to actual Bluetooth Manager functions. These "internal" methods are similar to what we had in Service Manager as public APIs.
// For example, the exposed "startScan" method is mapped to "startScanWrapper()" and that one calls to "startDeviceDiscovery()" internally,
// which finally calls to "BTRMGR_StartDeviceDiscovery()" in Bluetooth Manager.

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

const string WPEFramework::Plugin::Bluetooth::SERVICE_NAME = "org.rdk.Bluetooth";
const string WPEFramework::Plugin::Bluetooth::METHOD_START_SCAN = "startScan";
const string WPEFramework::Plugin::Bluetooth::METHOD_STOP_SCAN = "stopScan";
const string WPEFramework::Plugin::Bluetooth::METHOD_IS_DISCOVERABLE = "isDiscoverable";
const string WPEFramework::Plugin::Bluetooth::METHOD_GET_DISCOVERED_DEVICES = "getDiscoveredDevices";
const string WPEFramework::Plugin::Bluetooth::METHOD_GET_PAIRED_DEVICES = "getPairedDevices";
const string WPEFramework::Plugin::Bluetooth::METHOD_GET_CONNECTED_DEVICES = "getConnectedDevices";
const string WPEFramework::Plugin::Bluetooth::METHOD_CONNECT = "connect";
const string WPEFramework::Plugin::Bluetooth::METHOD_DISCONNECT = "disconnect";
const string WPEFramework::Plugin::Bluetooth::METHOD_SET_AUDIO_STREAM = "setAudioStream";
const string WPEFramework::Plugin::Bluetooth::METHOD_PAIR = "pair";
const string WPEFramework::Plugin::Bluetooth::METHOD_UNPAIR = "unpair";
const string WPEFramework::Plugin::Bluetooth::METHOD_ENABLE = "enable";
const string WPEFramework::Plugin::Bluetooth::METHOD_DISABLE = "disable";
const string WPEFramework::Plugin::Bluetooth::METHOD_SET_DISCOVERABLE = "setDiscoverable";
const string WPEFramework::Plugin::Bluetooth::METHOD_GET_NAME = "getName";
const string WPEFramework::Plugin::Bluetooth::METHOD_SET_NAME = "setName";
const string WPEFramework::Plugin::Bluetooth::METHOD_SET_AUDIO_PLAYBACK_COMMAND = "sendAudioPlaybackCommand";
const string WPEFramework::Plugin::Bluetooth::METHOD_SET_EVENT_RESPONSE = "respondToEvent";
const string WPEFramework::Plugin::Bluetooth::METHOD_GET_DEVICE_INFO = "getDeviceInfo";
const string WPEFramework::Plugin::Bluetooth::METHOD_GET_AUDIO_INFO = "getAudioInfo";
const string WPEFramework::Plugin::Bluetooth::METHOD_GET_API_VERSION_NUMBER = "getApiVersionNumber";
const string WPEFramework::Plugin::Bluetooth::METHOD_GET_DEVICE_VOLUME_MUTE_INFO = "getDeviceVolumeMuteInfo";
const string WPEFramework::Plugin::Bluetooth::METHOD_SET_DEVICE_VOLUME_MUTE_INFO = "setDeviceVolumeMuteInfo";

const string WPEFramework::Plugin::Bluetooth::EVT_STATUS_CHANGED = "onStatusChanged";
const string WPEFramework::Plugin::Bluetooth::EVT_PAIRING_REQUEST = "onPairingRequest";
const string WPEFramework::Plugin::Bluetooth::EVT_REQUEST_FAILED = "onRequestFailed";
const string WPEFramework::Plugin::Bluetooth::EVT_CONNECTION_REQUEST = "onConnectionRequest";
const string WPEFramework::Plugin::Bluetooth::EVT_PLAYBACK_REQUEST = "onPlaybackRequest";

const string WPEFramework::Plugin::Bluetooth::EVT_PLAYBACK_STARTED = "onPlaybackChange"; // action: started
const string WPEFramework::Plugin::Bluetooth::EVT_PLAYBACK_PAUSED = "onPlaybackChange";  // action: paused
const string WPEFramework::Plugin::Bluetooth::EVT_PLAYBACK_STOPPED = "onPlaybackChange"; // action: stopped
const string WPEFramework::Plugin::Bluetooth::EVT_PLAYBACK_ENDED = "onPlaybackChange";   // action: paused

const string WPEFramework::Plugin::Bluetooth::EVT_PLAYBACK_POSITION = "onPlaybackProgress";
const string WPEFramework::Plugin::Bluetooth::EVT_PLAYBACK_NEW_TRACK = "onPlaybackNewTrack";
const string WPEFramework::Plugin::Bluetooth::EVT_DEVICE_FOUND = "onDeviceFound";
const string WPEFramework::Plugin::Bluetooth::EVT_DEVICE_LOST_OR_OUT_OF_RANGE = "onDeviceLost";
const string WPEFramework::Plugin::Bluetooth::EVT_DEVICE_DISCOVERY_UPDATE = "onDiscoveredDevice";
const string WPEFramework::Plugin::Bluetooth::EVT_DEVICE_MEDIA_STATUS = "onDeviceMediaStatus";

const string WPEFramework::Plugin::Bluetooth::STATUS_NO_BLUETOOTH_HARDWARE = "NO_BLUETOOTH_HARDWARE";
const string WPEFramework::Plugin::Bluetooth::STATUS_SOFTWARE_DISABLED = "SOFTWARE_DISABLED";
const string WPEFramework::Plugin::Bluetooth::STATUS_AVAILABLE = "AVAILABLE";
const string WPEFramework::Plugin::Bluetooth::ENABLE_CONNECT = "CONNECT";
const string WPEFramework::Plugin::Bluetooth::ENABLE_DISCONNECT = "DISCONNECT";
const string WPEFramework::Plugin::Bluetooth::ENABLE_BLUETOOTH_ENABLED = "BLUETOOTH_ENABLED";
const string WPEFramework::Plugin::Bluetooth::ENABLE_BLUETOOTH_DISABLED = "BLUETOOTH_DISABLED";
const string WPEFramework::Plugin::Bluetooth::ENABLE_BLUETOOTH_INPUT_ENABLED = "BLUETOOTH_INPUT_ENABLED";

const string WPEFramework::Plugin::Bluetooth::ENABLE_PRIMARY_AUDIO = "PRIMARY";
const string WPEFramework::Plugin::Bluetooth::ENABLE_AUXILIARY_AUDIO = "AUXILIARY";
const string WPEFramework::Plugin::Bluetooth::STATUS_HARDWARE_AVAILABLE = "HARDWARE_AVAILABLE";
const string WPEFramework::Plugin::Bluetooth::STATUS_HARDWARE_DISABLED = "HARDWARE_DISABLED";
const string WPEFramework::Plugin::Bluetooth::STATUS_SOFTWARE_ENABLED = "SOFTWARE_ENABLED";
const string WPEFramework::Plugin::Bluetooth::STATUS_SOFTWARE_INPUT_ENABLED = "SOFTWARE_INPUT_ENABLED";
const string WPEFramework::Plugin::Bluetooth::STATUS_PAIRING_CHANGE = "PAIRING_CHANGE";
const string WPEFramework::Plugin::Bluetooth::STATUS_CONNECTION_CHANGE = "CONNECTION_CHANGE";
const string WPEFramework::Plugin::Bluetooth::STATUS_DISCOVERY_STARTED = "DISCOVERY_STARTED";
const string WPEFramework::Plugin::Bluetooth::STATUS_DISCOVERY_COMPLETED = "DISCOVERY_COMPLETED";
const string WPEFramework::Plugin::Bluetooth::STATUS_PAIRING_FAILED = "PAIRING_FAILED";
const string WPEFramework::Plugin::Bluetooth::STATUS_CONNECTION_FAILED= "CONNECTION_FAILED";

const string WPEFramework::Plugin::Bluetooth::CMD_AUDIO_CTRL_PLAY = "PLAY";
const string WPEFramework::Plugin::Bluetooth::CMD_AUDIO_CTRL_STOP = "STOP";
const string WPEFramework::Plugin::Bluetooth::CMD_AUDIO_CTRL_PAUSE = "PAUSE";
const string WPEFramework::Plugin::Bluetooth::CMD_AUDIO_CTRL_RESUME = "RESUME";
const string WPEFramework::Plugin::Bluetooth::CMD_AUDIO_CTRL_SKIP_NEXT = "SKIP_NEXT";
const string WPEFramework::Plugin::Bluetooth::CMD_AUDIO_CTRL_SKIP_PREV = "SKIP_PREV";
const string WPEFramework::Plugin::Bluetooth::CMD_AUDIO_CTRL_RESTART = "RESTART";
const string WPEFramework::Plugin::Bluetooth::CMD_AUDIO_CTRL_VOLUME_UP = "VOLUME_UP";
const string WPEFramework::Plugin::Bluetooth::CMD_AUDIO_CTRL_VOLUME_DOWN = "VOLUME_DOWN";
const string WPEFramework::Plugin::Bluetooth::CMD_AUDIO_CTRL_MUTE = "AUDIO_MUTE";
const string WPEFramework::Plugin::Bluetooth::CMD_AUDIO_CTRL_UNMUTE = "AUDIO_UNMUTE";
const string WPEFramework::Plugin::Bluetooth::CMD_AUDIO_CTRL_UNKNOWN = "CMD_UNKNOWN";

namespace WPEFramework
{
    namespace {

        static Plugin::Metadata<Plugin::Bluetooth> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }
    
    namespace Plugin
    {
        SERVICE_REGISTRATION(Bluetooth, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        Bluetooth* Bluetooth::_instance = nullptr;
        static Core::TimerType<DiscoveryTimer> _discoveryTimer(64 * 1024, "DiscoveryTimer");

        BTRMGR_Result_t bluetoothSrv_EventCallback (BTRMGR_EventMessage_t eventMsg)
        {
            if (!Bluetooth::_instance) {
                LOGERR ("Invalid pointer. Bluetooth is not initialized (yet?). Event of type %d ignored.", eventMsg.m_eventType);
                return BTRMGR_RESULT_INIT_FAILED;
            } else {
                Bluetooth::_instance->notifyEventWrapper(eventMsg);
                return BTRMGR_RESULT_SUCCESS;
            }
        }

        Bluetooth::Bluetooth()
        : PluginHost::JSONRPC()
        , m_apiVersionNumber(API_VERSION_NUMBER_MAJOR)
        , m_discoveryRunning(false)
        , m_discoveryTimer(this)
        {
            Bluetooth::_instance = this;
            Register(METHOD_GET_API_VERSION_NUMBER, &Bluetooth::getApiVersionNumber, this);
            Register(METHOD_START_SCAN, &Bluetooth::startScanWrapper, this);
            Register(METHOD_STOP_SCAN, &Bluetooth::stopScanWrapper, this);
            Register(METHOD_IS_DISCOVERABLE, &Bluetooth::isDiscoverableWrapper, this);
            Register(METHOD_GET_DISCOVERED_DEVICES, &Bluetooth::getDiscoveredDevicesWrapper, this);
            Register(METHOD_GET_PAIRED_DEVICES, &Bluetooth::getPairedDevicesWrapper, this);
            Register(METHOD_GET_CONNECTED_DEVICES, &Bluetooth::getConnectedDevicesWrapper, this);
            Register(METHOD_CONNECT, &Bluetooth::connectWrapper, this);
            Register(METHOD_DISCONNECT, &Bluetooth::disconnectWrapper, this);
            Register(METHOD_SET_AUDIO_STREAM, &Bluetooth::setAudioStreamWrapper, this);
            Register(METHOD_PAIR, &Bluetooth::pairWrapper, this);
            Register(METHOD_UNPAIR, &Bluetooth::unpairWrapper, this);
            Register(METHOD_ENABLE, &Bluetooth::enableWrapper, this);
            Register(METHOD_DISABLE, &Bluetooth::disableWrapper, this);
            Register(METHOD_SET_DISCOVERABLE, &Bluetooth::setDiscoverableWrapper, this);
            Register(METHOD_GET_NAME, &Bluetooth::getNameWrapper, this);
            Register(METHOD_SET_NAME, &Bluetooth::setNameWrapper, this);
            Register(METHOD_SET_AUDIO_PLAYBACK_COMMAND, &Bluetooth::sendAudioPlaybackCommandWrapper, this);
            Register(METHOD_SET_EVENT_RESPONSE, &Bluetooth::setEventResponseWrapper, this);
            Register(METHOD_GET_DEVICE_INFO, &Bluetooth::getDeviceInfoWrapper, this);
            Register(METHOD_GET_AUDIO_INFO, &Bluetooth::getMediaTrackInfoWrapper, this);
            Register(METHOD_GET_DEVICE_VOLUME_MUTE_INFO, &Bluetooth::getDeviceVolumeMuteInfoWrapper, this);
            Register(METHOD_SET_DEVICE_VOLUME_MUTE_INFO, &Bluetooth::setDeviceVolumeMuteInfoWrapper, this);

            Utils::IARM::init();

            BTRMGR_Result_t rc = BTRMGR_RegisterForCallbacks(Utils::IARM::NAME);
            if (BTRMGR_RESULT_SUCCESS != rc)
            {
                LOGWARN("Failed to Register BTRMgr...!");
            }
            else {
                BTRMGR_RegisterEventCallback(bluetoothSrv_EventCallback);
            }
        }

        Bluetooth::~Bluetooth()
        {
        }

        void Bluetooth::Deinitialize(PluginHost::IShell* /* service */)
        {
            Bluetooth::_instance = nullptr;

            BTRMGR_Result_t rc = BTRMGR_UnRegisterFromCallbacks(Utils::IARM::NAME);
            if (BTRMGR_RESULT_SUCCESS != rc)
            {
                LOGWARN("Failed to UnRegister BTRMgr...!");
            }
        }

        string Bluetooth::Information() const
        {
            return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
        }

        /// Internal methods begin
        //

        // This method is not exposed due to the missing match in Metrological API
        void Bluetooth::getStatusSupport(string& status)
        {
            unsigned char numOfAdapters = 0;
            BTRMGR_Result_t rc = BTRMGR_GetNumberOfAdapters(&numOfAdapters);
            status = STATUS_NO_BLUETOOTH_HARDWARE; //TODO: shall we introduce a more specific status? STATUS_BLUETOOTH_UNKNOWN?

            if (BTRMGR_RESULT_SUCCESS != rc)
                LOGERR("Failed to get the number of adapters..!");

            if (numOfAdapters) {
                unsigned char power_status = 0;
                rc = BTRMGR_GetAdapterPowerStatus(0, &power_status);
                if (BTRMGR_RESULT_SUCCESS != rc) {
                    LOGERR("Failed to get the power status of adapter..!");
                    status = STATUS_SOFTWARE_DISABLED;
                }

                if (power_status)
                    status = STATUS_AVAILABLE;
                else
                    status = STATUS_SOFTWARE_DISABLED;
            } else
                status = STATUS_NO_BLUETOOTH_HARDWARE;
            LOGINFO("getStatusSupport: returning %s", C_STR(status));
        }

        bool Bluetooth::isAdapterDiscoverable()
        {
            unsigned char numOfAdapters = 0;
            bool result = false;
            BTRMGR_Result_t rc = BTRMGR_GetNumberOfAdapters(&numOfAdapters);
            if (BTRMGR_RESULT_SUCCESS != rc)
                LOGERR("Failed to get the number of adapters..!");
            if (numOfAdapters) {
                unsigned char adapter_discoverable = 0;
                rc = BTRMGR_IsAdapterDiscoverable(0, &adapter_discoverable);
                if (BTRMGR_RESULT_SUCCESS != rc) {
                    LOGERR("Failed to get the discoverable status of adapter..!");
                    adapter_discoverable = 0;
                }
                if(adapter_discoverable) {
                    result = true;
                } else {
                    result = false;
                }
            }
            return result;
        }

        string Bluetooth::startDeviceDiscovery(int timeout, const string &discProfile)
        {
            BTRMGR_Result_t rc = BTRMGR_RESULT_SUCCESS;
            unsigned char numOfAdapters = 0;

            if (!m_discoveryRunning)
            {
                rc = BTRMGR_GetNumberOfAdapters(&numOfAdapters);
                if (BTRMGR_RESULT_SUCCESS != rc)
                    LOGERR("Failed to get the number of adapters..!");

                if (numOfAdapters) {
                    BTRMGR_DeviceOperationType_t lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_AUDIO_OUTPUT;

                    if (Utils::String::contains(discProfile, "LOUDSPEAKER") ||
                        Utils::String::contains(discProfile, "HEADPHONES") ||
                        Utils::String::contains(discProfile, "WEARABLE HEADSET") ||
                        Utils::String::contains(discProfile, "HIFI AUDIO DEVICE")) {
                        lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_AUDIO_OUTPUT;
                    }
                    else if (Utils::String::contains(discProfile, "SMARTPHONE") ||
                             Utils::String::contains(discProfile, "TABLET")) {
                        lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_AUDIO_INPUT;
                    }
                    else if (Utils::String::contains(discProfile, "KEYBOARD") ||
                             Utils::String::contains(discProfile, "MOUSE") ||
                             Utils::String::contains(discProfile, "JOYSTICK")) {
                        lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_HID;
                    }
                    else if (Utils::String::contains(discProfile, "LE TILE") ||
                             Utils::String::contains(discProfile, "LE")) {
                        lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_LE;
                    }
                    else if (Utils::String::contains(discProfile, "DEFAULT")) {
                        lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_UNKNOWN;
                    }

                    rc = BTRMGR_StartDeviceDiscovery(0, lenDevOpDiscType);
                    if (BTRMGR_RESULT_SUCCESS != rc)
                    {
                        LOGERR("Failed to start the discovery..!");
                    } else {
                        LOGWARN("Started discovery..!");
                    }

                    /* Set the discovery flag */
                    m_discoveryRunning = true;
                    if (timeout <= 0)
                    {
                        stopDeviceDiscovery();
                    }
                    else
                    {
                        startDiscoveryTimer(timeout * 1000);
                    }
                }
                else
                    return STATUS_NO_BLUETOOTH_HARDWARE;
            }
            else
                LOGWARN ("Discovery is in progress..!");

            return STATUS_AVAILABLE;
        }

        bool Bluetooth::stopDeviceDiscovery()
        {
            BTRMGR_Result_t rc = BTRMGR_RESULT_GENERIC_FAILURE;

            if (m_discoveryRunning)
            {
                stopDiscoveryTimer();

                rc = BTRMGR_StopDeviceDiscovery(0, BTRMGR_DEVICE_OP_TYPE_AUDIO_OUTPUT);
                if (BTRMGR_RESULT_SUCCESS != rc)
                {
                    LOGERR("Failed to stop the discovery..!");
                } else {
                    LOGWARN ("Stopped discovery..!");
                }

                m_discoveryRunning = false;
            }

            return BTRMGR_RESULT_SUCCESS == rc;
        }

        void Bluetooth::startDiscoveryTimer(int msec)
        {
            stopDiscoveryTimer();
            _discoveryTimer.Schedule(Core::Time::Now().Add(msec), m_discoveryTimer);
        }

        void Bluetooth::stopDiscoveryTimer()
        {
            _discoveryTimer.Revoke(m_discoveryTimer);
        }

        void Bluetooth::onDiscoveryTimer()
        {
            stopDeviceDiscovery();
        }

        JsonArray Bluetooth::getDiscoveredDevices()
        {
            JsonArray deviceArray;
            BTRMGR_DiscoveredDevicesList_t discoveredDevices;

            memset (&discoveredDevices, 0, sizeof(discoveredDevices));
            BTRMGR_Result_t rc = BTRMGR_GetDiscoveredDevices(0, &discoveredDevices);
            if (BTRMGR_RESULT_SUCCESS != rc)
            {
                LOGERR("Failed to get the discovered devices");
            }
            else
            {
                int i = 0;
                JsonObject deviceDetails;
                LOGINFO ("Success....   Discovered %d Devices", discoveredDevices.m_numOfDevices);
                for (; i < discoveredDevices.m_numOfDevices; i++)
                {
                    deviceDetails["deviceID"] = std::to_string(discoveredDevices.m_deviceProperty[i].m_deviceHandle);
                    deviceDetails["name"] = string(discoveredDevices.m_deviceProperty[i].m_name);
                    deviceDetails["deviceType"] = string(BTRMGR_GetDeviceTypeAsString(discoveredDevices.m_deviceProperty[i].m_deviceType));
                    deviceDetails["connected"] = discoveredDevices.m_deviceProperty[i].m_isConnected?true:false;
                    deviceDetails["paired"] = discoveredDevices.m_deviceProperty[i].m_isPairedDevice?true:false;
                    deviceArray.Add(deviceDetails);
                }
            }
            return deviceArray;
        }

        JsonArray Bluetooth::getPairedDevices()
        {
            JsonArray deviceArray;
            BTRMGR_PairedDevicesList_t pairedDevices;

            memset (&pairedDevices, 0, sizeof(pairedDevices));
            BTRMGR_Result_t rc = BTRMGR_GetPairedDevices(0, &pairedDevices);
            if (BTRMGR_RESULT_SUCCESS != rc)
            {
                LOGERR("Failed to get the paired devices");
            }
            else
            {
                int i = 0;
                JsonObject deviceDetails;
                LOGINFO ("Success....   Paired %d Devices", pairedDevices.m_numOfDevices);
                for (; i < pairedDevices.m_numOfDevices; i++)
                {
                    deviceDetails["deviceID"] = std::to_string(pairedDevices.m_deviceProperty[i].m_deviceHandle);
                    deviceDetails["name"] = string(pairedDevices.m_deviceProperty[i].m_name);
                    deviceDetails["deviceType"] = string(BTRMGR_GetDeviceTypeAsString(pairedDevices.m_deviceProperty[i].m_deviceType));
                    deviceDetails["connected"] = pairedDevices.m_deviceProperty[i].m_isConnected?true:false;
                    deviceArray.Add(deviceDetails);
                }
            }
            return deviceArray;
        }

        JsonArray Bluetooth::getConnectedDevices()
        {
            JsonArray deviceArray;
            BTRMGR_ConnectedDevicesList_t connectedDevices;

            memset (&connectedDevices, 0, sizeof(connectedDevices));
            BTRMGR_Result_t rc = BTRMGR_GetConnectedDevices(0, &connectedDevices);
            if (BTRMGR_RESULT_SUCCESS != rc)
            {
                LOGERR("Failed to get the connected devices");
            }
            else
            {
                int i = 0;
                JsonObject deviceDetails;
                LOGINFO ("Success....   Connected %d Devices", connectedDevices.m_numOfDevices);
                for (; i < connectedDevices.m_numOfDevices; i++)
                {
                    deviceDetails["deviceID"] = std::to_string(connectedDevices.m_deviceProperty[i].m_deviceHandle);
                    deviceDetails["name"] = string(connectedDevices.m_deviceProperty[i].m_name);
                    deviceDetails["deviceType"] = string(BTRMGR_GetDeviceTypeAsString(connectedDevices.m_deviceProperty[i].m_deviceType));
                    deviceDetails["activeState"] = std::to_string(connectedDevices.m_deviceProperty[i].m_powerStatus);
                    deviceArray.Add(deviceDetails);
                }
            }
            return deviceArray;
        }

        bool Bluetooth::setDeviceConnection(long long int deviceID, const string &enable, const string &deviceType)
        {
            BTRMGR_Result_t rc = BTRMGR_RESULT_SUCCESS;
            BTRMgrDeviceHandle deviceHandle = (BTRMgrDeviceHandle) deviceID;

            if (Utils::String::equal(deviceType, "LE TILE")) {
                if (Utils::String::equal(enable, "DISCONNECT")) {
                    rc = BTRMGR_DisconnectFromDevice(0, deviceHandle);
                }
                else if (Utils::String::equal(enable, "CONNECT")) {
                    BTRMGR_DeviceOperationType_t stream_pref = BTRMGR_DEVICE_OP_TYPE_LE;
                    rc = BTRMGR_ConnectToDevice(0, deviceHandle, stream_pref);
                }
            }
            else if (Utils::String::equal(deviceType, "HUMAN INTERFACE DEVICE")) {
                if (Utils::String::equal(enable, "DISCONNECT")) {
                    rc = BTRMGR_DisconnectFromDevice(0, deviceHandle);
                }
                else if (Utils::String::equal(enable, "CONNECT")) {
                    BTRMGR_DeviceOperationType_t stream_pref = BTRMGR_DEVICE_OP_TYPE_HID;
                    rc = BTRMGR_ConnectToDevice(0, deviceHandle, stream_pref);
                }
            }
            else if ((Utils::String::equal(deviceType, "SMARTPHONE")) || (Utils::String::equal(deviceType, "TABLET"))) {
                if (Utils::String::equal(enable, "DISCONNECT")) {
                    rc = BTRMGR_StopAudioStreamingIn(0, deviceHandle);
                }
                else if (Utils::String::equal(enable, "CONNECT")) {
                    BTRMGR_DeviceOperationType_t stream_pref = BTRMGR_DEVICE_OP_TYPE_AUDIO_INPUT;
                    rc = BTRMGR_StartAudioStreamingIn(0, deviceHandle, stream_pref);
                }
            }
            else {
                if (Utils::String::equal(enable, "DISCONNECT")) {
                    rc = BTRMGR_StopAudioStreamingOut(0, deviceHandle);
                }
                else if (Utils::String::equal(enable, "CONNECT")) {
                    BTRMGR_DeviceOperationType_t stream_pref = BTRMGR_DEVICE_OP_TYPE_AUDIO_OUTPUT;
                    rc = BTRMGR_StartAudioStreamingOut(0, deviceHandle, stream_pref);
                }
            }

            if (BTRMGR_RESULT_SUCCESS != rc)
                LOGERR("Failed to do setDeviceConnection");

            return BTRMGR_RESULT_SUCCESS == rc;
        }

        bool Bluetooth::setAudioStream(long long int deviceID, const string &audioStreamName)
        {
            BTRMGR_Result_t rc = BTRMGR_RESULT_SUCCESS;
            BTRMGR_StreamOut_Type_t streamOutPref = BTRMGR_STREAM_PRIMARY;
            if (Utils::String::equal(audioStreamName, "PRIMARY"))
            {
                streamOutPref = BTRMGR_STREAM_PRIMARY;
            } else if (Utils::String::equal(audioStreamName, "AUXILIARY")) {
                streamOutPref = BTRMGR_STREAM_AUXILIARY;
            }
            rc = BTRMGR_SetAudioStreamingOutType(0, streamOutPref);
            if (BTRMGR_RESULT_SUCCESS != rc)
            {
                LOGERR("Failed to do setAudioStream");
            }
            return BTRMGR_RESULT_SUCCESS == rc;
        }

        bool Bluetooth::setDevicePairing(long long int deviceID, bool pair)
        {
            BTRMGR_Result_t rc = BTRMGR_RESULT_SUCCESS;
            BTRMgrDeviceHandle deviceHandle = (BTRMgrDeviceHandle) deviceID;
            if (pair)
            {
                rc = BTRMGR_PairDevice(0, deviceHandle);
            } else{
                rc = BTRMGR_UnpairDevice(0, deviceHandle);
            }

            if (BTRMGR_RESULT_SUCCESS != rc)
            {
                LOGERR("Failed to do %s ", (pair ? "Pair" : "Unpair"));
            } else {
                LOGINFO("Successfully done %s ", (pair ? "Pair" : "Unpair"));
            }
            return BTRMGR_RESULT_SUCCESS == rc;
        }

        bool Bluetooth::setBluetoothEnabled(const string &enabled)
        {
            BTRMGR_Result_t rc = BTRMGR_RESULT_GENERIC_FAILURE;
            if (enabled == "BLUETOOTH_DISABLED")
            {
                rc = BTRMGR_SetAdapterPowerStatus (0, 0 /* FALSE */);
            }
            else if (enabled == "BLUETOOTH_ENABLED")
            {
                rc = BTRMGR_SetAdapterPowerStatus (0, 1 /* TRUE */);
            }
            else if (enabled == "BLUETOOTH_INPUT_ENABLED")
            {
                /* TODO: as the Audio IN is not supported yet */
                LOGERR("Bluetooth IN is not supported by STB");
            }

            if (BTRMGR_RESULT_SUCCESS != rc)
            {
                LOGERR("Failed to do setBluetoothEnabled");
            }

            return BTRMGR_RESULT_SUCCESS == rc;
        }

        bool Bluetooth::setBluetoothDiscoverable(bool enabled, int timeout)
        {
            BTRMGR_Result_t rc = BTRMGR_RESULT_GENERIC_FAILURE;
            if (enabled)
            {
                rc = BTRMGR_SetAdapterDiscoverable(0, 1, timeout);
            }
            else
            {
                rc = BTRMGR_SetAdapterDiscoverable(0, 0, timeout);
            }

            if (BTRMGR_RESULT_SUCCESS != rc)
            {
                LOGERR("Failed to do setBluetoothDiscoverable");
            }

            return BTRMGR_RESULT_SUCCESS == rc;
        }

        // Sets adapter name. No support for "power" yet
        bool Bluetooth::setBluetoothProperties(const JsonObject& parameters)
        {
            BTRMGR_Result_t rc = BTRMGR_RESULT_SUCCESS;
            if (parameters.HasLabel("name")) {
                string name;
                getStringParameter("name", name);
                LOGWARN ("Name received as %s", C_STR(name));
                rc = BTRMGR_SetAdapterName (0, C_STR(name));
                if (BTRMGR_RESULT_SUCCESS != rc)
                {
                    LOGERR("Failed to set Name in setBluetoothProperties");
                }
                else {
                    LOGINFO ("Successfully done setBluetoothProperties");
                }
            }
            return BTRMGR_RESULT_SUCCESS == rc;
        }

        // Gets adapter name. No support for "power" yet
        bool Bluetooth::getBluetoothProperties( JsonObject* rp)
        {
            BTRMGR_Result_t rc = BTRMGR_RESULT_SUCCESS;
            JsonObject response; // responding with a single object

            char adapterName[BTRMGR_NAME_LEN_MAX];
            rc = BTRMGR_GetAdapterName (0, &adapterName[0]);
            if (BTRMGR_RESULT_SUCCESS != rc)
            {
                LOGERR("Failed to get Name in getBluetoothProperties");
            }
            else {
                LOGINFO ("Successfully done getBluetoothProperties");
            }

            response["name"] = string(adapterName);
            LOGWARN ("Name set as %s", adapterName);
            if (rp) {
                *rp = response;
            }
            return BTRMGR_RESULT_SUCCESS == rc;
        }

        bool Bluetooth::setAudioControlCommand(long long int  deviceID, const string &audioCtrlCmd)
        {
            BTRMGR_Result_t rc = BTRMGR_RESULT_SUCCESS;
            BTRMgrDeviceHandle deviceHandle = (BTRMgrDeviceHandle) deviceID;

            if (audioCtrlCmd == CMD_AUDIO_CTRL_PLAY) {
                BTRMGR_DeviceOperationType_t stream_pref = BTRMGR_DEVICE_OP_TYPE_AUDIO_INPUT;
                rc = BTRMGR_StartAudioStreamingIn(0, deviceHandle, stream_pref);
            }
            else if (audioCtrlCmd == CMD_AUDIO_CTRL_PAUSE) {
                rc = BTRMGR_MediaControl (0, deviceHandle, BTRMGR_MEDIA_CTRL_PAUSE);
            }
            else if (audioCtrlCmd == CMD_AUDIO_CTRL_RESUME) {
                rc = BTRMGR_MediaControl (0, deviceHandle, BTRMGR_MEDIA_CTRL_PLAY);
            }
            else if (audioCtrlCmd == CMD_AUDIO_CTRL_STOP) {
                rc = BTRMGR_MediaControl (0, deviceHandle, BTRMGR_MEDIA_CTRL_STOP);
            }
            else if (audioCtrlCmd == CMD_AUDIO_CTRL_SKIP_NEXT) {
                rc = BTRMGR_MediaControl (0, deviceHandle, BTRMGR_MEDIA_CTRL_NEXT);
            }
            else if (audioCtrlCmd == CMD_AUDIO_CTRL_SKIP_PREV) {
                rc = BTRMGR_MediaControl (0, deviceHandle, BTRMGR_MEDIA_CTRL_PREVIOUS);
            } //TODO
            else if (audioCtrlCmd == CMD_AUDIO_CTRL_RESTART) {
                rc = BTRMGR_RESULT_GENERIC_FAILURE;
                /* could manipulate this action with skip track by setting Repeat - To confirm */
            } //TODO
            else if (audioCtrlCmd == CMD_AUDIO_CTRL_MUTE) {
                    LOGERR(" mute set calling ");
                    rc = BTRMGR_MediaControl (0, deviceHandle, BTRMGR_MEDIA_CTRL_MUTE);
            }
            else if (audioCtrlCmd == CMD_AUDIO_CTRL_UNMUTE) {
                     LOGERR(" un mute set calling ");
                     rc = BTRMGR_MediaControl (0, deviceHandle, BTRMGR_MEDIA_CTRL_UNMUTE);
            }
            else if (audioCtrlCmd == CMD_AUDIO_CTRL_VOLUME_UP) {
                    rc = BTRMGR_MediaControl (0, deviceHandle, BTRMGR_MEDIA_CTRL_VOLUMEUP);
            }
            else if (audioCtrlCmd == CMD_AUDIO_CTRL_VOLUME_DOWN) {
                    rc = BTRMGR_MediaControl (0, deviceHandle, BTRMGR_MEDIA_CTRL_VOLUMEDOWN);
            }

            if (rc != BTRMGR_RESULT_SUCCESS)
            {
                LOGERR("Failed to do setAudioControlCommand");
            } else {
                LOGINFO ("Successfully done setAudioControlCommand");
            }

            return BTRMGR_RESULT_SUCCESS == rc;
        }

        BTRMGR_DeviceOperationType_t Bluetooth::btmgrDeviceOperationTypeFromString(const string &deviceProfile)
        {
             BTRMGR_DeviceOperationType_t lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_AUDIO_OUTPUT;

             if (Utils::String::contains(deviceProfile, "LOUDSPEAKER") ||
                 Utils::String::contains(deviceProfile, "HEADPHONES") ||
                 Utils::String::contains(deviceProfile, "WEARABLE HEADSET") ||
                 Utils::String::contains(deviceProfile, "HIFI AUDIO DEVICE")) {
                 lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_AUDIO_OUTPUT;
             }
             else if (Utils::String::contains(deviceProfile, "SMARTPHONE") ||
                      Utils::String::contains(deviceProfile, "TABLET")) {
                      lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_AUDIO_INPUT;
             }
             else if (Utils::String::contains(deviceProfile, "KEYBOARD") ||
                      Utils::String::contains(deviceProfile, "MOUSE") ||
                      Utils::String::contains(deviceProfile, "JOYSTICK")) {
                      lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_HID;
             }
             else if (Utils::String::contains(deviceProfile, "LE TILE") ||
                      Utils::String::contains(deviceProfile, "LE")) {
                      lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_LE;
             }
             else if (Utils::String::contains(deviceProfile, "DEFAULT")) {
                      lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_UNKNOWN;
             }

             return lenDevOpDiscType;
        }

        bool Bluetooth::setDeviceVolumeMuteProperties(long long int  deviceID, const string &deviceProfile, unsigned char ui8volume, unsigned char mute)
        {
             BTRMGR_Result_t rc = BTRMGR_RESULT_SUCCESS;
             BTRMgrDeviceHandle deviceHandle = (BTRMgrDeviceHandle) deviceID;
             BTRMGR_DeviceOperationType_t lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_AUDIO_OUTPUT;

             lenDevOpDiscType = btmgrDeviceOperationTypeFromString(deviceProfile);
             rc = BTRMGR_SetDeviceVolumeMute (0, deviceHandle, lenDevOpDiscType, ui8volume, mute);
             return BTRMGR_RESULT_SUCCESS == rc;
        }

        JsonObject Bluetooth::getDeviceVolumeMuteProperties(long long int  deviceID, const string &deviceProfile)
        {
             BTRMGR_Result_t rc = BTRMGR_RESULT_SUCCESS;
             BTRMgrDeviceHandle deviceHandle = (BTRMgrDeviceHandle) deviceID;
             BTRMGR_DeviceOperationType_t lenDevOpDiscType = BTRMGR_DEVICE_OP_TYPE_AUDIO_OUTPUT;
             unsigned char ui8volume;
             unsigned char mute;
             JsonObject volumeInfo;

             lenDevOpDiscType = btmgrDeviceOperationTypeFromString(deviceProfile);
             rc = BTRMGR_GetDeviceVolumeMute (0, deviceHandle, lenDevOpDiscType, &ui8volume, &mute);
             if (BTRMGR_RESULT_SUCCESS != rc) {
                 LOGERR("Failed to get the volume info %d", rc);
             } else {
	         volumeInfo ["volume"] = std::to_string(ui8volume);
	         volumeInfo ["mute"]   = mute ? true : false ;
	     }

             return volumeInfo;
        }

        bool Bluetooth::setEventResponse(long long int  deviceID, const string &eventType, const string &respValue)
        {
            BTRMGR_Result_t         rc = BTRMGR_RESULT_SUCCESS;
            BTRMGR_EventResponse_t  lstBtrMgrEvtRsp;

            memset(&lstBtrMgrEvtRsp, 0, sizeof(lstBtrMgrEvtRsp));

            lstBtrMgrEvtRsp.m_deviceHandle = deviceID;

            if (eventType.compare(EVT_PAIRING_REQUEST)) {
                lstBtrMgrEvtRsp.m_eventType = BTRMGR_EVENT_RECEIVED_EXTERNAL_PAIR_REQUEST;
                lstBtrMgrEvtRsp.m_eventResp = Utils::String::equal(respValue, "ACCEPTED") ? 1 : 0;
            }
            else if (eventType.compare(EVT_CONNECTION_REQUEST)) {
                lstBtrMgrEvtRsp.m_eventType = BTRMGR_EVENT_RECEIVED_EXTERNAL_CONNECT_REQUEST;
                lstBtrMgrEvtRsp.m_eventResp = Utils::String::equal(respValue, "ACCEPTED") ? 1 : 0;
            }
            else if (eventType.compare(EVT_PLAYBACK_REQUEST)) {
                lstBtrMgrEvtRsp.m_eventType = BTRMGR_EVENT_RECEIVED_EXTERNAL_PLAYBACK_REQUEST;
                lstBtrMgrEvtRsp.m_eventResp = Utils::String::equal(respValue, "ACCEPTED") ? 1 : 0;
            }
            else {
                lstBtrMgrEvtRsp.m_eventType = BTRMGR_EVENT_MAX;
            }

            rc = BTRMGR_SetEventResponse(0, &lstBtrMgrEvtRsp);
            if (BTRMGR_RESULT_SUCCESS != rc)
            {
                LOGERR("Failed to do setEventResponse");
            } else {
                LOGINFO ("Successfully done setEventResponse");
            }

            return BTRMGR_RESULT_SUCCESS == rc;
        }

        JsonObject Bluetooth::getDeviceInfo(long long int deviceID)
        {
            JsonObject deviceDetails;
            string profileInfo;

            BTRMGR_Result_t rc = BTRMGR_RESULT_SUCCESS;
            BTRMgrDeviceHandle deviceHandle = (BTRMgrDeviceHandle) deviceID;
            string serviceInfo = "";
            BTRMGR_DevicesProperty_t deviceProperty;
            memset (&deviceProperty, 0, sizeof(deviceProperty));

            rc = BTRMGR_GetDeviceProperties(0, deviceHandle, &deviceProperty);
            if (BTRMGR_RESULT_SUCCESS != rc)
            {
                LOGERR("Failed to get device details");
            } else {
                deviceDetails["deviceID"] = std::to_string(deviceProperty.m_deviceHandle);
                deviceDetails["name"] =  string(deviceProperty.m_name);
                deviceDetails["deviceType"] = string(BTRMGR_GetDeviceTypeAsString(deviceProperty.m_deviceType));
                deviceDetails["manufacturer"] = std::to_string(deviceProperty.m_vendorID);
                deviceDetails["MAC"] = string(deviceProperty.m_deviceAddress);
                deviceDetails["signalStrength"] = std::to_string(deviceProperty.m_signalLevel);
                deviceDetails["rssi"] =  std::to_string(deviceProperty.m_rssi);
                for (int i = 0; i < deviceProperty.m_serviceInfo.m_numOfService; i++)
                {
                    profileInfo += string(deviceProperty.m_serviceInfo.m_profileInfo[i].m_profile);
                    if ((i + 1) < deviceProperty.m_serviceInfo.m_numOfService)
                    {
                        profileInfo +=string(";");
                    }
                }
                deviceDetails["supportedProfile"] = profileInfo;
            }
            return deviceDetails;
        }

        JsonObject Bluetooth::getMediaTrackInfo(long long int deviceID)
        {
            JsonObject               mediaTrackInfo;
            BTRMGR_Result_t          rc = BTRMGR_RESULT_SUCCESS;
            BTRMgrDeviceHandle       deviceHandle = (BTRMgrDeviceHandle) deviceID;
            BTRMGR_MediaTrackInfo_t  m_mediaTrackInfo;

            memset (&m_mediaTrackInfo, 0, sizeof(m_mediaTrackInfo));

            rc = BTRMGR_GetMediaTrackInfo (0, deviceHandle, &m_mediaTrackInfo);

            if (BTRMGR_RESULT_SUCCESS != rc)
            {
                LOGERR("Failed to get Track details");
            }
            else
            {
                mediaTrackInfo["album"] = string(m_mediaTrackInfo.pcAlbum);
                mediaTrackInfo["genre"] = string(m_mediaTrackInfo.pcGenre);
                mediaTrackInfo["title"] = string(m_mediaTrackInfo.pcTitle);
                mediaTrackInfo["artist"] = string(m_mediaTrackInfo.pcArtist);
                mediaTrackInfo["ui32Duration"] = std::to_string(m_mediaTrackInfo.ui32Duration);
                mediaTrackInfo["ui32TrackNumber"] = std::to_string(m_mediaTrackInfo.ui32TrackNumber);
                mediaTrackInfo["ui32NumberOfTracks"] = std::to_string(m_mediaTrackInfo.ui32NumberOfTracks);
            }
            return mediaTrackInfo;
        }

        void Bluetooth::notifyEventWrapper (BTRMGR_EventMessage_t eventMsg)
        {
            JsonObject params;
            string profileInfo;
            string eventId;
            LOGINFO ("Event notification: event of type %d received", eventMsg.m_eventType);
            switch (eventMsg.m_eventType) {
                case BTRMGR_EVENT_DEVICE_DISCOVERY_COMPLETE:
                    LOGINFO ("Received %s Event from BTRMgr", C_STR(STATUS_DISCOVERY_COMPLETED));
                    params["newStatus"] = STATUS_DISCOVERY_COMPLETED;
                    eventId = EVT_STATUS_CHANGED;

                    // TODO: Stopping the discovery timer and resetting the flag should not be needed on Discovery completed.
                    //       But is it logical to expect DISCOVERY_COMPLETED, when Bluetooth Service has not asked BTRMgr to
                    //       to Stop discovery. Should we change BTRMgr to send an alternate event to indicate DISCOVERY_PAUSED
                    //       and DISCOVERY_RESUMED.
                    //       Would it be sufficient to send the Discovery Type as part of DISCOVERY_STARTED and DISCOVERY_COMPLETE
                    //       events from BTRMgr ??
                    break;

                case BTRMGR_EVENT_DEVICE_PAIRING_COMPLETE:
                    LOGINFO ("Received %s Event from BTRMgr", C_STR(STATUS_PAIRING_CHANGE));
                    params["newStatus"] = STATUS_PAIRING_CHANGE;
                    params["deviceID"] = C_STR(std::to_string(eventMsg.m_discoveredDevice.m_deviceHandle));
                    params["name"] = string(eventMsg.m_discoveredDevice.m_name);
                    params["deviceType"] = BTRMGR_GetDeviceTypeAsString(eventMsg.m_discoveredDevice.m_deviceType);
                    params["rawDeviceType"] = C_STR(std::to_string(eventMsg.m_discoveredDevice.m_ui32DevClassBtSpec));
                    params["lastConnectedState"] = eventMsg.m_discoveredDevice.m_isLastConnectedDevice ? true : false;
                    params["paired"] = eventMsg.m_discoveredDevice.m_isPairedDevice ? true : false;
                    params["connected"] = eventMsg.m_discoveredDevice.m_isConnected ? true : false;

                    eventId = EVT_STATUS_CHANGED;
                    break;

                case BTRMGR_EVENT_DEVICE_UNPAIRING_COMPLETE:
                    LOGINFO ("Received %s Event from BTRMgr", C_STR(STATUS_PAIRING_CHANGE));
                    params["newStatus"] = STATUS_PAIRING_CHANGE;
                    params["deviceID"] = std::to_string(eventMsg.m_pairedDevice.m_deviceHandle);
                    params["name"] = string(eventMsg.m_pairedDevice.m_name);
                    params["deviceType"] = BTRMGR_GetDeviceTypeAsString(eventMsg.m_pairedDevice.m_deviceType);
                    params["rawDeviceType"] = std::to_string(eventMsg.m_pairedDevice.m_ui32DevClassBtSpec);
                    params["lastConnectedState"] = eventMsg.m_pairedDevice.m_isLastConnectedDevice ? true : false;
                    params["paired"] = false;
                    params["connected"] = eventMsg.m_pairedDevice.m_isConnected ? true : false;

                    eventId = EVT_STATUS_CHANGED;
                    break;

                case BTRMGR_EVENT_DEVICE_CONNECTION_COMPLETE:
                case BTRMGR_EVENT_DEVICE_DISCONNECT_COMPLETE: /* Allow only AudioIn/Out & HID Connection Event propogation to XRE for now */
                    if ((eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_WEARABLE_HEADSET)   ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_HANDSFREE)          ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_LOUDSPEAKER)        ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_HEADPHONES)         ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_PORTABLE_AUDIO)     ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_CAR_AUDIO)          ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_HIFI_AUDIO_DEVICE)  ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_SMARTPHONE)         ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_TABLET)             ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_HID)                ){

                        LOGINFO ("Received %s Event from BTRMgr", C_STR(STATUS_CONNECTION_CHANGE));
                        params["newStatus"] = STATUS_CONNECTION_CHANGE;
                        params["deviceID"] = std::to_string(eventMsg.m_pairedDevice.m_deviceHandle);
                        params["name"] = string(eventMsg.m_pairedDevice.m_name);
                        params["deviceType"] = BTRMGR_GetDeviceTypeAsString(eventMsg.m_pairedDevice.m_deviceType);
                        params["rawDeviceType"] = std::to_string(eventMsg.m_pairedDevice.m_ui32DevClassBtSpec);
                        params["lastConnectedState"] = eventMsg.m_pairedDevice.m_isLastConnectedDevice ? true : false;
                        params["paired"] = true;
                        params["connected"] = eventMsg.m_pairedDevice.m_isConnected ? true : false;

                        eventId = EVT_STATUS_CHANGED;
                    }
                    break;

                case BTRMGR_EVENT_DEVICE_DISCOVERY_STARTED:
                    LOGINFO ("Received %s Event from BTRMgr", C_STR(STATUS_DISCOVERY_STARTED));
                    params["newStatus"] = STATUS_DISCOVERY_STARTED;
                    eventId = EVT_STATUS_CHANGED;
                    break;

                case BTRMGR_EVENT_RECEIVED_EXTERNAL_PAIR_REQUEST:
                    LOGINFO ("Received %s Event from BTRMgr", "external pairing request");
                    params["deviceID"] = std::to_string(eventMsg.m_externalDevice.m_deviceHandle);
                    params["name"] = string(eventMsg.m_externalDevice.m_name);
                    params["deviceType"] = BTRMGR_GetDeviceTypeAsString(eventMsg.m_externalDevice.m_deviceType);
                    params["manufacturer"] = std::to_string(eventMsg.m_externalDevice.m_vendorID);
                    params["MAC"] = string(eventMsg.m_externalDevice.m_deviceAddress);

                    for (int i = 0; i < eventMsg.m_externalDevice.m_serviceInfo.m_numOfService; i++) {
                        profileInfo += string(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[i].m_profile);
                        if ((i + 1) < eventMsg.m_externalDevice.m_serviceInfo.m_numOfService)
                            profileInfo += string(";");
                    }

                    params["supportedProfile"] = profileInfo;

                    if (eventMsg.m_externalDevice.m_externalDevicePIN == 0) {
                        params["pinRequired"] = "false";
                    }
                    else {
                        params["pinRequired"] = "true";
                        params["pinValue"] = std::to_string(eventMsg.m_externalDevice.m_externalDevicePIN);
                    }

                    eventId = EVT_PAIRING_REQUEST;
                    break;

                case BTRMGR_EVENT_DEVICE_PAIRING_FAILED:
                    LOGERR("Received %s Event from BTRMgr", C_STR(STATUS_PAIRING_FAILED));
                    params["newStatus"] = STATUS_PAIRING_FAILED;
                    params["deviceID"] = std::to_string(eventMsg.m_discoveredDevice.m_deviceHandle);
                    params["name"] = string(eventMsg.m_discoveredDevice.m_name);
                    params["deviceType"] = BTRMGR_GetDeviceTypeAsString(eventMsg.m_discoveredDevice.m_deviceType);
                    params["rawDeviceType"] = std::to_string(eventMsg.m_discoveredDevice.m_ui32DevClassBtSpec);
                    params["lastConnectedState"] = eventMsg.m_discoveredDevice.m_isLastConnectedDevice ? true : false;
                    params["paired"] = eventMsg.m_discoveredDevice.m_isPairedDevice ? true : false;
                    params["connected"] = eventMsg.m_discoveredDevice.m_isConnected ? true : false;

                    eventId = EVT_REQUEST_FAILED;
                    break;

                case BTRMGR_EVENT_DEVICE_UNPAIRING_FAILED:
                    LOGERR("Received %s Event from BTRMgr", C_STR(STATUS_PAIRING_FAILED));
                    params["newStatus"] = STATUS_PAIRING_FAILED;
                    params["deviceID"] = std::to_string(eventMsg.m_pairedDevice.m_deviceHandle);
                    params["name"] = string(eventMsg.m_pairedDevice.m_name);
                    params["deviceType"] = BTRMGR_GetDeviceTypeAsString(eventMsg.m_pairedDevice.m_deviceType);
                    params["rawDeviceType"] = std::to_string(eventMsg.m_pairedDevice.m_ui32DevClassBtSpec);
                    params["lastConnectedState"] = eventMsg.m_pairedDevice.m_isLastConnectedDevice ? true : false;
                    params["paired"] = true;
                    params["connected"] = eventMsg.m_pairedDevice.m_isConnected ? true : false;

                    eventId = EVT_REQUEST_FAILED;
                    break;

                case BTRMGR_EVENT_DEVICE_CONNECTION_FAILED:
                case BTRMGR_EVENT_DEVICE_DISCONNECT_FAILED: /* Allow only AudioIn/Out & HID Connection Event propogation to XRE for now */
                    if ((eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_WEARABLE_HEADSET)   ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_HANDSFREE)          ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_LOUDSPEAKER)        ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_HEADPHONES)         ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_PORTABLE_AUDIO)     ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_CAR_AUDIO)          ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_HIFI_AUDIO_DEVICE)  ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_SMARTPHONE)         ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_TABLET)             ||
                        (eventMsg.m_pairedDevice.m_deviceType == BTRMGR_DEVICE_TYPE_HID)                ){

                        LOGERR("Received %s Event from BTRMgr", C_STR(STATUS_CONNECTION_FAILED));
                        params["newStatus"] = STATUS_CONNECTION_FAILED;
                        params["deviceID"] = std::to_string(eventMsg.m_pairedDevice.m_deviceHandle);
                        params["name"] = string(eventMsg.m_pairedDevice.m_name);
                        params["deviceType"] = BTRMGR_GetDeviceTypeAsString(eventMsg.m_pairedDevice.m_deviceType);
                        params["rawDeviceType"] = std::to_string(eventMsg.m_pairedDevice.m_ui32DevClassBtSpec);
                        params["lastConnectedState"] = eventMsg.m_pairedDevice.m_isLastConnectedDevice ? true : false;
                        params["paired"] = true;
                        params["connected"] = eventMsg.m_pairedDevice.m_isConnected ? true : false;

                        eventId = EVT_REQUEST_FAILED;
                    }
                    break;

                case BTRMGR_EVENT_RECEIVED_EXTERNAL_CONNECT_REQUEST:
                    LOGERR("Received %s Event from BTRMgr", "external connection request");
                    params["deviceID"] = std::to_string(eventMsg.m_externalDevice.m_deviceHandle);
                    params["name"] = string(eventMsg.m_externalDevice.m_name);
                    params["deviceType"] = BTRMGR_GetDeviceTypeAsString(eventMsg.m_externalDevice.m_deviceType);
                    params["manufacturer"] = std::to_string(eventMsg.m_externalDevice.m_vendorID);
                    params["MAC"] = string(eventMsg.m_externalDevice.m_deviceAddress);

                    for (int i = 0; i < eventMsg.m_externalDevice.m_serviceInfo.m_numOfService; i++) {
                        profileInfo += string(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[i].m_profile);
                        if ((i + 1) < eventMsg.m_externalDevice.m_serviceInfo.m_numOfService)
                            profileInfo += string(";");
                    }

                    params["supportedProfile"] = profileInfo;

                    eventId = EVT_CONNECTION_REQUEST;
                    break;

                case BTRMGR_EVENT_RECEIVED_EXTERNAL_PLAYBACK_REQUEST:
                    LOGERR("Received %s Event from BTRMgr", "external playback request");
                    params["deviceID"] = std::to_string(eventMsg.m_externalDevice.m_deviceHandle);
                    params["name"] = string(eventMsg.m_externalDevice.m_name);
                    params["deviceType"] = BTRMGR_GetDeviceTypeAsString(eventMsg.m_externalDevice.m_deviceType);
                    params["manufacturer"] = std::to_string(eventMsg.m_externalDevice.m_vendorID);
                    params["MAC"] = string(eventMsg.m_externalDevice.m_deviceAddress);

                    for (int i = 0; i < eventMsg.m_externalDevice.m_serviceInfo.m_numOfService; i++) {
                        profileInfo += string(eventMsg.m_externalDevice.m_serviceInfo.m_profileInfo[i].m_profile);
                        if ((i + 1) < eventMsg.m_externalDevice.m_serviceInfo.m_numOfService)
                            profileInfo += string(";");
                    }

                    params["supportedProfile"] = profileInfo;

                    eventId = EVT_PLAYBACK_REQUEST;
                    break;

                case BTRMGR_EVENT_MEDIA_TRACK_STARTED:
                    LOGINFO ("Received %s Event from BTRMgr", C_STR(EVT_PLAYBACK_STARTED));
                    params["action"]   = std::string("started");
                    params["deviceID"] = std::to_string(eventMsg.m_mediaInfo.m_deviceHandle);
                    params["position"] = std::to_string(eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaPosition);
                    params["Duration"] = std::to_string(eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaDuration);

                    eventId = EVT_PLAYBACK_STARTED;
                    break;
                case BTRMGR_EVENT_MEDIA_TRACK_PAUSED:
                    LOGINFO ("Received %s Event from BTRMgr", C_STR(EVT_PLAYBACK_PAUSED));
                    params["action"]   = std::string("paused");
                    params["deviceID"] = std::to_string(eventMsg.m_mediaInfo.m_deviceHandle);
                    params["position"] = std::to_string(eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaPosition);
                    params["Duration"] = std::to_string(eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaDuration);

                    eventId =EVT_PLAYBACK_PAUSED;
                    break;

                case BTRMGR_EVENT_MEDIA_TRACK_STOPPED:
                    LOGINFO ("Received %s Event from BTRMgr", C_STR(EVT_PLAYBACK_STOPPED));
                    params["action"]   = std::string("stopped");
                    params["deviceID"] = C_STR(std::to_string(eventMsg.m_mediaInfo.m_deviceHandle));
                    params["position"] = C_STR(std::to_string(eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaPosition));
                    params["Duration"] = C_STR(std::to_string(eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaDuration));

                    eventId = EVT_PLAYBACK_STOPPED;
                    break;

                case BTRMGR_EVENT_MEDIA_PLAYBACK_ENDED:
                    LOGINFO ("Received %s Event from BTRMgr", C_STR(EVT_PLAYBACK_ENDED));
                    params["action"]   = std::string("ended");
                    params["deviceID"] = std::to_string(eventMsg.m_mediaInfo.m_deviceHandle);

                    eventId = EVT_PLAYBACK_ENDED;
                    break;

                case BTRMGR_EVENT_MEDIA_TRACK_PLAYING:
                case BTRMGR_EVENT_MEDIA_TRACK_POSITION:
                    LOGINFO ("Received Playback Position Event from BTRMgr");
                    params["deviceID"] = std::to_string(eventMsg.m_mediaInfo.m_deviceHandle);
                    params["position"] = std::to_string(eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaPosition);
                    params["Duration"] = std::to_string(eventMsg.m_mediaInfo.m_mediaPositionInfo.m_mediaDuration);

                    eventId = EVT_PLAYBACK_POSITION;
                    break;

                case BTRMGR_EVENT_MEDIA_TRACK_CHANGED:
                    params["deviceID"] = std::to_string(eventMsg.m_mediaInfo.m_deviceHandle);
                    params["album"] = string(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcAlbum);
                    params["genre"] = string(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcGenre);
                    params["title"] = string(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcTitle);
                    params["artist"] = string(eventMsg.m_mediaInfo.m_mediaTrackInfo.pcArtist);
                    params["ui32Duration"] = std::to_string(eventMsg.m_mediaInfo.m_mediaTrackInfo.ui32Duration);
                    params["ui32TrackNumber"] = std::to_string(eventMsg.m_mediaInfo.m_mediaTrackInfo.ui32TrackNumber);
                    params["ui32NumberOfTracks"] = std::to_string(eventMsg.m_mediaInfo.m_mediaTrackInfo.ui32NumberOfTracks);

                    eventId = EVT_PLAYBACK_NEW_TRACK;
                    break;

                case BTRMGR_EVENT_DEVICE_FOUND:
                    params["deviceID"] = std::to_string(eventMsg.m_pairedDevice.m_deviceHandle);
                    params["name"] = string(eventMsg.m_pairedDevice.m_name);
                    params["deviceType"] = BTRMGR_GetDeviceTypeAsString(eventMsg.m_pairedDevice.m_deviceType);
                    params["rawDeviceType"] = std::to_string(eventMsg.m_pairedDevice.m_ui32DevClassBtSpec);
                    params["lastConnectedState"] = eventMsg.m_pairedDevice.m_isLastConnectedDevice?true:false;

                    eventId = EVT_DEVICE_FOUND;
                    break;

                case BTRMGR_EVENT_DEVICE_OUT_OF_RANGE:
                    params["deviceID"] = std::to_string(eventMsg.m_pairedDevice.m_deviceHandle);
                    params["name"] = string(eventMsg.m_pairedDevice.m_name);
                    params["deviceType"] = BTRMGR_GetDeviceTypeAsString(eventMsg.m_pairedDevice.m_deviceType);
                    params["rawDeviceType"] = std::to_string(eventMsg.m_pairedDevice.m_ui32DevClassBtSpec);
                    params["lastConnectedState"] = eventMsg.m_pairedDevice.m_isLastConnectedDevice?true:false;
                    eventId = EVT_DEVICE_LOST_OR_OUT_OF_RANGE;
                    break;

                case BTRMGR_EVENT_DEVICE_DISCOVERY_UPDATE:
                    params["deviceID"] = std::to_string(eventMsg.m_discoveredDevice.m_deviceHandle);
                    params["discoveryType"] = eventMsg.m_discoveredDevice.m_isDiscovered ? "DISCOVERED":"LOST";
                    params["name"] = string(eventMsg.m_discoveredDevice.m_name);
                    params["deviceType"] = BTRMGR_GetDeviceTypeAsString(eventMsg.m_discoveredDevice.m_deviceType);
                    params["rawDeviceType"] = std::to_string(eventMsg.m_discoveredDevice.m_ui32DevClassBtSpec);
                    params["lastConnectedState"] = eventMsg.m_discoveredDevice.m_isLastConnectedDevice? true:false;
                    params["paired"] = eventMsg.m_discoveredDevice.m_isPairedDevice ? true:false;

                    eventId = EVT_DEVICE_DISCOVERY_UPDATE;
                    break;

                case BTRMGR_EVENT_DEVICE_MEDIA_STATUS:
                    LOGINFO ("Received %s Event from BTRMgr", C_STR(EVT_DEVICE_MEDIA_STATUS));
                    params["deviceID"] = std::to_string(eventMsg.m_mediaInfo.m_deviceHandle);
                    params["name"] = string(eventMsg.m_mediaInfo.m_name);
                    params["deviceType"] = BTRMGR_GetDeviceTypeAsString(eventMsg.m_mediaInfo.m_deviceType);
                    params["volume"] = std::to_string(eventMsg.m_mediaInfo.m_mediaDevStatus.m_ui8mediaDevVolume);
                    params["mute"] = eventMsg.m_mediaInfo.m_mediaDevStatus.m_ui8mediaDevMute ? true : false;

                    if (eventMsg.m_mediaInfo.m_mediaDevStatus.m_enmediaCtrlCmd == BTRMGR_MEDIA_CTRL_VOLUMEUP) {
                        params["command"] = string(CMD_AUDIO_CTRL_VOLUME_UP);
                    }
                    else if (eventMsg.m_mediaInfo.m_mediaDevStatus.m_enmediaCtrlCmd == BTRMGR_MEDIA_CTRL_VOLUMEDOWN) {
                        params["command"] = string(CMD_AUDIO_CTRL_VOLUME_DOWN);
                    }
                    else if (eventMsg.m_mediaInfo.m_mediaDevStatus.m_enmediaCtrlCmd == BTRMGR_MEDIA_CTRL_MUTE) {
                        params["command"] = string(CMD_AUDIO_CTRL_MUTE);
                    }
                    else if (eventMsg.m_mediaInfo.m_mediaDevStatus.m_enmediaCtrlCmd == BTRMGR_MEDIA_CTRL_UNMUTE) {
                        params["command"] = string(CMD_AUDIO_CTRL_UNMUTE);
                    }
                    else {
                        params["command"] = string(CMD_AUDIO_CTRL_UNKNOWN);
                    }

                    eventId = EVT_DEVICE_MEDIA_STATUS;
                    break;

                    // TODO: implement or delete these values from enum
                case BTRMGR_EVENT_MAX:
                    break;
                case BTRMGR_EVENT_DEVICE_OP_READY:
                    break;
                case BTRMGR_EVENT_DEVICE_OP_INFORMATION:
                    break;
                case BTRMGR_EVENT_MEDIA_PLAYER_NAME:
                    break;
                case BTRMGR_EVENT_MEDIA_PLAYER_VOLUME:
                    break;
                case BTRMGR_EVENT_MEDIA_PLAYER_EQUALIZER_OFF:
                    break;
                case BTRMGR_EVENT_MEDIA_PLAYER_EQUALIZER_ON:
                    break;
                case BTRMGR_EVENT_MEDIA_PLAYER_SHUFFLE_OFF:
                    break;
                case BTRMGR_EVENT_MEDIA_PLAYER_SHUFFLE_ALLTRACKS:
                    break;
                case BTRMGR_EVENT_MEDIA_PLAYER_SHUFFLE_GROUP:
                    break;
                case BTRMGR_EVENT_MEDIA_PLAYER_REPEAT_OFF:
                    break;
                case BTRMGR_EVENT_MEDIA_PLAYER_REPEAT_SINGLETRACK:
                    break;
                case BTRMGR_EVENT_MEDIA_PLAYER_REPEAT_ALLTRACKS:
                    break;
                case BTRMGR_EVENT_MEDIA_PLAYER_REPEAT_GROUP:
                    break;
                case BTRMGR_EVENT_MEDIA_ALBUM_INFO:
                    break;
                case BTRMGR_EVENT_MEDIA_ARTIST_INFO:
                    break;
                case BTRMGR_EVENT_MEDIA_GENRE_INFO:
                    break;
                case BTRMGR_EVENT_MEDIA_COMPILATION_INFO:
                    break;
                case BTRMGR_EVENT_MEDIA_PLAYLIST_INFO:
                    break;
                case BTRMGR_EVENT_MEDIA_TRACKLIST_INFO:
                    break;
            }

            if (!eventId.empty())
            {
                sendNotify(C_STR(eventId), params);
            }
            return;
        }
        //
        /// Internal methods end

        /// Registered methods begin
        //
        uint32_t Bluetooth::getApiVersionNumber(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            response["version"] = m_apiVersionNumber;
            returnResponse(true);
        }

        uint32_t Bluetooth::startScanWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            int timeout = -1;
            string profile;
            bool timeoutDefined = false;
            bool profileDefined = false;
            bool successFlag;
            if (parameters.HasLabel("timeout"))
            {
                getNumberParameter("timeout", timeout);
                timeoutDefined = true;
            }

            if (parameters.HasLabel("profile"))
            {
                getStringParameter("profile", profile);
                profileDefined = true;
            }
            if (timeoutDefined && profileDefined)
            {
                LOGINFO("Making a call with timeout=%d sec profile=%s", timeout, profile.c_str());
                response["status"] = startDeviceDiscovery(timeout, profile);
                successFlag = true;
            } else if (timeoutDefined) {
                LOGINFO("Making a call with timeout=%d sec", timeout);
                response["status"] = startDeviceDiscovery(timeout);
                successFlag = true;
            } else {
                LOGERR("Please specify parameters. Example: \"params\": {\"timeout\": \"5\", \"profile\": \"SMARTPHONE\"}");
                successFlag = false;
            }
            returnResponse(successFlag);
        }

        uint32_t Bluetooth::stopScanWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            stopDeviceDiscovery();
            returnResponse(true);
        }

        uint32_t Bluetooth::isDiscoverableWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            response["discoverable"] = isAdapterDiscoverable();
            returnResponse(true);
        }

        uint32_t Bluetooth::setDiscoverableWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool successFlag;
            bool discoverable = false;
            int timeout;

            if (parameters.HasLabel("timeout"))
            {
                getNumberParameter("timeout", timeout);
            } else {
                timeout = -1;
            }

            if (parameters.HasLabel("discoverable")) {
                getBoolParameter("discoverable", discoverable);
                LOGINFO("Making a call with discoverable: %s timeout=%d", discoverable ? "YES" : "NO", timeout);
                successFlag = setBluetoothDiscoverable(discoverable, timeout);
            } else {
                LOGERR("Please specify parameters. Example (timeout is optional): \"params\": {\"discoverable\": true, \"timeout\": \"10\"}");
                successFlag = false;
            }

            returnResponse(successFlag);
        }

        uint32_t Bluetooth::getDiscoveredDevicesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            response["discoveredDevices"] = getDiscoveredDevices();
            returnResponse(true);
        }

        uint32_t Bluetooth::getPairedDevicesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            response["pairedDevices"] = getPairedDevices();
            returnResponse(true);
        }

        uint32_t Bluetooth::getConnectedDevicesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            response["connectedDevices"] = getConnectedDevices();
            returnResponse(true);
        }

        uint32_t Bluetooth::connectWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string deviceIDStr;
            long long int deviceID = 0;
            bool deviceIDDefined = false;
            string enable = "CONNECT"; // "CONNECT" or "DISCONNECT"
            string deviceType;
            bool deviceTypeDefined = false;
            bool successFlag;

            if (parameters.HasLabel("deviceID"))
            {
                getStringParameter("deviceID", deviceIDStr);
                deviceID = stoll(deviceIDStr);
                deviceIDDefined = true;
            }

            if (parameters.HasLabel("deviceType"))
            {
                getStringParameter("deviceType", deviceType);
                deviceTypeDefined = true;
            } else {
                deviceType = "SMARTPHONE";
            }

            if (deviceIDDefined && deviceTypeDefined)
            {
                LOGINFO("Making a call with deviceID=%llu enable=%s deviceType=%s", deviceID, enable.c_str(), deviceType.c_str());
                successFlag = setDeviceConnection(deviceID, enable, deviceType);
            } else if (deviceIDDefined) {
                LOGINFO("Making a call with deviceID=%llu enable=%s", deviceID, enable.c_str());
                successFlag = setDeviceConnection(deviceID, enable);
            } else {
                LOGERR("Please specify parameters. Example: \"params\": {\"deviceID\": \"271731989589742\"}");
                successFlag = false;
            }
            returnResponse(successFlag);
        }

        uint32_t Bluetooth::disconnectWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string deviceIDStr;
            long long int deviceID = 0;
            bool deviceIDDefined = false;
            string enable = "DISCONNECT"; // "CONNECT" or "DISCONNECT"
            string deviceType;
            bool deviceTypeDefined = false;
            bool successFlag;

            if (parameters.HasLabel("deviceID"))
            {
                getStringParameter("deviceID", deviceIDStr);
                deviceID = stoll(deviceIDStr);
                deviceIDDefined = true;
            }

            if (parameters.HasLabel("deviceType"))
            {
                getStringParameter("deviceType", deviceType);
                deviceTypeDefined = true;
            } else {
                deviceType = "SMARTPHONE";
            }

            if (deviceIDDefined && deviceTypeDefined)
            {
                LOGINFO("Making a call with deviceID=%llu enable=%s deviceType=%s", deviceID, enable.c_str(), deviceType.c_str());
                successFlag = setDeviceConnection(deviceID, enable, deviceType);
            } else if (deviceIDDefined) {
                LOGINFO("Making a call with deviceID=%llu enable=%s", deviceID, enable.c_str());
                successFlag = setDeviceConnection(deviceID, enable);
            } else {
                LOGERR("Please specify parameters. Example: \"params\": {\"deviceID\": \"271731989589742\"}");
                successFlag = false;
            }
            returnResponse(successFlag);
        }

        uint32_t Bluetooth::setAudioStreamWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string deviceIDStr;
            long long int deviceID = 0;
            bool deviceIDDefined = false;
            string audioStreamName; //"PRIMARY" or "AUXILIARY"
            bool audioStreamNameDefined = false;
            bool successFlag;

            if (parameters.HasLabel("deviceID"))
            {
                getStringParameter("deviceID", deviceIDStr);
                deviceID = stoll(deviceIDStr);
                deviceIDDefined = true;
            }

            if (parameters.HasLabel("audioStreamName"))
            {
                getStringParameter("audioStreamName", audioStreamName);
                audioStreamNameDefined = true;
            }
            if (deviceIDDefined && audioStreamNameDefined)
            {
                LOGINFO("Making a call with deviceID=%llu audioStreamName=%s", deviceID, audioStreamName.c_str());
                successFlag = setAudioStream(deviceID, audioStreamName);
            } else {
                LOGERR("Please specify parameters. Example: \"params\": {\"deviceID\": \"271731989589742\", \"audioStreamName\": \"PRIMARY\"}");
                successFlag = false;
            }
            returnResponse(successFlag);
        }

        uint32_t Bluetooth::pairWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool successFlag;
            string deviceIDStr;
            long long int deviceID = 0;
            bool deviceIDDefined = false;
            bool pair = true;

            if (parameters.HasLabel("deviceID")) {
                getStringParameter("deviceID", deviceIDStr);
                deviceID = stoll(deviceIDStr);
                deviceIDDefined = true;
            }

            if(deviceIDDefined)
            {
                LOGINFO("Making a call with deviceID=%llu pair=%s", deviceID, pair?"true":"false");
                successFlag = setDevicePairing(deviceID, pair);
            } else {
                LOGERR("Please specify parameters. Example: \"params\": {\"deviceID\": \"271731989589742\"}");
                successFlag = false;
            }
            returnResponse(successFlag);
        }

        uint32_t Bluetooth::unpairWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool successFlag;
            string deviceIDStr;
            long long int deviceID = 0;
            bool deviceIDDefined = false;
            bool pair = false;

            if (parameters.HasLabel("deviceID")) {
                getStringParameter("deviceID", deviceIDStr);
                deviceID = stoll(deviceIDStr);
                deviceIDDefined = true;
            }

            if(deviceIDDefined)
            {
                LOGINFO("Making a call with deviceID=%llu pair=%s", deviceID, pair?"true":"false");
                successFlag = setDevicePairing(deviceID, pair);
            } else {
                LOGERR("Please specify parameters. Example: \"params\": {\"deviceID\": \"271731989589742\"}");
                successFlag = false;
            }
            returnResponse(successFlag);
        }

        uint32_t Bluetooth::enableWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool successFlag;
            string enabled = ENABLE_BLUETOOTH_ENABLED;
            successFlag = setBluetoothEnabled(enabled);
            returnResponse(successFlag);
        }

        uint32_t Bluetooth::disableWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool successFlag;
            string enabled = ENABLE_BLUETOOTH_DISABLED;
            successFlag = setBluetoothEnabled(enabled);
            returnResponse(successFlag);
        }

        uint32_t Bluetooth::getNameWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool successFlag;
            successFlag = getBluetoothProperties(&response);
            returnResponse(successFlag);
        }

        uint32_t Bluetooth::setNameWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool successFlag;
            successFlag = setBluetoothProperties(parameters);
            returnResponse(successFlag);
        }

        uint32_t Bluetooth::sendAudioPlaybackCommandWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string deviceIDStr;
            long long int deviceID = 0;
            bool deviceIDDefined = false;
            string audioCtrlCmd; // see CMD_AUDIO_CTRL_ entries
            bool audioCtrlCmdDefined = false;
            bool successFlag;

            if (parameters.HasLabel("deviceID"))
            {
                getStringParameter("deviceID", deviceIDStr);
                deviceID = stoll(deviceIDStr);
                deviceIDDefined = true;
            }

            if (parameters.HasLabel("command"))
            {
                getStringParameter("command", audioCtrlCmd);
                audioCtrlCmdDefined = true;
            }

            if (deviceIDDefined && audioCtrlCmdDefined)
            {
                LOGINFO("Making a call with deviceID=%llu audioCtrlCmd=%s", deviceID, audioCtrlCmd.c_str());
                successFlag = setAudioControlCommand(deviceID, audioCtrlCmd);
            } else {
                LOGERR("Please specify parameters. Example: \"params\": {\"deviceID\": \"271731989589742\", \"command\": \"PLAY\"}");
                successFlag = false;
            }
            returnResponse(successFlag);
        }


        uint32_t Bluetooth::getDeviceVolumeMuteInfoWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool successFlag;
            string deviceIDStr;
            long long int deviceID = 0;
            bool deviceIDDefined = false;
            string deviceTypeStr;
            bool deviceTypeDefined = false;
            char ui8volume = 0;
	    char mute;


            if (parameters.HasLabel("deviceID"))
            {
                getStringParameter("deviceID", deviceIDStr);
                deviceID = stoll(deviceIDStr);
                deviceIDDefined = true;
            }
            if (parameters.HasLabel("deviceType"))
            {
                getStringParameter("deviceType", deviceTypeStr);
                deviceTypeDefined = true;
            }
            if (deviceIDDefined && deviceTypeDefined)
            {
                LOGINFO("Making a call with deviceID=%llu ", deviceID);
                response ["volumeinfo"] = getDeviceVolumeMuteProperties(deviceID, deviceTypeStr);
                successFlag = true;
            } else {
                LOGERR("Please specify parameters. Example: \"params\": {\"deviceID\": \"271731989589742\", \"deviceType\": \"HEADPHONES\"}");
                successFlag = false;
            }
            returnResponse(successFlag);
        }

        uint32_t Bluetooth::setDeviceVolumeMuteInfoWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool successFlag;
            string deviceIDStr;
            long long int deviceID = 0;
            bool deviceIDDefined = false;
            string deviceTypeStr;
            bool deviceTypeDefined = false;
            unsigned char ui8volume = 0;
            int ivolume = 0;
            bool volumeDefined = false;
            unsigned char mute;
            int imute = 0;
            bool muteDefined = false;


            if (parameters.HasLabel("deviceID"))
            {
                getStringParameter("deviceID", deviceIDStr);
                deviceID = stoll(deviceIDStr);
                deviceIDDefined = true;
            }
            if (parameters.HasLabel("deviceType"))
            {
                getStringParameter("deviceType", deviceTypeStr);
                deviceTypeDefined = true;
            }
            if (parameters.HasLabel("volume"))
            {
                getNumberParameterObject(parameters, "volume", ivolume);
                ui8volume = static_cast<unsigned char>(ivolume);
                volumeDefined = true;
            }
            if (parameters.HasLabel("mute"))
            {
                getNumberParameterObject(parameters, "mute", imute);
                mute = static_cast<unsigned char>(imute);
                muteDefined = true;
            }

            if (deviceIDDefined && deviceTypeDefined && volumeDefined && muteDefined)
            {
                LOGINFO("Making a call with deviceID=%llu ", deviceID);
                successFlag = setDeviceVolumeMuteProperties(deviceID, deviceTypeStr, ui8volume, mute);
            } else {
                LOGERR("Please specify parameters. Example: \"params\": {\"deviceID\": \"271731989589742\", \"deviceType\": \"HEADPHONES\", \"volume\": \"0-255\", \"mute\": \"0-1\"}");
                successFlag = false;
            }
            returnResponse(successFlag);
        }

        uint32_t Bluetooth::setEventResponseWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string deviceIDStr;
            long long int deviceID = 0;
            bool deviceIDDefined = false;
            string eventType; // see EVT_ definitions, e.g. EVT_PAIRING_REQUEST
            bool eventTypeDefined = false;
            bool successFlag;
            string responseValue;
            bool responseValueDefined = false;

            if (parameters.HasLabel("deviceID"))
            {
                getStringParameter("deviceID", deviceIDStr);
                deviceID = stoll(deviceIDStr);
                deviceIDDefined = true;
            }

            if (parameters.HasLabel("eventType"))
            {
                getStringParameter("eventType", eventType);
                eventTypeDefined = true;
            }

            if (parameters.HasLabel("responseValue"))
            {
                getStringParameter("responseValue", responseValue);
                responseValueDefined = true;
            }

            if(deviceIDDefined && responseValueDefined && eventTypeDefined)
            {
                LOGINFO("Making a call with deviceID=%llu eventType=%s responseValue=%s", deviceID, C_STR(eventType), C_STR(responseValue));
                successFlag = setEventResponse(deviceID, eventType, responseValue);
            } else {
                LOGERR("Please specify parameters. Example: \"params\": {\"deviceID\": \"271731989589742\", \"eventType\": \"pairingRequest\", \"responseValue\": \"ACCEPTED\"}");
                successFlag = false;
            }

            returnResponse(successFlag);
        }

        uint32_t Bluetooth::getDeviceInfoWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string deviceIDStr;
            long long int deviceID = 0;
            bool successFlag;
            if (parameters.HasLabel("deviceID"))
            {
                getStringParameter("deviceID", deviceIDStr);
                deviceID = stoll(deviceIDStr);
                response["deviceInfo"] = getDeviceInfo(deviceID);
                successFlag = true;
            } else {
                LOGERR("Please specify parameters. Example: \"params\": {\"deviceID\": \"271731989589742\"}");
                successFlag = false;
            }
            returnResponse(successFlag);
        }

        uint32_t Bluetooth::getMediaTrackInfoWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            string deviceIDStr;
            long long int deviceID = 0;
            bool successFlag;
            if (parameters.HasLabel("deviceID"))
            {
                getStringParameter("deviceID", deviceIDStr);
                deviceID = stoll(deviceIDStr);
                response["trackInfo"] = getMediaTrackInfo(deviceID);
                successFlag = true;
            } else {
                LOGERR("Please specify parameters. Example: \"params\": {\"deviceID\": \"271731989589742\"}");
                successFlag = false;
            }
            returnResponse(successFlag);
        }
        //
        /// Registered methods end

        uint64_t DiscoveryTimer::Timed(const uint64_t scheduledTime)
        {
            uint64_t result = 0;
            m_bt->onDiscoveryTimer();
            return(result);
        }
    } // Plugin
} // WPEFramework
