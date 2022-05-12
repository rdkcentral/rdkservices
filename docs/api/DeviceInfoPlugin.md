<!-- Generated automatically, DO NOT EDIT! -->
<a name="DeviceInfo_Plugin"></a>
# DeviceInfo Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A DeviceInfo plugin for Thunder framework.

### Table of Contents

- [Introduction](#Introduction)
- [Description](#Description)
- [Configuration](#Configuration)
- [Properties](#Properties)

<a name="Introduction"></a>
# Introduction

<a name="Scope"></a>
## Scope

This document describes purpose and functionality of the DeviceInfo plugin. It includes detailed specification about its configuration and properties provided.

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

The `DeviceInfo` plugin allows retrieving of various device-related information.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *DeviceInfo*) |
| classname | string | Class name: *DeviceInfo* |
| locator | string | Library name: *libWPEFrameworkDeviceInfo.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Properties"></a>
# Properties

The following properties are provided by the DeviceInfo plugin:

DeviceInfo interface properties:

| Property | Description |
| :-------- | :-------- |
| [systeminfo](#systeminfo) <sup>RO</sup> | Provides access to system general information |
| [addresses](#addresses) <sup>RO</sup> | Network interface addresses |
| [socketinfo](#socketinfo) <sup>RO</sup> | Socket information |


<a name="systeminfo"></a>
## *systeminfo [<sup>property</sup>](#Properties)*

Provides access to the provides access to system general information.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Provides access to system general information |
| (property).time | string | Current system date and time |
| (property).version | string | Software version (in form *version#hashtag*) |
| (property).uptime | number | System uptime (in seconds) |
| (property).freeram | number | Free system RAM memory (in bytes) |
| (property).totalram | number | Total installed system RAM memory (in bytes) |
| (property).devicename | string | Host name |
| (property).cpuload | string | Current CPU load (percentage) |
| (property).serialnumber | string | Device serial number |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.1.systeminfo"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "time": "Mon, 11 Mar 2019 14:38:18",
        "version": "1.0#14452f612c3747645d54974255d11b8f3b4faa54",
        "uptime": 120,
        "freeram": 563015680,
        "totalram": 655757312,
        "devicename": "buildroot",
        "cpuload": "2",
        "serialnumber": "WPEuCfrLF45"
    }
}
```

<a name="addresses"></a>
## *addresses [<sup>property</sup>](#Properties)*

Provides access to the network interface addresses.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | Network interface addresses |
| (property)[#] | object |  |
| (property)[#].name | string | Interface name |
| (property)[#].mac | string | Interface MAC address |
| (property)[#]?.ip | array | <sup>*(optional)*</sup>  |
| (property)[#]?.ip[#] | string | <sup>*(optional)*</sup> Interface IP address |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.1.addresses"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        {
            "name": "lo",
            "mac": "00:00:00:00:00",
            "ip": [
                "127.0.0.1"
            ]
        }
    ]
}
```

<a name="socketinfo"></a>
## *socketinfo [<sup>property</sup>](#Properties)*

Provides access to the socket information.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Socket information |
| (property)?.total | number | <sup>*(optional)*</sup> Total number of sockets |
| (property)?.open | number | <sup>*(optional)*</sup> Number of open sockets |
| (property)?.link | number | <sup>*(optional)*</sup> Number of links |
| (property)?.exception | number | <sup>*(optional)*</sup> Number of exceptions |
| (property)?.shutdown | number | <sup>*(optional)*</sup> Number of sockets that were shutdown |
| (property).runs | number | Number of runs |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.1.socketinfo"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "total": 0,
        "open": 0,
        "link": 0,
        "exception": 0,
        "shutdown": 0,
        "runs": 1
    }
}
```

