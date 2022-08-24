<!-- Generated automatically, DO NOT EDIT! -->
<a name="ActivityMonitor_Plugin"></a>
# ActivityMonitor Plugin

**Version: 1.0.0**

A org.rdk.ActivityMonitor plugin for Thunder framework.

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

The `ActivityMonitor` plugin allows you to monitor memory and CPU usage by application.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.ActivityMonitor*) |
| classname | string | Class name: *org.rdk.ActivityMonitor* |
| locator | string | Library name: *libWPEFrameworkActivityMonitor.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.ActivityMonitor plugin:

ActivityMonitor interface methods:

| Method | Description |
| :-------- | :-------- |
| [enableMonitoring](#enableMonitoring) | Enables monitoring for the given application PIDs using the given thresholds for memory and CPU usage at frequencies specified by the intervals |
| [disableMonitoring](#disableMonitoring) | Disables monitoring for all applications |
| [getApplicationMemoryUsage](#getApplicationMemoryUsage) | Returns memory usage for a specific monitor-enabled application |
| [getAllMemoryUsage](#getAllMemoryUsage) | Returns memory usage for all monitoring-enabled applications |


<a name="enableMonitoring"></a>
## *enableMonitoring*

Enables monitoring for the given application PIDs using the given thresholds for memory and CPU usage at frequencies specified by the intervals.
 
### Events 
| Event | Description | 
| :----------- | :----------- | 
| `onMemoryThresholdOccured` | Triggered when an application exceeds the given memory threshold | 
 | `onCPUThresholdOccured` | Triggered when an application exceeds the `cpuThresholdPercent` value for a duration longer than the `cpuThresholdSeconds` value |.

Also see: [onCPUThresholdOccurred](#onCPUThresholdOccurred), [onMemoryThresholdOccurred](#onMemoryThresholdOccurred)

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

<a name="disableMonitoring"></a>
## *disableMonitoring*

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

<a name="getApplicationMemoryUsage"></a>
## *getApplicationMemoryUsage*

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

<a name="getAllMemoryUsage"></a>
## *getAllMemoryUsage*

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

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.ActivityMonitor plugin:

ActivityMonitor interface events:

| Event | Description |
| :-------- | :-------- |
| [onMemoryThresholdOccurred](#onMemoryThresholdOccurred) | Triggered when an application exceeds the given memory threshold |
| [onCPUThresholdOccurred](#onCPUThresholdOccurred) | Triggered when an application exceeds the `cpuThresholdPercent` value for a duration longer than the `cpuThresholdSeconds` value |


<a name="onMemoryThresholdOccurred"></a>
## *onMemoryThresholdOccurred*

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

<a name="onCPUThresholdOccurred"></a>
## *onCPUThresholdOccurred*

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

