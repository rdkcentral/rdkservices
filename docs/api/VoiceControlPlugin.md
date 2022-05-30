<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.VoiceControl_Plugin"></a>
# VoiceControl Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.VoiceControl plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.VoiceControl plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `VoiceControl` plugin manages voice control sessions.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.VoiceControl*) |
| classname | string | Class name: *org.rdk.VoiceControl* |
| locator | string | Library name: *libWPEFrameworkVoiceControl.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.VoiceControl plugin:

VoiceControl interface methods:

| Method | Description |
| :-------- | :-------- |
| [configureVoice](#method.configureVoice) | Configures the RDK's voice stack |
| [sendVoiceMessage](#method.sendVoiceMessage) | Sends a message to the Voice Server |
| [setVoiceInit](#method.setVoiceInit) | Sets the application metadata in the INIT message that gets sent to the Voice Server |
| [voiceSessionByText](#method.voiceSessionByText) | Sends a voice session with a transcription string to simulate a real voice session for QA |
| [voiceStatus](#method.voiceStatus) | Returns the current status of the RDK voice stack |


<a name="method.configureVoice"></a>
## *configureVoice [<sup>method</sup>](#head.Methods)*

Configures the RDK's voice stack. NOTE: The URL Scheme determines which VREX API protocol is used. Supported URL schemes include:  
* http/https - VREX Legacy HTTP API  
* ws/wss - VREX XR18 WS API  
* vrng/vrngs - VREX NextGen WS API.

### Events

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.urlAll | string | <sup>*(optional)*</sup> Specifies the URL for all devices instead of individually specifying the URL for each device |
| params?.urlPtt | string | <sup>*(optional)*</sup> The PTT URL |
| params?.urlHf | string | <sup>*(optional)*</sup> The HF (ff and mic) URL |
| params?.enable | boolean | <sup>*(optional)*</sup> Enables or disables all of the voice devices instead of individually enabling or disabling each device |
| params?.prv | boolean | <sup>*(optional)*</sup> The Press & Release Voice feature. `true` for enable, `false` for disable |
| params?.wwFeedback | boolean | <sup>*(optional)*</sup> The Wake Word Feedback feature (typically an audible beep). `true` for enable, `false` for disable |
| params?.ptt | object | <sup>*(optional)*</sup> The settings for PTT devices |
| params?.ptt.enable | boolean | Enable (`true`) or disable (`false`) |
| params?.ff | object | <sup>*(optional)*</sup> The settings for FF devices |
| params?.ff.enable | boolean | Enable (`true`) or disable (`false`) |
| params?.mic | object | <sup>*(optional)*</sup> The settings for MIC devices |
| params?.mic.enable | boolean | Enable (`true`) or disable (`false`) |

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
    "method": "org.rdk.VoiceControl.1.configureVoice",
    "params": {
        "urlAll": "ws://voiceserver.com/voice/ptt",
        "urlPtt": "vrng://vrex-next-gen-api.vrexcore.net/vrex/speech/websocket",
        "urlHf": "ws://voiceserver.com/voice/hf",
        "enable": true,
        "prv": true,
        "wwFeedback": false,
        "ptt": {
            "enable": false
        },
        "ff": {
            "enable": false
        },
        "mic": {
            "enable": false
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

<a name="method.sendVoiceMessage"></a>
## *sendVoiceMessage [<sup>method</sup>](#head.Methods)*

Sends a message to the Voice Server. The specification of this message is not in the scope of this document. Example use cases for this API call include sending context or sending ASR blobs to the server.

### Events

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.msgType | string | <sup>*(optional)*</sup> msg type expected from server |
| params?.trx | string | <sup>*(optional)*</sup> The unique id of the voice session |
| params?.created | number | <sup>*(optional)*</sup> The timestamp for server information |
| params?.msgPayload | object | <sup>*(optional)*</sup> NA |

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
    "method": "org.rdk.VoiceControl.1.sendVoiceMessage",
    "params": {
        "msgType": "ars",
        "trx": "1b11359e-23fe-4f2f-9ba8-cc19b87203cf",
        "created": 91890278389232,
        "msgPayload": {}
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

<a name="method.setVoiceInit"></a>
## *setVoiceInit [<sup>method</sup>](#head.Methods)*

Sets the application metadata in the INIT message that gets sent to the Voice Server. The specification of this blob is not in the scope of this document, but it MUST be a JSON blob.

### Events

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.capabilities | array | <sup>*(optional)*</sup> A list of capabilities |
| params?.capabilities[#] | string | <sup>*(optional)*</sup>  |
| params?.language | string | <sup>*(optional)*</sup> Preferred user interface language |

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
    "method": "org.rdk.VoiceControl.1.setVoiceInit",
    "params": {
        "capabilities": [
            "PRV"
        ],
        "language": "eng-USA"
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

<a name="method.voiceSessionByText"></a>
## *voiceSessionByText [<sup>method</sup>](#head.Methods)*

Sends a voice session with a transcription string to simulate a real voice session for QA. Example use cases for this API call include rack and automation testing.

### Events

| Event | Description |
| :-------- | :-------- |
| `onSessionBegin` |Triggers if the voice session begins |
| `onStreamBegin` |Triggers if a device starts streaming voice data to the RDK|
| `onServerMessage` |Triggers if a message is received from the Voice Server |
| `onStreamEnd` |Triggers if streaming audio is stopped from the device |
| `onSessionEnd` |Triggers if interaction with the server is end|.

Also see: [onSessionBegin](#event.onSessionBegin), [onStreamBegin](#event.onStreamBegin), [onServerMessage](#event.onServerMessage), [onStreamEnd](#event.onStreamEnd), [onSessionEnd](#event.onSessionEnd)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.transcription | string | The transcription text to be sent to the voice server |
| params?.type | string | <sup>*(optional)*</sup> The device type to simulate the voice session from (PTT, FF, MIC) |

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
    "method": "org.rdk.VoiceControl.1.voiceSessionByText",
    "params": {
        "transcription": "Watch Comedy Central",
        "type": "PTT"
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

<a name="method.voiceStatus"></a>
## *voiceStatus [<sup>method</sup>](#head.Methods)*

Returns the current status of the RDK voice stack. This includes which URLs the stack is currently configured for along with the status for each device type.

### Events

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.capabilities | array | A list of capabilities |
| result.capabilities[#] | string |  |
| result.urlPtt | string | The PTT URL |
| result.urlHf | string | The HF (ff and mic) URL |
| result.prv | boolean | The Press & Release Voice feature. `true` for enable, `false` for disable |
| result.wwFeedback | boolean | The Wake Word Feedback feature (typically an audible beep). `true` for enable, `false` for disable |
| result.ptt | object | The status information for the PTT device type |
| result.ptt.status | string | The status of the device |
| result.ff | object | The status information for the FF device type |
| result.ff.status | string | The status of the device |
| result.mic | object | The status information for the MIC device type |
| result.mic.status | string | The status of the device |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.VoiceControl.1.voiceStatus"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "capabilities": [
            "PRV"
        ],
        "urlPtt": "vrng://vrex-next-gen-api.vrexcore.net/vrex/speech/websocket",
        "urlHf": "ws://voiceserver.com/voice/hf",
        "prv": true,
        "wwFeedback": false,
        "ptt": {
            "status": "ready"
        },
        "ff": {
            "status": "ready"
        },
        "mic": {
            "status": "ready"
        },
        "success": true
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.VoiceControl plugin:

VoiceControl interface events:

| Event | Description |
| :-------- | :-------- |
| [onKeywordVerification](#event.onKeywordVerification) | Triggered when a keyword verification result is received |
| [onServerMessage](#event.onServerMessage) | Triggered when a message is received from the Voice Server |
| [onSessionBegin](#event.onSessionBegin) | Triggered when a voice session begins |
| [onSessionEnd](#event.onSessionEnd) | Triggered when the interaction with the server has concluded |
| [onStreamBegin](#event.onStreamBegin) | Triggered when a device starts streaming voice data to the RDK |
| [onStreamEnd](#event.onStreamEnd) | Triggered when the device has stopped streaming audio |


<a name="event.onKeywordVerification"></a>
## *onKeywordVerification [<sup>event</sup>](#head.Notifications)*

Triggered when a keyword verification result is received.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The voice device identifier |
| params.sessionId | string | The unique identifier for the voice session, generated by the underlying RDK stack |
| params.verified | boolean | `true` if the keyword was verified, otherwise `false` |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onKeywordVerification",
    "params": {
        "remoteId": 1,
        "sessionId": "1b11359e-23fe-4f2f-9ba8-cc19b87203cf",
        "verified": true
    }
}
```

<a name="event.onServerMessage"></a>
## *onServerMessage [<sup>event</sup>](#head.Notifications)*

Triggered when a message is received from the Voice Server. The `params` value is a contract between the Voice Server and the Application. The definition of this object is outside of the scope of this document.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.msgType | string | Message type from the server |
| params.trx | string | The unique id of the voice session |
| params.created | number | The timestamp for server information |
| params.msgPayload | object | Vrex server information |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onServerMessage",
    "params": {
        "msgType": "ars",
        "trx": "1b11359e-23fe-4f2f-9ba8-cc19b87203cf",
        "created": 91890278389232,
        "msgPayload": {}
    }
}
```

<a name="event.onSessionBegin"></a>
## *onSessionBegin [<sup>event</sup>](#head.Notifications)*

Triggered when a voice session begins.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The voice device identifier |
| params.sessionId | string | The unique identifier for the voice session, generated by the underlying RDK stack |
| params.deviceType | string | The type of voice device starting the session (must be one of the following: *ptt*, *ff*, *mic*) |
| params.keywordVerification | boolean | `true` if the session uses keyword verification, otherwise `false` |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onSessionBegin",
    "params": {
        "remoteId": 1,
        "sessionId": "1b11359e-23fe-4f2f-9ba8-cc19b87203cf",
        "deviceType": "ptt",
        "keywordVerification": true
    }
}
```

<a name="event.onSessionEnd"></a>
## *onSessionEnd [<sup>event</sup>](#head.Notifications)*

Triggered when the interaction with the server has concluded.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.serverStats | object | <sup>*(optional)*</sup> Returns the voice server stats |
| params?.serverStats.dnsTime | number | The DNS time of the voice server |
| params?.serverStats.serverIp | string | The ip of the voice server |
| params?.serverStats.connectTime | number | The connection time of the voice server |
| params.remoteId | integer | The voice device identifier |
| params.sessionId | string | The unique identifier for the voice session, generated by the underlying RDK stack |
| params?.result | string | <sup>*(optional)*</sup> The result of the voice session. This also determines which object will be in the parameters (`success`, `error`, `abort`, `shortUtterance`) |
| params?.success | object | <sup>*(optional)*</sup> This optional object is included only if the `result` value is `success` |
| params?.success.transcription | string | The transcription provided by the voice server |
| params?.error | object | <sup>*(optional)*</sup> This optional object is included only if the `result` value is `error` |
| params?.error?.protocolErrorCode | integer | <sup>*(optional)*</sup> The return code from the protocol used to communicate with the server (HTTP, WSS, and so on) |
| params?.error?.protocolLibraryErrorCode | string | <sup>*(optional)*</sup> The return code from the library that was used to connect to the server (CURL, Nopoll, and so on) |
| params?.error?.serverErrorCode | integer | <sup>*(optional)*</sup> The return code from the voice server |
| params?.error?.serverErrorString | string | <sup>*(optional)*</sup> The error string from the voice server |
| params?.error?.internalErrorCode | integer | <sup>*(optional)*</sup> The return code from the underlying RDK stack |
| params?.abort | object | <sup>*(optional)*</sup> This optional object is included only if the `result` value is `abort` |
| params?.abort.reason | integer | The reason code that identifies why the session was aborted: `0` - Busy, `1` - Server Not Ready, `2` - Audio Format Error, `3` - General Failure, `4` - Voice Disabled, `5` - Device Update, `6` - Missing Authentication Data (that is, No Device ID, SAT, and so on), `7` - New Session, `8` - Controller ID Invalid, `9` - Application Restart (must be one of the following: *0*, *1*, *2*, *3*, *4*, *5*, *6*, *7*, *8*, *9*) |
| params?.shortUtterance | object | <sup>*(optional)*</sup> This optional object is included only if the `result` value is `abort` |
| params?.shortUtterance.reason | integer | The reason code that identifies why the session was aborted: `0` - End of Stream (Mic Key Released / EOS detected), `1` - First Packet Timeout, `2` - Inter-packet Timeout, `3` - Max Utterance Length, `4` - Adjacent Key Press, `5` - Other Key Press, `6` - Other / Unknown (must be one of the following: *0*, *1*, *2*, *3*, *4*, *5*, *6*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onSessionEnd",
    "params": {
        "serverStats": {
            "dnsTime": 1.0,
            "serverIp": "...",
            "connectTime": 1.0
        },
        "remoteId": 1,
        "sessionId": "1b11359e-23fe-4f2f-9ba8-cc19b87203cf",
        "result": "success",
        "success": {
            "transcription": "Comedy Central"
        },
        "error": {
            "protocolErrorCode": 200,
            "protocolLibraryErrorCode": "...",
            "serverErrorCode": 1,
            "serverErrorString": "Error",
            "internalErrorCode": 0
        },
        "abort": {
            "reason": 1
        },
        "shortUtterance": {
            "reason": 1
        }
    }
}
```

<a name="event.onStreamBegin"></a>
## *onStreamBegin [<sup>event</sup>](#head.Notifications)*

Triggered when a device starts streaming voice data to the RDK. This event is optional, and will most likely be used for follow up sessions.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The voice device identifier |
| params.sessionId | string | The unique identifier for the voice session, generated by the underlying RDK stack |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onStreamBegin",
    "params": {
        "remoteId": 1,
        "sessionId": "1b11359e-23fe-4f2f-9ba8-cc19b87203cf"
    }
}
```

<a name="event.onStreamEnd"></a>
## *onStreamEnd [<sup>event</sup>](#head.Notifications)*

Triggered when the device has stopped streaming audio.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The voice device identifier |
| params.sessionId | string | The unique identifier for the voice session, generated by the underlying RDK stack |
| params.reason | integer | The reason code for why the device stopped streaming audio: `0` - End of Stream (Mic Key Released / EOS detected), `1` - First Packet Timeout, `2` - Inter-packet Timeout, `3` - Max Utterance Length, `4` - Adjacent Key Press, `5`- Other Key Press, `6` - Other / Unknown (must be one of the following: *0*, *1*, *2*, *3*, *4*, *5*, *6*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onStreamEnd",
    "params": {
        "remoteId": 1,
        "sessionId": "1b11359e-23fe-4f2f-9ba8-cc19b87203cf",
        "reason": 0
    }
}
```

