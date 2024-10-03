<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.System_API"></a>
# System API

**Version: [1.0.0]()**

A org.rdk.System plugin for Thunder framework.

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

The `System` plugin is used to manage various system-level features such as power settings and firmware updates.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| classname | string | Class name: *org.rdk.System* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.System plugin:

org.rdk.System interface methods:

| Method | Description |
| :-------- | :-------- |
| [clearLastDeepSleepReason](#method.clearLastDeepSleepReason) | Clears the last deep sleep reason |
| [deletePersistentPath](#method.deletePersistentPath) | Deletes persistent path associated with a callsign |
| [enableMoca](#method.enableMoca) | Enables (or disables) Moca support for the platform |
| [enableXREConnectionRetention](#method.enableXREConnectionRetention) | Enables (or disables) XRE Connection Retention option |
| [fireFirmwarePendingReboot](#method.fireFirmwarePendingReboot) | Notifies the device about a pending reboot |
| [getAvailableStandbyModes](#method.getAvailableStandbyModes) | Queries the available standby modes |
| [getCoreTemperature](#method.getCoreTemperature) | Returns the core temperature of the device |
| [getDeviceInfo](#method.getDeviceInfo) | Collects device details |
| [getDownloadedFirmwareInfo](#method.getDownloadedFirmwareInfo) | Returns information about firmware downloads |
| [getFirmwareDownloadPercent](#method.getFirmwareDownloadPercent) | Gets the current download percentage |
| [getFirmwareUpdateInfo](#method.getFirmwareUpdateInfo) | Checks the firmware update information |
| [getFirmwareUpdateState](#method.getFirmwareUpdateState) | Checks the state of the firmware update |
| [getLastDeepSleepReason](#method.getLastDeepSleepReason) | Retrieves the last deep sleep reason |
| [getLastFirmwareFailureReason](#method.getLastFirmwareFailureReason) | Retrieves the last firmware failure reason |
| [getLastWakeupKeyCode](#method.getLastWakeupKeyCode) | Returns the last wakeup keycode |
| [getMacAddresses](#method.getMacAddresses) | Gets the MAC address of the device |
| [getMfgSerialNumber](#method.getMfgSerialNumber) | Gets the Manufacturing Serial Number |
| [getMilestones](#method.getMilestones) | Returns the list of milestones |
| [getMode](#method.getMode) | Returns the currently set mode information |
| [getNetworkStandbyMode](#method.getNetworkStandbyMode) | Returns the network standby mode of the device |
| [getOvertempGraceInterval](#method.getOvertempGraceInterval) | Returns the over-temperature grace interval value |
| [getPlatformConfiguration](#method.getPlatformConfiguration) | Returns the supported features and device/account info |
| [getPowerState](#method.getPowerState) | Returns the power state of the device |
| [getPowerStateBeforeReboot](#method.getPowerStateBeforeReboot) | Returns the power state before reboot |
| [getPowerStateIsManagedByDevice](#method.getPowerStateIsManagedByDevice) | Checks whether the power state is managed by the device |
| [getPreferredStandbyMode](#method.getPreferredStandbyMode) | Returns the preferred standby mode |
| [getPreviousRebootInfo](#method.getPreviousRebootInfo) | Returns basic information about a reboot |
| [getPreviousRebootInfo2](#method.getPreviousRebootInfo2) | Returns detailed information about a reboot |
| [getPreviousRebootReason](#method.getPreviousRebootReason) | Returns the last reboot reason |
| [getRFCConfig](#method.getRFCConfig) | Returns information that is related to RDK Feature Control (RFC) configurations |
| [getSerialNumber](#method.getSerialNumber) | Returns the device serial number |
| [getStateInfo](#method.getStateInfo) | Queries device state information of various properties |
| [getStoreDemoLink](#method.getStoreDemoLink) | Returns the store demo video link |
| [getSystemVersions](#method.getSystemVersions) | Returns system version details |
| [getTemperatureThresholds](#method.getTemperatureThresholds) | Returns temperature threshold values |
| [getFriendlyName](#method.getFriendlyName) | Returns the friendly name set by setFriendlyName API or default value |
| [getTerritory](#method.getTerritory) | Gets the configured system territory and region |
| [getTimeZones](#method.getTimeZones) | (Version2) Gets the available timezones from the system's time zone database |
| [getTimeZoneDST](#method.getTimeZoneDST) | Get the configured time zone from the file referenced by `TZ_FILE` |
| [getWakeupReason](#method.getWakeupReason) | Returns the reason for the device coming out of deep sleep |
| [getXconfParams](#method.getXconfParams) | Returns XCONF configuration parameters for the device |
| [isGzEnabled](#method.isGzEnabled) | Checks whether GZ is enabled |
| [isOptOutTelemetry](#method.isOptOutTelemetry) | Checks the telemetry opt-out status |
| [queryMocaStatus](#method.queryMocaStatus) | Checks whether MOCA is enabled |
| [reboot](#method.reboot) | Requests that the system performs a reboot of the set-top box |
| [requestSystemUptime](#method.requestSystemUptime) | Returns the device uptime |
| [setBootLoaderPattern](#method.setBootLoaderPattern) | Sets the boot loader pattern mode in MFR |
| [setDeepSleepTimer](#method.setDeepSleepTimer) | Sets the deep sleep timeout period |
| [setFirmwareAutoReboot](#method.setFirmwareAutoReboot) | Enables or disables the AutoReboot Feature |
| [setFirmwareRebootDelay](#method.setFirmwareRebootDelay) | Delays the firmware reboot |
| [setGzEnabled](#method.setGzEnabled) | Enables or disables GZ |
| [setMode](#method.setMode) | Sets the mode of the set-top box for a specific duration before returning to normal mode |
| [setNetworkStandbyMode](#method.setNetworkStandbyMode) | This API will be deprecated in the future |
| [setOptOutTelemetry](#method.setOptOutTelemetry) | Sets the telemetry opt-out status |
| [setOvertempGraceInterval](#method.setOvertempGraceInterval) | Sets the over-temperature grace interval value |
| [setPowerState](#method.setPowerState) | Sets the power state of the device |
| [setPreferredStandbyMode](#method.setPreferredStandbyMode) | Sets and persists the preferred standby mode |
| [setTemperatureThresholds](#method.setTemperatureThresholds) | Sets the temperature threshold values |
| [setFriendlyName](#method.setFriendlyName) | Sets the friendly name of device |
| [setBootLoaderSplashScreen](#method.setBootLoaderSplashScreen) | Install or update the BootLoader Splash Screens |
| [setTerritory](#method.setTerritory) | Sets the system territory and region |
| [setTimeZoneDST](#method.setTimeZoneDST) | Sets the system time zone |
| [setWakeupSrcConfiguration](#method.setWakeupSrcConfiguration) | Sets the wakeup source configuration for the input powerState |
| [getWakeupSrcConfiguration](#method.getWakeupSrcConfiguration) | Returns all the supported wakeup configurations and powerState |
| [updateFirmware](#method.updateFirmware) | Initiates a firmware update |
| [uploadLogs](#method.uploadLogs) | Uploads logs to a URL returned by SSR |
| [uploadLogsAsync](#method.uploadLogsAsync) | Starts background process to upload logs |
| [abortLogUpload](#method.abortLogUpload) | Stops background process to upload logs |
| [getThunderStartReason](#method.getThunderStartReason) | Returns the Thunder start reason |
| [SetPrivacyMode](#method.SetPrivacyMode) | Setting Privacy Mode |
| [getPrivacyMode](#method.getPrivacyMode) | Getting Privacy Mode |


<a name="method.clearLastDeepSleepReason"></a>
## *clearLastDeepSleepReason [<sup>method</sup>](#head.Methods)*

Clears the last deep sleep reason.

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
    "method": "org.rdk.System.clearLastDeepSleepReason"
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

Deletes persistent path associated with a callsign.

### Events

No Events

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
    "method": "org.rdk.System.deletePersistentPath",
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

<a name="method.enableMoca"></a>
## *enableMoca [<sup>method</sup>](#head.Methods)*

Enables (or disables) Moca support for the platform.

### Events

No Events

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
    "method": "org.rdk.System.enableMoca",
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

<a name="method.enableXREConnectionRetention"></a>
## *enableXREConnectionRetention [<sup>method</sup>](#head.Methods)*

Enables (or disables) XRE Connection Retention option.

### Events

No Events

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
    "method": "org.rdk.System.enableXREConnectionRetention",
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

<a name="method.fireFirmwarePendingReboot"></a>
## *fireFirmwarePendingReboot [<sup>method</sup>](#head.Methods)*

Notifies the device about a pending reboot.

### Events

| Event | Description |
| :-------- | :-------- |
| [onFirmwarePendingReboot](#event.onFirmwarePendingReboot) | Triggers when the firmware has a pending reboot |
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
    "method": "org.rdk.System.fireFirmwarePendingReboot"
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

### Events

No Events

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
    "method": "org.rdk.System.getAvailableStandbyModes"
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

<a name="method.getCoreTemperature"></a>
## *getCoreTemperature [<sup>method</sup>](#head.Methods)*

Returns the core temperature of the device. Not supported on all devices.

### Events

No Events

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
    "method": "org.rdk.System.getCoreTemperature"
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

<a name="method.getDeviceInfo"></a>
## *getDeviceInfo [<sup>method</sup>](#head.Methods)*

Collects device details. Sample keys include:  
* bluetooth_mac  
* boxIP  
* build_type  
* estb_mac  
* imageVersion  
* rf4ce_mac  
* wifi_mac.

### Events

No Events

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
    "method": "org.rdk.System.getDeviceInfo",
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

### Events

No Events

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
    "method": "org.rdk.System.getDownloadedFirmwareInfo"
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

<a name="method.getFirmwareDownloadPercent"></a>
## *getFirmwareDownloadPercent [<sup>method</sup>](#head.Methods)*

Gets the current download percentage.

### Events

No Events

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
    "method": "org.rdk.System.getFirmwareDownloadPercent"
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

Checks the firmware update information.

### Events

| Event | Description |
| :-------- | :-------- |
| [onFirmwareUpdateInfoReceived](#event.onFirmwareUpdateInfoReceived) | Triggers when the firmware update information is requested |
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
    "method": "org.rdk.System.getFirmwareUpdateInfo",
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

### Events

No Events

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
    "method": "org.rdk.System.getFirmwareUpdateState"
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

<a name="method.getLastDeepSleepReason"></a>
## *getLastDeepSleepReason [<sup>method</sup>](#head.Methods)*

Retrieves the last deep sleep reason.

### Events

No Events

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
    "method": "org.rdk.System.getLastDeepSleepReason"
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

<a name="method.getLastFirmwareFailureReason"></a>
## *getLastFirmwareFailureReason [<sup>method</sup>](#head.Methods)*

Retrieves the last firmware failure reason.

### Events

No Events

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
    "method": "org.rdk.System.getLastFirmwareFailureReason"
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

<a name="method.getLastWakeupKeyCode"></a>
## *getLastWakeupKeyCode [<sup>method</sup>](#head.Methods)*

Returns the last wakeup keycode.

### Events

No Events

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
    "method": "org.rdk.System.getLastWakeupKeyCode"
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

<a name="method.getMacAddresses"></a>
## *getMacAddresses [<sup>method</sup>](#head.Methods)*

Gets the MAC address of the device.

### Events

| Event | Description |
| :-------- | :-------- |
| [onMacAddressesRetreived](#event.onMacAddressesRetreived) | Triggers when the MAC addresses are requested |
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
    "method": "org.rdk.System.getMacAddresses",
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

<a name="method.getMfgSerialNumber"></a>
## *getMfgSerialNumber [<sup>method</sup>](#head.Methods)*

Gets the Manufacturing Serial Number.

### Events

No Events

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
    "method": "org.rdk.System.getMfgSerialNumber"
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

<a name="method.getMilestones"></a>
## *getMilestones [<sup>method</sup>](#head.Methods)*

Returns the list of milestones.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations. [Refer this link for the new api](https://rdkcentral.github.io/rdkservices/#/api/DeviceDiagnosticsPlugin?id=getmilestones)

### Events

No Events

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
    "method": "org.rdk.System.getMilestones"
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

### Events

No Events

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
    "method": "org.rdk.System.getMode"
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

### Events

No Events

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
    "method": "org.rdk.System.getNetworkStandbyMode"
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

<a name="method.getOvertempGraceInterval"></a>
## *getOvertempGraceInterval [<sup>method</sup>](#head.Methods)*

Returns the over-temperature grace interval value. Not supported on all devices.

### Events

No Events

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
    "method": "org.rdk.System.getOvertempGraceInterval"
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

<a name="method.getPlatformConfiguration"></a>
## *getPlatformConfiguration [<sup>method</sup>](#head.Methods)*

Returns the supported features and device/account info.

### Events

No Events

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
    "method": "org.rdk.System.getPlatformConfiguration",
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

<a name="method.getPowerState"></a>
## *getPowerState [<sup>method</sup>](#head.Methods)*

Returns the power state of the device.

### Events

No Events

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
    "method": "org.rdk.System.getPowerState"
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

Returns the power state before reboot.

### Events

No Events

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
    "method": "org.rdk.System.getPowerStateBeforeReboot"
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

<a name="method.getPowerStateIsManagedByDevice"></a>
## *getPowerStateIsManagedByDevice [<sup>method</sup>](#head.Methods)*

Checks whether the power state is managed by the device.

### Events

No Events

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
    "method": "org.rdk.System.getPowerStateIsManagedByDevice"
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

<a name="method.getPreferredStandbyMode"></a>
## *getPreferredStandbyMode [<sup>method</sup>](#head.Methods)*

Returns the preferred standby mode. This method returns an empty string if the preferred mode has not been set.

### Events

No Events

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
    "method": "org.rdk.System.getPreferredStandbyMode"
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

### Events

No Events

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
    "method": "org.rdk.System.getPreviousRebootInfo"
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

<a name="method.getPreviousRebootInfo2"></a>
## *getPreviousRebootInfo2 [<sup>method</sup>](#head.Methods)*

Returns detailed information about a reboot.

### Events

No Events

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
    "method": "org.rdk.System.getPreviousRebootInfo2"
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

### Events

No Events

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
    "method": "org.rdk.System.getPreviousRebootReason"
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

### Events

No Events

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
    "method": "org.rdk.System.getRFCConfig",
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

### Events

No Events

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
    "method": "org.rdk.System.getSerialNumber"
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

### Events

No Events

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
    "method": "org.rdk.System.getStateInfo",
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

<a name="method.getStoreDemoLink"></a>
## *getStoreDemoLink [<sup>method</sup>](#head.Methods)*

Returns the store demo video link.

### Events

No Events

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
    "method": "org.rdk.System.getStoreDemoLink"
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

<a name="method.getSystemVersions"></a>
## *getSystemVersions [<sup>method</sup>](#head.Methods)*

Returns system version details.

### Events

No Events

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
    "method": "org.rdk.System.getSystemVersions"
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

### Events

No Events

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
    "method": "org.rdk.System.getTemperatureThresholds"
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

<a name="method.getFriendlyName"></a>
## *getFriendlyName [<sup>method</sup>](#head.Methods)*

Returns the friendly name set by setFriendlyName API or default value.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.friendlyName | string | The friendly name of the device which used to display on the client device list |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.getFriendlyName"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "friendlyName": "My Device",
        "success": true
    }
}
```

<a name="method.getTerritory"></a>
## *getTerritory [<sup>method</sup>](#head.Methods)*

Gets the configured system territory and region. Territory is a ISO-3166-1 alpha-3 standard (see https://en.wikipedia.org/wiki/ISO_3166-1). Region is a ISO-3166-2 alpha-2 standard (see https://en.wikipedia.org/wiki/ISO_3166-2).

### Events

No Events

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
    "method": "org.rdk.System.getTerritory"
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

<a name="method.getTimeZones"></a>
## *getTimeZones [<sup>method</sup>](#head.Methods)*

(Version2) Gets the available timezones from the system's time zone database. This method is useful for determining time offsets per zone.

### Events

No Events

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
    "method": "org.rdk.System.getTimeZones"
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

<a name="method.getTimeZoneDST"></a>
## *getTimeZoneDST [<sup>method</sup>](#head.Methods)*

Get the configured time zone from the file referenced by `TZ_FILE`. If the time zone is not set, then `null` is returned.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.timeZone | string | The timezone |
| result.accuracy | string | The timezone accuracy (must be one of the following: *INITIAL*, *INTERIM*, *FINAL*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.getTimeZoneDST"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "timeZone": "America/New_York",
        "accuracy": "INITIAL",
        "success": true
    }
}
```

<a name="method.getWakeupReason"></a>
## *getWakeupReason [<sup>method</sup>](#head.Methods)*

Returns the reason for the device coming out of deep sleep.

### Events

No Events

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
    "method": "org.rdk.System.getWakeupReason"
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

### Events

No Events

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
    "method": "org.rdk.System.getXconfParams"
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

<a name="method.isGzEnabled"></a>
## *isGzEnabled [<sup>method</sup>](#head.Methods)*

Checks whether GZ is enabled.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations.

### Events

No Events

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
    "method": "org.rdk.System.isGzEnabled"
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

Checks the telemetry opt-out status.

### Events

No Events

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
    "method": "org.rdk.System.isOptOutTelemetry"
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

### Events

No Events

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
    "method": "org.rdk.System.queryMocaStatus"
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

### Events

| Event | Description |
| :-------- | :-------- |
| [onRebootRequest](#event.onRebootRequest) | Triggers when a device reboot request is made |
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
    "method": "org.rdk.System.reboot",
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

<a name="method.requestSystemUptime"></a>
## *requestSystemUptime [<sup>method</sup>](#head.Methods)*

Returns the device uptime.

### Events

No Events

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
    "method": "org.rdk.System.requestSystemUptime"
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

<a name="method.setBootLoaderPattern"></a>
## *setBootLoaderPattern [<sup>method</sup>](#head.Methods)*

Sets the boot loader pattern mode in MFR.

### Events

No Events

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
    "method": "org.rdk.System.setBootLoaderPattern",
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

<a name="method.setDeepSleepTimer"></a>
## *setDeepSleepTimer [<sup>method</sup>](#head.Methods)*

Sets the deep sleep timeout period.

### Events

No Events

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
    "method": "org.rdk.System.setDeepSleepTimer",
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

Enables or disables the AutoReboot Feature. This method internally sets the tr181 `AutoReboot.Enable` parameter to `true` or `false`.

### Events

No Events

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
    "method": "org.rdk.System.setFirmwareAutoReboot",
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

Delays the firmware reboot.

### Events

No Events

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
    "method": "org.rdk.System.setFirmwareRebootDelay",
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

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations.

### Events

No Events

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
    "method": "org.rdk.System.setGzEnabled",
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

### Events

| Event | Description |
| :-------- | :-------- |
| [onSystemModeChanged](#event.onSystemModeChanged) | Triggers when the system mode is changed successfully |
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
    "method": "org.rdk.System.setMode",
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

This API will be deprecated in the future. Please refer setWakeupSrcConfiguration to Migrate. This API Enables or disables the network standby mode of the device. If network standby is enabled, the device supports `WakeOnLAN` and `WakeOnWLAN` actions in STR (S3) mode.

### Events

No Events

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
    "method": "org.rdk.System.setNetworkStandbyMode",
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

<a name="method.setOptOutTelemetry"></a>
## *setOptOutTelemetry [<sup>method</sup>](#head.Methods)*

Sets the telemetry opt-out status.

### Events

No Events

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
    "method": "org.rdk.System.setOptOutTelemetry",
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

<a name="method.setOvertempGraceInterval"></a>
## *setOvertempGraceInterval [<sup>method</sup>](#head.Methods)*

Sets the over-temperature grace interval value. Not supported on all devices.

### Events

No Events

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
    "method": "org.rdk.System.setOvertempGraceInterval",
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

<a name="method.setPowerState"></a>
## *setPowerState [<sup>method</sup>](#head.Methods)*

Sets the power state of the device.

### Events

| Event | Description |
| :-------- | :-------- |
| [onSystemPowerStateChanged](#event.onSystemPowerStateChanged) | Triggers when the system power state changes |
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
    "method": "org.rdk.System.setPowerState",
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

Sets and persists the preferred standby mode. See [getAvailableStandbyModes](#getAvailableStandbyModes) for valid modes. Invoking this function does not change the power state of the device. It only sets the user preference for the preferred action when the [setPowerState](#setPowerState) method is invoked with a value of `STANDBY`.

### Events

No Events

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
    "method": "org.rdk.System.setPreferredStandbyMode",
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

### Events

No Events

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
    "method": "org.rdk.System.setTemperatureThresholds",
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

<a name="method.setFriendlyName"></a>
## *setFriendlyName [<sup>method</sup>](#head.Methods)*

Sets the friendly name of device. It allows an applications to set friendly name value which could be used by different applications to list this device on client mobile application. The provided name should not be empty. Friendly name is persisted on device and reuse after each reboot until updated by the user.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.friendlyName | string | The friendly name of the device which used to display on the client device list |

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
    "method": "org.rdk.System.setFriendlyName",
    "params": {
        "friendlyName": "My Device"
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

<a name="method.setBootLoaderSplashScreen"></a>
## *setBootLoaderSplashScreen [<sup>method</sup>](#head.Methods)*

Install or update the BootLoader Splash Screens. This is typically called by Resident Apps to configure the Splash Screen shown during device boot up.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.path | string | Path to the pre-downloaded splash screen file location. Full path with file name |

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
    "method": "org.rdk.System.setBootLoaderSplashScreen",
    "params": {
        "path": "/tmp/osd1"
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

<a name="method.setTerritory"></a>
## *setTerritory [<sup>method</sup>](#head.Methods)*

Sets the system territory and region.Territory is a ISO-3166-1 alpha-3 standard (see https://en.wikipedia.org/wiki/ISO_3166-1). Region is a ISO-3166-2 alpha-2 standard (see https://en.wikipedia.org/wiki/ISO_3166-2).

### Events

| Event | Description |
| :-------- | :-------- |
| [onTerritoryChanged](#event.onTerritoryChanged) | Triggered when territory is set |
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
    "method": "org.rdk.System.setTerritory",
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

<a name="method.setTimeZoneDST"></a>
## *setTimeZoneDST [<sup>method</sup>](#head.Methods)*

Sets the system time zone. See `getTimeZones` to get a list of available timezones on the system.

### Events

| Event | Description |
| :-------- | :-------- |
| [onTimeZoneDSTChanged](#event.onTimeZoneDSTChanged) | Triggered when device time zone changed |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.timeZone | string | The timezone |
| params?.accuracy | string | <sup>*(optional)*</sup> The timezone accuracy (must be one of the following: *INITIAL*, *INTERIM*, *FINAL*) |

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
    "method": "org.rdk.System.setTimeZoneDST",
    "params": {
        "timeZone": "America/New_York",
        "accuracy": "INITIAL"
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

Sets the wakeup source configuration for the input powerState. if you are using setNetworkStandbyMode API, Please do not use this API to set LAN and WIFI wakeup. Please migrate to setWakeupSrcConfiguration API to control all wakeup source settings. This API does not persist. Please call this API on Every bootup to set the values.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.powerState | string | <sup>*(optional)*</sup> The power state (must be one of the following: *STANDBY*, *DEEP_SLEEP*, *LIGHT_SLEEP*, *ON*) |
| params.wakeupSources | array | Array of Key value pair with wake up sources and its configurations |
| params.wakeupSources[#] | object |  |
| params.wakeupSources[#]?.WAKEUPSRC_VOICE | boolean | <sup>*(optional)*</sup> Voice Wake up |
| params.wakeupSources[#]?.WAKEUPSRC_PRESENCE_DETECTION | boolean | <sup>*(optional)*</sup> Presense detection wake up |
| params.wakeupSources[#]?.WAKEUPSRC_BLUETOOTH | boolean | <sup>*(optional)*</sup> Bluetooth Wakeup |
| params.wakeupSources[#]?.WAKEUPSRC_WIFI | boolean | <sup>*(optional)*</sup> WiFi Wake up |
| params.wakeupSources[#]?.WAKEUPSRC_IR | boolean | <sup>*(optional)*</sup> IR Remote Wake up |
| params.wakeupSources[#]?.WAKEUPSRC_POWER_KEY | boolean | <sup>*(optional)*</sup> Power Button Wake up - GPIO |
| params.wakeupSources[#]?.WAKEUPSRC_CEC | boolean | <sup>*(optional)*</sup> HDMI CEC commadn Wake up |
| params.wakeupSources[#]?.WAKEUPSRC_LAN | boolean | <sup>*(optional)*</sup> LAN wake up |
| params.wakeupSources[#]?.WAKEUPSRC_TIMER | boolean | <sup>*(optional)*</sup> TImer Wake up |

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
    "method": "org.rdk.System.setWakeupSrcConfiguration",
    "params": {
        "powerState": "ON",
        "wakeupSources": [
            {
                "WAKEUPSRC_VOICE": true,
                "WAKEUPSRC_PRESENCE_DETECTION": true,
                "WAKEUPSRC_BLUETOOTH": true,
                "WAKEUPSRC_WIFI": true,
                "WAKEUPSRC_IR": true,
                "WAKEUPSRC_POWER_KEY": true,
                "WAKEUPSRC_CEC": true,
                "WAKEUPSRC_LAN": true,
                "WAKEUPSRC_TIMER": true
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
        "success": true
    }
}
```

<a name="method.getWakeupSrcConfiguration"></a>
## *getWakeupSrcConfiguration [<sup>method</sup>](#head.Methods)*

Returns all the supported wakeup configurations and powerState.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.powerState | string | The power state (must be one of the following: *STANDBY*, *DEEP_SLEEP*, *LIGHT_SLEEP*, *ON*) |
| result.wakeupSources | array | Array of Key value pair with wake up sources and its configurations |
| result.wakeupSources[#] | object |  |
| result.wakeupSources[#]?.WAKEUPSRC_VOICE | boolean | <sup>*(optional)*</sup> Voice Wake up |
| result.wakeupSources[#]?.WAKEUPSRC_PRESENCE_DETECTION | boolean | <sup>*(optional)*</sup> Presense detection wake up |
| result.wakeupSources[#]?.WAKEUPSRC_BLUETOOTH | boolean | <sup>*(optional)*</sup> Bluetooth Wakeup |
| result.wakeupSources[#]?.WAKEUPSRC_WIFI | boolean | <sup>*(optional)*</sup> WiFi Wake up |
| result.wakeupSources[#]?.WAKEUPSRC_IR | boolean | <sup>*(optional)*</sup> IR Remote Wake up |
| result.wakeupSources[#]?.WAKEUPSRC_POWER_KEY | boolean | <sup>*(optional)*</sup> Power Button Wake up - GPIO |
| result.wakeupSources[#]?.WAKEUPSRC_CEC | boolean | <sup>*(optional)*</sup> HDMI CEC commadn Wake up |
| result.wakeupSources[#]?.WAKEUPSRC_LAN | boolean | <sup>*(optional)*</sup> LAN wake up |
| result.wakeupSources[#]?.WAKEUPSRC_TIMER | boolean | <sup>*(optional)*</sup> TImer Wake up |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.getWakeupSrcConfiguration"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "powerState": "ON",
        "wakeupSources": [
            {
                "WAKEUPSRC_VOICE": true,
                "WAKEUPSRC_PRESENCE_DETECTION": true,
                "WAKEUPSRC_BLUETOOTH": true,
                "WAKEUPSRC_WIFI": true,
                "WAKEUPSRC_IR": true,
                "WAKEUPSRC_POWER_KEY": true,
                "WAKEUPSRC_CEC": true,
                "WAKEUPSRC_LAN": true,
                "WAKEUPSRC_TIMER": true
            }
        ],
        "success": true
    }
}
```

<a name="method.updateFirmware"></a>
## *updateFirmware [<sup>method</sup>](#head.Methods)*

Initiates a firmware update. This method has no affect if an update is not available.

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
    "method": "org.rdk.System.updateFirmware"
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

Uploads logs to a URL returned by SSR.

> This API is **deprecated** and may be removed in the future. It is no longer recommended for use in new implementations.

### Events

No Events

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
    "method": "org.rdk.System.uploadLogs",
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

<a name="method.uploadLogsAsync"></a>
## *uploadLogsAsync [<sup>method</sup>](#head.Methods)*

Starts background process to upload logs.

### Events

| Event | Description |
| :-------- | :-------- |
| [onLogUpload](#event.onLogUpload) | Triggered when logs upload process is done |
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
    "method": "org.rdk.System.uploadLogsAsync"
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

<a name="method.abortLogUpload"></a>
## *abortLogUpload [<sup>method</sup>](#head.Methods)*

Stops background process to upload logs.

### Events

| Event | Description |
| :-------- | :-------- |
| [onLogUpload](#event.onLogUpload) | Triggered when logs upload process is stopped |
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
    "method": "org.rdk.System.abortLogUpload"
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

<a name="method.getThunderStartReason"></a>
## *getThunderStartReason [<sup>method</sup>](#head.Methods)*

Returns the Thunder start reason.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.startReason | string | Thunder start reason (must be one of the following: *NORMAL*, *RESTART*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.getThunderStartReason"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "startReason": "NORMAL",
        "success": true
    }
}
```

<a name="method.SetPrivacyMode"></a>
## *SetPrivacyMode [<sup>method</sup>](#head.Methods)*

Setting Privacy Mode.

### Events

| Event | Description |
| :-------- | :-------- |
| [onPrivacyModeChanged](#event.onPrivacyModeChanged) | Triggered when the Privacy Mode changes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.privacyMode | string | New Privacy Mode |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | On success null will be returned |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.SetPrivacyMode",
    "params": {
        "privacyMode": "DO_NOT_SHARE"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="method.getPrivacyMode"></a>
## *getPrivacyMode [<sup>method</sup>](#head.Methods)*

Getting Privacy Mode.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | string | Current Privacy Mode |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.System.getPrivacyMode"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "DO_NOT_SHARE"
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.System plugin:

org.rdk.System interface events:

| Event | Description |
| :-------- | :-------- |
| [onFirmwarePendingReboot](#event.onFirmwarePendingReboot) | Triggered when the `fireFirmwarePendingReboot` method is invoked |
| [onFirmwareUpdateInfoReceived](#event.onFirmwareUpdateInfoReceived) | Triggered when the `getFirmwareUpdateInfo` asynchronous method is invoked |
| [onFirmwareUpdateStateChange](#event.onFirmwareUpdateStateChange) | Triggered when the state of a firmware update changes |
| [onRecoveryStateChange](#event.onRecoveryStateChange) | Triggered when the state of red recovery transistion State details are:  `0`: Recovery completed `1`: Recovery Started - Fatal error detected `2`: Recovery FW Downloaded `3`: Recovery FW programmed |
| [onMacAddressesRetreived](#event.onMacAddressesRetreived) | Triggered when the `getMacAddresses` asynchronous method is invoked |
| [onNetworkStandbyModeChanged](#event.onNetworkStandbyModeChanged) | Triggered when the network standby mode setting changes |
| [onRebootRequest](#event.onRebootRequest) | Triggered when an application invokes the reboot method |
| [onSystemClockSet](#event.onSystemClockSet) | Triggered when the clock on the set-top device is updated |
| [onSystemModeChanged](#event.onSystemModeChanged) | Triggered when the device operating mode changes |
| [onSystemPowerStateChanged](#event.onSystemPowerStateChanged) | Triggered when the power manager detects a device power state change |
| [onFriendlyNameChanged](#event.onFriendlyNameChanged) | Triggered when the device friendly name change |
| [onTemperatureThresholdChanged](#event.onTemperatureThresholdChanged) | Triggered when the device temperature changes beyond the `WARN` or `MAX` limits (see `setTemperatureThresholds`) |
| [onTerritoryChanged](#event.onTerritoryChanged) | Triggered when the device territory changed |
| [onDeviceMgtUpdateReceived](#event.onDeviceMgtUpdateReceived) | Triggered when the device management update completes |
| [onTimeZoneDSTChanged](#event.onTimeZoneDSTChanged) | Triggered when device time zone changed |
| [onLogUpload](#event.onLogUpload) | Triggered when logs upload process is done or stopped |
| [onPrivacyModeChanged](#event.onPrivacyModeChanged) | Triggered after the Privacy Mode changes (see `SetPrivacyMode`) |


<a name="event.onFirmwarePendingReboot"></a>
## *onFirmwarePendingReboot [<sup>event</sup>](#head.Notifications)*

Triggered when the `fireFirmwarePendingReboot` method is invoked.

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
    "method": "client.events.onFirmwarePendingReboot",
    "params": {
        "fireFirmwarePendingReboot": 3,
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
| params?.firmwareUpdateVersion | string | <sup>*(optional)*</sup> The next firmware update version |
| params.rebootImmediately | boolean | The value `true` indicates that the device has to be rebooted immediately or `false` otherwise |
| params.updateAvailable | boolean | The value `false` indicates that there is no update available, either because there was no firmware update version returned from XCONF, or because the version returned from XCONF matches the version already on the device. The value of `true` indicates there is a firmware version available for update |
| params.updateAvailableEnum | integer | The update available details (must be one of the following: *0*, *1*, *2*, *3*) |
| params.success | boolean | Whether the request succeeded |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onFirmwareUpdateInfoReceived",
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
    "method": "client.events.onFirmwareUpdateStateChange",
    "params": {
        "firmwareUpdateStateChange": 5
    }
}
```

<a name="event.onRecoveryStateChange"></a>
## *onRecoveryStateChange [<sup>event</sup>](#head.Notifications)*

Triggered when the state of red recovery transistion State details are:  `0`: Recovery completed `1`: Recovery Started - Fatal error detected `2`: Recovery FW Downloaded `3`: Recovery FW programmed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.recoveryStateChange | integer | The state (must be one of the following: *Recovery Completed*, *Recovery Started*, *Recovery Downloaded*, *Recovery Programmed*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onRecoveryStateChange",
    "params": {
        "recoveryStateChange": 3
    }
}
```

<a name="event.onMacAddressesRetreived"></a>
## *onMacAddressesRetreived [<sup>event</sup>](#head.Notifications)*

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
    "method": "client.events.onMacAddressesRetreived",
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

<a name="event.onNetworkStandbyModeChanged"></a>
## *onNetworkStandbyModeChanged [<sup>event</sup>](#head.Notifications)*

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
    "method": "client.events.onNetworkStandbyModeChanged",
    "params": {
        "nwStandby": true
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
    "method": "client.events.onRebootRequest",
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
    "method": "client.events.onSystemClockSet"
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
    "method": "client.events.onSystemModeChanged",
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
| params.currentPowerState | string | The current power state |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onSystemPowerStateChanged",
    "params": {
        "powerState": "ON",
        "currentPowerState": "ON"
    }
}
```

<a name="event.onFriendlyNameChanged"></a>
## *onFriendlyNameChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the device friendly name change.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.friendlyName | string | The friendly name of the device which used to display on the client device list |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onFriendlyNameChanged",
    "params": {
        "friendlyName": "My Device"
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
| params.temperature | string | The temperature |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onTemperatureThresholdChanged",
    "params": {
        "thresholdType": "MAX",
        "exceeded": true,
        "temperature": "48.000000"
    }
}
```

<a name="event.onTerritoryChanged"></a>
## *onTerritoryChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the device territory changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.oldTerritory | string |  old territory |
| params.newTerritory | string |  new territory |
| params.oldRegion | string | old region |
| params.newRegion | string | new region |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onTerritoryChanged",
    "params": {
        "oldTerritory": "GBR",
        "newTerritory": "USA",
        "oldRegion": "GB-ENG",
        "newRegion": "US-NY"
    }
}
```

<a name="event.onDeviceMgtUpdateReceived"></a>
## *onDeviceMgtUpdateReceived [<sup>event</sup>](#head.Notifications)*

Triggered when the device management update completes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.source | string | Source information from where the event on update is posted |
| params.type | string |  Type of Update received currently it will be used as initial |
| params.success | boolean | Status information of update whether success or failure |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onDeviceMgtUpdateReceived",
    "params": {
        "source": "rfc",
        "type": "initial",
        "success": true
    }
}
```

<a name="event.onTimeZoneDSTChanged"></a>
## *onTimeZoneDSTChanged [<sup>event</sup>](#head.Notifications)*

Triggered when device time zone changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.oldTimeZone | string | old time zone |
| params.newTimeZone | string | new time zone |
| params.oldAccuracy | string | old time zone accuracy |
| params.newAccuracy | string | new time zone accuracy |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onTimeZoneDSTChanged",
    "params": {
        "oldTimeZone": "America/New_York",
        "newTimeZone": "Europe/London",
        "oldAccuracy": "INITIAL",
        "newAccuracy": "FINAL"
    }
}
```

<a name="event.onLogUpload"></a>
## *onLogUpload [<sup>event</sup>](#head.Notifications)*

Triggered when logs upload process is done or stopped.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logUploadStatus | string | Upload status (must be one of the following: *UPLOAD_SUCCESS*, *UPLOAD_FAILURE*, *UPLOAD_ABORTED*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onLogUpload",
    "params": {
        "logUploadStatus": "UPLOAD_SUCCESS"
    }
}
```

<a name="event.onPrivacyModeChanged"></a>
## *onPrivacyModeChanged [<sup>event</sup>](#head.Notifications)*

Triggered after the Privacy Mode changes (see `SetPrivacyMode`).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.privacyMode | string | Receive Privacy Mode changes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onPrivacyModeChanged",
    "params": {
        "privacyMode": "DO_NOT_SHARE"
    }
}
```

