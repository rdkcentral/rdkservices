<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.DeviceInfo_Plugin"></a>
# DeviceInfo Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A DeviceInfo plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Interfaces](#head.Interfaces)
- [Properties](#head.Properties)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the DeviceInfo plugin. It includes detailed specification about its configuration and properties provided.

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

The `DeviceInfo` plugin allows retrieving of various device-related information.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *DeviceInfo*) |
| classname | string | Class name: *DeviceInfo* |
| locator | string | Library name: *libWPEFrameworkDeviceInfo.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Interfaces"></a>
# Interfaces

This plugin implements the following interfaces:

- [DeviceInfo.json](https://github.com/rdkcentral/ThunderInterfaces/blob/master/interfaces/DeviceInfo.json)

<a name="head.Properties"></a>
# Properties

The following properties are provided by the DeviceInfo plugin:

DeviceInfo interface properties:

| Property | Description |
| :-------- | :-------- |
| [stbcapabilities](#property.stbcapabilities) <sup>RO</sup> | Capabilities of the STB |
| [systeminfo](#property.systeminfo) <sup>RO</sup> | System general information |
| [addresses](#property.addresses) <sup>RO</sup> | Network interface addresses |
| [socketinfo](#property.socketinfo) <sup>RO</sup> | Socket information |
| [firmwareversion](#property.firmwareversion) <sup>RO</sup> | Versions maintained in version |
| [serialnumber](#property.serialnumber) <sup>RO</sup> | Serial number set by manufacturer |
| [modelid](#property.modelid) <sup>RO</sup> | Device model number or SKU |
| [make](#property.make) <sup>RO</sup> | Device manufacturer |
| [modelname](#property.modelname) <sup>RO</sup> | Friendly device model name |
| [devicetype](#property.devicetype) <sup>RO</sup> | Device type |
| [distributorid](#property.distributorid) <sup>RO</sup> | Partner ID or distributor ID for device |


<a name="property.stbcapabilities"></a>
## *stbcapabilities [<sup>property</sup>](#head.Properties)*

Provides access to the capabilities of the STB.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Capabilities of the STB |
| (property).hdr | boolean | Is HDR supported by this device |
| (property).atmos | boolean | Is Atmos supported by this device |
| (property).cec | boolean | Is CEC supported by this device |
| (property).hdcp | string | HDCP support (must be one of the following: *unavailable*, *hdcp_14*, *hdcp_20*, *hdcp_21*, *hdcp_22*) |
| (property).audio_outputs | array | Audio Output support |
| (property).audio_outputs[#] | string | Audio output supported by the STB (must be one of the following: *other*, *rf_modulator*, *analog*, *spdif*, *hdmi*, *displayport*) |
| (property).video_outputs | array | Video Output support |
| (property).video_outputs[#] | string | Video output supported by the STB (must be one of the following: *other*, *rf_modulator*, *composite*, *svideo*, *component*, *scart_rgb*, *hdmi*, *displayport*) |
| (property).output_resolutions | array | Supported resolutions |
| (property).output_resolutions[#] | string | Resolution supported by the STB (must be one of the following: *unknown*, *480i*, *480p*, *576i*, *576p*, *720p*, *1080i*, *1080p*, *2160p30*, *2160p60*, *4320p30*, *4320p60*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.1.stbcapabilities"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "hdr": false,
        "atmos": false,
        "cec": true,
        "hdcp": "hdcp_20",
        "audio_outputs": [
            "analog"
        ],
        "video_outputs": [
            "displayport"
        ],
        "output_resolutions": [
            "1080p"
        ]
    }
}
```

<a name="property.systeminfo"></a>
## *systeminfo [<sup>property</sup>](#head.Properties)*

Provides access to the system general information.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | System general information |
| (property).version | string | Software version (in form *version#hashtag*) |
| (property).uptime | number | System uptime (in seconds) |
| (property).totalram | number | Total installed system RAM memory (in bytes) |
| (property).freeram | number | Free system RAM memory (in bytes) |
| (property).totalswap | number | Total swap space (in bytes) |
| (property).freeswap | number | swap space still available (in bytes) |
| (property).devicename | string | Host name |
| (property).cpuload | string | Current CPU load (percentage) |
| (property).cpuloadavg | object | CPU load average |
| (property).cpuloadavg.avg1min | number | 1min cpuload average |
| (property).cpuloadavg.avg5min | number | 5min cpuload average |
| (property).cpuloadavg.avg15min | number | 15min cpuload average |
| (property).serialnumber | string | Device serial number |
| (property).time | string | Current system date and time |

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
        "version": "1.0#14452f612c3747645d54974255d11b8f3b4faa54",
        "uptime": 120,
        "totalram": 655757312,
        "freeram": 563015680,
        "totalswap": 789132680,
        "freeswap": 789132680,
        "devicename": "buildroot",
        "cpuload": "2",
        "cpuloadavg": {
            "avg1min": 789132680,
            "avg5min": 789132680,
            "avg15min": 789132680
        },
        "serialnumber": "WPEuCfrLF45",
        "time": "Mon, 11 Mar 2019 14:38:18"
    }
}
```

<a name="property.addresses"></a>
## *addresses [<sup>property</sup>](#head.Properties)*

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

<a name="property.socketinfo"></a>
## *socketinfo [<sup>property</sup>](#head.Properties)*

Provides access to the socket information.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Socket information |
| (property)?.total | number | <sup>*(optional)*</sup>  |
| (property)?.open | number | <sup>*(optional)*</sup>  |
| (property)?.link | number | <sup>*(optional)*</sup>  |
| (property)?.exception | number | <sup>*(optional)*</sup>  |
| (property)?.shutdown | number | <sup>*(optional)*</sup>  |
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

<a name="property.firmwareversion"></a>
## *firmwareversion [<sup>property</sup>](#head.Properties)*

Provides access to the versions maintained in version.txt.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Versions maintained in version.txt |
| (property)?.imagename | string | <sup>*(optional)*</sup>  |
| (property)?.sdk | string | <sup>*(optional)*</sup>  |
| (property)?.mediarite | string | <sup>*(optional)*</sup>  |
| (property)?.yocto | string | <sup>*(optional)*</sup> Yocto version (must be one of the following: *dunfell*, *morty*, *daisy*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.1.firmwareversion"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "imagename": "PX051AEI_VBN_2203_sprint_20220331225312sdy_NG",
        "sdk": "17.3",
        "mediarite": "8.3.53",
        "yocto": "dunfell"
    }
}
```

<a name="property.serialnumber"></a>
## *serialnumber [<sup>property</sup>](#head.Properties)*

Provides access to the serial number set by manufacturer.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Serial number set by manufacturer |
| (property).serialnumber | string |  |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.1.serialnumber"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "serialnumber": "PAW400003744"
    }
}
```

<a name="property.modelid"></a>
## *modelid [<sup>property</sup>](#head.Properties)*

Provides access to the device model number or SKU.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Device model number or SKU |
| (property).sku | string | Device model number or SKU (must be one of the following: *PLTL11AEI*, *ZWCN11MWI*, *SKTL11AEI*, *LS301*, *HSTP11MWR*, *HSTP11MWRFX50*, *ELTE11MWR*, *SKXI11ADS*, *SKXI11AIS*, *SKXI11ANS*, *SCXI11AIC*, *SCXI11BEI*, *CMXI11BEI*, *AX013AN*, *AX014AN*, *AX061AEI*, *MX011AN*, *CS011AN*, *CXD01ANI*, *PX001AN*, *PX013AN*, *PX022AN*, *PX032ANI*, *PX051AEI*, *PXD01ANI*, *SX022AN*, *TX061AEI*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.1.modelid"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "sku": "PX051AEI"
    }
}
```

<a name="property.make"></a>
## *make [<sup>property</sup>](#head.Properties)*

Provides access to the device manufacturer.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Device manufacturer |
| (property).make | string | Device manufacturer (must be one of the following: *platco*, *llama*, *hisense*, *element*, *sky*, *sercomm*, *commscope*, *arris*, *cisco*, *pace*, *samsung*, *technicolor*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.1.make"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "make": "pace"
    }
}
```

<a name="property.modelname"></a>
## *modelname [<sup>property</sup>](#head.Properties)*

Provides access to the friendly device model name.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Friendly device model name |
| (property).model | string |  |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.1.modelname"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "model": "Pace Xi5"
    }
}
```

<a name="property.devicetype"></a>
## *devicetype [<sup>property</sup>](#head.Properties)*

Provides access to the device type.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Device type |
| (property).devicetype | string | Device type (must be one of the following: *tv*, *IpStb*, *QamIpStb*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.1.devicetype"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "devicetype": "IpStb"
    }
}
```

<a name="property.distributorid"></a>
## *distributorid [<sup>property</sup>](#head.Properties)*

Provides access to the partner ID or distributor ID for device.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Partner ID or distributor ID for device |
| (property).distributorid | string | Partner ID or distributor ID for device (must be one of the following: *comcast*, *xglobal*) |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.1.distributorid"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "distributorid": "comcast"
    }
}
```

