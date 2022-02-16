<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.HdmiCecPlugin"></a>
# HdmiCecPlugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

A org.rdk.HdmiCec plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.HdmiCec plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `HdmiCec` plugin allows you to configure HDMI Consumer Electronics Control (CEC) on a set-top box.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.HdmiCec*) |
| classname | string | Class name: *org.rdk.HdmiCec* |
| locator | string | Library name: *libWPEFrameworkHdmiCec.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.HdmiCec plugin:

HdmiCec interface methods:

| Method | Description |
| :-------- | :-------- |
| [getCECAddresses](#method.getCECAddresses) | Returns the HDMI-CEC addresses that are assigned to the local device |
| [getDeviceList](#method.getDeviceList) | Gets the list of number of CEC enabled devices connected and system information for each device |
| [getEnabled](#method.getEnabled) | Returns whether HDMI-CEC is enabled |
| [sendMessage](#method.sendMessage) | Writes HDMI-CEC frame to the driver |
| [setEnabled](#method.setEnabled) | Enables or disables HDMI-CEC driver |


<a name="method.getCECAddresses"></a>
## *getCECAddresses [<sup>method</sup>](#head.Methods)*

Returns the HDMI-CEC addresses that are assigned to the local device.
  
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.CECAddresses | object | An object that includes both the physical and logical HDMI-CEC addresses |
| result.CECAddresses.physicalAddress | array | The physical IP address of the device |
| result.CECAddresses.physicalAddress[#] | string |  |
| result.CECAddresses.logicalAddresses | array | The logical address including the device type |
| result.CECAddresses.logicalAddresses[#] | object |  |
| result.CECAddresses.logicalAddresses[#].deviceType | string | The type of device |
| result.CECAddresses.logicalAddresses[#].logicalAddress | integer | The logical address of the device |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCec.1.getCECAddresses"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "CECAddresses": {
            "physicalAddress": [
                "255, 255, 255, 255"
            ],
            "logicalAddresses": [
                {
                    "deviceType": "Tuner",
                    "logicalAddress": 3
                }
            ]
        },
        "success": true
    }
}
```

<a name="method.getDeviceList"></a>
## *getDeviceList [<sup>method</sup>](#head.Methods)*

Gets the list of number of CEC enabled devices connected and system information for each device. The information includes logicalAddress,OSD name and vendor ID.
  
### Event 

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
    "method": "org.rdk.HdmiCec.1.getDeviceList"
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
                "osdName": "TV",
                "vendorID": "00f0"
            }
        ],
        "success": true
    }
}
```

<a name="method.getEnabled"></a>
## *getEnabled [<sup>method</sup>](#head.Methods)*

Returns whether HDMI-CEC is enabled.
  
### Event 

 No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.enabled | boolean | Indicates whether HDMI-CEC is enabled (`true`) or disabled (`false`) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.HdmiCec.1.getEnabled"
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

<a name="method.sendMessage"></a>
## *sendMessage [<sup>method</sup>](#head.Methods)*

Writes HDMI-CEC frame to the driver.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onMessage`|Triggered when a message is sent from an HDMI device|.

Also see: [onMessage](#event.onMessage)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.message | string | The message is a base64 encoded byte array of the raw CEC bytes. The CEC message includes the device ID for the intended destination |

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
    "method": "org.rdk.HdmiCec.1.sendMessage",
    "params": {
        "message": "1234567890"
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

<a name="method.setEnabled"></a>
## *setEnabled [<sup>method</sup>](#head.Methods)*

Enables or disables HDMI-CEC driver.
  
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | Indicates whether HDMI-CEC is enabled (`true`) or disabled (`false`) |

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
    "method": "org.rdk.HdmiCec.1.setEnabled",
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

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.HdmiCec plugin:

HdmiCec interface events:

| Event | Description |
| :-------- | :-------- |
| [cecAddressesChanged](#event.cecAddressesChanged) | Triggered when the address of the host CEC device has changed |
| [onMessage](#event.onMessage) | Triggered when a message is sent from an HDMI device |
| [onDeviceAdded](#event.onDeviceAdded) | Triggered when an HDMI cable is physically connected to the HDMI port on a TV, or the power cable is connected to the source device |
| [onDeviceInfoUpdated](#event.onDeviceInfoUpdated) | Triggered when device information changes (vendorID, osdName) |
| [onDeviceRemoved](#event.onDeviceRemoved) | Triggered when HDMI cable is physically removed from the HDMI port on a TV or the power cable is removed from the source device |


<a name="event.cecAddressesChanged"></a>
## *cecAddressesChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the address of the host CEC device has changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.CECAddresses | object | Includes either the `physicalAddress` or `logicalAddresses` |
| params.CECAddresses.physicalAddress | array | The physical IP address of the device |
| params.CECAddresses.physicalAddress[#] | string |  |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.cecAddressesChanged",
    "params": {
        "CECAddresses": {
            "physicalAddress": [
                "255, 255, 255, 255"
            ]
        }
    }
}
```

<a name="event.onMessage"></a>
## *onMessage [<sup>event</sup>](#head.Notifications)*

Triggered when a message is sent from an HDMI device.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.message | string | The message is a base64 encoded byte array of the raw CEC bytes. The CEC message includes the device ID for the intended destination |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onMessage",
    "params": {
        "message": "1234567890"
    }
}
```

<a name="event.onDeviceAdded"></a>
## *onDeviceAdded [<sup>event</sup>](#head.Notifications)*

Triggered when an HDMI cable is physically connected to the HDMI port on a TV, or the power cable is connected to the source device.  After a new device is hotplugged to the port, various information such as OSD name and vendor ID is collected.The `onDeviceAdded` event is sent as soon as any of these details are available. However, the connected device sends the information asynchronously; therefore, the information may not be collected immediately.

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

<a name="event.onDeviceInfoUpdated"></a>
## *onDeviceInfoUpdated [<sup>event</sup>](#head.Notifications)*

Triggered when device information changes (vendorID, osdName).

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

<a name="event.onDeviceRemoved"></a>
## *onDeviceRemoved [<sup>event</sup>](#head.Notifications)*

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

