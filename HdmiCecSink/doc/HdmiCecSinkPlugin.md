<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.HdmiCecSinkPlugin"></a>
# HdmiCecSinkPlugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.HdmiCecSink plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.HdmiCecSink plugin. It includes detailed specification about its configuration and methods provided.

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

The `HdmiCecSink` plugin allows you to manage HDMI Consumer Electronics Control (CEC) sink for connected devices.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.HdmiCecSink*) |
| classname | string | Class name: *org.rdk.HdmiCecSink* |
| locator | string | Library name: *libWPEFrameworkHdmiCecSink.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.HdmiCecSink plugin:

HdmiCecSink interface methods:

| Method | Description |
| :-------- | :-------- |
| [getActiveRoute](#method.getActiveRoute) | Gets details for the current route from the source to sink devices |
| [getActiveSource](#method.getActiveSource) | Gets details for the current active source |
| [getDeviceList](#method.getDeviceList) | Gets the number of connected source devices and system information for each device |
| [getEnabled](#method.getEnabled) | Check whether HDMI-CEC sink is enabled |
| [getVendorId](#method.getVendorId) | Gets the current vendor ID of this sink device |
| [requestActiveSource](#method.requestActiveSource) | Requests for the active source in the network |
| [setActivePath](#method.setActivePath) | Sets the source device to active (`setStreamPath`) |
| [setActiveSource](#method.setActiveSource) | Sets the current active source as TV (physical address 0 |
| [setEnabled](#method.setEnabled) | Enables or disables HDMI-CEC sink |
| [setRoutingChange](#method.setRoutingChange) | Changes routing while switching between HDMI inputs and TV |
| [setVendorId](#method.setVendorId) | Sets a vendor ID for the sink device |


<a name="method.getActiveRoute"></a>
## *getActiveRoute <sup>method</sup>*

Gets details for the current route from the source to sink devices. This API is used for debugging the route.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.available | boolean | If `true`, then there is an active source available and source details are included in the result. If `false`, then there is no active source |
| result.length | integer | The number of devices in the path list |
| result.pathList | array | Object [] of information about each device in the active path |
| result.pathList[#] | object |  |
| result.pathList[#].logicalAddress | integer | Logical address of the device |
| result.pathList[#].physicalAddress | string | Physical address of the device |
| result.pathList[#].deviceType | string | Type of the device |
| result.pathList[#].osdName | string | OSD name of the device if available |
| result.pathList[#].vendorID | string | Vendor ID of the device |
| result.ActiveRoute | string | Gives the route from source to sink with the device type and OSD name as an identifier |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.HdmiCecSink.1.getActiveRoute"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "available": true,
        "length": 1,
        "pathList": [
            {
                "logicalAddress": 4,
                "physicalAddress": "1.0.0.0",
                "deviceType": "Playback Device",
                "osdName": "Fire TV Stick",
                "vendorID": "0ce7"
            }
        ],
        "ActiveRoute": "Playback Device 1(Fire TV Stick)-->HDMI0",
        "success": true
    }
}
```

<a name="method.getActiveSource"></a>
## *getActiveSource <sup>method</sup>*

Gets details for the current active source.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.available | boolean | If `true`, then there is an active source available and source details are included in the result. If `false`, then there is no active source |
| result.logicalAddress | integer | Logical address of the device |
| result.physicalAddress | string | Physical address of the device |
| result.deviceType | string | Type of the device |
| result.cecVersion | string | CEC version supported |
| result.osdName | string | OSD name of the device if available |
| result.vendorID | string | Vendor ID of the device |
| result.powerStatus | string | Power status of the device |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.HdmiCecSink.1.getActiveSource"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "available": true,
        "logicalAddress": 4,
        "physicalAddress": "1.0.0.0",
        "deviceType": "Playback Device",
        "cecVersion": "Version 1.4",
        "osdName": "Fire TV Stick",
        "vendorID": "0ce7",
        "powerStatus": "Standby",
        "success": true
    }
}
```

<a name="method.getDeviceList"></a>
## *getDeviceList <sup>method</sup>*

Gets the number of connected source devices and system information for each device. The information includes device type, physical address, CEC version, vendor ID, power status and OSD name.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.numberofdevices | integer | number of devices in the `devicelist` array |
| result.devicelist | array | Object [] of information about each device |
| result.devicelist[#] | object |  |
| result.devicelist[#].logicalAddress | integer | Logical address of the device |
| result.devicelist[#].physicalAddress | string | Physical address of the device |
| result.devicelist[#].deviceType | string | Type of the device |
| result.devicelist[#].cecVersion | string | CEC version supported |
| result.devicelist[#].osdName | string | OSD name of the device if available |
| result.devicelist[#].vendorID | string | Vendor ID of the device |
| result.devicelist[#].powerStatus | string | Power status of the device |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.HdmiCecSink.1.getDeviceList"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "numberofdevices": 1,
        "devicelist": [
            {
                "logicalAddress": 4,
                "physicalAddress": "1.0.0.0",
                "deviceType": "Playback Device",
                "cecVersion": "Version 1.4",
                "osdName": "Fire TV Stick",
                "vendorID": "0ce7",
                "powerStatus": "Standby"
            }
        ],
        "success": true
    }
}
```

<a name="method.getEnabled"></a>
## *getEnabled <sup>method</sup>*

Check whether HDMI-CEC sink is enabled.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enabled | boolean | Indicates whether HDMI-CEC sink is enabled (`true`) or disabled (`false`) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.HdmiCecSink.1.getEnabled"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "enabled": false,
        "success": true
    }
}
```

<a name="method.getVendorId"></a>
## *getVendorId <sup>method</sup>*

Gets the current vendor ID of this sink device.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.vendorid | string | Vendor ID for this device. The ID can have a maximum of 6 characters |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.HdmiCecSink.1.getVendorId"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "vendorid": "0019fc",
        "success": true
    }
}
```

<a name="method.requestActiveSource"></a>
## *requestActiveSource <sup>method</sup>*

Requests for the active source in the network.

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
    "id": 1234567890,
    "method": "org.rdk.HdmiCecSink.1.requestActiveSource"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.setActivePath"></a>
## *setActivePath <sup>method</sup>*

Sets the source device to active (`setStreamPath`). The source wakes from standby if it's in the standby state.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.activePath | string | Physical address of the source device |

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
    "id": 1234567890,
    "method": "org.rdk.HdmiCecSink.1.setActivePath",
    "params": {
        "activePath": "1.0.0.0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.setActiveSource"></a>
## *setActiveSource <sup>method</sup>*

Sets the current active source as TV (physical address 0.0.0.0). This call needs to be made when the TV switches to internal tuner or any apps.

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
    "id": 1234567890,
    "method": "org.rdk.HdmiCecSink.1.setActiveSource"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.setEnabled"></a>
## *setEnabled <sup>method</sup>*

Enables or disables HDMI-CEC sink.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Indicates whether HDMI-CEC sink is enabled (`true`) or disabled (`false`) |

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
    "id": 1234567890,
    "method": "org.rdk.HdmiCecSink.1.setEnabled",
    "params": {
        "enabled": false
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.setRoutingChange"></a>
## *setRoutingChange <sup>method</sup>*

Changes routing while switching between HDMI inputs and TV.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.oldPort | string | Current active port, such as `TV`, `HDMI0`, `HDMI1`, `HDMI2` |
| params.newPort | string | New port to switch to, such as `TV`, `HDMI0`, `HDMI1`, `HDMI2` |

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
    "id": 1234567890,
    "method": "org.rdk.HdmiCecSink.1.setRoutingChange",
    "params": {
        "oldPort": "HDMI0",
        "newPort": "TV"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.setVendorId"></a>
## *setVendorId <sup>method</sup>*

Sets a vendor ID for the sink device.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.vendorid | string | Vendor ID for this device. The ID can have a maximum of 6 characters |

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
    "id": 1234567890,
    "method": "org.rdk.HdmiCecSink.1.setVendorId",
    "params": {
        "vendorid": "0019fc"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

