<!-- Generated automatically, DO NOT EDIT! -->
<a name="DeviceDiagnostics_Plugin"></a>
# DeviceDiagnostics Plugin

**Version: [1.1.2](https://github.com/rdkcentral/rdkservices/blob/main/DeviceDiagnostics/CHANGELOG.md)**

A org.rdk.DeviceDiagnostics plugin for Thunder framework.

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

The `DeviceDiagnostics` plugin provides additional diagnostics information which includes device configuration and AV decoder status.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.DeviceDiagnostics*) |
| classname | string | Class name: *org.rdk.DeviceDiagnostics* |
| locator | string | Library name: *libWPEFrameworkDeviceDiagnostics.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.DeviceDiagnostics plugin:

DeviceDiagnostics interface methods:

| Method | Description |
| :-------- | :-------- |
| [getConfiguration](#getConfiguration) | Gets the values associated with the corresponding property names |
| [getMilestones](#getMilestones) | Returns the list of milestones |
| [logMilestone](#logMilestone) | Log marker string to rdk milestones log |
| [getAVDecoderStatus](#getAVDecoderStatus) | Gets the most active status of audio/video decoder/pipeline |


<a name="getConfiguration"></a>
## *getConfiguration*

Gets the values associated with the corresponding property names.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.names | array | String array of property names |
| params.names[#] | string | Property names as represented in the data model like `Device.X_CISCO_COM_LED.RedPwm`, `Device.DeviceInfo.Manufacturer`, `Device.DeviceInfo.UpTime`, `Device.DeviceInfo.ProcessStatus.CPUUsage`, etc |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.paramList | array | An array of JSON objects with the specified properties and their values |
| result.paramList[#] | object |  |
| result.paramList[#].name | string | The property name; Empty, if the property name is not supported |
| result.paramList[#].value | string | The property value; Empty, if the property name is not supported |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DeviceDiagnostics.getConfiguration",
    "params": {
        "names": [
            "Device.X_CISCO_COM_LED.RedPwm"
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
        "paramList": [
            {
                "name": "Device.X_CISCO_COM_LED.RedPwm",
                "value": "123"
            }
        ],
        "success": true
    }
}
```

<a name="getMilestones"></a>
## *getMilestones*

Returns the list of milestones.

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
    "method": "org.rdk.DeviceDiagnostics.getMilestones"
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

<a name="logMilestone"></a>
## *logMilestone*

Log marker string to rdk milestones log.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.marker | string | Milestone marker string |

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
    "method": "org.rdk.DeviceDiagnostics.logMilestone",
    "params": {
        "marker": "..."
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

<a name="getAVDecoderStatus"></a>
## *getAVDecoderStatus*

Gets the most active status of audio/video decoder/pipeline. This API doesn't track individual pipelines. It will aggregate and report the pipeline status, and the pipeline states are prioritized from High to Low (`ACTIVE`, `PAUSED`, and `IDLE`). Therefore, if any of the pipelines is in active state, then `getAVDecoderStatus` will return `ACTIVE`. If none of the pipelines are active but one is in a paused state, then `getAVDecoderStatus` will return `PAUSED`, and if all the pipelines are idle only then, `IDLE` will be returned.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.AVDecoderStatus | string | The status. If AV decoder status is not supported, the default state will always be IDLE. (must be one of the following: *ACTIVE*, *PAUSED*, *IDLE*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.DeviceDiagnostics.getAVDecoderStatus"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "AVDecoderStatus": "ACTIVE",
        "success": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.DeviceDiagnostics plugin:

DeviceDiagnostics interface events:

| Event | Description |
| :-------- | :-------- |
| [onAVDecoderStatusChanged](#onAVDecoderStatusChanged) | Triggered when the most active status of audio/video decoder/pipeline changes |


<a name="onAVDecoderStatusChanged"></a>
## *onAVDecoderStatusChanged*

Triggered when the most active status of audio/video decoder/pipeline changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.AVDecoderStatus | string | The status. If AV decoder status is not supported, the default state will always be IDLE. (must be one of the following: *ACTIVE*, *PAUSED*, *IDLE*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onAVDecoderStatusChanged",
    "params": {
        "AVDecoderStatus": "ACTIVE"
    }
}
```

