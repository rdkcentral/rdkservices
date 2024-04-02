<!-- Generated automatically, DO NOT EDIT! -->
<a name="TraceControl_Plugin"></a>
# TraceControl Plugin

**Version: [1.0.2](https://github.com/rdkcentral/rdkservices/blob/main/TraceControl/CHANGELOG.md)**

A TraceControl plugin for Thunder framework.

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

The `TraceControl` plugin provides ability to disable/enable trace output and set its verbosity level.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *TraceControl*) |
| classname | string | Class name: *TraceControl* |
| locator | string | Library name: *libWPEFrameworkTraceControl.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |
| configuration | object | <sup>*(optional)*</sup>  |
| configuration?.console | boolean | <sup>*(optional)*</sup> Enable console |
| configuration?.syslog | boolean | <sup>*(optional)*</sup> Enable SysLog |
| configuration?.abbreviated | boolean | <sup>*(optional)*</sup> Enable abbreviated logging |
| configuration?.remotes | object | <sup>*(optional)*</sup>  |
| configuration?.remotes?.port | number | <sup>*(optional)*</sup> Port |
| configuration?.remotes?.binding | string | <sup>*(optional)*</sup> Binding |

<a name="Methods"></a>
# Methods

The following methods are provided by the TraceControl plugin:

TraceControl interface methods:

| Method | Description |
| :-------- | :-------- |
| [set](#set) | Sets traces |
| [status](#status) | Retrieves the actual trace status information for the specified module and category |


<a name="set"></a>
## *set*

Sets traces. Enables or disables all or select category traces for the specified module.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Trace information |
| params.module | string | The module name. If the module name is not specified then, it returns all modules |
| params.category | string | The category name. If the category name is not specified then, it returns all categories |
| params.state | string | The state value (must be one of the following: *enabled*, *disabled*, *tristated*) |

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
    "method": "TraceControl.set",
    "params": {
        "module": "Plugin_Monitor",
        "category": "Information",
        "state": "disabled"
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

<a name="status"></a>
## *status*

Retrieves the actual trace status information for the specified module and category. If the category or module is not specified then, all the information is returned. If both module and category are not specified then, the result is empty. It retrieves the details about the console status and remote address (port and binding), if these are configured.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.module | string | The module name. If the module name is not specified then, it returns all modules |
| params.category | string | The category name. If the category name is not specified then, it returns all categories |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.console | boolean | `true` if a console, otherwise `false` |
| result.remote | object | The remote address |
| result.remote.port | number | The port number |
| result.remote.binding | string | The binding address |
| result.settings | array |  |
| result.settings[#] | object | Trace information |
| result.settings[#].module | string | The module name. If the module name is not specified then, it returns all modules |
| result.settings[#].category | string | The category name. If the category name is not specified then, it returns all categories |
| result.settings[#].state | string | The state value (must be one of the following: *enabled*, *disabled*, *tristated*) |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "TraceControl.status",
    "params": {
        "module": "Plugin_Monitor",
        "category": "Information"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "console": false,
        "remote": {
            "port": 2200,
            "binding": "0.0.0.0"
        },
        "settings": [
            {
                "module": "Plugin_Monitor",
                "category": "Information",
                "state": "disabled"
            }
        ]
    }
}
```

