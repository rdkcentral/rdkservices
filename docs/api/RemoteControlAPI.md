<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.RemoteControl_API"></a>
# RemoteControl API

**Version: [1.1.0](https://github.com/rdkcentral/rdkservices/blob/main/RemoteControl/CHANGELOG.md)**

A RemoteControl plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#head.Abbreviation,_Acronyms_and_Terms)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)
- [Notifications](#head.Notifications)

<a name="head.Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="head.Description"></a>
# Description

The `RemoteControl` plugin provides the ability to pair and IR-program remote controls.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| classname | string | Class name: *RemoteControl* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the RemoteControl plugin:

RemoteControl interface methods:

| Method | Description |
| :-------- | :-------- |
| [getApiVersionNumber](#method.getApiVersionNumber) | Gets the current API version number |
| [getNetStatus](#method.getNetStatus) | Returns the status information provided by the last `onStatus` event for the specified network |
| [startPairing](#method.startPairing) | Initiates pairing a remote with the STB on the specified network |
| [initializeIRDB](#method.initializeIRDB) | Initializes the IR database |
| [clearIRCodes](#method.clearIRCodes) | Clears the IR codes from the specified remote |
| [setIRCode](#method.setIRCode) | Programs an IR code into the specified remote control |
| [getIRCodesByAutoLookup](#method.getIRCodesByAutoLookup) | Returns a list of available IR codes for the TV and AVRs specified by the input parameters |
| [getIRCodesByNames](#method.getIRCodesByNames) | Returns a list of IR codes for the AV device specified by the input parameters |
| [getIRDBManufacturers](#method.getIRDBManufacturers) | Returns a list of manufacturer names based on the specified input parameters |
| [getIRDBModels](#method.getIRDBModels) | Returns a list of model names based on the specified input parameters |
| [getLastKeypressSource](#method.getLastKeypressSource) | Returns last key press source data |
| [configureWakeupKeys](#method.configureWakeupKeys) | Configures which keys on the remote will wake the target from deepsleep |
| [findMyRemote](#method.findMyRemote) | Tells the most recently used remote to beep |
| [factoryReset](#method.factoryReset) | Tells all paired and connected remotes to factory reset |


<a name="method.getApiVersionNumber"></a>
## *getApiVersionNumber [<sup>method</sup>](#head.Methods)*

Gets the current API version number. 
  
### Events 

 No events.

### Events

No Events

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
    "method": "RemoteControl.getApiVersionNumber"
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

<a name="method.getNetStatus"></a>
## *getNetStatus [<sup>method</sup>](#head.Methods)*

Returns the status information provided by the last `onStatus` event for the specified network. 
  
### Events 

 No events.

### Events

No Events

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
| result.status.pairingState | string | The current overall pairing state of the specified network (must be one of the following: *INITIALISING*, *IDLE*, *SEARCHING*, *PAIRING*, *COMPLETE*, *FAILED*) |
| result.status.irProgState | string | The current state of the IR code programming request to the remote (must be one of the following: *IDLE*, *WAITING*, *COMPLETE*, *FAILED*) |
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
    "method": "RemoteControl.getNetStatus",
    "params": {
        "netType": 21
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
            "netType": 21,
            "netTypeSupported": [
                [
                    21
                ]
            ],
            "pairingState": "COMPLETE",
            "irProgState": "COMPLETE",
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

### Events

| Event | Description |
| :-------- | :-------- |
| [onStatus](#event.onStatus) | pairingState will be updated to relect the current status of the request, along with updated remoteData upon a successful pairing. |
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
    "method": "RemoteControl.startPairing",
    "params": {
        "netType": 21,
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

<a name="method.initializeIRDB"></a>
## *initializeIRDB [<sup>method</sup>](#head.Methods)*

Initializes the IR database.
  
### Events 

 No events.

### Events

No Events

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
    "method": "RemoteControl.initializeIRDB",
    "params": {
        "netType": 21
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

<a name="method.clearIRCodes"></a>
## *clearIRCodes [<sup>method</sup>](#head.Methods)*

Clears the IR codes from the specified remote.
 
Events
 
| Event | Description | 
| :-------- | :-------- | 
| `onStatus` | Triggered if the IR codes cleared from remote on the specified network |.

### Events

| Event | Description |
| :-------- | :-------- |
| [onStatus](#event.onStatus) | irProgState will be updated to relect the current status of the request |
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
    "method": "RemoteControl.clearIRCodes",
    "params": {
        "remoteId": 1,
        "netType": 21
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

### Events

| Event | Description |
| :-------- | :-------- |
| [onStatus](#event.onStatus) | irProgState will be updated to relect the current status of the request |
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
    "method": "RemoteControl.setIRCode",
    "params": {
        "remoteId": 1,
        "netType": 21,
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

<a name="method.getIRCodesByAutoLookup"></a>
## *getIRCodesByAutoLookup [<sup>method</sup>](#head.Methods)*

Returns a list of available IR codes for the TV and AVRs specified by the input parameters. 
  
### Events 

 No events.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.netType | integer | The type of network |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.tvManufacturer | string | <sup>*(optional)*</sup> The TV manufacturer for which codes are provided |
| result?.tvModel | string | <sup>*(optional)*</sup> The TV model for which codes are provided (this is a best guess and might not be completely accurate) |
| result.tvCodes | array | A list of TV IR codes |
| result.tvCodes[#] | string |  |
| result?.avrManufacturer | string | <sup>*(optional)*</sup> The AVR manufacturer for which codes are provided |
| result?.avrModel | string | <sup>*(optional)*</sup> The AVR model for which codes are provided (this is a best guess and might not be completely accurate) |
| result.avrCodes | array | A list of AVR IR codes |
| result.avrCodes[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "RemoteControl.getIRCodesByAutoLookup",
    "params": {
        "netType": 21
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "tvManufacturer": "Samsung",
        "tvModel": "AH5",
        "tvCodes": [
            "1156"
        ],
        "avrManufacturer": "Samsung",
        "avrModel": "AH5",
        "avrCodes": [
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

### Events

No Events

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
    "method": "RemoteControl.getIRCodesByNames",
    "params": {
        "netType": 21,
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

<a name="method.getIRDBManufacturers"></a>
## *getIRDBManufacturers [<sup>method</sup>](#head.Methods)*

Returns a list of manufacturer names based on the specified input parameters. 
  
### Events 

 No events.

### Events

No Events

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
    "method": "RemoteControl.getIRDBManufacturers",
    "params": {
        "netType": 21,
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

### Events

No Events

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
    "method": "RemoteControl.getIRDBModels",
    "params": {
        "netType": 21,
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

<a name="method.getLastKeypressSource"></a>
## *getLastKeypressSource [<sup>method</sup>](#head.Methods)*

Returns last key press source data. 
  
### Events 

 No events.

### Events

No Events

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
    "method": "RemoteControl.getLastKeypressSource",
    "params": {
        "netType": 21
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

<a name="method.configureWakeupKeys"></a>
## *configureWakeupKeys [<sup>method</sup>](#head.Methods)*

Configures which keys on the remote will wake the target from deepsleep. 
  
### Events 

 No events.

### Events

No Events

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
    "method": "RemoteControl.configureWakeupKeys",
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

<a name="method.findMyRemote"></a>
## *findMyRemote [<sup>method</sup>](#head.Methods)*

Tells the most recently used remote to beep.
  
### Events 

 No events.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.netType | integer | The type of network |
| params.level | string | The level at which the remote will beep (must be one of the following: *off*, *mid*, *high*) |

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
    "method": "RemoteControl.findMyRemote",
    "params": {
        "netType": 21,
        "level": "mid"
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

<a name="method.factoryReset"></a>
## *factoryReset [<sup>method</sup>](#head.Methods)*

Tells all paired and connected remotes to factory reset.
  
### Events 

 No events.

### Events

No Events

### Parameters

This method takes no parameters.

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
    "method": "RemoteControl.factoryReset"
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

The following events are provided by the RemoteControl plugin:

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
| params.status.pairingState | string | The current overall pairing state of the specified network (must be one of the following: *INITIALISING*, *IDLE*, *SEARCHING*, *PAIRING*, *COMPLETE*, *FAILED*) |
| params.status.irProgState | string | The current state of the IR code programming request to the remote (must be one of the following: *IDLE*, *WAITING*, *COMPLETE*, *FAILED*) |
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
    "method": "client.events.onStatus",
    "params": {
        "status": {
            "netType": 21,
            "netTypeSupported": [
                [
                    21
                ]
            ],
            "pairingState": "COMPLETE",
            "irProgState": "COMPLETE",
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

