<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.AVInput_Plugin"></a>
# AVInput Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.AVInput plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.AVInput plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `AVInput` plugin allows you to control the Hdmi and Composite input source on a device.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.AVInput*) |
| classname | string | Class name: *org.rdk.AVInput* |
| locator | string | Library name: *libWPEFrameworkAVInput.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.AVInput plugin:

AVInput interface methods:

| Method | Description |
| :-------- | :-------- |
| [contentProtected](#method.contentProtected) | Returns `true` if the content coming in the HDMI input is protected; otherwise, it returns `false` |
| [currentVideoMode](#method.currentVideoMode) | Returns a string encoding the video mode being supplied by the device currently attached to the HDMI input |
| [numberOfInputs](#method.numberOfInputs) | Returns an integer that specifies the number of available inputs |
| [getInputDevices](#method.getInputDevices) | Returns an array of available HDMI/Composite Input ports |
| [getEdidVersion](#method.getEdidVersion) | (Version 2) Returns the EDID version |
| [getSPD](#method.getSPD) | (Version 2) Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device |
| [getRawSPD](#method.getRawSPD) | (Version 2) Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device as raw bits |
| [readEDID](#method.readEDID) | Returns the current EDID value |
| [startInput](#method.startInput) | Activates the specified HDMI/Composite Input port as the primary video source |
| [stopInput](#method.stopInput) | Deactivates the HDMI/Composite Input port currently selected as the primary video source |
| [setEdidVersion](#method.setEdidVersion) | (Version 2) Sets an HDMI EDID version |
| [setVideoRectangle](#method.setVideoRectangle) | Sets an HDMI/Composite Input video window |
| [writeEDID](#method.writeEDID) | Changes a current EDID value |
| [getSupportedGameFeatures](#method.getSupportedGameFeatures) | Returns the list of supported game features |
| [getGameFeatureStatus](#method.getGameFeatureStatus) | Returns the Game Feature Status |


<a name="method.contentProtected"></a>
## *contentProtected [<sup>method</sup>](#head.Methods)*

Returns `true` if the content coming in the HDMI input is protected; otherwise, it returns `false`. If the content is protected, then it is only presented if the component and composite outputs of the box are disabled.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.isContentProtected | boolean | Whether the HDMI input is protected |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.1.contentProtected",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "isContentProtected": true,
        "success": true
    }
}
```

<a name="method.currentVideoMode"></a>
## *currentVideoMode [<sup>method</sup>](#head.Methods)*

Returns a string encoding the video mode being supplied by the device currently attached to the HDMI input. The format of the string is the same format used for the `resolutionName` parameter of the XRE `setResolution` messages. HDMI input is presentable if its resolution is less than or equal to the current Parker display resolution. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.currentVideoMode | string | The current video mode |
| result.message | string | `Success` if plugin is activated successfully and gets the current Videomode. `org.rdk.HdmiInput plugin is not ready` if plugin is not activated or activation failed |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.1.currentVideoMode",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "currentVideoMode": "Unknown",
        "message": "Success",
        "success": true
    }
}
```

<a name="method.numberOfInputs"></a>
## *numberOfInputs [<sup>method</sup>](#head.Methods)*

Returns an integer that specifies the number of available inputs. For example, a value of `2` indicates that there are two available inputs that can be selected using `avin://input0` and `avin://input1`. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.numberOfInputs | number | The number of inputs that are available for selection |
| result.message | string | `Success` if plugin is activated successfully and gets the current Videomode. `org.rdk.HdmiInput plugin is not ready` if plugin is not activated or activation failed |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.1.numberOfInputs",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "numberOfInputs": 1,
        "message": "Success",
        "success": true
    }
}
```

<a name="method.getInputDevices"></a>
## *getInputDevices [<sup>method</sup>](#head.Methods)*

Returns an array of available HDMI/Composite Input ports.
 
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.devices | array | An object [] that describes each HDMI/Composite Input port |
| result.devices[#] | object |  |
| result.devices[#].id | number | The port identifier for the HDMI/Composite Input |
| result.devices[#].locator | string | A URL corresponding to the HDMI/Composite Input port |
| result.devices[#].connected | boolean | Whether a device is currently connected to this HDMI/Composite Input port |
| result.success | boolean | Whether the request succeeded |
| result?.typeOfInput | string | <sup>*(optional)*</sup> The type of Input - HDMI/COMPOSITE |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.1.getInputDevices"
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
        "success": true,
        "typeOfInput": "HDMI"
    }
}
```

<a name="method.getEdidVersion"></a>
## *getEdidVersion [<sup>method</sup>](#head.Methods)*

(Version 2) Returns the EDID version.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.portId | string | <sup>*(optional)*</sup> An ID of an HDMI/Composite Input port as returned by the `getInputDevices` method |

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
    "method": "org.rdk.AVInput.1.getEdidVersion",
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

<a name="method.getSPD"></a>
## *getSPD [<sup>method</sup>](#head.Methods)*

(Version 2) Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device. The SPD infoFrame packet includes vendor name, product description, and source information.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.portId | string | <sup>*(optional)*</sup> An ID of an HDMI/Composite Input port as returned by the `getInputDevices` method |

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
    "method": "org.rdk.AVInput.1.getSPD",
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

<a name="method.getRawSPD"></a>
## *getRawSPD [<sup>method</sup>](#head.Methods)*

(Version 2) Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device as raw bits.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.portId | string | An ID of an HDMI/Composite Input port as returned by the `getInputDevices` method |

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
    "method": "org.rdk.AVInput.1.getRawSPD",
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

<a name="method.readEDID"></a>
## *readEDID [<sup>method</sup>](#head.Methods)*

Returns the current EDID value.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceId | number | The port identifier for the HDMI/Composite Input |

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
    "method": "org.rdk.AVInput.1.readEDID",
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

<a name="method.startInput"></a>
## *startInput [<sup>method</sup>](#head.Methods)*

Activates the specified HDMI/Composite Input port as the primary video source.
 
### Events 
| Event | Description | 
| :----------- | :----------- | 
| `onInputStatusChanged` | Triggers the event when HDMI/Composite Input source is activated and Input status changes to `started` | 
| `onSignalChanged` | Triggers the event when HDMI/Composite Input signal changes (must be one of the following:noSignal, unstableSignal, notSupportedSignal, stableSignal).

Also see: [onInputStatusChanged](#event.onInputStatusChanged), [onSignalChanged](#event.onSignalChanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.portId | string | <sup>*(optional)*</sup> An ID of an HDMI/Composite Input port as returned by the `getInputDevices` method |
| params.typeOfInput | string | The type of Input - HDMI/COMPOSITE |

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
    "method": "org.rdk.AVInput.1.startInput",
    "params": {
        "portId": "0",
        "typeOfInput": "HDMI"
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

<a name="method.stopInput"></a>
## *stopInput [<sup>method</sup>](#head.Methods)*

Deactivates the HDMI/Composite Input port currently selected as the primary video source.
 
### Events 
| Event | Description | 
| :----------- | :----------- | 
| `onInputStatusChanged` | Triggers the event when HDMI/Composite Input source is deactivated and Input status changes to `stopped`.

Also see: [onInputStatusChanged](#event.onInputStatusChanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.typeOfInput | string | The type of Input - HDMI/COMPOSITE |

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
    "method": "org.rdk.AVInput.1.stopInput",
    "params": {
        "typeOfInput": "HDMI"
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

<a name="method.setEdidVersion"></a>
## *setEdidVersion [<sup>method</sup>](#head.Methods)*

(Version 2) Sets an HDMI EDID version.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.portId | string | An ID of an HDMI/Composite Input port as returned by the `getInputDevices` method |
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
    "method": "org.rdk.AVInput.1.setEdidVersion",
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

<a name="method.setVideoRectangle"></a>
## *setVideoRectangle [<sup>method</sup>](#head.Methods)*

Sets an HDMI/Composite Input video window.
 
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
| params.typeOfInput | string | The type of Input - HDMI/COMPOSITE |

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
    "method": "org.rdk.AVInput.1.setVideoRectangle",
    "params": {
        "x": 0,
        "y": 0,
        "w": 1920,
        "h": 1080,
        "typeOfInput": "HDMI"
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

<a name="method.writeEDID"></a>
## *writeEDID [<sup>method</sup>](#head.Methods)*

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
    "method": "org.rdk.AVInput.1.writeEDID",
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

<a name="method.getSupportedGameFeatures"></a>
## *getSupportedGameFeatures [<sup>method</sup>](#head.Methods)*

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
    "method": "org.rdk.AVInput.1.getSupportedGameFeatures"
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

<a name="method.getGameFeatureStatus"></a>
## *getGameFeatureStatus [<sup>method</sup>](#head.Methods)*

Returns the Game Feature Status. For example: ALLM
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.portId | string | <sup>*(optional)*</sup> An ID of an HDMI/Composite Input port as returned by the `getInputDevices` method |
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
    "method": "org.rdk.AVInput.1.getGameFeatureStatus",
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

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.AVInput plugin:

AVInput interface events:

| Event | Description |
| :-------- | :-------- |
| [onAVInputActive](#event.onAVInputActive) | Triggered when an active device is connected to an AVInput port |
| [onAVInputInActive](#event.onAVInputInActive) | Triggered when an active device is disconnected from an AVInput port or when the device becomes inactive |
| [onDevicesChanged](#event.onDevicesChanged) | Triggered whenever a new HDMI/Composite device is connected to an HDMI/Composite Input |
| [onInputStatusChanged](#event.onInputStatusChanged) | Triggered whenever the status changes for an HDMI/Composite Input |
| [onSignalChanged](#event.onSignalChanged) | Triggered whenever the signal status changes for an HDMI/Composite Input |
| [videoStreamInfoUpdate](#event.videoStreamInfoUpdate) | Triggered whenever there is an update in HDMI Input video stream info |
| [AVGameFeatureStatusUpdate](#event.AVGameFeatureStatusUpdate) | Triggered whenever game feature(ALLM) status changes for an HDMI Input |


<a name="event.onAVInputActive"></a>
## *onAVInputActive [<sup>event</sup>](#head.Notifications)*

Triggered when an active device is connected to an AVInput port.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.url | string | The URL of the port with an active device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onAVInputActive",
    "params": {
        "url": "avin://input0"
    }
}
```

<a name="event.onAVInputInActive"></a>
## *onAVInputInActive [<sup>event</sup>](#head.Notifications)*

Triggered when an active device is disconnected from an AVInput port or when the device becomes inactive.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.url | string | The URL of the port with an inactive device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onAVInputInActive",
    "params": {
        "url": "avin://input0"
    }
}
```

<a name="event.onDevicesChanged"></a>
## *onDevicesChanged [<sup>event</sup>](#head.Notifications)*

Triggered whenever a new HDMI/Composite device is connected to an HDMI/Composite Input.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.devices | array | An object [] that describes each HDMI/Composite Input port |
| params.devices[#] | object |  |
| params.devices[#].id | number | The port identifier for the HDMI/Composite Input |
| params.devices[#].locator | string | A URL corresponding to the HDMI/Composite Input port |
| params.devices[#].connected | boolean | Whether a device is currently connected to this HDMI/Composite Input port |

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

<a name="event.onInputStatusChanged"></a>
## *onInputStatusChanged [<sup>event</sup>](#head.Notifications)*

Triggered whenever the status changes for an HDMI/Composite Input.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | number | The port identifier for the HDMI/Composite Input |
| params.locator | string | A URL corresponding to the HDMI/Composite Input port |
| params.status | string | Status of the HDMI/Composite Input. Valid values are `started` or `stopped` |

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

<a name="event.onSignalChanged"></a>
## *onSignalChanged [<sup>event</sup>](#head.Notifications)*

Triggered whenever the signal status changes for an HDMI/Composite Input.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | number | The port identifier for the HDMI/Composite Input |
| params.locator | string | A URL corresponding to the HDMI/Composite Input port |
| params.signalStatus | string | Signal Status of the HDMI/Composite Input. Valid values are `noSignal`, `unstableSignal`, `notSupportedSignal`, `stableSignal` |

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

<a name="event.videoStreamInfoUpdate"></a>
## *videoStreamInfoUpdate [<sup>event</sup>](#head.Notifications)*

Triggered whenever there is an update in HDMI Input video stream info.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | number | The port identifier for the HDMI/Composite Input |
| params.locator | string | A URL corresponding to the HDMI/Composite Input port |
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

<a name="event.AVGameFeatureStatusUpdate"></a>
## *AVGameFeatureStatusUpdate [<sup>event</sup>](#head.Notifications)*

Triggered whenever game feature(ALLM) status changes for an HDMI Input.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.portId | string | An ID of an HDMI/Composite Input port as returned by the `getInputDevices` method |
| params.gameFeature | string | Game Feature to which current status requested |
| params.mode | boolean | The current game feature status. Mode is required only for ALLM. Need to add support for future game features |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.AVGameFeatureStatusUpdate",
    "params": {
        "portId": "0",
        "gameFeature": "ALLM",
        "mode": true
    }
}
```

