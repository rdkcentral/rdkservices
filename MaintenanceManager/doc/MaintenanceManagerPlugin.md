<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.MaintenanceManagerPlugin"></a>
# MaintenanceManagerPlugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.MaintenanceManager plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.MaintenanceManager plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `MaintenanceManager` plugin allows you to control and monitor maintenance activities on set-top devices.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.MaintenanceManager*) |
| classname | string | Class name: *org.rdk.MaintenanceManager* |
| locator | string | Library name: *libWPEFrameworkMaintenanceManager.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.MaintenanceManager plugin:

MaintenanceManager interface methods:

| Method | Description |
| :-------- | :-------- |
| [getMaintenanceActivityStatus](#method.getMaintenanceActivityStatus) | Gets the maintenance activity status details |
| [getMaintenanceStartTime](#method.getMaintenanceStartTime) | Gets the scheduled maintenance start time |
| [setMaintenanceMode](#method.setMaintenanceMode) | Sets the maintenance mode |
| [startMaintenance](#method.startMaintenance) | Starts maintenance activities |


<a name="method.getMaintenanceActivityStatus"></a>
## *getMaintenanceActivityStatus <sup>method</sup>*

Gets the maintenance activity status details.  
**Maintenance Statuses**  
* `MAINTENANCE_IDLE` - Sent when the Maintenance service is not executing any activities and until the first maintenance task is started  
* `MAINTENANCE_STARTED` - Sent immediately on maintenance startup either scheduled or on boot  
* `MAINTENANCE_ERROR` - Sent after receiving error notification while executing any of the maintenance activities  
* `MAINTENANCE_COMPLETE` - Sent after receiving `*_COMPLETE` notification from all critical maintenance tasks  
* `MAINTENANCE_INCOMPLETE` - Sent whenever the Maintenance service doesn't execute one or more of the tasks. `MAINTENANCE_ERROR` is returned even if only one task returns error.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.maintenanceStatus | string | The current maintenance status |
| result.lastSuccessfulCompletionTime | integer | The time the last maintenance completed or `0` if not applicable |
| result.isCriticalMaintenance | boolean | `true` if the maintenance activity cannot be aborted, otherwise `false` |
| result.isRebootPending | boolean | `true` if the device is going to reboot, otherwise `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.MaintenanceManager.1.getMaintenanceActivityStatus"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "maintenanceStatus": "MAINTENANCE_STARTED",
        "lastSuccessfulCompletionTime": 12345678,
        "isCriticalMaintenance": true,
        "isRebootPending": false,
        "success": true
    }
}
```

<a name="method.getMaintenanceStartTime"></a>
## *getMaintenanceStartTime <sup>method</sup>*

Gets the scheduled maintenance start time.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.maintenanceStartTime | integer | The start time |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.MaintenanceManager.1.getMaintenanceStartTime"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "maintenanceStartTime": 12345678,
        "success": true
    }
}
```

<a name="method.setMaintenanceMode"></a>
## *setMaintenanceMode <sup>method</sup>*

Sets the maintenance mode.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.maintenanceMode | string | The maintenance mode. The `FOREGROUND` mode runs all maintenance tasks. The `BACKGROUND` mode runs maintenance tasks that do not impact the user experience. (must be one of the following: *FOREGROUND*, *BACKGROUND*) |

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
    "id": 1234567890,
    "method": "org.rdk.MaintenanceManager.1.setMaintenanceMode",
    "params": {
        "maintenanceMode": "BACKGROUND"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.startMaintenance"></a>
## *startMaintenance <sup>method</sup>*

Starts maintenance activities.

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
    "id": 1234567890,
    "method": "org.rdk.MaintenanceManager.1.startMaintenance"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.MaintenanceManager plugin:

MaintenanceManager interface events:

| Event | Description |
| :-------- | :-------- |
| [onMaintenanceStatusChange](#event.onMaintenanceStatusChange) | Triggered when the maintenance manager status changes |


<a name="event.onMaintenanceStatusChange"></a>
## *onMaintenanceStatusChange <sup>event</sup>*

Triggered when the maintenance manager status changes. See `getMaintenanceActivityStatus` for a list of statuses.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.maintenanceStatus | string | The current maintenance status |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onMaintenanceStatusChange",
    "params": {
        "maintenanceStatus": "MAINTENANCE_STARTED"
    }
}
```

