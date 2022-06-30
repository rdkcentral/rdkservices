<!-- Generated automatically, DO NOT EDIT! -->
<a name="SystemAudioPlayer_Plugin"></a>
# SystemAudioPlayer Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.SystemAudioPlayer plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.SystemAudioPlayer plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `SystemAudioPlayer` plugin provides system audio playback functionality for client applications. It supports various audio types (viz., pcm, mp3, wav) and can play them from various sources (viz., websocket, httpsrc, filesrc, data buffer).  

**Note**: MP3 playback development remains a work-in-progress.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.SystemAudioPlayer*) |
| classname | string | Class name: *org.rdk.SystemAudioPlayer* |
| locator | string | Library name: *libWPEFrameworkSystemAudioPlayer.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.SystemAudioPlayer plugin:

SystemAudioPlayer interface methods:

| Method | Description |
| :-------- | :-------- |
| [close](#close) | Closes the system audio player with the specified ID |
| [config](#config) | Configures playback for a PCM audio source (audio/x-raw) on the specified player |
| [getPlayerSessionId](#getPlayerSessionId) | Gets the session ID from the provided the URL |
| [isspeaking](#isspeaking) | Checks if playback is in progress |
| [open](#open) | Opens a player instance and assigns it a unique ID |
| [pause](#pause) | Pauses playback on the specified player |
| [play](#play) | Plays audio on the specified player |
| [playbuffer](#playbuffer) | Buffers the audio playback on the specified player |
| [resume](#resume) | Resumes playback on the specified player |
| [setMixerLevels](#setMixerLevels) | Sets the audio level on the specified player |
| [setSmartVolControl](#setSmartVolControl) | Sets the smart volume audio control on the specified player |
| [stop](#stop) | Stops playback on the specified player |


<a name="close"></a>
## *close*

Closes the system audio player with the specified ID. The `SystemAudioPlayer` plugin destroys the player object. That is, if the player is playing, then it is stopped and closed. All volume mixer level settings are restored. 

 Also See: [open](#open).
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | integer | A unique identifier for a player instance |

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
    "method": "org.rdk.SystemAudioPlayer.1.close",
    "params": {
        "id": 1
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

<a name="config"></a>
## *config*

Configures playback for a PCM audio source (audio/x-raw) on the specified player. This method must be called before the [play](#play)  There may be more optional configuration parameters added in the future for PCM as well as for other audio types. Supported audio/x-raw configuration parameters can be found at https://gstreamer.freedesktop.org/documentation/rawparse/rawaudioparse.html#src.
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | integer | A unique identifier for a player instance |
| params.pcmconfig | object | PCM configuration properties |
| params.pcmconfig?.format | string | <sup>*(optional)*</sup>  |
| params.pcmconfig?.rate | string | <sup>*(optional)*</sup>  |
| params.pcmconfig?.channels | string | <sup>*(optional)*</sup>  |
| params.pcmconfig?.layout | string | <sup>*(optional)*</sup>  |
| params?.websocketsecparam | object | <sup>*(optional)*</sup> Parameters that are needed to establish a secured websocket connection |
| params?.websocketsecparam?.cafilenames | array | <sup>*(optional)*</sup> A list of Certificate Authorities file names. If empty, code will try to load CAs from default system path for wss connection |
| params?.websocketsecparam?.cafilenames[#] | object | <sup>*(optional)*</sup>  |
| params?.websocketsecparam?.cafilenames[#]?.cafilename | string | <sup>*(optional)*</sup> A certificate file name in pem format |
| params?.websocketsecparam?.certfilename | string | <sup>*(optional)*</sup> Full file name of cert file in pem format. If a file name is not provided, then the other end of the communication needs to be configured to not validate a client certificate |
| params?.websocketsecparam?.keyfilename | string | <sup>*(optional)*</sup> Full file name of key file in pem format. A key file name must be provided if the certificate file name is provided |

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
    "method": "org.rdk.SystemAudioPlayer.1.config",
    "params": {
        "id": 1,
        "pcmconfig": {
            "format": "S16LE",
            "rate": "22050",
            "channels": "1",
            "layout": "interleaved"
        },
        "websocketsecparam": {
            "cafilenames": [
                {
                    "cafilename": "/etc/ssl/certs/Xfinity_Subscriber_ECC_Root.pem"
                }
            ],
            "certfilename": "...",
            "keyfilename": "..."
        }
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

<a name="getPlayerSessionId"></a>
## *getPlayerSessionId*

Gets the session ID from the provided the URL. The session is the ID returned in open cal. 
 
### Events 

 No Events .

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.url | string | The URL for returning the session ID |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.sessionId | integer | A unique identifier for a player instance |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.SystemAudioPlayer.1.getPlayerSessionId",
    "params": {
        "url": "http://localhost:50050/nuanceEve/tts?voice=ava&language=en-US&rate=50&text=SETTINGS"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "sessionId": 1,
        "success": true
    }
}
```

<a name="isspeaking"></a>
## *isspeaking*

Checks if playback is in progress.
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | integer | A unique identifier for a player instance |

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
    "method": "org.rdk.SystemAudioPlayer.1.isspeaking",
    "params": {
        "id": 1
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

<a name="open"></a>
## *open*

Opens a player instance and assigns it a unique ID. The player ID is used to reference the player when calling other methods.  

**Note**: The `SystemAudioPlayer` plugin can have a maximum of 1 system and 1 application play mode player at a time.  

Also See: [close](#close).
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.audiotype | string | The audio type. If the audio type is `pcm`, then the application is expected to also provide the format using the `playmode` parameter. The `playmode` parameter is ignored for the other audio types. (must be one of the following: *pcm*, *mp3*, *wav*) |
| params.sourcetype | string | The source type (must be one of the following: *data*, *httpsrc*, *filesrc*, *websocket*) |
| params.playmode | string | The play mode. The play mode is only set if the `audiotype` parameter is set to `pcm`. (must be one of the following: *system*, *app*) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.id | integer | A unique identifier for a player instance |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.SystemAudioPlayer.1.open",
    "params": {
        "audiotype": "pcm",
        "sourcetype": "websocket",
        "playmode": "system"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "id": 1,
        "success": true
    }
}
```

<a name="pause"></a>
## *pause*

Pauses playback on the specified player. Pause is only supported for HTTP and file source types.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onsapevents:PLAYBACK_PAUSED`| Triggered if the playback paused on the specified player.|.

Also see: [onsapevents](#onsapevents)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | integer | A unique identifier for a player instance |

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
    "method": "org.rdk.SystemAudioPlayer.1.pause",
    "params": {
        "id": 1
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

Plays audio on the specified player.  

**Note**: If a player is using one play mode and another player tries to play audio using the same play mode, then an error returns indicating that the hardware resource has already been acquired by the session and includes the player ID.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onsapevents:PLAYBACK_STARTED`| Triggered if the playback is started to play on the specified player.|
| `onsapevents:PLAYBACK_FINISHED`| Triggered if the playback is finished  normally on the specified player.|.

Also see: [onsapevents](#onsapevents)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | integer | A unique identifier for a player instance |
| params.url | string | The source URL. If no port number is provided for a web socket source, then the player uses `40001` as the default port  |

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
    "method": "org.rdk.SystemAudioPlayer.1.play",
    "params": {
        "id": 1,
        "url": "http://localhost:50050/nuanceEve/tts?voice=ava&language=en-US&rate=50&text=SETTINGS"
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

<a name="playbuffer"></a>
## *playbuffer*

Buffers the audio playback on the specified player.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onsapevents:NEED_DATA`| Triggered if  the buffer needs more data to play|.

Also see: [onsapevents](#onsapevents)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | integer | A unique identifier for a player instance |
| params.data | string | Size of the buffer |

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
    "method": "org.rdk.SystemAudioPlayer.1.playbuffer",
    "params": {
        "id": 1,
        "data": "180"
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

<a name="resume"></a>
## *resume*

Resumes playback on the specified player. Resume is only supported for HTTP and file source types.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onsapevents:PLAYBACK_RESUMED`| Triggered if the playback resumed on the specified player.|.

Also see: [onsapevents](#onsapevents)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | integer | A unique identifier for a player instance |

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
    "method": "org.rdk.SystemAudioPlayer.1.resume",
    "params": {
        "id": 1
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

<a name="setMixerLevels"></a>
## *setMixerLevels*

Sets the audio level on the specified player. The `SystemAudioPlayer` plugin can control the volume of the content being played back as well as the primary program audio; thus, an application can duck down the volume on the primary program audio when system audio is played and then restore it back when the system audio playback is complete.
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | integer | A unique identifier for a player instance |
| params.primaryVolume | string | The primary volume. Valid values are `0` to `100` where `0` is the minimum and `100` is the maximum volume. A value of `0` indicates that the user will not hear any audio during playback |
| params.playerVolume | string | The player volume. Valid values are `0` to `100` where `0` is the minimum and `100` is the maximum volume. A value of `0` indicates that the user will not hear any audio during playback |

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
    "method": "org.rdk.SystemAudioPlayer.1.setMixerLevels",
    "params": {
        "id": 1,
        "primaryVolume": "20",
        "playerVolume": "7"
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

<a name="setSmartVolControl"></a>
## *setSmartVolControl*

Sets the smart volume audio control on the specified player. The plugin can control the smart volume of the content being played back as well as the primary program audio; thus, an application can duck down the volume on the primary program audio when system audio is played and then restore it back when the system audio playback is complete.
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | integer | A unique identifier for a player instance |
| params.enable | boolean | Enables or disables smart volume control |
| params.playerAudioLevelThreshold | number | The minimum audio level threshold in the player source stream above which smart audio mixing detection is triggered. Range: 0 to 1 (in real number) |
| params.playerDetectTimeMs | integer | The duration that the `playerAudioLevelThreshold` value must be detected before primary audio ducking is started. Range: Above 0 (in milliseconds) |
| params.playerHoldTimeMs | integer | The duration that primary audio ducking is enabled after the 'playerAudioLevelThreshold' value is no longer met and before primary audio ducking is stopped. Range: Above 0 (in milliseconds) |
| params.primaryDuckingPercent | integer | The percentage to duck the primary audio volume. If `setMixerLevels` has been called, then this percentage is scaled to the `params.primVolume` range. Range: 0 to 100 (in percentage)  |

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
    "method": "org.rdk.SystemAudioPlayer.1.setSmartVolControl",
    "params": {
        "id": 1,
        "enable": true,
        "playerAudioLevelThreshold": 0.1,
        "playerDetectTimeMs": 200,
        "playerHoldTimeMs": 1000,
        "primaryDuckingPercent": 1
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

Stops playback on the specified player.
 
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | integer | A unique identifier for a player instance |

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
    "method": "org.rdk.SystemAudioPlayer.1.stop",
    "params": {
        "id": 1
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

The following events are provided by the org.rdk.SystemAudioPlayer plugin:

SystemAudioPlayer interface events:

| Event | Description |
| :-------- | :-------- |
| [onsapevents](#onsapevents) | Triggered during playback for each player |


<a name="onsapevents"></a>
## *onsapevents*

Triggered during playback for each player. Events from each player are broadcast to all registered clients. The client is responsible for checking the player `id` attribute and discarding events for unwanted players. 

### Notifications  

The following events are supported.  
| Event Name | Description |  
| :-------- | :-------- |  
| PLAYBACK_STARTED| Triggered when playback starts  |  
| PLAYBACK_FINISHED | Triggered when playback finishes normally. **Note**: Web socket playback is continuous and does not receive the `PLAYBACK_FINISHED` event until the stream contains `EOS`. |  
| PLAYBACK_PAUSED| Triggered when playback is paused | 
 |PLAYBACK_RESUMED | Triggered when playback resumes |  
| NETWORK_ERROR | Triggered when a playback network error occurs (httpsrc/web socket) |  
| PLAYBACK_ERROR| Triggered when any other playback error occurs (internal issue)|  
| NEED_DATA|  Triggered when the buffer needs more data to play|.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id | integer | A unique identifier for a player instance |
| params.event | string | A playback event |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onsapevents",
    "params": {
        "id": 1,
        "event": "PLAYBACK_STARTED"
    }
}
```

