<!-- Generated automatically, DO NOT EDIT! -->
<a name="Warehouse_Plugin"></a>
# Warehouse Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.Warehouse plugin for Thunder framework.

### Table of Contents

- [Introduction](#Introduction)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Notifications](#Notifications)

<a name="Introduction"></a>
# Introduction

<a name="Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.Warehouse plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

<a name="Case_Sensitivity"></a>
## Case Sensitivity

All identifiers of the interfaces described in this document are case-sensitive. Thus, unless stated otherwise, all keywords, entities, properties, relations and actions should be treated as such.

<a name="Acronyms,_Abbreviations_and_Terms"></a>
## Acronyms, Abbreviations and Terms

The table below provides and overview of acronyms used in this document and their definitions.

| Acronym | Description |
| :-------- | :-------- |
| <a name="API">API</a> | Application Programming Interface |
| <a name="HTTP">HTTP</a> | Hypertext Transfer Protocol |
| <a name="JSON">JSON</a> | JavaScript Object Notation; a data interchange format |
| <a name="JSON-RPC">JSON-RPC</a> | A remote procedure call protocol encoded in JSON |

The table below provides and overview of terms and abbreviations used in this document and their definitions.

| Term | Description |
| :-------- | :-------- |
| <a name="callsign">callsign</a> | The name given to an instance of a plugin. One plugin can be instantiated multiple times, but each instance the instance name, callsign, must be unique. |

<a name="References"></a>
## References

| Ref ID | Description |
| :-------- | :-------- |
| <a name="HTTP">[HTTP](http://www.w3.org/Protocols)</a> | HTTP specification |
| <a name="JSON-RPC">[JSON-RPC](https://www.jsonrpc.org/specification)</a> | JSON-RPC 2.0 specification |
| <a name="JSON">[JSON](http://www.json.org/)</a> | JSON specification |
| <a name="Thunder">[Thunder](https://github.com/WebPlatformForEmbedded/Thunder/blob/master/doc/WPE%20-%20API%20-%20WPEFramework.docx)</a> | Thunder API Reference |

<a name="Description"></a>
# Description

The `Warehouse` plugin performs various types of resets (data, warehouse, etc.).

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.Warehouse*) |
| classname | string | Class name: *org.rdk.Warehouse* |
| locator | string | Library name: *libWPEFrameworkWarehouse.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.Warehouse plugin:

Warehouse interface methods:

| Method | Description |
| :-------- | :-------- |
| [executeHardwareTest](#executeHardwareTest) | (Version 2) Starts a hardware test on the device |
| [getDeviceInfo](#getDeviceInfo) | Returns STB device information gathered from `/lib/rdk/getDeviceDetails |
| [getHardwareTestResults](#getHardwareTestResults) | (Version 2) Returns the results of the last hardware test |
| [internalReset](#internalReset) | Invokes the internal reset script, which reboots the Warehouse service (`/rebootNow |
| [isClean](#isClean) | Checks the locations on the device where customer data may be stored |
| [lightReset](#lightReset) | Resets the application data |
| [resetDevice](#resetDevice) | Resets the STB to the warehouse state |
| [setFrontPanelState](#setFrontPanelState) | Sets the state of the front panel LEDs to indicate the download state of the STB software image |


<a name="executeHardwareTest"></a>
## *executeHardwareTest*

(Version 2) Starts a hardware test on the device. See `getHardwareTestResults`. 
 
### Events
 
 No Events.

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
    "method": "org.rdk.Warehouse.1.executeHardwareTest"
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

<a name="getDeviceInfo"></a>
## *getDeviceInfo*

Returns STB device information gathered from `/lib/rdk/getDeviceDetails.sh`.(DEPRECATED - Use `getDeviceInfo` from `org.rdk.System` instead.) 
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.bluetooth_mac | string | The bluetooth MAC address |
| result.boxIP | string | The device IP address |
| result.build_type | string | The device build type |
| result.estb_mac | string | The embedded set-top box MAC address |
| result.eth_mac | string | The Ethernet MAC address |
| result.imageVersion | string | The build image version |
| result.version | string | The version |
| result.software_version | string | The software version |
| result.model_number | string | The device model number |
| result.rf4ce_mac | string | The Radio Frequency for Consumer Electronics MAC address |
| result.wifi_mac | string | The Wifi address |
| result.success | boolean | Whether the request succeeded |
| result?.error | string | <sup>*(optional)*</sup> An error message in case of a failure |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Warehouse.1.getDeviceInfo"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
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
        "wifi_mac": "A8:11:XX:FD:0C:XX",
        "success": true,
        "error": "..."
    }
}
```

<a name="getHardwareTestResults"></a>
## *getHardwareTestResults*

(Version 2) Returns the results of the last hardware test. 
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.testResults | string | The test results |

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
    "method": "org.rdk.Warehouse.1.getHardwareTestResults",
    "params": {
        "testResults": "Timezone: NA 2021-04-15 10:35:06 Test execution start, remote trigger ver. 0011 2021-04-15 10:35:10 Test result: Audio/Video Decoder:PASSED 2021-04-15 10:35:06 Test result: Dynamic RAM:PASSED 2021-04-15 10:35:06 Test result: Flash Memory:PASSED 2021-04-15 10:35:06 Test result: HDMI Output:PASSED 2021-04-15 10:35:38 Test result: IR Remote Interface:WARNING_IR_Not_Detected 2021-04-15 10:35:06 Test result: Bluetooth:PASSED 2021-04-15 10:35:06 Test result: SD Card:PASSED 2021-04-15 10:35:06 Test result: WAN:PASSED 2021-04-15 10:35:38 Test execution completed:PASSED"
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

<a name="internalReset"></a>
## *internalReset*

Invokes the internal reset script, which reboots the Warehouse service (`/rebootNow.sh -s WarehouseService &`). Note that this method checks the `/version.txt` file for the image name and fails to run if the STB image version is marked as production (`PROD`). 
 
### Events
 
 No Events.

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
    "id": 42,
    "method": "org.rdk.Warehouse.1.internalReset",
    "params": {
        "passPhrase": "FOR TEST PURPOSES ONLY"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true,
        "error": "..."
    }
}
```

<a name="isClean"></a>
## *isClean*

Checks the locations on the device where customer data may be stored. If there are contents contained in those folders, then the device is not clean. 
 
### Events
 
 No Events.

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
    "id": 42,
    "method": "org.rdk.Warehouse.1.isClean"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "clean": false,
        "files": [
            "/opt/ctrlm.sql"
        ],
        "success": true
    }
}
```

<a name="lightReset"></a>
## *lightReset*

Resets the application data. 
 
### Events
 
 No Events.

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
    "id": 42,
    "method": "org.rdk.Warehouse.1.lightReset"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true,
        "error": "..."
    }
}
```

<a name="resetDevice"></a>
## *resetDevice*

Resets the STB to the warehouse state. 
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `resetDone` | Triggers when the device reset is finished indicating a successful reset or failure|.

Also see: [resetDone](#resetDone)

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
| result.success | boolean | Whether the request succeeded |
| result?.error | string | <sup>*(optional)*</sup> An error message in case of a failure |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
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
    "id": 42,
    "result": {
        "success": true,
        "error": "..."
    }
}
```

<a name="setFrontPanelState"></a>
## *setFrontPanelState*

Sets the state of the front panel LEDs to indicate the download state of the STB software image. 
 
### Events
 
 No Events.

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
    "id": 42,
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
    "id": 42,
    "result": {
        "success": true,
        "error": "..."
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.Warehouse plugin:

Warehouse interface events:

| Event | Description |
| :-------- | :-------- |
| [resetDone](#resetDone) | Notifies subscribers about the status of the warehouse reset operation |


<a name="resetDone"></a>
## *resetDone*

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
        "error": "..."
    }
}
```

