<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.HdcpProfile_Plugin"></a>
# HdcpProfile Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.HdcpProfile plugin for Thunder framework.

### Table of Contents

- [Introduction](#head.Introduction)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Methods](#head.Methods)
- [Notifications](#head.Notifications)

<a name="head.Introduction"></a>
# Introduction

<a name="head.Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.HdcpProfile plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The HdcpProfile plugin provides an interface for HDCP-related data and events.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.HdcpProfile*) |
| classname | string | Class name: *org.rdk.HdcpProfile* |
| locator | string | Library name: *libWPEFrameworkHdcpProfile.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.HdcpProfile plugin:

HdcpProfile interface methods:

| Method | Description |
| :-------- | :-------- |
| [getHDCPStatus](#method.getHDCPStatus) | Returns HDCP-related data |
| [getSettopHDCPSupport](#method.getSettopHDCPSupport) | Returns which version of HDCP is supported by the STB |


<a name="method.getHDCPStatus"></a>
## *getHDCPStatus [<sup>method</sup>](#head.Methods)*

Returns HDCP-related data.  
**hdcpReason Argument Values**  
* `0`: HDMI cable is not connected or rx sense status is `off`  
* `1`: Rx device is connected with power ON state, and HDCP authentication is not initiated  
* `2`: HDCP success  
* `3`:  HDCP authentication failed after multiple retries  
* `4`:  HDCP authentication in progress   
* `5`: HDMI video port is disabled. 
 
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.HDCPStatus | object | Contains HDCP-related data as separate properties |
| result.HDCPStatus.isConnected | boolean | Indicates whether a display is connected |
| result.HDCPStatus.isHDCPCompliant | boolean | Indicates whether the display is HDCP compliant |
| result.HDCPStatus.isHDCPEnabled | boolean | Indicates whether content is protected |
| result.HDCPStatus.hdcpReason | integer | The HDCP status reason |
| result.HDCPStatus.supportedHDCPVersion | string | Supported HDCP protocol version by the host device |
| result.HDCPStatus.receiverHDCPVersion | string | Supported HDCP protocol version by the receiver device (display) |
| result.HDCPStatus.currentHDCPVersion | string | Currently used HDCP protocol version |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdcpProfile.1.getHDCPStatus"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "HDCPStatus": {
            "isConnected": false,
            "isHDCPCompliant": false,
            "isHDCPEnabled": false,
            "hdcpReason": 1,
            "supportedHDCPVersion": "2.2",
            "receiverHDCPVersion": "1.4",
            "currentHDCPVersion": "1.4"
        },
        "success": true
    }
}
```

<a name="method.getSettopHDCPSupport"></a>
## *getSettopHDCPSupport [<sup>method</sup>](#head.Methods)*

Returns which version of HDCP is supported by the STB. 
 
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.supportedHDCPVersion | string | Supported HDCP protocol version by the host device |
| result.isHDCPSupported | boolean | Indicates whether HDCP is supported by the STB |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdcpProfile.1.getSettopHDCPSupport"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "supportedHDCPVersion": "2.2",
        "isHDCPSupported": true,
        "success": true
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.HdcpProfile plugin:

HdcpProfile interface events:

| Event | Description |
| :-------- | :-------- |
| [onDisplayConnectionChanged](#event.onDisplayConnectionChanged) | Triggered if HDMI was connected or disconnected upon receiving `onHdmiOutputHotPlug` event |


<a name="event.onDisplayConnectionChanged"></a>
## *onDisplayConnectionChanged [<sup>event</sup>](#head.Notifications)*

Triggered if HDMI was connected or disconnected upon receiving `onHdmiOutputHotPlug` event.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.HDCPStatus | object | Contains HDCP-related data as separate properties |
| params.HDCPStatus.isConnected | boolean | Indicates whether a display is connected |
| params.HDCPStatus.isHDCPCompliant | boolean | Indicates whether the display is HDCP compliant |
| params.HDCPStatus.isHDCPEnabled | boolean | Indicates whether content is protected |
| params.HDCPStatus.hdcpReason | integer | The HDCP status reason |
| params.HDCPStatus.supportedHDCPVersion | string | Supported HDCP protocol version by the host device |
| params.HDCPStatus.receiverHDCPVersion | string | Supported HDCP protocol version by the receiver device (display) |
| params.HDCPStatus.currentHDCPVersion | string | Currently used HDCP protocol version |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDisplayConnectionChanged",
    "params": {
        "HDCPStatus": {
            "isConnected": false,
            "isHDCPCompliant": false,
            "isHDCPEnabled": false,
            "hdcpReason": 1,
            "supportedHDCPVersion": "2.2",
            "receiverHDCPVersion": "1.4",
            "currentHDCPVersion": "1.4"
        }
    }
}
```

