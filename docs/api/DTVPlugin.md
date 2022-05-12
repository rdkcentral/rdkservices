<!-- Generated automatically, DO NOT EDIT! -->
<a name="DTV_Plugin"></a>
# DTV Plugin

**Version: 2.0**

**Status: :black_circle::black_circle::white_circle:**

A DTV plugin for Thunder framework.

### Table of Contents

- [Introduction](#Introduction)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Properties](#Properties)
- [Notifications](#Notifications)

<a name="Introduction"></a>
# Introduction

<a name="Scope"></a>
## Scope

This document describes purpose and functionality of the DTV plugin. It includes detailed specification about its configuration, methods and properties provided, as well as notifications sent.

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

The DTV plugin provides access to DVB services provided by a DVB stack.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *DTV*) |
| classname | string | Class name: *DTV* |
| locator | string | Library name: *libWPEFrameworkDTV.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the DTV plugin:

DTV interface methods:

| Method | Description |
| :-------- | :-------- |
| [addLnb](#addLnb) | Add a new LNB to the database |
| [addSatellite](#addSatellite) | Add a new satellite to the database |
| [startServiceSearch](#startServiceSearch) | Starts a service search |
| [finishServiceSearch](#finishServiceSearch) | Finishes a service search |
| [startPlaying](#startPlaying) | Starts playing the specified service |
| [stopPlaying](#stopPlaying) | Stops playing the specified service |


<a name="addLnb"></a>
## *addLnb*

Add a new LNB to the database.
  
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.name | string | User specified name for the LNB settings |
| params.type | string |  (must be one of the following: *single*, *universal*, *unicable*) |
| params.power | string |  (must be one of the following: *off*, *on*, *auto*) |
| params.diseqc_tone | string |  (must be one of the following: *off*, *a*, *b*) |
| params.diseqc_cswitch | string |  (must be one of the following: *off*, *a*, *b*, *c*, *d*) |
| params.is22k | boolean | 22kHz tone on/off |
| params.is12v | boolean | 12 volts on/off |
| params.ispulseposition | boolean | Pulse position motor used to control dish position |
| params.isdiseqcposition | boolean | Dish positioning is controlled by DiSEqC |
| params.issmatv | boolean | SMATV switcher on/off |
| params.diseqcrepeats | number | Number of times to send a DiSEqC message to make sure it's been received |
| params.u_switch | number | DiSEqC uncommitted switch selector |
| params.unicablechannel | number | Unicable channel |
| params?.unicablefreq | number | <sup>*(optional)*</sup> Unicable frequency |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean | true if the LNB is added, false otherwise |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.addLnb",
    "params": {
        "name": "Universal",
        "type": "single",
        "power": "off",
        "diseqc_tone": "off",
        "diseqc_cswitch": "off",
        "is22k": false,
        "is12v": false,
        "ispulseposition": false,
        "isdiseqcposition": false,
        "issmatv": false,
        "diseqcrepeats": 0,
        "u_switch": 0,
        "unicablechannel": 0,
        "unicablefreq": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": true
}
```

<a name="addSatellite"></a>
## *addSatellite*

Add a new satellite to the database.
  
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.name | string | User specified name for the satellite settings |
| params.longitude | number | Longitudinal location of the satellite in 1/10ths of a degree, with an east coordinate given as a positive value and a west coordinate as negative. Astra 28.2E would be defined as 282 and Eutelsat 5.0W would be -50 |
| params.lnb | string | Name of the LNB settings to be used when tuning to this satellite |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean | true if the satellite is added, false otherwise |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.addSatellite",
    "params": {
        "name": "Astra 28.2E",
        "longitude": 282,
        "lnb": "Universal"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": true
}
```

<a name="startServiceSearch"></a>
## *startServiceSearch*

Starts a service search.
 
### Events  
| Event | Description | 
| :----------- | :----------- | 
|`searchstatus`|Triggered during the course of a service search.|
 |`serviceupdated`|Triggered when a service is added|.

Also see: [searchstatus](#searchstatus), [serviceupdated](#serviceupdated)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.tunertype | string | Tuner type (must be one of the following: *none*, *dvbs*, *dvbt*, *dvbc*) |
| params.searchtype | string |  (must be one of the following: *frequency*, *network*) |
| params.retune | boolean | true if current services are to be replaced in the database, false if the scan is to update the existing services |
| params.usetuningparams | boolean | Set to true if the optional tuning parameters are defined |
| params?.dvbstuningparams | object | <sup>*(optional)*</sup> Parameters required to tune to a DVB-S/S2 signal |
| params?.dvbstuningparams.satellite | string | Name of the satellite settings to be used |
| params?.dvbstuningparams.frequency | number | Frequency of the transponder, in kHz |
| params?.dvbstuningparams.polarity | string |  (must be one of the following: *horizontal*, *vertical*, *left*, *right*) |
| params?.dvbstuningparams.symbolrate | number | Symbol rate in Ksymb/sec |
| params?.dvbstuningparams.fec | string | Forward error correction setting (must be one of the following: *fecauto*, *fec1_2*, *fec2_3*, *fec3_4*, *fec5_6*, *fec7_8*, *fec1_4*, *fec1_3*, *fec2_5*, *fec8_9*, *fec9_10*, *fec3_5*, *fec4_5*) |
| params?.dvbstuningparams.modulation | string |  (must be one of the following: *auto*, *qpsk*, *8psk*, *16qam*) |
| params?.dvbstuningparams.dvbs2 | boolean | True when tuning to a DVB-S2 signal |
| params?.dvbctuningparams | object | <sup>*(optional)*</sup> Parameters required to tune to a DVB-C signal |
| params?.dvbctuningparams.frequency | number | Frequency of the transport, in Hz |
| params?.dvbctuningparams.symbolrate | number | Symbol rate in Ksymb/sec |
| params?.dvbctuningparams.modulation | string |  (must be one of the following: *auto*, *4qam*, *8qam*, *16qam*, *32qam*, *64qam*, *128qam*, *256qam*) |
| params?.dvbttuningparams | object | <sup>*(optional)*</sup> Parameters required to tune to a DVB-T/T2 signal |
| params?.dvbttuningparams.frequency | number | Frequency of the transport, in Hz |
| params?.dvbttuningparams.bandwidth | string | Channel bandwidth (must be one of the following: *5MHz*, *6MHz*, *7MHz*, *8MHz*, *10MHz*, *undefined*) |
| params?.dvbttuningparams.mode | string | OFDM mode (must be one of the following: *ofdm_1K*, *ofdm_2K*, *ofdm_4K*, *ofdm_8K*, *ofdm_16K*, *ofdm_32K*, *undefined*) |
| params?.dvbttuningparams.dvbt2 | boolean | True when tuning to a DVB-T2 signal |
| params?.dvbttuningparams?.plpid | number | <sup>*(optional)*</sup> PLP ID for tuning to a DVB-T2 signal |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean | true if the search is started, false otherwise |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.startServiceSearch",
    "params": {
        "tunertype": "none",
        "searchtype": "frequency",
        "retune": false,
        "usetuningparams": true,
        "dvbstuningparams": {
            "satellite": "Astra 28.2",
            "frequency": 10714000,
            "polarity": "horizontal",
            "symbolrate": 22000,
            "fec": "fecauto",
            "modulation": "auto",
            "dvbs2": false
        },
        "dvbctuningparams": {
            "frequency": 474000000,
            "symbolrate": 6900,
            "modulation": "auto"
        },
        "dvbttuningparams": {
            "frequency": 474000000,
            "bandwidth": "5MHz",
            "mode": "ofdm_1K",
            "dvbt2": false,
            "plpid": 0
        }
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": true
}
```

<a name="finishServiceSearch"></a>
## *finishServiceSearch*

Finishes a service search.
  
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.tunertype | string | Tuner type (must be one of the following: *none*, *dvbs*, *dvbt*, *dvbc*) |
| params.savechanges | boolean | true if the services found during the search should be saved |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean | false if the tunertype isn't valid, true otherwise |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.finishServiceSearch",
    "params": {
        "tunertype": "none",
        "savechanges": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": true
}
```

<a name="startPlaying"></a>
## *startPlaying*

Starts playing the specified service.
  
### Events
| Event | Description |
| :----------- | :----------- |
|`serviceupdated`|Triggered when info for a service changes|
|`eventchanged`|Triggered when the EIT 'now' event changes|
|`videochanged`|Triggered when the video PID or codec are changed|
|`audiochanged`|Triggered when the audio PID or codec are changed|
|`subtitleschanged`|Triggered when the subtitle PID or details are changed| .

Also see: [serviceupdated](#serviceupdated), [eventchanged](#eventchanged), [videochanged](#videochanged), [audiochanged](#audiochanged), [subtitleschanged](#subtitleschanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.dvburi | string | <sup>*(optional)*</sup> DVB triplet of the form a.b.c, where 'a' is the original network ID, 'b' is the transport ID and 'c' is the service ID, in decimal |
| params?.lcn | number | <sup>*(optional)*</sup> LCN of the service to be tuned to |
| params?.monitoronly | boolean | <sup>*(optional)*</sup> (Version 2) set to 'true' to monitor SI/PSI data only. Will default to 'false' if not defined |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | number | The play handle assigned to play the given service. Will be -1 if the service can't be played |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.startPlaying",
    "params": {
        "dvburi": "2.2041.9212",
        "lcn": 0,
        "monitoronly": true
    }
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

<a name="stopPlaying"></a>
## *stopPlaying*

Stops playing the specified service.
  
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | number | The play handle returned by startPlaying |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | null | Always null |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.stopPlaying",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": null
}
```

<a name="Properties"></a>
# Properties

The following properties are provided by the DTV plugin:

DTV interface properties:

| Property | Description |
| :-------- | :-------- |
| [numberOfCountries](#numberOfCountries) <sup>RO</sup> | Number of country configurations available |
| [countryList](#countryList) <sup>RO</sup> | Array containing the name and 3 character ISO country code for all the available country configurations |
| [country](#country) | Country configuration using the ISO 3-character country code |
| [lnbList](#lnbList) <sup>RO</sup> | Array of LNBs defined in the database |
| [satelliteList](#satelliteList) <sup>RO</sup> | Array of satellites defined in the database |
| [numberOfServices](#numberOfServices) <sup>RO</sup> | Total number of services in the service database |
| [serviceList](#serviceList) <sup>RO</sup> | List of services for the given type of tuner, transport (version 2), or all services if neither is given |
| [serviceInfo](#serviceInfo) <sup>RO</sup> | Information for the given service as defined by its DVB triplet URI |
| [serviceComponents](#serviceComponents) <sup>RO</sup> | (Version 2) array of components for the given service defined by its URI |
| [transportInfo](#transportInfo) <sup>RO</sup> | Information for the given transport as defined by its DVB doublet URI |
| [nowNextEvents](#nowNextEvents) <sup>RO</sup> | Now and next events (EITp/f) for the given service |
| [scheduleEvents](#scheduleEvents) <sup>RO</sup> | Events which are scheduled (EITsched) for the given service |
| [extendedEventInfo](#extendedEventInfo) <sup>RO</sup> | Extended event info for the given service and event ID (version 2) |
| [status](#status) <sup>RO</sup> | Information related to the play handle defined by the index |
| [signalInfo](#signalInfo) <sup>RO</sup> | strength and quality of the currently tuned signal for the given play handle (version 2) |


<a name="numberOfCountries"></a>
## *numberOfCountries [<sup>property</sup>](#Properties)*

Provides access to the number of country configurations available.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | number | Number of country configurations available |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.numberOfCountries"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": 5
}
```

<a name="countryList"></a>
## *countryList [<sup>property</sup>](#Properties)*

Provides access to the array containing the name and 3 character ISO country code for all the available country configurations.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | List of available country configurations |
| (property)[#] | object |  |
| (property)[#].name | string | Name of the country as a UTF-8 string |
| (property)[#].code | number | 3-character ISO code for the country |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.countryList"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        {
            "name": "UK",
            "code": 6775410
        }
    ]
}
```

<a name="country"></a>
## *country [<sup>property</sup>](#Properties)*

Provides access to the country configuration using the ISO 3-character country code.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | number | 3-character ISO code for the country |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.country"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": 6775410
}
```

#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.country",
    "params": 6775410
}
```

#### Set Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": "null"
}
```

<a name="lnbList"></a>
## *lnbList [<sup>property</sup>](#Properties)*

Provides access to the array of LNBs defined in the database.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | Array of LNBs defined in the database |
| (property)[#] | object |  |
| (property)[#].name | string | User specified name for the LNB settings |
| (property)[#].type | string |  (must be one of the following: *single*, *universal*, *unicable*) |
| (property)[#].power | string |  (must be one of the following: *off*, *on*, *auto*) |
| (property)[#].diseqc_tone | string |  (must be one of the following: *off*, *a*, *b*) |
| (property)[#].diseqc_cswitch | string |  (must be one of the following: *off*, *a*, *b*, *c*, *d*) |
| (property)[#].is22k | boolean | 22kHz tone on/off |
| (property)[#].is12v | boolean | 12 volts on/off |
| (property)[#].ispulseposition | boolean | Pulse position motor used to control dish position |
| (property)[#].isdiseqcposition | boolean | Dish positioning is controlled by DiSEqC |
| (property)[#].issmatv | boolean | SMATV switcher on/off |
| (property)[#].diseqcrepeats | number | Number of times to send a DiSEqC message to make sure it's been received |
| (property)[#].u_switch | number | DiSEqC uncommitted switch selector |
| (property)[#].unicablechannel | number | Unicable channel |
| (property)[#]?.unicablefreq | number | <sup>*(optional)*</sup> Unicable frequency |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.lnbList"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        {
            "name": "Universal",
            "type": "single",
            "power": "off",
            "diseqc_tone": "off",
            "diseqc_cswitch": "off",
            "is22k": false,
            "is12v": false,
            "ispulseposition": false,
            "isdiseqcposition": false,
            "issmatv": false,
            "diseqcrepeats": 0,
            "u_switch": 0,
            "unicablechannel": 0,
            "unicablefreq": 0
        }
    ]
}
```

<a name="satelliteList"></a>
## *satelliteList [<sup>property</sup>](#Properties)*

Provides access to the array of satellites defined in the database.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | Array of satellites defined in the database |
| (property)[#] | object |  |
| (property)[#].name | string | User specified name for the satellite settings |
| (property)[#].longitude | number | Longitudinal location of the satellite in 1/10ths of a degree, with an east coordinate given as a positive value and a west coordinate as negative. Astra 28.2E would be defined as 282 and Eutelsat 5.0W would be -50 |
| (property)[#].lnb | string | Name of the LNB settings to be used when tuning to this satellite |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.satelliteList"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        {
            "name": "Astra 28.2E",
            "longitude": 282,
            "lnb": "Universal"
        }
    ]
}
```

<a name="numberOfServices"></a>
## *numberOfServices [<sup>property</sup>](#Properties)*

Provides access to the total number of services in the service database.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | number | Total number of services in the service database |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.numberOfServices"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": 145
}
```

<a name="serviceList"></a>
## *serviceList [<sup>property</sup>](#Properties)*

Provides access to the list of services for the given type of tuner, transport (version 2), or all services if neither is given.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | List of services for the given type of tuner, transport (version 2), or all services if neither is given |
| (property)[#] | object | Service information |
| (property)[#].fullname | string | (Version 2) Full service name as given by the service descriptor in the SDT |
| (property)[#].shortname | string | Short service name as given by the service descriptor in the SDT |
| (property)[#].dvburi | string | DVB triplet of the form a.b.c, where 'a' is the original network ID, 'b' is the transport ID and 'c' is the service ID, in decimal |
| (property)[#].servicetype | string | (Version 2) Service type as given by the service descriptor in the SDT (must be one of the following: *tv*, *radio*, *teletext*, *nvod*, *nvod_timeshift*, *mosaic*, *avc_radio*, *avc_mosaic*, *data*, *mpeg2_hd*, *avc_sd_tv*, *avc_sd_nvod_timeshift*, *avc_sd_nvod*, *hd_tv*, *avc_hd_nvod_timeshift*, *avc_hd_nvod*, *uhd_tv*, *unknown*) |
| (property)[#].lcn | number | Logical channel number |
| (property)[#].scrambled | boolean | (Version 2) True if the service is marked as scrambled in the SDT |
| (property)[#].hascadescriptor | boolean | (Version 2) True if there is at least one CA descriptor in the service's PMT |
| (property)[#].hidden | boolean | (Version 2) True if the service should be hidden |
| (property)[#].selectable | boolean | (Version 2) True if the service should be user selectable |
| (property)[#].runningstatus | string | (Version 2) The running status as given by the service descriptor in the SDT (must be one of the following: *undefined*, *notrunning*, *startssoon*, *pausing*, *running*, *offair*) |

> The *optional tuner type, or transport dvb doublet* argument shall be passed as the index to the property, e.g. *DTV.1.serviceList@dvbs or 9018.4161*.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.serviceList@dvbs or 9018.4161"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        {
            "fullname": "Channel 4",
            "shortname": "C4",
            "dvburi": "2.2041.9212",
            "servicetype": "tv",
            "lcn": 1001,
            "scrambled": false,
            "hascadescriptor": false,
            "hidden": false,
            "selectable": false,
            "runningstatus": "running"
        }
    ]
}
```

<a name="serviceInfo"></a>
## *serviceInfo [<sup>property</sup>](#Properties)*

Provides access to the information for the given service as defined by its DVB triplet URI.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Information for the given service as defined by its DVB triplet URI |
| (property).fullname | string | (Version 2) Full service name as given by the service descriptor in the SDT |
| (property).shortname | string | Short service name as given by the service descriptor in the SDT |
| (property).dvburi | string | DVB triplet of the form a.b.c, where 'a' is the original network ID, 'b' is the transport ID and 'c' is the service ID, in decimal |
| (property).servicetype | string | (Version 2) Service type as given by the service descriptor in the SDT (must be one of the following: *tv*, *radio*, *teletext*, *nvod*, *nvod_timeshift*, *mosaic*, *avc_radio*, *avc_mosaic*, *data*, *mpeg2_hd*, *avc_sd_tv*, *avc_sd_nvod_timeshift*, *avc_sd_nvod*, *hd_tv*, *avc_hd_nvod_timeshift*, *avc_hd_nvod*, *uhd_tv*, *unknown*) |
| (property).lcn | number | Logical channel number |
| (property).scrambled | boolean | (Version 2) True if the service is marked as scrambled in the SDT |
| (property).hascadescriptor | boolean | (Version 2) True if there is at least one CA descriptor in the service's PMT |
| (property).hidden | boolean | (Version 2) True if the service should be hidden |
| (property).selectable | boolean | (Version 2) True if the service should be user selectable |
| (property).runningstatus | string | (Version 2) The running status as given by the service descriptor in the SDT (must be one of the following: *undefined*, *notrunning*, *startssoon*, *pausing*, *running*, *offair*) |

> The *service uri string* argument shall be passed as the index to the property, e.g. *DTV.1.serviceInfo@9018.4161.1001*.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.serviceInfo@9018.4161.1001"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "fullname": "Channel 4",
        "shortname": "C4",
        "dvburi": "2.2041.9212",
        "servicetype": "tv",
        "lcn": 1001,
        "scrambled": false,
        "hascadescriptor": false,
        "hidden": false,
        "selectable": false,
        "runningstatus": "running"
    }
}
```

<a name="serviceComponents"></a>
## *serviceComponents [<sup>property</sup>](#Properties)*

Provides access to the (Version 2) array of components for the given service defined by its URI.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | (Version 2) array of components for the given service defined by its URI |
| (property)[#] | object | Video, audio, subtitle, teletext or data component info |
| (property)[#].type | string | Type of data contained in this component (must be one of the following: *video*, *audio*, *subtitles*, *teletext*, *data*) |
| (property)[#]?.tags | array | <sup>*(optional)*</sup> Array of component tags |
| (property)[#]?.tags[#] | number | <sup>*(optional)*</sup>  |
| (property)[#].pid | number | PID of the component |
| (property)[#]?.video | object | <sup>*(optional)*</sup> Video related information for a video component |
| (property)[#]?.video.codec | string | Video or audio codec type (must be one of the following: *mpeg2*, *h264*, *h265*, *avs*, *mpeg1*, *aac*, *heaac*, *heaacv2*, *ac3*, *eac3*) |
| (property)[#]?.audio | object | <sup>*(optional)*</sup> Audio related information for an audio component |
| (property)[#]?.audio?.codec | string | <sup>*(optional)*</sup> Video or audio codec type (must be one of the following: *mpeg2*, *h264*, *h265*, *avs*, *mpeg1*, *aac*, *heaac*, *heaacv2*, *ac3*, *eac3*) |
| (property)[#]?.audio.language | string | ISO 3-char language code |
| (property)[#]?.audio.type | string | Type of the audio contained in the component (must be one of the following: *undefined*, *clean*, *hearingimpaired*, *visuallyimpaired*, *unknown*) |
| (property)[#]?.audio.mode | string | Audio mode (must be one of the following: *mono*, *left*, *right*, *stereo*, *multichannel*, *undefined*) |
| (property)[#]?.subtitles | object | <sup>*(optional)*</sup> DVB subtitle related information for a subtitle component |
| (property)[#]?.subtitles.language | string | ISO 3-char language code |
| (property)[#]?.subtitles.format | string | Intended TV format for the subtitles (must be one of the following: *default*, *4_3*, *16_9*, *221_1*, *hd*, *hardofhearing*, *hardofhearing4_3*, *hardofhearing16_9*, *hardofhearing221_1*, *hardofhearinghd*) |
| (property)[#]?.subtitles.compositionpage | number | Composition page to be used for subtitles in this component |
| (property)[#]?.subtitles.ancillarypage | number | Ancillary page to be used for subtitles in this component |
| (property)[#]?.teletext | object | <sup>*(optional)*</sup> Teletext related information for a teletext component |
| (property)[#]?.teletext.language | string | ISO 3-char language code |
| (property)[#]?.teletext.type | number | Type of teletext data |
| (property)[#]?.teletext.magazine | number | Magazine number for this type and language |
| (property)[#]?.teletext.page | number | Page number for this type and language |

> The *service uri string* argument shall be passed as the index to the property, e.g. *DTV.1.serviceComponents@9018.4161.1001*.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.serviceComponents@9018.4161.1001"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        {
            "type": "video",
            "tags": [
                0
            ],
            "pid": 1100,
            "video": {
                "codec": "mpeg2"
            },
            "audio": {
                "codec": "mpeg2",
                "language": "eng",
                "type": "normal",
                "mode": "mono"
            },
            "subtitles": {
                "language": "eng",
                "format": "default",
                "compositionpage": 0,
                "ancillarypage": 0
            },
            "teletext": {
                "language": "eng",
                "type": 0,
                "magazine": 0,
                "page": 0
            }
        }
    ]
}
```

<a name="transportInfo"></a>
## *transportInfo [<sup>property</sup>](#Properties)*

Provides access to the information for the given transport as defined by its DVB doublet URI.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Information for the given transport as defined by its DVB doublet URI |
| (property).tunertype | string | Tuner type (must be one of the following: *none*, *dvbs*, *dvbt*, *dvbc*) |
| (property).originalnetworkid | number | Original network ID of the transport |
| (property).transportid | number | Transport ID of the transport |
| (property).strength | number | Signal strength as a percentage |
| (property).quality | number | Signal quality as a percentage |
| (property)?.dvbctuningparams | object | <sup>*(optional)*</sup> Parameters required to tune to a DVB-C signal |
| (property)?.dvbctuningparams.frequency | number | Frequency of the transport, in Hz |
| (property)?.dvbctuningparams.symbolrate | number | Symbol rate in Ksymb/sec |
| (property)?.dvbctuningparams.modulation | string |  (must be one of the following: *auto*, *4qam*, *8qam*, *16qam*, *32qam*, *64qam*, *128qam*, *256qam*) |
| (property)?.dvbstuningparams | object | <sup>*(optional)*</sup> Parameters required to tune to a DVB-S/S2 signal |
| (property)?.dvbstuningparams.satellite | string | Name of the satellite settings to be used |
| (property)?.dvbstuningparams.frequency | number | Frequency of the transponder, in kHz |
| (property)?.dvbstuningparams.polarity | string |  (must be one of the following: *horizontal*, *vertical*, *left*, *right*) |
| (property)?.dvbstuningparams.symbolrate | number | Symbol rate in Ksymb/sec |
| (property)?.dvbstuningparams.fec | string | Forward error correction setting (must be one of the following: *fecauto*, *fec1_2*, *fec2_3*, *fec3_4*, *fec5_6*, *fec7_8*, *fec1_4*, *fec1_3*, *fec2_5*, *fec8_9*, *fec9_10*, *fec3_5*, *fec4_5*) |
| (property)?.dvbstuningparams.modulation | string |  (must be one of the following: *auto*, *qpsk*, *8psk*, *16qam*) |
| (property)?.dvbstuningparams.dvbs2 | boolean | True when tuning to a DVB-S2 signal |
| (property)?.dvbttuningparams | object | <sup>*(optional)*</sup> Parameters required to tune to a DVB-T/T2 signal |
| (property)?.dvbttuningparams.frequency | number | Frequency of the transport, in Hz |
| (property)?.dvbttuningparams.bandwidth | string | Channel bandwidth (must be one of the following: *5MHz*, *6MHz*, *7MHz*, *8MHz*, *10MHz*, *undefined*) |
| (property)?.dvbttuningparams.mode | string | OFDM mode (must be one of the following: *ofdm_1K*, *ofdm_2K*, *ofdm_4K*, *ofdm_8K*, *ofdm_16K*, *ofdm_32K*, *undefined*) |
| (property)?.dvbttuningparams.dvbt2 | boolean | True when tuning to a DVB-T2 signal |
| (property)?.dvbttuningparams?.plpid | number | <sup>*(optional)*</sup> PLP ID for tuning to a DVB-T2 signal |

> The *transport uri string* argument shall be passed as the index to the property, e.g. *DTV.1.transportInfo@9018.4161*.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.transportInfo@9018.4161"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "tunertype": "none",
        "originalnetworkid": 8204,
        "transportid": 8204,
        "strength": 76,
        "quality": 95,
        "dvbctuningparams": {
            "frequency": 474000000,
            "symbolrate": 6900,
            "modulation": "auto"
        },
        "dvbstuningparams": {
            "satellite": "Astra 28.2",
            "frequency": 10714000,
            "polarity": "horizontal",
            "symbolrate": 22000,
            "fec": "fecauto",
            "modulation": "auto",
            "dvbs2": false
        },
        "dvbttuningparams": {
            "frequency": 474000000,
            "bandwidth": "5MHz",
            "mode": "ofdm_1K",
            "dvbt2": false,
            "plpid": 0
        }
    }
}
```

<a name="nowNextEvents"></a>
## *nowNextEvents [<sup>property</sup>](#Properties)*

Provides access to the now and next events (EITp/f) for the given service.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Now and next events (EITp/f) for the given service |
| (property)?.now | object | <sup>*(optional)*</sup> EIT event information |
| (property)?.now.name | string | Name of the DVB event as defined in the short event descriptor |
| (property)?.now.starttime | number | UTC start time of the event in seconds |
| (property)?.now.duration | number | Duration of the event in seconds |
| (property)?.now.eventid | number | ID of the event from the event information table |
| (property)?.now.shortdescription | string | Event description from the EIT short event descriptor |
| (property)?.now.hassubtitles | boolean | (Version 2) True if subtitles are available |
| (property)?.now.hasaudiodescription | boolean | (Version 2) True if audio description is available |
| (property)?.now.parentalrating | number | (Version 2) Parental rating for the event, will be 0 if undefined |
| (property)?.now.contentdata | array | (Version 2) The first 2 4-bit values from the content descriptor as separate bytes |
| (property)?.now.contentdata[#] | number |  |
| (property)?.now.hasextendedinfo | boolean | (Version 2) True if there's extended event information available |
| (property)?.next | object | <sup>*(optional)*</sup> EIT event information |
| (property)?.next.name | string | Name of the DVB event as defined in the short event descriptor |
| (property)?.next.starttime | number | UTC start time of the event in seconds |
| (property)?.next.duration | number | Duration of the event in seconds |
| (property)?.next.eventid | number | ID of the event from the event information table |
| (property)?.next.shortdescription | string | Event description from the EIT short event descriptor |
| (property)?.next.hassubtitles | boolean | (Version 2) True if subtitles are available |
| (property)?.next.hasaudiodescription | boolean | (Version 2) True if audio description is available |
| (property)?.next.parentalrating | number | (Version 2) Parental rating for the event, will be 0 if undefined |
| (property)?.next.contentdata | array | (Version 2) The first 2 4-bit values from the content descriptor as separate bytes |
| (property)?.next.contentdata[#] | number |  |
| (property)?.next.hasextendedinfo | boolean | (Version 2) True if there's extended event information available |

> The *service uri string* argument shall be passed as the index to the property, e.g. *DTV.1.nowNextEvents@9018.4161.1001*.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.nowNextEvents@9018.4161.1001"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "now": {
            "name": "Channel 4 News",
            "starttime": 1587562065,
            "duration": 1800,
            "eventid": 3012,
            "shortdescription": "The current national and world news",
            "hassubtitles": false,
            "hasaudiodescription": false,
            "parentalrating": 12,
            "contentdata": [
                0
            ],
            "hasextendedinfo": false
        },
        "next": {
            "name": "Channel 4 News",
            "starttime": 1587562065,
            "duration": 1800,
            "eventid": 3012,
            "shortdescription": "The current national and world news",
            "hassubtitles": false,
            "hasaudiodescription": false,
            "parentalrating": 12,
            "contentdata": [
                0
            ],
            "hasextendedinfo": false
        }
    }
}
```

<a name="scheduleEvents"></a>
## *scheduleEvents [<sup>property</sup>](#Properties)*

Provides access to the events which are scheduled (EITsched) for the given service.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | Events which are scheduled (EITsched) for the given service |
| (property)[#] | object | EIT event information |
| (property)[#].name | string | Name of the DVB event as defined in the short event descriptor |
| (property)[#].starttime | number | UTC start time of the event in seconds |
| (property)[#].duration | number | Duration of the event in seconds |
| (property)[#].eventid | number | ID of the event from the event information table |
| (property)[#].shortdescription | string | Event description from the EIT short event descriptor |
| (property)[#].hassubtitles | boolean | (Version 2) True if subtitles are available |
| (property)[#].hasaudiodescription | boolean | (Version 2) True if audio description is available |
| (property)[#].parentalrating | number | (Version 2) Parental rating for the event, will be 0 if undefined |
| (property)[#].contentdata | array | (Version 2) The first 2 4-bit values from the content descriptor as separate bytes |
| (property)[#].contentdata[#] | number |  |
| (property)[#].hasextendedinfo | boolean | (Version 2) True if there's extended event information available |

> The *service uri string, with optional start and end times as number of seconds utc* argument shall be passed as the index to the property, e.g. *DTV.1.scheduleEvents@9018.4161.1001:12345000,12346000*.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.scheduleEvents@9018.4161.1001:12345000,12346000"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": [
        {
            "name": "Channel 4 News",
            "starttime": 1587562065,
            "duration": 1800,
            "eventid": 3012,
            "shortdescription": "The current national and world news",
            "hassubtitles": false,
            "hasaudiodescription": false,
            "parentalrating": 12,
            "contentdata": [
                0
            ],
            "hasextendedinfo": false
        }
    ]
}
```

<a name="extendedEventInfo"></a>
## *extendedEventInfo [<sup>property</sup>](#Properties)*

Provides access to the extended event info for the given service and event ID (version 2).

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Extended event info for the given service and event ID (version 2) |
| (property)?.description | string | <sup>*(optional)*</sup> Extended event description text |
| (property)?.items | array | <sup>*(optional)*</sup>  |
| (property)?.items[#] | object | <sup>*(optional)*</sup>  |
| (property)?.items[#]?.description | string | <sup>*(optional)*</sup> Description of the item |
| (property)?.items[#]?.item | string | <sup>*(optional)*</sup> Item text |

> The *service uri string and event id* argument shall be passed as the index to the property, e.g. *DTV.1.extendedEventInfo@9018.4161.1001:3012*.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.extendedEventInfo@9018.4161.1001:3012"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "description": "A long event description",
        "items": [
            {
                "description": "Actor 1",
                "item": "Kermit The Frog"
            }
        ]
    }
}
```

<a name="status"></a>
## *status [<sup>property</sup>](#Properties)*

Provides access to the information related to the play handle defined by the index.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | Information related to the play handle defined by the index |
| (property).tuner | number | The tuner id used by the play handle |
| (property).demux | number | The demux id used by the play handle |
| (property).pmtpid | number | The PMT PID of the service being played |
| (property).dvburi | string | DVB triplet of the form a.b.c, where 'a' is the original network ID, 'b' is the transport ID and 'c' is the service ID, in decimal |
| (property).lcn | number | LCN of the service being played |

> The *play handle* argument shall be passed as the index to the property, e.g. *DTV.1.status@0*.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.status@0"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "tuner": 0,
        "demux": 0,
        "pmtpid": 1100,
        "dvburi": "2.2041.9212",
        "lcn": 1001
    }
}
```

<a name="signalInfo"></a>
## *signalInfo [<sup>property</sup>](#Properties)*

Provides access to the strength and quality of the currently tuned signal for the given play handle (version 2).

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | object | strength and quality of the currently tuned signal for the given play handle (version 2) |
| (property).locked | boolean | Tuned locked status |
| (property).strength | number | The signal strength as a percentage |
| (property).quality | number | The signal quality as a percentage |

> The *play handle* argument shall be passed as the index to the property, e.g. *DTV.1.signalInfo@0*.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "DTV.1.signalInfo@0"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "locked": false,
        "strength": 76,
        "quality": 95
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the DTV plugin:

DTV interface events:

| Event | Description |
| :-------- | :-------- |
| [searchstatus](#searchstatus) | Triggered during the course of a service search |
| [serviceupdated](#serviceupdated) | (Version 2) Notification sent when details of a service have changed, such as its name, service type, running or scrambled state |
| [serviceadded](#serviceadded) | (Version 2) Notification sent when a new service has been dynamically added to the service list |
| [servicedeleted](#servicedeleted) | (Version 2) Notification sent when a service has been deleted from the service list |
| [videochanged](#videochanged) | (Version 2) Notification sent when the video PID or codec has changed |
| [audiochanged](#audiochanged) | (Version 2) Notification sent when the audio PID or codec has changed |
| [subtitleschanged](#subtitleschanged) | (Version 2) Notification sent when the subtitles have changed |
| [eventchanged](#eventchanged) | (Version 2) Notification sent when the EIT 'now' event is received when starting to play a service and when the 'now' event changes |


<a name="searchstatus"></a>
## *searchstatus*

Triggered during the course of a service search.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.handle | number | The handle assigned for the search and to which this information is relevant |
| params.eventtype | string | Event type that's sent as an asynchronous notification (must be one of the following: *ServiceSearchStatus*, *serviceupdated*, *serviceadded*, *servicedeleted*, *videochanged*, *audiochanged*, *subtitleschanged*, *eventchanged*) |
| params.finished | boolean | true if the service search has finished, false otherwise |
| params.progress | number | Progress of the search expressed as a percentage |
| params?.transport | object | <sup>*(optional)*</sup> Information for the given transport as defined by its DVB doublet URI |
| params?.transport.tunertype | string | Tuner type (must be one of the following: *none*, *dvbs*, *dvbt*, *dvbc*) |
| params?.transport.originalnetworkid | number | Original network ID of the transport |
| params?.transport.transportid | number | Transport ID of the transport |
| params?.transport.strength | number | Signal strength as a percentage |
| params?.transport.quality | number | Signal quality as a percentage |
| params?.transport?.dvbctuningparams | object | <sup>*(optional)*</sup> Parameters required to tune to a DVB-C signal |
| params?.transport?.dvbctuningparams.frequency | number | Frequency of the transport, in Hz |
| params?.transport?.dvbctuningparams.symbolrate | number | Symbol rate in Ksymb/sec |
| params?.transport?.dvbctuningparams.modulation | string |  (must be one of the following: *auto*, *4qam*, *8qam*, *16qam*, *32qam*, *64qam*, *128qam*, *256qam*) |
| params?.transport?.dvbstuningparams | object | <sup>*(optional)*</sup> Parameters required to tune to a DVB-S/S2 signal |
| params?.transport?.dvbstuningparams.satellite | string | Name of the satellite settings to be used |
| params?.transport?.dvbstuningparams.frequency | number | Frequency of the transponder, in kHz |
| params?.transport?.dvbstuningparams.polarity | string |  (must be one of the following: *horizontal*, *vertical*, *left*, *right*) |
| params?.transport?.dvbstuningparams.symbolrate | number | Symbol rate in Ksymb/sec |
| params?.transport?.dvbstuningparams.fec | string | Forward error correction setting (must be one of the following: *fecauto*, *fec1_2*, *fec2_3*, *fec3_4*, *fec5_6*, *fec7_8*, *fec1_4*, *fec1_3*, *fec2_5*, *fec8_9*, *fec9_10*, *fec3_5*, *fec4_5*) |
| params?.transport?.dvbstuningparams.modulation | string |  (must be one of the following: *auto*, *qpsk*, *8psk*, *16qam*) |
| params?.transport?.dvbstuningparams.dvbs2 | boolean | True when tuning to a DVB-S2 signal |
| params?.transport?.dvbttuningparams | object | <sup>*(optional)*</sup> Parameters required to tune to a DVB-T/T2 signal |
| params?.transport?.dvbttuningparams.frequency | number | Frequency of the transport, in Hz |
| params?.transport?.dvbttuningparams.bandwidth | string | Channel bandwidth (must be one of the following: *5MHz*, *6MHz*, *7MHz*, *8MHz*, *10MHz*, *undefined*) |
| params?.transport?.dvbttuningparams.mode | string | OFDM mode (must be one of the following: *ofdm_1K*, *ofdm_2K*, *ofdm_4K*, *ofdm_8K*, *ofdm_16K*, *ofdm_32K*, *undefined*) |
| params?.transport?.dvbttuningparams.dvbt2 | boolean | True when tuning to a DVB-T2 signal |
| params?.transport?.dvbttuningparams?.plpid | number | <sup>*(optional)*</sup> PLP ID for tuning to a DVB-T2 signal |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.searchstatus",
    "params": {
        "handle": 0,
        "eventtype": "ServiceSearchStatus",
        "finished": false,
        "progress": 65,
        "transport": {
            "tunertype": "none",
            "originalnetworkid": 8204,
            "transportid": 8204,
            "strength": 76,
            "quality": 95,
            "dvbctuningparams": {
                "frequency": 474000000,
                "symbolrate": 6900,
                "modulation": "auto"
            },
            "dvbstuningparams": {
                "satellite": "Astra 28.2",
                "frequency": 10714000,
                "polarity": "horizontal",
                "symbolrate": 22000,
                "fec": "fecauto",
                "modulation": "auto",
                "dvbs2": false
            },
            "dvbttuningparams": {
                "frequency": 474000000,
                "bandwidth": "5MHz",
                "mode": "ofdm_1K",
                "dvbt2": false,
                "plpid": 0
            }
        }
    }
}
```

<a name="serviceupdated"></a>
## *serviceupdated*

(Version 2) Notification sent when details of a service have changed, such as its name, service type, running or scrambled state.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.eventtype | string | Event type that's sent as an asynchronous notification (must be one of the following: *ServiceSearchStatus*, *serviceupdated*, *serviceadded*, *servicedeleted*, *videochanged*, *audiochanged*, *subtitleschanged*, *eventchanged*) |
| params.service | object | Information for the given service as defined by its DVB triplet URI |
| params.service.fullname | string | (Version 2) Full service name as given by the service descriptor in the SDT |
| params.service.shortname | string | Short service name as given by the service descriptor in the SDT |
| params.service.dvburi | string | DVB triplet of the form a.b.c, where 'a' is the original network ID, 'b' is the transport ID and 'c' is the service ID, in decimal |
| params.service.servicetype | string | (Version 2) Service type as given by the service descriptor in the SDT (must be one of the following: *tv*, *radio*, *teletext*, *nvod*, *nvod_timeshift*, *mosaic*, *avc_radio*, *avc_mosaic*, *data*, *mpeg2_hd*, *avc_sd_tv*, *avc_sd_nvod_timeshift*, *avc_sd_nvod*, *hd_tv*, *avc_hd_nvod_timeshift*, *avc_hd_nvod*, *uhd_tv*, *unknown*) |
| params.service.lcn | number | Logical channel number |
| params.service.scrambled | boolean | (Version 2) True if the service is marked as scrambled in the SDT |
| params.service.hascadescriptor | boolean | (Version 2) True if there is at least one CA descriptor in the service's PMT |
| params.service.hidden | boolean | (Version 2) True if the service should be hidden |
| params.service.selectable | boolean | (Version 2) True if the service should be user selectable |
| params.service.runningstatus | string | (Version 2) The running status as given by the service descriptor in the SDT (must be one of the following: *undefined*, *notrunning*, *startssoon*, *pausing*, *running*, *offair*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.serviceupdated",
    "params": {
        "eventtype": "ServiceSearchStatus",
        "service": {
            "fullname": "Channel 4",
            "shortname": "C4",
            "dvburi": "2.2041.9212",
            "servicetype": "tv",
            "lcn": 1001,
            "scrambled": false,
            "hascadescriptor": false,
            "hidden": false,
            "selectable": false,
            "runningstatus": "running"
        }
    }
}
```

<a name="serviceadded"></a>
## *serviceadded*

(Version 2) Notification sent when a new service has been dynamically added to the service list.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.eventtype | string | Event type that's sent as an asynchronous notification (must be one of the following: *ServiceSearchStatus*, *serviceupdated*, *serviceadded*, *servicedeleted*, *videochanged*, *audiochanged*, *subtitleschanged*, *eventchanged*) |
| params.service | object | Information for the given service as defined by its DVB triplet URI |
| params.service.fullname | string | (Version 2) Full service name as given by the service descriptor in the SDT |
| params.service.shortname | string | Short service name as given by the service descriptor in the SDT |
| params.service.dvburi | string | DVB triplet of the form a.b.c, where 'a' is the original network ID, 'b' is the transport ID and 'c' is the service ID, in decimal |
| params.service.servicetype | string | (Version 2) Service type as given by the service descriptor in the SDT (must be one of the following: *tv*, *radio*, *teletext*, *nvod*, *nvod_timeshift*, *mosaic*, *avc_radio*, *avc_mosaic*, *data*, *mpeg2_hd*, *avc_sd_tv*, *avc_sd_nvod_timeshift*, *avc_sd_nvod*, *hd_tv*, *avc_hd_nvod_timeshift*, *avc_hd_nvod*, *uhd_tv*, *unknown*) |
| params.service.lcn | number | Logical channel number |
| params.service.scrambled | boolean | (Version 2) True if the service is marked as scrambled in the SDT |
| params.service.hascadescriptor | boolean | (Version 2) True if there is at least one CA descriptor in the service's PMT |
| params.service.hidden | boolean | (Version 2) True if the service should be hidden |
| params.service.selectable | boolean | (Version 2) True if the service should be user selectable |
| params.service.runningstatus | string | (Version 2) The running status as given by the service descriptor in the SDT (must be one of the following: *undefined*, *notrunning*, *startssoon*, *pausing*, *running*, *offair*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.serviceadded",
    "params": {
        "eventtype": "ServiceSearchStatus",
        "service": {
            "fullname": "Channel 4",
            "shortname": "C4",
            "dvburi": "2.2041.9212",
            "servicetype": "tv",
            "lcn": 1001,
            "scrambled": false,
            "hascadescriptor": false,
            "hidden": false,
            "selectable": false,
            "runningstatus": "running"
        }
    }
}
```

<a name="servicedeleted"></a>
## *servicedeleted*

(Version 2) Notification sent when a service has been deleted from the service list.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.eventtype | string | Event type that's sent as an asynchronous notification (must be one of the following: *ServiceSearchStatus*, *serviceupdated*, *serviceadded*, *servicedeleted*, *videochanged*, *audiochanged*, *subtitleschanged*, *eventchanged*) |
| params.service | object | Information for the given service as defined by its DVB triplet URI |
| params.service.fullname | string | (Version 2) Full service name as given by the service descriptor in the SDT |
| params.service.shortname | string | Short service name as given by the service descriptor in the SDT |
| params.service.dvburi | string | DVB triplet of the form a.b.c, where 'a' is the original network ID, 'b' is the transport ID and 'c' is the service ID, in decimal |
| params.service.servicetype | string | (Version 2) Service type as given by the service descriptor in the SDT (must be one of the following: *tv*, *radio*, *teletext*, *nvod*, *nvod_timeshift*, *mosaic*, *avc_radio*, *avc_mosaic*, *data*, *mpeg2_hd*, *avc_sd_tv*, *avc_sd_nvod_timeshift*, *avc_sd_nvod*, *hd_tv*, *avc_hd_nvod_timeshift*, *avc_hd_nvod*, *uhd_tv*, *unknown*) |
| params.service.lcn | number | Logical channel number |
| params.service.scrambled | boolean | (Version 2) True if the service is marked as scrambled in the SDT |
| params.service.hascadescriptor | boolean | (Version 2) True if there is at least one CA descriptor in the service's PMT |
| params.service.hidden | boolean | (Version 2) True if the service should be hidden |
| params.service.selectable | boolean | (Version 2) True if the service should be user selectable |
| params.service.runningstatus | string | (Version 2) The running status as given by the service descriptor in the SDT (must be one of the following: *undefined*, *notrunning*, *startssoon*, *pausing*, *running*, *offair*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.servicedeleted",
    "params": {
        "eventtype": "ServiceSearchStatus",
        "service": {
            "fullname": "Channel 4",
            "shortname": "C4",
            "dvburi": "2.2041.9212",
            "servicetype": "tv",
            "lcn": 1001,
            "scrambled": false,
            "hascadescriptor": false,
            "hidden": false,
            "selectable": false,
            "runningstatus": "running"
        }
    }
}
```

<a name="videochanged"></a>
## *videochanged*

(Version 2) Notification sent when the video PID or codec has changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.eventtype | string | Event type that's sent as an asynchronous notification (must be one of the following: *ServiceSearchStatus*, *serviceupdated*, *serviceadded*, *servicedeleted*, *videochanged*, *audiochanged*, *subtitleschanged*, *eventchanged*) |
| params.service | object | Information for the given service as defined by its DVB triplet URI |
| params.service.fullname | string | (Version 2) Full service name as given by the service descriptor in the SDT |
| params.service.shortname | string | Short service name as given by the service descriptor in the SDT |
| params.service.dvburi | string | DVB triplet of the form a.b.c, where 'a' is the original network ID, 'b' is the transport ID and 'c' is the service ID, in decimal |
| params.service.servicetype | string | (Version 2) Service type as given by the service descriptor in the SDT (must be one of the following: *tv*, *radio*, *teletext*, *nvod*, *nvod_timeshift*, *mosaic*, *avc_radio*, *avc_mosaic*, *data*, *mpeg2_hd*, *avc_sd_tv*, *avc_sd_nvod_timeshift*, *avc_sd_nvod*, *hd_tv*, *avc_hd_nvod_timeshift*, *avc_hd_nvod*, *uhd_tv*, *unknown*) |
| params.service.lcn | number | Logical channel number |
| params.service.scrambled | boolean | (Version 2) True if the service is marked as scrambled in the SDT |
| params.service.hascadescriptor | boolean | (Version 2) True if there is at least one CA descriptor in the service's PMT |
| params.service.hidden | boolean | (Version 2) True if the service should be hidden |
| params.service.selectable | boolean | (Version 2) True if the service should be user selectable |
| params.service.runningstatus | string | (Version 2) The running status as given by the service descriptor in the SDT (must be one of the following: *undefined*, *notrunning*, *startssoon*, *pausing*, *running*, *offair*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.videochanged",
    "params": {
        "eventtype": "ServiceSearchStatus",
        "service": {
            "fullname": "Channel 4",
            "shortname": "C4",
            "dvburi": "2.2041.9212",
            "servicetype": "tv",
            "lcn": 1001,
            "scrambled": false,
            "hascadescriptor": false,
            "hidden": false,
            "selectable": false,
            "runningstatus": "running"
        }
    }
}
```

<a name="audiochanged"></a>
## *audiochanged*

(Version 2) Notification sent when the audio PID or codec has changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.eventtype | string | Event type that's sent as an asynchronous notification (must be one of the following: *ServiceSearchStatus*, *serviceupdated*, *serviceadded*, *servicedeleted*, *videochanged*, *audiochanged*, *subtitleschanged*, *eventchanged*) |
| params.service | object | Information for the given service as defined by its DVB triplet URI |
| params.service.fullname | string | (Version 2) Full service name as given by the service descriptor in the SDT |
| params.service.shortname | string | Short service name as given by the service descriptor in the SDT |
| params.service.dvburi | string | DVB triplet of the form a.b.c, where 'a' is the original network ID, 'b' is the transport ID and 'c' is the service ID, in decimal |
| params.service.servicetype | string | (Version 2) Service type as given by the service descriptor in the SDT (must be one of the following: *tv*, *radio*, *teletext*, *nvod*, *nvod_timeshift*, *mosaic*, *avc_radio*, *avc_mosaic*, *data*, *mpeg2_hd*, *avc_sd_tv*, *avc_sd_nvod_timeshift*, *avc_sd_nvod*, *hd_tv*, *avc_hd_nvod_timeshift*, *avc_hd_nvod*, *uhd_tv*, *unknown*) |
| params.service.lcn | number | Logical channel number |
| params.service.scrambled | boolean | (Version 2) True if the service is marked as scrambled in the SDT |
| params.service.hascadescriptor | boolean | (Version 2) True if there is at least one CA descriptor in the service's PMT |
| params.service.hidden | boolean | (Version 2) True if the service should be hidden |
| params.service.selectable | boolean | (Version 2) True if the service should be user selectable |
| params.service.runningstatus | string | (Version 2) The running status as given by the service descriptor in the SDT (must be one of the following: *undefined*, *notrunning*, *startssoon*, *pausing*, *running*, *offair*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.audiochanged",
    "params": {
        "eventtype": "ServiceSearchStatus",
        "service": {
            "fullname": "Channel 4",
            "shortname": "C4",
            "dvburi": "2.2041.9212",
            "servicetype": "tv",
            "lcn": 1001,
            "scrambled": false,
            "hascadescriptor": false,
            "hidden": false,
            "selectable": false,
            "runningstatus": "running"
        }
    }
}
```

<a name="subtitleschanged"></a>
## *subtitleschanged*

(Version 2) Notification sent when the subtitles have changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.eventtype | string | Event type that's sent as an asynchronous notification (must be one of the following: *ServiceSearchStatus*, *serviceupdated*, *serviceadded*, *servicedeleted*, *videochanged*, *audiochanged*, *subtitleschanged*, *eventchanged*) |
| params.service | object | Information for the given service as defined by its DVB triplet URI |
| params.service.fullname | string | (Version 2) Full service name as given by the service descriptor in the SDT |
| params.service.shortname | string | Short service name as given by the service descriptor in the SDT |
| params.service.dvburi | string | DVB triplet of the form a.b.c, where 'a' is the original network ID, 'b' is the transport ID and 'c' is the service ID, in decimal |
| params.service.servicetype | string | (Version 2) Service type as given by the service descriptor in the SDT (must be one of the following: *tv*, *radio*, *teletext*, *nvod*, *nvod_timeshift*, *mosaic*, *avc_radio*, *avc_mosaic*, *data*, *mpeg2_hd*, *avc_sd_tv*, *avc_sd_nvod_timeshift*, *avc_sd_nvod*, *hd_tv*, *avc_hd_nvod_timeshift*, *avc_hd_nvod*, *uhd_tv*, *unknown*) |
| params.service.lcn | number | Logical channel number |
| params.service.scrambled | boolean | (Version 2) True if the service is marked as scrambled in the SDT |
| params.service.hascadescriptor | boolean | (Version 2) True if there is at least one CA descriptor in the service's PMT |
| params.service.hidden | boolean | (Version 2) True if the service should be hidden |
| params.service.selectable | boolean | (Version 2) True if the service should be user selectable |
| params.service.runningstatus | string | (Version 2) The running status as given by the service descriptor in the SDT (must be one of the following: *undefined*, *notrunning*, *startssoon*, *pausing*, *running*, *offair*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.subtitleschanged",
    "params": {
        "eventtype": "ServiceSearchStatus",
        "service": {
            "fullname": "Channel 4",
            "shortname": "C4",
            "dvburi": "2.2041.9212",
            "servicetype": "tv",
            "lcn": 1001,
            "scrambled": false,
            "hascadescriptor": false,
            "hidden": false,
            "selectable": false,
            "runningstatus": "running"
        }
    }
}
```

<a name="eventchanged"></a>
## *eventchanged*

(Version 2) Notification sent when the EIT 'now' event is received when starting to play a service and when the 'now' event changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.eventtype | string | Event type that's sent as an asynchronous notification (must be one of the following: *ServiceSearchStatus*, *serviceupdated*, *serviceadded*, *servicedeleted*, *videochanged*, *audiochanged*, *subtitleschanged*, *eventchanged*) |
| params.service | object | Information for the given service as defined by its DVB triplet URI |
| params.service.fullname | string | (Version 2) Full service name as given by the service descriptor in the SDT |
| params.service.shortname | string | Short service name as given by the service descriptor in the SDT |
| params.service.dvburi | string | DVB triplet of the form a.b.c, where 'a' is the original network ID, 'b' is the transport ID and 'c' is the service ID, in decimal |
| params.service.servicetype | string | (Version 2) Service type as given by the service descriptor in the SDT (must be one of the following: *tv*, *radio*, *teletext*, *nvod*, *nvod_timeshift*, *mosaic*, *avc_radio*, *avc_mosaic*, *data*, *mpeg2_hd*, *avc_sd_tv*, *avc_sd_nvod_timeshift*, *avc_sd_nvod*, *hd_tv*, *avc_hd_nvod_timeshift*, *avc_hd_nvod*, *uhd_tv*, *unknown*) |
| params.service.lcn | number | Logical channel number |
| params.service.scrambled | boolean | (Version 2) True if the service is marked as scrambled in the SDT |
| params.service.hascadescriptor | boolean | (Version 2) True if there is at least one CA descriptor in the service's PMT |
| params.service.hidden | boolean | (Version 2) True if the service should be hidden |
| params.service.selectable | boolean | (Version 2) True if the service should be user selectable |
| params.service.runningstatus | string | (Version 2) The running status as given by the service descriptor in the SDT (must be one of the following: *undefined*, *notrunning*, *startssoon*, *pausing*, *running*, *offair*) |
| params.event | object | EIT event information |
| params.name | string | Name of the DVB event as defined in the short event descriptor |
| params.starttime | number | UTC start time of the event in seconds |
| params.duration | number | Duration of the event in seconds |
| params.eventid | number | ID of the event from the event information table |
| params.shortdescription | string | Event description from the EIT short event descriptor |
| params.hassubtitles | boolean | (Version 2) True if subtitles are available |
| params.hasaudiodescription | boolean | (Version 2) True if audio description is available |
| params.parentalrating | number | (Version 2) Parental rating for the event, will be 0 if undefined |
| params.contentdata | array | (Version 2) The first 2 4-bit values from the content descriptor as separate bytes |
| params.contentdata[#] | number |  |
| params.hasextendedinfo | boolean | (Version 2) True if there's extended event information available |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.eventchanged",
    "params": {
        "eventtype": "ServiceSearchStatus",
        "service": {
            "fullname": "Channel 4",
            "shortname": "C4",
            "dvburi": "2.2041.9212",
            "servicetype": "tv",
            "lcn": 1001,
            "scrambled": false,
            "hascadescriptor": false,
            "hidden": false,
            "selectable": false,
            "runningstatus": "running"
        },
        "event": {
            "name": "Channel 4 News",
            "starttime": 1587562065,
            "duration": 1800,
            "eventid": 3012,
            "shortdescription": "The current national and world news",
            "hassubtitles": false,
            "hasaudiodescription": false,
            "parentalrating": 12,
            "contentdata": [
                0
            ],
            "hasextendedinfo": false
        }
    }
}
```

