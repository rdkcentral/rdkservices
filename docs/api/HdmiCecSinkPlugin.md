<!-- Generated automatically, DO NOT EDIT! -->
<a name="HdmiCecSinkPlugin"></a>
# HdmiCecSinkPlugin

**Version: [1.3.5](https://github.com/rdkcentral/rdkservices/blob/main/HdmiCecSink/CHANGELOG.md)**

A org.rdk.HdmiCecSink plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Notifications](#Notifications)

<a name="Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="Description"></a>
# Description

The `HdmiCecSink` plugin allows you to manage HDMI Consumer Electronics Control (CEC) sink for connected devices.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.HdmiCecSink*) |
| classname | string | Class name: *org.rdk.HdmiCecSink* |
| locator | string | Library name: *libWPEFrameworkHdmiCecSink.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.HdmiCecSink plugin:

HdmiCecSink interface methods:

| Method | Description |
| :-------- | :-------- |
| [getActiveRoute](#getActiveRoute) | Gets details for the current route from the source to sink devices |
| [getActiveSource](#getActiveSource) | Gets details for the current active source |
| [getAudioDeviceConnectedStatus](#getAudioDeviceConnectedStatus) | Get status of audio device connection |
| [getDeviceList](#getDeviceList) | Gets the number of connected source devices and system information for each device |
| [getEnabled](#getEnabled) | Returns whether HDMI-CEC is enabled on platform or not |
| [getOSDName](#getOSDName) | Returns the OSD name used by host device |
| [getVendorId](#getVendorId) | Gets the current vendor ID used by host device |
| [printDeviceList](#printDeviceList) | This is a helper debug command for developers |
| [requestActiveSource](#requestActiveSource) | Requests the active source in the network |
| [requestShortAudioDescriptor](#requestShortAudioDescriptor) | Sends the CEC Request Short Audio Descriptor (SAD) message as an event |
| [sendAudioDevicePowerOnMessage](#sendAudioDevicePowerOnMessage) | This message is used to power on the connected audio device |
| [sendGetAudioStatusMessage](#sendGetAudioStatusMessage) | Sends the CEC \<Give Audio Status\> message to request the audio status |
| [sendKeyPressEvent](#sendKeyPressEvent) | Sends the CEC \<User Control Pressed\> message when TV remote key is pressed |
| [sendUserControlPressed](#sendUserControlPressed) | Sends the CEC \<User Control Pressed\> message when TV remote key is pressed |
| [sendUserControlReleased](#sendUserControlReleased) | Sends the CEC \<User Control released\> message when TV remote key is released |
| [sendStandbyMessage](#sendStandbyMessage) | Sends a CEC \<Standby\> message to the logical address of the device |
| [setActivePath](#setActivePath) | Sets the source device to active (`setStreamPath`) |
| [setActiveSource](#setActiveSource) | Sets the current active source as TV (physical address 0 |
| [setEnabled](#setEnabled) | Enables or disables HDMI-CEC support in the platform |
| [setMenuLanguage](#setMenuLanguage) | Updates the internal data structure with the new menu Language and also broadcasts the \<Set Menu Language\> CEC message |
| [setOSDName](#setOSDName) | Sets the OSD Name used by host device |
| [setRoutingChange](#setRoutingChange) | Changes routing while switching between HDMI inputs and TV |
| [setupARCRouting](#setupARCRouting) | Enable (or disable) HDMI-CEC Audio Return Channel (ARC) routing |
| [setVendorId](#setVendorId) | Sets a vendor ID used by host device |
| [setLatencyInfo](#setLatencyInfo) | Sets the Current Latency Values such as Video Latency, Latency Flags,Audio Output Compensated value and Audio Output Delay by sending \<Report Current Latency\> message for Dynamic Auto LipSync Feature |


<a name="getActiveRoute"></a>
## *getActiveRoute*

Gets details for the current route from the source to sink devices. This API is used for debugging the route.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.available | boolean | If `true`, then there is an active source available and source details are included in the result. If `false`, then there is no active source |
| result.length | integer | The number of devices in the path list |
| result.pathList | array | Object [] of information about each device in the active path |
| result.pathList[#] | object |  |
| result.pathList[#].logicalAddress | integer | Logical address of the device |
| result.pathList[#].physicalAddress | string | Physical address of the device |
| result.pathList[#].deviceType | string | Type of the device |
| result.pathList[#].osdName | string | OSD name of the device if available |
| result.pathList[#].vendorID | string | Vendor ID of the device |
| result.ActiveRoute | string | Gives the route from source to sink with the device type and OSD name as an identifier |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.getActiveRoute"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "available": true,
        "length": 1,
        "pathList": [
            {
                "logicalAddress": 4,
                "physicalAddress": "1.0.0.0",
                "deviceType": "Playback Device",
                "osdName": "Fire TV Stick",
                "vendorID": "0ce7"
            }
        ],
        "ActiveRoute": "Playback Device 1(Fire TV Stick)-->HDMI0",
        "success": true
    }
}
```

<a name="getActiveSource"></a>
## *getActiveSource*

Gets details for the current active source.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.available | boolean | If `true`, then there is an active source available and source details are included in the result. If `false`, then there is no active source |
| result.logicalAddress | integer | Logical address of the device |
| result.physicalAddress | string | Physical address of the device |
| result.deviceType | string | Type of the device |
| result.cecVersion | string | CEC version supported |
| result.osdName | string | OSD name of the device if available |
| result.vendorID | string | Vendor ID of the device |
| result.powerStatus | string | Power status of the device |
| result.port | string | Port of the device |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.getActiveSource"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "available": true,
        "logicalAddress": 4,
        "physicalAddress": "1.0.0.0",
        "deviceType": "Playback Device",
        "cecVersion": "Version 1.4",
        "osdName": "Fire TV Stick",
        "vendorID": "0ce7",
        "powerStatus": "Standby",
        "port": "HDMI0",
        "success": true
    }
}
```

<a name="getAudioDeviceConnectedStatus"></a>
## *getAudioDeviceConnectedStatus*

Get status of audio device connection.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.connected | boolean | `true` if an audio device is connected, otherwise `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.getAudioDeviceConnectedStatus"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "connected": true,
        "success": true
    }
}
```

<a name="getDeviceList"></a>
## *getDeviceList*

Gets the number of connected source devices and system information for each device. The information includes device type, physical address, CEC version, vendor ID, power status and OSD name.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.numberofdevices | integer | number of devices in the `devicelist` array |
| result.deviceList | array | Object [] of information about each device |
| result.deviceList[#] | object |  |
| result.deviceList[#].logicalAddress | integer | Logical address of the device |
| result.deviceList[#].physicalAddress | string | Physical address of the device |
| result.deviceList[#].deviceType | string | Type of the device |
| result.deviceList[#].cecVersion | string | CEC version supported |
| result.deviceList[#].osdName | string | OSD name of the device if available |
| result.deviceList[#].vendorID | string | Vendor ID of the device |
| result.deviceList[#].powerStatus | string | Power status of the device |
| result.deviceList[#].portNumber | integer |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.getDeviceList"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "numberofdevices": 1,
        "deviceList": [
            {
                "logicalAddress": 4,
                "physicalAddress": "1.0.0.0",
                "deviceType": "Playback Device",
                "cecVersion": "Version 1.4",
                "osdName": "Fire TV Stick",
                "vendorID": "0ce7",
                "powerStatus": "Standby",
                "portNumber": 0
            }
        ],
        "success": true
    }
}
```

<a name="getEnabled"></a>
## *getEnabled*

Returns whether HDMI-CEC is enabled on platform or not.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enabled | boolean | Indicates whether HDMI-CEC is enabled (`true`) or disabled (`false`) in the platform |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.getEnabled"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enabled": false,
        "success": true
    }
}
```

<a name="getOSDName"></a>
## *getOSDName*

Returns the OSD name used by host device.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.name | string | The OSD Name |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.getOSDName"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "name": "Fire TV Stick",
        "success": true
    }
}
```

<a name="getVendorId"></a>
## *getVendorId*

Gets the current vendor ID used by host device.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.vendorid | string | Vendor ID for this device. The ID can have a maximum of 6 characters |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.getVendorId"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "vendorid": "0019fc",
        "success": true
    }
}
```

<a name="printDeviceList"></a>
## *printDeviceList*

This is a helper debug command for developers. It prints the list of connected devices and properties of connected devices like deviceType, VendorID, CEC version, PowerStatus, OSDName, PhysicalAddress etc.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.printed | boolean | Whether device list is printed |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.printDeviceList"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "printed": true,
        "success": true
    }
}
```

<a name="requestActiveSource"></a>
## *requestActiveSource*

Requests the active source in the network.

### Events

| Event | Description |
| :-------- | :-------- |
| [onActiveSourceChange](#onActiveSourceChange) | Triggered with the active source device changes. |
| [onDeviceAdded](#onDeviceAdded) | Triggered when an HDMI cable is physically connected to the HDMI port on a TV, or the power cable is connected to the source device. |
| [onDeviceInfoUpdated](#onDeviceInfoUpdated) | Triggered when device information changes (physicalAddress, deviceType, vendorID, osdName, cecVersion, powerStatus). |
### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.requestActiveSource"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="requestShortAudioDescriptor"></a>
## *requestShortAudioDescriptor*

Sends the CEC Request Short Audio Descriptor (SAD) message as an 

### Events

| Event | Description |
| :-------- | :-------- |
| [shortAudiodesciptorEvent](#shortAudiodesciptorEvent) | Triggered when SAD is received from the connected audio device. |
### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.requestShortAudioDescriptor"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="sendAudioDevicePowerOnMessage"></a>
## *sendAudioDevicePowerOnMessage*

This message is used to power on the connected audio device. Usually sent by the TV when it comes out of standby and detects audio device connected in the network.

### Events

| Event | Description |
| :-------- | :-------- |
| [setSystemAudioModeEvent](#setSystemAudioModeEvent) | Triggered when CEC <Set System Audio Mode> message of device is received. |
### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.sendAudioDevicePowerOnMessage"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="sendGetAudioStatusMessage"></a>
## *sendGetAudioStatusMessage*

Sends the CEC \<Give Audio Status\> message to request the audio status.

### Events

| Event | Description |
| :-------- | :-------- |
| [reportAudioStatusEvent](#reportAudioStatusEvent) | Triggered when CEC <Report Audio Status> message of device is received. |
### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.sendGetAudioStatusMessage"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="sendKeyPressEvent"></a>
## *sendKeyPressEvent*

Sends the CEC \<User Control Pressed\> message when TV remote key is pressed.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |
| params.keyCode | integer | The key code for the pressed key. Possible values : `0x41` (VOLUME_UP), `0x42` (VOLUME_DOWN), `0x43` (MUTE), `0x01` (UP), `0x02` (DOWN), `0x03` (LEFT), `0x04` (RIGHT), `0x00` (SELECT), `0x09` (HOME), `0x0D` (BACK), `0x20` (NUMBER_0), `0x21` (NUMBER_1), `0x22` (NUMBER_2), `0x23` (NUMBER_3), `0x24` (NUMBER_4), `0x25` (NUMBER_5), `0x26` (NUMBER_6), `0x27` (NUMBER_7), `0x28` (NUMBER_8), `0x29` (NUMBER_9) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.sendKeyPressEvent",
    "params": {
        "logicalAddress": 4,
        "keyCode": 65
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="sendUserControlPressed"></a>
## *sendUserControlPressed*

Sends the CEC \<User Control Pressed\> message when TV remote key is pressed.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |
| params.keyCode | integer | The key code for the pressed key. Possible values : `0x41` (VOLUME_UP), `0x42` (VOLUME_DOWN), `0x43` (MUTE), `0x01` (UP), `0x02` (DOWN), `0x03` (LEFT), `0x04` (RIGHT), `0x00` (SELECT), `0x09` (HOME), `0x0D` (BACK), `0x20` (NUMBER_0), `0x21` (NUMBER_1), `0x22` (NUMBER_2), `0x23` (NUMBER_3), `0x24` (NUMBER_4), `0x25` (NUMBER_5), `0x26` (NUMBER_6), `0x27` (NUMBER_7), `0x28` (NUMBER_8), `0x29` (NUMBER_9) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.sendUserControlPressed",
    "params": {
        "logicalAddress": 4,
        "keyCode": 65
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="sendUserControlReleased"></a>
## *sendUserControlReleased*

Sends the CEC \<User Control released\> message when TV remote key is released.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.sendUserControlReleased",
    "params": {
        "logicalAddress": 4
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="sendStandbyMessage"></a>
## *sendStandbyMessage*

Sends a CEC \<Standby\> message to the logical address of the device.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.sendStandbyMessage"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setActivePath"></a>
## *setActivePath*

Sets the source device to active (`setStreamPath`). The source wakes from standby if it's in the standby state.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.activePath | string | Physical address of the source device |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.setActivePath",
    "params": {
        "activePath": "1.0.0.0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setActiveSource"></a>
## *setActiveSource*

Sets the current active source as TV (physical address 0.0.0.0). This call needs to be made when the TV switches to internal tuner or any apps.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.setActiveSource"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setEnabled"></a>
## *setEnabled*

Enables or disables HDMI-CEC support in the platform.

### Events

| Event | Description |
| :-------- | :-------- |
| [reportCecEnabledEvent](#reportCecEnabledEvent) | Triggered when the HDMI-CEC is enabled. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Indicates whether HDMI-CEC is enabled (`true`) or disabled (`false`) in the platform |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.setEnabled",
    "params": {
        "enabled": false
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setMenuLanguage"></a>
## *setMenuLanguage*

Updates the internal data structure with the new menu Language and also broadcasts the \<Set Menu Language\> CEC message.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.language | string | 3 byte ASCII defined in ISO_639-2_codes (https://en.wikipedia.org/wiki/List_of_ISO_639-2_codes) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.setMenuLanguage",
    "params": {
        "language": "chi"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setOSDName"></a>
## *setOSDName*

Sets the OSD Name used by host device.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.name | string | The OSD Name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.setOSDName",
    "params": {
        "name": "Fire TV Stick"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setRoutingChange"></a>
## *setRoutingChange*

Changes routing while switching between HDMI inputs and TV.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.oldPort | string | Current active port, such as `TV`, `HDMI0`, `HDMI1`, `HDMI2` |
| params.newPort | string | New port to switch to, such as `TV`, `HDMI0`, `HDMI1`, `HDMI2` |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.setRoutingChange",
    "params": {
        "oldPort": "HDMI0",
        "newPort": "TV"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setupARCRouting"></a>
## *setupARCRouting*

Enable (or disable) HDMI-CEC Audio Return Channel (ARC) routing. Upon enabling, triggers arcInitiationEvent and upon disabling, triggers arcTerminationEvent.

### Events

| Event | Description |
| :-------- | :-------- |
| [arcInitiationEvent](#arcInitiationEvent) | Triggered when routing though the HDMI ARC port is successfully established. |
| [arcTerminationEvent](#arcTerminationEvent) | Triggered when routing though the HDMI ARC port terminates. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Indicates whether HDMI-CEC ARC is enabled (`true`) or disabled (`false`). If enabled, the CEC \<Request ARC Initiation\> and \<Report ARC Initiated\> messages are sent. If disabled, the CEC \<Request ARC Termination\> and \<Report ARC Terminated\> messages are sent |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.setupARCRouting",
    "params": {
        "enabled": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setVendorId"></a>
## *setVendorId*

Sets a vendor ID used by host device.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.vendorid | string | Vendor ID for this device. The ID can have a maximum of 6 characters |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.setVendorId",
    "params": {
        "vendorid": "0019fc"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setLatencyInfo"></a>
## *setLatencyInfo*

Sets the Current Latency Values such as Video Latency, Latency Flags,Audio Output Compensated value and Audio Output Delay by sending \<Report Current Latency\> message for Dynamic Auto LipSync Feature.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.videoLatency | string | Indicates the Video Latency value of the target device |
| params.lowLatencyMode | string | Indicates whether low latency Mode is 0 or 1 |
| params.audioOutputCompensated | string | Indicates whether Audio Output is delay compensated or not. 0 = (NA)Sent by Non-TV, 1 = TV's audio Output is delay compensated, 2 = Not delay compensated, 3 = Partially delayed |
| params.audioOutputDelay | string | Indicates the Audio Output Delay value of the target device |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCecSink.setLatencyInfo",
    "params": {
        "videoLatency": "2",
        "lowLatencyMode": "1",
        "audioOutputCompensated": "1",
        "audioOutputDelay": "20"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.HdmiCecSink plugin:

HdmiCecSink interface events:

| Event | Description |
| :-------- | :-------- |
| [arcInitiationEvent](#arcInitiationEvent) | Triggered when routing though the HDMI ARC port is successfully established |
| [arcTerminationEvent](#arcTerminationEvent) | Triggered when routing though the HDMI ARC port terminates |
| [onActiveSourceChange](#onActiveSourceChange) | Triggered with the active source device changes |
| [onDeviceAdded](#onDeviceAdded) | Triggered when an HDMI cable is physically connected to the HDMI port on a TV, or the power cable is connected to the source device |
| [onDeviceInfoUpdated](#onDeviceInfoUpdated) | Triggered when device information changes (physicalAddress, deviceType, vendorID, osdName, cecVersion, powerStatus) |
| [onDeviceRemoved](#onDeviceRemoved) | Triggered when HDMI cable is physically removed from the HDMI port on a TV or the power cable is removed from the source device |
| [onImageViewOnMsg](#onImageViewOnMsg) | Triggered when an \<Image View ON\> CEC message is received from the source device |
| [onInActiveSource](#onInActiveSource) | Triggered when the source is no longer active |
| [onTextViewOnMsg](#onTextViewOnMsg) | Triggered when a \<Text View ON\> CEC message is received from the source device |
| [onWakeupFromStandby](#onWakeupFromStandby) | Triggered when the TV is in standby mode and it receives \<Image View ON\>/ \<Text View ON\>/ \<Active Source\> CEC message from the connected source device |
| [reportAudioDeviceConnectedStatus](#reportAudioDeviceConnectedStatus) | Triggered when an audio device is added or removed |
| [reportAudioStatusEvent](#reportAudioStatusEvent) | Triggered when CEC \<Report Audio Status\> message of device is received |
| [reportFeatureAbortEvent](#reportFeatureAbortEvent) | Triggered when CEC \<Feature Abort\> message of device is received |
| [reportCecEnabledEvent](#reportCecEnabledEvent) | Triggered when the HDMI-CEC is enabled |
| [setSystemAudioModeEvent](#setSystemAudioModeEvent) | Triggered when CEC \<Set System Audio Mode\> message of device is received |
| [shortAudiodesciptorEvent](#shortAudiodesciptorEvent) | Triggered when SAD is received from the connected audio device |
| [standbyMessageReceived](#standbyMessageReceived) | Triggered when the source device changes status to `STANDBY` |


<a name="arcInitiationEvent"></a>
## *arcInitiationEvent*

Triggered when routing though the HDMI ARC port is successfully established.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.status | string | Whether the operation succeeded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.arcInitiationEvent",
    "params": {
        "status": "success"
    }
}
```

<a name="arcTerminationEvent"></a>
## *arcTerminationEvent*

Triggered when routing though the HDMI ARC port terminates.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.status | string | Whether the operation succeeded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.arcTerminationEvent",
    "params": {
        "status": "success"
    }
}
```

<a name="onActiveSourceChange"></a>
## *onActiveSourceChange*

Triggered with the active source device changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |
| params.physicalAddress | string | Physical address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onActiveSourceChange",
    "params": {
        "logicalAddress": 4,
        "physicalAddress": "1.0.0.0"
    }
}
```

<a name="onDeviceAdded"></a>
## *onDeviceAdded*

Triggered when an HDMI cable is physically connected to the HDMI port on a TV, or the power cable is connected to the source device.  After a new device is hotplugged to the port, various information is collected such as CEC version, OSD name, vendor ID, and power status. The `onDeviceAdded` event is sent as soon as any of these details are available. However, the connected device sends the information asynchronously; therefore, the information may not be collected immediately.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onDeviceAdded",
    "params": {
        "logicalAddress": 4
    }
}
```

<a name="onDeviceInfoUpdated"></a>
## *onDeviceInfoUpdated*

Triggered when device information changes (physicalAddress, deviceType, vendorID, osdName, cecVersion, powerStatus).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onDeviceInfoUpdated",
    "params": {
        "logicalAddress": 4
    }
}
```

<a name="onDeviceRemoved"></a>
## *onDeviceRemoved*

Triggered when HDMI cable is physically removed from the HDMI port on a TV or the power cable is removed from the source device. The device is considered removed when no ACK messages are received after pinging the device.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onDeviceRemoved",
    "params": {
        "logicalAddress": 4
    }
}
```

<a name="onImageViewOnMsg"></a>
## *onImageViewOnMsg*

Triggered when an \<Image View ON\> CEC message is received from the source device.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onImageViewOnMsg",
    "params": {
        "logicalAddress": 4
    }
}
```

<a name="onInActiveSource"></a>
## *onInActiveSource*

Triggered when the source is no longer active.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |
| params.physicalAddress | string | Physical address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onInActiveSource",
    "params": {
        "logicalAddress": 4,
        "physicalAddress": "1.0.0.0"
    }
}
```

<a name="onTextViewOnMsg"></a>
## *onTextViewOnMsg*

Triggered when a \<Text View ON\> CEC message is received from the source device.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onTextViewOnMsg",
    "params": {
        "logicalAddress": 4
    }
}
```

<a name="onWakeupFromStandby"></a>
## *onWakeupFromStandby*

Triggered when the TV is in standby mode and it receives \<Image View ON\>/ \<Text View ON\>/ \<Active Source\> CEC message from the connected source device. This event will be notified to UI/Application and application will bring the TV out of standby.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onWakeupFromStandby",
    "params": {
        "logicalAddress": 4
    }
}
```

<a name="reportAudioDeviceConnectedStatus"></a>
## *reportAudioDeviceConnectedStatus*

Triggered when an audio device is added or removed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.status | string | The status |
| params.audioDeviceConnected | string | `true` if an audio device is connected, otherwise `false` |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.reportAudioDeviceConnectedStatus",
    "params": {
        "status": "success",
        "audioDeviceConnected": "true"
    }
}
```

<a name="reportAudioStatusEvent"></a>
## *reportAudioStatusEvent*

Triggered when CEC \<Report Audio Status\> message of device is received.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.muteStatus | integer | The mute status |
| params.volumeLevel | integer | The volume level of device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.reportAudioStatusEvent",
    "params": {
        "muteStatus": 0,
        "volumeLevel": 28
    }
}
```

<a name="reportFeatureAbortEvent"></a>
## *reportFeatureAbortEvent*

Triggered when CEC \<Feature Abort\> message of device is received.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |
| params.opcode | integer | The opcode send to the device |
| params.FeatureAbortReason | integer | Reason for the feature abort |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.reportFeatureAbortEvent",
    "params": {
        "logicalAddress": 4,
        "opcode": 0,
        "FeatureAbortReason": 0
    }
}
```

<a name="reportCecEnabledEvent"></a>
## *reportCecEnabledEvent*

Triggered when the HDMI-CEC is enabled.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.cecEnable | string | Whether the cec is enabled |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.reportCecEnabledEvent",
    "params": {
        "cecEnable": "true"
    }
}
```

<a name="setSystemAudioModeEvent"></a>
## *setSystemAudioModeEvent*

Triggered when CEC \<Set System Audio Mode\> message of device is received.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.audioMode | string | Audio mode of system |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.setSystemAudioModeEvent",
    "params": {
        "audioMode": "On"
    }
}
```

<a name="shortAudiodesciptorEvent"></a>
## *shortAudiodesciptorEvent*

Triggered when SAD is received from the connected audio device. See `requestShortAudioDescriptor`.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ShortAudioDescriptor | array | The SAD information (formatid, audioFormatCode, numberofdescriptor) |
| params.ShortAudioDescriptor[#] | integer |  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.shortAudiodesciptorEvent",
    "params": {
        "ShortAudioDescriptor": [
            [
                0,
                10,
                2
            ]
        ]
    }
}
```

<a name="standbyMessageReceived"></a>
## *standbyMessageReceived*

Triggered when the source device changes status to `STANDBY`.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.standbyMessageReceived",
    "params": {
        "logicalAddress": 4
    }
}
```

