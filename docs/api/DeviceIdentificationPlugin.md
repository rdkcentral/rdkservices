<!-- Generated automatically, DO NOT EDIT! -->
<a name="DeviceIdentification_Plugin"></a>
# DeviceIdentification Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A DeviceIdentification plugin for Thunder framework.

### Table of Contents

- [Introduction](#Introduction)
- [Description](#Description)
- [Configuration](#Configuration)
- [Properties](#Properties)

<a name="Introduction"></a>
# Introduction

<a name="Scope"></a>
## Scope

This document describes purpose and functionality of the DeviceIdentification plugin. It includes detailed specification about its configuration and properties provided.

<a name="Case_Sensitivity"></a>
## Case Sensitivity

All identifiers of the interfaces described in this document are case-sensitive. Thus, unless stated otherwise, all keywords, entities, properties, relations and actions should be treated as such.

<a name="Acronyms,_Abbreviations_and_Terms"></a>
## Acronyms, Abbreviations and Terms

The table below provides and overview of acronyms used in this document and their definitions.

| Acronym | Description |
| :-------- | :-------- |
| <a name="API">API</a> | Application Programming Interface |
| <a name="HTTP">HTTP</a> | Hypertext Transfer Protocol |
| <a name="JSON">JSON</a> | JavaScript Object Notation; a data interchange format |
| <a name="JSON-RPC">JSON-RPC</a> | A remote procedure call protocol encoded in JSON |

The table below provides and overview of terms and abbreviations used in this document and their definitions.

| Term | Description |
| :-------- | :-------- |
| <a name="callsign">callsign</a> | The name given to an instance of a plugin. One plugin can be instantiated multiple times, but each instance the instance name, callsign, must be unique. |

<a name="References"></a>
## References

| Ref ID | Description |
| :-------- | :-------- |
| <a name="HTTP">[HTTP](http://www.w3.org/Protocols)</a> | HTTP specification |
| <a name="JSON-RPC">[JSON-RPC](https://www.jsonrpc.org/specification)</a> | JSON-RPC 2.0 specification |
| <a name="JSON">[JSON](http://www.json.org/)</a> | JSON specification |
| <a name="Thunder">[Thunder](https://github.com/WebPlatformForEmbedded/Thunder/blob/master/doc/WPE%20-%20API%20-%20WPEFramework.docx)</a> | Thunder API Reference |

<a name="Description"></a>
# Description

The `DeviceIdentification` plugin allows you to retrieve various device-related information.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *DeviceIdentification*) |
| classname | string | Class name: *DeviceIdentification* |
| locator | string | Library name: *libWPEFrameworkDeviceIdentification.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Properties"></a>
# Properties

The following properties are provided by the DeviceIdentification plugin:

DeviceIdentification interface properties:

| Property | Description |
| :-------- | :-------- |
| [deviceidentification](#deviceidentification) <sup>RO</sup> | Device platform specific information |


<a name="deviceidentification"></a>
## *deviceidentification*

Provides access to the device platform specific information.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Device platform specific information |
| (property).firmwareversion | string | Version of the device firmware |
| (property).chipset | string | Chipset used for this device |
| (property)?.identifier | string | <sup>*(optional)*</sup> Device unique identifier |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceIdentification.1.deviceidentification"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "firmwareversion": "1.0.0",
        "chipset": "BCM2711",
        "identifier": "WPEuCfrLF45"
    }
}
```

