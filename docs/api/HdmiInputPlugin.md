<!-- Generated automatically, DO NOT EDIT! -->
<a name="HdmiInputPlugin"></a>
# HdmiInputPlugin

**Version: 2.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.HdmiInput plugin for Thunder framework.

### Table of Contents

- [Introduction](#Introduction)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Notifications](#Notifications)

<a name="Introduction"></a>
# Introduction

<a name="Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.HdmiInput plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

<a name="Case_Sensitivity"></a>
## Case Sensitivity

All identifiers of the interfaces described in this document are case-sensitive. Thus, unless stated otherwise, all keywords, entities, properties, relations and actions should be treated as such.

<a name="Acronyms,_Abbreviations_and_Terms"></a>
## Acronyms, Abbreviations and Terms

The table below provides and overview of acronyms used in this document and their definitions.

| Acronym | Description |
| :-------- | :-------- |
| <a name="API">API</a> | Application Programming Interface |
| <a name="HTTP">HTTP</a> | Hypertext Transfer Protocol |
| <a name="JSON">JSON</a> | JavaScript Object Notation; a data interchange format |
| <a name="JSON-RPC">JSON-RPC</a> | A remote procedure call protocol encoded in JSON |

The table below provides and overview of terms and abbreviations used in this document and their definitions.

| Term | Description |
| :-------- | :-------- |
| <a name="callsign">callsign</a> | The name given to an instance of a plugin. One plugin can be instantiated multiple times, but each instance the instance name, callsign, must be unique. |

<a name="References"></a>
## References

| Ref ID | Description |
| :-------- | :-------- |
| <a name="HTTP">[HTTP](http://www.w3.org/Protocols)</a> | HTTP specification |
| <a name="JSON-RPC">[JSON-RPC](https://www.jsonrpc.org/specification)</a> | JSON-RPC 2.0 specification |
| <a name="JSON">[JSON](http://www.json.org/)</a> | JSON specification |
| <a name="Thunder">[Thunder](https://github.com/WebPlatformForEmbedded/Thunder/blob/master/doc/WPE%20-%20API%20-%20WPEFramework.docx)</a> | Thunder API Reference |

<a name="Description"></a>
# Description

The `HdmiInput` plugin allows you to control the HDMI Input on a set-top box.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.HdmiInput*) |
| classname | string | Class name: *org.rdk.HdmiInput* |
| locator | string | Library name: *libWPEFrameworkHdmiInput.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.HdmiInput plugin:

HdmiInput interface methods:

| Method | Description |
| :-------- | :-------- |
| [getHDMIInputDevices](#getHDMIInputDevices) | Returns an array of available HDMI Input ports |
| [getEdidVersion](#getEdidVersion) | (Version 2) Returns the EDID version |
| [getHDMISPD](#getHDMISPD) | (Version 2) Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device |
| [getRawHDMISPD](#getRawHDMISPD) | (Version 2) Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device as raw bits |
| [readEDID](#readEDID) | Returns the current EDID value |
| [startHdmiInput](#startHdmiInput) | Activates the specified HDMI Input port as the primary video source |
| [stopHdmiInput](#stopHdmiInput) | Deactivates the HDMI Input port currently selected as the primary video source |
| [setEdidVersion](#setEdidVersion) | (Version 2) Sets an HDMI EDID version |
| [setVideoRectangle](#setVideoRectangle) | Sets an HDMI Input video window |
| [writeEDID](#writeEDID) | Changes a current EDID value |
| [getSupportedGameFeatures](#getSupportedGameFeatures) | Returns the list of supported game features |
| [getHdmiGameFeatureStatus](#getHdmiGameFeatureStatus) | Returns the Game Feature Status |


<a name="getHDMIInputDevices"></a>
## *getHDMIInputDevices*

Returns an array of available HDMI Input ports.
 
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.devices | array | An object [] that describes each HDMI Input port |
| result.devices[#] | object |  |
| result.devices[#].id | number | The port identifier for the HDMI Input |
| result.devices[#].locator | string | A URL corresponding to the HDMI Input port |
| result.devices[#].connected | boolean | Whether a device is currently connected to this HDMI Input port |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiInput.1.getHDMIInputDevices"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "devices": [
            {
                "id": 0,
                "locator": "hdmiin://localhost/deviceid/0",
                "connected": true
            }
        ],
        "success": true
    }
}
```

<a name="getEdidVersion"></a>
## *getEdidVersion*

(Version 2) Returns the EDID version.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.portId | string | <sup>*(optional)*</sup> An ID of an HDMI Input port as returned by the `getHdmiInputDevices` method |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.edidVersion | string | The EDID version |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiInput.1.getEdidVersion",
    "params": {
        "portId": "0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "edidVersion": "HDMI2.0",
        "success": true
    }
}
```

<a name="getHDMISPD"></a>
## *getHDMISPD*

(Version 2) Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device. The SPD infoFrame packet includes vendor name, product description, and source information.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.portId | string | <sup>*(optional)*</sup> An ID of an HDMI Input port as returned by the `getHdmiInputDevices` method |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.HDMISPD | string | The SPD information |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiInput.1.getHDMISPD",
    "params": {
        "portId": "0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "HDMISPD": "...",
        "success": true
    }
}
```

<a name="getRawHDMISPD"></a>
## *getRawHDMISPD*

(Version 2) Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device as raw bits.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.portId | string | An ID of an HDMI Input port as returned by the `getHdmiInputDevices` method |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.HDMISPD | string | The SPD information as raw bits |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiInput.1.getRawHDMISPD",
    "params": {
        "portId": "0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "HDMISPD": "...",
        "success": true
    }
}
```

<a name="readEDID"></a>
## *readEDID*

Returns the current EDID value.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceId | number | The port identifier for the HDMI Input |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.EDID | string | The EDID Value |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiInput.1.readEDID",
    "params": {
        "deviceId": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "EDID": "...",
        "success": true
    }
}
```

<a name="startHdmiInput"></a>
## *startHdmiInput*

Activates the specified HDMI Input port as the primary video source.
 
### Events 
| Event | Description | 
| :----------- | :----------- | 
| `onInputStatusChanged` | Triggers the event when HDMI Input source is activated and Input status changes to `started` | 
| `onSignalChanged` | Triggers the event when HDMI Input signal changes (must be one of the following:noSignal, unstableSignal, notSupportedSignal, stableSignal).

Also see: [onInputStatusChanged](#onInputStatusChanged), [onSignalChanged](#onSignalChanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.portId | string | <sup>*(optional)*</sup> An ID of an HDMI Input port as returned by the `getHdmiInputDevices` method |

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
    "method": "org.rdk.HdmiInput.1.startHdmiInput",
    "params": {
        "portId": "0"
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

<a name="stopHdmiInput"></a>
## *stopHdmiInput*

Deactivates the HDMI Input port currently selected as the primary video source.
 
### Events 
| Event | Description | 
| :----------- | :----------- | 
| `onInputStatusChanged` | Triggers the event when HDMI Input source is deactivated and Input status changes to `stopped`.

Also see: [onInputStatusChanged](#onInputStatusChanged)

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
    "method": "org.rdk.HdmiInput.1.stopHdmiInput"
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

<a name="setEdidVersion"></a>
## *setEdidVersion*

(Version 2) Sets an HDMI EDID version.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.portId | string | An ID of an HDMI Input port as returned by the `getHdmiInputDevices` method |
| params.edidVersion | string | The EDID version |

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
    "method": "org.rdk.HdmiInput.1.setEdidVersion",
    "params": {
        "portId": "0",
        "edidVersion": "HDMI2.0"
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

<a name="setVideoRectangle"></a>
## *setVideoRectangle*

Sets an HDMI Input video window.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.x | integer | The x-coordinate of the video rectangle |
| params.y | integer | The y-coordinate of the video rectangle |
| params.w | integer | The width of the video rectangle |
| params.h | integer | The height of the video rectangle |

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
    "method": "org.rdk.HdmiInput.1.setVideoRectangle",
    "params": {
        "x": 0,
        "y": 0,
        "w": 1920,
        "h": 1080
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

<a name="writeEDID"></a>
## *writeEDID*

Changes a current EDID value.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceId | number | The ID of an input device for which the EDID should be changed |
| params.message | string | A new EDID value |

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
    "method": "org.rdk.HdmiInput.1.writeEDID",
    "params": {
        "deviceId": 0,
        "message": "EDID"
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

<a name="getSupportedGameFeatures"></a>
## *getSupportedGameFeatures*

Returns the list of supported game features.
 
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedGameFeatures | string | The supported game Features |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiInput.1.getSupportedGameFeatures"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedGameFeatures": "ALLM",
        "success": true
    }
}
```

<a name="getHdmiGameFeatureStatus"></a>
## *getHdmiGameFeatureStatus*

Returns the Game Feature Status. For example: ALLM
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.portId | string | <sup>*(optional)*</sup> An ID of an HDMI Input port as returned by the `getHdmiInputDevices` method |
| params.gameFeature | string | Game Feature to which current status requested |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.mode | boolean | The current game feature status. Mode is required only for ALLM. Need to add support for future game features |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiInput.1.getHdmiGameFeatureStatus",
    "params": {
        "portId": "0",
        "gameFeature": "ALLM"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "mode": true,
        "success": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.HdmiInput plugin:

HdmiInput interface events:

| Event | Description |
| :-------- | :-------- |
| [onDevicesChanged](#onDevicesChanged) | Triggered whenever a new HDMI device is connected to an HDMI Input |
| [onInputStatusChanged](#onInputStatusChanged) | Triggered whenever the status changes for an HDMI Input |
| [onSignalChanged](#onSignalChanged) | Triggered whenever the signal status changes for an HDMI Input |
| [videoStreamInfoUpdate](#videoStreamInfoUpdate) | Triggered whenever there is an update in HDMI Input video stream info |
| [hdmiGameFeatureStatusUpdate](#hdmiGameFeatureStatusUpdate) | Triggered whenever game feature(ALLM) status changes for an HDMI Input |


<a name="onDevicesChanged"></a>
## *onDevicesChanged*

Triggered whenever a new HDMI device is connected to an HDMI Input.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.devices | array | An object [] that describes each HDMI Input port |
| params.devices[#] | object |  |
| params.devices[#].id | number | The port identifier for the HDMI Input |
| params.devices[#].locator | string | A URL corresponding to the HDMI Input port |
| params.devices[#].connected | boolean | Whether a device is currently connected to this HDMI Input port |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDevicesChanged",
    "params": {
        "devices": [
            {
                "id": 0,
                "locator": "hdmiin://localhost/deviceid/0",
                "connected": true
            }
        ]
    }
}
```

<a name="onInputStatusChanged"></a>
## *onInputStatusChanged*

Triggered whenever the status changes for an HDMI Input.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | number | The port identifier for the HDMI Input |
| params.locator | string | A URL corresponding to the HDMI Input port |
| params.status | string | Status of the HDMI Input. Valid values are `started` or `stopped` |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onInputStatusChanged",
    "params": {
        "id": 0,
        "locator": "hdmiin://localhost/deviceid/0",
        "status": "started"
    }
}
```

<a name="onSignalChanged"></a>
## *onSignalChanged*

Triggered whenever the signal status changes for an HDMI Input.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | number | The port identifier for the HDMI Input |
| params.locator | string | A URL corresponding to the HDMI Input port |
| params.signalStatus | string | Signal Status of the HDMI Input. Valid values are `noSignal`, `unstableSignal`, `notSupportedSignal`, `stableSignal` |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onSignalChanged",
    "params": {
        "id": 0,
        "locator": "hdmiin://localhost/deviceid/0",
        "signalStatus": "stableSignal"
    }
}
```

<a name="videoStreamInfoUpdate"></a>
## *videoStreamInfoUpdate*

Triggered whenever there is an update in HDMI Input video stream info.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | number | The port identifier for the HDMI Input |
| params.locator | string | A URL corresponding to the HDMI Input port |
| params.width | integer | Width of the Video Stream |
| params.height | integer | Height of the Video Stream |
| params.progressive | boolean | Whether the streaming video is progressive or not? |
| params.frameRateN | integer | FrameRate Numerator |
| params.frameRateD | integer | FrameRate Denomirator |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.videoStreamInfoUpdate",
    "params": {
        "id": 0,
        "locator": "hdmiin://localhost/deviceid/0",
        "width": 3840,
        "height": 2160,
        "progressive": true,
        "frameRateN": 60000,
        "frameRateD": 1001
    }
}
```

<a name="hdmiGameFeatureStatusUpdate"></a>
## *hdmiGameFeatureStatusUpdate*

Triggered whenever game feature(ALLM) status changes for an HDMI Input.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.portId | string | An ID of an HDMI Input port as returned by the `getHdmiInputDevices` method |
| params.gameFeature | string | Game Feature to which current status requested |
| params.mode | boolean | The current game feature status. Mode is required only for ALLM. Need to add support for future game features |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.hdmiGameFeatureStatusUpdate",
    "params": {
        "portId": "0",
        "gameFeature": "ALLM",
        "mode": true
    }
}
```

