<!-- Generated automatically, DO NOT EDIT! -->
<a name="LEDControl_Plugin"></a>
# LEDControl Plugin

**Version: [1.0.2](https://github.com/rdkcentral/rdkservices/blob/main/LEDControl/CHANGELOG.md)**

A org.rdk.LEDControl plugin for Thunder framework.

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

The `LEDControl` plugin allows you to control brightness and power state for LEDs on a device.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.LEDControl*) |
| classname | string | Class name: *org.rdk.LEDControl* |
| locator | string | Library name: *libWPEFrameworkLEDControl.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.LEDControl plugin:

LEDControl interface methods:

| Method | Description |
| :-------- | :-------- |
| [getSupportedLEDStates](#getSupportedLEDStates) | Returns all the LED states supported by the platform |
| [getLEDState](#getLEDState) | Returns current LED state |
| [setLEDState](#setLEDState) | Change the device LED state to one mentioned in the input argument |


<a name="getSupportedLEDStates"></a>
## *getSupportedLEDStates*

Returns all the LED states supported by the platform.

### Events

No Events

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
    "method": "org.rdk.LEDControl.getSupportedLEDStates"
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

<a name="getLEDState"></a>
## *getLEDState*

Returns current LED state.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.state | string | Indicates a platform supported LED state |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.LEDControl.getLEDState"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "state": "ACTIVE",
        "success": true
    }
}
```

<a name="setLEDState"></a>
## *setLEDState*

Change the device LED state to one mentioned in the input argument.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.state | string | Indicates a platform supported LED state |

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
    "method": "org.rdk.LEDControl.setLEDState",
    "params": {
        "state": "ACTIVE"
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

