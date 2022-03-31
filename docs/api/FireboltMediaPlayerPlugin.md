<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.FireboltMediaPlayerPlugin"></a>
# FireboltMediaPlayerPlugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.FireboltMediaPlayer plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.FireboltMediaPlayer plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `FireboltMediaPlayer` plugin allows you to control an AAMP media player for media stream playback. 

**Summary**
* The service is a wrapper for AAMP IP Video (DASH, HLS, mp4, etc.) playback intended for the RDK community as an alternative to native or UVE-JS integration. 
* The service starts an AAMP dedicated process with the environment variables `AAMP_ENABLE_OPT_OVERRIDE=1` (enables `/opt/aamp.cfg`) and `AAMP_ENABLE_WESTEROS_SINK=1`.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.FireboltMediaPlayer*) |
| classname | string | Class name: *org.rdk.FireboltMediaPlayer* |
| locator | string | Library name: *libWPEFrameworkFireboltMediaPlayer.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.FireboltMediaPlayer plugin:

FireboltMediaPlayer interface methods:

| Method | Description |
| :-------- | :-------- |
| [create](#method.create) | Initiates a new AAMP player instance suitable for playback of IP feeds |
| [initConfig](#method.initConfig) | Modifies the default AAMP configuration |
| [load](#method.load) | Associates a specified URL with a player instance |
| [pause](#method.pause) | Pauses streaming content that is associated with the specified player instance |
| [play](#method.play) | Begins or resumes streaming content that is associated with the specified player instance |
| [release](#method.release) | Decreases the ref-count of the player |
| [seek](#method.seek) | Moves the media to a specific position |
| [setDRMConfig](#method.setDRMConfig) | Modifies the default configuration of the DRM used by AAMP |
| [stop](#method.stop) | Stops streaming content |


<a name="method.create"></a>
## *create [<sup>method</sup>](#head.Methods)*

Initiates a new AAMP player instance suitable for playback of IP feeds. If a player that is identified by the specified ID already exists, then it will be ref-counted.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | string | The player ID |

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
    "method": "org.rdk.FireboltMediaPlayer.1.create",
    "params": {
        "id": "MainPlayer"
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

<a name="method.initConfig"></a>
## *initConfig [<sup>method</sup>](#head.Methods)*

Modifies the default AAMP configuration.  

For complete list of configuration properties, see:https://wiki.rdkcentral.com/display/RDK/AAMP+UVE+-+API#AAMPUVEAPI-Configuration.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.initialBitrate | integer | <sup>*(optional)*</sup>  |

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
    "method": "org.rdk.FireboltMediaPlayer.1.initConfig",
    "params": {
        "initialBitrate": 3000000
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

<a name="method.load"></a>
## *load [<sup>method</sup>](#head.Methods)*

Associates a specified URL with a player instance.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | string | The player ID |
| params.url | string | The address of the stream |
| params?.autoplay | boolean | <sup>*(optional)*</sup> Instructs the player to start playing immediately |

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
    "method": "org.rdk.FireboltMediaPlayer.1.load",
    "params": {
        "id": "MainPlayer",
        "url": "https://cpetestutility.stb.r53.xcal.tv/VideoTestStream/main.m3u8",
        "autoplay": true
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

<a name="method.pause"></a>
## *pause [<sup>method</sup>](#head.Methods)*

Pauses streaming content that is associated with the specified player instance. The play speed is set to 0.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | string | The player ID |

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
    "method": "org.rdk.FireboltMediaPlayer.1.pause",
    "params": {
        "id": "MainPlayer"
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

<a name="method.play"></a>
## *play [<sup>method</sup>](#head.Methods)*

Begins or resumes streaming content that is associated with the specified player instance. The play speed is set to 1.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | string | The player ID |

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
    "method": "org.rdk.FireboltMediaPlayer.1.play",
    "params": {
        "id": "MainPlayer"
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

<a name="method.release"></a>
## *release [<sup>method</sup>](#head.Methods)*

Decreases the ref-count of the player. The player gets destroyed when the ref-count reaches 0.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | string | The player ID |

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
    "method": "org.rdk.FireboltMediaPlayer.1.release",
    "params": {
        "id": "MainPlayer"
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

<a name="method.seek"></a>
## *seek [<sup>method</sup>](#head.Methods)*

Moves the media to a specific position.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | string | The player ID |
| params.positionSec | integer | The position in seconds. For VOD streams, the value is relative to the beginning of media. For Live streams, the value is either relative to the position of the first tune or eldest part of the live window if culling has since occurred. A value of `-1` indicates the position of the current live point |

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
    "method": "org.rdk.FireboltMediaPlayer.1.seek",
    "params": {
        "id": "MainPlayer",
        "positionSec": 30
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

<a name="method.setDRMConfig"></a>
## *setDRMConfig [<sup>method</sup>](#head.Methods)*

Modifies the default configuration of the DRM used by AAMP.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.id | string | <sup>*(optional)*</sup> The player ID |
| params?.com.microsoft.playready | string | <sup>*(optional)*</sup> The license server endpoint to use with PlayReady DRM |
| params?.com.widevine.alpha | string | <sup>*(optional)*</sup> The license server endpoint to use with Widevine DRM |
| params?.preferredKeysystem | string | <sup>*(optional)*</sup> The preferred DRM when a manifest advertises multiple supported DRM systems |

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
    "method": "org.rdk.FireboltMediaPlayer.1.setDRMConfig",
    "params": {
        "id": "MainPlayer",
        "com.microsoft.playready": "http://test.playready.microsoft.com/service/rightsmanager.asmx",
        "com.widevine.alpha": "http://test.playready.microsoft.com/service/rightsmanager.asmx",
        "preferredKeysystem": "com.microsoft.playready"
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

<a name="method.stop"></a>
## *stop [<sup>method</sup>](#head.Methods)*

Stops streaming content.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | string | The player ID |

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
    "method": "org.rdk.FireboltMediaPlayer.1.stop",
    "params": {
        "id": "MainPlayer"
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

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.FireboltMediaPlayer plugin:

FireboltMediaPlayer interface events:

| Event | Description |
| :-------- | :-------- |
| [onMediaStreamEvent](#event.onMediaStreamEvent) | Triggered when the media stream starts |


<a name="event.onMediaStreamEvent"></a>
## *onMediaStreamEvent [<sup>event</sup>](#head.Notifications)*

Triggered when the media stream starts.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | string | The player ID |
| params?.parametersJson | object | <sup>*(optional)*</sup> Media stream properties |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onMediaStreamEvent",
    "params": {
        "id": "MainPlayer",
        "parametersJson": {}
    }
}
```

