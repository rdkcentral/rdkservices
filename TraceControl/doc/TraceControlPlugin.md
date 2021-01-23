<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.Trace_Control_Plugin"></a>
# Trace Control Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

TraceControl plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the TraceControl plugin. It includes detailed specification about its configuration and methods provided.

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

The Trace Control plugin provides ability to disable/enable trace output an set its verbosity level.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
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
| configuration?.remotes?.bindig | bindig | <sup>*(optional)*</sup> Binding |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the TraceControl plugin:

TraceControl interface methods:

| Method | Description |
| :-------- | :-------- |
| [status](#method.status) | Retrieves general information |
| [set](#method.set) | Sets traces |


<a name="method.status"></a>
## *status <sup>method</sup>*

Retrieves general information.

### Description

Retrieves the actual trace status information for targeted module and category, if either category nor module is given, all information is returned. It will retrieves the details about console status and remote address(port and binding address), if these are configured.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.module | string | Module name |
| params.category | string | Category name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.console | boolean | Config attribute (Console) |
| result.remote | object |  |
| result.remote.port | number | Config attribute (port) |
| result.remote.binding | string | Config attribute (binding) |
| result.settings | array |  |
| result.settings[#] | object |  |
| result.settings[#].module | string | Module name |
| result.settings[#].category | string | Category name |
| result.settings[#].state | string | State value (must be one of the following: *enabled*, *disabled*, *tristated*) |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "TraceControl.1.status",
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
    "id": 1234567890,
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

<a name="method.set"></a>
## *set <sup>method</sup>*

Sets traces.

### Description

Disables/enables all/select category traces for particular module.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.module | string | Module name |
| params.category | string | Category name |
| params.state | string | State value (must be one of the following: *enabled*, *disabled*, *tristated*) |

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
    "method": "TraceControl.1.set",
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
    "id": 1234567890,
    "result": null
}
```

