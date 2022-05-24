<!-- Generated automatically, DO NOT EDIT! -->
<a name="HdmiCec_2Plugin"></a>
# HdmiCec_2Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.HdmiCec_2 plugin for Thunder framework.

### Table of Contents

- [Introduction](#Introduction)
- [Description](#Description)
- [Configuration](#Configuration)
- [Methods](#Methods)
- [Notifications](#Notifications)

<a name="Introduction"></a>
# Introduction

<a name="Scope"></a>
## Scope

This document describes purpose and functionality of the org.rdk.HdmiCec_2 plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `HdmiCec_2` plugin allows you to configure HDMI Consumer Electronics Control (CEC) on a set-top device. The HdmiCec_2 plugin is meant to be used on the source devices where an application relies on the Thunder plugin to handle protocol related messaging. The plugin also provides API's and events to implement the CEC use cases.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.HdmiCec_2*) |
| classname | string | Class name: *org.rdk.HdmiCec_2* |
| locator | string | Library name: *libWPEFrameworkHdmiCec_2.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.HdmiCec_2 plugin:

HdmiCec_2 interface methods:

| Method | Description |
| :-------- | :-------- |
| [getActiveSourceStatus](#getActiveSourceStatus) | Gets the active source status of the device |
| [getDeviceList](#getDeviceList) | Gets the list of CEC enabled devices connected and system information for each device |
| [getEnabled](#getEnabled) | Returns HDMI-CEC driver enabled status |
| [getOSDName](#getOSDName) | Returns the OSD name set by the application |
| [getOTPEnabled](#getOTPEnabled) | Returns HDMI-CEC OTP option enabled status |
| [getVendorId](#getVendorId) | Returns the vendor ID set by the application |
| [performOTPAction](#performOTPAction) | Turns on the TV and takes back the input to the device |
| [sendStandbyMessage](#sendStandbyMessage) | Sends a CEC \<Standby\> message to the logical address of the device |
| [setEnabled](#setEnabled) | Enables or disables HDMI-CEC driver |
| [setOSDName](#setOSDName) | Sets the OSD name of the application |
| [setOTPEnabled](#setOTPEnabled) | Enables or disables HDMI-CEC OTP option |
| [setVendorId](#setVendorId) | Sets the vendor ID of the application |


<a name="getActiveSourceStatus"></a>
## *getActiveSourceStatus*

Gets the active source status of the device.
  
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.status | boolean | `true` if the device is active source otherwise, `false` |

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
    "id": 42,
    "method": "org.rdk.HdmiCec_2.1.getActiveSourceStatus",
    "params": {
        "status": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="getDeviceList"></a>
## *getDeviceList*

Gets the list of CEC enabled devices connected and system information for each device. The information includes logicalAddress,OSD name and vendor ID.
  
### Events 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.numberofdevices | integer | number of devices in the `deviceList` array |
| result.deviceList | array | Object [] of information about each device |
| result.deviceList[#] | object |  |
| result.deviceList[#].logicalAddress | integer | Logical address of the device |
| result.deviceList[#].osdName | string | OSD name of the device |
| result.deviceList[#].vendorID | string | Vendor ID of the device |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCec_2.1.getDeviceList"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "numberofdevices": 1,
        "deviceList": [
            {
                "logicalAddress": 0,
                "osdName": "TV Box",
                "vendorID": "019fb"
            }
        ],
        "success": true
    }
}
```

<a name="getEnabled"></a>
## *getEnabled*

Returns HDMI-CEC driver enabled status.
  
### Events 

 No Events.

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
    "id": 42,
    "method": "org.rdk.HdmiCec_2.1.getEnabled"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enabled": false,
        "success": true
    }
}
```

<a name="getOSDName"></a>
## *getOSDName*

Returns the OSD name set by the application.
  
### Events 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.name | string | The OSD name. The default value is `TV Box` if no value is set |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCec_2.1.getOSDName"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "name": "Sky TV",
        "success": true
    }
}
```

<a name="getOTPEnabled"></a>
## *getOTPEnabled*

Returns HDMI-CEC OTP option enabled status.
  
### Events 

 No Events.

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
    "id": 42,
    "method": "org.rdk.HdmiCec_2.1.getOTPEnabled"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "enabled": false,
        "success": true
    }
}
```

<a name="getVendorId"></a>
## *getVendorId*

Returns the vendor ID set by the application.
  
### Events 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.vendorid | string | The vendor ID. The default value is `0019FB` if no value is set. If the device is connected to an LG TV, then `00E091` is used as the vendor ID |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCec_2.1.getVendorId"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "vendorid": "0x0019FB",
        "success": true
    }
}
```

<a name="performOTPAction"></a>
## *performOTPAction*

Turns on the TV and takes back the input to the device.
  
### Events 

 No Events.

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
    "id": 42,
    "method": "org.rdk.HdmiCec_2.1.performOTPAction"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="sendStandbyMessage"></a>
## *sendStandbyMessage*

Sends a CEC \<Standby\> message to the logical address of the device.
  
### Events 

 No Events.

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
    "id": 42,
    "method": "org.rdk.HdmiCec_2.1.sendStandbyMessage"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setEnabled"></a>
## *setEnabled*

Enables or disables HDMI-CEC driver.
  
### Events 

 No Events.

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
    "id": 42,
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
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setOSDName"></a>
## *setOSDName*

Sets the OSD name of the application.
  
### Events 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.name | string | The OSD name. The default value is `TV Box` if no value is set |

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
    "id": 42,
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
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setOTPEnabled"></a>
## *setOTPEnabled*

Enables or disables HDMI-CEC OTP option.
  
### Events 

 No Events.

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
    "id": 42,
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
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="setVendorId"></a>
## *setVendorId*

Sets the vendor ID of the application.
  
### Events 

 No Events.

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
    "id": 42,
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
    "id": 42,
    "result": {
        "success": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.HdmiCec_2 plugin:

HdmiCec_2 interface events:

| Event | Description |
| :-------- | :-------- |
| [onActiveSourceStatusUpdated](#onActiveSourceStatusUpdated) | Triggered when the device active source status changes |
| [onDeviceAdded](#onDeviceAdded) | Triggered when an HDMI cable is physically connected to the HDMI port on a TV, or the power cable is connected to the source device |
| [onDeviceInfoUpdated](#onDeviceInfoUpdated) | Triggered when device system information is updated (vendorID, osdName) |
| [onDeviceRemoved](#onDeviceRemoved) | Triggered when HDMI cable is physically removed from the HDMI port on a TV or the power cable is removed from the source device |
| [standbyMessageReceived](#standbyMessageReceived) | Triggered when the source device changes status to `STANDBY` |


<a name="onActiveSourceStatusUpdated"></a>
## *onActiveSourceStatusUpdated*

Triggered when the device active source status changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.status | boolean | `true` if the device is active source otherwise, `false` |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onActiveSourceStatusUpdated",
    "params": {
        "status": true
    }
}
```

<a name="onDeviceAdded"></a>
## *onDeviceAdded*

Triggered when an HDMI cable is physically connected to the HDMI port on a TV, or the power cable is connected to the source device.  After a new device is hotplugged to the port, various information such as OSD name and vendor ID is collected. The `onDeviceAdded` event is sent as soon as any of these details are available. However, the connected device sends the information asynchronously; therefore, the information may not be collected immediately.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDeviceAdded",
    "params": {
        "logicalAddress": 0
    }
}
```

<a name="onDeviceInfoUpdated"></a>
## *onDeviceInfoUpdated*

Triggered when device system information is updated (vendorID, osdName).

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDeviceInfoUpdated",
    "params": {
        "logicalAddress": 0
    }
}
```

<a name="onDeviceRemoved"></a>
## *onDeviceRemoved*

Triggered when HDMI cable is physically removed from the HDMI port on a TV or the power cable is removed from the source device. The device is considered removed when no ACK messages are received after pinging the device.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onDeviceRemoved",
    "params": {
        "logicalAddress": 0
    }
}
```

<a name="standbyMessageReceived"></a>
## *standbyMessageReceived*

Triggered when the source device changes status to `STANDBY`.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.standbyMessageReceived",
    "params": {
        "logicalAddress": 0
    }
}
```

