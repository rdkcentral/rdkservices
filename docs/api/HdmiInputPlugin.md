<!-- Generated automatically, DO NOT EDIT! -->
<a name="HdmiInputPlugin"></a>
# HdmiInputPlugin

**Version: [1.0.3](https://github.com/rdkcentral/rdkservices/blob/main/HdmiInput/CHANGELOG.md)**

A org.rdk.HdmiInput plugin for Thunder framework.

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

The `HdmiInput` plugin allows you to control the HDMI Input on a set-top box.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

> This Plugin is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new plugin](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin)

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
| [getEdidVersion](#getEdidVersion) | Returns the EDID version |
| [getHDMISPD](#getHDMISPD) | Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device |
| [getRawHDMISPD](#getRawHDMISPD) | Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device as raw bits |
| [readEDID](#readEDID) | Returns the current EDID value |
| [startHdmiInput](#startHdmiInput) | Activates the specified HDMI Input port as the primary video source |
| [stopHdmiInput](#stopHdmiInput) | Deactivates the HDMI Input port currently selected as the primary video source |
| [setEdidVersion](#setEdidVersion) | Sets an HDMI EDID version |
| [setVideoRectangle](#setVideoRectangle) | Sets an HDMI Input video window |
| [writeEDID](#writeEDID) | Changes a current EDID value |
| [getSupportedGameFeatures](#getSupportedGameFeatures) | Returns the list of supported game features |
| [getHdmiGameFeatureStatus](#getHdmiGameFeatureStatus) | Returns the Game Feature Status |


<a name="getHDMIInputDevices"></a>
## *getHDMIInputDevices*

Returns an array of available HDMI Input ports.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=getinputdevices)

### Events

No Events

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
    "method": "org.rdk.HdmiInput.getHDMIInputDevices"
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

Returns the EDID version.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=getedidversion)

### Events

No Events

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
    "method": "org.rdk.HdmiInput.getEdidVersion",
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

Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device. The SPD infoFrame packet includes vendor name, product description, and source information.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=getspd)

### Events

No Events

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
    "method": "org.rdk.HdmiInput.getHDMISPD",
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

Returns the Source Data Product Descriptor (SPD) infoFrame packet information for the specified HDMI Input device as raw bits.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=getrawspd)

### Events

No Events

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
    "method": "org.rdk.HdmiInput.getRawHDMISPD",
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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=readedid)

### Events

No Events

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
    "method": "org.rdk.HdmiInput.readEDID",
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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=startinput)

### Events

| Event | Description |
| :-------- | :-------- |
| [onInputStatusChanged](#onInputStatusChanged) | Triggers the event when HDMI Input source is activated and Input status changes to started |
| [onSignalChanged](#onSignalChanged) | Triggers the event when HDMI Input signal changes (must be one of the following:noSignal, unstableSignal, notSupportedSignal, stableSignal). |
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
    "method": "org.rdk.HdmiInput.startHdmiInput",
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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=stopinput)

### Events

| Event | Description |
| :-------- | :-------- |
| [onInputStatusChanged](#onInputStatusChanged) | Triggers the event when HDMI Input source is deactivated and Input status changes to `stopped` |
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
    "method": "org.rdk.HdmiInput.stopHdmiInput"
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

Sets an HDMI EDID version.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=setedidversion)

### Events

No Events

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
    "method": "org.rdk.HdmiInput.setEdidVersion",
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
    "method": "org.rdk.HdmiInput.setVideoRectangle",
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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=writeedid)

### Events

No Events

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
    "method": "org.rdk.HdmiInput.writeEDID",
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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=getsupportedgamefeatures)

### Events

No Events

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
    "method": "org.rdk.HdmiInput.getSupportedGameFeatures"
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

Returns the Game Feature Status. For example: ALLM.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=getgamefeaturestatus)

### Events

No Events

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
    "method": "org.rdk.HdmiInput.getHdmiGameFeatureStatus",
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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=ondeviceschanged)

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

Triggered whenever the status changes for an HDMI Input.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=oninputstatuschanged)

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

Triggered whenever the signal status changes for an HDMI Input.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=onsignalchanged)

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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=videostreaminfoupdate)

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

<a name="hdmiGameFeatureStatusUpdate"></a>
## *hdmiGameFeatureStatusUpdate*

Triggered whenever game feature(ALLM) status changes for an HDMI Input.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=gamefeaturestatusupdate)

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
    "method": "client.events.hdmiGameFeatureStatusUpdate",
    "params": {
        "portId": "0",
        "gameFeature": "ALLM",
        "mode": true
    }
}
```
<a name="hdmiContentTypeUpdate"></a>
## *hdmiContentTypeUpdate*

Triggered whenever AVI content type changed for a HDMI Input.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/AVInputPlugin?id=hdmiContentTypeUpdate)
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | integer | An ID of an HDMI Input port as returned by the `getHdmiInputDevices` method |
| params.aviContentType | integer | Content type info of a Hdmi Input of type dsAviContentType_t and the integer values indicates following accordingly 0 - Graphics, 1 - Photo, 2 - Cinema, 3 - Game, 4 - Invalid data|

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
