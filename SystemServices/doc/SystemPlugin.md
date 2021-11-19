<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.System_Plugin"></a>
# System Plugin

**Version: 2.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.System plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Interfaces](#head.Interfaces)
- [Methods](#head.Methods)
- [Notifications](#head.Notifications)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.System plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `System` plugin is used to manage various system-level features such as power settings and firmware updates.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.System*) |
| classname | string | Class name: *org.rdk.System* |
| locator | string | Library name: *libWPEFrameworkSystemServices.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Interfaces"></a>
# Interfaces

This plugin implements the following interfaces:

- [SystemServices.json](https://github.com/rdkcentral/ThunderInterfaces/tree/master/interfaces/SystemServices.json)

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.System plugin:

SystemServices interface methods:

| Method | Description |
| :-------- | :-------- |
| [cacheContains](#method.cacheContains) | Checks if a key is present in the cache |
| [clearLastDeepSleepReason](#method.clearLastDeepSleepReason) | Clears the last deep sleep reason |
| [deletePersistentPath](#method.deletePersistentPath) | (Version 2) Deletes persistent path associated with a callsign |
| [enableMoca](#method.enableMoca) | Enables (or disable) Moca support for the platform |
| [fireFirmwarePendingReboot](#method.fireFirmwarePendingReboot) | (Version 2) Notifies the device about a pending reboot by sending an `onFirmwarePendingReboot` event |
| [getAvailableStandbyModes](#method.getAvailableStandbyModes) | Queries the available standby modes |
| [getCachedValue](#method.getCachedValue) | Gets the value of a key in the cache |
| [getCoreTemperature](#method.getCoreTemperature) | Returns the core temperature of the device |
| [getDeviceInfo](#method.getDeviceInfo) | Collects device details available from `/etc/device |
| [getDownloadedFirmwareInfo](#method.getDownloadedFirmwareInfo) | Returns information about firmware downloads |
| [getFirmwareDownloadPercent](#method.getFirmwareDownloadPercent) | Gets the current download percentage |
| [getFirmwareUpdateInfo](#method.getFirmwareUpdateInfo) | Returns information about a firmware update and includes whether or not an update is mandatory |
| [getFirmwareUpdateState](#method.getFirmwareUpdateState) | Checks the state of the firmware update |
| [getLastDeepSleepReason](#method.getLastDeepSleepReason) | Retrieves the last deep sleep reason |
| [getLastFirmwareFailureReason](#method.getLastFirmwareFailureReason) | (Version 2) Retrieves the last firmware failure reason |
| [getMacAddresses](#method.getMacAddresses) | Gets the MAC address of the device |
| [getMilestones](#method.getMilestones) | Returns the list of milestones |
| [getMode](#method.getMode) | Returns the currently set mode information |
| [getNetworkStandbyMode](#method.getNetworkStandbyMode) | Returns the network standby mode of the device |
| [getPlatformConfiguration](#method.getPlatformConfiguration) | (Version 2) Returns the supported features and device/account info |
| [getPowerState](#method.getPowerState) | Returns the power state of the device |
| [getPowerStateBeforeReboot](#method.getPowerStateBeforeReboot) | (Version 2) Returns the power state before reboot |
| [getPreferredStandbyMode](#method.getPreferredStandbyMode) | Returns the preferred standby mode |
| [getPreviousRebootInfo](#method.getPreviousRebootInfo) | Returns basic information about a reboot |
| [getPreviousRebootInfo2](#method.getPreviousRebootInfo2) | (Version 2) Returns detailed information about a reboot |
| [getPreviousRebootReason](#method.getPreviousRebootReason) | Returns the last reboot reason |
| [getRFCConfig](#method.getRFCConfig) | Returns information that is related to RDK Feature Control (RFC) configurations |
| [getSerialNumber](#method.getSerialNumber) | Returns the device serial number |
| [getStateInfo](#method.getStateInfo) | Queries device state information of various properties |
| [getSystemVersions](#method.getSystemVersions) | Returns system version details |
| [getTemperatureThresholds](#method.getTemperatureThresholds) | Returns temperature threshold values |
| [getTimeStatus](#method.getTimeStatus) | Returns the timer status |
| [getTimeZoneDST](#method.getTimeZoneDST) | Get the configured time zone from the file referenced by `TZ_FILE` |
| [getTimeZones](#method.getTimeZones) | (Version2) Gets the available timezones from the system's time zone database |
| [getWakeupReason](#method.getWakeupReason) | (Version 2) Returns the reason for the device coming out of deep sleep |
| [getXconfParams](#method.getXconfParams) | Returns XCONF configuration parameters for the device |
| [hasRebootBeenRequested](#method.hasRebootBeenRequested) | Checks whether a reboot has been requested |
| [isGzEnabled](#method.isGzEnabled) | Checks whether GZ is enabled |
| [isOptOutTelemetry](#method.isOptOutTelemetry) | (Version 2) Checks the telemetry opt-out status |
| [queryMocaStatus](#method.queryMocaStatus) | Checks whether MOCA is enabled |
| [reboot](#method.reboot) | Requests that the system performs a reboot of the set-top box |
| [removeCacheKey](#method.removeCacheKey) | Removes the specified key from the cache |
| [requestSystemUptime](#method.requestSystemUptime) | Returns the device uptime |
| [setCachedValue](#method.setCachedValue) | Sets the value for a key in the cache |
| [setDeepSleepTimer](#method.setDeepSleepTimer) | Sets the deep sleep timeout period |
| [setFirmwareAutoReboot](#method.setFirmwareAutoReboot) | (Version 2) Enables or disables the AutoReboot Feature |
| [setFirmwareRebootDelay](#method.setFirmwareRebootDelay) | (Version 2) Delays the firmware reboot |
| [setGzEnabled](#method.setGzEnabled) | Enables or disables GZ |
| [setMode](#method.setMode) | Sets the mode of the set-top box for a specific duration before returning to normal mode |
| [setNetworkStandbyMode](#method.setNetworkStandbyMode) | Enables or disables the network standby mode of the device |
| [SetOptOutTelemetry](#method.SetOptOutTelemetry) | Sets the telemetry opt-out status |
| [setPowerState](#method.setPowerState) | Sets the power state of the device |
| [setPreferredStandbyMode](#method.setPreferredStandbyMode) | Sets and persists the preferred standby mode (see `getAvailableStandbyModes` for valid modes) |
| [setTemperatureThresholds](#method.setTemperatureThresholds) | Sets the temperature threshold values |
| [setTimeZoneDST](#method.setTimeZoneDST) | Sets the system time zone |
| [setWakeupSrcConfiguration](#method.setWakeupSrcConfiguration) | Sets the device wake-up configuration |
| [updateFirmware](#method.updateFirmware) | Initiates a firmware update |
| [uploadLogs](#method.uploadLogs) | (Version 2) Uploads logs to a URL returned by SSR |


<a name="method.cacheContains"></a>
## *cacheContains [<sup>method</sup>](#head.Methods)*

Checks if a key is present in the cache.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.key | string | The cache key |

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
    "method": "org.rdk.System.1.cacheContains",
    "params": {
        "key": "sampleKey"
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

<a name="method.clearLastDeepSleepReason"></a>
## *clearLastDeepSleepReason [<sup>method</sup>](#head.Methods)*

Clears the last deep sleep reason.

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
    "method": "org.rdk.System.1.clearLastDeepSleepReason"
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

<a name="method.deletePersistentPath"></a>
## *deletePersistentPath [<sup>method</sup>](#head.Methods)*

(Version 2) Deletes persistent path associated with a callsign.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.callsign | string | Callsign of the service to delete persistent path |

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
    "method": "org.rdk.System.1.deletePersistentPath",
    "params": {
        "callsign": "HtmlApp"
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

<a name="method.enableMoca"></a>
## *enableMoca [<sup>method</sup>](#head.Methods)*

Enables (or disable) Moca support for the platform.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.value | boolean | `true` to enable Moca support or `false` to disable Moca support |

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
    "method": "org.rdk.System.1.enableMoca",
    "params": {
        "value": true
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

<a name="method.fireFirmwarePendingReboot"></a>
## *fireFirmwarePendingReboot [<sup>method</sup>](#head.Methods)*

(Version 2) Notifies the device about a pending reboot by sending an `onFirmwarePendingReboot` event. This method is called after the firmware image is successfully flashed on the device.

Also see: [onFirmwarePendingReboot](#event.onFirmwarePendingReboot)

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
    "method": "org.rdk.System.1.fireFirmwarePendingReboot"
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

<a name="method.getAvailableStandbyModes"></a>
## *getAvailableStandbyModes [<sup>method</sup>](#head.Methods)*

Queries the available standby modes.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedStandbyModes | array | The supported standby modes |
| result.supportedStandbyModes[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getAvailableStandbyModes"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedStandbyModes": [
            "LIGHT_SLEEP"
        ],
        "success": true
    }
}
```

<a name="method.getCachedValue"></a>
## *getCachedValue [<sup>method</sup>](#head.Methods)*

Gets the value of a key in the cache.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.key | string | The cache key |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.sampleKey | string | Value for the specified key name |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getCachedValue",
    "params": {
        "key": "sampleKey"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "sampleKey": "4343.3434",
        "success": true
    }
}
```

<a name="method.getCoreTemperature"></a>
## *getCoreTemperature [<sup>method</sup>](#head.Methods)*

Returns the core temperature of the device. Not supported on all devices.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.temperature | number | The temperature |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getCoreTemperature"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "temperature": 48.0,
        "success": true
    }
}
```

<a name="method.getDeviceInfo"></a>
## *getDeviceInfo [<sup>method</sup>](#head.Methods)*

Collects device details available from `/etc/device.properties`. Sample keys include:  
* bluetooth_mac  
* boxIP  
* build_type  
* estb_mac  
* imageVersion  
* model_num  
* make  
* friendly_id  
* rf4ce_mac  
* wifi_mac.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.params | array | A list of supported device keys. If no parameters are included, then all keys are returned |
| params.params[#] | string |  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.estb_mac | string | Value for the specified key |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getDeviceInfo",
    "params": {
        "params": [
            "estb_mac"
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
        "estb_mac": "20:F1:9E:EE:62:08",
        "success": true
    }
}
```

<a name="method.getDownloadedFirmwareInfo"></a>
## *getDownloadedFirmwareInfo [<sup>method</sup>](#head.Methods)*

Returns information about firmware downloads.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.currentFWVersion | string | The current firmware version |
| result?.downloadFWVersion | string | <sup>*(optional)*</sup> The downloaded firmware version |
| result?.downloadFWLocation | string | <sup>*(optional)*</sup> The location of the downloaded firmware |
| result.isRebootDeferred | boolean | Whether the device should be rebooted |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getDownloadedFirmwareInfo"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "currentFWVersion": "AX061AEI_VBN_1911_sprint_20200109040424sdy",
        "downloadFWVersion": "AX061AEI_VBN_1911_sprint_20200510040450sdy",
        "downloadFWLocation": "/tmp",
        "isRebootDeferred": false,
        "success": true
    }
}
```

<a name="method.getFirmwareDownloadPercent"></a>
## *getFirmwareDownloadPercent [<sup>method</sup>](#head.Methods)*

Gets the current download percentage.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.downloadPercent | integer | The percentage completed |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getFirmwareDownloadPercent"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "downloadPercent": 25,
        "success": true
    }
}
```

<a name="method.getFirmwareUpdateInfo"></a>
## *getFirmwareUpdateInfo [<sup>method</sup>](#head.Methods)*

Returns information about a firmware update and includes whether or not an update is mandatory. The details are sent as an `onFirmwareUpdateInfoReceived` event.

Also see: [onFirmwareUpdateInfoReceived](#event.onFirmwareUpdateInfoReceived)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.GUID | string | A unique identifier |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.asyncResponse | boolean | Whether the event notification succeeded |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getFirmwareUpdateInfo",
    "params": {
        "GUID": "1234abcd"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "asyncResponse": true,
        "success": true
    }
}
```

<a name="method.getFirmwareUpdateState"></a>
## *getFirmwareUpdateState [<sup>method</sup>](#head.Methods)*

Checks the state of the firmware update.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.firmwareUpdateState | string | The state (must be one of the following: *Uninitialized*, *Requesting*, *Downloading*, *Failed*, *Download Complete*, *Validation Complete*, *Preparing to Reboot*, *On Hold for opt-out*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getFirmwareUpdateState"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "firmwareUpdateState": "Download Complete",
        "success": true
    }
}
```

<a name="method.getLastDeepSleepReason"></a>
## *getLastDeepSleepReason [<sup>method</sup>](#head.Methods)*

Retrieves the last deep sleep reason.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.lastDeepSleepReason | string | The deep sleep reason |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getLastDeepSleepReason"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "lastDeepSleepReason": "thermal_deepsleep_critical_threshold",
        "success": true
    }
}
```

<a name="method.getLastFirmwareFailureReason"></a>
## *getLastFirmwareFailureReason [<sup>method</sup>](#head.Methods)*

(Version 2) Retrieves the last firmware failure reason.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.failureReason | string | <sup>*(optional)*</sup> The reason the failure occurred |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getLastFirmwareFailureReason"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "failureReason": "Image Download Failed - Unable to connect",
        "success": true
    }
}
```

<a name="method.getMacAddresses"></a>
## *getMacAddresses [<sup>method</sup>](#head.Methods)*

Gets the MAC address of the device. The requested details are sent as an `onMacAddressesRetrieved` event.

Also see: [onMACAddressesRetrieved](#event.onMACAddressesRetrieved)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.GUID | string | A unique identifier |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.asyncResponse | boolean | Whether the event notification succeeded |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getMacAddresses",
    "params": {
        "GUID": "1234abcd"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "asyncResponse": true,
        "success": true
    }
}
```

<a name="method.getMilestones"></a>
## *getMilestones [<sup>method</sup>](#head.Methods)*

Returns the list of milestones.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.milestones | array | A string [] of milestones |
| result.milestones[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getMilestones"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "milestones": [
            "2020 Jan 28 08:24:06.762355 arrisxi6 systemd[1]: Starting Log RDK Started Service..."
        ],
        "success": true
    }
}
```

<a name="method.getMode"></a>
## *getMode [<sup>method</sup>](#head.Methods)*

Returns the currently set mode information.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.modeInfo | object | The mode information |
| result.modeInfo.mode | string | The mode (must be one of the following: *NORMAL*, *EAS*, *WAREHOUSE*) |
| result.modeInfo.duration | integer | The duration |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getMode"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "modeInfo": {
            "mode": "NORMAL",
            "duration": 0
        },
        "success": true
    }
}
```

<a name="method.getNetworkStandbyMode"></a>
## *getNetworkStandbyMode [<sup>method</sup>](#head.Methods)*

Returns the network standby mode of the device. If network standby is `true`, the device supports `WakeOnLAN` and `WakeOnWLAN` actions in STR (S3) mode.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.nwStandby | boolean | Whether `WakeOnLAN` and `WakeOnWLAN` is supported (`true`); otherwise, `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getNetworkStandbyMode"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "nwStandby": false,
        "success": true
    }
}
```

<a name="method.getPlatformConfiguration"></a>
## *getPlatformConfiguration [<sup>method</sup>](#head.Methods)*

(Version 2) Returns the supported features and device/account info.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.query | string | Query for support of a particular feature, e.g. AccountInfo.accountId |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.AccountInfo | object | <sup>*(optional)*</sup>  |
| result?.AccountInfo.accountId | string | Account Id |
| result?.AccountInfo.x1DeviceId | string | X1 Device Id |
| result?.AccountInfo.XCALSessionTokenAvailable | boolean |  |
| result?.AccountInfo.experience | string | Experience |
| result?.AccountInfo.deviceMACAddress | string | Device MAC Address |
| result?.AccountInfo.firmwareUpdateDisabled | boolean |  |
| result?.DeviceInfo | object | <sup>*(optional)*</sup>  |
| result?.DeviceInfo.quirks | array | The list of installed "quirks" |
| result?.DeviceInfo.quirks[#] | string |  |
| result?.DeviceInfo.mimeTypeExclusions | object |  |
| result?.DeviceInfo.features | object |  |
| result?.DeviceInfo.mimeTypes | array |  |
| result?.DeviceInfo.mimeTypes[#] | string |  |
| result?.DeviceInfo.model | string |  |
| result?.DeviceInfo.deviceType | string |  |
| result?.DeviceInfo.supportsTrueSD | boolean |  |
| result?.DeviceInfo.webBrowser | object |  |
| result?.DeviceInfo.webBrowser.browserType | string |  |
| result?.DeviceInfo.webBrowser.version | string |  |
| result?.DeviceInfo.webBrowser.userAgent | string |  |
| result?.DeviceInfo.HdrCapability | string | e.g. HDR10,Dolby Vision,Technicolor Prime |
| result?.DeviceInfo.canMixPCMWithSurround | boolean |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getPlatformConfiguration",
    "params": {
        "query": "..."
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "AccountInfo": {
            "accountId": "1000000000000000000",
            "x1DeviceId": "1000000000000000000",
            "XCALSessionTokenAvailable": false,
            "experience": "X1",
            "deviceMACAddress": "44:AA:F5:39:D3:42",
            "firmwareUpdateDisabled": false
        },
        "DeviceInfo": {
            "quirks": [
                "XRE-4621"
            ],
            "mimeTypeExclusions": {},
            "features": {},
            "mimeTypes": [
                "audio/mpeg"
            ],
            "model": "PX051AEI",
            "deviceType": "IpStb",
            "supportsTrueSD": true,
            "webBrowser": {
                "browserType": "WPE",
                "version": "1.0.0.0",
                "userAgent": "Mozilla/5.0 (Linux; x86_64 GNU/Linux) AppleWebKit/601.1 (KHTML, like Gecko) Version/8.0 Safari/601.1 WPE"
            },
            "HdrCapability": "none",
            "canMixPCMWithSurround": true
        },
        "success": true
    }
}
```

<a name="method.getPowerState"></a>
## *getPowerState [<sup>method</sup>](#head.Methods)*

Returns the power state of the device.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.powerState | string | The power state (must be one of the following: *STANDBY*, *DEEP_SLEEP*, *LIGHT_SLEEP*, *ON*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getPowerState"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "powerState": "ON",
        "success": true
    }
}
```

<a name="method.getPowerStateBeforeReboot"></a>
## *getPowerStateBeforeReboot [<sup>method</sup>](#head.Methods)*

(Version 2) Returns the power state before reboot.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.state | string | The power state |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getPowerStateBeforeReboot"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "state": "ON",
        "success": true
    }
}
```

<a name="method.getPreferredStandbyMode"></a>
## *getPreferredStandbyMode [<sup>method</sup>](#head.Methods)*

Returns the preferred standby mode. This method returns an empty string if the preferred mode has not been set.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.preferredStandbyMode | string | The standby mode (must be one of the following: *LIGHT_SLEEP*, *DEEP_SLEEP*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getPreferredStandbyMode"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "preferredStandbyMode": "DEEP_SLEEP",
        "success": true
    }
}
```

<a name="method.getPreviousRebootInfo"></a>
## *getPreviousRebootInfo [<sup>method</sup>](#head.Methods)*

Returns basic information about a reboot.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.timestamp | string | The last reboot time |
| result.reason | string | The reboot reason |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getPreviousRebootInfo"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "timestamp": "20200128083540",
        "reason": "FIRMWARE_FAILURE",
        "success": true
    }
}
```

<a name="method.getPreviousRebootInfo2"></a>
## *getPreviousRebootInfo2 [<sup>method</sup>](#head.Methods)*

(Version 2) Returns detailed information about a reboot.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.rebootInfo | object |  |
| result.rebootInfo.timestamp | string | The last reboot time |
| result.rebootInfo.source | string | The source of the reboot |
| result.rebootInfo.reason | string | The reboot reason |
| result.rebootInfo.customReason | string | A custom reason |
| result.rebootInfo.lastHardPowerReset | string | The timestamp of the last hard power reset |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getPreviousRebootInfo2"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "rebootInfo": {
            "timestamp": "20200128083540",
            "source": "SystemPlugin",
            "reason": "FIRMWARE_FAILURE",
            "customReason": "API Validation",
            "lastHardPowerReset": "Tue Jan 28 08:22:22 UTC 2020"
        },
        "success": true
    }
}
```

<a name="method.getPreviousRebootReason"></a>
## *getPreviousRebootReason [<sup>method</sup>](#head.Methods)*

Returns the last reboot reason.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.reason | string | The reboot reason |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getPreviousRebootReason"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "reason": "FIRMWARE_FAILURE",
        "success": true
    }
}
```

<a name="method.getRFCConfig"></a>
## *getRFCConfig [<sup>method</sup>](#head.Methods)*

Returns information that is related to RDK Feature Control (RFC) configurations.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.rfcList | array | A list of RFC properties to query |
| params.rfcList[#] | string |  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.RFCConfig | object | A list of specified RFC properties |
| result.RFCConfig?.Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AccountInfo.AccountID | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getRFCConfig",
    "params": {
        "rfcList": [
            "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AccountInfo.AccountID"
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
        "RFCConfig": {
            "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AccountInfo.AccountID": "1181405614235499371"
        },
        "success": true
    }
}
```

<a name="method.getSerialNumber"></a>
## *getSerialNumber [<sup>method</sup>](#head.Methods)*

Returns the device serial number.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.serialNumber | string | The serial number |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getSerialNumber"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "serialNumber": "M11806TK0024",
        "success": true
    }
}
```

<a name="method.getStateInfo"></a>
## *getStateInfo [<sup>method</sup>](#head.Methods)*

Queries device state information of various properties.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.param | string | The property to query as a string |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.com.comcast.channel_map | number | <sup>*(optional)*</sup> The specified property |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getStateInfo",
    "params": {
        "param": "com.comcast.channel_map"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "com.comcast.channel_map": 2,
        "success": true
    }
}
```

<a name="method.getSystemVersions"></a>
## *getSystemVersions [<sup>method</sup>](#head.Methods)*

Returns system version details.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.stbVersion | string | The STB version |
| result.receiverVersion | string | The receiver version |
| result.stbTimestamp | string | The STB timestamp |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getSystemVersions"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "stbVersion": "AX061AEI_VBN_1911_sprint_20200109040424sdy",
        "receiverVersion": "3.14.0.0",
        "stbTimestamp": "Thu 09 Jan 2020 04:04:24 AP UTC",
        "success": true
    }
}
```

<a name="method.getTemperatureThresholds"></a>
## *getTemperatureThresholds [<sup>method</sup>](#head.Methods)*

Returns temperature threshold values. Not supported on all devices.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.temperatureThresholds | object |  |
| result.temperatureThresholds.WARN | string | The warning threshold |
| result.temperatureThresholds.MAX | string | The max temperature threshold |
| result.temperatureThresholds.temperature | number | The temperature |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getTemperatureThresholds"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "temperatureThresholds": {
            "WARN": "100.000000",
            "MAX": "110.000000",
            "temperature": 48.0
        },
        "success": true
    }
}
```

<a name="method.getTimeStatus"></a>
## *getTimeStatus [<sup>method</sup>](#head.Methods)*

Returns the timer status.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.timeQuality | string | The quality of the time source in terms of security (must be one of the following: *poor*, *good*, *secure*, *unknown*) |
| result.timeSrc | string | The source that is providing the time |
| result.time | string | The current time |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getTimeStatus"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "timeQuality": "poor",
        "timeSrc": "NTP",
        "time": "20200128083540"
    }
}
```

<a name="method.getTimeZoneDST"></a>
## *getTimeZoneDST [<sup>method</sup>](#head.Methods)*

Get the configured time zone from the file referenced by `TZ_FILE`. If the time zone is not set, then `null` is returned.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.timeZone | string | The timezone |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getTimeZoneDST"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "timeZone": "America/New_York",
        "success": true
    }
}
```

<a name="method.getTimeZones"></a>
## *getTimeZones [<sup>method</sup>](#head.Methods)*

(Version2) Gets the available timezones from the system's time zone database. This method is useful for determining time offsets per zone.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.zoneInfo | object | <sup>*(optional)*</sup> A timezone area |
| result?.zoneInfo?.America | object | <sup>*(optional)*</sup>  |
| result?.zoneInfo?.America?.New_York | string | <sup>*(optional)*</sup>  |
| result?.zoneInfo?.America?.Los_Angeles | string | <sup>*(optional)*</sup>  |
| result?.zoneInfo?.Europe | object | <sup>*(optional)*</sup>  |
| result?.zoneInfo?.Europe?.London | string | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getTimeZones"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "zoneInfo": {
            "America": {
                "New_York": "Thu Nov 5 15:21:17 2020 EST",
                "Los_Angeles": "Thu Nov 5 12:21:17 2020 PST"
            },
            "Europe": {
                "London": "Thu Nov 5 14:21:18 2020 CST"
            }
        },
        "success": true
    }
}
```

<a name="method.getWakeupReason"></a>
## *getWakeupReason [<sup>method</sup>](#head.Methods)*

(Version 2) Returns the reason for the device coming out of deep sleep.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.wakeupReason | string | The reason (must be one of the following: *WAKEUP_REASON_IR*, *WAKEUP_REASON_RCU_BT*, *WAKEUP_REASON_RCU_RF4CE*, *WAKEUP_REASON_GPIO*, *WAKEUP_REASON_LAN*, *WAKEUP_REASON_WLAN*, *WAKEUP_REASON_TIMER*, *WAKEUP_REASON_FRONT_PANEL*, *WAKEUP_REASON_WATCHDOG*, *WAKEUP_REASON_SOFTWARE_RESET*, *WAKEUP_REASON_THERMAL_RESET*, *WAKEUP_REASON_WARM_RESET*, *WAKEUP_REASON_COLDBOOT*, *WAKEUP_REASON_STR_AUTH_FAILURE*, *WAKEUP_REASON_CEC*, *WAKEUP_REASON_PRESENCE*, *WAKEUP_REASON_VOICE*, *WAKEUP_REASON_UNKNOWN*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getWakeupReason"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "wakeupReason": "WAKEUP_REASON_VOICE",
        "success": true
    }
}
```

<a name="method.getXconfParams"></a>
## *getXconfParams [<sup>method</sup>](#head.Methods)*

Returns XCONF configuration parameters for the device.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.xconfParams | object | A timezone area |
| result.xconfParams.env | string | The environment |
| result.xconfParams.eStbMac | string | The embedded set-top box MAC address |
| result.xconfParams.model | string | The device model number |
| result.xconfParams.firmwareVersion | string | The firmware version |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getXconfParams"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "xconfParams": {
            "env": "vbn",
            "eStbMac": "20:F1:9E:EE:62:08",
            "model": "AX061AEI",
            "firmwareVersion": "AX061AEI_VBN_1911_sprint_20200109040424sdy"
        },
        "success": true
    }
}
```

<a name="method.hasRebootBeenRequested"></a>
## *hasRebootBeenRequested [<sup>method</sup>](#head.Methods)*

Checks whether a reboot has been requested.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.rebootRequested | boolean | `true` if a reboot has been requested, otherwise `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.hasRebootBeenRequested"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "rebootRequested": false,
        "success": true
    }
}
```

<a name="method.isGzEnabled"></a>
## *isGzEnabled [<sup>method</sup>](#head.Methods)*

Checks whether GZ is enabled.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enabled | boolean | Enabled (`true`) or disabled (`false`) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.isGzEnabled"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enabled": false,
        "success": true
    }
}
```

<a name="method.isOptOutTelemetry"></a>
## *isOptOutTelemetry [<sup>method</sup>](#head.Methods)*

(Version 2) Checks the telemetry opt-out status.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.Opt-Out | boolean | `true` for opt-out, otherwise `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.isOptOutTelemetry"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "Opt-Out": false,
        "success": true
    }
}
```

<a name="method.queryMocaStatus"></a>
## *queryMocaStatus [<sup>method</sup>](#head.Methods)*

Checks whether MOCA is enabled.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.mocaEnabled | boolean | `true` if MOCA is enabled, otherwise `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.queryMocaStatus"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "mocaEnabled": true,
        "success": true
    }
}
```

<a name="method.reboot"></a>
## *reboot [<sup>method</sup>](#head.Methods)*

Requests that the system performs a reboot of the set-top box.

Also see: [onRebootRequest](#event.onRebootRequest)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.rebootReason | string | The reboot reason |

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
    "method": "org.rdk.System.1.reboot",
    "params": {
        "rebootReason": "FIRMWARE_FAILURE"
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

<a name="method.removeCacheKey"></a>
## *removeCacheKey [<sup>method</sup>](#head.Methods)*

Removes the specified key from the cache.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.key | string | The cache key |

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
    "method": "org.rdk.System.1.removeCacheKey",
    "params": {
        "key": "sampleKey"
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

<a name="method.requestSystemUptime"></a>
## *requestSystemUptime [<sup>method</sup>](#head.Methods)*

Returns the device uptime.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.systemUptime | string | The uptime, in seconds, of the device |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.requestSystemUptime"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "systemUptime": "1655.92",
        "success": true
    }
}
```

<a name="method.setCachedValue"></a>
## *setCachedValue [<sup>method</sup>](#head.Methods)*

Sets the value for a key in the cache.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.key | string | The cache key |
| params.value | number | The value to set |

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
    "method": "org.rdk.System.1.setCachedValue",
    "params": {
        "key": "sampleKey",
        "value": 4343.3434
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

<a name="method.setDeepSleepTimer"></a>
## *setDeepSleepTimer [<sup>method</sup>](#head.Methods)*

Sets the deep sleep timeout period.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.seconds | integer | The deep sleep timeout in seconds |

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
    "method": "org.rdk.System.1.setDeepSleepTimer",
    "params": {
        "seconds": 3
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

<a name="method.setFirmwareAutoReboot"></a>
## *setFirmwareAutoReboot [<sup>method</sup>](#head.Methods)*

(Version 2) Enables or disables the AutoReboot Feature. This method internally sets the tr181 `AutoReboot.Enable` parameter to `true` or `false`.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enable | boolean | `true` to enable Autoreboot or `false` to disable |

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
    "method": "org.rdk.System.1.setFirmwareAutoReboot",
    "params": {
        "enable": true
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

<a name="method.setFirmwareRebootDelay"></a>
## *setFirmwareRebootDelay [<sup>method</sup>](#head.Methods)*

(Version 2) Delays the firmware reboot.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.delaySeconds | integer | The reboot delay in seconds |

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
    "method": "org.rdk.System.1.setFirmwareRebootDelay",
    "params": {
        "delaySeconds": 60
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

<a name="method.setGzEnabled"></a>
## *setGzEnabled [<sup>method</sup>](#head.Methods)*

Enables or disables GZ.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Enabled (`true`) or disabled (`false`) |

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
    "method": "org.rdk.System.1.setGzEnabled",
    "params": {
        "enabled": false
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

<a name="method.setMode"></a>
## *setMode [<sup>method</sup>](#head.Methods)*

Sets the mode of the set-top box for a specific duration before returning to normal mode. Valid modes are:  
* `NORMAL` - The set-top box is operating in normal mode.  
* `EAS` - The set-top box is operating in Emergency Alert System (EAS) mode. This mode is set when the device needs to perform certain tasks when entering EAS mode, such as setting the clock display or preventing the user from using the diagnostics menu.  
* `WAREHOUSE` - The set-top box is operating in warehouse mode.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.modeInfo | object | The mode information |
| params.modeInfo.mode | string | The mode (must be one of the following: *NORMAL*, *EAS*, *WAREHOUSE*) |
| params.modeInfo.duration | integer | The duration |

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
    "method": "org.rdk.System.1.setMode",
    "params": {
        "modeInfo": {
            "mode": "NORMAL",
            "duration": 0
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

<a name="method.setNetworkStandbyMode"></a>
## *setNetworkStandbyMode [<sup>method</sup>](#head.Methods)*

Enables or disables the network standby mode of the device. If network standby is enabled, the device supports `WakeOnLAN` and `WakeOnWLAN` actions in STR (S3) mode.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.nwStandby | boolean | Whether `WakeOnLAN` and `WakeOnWLAN` is supported (`true`); otherwise, `false` |

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
    "method": "org.rdk.System.1.setNetworkStandbyMode",
    "params": {
        "nwStandby": false
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

<a name="method.SetOptOutTelemetry"></a>
## *SetOptOutTelemetry [<sup>method</sup>](#head.Methods)*

Sets the telemetry opt-out status.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.Opt-Out | boolean | `true` for opt-out, otherwise `false` |

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
    "method": "org.rdk.System.1.SetOptOutTelemetry",
    "params": {
        "Opt-Out": false
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

<a name="method.setPowerState"></a>
## *setPowerState [<sup>method</sup>](#head.Methods)*

Sets the power state of the device.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.powerState | string | The power state (must be one of the following: *STANDBY*, *DEEP_SLEEP*, *LIGHT_SLEEP*, *ON*) |
| params.standbyReason | string | The reason for a standby state |

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
    "method": "org.rdk.System.1.setPowerState",
    "params": {
        "powerState": "ON",
        "standbyReason": "APIUnitTest"
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

<a name="method.setPreferredStandbyMode"></a>
## *setPreferredStandbyMode [<sup>method</sup>](#head.Methods)*

Sets and persists the preferred standby mode (see `getAvailableStandbyModes` for valid modes). Invoking this function does not change the power state of the device. It only sets the user preference for the preferred action when the `setPowerState` method is invoked with a value of `STANDBY`.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.standbyMode | string | The standby mode (must be one of the following: *LIGHT_SLEEP*, *DEEP_SLEEP*) |

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
    "method": "org.rdk.System.1.setPreferredStandbyMode",
    "params": {
        "standbyMode": "DEEP_SLEEP"
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

<a name="method.setTemperatureThresholds"></a>
## *setTemperatureThresholds [<sup>method</sup>](#head.Methods)*

Sets the temperature threshold values. Not supported on all devices.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.Thresholds | object |  |
| params.Thresholds.WARN | string | The warning threshold |
| params.Thresholds.MAX | string | The max temperature threshold |

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
    "method": "org.rdk.System.1.setTemperatureThresholds",
    "params": {
        "Thresholds": {
            "WARN": "100.000000",
            "MAX": "110.000000"
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

<a name="method.setTimeZoneDST"></a>
## *setTimeZoneDST [<sup>method</sup>](#head.Methods)*

Sets the system time zone. See `getTimeZones` to get a list of available timezones on the system.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.timeZone | string | The timezone |

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
    "method": "org.rdk.System.1.setTimeZoneDST",
    "params": {
        "timeZone": "America/New_York"
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

<a name="method.setWakeupSrcConfiguration"></a>
## *setWakeupSrcConfiguration [<sup>method</sup>](#head.Methods)*

Sets the device wake-up configuration.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.wakeupSrc | integer | The wake-up source as an integer from 0 - 8. (must be one of the following: *WAKEUPSRC_VOICE*, *WAKEUPSRC_PRESENCE_DETECTION*, *WAKEUPSRC_BLUETOOTH*, *WAKEUPSRC_WIFI*, *WAKEUPSRC_IR*, *WAKEUPSRC_POWER_KEY*, *WAKEUPSRC_TIMER*, *WAKEUPSRC_CEC*, *WAKEUPSRC_LAN*) |
| params.config | integer | `0` to disable the wake-up source configuration, or `1` to enable the wake-up source configuration (must be one of the following: *0*, *1*) |

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
    "method": "org.rdk.System.1.setWakeupSrcConfiguration",
    "params": {
        "wakeupSrc": 3,
        "config": 1
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

<a name="method.updateFirmware"></a>
## *updateFirmware [<sup>method</sup>](#head.Methods)*

Initiates a firmware update. This method has no affect if an update is not available.

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
    "method": "org.rdk.System.1.updateFirmware"
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

<a name="method.uploadLogs"></a>
## *uploadLogs [<sup>method</sup>](#head.Methods)*

(Version 2) Uploads logs to a URL returned by SSR.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.url | string | <sup>*(optional)*</sup> SSR URL |

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
    "method": "org.rdk.System.1.uploadLogs",
    "params": {
        "url": "https://ssr.ccp.xcal.tv/cgi-bin/rdkb_snmp.cgi"
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

The following events are provided by the org.rdk.System plugin:

SystemServices interface events:

| Event | Description |
| :-------- | :-------- |
| [onFirmwareDownloadProgress](#event.onFirmwareDownloadProgress) | Triggered once per second when firmware download is started |
| [onFirmwarePendingReboot](#event.onFirmwarePendingReboot) | (Version 2) Triggered when the `fireFirmwarePendingReboot` method is invoked |
| [onFirmwareUpdateInfoReceived](#event.onFirmwareUpdateInfoReceived) | Triggered when the `getFirmwareUpdateInfo` asynchronous method is invoked |
| [onFirmwareUpdateStateChange](#event.onFirmwareUpdateStateChange) | Triggered when the state of a firmware update changes |
| [onMacAddressesRetrieved](#event.onMacAddressesRetrieved) | Triggered when the `getMacAddresses` asynchronous method is invoked |
| [onRebootRequest](#event.onRebootRequest) | Triggered when an application invokes the reboot method |
| [onSystemClockSet](#event.onSystemClockSet) | Triggered when the clock on the set-top device is updated |
| [onSystemModeChanged](#event.onSystemModeChanged) | Triggered when the device operating mode changes |
| [onSystemPowerStateChanged](#event.onSystemPowerStateChanged) | Triggered when the power manager detects a device power state change |
| [onTemperatureThresholdChanged](#event.onTemperatureThresholdChanged) | Triggered when the device temperature changes beyond the `WARN` or `MAX` limits (see `setTemperatureThresholds`) |
| [onTimeStatusChanged](#event.onTimeStatusChanged) | Triggered when the device timer settings change |


<a name="event.onFirmwareDownloadProgress"></a>
## *onFirmwareDownloadProgress [<sup>event</sup>](#head.Notifications)*

Triggered once per second when firmware download is started. Will be stopped at 100%.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.downloadPercent | number | Download progress |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onFirmwareDownloadProgress",
    "params": {
        "downloadPercent": 12
    }
}
```

<a name="event.onFirmwarePendingReboot"></a>
## *onFirmwarePendingReboot [<sup>event</sup>](#head.Notifications)*

(Version 2) Triggered when the `fireFirmwarePendingReboot` method is invoked.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.seconds | integer | The amount of time to wait, in seconds, before forcing a reboot. A reboot is mandatory when the `rebootImmediately` parameter is `true`. See [onFirmwareUpdateInfoReceived](#event.onFirmwareUpdateInfoReceived) |
| params.success | boolean | Whether the request succeeded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onFirmwarePendingReboot",
    "params": {
        "seconds": 600,
        "success": true
    }
}
```

<a name="event.onFirmwareUpdateInfoReceived"></a>
## *onFirmwareUpdateInfoReceived [<sup>event</sup>](#head.Notifications)*

Triggered when the `getFirmwareUpdateInfo` asynchronous method is invoked.  
Update details are:  
* `0` - A new firmware version is available.  
* `1` - The firmware version is at the current version.  
* `2` - XCONF did not return a firmware version (timeout or other XCONF error).  
* `3` - The device is configured not to update the firmware (`swupdate.conf` exists on the device).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.status | integer | The firmware update status |
| params.responseString | string | A custom response |
| params.rebootImmediately | boolean | When `true`, indicates that an update is mandatory and requires a reboot. The application is responsible for coordinating the reboot with the user. An [onFirmwarePendingReboot](#event.onFirmwarePendingReboot) event indicates that the device has been successfully flashed and that the reboot is pending. See [fireFirmwarePendingReboot](#method.fireFirmwarePendingReboot) |
| params.firmwareUpdateVersion | string | The next firmware update version |
| params.updateAvailable | boolean | The value `false` indicates that there is no update available, either because there was no firmware update version returned from XCONF, or because the version returned from XCONF matches the version already on the device. The value of `true` indicates there is a firmware version available for update |
| params.updateAvailableEnum | integer | The update available details (must be one of the following: *0*, *1*, *2*, *3*) |
| params.success | boolean | Whether the request succeeded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onFirmwareUpdateInfoReceived",
    "params": {
        "status": 0,
        "responseString": "...",
        "rebootImmediately": true,
        "firmwareUpdateVersion": "AX061AEI_VBN_1911_sprint_20200109040424sdy",
        "updateAvailable": true,
        "updateAvailableEnum": 0,
        "success": true
    }
}
```

<a name="event.onFirmwareUpdateStateChange"></a>
## *onFirmwareUpdateStateChange [<sup>event</sup>](#head.Notifications)*

Triggered when the state of a firmware update changes.  
State details are:  
* `0`: Uninitialized - there is no firmware update in progress  
* `1`: Requesting - requesting firmware update information  
* `2`: Downloading  
* `3`: Failed  
* `4`: Download Complete  
* `5`: Validation Complete  
* `6`: Preparing to Reboot  
* `7`: On Hold for opt-out.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.firmwareUpdateState | string | The state (must be one of the following: *Uninitialized*, *Requesting*, *Downloading*, *Failed*, *Download Complete*, *Validation Complete*, *Preparing to Reboot*, *On Hold for opt-out*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onFirmwareUpdateStateChange",
    "params": {
        "firmwareUpdateState": "Download Complete"
    }
}
```

<a name="event.onMacAddressesRetrieved"></a>
## *onMacAddressesRetrieved [<sup>event</sup>](#head.Notifications)*

Triggered when the `getMacAddresses` asynchronous method is invoked.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ecm_mac | string | The embedded cable modem MAC address |
| params.estb_mac | string | The embedded set-top box MAC address |
| params.moca_mac | string | The MOCA MAC address |
| params.eth_mac | string | The Ethernet MAC address |
| params.wifi_mac | string | The Wifi MAC address |
| params.info | string | Additional information |
| params.success | boolean | Whether the request succeeded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onMacAddressesRetrieved",
    "params": {
        "ecm_mac": "A8:11:XX:FD:0C:XX",
        "estb_mac": "A8:11:XX:FD:0C:XX",
        "moca_mac": "00:15:5F:XX:20:5E:57:XX",
        "eth_mac": "A8:11:XX:FD:0C:XX",
        "wifi_mac": "A8:11:XX:FD:0C:XX",
        "info": "...",
        "success": true
    }
}
```

<a name="event.onRebootRequest"></a>
## *onRebootRequest [<sup>event</sup>](#head.Notifications)*

Triggered when an application invokes the reboot method.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.requestedApp | string | The source of the reboot |
| params.rebootReason | string | The reboot reason |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onRebootRequest",
    "params": {
        "requestedApp": "SystemPlugin",
        "rebootReason": "FIRMWARE_FAILURE"
    }
}
```

<a name="event.onSystemClockSet"></a>
## *onSystemClockSet [<sup>event</sup>](#head.Notifications)*

Triggered when the clock on the set-top device is updated.

### Parameters

This event carries no parameters.

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onSystemClockSet"
}
```

<a name="event.onSystemModeChanged"></a>
## *onSystemModeChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the device operating mode changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.mode | string | The mode (must be one of the following: *NORMAL*, *EAS*, *WAREHOUSE*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onSystemModeChanged",
    "params": {
        "mode": "NORMAL"
    }
}
```

<a name="event.onSystemPowerStateChanged"></a>
## *onSystemPowerStateChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the power manager detects a device power state change.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.powerState | string | The power state (must be one of the following: *STANDBY*, *DEEP_SLEEP*, *LIGHT_SLEEP*, *ON*) |
| params?.currentPowerState | string | <sup>*(optional)*</sup> The current power state |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onSystemPowerStateChanged",
    "params": {
        "powerState": "ON",
        "currentPowerState": "ON"
    }
}
```

<a name="event.onTemperatureThresholdChanged"></a>
## *onTemperatureThresholdChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the device temperature changes beyond the `WARN` or `MAX` limits (see `setTemperatureThresholds`). Not supported on all devices.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.thresholdType | string | The exceeded threshold (must be one of the following: *MAX*, *MIN*) |
| params.exceeded | boolean | Whether the threshold exceeded the configured value |
| params.temperature | number | The temperature |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onTemperatureThresholdChanged",
    "params": {
        "thresholdType": "MAX",
        "exceeded": true,
        "temperature": 48.0
    }
}
```

<a name="event.onTimeStatusChanged"></a>
## *onTimeStatusChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the device timer settings change.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.timeQuality | string | The quality of the time source in terms of security (must be one of the following: *poor*, *good*, *secure*, *unknown*) |
| params.timeSrc | string | The source that is providing the time |
| params.time | string | The current time |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onTimeStatusChanged",
    "params": {
        "timeQuality": "poor",
        "timeSrc": "NTP",
        "time": "20200128083540"
    }
}
```

