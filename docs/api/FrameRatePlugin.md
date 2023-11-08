<!-- Generated automatically, DO NOT EDIT! -->
<a name="FrameRate_Plugin"></a>
# FrameRate Plugin

**Version: [1.0.5](https://github.com/rdkcentral/rdkservices/blob/main/FrameRate/CHANGELOG.md)**

A org.rdk.FrameRate plugin for Thunder framework.

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

The `FrameRate` plugin allows you to collect FPS data.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.FrameRate*) |
| classname | string | Class name: *org.rdk.FrameRate* |
| locator | string | Library name: *libWPEFrameworkFrameRate.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.FrameRate plugin:

FrameRate interface methods:

| Method | Description |
| :-------- | :-------- |
| [getDisplayFrameRate](#getDisplayFrameRate) | Returns the current display frame rate values |
| [getFrmMode](#getFrmMode) | Returns the current auto framerate mode |
| [setCollectionFrequency](#setCollectionFrequency) | Sets the FPS data collection interval |
| [setDisplayFrameRate](#setDisplayFrameRate) | Sets the display framerate values |
| [setFrmMode](#setFrmMode) | Sets the auto framerate mode |
| [startFpsCollection](#startFpsCollection) | Starts the FPS data collection |
| [stopFpsCollection](#stopFpsCollection) | Stops the FPS data collection |
| [updateFps](#updateFps) | Updates Fps values |


<a name="getDisplayFrameRate"></a>
## *getDisplayFrameRate*

Returns the current display frame rate values.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.framerate | string | The display framerate setting (width x height x framerate) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.FrameRate.getDisplayFrameRate"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "framerate": "3840x2160px48",
        "success": true
    }
}
```

<a name="getFrmMode"></a>
## *getFrmMode*

Returns the current auto framerate mode.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.auto-frm-mode | integer | `0` for auto framerate mode disabled, `1` for auto framerate mode enabled (must be one of the following: *0*, *1*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.FrameRate.getFrmMode"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "auto-frm-mode": 0,
        "success": true
    }
}
```

<a name="setCollectionFrequency"></a>
## *setCollectionFrequency*

Sets the FPS data collection interval.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.frequency | integer | The amount of time in milliseconds. The default frequency is 10000 milliseconds |

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
    "method": "org.rdk.FrameRate.setCollectionFrequency",
    "params": {
        "frequency": 1000
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

<a name="setDisplayFrameRate"></a>
## *setDisplayFrameRate*

Sets the display framerate values.

### Events

| Event | Description |
| :-------- | :-------- |
| [onDisplayFrameRateChanging](#onDisplayFrameRateChanging) | Triggered when the framerate changes started. |
| [onDisplayFrameRateChanged](#onDisplayFrameRateChanged) | Triggered when the framerate changed |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.framerate | string | The display framerate setting (width x height x framerate) |

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
    "method": "org.rdk.FrameRate.setDisplayFrameRate",
    "params": {
        "framerate": "3840x2160px48"
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

<a name="setFrmMode"></a>
## *setFrmMode*

Sets the auto framerate mode.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.frmmode | integer | `0` for auto framerate mode disabled, `1` for auto framerate mode enabled (must be one of the following: *0*, *1*) |

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
    "method": "org.rdk.FrameRate.setFrmMode",
    "params": {
        "frmmode": 0
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

<a name="startFpsCollection"></a>
## *startFpsCollection*

Starts the FPS data collection.

### Events

| Event | Description |
| :-------- | :-------- |
| [onFpsEvent](#onFpsEvent) | Triggered at the end of each interval as defined by the setCollectionFrequency |
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
    "method": "org.rdk.FrameRate.startFpsCollection"
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

<a name="stopFpsCollection"></a>
## *stopFpsCollection*

Stops the FPS data collection.

### Events

| Event | Description |
| :-------- | :-------- |
| [onFpsEvent](#onFpsEvent) | Triggered once after the stopFpsCollection method is invoked. |
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
    "method": "org.rdk.FrameRate.stopFpsCollection"
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

<a name="updateFps"></a>
## *updateFps*

Updates Fps values.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.newFpsValue | integer | New Frames per Second (Fps) value |

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
    "method": "org.rdk.FrameRate.updateFps",
    "params": {
        "newFpsValue": 60
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

The following events are provided by the org.rdk.FrameRate plugin:

FrameRate interface events:

| Event | Description |
| :-------- | :-------- |
| [onDisplayFrameRateChanging](#onDisplayFrameRateChanging) | Triggered when the framerate changes started |
| [onDisplayFrameRateChanged](#onDisplayFrameRateChanged) | Triggered when the framerate changed |
| [onFpsEvent](#onFpsEvent) | Triggered at the end of each interval as defined by the `setCollectionFrequency` method and once after the `stopFpsCollection` method is invoked |


<a name="onDisplayFrameRateChanging"></a>
## *onDisplayFrameRateChanging*

Triggered when the framerate changes started.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.displayFrameRate | string | Video Display FrameRate changing |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onDisplayFrameRateChanging",
    "params": {
        "displayFrameRate": "1920x1080x60"
    }
}
```

<a name="onDisplayFrameRateChanged"></a>
## *onDisplayFrameRateChanged*

Triggered when the framerate changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.displayFrameRate | string | Video Display FrameRate changed |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onDisplayFrameRateChanged",
    "params": {
        "displayFrameRate": "1920x1080x60"
    }
}
```

<a name="onFpsEvent"></a>
## *onFpsEvent*

Triggered at the end of each interval as defined by the `setCollectionFrequency` method and once after the `stopFpsCollection` method is invoked.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.average | integer | The average FPS |
| params.min | integer | The minimum FPS |
| params.max | integer | The maximum FPS |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onFpsEvent",
    "params": {
        "average": 0,
        "min": 0,
        "max": 0
    }
}
```

