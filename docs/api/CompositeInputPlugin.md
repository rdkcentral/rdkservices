<!-- Generated automatically, DO NOT EDIT! -->
<a name="CompositeInput_Plugin"></a>
# CompositeInput Plugin

**Version: [1.0.5](https://github.com/rdkcentral/rdkservices/blob/main/CompositeInput/CHANGELOG.md)**

A org.rdk.CompositeInput plugin for Thunder framework.

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

The `CompositeInput` Plugin allows you to control the composite input source on a device.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

> This Plugin is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new plugin](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin)

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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=getinputdevices)

### Events

No Events

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
    "method": "org.rdk.CompositeInput.getCompositeInputDevices"
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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=setvideorectangle)

### Events

No Events

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
    "method": "org.rdk.CompositeInput.setVideoRectangle",
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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=startinput)

### Events

| Event | Description |
| :-------- | :-------- |
| [onInputStatusChanged](#onInputStatusChanged) | Triggers this event when activates composite input source and input status changes to started |
| [onSignalChanged](#onSignalChanged) | Triggers this event when composite input signal changes (must be one of the following:noSignal, unstableSignal, notSupportedSignal, stableSignal) |
| [videoStreamInfoUpdate](#videoStreamInfoUpdate) | Triggered whenever there is an update in Composite Input video stream info |

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
    "method": "org.rdk.CompositeInput.startCompositeInput",
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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=stopinput)

### Events

| Event | Description |
| :-------- | :-------- |
| [onInputStatusChanged](#onInputStatusChanged) | Triggers this event when deactivates composite input source and input status changes to stopped |
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
    "method": "org.rdk.CompositeInput.stopCompositeInput"
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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=ondeviceschanged)

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
    "method": "client.events.onDevicesChanged",
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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=oninputstatuschanged)

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
    "method": "client.events.onInputStatusChanged",
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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=onsignalchanged)

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
    "method": "client.events.onSignalChanged",
    "params": {
        "id": 0,
        "locator": "cvbsin://localhost/deviceid/0",
        "signalStatus": "noSignal"
    }
}
```
<a name="videoStreamInfoUpdate"></a>
## *videoStreamInfoUpdate*

Triggered whenever there is an update in Composite Input video stream info.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | number | The port identifier for the Composite Input |
| params.locator | string | A URL corresponding to the Composite Input port |
| params.width | integer | Width of the Video Stream |
| params.height | integer | Height of the Video Stream |
| params.progressive | boolean | Whether the streaming video is progressive or not? |
| params.frameRateN | integer | FrameRate Numerator |
| params.frameRateD | integer | FrameRate Denomirator |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.videoStreamInfoUpdate",
    "params": {
        "id": 0,
        "locator": "cvbsin://localhost/deviceid/0",
        "width": 3840,
        "height": 2160,
        "progressive": false,
        "frameRateN": 60000,
        "frameRateD": 1001
    }
}
```

