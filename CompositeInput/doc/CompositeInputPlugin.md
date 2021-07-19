<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.CompositeInput_Plugin"></a>
# CompositeInput Plugin

**Version: 2.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.CompositeInput plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.CompositeInput plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `CompositeInput` Plugin allows you to control the composite input source on a device.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.CompositeInput*) |
| classname | string | Class name: *org.rdk.CompositeInput* |
| locator | string | Library name: *libWPEFrameworkCompositeInput.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.CompositeInput plugin:

CompositeInput interface methods:

| Method | Description |
| :-------- | :-------- |
| [getCompositeInputDevices](#method.getCompositeInputDevices) | returns a list of composite input devices |
| [setVideoRectangle](#method.setVideoRectangle) | Sets the composite input video window |
| [startCompositeInput](#method.startCompositeInput) | Activates the specified composite input as the primary video source |
| [stopCompositeInput](#method.stopCompositeInput) | Deactivates the current composite input source that is the primary video source |


<a name="method.getCompositeInputDevices"></a>
## *getCompositeInputDevices <sup>method</sup>*

returns a list of composite input devices.

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
    "id": 1234567890,
    "method": "org.rdk.CompositeInput.1.getCompositeInputDevices"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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

<a name="method.setVideoRectangle"></a>
## *setVideoRectangle <sup>method</sup>*

Sets the composite input video window.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.startCompositeInput"></a>
## *startCompositeInput <sup>method</sup>*

Activates the specified composite input as the primary video source.

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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.stopCompositeInput"></a>
## *stopCompositeInput <sup>method</sup>*

Deactivates the current composite input source that is the primary video source.

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
    "id": 1234567890,
    "method": "org.rdk.CompositeInput.1.stopCompositeInput"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.CompositeInput plugin:

CompositeInput interface events:

| Event | Description |
| :-------- | :-------- |
| [onDeviceChanged](#event.onDeviceChanged) | Triggered when the composite input device changes |
| [onInputStatusChanged](#event.onInputStatusChanged) | Triggered when the status of the composite input changes |
| [onSignalChanged](#event.onSignalChanged) | Triggered when the status of the composite input signal changes |


<a name="event.onDeviceChanged"></a>
## *onDeviceChanged <sup>event</sup>*

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
    "method": "client.events.1.onDeviceChanged",
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

<a name="event.onInputStatusChanged"></a>
## *onInputStatusChanged <sup>event</sup>*

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

<a name="event.onSignalChanged"></a>
## *onSignalChanged <sup>event</sup>*

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

