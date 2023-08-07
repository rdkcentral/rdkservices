<!-- Generated automatically, DO NOT EDIT! -->
<a name="MaintenanceManagerPlugin"></a>
# MaintenanceManagerPlugin

**Version: [1.0.21](https://github.com/rdkcentral/rdkservices/blob/main/MaintenanceManager/CHANGELOG.md)**

A org.rdk.MaintenanceManager plugin for Thunder framework.

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

The `MaintenanceManager` plugin allows you to control and monitor maintenance activities on set-top devices.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.MaintenanceManager*) |
| classname | string | Class name: *org.rdk.MaintenanceManager* |
| locator | string | Library name: *libWPEFrameworkMaintenanceManager.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.MaintenanceManager plugin:

MaintenanceManager interface methods:

| Method | Description |
| :-------- | :-------- |
| [getMaintenanceActivityStatus](#getMaintenanceActivityStatus) | Gets the maintenance activity status details |
| [getMaintenanceStartTime](#getMaintenanceStartTime) | Gets the scheduled maintenance start time |
| [setMaintenanceMode](#setMaintenanceMode) | Sets the maintenance mode and software upgrade opt-out mode |
| [startMaintenance](#startMaintenance) | Starts maintenance activities |
| [stopMaintenance](#stopMaintenance) | Stops maintenance activities |
| [getMaintenanceMode](#getMaintenanceMode) | Gets the current maintenance mode and software upgrade opt-out mode which are stored in the persistent location |


<a name="getMaintenanceActivityStatus"></a>
## *getMaintenanceActivityStatus*

Gets the maintenance activity status details.  
**Maintenance Status**  
* `MAINTENANCE_IDLE` - Maintenance service is not executing any activities before the start of first maintenance task  
* `MAINTENANCE_STARTED` - Maintenance has started either by schedule or on boot  
* `MAINTENANCE_ERROR` - One or more tasks of the maintenance service has failed  
* `MAINTENANCE_COMPLETE` - All critical maintenance tasks are completed successfully  
* `MAINTENANCE_INCOMPLETE` - Maintenance service didn't execute one or more of the tasks.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.maintenanceStatus | string | The current maintenance status |
| result.LastSuccessfulCompletionTime | integer | The time (in epoch time) the last maintenance completed or `0` if not applicable |
| result.isCriticalMaintenance | boolean | `true` if the maintenance activity cannot be aborted, otherwise `false` |
| result.isRebootPending | boolean | `true` if the device is going to reboot, otherwise `false` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.MaintenanceManager.getMaintenanceActivityStatus"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "maintenanceStatus": "MAINTENANCE_STARTED",
        "LastSuccessfulCompletionTime": 12345678,
        "isCriticalMaintenance": true,
        "isRebootPending": false,
        "success": true
    }
}
```

<a name="getMaintenanceStartTime"></a>
## *getMaintenanceStartTime*

Gets the scheduled maintenance start time.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.maintenanceStartTime | integer | The start time (in epoch time) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.MaintenanceManager.getMaintenanceStartTime"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "maintenanceStartTime": 12345678,
        "success": true
    }
}
```

<a name="setMaintenanceMode"></a>
## *setMaintenanceMode*

Sets the maintenance mode and software upgrade opt-out mode.  
*Opt-Out Modes*  
* `NONE` - The software upgrade process is unaffected and proceeds with the download and update.  
* `ENFORCE_OPTPOUT` - The software upgrade process pauses after discovering an update is available and sends a `System` service `onFirmwareUpdateStateChange` event with the `On Hold for opt-out` state. An application must give the user the option of whether or not to accept the update. If the user accepts the update, then the opt-out mode must be set to `BYPASS-OPTOUT`.  
* `BYPASS_OPTOUT` The software upgrade process proceeds with a download and update, as directed by the application, for this occurrence of the maintenance window (used when the user accepts the software update).  
* `IGNORE-UPDATE` -  The software upgrade process ignores any non-mandatory firmware updates, and will NOT send any notification. Note that in this mode, the software upgrade process still sets `ENFORCE-OPTOUT` if the update is mandatory. Use the `getFirmwareUpdateInfo` method from the `System` service to determine what software version is available for download and to determine if the update is consider mandatory (using the `rebootImmediately` parameter).

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.maintenanceMode | string | The maintenance mode. The `FOREGROUND` mode runs all maintenance tasks. The `BACKGROUND` mode aborts activities currently running, if the task can't run in the background (if maintenance was already started with FOREGROUND mode) and executes tasks without impacting the user experience, if it is set before calling startMaintenance. (must be one of the following: *FOREGROUND*, *BACKGROUND*) |
| params.optOut | string | The opt-out mode.  (must be one of the following: *NONE*, *ENFORCE_OPTOUT*, *BYPASS_OPTOUT*, *IGNORE_UPDATE*) |

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
    "method": "org.rdk.MaintenanceManager.setMaintenanceMode",
    "params": {
        "maintenanceMode": "BACKGROUND",
        "optOut": "ENFORCE_OPTOUT"
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

<a name="startMaintenance"></a>
## *startMaintenance*

Starts maintenance activities.

### Events

| Event | Description |
| :-------- | :-------- |
| [onMaintenanceStatusChange](#onMaintenanceStatusChange) | Triggers whenever the maintenance status changes |
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
    "method": "org.rdk.MaintenanceManager.startMaintenance"
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

<a name="stopMaintenance"></a>
## *stopMaintenance*

Stops maintenance activities.

### Events

| Event | Description |
| :-------- | :-------- |
| [onMaintenanceStatusChange](#onMaintenanceStatusChange) | Triggers whenever the maintenance status changes |
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
    "method": "org.rdk.MaintenanceManager.stopMaintenance"
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

<a name="getMaintenanceMode"></a>
## *getMaintenanceMode*

Gets the current maintenance mode and software upgrade opt-out mode which are stored in the persistent location.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.maintenanceMode | string | The maintenance mode. The `FOREGROUND` mode runs all maintenance tasks. The `BACKGROUND` mode aborts activities currently running, if the task can't run in the background (if maintenance was already started with FOREGROUND mode) and executes tasks without impacting the user experience, if it is set before calling startMaintenance. (must be one of the following: *FOREGROUND*, *BACKGROUND*) |
| result.optOut | string | The opt-out mode. See [setMaintenanceMode](#setMaintenanceMode) for a description of each opt-out mode (must be one of the following: *NONE*, *ENFORCE_OPTOUT*, *BYPASS_OPTOUT*, *IGNORE_UPDATE*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.MaintenanceManager.getMaintenanceMode"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "maintenanceMode": "BACKGROUND",
        "optOut": "ENFORCE_OPTOUT",
        "success": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.MaintenanceManager plugin:

MaintenanceManager interface events:

| Event | Description |
| :-------- | :-------- |
| [onMaintenanceStatusChange](#onMaintenanceStatusChange) | Triggered when the maintenance manager status changes |


<a name="onMaintenanceStatusChange"></a>
## *onMaintenanceStatusChange*

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
    "method": "client.events.onMaintenanceStatusChange",
    "params": {
        "maintenanceStatus": "MAINTENANCE_STARTED"
    }
}
```

