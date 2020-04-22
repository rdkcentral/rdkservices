<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.Device_Identification_API"></a>
# Device Identification API

**Version: 1.0**

**Status: :black_circle::white_circle::white_circle:**

DeviceIdentification plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Properties](#head.Properties)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the DeviceIdentification plugin. It includes detailed specification of its configuration and properties provided.

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

DeviceIdentification JSON-RPC interface

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| classname | string | Class name: *DeviceIdentification* |
| autostart | boolean | Determines if the plugin is to be started automatically along with the framework |

<a name="head.Properties"></a>
# Properties

The following properties are provided by the DeviceIdentification plugin:

DeviceIdentification interface properties:

| Property | Description |
| :-------- | :-------- |
| [deviceidentification](#property.deviceidentification) <sup>RO</sup> | Device paltform specific information |

<a name="property.deviceidentification"></a>
## *deviceidentification <sup>property</sup>*

Provides access to the device paltform specific information.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Device paltform specific information |
| (property).firmwareversion | string | Version of the device firmware |
| (property).chipset | string | Chipset used for this device |
| (property)?.identifier | string | <sup>*(optional)*</sup> Device unique identifier |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DeviceIdentification.1.deviceidentification"
}
```
#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "firmwareversion": "1.0.0",
        "chipset": "BCM2711",
        "identifier": ""
    }
}
```
