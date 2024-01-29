<!-- Generated automatically, DO NOT EDIT! -->
<a name="RemoteActionMapping_Plugin"></a>
# RemoteActionMapping Plugin

**Version: [1.0.6](https://github.com/rdkcentral/rdkservices/blob/main/RemoteActionMapping/CHANGELOG.md)**

A org.rdk.RemoteActionMapping plugin for Thunder framework.

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

The `RemoteActionMapping` plugin controls and maps remote controllers.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.RemoteActionMapping*) |
| classname | string | Class name: *org.rdk.RemoteActionMapping* |
| locator | string | Library name: *libWPEFrameworkRemoteActionMapping.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.RemoteActionMapping plugin:

RemoteActionMapping interface methods:

| Method | Description |
| :-------- | :-------- |
| [cancelCodeDownload](#cancelCodeDownload) | Cancels downloading IR and five digit codes from the IRRF database |
| [clearKeyActionMapping](#clearKeyActionMapping) | Clears an action mapping for the specified keys |
| [getApiVersionNumber](#getApiVersionNumber) | Returns the API version number |
| [getFullKeyActionMapping](#getFullKeyActionMapping) | Returns the mapping of all action keys |
| [getKeymap](#getKeymap) | Returns a hard-coded list of key names |
| [getLastUsedDeviceID](#getLastUsedDeviceID) | Returns the last used remote information |
| [getSingleKeyActionMapping](#getSingleKeyActionMapping) | Returns the mapping for a single action key |
| [setFiveDigitCode](#setFiveDigitCode) | Sets the TV and AVR five digit code |
| [setKeyActionMapping](#setKeyActionMapping) | Sets the mapping of a single action key |


<a name="cancelCodeDownload"></a>
## *cancelCodeDownload*

Cancels downloading IR and five digit codes from the IRRF database.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | integer | An ID in the range of `0` - `_CTRLM_RCU_IARM_BUS_API_REVISION_` |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status_code | integer | Returns `0` if no error has occurred |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteActionMapping.cancelCodeDownload",
    "params": {
        "deviceID": 1
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "status_code": 0,
        "success": true
    }
}
```

<a name="clearKeyActionMapping"></a>
## *clearKeyActionMapping*

Clears an action mapping for the specified keys.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | integer | An ID in the range of `0` - `_CTRLM_RCU_IARM_BUS_API_REVISION_` |
| params.keymapType | integer | The keymap type (must be one of the following: *0*, *1*) |
| params.keyNames | array | The remote key name |
| params.keyNames[#] | integer |  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.keyNames | array | The remote key name |
| result.keyNames[#] | integer |  |
| result.status_code | integer | Returns `0` if no error has occurred |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteActionMapping.clearKeyActionMapping",
    "params": {
        "deviceID": 1,
        "keymapType": 1,
        "keyNames": [
            2
        ]
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "keyNames": [
            2
        ],
        "status_code": 0,
        "success": true
    }
}
```

<a name="getApiVersionNumber"></a>
## *getApiVersionNumber*

Returns the API version number.

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
    "method": "org.rdk.RemoteActionMapping.getApiVersionNumber"
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

<a name="getFullKeyActionMapping"></a>
## *getFullKeyActionMapping*

Returns the mapping of all action keys.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | integer | An ID in the range of `0` - `_CTRLM_RCU_IARM_BUS_API_REVISION_` |
| params.keymapType | integer | The keymap type (must be one of the following: *0*, *1*) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.KeyMappings | array | <sup>*(optional)*</sup> An object [] that lists the mappings for each supported key |
| result?.KeyMappings[#] | object | <sup>*(optional)*</sup>  |
| result?.KeyMappings[#].keyName | integer | The name of a key |
| result?.KeyMappings[#].rfKeyCode | integer | The rf code of a particular key name. `rfKeyCodes` index corresponds to `keyNames` index. A value from 0-255 |
| result?.KeyMappings[#].tvIRKeyCode | array | TV IR code of a particular key name. A byte [] of length 12 |
| result?.KeyMappings[#].tvIRKeyCode[#] | integer |  |
| result?.KeyMappings[#].avrIRKeyCode | array | AVR IR code of a particular key name. A byte [] of length 12 |
| result?.KeyMappings[#].avrIRKeyCode[#] | integer |  |
| result.status_code | integer | Returns `0` if no error has occurred |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteActionMapping.getFullKeyActionMapping",
    "params": {
        "deviceID": 1,
        "keymapType": 1
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "KeyMappings": [
            {
                "keyName": 2,
                "rfKeyCode": 2,
                "tvIRKeyCode": [
                    20
                ],
                "avrIRKeyCode": [
                    18
                ]
            }
        ],
        "status_code": 0,
        "success": true
    }
}
```

<a name="getKeymap"></a>
## *getKeymap*

Returns a hard-coded list of key names.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | integer | An ID in the range of `0` - `_CTRLM_RCU_IARM_BUS_API_REVISION_` |
| params.keymapType | integer | The keymap type (must be one of the following: *0*, *1*) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.keyNames | array | The remote key name |
| result.keyNames[#] | integer |  |
| result.status_code | integer | Returns `0` if no error has occurred |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteActionMapping.getKeymap",
    "params": {
        "deviceID": 1,
        "keymapType": 1
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "keyNames": [
            2
        ],
        "status_code": 0,
        "success": true
    }
}
```

<a name="getLastUsedDeviceID"></a>
## *getLastUsedDeviceID*

Returns the last used remote information.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.deviceID | integer | An ID in the range of `0` - `_CTRLM_RCU_IARM_BUS_API_REVISION_` |
| result.remoteType | string | The remote type as returned by `CTRLM_MAIN_IARM_CALL_NETWORK_STATUS_GET` |
| result.fiveDigitCodePresent | boolean | Returns `true` if the remote has the TV's five digits code saved |
| result.setFiveDigitCodeSupported | boolean | Returns `true` if five digit code support is enabled |
| result.status_code | integer | Returns `0` if no error has occurred |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteActionMapping.getLastUsedDeviceID"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "deviceID": 1,
        "remoteType": "...",
        "fiveDigitCodePresent": true,
        "setFiveDigitCodeSupported": true,
        "status_code": 0,
        "success": true
    }
}
```

<a name="getSingleKeyActionMapping"></a>
## *getSingleKeyActionMapping*

Returns the mapping for a single action key.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | integer | An ID in the range of `0` - `_CTRLM_RCU_IARM_BUS_API_REVISION_` |
| params.keymapType | integer | The keymap type (must be one of the following: *0*, *1*) |
| params.keyName | integer | The name of a key |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.KeyMapping | object | <sup>*(optional)*</sup> The mapping for the specified key |
| result?.KeyMapping.keyName | integer | The name of a key |
| result?.KeyMapping.rfKeyCode | integer | The rf code of a particular key name. `rfKeyCodes` index corresponds to `keyNames` index. A value from 0-255 |
| result?.KeyMapping.tvIRKeyCode | array | TV IR code of a particular key name. A byte [] of length 12 |
| result?.KeyMapping.tvIRKeyCode[#] | integer |  |
| result?.KeyMapping.avrIRKeyCode | array | AVR IR code of a particular key name. A byte [] of length 12 |
| result?.KeyMapping.avrIRKeyCode[#] | integer |  |
| result.status_code | integer | Returns `0` if no error has occurred |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteActionMapping.getSingleKeyActionMapping",
    "params": {
        "deviceID": 1,
        "keymapType": 1,
        "keyName": 2
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "KeyMapping": {
            "keyName": 2,
            "rfKeyCode": 2,
            "tvIRKeyCode": [
                20
            ],
            "avrIRKeyCode": [
                18
            ]
        },
        "status_code": 0,
        "success": true
    }
}
```

<a name="setFiveDigitCode"></a>
## *setFiveDigitCode*

Sets the TV and AVR five digit code.

### Events

| Event | Description |
| :-------- | :-------- |
| [onFiveDigitCodeLoad](#onFiveDigitCodeLoad) | Triggered if new five digit codes are loaded successfully |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | integer | An ID in the range of `0` - `_CTRLM_RCU_IARM_BUS_API_REVISION_` |
| params.tvFiveDigitCode | integer | Five digit code in the range of 0 - 99999 |
| params.avrFiveDigitCode | integer | Five digit code in the range of 0 - 99999 |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status_code | integer | Returns `0` if no error has occurred |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteActionMapping.setFiveDigitCode",
    "params": {
        "deviceID": 1,
        "tvFiveDigitCode": 12345,
        "avrFiveDigitCode": 56789
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "status_code": 0,
        "success": true
    }
}
```

<a name="setKeyActionMapping"></a>
## *setKeyActionMapping*

Sets the mapping of a single action key. This method is unavailable (returns error) if the remote supports 5 digit codes.

### Events

| Event | Description |
| :-------- | :-------- |
| [onIRCodeLoad](#onIRCodeLoad) | Triggered if new IR codes are loaded successfully |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | integer | An ID in the range of `0` - `_CTRLM_RCU_IARM_BUS_API_REVISION_` |
| params.keymapType | integer | The keymap type (must be one of the following: *0*, *1*) |
| params.KeyActionMapping | array | An object [] that lists the mappings for the specified key. The length has to be in the range [ 1, 7 ] |
| params.KeyActionMapping[#] | object |  |
| params.KeyActionMapping[#].keyName | integer | The name of a key |
| params.KeyActionMapping[#].rfKeyCode | integer | The rf code of a particular key name. `rfKeyCodes` index corresponds to `keyNames` index. A value from 0-255 |
| params.KeyActionMapping[#].tvIRKeyCode | array | TV IR code of a particular key name. A byte [] of length 12 |
| params.KeyActionMapping[#].tvIRKeyCode[#] | integer |  |
| params.KeyActionMapping[#].avrIRKeyCode | array | AVR IR code of a particular key name. A byte [] of length 12 |
| params.KeyActionMapping[#].avrIRKeyCode[#] | integer |  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.status_code | integer | Returns `0` if no error has occurred |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.RemoteActionMapping.setKeyActionMapping",
    "params": {
        "deviceID": 1,
        "keymapType": 1,
        "KeyActionMapping": [
            {
                "keyName": 2,
                "rfKeyCode": 2,
                "tvIRKeyCode": [
                    20
                ],
                "avrIRKeyCode": [
                    18
                ]
            }
        ]
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "status_code": 0,
        "success": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.RemoteActionMapping plugin:

RemoteActionMapping interface events:

| Event | Description |
| :-------- | :-------- |
| [onFiveDigitCodeLoad](#onFiveDigitCodeLoad) | Triggered when new five digit codes are loaded |
| [onIRCodeLoad](#onIRCodeLoad) | Triggered when new IR codes are loaded |


<a name="onFiveDigitCodeLoad"></a>
## *onFiveDigitCodeLoad*

Triggered when new five digit codes are loaded.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | integer | An ID in the range of `0` - `_CTRLM_RCU_IARM_BUS_API_REVISION_` |
| params.tvLoadStatus | integer | Status of the TV five digit code loaded |
| params.avrLoadStatus | integer | Status of the AVR five digit code loaded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onFiveDigitCodeLoad",
    "params": {
        "deviceID": 1,
        "tvLoadStatus": 0,
        "avrLoadStatus": 0
    }
}
```

<a name="onIRCodeLoad"></a>
## *onIRCodeLoad*

Triggered when new IR codes are loaded.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.deviceID | integer | An ID in the range of `0` - `_CTRLM_RCU_IARM_BUS_API_REVISION_` |
| params.keyNames | array | The remote key name |
| params.keyNames[#] | integer |  |
| params.rfKeyCodes | array | The RF code of a particular key name. `rfKeyCodes` index corresponds to `keyNames` index |
| params.rfKeyCodes[#] | integer |  |
| params.loadStatus | integer | Operation status |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onIRCodeLoad",
    "params": {
        "deviceID": 1,
        "keyNames": [
            2
        ],
        "rfKeyCodes": [
            214
        ],
        "loadStatus": 0
    }
}
```

