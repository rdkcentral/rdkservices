<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.Wifi_Plugin"></a>
# Wifi Plugin

**Version: 1.0**

**Status: :black_circle::black_circle::black_circle:**

org.rdk.Wifi plugin for Thunder framework.

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

This document describes purpose and functionality of the org.rdk.Wifi plugin. It includes detailed specification about its configuration, methods provided and notifications sent.

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

The `Wifi` plugin is used to manage Wifi network connections on a set-top device.

The plugin is designed to be loaded and executed within the Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.Wifi*) |
| classname | string | Class name: *org.rdk.Wifi* |
| locator | string | Library name: *libWPEFrameworkWifiManager.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.Wifi plugin:

WifiManager interface methods:

| Method | Description |
| :-------- | :-------- |
| [cancelWPSPairing](#method.cancelWPSPairing) | Cancels the in-progress WPS pairing operation |
| [clearSSID](#method.clearSSID) | Clears the saved SSID |
| [connect](#method.connect) | Attempts to connect to the specified SSID with the given passphrase |
| [disconnect](#method.disconnect) | Disconnects from the connected SSID |
| [getConnectedSSID](#method.getConnectedSSID) | Returns the connected SSID information such as SSID, BSSID, rate, noise, security, signal strength and frequency of the connected WiFi network |
| [getCurrentState](#method.getCurrentState) | Returns the current WiFi States |
| [getPairedSSID](#method.getPairedSSID) | Returns the SSID to which the device is currently paired |
| [getPairedSSIDInfo](#method.getPairedSSIDInfo) | Returns the SSID and BSSID to which the device is currently paired |
| [getSupportedSecurityModes](#method.getSupportedSecurityModes) | Returns the WiFi security modes that the device supports |
| [initiateWPSPairing](#method.initiateWPSPairing) | Initiates a connection using WiFi Protected Setup (WPS) |
| [isPaired](#method.isPaired) | Determines whether the device is paired to a specified SSID or not |
| [isSignalThresholdChangeEnabled](#method.isSignalThresholdChangeEnabled) | Checks whether `onWifiSignalThresholdChanged` event is enabled or not |
| [saveSSID](#method.saveSSID) | Saves the SSID, passphrase, and security mode of the WiFi network for future sessions |
| [setEnabled](#method.setEnabled) | Enables or disables the WiFi adapter for this device |
| [setSignalThresholdChangeEnabled](#method.setSignalThresholdChangeEnabled) | Sets the `time interval` to check the WiFi signal strength in a given time interval |
| [startScan](#method.startScan) | Scans for available SSIDs |
| [stopScan](#method.stopScan) | Stops scanning for SSIDs |


<a name="method.cancelWPSPairing"></a>
## *cancelWPSPairing <sup>method</sup>*

Cancels the in-progress WPS pairing operation. The operation forcefully stops the in-progress paring attempt and aborts the current scan. WPS pairing must be in-progress for the operation to get succeed.
  
### Events
 
#### onError
 
| Event | Description | 
| :-------- | :-------- | 
| `ErrorCode: NO_SSID` | WPS pairing gets successfully cancelled. A new WPS can be initiated |.

Also see: [onError](#event.onError)

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.result | string | Not supported |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.cancelWPSPairing"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "result": "",
        "success": true
    }
}
```

<a name="method.clearSSID"></a>
## *clearSSID <sup>method</sup>*

Clears the saved SSID. Triggers `onWiFiStateChanged` event.
 
### Events
 
onWIFIStateChanged
 
| Event | Description | 
| :-------- | :-------- | 
| `WifiState`: `DISCONNECTED` | After you clear the saved SSID, the WiFi gets disconnected |.

Also see: [onWIFIStateChanged](#event.onWIFIStateChanged)

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.result | integer | The result of the operation (must be one of the following: *0*, *1*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.clearSSID"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "result": 0,
        "success": true
    }
}
```

<a name="method.connect"></a>
## *connect <sup>method</sup>*

Attempts to connect to the specified SSID with the given passphrase. Passphrase can be null when the network security is `NONE`. This method attempts to connect to the saved SSID and password, when called with no arguments.
 
### Events 
 
#### onWiFiStateChanged
 
| Event | Description | 
| :-------- | :-------- | 
| `WifiState`: `DISCONNECTED` | When connected to any SSID, the device first switches to disconnected state | 
| `WifiState`: `CONNECTING` | The WiFi is connecting with the requested SSID and Passphrase | 
| `WifiState`: `CONNECTED` | The WiFi is successfully connected to the device | 
| `WifiState`: `FAILED` | If the device has encountered with any recoverable or unexpected WiFi error occurs | 
 
#### onError
 
| Event | Description |  
| :-------- | :-------- |  
| `ErrorCode`: `SSID_CHANGED` | This error occurs, when the same SID gets changed | 
| `ErrorCode`: `CONNECTION_FAILED` | This error occurs, when the network connection is failed for an unknown technical reason | 
| `ErrorCode`: `INVALID_CREDENTIALS` | This error occurs, when the networkconnection gets failed due to invalid network credentials | 
| `ErrorCode`: `NO_SSID` | This error occurs, when the requested SSID does not exist |.

Also see: [onWiFiStateChanged](#event.onWiFiStateChanged), [onError](#event.onError)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ssid | string | The paired SSID |
| params.passphrase | string | The access point password |
| params.securityMode | integer | The security mode. See `getSupportedSecurityModes` |

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
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.connect",
    "params": {
        "ssid": "123412341234",
        "passphrase": "password",
        "securityMode": 2
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.disconnect"></a>
## *disconnect <sup>method</sup>*

Disconnects from the connected SSID. The `onWiFiStateChanged` event triggers on WiFi state changes.
 
### Events
 
#### onWIFIStateChanged
 
| Event | Description |  
| :-------- | :-------- |  
| `WifiState`: `DISCONNECTED` | The WiFi gets disconnect when the device is connected to any of the SSID.|
| `WifiState`: `FAILED` | If the device has encountered with any recoverable or unexpected WiFi errors |.

Also see: [onWiFiStateChanged](#event.onWiFiStateChanged)

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.result | string | Not supported |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.disconnect"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "result": "",
        "success": true
    }
}
```

<a name="method.getConnectedSSID"></a>
## *getConnectedSSID <sup>method</sup>*

Returns the connected SSID information such as SSID, BSSID, rate, noise, security, signal strength and frequency of the connected WiFi network.
 
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.ssid | string | The paired SSID |
| result.bssid | string | The paired BSSID |
| result.rate | string | The physical data rate in Mbps |
| result.noise | string | The average noise strength in dBm |
| result.security | string | The security mode. See the `connect` method |
| result.signalStrength | string | The RSSI value in dBm |
| result.frequency | string | The supported frequency for this SSID in GHz |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.getConnectedSSID"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "ssid": "123412341234",
        "bssid": "ff:ff:ff:ff:ff:ff",
        "rate": "144.000000",
        "noise": "-121.000000",
        "security": "5",
        "signalStrength": "-27.000000",
        "frequency": "2.442000",
        "success": true
    }
}
```

<a name="method.getCurrentState"></a>
## *getCurrentState <sup>method</sup>*

Returns the current WiFi States. The possible WiFi states are as follows.
 
### WiFi States
  
`0`: UNINSTALLED - The device was installed and uninstalled; or it does not have a Wifi radio installed
 
`1`: DISABLED - The device is installed but not yet enabled
 
`2`: DISCONNECTED - The device is installed and enabled, but not yet connected to a network
 
`3`: PAIRING - The device is in process of pairing, but not yet connected to a network
 
`4`: CONNECTING - The device is attempting to connect to a network
 
`5`: CONNECTED - The device is successfully connected to a network
 
`6`: FAILED - The device has encountered an unrecoverable error with the WiFi adapter
 
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.state | integer | The Wifi operational state |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.getCurrentState"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "state": 2,
        "success": true
    }
}
```

<a name="method.getPairedSSID"></a>
## *getPairedSSID <sup>method</sup>*

Returns the SSID to which the device is currently paired.
 
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.ssid | string | The paired SSID |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.getPairedSSID"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "ssid": "123412341234",
        "success": true
    }
}
```

<a name="method.getPairedSSIDInfo"></a>
## *getPairedSSIDInfo <sup>method</sup>*

Returns the SSID and BSSID to which the device is currently paired. The BSSID is the MAC address of a connected WiFi network.
 
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.ssid | string | The paired SSID |
| result.bssid | string | The paired BSSID |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.getPairedSSIDInfo"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "ssid": "123412341234",
        "bssid": "ff:ff:ff:ff:ff:ff",
        "success": true
    }
}
```

<a name="method.getSupportedSecurityModes"></a>
## *getSupportedSecurityModes <sup>method</sup>*

Returns the WiFi security modes that the device supports.
  
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.security_modes | object | The supported security modes and its associated integer value |
| result.security_modes?.NET_WIFI_SECURITY_NONE | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WEP_64 | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WEP_128 | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WPA_PSK_TKIP | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WPA_PSK_AES | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WPA2_PSK_TKIP | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WPA2_PSK_AES | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WPA_ENTERPRISE_TKIP | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WPA_ENTERPRISE_AES | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WPA2_ENTERPRISE_TKIP | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WPA2_ENTERPRISE_AES | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WPA_WPA2_PSK | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WPA_WPA2_ENTERPRISE | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WPA3_PSK_AES | integer | <sup>*(optional)*</sup>  |
| result.security_modes?.NET_WIFI_SECURITY_WPA3_SAE | integer | <sup>*(optional)*</sup>  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.getSupportedSecurityModes"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "security_modes": {
            "NET_WIFI_SECURITY_NONE": 0,
            "NET_WIFI_SECURITY_WEP_64": 1,
            "NET_WIFI_SECURITY_WEP_128": 2,
            "NET_WIFI_SECURITY_WPA_PSK_TKIP": 3,
            "NET_WIFI_SECURITY_WPA_PSK_AES": 4,
            "NET_WIFI_SECURITY_WPA2_PSK_TKIP": 5,
            "NET_WIFI_SECURITY_WPA2_PSK_AES": 6,
            "NET_WIFI_SECURITY_WPA_ENTERPRISE_TKIP": 7,
            "NET_WIFI_SECURITY_WPA_ENTERPRISE_AES": 8,
            "NET_WIFI_SECURITY_WPA2_ENTERPRISE_TKIP": 9,
            "NET_WIFI_SECURITY_WPA2_ENTERPRISE_AES": 10,
            "NET_WIFI_SECURITY_WPA_WPA2_PSK": 11,
            "NET_WIFI_SECURITY_WPA_WPA2_ENTERPRISE": 12,
            "NET_WIFI_SECURITY_WPA3_PSK_AES": 13,
            "NET_WIFI_SECURITY_WPA3_SAE": 14
        },
        "success": true
    }
}
```

<a name="method.initiateWPSPairing"></a>
## *initiateWPSPairing <sup>method</sup>*

Initiates a connection using WiFi Protected Setup (WPS). An existing connection is disconnected before attempting to initiate a new connection. If the existing connection cannot be disconnected after 60 seconds, then the WPS pairing will stop, and an error event triggers. This method sends both `onWifiStateChanged` and `onError` events.
 
### Events
 
#### onWifiStateChanged
 
| Event | Description | 
| :-------- | :-------- |  
| `WifiState`: `CONNECTING` | Indicates that a WPS key code has been received from the access point. The device is in the process of connecting to the network | 
| `WifiState`: `CONNECTED` | Indicates that WPS pairing succeeded, and the device is connected to the access point | 
| `WifiState`: `FAILED` | Indicates that the connection attempt failed and should be attempted again. Failures includes a connection is already in progress; the WPA supplicant configuration file could not be opened or saved; the WPS key code was not received or an existing connection could not be disconnected |
 
#### onError
 
| Event | Description | 
| :-------- | :-------- | 
| `ErrorCode`: `CONNECTION_FAILED` | Indicates a failure to connect to the access point | 
| `ErrorCode`: `NO_SSID` | Indicates that the connection timeout threshold of 120 seconds was reached or that a WPS overlap was detected |.

Also see: [onWifiStateChanged](#event.onWifiStateChanged), [onError](#event.onError)

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.result | string | Not supported |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.initiateWPSPairing"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "result": "",
        "success": true
    }
}
```

<a name="method.isPaired"></a>
## *isPaired <sup>method</sup>*

Determines whether the device is paired to a specified SSID or not.
 
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.result | integer | The result of the operation (must be one of the following: *0*, *1*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.isPaired"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "result": 0,
        "success": true
    }
}
```

<a name="method.isSignalThresholdChangeEnabled"></a>
## *isSignalThresholdChangeEnabled <sup>method</sup>*

Checks whether `onWifiSignalThresholdChanged` event is enabled or not.
 
### Events
 
No Events.

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.result | integer | The result of the operation (must be one of the following: *0*, *1*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.isSignalThresholdChangeEnabled"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "result": 0,
        "success": true
    }
}
```

<a name="method.saveSSID"></a>
## *saveSSID <sup>method</sup>*

Saves the SSID, passphrase, and security mode of the WiFi network for future sessions. If the SSID was previously saved then, the new SSID and passphrase will overwrite the existing values.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ssid | string | The paired SSID |
| params.passphrase | string | The access point password |
| params.securityMode | integer | The security mode. See `getSupportedSecurityModes` |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.result | integer | The result of the operation (must be one of the following: *0*, *1*) |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.saveSSID",
    "params": {
        "ssid": "123412341234",
        "passphrase": "password",
        "securityMode": 2
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "result": 0,
        "success": true
    }
}
```

<a name="method.setEnabled"></a>
## *setEnabled <sup>method</sup>*

Enables or disables the WiFi adapter for this device.
 
### Events
 
No Events.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enable | boolean | `true` to enable, or `false` to disable |

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
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.setEnabled",
    "params": {
        "enable": true
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.setSignalThresholdChangeEnabled"></a>
## *setSignalThresholdChangeEnabled <sup>method</sup>*

Sets the `time interval` to check the WiFi signal strength in a given time interval. The `onWifiSignalThresholdChanged` event is triggered when the `setSignalThresholdChangedEnabled` method sets the `enabled` parameter value as `true`.
 
### Events
 
| Event | Description | 
| :-------- | :-------- | 
| `onWifiSignalThresholdChanged` | This event gets triggered recursively in the specified value in interval param of  `setSignalThresholdChangeEnabled` method |.

Also see: [onWifiSignalThresholdChanged](#event.onWifiSignalThresholdChanged)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.enabled | boolean | `true` to enable the `onWifiSignalThresholdChanged` event or `false` to disable the `onWifiSignalThresholdChanged` event |
| params.interval | integer | It is the time interval value in milliseconds, to determine if the network strength crossed a threshold value for the given time interval |

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
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.setSignalThresholdChangeEnabled",
    "params": {
        "enabled": true,
        "interval": 2000
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="method.startScan"></a>
## *startScan <sup>method</sup>*

Scans for available SSIDs. Available SSIDs are returned in an `onAvailableSSIDs` event.
 
### Events
 
| Event | Description | 
| :-------- | :-------- | 
| `onAvailableSSIDs` | Triggered when the `startScan method` is called and SSIDs are obtained. If the scan method is called with the incremental property set to `true`, then `moreData` is `false` when the last set of results are received. If the incremental property in `startScan` method is set to `false`, then the event returns with `moreData` is `false` after a single event |.

Also see: [onAvailableSSIDs](#event.onAvailableSSIDs)

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.incremental | boolean | If set to `true`, SSIDs are returned in multiple events as the SSIDs are discovered. This may allow the UI to populate faster on screen rather than waiting on the full set of results in one shot |
| params.ssid | string | The SSIDs to scan. An empty or `null` value scans for all SSIDs. If an SSID is specified, then the results are only returned for matching SSID names. SSIDs may be entered as a string literal or regular expression |
| params.frequency | string | The frequency to scan. An empty or `null` value scans all frequencies. If a frequency is specified (2.4 or 5.0), then the results are only returned for matching frequencies |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.success | boolean | Whether the request succeeded |
| result.error | string | Error string of scan failure |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.startScan",
    "params": {
        "incremental": false,
        "ssid": "",
        "frequency": ""
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
        "error": ""
    }
}
```

<a name="method.stopScan"></a>
## *stopScan <sup>method</sup>*

Stops scanning for SSIDs. From the `startScan` method up to the point where this method is called, all discovered SSIDs are still returned.
 
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
    "id": 1234567890,
    "method": "org.rdk.Wifi.1.stopScan"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "success": true
    }
}
```

<a name="head.Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#ref.Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.Wifi plugin:

WifiManager interface events:

| Event | Description |
| :-------- | :-------- |
| [onWIFIStateChanged](#event.onWIFIStateChanged) | Triggered when the WiFi state changes |
| [onError](#event.onError) | Triggered when a recoverable unexpected WiFi error occurs |
| [onSSIDsChanged](#event.onSSIDsChanged) | Triggered when a new SSID becomes available or an existing SSID is no longer available |
| [onWifiSignalThresholdChanged](#event.onWifiSignalThresholdChanged) | Triggered at intervals specified in the `setSignalThresholdChangeEnabled` method in order to monitor changes in WiFi strength |
| [onAvailableSSIDs](#event.onAvailableSSIDs) | Triggered when the scan method is called and current SSIDs are obtained |


<a name="event.onWIFIStateChanged"></a>
## *onWIFIStateChanged <sup>event</sup>*

Triggered when the WiFi state changes. See `getCurrentState` for a list of valid WiFi states.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.state | integer | The Wifi operational state |
| params.isLNF | boolean | `true` if connected to a LNF SSID, otherwise `false` |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onWIFIStateChanged",
    "params": {
        "state": 2,
        "isLNF": false
    }
}
```

<a name="event.onError"></a>
## *onError <sup>event</sup>*

Triggered when a recoverable unexpected WiFi error occurs.
 
### Error Codes
 
`0`: SSID_CHANGED - The SSID of the network changed
 
`1`: CONNECTION_LOST - The connection to the network is lost
 
`2`: CONNECTION_FAILED - The connection failed for an unknown reason
 
`3`: CONNECTION_INTERRUPTED - The connection was interrupted
 
`4`: INVALID_CREDENTIALS - The connection failed due to invalid credentials
 
`5`: NO_SSID - The SSID does not exist
 
`6`: UNKNOWN - Any other error.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.code | integer | The error code |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onError",
    "params": {
        "code": 2
    }
}
```

<a name="event.onSSIDsChanged"></a>
## *onSSIDsChanged <sup>event</sup>*

Triggered when a new SSID becomes available or an existing SSID is no longer available.

### Parameters

This event carries no parameters.

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onSSIDsChanged"
}
```

<a name="event.onWifiSignalThresholdChanged"></a>
## *onWifiSignalThresholdChanged <sup>event</sup>*

Triggered at intervals specified in the `setSignalThresholdChangeEnabled` method in order to monitor changes in WiFi strength.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.signalStrength | string | The RSSI value in dBm |
| params.strength | string | WiFi signal strength indicates as excellent, good, fair, and weak  (must be one of the following: *Excellent*, *Good*, *Fair*, *Weak*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onWifiSignalThresholdChanged",
    "params": {
        "signalStrength": "-27.000000",
        "strength": "Excellent"
    }
}
```

<a name="event.onAvailableSSIDs"></a>
## *onAvailableSSIDs <sup>event</sup>*

Triggered when the scan method is called and current SSIDs are obtained.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ssids | array | A list of SSIDs and their information |
| params.ssids[#] | object |  |
| params.ssids[#].ssid | string | The paired SSID |
| params.ssids[#].security | integer | The security mode. See `getSupportedSecurityModes` |
| params.ssids[#].signalStrength | string | The RSSI value in dBm |
| params.ssids[#].frequency | string | The supported frequency for this SSID in GHz |
| params.moreData | boolean | If the scan method is called with the incremental property set to `true`, then `moreData` is `false` when the last set of results are received. If the incremental property is set to `false`, then `moreData` is `false` after a single event |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.1.onAvailableSSIDs",
    "params": {
        "ssids": [
            {
                "ssid": "123412341234",
                "security": 2,
                "signalStrength": "-27.000000",
                "frequency": "2.442000"
            }
        ],
        "moreData": true
    }
}
```

