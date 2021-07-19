<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.HdmiCec_2Plugin"></a>
# HdmiCec_2Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.HdmiCec_2 plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.HdmiCec_2 plugin. It includes detailed specification about its configuration and methods provided.

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

The `HdmiCec_2` plugin allows you to configure HDMI Consumer Electronics Control (CEC) on a set-top box.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.HdmiCec_2*) |
| classname | string | Class name: *org.rdk.HdmiCec_2* |
| locator | string | Library name: *libWPEFrameworkHdmiCec_2.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.HdmiCec_2 plugin:

HdmiCec_2 interface methods:

| Method | Description |
| :-------- | :-------- |
| [getEnabled](#method.getEnabled) | Returns whether HDMI-CEC is enabled |
| [getOSDName](#method.getOSDName) | Returns the OSD name used by device |
| [getOTPEnabled](#method.getOTPEnabled) | Returns whether HDMI-CEC OPT option is enabled |
| [getVendorId](#method.getVendorId) | Returns the vendor ID used by the device |
| [performOTPAction](#method.performOTPAction) | Turns on the TV and takes back the input to the device |
| [setEnabled](#method.setEnabled) | Enables or disables HDMI-CEC |
| [setOSDName](#method.setOSDName) | Sets the OSD name |
| [setOTPEnabled](#method.setOTPEnabled) | Enables or disables HDMI-CEC OPT option |
| [setVendorId](#method.setVendorId) | Sets the vendor ID used by the device |


<a name="method.getEnabled"></a>
## *getEnabled <sup>method</sup>*

Returns whether HDMI-CEC is enabled.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enabled | boolean | Indicates whether HDMI-CEC is enabled (`true`) or disabled (`false`). The default value is `true` if the parameter has not been set before |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.HdmiCec_2.1.getEnabled"
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

<a name="method.getOSDName"></a>
## *getOSDName <sup>method</sup>*

Returns the OSD name used by device.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.name | string | The OSD name. The default value is `TV Box` id no value is set |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.HdmiCec_2.1.getOSDName"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "name": "Sky TV",
        "success": true
    }
}
```

<a name="method.getOTPEnabled"></a>
## *getOTPEnabled <sup>method</sup>*

Returns whether HDMI-CEC OPT option is enabled.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enabled | boolean | Indicates whether HDMI-CEC OTP is enabled (`true`) or disabled (`false`). The default value is `true` if the parameter has not been set before |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.HdmiCec_2.1.getOTPEnabled"
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

Returns the vendor ID used by the device.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.vendorid | string | <sup>*(optional)*</sup> The vendor ID. The default value is `0019FB` if no value is set. If the device is connected to an LG TV, then `00E091` is used as the vendor ID |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.HdmiCec_2.1.getVendorId"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "vendorid": "0x0019FB",
        "success": true
    }
}
```

<a name="method.performOTPAction"></a>
## *performOTPAction <sup>method</sup>*

Turns on the TV and takes back the input to the device.

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
    "method": "org.rdk.HdmiCec_2.1.performOTPAction"
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

Enables or disables HDMI-CEC.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Indicates whether HDMI-CEC is enabled (`true`) or disabled (`false`). The default value is `true` if the parameter has not been set before |

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
    "method": "org.rdk.HdmiCec_2.1.setEnabled",
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

<a name="method.setOSDName"></a>
## *setOSDName <sup>method</sup>*

Sets the OSD name.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.name | string | The OSD name. The default value is `TV Box` id no value is set |

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
    "method": "org.rdk.HdmiCec_2.1.setOSDName",
    "params": {
        "name": "Sky TV"
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

<a name="method.setOTPEnabled"></a>
## *setOTPEnabled <sup>method</sup>*

Enables or disables HDMI-CEC OPT option.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Indicates whether HDMI-CEC OTP is enabled (`true`) or disabled (`false`). The default value is `true` if the parameter has not been set before |

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
    "method": "org.rdk.HdmiCec_2.1.setOTPEnabled",
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

<a name="method.setVendorId"></a>
## *setVendorId <sup>method</sup>*

Sets the vendor ID used by the device.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.vendorid | string | The vendor ID. The default value is `0019FB` if no value is set. If the device is connected to an LG TV, then `00E091` is used as the vendor ID |

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
    "method": "org.rdk.HdmiCec_2.1.setVendorId",
    "params": {
        "vendorid": "0x0019FB"
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

