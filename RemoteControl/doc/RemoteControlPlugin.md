<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.RemoteControl_Plugin"></a>
# RemoteControl Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.RemoteControl plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.RemoteControl plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `RemoteControl` plugin provides the ability to pair and IR-program remote controls.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.RemoteControl*) |
| classname | string | Class name: *org.rdk.RemoteControl* |
| locator | string | Library name: *libWPEFrameworkRemoteControl.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.RemoteControl plugin:

RemoteControl interface methods:

| Method | Description |
| :-------- | :-------- |
| [clearIRCodes](#method.clearIRCodes) | Clears the IR codes from the specified remote |
| [configureWakeupKeys](#method.configureWakeupKeys) | Configures which keys on the remote will wake the target from deepsleep |
| [getApiVersionNumber](#method.getApiVersionNumber) | Gets the current API version number |
| [getIRCodesByAutoLookup](#method.getIRCodesByAutoLookup) | Returns a list of available IR codes for the TV and AVRs specified by the input parameters |
| [getIRCodesByNames](#method.getIRCodesByNames) | Returns a list of IR codes for the AV device specified by the input parameters |
| [getLastKeypressSource](#method.getLastKeypressSource) | Returns last key press source data |
| [getIRDBManufacturers](#method.getIRDBManufacturers) | Returns a list of manufacturer names based on the specified input parameters |
| [getIRDBModels](#method.getIRDBModels) | Returns a list of model names based on the specified input parameters |
| [getNetStatus](#method.getNetStatus) | Returns the status information provided by the last `onStatus` event for the specified network |
| [startPairing](#method.startPairing) | Initiates pairing a remote with the STB on the specified network |
| [setIRCode](#method.setIRCode) | Programs an IR code into the specified remote control |
| [initializeIRDB](#method.initializeIRDB) | Initializes the IR database |


<a name="method.clearIRCodes"></a>
## *clearIRCodes [<sup>method</sup>](#head.Methods)*

Clears the IR codes from the specified remote.
 
Events
 
| Event | Description | 
| :-------- | :-------- | 
| `onStatus` | Triggered if the IR codes cleared from remote on the specified network |.

Also see: [onStatus](#event.onStatus)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote ID of the target remote on the specified network |
| params.netType | integer | The type of network |

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
    "method": "org.rdk.RemoteControl.1.clearIRCodes",
    "params": {
        "remoteId": 1,
        "netType": 1
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

<a name="method.configureWakeupKeys"></a>
## *configureWakeupKeys [<sup>method</sup>](#head.Methods)*

Configures which keys on the remote will wake the target from deepsleep. 
  
### Events 

 No events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.netType | integer | The type of network.(currently on Bluetooth Low Energy network supports this feature) |
| params.wakeupConfig | string | The deepsleep wakeup key configuration of the remote. (possible values: all - all keys on the remote will wake target from deepsleep, none - no keys on the remote will wake target from deepsleep, custom - the custom list of Linux key codes provided in customKeys will wake target from deepsleep) (must be one of the following: *all*, *none*, *custom*) |
| params?.customKeys | string | <sup>*(optional)*</sup> Mandatory param only if wakeupConfig is custom, otherwise it should be omitted. This parameter is a list of linux keycodes that can wake the target from deepsleep |

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
    "method": "org.rdk.RemoteControl.1.configureWakeupKeys",
    "params": {
        "netType": 1,
        "wakeupConfig": "custom",
        "customKeys": "3,1"
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

<a name="method.getApiVersionNumber"></a>
## *getApiVersionNumber [<sup>method</sup>](#head.Methods)*

Gets the current API version number. 
  
### Events 

 No events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.version | integer | The API version number |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteControl.1.getApiVersionNumber"
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

<a name="method.getIRCodesByAutoLookup"></a>
## *getIRCodesByAutoLookup [<sup>method</sup>](#head.Methods)*

Returns a list of available IR codes for the TV and AVRs specified by the input parameters. 
  
### Events 

 No events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.netType | integer | The type of network |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.tvcodes | array | <sup>*(optional)*</sup> A list of TV IR codes |
| result?.tvcodes[#] | string | <sup>*(optional)*</sup>  |
| result?.avrcodes | array | <sup>*(optional)*</sup> A list of AVR IR codes |
| result?.avrcodes[#] | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteControl.1.getIRCodesByAutoLookup",
    "params": {
        "netType": 1
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "tvcodes": [
            "1156"
        ],
        "avrcodes": [
            "R2467"
        ],
        "success": true
    }
}
```

<a name="method.getIRCodesByNames"></a>
## *getIRCodesByNames [<sup>method</sup>](#head.Methods)*

Returns a list of IR codes for the AV device specified by the input parameters. 
  
### Events 

 No events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.netType | integer | The type of network |
| params.avDevType | string | Whether the device is a video (TV) or audio (AVR, amplifier, or soundbar) device (must be one of the following: *TV*, *AMP*) |
| params.manufacturer | string | The manufacturer name of the AV device |
| params.model | string | The model name of the AV device |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.avDevType | string | Whether the device is a video (TV) or audio (AVR, amplifier, or soundbar) device (must be one of the following: *TV*, *AMP*) |
| result.manufacturer | string | The manufacturer name of the AV device |
| result.model | string | The model name of the AV device |
| result.codes | array | A list of IR codes |
| result.codes[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteControl.1.getIRCodesByNames",
    "params": {
        "netType": 1,
        "avDevType": "AMP",
        "manufacturer": "Samsung",
        "model": "AH5901068L"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "avDevType": "AMP",
        "manufacturer": "Samsung",
        "model": "AH5901068L",
        "codes": [
            "R2467"
        ],
        "success": true
    }
}
```

<a name="method.getLastKeypressSource"></a>
## *getLastKeypressSource [<sup>method</sup>](#head.Methods)*

Returns last key press source data. 
  
### Events 

 No events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.netType | integer | The type of network |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.controllerId | integer | The controller ID of the target remote on the specified network |
| result.timestamp | integer | The time of the last key press |
| result.sourceName | string | The source of the last key press |
| result.sourceType | string | The source type of the last key press |
| result.sourceKeyCode | integer | The source key code |
| result.bIsScreenBindMode | boolean | `true` if in screen bind mode, otherwise `false` |
| result.remoteKeypadConfig | integer | The configuration of the remote keypad |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteControl.1.getLastKeypressSource",
    "params": {
        "netType": 1
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "controllerId": 1,
        "timestamp": 1621005572000,
        "sourceName": "XR15-10",
        "sourceType": "IR",
        "sourceKeyCode": 192,
        "bIsScreenBindMode": false,
        "remoteKeypadConfig": 1,
        "success": true
    }
}
```

<a name="method.getIRDBManufacturers"></a>
## *getIRDBManufacturers [<sup>method</sup>](#head.Methods)*

Returns a list of manufacturer names based on the specified input parameters. 
  
### Events 

 No events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.netType | integer | The type of network |
| params.avDevType | string | Whether the device is a video (TV) or audio (AVR, amplifier, or soundbar) device (must be one of the following: *TV*, *AMP*) |
| params.manufacturer | string | A part of the name of the manufacturer of the AV device |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.avDevType | string | Whether the device is a video (TV) or audio (AVR, amplifier, or soundbar) device (must be one of the following: *TV*, *AMP*) |
| result.manufacturers | array | A list of manufacturer names |
| result.manufacturers[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteControl.1.getIRDBManufacturers",
    "params": {
        "netType": 1,
        "avDevType": "AMP",
        "manufacturer": "Sa"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "avDevType": "AMP",
        "manufacturers": [
            "Samsung"
        ],
        "success": true
    }
}
```

<a name="method.getIRDBModels"></a>
## *getIRDBModels [<sup>method</sup>](#head.Methods)*

Returns a list of model names based on the specified input parameters. 
  
### Events 

 No events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.netType | integer | The type of network |
| params.avDevType | string | Whether the device is a video (TV) or audio (AVR, amplifier, or soundbar) device (must be one of the following: *TV*, *AMP*) |
| params.manufacturer | string | The manufacturer name of the AV device |
| params.model | string | A part (minimum of 3 characters) of the model name of the AV device |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.avDevType | string | Whether the device is a video (TV) or audio (AVR, amplifier, or soundbar) device (must be one of the following: *TV*, *AMP*) |
| result.manufacturer | string | The manufacturer name of the AV device |
| result.models | array | A list of model names |
| result.models[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteControl.1.getIRDBModels",
    "params": {
        "netType": 1,
        "avDevType": "AMP",
        "manufacturer": "Samsung",
        "model": "AH5"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "avDevType": "AMP",
        "manufacturer": "Samsung",
        "models": [
            "AH5901068L"
        ],
        "success": true
    }
}
```

<a name="method.getNetStatus"></a>
## *getNetStatus [<sup>method</sup>](#head.Methods)*

Returns the status information provided by the last `onStatus` event for the specified network. 
  
### Events 

 No events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.netType | integer | The type of network |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status | object |  |
| result.status.netType | integer | The type of network |
| result.status.netTypeSupported | array | a list of the network types that the STB supports, Note that some devices may only support one network type |
| result.status.netTypeSupported[#] | integer |  |
| result.status.pairingState | string | The current overall pairing state of the specified network: `INITIALISING` - starting up, no paired remotes, `IDLE` - no activity, `SEARCHING` - searching for remotes, `PAIRING` - paring to a remote, `COMPLETE` - successfully paired to a remote, `FAILED` - failed to find or pair to a remote (must be one of the following: *INITIALISING*, *IDLE*, *SEARCHING*, *PAIRING*, *COMPLETE*, *FAILED*) |
| result.status.remoteData | array | Remote information for each paired remote control. This array may be empty, if there are no paired remotes on the specified network |
| result.status.remoteData[#] | object |  |
| result.status.remoteData[#].macAddress | string | The MAC address of the remote in hex-colon format |
| result.status.remoteData[#].connected | boolean | `true` if the remote is connected, otherwise `false` |
| result.status.remoteData[#]?.name | string | <sup>*(optional)*</sup> The remote name |
| result.status.remoteData[#].remoteId | integer | This integer is the remote ID number, assigned by the network |
| result.status.remoteData[#]?.deviceId | integer | <sup>*(optional)*</sup> The device ID number that is assigned by the network |
| result.status.remoteData[#]?.make | string | <sup>*(optional)*</sup> The manufacturer name of the remote |
| result.status.remoteData[#]?.model | string | <sup>*(optional)*</sup> The remote model name |
| result.status.remoteData[#]?.hwVersion | string | <sup>*(optional)*</sup> The remote hardware revision |
| result.status.remoteData[#]?.swVersion | string | <sup>*(optional)*</sup> The remote software revision |
| result.status.remoteData[#]?.btlVersion | string | <sup>*(optional)*</sup> The remote bootloader revision |
| result.status.remoteData[#]?.serialNumber | string | <sup>*(optional)*</sup> The remote serial number |
| result.status.remoteData[#]?.batteryPercent | integer | <sup>*(optional)*</sup> The current remote battery level as a percentage (0 to 100) |
| result.status.remoteData[#]?.tvIRCode | string | <sup>*(optional)*</sup> The current TV IR code that the remote is programmed with. If the remote is not presently programmed with an IR code or if the TV IR code is not known, then this value is not returned |
| result.status.remoteData[#]?.ampIRCode | string | <sup>*(optional)*</sup> The current AVR/AMP IR code that the remote is programmed with. If the remote is not presently programmed with an IR code or if the AVR/AMP IR code is unknown, then this value is not returned |
| result.status.remoteData[#]?.wakeupKeyCode | integer | <sup>*(optional)*</sup> The Linux key code of the last button to be pressed on the remote before wakeup from deepsleep |
| result.status.remoteData[#]?.wakeupConfig | string | <sup>*(optional)*</sup> The current deepsleep wakeup key configuration of the remote. (possible values: all - all keys on the remote will wake target from deepsleep, none - no keys on the remote will wake target from deepsleep, custom - the custom list of Linux key codes provided in wakeupCustomList will wake target from deepsleep) (must be one of the following: *all*, *none*, *custom*) |
| result.status.remoteData[#]?.wakeupCustomList | string | <sup>*(optional)*</sup> will only be present if wakeupConfig is custom, this parameter will list the linux keycodes that can wake the target from deepsleep |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteControl.1.getNetStatus",
    "params": {
        "netType": 1
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "status": {
            "netType": 1,
            "netTypeSupported": [
                0
            ],
            "pairingState": "COMPLETE",
            "remoteData": [
                {
                    "macAddress": "E8:1C:FD:9A:07:1E",
                    "connected": true,
                    "name": "P073 SkyQ EC201",
                    "remoteId": 1,
                    "deviceId": 65,
                    "make": "Omni Remotes",
                    "model": "EC201",
                    "hwVersion": "201.2.0.0",
                    "swVersion": "1.0.0",
                    "btlVersion": "2.0",
                    "serialNumber": "18464408B544",
                    "batteryPercent": 82,
                    "tvIRCode": "1",
                    "ampIRCode": "1",
                    "wakeupKeyCode": 65,
                    "wakeupConfig": "custom",
                    "wakeupCustomList": "[3,1]"
                }
            ]
        },
        "success": true
    }
}
```

<a name="method.startPairing"></a>
## *startPairing [<sup>method</sup>](#head.Methods)*

Initiates pairing a remote with the STB on the specified network. 
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onStatus` | Triggered if pairing starts on the specified network |.

Also see: [onStatus](#event.onStatus)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.netType | integer | The type of network |
| params?.timeout | integer | <sup>*(optional)*</sup> The amount of time, in seconds, to attempt pairing before timing out. If this parameter is not present, an STB-defined default is used for the specified network. A value of `0` indicates no timeout |

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
    "method": "org.rdk.RemoteControl.1.startPairing",
    "params": {
        "netType": 1,
        "timeout": 30
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

<a name="method.setIRCode"></a>
## *setIRCode [<sup>method</sup>](#head.Methods)*

Programs an IR code into the specified remote control.
 
Events
 
| Event | Description | 
| :-------- | :-------- | 
| `onStatus` | Triggered if the IR codes sets into remote on the specified network|.

Also see: [onStatus](#event.onStatus)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.remoteId | integer | The remote ID of the target remote on the specified network |
| params.netType | integer | The type of network |
| params.avDevType | string | Whether the device is a video (TV) or audio (AVR, amplifier, or soundbar) device (must be one of the following: *TV*, *AMP*) |
| params.code | string | The IR code being programmed into the remote |

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
    "method": "org.rdk.RemoteControl.1.setIRCode",
    "params": {
        "remoteId": 1,
        "netType": 1,
        "avDevType": "AMP",
        "code": "M3320"
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

<a name="method.initializeIRDB"></a>
## *initializeIRDB [<sup>method</sup>](#head.Methods)*

Initializes the IR database.
  
### Events 

 No events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.netType | integer | The type of network |

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
    "method": "org.rdk.RemoteControl.1.initializeIRDB",
    "params": {
        "netType": 1
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

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.RemoteControl plugin:

RemoteControl interface events:

| Event | Description |
| :-------- | :-------- |
| [onStatus](#event.onStatus) | Triggered at any time when the status of any one of the supported STB remote networks changes |


<a name="event.onStatus"></a>
## *onStatus [<sup>event</sup>](#head.Notifications)*

Triggered at any time when the status of any one of the supported STB remote networks changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.status | object |  |
| params.status.netType | integer | The type of network |
| params.status.netTypeSupported | array | a list of the network types that the STB supports, Note that some devices may only support one network type |
| params.status.netTypeSupported[#] | integer |  |
| params.status.pairingState | string | The current overall pairing state of the specified network: `INITIALISING` - starting up, no paired remotes, `IDLE` - no activity, `SEARCHING` - searching for remotes, `PAIRING` - paring to a remote, `COMPLETE` - successfully paired to a remote, `FAILED` - failed to find or pair to a remote (must be one of the following: *INITIALISING*, *IDLE*, *SEARCHING*, *PAIRING*, *COMPLETE*, *FAILED*) |
| params.status.remoteData | array | Remote information for each paired remote control. This array may be empty, if there are no paired remotes on the specified network |
| params.status.remoteData[#] | object |  |
| params.status.remoteData[#].macAddress | string | The MAC address of the remote in hex-colon format |
| params.status.remoteData[#].connected | boolean | `true` if the remote is connected, otherwise `false` |
| params.status.remoteData[#]?.name | string | <sup>*(optional)*</sup> The remote name |
| params.status.remoteData[#].remoteId | integer | This integer is the remote ID number, assigned by the network |
| params.status.remoteData[#]?.deviceId | integer | <sup>*(optional)*</sup> The device ID number that is assigned by the network |
| params.status.remoteData[#]?.make | string | <sup>*(optional)*</sup> The manufacturer name of the remote |
| params.status.remoteData[#]?.model | string | <sup>*(optional)*</sup> The remote model name |
| params.status.remoteData[#]?.hwVersion | string | <sup>*(optional)*</sup> The remote hardware revision |
| params.status.remoteData[#]?.swVersion | string | <sup>*(optional)*</sup> The remote software revision |
| params.status.remoteData[#]?.btlVersion | string | <sup>*(optional)*</sup> The remote bootloader revision |
| params.status.remoteData[#]?.serialNumber | string | <sup>*(optional)*</sup> The remote serial number |
| params.status.remoteData[#]?.batteryPercent | integer | <sup>*(optional)*</sup> The current remote battery level as a percentage (0 to 100) |
| params.status.remoteData[#]?.tvIRCode | string | <sup>*(optional)*</sup> The current TV IR code that the remote is programmed with. If the remote is not presently programmed with an IR code or if the TV IR code is not known, then this value is not returned |
| params.status.remoteData[#]?.ampIRCode | string | <sup>*(optional)*</sup> The current AVR/AMP IR code that the remote is programmed with. If the remote is not presently programmed with an IR code or if the AVR/AMP IR code is unknown, then this value is not returned |
| params.status.remoteData[#]?.wakeupKeyCode | integer | <sup>*(optional)*</sup> The Linux key code of the last button to be pressed on the remote before wakeup from deepsleep |
| params.status.remoteData[#]?.wakeupConfig | string | <sup>*(optional)*</sup> The current deepsleep wakeup key configuration of the remote. (possible values: all - all keys on the remote will wake target from deepsleep, none - no keys on the remote will wake target from deepsleep, custom - the custom list of Linux key codes provided in wakeupCustomList will wake target from deepsleep) (must be one of the following: *all*, *none*, *custom*) |
| params.status.remoteData[#]?.wakeupCustomList | string | <sup>*(optional)*</sup> will only be present if wakeupConfig is custom, this parameter will list the linux keycodes that can wake the target from deepsleep |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onStatus",
    "params": {
        "status": {
            "netType": 1,
            "netTypeSupported": [
                0
            ],
            "pairingState": "COMPLETE",
            "remoteData": [
                {
                    "macAddress": "E8:1C:FD:9A:07:1E",
                    "connected": true,
                    "name": "P073 SkyQ EC201",
                    "remoteId": 1,
                    "deviceId": 65,
                    "make": "Omni Remotes",
                    "model": "EC201",
                    "hwVersion": "201.2.0.0",
                    "swVersion": "1.0.0",
                    "btlVersion": "2.0",
                    "serialNumber": "18464408B544",
                    "batteryPercent": 82,
                    "tvIRCode": "1",
                    "ampIRCode": "1",
                    "wakeupKeyCode": 65,
                    "wakeupConfig": "custom",
                    "wakeupCustomList": "[3,1]"
                }
            ]
        }
    }
}
```

