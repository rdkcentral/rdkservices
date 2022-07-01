<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.Unified_CAS_Management_Service"></a>
# Unified CAS Management Service

**Version: 1.0**

**Status: :black_circle::white_circle::white_circle:**

UnifiedCASManagement interface for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Methods](#head.Methods)
- [Notifications](#head.Notifications)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the UnifiedCASManagement interface. It includes detailed specification about its methods provided and notifications sent.

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

Simple service to allow the management of OCDM CAS.

<a name="head.Methods"></a>
# Methods

The following methods are provided by the UnifiedCASManagement interface:

UnifiedCASManagement interface methods:

| Method | Description |
| :-------- | :-------- |
| [manage](#method.manage) | Manage a well-known CAS |
| [unmanage](#method.unmanage) | Destroy a management session |
| [send](#method.send) | Sends data to the remote CAS |


<a name="method.manage"></a>
## *manage <sup>method</sup>*

Manage a well-known CAS.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Specifies how to manage a CAS |
| params?.mediaurl | string | <sup>*(optional)*</sup> The URL to tune to can be tune://, ocap:// http:// https:// |
| params?.mode | string | <sup>*(optional)*</sup> The use of the tune request (must be one of the following: *MODE_NONE*, *MODE_LIVE*, *MODE_RECORD*, *MODE_PLAYBACK*) |
| params?.manage | string | <sup>*(optional)*</sup> The type of CAS management to attach to the tune (must be one of the following: *MANAGE_NONE*, *MANAGE_FULL*, *MANAGE_NO_PSI*, *MANAGE_NO_TUNER*) |
| params?.casinitdata | string | <sup>*(optional)*</sup> CAS specific initdata for the selected media |
| params.casocdmid | string | The well-known OCDM ID of the CAS to use |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedCASManagement.1.manage",
    "params": {
        "mediaurl": "tune://tuner?frequency=175000000&modulation=16&pgmno=12",
        "mode": "MODE_LIVE",
        "manage": "MANAGE_NONE",
        "casinitdata": "<base64 data>",
        "casocdmid": ""
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.unmanage"></a>
## *unmanage <sup>method</sup>*

Destroy a management session.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | string | The well-known OCDM ID of the CAS to use |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedCASManagement.1.unmanage",
    "params": ""
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="method.send"></a>
## *send <sup>method</sup>*

Sends data to the remote CAS.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Object transfer data to/from the remote CAS. The actual payload is Client/CAS specific |
| params.payload | string | Data to transfer. Can be base64 coded if required |
| params?.source | string | <sup>*(optional)*</sup> Origin of the data. (must be one of the following: *PUBLIC*, *PRIVATE*) |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object | Generic Result Object |
| result.success | boolean | Returning whether this method failed or succeed |
| result?.failurereason | number | <sup>*(optional)*</sup> Reason why it's failed |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "UnifiedCASManagement.1.send",
    "params": {
        "payload": "",
        "source": "PUBLIC"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true,
        "failurereason": 0
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the UnifiedCASManagement interface:

UnifiedCASManagement interface events:

| Event | Description |
| :-------- | :-------- |
| [data](#event.data) | Sent when the CAS needs to send data to the caller |


<a name="event.data"></a>
## *data <sup>event</sup>*

Sent when the CAS needs to send data to the caller.

### Description

Register for this event to be sent private data from the CAS

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | Object transfer data to/from the remote CAS. The actual payload is Client/CAS specific |
| params.payload | string | Data to transfer. Can be base64 coded if required |
| params?.source | string | <sup>*(optional)*</sup> Origin of the data. (must be one of the following: *PUBLIC*, *PRIVATE*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.data",
    "params": {
        "payload": "",
        "source": "PUBLIC"
    }
}
```

