<!-- Generated automatically, DO NOT EDIT! -->
<a name="Telemetry_Plugin"></a>
# Telemetry Plugin

**Version: [1.2.1](https://github.com/rdkcentral/rdkservices/blob/main/Telemetry/CHANGELOG.md)**

A org.rdk.Telemetry plugin for Thunder framework.

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

The `Telemetry` plugin allows you to persist event data for monitoring applications.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.Telemetry*) |
| classname | string | Class name: *org.rdk.Telemetry* |
| locator | string | Library name: *libWPEFrameworkTelemetry.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.Telemetry plugin:

Telemtry interface methods:

| Method | Description |
| :-------- | :-------- |
| [setReportProfileStatus](#setReportProfileStatus) | Sets the status of telemetry reporting |
| [logApplicationEvent](#logApplicationEvent) | Logs an application event |
| [uploadReport](#uploadReport) | Triggers processing and uploading of telemetry report for legacy Xconf based configuration |
| [abortReport](#abortReport) | Makes request to Telemetry service to abort report upload |


<a name="setReportProfileStatus"></a>
## *setReportProfileStatus*

Sets the status of telemetry reporting.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.status | string | The report status (must be one of the following: *STARTED*, *COMPLETE*) |

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
    "method": "org.rdk.Telemetry.setReportProfileStatus",
    "params": {
        "status": "STARTED"
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

<a name="logApplicationEvent"></a>
## *logApplicationEvent*

Logs an application 

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.eventName | string | The event name |
| params.eventValue | string | The event value |

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
    "method": "org.rdk.Telemetry.logApplicationEvent",
    "params": {
        "eventName": "...",
        "eventValue": "..."
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

<a name="uploadReport"></a>
## *uploadReport*

Triggers processing and uploading of telemetry report for legacy Xconf based configuration.

### Events

| Event | Description |
| :-------- | :-------- |
| [onReportUpload](#onReportUpload) | Triggered by callback from Telemetry after report uploading |
### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 25 | ```ERROR_RPC_CALL_FAILED``` | RBus call report uploading failed |
| 6 | ```ERROR_OPENING_FAILED``` | Failed to open RBus handle |
| 43 | ```ERROR_NOT_EXIST``` | Built with no support for RBus |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Telemetry.uploadReport"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": null
}
```

<a name="abortReport"></a>
## *abortReport*

Makes request to Telemetry service to abort report upload.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 25 | ```ERROR_RPC_CALL_FAILED``` | RBus call report uploading failed |
| 6 | ```ERROR_OPENING_FAILED``` | Failed to open RBus handle |
| 43 | ```ERROR_NOT_EXIST``` | Built with no support for RBus |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.Telemetry.abortReport"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": null
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.Telemetry plugin:

Telemtry interface events:

| Event | Description |
| :-------- | :-------- |
| [onReportUpload](#onReportUpload) | Triggered by callback from Telemetry after report uploading |


<a name="onReportUpload"></a>
## *onReportUpload*

Triggered by callback from Telemetry after report uploading.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.telemetryUploadStatus | string | Indicates if the upload was successful |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onReportUpload",
    "params": {
        "telemetryUploadStatus": "UPLOAD_SUCCESS"
    }
}
```

