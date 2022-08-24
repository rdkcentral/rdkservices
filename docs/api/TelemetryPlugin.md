<!-- Generated automatically, DO NOT EDIT! -->
<a name="Telemetry_Plugin"></a>
# Telemetry Plugin

**Version: 1.0.0**

A org.rdk.Telemetry plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)

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


<a name="setReportProfileStatus"></a>
## *setReportProfileStatus*

Sets the status of telemetry reporting.

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
    "method": "org.rdk.Telemetry.1.setReportProfileStatus",
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
    "method": "org.rdk.Telemetry.1.logApplicationEvent",
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

