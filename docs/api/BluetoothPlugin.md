<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.Bluetooth_Plugin"></a>
# Bluetooth Plugin

**Version: 2.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.Bluetooth plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)
- [Notifications](#head.Notifications)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.Bluetooth plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

<a name="head.Case_Sensitivity"></a>
## Case Sensitivity

All identifiers of the interfaces described in this document are case-sensitive. Thus, unless stated otherwise, all keywords, entities, properties, relations and actions should be treated as such.

<a name="head.Acronyms,_Abbreviations_and_Terms"></a>
## Acronyms, Abbreviations and Terms

The table below provides and overview of acronyms used in this document and their definitions.

| Acronym | Description |
| :-------- | :-------- |
| <a name="acronym.API">API</a> | Application Programming Interface |
| <a name="acronym.HTTP">HTTP</a> | Hypertext Transfer Protocol |
| <a name="acronym.JSON">JSON</a> | JavaScript Object Notation; a data interchange format |
| <a name="acronym.JSON-RPC">JSON-RPC</a> | A remote procedure call protocol encoded in JSON |

The table below provides and overview of terms and abbreviations used in this document and their definitions.

| Term | Description |
| :-------- | :-------- |
| <a name="term.callsign">callsign</a> | The name given to an instance of a plugin. One plugin can be instantiated multiple times, but each instance the instance name, callsign, must be unique. |

<a name="head.References"></a>
## References

| Ref ID | Description |
| :-------- | :-------- |
| <a name="ref.HTTP">[HTTP](http://www.w3.org/Protocols)</a> | HTTP specification |
| <a name="ref.JSON-RPC">[JSON-RPC](https://www.jsonrpc.org/specification)</a> | JSON-RPC 2.0 specification |
| <a name="ref.JSON">[JSON](http://www.json.org/)</a> | JSON specification |
| <a name="ref.Thunder">[Thunder](https://github.com/WebPlatformForEmbedded/Thunder/blob/master/doc/WPE%20-%20API%20-%20WPEFramework.docx)</a> | Thunder API Reference |

<a name="head.Description"></a>
# Description

The Bluetooth Plugin allows applications to interact with the Bluetooth stack and manage Bluetooth devices.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.Bluetooth*) |
| classname | string | Class name: *org.rdk.Bluetooth* |
| locator | string | Library name: *libWPEFrameworkBluetooth.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.Bluetooth plugin:

Bluetooth interface methods:

| Method | Description |
| :-------- | :-------- |
| [connect](#method.connect) | Initiates the connection with the given Bluetooth device |
| [disable](#method.disable) | Disables the Bluetooth stack |
| [disconnect](#method.disconnect) | Disconnects the given device from this device ID and triggers `onStatusChanged` Event |
| [enable](#method.enable) | Enables the Bluetooth stack |
| [getAudioInfo](#method.getAudioInfo) | Provides information on the currently playing song/audio from an external source |
| [getConnectedDevices](#method.getConnectedDevices) | Returns a list of devices connected to this device |
| [getDeviceInfo](#method.getDeviceInfo) | Returns information for the given device ID |
| [getDiscoveredDevices](#method.getDiscoveredDevices) | This method should be called after getting at least one event `onDiscoveredDevice` event and it returns an array of discovered devices |
| [getName](#method.getName) | Returns the name of this device as seen by other Bluetooth devices |
| [getPairedDevices](#method.getPairedDevices) | Returns a list of devices that have paired with this device |
| [isDiscoverable](#method.isDiscoverable) | Returns `true`, if this device can be discovered by other Bluetooth devices |
| [pair](#method.pair) | Pairs this device with device ID of Bluetooth |
| [respondToEvent](#method.respondToEvent) | Provides the ability to respond the client Bluetooth event |
| [sendAudioPlaybackCommand](#method.sendAudioPlaybackCommand) | Provides control over the connected source |
| [setAudioStream](#method.setAudioStream) | Sets the primary or secondary audio-out to the given Bluetooth device |
| [setDiscoverable](#method.setDiscoverable) | When true, this device can be discovered by other Bluetooth devices |
| [setName](#method.setName) | Sets the name of this device as seen by other Bluetooth devices |
| [startScan](#method.startScan) | Starts scanning for other Bluetooth devices that match the given profile |
| [stopScan](#method.stopScan) | Stops scanning for Bluetooth devices  if already scan is in-progress and triggers `onStatusChanged` event |
| [unpair](#method.unpair) | Unpairs the given device ID from this device |
| [getDeviceVolumeMuteInfo](#method.getDeviceVolumeMuteInfo) | Gets the volume information of the given Bluetooth device ID |
| [setDeviceVolumeMuteInfo](#method.setDeviceVolumeMuteInfo) | Sets the volume of the connected Bluetooth device ID |
| [getApiVersionNumber](#method.getApiVersionNumber) | Provides the current API version number |


<a name="method.connect"></a>
## *connect [<sup>method</sup>](#head.Methods)*

Initiates the connection with the given Bluetooth device. Triggers `onStatusChanged` event.
 
### Events 
| Event | Description | 
| :----------- | :----------- | 
| `BluetoothState: CONNECTION_CHANGE` | Triggers `onStatusChanged` event once it is  connected to the given deviceID. |.

Also see: [onStatuschanged](#event.onStatuschanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |
| params.connectedProfile | string | Profile of the connected device. See [startscan](#method.startscan) for supported profiles |

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
    "method": "org.rdk.Bluetooth.1.connect",
    "params": {
        "deviceID": "61579454946360",
        "deviceType": "TV",
        "connectedProfile": "SMARTPHONE"
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

<a name="method.disable"></a>
## *disable [<sup>method</sup>](#head.Methods)*

Disables the Bluetooth stack. 
 
### Events 
  
 No Events.

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
    "method": "org.rdk.Bluetooth.1.disable"
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

<a name="method.disconnect"></a>
## *disconnect [<sup>method</sup>](#head.Methods)*

Disconnects the given device from this device ID and triggers `onStatusChanged` Event. 
 
### Events  
| Event | Description | 
| :----------- | :----------- | 
| `BluetoothState`: `CONNECTION_CHANGE` |Triggers `onStatusChanged` event once it is disconnected from given deviceID.| .

Also see: [onStatusChanged](#event.onStatusChanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |

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
    "method": "org.rdk.Bluetooth.1.disconnect",
    "params": {
        "deviceID": "61579454946360",
        "deviceType": "TV"
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

<a name="method.enable"></a>
## *enable [<sup>method</sup>](#head.Methods)*

Enables the Bluetooth stack. 
  
### Events 

  No Events.

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
    "method": "org.rdk.Bluetooth.1.enable"
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

<a name="method.getAudioInfo"></a>
## *getAudioInfo [<sup>method</sup>](#head.Methods)*

Provides information on the currently playing song/audio from an external source. The returned information from Bluetooth-In device provides information that could be displayed on a TV screen.  
  
### Events 

  No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.trackInfo | object | An object that contains information about the currently playing audio selection |
| result.trackInfo.album | string | Name of the album |
| result.trackInfo.genre | string | Genre of the album |
| result.trackInfo.title | string | Title of the track |
| result.trackInfo.artist | string | name of the artist |
| result.trackInfo.ui32Duration | string | Duration of the track in milliseconds |
| result.trackInfo.ui32TrackNumber | string | Currently playing track |
| result.trackInfo.ui32NumberOfTracks | string | Number of total tracks |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Bluetooth.1.getAudioInfo",
    "params": {
        "deviceID": "61579454946360"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "trackInfo": {
            "album": "Spacebound Apes",
            "genre": "Jazz",
            "title": "Grace",
            "artist": "Neil Cowley Trio",
            "ui32Duration": "217292",
            "ui32TrackNumber": "1",
            "ui32NumberOfTracks": "10"
        },
        "success": true
    }
}
```

<a name="method.getConnectedDevices"></a>
## *getConnectedDevices [<sup>method</sup>](#head.Methods)*

Returns a list of devices connected to this device.  
  
### Events 

  No Events .

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.connectedDevices | array | An array of objects where each object represents a connected device |
| result.connectedDevices[#] | object |  |
| result.connectedDevices[#].deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| result.connectedDevices[#].name | string | Name of the Bluetooth Device |
| result.connectedDevices[#].deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |
| result.connectedDevices[#].activeState | string | for devices that support low power mode this parameter indicates if the device is in `STANDBY` mode (`0`), `LOW_POWER` mode (`1`), or `ACTIVE` mode (`2`) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Bluetooth.1.getConnectedDevices"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "connectedDevices": [
            {
                "deviceID": "61579454946360",
                "name": "[TV] UE32J5530",
                "deviceType": "TV",
                "activeState": "0"
            }
        ],
        "success": true
    }
}
```

<a name="method.getDeviceInfo"></a>
## *getDeviceInfo [<sup>method</sup>](#head.Methods)*

Returns information for the given device ID. 
  
### Events 

  No Events .

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.deviceInfo | object | An object that contains information about the device |
| result.deviceInfo.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| result.deviceInfo.name | string | Name of the Bluetooth Device |
| result.deviceInfo.deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |
| result.deviceInfo.supportedProfile | string | Bluetooth profile supported by the device |
| result.deviceInfo.manufacturer | string | Manufacturer of the device |
| result.deviceInfo.MAC | string | MAC address of the device |
| result.deviceInfo.rssi | string | Received signal strength of the device |
| result.deviceInfo.signalStrength | string | Bluetooth signal strength |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Bluetooth.1.getDeviceInfo",
    "params": {
        "deviceID": "61579454946360"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "deviceInfo": {
            "deviceID": "61579454946360",
            "name": "[TV] UE32J5530",
            "deviceType": "TV",
            "supportedProfile": "SMARTPHONE",
            "manufacturer": "640",
            "MAC": "E8:FB:E9:0C:XX:80",
            "rssi": "0",
            "signalStrength": "0"
        },
        "success": true
    }
}
```

<a name="method.getDiscoveredDevices"></a>
## *getDiscoveredDevices [<sup>method</sup>](#head.Methods)*

This method should be called after getting at least one event `onDiscoveredDevice` event and it returns an array of discovered devices. 
  
### Events 

  No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.discoveredDevices | array | An array of objects where each object represents a discovered device |
| result.discoveredDevices[#] | object |  |
| result.discoveredDevices[#].deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| result.discoveredDevices[#].name | string | Name of the Bluetooth Device |
| result.discoveredDevices[#].deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |
| result.discoveredDevices[#].connected | boolean | Whether the device is connected |
| result.discoveredDevices[#].paired | boolean | Whether paired or not |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Bluetooth.1.getDiscoveredDevices"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "discoveredDevices": [
            {
                "deviceID": "61579454946360",
                "name": "[TV] UE32J5530",
                "deviceType": "TV",
                "connected": true,
                "paired": true
            }
        ],
        "success": true
    }
}
```

<a name="method.getName"></a>
## *getName [<sup>method</sup>](#head.Methods)*

Returns the name of this device as seen by other Bluetooth devices. 
  
### Events 

  No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.name | string | The name of the device |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Bluetooth.1.getName"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "name": "RDK Bluetooth Device",
        "success": true
    }
}
```

<a name="method.getPairedDevices"></a>
## *getPairedDevices [<sup>method</sup>](#head.Methods)*

Returns a list of devices that have paired with this device. 
  
### Events 

  No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.pairedDevices | array | An array of objects where each object represents a paired device |
| result.pairedDevices[#] | object |  |
| result.pairedDevices[#].deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| result.pairedDevices[#].name | string | Name of the Bluetooth Device |
| result.pairedDevices[#].deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |
| result.pairedDevices[#].connected | boolean | Whether the device is connected |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Bluetooth.1.getPairedDevices"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "pairedDevices": [
            {
                "deviceID": "61579454946360",
                "name": "[TV] UE32J5530",
                "deviceType": "TV",
                "connected": true
            }
        ],
        "success": true
    }
}
```

<a name="method.isDiscoverable"></a>
## *isDiscoverable [<sup>method</sup>](#head.Methods)*

Returns `true`, if this device can be discovered by other Bluetooth devices.
  
### Events 

  No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.discoverable | boolean | Whether the device is discoverable |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Bluetooth.1.isDiscoverable"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "discoverable": true,
        "success": true
    }
}
```

<a name="method.pair"></a>
## *pair [<sup>method</sup>](#head.Methods)*

Pairs this device with device ID of Bluetooth. Triggers `onStatusChanged` and `onRequestFailed` events.
 
### Events  
| Event | Description | 
| :----------- | :----------- | 
| `BluetoothState`: `PAIRING_CHANGE` | Triggers `onStatusChanged` event when the device gets paired to given device ID. | 
| `BluetoothState`: `PAIRING_FAILED` | Triggers `onRequestFailed` event, when the device is unable to pair.|.

Also see: [onStatusChanged](#event.onStatusChanged), [onRequestFailed](#event.onRequestFailed)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |

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
    "method": "org.rdk.Bluetooth.1.pair",
    "params": {
        "deviceID": "61579454946360"
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

<a name="method.respondToEvent"></a>
## *respondToEvent [<sup>method</sup>](#head.Methods)*

Provides the ability to respond the client Bluetooth event. For example, this device can respond to a pairing or connection event and indicate the proper response to the requested device, such as the connection request accepted. 
  
### Events 

  No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.eventType | string | Name of a request-time event (for example, `onPairingRequest`, `onConnectionRequest`, `onPlaybackRequest`) |
| params.responseValue | string | one of `ACCEPTED` or `REJECTED` |

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
    "method": "org.rdk.Bluetooth.1.respondToEvent",
    "params": {
        "deviceID": "61579454946360",
        "eventType": "onPairingRequest",
        "responseValue": "ACCEPTED"
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

<a name="method.sendAudioPlaybackCommand"></a>
## *sendAudioPlaybackCommand [<sup>method</sup>](#head.Methods)*

Provides control over the connected source. Requests can have one of the following values: PLAY, PAUSE, RESUME, STOP, SKIP_NEXT, SKIP_PREV, RESTART, MUTE, UNMUTE, VOLUME_UP, VOLUME_DOWN. 
 
### Events  
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.command | string | Command to send to the connected source |

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
    "method": "org.rdk.Bluetooth.1.sendAudioPlaybackCommand",
    "params": {
        "deviceID": "61579454946360",
        "command": "PLAY"
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

<a name="method.setAudioStream"></a>
## *setAudioStream [<sup>method</sup>](#head.Methods)*

Sets the primary or secondary audio-out to the given Bluetooth device. 
  
### Events 

  No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.audioStreamName | string | The audio out to set. Either `PRIMARY` or `AUXILIARY` |

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
    "method": "org.rdk.Bluetooth.1.setAudioStream",
    "params": {
        "deviceID": "61579454946360",
        "audioStreamName": "PRIMARY"
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

<a name="method.setDiscoverable"></a>
## *setDiscoverable [<sup>method</sup>](#head.Methods)*

When true, this device can be discovered by other Bluetooth devices. When false, this device is not discoverable. 
  
### Events 

  No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.timeout | integer | <sup>*(optional)*</sup> Discoverable window timeout |
| params.discoverable | boolean | Whether the device is discoverable |

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
    "method": "org.rdk.Bluetooth.1.setDiscoverable",
    "params": {
        "timeout": 5,
        "discoverable": true
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

<a name="method.setName"></a>
## *setName [<sup>method</sup>](#head.Methods)*

Sets the name of this device as seen by other Bluetooth devices.
  
### Events 

  No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.name | string | Name of the Bluetooth Device |

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
    "method": "org.rdk.Bluetooth.1.setName",
    "params": {
        "name": "RDK Bluetooth Device"
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

<a name="method.startScan"></a>
## *startScan [<sup>method</sup>](#head.Methods)*

Starts scanning for other Bluetooth devices that match the given profile. 
 Supported profiles include:  
* For Audio-Out: `LOUDSPEAKER`, `HEADPHONES`, `WEARABLE HEADSET`, `HIFI AUDIO DEVICE`  
* For Audio-In: `SMARTPHONE`, `TABLET`  
* For HID: `KEYBOARD`, `MOUSE`, `JOYSTICK`  
* For HandsFree: `HandsFree`.  

 The method returns one of the following statuses:  
* `AVAILABLE` - Bluetooth stack is initialized, not software disabled, and hardware is running  
* `NO_BLUETOOTH_HARDWARE` - Bluetooth is supported in RDK software, but no Bluetooth hardware was found.
* This method sends both `onStatusChanged` and `onDiscoveredDevice` events.
 
### Events 
  
| Event | Description | 
| :----------- | :----------- | 
| `BluetoothState:` `DISCOVERY_STARTED` |Triggered `onStatusChanged`event when device starts scanning the other available Bluetooth devices. | 
| `BluetoothState:` `DISCOVERY_COMPLETED` | Triggered `onStatusChanged`event when timeout (timeout param) is completed or the `StopScan` method called.| 
| `DiscoveryType:` `DISCOVERED` |Triggered `onDiscoveredDevice` event when device is in scanning mode and at least one device is discovered | 
|`DiscoveryType:` `LOST` | Triggered `onDiscoveredDevice` event when the scanned device is lost|.

Also see: [onStatusChanged](#event.onStatusChanged), [onDiscoveredDevice](#event.onDiscoveredDevice)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.timeout | integer | Discoverable window timeout |
| params?.profile | string | <sup>*(optional)*</sup> List of Profiles to scan |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status | string | Discovery status |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Bluetooth.1.startScan",
    "params": {
        "timeout": 5,
        "profile": "SMARTPHONE, HEADSET"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "status": "AVAIlABLE",
        "success": true
    }
}
```

<a name="method.stopScan"></a>
## *stopScan [<sup>method</sup>](#head.Methods)*

Stops scanning for Bluetooth devices  if already scan is in-progress and triggers `onStatusChanged` event.  
 
### Events  
| Event | Description | 
| :----------- | :----------- | 
| `BluetoothState:` `DISCOVERY_COMPLETED` | Triggered `onStatusChanged` event when scan is stopped.| .

Also see: [onStatusChanged](#event.onStatusChanged)

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
    "method": "org.rdk.Bluetooth.1.stopScan"
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

<a name="method.unpair"></a>
## *unpair [<sup>method</sup>](#head.Methods)*

Unpairs the given device ID from this device. Triggers `onStatusChanged` event.
 
### Events  
| Event | Description | 
| :----------- | :----------- | 
| `BluetoothState: PAIRING_CHANGE` | Triggers `onStatusChanged` event when device is unpaired |.

Also see: [onStatusChanged](#event.onStatusChanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |

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
    "method": "org.rdk.Bluetooth.1.unpair",
    "params": {
        "deviceID": "61579454946360"
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

<a name="method.getDeviceVolumeMuteInfo"></a>
## *getDeviceVolumeMuteInfo [<sup>method</sup>](#head.Methods)*

Gets the volume information of the given Bluetooth device ID. 
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.deviceProfile | string | Profile of the Bluetooth device |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.volumeInfo | array | An array of objects where each object represents a device volume information |
| result.volumeInfo[#] | object |  |
| result.volumeInfo[#].volume | string | Volume value is in between 0 and 255 |
| result.volumeInfo[#].mute | boolean | Mute value of the device is either 0 or 1 |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Bluetooth.1.getDeviceVolumeMuteInfo",
    "params": {
        "deviceID": "61579454946360",
        "deviceProfile": "SMARTPHONE"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "volumeInfo": [
            {
                "volume": "50",
                "mute": false
            }
        ],
        "success": true
    }
}
```

<a name="method.setDeviceVolumeMuteInfo"></a>
## *setDeviceVolumeMuteInfo [<sup>method</sup>](#head.Methods)*

Sets the volume of the connected Bluetooth device ID. 
 
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.deviceProfile | string | Profile of the Bluetooth device |
| params.volume | string | Volume value is in between 0 and 255 |
| params.mute | boolean | Mute value of the device is either 0 or 1 |

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
    "method": "org.rdk.Bluetooth.1.setDeviceVolumeMuteInfo",
    "params": {
        "deviceID": "61579454946360",
        "deviceProfile": "SMARTPHONE",
        "volume": "50",
        "mute": false
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

<a name="method.getApiVersionNumber"></a>
## *getApiVersionNumber [<sup>method</sup>](#head.Methods)*

Provides the current API version number. 
 
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.version | integer | API Version Number |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Bluetooth.1.getApiVersionNumber"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "version": 1,
        "success": true
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.Bluetooth plugin:

Bluetooth interface events:

| Event | Description |
| :-------- | :-------- |
| [onConnectionRequest](#event.onConnectionRequest) | Triggered when a connection is requested by third party device that has already been paired to the set-top box |
| [onDiscoveredDevice](#event.onDiscoveredDevice) | Triggered during device discovery when a new device is discovered or a discovered device has been lost in real time |
| [onPairingRequest](#event.onPairingRequest) | Triggered when pairing is requested by a third party device that supports A2DP profile |
| [onPlaybackChange](#event.onPlaybackChange) | Triggered when playback is interrupted or changed |
| [onPlaybackNewTrack](#event.onPlaybackNewTrack) | Triggered whenever the user plays a new track or when the music player selects a next track automatically from its playlist |
| [onPlaybackProgress](#event.onPlaybackProgress) | Triggered in one second intervals as long as the status of the playback is playing |
| [onPlaybackRequest](#event.onPlaybackRequest) | Triggered when playback is requested by third party device that has already been paired to the set-top box |
| [onRequestFailed](#event.onRequestFailed) | Triggered when the previous request to pair or connect failed |
| [onStatusChanged](#event.onStatusChanged) | Triggered when the Bluetooth functionality status changes |
| [onDeviceFound](#event.onDeviceFound) | Triggered when the new device got discovered |
| [onDeviceLost](#event.onDeviceLost) | Triggered when any discovered device lost or out of range |


<a name="event.onConnectionRequest"></a>
## *onConnectionRequest [<sup>event</sup>](#head.Notifications)*

Triggered when a connection is requested by third party device that has already been paired to the set-top box.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.name | string | Name of the Bluetooth Device |
| params.deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |
| params.supportedProfile | string | Bluetooth profile supported by the device |
| params.manufacturer | string | Manufacturer of the device |
| params.MAC | string | MAC address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onConnectionRequest",
    "params": {
        "deviceID": "61579454946360",
        "name": "[TV] UE32J5530",
        "deviceType": "TV",
        "supportedProfile": "SMARTPHONE",
        "manufacturer": "640",
        "MAC": "E8:FB:E9:0C:XX:80"
    }
}
```

<a name="event.onDiscoveredDevice"></a>
## *onDiscoveredDevice [<sup>event</sup>](#head.Notifications)*

Triggered during device discovery when a new device is discovered or a discovered device has been lost in real time.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.discoveryType | string | either `DISCOVERED` or `LOST` |
| params.name | string | Name of the Bluetooth Device |
| params.deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |
| params.rawDeviceType | string | Bluetooth device class as hex code |
| params.lastConnectedState | boolean | Whether the device was last to connect. Only the last connected device has a value of `true` |
| params.paired | boolean | Whether the device is paired. 1. `true` if the device is paired when the PAIRING_CHANGE status is sent 2. `false` if the device is unpaired. **Note** The set-top box does not retain/store all paired devices across previous power cycles. In addition, if the device is unpaired as part of a previous operation and the same device gets detected in a new discovery cycle, the device will not be a paired device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDiscoveredDevice",
    "params": {
        "deviceID": "61579454946360",
        "discoveryType": "DISCOVERED",
        "name": "[TV] UE32J5530",
        "deviceType": "TV",
        "rawDeviceType": "0x060104",
        "lastConnectedState": true,
        "paired": true
    }
}
```

<a name="event.onPairingRequest"></a>
## *onPairingRequest [<sup>event</sup>](#head.Notifications)*

Triggered when pairing is requested by a third party device that supports A2DP profile. The set-top box must be enabled as an A2DP Sink device (is put in discoverable mode). 
 
 **Note** : External pairing (process initiated by an external device) is not recommended for usage in the current edition, as it might lead to unexpected results. This warning will be removed when this scenario is fully supported.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.name | string | Name of the Bluetooth Device |
| params.deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |
| params.supportedProfile | string | Bluetooth profile supported by the device |
| params.manufacturer | string | Manufacturer of the device |
| params.MAC | string | MAC address of the device |
| params.pinRequired | boolean | Whether a pin is required to complete pairing. If `true`, the pin is displayed on the TV screen |
| params.pinValue | string | Pin that is used for paring |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onPairingRequest",
    "params": {
        "deviceID": "61579454946360",
        "name": "[TV] UE32J5530",
        "deviceType": "TV",
        "supportedProfile": "SMARTPHONE",
        "manufacturer": "640",
        "MAC": "E8:FB:E9:0C:XX:80",
        "pinRequired": true,
        "pinValue": "0601"
    }
}
```

<a name="event.onPlaybackChange"></a>
## *onPlaybackChange [<sup>event</sup>](#head.Notifications)*

Triggered when playback is interrupted or changed. Note that there is no resume event. After the previously paused playback is resumed, it is followed by `onPlaybackProgress` events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.action | string | Current playback state. Either `started`, `paused`, `stopped` or `ended`. If the state is `ended`, then `position` and `Duration` are omitted |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.position | string | Current track's position in milliseconds |
| params.Duration | string | Current track's duration in milliseconds |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onPlaybackChange",
    "params": {
        "action": "started",
        "deviceID": "61579454946360",
        "position": "217000",
        "Duration": "217292"
    }
}
```

<a name="event.onPlaybackNewTrack"></a>
## *onPlaybackNewTrack [<sup>event</sup>](#head.Notifications)*

Triggered whenever the user plays a new track or when the music player selects a next track automatically from its playlist.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.album | string | Name of the album |
| params.genre | string | Genre of the album |
| params.title | string | Title of the track |
| params.artist | string | name of the artist |
| params.ui32Duration | string | Duration of the track in milliseconds |
| params.ui32TrackNumber | string | Currently playing track |
| params.ui32NumberOfTracks | string | Number of total tracks |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onPlaybackNewTrack",
    "params": {
        "deviceID": "61579454946360",
        "album": "Spacebound Apes",
        "genre": "Jazz",
        "title": "Grace",
        "artist": "Neil Cowley Trio",
        "ui32Duration": "217292",
        "ui32TrackNumber": "1",
        "ui32NumberOfTracks": "10"
    }
}
```

<a name="event.onPlaybackProgress"></a>
## *onPlaybackProgress [<sup>event</sup>](#head.Notifications)*

Triggered in one second intervals as long as the status of the playback is playing.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.position | string | Current track's position in milliseconds |
| params.Duration | string | Current track's duration in milliseconds |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onPlaybackProgress",
    "params": {
        "deviceID": "61579454946360",
        "position": "217000",
        "Duration": "217292"
    }
}
```

<a name="event.onPlaybackRequest"></a>
## *onPlaybackRequest [<sup>event</sup>](#head.Notifications)*

Triggered when playback is requested by third party device that has already been paired to the set-top box.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.name | string | Name of the Bluetooth Device |
| params.deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |
| params.supportedProfile | string | Bluetooth profile supported by the device |
| params.manufacturer | string | Manufacturer of the device |
| params.MAC | string | MAC address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onPlaybackRequest",
    "params": {
        "deviceID": "61579454946360",
        "name": "[TV] UE32J5530",
        "deviceType": "TV",
        "supportedProfile": "SMARTPHONE",
        "manufacturer": "640",
        "MAC": "E8:FB:E9:0C:XX:80"
    }
}
```

<a name="event.onRequestFailed"></a>
## *onRequestFailed [<sup>event</sup>](#head.Notifications)*

Triggered when the previous request to pair or connect failed. In absence of a failure, XRE receives `statusChanged` when pairing or connecting succeeds.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.newStatus | string | Bluetooth status on the device |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.name | string | Name of the Bluetooth Device |
| params.deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |
| params.rawDeviceType | string | Bluetooth device class as hex code |
| params.lastConnectedState | boolean | Whether the device was last to connect. Only the last connected device has a value of `true` |
| params.paired | boolean | Whether paired or not |
| params.connected | boolean | Whether the device is connected. `true` if the device is connected when the `CONNECTION_CHANGE` status is sent. `false` if the device is disconnected |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onRequestFailed",
    "params": {
        "newStatus": "DISCOVERY_COMPLETED",
        "deviceID": "61579454946360",
        "name": "[TV] UE32J5530",
        "deviceType": "TV",
        "rawDeviceType": "0x060104",
        "lastConnectedState": true,
        "paired": true,
        "connected": true
    }
}
```

<a name="event.onStatusChanged"></a>
## *onStatusChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the Bluetooth functionality status changes. Supported statuses are:  
* `PAIRING_CHANGE` - Pairing status changed. Applications get the device which got paired/unpaired as part of this message, but it's up to the application to obtain an updated list of paired devices by calling [getPairedDevices](#method.getpaireddevices).  
* `CONNECTION_CHANGE` - one or more Bluetooth connections changed status. Applications get the device that got connected/disconnected as part of this message, but it's up to the application to obtain an updated list of connected devices by calling [getConnectedDevices](#method.getconnecteddevices).  
* `DISCOVERY_COMPLETED` - Bluetooth device discovery is complete, at least one device is available. Applications should obtain an updated list of discovered devices by calling [getDiscoveredDevices](#method.getdiscovereddevices)  
* `DISCOVERY_STARTED`- The Bluetooth device discovery will be triggered, after startScan method started.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.newStatus | string | Bluetooth status on the device |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.name | string | Name of the Bluetooth Device |
| params.deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |
| params.rawDeviceType | string | Bluetooth device class as hex code |
| params.lastConnectedState | boolean | Whether the device was last to connect. Only the last connected device has a value of `true` |
| params.paired | boolean | Whether paired or not |
| params.connected | boolean | Whether device connected or not |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onStatusChanged",
    "params": {
        "newStatus": "DISCOVERY_COMPLETED",
        "deviceID": "61579454946360",
        "name": "[TV] UE32J5530",
        "deviceType": "TV",
        "rawDeviceType": "0x060104",
        "lastConnectedState": true,
        "paired": true,
        "connected": false
    }
}
```

<a name="event.onDeviceFound"></a>
## *onDeviceFound [<sup>event</sup>](#head.Notifications)*

Triggered when the new device got discovered.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.name | string | Name of the Bluetooth Device |
| params.deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |
| params.rawDeviceType | string | Bluetooth device class as hex code |
| params.lastConnectedState | boolean | Whether the device was last to connect. Only the last connected device has a value of `true` |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDeviceFound",
    "params": {
        "deviceID": "61579454946360",
        "name": "[TV] UE32J5530",
        "deviceType": "TV",
        "rawDeviceType": "0x060104",
        "lastConnectedState": true
    }
}
```

<a name="event.onDeviceLost"></a>
## *onDeviceLost [<sup>event</sup>](#head.Notifications)*

Triggered when any discovered device lost or out of range.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | string | ID that is derived from the Bluetooth MAC address. 6 byte MAC value is packed into 8 byte with leading zeros for first 2 bytes |
| params.name | string | Name of the Bluetooth Device |
| params.deviceType | string | Device class (for example: `headset`, `speakers`, etc.) |
| params.rawDeviceType | string | Bluetooth device class as hex code |
| params.lastConnectedState | boolean | Whether the device was last to connect. Only the last connected device has a value of `true` |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDeviceLost",
    "params": {
        "deviceID": "61579454946360",
        "name": "[TV] UE32J5530",
        "deviceType": "TV",
        "rawDeviceType": "0x060104",
        "lastConnectedState": true
    }
}
```

