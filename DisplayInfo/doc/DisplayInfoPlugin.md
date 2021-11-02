<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.DisplayInfo_Plugin"></a>
# DisplayInfo Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A DisplayInfo plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Interfaces](#head.Interfaces)
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

The `DisplayInfo` plugin allows you to retrieve various display-related information.

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

<a name="head.Interfaces"></a>
# Interfaces

This plugin implements the following interfaces:

- [DisplayInfo.json](https://github.com/rdkcentral/ThunderInterfaces/tree/master/interfaces/DisplayInfo.json)

<a name="head.Methods"></a>
# Methods

The following methods are provided by the DisplayInfo plugin:

DisplayInfo interface methods:

| Method | Description |
| :-------- | :-------- |
| [edid](#method.edid) | Returns the TV's Extended Display Identification Data (EDID) |
| [widthincentimeters](#method.widthincentimeters) | Horizontal size in centimeters |
| [heightincentimeters](#method.heightincentimeters) | Vertical size in centimeters |


<a name="method.edid"></a>
## *edid [<sup>method</sup>](#head.Methods)*

Returns the TV's Extended Display Identification Data (EDID).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.length | integer | The EDID length |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.length | integer | The EDID length |
| result.data | string | The EDID data |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
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
    "id": 42,
    "result": {
        "length": 0,
        "data": "..."
    }
}
```

<a name="method.widthincentimeters"></a>
## *widthincentimeters [<sup>method</sup>](#head.Methods)*

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
    "id": 42,
    "method": "DisplayInfo.1.widthincentimeters"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": 0
}
```

<a name="method.heightincentimeters"></a>
## *heightincentimeters [<sup>method</sup>](#head.Methods)*

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
    "id": 42,
    "method": "DisplayInfo.1.heightincentimeters"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": 0
}
```

<a name="head.Properties"></a>
# Properties

The following properties are provided by the DisplayInfo plugin:

DisplayInfo interface properties:

| Property | Description |
| :-------- | :-------- |
| [totalgpuram](#property.totalgpuram) <sup>RO</sup> | Total GPU DRAM memory (in bytes) |
| [freegpuram](#property.freegpuram) <sup>RO</sup> | Free GPU DRAM memory (in bytes) |
| [isaudiopassthrough](#property.isaudiopassthrough) <sup>RO</sup> | Current audio passthrough status on HDMI |
| [connected](#property.connected) <sup>RO</sup> | Current HDMI connection status |
| [width](#property.width) <sup>RO</sup> | Horizontal resolution of the TV |
| [height](#property.height) <sup>RO</sup> | Vertical resolution of the TV |
| [verticalfreq](#property.verticalfreq) <sup>RO</sup> | Vertical Frequency |
| [hdcpprotection](#property.hdcpprotection) <sup>RO</sup> | HDCP protocol used for transmission |
| [portname](#property.portname) <sup>RO</sup> | Video output port on the STB used for connecting to the TV |
| [tvcapabilities](#property.tvcapabilities) <sup>RO</sup> | HDR formats supported by the TV |
| [stbcapabilities](#property.stbcapabilities) <sup>RO</sup> | HDR formats supported by the STB |
| [hdrsetting](#property.hdrsetting) <sup>RO</sup> | HDR format in use |
| [colorspace](#property.colorspace) <sup>RO</sup> | Display color space (chroma subsampling format) |
| [framerate](#property.framerate) <sup>RO</sup> | Display frame rate |
| [colourdepth](#property.colourdepth) <sup>RO</sup> | Display colour depth |
| [quantizationrange](#property.quantizationrange) <sup>RO</sup> | Display quantization range |
| [colorimetry](#property.colorimetry) <sup>RO</sup> | Display colorimetry |
| [eotf](#property.eotf) <sup>RO</sup> | Display Electro Optical Transfer Function (EOTF) |


<a name="property.totalgpuram"></a>
## *totalgpuram [<sup>property</sup>](#head.Properties)*

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
    "id": 42,
    "method": "DisplayInfo.1.totalgpuram"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": 381681664
}
```

<a name="property.freegpuram"></a>
## *freegpuram [<sup>property</sup>](#head.Properties)*

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
    "id": 42,
    "method": "DisplayInfo.1.freegpuram"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": 358612992
}
```

<a name="property.isaudiopassthrough"></a>
## *isaudiopassthrough [<sup>property</sup>](#head.Properties)*

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
    "id": 42,
    "method": "DisplayInfo.1.isaudiopassthrough"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": false
}
```

<a name="property.connected"></a>
## *connected [<sup>property</sup>](#head.Properties)*

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
    "id": 42,
    "method": "DisplayInfo.1.connected"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": true
}
```

<a name="property.width"></a>
## *width [<sup>property</sup>](#head.Properties)*

Provides access to the horizontal resolution of the TV.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | integer | Horizontal resolution of the TV |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DisplayInfo.1.width"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": 1280
}
```

<a name="property.height"></a>
## *height [<sup>property</sup>](#head.Properties)*

Provides access to the vertical resolution of the TV.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | integer | Vertical resolution of the TV |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DisplayInfo.1.height"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": 720
}
```

<a name="property.verticalfreq"></a>
## *verticalfreq [<sup>property</sup>](#head.Properties)*

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
    "id": 42,
    "method": "DisplayInfo.1.verticalfreq"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": 0
}
```

<a name="property.hdcpprotection"></a>
## *hdcpprotection [<sup>property</sup>](#head.Properties)*

Provides access to the HDCP protocol used for transmission.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | HDCP protocol used for transmission (must be one of the following: *HdcpUnencrypted*, *Hdcp1x*, *Hdcp2x*, *HdcpAuto*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DisplayInfo.1.hdcpprotection"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "Hdcp1x"
}
```

<a name="property.portname"></a>
## *portname [<sup>property</sup>](#head.Properties)*

Provides access to the video output port on the STB used for connecting to the TV.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Video output port on the STB used for connecting to the TV |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DisplayInfo.1.portname"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "hdmi"
}
```

<a name="property.tvcapabilities"></a>
## *tvcapabilities [<sup>property</sup>](#head.Properties)*

Provides access to the HDR formats supported by the TV.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | HDR formats supported by the TV (must be one of the following: *HdrOff*, *Hdr10*, *Hdr10Plus*, *HdrHlg*, *HdrDolbyvision*, *HdrTechnicolor*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DisplayInfo.1.tvcapabilities"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "HdrOff"
}
```

<a name="property.stbcapabilities"></a>
## *stbcapabilities [<sup>property</sup>](#head.Properties)*

Provides access to the HDR formats supported by the STB.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | HDR formats supported by the STB (must be one of the following: *HdrOff*, *Hdr10*, *Hdr10Plus*, *HdrHlg*, *HdrDolbyvision*, *HdrTechnicolor*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DisplayInfo.1.stbcapabilities"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "HdrOff"
}
```

<a name="property.hdrsetting"></a>
## *hdrsetting [<sup>property</sup>](#head.Properties)*

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
    "id": 42,
    "method": "DisplayInfo.1.hdrsetting"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "HdrOff"
}
```

<a name="property.colorspace"></a>
## *colorspace [<sup>property</sup>](#head.Properties)*

Provides access to the display color space (chroma subsampling format).

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Display color space (chroma subsampling format) (must be one of the following: *FORMAT_UNKNOWN*, *FORMAT_OTHER*, *FORMAT_RGB_444*, *FORMAT_YCBCR_444*, *FORMAT_YCBCR_422*, *FORMAT_YCBCR_420*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DisplayInfo.1.colorspace"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "FORMAT_RGB_444"
}
```

<a name="property.framerate"></a>
## *framerate [<sup>property</sup>](#head.Properties)*

Provides access to the display frame rate.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Display frame rate (must be one of the following: *FRAMERATE_UNKNOWN*, *FRAMERATE_23_976*, *FRAMERATE_24*, *FRAMERATE_25*, *FRAMERATE_29_97*, *FRAMERATE_30*, *FRAMERATE_47_952*, *FRAMERATE_48*, *FRAMERATE_50*, *FRAMERATE_59_94*, *FRAMERATE_60*, *FRAMERATE_119_88*, *FRAMERATE_120*, *FRAMERATE_144*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DisplayInfo.1.framerate"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "FRAMERATE_60"
}
```

<a name="property.colourdepth"></a>
## *colourdepth [<sup>property</sup>](#head.Properties)*

Provides access to the display colour depth.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Display colour depth (must be one of the following: *COLORDEPTH_UNKNOWN*, *COLORDEPTH_8_BIT*, *COLORDEPTH_10_BIT*, *COLORDEPTH_12_BIT*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DisplayInfo.1.colourdepth"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "COLORDEPTH_8_BIT"
}
```

<a name="property.quantizationrange"></a>
## *quantizationrange [<sup>property</sup>](#head.Properties)*

Provides access to the display quantization range.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Display quantization range (must be one of the following: *QUANTIZATIONRANGE_UNKNOWN*, *QUANTIZATIONRANGE_LIMITED*, *QUANTIZATIONRANGE_FULL*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DisplayInfo.1.quantizationrange"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "QUANTIZATIONRANGE_UNKNOWN"
}
```

<a name="property.colorimetry"></a>
## *colorimetry [<sup>property</sup>](#head.Properties)*

Provides access to the display colorimetry.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Display colorimetry (must be one of the following: *COLORIMETRY_UNKNOWN*, *COLORIMETRY_OTHER*, *COLORIMETRY_SMPTE170M*, *COLORIMETRY_BT709*, *COLORIMETRY_XVYCC601*, *COLORIMETRY_XVYCC709*, *COLORIMETRY_SYCC601*, *COLORIMETRY_OPYCC601*, *COLORIMETRY_OPRGB*, *COLORIMETRY_BT2020YCCBCBRC*, *COLORIMETRY_BT2020RGB_YCBCR*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DisplayInfo.1.colorimetry"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "COLORIMETRY_OTHER"
}
```

<a name="property.eotf"></a>
## *eotf [<sup>property</sup>](#head.Properties)*

Provides access to the display Electro Optical Transfer Function (EOTF).

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | string | Display Electro Optical Transfer Function (EOTF) (must be one of the following: *EOTF_UNKNOWN*, *EOTF_OTHER*, *EOTF_BT1886*, *EOTF_BT2100*, *EOTF_SMPTE_ST_2084*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DisplayInfo.1.eotf"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "EOTF_SMPTE_ST_2084"
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the DisplayInfo plugin:

DisplayInfo interface events:

| Event | Description |
| :-------- | :-------- |
| [updated](#event.updated) | Triggered when the connection changes or is updated |


<a name="event.updated"></a>
## *updated [<sup>event</sup>](#head.Notifications)*

Triggered when the connection changes or is updated.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.event | string | The type of change (must be one of the following: *PreResolutionChange*, *PostResolutionChange*, *HdmiChange*, *HdcpChange*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.updated",
    "params": {
        "event": "HdmiChange"
    }
}
```

