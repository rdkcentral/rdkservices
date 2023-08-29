<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.MessageControl_Plugin"></a>
# MessageControl Plugin

**Version: 1.0**

**Status: :black_circle::white_circle::white_circle:**

MessageControl plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Interfaces](#head.Interfaces)
- [Methods](#head.Methods)
- [Properties](#head.Properties)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the MessageControl plugin. It includes detailed specification about its configuration, methods and properties provided.

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

The MessageControl plugin allows reading of the traces from WPEFramework, and controlling them tracing and logging. Allows for outputting logging messages to the websocket.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *MessageControl*) |
| classname | string | Class name: *MessageControl* |
| locator | string | Library name: *libWPEFrameworkMessageControl.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |
| console | boolean | <sup>*(optional)*</sup> Enables message output messages to the console |
| syslog | boolean | <sup>*(optional)*</sup> Enables message ouutput to syslog |
| filepath | string | <sup>*(optional)*</sup> Path to file (inside VolatilePath) where messages will be stored |
| abbreviated | boolean | <sup>*(optional)*</sup> Denotes if the messages should be abbreviated |
| maxexportconnections | number | <sup>*(optional)*</sup> Specifies to how many websockets can the messages be outputted |
| remote | object | <sup>*(optional)*</sup>  |
| remote.port | number | Port |
| remote?.bindig | string | <sup>*(optional)*</sup> Binding address |

<a name="head.Interfaces"></a>
# Interfaces

This plugin implements the following interfaces:

- Exchange::IMessageControl ([IMessageControl.h](https://github.com/rdkcentral/ThunderInterfaces/blob/master/interfaces/IMessageControl.h)) (version 1.0.0) (compliant format)

<a name="head.Methods"></a>
# Methods

The following methods are provided by the MessageControl plugin:

MessageControl interface methods:

| Method | Description |
| :-------- | :-------- |
| [enable](#method.enable) | Enables/disables a message control |


<a name="method.enable"></a>
## *enable [<sup>method</sup>](#head.Methods)*

Enables/disables a message control.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.type | string | Message type (must be one of the following: *Tracing*, *Logging*) |
| params.category | string | Name of the message category |
| params.module | string | Name of the module the message is originating from |
| params.enabled | boolean | Denotes if control should be enabled (true) or disabled (false) |

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
    "method": "MessageControl.1.enable",
    "params": {
        "type": "Tracing",
        "category": "Information",
        "module": "Plugin_BluetoothControl",
        "enabled": false
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

<a name="head.Properties"></a>
# Properties

The following properties are provided by the MessageControl plugin:

MessageControl interface properties:

| Property | Description |
| :-------- | :-------- |
| [controls](#property.controls) <sup>RO</sup> | Retrieves a list of current message controls |


<a name="property.controls"></a>
## *controls [<sup>property</sup>](#head.Properties)*

Provides access to the retrieves a list of current message controls.

> This property is **read-only**.

### Value

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | array | Retrieves a list of current message controls |
| result[#] | object |  |
| result[#].type | string | Type of message (must be one of the following: *Tracing*, *Logging*) |
| result[#].category | string | Name of the message category |
| result[#].module | string | Name of the module the message is originating from |
| result[#].enabled | boolean | Denotes if the control is enabled (true) or disabled (false) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "MessageControl.1.controls"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        {
            "type": "Tracing",
            "category": "Information",
            "module": "Plugin_BluetoothControl",
            "enabled": false
        }
    ]
}
```

