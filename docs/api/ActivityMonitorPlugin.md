<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.ActivityMonitor_Plugin"></a>
# ActivityMonitor Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.ActivityMonitor plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.ActivityMonitor plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `ActivityMonitor` plugin allows you to monitor memory and CPU usage by application.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.ActivityMonitor*) |
| classname | string | Class name: *org.rdk.ActivityMonitor* |
| locator | string | Library name: *libWPEFrameworkActivityMonitor.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.ActivityMonitor plugin:

ActivityMonitor interface methods:

| Method | Description |
| :-------- | :-------- |
| [enableMonitoring](#method.enableMonitoring) | Enables monitoring for the given application PIDs using the given thresholds for memory and CPU usage at frequencies specified by the intervals |
| [disableMonitoring](#method.disableMonitoring) | Disables monitoring for all applications |
| [getApplicationMemoryUsage](#method.getApplicationMemoryUsage) | Returns memory usage for a specific monitor-enabled application |
| [getAllMemoryUsage](#method.getAllMemoryUsage) | Returns memory usage for all monitoring-enabled applications |


<a name="method.enableMonitoring"></a>
## *enableMonitoring [<sup>method</sup>](#head.Methods)*

Enables monitoring for the given application PIDs using the given thresholds for memory and CPU usage at frequencies specified by the intervals.
 
### Events 
| Event | Description | 
| :----------- | :----------- | 
| `onMemoryThresholdOccured` | Triggered when an application exceeds the given memory threshold | 
 | `onCPUThresholdOccured` | Triggered when an application exceeds the `cpuThresholdPercent` value for a duration longer than the `cpuThresholdSeconds` value |.

Also see: [onCPUThresholdOccurred](#event.onCPUThresholdOccurred), [onMemoryThresholdOccurred](#event.onMemoryThresholdOccurred)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.config | array | An array of configuration objects for each application to be monitored |
| params.config[#] | object |  |
| params.config[#].appPid | integer | The application process identifier |
| params.config[#].memoryThresholdMB | integer | The maximum memory usage, in Megabytes, allowed before triggering an `onMemoryThresholdOccurred` event |
| params.config[#].cpuThresholdPercent | integer | The maximum CPU usage percent allowed before triggering an `onCPUThresholdOccurred` event |
| params.config[#].cpuThresholdSeconds | integer | The maximum duration, in seconds, that the CPU usage percent must be exceeded before triggering an `onCPUThresholdOccurred` event |
| params.memoryIntervalSeconds | string | The memory check interval in seconds |
| params.cpuIntervalSeconds | string | The CPU check interval in seconds |

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
    "method": "org.rdk.ActivityMonitor.1.enableMonitoring",
    "params": {
        "config": [
            {
                "appPid": 6763,
                "memoryThresholdMB": 10,
                "cpuThresholdPercent": 50,
                "cpuThresholdSeconds": 2
            }
        ],
        "memoryIntervalSeconds": "0.02",
        "cpuIntervalSeconds": "0.02"
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

<a name="method.disableMonitoring"></a>
## *disableMonitoring [<sup>method</sup>](#head.Methods)*

Disables monitoring for all applications. Monitoring stops immediately even if the full collection interval has not been reached.
 
### Events 
 
No events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty parameter object |

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
    "method": "org.rdk.ActivityMonitor.1.disableMonitoring",
    "params": {}
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

<a name="method.getApplicationMemoryUsage"></a>
## *getApplicationMemoryUsage [<sup>method</sup>](#head.Methods)*

Returns memory usage for a specific monitor-enabled application.
 
### Events 
 
No events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.pid | integer | The application process identifier |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.applicationMemory | object | The amount of memory used by this application |
| result.applicationMemory.appPid | integer | The application process identifier |
| result.applicationMemory.appName | string | The application name associated with `appPid` |
| result.applicationMemory.memoryMB | integer | The total memory used by an application in Megabytes |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ActivityMonitor.1.getApplicationMemoryUsage",
    "params": {
        "pid": 6763
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "applicationMemory": {
            "appPid": 6763,
            "appName": "TTSEngine",
            "memoryMB": 6
        },
        "success": true
    }
}
```

<a name="method.getAllMemoryUsage"></a>
## *getAllMemoryUsage [<sup>method</sup>](#head.Methods)*

Returns memory usage for all monitoring-enabled applications.
 
### Events 
 
No events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | An empty param object |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.freeMemoryMB | integer | The amount of free memory available |
| result.applicationMemory | array | An array of monitoring-enabled applications and their respective memory usage |
| result.applicationMemory[#] | object |  |
| result.applicationMemory[#].appPid | integer | The application process identifier |
| result.applicationMemory[#].appName | string | The application name associated with `appPid` |
| result.applicationMemory[#].memoryMB | integer | The total memory used by an application in Megabytes |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.ActivityMonitor.1.getAllMemoryUsage",
    "params": {}
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "freeMemoryMB": 100,
        "applicationMemory": [
            {
                "appPid": 6763,
                "appName": "TTSEngine",
                "memoryMB": 6
            }
        ],
        "success": true
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.ActivityMonitor plugin:

ActivityMonitor interface events:

| Event | Description |
| :-------- | :-------- |
| [onMemoryThresholdOccurred](#event.onMemoryThresholdOccurred) | Triggered when an application exceeds the given memory threshold |
| [onCPUThresholdOccurred](#event.onCPUThresholdOccurred) | Triggered when an application exceeds the `cpuThresholdPercent` value for a duration longer than the `cpuThresholdSeconds` value |


<a name="event.onMemoryThresholdOccurred"></a>
## *onMemoryThresholdOccurred [<sup>event</sup>](#head.Notifications)*

Triggered when an application exceeds the given memory threshold.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.appPid | integer | The application process identifier |
| params.threshold | string | Reports whether memory consumption has increased or decreased past the threshold value (must be one of the following: *exceeded*, *receded*) |
| params.memoryMB | integer | The total memory used by an application in Megabytes |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onMemoryThresholdOccurred",
    "params": {
        "appPid": 6763,
        "threshold": "exceeded",
        "memoryMB": 6
    }
}
```

<a name="event.onCPUThresholdOccurred"></a>
## *onCPUThresholdOccurred [<sup>event</sup>](#head.Notifications)*

Triggered when an application exceeds the `cpuThresholdPercent` value for a duration longer than the `cpuThresholdSeconds` value.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.appPid | integer | The application process identifier |
| params.threshold | string | Reports whether memory consumption has increased or decreased past the threshold value (must be one of the following: *exceeded*, *receded*) |
| params.cpuPercent | integer | The application CPU usage as a percent of the total CPU usage |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onCPUThresholdOccurred",
    "params": {
        "appPid": 6763,
        "threshold": "exceeded",
        "cpuPercent": 10.0
    }
}
```

