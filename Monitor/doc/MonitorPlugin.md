<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.Monitor_Plugin"></a>
# Monitor Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

Monitor plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)
- [Properties](#head.Properties)
- [Notifications](#head.Notifications)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the Monitor plugin. It includes detailed specification of its configuration, methods and properties provided, as well as notifications sent.

<a name="head.Case_Sensitivity"></a>
## Case Sensitivity

All identifiers on the interface described in this document are case-sensitive. Thus, unless stated otherwise, all keywords, entities, properties, relations and actions should be treated as such.

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

The Monitor plugin provides a watchdog-like functionality for framework processes.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *Monitor*) |
| classname | string | Class name: *Monitor* |
| locator | string | Library name: *libWPEFrameworkMonitor.so* |
| autostart | boolean | Determines if the plugin is to be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the Monitor plugin:

Monitor interface methods:

| Method | Description |
| :-------- | :-------- |
| [restartlimits](#method.restartlimits) | Sets new restart limits for a service |
| [resetstats](#method.resetstats) | Resets memory and process statistics for a single service watched by the Monitor |

<a name="method.restartlimits"></a>
## *restartlimits <sup>method</sup>*

Sets new restart limits for a service.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.callsign | string | The callsign of a service to reset measurements snapshot of |
| params.restart | object |  |
| params.restart.limit | number | Maximum number or restarts to be attempted |
| params.restart.window | number | Time period (in seconds) within which failures must happen for the limit to be considered crossed |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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
    "id": 1234567890,
    "result": null
}
```
<a name="method.resetstats"></a>
## *resetstats <sup>method</sup>*

Resets memory and process statistics for a single service watched by the Monitor.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.callsign | string | The callsign of a service to reset statistics of |

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
| result.restart | object | Restart limits for memory/operational failures applying to the service |
| result.restart.limit | number | Maximum number or restarts to be attempted |
| result.restart.window | number | Time period (in seconds) within which failures must happen for the limit to be considered crossed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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
    "id": 1234567890,
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
<a name="head.Properties"></a>
# Properties

The following properties are provided by the Monitor plugin:

Monitor interface properties:

| Property | Description |
| :-------- | :-------- |
| [status](#property.status) <sup>RO</sup> | Service statistics |

<a name="property.status"></a>
## *status <sup>property</sup>*

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
| (property)[#].restart | object | Restart limits for memory/operational failures applying to the service |
| (property)[#].restart.limit | number | Maximum number or restarts to be attempted |
| (property)[#].restart.window | number | Time period (in seconds) within which failures must happen for the limit to be considered crossed |

> The *callsign* shall be passed as the index to the property, e.g. *Monitor.1.status@WebServer*. If omitted then all observed objects will be returned on read.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "Monitor.1.status@WebServer"
}
```
#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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
<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers.Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the Monitor plugin:

Monitor interface events:

| Event | Description |
| :-------- | :-------- |
| [action](#event.action) | Signals an action taken by the Monitor |

<a name="event.action"></a>
## *action <sup>event</sup>*

Signals an action taken by the Monitor.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.callsign | string | Callsign of the service the Monitor acted upon |
| params.action | string | The action executed by the Monitor on a service. One of: "Activate", "Deactivate", "StoppedRestarting" |
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
