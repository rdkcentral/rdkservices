<!-- Generated automatically, DO NOT EDIT! -->
<a name="CompositeInput_Plugin"></a>
# CompositeInput Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.CompositeInput plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.CompositeInput plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `CompositeInput` Plugin allows you to control the composite input source on a device.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.CompositeInput*) |
| classname | string | Class name: *org.rdk.CompositeInput* |
| locator | string | Library name: *libWPEFrameworkCompositeInput.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.CompositeInput plugin:

CompositeInput interface methods:

| Method | Description |
| :-------- | :-------- |
| [getCompositeInputDevices](#getCompositeInputDevices) | returns a list of composite input devices |
| [setVideoRectangle](#setVideoRectangle) | Sets the composite input video window |
| [startCompositeInput](#startCompositeInput) | Activates the specified composite input as the primary video source |
| [stopCompositeInput](#stopCompositeInput) | Deactivates the current composite input source that is the primary video source |


<a name="getCompositeInputDevices"></a>
## *getCompositeInputDevices*

returns a list of composite input devices.
 
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.devices | array | An object array that lists the composite input ports on the device |
| result.devices[#] | object |  |
| result.devices[#].id | number | The ID of the composite input source |
| result.devices[#].locator | string | The location of the input source on the device |
| result.devices[#].connected | boolean | `true` if the input source is connected, otherwise `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.CompositeInput.1.getCompositeInputDevices"
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
                "locator": "cvbsin://localhost/deviceid/0",
                "connected": true
            }
        ],
        "success": true
    }
}
```

<a name="setVideoRectangle"></a>
## *setVideoRectangle*

Sets the composite input video window.
 
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
    "method": "org.rdk.CompositeInput.1.setVideoRectangle",
    "params": {
        "x": 900,
        "y": 500,
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

<a name="startCompositeInput"></a>
## *startCompositeInput*

Activates the specified composite input as the primary video source.
 
### Events 
| Event | Description | 
| :----------- | :----------- | 
| `onInputStatusChanged` | Triggers this event when activates composite input source and input status changes to `started` | 
| `onSignalChanged` | Triggers this event when composite input signal changes (must be one of the following:noSignal, unstableSignal, notSupportedSignal, stableSignal) |.

Also see: [onInputStatusChanged](#onInputStatusChanged), [onSignalChanged](#onSignalChanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.portId | number | The id of a composite input source as returned by `getCompositeInputDevices` |

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
    "method": "org.rdk.CompositeInput.1.startCompositeInput",
    "params": {
        "portId": 0
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

<a name="stopCompositeInput"></a>
## *stopCompositeInput*

Deactivates the current composite input source that is the primary video source.
 
### Events 
| Event | Description | 
| :----------- | :----------- | 
| `onInputStatusChanged` | Triggers this event when deactivates composite input source and input status changes to `stopped` |.

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
    "method": "org.rdk.CompositeInput.1.stopCompositeInput"
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

The following events are provided by the org.rdk.CompositeInput plugin:

CompositeInput interface events:

| Event | Description |
| :-------- | :-------- |
| [onDevicesChanged](#onDevicesChanged) | Triggered when the composite input device changes |
| [onInputStatusChanged](#onInputStatusChanged) | Triggered when the status of the composite input changes |
| [onSignalChanged](#onSignalChanged) | Triggered when the status of the composite input signal changes |


<a name="onDevicesChanged"></a>
## *onDevicesChanged*

Triggered when the composite input device changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.devices | array | An object array that lists the composite input ports on the device |
| params.devices[#] | object |  |
| params.devices[#].id | number | The ID of the composite input source |
| params.devices[#].locator | string | The location of the input source on the device |
| params.devices[#].connected | boolean | `true` if the input source is connected, otherwise `false` |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDevicesChanged",
    "params": {
        "devices": [
            {
                "id": 0,
                "locator": "cvbsin://localhost/deviceid/0",
                "connected": true
            }
        ]
    }
}
```

<a name="onInputStatusChanged"></a>
## *onInputStatusChanged*

Triggered when the status of the composite input changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | number | The ID of the composite input source |
| params.locator | string | The location of the input source on the device |
| params.status | string | The status of the composite input source. (must be one of the following: *started*, *stopped*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onInputStatusChanged",
    "params": {
        "id": 0,
        "locator": "cvbsin://localhost/deviceid/0",
        "status": "started"
    }
}
```

<a name="onSignalChanged"></a>
## *onSignalChanged*

Triggered when the status of the composite input signal changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | number | The ID of the composite input source |
| params.locator | string | The location of the input source on the device |
| params.signalStatus | string | The status of the composite input source signal. (must be one of the following: *noSignal*, *unstableSignal*, *notSupportedSignal*, *stableSignal*, *none*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onSignalChanged",
    "params": {
        "id": 0,
        "locator": "cvbsin://localhost/deviceid/0",
        "signalStatus": "noSignal"
    }
}
```

