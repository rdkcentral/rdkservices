<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.ControlService_Plugin"></a>
# ControlService Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.ControlService plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.ControlService plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `ControlService` plugin provides the ability to manage the remote controls of a STB.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.ControlService*) |
| classname | string | Class name: *org.rdk.ControlService* |
| locator | string | Library name: ** |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.ControlService plugin:

ControlService interface methods:

| Method | Description |
| :-------- | :-------- |
| [canFindMyRemote](#method.canFindMyRemote) | Checks if the Control Manager can search for the remote |
| [endPairingMode](#method.endPairingMode) | Leaves pairing mode |
| [findMyRemote](#method.findMyRemote) | Searches for the remote |
| [getAllRemoteData](#method.getAllRemoteData) | Returns all remote data |
| [getLastKeypressSource](#method.getLastKeypressSource) | Returns last key press source data |
| [getLastPairedRemoteData](#method.getLastPairedRemoteData) | Returns all remote data for the last paired remote |
| [getSingleRemoteData](#method.getSingleRemoteData) | Returns all remote data for the specified remote |
| [getValues](#method.getValues) | Returns remote setting values |
| [setValues](#method.setValues) | Sets remote setting values |
| [startPairingMode](#method.startPairingMode) | Enters pairing mode |


<a name="method.canFindMyRemote"></a>
## *canFindMyRemote <sup>method</sup>*

Checks if the Control Manager can search for the remote.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.result | boolean | The operation status |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.ControlService.1.canFindMyRemote",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "result": true,
        "success": true
    }
}
```

<a name="method.endPairingMode"></a>
## *endPairingMode <sup>method</sup>*

Leaves pairing mode.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.bindStatus | integer | The binding status |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.ControlService.1.endPairingMode",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "bindStatus": 1,
        "status_code": 0,
        "success": true
    }
}
```

<a name="method.findMyRemote"></a>
## *findMyRemote <sup>method</sup>*

Searches for the remote.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.timeOutPeriod | integer | The timeout interval in milliseconds |
| params.bOnlyLastUsed | boolean | Whether to use the last controller ID |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.ControlService.1.findMyRemote",
    "params": {
        "timeOutPeriod": 100,
        "bOnlyLastUsed": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "status_code": 0,
        "success": true
    }
}
```

<a name="method.getAllRemoteData"></a>
## *getAllRemoteData <sup>method</sup>*

Returns all remote data.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.stbRf4ceMACAddress | string | The STB MAC address for the RF4CE network |
| result.stbRf4ceSocMfr | string | The STB SOC Manufacturer |
| result.stbHALVersion | string | The STB HAL version |
| result.stbRf4ceShortAddress | integer | The STB short address for the RF4CE |
| result.stbPanId | integer | The STB PAN Identifier |
| result.stbActiveChannel | integer | The STB active channel |
| result.stbNumPairedRemotes | integer | The number of remotes paired to the STB |
| result.stbNumScreenBindFailures | integer | The number of screen bind failures for the STB |
| result.stbLastScreenBindErrorCode | integer | The last screen bind error code on the STB |
| result.stbLastScreenBindErrorRemoteType | string | The remote type for the last screen bind error on the STB |
| result.stbLastScreenBindErrorTimestamp | integer | The last screen bind error timestamp on the STB |
| result.stbNumOtherBindFailures | integer | The number of other bind failures on the STB |
| result.stbLastOtherBindErrorCode | integer | The last other bind error code on the STB |
| result.stbLastOtherBindErrorRemoteType | string | The remote type for the last other bind error on the STB |
| result.stbLastOtherBindErrorBindType | integer | The last other bind error bind type on the STB |
| result.stbLastOtherBindErrorTimestamp | integer | The last other bind error timestamp |
| result.bHasIrRemotePreviousDay | boolean | `true` or `false` |
| result.bHasIrRemoteCurrentDay | boolean | `true` or `false` |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.ControlService.1.getAllRemoteData",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "stbRf4ceMACAddress": "0x00155F00205E1XXX",
        "stbRf4ceSocMfr": "GP502KXBG",
        "stbHALVersion": "GPv2.6.3.514598",
        "stbRf4ceShortAddress": 64253,
        "stbPanId": 25684,
        "stbActiveChannel": 25,
        "stbNumPairedRemotes": 0,
        "stbNumScreenBindFailures": 1,
        "stbLastScreenBindErrorCode": 1,
        "stbLastScreenBindErrorRemoteType": "",
        "stbLastScreenBindErrorTimestamp": 1589356931000,
        "stbNumOtherBindFailures": 3,
        "stbLastOtherBindErrorCode": 1,
        "stbLastOtherBindErrorRemoteType": "",
        "stbLastOtherBindErrorBindType": 2,
        "stbLastOtherBindErrorTimestamp": 1589359161000,
        "bHasIrRemotePreviousDay": false,
        "bHasIrRemoteCurrentDay": false,
        "status_code": 0,
        "success": true
    }
}
```

<a name="method.getLastKeypressSource"></a>
## *getLastKeypressSource <sup>method</sup>*

Returns last key press source data. The data, if any, is returned as part of the `result` object.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.ControlService.1.getLastKeypressSource",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "status_code": 0,
        "success": true
    }
}
```

<a name="method.getLastPairedRemoteData"></a>
## *getLastPairedRemoteData <sup>method</sup>*

Returns all remote data for the last paired remote. The data, if any, is returned as part of the `result` object.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.ControlService.1.getLastPairedRemoteData",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "status_code": 0,
        "success": true
    }
}
```

<a name="method.getSingleRemoteData"></a>
## *getSingleRemoteData <sup>method</sup>*

Returns all remote data for the specified remote. The data, if any, is returned as part of the `result` object.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote control identifier |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.ControlService.1.getSingleRemoteData",
    "params": {
        "remoteId": 1
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "status_code": 0,
        "success": true
    }
}
```

<a name="method.getValues"></a>
## *getValues <sup>method</sup>*

Returns remote setting values.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty `params` object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportsASB | boolean | Whether the remote supports ASB |
| result.enableASB | boolean | Whether ASB is enabled |
| result.enableOpenChime | boolean | Whether the open chime is enabled |
| result.enableCloseChime | boolean | Whether the close chime is enabled |
| result.enablePrivacyChime | boolean | Whether the privacy chime is enabled |
| result.conversationalMode | integer | The conversational mode |
| result.chimeVolume | integer | The chime volume |
| result.irCommandRepeats | integer | The number of command repeats |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.ControlService.1.getValues",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "supportsASB": true,
        "enableASB": false,
        "enableOpenChime": false,
        "enableCloseChime": true,
        "enablePrivacyChime": true,
        "conversationalMode": 6,
        "chimeVolume": 1,
        "irCommandRepeats": 3,
        "status_code": 0,
        "success": true
    }
}
```

<a name="method.setValues"></a>
## *setValues <sup>method</sup>*

Sets remote setting values.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.supportsASB | boolean | Whether the remote supports ASB |
| params.enableASB | boolean | Whether ASB is enabled |
| params.enableOpenChime | boolean | Whether the open chime is enabled |
| params.enableCloseChime | boolean | Whether the close chime is enabled |
| params.enablePrivacyChime | boolean | Whether the privacy chime is enabled |
| params.conversationalMode | integer | The conversational mode |
| params.chimeVolume | integer | The chime volume |
| params.irCommandRepeats | integer | The number of command repeats |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.ControlService.1.setValues",
    "params": {
        "supportsASB": true,
        "enableASB": false,
        "enableOpenChime": false,
        "enableCloseChime": true,
        "enablePrivacyChime": true,
        "conversationalMode": 6,
        "chimeVolume": 1,
        "irCommandRepeats": 3
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "status_code": 0,
        "success": true
    }
}
```

<a name="method.startPairingMode"></a>
## *startPairingMode <sup>method</sup>*

Enters pairing mode.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.pairingMode | integer | The pairing mode |
| params.restrictPairing | integer | The pairing restrictions |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status_code | integer | An operations status code |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.ControlService.1.startPairingMode",
    "params": {
        "pairingMode": 0,
        "restrictPairing": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "status_code": 0,
        "success": true
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.ControlService plugin:

ControlService interface events:

| Event | Description |
| :-------- | :-------- |
| [onControl](#event.onControl) | Triggered on control manager events |
| [onXRConfigurationComplete](#event.onXRConfigurationComplete) | Triggered on control manager configuration complete event |
| [onXRPairingStart](#event.onXRPairingStart) | Triggered on control manager validation/pairing key press event |
| [onXRValidationComplete](#event.onXRValidationComplete) | Triggered on control manager validation/pairing end event |
| [onXRValidationUpdate](#event.onXRValidationUpdate) | Triggered on control manager events |


<a name="event.onControl"></a>
## *onControl <sup>event</sup>*

Triggered on control manager events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote control identifier |
| params.eventValue | integer | The event value |
| params.eventSource | string | The event source |
| params.eventType | string | The event type |
| params.eventData | string | The event Data |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onControl",
    "params": {
        "remoteId": 1,
        "eventValue": 0,
        "eventSource": "",
        "eventType": "",
        "eventData": ""
    }
}
```

<a name="event.onXRConfigurationComplete"></a>
## *onXRConfigurationComplete <sup>event</sup>*

Triggered on control manager configuration complete event.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote control identifier |
| params.remoteType | string | The remote control type |
| params.bindingType | integer | The remote control binding type |
| params.configurationStatus | integer | The remote control configuration status |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onXRConfigurationComplete",
    "params": {
        "remoteId": 1,
        "remoteType": "XR11",
        "bindingType": 1,
        "configurationStatus": 1
    }
}
```

<a name="event.onXRPairingStart"></a>
## *onXRPairingStart <sup>event</sup>*

Triggered on control manager validation/pairing key press event.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote control identifier |
| params.remoteType | string | The remote control type |
| params.bindingType | integer | The remote control binding type |
| params.validationDigit1 | integer | The first digit for pairing |
| params.validationDigit2 | integer | The second digit for pairing |
| params.validationDigit3 | integer | The third digit for pairing |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onXRPairingStart",
    "params": {
        "remoteId": 1,
        "remoteType": "XR11",
        "bindingType": 1,
        "validationDigit1": 1,
        "validationDigit2": 3,
        "validationDigit3": 5
    }
}
```

<a name="event.onXRValidationComplete"></a>
## *onXRValidationComplete <sup>event</sup>*

Triggered on control manager validation/pairing end event.  
Validation status codes:  
* `0` - VALIDATION_SUCCESS   
* `1` - VALIDATION_TIMEOUT  
* `2` - VALIDATION_FAILURE  
* `3` - VALIDATION_ABORT  
* `4` - VALIDATION_WRONG_CODE.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote control identifier |
| params.remoteType | string | The remote control type |
| params.bindingType | integer | The remote control binding type |
| params.validationStatus | integer | The validation status code |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onXRValidationComplete",
    "params": {
        "remoteId": 1,
        "remoteType": "XR11",
        "bindingType": 1,
        "validationStatus": 0
    }
}
```

<a name="event.onXRValidationUpdate"></a>
## *onXRValidationUpdate <sup>event</sup>*

Triggered on control manager events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote control identifier |
| params.remoteType | string | The remote control type |
| params.bindingType | integer | The remote control binding type |
| params.validationDigit1 | integer | The first digit for pairing |
| params.validationDigit2 | integer | The second digit for pairing |
| params.validationDigit3 | integer | The third digit for pairing |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onXRValidationUpdate",
    "params": {
        "remoteId": 1,
        "remoteType": "XR11",
        "bindingType": 1,
        "validationDigit1": 1,
        "validationDigit2": 3,
        "validationDigit3": 5
    }
}
```

