<!-- Generated automatically, DO NOT EDIT! -->
<a name="HdcpProfile_Plugin"></a>
# HdcpProfile Plugin

**Version: 1.0.0**

A org.rdk.HdcpProfile plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Notifications](#Notifications)

<a name="Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="Description"></a>
# Description

The HdcpProfile plugin provides an interface for HDCP-related data and events.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.HdcpProfile*) |
| classname | string | Class name: *org.rdk.HdcpProfile* |
| locator | string | Library name: *libWPEFrameworkHdcpProfile.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.HdcpProfile plugin:

HdcpProfile interface methods:

| Method | Description |
| :-------- | :-------- |
| [getHDCPStatus](#getHDCPStatus) | Returns HDCP-related data |
| [getSettopHDCPSupport](#getSettopHDCPSupport) | Returns which version of HDCP is supported by the STB |


<a name="getHDCPStatus"></a>
## *getHDCPStatus*

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

<a name="getSettopHDCPSupport"></a>
## *getSettopHDCPSupport*

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

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.HdcpProfile plugin:

HdcpProfile interface events:

| Event | Description |
| :-------- | :-------- |
| [onDisplayConnectionChanged](#onDisplayConnectionChanged) | Triggered if HDMI was connected or disconnected upon receiving `onHdmiOutputHotPlug` event |


<a name="onDisplayConnectionChanged"></a>
## *onDisplayConnectionChanged*

Triggered if HDMI was connected or disconnected upon receiving `onHdmiOutputHotPlug` 

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

