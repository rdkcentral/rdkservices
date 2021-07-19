<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.DTV_Plugin"></a>
# DTV Plugin

**Version: 1.0**

**Status: :black_circle::white_circle::white_circle:**

DTV plugin for Thunder framework.

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

This document describes purpose and functionality of the DTV plugin. It includes detailed specification about its configuration, methods and properties provided, as well as notifications sent.

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

The DTV plugin provides access to DVB services provided by a DVB stack.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *DTV*) |
| classname | string | Class name: *DTV* |
| locator | string | Library name: *libWPEFrameworkDTV.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |
| configuration | object | <sup>*(optional)*</sup>  |
| configuration?.subtitleprocessing | boolean | <sup>*(optional)*</sup> Enable subtitle processing |
| configuration?.teletextproessing | boolean | <sup>*(optional)*</sup> Enable teletext processing |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the DTV plugin:

DTV interface methods:

| Method | Description |
| :-------- | :-------- |
| [addLnb](#method.addLnb) | Add a new LNB to the database |
| [addSatellite](#method.addSatellite) | Add a new satellite to the database |
| [finishServiceSearch](#method.finishServiceSearch) | Finishes a service search |
| [startPlaying](#method.startPlaying) | Starts playing the specified service |
| [startServiceSearch](#method.startServiceSearch) | Starts a service search |
| [stopPlaying](#method.stopPlaying) | Stops playing the specified service |


<a name="method.addLnb"></a>
## *addLnb <sup>method</sup>*

Add a new LNB to the database.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.name | string |  |
| params.type | string |  (must be one of the following: *single*, *universal*, *unicable*) |
| params.power | string |  (must be one of the following: *off*, *on*, *auto*) |
| params.diseqc_tone | string |  (must be one of the following: *off*, *a*, *b*) |
| params.diseqc_cswitch | string |  (must be one of the following: *off*, *a*, *b*, *c*, *d*) |
| params.is22k | boolean |  |
| params.is12v | boolean |  |
| params.ispulseposition | boolean |  |
| params.isdiseqcposition | boolean |  |
| params.issmatv | boolean |  |
| params.diseqcrepeats | number |  |
| params.u_switch | number |  |
| params.unicablechannel | number |  |
| params.unicableinterface | number |  |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean | true if the LNB is added, false otherwise |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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
        "unicableinterface": 0
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": true
}
```

<a name="method.addSatellite"></a>
## *addSatellite <sup>method</sup>*

Add a new satellite to the database.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.name | string |  |
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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": true
}
```

<a name="method.finishServiceSearch"></a>
## *finishServiceSearch <sup>method</sup>*

Finishes a service search.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.tunertype | string |  (must be one of the following: *none*, *dvbs*, *dvbt*, *dvbc*) |
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
    "id": 1234567890,
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
    "id": 1234567890,
    "result": true
}
```

<a name="method.startPlaying"></a>
## *startPlaying <sup>method</sup>*

Starts playing the specified service.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params?.dvburi | string | <sup>*(optional)*</sup> DVB triplet of the form a.b.c, where 'a' is the original network ID, 'b' is the transport ID and 'c' is the service ID, in decimal |
| params?.lcn | number | <sup>*(optional)*</sup> LCN of the service to be tuned to |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | number | The play handle assigned to play the given service. Will be -1 if the service can't be played |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DTV.1.startPlaying",
    "params": {
        "dvburi": "2.2041.9212",
        "lcn": 0
    }
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

<a name="method.startServiceSearch"></a>
## *startServiceSearch <sup>method</sup>*

Starts a service search.

Also see: [searchstatus](#event.searchstatus)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.tunertype | string |  (must be one of the following: *none*, *dvbs*, *dvbt*, *dvbc*) |
| params.searchtype | string |  (must be one of the following: *frequency*, *network*) |
| params.retune | boolean | true if current services are to be replaced in the database, false if the scan is to update the existing services |
| params.usetuningparams | boolean | Set to true if the optional tuning parameters are defined |
| params?.dvbstuningparams | object | <sup>*(optional)*</sup>  |
| params?.dvbstuningparams.satellite | string |  |
| params?.dvbstuningparams.frequency | number |  |
| params?.dvbstuningparams.polarity | string |  (must be one of the following: *horizontal*, *vertical*, *left*, *right*) |
| params?.dvbstuningparams.symbolrate | number |  |
| params?.dvbstuningparams.fec | string | Forward error correction setting (must be one of the following: *fecauto*, *fec1_2*, *fec2_3*, *fec3_4*, *fec5_6*, *fec7_8*, *fec1_4*, *fec1_3*, *fec2_5*, *fec8_9*, *fec9_10*) |
| params?.dvbstuningparams.modulation | string |  (must be one of the following: *auto*, *qpsk*, *8psk*, *16qam*) |
| params?.dvbstuningparams.dvbs2 | boolean |  |
| params?.dvbctuningparams | object | <sup>*(optional)*</sup>  |
| params?.dvbctuningparams.frequency | number |  |
| params?.dvbctuningparams.symbolrate | number |  |
| params?.dvbctuningparams.modulation | string |  (must be one of the following: *auto*, *64qam*, *128qam*, *256qam*) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | boolean | true if the search is started, false otherwise |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DTV.1.startServiceSearch",
    "params": {
        "tunertype": "none",
        "searchtype": "frequency",
        "retune": false,
        "usetuningparams": true,
        "dvbstuningparams": {
            "satellite": "",
            "frequency": 10714,
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
        }
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": true
}
```

<a name="method.stopPlaying"></a>
## *stopPlaying <sup>method</sup>*

Stops playing the specified service.

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
    "id": 1234567890,
    "method": "DTV.1.stopPlaying",
    "params": 0
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": null
}
```

<a name="head.Properties"></a>
# Properties

The following properties are provided by the DTV plugin:

DTV interface properties:

| Property | Description |
| :-------- | :-------- |
| [numberOfCountries](#property.numberOfCountries) <sup>RO</sup> | Number of country configurations available |
| [countryList](#property.countryList) <sup>RO</sup> | Array containing the name and 3 character ISO country code for all the available country configurations |
| [country](#property.country) | Country configuration using the ISO 3-character country code |
| [lnbList](#property.lnbList) <sup>RO</sup> | Array of LNBs defined in the database |
| [satelliteList](#property.satelliteList) <sup>RO</sup> | Array of satellites defined in the database |
| [numberOfServices](#property.numberOfServices) <sup>RO</sup> | Total number of services in the service database |
| [serviceList](#property.serviceList) <sup>RO</sup> | List of services for the given type of tuner or all services if no tuner is defined |
| [nowNextEvents](#property.nowNextEvents) <sup>RO</sup> | Now and next events (EITp/f) for the given service |
| [scheduleEvents](#property.scheduleEvents) <sup>RO</sup> | Schedule events (EITsched) for the given service |
| [status](#property.status) <sup>RO</sup> | Information related to the play handle defined by the index |


<a name="property.numberOfCountries"></a>
## *numberOfCountries <sup>property</sup>*

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
    "id": 1234567890,
    "method": "DTV.1.numberOfCountries"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": 5
}
```

<a name="property.countryList"></a>
## *countryList <sup>property</sup>*

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
    "id": 1234567890,
    "method": "DTV.1.countryList"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": [
        {
            "name": "UK",
            "code": 6775410
        }
    ]
}
```

<a name="property.country"></a>
## *country <sup>property</sup>*

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
    "id": 1234567890,
    "method": "DTV.1.country"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": 6775410
}
```

#### Set Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DTV.1.country",
    "params": 6775410
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

<a name="property.lnbList"></a>
## *lnbList <sup>property</sup>*

Provides access to the array of LNBs defined in the database.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | Array of LNBs defined in the database |
| (property)[#] | object |  |
| (property)[#].name | string |  |
| (property)[#].type | string |  (must be one of the following: *single*, *universal*, *unicable*) |
| (property)[#].power | string |  (must be one of the following: *off*, *on*, *auto*) |
| (property)[#].diseqc_tone | string |  (must be one of the following: *off*, *a*, *b*) |
| (property)[#].diseqc_cswitch | string |  (must be one of the following: *off*, *a*, *b*, *c*, *d*) |
| (property)[#].is22k | boolean |  |
| (property)[#].is12v | boolean |  |
| (property)[#].ispulseposition | boolean |  |
| (property)[#].isdiseqcposition | boolean |  |
| (property)[#].issmatv | boolean |  |
| (property)[#].diseqcrepeats | number |  |
| (property)[#].u_switch | number |  |
| (property)[#].unicablechannel | number |  |
| (property)[#].unicableinterface | number |  |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DTV.1.lnbList"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
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
            "unicableinterface": 0
        }
    ]
}
```

<a name="property.satelliteList"></a>
## *satelliteList <sup>property</sup>*

Provides access to the array of satellites defined in the database.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | Array of satellites defined in the database |
| (property)[#] | object |  |
| (property)[#].name | string |  |
| (property)[#].longitude | number | Longitudinal location of the satellite in 1/10ths of a degree, with an east coordinate given as a positive value and a west coordinate as negative. Astra 28.2E would be defined as 282 and Eutelsat 5.0W would be -50 |
| (property)[#].lnb | string | Name of the LNB settings to be used when tuning to this satellite |

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DTV.1.satelliteList"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": [
        {
            "name": "Astra 28.2E",
            "longitude": 282,
            "lnb": "Universal"
        }
    ]
}
```

<a name="property.numberOfServices"></a>
## *numberOfServices <sup>property</sup>*

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
    "id": 1234567890,
    "method": "DTV.1.numberOfServices"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": 145
}
```

<a name="property.serviceList"></a>
## *serviceList <sup>property</sup>*

Provides access to the list of services for the given type of tuner or all services if no tuner is defined.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | List of services for the given type of tuner or all services if no tuner is defined |
| (property)[#] | object |  |
| (property)[#].shortname | string | Service name as given by the service descriptor in the SDT |
| (property)[#].dvburi | string | DVB triplet of the form a.b.c, where 'a' is the original network ID, 'b' is the transport ID and 'c' is the service ID, in decimal |
| (property)[#].lcn | number | Logical channel number |

> The *optional tuner type* shall be passed as the index to the property, e.g. *DTV.1.serviceList@dvbs*.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DTV.1.serviceList@dvbs"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": [
        {
            "shortname": "Channel 4",
            "dvburi": "2.2041.9212",
            "lcn": 1001
        }
    ]
}
```

<a name="property.nowNextEvents"></a>
## *nowNextEvents <sup>property</sup>*

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
| (property)?.next | object | <sup>*(optional)*</sup> EIT event information |
| (property)?.next.name | string | Name of the DVB event as defined in the short event descriptor |
| (property)?.next.starttime | number | UTC start time of the event in seconds |
| (property)?.next.duration | number | Duration of the event in seconds |
| (property)?.next.eventid | number | ID of the event from the event information table |
| (property)?.next.shortdescription | string | Event description from the EIT short event descriptor |

> The *service uri string* shall be passed as the index to the property, e.g. *DTV.1.nowNextEvents@9018.4161.1001*.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DTV.1.nowNextEvents@9018.4161.1001"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "now": {
            "name": "Channel 4 News",
            "starttime": 1587562065,
            "duration": 1800,
            "eventid": 3012,
            "shortdescription": "The current national and world news"
        },
        "next": {
            "name": "Channel 4 News",
            "starttime": 1587562065,
            "duration": 1800,
            "eventid": 3012,
            "shortdescription": "The current national and world news"
        }
    }
}
```

<a name="property.scheduleEvents"></a>
## *scheduleEvents <sup>property</sup>*

Provides access to the schedule events (EITsched) for the given service.

> This property is **read-only**.

### Value

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| (property) | array | Schedule events (EITsched) for the given service |
| (property)[#] | object | EIT event information |
| (property)[#].name | string | Name of the DVB event as defined in the short event descriptor |
| (property)[#].starttime | number | UTC start time of the event in seconds |
| (property)[#].duration | number | Duration of the event in seconds |
| (property)[#].eventid | number | ID of the event from the event information table |
| (property)[#].shortdescription | string | Event description from the EIT short event descriptor |

> The *service uri string, with optional start and end times as number of seconds utc* shall be passed as the index to the property, e.g. *DTV.1.scheduleEvents@9018.4161.1001:12345000,12346000*.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DTV.1.scheduleEvents@9018.4161.1001:12345000,12346000"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": [
        {
            "name": "Channel 4 News",
            "starttime": 1587562065,
            "duration": 1800,
            "eventid": 3012,
            "shortdescription": "The current national and world news"
        }
    ]
}
```

<a name="property.status"></a>
## *status <sup>property</sup>*

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

> The *play handle* shall be passed as the index to the property, e.g. *DTV.1.status@0*.

### Example

#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DTV.1.status@0"
}
```

#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "tuner": 0,
        "demux": 0,
        "pmtpid": 1100,
        "dvburi": "2.2041.9212",
        "lcn": 1001
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the DTV plugin:

DTV interface events:

| Event | Description |
| :-------- | :-------- |
| [searchstatus](#event.searchstatus) | Triggered during the course of a service search |


<a name="event.searchstatus"></a>
## *searchstatus <sup>event</sup>*

Triggered during the course of a service search.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.handle | number | The handle assigned for the search and to which this information is relevant |
| params.eventtype | string | Event type that's sent as an asynchronous notification (must be one of the following: *ServiceSearchStatus*) |
| params.finished | boolean | true if the service search has finished, false otherwise |
| params.progress | number | Progress of the search expressed as a percentage |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.searchstatus",
    "params": {
        "handle": 0,
        "eventtype": "ServiceSearchStatus",
        "finished": false,
        "progress": 65
    }
}
```

