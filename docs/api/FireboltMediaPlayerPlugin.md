<!-- Generated automatically, DO NOT EDIT! -->
<a name="FireboltMediaPlayerPlugin"></a>
# FireboltMediaPlayerPlugin

**Version: 1.0.0**

A org.rdk.FireboltMediaPlayer plugin for Thunder framework.

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

The `FireboltMediaPlayer` plugin allows you to control an AAMP media player for media stream playback. 

**Summary**
* The service is a wrapper for AAMP IP Video (DASH, HLS, mp4, etc.) playback intended for the RDK community as an alternative to native or UVE-JS integration. 
* The service starts an AAMP dedicated process with the environment variables `AAMP_ENABLE_OPT_OVERRIDE=1` (enables `/opt/aamp.cfg`) and `AAMP_ENABLE_WESTEROS_SINK=1`.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.FireboltMediaPlayer*) |
| classname | string | Class name: *org.rdk.FireboltMediaPlayer* |
| locator | string | Library name: *libWPEFrameworkFireboltMediaPlayer.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.FireboltMediaPlayer plugin:

FireboltMediaPlayer interface methods:

| Method | Description |
| :-------- | :-------- |
| [create](#create) | Initiates a new AAMP player instance suitable for playback of IP feeds |
| [initConfig](#initConfig) | Modifies the default AAMP configuration |
| [load](#load) | Associates a specified URL with a player instance |
| [pause](#pause) | Pauses streaming content that is associated with the specified player instance |
| [play](#play) | Begins or resumes streaming content that is associated with the specified player instance |
| [release](#release) | Decreases the ref-count of the player |
| [seek](#seek) | Moves the media to a specific position |
| [setDRMConfig](#setDRMConfig) | Modifies the default configuration of the DRM used by AAMP |
| [stop](#stop) | Stops streaming content |


<a name="create"></a>
## *create*

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

<a name="initConfig"></a>
## *initConfig*

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

<a name="load"></a>
## *load*

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

<a name="pause"></a>
## *pause*

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

<a name="play"></a>
## *play*

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

<a name="release"></a>
## *release*

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

<a name="seek"></a>
## *seek*

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

<a name="setDRMConfig"></a>
## *setDRMConfig*

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

<a name="stop"></a>
## *stop*

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

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.FireboltMediaPlayer plugin:

FireboltMediaPlayer interface events:

| Event | Description |
| :-------- | :-------- |
| [onMediaStreamEvent](#onMediaStreamEvent) | Triggered when the media stream starts |


<a name="onMediaStreamEvent"></a>
## *onMediaStreamEvent*

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

