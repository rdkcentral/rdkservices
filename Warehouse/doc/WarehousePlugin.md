<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.Warehouse_Plugin"></a>
# Warehouse Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.Warehouse plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.Warehouse plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `Warehouse` plugin performs various types of resets (data, warehouse, etc.).

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.Warehouse*) |
| classname | string | Class name: *org.rdk.Warehouse* |
| locator | string | Library name: *libWPEFrameworkWarehouse.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.Warehouse plugin:

Warehouse interface methods:

| Method | Description |
| :-------- | :-------- |
| [getDeviceInfo](#method.getDeviceInfo) | Returns STB device information gathered from `/lib/rdk/getDeviceDetails |
| [internalReset](#method.internalReset) | Invokes the internal reset script, which reboots the Warehouse service (`/rebootNow |
| [isClean](#method.isClean) | Checks the locations on the device where customer data may be stored |
| [lightReset](#method.lightReset) | Resets the application data |
| [resetDevice](#method.resetDevice) | Resets the STB to the warehouse state |
| [setFrontPanelState](#method.setFrontPanelState) | Sets the state of the front panel LEDs to indicate the download state of the STB software image |


<a name="method.getDeviceInfo"></a>
## *getDeviceInfo <sup>method</sup>*

Returns STB device information gathered from `/lib/rdk/getDeviceDetails.sh`.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.deviceInfo | object | A hash of properties containing device information |
| result.deviceInfo.bluetooth_mac | string | The bluetooth MAC address |
| result.deviceInfo.boxIP | string | The device IP address |
| result.deviceInfo.build_type | string | The device build type |
| result.deviceInfo.estb_mac | string | The embedded set-top box MAC address |
| result.deviceInfo.eth_mac | string | The Ethernet MAC address |
| result.deviceInfo.imageVersion | string | The build image version |
| result.deviceInfo.version | string | The version |
| result.deviceInfo.software_version | string | The software version |
| result.deviceInfo.model_number | string | The device model number |
| result.deviceInfo.rf4ce_mac | string | The Radio Frequency for Consumer Electronics MAC address |
| result.deviceInfo.wifi_mac | string | The Wifi address |
| result.PARAM_SUCCESS | boolean | Whether the parameter succeeded |
| result.success | boolean | Whether the request succeeded |
| result?.error | string | <sup>*(optional)*</sup> An error message in case of a failure |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Warehouse.1.getDeviceInfo"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "deviceInfo": {
            "bluetooth_mac": "A8:11:XX:FD:0C:XX",
            "boxIP": "192.168.0.1",
            "build_type": "VBN",
            "estb_mac": "A8:11:XX:FD:0C:XX",
            "eth_mac": "A8:11:XX:FD:0C:XX",
            "imageVersion": "AX061AEI_VBN_2003_sprint_20200507224225sdy",
            "version": "X061AEI_VBN_2003_sprint_20200507224225sdy",
            "software_version": "AX061AEI_VBN_2003_sprint_20200507224225sdy",
            "model_number": "AX061AEI",
            "rf4ce_mac": "00:15:5F:XX:20:5E:57:XX",
            "wifi_mac": "A8:11:XX:FD:0C:XX"
        },
        "PARAM_SUCCESS": true,
        "success": true,
        "error": ""
    }
}
```

<a name="method.internalReset"></a>
## *internalReset <sup>method</sup>*

Invokes the internal reset script, which reboots the Warehouse service (`/rebootNow.sh -s WarehouseService &`). Note that this method checks the `/version.txt` file for the image name and fails to run if the STB image version is marked as production (`PROD`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.passPhrase | string | The passphrase for running the internal reset (`FOR TEST PURPOSES ONLY`) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |
| result?.error | string | <sup>*(optional)*</sup> An error message in case of a failure |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Warehouse.1.internalReset",
    "params": {
        "passPhrase": "FOR TEST PURPOSES ONLY2"
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
        "error": ""
    }
}
```

<a name="method.isClean"></a>
## *isClean <sup>method</sup>*

Checks the locations on the device where customer data may be stored. If there are contents contained in those folders, then the device is not clean.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.clean | boolean | If `true`, then the device has no customer data, otherwise `false` |
| result.files | array | A string [] of file locations for each file that is found that should have been deleted in the cleaning process. If the `clean` property is `true`, then this array is empty or `null` |
| result.files[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Warehouse.1.isClean"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "clean": false,
        "files": [
            "/opt/ctrlm.sql"
        ],
        "success": true
    }
}
```

<a name="method.lightReset"></a>
## *lightReset <sup>method</sup>*

Resets the application data.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |
| result?.error | string | <sup>*(optional)*</sup> An error message in case of a failure |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Warehouse.1.lightReset"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "error": ""
    }
}
```

<a name="method.resetDevice"></a>
## *resetDevice <sup>method</sup>*

Resets the STB to the warehouse state. It dispatches a `resetDone` event notification indicating a successful reset or failure.

Also see: [resetDone](#event.resetDone)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.suppressReboot | boolean | if `true`, the STB should not be rebooted, otherwise `false`. Only the `WAREHOUSE` reset type supports suppressing the reboot |
| params.resetType | string | The reset type. If `resetType` is not specified, then `WAREHOUSE` is the default. (must be one of the following: *WAREHOUSE*, *FACTORY*, *USERFACTORY*, *COLDFACTORY*) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.PARAM_SUCCESS | boolean | Whether the parameter succeeded |
| result.success | boolean | Whether the request succeeded |
| result?.error | string | <sup>*(optional)*</sup> An error message in case of a failure |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Warehouse.1.resetDevice",
    "params": {
        "suppressReboot": true,
        "resetType": "WAREHOUSE"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "PARAM_SUCCESS": true,
        "success": true,
        "error": ""
    }
}
```

<a name="method.setFrontPanelState"></a>
## *setFrontPanelState <sup>method</sup>*

Sets the state of the front panel LEDs to indicate the download state of the STB software image.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.state | integer | The front panel state. Possible state values are: `-1` (NONE), `1` (DOWNLOAD IN PROGRESS), `3` (DOWNLOAD FAILED) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |
| result?.error | string | <sup>*(optional)*</sup> An error message in case of a failure |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Warehouse.1.setFrontPanelState",
    "params": {
        "state": 1
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
        "error": ""
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.Warehouse plugin:

Warehouse interface events:

| Event | Description |
| :-------- | :-------- |
| [resetDone](#event.resetDone) | Notifies subscribers about the status of the warehouse reset operation |


<a name="event.resetDone"></a>
## *resetDone <sup>event</sup>*

Notifies subscribers about the status of the warehouse reset operation.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.success | boolean | Whether the request succeeded |
| params?.error | string | <sup>*(optional)*</sup> An error message in case of a failure |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.resetDone",
    "params": {
        "success": true,
        "error": ""
    }
}
```

