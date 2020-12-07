<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.Display_Info_Plugin"></a>
# Display Info Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

DisplayInfo plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)
- [Properties](#head.Properties)
- [Notifications](#head.Notifications)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the DisplayInfo plugin. It includes detailed specification about its configuration, methods and properties provided, as well as notifications sent.

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

The DisplayInfo plugin allows retrieving of various display-related information.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *DisplayInfo*) |
| classname | string | Class name: *DisplayInfo* |
| locator | string | Library name: *libWPEFrameworkDisplayInfo.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the DisplayInfo plugin:

ConnectionProperties interface methods:

| Method | Description |
| :-------- | :-------- |
| [edid](#method.edid) | TV's Extended Display Identification Data |
| [widthincentimeters](#method.widthincentimeters) | Horizontal size in centimeters |
| [heightincentimeters](#method.heightincentimeters) | Vertical size in centimeters |


<a name="method.edid"></a>
## *edid <sup>method</sup>*

TV's Extended Display Identification Data.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.length | integer |  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.length | integer |  |
| result.data | string |  |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.edid",
    "params": {
        "length": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "length": 0,
        "data": ""
    }
}
```

<a name="method.widthincentimeters"></a>
## *widthincentimeters <sup>method</sup>*

Horizontal size in centimeters.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | integer |  |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.widthincentimeters"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": 0
}
```

<a name="method.heightincentimeters"></a>
## *heightincentimeters <sup>method</sup>*

Vertical size in centimeters.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | integer |  |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.heightincentimeters"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": 0
}
```

<a name="head.Properties"></a>
# Properties

The following properties are provided by the DisplayInfo plugin:

GraphicsProperties interface properties:

| Property | Description |
| :-------- | :-------- |
| [totalgpuram](#property.totalgpuram) <sup>RO</sup> | Total GPU DRAM memory (in bytes) |
| [freegpuram](#property.freegpuram) <sup>RO</sup> | Free GPU DRAM memory (in bytes) |

ConnectionProperties interface properties:

| Property | Description |
| :-------- | :-------- |
| [isaudiopassthrough](#property.isaudiopassthrough) <sup>RO</sup> | Current audio passthrough status on HDMI |
| [connected](#property.connected) <sup>RO</sup> | Current HDMI connection status |
| [width](#property.width) <sup>RO</sup> | Horizontal resolution of TV |
| [height](#property.height) <sup>RO</sup> | Vertical resolution of TV |
| [verticalfreq](#property.verticalfreq) <sup>RO</sup> | Vertical Frequency |
| [hdcpprotection](#property.hdcpprotection) | HDCP protocol used for transmission |
| [portname](#property.portname) <sup>RO</sup> | Video output port on the STB used for connection to TV |

HDRProperties interface properties:

| Property | Description |
| :-------- | :-------- |
| [tvcapabilities](#property.tvcapabilities) <sup>RO</sup> | HDR formats supported by TV |
| [stbcapabilities](#property.stbcapabilities) <sup>RO</sup> | HDR formats supported by STB |
| [hdrsetting](#property.hdrsetting) <sup>RO</sup> | HDR format in use |


<a name="property.totalgpuram"></a>
## *totalgpuram <sup>property</sup>*

Provides access to the total GPU DRAM memory (in bytes).

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | integer | Total GPU DRAM memory (in bytes) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.totalgpuram"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": 0
}
```

<a name="property.freegpuram"></a>
## *freegpuram <sup>property</sup>*

Provides access to the free GPU DRAM memory (in bytes).

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | integer | Free GPU DRAM memory (in bytes) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.freegpuram"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": 0
}
```

<a name="property.isaudiopassthrough"></a>
## *isaudiopassthrough <sup>property</sup>*

Provides access to the current audio passthrough status on HDMI.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | boolean | Current audio passthrough status on HDMI |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.isaudiopassthrough"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": false
}
```

<a name="property.connected"></a>
## *connected <sup>property</sup>*

Provides access to the current HDMI connection status.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | boolean | Current HDMI connection status |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.connected"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": false
}
```

<a name="property.width"></a>
## *width <sup>property</sup>*

Provides access to the horizontal resolution of TV.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | integer | Horizontal resolution of TV |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.width"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": 0
}
```

<a name="property.height"></a>
## *height <sup>property</sup>*

Provides access to the vertical resolution of TV.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | integer | Vertical resolution of TV |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.height"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": 0
}
```

<a name="property.verticalfreq"></a>
## *verticalfreq <sup>property</sup>*

Provides access to the vertical Frequency.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | integer | Vertical Frequency |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.verticalfreq"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": 0
}
```

<a name="property.hdcpprotection"></a>
## *hdcpprotection <sup>property</sup>*

Provides access to the HDCP protocol used for transmission.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | HDCP protocol used for transmission (must be one of the following: *HdcpUnencrypted*, *Hdcp1X*, *Hdcp2X*, *HdcpAuto*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.hdcpprotection"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "HdcpUnencrypted"
}
```

#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.hdcpprotection",
    "params": "HdcpUnencrypted"
}
```

#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "null"
}
```

<a name="property.portname"></a>
## *portname <sup>property</sup>*

Provides access to the video output port on the STB used for connection to TV.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Video output port on the STB used for connection to TV |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.portname"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": ""
}
```

<a name="property.tvcapabilities"></a>
## *tvcapabilities <sup>property</sup>*

Provides access to the HDR formats supported by TV.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | HDR formats supported by TV |
| (property)[#] | string |  (must be one of the following: *HdrOff*, *Hdr10*, *Hdr10Plus*, *HdrHlg*, *HdrDolbyvision*, *HdrTechnicolor*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.tvcapabilities"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": [
        "HdrOff"
    ]
}
```

<a name="property.stbcapabilities"></a>
## *stbcapabilities <sup>property</sup>*

Provides access to the HDR formats supported by STB.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | HDR formats supported by STB |
| (property)[#] | string |  (must be one of the following: *HdrOff*, *Hdr10*, *Hdr10Plus*, *HdrHlg*, *HdrDolbyvision*, *HdrTechnicolor*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.stbcapabilities"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": [
        "HdrOff"
    ]
}
```

<a name="property.hdrsetting"></a>
## *hdrsetting <sup>property</sup>*

Provides access to the HDR format in use.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | HDR format in use (must be one of the following: *HdrOff*, *Hdr10*, *Hdr10Plus*, *HdrHlg*, *HdrDolbyvision*, *HdrTechnicolor*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DisplayInfo.1.hdrsetting"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": "HdrOff"
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the DisplayInfo plugin:

ConnectionProperties interface events:

| Event | Description |
| :-------- | :-------- |
| [updated](#event.updated) |  |


<a name="event.updated"></a>
## *updated <sup>event</sup>*

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.event | string |  (must be one of the following: *PreResolutionChange*, *PostResolutionChange*, *HdmiChange*, *HdcpChange*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.updated",
    "params": {
        "event": "PreResolutionChange"
    }
}
```

