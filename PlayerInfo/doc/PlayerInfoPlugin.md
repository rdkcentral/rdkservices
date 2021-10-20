<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.Player_Info_Plugin"></a>
# Player Info Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::white_circle:**

PlayerInfo plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Properties](#head.Properties)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the PlayerInfo plugin. It includes detailed specification about its configuration and properties provided.

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

The `PlayerInfo` plugin helps to get system supported audio and video codecs.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *PlayerInfo*) |
| classname | string | Class name: *PlayerInfo* |
| locator | string | Library name: *libWPEPlayerInfo.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Properties"></a>
# Properties

The following properties are provided by the PlayerInfo plugin:

PlayerInfo interface properties:

| Property | Description |
| :-------- | :-------- |
| [playerinfo](#property.playerinfo) <sup>RO</sup> | Player general information |


<a name="property.playerinfo"></a>
## *playerinfo <sup>property</sup>*

Provides access to the player general information.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Player general information |
| (property).audio | array |  |
| (property).audio[#] | string | Audio Codec supported by the platform (must be one of the following: *Undefined*, *AAC*, *AC3*, *AC3Plus*, *DTS*, *MPEG1*, *MPEG2*, *MPEG3*, *MPEG4*, *OPUS*, *VorbisOGG*, *WAV*) |
| (property).video | array |  |
| (property).video[#] | string | Video Codec supported by the platform (must be one of the following: *Undefined*, *H263*, *H264*, *H265*, *H26510*, *MPEG*, *VP8*, *VP9*, *VP10*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "PlayerInfo.1.playerinfo"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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

