<!-- Generated automatically, DO NOT EDIT! -->
<a name="MediaEngineRMF_Plugin"></a>
# MediaEngineRMF Plugin

**Version: [1.0.1](https://github.com/rdkcentral/rdkservices/blob/main/MediaEngineRMF/CHANGELOG.md)**

A org.rdk.MediaEngineRMF plugin for Thunder framework.

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

The `MediaEngineRMF` plugin allows you to play QAM video using Thunder.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.MediaEngineRMF*) |
| classname | string | Class name: *org.rdk.MediaEngineRMF* |
| locator | string | Library name: *libWPEFrameworkMediaEngineRMF.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.MediaEngineRMF plugin:

MediaEngineRMF interface methods:

| Method | Description |
| :-------- | :-------- |
| [initialize](#initialize) | Performs one-time initialization necessary to use the specified source_type |
| [deinitialize](#deinitialize) | Performs final clean-up related to specified source_type |
| [play](#play) | Play specified QAM service |
| [stop](#stop) | Stops playback of the current service |
| [mute](#mute) | Mute/unmute audio |
| [setVolume](#setVolume) | Set audio volume |
| [setVideoRectangle](#setVideoRectangle) | Set video rectangle |


<a name="initialize"></a>
## *initialize*

Performs one-time initialization necessary to use the specified source_type.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.source_type | string | The type of source by RMF pipeline |

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
    "method": "org.rdk.MediaEngineRMF.initialize",
    "params": {
        "source_type": "qam"
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

<a name="deinitialize"></a>
## *deinitialize*

Performs final clean-up related to specified source_type.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.source_type | string | The type of source by RMF pipeline |

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
    "method": "org.rdk.MediaEngineRMF.deinitialize",
    "params": {
        "source_type": "qam"
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

<a name="play"></a>
## *play*

Play specified QAM service.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.source_type | string | The type of source by RMF pipeline |
| params.identifier | string | A string that uniquely identifies the QAM/RMF service to play |

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
    "method": "org.rdk.MediaEngineRMF.play",
    "params": {
        "source_type": "qam",
        "identifier": "ocap://0x3f0d"
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

<a name="stop"></a>
## *stop*

Stops playback of the current service.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
