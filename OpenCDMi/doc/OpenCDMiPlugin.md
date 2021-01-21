<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.OpenCDMi_Plugin"></a>
# OpenCDMi Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

OCDM plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Configuration](#head.Configuration)
- [Properties](#head.Properties)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the OCDM plugin. It includes detailed specification about its configuration and properties provided.

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

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *OCDM*) |
| classname | string | Class name: *OCDM* |
| locator | string | Library name: *libWPEFrameworkOCDM.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |
| configuration | object | <sup>*(optional)*</sup>  |
| configuration?.location | string | <sup>*(optional)*</sup> Location |
| configuration?.connector | string | <sup>*(optional)*</sup> Connector |
| configuration?.sharepath | string | <sup>*(optional)*</sup> Sharepath |
| configuration?.sharesize | string | <sup>*(optional)*</sup> Sharesize |
| configuration?.systems | array | <sup>*(optional)*</sup> List of key systems |
| configuration?.systems[#] | object | <sup>*(optional)*</sup> System properties |
| configuration?.systems[#]?.name | string | <sup>*(optional)*</sup> Name |
| configuration?.systems[#]?.designators | array | <sup>*(optional)*</sup>  |
| configuration?.systems[#]?.designators[#] | object | <sup>*(optional)*</sup> designator |
| configuration?.systems[#]?.designators[#].name | string |  |

<a name="head.Properties"></a>
# Properties

The following properties are provided by the OCDM plugin:

OCDM interface properties:

| Property | Description |
| :-------- | :-------- |
| [drms](#property.drms) <sup>RO</sup> | Supported DRM systems |
| [keysystems](#property.keysystems) <sup>RO</sup> | DRM key systems |


<a name="property.drms"></a>
## *drms <sup>property</sup>*

Provides access to the supported DRM systems.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | Supported DRM systems |
| (property)[#] | object |  |
| (property)[#].name | string | Name of the DRM |
| (property)[#].keysystems | array |  |
| (property)[#].keysystems[#] | string | Identifier of a key system |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "OCDM.1.drms"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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

<a name="property.keysystems"></a>
## *keysystems <sup>property</sup>*

Provides access to the DRM key systems.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | DRM key systems |
| (property)[#] | string | Identifier of a key system |

> The *drm system* shall be passed as the index to the property, e.g. *OCDM.1.keysystems@PlayReady*.

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 30 | ```ERROR_BAD_REQUEST``` | Invalid DRM name |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "OCDM.1.keysystems@PlayReady"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": [
        "com.microsoft.playready"
    ]
}
```

