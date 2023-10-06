<!-- Generated automatically, DO NOT EDIT! -->
<a name="AVInput_Plugin"></a>
# AVInput Plugin

**Version: [1.4.1](https://github.com/rdkcentral/rdkservices/blob/main/AVInput/CHANGELOG.md)**

A org.rdk.AVInput plugin for Thunder framework.

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

The `AVInput` plugin allows you to control the Hdmi and Composite input source on a device.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.AVInput*) |
| classname | string | Class name: *org.rdk.AVInput* |
| locator | string | Library name: *libWPEFrameworkAVInput.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.AVInput plugin:

AVInput interface methods:

| Method | Description |
| :-------- | :-------- |
| [contentProtected](#contentProtected) | Returns `true` if the content coming in the HDMI input is protected; otherwise, it returns `false` |
| [currentVideoMode](#currentVideoMode) | Returns a string encoding the video mode being supplied by the device currently attached to the HDMI input |
| [numberOfInputs](#numberOfInputs) | Returns an integer that specifies the number of available inputs |
| [getInputDevices](#getInputDevices) | Returns an array of available HDMI/Composite Input ports |
| [getEdidVersion](#getEdidVersion) | Returns the EDID version |
| [getSPD](#getSPD) | Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device |
| [getRawSPD](#getRawSPD) | Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device as raw bits |
| [readEDID](#readEDID) | Returns the current EDID value |
| [startInput](#startInput) | Activates the specified HDMI/Composite Input port as the primary video source |
| [stopInput](#stopInput) | Deactivates the HDMI/Composite Input port currently selected as the primary video source |
| [setEdidVersion](#setEdidVersion) | Sets an HDMI EDID version |
| [setVideoRectangle](#setVideoRectangle) | Sets an HDMI/Composite Input video window |
| [writeEDID](#writeEDID) | Changes a current EDID value |
| [getSupportedGameFeatures](#getSupportedGameFeatures) | Returns the list of supported game features |
| [getGameFeatureStatus](#getGameFeatureStatus) | Returns the Game Feature Status |


<a name="contentProtected"></a>
## *contentProtected*

Returns `true` if the content coming in the HDMI input is protected; otherwise, it returns `false`. If the content is protected, then it is only presented if the component and composite outputs of the box are disabled.

### Events

No Events

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
    "method": "org.rdk.AVInput.contentProtected",
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

<a name="currentVideoMode"></a>
## *currentVideoMode*

Returns a string encoding the video mode being supplied by the device currently attached to the HDMI input. The format of the string is the same format used for the `resolutionName` parameter of the XRE `setResolution` messages. HDMI input is presentable if its resolution is less than or equal to the current Parker display resolution.

### Events

No Events

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
    "method": "org.rdk.AVInput.currentVideoMode",
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

<a name="numberOfInputs"></a>
## *numberOfInputs*

Returns an integer that specifies the number of available inputs. For example, a value of `2` indicates that there are two available inputs that can be selected using `avin://input0` and `avin://input1`.

### Events

No Events

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
    "method": "org.rdk.AVInput.numberOfInputs",
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

<a name="getInputDevices"></a>
## *getInputDevices*

Returns an array of available HDMI/Composite Input ports.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.typeOfInput | string | The type of Input - HDMI/COMPOSITE |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.devices | array | An object [] that describes each HDMI/Composite Input port |
| result.devices[#] | object |  |
| result.devices[#].id | number | The port identifier for the HDMI/Composite Input |
| result.devices[#].locator | string | A URL corresponding to the HDMI/Composite Input port |
| result.devices[#].connected | boolean | Whether a device is currently connected to this HDMI/Composite Input port |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 30 | ```ERROR_BAD_REQUEST``` | Type of Input is invalid |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.getInputDevices",
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

<a name="getEdidVersion"></a>
## *getEdidVersion*

Returns the EDID version.

### Events

No Events

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

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |
| 30 | ```ERROR_BAD_REQUEST``` | portId is invalid |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.getEdidVersion",
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
        "edidVersion": "HDMI2.0"
    }
}
```

<a name="getSPD"></a>
## *getSPD*

Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device. The SPD infoFrame packet includes vendor name, product description, and source information.

### Events

No Events

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

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |
| 30 | ```ERROR_BAD_REQUEST``` | portId is invalid |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.getSPD",
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
        "HDMISPD": "..."
    }
}
```

<a name="getRawSPD"></a>
## *getRawSPD*

Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device as raw bits.

### Events

No Events

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

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |
| 30 | ```ERROR_BAD_REQUEST``` | portId is invalid |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.getRawSPD",
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
        "HDMISPD": "..."
    }
}
```

<a name="readEDID"></a>
## *readEDID*

Returns the current EDID value.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.portId | string | An ID of an HDMI/Composite Input port as returned by the `getInputDevices` method |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.EDID | string | The EDID Value |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |
| 30 | ```ERROR_BAD_REQUEST``` | portId is invalid |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.readEDID",
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
        "EDID": "..."
    }
}
```

<a name="startInput"></a>
## *startInput*

Activates the specified HDMI/Composite Input port as the primary video source.

### Events

| Event | Description |
| :-------- | :-------- |
| [onInputStatusChanged](#onInputStatusChanged) | Triggers the event when HDMI/Composite Input source is activated and Input status changes to `started` |
| [onSignalChanged](#onSignalChanged) | Triggers the event when HDMI/Composite Input signal changes (must be one of the following:noSignal, unstableSignal, notSupportedSignal, stableSignal) |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.portId | string | <sup>*(optional)*</sup> An ID of an HDMI/Composite Input port as returned by the `getInputDevices` method |
| params.typeOfInput | string | The type of Input - HDMI/COMPOSITE |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |
| 30 | ```ERROR_BAD_REQUEST``` | portId/Type of Input is invalid |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.startInput",
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
    "result": null
}
```

<a name="stopInput"></a>
## *stopInput*

Deactivates the HDMI/Composite Input port currently selected as the primary video source.

### Events

| Event | Description |
| :-------- | :-------- |
| [onInputStatusChanged](#onInputStatusChanged) | Triggers the event when HDMI/Composite Input source is deactivated and Input status changes to `stopped` |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.typeOfInput | string | The type of Input - HDMI/COMPOSITE |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |
| 30 | ```ERROR_BAD_REQUEST``` | Type of Input is invalid |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.stopInput",
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
    "result": null
}
```

<a name="setEdidVersion"></a>
## *setEdidVersion*

Sets an HDMI EDID version.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.portId | string | An ID of an HDMI/Composite Input port as returned by the `getInputDevices` method |
| params.edidVersion | string | The EDID version |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |
| 30 | ```ERROR_BAD_REQUEST``` | portId/edidVersion is invalid |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.setEdidVersion",
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
    "result": null
}
```

<a name="setVideoRectangle"></a>
## *setVideoRectangle*

Sets an HDMI/Composite Input video window.

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
| params.typeOfInput | string | The type of Input - HDMI/COMPOSITE |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |
| 30 | ```ERROR_BAD_REQUEST``` | Coordinates/Type of Input is invalid |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.setVideoRectangle",
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
    "result": null
}
```

<a name="writeEDID"></a>
## *writeEDID*

Changes a current EDID value.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.portId | string | An ID of an HDMI/Composite Input port as returned by the `getInputDevices` method |
| params.message | string | A new EDID value |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 30 | ```ERROR_BAD_REQUEST``` | Coordinates/Type of Input is invalid |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.writeEDID",
    "params": {
        "portId": "0",
        "message": "EDID"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": null
}
```

<a name="getSupportedGameFeatures"></a>
## *getSupportedGameFeatures*

Returns the list of supported game features.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedGameFeatures | array | The supported game Features |
| result.supportedGameFeatures[#] | string |  |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.getSupportedGameFeatures"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedGameFeatures": [
            "ALLM"
        ]
    }
}
```

<a name="getGameFeatureStatus"></a>
## *getGameFeatureStatus*

Returns the Game Feature Status. For example: ALLM.

### Events

No Events

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

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 30 | ```ERROR_BAD_REQUEST``` | Coordinates/Type of Input is invalid |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.AVInput.getGameFeatureStatus",
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
        "mode": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.AVInput plugin:

AVInput interface events:

| Event | Description |
| :-------- | :-------- |
| [onDevicesChanged](#onDevicesChanged) | Triggered whenever a new HDMI/Composite device is connected to an HDMI/Composite Input |
| [onInputStatusChanged](#onInputStatusChanged) | Triggered whenever the status changes for an HDMI/Composite Input |
| [onSignalChanged](#onSignalChanged) | Triggered whenever the signal status changes for an HDMI/Composite Input |
| [videoStreamInfoUpdate](#videoStreamInfoUpdate) | Triggered whenever there is an update in HDMI Input video stream info |
| [gameFeatureStatusUpdate](#gameFeatureStatusUpdate) | Triggered whenever game feature(ALLM) status changes for an HDMI Input |
| [hdmiContentTypeUpdate](#hdmiContentTypeUpdate) | Triggered whenever AV Infoframe content type changes for an HDMI Input |


<a name="onDevicesChanged"></a>
## *onDevicesChanged*

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
    "method": "client.events.onDevicesChanged",
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
    "method": "client.events.onInputStatusChanged",
    "params": {
        "id": 0,
        "locator": "hdmiin://localhost/deviceid/0",
        "status": "started"
    }
}
```

<a name="onSignalChanged"></a>
## *onSignalChanged*

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
    "method": "client.events.onSignalChanged",
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
    "method": "client.events.videoStreamInfoUpdate",
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

<a name="gameFeatureStatusUpdate"></a>
## *gameFeatureStatusUpdate*

Triggered whenever game feature(ALLM) status changes for an HDMI Input.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | number | The port identifier for the HDMI/Composite Input |
| params.gameFeature | string | Game Feature to which current status requested |
| params.mode | boolean | The current game feature status. Mode is required only for ALLM. Need to add support for future game features |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.gameFeatureStatusUpdate",
    "params": {
        "id": 0,
        "gameFeature": "ALLM",
        "mode": true
    }
}
```

<a name="hdmiContentTypeUpdate"></a>
## *hdmiContentTypeUpdate*

Triggered whenever AV Infoframe content type changes for an HDMI Input.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=hdmiContentTypeUpdate)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | integer | Hdmi Input port ID for which content type change event received and possible values are port id 0, 1 and 2 for three Hdmi Input ports |
| params.aviContentType | integer | new Content type received for the active hdmi input port and the possible integer values indicates following accordingly 0 - Graphics, 1 - Photo, 2 - Cinema, 3 - Game, 4 - Invalid data |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.hdmiContentTypeUpdate",
    "params": {
        "id": 1,
        "aviContentType": 1
    }
}
```

