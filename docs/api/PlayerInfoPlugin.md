<!-- Generated automatically, DO NOT EDIT! -->
<a name="Player_Info_Plugin"></a>
# Player Info Plugin

**Version: [1.1.0](https://github.com/rdkcentral/rdkservices/blob/main/PlayerInfo/CHANGELOG.md)**

A PlayerInfo plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Properties](#Properties)
- [Notifications](#Notifications)

<a name="Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="Description"></a>
# Description

The `PlayerInfo` plugin helps to get system supported audio and video codecs.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *PlayerInfo*) |
| classname | string | Class name: *PlayerInfo* |
| locator | string | Library name: *libWPEPlayerInfo.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the PlayerInfo plugin:

PlayerInfo interface methods:

| Method | Description |
| :-------- | :-------- |
| [audiocodecs](#audiocodecs) | Returns the audio codec supported by the platform |
| [videocodecs](#videocodecs) | Returns the video codec supported by the platform |


<a name="audiocodecs"></a>
## *audiocodecs*

Returns the audio codec supported by the platform.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | array |  |
| result[#] | string |  (must be one of the following: *AudioUndefined*, *AudioAac*, *AudioAc3*, *AudioAc3Plus*, *AudioDts*, *AudioMpeg1*, *AudioMpeg2*, *AudioMpeg3*, *AudioMpeg4*, *AudioOpus*, *AudioVorbisOgg*, *AudioWav*) |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "PlayerInfo.audiocodecs"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        "AudioUndefined"
    ]
}
```

<a name="videocodecs"></a>
## *videocodecs*

Returns the video codec supported by the platform.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | array |  |
| result[#] | string |  (must be one of the following: *VideoUndefined*, *VideoH263*, *VideoH264*, *VideoH265*, *VideoH26510*, *VideoMpeg*, *VideoVp8*, *VideoVp9*, *VideoVp10*) |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "PlayerInfo.videocodecs"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        "VideoUndefined"
    ]
}
```

<a name="Properties"></a>
# Properties

The following properties are provided by the PlayerInfo plugin:

PlayerInfo interface properties:

| Property | Description |
| :-------- | :-------- |
| [playerinfo](#playerinfo) <sup>RO</sup> | Player general information |
| [resolution](#resolution) <sup>RO</sup> | Current configured video output port resolution |
| [isaudioequivalenceenabled](#isaudioequivalenceenabled) <sup>RO</sup> | Check for Loudness Equivalence in the platform |
| [dolby atmosmetadata](#dolby_atmosmetadata) <sup>RO</sup> | Atmos capabilities of Sink |
| [dolby soundmode](#dolby_soundmode) <sup>RO</sup> | Current sound mode |
| [dolby enableatmosoutput](#dolby_enableatmosoutput) <sup>WO</sup> | Audio output enablement for Atmos |
| [dolby mode](#dolby_mode) | Dolby mode |


<a name="playerinfo"></a>
## *playerinfo*

Provides access to the player general information.

> This property is **read-only**.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Player general information |
| (property).audio | array |  |
| (property).audio[#] | string | Audio Codec supported by the platform (must be one of the following: *Undefined*, *AAC*, *AC3*, *AC3Plus*, *DTS*, *MPEG1*, *MPEG2*, *MPEG3*, *MPEG4*, *OPUS*, *VorbisOGG*, *WAV*) |
| (property).video | array |  |
| (property).video[#] | string | Video Codec supported by the platform (must be one of the following: *Undefined*, *H263*, *H264*, *H265*, *H26510*, *MPEG*, *MPEG2*, *MPEG4*, *VP8*, *VP9*, *VP10*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "PlayerInfo.playerinfo"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "audio": [
            "AudioAAC"
        ],
        "video": [
            "VideoH264"
        ]
    }
}
```

<a name="resolution"></a>
## *resolution*

Provides access to the current configured video output port resolution.

> This property is **read-only**.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Current configured video output port resolution (must be one of the following: *ResolutionUnknown*, *Resolution480I*, *Resolution480P*, *Resolution576I*, *Resolution576P*, *Resolution720P*, *Resolution1080I*, *Resolution1080P*, *Resolution2160P30*, *Resolution2160P60*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "PlayerInfo.resolution"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "ResolutionUnknown"
}
```

<a name="isaudioequivalenceenabled"></a>
## *isaudioequivalenceenabled*

Provides access to the check for Loudness Equivalence in the platform.

> This property is **read-only**.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | boolean | Check for Loudness Equivalence in the platform |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "PlayerInfo.isaudioequivalenceenabled"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": false
}
```

<a name="dolby_atmosmetadata"></a>
## *dolby_atmosmetadata*

Provides access to the atmos capabilities of Sink.

> This property is **read-only**.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | boolean | Atmos capabilities of Sink |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "PlayerInfo.dolby_atmosmetadata"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": false
}
```

<a name="dolby_soundmode"></a>
## *dolby_soundmode*

Provides access to the current sound mode.

> This property is **read-only**.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Current sound mode (must be one of the following: *Unknown*, *Mono*, *Stereo*, *Surround*, *Passthru*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "PlayerInfo.dolby_soundmode"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "Unknown"
}
```

<a name="dolby_enableatmosoutput"></a>
## *dolby_enableatmosoutput*

Provides access to the audio output enablement for Atmos.

> This property is **write-only**.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | boolean | Audio output enablement for Atmos |

### Example

#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "PlayerInfo.dolby_enableatmosoutput",
    "params": false
}
```

#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="dolby_mode"></a>
## *dolby_mode*

Provides access to the dolby mode.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Dolby mode (must be one of the following: *DigitalPcm*, *DigitalPlus*, *DigitalAc3*, *Auto*, *Ms12*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "PlayerInfo.dolby_mode"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "DigitalPcm"
}
```

#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "PlayerInfo.dolby_mode",
    "params": "DigitalPcm"
}
```

#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the PlayerInfo plugin:

PlayerInfo interface events:

| Event | Description |
| :-------- | :-------- |
| [dolby audiomodechanged](#dolby_audiomodechanged) | Triggered after the audio sound mode changes |


<a name="dolby_audiomodechanged"></a>
## *dolby_audiomodechanged*

Triggered after the audio sound mode changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.mode | string | The sound mode (must be one of the following: *Unknown*, *Mono*, *Stereo*, *Surround*, *Passthru*) |
| params.enable | boolean |  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.dolby_audiomodechanged",
    "params": {
        "mode": "Unknown",
        "enable": true
    }
}
```

