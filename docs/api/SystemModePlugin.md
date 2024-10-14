<!-- Generated automatically, DO NOT EDIT! -->
<a name="System_Mode_Plugin"></a>
# System Mode Plugin

**Version: [1.0.0](https://github.com/rdkcentral/rdkservices/blob/main/SystemMode/CHANGELOG.md)**

A org.rdk.SystemMode plugin for Thunder framework.

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

The `SystemMode` plugin coordinates state changes that take effect across multiple components in the system.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.SystemMode*) |
| classname | string | Class name: *org.rdk.SystemMode* |
| locator | string | Library name: *libWPEFrameworkSystemMode.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.SystemMode plugin:

org.rdk.SystemMode interface methods:

| Method | Description |
| :-------- | :-------- |
| [requestState](#requestState) | Requests a new system mode state in the device |
| [getState](#getState) | Gets the current state for a given system property |


<a name="requestState"></a>
## *requestState*

Requests a new system mode state in the device.  Thunder components asynchronously reconfigure themselves so the caller cannot be guaranteed a full state transition upon return.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.systemMode | string | System mode (must be one of the following: device_optimize) |
| params.state | string | The target state (must be one of the following: video, game) |

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
    "method": "org.rdk.SystemMode.requestState",
    "params": {
        "systemMode": "device_optimize",
        "state": "game"
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

<a name="getState"></a>
## *getState*

Gets the current state for a given system 

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.systemMode | string | System mode (must be one of the following: device_optimize) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.state | string | The target state (must be one of the following: video, game) |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.SystemMode.getState",
    "params": {
        "systemMode": "device_optimize"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "state": "game"
    }
}
```

