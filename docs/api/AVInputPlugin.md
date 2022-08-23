<!-- Generated automatically, DO NOT EDIT! -->
<a name="AVInput_Plugin"></a>
# AVInput Plugin

**Version: 1.0.0**

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

The `AVInput` plugin facilitates interactions with the Parker STB HDMI input. The HDMI input is presented by using a `VideoResource` whose URL starts with `avin:`. For example: `avin://input1`.

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


<a name="contentProtected"></a>
## *contentProtected*

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

<a name="currentVideoMode"></a>
## *currentVideoMode*

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

<a name="numberOfInputs"></a>
## *numberOfInputs*

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

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.AVInput plugin:

AVInput interface events:

| Event | Description |
| :-------- | :-------- |
| [onAVInputActive](#onAVInputActive) | Triggered when an active device is connected to an AVInput port |
| [onAVInputInActive](#onAVInputInActive) | Triggered when an active device is disconnected from an AVInput port or when the device becomes inactive |


<a name="onAVInputActive"></a>
## *onAVInputActive*

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

<a name="onAVInputInActive"></a>
## *onAVInputInActive*

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

