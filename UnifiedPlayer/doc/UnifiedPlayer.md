<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.Unified_Player"></a>
# Unified Player

**Version: 1.0**

**Status: :black_circle::white_circle::white_circle:**

UnifiedPlayer interface for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Methods](#head.Methods)
- [Notifications](#head.Notifications)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the UnifiedPlayer interface. It includes detailed specification about its methods provided and notifications sent.

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

Unified Player for Linear, Multicast and ABR video.

<a name="head.Methods"></a>
# Methods

The following methods are provided by the UnifiedPlayer interface:

UnifiedPlayer interface methods:

| Method | Description |
| :-------- | :-------- |
| [open](#method.open) | Open [and play] media |
| [pretune](#method.pretune) | Pretune a non-HTTP source |
| [park](#method.park) | Park an active non-HTTP source for later use |
| [destroy](#method.destroy) | Destroy a media playback session |
| [setposition](#method.setposition) | Sets playback position |
| [setpositionrelative](#method.setpositionrelative) | Sets playback position relative to current position, aka skip |
| [getposition](#method.getposition) | Gets playback position |
| [setspeed](#method.setspeed) | Sets playback speed |
| [getspeed](#method.getspeed) | Gets playback speed |
| [getduration](#method.getduration) | Gets duration of Closed Assests |
| [getavailablespeeds](#method.getavailablespeeds) | Gets supported playback speeds |
| [setdefaultaudiolanguage](#method.setdefaultaudiolanguage) | Sets default audio language |
| [getdefaultaudiolanguage](#method.getdefaultaudiolanguage) | Gets default audio language |
| [setprimaryaudiolanguage](#method.setprimaryaudiolanguage) | Sets primary, presenting, audio language |
| [getprimaryaudiolanguage](#method.getprimaryaudiolanguage) | Gets primary audio language |
| [setsecondaryaudiolanguage](#method.setsecondaryaudiolanguage) | Sets secondary, audio-description, audio language |
| [getsecondaryaudiolanguage](#method.getsecondaryaudiolanguage) | Gets secondary audio language |
| [getcurrentaudiolanguage](#method.getcurrentaudiolanguage) | Gets the audio language currently presented to the user |
| [getavailableaudiolanguages](#method.getavailableaudiolanguages) | Gets all the audio languages currently in the PMT or Manifest |
| [setdefaultsubtitleslanguage](#method.setdefaultsubtitleslanguage) | Sets default subtitles language |
| [getdefaultsubtitleslanguage](#method.getdefaultsubtitleslanguage) | Gets default subtitles language |
| [setprimarysubtitleslanguage](#method.setprimarysubtitleslanguage) | Sets primary, presenting, subtitles language |
| [getprimarysubtitleslanguage](#method.getprimarysubtitleslanguage) | Gets primary subtitles language |
| [getcurrentsubtitleslanguage](#method.getcurrentsubtitleslanguage) | Gets the subtitles language currently presented to the user |
| [getavailablesubtitleslanguages](#method.getavailablesubtitleslanguages) | Gets all the audio languages currently in the PMT or Manifest |
| [setmute](#method.setmute) | Sets if media compopnents should be muted or not |
| [getmute](#method.getmute) | Gets status of media components muting |
| [setsubtitlesenabled](#method.setsubtitlesenabled) | Sets if subtitles are decoded and presented |
| [getsubtitlesenabled](#method.getsubtitlesenabled) | Gets if subtitles are enabled |
| [setsubtitlesdisplayed](#method.setsubtitlesdisplayed) | Sets if subtitles are presented |
| [getsubtitlesdisplayed](#method.getsubtitlesdisplayed) | Gets if subtitles are presented |
| [setvideorectangle](#method.setvideorectangle) | Sets the size and position of the video |
| [play](#method.play) | Play the Video at x1 speed, HTTP sources only |
| [pause](#method.pause) | Pauses the video, HTTP sources only |
| [seektolive](#method.seektolive) | seeks to the live point, HTTP sources only, non-closed manifests only |
| [stop](#method.stop) | Stops playback and closes the source |
| [requeststatus](#method.requeststatus) | Causes a status event to be triggered |


<a name="method.open"></a>
## *open <sup>method</sup>*

Open [and play] media.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Specifies how to open a media playback or cas management session |
| params?.mediaurl | string | <sup>*(optional)*</sup> The URL to tune to can be tune://, ocap:// http:// https:// |
| params?.mode | string | <sup>*(optional)*</sup> The use of the tune request (must be one of the following: *MODE_NONE*, *MODE_LIVE*, *MODE_RECORD*, *MODE_PLAYBACK*) |
| params?.manage | string | <sup>*(optional)*</sup> The type of CAS management to attach to the tune (must be one of the following: *MANAGE_NONE*, *MANAGE_FULL*, *MANAGE_NO_PSI*, *MANAGE_NO_TUNER*) |
| params?.casocdmid | string | <sup>*(optional)*</sup> The well-known OCDM ID of the CAS to use |
| params?.casinitdata | string | <sup>*(optional)*</sup> CAS specific initdata for the selected media |
| params?.mute | object | <sup>*(optional)*</sup> Sets if presentation should be muted [audio and video] |
| params?.mute?.video | boolean | <sup>*(optional)*</sup>  |
| params?.mute?.audio | boolean | <sup>*(optional)*</sup>  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Session ID Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.sessionid | integer | <sup>*(optional)*</sup> The session ID created |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.open",
    "params": {
        "mediaurl": "tune://tuner?frequency=175000000&modulation=16&pgmno=12",
        "mode": "MODE_LIVE",
        "manage": "MANAGE_NONE",
        "casocdmid": "",
        "casinitdata": "<base64 data>",
        "mute": {
            "video": false,
            "audio": false
        }
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "sessionid": 0,
        "failurereason": 0
    }
}
```

<a name="method.pretune"></a>
## *pretune <sup>method</sup>*

Pretune a non-HTTP source.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Specifies how to open a media playback or cas management session |
| params?.mediaurl | string | <sup>*(optional)*</sup> The URL to tune to can be tune://, ocap:// http:// https:// |
| params?.mode | string | <sup>*(optional)*</sup> The use of the tune request (must be one of the following: *MODE_NONE*, *MODE_LIVE*, *MODE_RECORD*, *MODE_PLAYBACK*) |
| params?.manage | string | <sup>*(optional)*</sup> The type of CAS management to attach to the tune (must be one of the following: *MANAGE_NONE*, *MANAGE_FULL*, *MANAGE_NO_PSI*, *MANAGE_NO_TUNER*) |
| params?.casocdmid | string | <sup>*(optional)*</sup> The well-known OCDM ID of the CAS to use |
| params?.casinitdata | string | <sup>*(optional)*</sup> CAS specific initdata for the selected media |
| params?.mute | object | <sup>*(optional)*</sup> Sets if presentation should be muted [audio and video] |
| params?.mute?.video | boolean | <sup>*(optional)*</sup>  |
| params?.mute?.audio | boolean | <sup>*(optional)*</sup>  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Session ID Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.sessionid | integer | <sup>*(optional)*</sup> The session ID created |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.pretune",
    "params": {
        "mediaurl": "tune://tuner?frequency=175000000&modulation=16&pgmno=12",
        "mode": "MODE_LIVE",
        "manage": "MANAGE_NONE",
        "casocdmid": "",
        "casinitdata": "<base64 data>",
        "mute": {
            "video": false,
            "audio": false
        }
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "sessionid": 0,
        "failurereason": 0
    }
}
```

<a name="method.park"></a>
## *park <sup>method</sup>*

Park an active non-HTTP source for later use.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.park",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.destroy"></a>
## *destroy <sup>method</sup>*

Destroy a media playback session.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.destroy",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.setposition"></a>
## *setposition <sup>method</sup>*

Sets playback position.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | A float value targetted to a session |
| params.value | float | Generic Floating Point [32 bit] value |
| params?.session | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.setposition",
    "params": {
        "value": 0.0,
        "session": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.setpositionrelative"></a>
## *setpositionrelative <sup>method</sup>*

Sets playback position relative to current position, aka skip.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | A float value targetted to a session |
| params.value | float | Generic Floating Point [32 bit] value |
| params?.session | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.setpositionrelative",
    "params": {
        "value": 0.0,
        "session": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.getposition"></a>
## *getposition <sup>method</sup>*

Gets playback position.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Floating Point Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | float | <sup>*(optional)*</sup> The returned float value |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getposition",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": 0.0,
        "failurereason": 0
    }
}
```

<a name="method.setspeed"></a>
## *setspeed <sup>method</sup>*

Sets playback speed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | A float value targetted to a session |
| params.value | float | Generic Floating Point [32 bit] value |
| params?.session | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.setspeed",
    "params": {
        "value": 0.0,
        "session": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.getspeed"></a>
## *getspeed <sup>method</sup>*

Gets playback speed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Floating Point Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | float | <sup>*(optional)*</sup> The returned float value |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getspeed",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": 0.0,
        "failurereason": 0
    }
}
```

<a name="method.getduration"></a>
## *getduration <sup>method</sup>*

Gets duration of Closed Assests. Live Assests return -1.0f.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Floating Point Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | float | <sup>*(optional)*</sup> The returned float value |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getduration",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": 0.0,
        "failurereason": 0
    }
}
```

<a name="method.getavailablespeeds"></a>
## *getavailablespeeds <sup>method</sup>*

Gets supported playback speeds.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Floating Point Array Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | array | <sup>*(optional)*</sup> The returned float array value |
| result?.value[#] | float | <sup>*(optional)*</sup>  |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getavailablespeeds",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": [
            0.0
        ],
        "failurereason": 0
    }
}
```

<a name="method.setdefaultaudiolanguage"></a>
## *setdefaultaudiolanguage <sup>method</sup>*

Sets default audio language.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | A string value targetted to a session |
| params.value | string | Generic string value |
| params?.session | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.setdefaultaudiolanguage",
    "params": {
        "value": "",
        "session": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.getdefaultaudiolanguage"></a>
## *getdefaultaudiolanguage <sup>method</sup>*

Gets default audio language.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic String Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | string | <sup>*(optional)*</sup> The returned string value |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getdefaultaudiolanguage",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": "",
        "failurereason": 0
    }
}
```

<a name="method.setprimaryaudiolanguage"></a>
## *setprimaryaudiolanguage <sup>method</sup>*

Sets primary, presenting, audio language.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | A string value targetted to a session |
| params.value | string | Generic string value |
| params?.session | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.setprimaryaudiolanguage",
    "params": {
        "value": "",
        "session": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.getprimaryaudiolanguage"></a>
## *getprimaryaudiolanguage <sup>method</sup>*

Gets primary audio language.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic String Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | string | <sup>*(optional)*</sup> The returned string value |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getprimaryaudiolanguage",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": "",
        "failurereason": 0
    }
}
```

<a name="method.setsecondaryaudiolanguage"></a>
## *setsecondaryaudiolanguage <sup>method</sup>*

Sets secondary, audio-description, audio language.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | A string value targetted to a session |
| params.value | string | Generic string value |
| params?.session | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.setsecondaryaudiolanguage",
    "params": {
        "value": "",
        "session": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.getsecondaryaudiolanguage"></a>
## *getsecondaryaudiolanguage <sup>method</sup>*

Gets secondary audio language.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic String Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | string | <sup>*(optional)*</sup> The returned string value |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getsecondaryaudiolanguage",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": "",
        "failurereason": 0
    }
}
```

<a name="method.getcurrentaudiolanguage"></a>
## *getcurrentaudiolanguage <sup>method</sup>*

Gets the audio language currently presented to the user.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic String Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | string | <sup>*(optional)*</sup> The returned string value |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getcurrentaudiolanguage",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": "",
        "failurereason": 0
    }
}
```

<a name="method.getavailableaudiolanguages"></a>
## *getavailableaudiolanguages <sup>method</sup>*

Gets all the audio languages currently in the PMT or Manifest.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic String Array Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | array | <sup>*(optional)*</sup> The returned string array value |
| result?.value[#] | string | <sup>*(optional)*</sup>  |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getavailableaudiolanguages",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": [
            ""
        ],
        "failurereason": 0
    }
}
```

<a name="method.setdefaultsubtitleslanguage"></a>
## *setdefaultsubtitleslanguage <sup>method</sup>*

Sets default subtitles language.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | A string value targetted to a session |
| params.value | string | Generic string value |
| params?.session | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.setdefaultsubtitleslanguage",
    "params": {
        "value": "",
        "session": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.getdefaultsubtitleslanguage"></a>
## *getdefaultsubtitleslanguage <sup>method</sup>*

Gets default subtitles language.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic String Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | string | <sup>*(optional)*</sup> The returned string value |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getdefaultsubtitleslanguage",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": "",
        "failurereason": 0
    }
}
```

<a name="method.setprimarysubtitleslanguage"></a>
## *setprimarysubtitleslanguage <sup>method</sup>*

Sets primary, presenting, subtitles language.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | A string value targetted to a session |
| params.value | string | Generic string value |
| params?.session | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.setprimarysubtitleslanguage",
    "params": {
        "value": "",
        "session": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.getprimarysubtitleslanguage"></a>
## *getprimarysubtitleslanguage <sup>method</sup>*

Gets primary subtitles language.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic String Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | string | <sup>*(optional)*</sup> The returned string value |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getprimarysubtitleslanguage",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": "",
        "failurereason": 0
    }
}
```

<a name="method.getcurrentsubtitleslanguage"></a>
## *getcurrentsubtitleslanguage <sup>method</sup>*

Gets the subtitles language currently presented to the user.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic String Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | string | <sup>*(optional)*</sup> The returned string value |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getcurrentsubtitleslanguage",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": "",
        "failurereason": 0
    }
}
```

<a name="method.getavailablesubtitleslanguages"></a>
## *getavailablesubtitleslanguages <sup>method</sup>*

Gets all the audio languages currently in the PMT or Manifest.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic String Array Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | array | <sup>*(optional)*</sup> The returned string array value |
| result?.value[#] | string | <sup>*(optional)*</sup>  |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getavailablesubtitleslanguages",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": [
            ""
        ],
        "failurereason": 0
    }
}
```

<a name="method.setmute"></a>
## *setmute <sup>method</sup>*

Sets if media compopnents should be muted or not.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Sets if presentation should be muted [audio and video] |
| params?.session | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |
| params?.video | boolean | <sup>*(optional)*</sup>  |
| params?.audio | boolean | <sup>*(optional)*</sup>  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.setmute",
    "params": {
        "session": 0,
        "video": false,
        "audio": false
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.getmute"></a>
## *getmute <sup>method</sup>*

Gets status of media components muting.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.video | boolean | <sup>*(optional)*</sup> Muted [true] or unmuted [false] video |
| result?.audio | boolean | <sup>*(optional)*</sup> Mute [true] or unmuted [false] audio |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getmute",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "video": false,
        "audio": false,
        "failurereason": 0
    }
}
```

<a name="method.setsubtitlesenabled"></a>
## *setsubtitlesenabled <sup>method</sup>*

Sets if subtitles are decoded and presented.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | A Boolean value targetted to a session |
| params.value | boolean | Generic Boolean value |
| params?.session | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.setsubtitlesenabled",
    "params": {
        "value": false,
        "session": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.getsubtitlesenabled"></a>
## *getsubtitlesenabled <sup>method</sup>*

Gets if subtitles are enabled.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Boolean Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | boolean | <sup>*(optional)*</sup> The returned boolean value |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getsubtitlesenabled",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": false,
        "failurereason": 0
    }
}
```

<a name="method.setsubtitlesdisplayed"></a>
## *setsubtitlesdisplayed <sup>method</sup>*

Sets if subtitles are presented.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | A Boolean value targetted to a session |
| params.value | boolean | Generic Boolean value |
| params?.session | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.setsubtitlesdisplayed",
    "params": {
        "value": false,
        "session": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.getsubtitlesdisplayed"></a>
## *getsubtitlesdisplayed <sup>method</sup>*

Gets if subtitles are presented.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Boolean Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.value | boolean | <sup>*(optional)*</sup> The returned boolean value |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.getsubtitlesdisplayed",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "value": false,
        "failurereason": 0
    }
}
```

<a name="method.setvideorectangle"></a>
## *setvideorectangle <sup>method</sup>*

Sets the size and position of the video.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | A rectangle optionally targetted to a session |
| params.x | number | Rectangle X Position |
| params.y | number | Rectangle Y Position |
| params.w | number | Rectangle Width |
| params.h | number | Rectangle Height |
| params?.session | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.setvideorectangle",
    "params": {
        "x": 0,
        "y": 0,
        "w": 0,
        "h": 0,
        "session": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.play"></a>
## *play <sup>method</sup>*

Play the Video at x1 speed, HTTP sources only.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.play",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.pause"></a>
## *pause <sup>method</sup>*

Pauses the video, HTTP sources only.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.pause",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.seektolive"></a>
## *seektolive <sup>method</sup>*

seeks to the live point, HTTP sources only, non-closed manifests only.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.seektolive",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.stop"></a>
## *stop <sup>method</sup>*

Stops playback and closes the source.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.stop",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.requeststatus"></a>
## *requeststatus <sup>method</sup>*

Causes a status event to be triggered.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | integer | ID of created session, used to identify sessions to control |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedPlayer.1.requeststatus",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the UnifiedPlayer interface:

UnifiedPlayer interface events:

| Event | Description |
| :-------- | :-------- |
| [onplayerinitialized](#event.onplayerinitialized) | Sent when the player has accepted a media URL |
| [onmediaopened](#event.onmediaopened) | Sent when media is successfully opened |
| [onplaying](#event.onplaying) | Sent when the player transitions to x1 play speed |
| [onpaused](#event.onpaused) | Sent when the player enters a paused state |
| [onclosed](#event.onclosed) | Sent when media is closed |
| [oncomplete](#event.oncomplete) | Sent when the stream complete |
| [onstatus](#event.onstatus) | Sent when status changes |
| [onprogress](#event.onprogress) | Sent when progress or duration changes |
| [onwarning](#event.onwarning) | Sent when a warning is issued |
| [onerror](#event.onerror) | Sent when an error is issued |
| [onbitratechange](#event.onbitratechange) | Sent when current ABR bitrate changes |


<a name="event.onplayerinitialized"></a>
## *onplayerinitialized <sup>event</sup>*

Sent when the player has accepted a media URL.

### Description

Register for this event to be notified about player preparedness

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Event sent when a player completes initialization |
| params.sessionid | integer | ID of created session, used to identify sessions to control |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onplayerinitialized",
    "params": {
        "sessionid": 0
    }
}
```

<a name="event.onmediaopened"></a>
## *onmediaopened <sup>event</sup>*

Sent when media is successfully opened.

### Description

Register for this event to be notified about media opening events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Event sent when media is opened |
| params?.sessionid | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |
| params?.mediatype | string | <sup>*(optional)*</sup> The type of media |
| params?.duration | number | <sup>*(optional)*</sup> The duration in seconds the media lasts, if known |
| params?.width | number | <sup>*(optional)*</sup> The horizontal size of the image |
| params?.height | number | <sup>*(optional)*</sup> The vertical size of the image |
| params?.availablespeeds | array | <sup>*(optional)*</sup>  |
| params?.availablespeeds[#] | float | <sup>*(optional)*</sup>  |
| params?.availableaudiolanguages | array | <sup>*(optional)*</sup>  |
| params?.availableaudiolanguages[#] | string | <sup>*(optional)*</sup>  |
| params?.availablesubtitleslanguages | array | <sup>*(optional)*</sup>  |
| params?.availablesubtitleslanguages[#] | string | <sup>*(optional)*</sup>  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onmediaopened",
    "params": {
        "sessionid": 0,
        "mediatype": "Live",
        "duration": 0,
        "width": 0,
        "height": 0,
        "availablespeeds": [
            0.0
        ],
        "availableaudiolanguages": [
            ""
        ],
        "availablesubtitleslanguages": [
            ""
        ]
    }
}
```

<a name="event.onplaying"></a>
## *onplaying <sup>event</sup>*

Sent when the player transitions to x1 play speed.

### Description

Register for this event to be notified about transport status

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Event sent when playback starts |
| params.sessionid | integer | ID of created session, used to identify sessions to control |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onplaying",
    "params": {
        "sessionid": 0
    }
}
```

<a name="event.onpaused"></a>
## *onpaused <sup>event</sup>*

Sent when the player enters a paused state.

### Description

Register for this event to be notified about transport status

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Event sent when playback pauses |
| params.sessionid | integer | ID of created session, used to identify sessions to control |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onpaused",
    "params": {
        "sessionid": 0
    }
}
```

<a name="event.onclosed"></a>
## *onclosed <sup>event</sup>*

Sent when media is closed.

### Description

Register for this event to be notified about media state

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Event sent when a session closes |
| params.sessionid | integer | ID of created session, used to identify sessions to control |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onclosed",
    "params": {
        "sessionid": 0
    }
}
```

<a name="event.oncomplete"></a>
## *oncomplete <sup>event</sup>*

Sent when the stream complete.

### Description

Register for this event to be notified when the player recieves EOS

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Event sent when playback completes |
| params.sessionid | integer | ID of created session, used to identify sessions to control |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.oncomplete",
    "params": {
        "sessionid": 0
    }
}
```

<a name="event.onstatus"></a>
## *onstatus <sup>event</sup>*

Sent when status changes.

### Description

Register for this event to be notified about status change, or when requesting status

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Event sent when status changes, or in response to a request |
| params?.sessionid | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |
| params.position | float |  |
| params.duration | float |  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onstatus",
    "params": {
        "sessionid": 0,
        "position": 0.0,
        "duration": 0.0
    }
}
```

<a name="event.onprogress"></a>
## *onprogress <sup>event</sup>*

Sent when progress or duration changes.

### Description

Register for this event to be notified about progess or when the media length changes

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Event sent when progress changes, or in response to a position or transport command |
| params?.sessionid | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |
| params.position | float |  |
| params.duration | float |  |
| params?.speed | float | <sup>*(optional)*</sup>  |
| params?.start | float | <sup>*(optional)*</sup>  |
| params?.end | float | <sup>*(optional)*</sup>  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onprogress",
    "params": {
        "sessionid": 0,
        "position": 0.0,
        "duration": 0.0,
        "speed": 0.0,
        "start": 0.0,
        "end": 0.0
    }
}
```

<a name="event.onwarning"></a>
## *onwarning <sup>event</sup>*

Sent when a warning is issued.

### Description

Register for this event to be notified about warnings

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Event sent when a warning or error is encountered |
| params?.sessionid | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |
| params.code | number |  |
| params?.description | string | <sup>*(optional)*</sup>  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onwarning",
    "params": {
        "sessionid": 0,
        "code": 0,
        "description": ""
    }
}
```

<a name="event.onerror"></a>
## *onerror <sup>event</sup>*

Sent when an error is issued.

### Description

Register for this event to be notified about errors

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Event sent when a warning or error is encountered |
| params?.sessionid | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |
| params.code | number |  |
| params?.description | string | <sup>*(optional)*</sup>  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onerror",
    "params": {
        "sessionid": 0,
        "code": 0,
        "description": ""
    }
}
```

<a name="event.onbitratechange"></a>
## *onbitratechange <sup>event</sup>*

Sent when current ABR bitrate changes.

### Description

Register for this event to be notified about ABR bitrate changes

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Event sent when an ABR stream changes bitrate |
| params?.sessionid | integer | <sup>*(optional)*</sup> ID of created session, used to identify sessions to control |
| params.bitrate | number |  |
| params?.reason | string | <sup>*(optional)*</sup>  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onbitratechange",
    "params": {
        "sessionid": 0,
        "bitrate": 0,
        "reason": ""
    }
}
```

