<!-- Generated automatically, DO NOT EDIT! -->
<a name="HdmiCecSourcePlugin"></a>
# HdmiCecSourcePlugin

**Version: [1.0.5](https://github.com/rdkcentral/rdkservices/blob/main/HdmiCecSource/CHANGELOG.md)**

A org.rdk.HdmiCecSource plugin for Thunder framework.

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

The `HdmiCecSource` plugin allows you to configure HDMI Consumer Electronics Control (CEC) on a set-top device. The HdmiCecSource plugin is meant to be used on the source devices where an application relies on the Thunder plugin to handle protocol related messaging. The plugin also provides API's and events to implement the CEC use cases.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.HdmiCecSource*) |
| classname | string | Class name: *org.rdk.HdmiCecSource* |
| locator | string | Library name: *libWPEFrameworkHdmiCecSource.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.HdmiCecSource plugin:

HdmiCecSource interface methods:

| Method | Description |
| :-------- | :-------- |
| [getActiveSourceStatus](#getActiveSourceStatus) | Gets the active source status of the device |
| [getDeviceList](#getDeviceList) | Gets the list of CEC enabled devices connected and system information for each device |
| [getEnabled](#getEnabled) | Returns HDMI-CEC driver enabled status |
| [getOSDName](#getOSDName) | Returns the OSD name set by the application |
| [getOTPEnabled](#getOTPEnabled) | Returns HDMI-CEC OTP option enabled status |
| [getVendorId](#getVendorId) | Returns the vendor ID set by the application |
| [performOTPAction](#performOTPAction) | Turns on the TV and takes back the input to the device |
| [sendKeyPressEvent](#sendKeyPressEvent) | Sends the CEC \<User Control Pressed\> and \<User Control Release\> message when TV remote key is pressed |
| [sendStandbyMessage](#sendStandbyMessage) | Sends a CEC \<Standby\> message to the logical address of the device |
| [setEnabled](#setEnabled) | Enables or disables HDMI-CEC driver |
| [setOSDName](#setOSDName) | Sets the OSD name of the application |
| [setOTPEnabled](#setOTPEnabled) | Enables or disables HDMI-CEC OTP option |
| [setVendorId](#setVendorId) | Sets the vendor ID of the application |


<a name="getActiveSourceStatus"></a>
## *getActiveSourceStatus*

Gets the active source status of the device.

### Events

No Events

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
    "method": "org.rdk.HdmiCecSource.getActiveSourceStatus",
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

No Events

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
    "method": "org.rdk.HdmiCecSource.getDeviceList"
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

No Events

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
    "method": "org.rdk.HdmiCecSource.getEnabled"
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

No Events

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
    "method": "org.rdk.HdmiCecSource.getOSDName"
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

No Events

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
    "method": "org.rdk.HdmiCecSource.getOTPEnabled"
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

No Events

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
    "method": "org.rdk.HdmiCecSource.getVendorId"
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

No Events

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
    "method": "org.rdk.HdmiCecSource.performOTPAction"
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

<a name="sendKeyPressEvent"></a>
## *sendKeyPressEvent*

Sends the CEC \<User Control Pressed\> and \<User Control Release\> message when TV remote key is pressed.
  
### Event 

 No Events.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logicalAddress | integer | Logical address of the device |
| params.keyCode | integer | The key code for the pressed key |

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
    "method": "org.rdk.HdmiCecSource.sendKeyPressEvent",
    "params": {
        "logicalAddress": 0,
        "keyCode": 65
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

<a name="sendStandbyMessage"></a>
## *sendStandbyMessage*

Sends a CEC \<Standby\> message to the logical address of the device.

### Events

No Events

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
    "method": "org.rdk.HdmiCecSource.sendStandbyMessage"
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

No Events

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
    "method": "org.rdk.HdmiCecSource.setEnabled",
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

No Events

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
    "method": "org.rdk.HdmiCecSource.setOSDName",
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

No Events

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
    "method": "org.rdk.HdmiCecSource.setOTPEnabled",
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

No Events

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
    "method": "org.rdk.HdmiCecSource.setVendorId",
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

The following events are provided by the org.rdk.HdmiCecSource plugin:

HdmiCecSource interface events:

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
    "method": "client.events.onActiveSourceStatusUpdated",
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
    "method": "client.events.onDeviceAdded",
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
    "method": "client.events.onDeviceInfoUpdated",
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
    "method": "client.events.onDeviceRemoved",
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
    "method": "client.events.standbyMessageReceived",
    "params": {
        "logicalAddress": 0
    }
}
```

