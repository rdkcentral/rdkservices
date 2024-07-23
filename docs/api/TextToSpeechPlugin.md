<!-- Generated automatically, DO NOT EDIT! -->
<a name="TextToSpeech_Plugin"></a>
# TextToSpeech Plugin

**Version: [1.0.31](https://github.com/rdkcentral/rdkservices/blob/main/TextToSpeech/CHANGELOG.md)**

A org.rdk.TextToSpeech plugin for Thunder framework.

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

The `TextToSpeech` plugin provides TTS functionality (Voice Guidance & Speech Synthesis) for the client application.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.TextToSpeech*) |
| classname | string | Class name: *org.rdk.TextToSpeech* |
| locator | string | Library name: *libWPEFrameworkTextToSpeech.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.TextToSpeech plugin:

TextToSpeech interface methods:

| Method | Description |
| :-------- | :-------- |
| [cancel](#cancel) | Cancels the speech |
| [enabletts](#enabletts) | (For Resident App) Enables or disables the TTS conversion processing |
| [getapiversion](#getapiversion) | Gets the API Version |
| [getspeechstate](#getspeechstate) | Returns the current state of the speech request |
| [getttsconfiguration](#getttsconfiguration) | Gets the current TTS configuration |
| [isspeaking](#isspeaking) | Checks if speech is in progress |
| [isttsenabled](#isttsenabled) | Returns whether the TTS engine is enabled or disabled |
| [listvoices](#listvoices) | Lists the available voices for the specified language |
| [pause](#pause) | Pauses the speech |
| [resume](#resume) | Resumes the speech |
| [setttsconfiguration](#setttsconfiguration) | Sets the TTS configuration |
| [speak](#speak) | Converts the input text to speech when TTS is enabled |
| [setACL](#setACL) | Configures app to speak |


<a name="cancel"></a>
## *cancel*

Cancels the speech. Triggers the `onspeechinterrupted` 

### Events

| Event | Description |
| :-------- | :-------- |
| [onspeechinterrupted](#onspeechinterrupted) | Triggered when ongoing speech is cancelled. Event is not triggered: if TTS is not enabled; if ongoing Speech is completed |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.speechid | number | The speech ID |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.TTS_Status | number |  (must be one of the following: *TTS_OK(0)*, *TTS_FAIL(1)*, *TTS_NOT_ENABLED(2)*, *TTS_INVALID_CONFIGURATION(3)*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.TextToSpeech.cancel",
    "params": {
        "speechid": 1
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "TTS_Status": 0,
        "success": true
    }
}
```

<a name="enabletts"></a>
## *enabletts*

(For Resident App) Enables or disables the TTS conversion processing. Triggered `onttsstatechanged` event when state changes and `onspeechinterrupted` event when disabling TTS while speech is in-progress.

### Events

| Event | Description |
| :-------- | :-------- |
| [onttsstatechanged](#onttsstatechanged) | state : true Triggered when TTS is enabled; state : false Triggered when TTS is disabled; otherwise No event When TTS enable or disable is in-progress |
| [onspeechinterrupted](#onspeechinterrupted) | Triggered when disabling TTS while speech is in-progress. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabletts | boolean | Enable or Disable TTS |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.TTS_Status | number |  (must be one of the following: *TTS_OK(0)*, *TTS_FAIL(1)*, *TTS_NOT_ENABLED(2)*, *TTS_INVALID_CONFIGURATION(3)*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.TextToSpeech.enabletts",
    "params": {
        "enabletts": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "TTS_Status": 0,
        "success": true
    }
}
```

<a name="getapiversion"></a>
## *getapiversion*

Gets the API Version.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.version | number | Indicates the  API Version |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.TextToSpeech.getapiversion"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "version": 1,
        "success": true
    }
}
```

<a name="getspeechstate"></a>
## *getspeechstate*

Returns the current state of the speech request.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.speechid | number | The speech ID |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.speechstate | number | The speech state (must be one of the following: *SPEECH_PENDING(0)*, *SPEECH_IN_PROGRESS(1)*, *SPEECH_PAUSED(2)*, *SPEECH_NOT_FOUND(3)*) |
| result.TTS_Status | number |  (must be one of the following: *TTS_OK(0)*, *TTS_FAIL(1)*, *TTS_NOT_ENABLED(2)*, *TTS_INVALID_CONFIGURATION(3)*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.TextToSpeech.getspeechstate",
    "params": {
        "speechid": 1
    }
}
```

#### Response

