<!-- Generated automatically, DO NOT EDIT! -->
<a name="System_Plugin"></a>
# System Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.System plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.System plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `System` plugin is used to manage various system-level features such as power settings and firmware updates.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.System*) |
| classname | string | Class name: *org.rdk.System* |
| locator | string | Library name: *libWPEFrameworkSystemServices.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.System plugin:

SystemServices interface methods:

| Method | Description |
| :-------- | :-------- |
| [cacheContains](#cacheContains) | Checks if a key is present in the cache |
| [clearLastDeepSleepReason](#clearLastDeepSleepReason) | Clears the last deep sleep reason |
| [deletePersistentPath](#deletePersistentPath) | (Version 2) Deletes persistent path associated with a callsign |
| [enableMoca](#enableMoca) | Enables (or disables) Moca support for the platform |
| [enableXREConnectionRetention](#enableXREConnectionRetention) | Enables (or disables) XRE Connection Retention option |
| [fireFirmwarePendingReboot](#fireFirmwarePendingReboot) | (Version 2) Notifies the device about a pending reboot |
| [getAvailableStandbyModes](#getAvailableStandbyModes) | Queries the available standby modes |
| [getCachedValue](#getCachedValue) | Gets the value of a key in the cache |
| [getCoreTemperature](#getCoreTemperature) | Returns the core temperature of the device |
| [getDeviceInfo](#getDeviceInfo) | Collects device details |
| [getDownloadedFirmwareInfo](#getDownloadedFirmwareInfo) | Returns information about firmware downloads |
| [getFirmwareDownloadPercent](#getFirmwareDownloadPercent) | Gets the current download percentage |
| [getFirmwareUpdateInfo](#getFirmwareUpdateInfo) | Checks the firmware update information |
| [getFirmwareUpdateState](#getFirmwareUpdateState) | Checks the state of the firmware update |
| [getLastDeepSleepReason](#getLastDeepSleepReason) | Retrieves the last deep sleep reason |
| [getLastFirmwareFailureReason](#getLastFirmwareFailureReason) | (Version 2) Retrieves the last firmware failure reason |
| [getLastWakeupKeyCode](#getLastWakeupKeyCode) | (Version 2) Returns the last wakeup keycode |
| [getMacAddresses](#getMacAddresses) | Gets the MAC address of the device |
| [getMfgSerialNumber](#getMfgSerialNumber) | Gets the Manufacturing Serial Number |
| [getMilestones](#getMilestones) | Returns the list of milestones |
| [getMode](#getMode) | Returns the currently set mode information |
| [getNetworkStandbyMode](#getNetworkStandbyMode) | Returns the network standby mode of the device |
| [getOvertempGraceInterval](#getOvertempGraceInterval) | Returns the over-temperature grace interval value |
| [getPlatformConfiguration](#getPlatformConfiguration) | (Version 2) Returns the supported features and device/account info |
| [getPowerState](#getPowerState) | Returns the power state of the device |
| [getPowerStateBeforeReboot](#getPowerStateBeforeReboot) | Returns the power state before reboot |
| [getPowerStateIsManagedByDevice](#getPowerStateIsManagedByDevice) | Checks whether the power state is managed by the device |
| [getPreferredStandbyMode](#getPreferredStandbyMode) | Returns the preferred standby mode |
| [getPreviousRebootInfo](#getPreviousRebootInfo) | Returns basic information about a reboot |
| [getPreviousRebootInfo2](#getPreviousRebootInfo2) | Returns detailed information about a reboot |
| [getPreviousRebootReason](#getPreviousRebootReason) | Returns the last reboot reason |
| [getRFCConfig](#getRFCConfig) | Returns information that is related to RDK Feature Control (RFC) configurations |
| [getSerialNumber](#getSerialNumber) | Returns the device serial number |
| [getStateInfo](#getStateInfo) | Queries device state information of various properties |
| [getStoreDemoLink](#getStoreDemoLink) | (Version 2) Returns the store demo video link |
| [getSystemVersions](#getSystemVersions) | Returns system version details |
| [getTemperatureThresholds](#getTemperatureThresholds) | Returns temperature threshold values |
| [getTerritory](#getTerritory) | Gets the configured system territory and region |
| [getTimeZones](#getTimeZones) | (Version2) Gets the available timezones from the system's time zone database |
| [getTimeZoneDST](#getTimeZoneDST) | Get the configured time zone from the file referenced by `TZ_FILE` |
| [getWakeupReason](#getWakeupReason) | (Version 2) Returns the reason for the device coming out of deep sleep |
| [getXconfParams](#getXconfParams) | Returns XCONF configuration parameters for the device |
| [hasRebootBeenRequested](#hasRebootBeenRequested) | Checks whether a reboot has been requested |
| [isGzEnabled](#isGzEnabled) | Checks whether GZ is enabled |
| [isOptOutTelemetry](#isOptOutTelemetry) | Checks the telemetry opt-out status |
| [queryMocaStatus](#queryMocaStatus) | Checks whether MOCA is enabled |
| [reboot](#reboot) | Requests that the system performs a reboot of the set-top box |
| [removeCacheKey](#removeCacheKey) | Removes the cache key |
| [requestSystemUptime](#requestSystemUptime) | Returns the device uptime |
| [setBootLoaderPattern](#setBootLoaderPattern) | Sets the boot loader pattern mode in MFR |
| [setCachedValue](#setCachedValue) | Sets the value for a key in the cache |
| [setDeepSleepTimer](#setDeepSleepTimer) | Sets the deep sleep timeout period |
| [setFirmwareAutoReboot](#setFirmwareAutoReboot) | (Version 2) Enables or disables the AutoReboot Feature |
| [setFirmwareRebootDelay](#setFirmwareRebootDelay) | (Version 2) Delays the firmware reboot |
| [setGzEnabled](#setGzEnabled) | Enables or disables GZ |
| [setMode](#setMode) | Sets the mode of the set-top box for a specific duration before returning to normal mode |
| [setNetworkStandbyMode](#setNetworkStandbyMode) | Enables or disables the network standby mode of the device |
| [setOptOutTelemetry](#setOptOutTelemetry) | Sets the telemetry opt-out status |
| [setOvertempGraceInterval](#setOvertempGraceInterval) | Sets the over-temperature grace interval value |
| [setPowerState](#setPowerState) | Sets the power state of the device |
| [setPreferredStandbyMode](#setPreferredStandbyMode) | Sets and persists the preferred standby mode |
| [setTemperatureThresholds](#setTemperatureThresholds) | Sets the temperature threshold values |
| [setTerritory](#setTerritory) | Sets the system territory and region |
| [setTimeZoneDST](#setTimeZoneDST) | Sets the system time zone |
| [setWakeupSrcConfiguration](#setWakeupSrcConfiguration) | Sets the wakeup source configuration |
| [updateFirmware](#updateFirmware) | Initiates a firmware update |
| [uploadLogs](#uploadLogs) | (Version 2) Uploads logs to a URL returned by SSR |


<a name="cacheContains"></a>
## *cacheContains*

Checks if a key is present in the cache.
 
### Events
 
 No Events.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations.

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

<a name="clearLastDeepSleepReason"></a>
## *clearLastDeepSleepReason*

Clears the last deep sleep reason.
 
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

<a name="deletePersistentPath"></a>
## *deletePersistentPath*

(Version 2) Deletes persistent path associated with a callsign.
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.callsign | string | <sup>*(optional)*</sup> Callsign of the service to delete persistent path |
| params?.type | string | <sup>*(optional)*</sup> The type of execution environment. (can be used instead of callsign) |

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
        "callsign": "HtmlApp",
        "type": "HtmlApp"
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

<a name="enableMoca"></a>
## *enableMoca*

Enables (or disables) Moca support for the platform.
 
### Events
 
 No Events.

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

<a name="enableXREConnectionRetention"></a>
## *enableXREConnectionRetention*

Enables (or disables) XRE Connection Retention option.
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enable | boolean | `true` to enable XRE connection retention or `false` to disable |

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
    "method": "org.rdk.System.1.enableXREConnectionRetention",
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

<a name="fireFirmwarePendingReboot"></a>
## *fireFirmwarePendingReboot*

(Version 2) Notifies the device about a pending reboot.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onFirmwarePendingReboot` | Triggers when the firmware has a pending reboot |.

Also see: [onFirmwarePendingReboot](#onFirmwarePendingReboot)

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

<a name="getAvailableStandbyModes"></a>
## *getAvailableStandbyModes*

Queries the available standby modes.
 
### Events
 
 No Events.

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

<a name="getCachedValue"></a>
## *getCachedValue*

Gets the value of a key in the cache.
 
### Events
 
 No Events.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations.

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

<a name="getCoreTemperature"></a>
## *getCoreTemperature*

Returns the core temperature of the device. Not supported on all devices.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.temperature | string | The temperature |
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
        "temperature": "48.000000",
        "success": true
    }
}
```

<a name="getDeviceInfo"></a>
## *getDeviceInfo*

Collects device details. Sample keys include:  
* bluetooth_mac  
* boxIP  
* build_type  
* estb_mac  
* imageVersion  
* rf4ce_mac  
* wifi_mac.
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.params | array | A list of supported device keys |
| params.params[#] | string |  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.estb_mac | string | Value for the specified key name |
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

<a name="getDownloadedFirmwareInfo"></a>
## *getDownloadedFirmwareInfo*

Returns information about firmware downloads.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.currentFWVersion | string | The current firmware version |
| result.downloadedFWVersion | string | The downloaded firmware version |
| result.downloadedFWLocation | string | The location of the downloaded firmware |
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
        "downloadedFWVersion": "AX061AEI_VBN_1911_sprint_20200510040450sdy",
        "downloadedFWLocation": "/tmp",
        "isRebootDeferred": false,
        "success": true
    }
}
```

<a name="getFirmwareDownloadPercent"></a>
## *getFirmwareDownloadPercent*

Gets the current download percentage.
 
### Events
 
 No Events.

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

<a name="getFirmwareUpdateInfo"></a>
## *getFirmwareUpdateInfo*

Checks the firmware update information.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onFirmwareUpdateInfoReceived` | Triggers when the firmware update information is requested |.

Also see: [onFirmwareUpdateInfoReceived](#onFirmwareUpdateInfoReceived)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.GUID | string | <sup>*(optional)*</sup> A unique identifier |

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

<a name="getFirmwareUpdateState"></a>
## *getFirmwareUpdateState*

Checks the state of the firmware update.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.firmwareUpdateState | integer | The state (must be one of the following: *Uninitialized*, *Requesting*, *Downloading*, *Failed*, *DownLoad Complete*, *Validation Complete*, *Preparing to Reboot*) |
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
        "firmwareUpdateState": 5,
        "success": true
    }
}
```

<a name="getLastDeepSleepReason"></a>
## *getLastDeepSleepReason*

Retrieves the last deep sleep reason.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.reason | string | The deep sleep reason |
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
        "reason": "thermal_deepsleep_critical_threshold",
        "success": true
    }
}
```

<a name="getLastFirmwareFailureReason"></a>
## *getLastFirmwareFailureReason*

(Version 2) Retrieves the last firmware failure reason.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.failReason | string | The reason the failure occurred |
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
        "failReason": "Image Download Failed - Unable to connect",
        "success": true
    }
}
```

<a name="getLastWakeupKeyCode"></a>
## *getLastWakeupKeyCode*

(Version 2) Returns the last wakeup keycode.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.wakeupKeyCode | string | The last wakeup keycode |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getLastWakeupKeyCode"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "wakeupKeyCode": "59",
        "success": true
    }
}
```

<a name="getMacAddresses"></a>
## *getMacAddresses*

Gets the MAC address of the device.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onMacAddressesRetreived` | Triggers when the MAC addresses are requested |.

Also see: [onMacAddressesRetreived](#onMacAddressesRetreived)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.GUID | string | <sup>*(optional)*</sup> A unique identifier |

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

<a name="getMfgSerialNumber"></a>
## *getMfgSerialNumber*

Gets the Manufacturing Serial Number.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.mfgSerialNumber | string | Manufacturing Serial Number |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getMfgSerialNumber"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "mfgSerialNumber": "F00020CE000003",
        "success": true
    }
}
```

<a name="getMilestones"></a>
## *getMilestones*

Returns the list of milestones.
 
### Events
 
 No Events.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations.

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

<a name="getMode"></a>
## *getMode*

Returns the currently set mode information.
 
### Events
 
 No Events.

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

<a name="getNetworkStandbyMode"></a>
## *getNetworkStandbyMode*

Returns the network standby mode of the device. If network standby is `true`, the device supports `WakeOnLAN` and `WakeOnWLAN` actions in STR (S3) mode.
 
### Events
 
 No Events.

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

<a name="getOvertempGraceInterval"></a>
## *getOvertempGraceInterval*

Returns the over-temperature grace interval value. Not supported on all devices.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.graceInterval | string | The over temperature grace interval |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getOvertempGraceInterval"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "graceInterval": "600",
        "success": true
    }
}
```

<a name="getPlatformConfiguration"></a>
## *getPlatformConfiguration*

(Version 2) Returns the supported features and device/account info.
 
### Events
 
 No Events.

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
| result?.AccountInfo?.accountId | string | <sup>*(optional)*</sup> Account Id |
| result?.AccountInfo?.x1DeviceId | string | <sup>*(optional)*</sup> X1 Device Id |
| result?.AccountInfo?.XCALSessionTokenAvailable | boolean | <sup>*(optional)*</sup>  |
| result?.AccountInfo?.experience | string | <sup>*(optional)*</sup> Experience |
| result?.AccountInfo?.deviceMACAddress | string | <sup>*(optional)*</sup> Device MAC Address |
| result?.AccountInfo?.firmwareUpdateDisabled | boolean | <sup>*(optional)*</sup>  |
| result?.DeviceInfo | object | <sup>*(optional)*</sup>  |
| result?.DeviceInfo?.quirks | array | <sup>*(optional)*</sup> The list of installed "quirks" |
| result?.DeviceInfo?.quirks[#] | string | <sup>*(optional)*</sup>  |
| result?.DeviceInfo?.mimeTypeExclusions | object | <sup>*(optional)*</sup>  |
| result?.DeviceInfo?.features | object | <sup>*(optional)*</sup>  |
| result?.DeviceInfo?.mimeTypes | array | <sup>*(optional)*</sup>  |
| result?.DeviceInfo?.mimeTypes[#] | string | <sup>*(optional)*</sup>  |
| result?.DeviceInfo?.model | string | <sup>*(optional)*</sup>  |
| result?.DeviceInfo?.deviceType | string | <sup>*(optional)*</sup>  |
| result?.DeviceInfo?.supportsTrueSD | boolean | <sup>*(optional)*</sup>  |
| result?.DeviceInfo?.webBrowser | object | <sup>*(optional)*</sup>  |
| result?.DeviceInfo?.webBrowser.browserType | string |  |
| result?.DeviceInfo?.webBrowser.version | string |  |
| result?.DeviceInfo?.webBrowser.userAgent | string |  |
| result?.DeviceInfo?.HdrCapability | string | <sup>*(optional)*</sup> e.g. HDR10,Dolby Vision,Technicolor Prime |
| result?.DeviceInfo?.canMixPCMWithSurround | boolean | <sup>*(optional)*</sup>  |
| result?.DeviceInfo?.publicIP | string | <sup>*(optional)*</sup> Public IP |
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
            "canMixPCMWithSurround": true,
            "publicIP": "12.34.56.78"
        },
        "success": true
    }
}
```

<a name="getPowerState"></a>
## *getPowerState*

Returns the power state of the device.
 
### Events
 
 No Events.

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

<a name="getPowerStateBeforeReboot"></a>
## *getPowerStateBeforeReboot*

Returns the power state before reboot.
 
### Events
 
 No Events.

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

<a name="getPowerStateIsManagedByDevice"></a>
## *getPowerStateIsManagedByDevice*

Checks whether the power state is managed by the device.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.powerStateManagedByDevice | boolean | `true` if power state is managed by the device, `false` otherwise |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getPowerStateIsManagedByDevice"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "powerStateManagedByDevice": false,
        "success": true
    }
}
```

<a name="getPreferredStandbyMode"></a>
## *getPreferredStandbyMode*

Returns the preferred standby mode. This method returns an empty string if the preferred mode has not been set.
 
### Events
 
 No Events.

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

<a name="getPreviousRebootInfo"></a>
## *getPreviousRebootInfo*

Returns basic information about a reboot.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.timestamp | string | The last reboot time |
| result.reason | string | The reboot reason |
| result.source | string | Source that trigerred reboot |
| result.customReason | string | A custom reason |
| result.otherReason | string | Other reasons for reboot |
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
        "source": "-sh",
        "customReason": "API Validation",
        "otherReason": "API Validation",
        "success": true
    }
}
```

<a name="getPreviousRebootInfo2"></a>
## *getPreviousRebootInfo2*

Returns detailed information about a reboot.
 
### Events
 
 No Events.

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

<a name="getPreviousRebootReason"></a>
## *getPreviousRebootReason*

Returns the last reboot reason.
 
### Events
 
 No Events.

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

<a name="getRFCConfig"></a>
## *getRFCConfig*

Returns information that is related to RDK Feature Control (RFC) configurations.
 
### Events
 
 No Events.

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

<a name="getSerialNumber"></a>
## *getSerialNumber*

Returns the device serial number.
 
### Events
 
 No Events.

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

<a name="getStateInfo"></a>
## *getStateInfo*

Queries device state information of various properties.
 
### Events
 
 No Events.

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

<a name="getStoreDemoLink"></a>
## *getStoreDemoLink*

(Version 2) Returns the store demo video link.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.fileURL | string | <sup>*(optional)*</sup> The file URL of stored demo video |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getStoreDemoLink"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "fileURL": "http://127.0.0.1:50050/store-mode-video/videoFile.mp4",
        "success": true
    }
}
```

<a name="getSystemVersions"></a>
## *getSystemVersions*

Returns system version details.
 
### Events
 
 No Events.

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

<a name="getTemperatureThresholds"></a>
## *getTemperatureThresholds*

Returns temperature threshold values. Not supported on all devices.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.temperatureThresholds | object |  |
| result.temperatureThresholds.WARN | string | The warning threshold |
| result.temperatureThresholds.MAX | string | The max temperature threshold |
| result.temperatureThresholds.temperature | string | The temperature |
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
            "temperature": "48.000000"
        },
        "success": true
    }
}
```

<a name="getTerritory"></a>
## *getTerritory*

Gets the configured system territory and region. Territory is a ISO-3166-1 alpha-3 standard (see https://en.wikipedia.org/wiki/ISO_3166-1). Region is a ISO-3166-2 alpha-2 standard (see https://en.wikipedia.org/wiki/ISO_3166-2).
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.territory | string | territory name |
| result.region | string | region name |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.1.getTerritory"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "territory": "USA",
        "region": "US-NY",
        "success": true
    }
}
```

<a name="getTimeZones"></a>
## *getTimeZones*

(Version2) Gets the available timezones from the system's time zone database. This method is useful for determining time offsets per zone.
 
### Events
 
 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.zoneinfo | object | A timezone area |
| result.zoneinfo?.America | object | <sup>*(optional)*</sup>  |
| result.zoneinfo?.America?.New_York | string | <sup>*(optional)*</sup>  |
| result.zoneinfo?.America?.Los_Angeles | string | <sup>*(optional)*</sup>  |
| result.zoneinfo?.Europe | object | <sup>*(optional)*</sup>  |
| result.zoneinfo?.Europe?.London | string | <sup>*(optional)*</sup>  |
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
        "zoneinfo": {
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

<a name="getTimeZoneDST"></a>
## *getTimeZoneDST*

Get the configured time zone from the file referenced by `TZ_FILE`. If the time zone is not set, then `null` is returned.
 
### Events
 
 No Events.

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

<a name="getWakeupReason"></a>
## *getWakeupReason*

(Version 2) Returns the reason for the device coming out of deep sleep.
 
### Events
 
 No Events.

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

<a name="getXconfParams"></a>
## *getXconfParams*

Returns XCONF configuration parameters for the device.
 
### Events
 
 No Events.

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

<a name="hasRebootBeenRequested"></a>
## *hasRebootBeenRequested*

Checks whether a reboot has been requested.
 
### Events
 
 No Events.

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

<a name="isGzEnabled"></a>
## *isGzEnabled*

Checks whether GZ is enabled.
 
### Events
 
 No Events.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations.

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

<a name="isOptOutTelemetry"></a>
## *isOptOutTelemetry*

Checks the telemetry opt-out status.
 
### Events
 
 No Events.

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

<a name="queryMocaStatus"></a>
## *queryMocaStatus*

Checks whether MOCA is enabled.
 
### Events
 
 No Events.

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

<a name="reboot"></a>
## *reboot*

Requests that the system performs a reboot of the set-top box.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onRebootRequest` | Triggers when a device reboot request is made |.

Also see: [onRebootRequest](#onRebootRequest)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.rebootReason | string | The reboot reason |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.IARM_Bus_Call_STATUS | integer | IARM BUS status |
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
        "IARM_Bus_Call_STATUS": 0,
        "success": true
    }
}
```

<a name="removeCacheKey"></a>
## *removeCacheKey*

Removes the cache key.
 
### Events
 
 No Events.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations.

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

<a name="requestSystemUptime"></a>
## *requestSystemUptime*

Returns the device uptime.
 
### Events
 
 No Events.

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

<a name="setBootLoaderPattern"></a>
## *setBootLoaderPattern*

Sets the boot loader pattern mode in MFR. 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.pattern | string | Bootloader pattern mode (must be one of the following: *NORMAL*, *SILENT*, *SILENT_LED_ON*) |

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
    "method": "org.rdk.System.1.setBootLoaderPattern",
    "params": {
        "pattern": "NORMAL"
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

<a name="setCachedValue"></a>
## *setCachedValue*

Sets the value for a key in the cache.
 
### Events
 
 No Events.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations.

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

<a name="setDeepSleepTimer"></a>
## *setDeepSleepTimer*

Sets the deep sleep timeout period.
 
### Events
 
 No Events.

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

<a name="setFirmwareAutoReboot"></a>
## *setFirmwareAutoReboot*

(Version 2) Enables or disables the AutoReboot Feature. This method internally sets the tr181 `AutoReboot.Enable` parameter to `true` or `false`.
 
### Events
 
 No Events.

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

<a name="setFirmwareRebootDelay"></a>
## *setFirmwareRebootDelay*

(Version 2) Delays the firmware reboot.
 
### Events
 
 No Events.

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

<a name="setGzEnabled"></a>
## *setGzEnabled*

Enables or disables GZ.
 
### Events
 
 No Events.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations.

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

<a name="setMode"></a>
## *setMode*

Sets the mode of the set-top box for a specific duration before returning to normal mode. Valid modes are:  
* `NORMAL` - The set-top box is operating in normal mode.  
* `EAS` - The set-top box is operating in Emergency Alert System (EAS) mode. This mode is set when the device needs to perform certain tasks when entering EAS mode, such as setting the clock display or preventing the user from using the diagnostics menu.  
* `WAREHOUSE` - The set-top box is operating in warehouse mode.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onSystemModeChanged` | Triggers when the system mode is changed successfully |.

Also see: [onSystemModeChanged](#onSystemModeChanged)

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

<a name="setNetworkStandbyMode"></a>
## *setNetworkStandbyMode*

Enables or disables the network standby mode of the device. If network standby is enabled, the device supports `WakeOnLAN` and `WakeOnWLAN` actions in STR (S3) mode.
 
### Events
 
 No Events.

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

<a name="setOptOutTelemetry"></a>
## *setOptOutTelemetry*

Sets the telemetry opt-out status.
 
### Events
 
 No Events.

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
    "method": "org.rdk.System.1.setOptOutTelemetry",
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

<a name="setOvertempGraceInterval"></a>
## *setOvertempGraceInterval*

Sets the over-temperature grace interval value. Not supported on all devices.
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.graceInterval | string | The over temperature grace interval |

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
    "method": "org.rdk.System.1.setOvertempGraceInterval",
    "params": {
        "graceInterval": "600"
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

<a name="setPowerState"></a>
## *setPowerState*

Sets the power state of the device.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onSystemPowerStateChanged` | Triggers when the system power state changes |.

Also see: [onSystemPowerStateChanged](#onSystemPowerStateChanged)

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

<a name="setPreferredStandbyMode"></a>
## *setPreferredStandbyMode*

Sets and persists the preferred standby mode. See [getAvailableStandbyModes](#getAvailableStandbyModes) for valid modes. Invoking this function does not change the power state of the device. It only sets the user preference for the preferred action when the [setPowerState](#setPowerState) method is invoked with a value of `STANDBY`.
 
### Events
 
 No Events.

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

<a name="setTemperatureThresholds"></a>
## *setTemperatureThresholds*

Sets the temperature threshold values. Not supported on all devices.
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.thresholds | object |  |
| params.thresholds.WARN | string | The warning threshold |
| params.thresholds.MAX | string | The max temperature threshold |

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
        "thresholds": {
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

<a name="setTerritory"></a>
## *setTerritory*

Sets the system territory and region.Territory is a ISO-3166-1 alpha-3 standard (see https://en.wikipedia.org/wiki/ISO_3166-1). Region is a ISO-3166-2 alpha-2 standard (see https://en.wikipedia.org/wiki/ISO_3166-2).
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.territory | string | territory name |
| params?.region | string | <sup>*(optional)*</sup> region name |

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
    "method": "org.rdk.System.1.setTerritory",
    "params": {
        "territory": "USA",
        "region": "US-NY"
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

<a name="setTimeZoneDST"></a>
## *setTimeZoneDST*

Sets the system time zone. See `getTimeZones` to get a list of available timezones on the system.
 
### Events
 
 No Events.

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

<a name="setWakeupSrcConfiguration"></a>
## *setWakeupSrcConfiguration*

Sets the wakeup source configuration.
 
### Events
 
 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.wakeupSrc | string | The wakeup source |
| params.config | string | The source configuration |

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
        "wakeupSrc": "3",
        "config": "1"
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

<a name="updateFirmware"></a>
## *updateFirmware*

Initiates a firmware update. This method has no affect if an update is not available.
 
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

<a name="uploadLogs"></a>
## *uploadLogs*

(Version 2) Uploads logs to a URL returned by SSR.
 
### Events
 
 No Events.

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

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.System plugin:

SystemServices interface events:

| Event | Description |
| :-------- | :-------- |
| [onFirmwarePendingReboot](#onFirmwarePendingReboot) | (Version 2) Triggered when the `fireFirmwarePendingReboot` method is invoked |
| [onFirmwareUpdateInfoReceived](#onFirmwareUpdateInfoReceived) | Triggered when the `getFirmwareUpdateInfo` asynchronous method is invoked |
| [onFirmwareUpdateStateChange](#onFirmwareUpdateStateChange) | Triggered when the state of a firmware update changes |
| [onMacAddressesRetreived](#onMacAddressesRetreived) | Triggered when the `getMacAddresses` asynchronous method is invoked |
| [onNetworkStandbyModeChanged](#onNetworkStandbyModeChanged) | Triggered when the network standby mode setting changes |
| [onRebootRequest](#onRebootRequest) | Triggered when an application invokes the reboot method |
| [onSystemClockSet](#onSystemClockSet) | Triggered when the clock on the set-top device is updated |
| [onSystemModeChanged](#onSystemModeChanged) | Triggered when the device operating mode changes |
| [onSystemPowerStateChanged](#onSystemPowerStateChanged) | Triggered when the power manager detects a device power state change |
| [onTemperatureThresholdChanged](#onTemperatureThresholdChanged) | Triggered when the device temperature changes beyond the `WARN` or `MAX` limits (see `setTemperatureThresholds`) |


<a name="onFirmwarePendingReboot"></a>
## *onFirmwarePendingReboot*

(Version 2) Triggered when the `fireFirmwarePendingReboot` method is invoked.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.fireFirmwarePendingReboot | integer | The deep sleep timeout in seconds |
| params.success | boolean | Whether the request succeeded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onFirmwarePendingReboot",
    "params": {
        "fireFirmwarePendingReboot": 3,
        "success": true
    }
}
```

<a name="onFirmwareUpdateInfoReceived"></a>
## *onFirmwareUpdateInfoReceived*

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
| params?.firmwareUpdateVersion | string | <sup>*(optional)*</sup> The next firmware update version |
| params.rebootImmediately | boolean | The value `true` indicates that the device has to be rebooted immediately or `false` otherwise |
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
        "firmwareUpdateVersion": "AX061AEI_VBN_1911_sprint_20200109040424sdy",
        "rebootImmediately": true,
        "updateAvailable": true,
        "updateAvailableEnum": 0,
        "success": true
    }
}
```

<a name="onFirmwareUpdateStateChange"></a>
## *onFirmwareUpdateStateChange*

Triggered when the state of a firmware update changes.  
State details are:  
* `0`: Uninitialized - there is no firmware update in progress  
* `1`: Requesting - requesting firmware update information  
* `2`: Downloading  
* `3`: Failed  
* `4`: Download Complete  
* `5`: Validation Complete  
* `6`: Preparing to Reboot.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.firmwareUpdateStateChange | integer | The state (must be one of the following: *Uninitialized*, *Requesting*, *Downloading*, *Failed*, *DownLoad Complete*, *Validation Complete*, *Preparing to Reboot*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onFirmwareUpdateStateChange",
    "params": {
        "firmwareUpdateStateChange": 5
    }
}
```

<a name="onMacAddressesRetreived"></a>
## *onMacAddressesRetreived*

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
| params.bluetooth_mac | string | The Bluetooth MAC address |
| params.rf4ce_mac | string | The Rf4ce MAC address |
| params?.info | string | <sup>*(optional)*</sup> Additional information (only if any of the above data is missing) |
| params.success | boolean | Whether the request succeeded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onMacAddressesRetreived",
    "params": {
        "ecm_mac": "A8:11:XX:FD:0C:XX",
        "estb_mac": "A8:11:XX:FD:0C:XX",
        "moca_mac": "00:15:5F:XX:20:5E:57:XX",
        "eth_mac": "A8:11:XX:FD:0C:XX",
        "wifi_mac": "A8:11:XX:FD:0C:XX",
        "bluetooth_mac": "AA:AA:AA:AA:AA:AA",
        "rf4ce_mac": "00:00:00:00:00:00",
        "info": "Details fetch: all are not success",
        "success": true
    }
}
```

<a name="onNetworkStandbyModeChanged"></a>
## *onNetworkStandbyModeChanged*

Triggered when the network standby mode setting changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.nwStandby | boolean | Network standby mode |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onNetworkStandbyModeChanged",
    "params": {
        "nwStandby": true
    }
}
```

<a name="onRebootRequest"></a>
## *onRebootRequest*

Triggered when an application invokes the reboot 

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

<a name="onSystemClockSet"></a>
## *onSystemClockSet*

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

<a name="onSystemModeChanged"></a>
## *onSystemModeChanged*

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

<a name="onSystemPowerStateChanged"></a>
## *onSystemPowerStateChanged*

Triggered when the power manager detects a device power state change.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.powerState | string | The power state (must be one of the following: *STANDBY*, *DEEP_SLEEP*, *LIGHT_SLEEP*, *ON*) |
| params.currentPowerState | string | The current power state |

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

<a name="onTemperatureThresholdChanged"></a>
## *onTemperatureThresholdChanged*

Triggered when the device temperature changes beyond the `WARN` or `MAX` limits (see `setTemperatureThresholds`). Not supported on all devices.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.thresholdType | string | The exceeded threshold (must be one of the following: *MAX*, *MIN*) |
| params.exceeded | boolean | Whether the threshold exceeded the configured value |
| params.temperature | string | The temperature |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onTemperatureThresholdChanged",
    "params": {
        "thresholdType": "MAX",
        "exceeded": true,
        "temperature": "48.000000"
    }
}
```

