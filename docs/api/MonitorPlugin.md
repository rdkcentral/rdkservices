<!-- Generated automatically, DO NOT EDIT! -->
<a name="Monitor_Plugin"></a>
# Monitor Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A Monitor plugin for Thunder framework.

### Table of Contents

- [Introduction](#Introduction)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Properties](#Properties)
- [Notifications](#Notifications)

<a name="Introduction"></a>
# Introduction

<a name="Scope"></a>
## Scope

This document describes purpose and functionality of the Monitor plugin. It includes detailed specification about its configuration, methods and properties provided, as well as notifications sent.

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

The `Monitor` plugin provides a watchdog-like functionality for framework processes.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *Monitor*) |
| classname | string | Class name: *Monitor* |
| locator | string | Library name: *libWPEFrameworkMonitor.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the Monitor plugin:

Monitor interface methods:

| Method | Description |
| :-------- | :-------- |
| [restartlimits](#restartlimits) | Sets new restart limits for a service |
| [resetstats](#resetstats) | Resets memory and process statistics for a single service watched by the Monitor |


<a name="restartlimits"></a>
## *restartlimits*

Sets new restart limits for a service.
 ### Events 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.callsign | string | The callsign of a service for which measurement snapshots are reset |
| params.restart | object |  |
| params.restart.limit | number | Maximum number of restarts to be attempted. If the limit parameter is not passed in request then, the default value is 0 |
| params.restart.window | number | Time period (in seconds) within which failures must happen for the limit to be considered crossed. If the window parameter is not passed in request then, the default value is 0 |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "Monitor.1.restartlimits",
    "params": {
        "callsign": "WebServer",
        "restart": {
            "limit": 3,
            "window": 60
        }
    }
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

<a name="resetstats"></a>
## *resetstats*

Resets memory and process statistics for a single service watched by the Monitor.
 ### Events 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.callsign | string | The callsign of a service for which statistics are reset |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Measurements for the service before reset |
| result.measurements | object | Measurements for the service |
| result.measurements.resident | object | Resident memory measurement |
| result.measurements.resident.min | number | Minimal value measured |
| result.measurements.resident.max | number | Maximal value measured |
| result.measurements.resident.average | number | Average of all measurements |
| result.measurements.resident.last | number | Last measured value |
| result.measurements.allocated | object | Allocated memory measurement |
| result.measurements.allocated.min | number | Minimal value measured |
| result.measurements.allocated.max | number | Maximal value measured |
| result.measurements.allocated.average | number | Average of all measurements |
| result.measurements.allocated.last | number | Last measured value |
| result.measurements.shared | object | Shared memory measurement |
| result.measurements.shared.min | number | Minimal value measured |
| result.measurements.shared.max | number | Maximal value measured |
| result.measurements.shared.average | number | Average of all measurements |
| result.measurements.shared.last | number | Last measured value |
| result.measurements.process | object | Processes measurement |
| result.measurements.process.min | number | Minimal value measured |
| result.measurements.process.max | number | Maximal value measured |
| result.measurements.process.average | number | Average of all measurements |
| result.measurements.process.last | number | Last measured value |
| result.measurements.operational | boolean | Whether the service is up and running |
| result.measurements.count | number | Number of measurements |
| result.observable | string | A callsign of the watched service |
| result.restart | object | Restart limits for failures applying to the service |
| result.restart.limit | number | Maximum number of restarts to be attempted. If the limit parameter is not passed in request then, the default value is 0 |
| result.restart.window | number | Time period (in seconds) within which failures must happen for the limit to be considered crossed. If the window parameter is not passed in request then, the default value is 0 |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "Monitor.1.resetstats",
    "params": {
        "callsign": "WebServer"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "measurements": {
            "resident": {
                "min": 0,
                "max": 100,
                "average": 50,
                "last": 100
            },
            "allocated": {
                "min": 0,
                "max": 100,
                "average": 50,
                "last": 100
            },
            "shared": {
                "min": 0,
                "max": 100,
                "average": 50,
                "last": 100
            },
            "process": {
                "min": 0,
                "max": 100,
                "average": 50,
                "last": 100
            },
            "operational": true,
            "count": 100
        },
        "observable": "callsign",
        "restart": {
            "limit": 3,
            "window": 60
        }
    }
}
```

<a name="Properties"></a>
# Properties

The following properties are provided by the Monitor plugin:

Monitor interface properties:

| Property | Description |
| :-------- | :-------- |
| [status](#status) <sup>RO</sup> | Service statistics |


<a name="status"></a>
## *status [<sup>property</sup>](#Properties)*

Provides access to the service statistics.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | Service statistics |
| (property)[#] | object |  |
| (property)[#].measurements | object | Measurements for the service |
| (property)[#].measurements.resident | object | Resident memory measurement |
| (property)[#].measurements.resident.min | number | Minimal value measured |
| (property)[#].measurements.resident.max | number | Maximal value measured |
| (property)[#].measurements.resident.average | number | Average of all measurements |
| (property)[#].measurements.resident.last | number | Last measured value |
| (property)[#].measurements.allocated | object | Allocated memory measurement |
| (property)[#].measurements.allocated.min | number | Minimal value measured |
| (property)[#].measurements.allocated.max | number | Maximal value measured |
| (property)[#].measurements.allocated.average | number | Average of all measurements |
| (property)[#].measurements.allocated.last | number | Last measured value |
| (property)[#].measurements.shared | object | Shared memory measurement |
| (property)[#].measurements.shared.min | number | Minimal value measured |
| (property)[#].measurements.shared.max | number | Maximal value measured |
| (property)[#].measurements.shared.average | number | Average of all measurements |
| (property)[#].measurements.shared.last | number | Last measured value |
| (property)[#].measurements.process | object | Processes measurement |
| (property)[#].measurements.process.min | number | Minimal value measured |
| (property)[#].measurements.process.max | number | Maximal value measured |
| (property)[#].measurements.process.average | number | Average of all measurements |
| (property)[#].measurements.process.last | number | Last measured value |
| (property)[#].measurements.operational | boolean | Whether the service is up and running |
| (property)[#].measurements.count | number | Number of measurements |
| (property)[#].observable | string | A callsign of the watched service |
| (property)[#].restart | object | Restart limits for failures applying to the service |
| (property)[#].restart.limit | number | Maximum number of restarts to be attempted. If the limit parameter is not passed in request then, the default value is 0 |
| (property)[#].restart.window | number | Time period (in seconds) within which failures must happen for the limit to be considered crossed. If the window parameter is not passed in request then, the default value is 0 |

> The *callsign* argument shall be passed as the index to the property, e.g. *Monitor.1.status@WebServer*. If omitted then, all the observed objects are returned on read. Invalid callsign results are in empty result.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "Monitor.1.status@WebServer"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        {
            "measurements": {
                "resident": {
                    "min": 0,
                    "max": 100,
                    "average": 50,
                    "last": 100
                },
                "allocated": {
                    "min": 0,
                    "max": 100,
                    "average": 50,
                    "last": 100
                },
                "shared": {
                    "min": 0,
                    "max": 100,
                    "average": 50,
                    "last": 100
                },
                "process": {
                    "min": 0,
                    "max": 100,
                    "average": 50,
                    "last": 100
                },
                "operational": true,
                "count": 100
            },
            "observable": "callsign",
            "restart": {
                "limit": 3,
                "window": 60
            }
        }
    ]
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the Monitor plugin:

Monitor interface events:

| Event | Description |
| :-------- | :-------- |
| [action](#action) | Signals an action taken by the Monitor |


<a name="action"></a>
## *action*

Signals an action taken by the Monitor.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.callsign | string | Callsign of the service the Monitor acted upon |
| params.action | string | The action executed by the Monitor on a service (must be one of the following: *Activate*, *Deactivate*, *StoppedRestarting*) |
| params.reason | string | A message describing the reason the action was taken |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.action",
    "params": {
        "callsign": "WebServer",
        "action": "Deactivate",
        "reason": "EXCEEDED_MEMORY"
    }
}
```

