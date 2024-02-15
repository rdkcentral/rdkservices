<!-- Generated automatically, DO NOT EDIT! -->
<a name="DataCapture_Plugin"></a>
# DataCapture Plugin

**Version: [1.0.5](https://github.com/rdkcentral/rdkservices/blob/main/DataCapture/CHANGELOG.md)**

A org.rdk.dataCapture plugin for Thunder framework.

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

The DataCapture plugin allows you to capture an audio clip from an audio source and send it to a URL such as a music ID service.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.dataCapture*) |
| classname | string | Class name: *org.rdk.dataCapture* |
| locator | string | Library name: *libWPEFrameworkDataCapture.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.dataCapture plugin:

DataCapture interface methods:

| Method | Description |
| :-------- | :-------- |
| [enableAudioCapture](#enableAudioCapture) | Enables audio capturing to buffer |
| [getAudioClip](#getAudioClip) | Requests the audio driver to capture an audio sample from the specified stream and then delivers the stream sample to a specified URL |


<a name="enableAudioCapture"></a>
## *enableAudioCapture*

Enables audio capturing to buffer.  
Return Values:  
* `0` - No error  
* `1 - 254` - request exceeds the maximum allowed buffer size. The error number represents the maximum buffer length, in seconds, that the set-top device can support.  
* `255` - set top device cannot accommodate any level of audio buffering.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.bufferMaxDuration | integer | Buffer duration in seconds. RDK calculates actual buffer size based on duration length and PCM parameters. Specifying a duration of `0` disables capturing |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.error | integer | An error value |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.dataCapture.enableAudioCapture",
    "params": {
        "bufferMaxDuration": 6
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "error": 0,
        "success": true
    }
}
```

<a name="getAudioClip"></a>
## *getAudioClip*

Requests the audio driver to capture an audio sample from the specified stream and then delivers the stream sample to a specified URL.  
Supported streams:  
* `primary` - The stream going to the analog output and the stream included in the HDMI output. This is the only stream that is valid when the request is made through a voice request.  
* `secondary` - The stream is captured from a secondary decoder. A potential use case includes initiating a capture from a screen overlay where the user has a choice between primary or secondary audio (or the type of audio output to which the user listens – TV or Bluetooth).

### Events

| Event | Description |
| :-------- | :-------- |
| [onAudioClipReady](#onAudioClipReady) | Triggered if an audio clip uploaded successfully or not |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.clipRequest | object | An object that contains audio capture parameters |
| params.clipRequest.stream | string | Type of audio stream that will be captured |
| params.clipRequest.url | string | Destination where to deliver data and any required application parameters. The example shows a URL for a music ID service |
| params.clipRequest.duration | number | Duration of clip in seconds |
| params.clipRequest.captureMode | string | Audio can be captured in the past or it can be captured starting with a trigger. Valid capture modes are: `precapture` - an audio clip is already stored in the buffer and capturing concludes when a call to this function is made. The audio data is sent immediately to the requested URL. `postCapture` - An audio capture starts when a call to this function is made and ends when the duration is reached. Sending data is delayed for the `duration` length. **Note**: This mode is not supported in the current implementation of the audio capture manager |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.error | integer | Returns `0` if the request is supported or an error if the request could not be fulfilled |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.dataCapture.getAudioClip",
    "params": {
        "clipRequest": {
            "stream": "primary",
            "url": "http://musicid.comcast.net/media-service-backend/analyze?trx=83cf6049-b722-4c44-b92e-79a504ae8f85:1458580048400&codec=PCM_16_16K&deviceId=5082732351093257712",
            "duration": 6,
            "captureMode": "preCapture"
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
        "error": 0,
        "success": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.dataCapture plugin:

DataCapture interface events:

| Event | Description |
| :-------- | :-------- |
| [onAudioClipReady](#onAudioClipReady) | Indicates whether an audio clip succeeded or failed to upload |


<a name="onAudioClipReady"></a>
## *onAudioClipReady*

Indicates whether an audio clip succeeded or failed to upload.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.fileName | string | The audio clip name |
| params.status | boolean | Whether the upload succeeded or failed |
| params.message | string | Either `Success` or an error message |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onAudioClipReady",
    "params": {
        "fileName": "acm-songid0",
        "status": true,
        "message": "Success"
    }
}
```

