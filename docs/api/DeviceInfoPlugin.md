<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.Device_Info_API"></a>
# Device Info API

**Version: [1.0.0]()**

A DeviceInfo interface for Thunder framework.

(Defined by [DeviceInfo.json](https://github.com/rdkcentral/ThunderInterfaces/blob/master/interfaces/DeviceInfo.json))

### Table of Contents

- [Abbreviation, Acronyms and Terms](#head.Abbreviation,_Acronyms_and_Terms)
- [Description](#head.Description)
- [Methods](#head.Methods)
- [Properties](#head.Properties)

<a name="head.Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="head.Description"></a>
# Description

DeviceInfo JSON-RPC interface.

<a name="head.Methods"></a>
# Methods

The following methods are provided by the DeviceInfo interface:

DeviceInfo interface methods:

| Method | Description |
| :-------- | :-------- |
| [supportedresolutions](#method.supportedresolutions) | Supported resolutions on the selected video display port |
| [defaultresolution](#method.defaultresolution) | Default resolution on the selected video display port |
| [supportedhdcp](#method.supportedhdcp) | Supported HDCP version on the selected video display port |
| [audiocapabilities](#method.audiocapabilities) | Audio capabilities for the specified audio port |
| [ms12capabilities](#method.ms12capabilities) | MS12 audio capabilities for the specified audio port |
| [supportedms12audioprofiles](#method.supportedms12audioprofiles) | Supported MS12 audio profiles for the specified audio port |


<a name="method.supportedresolutions"></a>
## *supportedresolutions [<sup>method</sup>](#head.Methods)*

Supported resolutions on the selected video display port.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.videoDisplay | string | <sup>*(optional)*</sup> Video display port name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedResolutions | array |  |
| result.supportedResolutions[#] | string | Resolution supported by the STB (must be one of the following: *unknown*, *480i*, *480p*, *576i*, *576p*, *576p50*, *720p*, *720p50*, *1080i*, *1080i25*, *1080i50*, *1080p*, *1080p24*, *1080p25*, *1080p30*, *1080p50*, *1080p60*, *2160p30*, *2160p50*, *2160p60*, *4320p30*, *4320p60*) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.supportedresolutions",
    "params": {
        "videoDisplay": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedResolutions": [
            "1080p"
        ]
    }
}
```

<a name="method.defaultresolution"></a>
## *defaultresolution [<sup>method</sup>](#head.Methods)*

Default resolution on the selected video display port.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.videoDisplay | string | <sup>*(optional)*</sup> Video display port name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.defaultResolution | string | Resolution supported by the STB (must be one of the following: *unknown*, *480i*, *480p*, *576i*, *576p*, *576p50*, *720p*, *720p50*, *1080i*, *1080i25*, *1080i50*, *1080p*, *1080p24*, *1080p25*, *1080p30*, *1080p50*, *1080p60*, *2160p30*, *2160p50*, *2160p60*, *4320p30*, *4320p60*) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.defaultresolution",
    "params": {
        "videoDisplay": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "defaultResolution": "1080p"
    }
}
```

<a name="method.supportedhdcp"></a>
## *supportedhdcp [<sup>method</sup>](#head.Methods)*

Supported HDCP version on the selected video display port.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.videoDisplay | string | <sup>*(optional)*</sup> Video display port name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedHDCPVersion | string | HDCP support (must be one of the following: *unavailable*, *1.4*, *2.0*, *2.1*, *2.2*) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.supportedhdcp",
    "params": {
        "videoDisplay": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedHDCPVersion": "2.0"
    }
}
```

<a name="method.audiocapabilities"></a>
## *audiocapabilities [<sup>method</sup>](#head.Methods)*

Audio capabilities for the specified audio port.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.AudioCapabilities | array | An array of audio capabilities |
| result.AudioCapabilities[#] | string | Audio capability (must be one of the following: *none*, *ATMOS*, *DOLBY DIGITAL*, *DOLBY DIGITAL PLUS*, *Dual Audio Decode*, *DAPv2*, *MS12*) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.audiocapabilities",
    "params": {
        "audioPort": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "AudioCapabilities": [
            "none"
        ]
    }
}
```

<a name="method.ms12capabilities"></a>
## *ms12capabilities [<sup>method</sup>](#head.Methods)*

MS12 audio capabilities for the specified audio port.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.MS12Capabilities | array | An array of MS12 audio capabilities |
| result.MS12Capabilities[#] | string | MS12 audio capability (must be one of the following: *none*, *Dolby Volume*, *Inteligent Equalizer*, *Dialogue Enhancer*) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.ms12capabilities",
    "params": {
        "audioPort": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "MS12Capabilities": [
            "Dolby Volume"
        ]
    }
}
```

<a name="method.supportedms12audioprofiles"></a>
## *supportedms12audioprofiles [<sup>method</sup>](#head.Methods)*

Supported MS12 audio profiles for the specified audio port.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.audioPort | string | <sup>*(optional)*</sup> Audio port name |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedMS12AudioProfiles | array | An array of MS12 audio profiles |
| result.supportedMS12AudioProfiles[#] | string |  |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.supportedms12audioprofiles",
    "params": {
        "audioPort": "HDMI0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedMS12AudioProfiles": [
            "Movie"
        ]
    }
}
```

<a name="head.Properties"></a>
# Properties

The following properties are provided by the DeviceInfo interface:

DeviceInfo interface properties:

| Property | Description |
| :-------- | :-------- |
| [systeminfo](#property.systeminfo) <sup>RO</sup> | System general information |
| [addresses](#property.addresses) <sup>RO</sup> | Network interface addresses |
| [socketinfo](#property.socketinfo) <sup>RO</sup> | Socket information |
| [firmwareversion](#property.firmwareversion) <sup>RO</sup> | Versions maintained in version |
| [serialnumber](#property.serialnumber) <sup>RO</sup> | Serial number set by manufacturer |
| [modelid](#property.modelid) <sup>RO</sup> | Device model number or SKU |
| [make](#property.make) <sup>RO</sup> | Device manufacturer |
| [modelname](#property.modelname) <sup>RO</sup> | Friendly device model name |
| [devicetype](#property.devicetype) <sup>RO</sup> | Device type |
| [socname](#property.socname) <sup>RO</sup> | SOC Name |
| [distributorid](#property.distributorid) <sup>RO</sup> | Partner ID or distributor ID for device |
| [supportedaudioports](#property.supportedaudioports) <sup>RO</sup> | Audio ports supported on the device (all ports that are physically present) |
| [supportedvideodisplays](#property.supportedvideodisplays) <sup>RO</sup> | Video ports supported on the device (all ports that are physically present) |
| [hostedid](#property.hostedid) <sup>RO</sup> | EDID of the host |


<a name="property.systeminfo"></a>
## *systeminfo [<sup>property</sup>](#head.Properties)*

Provides access to the system general information.

> This property is **read-only**.

### Events

No Events

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
    "method": "DeviceInfo.systeminfo"
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

### Events

No Events

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
    "method": "DeviceInfo.addresses"
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

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Socket information |
| (property).runs | number | Number of runs |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.socketinfo"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "runs": 1
    }
}
```

<a name="property.firmwareversion"></a>
## *firmwareversion [<sup>property</sup>](#head.Properties)*

Provides access to the versions maintained in version.txt.

> This property is **read-only**.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Versions maintained in version.txt |
| (property).imagename | string |  |
| (property)?.sdk | string | <sup>*(optional)*</sup>  |
| (property)?.mediarite | string | <sup>*(optional)*</sup>  |
| (property)?.yocto | string | <sup>*(optional)*</sup> Yocto version (must be one of the following: *dunfell*, *morty*, *daisy*) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.firmwareversion"
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

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Serial number set by manufacturer |
| (property).serialnumber | string |  |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.serialnumber"
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

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Device model number or SKU |
| (property).sku | string | Device model number or SKU (must be one of the following: *PLTL11AEI*, *ZWCN11MWI*, *SKTL11AEI*, *LS301*, *HSTP11MWR*, *HSTP11MWRFX50*, *ELTE11MWR*, *SKXI11ADS*, *SKXI11AIS*, *SKXI11ANS*, *SCXI11AIC*, *SCXI11BEI*, *CMXI11BEI*, *AX013AN*, *AX014AN*, *AX061AEI*, *MX011AN*, *CS011AN*, *CXD01ANI*, *PX001AN*, *PX013AN*, *PX022AN*, *PX032ANI*, *PX051AEI*, *PXD01ANI*, *SX022AN*, *TX061AEI*) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.modelid"
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

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Device manufacturer |
| (property).make | string | Device manufacturer (must be one of the following: *platco*, *llama*, *hisense*, *element*, *sky*, *sercomm*, *commscope*, *arris*, *cisco*, *pace*, *samsung*, *technicolor*, *Amlogic_Inc*) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.make"
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

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Friendly device model name |
| (property).model | string |  |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.modelname"
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

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Device type |
| (property).devicetype | string | Device type (must be one of the following: *tv*, *IpStb*, *QamIpStb*) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.devicetype"
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

<a name="property.socname"></a>
## *socname [<sup>property</sup>](#head.Properties)*

Provides access to the SOC Name.

> This property is **read-only**.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | SOC Name |
| (property).socname | string | SOC Name (must be one of the following: *Amlogic*, *Realtek*, *Broadcom*) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.socname"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "socname": "Realtek"
    }
}
```

<a name="property.distributorid"></a>
## *distributorid [<sup>property</sup>](#head.Properties)*

Provides access to the partner ID or distributor ID for device.

> This property is **read-only**.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Partner ID or distributor ID for device |
| (property).distributorid | string | Partner ID or distributor ID for device (must be one of the following: *comcast*, *xglobal*, *sky-de*, *sky-italia*, *sky-uk*, *sky-uk-dev*, *sky-deu*, *sky-deu-dev*, *sky-it*, *sky-it-dev*, *cox*, *cox-hospitality*, *cox-dev*, *cox-qa*, *MIT*, *shaw*, *shaw-dev*, *rogers*, *rogers-dev*, *videotron*, *charter*, *charter-dev*) |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.distributorid"
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

<a name="property.supportedaudioports"></a>
## *supportedaudioports [<sup>property</sup>](#head.Properties)*

Provides access to the audio ports supported on the device (all ports that are physically present).

> This property is **read-only**.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Audio ports supported on the device (all ports that are physically present) |
| (property).supportedAudioPorts | array |  |
| (property).supportedAudioPorts[#] | string |  |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.supportedaudioports"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedAudioPorts": [
            "HDMI0"
        ]
    }
}
```

<a name="property.supportedvideodisplays"></a>
## *supportedvideodisplays [<sup>property</sup>](#head.Properties)*

Provides access to the video ports supported on the device (all ports that are physically present).

> This property is **read-only**.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Video ports supported on the device (all ports that are physically present) |
| (property).supportedVideoDisplays | array |  |
| (property).supportedVideoDisplays[#] | string |  |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.supportedvideodisplays"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedVideoDisplays": [
            "HDMI0"
        ]
    }
}
```

<a name="property.hostedid"></a>
## *hostedid [<sup>property</sup>](#head.Properties)*

Provides access to the EDID of the host.

> This property is **read-only**.

### Events

No Events

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | EDID of the host |
| (property).EDID | string | A base64 encoded byte array string representing the EDID |

### Errors

| Code | Message | Description |
| :-------- | :-------- | :-------- |
| 1 | ```ERROR_GENERAL``` | General error |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DeviceInfo.hostedid"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "EDID": "AP///////wAQrMLQVEJTMQUdAQOANR546q11qVRNnSYPUFSlSwCBALMA0QBxT6lAgYDRwAEBVl4AoKCgKVAwIDUADighAAAaAAAA/wBNWTNORDkxVjFTQlQKAAAA/ABERUxMIFAyNDE4RAogAAAA/QAxVh1xHAAKICAgICAgARsCAxuxUJAFBAMCBxYBBhESFRMUHyBlAwwAEAACOoAYcTgtQFgsRQAOKCEAAB4BHYAYcRwWIFgsJQAOKCEAAJ6/FgCggDgTQDAgOgAOKCEAABp+OQCggDgfQDAgOgAOKCEAABoAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA2A"
    }
}
```

