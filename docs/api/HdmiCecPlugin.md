<!-- Generated automatically, DO NOT EDIT! -->
<a name="HdmiCecPlugin"></a>
# HdmiCecPlugin

**Version: 1.0.0**

A org.rdk.HdmiCec plugin for Thunder framework.

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

The `HdmiCec` plugin allows you to configure HDMI Consumer Electronics Control (CEC) on a set-top device. HdmiCec plugin can be used on an HDMI Source device where the application has complete control on implementing the CEC messages, and Thunder plugin provides the transport mechanism to send and receive the messages to other CEC devices in the network.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.HdmiCec*) |
| classname | string | Class name: *org.rdk.HdmiCec* |
| locator | string | Library name: *libWPEFrameworkHdmiCec.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.HdmiCec plugin:

HdmiCec interface methods:

| Method | Description |
| :-------- | :-------- |
| [getActiveSourceStatus](#getActiveSourceStatus) | Gets the active source status of the device |
| [getCECAddresses](#getCECAddresses) | Returns the HDMI-CEC addresses that are assigned to the local device |
| [getDeviceList](#getDeviceList) | Gets the list of number of CEC enabled devices connected and system information for each device |
| [getEnabled](#getEnabled) | Returns whether HDMI-CEC is enabled |
| [sendMessage](#sendMessage) | Writes HDMI-CEC frame to the driver |
| [setEnabled](#setEnabled) | Enables or disables HDMI-CEC driver |


<a name="getActiveSourceStatus"></a>
## *getActiveSourceStatus*

Gets the active source status of the device.
  
### Event 

 No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.status | boolean | `true` if device is active source, otherwise `false` |

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
    "method": "org.rdk.HdmiCec.1.getActiveSourceStatus",
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

<a name="getCECAddresses"></a>
## *getCECAddresses*

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
| result.CECAddresses.deviceType | string | The type of device |
| result.CECAddresses.logicalAddress | integer | The logical address of the device |
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
            "deviceType": "Tuner",
            "logicalAddress": 3
        },
        "success": true
    }
}
```

<a name="getDeviceList"></a>
## *getDeviceList*

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

<a name="getEnabled"></a>
## *getEnabled*

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

<a name="sendMessage"></a>
## *sendMessage*

Writes HDMI-CEC frame to the driver.
 
### Events 
| Event | Description | 
| :----------- | :----------- |
| `onMessage`|Triggered when a message is sent from an HDMI device|.

Also see: [onMessage](#onMessage)

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

<a name="setEnabled"></a>
## *setEnabled*

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

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.HdmiCec plugin:

HdmiCec interface events:

| Event | Description |
| :-------- | :-------- |
| [cecAddressesChanged](#cecAddressesChanged) | Triggered when the address of the host CEC device has changed |
| [onActiveSourceStatusUpdated](#onActiveSourceStatusUpdated) | Triggered when device active source status changes |
| [onDeviceAdded](#onDeviceAdded) | Triggered when an HDMI cable is physically connected to the HDMI port on a TV, or the power cable is connected to the source device |
| [onDeviceInfoUpdated](#onDeviceInfoUpdated) | Triggered when device information changes (vendorID, osdName) |
| [onDeviceRemoved](#onDeviceRemoved) | Triggered when HDMI cable is physically removed from the HDMI port on a TV or the power cable is removed from the source device |
| [onMessage](#onMessage) | Triggered when a message is sent from an HDMI device |


<a name="cecAddressesChanged"></a>
## *cecAddressesChanged*

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

<a name="onActiveSourceStatusUpdated"></a>
## *onActiveSourceStatusUpdated*

Triggered when device active source status changes.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.status | boolean | `true` if device is active source, otherwise `false` |

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

<a name="onDeviceInfoUpdated"></a>
## *onDeviceInfoUpdated*

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

<a name="onMessage"></a>
## *onMessage*

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

