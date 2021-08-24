
<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.SystemAudioPlayer_Plugin"></a>
# System Audio Player Plugin

**Version: 1.1**

**Status:**

org.rdk.SystemAudioPlayer plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)
- [Event](#head.Event)
- [Notifications](#head.Notifications)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.SystemAudioPlayer  plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The SystemAudioPlayer plugin provides System Audio Playback functionality for the client applications.<br>
It supports various audio types (viz., pcm, mp3, wav) and can play them from one of the chosen source (viz., websocket, httpsrc, filesrc, data buffer).<br>
For PCM content, the application is expected to also provide the "format" of the content in addition to source type and audio type.
The format, audio type and source are [configurable](#method.config) at run time.<br>
It also supports [volume control](#method.setMixerLevels) of the content being played back, as well as primary program audio and
thus allowing the application to duck down volume of primary program audio when a system audio is played back, and restore it back when the system audio playback is complete.<br>

**Note**: mp3 playback development is work in progress.<br>

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration
This plugin will start before TextToSpeech plugin.
The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.SystemAudioPlayer*) |
| classname | string | Class name: *org.rdk.SystemAudioPlayer* |
| locator | string | Library name: *libWPEFrameworkSystemAudioPlayer.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.SystemAudioPlayer plugin:

SystemAudioPlayer interface methods:

| Method | Description |
| :-------- | :-------- |
| [open](#method.open) | Open the System Audio Player
| [config](#method.config) | Configure PCM/MP3 audio source options  |
| [setMixerLevels](#method.setMixerLevels) | Control Primary and audio player's volume |
| [play](#method.play) | Play the audio provided |
| [stop](#method.stop) | Stop the audio playback |
| [pause](#method.pause) | Pause the audio playback|
| [resume](#method.resume) | Resume the playback from where its paused |
| [close](#method.close) |  Close the System Audio Player|


<a name="method.open"></a>
## *open<sup>method</sup>*

Open System Audio player. 
<p> Open has 3 parameters.<br>
1- audiotype [ pcm, mp3, wav ] <br>
2- sourcetype [ websocket, httpsrc, filesrc, data ]<br>
3- playmode [ system , app ]<br>
</p>
audiotype pcm has option of playmode [ system, app ]
If any open() call  playmode with non pcm audio type, it will be ignored. 


**Note**: SystemAudioPlayer can have maximum 1 system and 1 app playmode  player at a time.<br>
Open returns a unique player object "id", which shall be used by the app to do rest of the operations on the player.</p>
Also see: [close](#method.close)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.audiotype| string |  pcm, mp3, wav  can be passed as parameter
| params.sourcetype| string | websocket, httpsrc, filesrc, data  can be passed as parameter  |
| params.playmode| string | system, app can be passed as parameter 

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.id | string | Unique player object "id" |
| result.success | boolean | Whether the request succeeded |


### Example

#### Request

```json
{
	"jsonrpc": "2.0",
	"id": "2105",
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
	"id": 2105,
	"result": {
		"id": 1,
		"success": true
	}
}
```

<a name="method.close"></a>
## *close <sup>method</sup>*

Close the Player. 
SystemAudioPlayer destroy the player object. i.e  if its playing , player will be stopped and closed. All the setMixerlevels() settings will be restored. 

Also see: [open](#method.open)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id| string| open api return id |

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
	"id": "3",
	"method": "org.rdk.SystemAudioPlayer.1.close",
	"params": {
		"id": "1"
	}
}
```

#### Response

```json
{
	"jsonrpc": "2.0",
	"id": 2105,
	"result": {
		"success": true
	}
}
```

<a name="method.stop"></a>
## *stop <sup>method</sup>*

Stop the playback. 

Also see: [pause](#method.pause)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id| string| open api return id |

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
	"id": "3",
	"method": "org.rdk.SystemAudioPlayer.1.stop",
	"params": {
		"id": "1"
	}
}
```

#### Response

```json
{
	"jsonrpc": "2.0",
	"id": 2105,
	"result": {
		"success": true
	}
}
```

<a name="method.play"></a>
## *play<sup>method</sup>*

Play the audio.<br>
audio source will be passed in this.<br>
**Note** If one player is using one playmode and another player try to play using same playmode,<br>
         SystemAudioPlayer returns error(Hardware resource already acquired by session with the player's id).<br>
		 
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id| string | Payer id |
| params.url| string | url source<br> Example: file source file:///opt/beep.wav<br>  Example: httpsource http://localhost:50050/nuanceEve/tts?voice=ava&language=en-US&rate=50&text=SETTINGS <br> Example: WebSocket ws://myserver:40001<br> Note: if no port number provided for web socket, player uses default port 40001<br> |

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
	"id": "3",
	"method": "org.rdk.SystemAudioPlayer.1.play",
	"params": {
		"id": "1",
		"url": "ws://myserver:40001"
	}
}
```
#### Response

```json
{
	"jsonrpc": "2.0",
	"id": 2105,
	"result": {
		"success": true
	}
}
```
<a name="method.pause"></a>
## *pause<sup>method</sup>*

Pause the playback. 
<br>Pause is supported only for httpsrc and file source type.</br>

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id| string | player Object  |
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
	"id": "2105",
	"method": "org.rdk.SystemAudioPlayer.1.pause",
	"params": {
		"id": "1"
	}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 2105,
    "result": {
        "success": true
    }
}
```
<a name="method.resume"></a>
## *resume<sup>method</sup>*

Resume the playback. 
<br>Resume is supported only  httpsrc and file source type.</br>

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id| string | player id |
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
	"id": "2105",
	"method": "org.rdk.SystemAudioPlayer.1.resume",
	"params": {
		"id": "1"
	}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 2105,
    "result": {
        "success": true
    }
}
```

<a name="method.config"></a>
## *config<sup>method</sup>*

Configure the playback.
<br> This API must be called before [play](#method.play). </br>
Currently, only PCM audio source (audio/x-raw) requires to be configured using the "pcmconfig" parameter as one of the input to this method.<br>
Please refer the sample request below for more details.<br> 
There may be more optional configuration params added in the future for PCM as well as other audio types.<br>
Supported audio/x-raw can be found from the below link.
https://gstreamer.freedesktop.org/documentation/rawparse/rawaudioparse.html#src 

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id| string | id the audio player object |
| params.pcmconfig | object | pcm config values see below example  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
   "jsonrpc":"2.0",
   "id":"378001",
   "method":"org.rdk.SystemAudioPlayer.1.config",
   "params":{
      "id":"1",
      "pcmconfig":{
         "format":"S16LE",
         "rate":"22050",
         "channels":"1",
         "layout":"interleaved"
      }
   }
 
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 2105,
    "result": {
        "success": true
    }
}
```
<a name="method.setMixerLevels"></a>
## *setMixerLevels<sup>method</sup>*

App can set mixer level of the audio.  
It can control primary volume audio as well as player's volume.<br>
Primary Volume & Player Volume are from 0-100.<br>
0 is minimum & 100 is maximum volume.</br>
0 volume means, user will not hear any audio on playback.<br> 
 
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.id| string | player id |
| params.primVolume| string| Primary audio volume   |
| params.playerVolume| string| Player audio volume   |

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
	"id": "2105",
	"method": "org.rdk.SystemAudioPlayer.1.setMixerLevels",
	"params": {
		"id": "1",
		"primaryVolume": "20",
		"playerVolume": "7"
	}
}
 
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 2105,
    "result": {
        "success": true
    }
}
```

<a name="head.Event"></a>
# Event

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following event is provided by the org.rdk.SystemAudioPlayer plugin:

SystemAudioPlayer interface event.
App has to register onsapevents() to get all notifications.
App will get event notification based on playback.


| Event | Description |
| :-------- | :-------- |
| [onsapevents](#event.onsapevents) |

Player will be broadcasting the events to all the clients registered.<br>
it is the responsiblity of the client to check the id attribute and discards it if the id does not match.<br>

eg:<br>
client 1 invokes open API(pcm,filesrc,system) and receives the id 1<br>
client 2 invokes open API(mp3,httpsrc,app) and receives the id 2<br>

For all events happening **player will broadcast** to all the listners.<br>
```json 
{
"id":"2",
"event":"PLAYBACK_STARTED"
}
```

For the above event only client2 needs to respond, if any. <br>

 Each event will be of this format<br>
  **Example**:
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
   id is Player id.
<a name="head.Notifications"></a>
# *Notifications*

 The following are notifications which the register app will get. <br>
 **Note**: web socket playback is continious, does not receive PLAYBACK_FINISHED, until and unless stream contains EOS.<br>

| Event Name| Description |
| :-------- | :-------- |
| PLAYBACK_STARTED| When playback starts  |
| PLAYBACK_FINISHED | When playback finishes normally ( web socket does not receive this event) |
| PLAYBACK_PAUSED| When playback paused
| PLAYBACK_RESUMED | When playback resumed |
| NETWORK_ERROR | When playback network error( httpsrc/web socket) |
| PLAYBACK_ERROR| Any other playback error(internal issue )|
| NEED_DATA|  When buffer needs more data to play|

 

```


