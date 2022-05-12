<!-- Generated automatically, DO NOT EDIT! -->
<a name="OpenCDMi_Plugin"></a>
# OpenCDMi Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A OCDM plugin for Thunder framework.

### Table of Contents

- [Introduction](#Introduction)
- [Description](#Description)
- [Configuration](#Configuration)
- [Properties](#Properties)

<a name="Introduction"></a>
# Introduction

<a name="Scope"></a>
## Scope

This document describes purpose and functionality of the OCDM plugin. It includes detailed specification about its configuration and properties provided.

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

The `OpenCDMi` plugin allows you view Open Content Decryption Module (OCDM) properties.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *OCDM*) |
| classname | string | Class name: *OCDM* |
| locator | string | Library name: *libWPEFrameworkOCDM.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |
| configuration | object | <sup>*(optional)*</sup>  |
| configuration?.location | string | <sup>*(optional)*</sup> The location |
| configuration?.connector | string | <sup>*(optional)*</sup> The connector |
| configuration?.sharepath | string | <sup>*(optional)*</sup> The sharepath |
| configuration?.sharesize | string | <sup>*(optional)*</sup> The sharesize |
| configuration?.systems | array | <sup>*(optional)*</sup> A list of key systems |
| configuration?.systems[#] | object | <sup>*(optional)*</sup> System properties |
| configuration?.systems[#]?.name | string | <sup>*(optional)*</sup> Property name |
| configuration?.systems[#]?.designators | array | <sup>*(optional)*</sup> designator |
| configuration?.systems[#]?.designators[#] | object | <sup>*(optional)*</sup> System properties |
| configuration?.systems[#]?.designators[#].name | string | Property name |

<a name="Properties"></a>
# Properties

The following properties are provided by the OCDM plugin:

OCDM interface properties:

| Property | Description |
| :-------- | :-------- |
| [drms](#drms) <sup>RO</sup> | Supported DRM systems |
| [keysystems](#keysystems) <sup>RO</sup> | DRM key systems |


<a name="drms"></a>
## *drms [<sup>property</sup>](#Properties)*

Provides access to the supported DRM systems.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | Supported DRM systems |
| (property)[#] | object |  |
| (property)[#].name | string | The name of the DRM system |
| (property)[#].keysystems | array |  |
| (property)[#].keysystems[#] | string | An identifier of a key system |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "OCDM.1.drms"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        {
            "name": "PlayReady",
            "keysystems": [
                "com.microsoft.playready"
            ]
        }
    ]
}
```

<a name="keysystems"></a>
## *keysystems [<sup>property</sup>](#Properties)*

Provides access to the DRM key systems.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | DRM key systems |
| (property)[#] | string | An identifier of a key system |

> The *drm system* argument shall be passed as the index to the property, e.g. *OCDM.1.keysystems@PlayReady*.

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 30 | ```ERROR_BAD_REQUEST``` | Invalid DRM name |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "OCDM.1.keysystems@PlayReady"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        "com.microsoft.playready"
    ]
}
```

