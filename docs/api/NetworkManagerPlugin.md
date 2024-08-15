<!-- Generated automatically, DO NOT EDIT! -->
<a name="NetworkManager_Plugin"></a>
# NetworkManager Plugin

**Version: [0.2.10]()**

A NetworkManager plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#Abbreviation,_Acronyms_and_Terms)
- [Description](#Description)
- [Configuration](#Configuration)
- [Interfaces](#Interfaces)
- [Methods](#Methods)
- [Notifications](#Notifications)

<a name="Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="Description"></a>
# Description

A Unified `NetworkManager` plugin that allows you to manage Ethernet and Wifi interfaces on the device.

 The plugin is designed to be an out-of-process service to be handled by Thunder framework. For more information about the framework refer to [[Thunder](#Thunder)].

<a name="Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name (default: *org.rdk.NetworkManager*) |
| classname | string | Class name: *NetworkManager* |
| locator | string | Library name: *libWPEFrameworkNetworkManager.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="Interfaces"></a>
# Interfaces

This plugin implements the following interfaces:

- [NetworkManager.json](https://github.com/rdkcentral/ThunderInterfaces/blob/master/interfaces/NetworkManager.json)

<a name="Methods"></a>
# Methods

The following methods are provided by the org.rdk.NetworkManager plugin:

NetworkManager interface methods:

| Method | Description |
| :-------- | :-------- |
| [GetAvailableInterfaces](#method.GetAvailableInterfaces) | Get device supported list of available interface including their state |
| [GetPrimaryInterface](#method.GetPrimaryInterface) | Gets the primary/default network interface for the device |
| [SetPrimaryInterface](#method.SetPrimaryInterface) | Sets the primary/default interface for the device |
| [SetInterfaceState](#method.SetInterfaceState) | Enable the interface |
| [GetInterfaceState](#method.GetInterfaceState) | Disable the interface |
| [GetIPSettings](#method.GetIPSettings) | Gets the IP setting for the given interface |
| [SetIPSettings](#method.SetIPSettings) | Sets the IP settings for the given interface |
| [GetStunEndpoint](#method.GetStunEndpoint) | Get the STUN Endpoint that is used to identify public IP of the device |
| [SetStunEndpoint](#method.SetStunEndpoint) | Set the STUN Endpoint to be used to identify public IP of the device |
| [GetConnectivityTestEndpoints](#method.GetConnectivityTestEndpoints) | Gets currently used test endpoints |
| [SetConnectivityTestEndpoints](#method.SetConnectivityTestEndpoints) | This method used to set up to 5 endpoints for a connectivity test |
| [IsConnectedToInternet](#method.IsConnectedToInternet) | Seeks Whether the device has internet connectivity |
| [GetCaptivePortalURI](#method.GetCaptivePortalURI) | Gets the captive portal URI if connected to any captive portal network |
| [StartConnectivityMonitoring](#method.StartConnectivityMonitoring) | Enable a continuous monitoring of internet connectivity with heart beat interval thats given |
| [StopConnectivityMonitoring](#method.StopConnectivityMonitoring) | Stops the connectivity monitoring |
| [GetPublicIP](#method.GetPublicIP) | Gets the internet/public IP Address of the device |
| [Ping](#method.Ping) | Pings the specified endpoint with the specified number of packets |
| [Trace](#method.Trace) | Traces the specified endpoint with the specified number of packets using `traceroute` |
| [StartWiFiScan](#method.StartWiFiScan) | Initiates WiFi scaning |
| [StopWiFiScan](#method.StopWiFiScan) | Stops WiFi scanning |
| [GetKnownSSIDs](#method.GetKnownSSIDs) | Gets list of saved SSIDs |
| [AddToKnownSSIDs](#method.AddToKnownSSIDs) | Saves the SSID, passphrase, and security mode for upcoming and future sessions |
| [RemoveKnownSSID](#method.RemoveKnownSSID) | Remove given SSID from saved SSIDs |
| [WiFiConnect](#method.WiFiConnect) | Initiates request to connect to the specified SSID with the given passphrase |
| [WiFiDisconnect](#method.WiFiDisconnect) | Disconnects from the currently connected SSID |
| [GetConnectedSSID](#method.GetConnectedSSID) | Returns the connected SSID information |
| [StartWPS](#method.StartWPS) | Initiates a connection using Wifi Protected Setup (WPS) |
| [StopWPS](#method.StopWPS) | Cancels the in-progress WPS pairing operation |
| [GetWiFiSignalStrength](#method.GetWiFiSignalStrength) | Get WiFiSignalStrength of connected SSID |
| [GetSupportedSecurityModes](#method.GetSupportedSecurityModes) | Returns the Wifi security modes that the device supports |
| [SetLogLevel](#method.SetLogLevel) | Set Log level for more information |
| [GetWifiState](#method.GetWifiState) | Returns the current Wifi State |


<a name="GetAvailableInterfaces"></a>
## *GetAvailableInterfaces*

Get device supported list of available interface including their state.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.Interfaces | array | <sup>*(optional)*</sup> An interface |
| result?.Interfaces[#] | object | <sup>*(optional)*</sup>  |
| result?.Interfaces[#].type | string | Interface  Type |
| result?.Interfaces[#].name | string | Interface Name. ex: eth0 or wlan0 |
| result?.Interfaces[#].mac | string | Interface MAC address |
| result?.Interfaces[#].isEnabled | boolean | Whether the interface is currently enabled |
| result?.Interfaces[#].isConnected | boolean | Whether the interface is currently connected |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.GetAvailableInterfaces"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "Interfaces": [
            {
                "type": "ETHERNET",
                "name": "eth0",
                "mac": "AA:AA:AA:AA:AA:AA",
                "isEnabled": true,
                "isConnected": true
            }
        ],
        "success": true
    }
}
```

<a name="GetPrimaryInterface"></a>
## *GetPrimaryInterface*

Gets the primary/default network interface for the device. The active network interface is defined as the one that can make requests to the external network. Returns one of the supported interfaces as per `GetAvailableInterfaces`, or an empty value which indicates that there is no default network interface.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.GetPrimaryInterface"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "interface": "wlan0"
    }
}
```

<a name="SetPrimaryInterface"></a>
## *SetPrimaryInterface*

Sets the primary/default interface for the device. This call fails if the interface is not enabled.

### Events

| Event | Description |
| :-------- | :-------- |
| [onActiveInterfaceChange](#onActiveInterfaceChange) | Triggered when device’s default interface changed. |
| [onInterfaceStateChange](#onInterfaceStateChange) | Triggered when interface’s state changed |
| [onAddressChange](#onAddressChange) | Triggered when the device connects to router. |
| [onInternetStatusChange](#onInternetStatusChange) | Triggered when each IP address is lost or acquired. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface |

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
    "method": "org.rdk.NetworkManager.SetPrimaryInterface",
    "params": {
        "interface": "wlan0"
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

<a name="method.SetInterfaceState"></a>
## *SetInterfaceState [<sup>method</sup>](#head.Methods)*

Enable or Disable the specified interface.

### Events

| Event | Description |
| :-------- | :-------- |
| [onInterfaceStateChange](#event.onInterfaceStateChange) | Triggered when interface’s status changes to enabled or disabled. |

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface in `GetAvailableInterfaces` |
| params.enabled | boolean | Set the state of the interface to be Enabled or Disabled |

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
    "method": "org.rdk.NetworkManager.SetInterfaceState",
    "params": {
        "interface": "wlan0",
        "enabled": true
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

<a name="method.GetInterfaceState"></a>
## *GetInterfaceState [<sup>method</sup>](#head.Methods)*

Disable the specified interface.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface in `GetAvailableInterfaces` |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.isEnabled | boolean | Whether the Interface is enabled or disabled |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.GetInterfaceState",
    "params": {
        "interface": "wlan0"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "isEnabled": true,
        "success": true
    }
}
```

<a name="method.GetIPSettings"></a>
## *GetIPSettings [<sup>method</sup>](#head.Methods)*

Gets the IP setting for the given interface.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface |
| params?.ipversion | string | <sup>*(optional)*</sup> either IPv4 or IPv6 |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface |
| result.ipversion | string | either IPv4 or IPv6 |
| result.autoconfig | boolean | `true` if DHCP is used, `false` if IP is configured manually |
| result?.dhcpserver | string | <sup>*(optional)*</sup> The DHCP Server address |
| result.ipaddress | string | The IP address |
| result.prefix | integer | The prefix number |
| result.gateway | string | The gateway address |
| result.v6LinkLocal | string | The v6LinkLocal |
| result.primarydns | string | The primary DNS address |
| result.secondarydns | string | The secondary DNS address |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.GetIPSettings",
    "params": {
        "interface": "wlan0",
        "ipversion": "IPv4"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "interface": "wlan0",
        "ipversion": "IPv4",
        "autoconfig": true,
        "dhcpserver": "192.168.1.1",
        "ipaddress": "192.168.1.101",
        "prefix": 24,
        "gateway": "192.168.1.1",
        "v6LinkLocal": "192.168.1.1",
        "primarydns": "192.168.1.1",
        "secondarydns": "192.168.1.2",
        "success": true
    }
}
```

<a name="SetIPSettings"></a>
## *SetIPSettings*

Sets the IP settings for the given interface.

### Events

| Event | Description |
| :-------- | :-------- |
| [onAddressChange](#onAddressChange) | Triggered when the device connects to router. |
| [onInternetStatusChange](#onInternetStatusChange) | Triggered when each IP address is lost or acquired. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface |
| params.ipversion | string | either IPv4 or IPv6 |
| params.autoconfig | boolean | `true` if DHCP is used, `false` if IP is configured manually |
| params.ipaddress | string | The IP address |
| params.prefix | integer | The prefix number |
| params.gateway | string | The gateway address |
| params.primarydns | string | The primary DNS address |
| params.secondarydns | string | The secondary DNS address |

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
    "method": "org.rdk.NetworkManager.SetIPSettings",
    "params": {
        "interface": "wlan0",
        "ipversion": "IPv4",
        "autoconfig": true,
        "ipaddress": "192.168.1.101",
        "prefix": 24,
        "gateway": "192.168.1.1",
        "primarydns": "192.168.1.1",
        "secondarydns": "192.168.1.2"
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

<a name="GetStunEndpoint"></a>
## *GetStunEndpoint*

Get the STUN Endpoint that is used to identify public IP of the device.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.endPoint | string | STUN server endPoint |
| result.port | integer | STUN server port |
| result.bindTimeout | integer | STUN server bind timeout |
| result.cacheTimeout | integer | STUN server cache timeout |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.GetStunEndpoint"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "endPoint": "stun.l.google.com",
        "port": 3478,
        "bindTimeout": 30,
        "cacheTimeout": 0,
        "success": true
    }
}
```

<a name="SetStunEndpoint"></a>
## *SetStunEndpoint*

Set the STUN Endpoint to be used to identify public IP of the device.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.endPoint | string | STUN server endPoint |
| params.port | integer | STUN server port |
| params?.bindTimeout | integer | <sup>*(optional)*</sup> STUN server bind timeout |
| params?.cacheTimeout | integer | <sup>*(optional)*</sup> STUN server cache timeout |

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
    "method": "org.rdk.NetworkManager.SetStunEndpoint",
    "params": {
        "endPoint": "stun.l.google.com",
        "port": 3478,
        "bindTimeout": 30,
        "cacheTimeout": 0
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

<a name="GetConnectivityTestEndpoints"></a>
## *GetConnectivityTestEndpoints*

Gets currently used test endpoints. on success list out the connectivity test points connections.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.endpoints | array |  |
| result.endpoints[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.GetConnectivityTestEndpoints"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "endpoints": [
            "http://clients3.google.com/generate_204"
        ],
        "success": true
    }
}
```

<a name="SetConnectivityTestEndpoints"></a>
## *SetConnectivityTestEndpoints*

This method used to set up to 5 endpoints for a connectivity test. Successful connections are verified with HTTP Status code 204 (No Content).

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.endpoints | array | A list of endpoints to test |
| params.endpoints[#] | string |  |

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
    "method": "org.rdk.NetworkManager.SetConnectivityTestEndpoints",
    "params": {
        "endpoints": [
            "http://clients3.google.com/generate_204"
        ]
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

<a name="IsConnectedToInternet"></a>
## *IsConnectedToInternet*

Seeks Whether the device has internet connectivity. This API might take up to 3s to validate internet connectivity.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ipversion | string | either IPv4 or IPv6 |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.isConnectedToInternet | boolean | `true` if internet connectivity is detected, otherwise `false` |
| result.internetState | integer | The internet state |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.IsConnectedToInternet",
    "params": {
        "ipversion": "IPv4"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "isConnectedToInternet": true,
        "internetState": 4,
        "success": true
    }
}
```

<a name="GetCaptivePortalURI"></a>
## *GetCaptivePortalURI*

Gets the captive portal URI if connected to any captive portal network.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.uri | string | Captive portal URI |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.GetCaptivePortalURI"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "uri": "http://10.0.0.1/captiveportal.jst",
        "success": true
    }
}
```

<a name="StartConnectivityMonitoring"></a>
## *StartConnectivityMonitoring*

Enable a continuous monitoring of internet connectivity with heart beat interval thats given. If the monitoring is already happening, it will be restarted with new given interval.

### Events

| Event | Description |
| :-------- | :-------- |
| [onInternetStatusChange](#onInternetStatusChange) | Triggered when internet connection state changed. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interval | number | Interval in sec |

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
    "method": "org.rdk.NetworkManager.StartConnectivityMonitoring",
    "params": {
        "interval": 30
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

<a name="StopConnectivityMonitoring"></a>
## *StopConnectivityMonitoring*

Stops the connectivity monitoring.

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
    "method": "org.rdk.NetworkManager.StopConnectivityMonitoring"
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

<a name="GetPublicIP"></a>
## *GetPublicIP*

Gets the internet/public IP Address of the device.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object | it allows empty parameter too |
| params.ipversion | string | either IPv4 or IPv6 |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.publicIP | string | Returns an public ip of the device |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.GetPublicIP",
    "params": {
        "ipversion": "IPv4"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "publicIP": "69.136.49.95",
        "success": true
    }
}
```

<a name="Ping"></a>
## *Ping*

Pings the specified endpoint with the specified number of packets.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.endpoint | string | The host name or IP address |
| params.ipversion | string | either IPv4 or IPv6 |
| params.noOfRequest | integer | The number of packets to send. Default is 15 |
| params.timeout | integer | Timeout |
| params.guid | string | The globally unique identifier |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.target | string | The target IP address |
| result.packetsTransmitted | integer | The number of packets sent |
| result.packetsReceived | integer | The number of packets received |
| result.packetLoss | string | The number of packets lost |
| result.tripMin | string | The minimum amount of time to receive the packets |
| result.tripAvg | string | The average time to receive the packets |
| result.tripMax | string | The maximum amount of time to receive the packets |
| result.tripStdDev | string | The standard deviation for the trip |
| result.error | string | An error message |
| result.guid | string | The globally unique identifier |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.Ping",
    "params": {
        "endpoint": "45.57.221.20",
        "ipversion": "IPv4",
        "noOfRequest": 10,
        "timeout": 30,
        "guid": "..."
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "target": "45.57.221.20",
        "packetsTransmitted": 10,
        "packetsReceived": 10,
        "packetLoss": "0.0",
        "tripMin": "61.264",
        "tripAvg": "130.397",
        "tripMax": "230.832",
        "tripStdDev": "80.919",
        "error": "...",
        "guid": "...",
        "success": true
    }
}
```

<a name="Trace"></a>
## *Trace*

Traces the specified endpoint with the specified number of packets using `traceroute`.

### Events

No Events

### Parameters

| Name            | Type    | Description |
| :--------       | :-------- | :-------- |
| params | object |  |
| params.ipversion| string  | <sup>*(optional)*</sup> The host name or IP address |
| params.endpoint | string  | The host name or IP address |
| params.packets  | integer | <sup>*(optional)*</sup> The number of packets to send. Default is 10 |
| params.guid     | string  | <sup>*(optional)*</sup> The globally unique identifier |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.target | string | The target IP address |
| result.results | string | The results from `traceroute` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.Trace",
    "params": {
        "endpoint": "45.57.221.20",
        "packets": 10
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "target": "45.57.221.20",
        "success": true,
        "results": "<<<traceroute command results>>>"
    }
}
```

<a name="StartWiFiScan"></a>
## *StartWiFiScan*

Initiates WiFi scaning. This method supports scanning for specific range of frequency like 2.4GHz only or 5GHz only or 6GHz only or ALL. When no input passed about the frequency to be scanned, it scans for all. It publishes 'onAvailableSSIDs' event upon completion.

### Events

| Event | Description |
| :-------- | :-------- |
| [onAvailableSSIDs](#onAvailableSSIDs) | Triggered when list of SSIDs is available after the scan completes. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.frequency | string | The frequency to scan. An empty or `null` value scans all frequencies. If a frequency is specified (2.4 or 5.0), then the results are only returned for matching frequencies |

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
    "method": "org.rdk.NetworkManager.StartWiFiScan",
    "params": {
        "frequency": "..."
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

<a name="StopWiFiScan"></a>
## *StopWiFiScan*

Stops WiFi scanning. Any discovered SSIDs from the call to the `StartWiFiScan` method up to the point where this method is called are still returned as 

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
    "method": "org.rdk.NetworkManager.StopWiFiScan"
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

<a name="GetKnownSSIDs"></a>
## *GetKnownSSIDs*

Gets list of saved SSIDs. This method returns all the SSIDs that are saved as array.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.ssids | array | Known SSIDS |
| result.ssids[#] | string |  |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.GetKnownSSIDs"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "ssids": [
            "Xfinity_Guest"
        ],
        "success": true
    }
}
```

<a name="AddToKnownSSIDs"></a>
## *AddToKnownSSIDs*

Saves the SSID, passphrase, and security mode for upcoming and future sessions. This method only adds to the persistent memory; does not disconnect from currently connected SSID.

### Events

No Events

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
    "id": 42,
    "method": "org.rdk.NetworkManager.AddToKnownSSIDs",
    "params": {
        "ssid": "123412341234",
        "passphrase": "password",
        "securityMode": 6
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

<a name="RemoveKnownSSID"></a>
## *RemoveKnownSSID*

Remove given SSID from saved SSIDs. This method just removes from the list and of the list is having only one entry thats being removed, it will initiate a disconnect.

### Events

| Event | Description |
| :-------- | :-------- |
| [onWiFiStateChange](#onWiFiStateChange) | Triggered when Wifi state changes to DISCONNECTED |
| [onAddressChange](#onAddressChange) | Triggered when an IP Address is assigned or lost |
| [onInternetStatusChange](#onInternetStatusChange) | Triggered when internet connection state changed |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ssid | string | The paired SSID |

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
    "method": "org.rdk.NetworkManager.RemoveKnownSSID",
    "params": {
        "ssid": "123412341234"
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

<a name="WiFiConnect"></a>
## *WiFiConnect*

Initiates request to connect to the specified SSID with the given passphrase. Passphrase can be `null` when the network security is `NONE`. When called with no arguments, this method attempts to connect to the saved SSID and password. See `AddToKnownSSIDs`.

### Events

| Event | Description |
| :-------- | :-------- |
| [onWiFiStateChange](#onWiFiStateChange) | Triggered when Wifi state changes to CONNECTING, CONNECTED . |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ssid | string | The paired SSID |
| params.passphrase | string | The access point password |
| params.securityMode | integer | The security mode. See `getSupportedSecurityModes` |
| params.persistSSIDInfo | boolean | <sup>*(optional)*</sup> Option to connect to SSID without persisting AccessPoint details (default: *true*; always persist) |

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
    "method": "org.rdk.NetworkManager.WiFiConnect",
    "params": {
        "ssid": "123412341234",
        "passphrase": "password",
        "securityMode": 6,
        "persistSSIDInfo": true
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

<a name="WiFiDisconnect"></a>
## *WiFiDisconnect*

Disconnects from the currently connected SSID. A event will be posted upon completion.

### Events

| Event | Description |
| :-------- | :-------- |
| [onWIFIStateChange](#onWIFIStateChange) | Triggered when Wifi state changes to DISCONNECTED (only if currently connected). |
| [onAddressChange](#onAddressChange) | Triggered when an IP Address is assigned or lost |
| [onInternetStatusChange](#onInternetStatusChange) | Triggered when internet connection state changed |
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
    "method": "org.rdk.NetworkManager.WiFiDisconnect"
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

<a name="GetConnectedSSID"></a>
## *GetConnectedSSID*

Returns the connected SSID information.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.ssid | string | The paired SSID |
| result.bssid | string | The paired BSSID |
| result.securityMode | string | The security mode. See the `connect` method |
| result.signalStrength | string | The RSSI value in dBm |
| result.frequency | string | The supported frequency for this SSID in GHz |
| result.rate | string | The physical data rate in Mbps |
| result.noise | string | The average noise strength in dBm |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.GetConnectedSSID"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "ssid": "123412341234",
        "bssid": "ff:ff:ff:ff:ff:ff",
        "securityMode": "5",
        "signalStrength": "-27.000000",
        "frequency": "2.442000",
        "rate": "144.000000",
        "noise": "-121.000000",
        "success": true
    }
}
```

<a name="StartWPS"></a>
## *StartWPS*

Initiates a connection using Wifi Protected Setup (WPS). An existing connection will be disconnected before attempting to initiate a new connection. Failure in WPS pairing will trigger an error 

If the `method` parameter is set to `SERIALIZED_PIN`, then RDK retrieves the serialized pin using the Manufacturer (MFR) API. If the `method` parameter is set to `PIN`, then RDK use the pin supplied as part of the request. If the `method` parameter is set to `PBC`, then RDK uses Push Button Configuration (PBC) to obtain the pin.

### Events

| Event | Description |
| :-------- | :-------- |
| [onWIFIStateChange](#onWIFIStateChange) | Triggered when Wifi state changes to DISCONNECTED (only if currently connected), CONNECTING, CONNECTED. |
| [onAddressChange](#onAddressChange) | Triggered when an IP Address is assigned or lost |
| [onInternetStatusChange](#onInternetStatusChange) | Triggered when internet connection state changed |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.method | integer | The method used to obtain the pin (must be one of the following: PBC=0, PIN=1, SERIALIZED_PIN=2) |
| params.wps_pin | string | A valid 8 digit WPS pin number. Use this parameter when the `method` parameter is set to `PIN` |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result?.pin | string | <sup>*(optional)*</sup> The WPS pin value. Valid only when `method` is set to `PIN` or `SERIALIZED_PIN` |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.StartWPS",
    "params": {
        "method": 1,
        "wps_pin": "88888888"
    }
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "pin": "88888888",
        "success": true
    }
}
```

<a name="StopWPS"></a>
## *StopWPS*

Cancels the in-progress WPS pairing operation. The operation forcefully stops the in-progress pairing attempt and aborts the current scan. WPS pairing must be in-progress for the operation to succeed.

### Events

| Event | Description |
| :-------- | :-------- |
| [onWIFIStateChange](#onWIFIStateChange) | Triggered when Wifi state changes to DISCONNECTED. |
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
    "method": "org.rdk.NetworkManager.StopWPS"
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

<a name="GetWiFiSignalStrength"></a>
## *GetWiFiSignalStrength*

Get WiFiSignalStrength of connected SSID.

### Events

| Event | Description |
| :-------- | :-------- |
| [onWiFiSignalStrengthChange](#onWiFiSignalStrengthChange) | Triggered when Wifi signal strength switches between Excellent, Good, Fair, Weak. |
### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.ssid | string | The paired SSID |
| result.signalStrength | string | The RSSI value in dBm |
| result.quality | integer | Signal strength Quality |
| result.success | boolean | Whether the request succeeded |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.GetWiFiSignalStrength"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "ssid": "123412341234",
        "signalStrength": "-27.000000",
        "quality": 123,
        "success": true
    }
}
```

<a name="GetSupportedSecurityModes"></a>
## *GetSupportedSecurityModes*

Returns the Wifi security modes that the device supports.

### Events

No Events

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
    "id": 42,
    "method": "org.rdk.NetworkManager.GetSupportedSecurityModes"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
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

<a name="SetLogLevel"></a>
## *SetLogLevel*

Set Log level for more information. The possible set log level are as follows. 
* `0`: FATAL  
* `1`: ERROR  
* `2`: WARNING  
* `3`: INFO 
* `4`: VERBOSE 
* `5`: TRACE 
.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.logLevel | integer | Set Log level to get more information |

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
    "method": "org.rdk.NetworkManager.SetLogLevel",
    "params": {
        "logLevel": 1
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

<a name="method.GetWifiState"></a>
## *GetWifiState [<sup>method</sup>](#head.Methods)*

Returns the current Wifi State. The possible Wifi states are as follows.
* `0`: UNINSTALLED - The device was in an installed state and was uninstalled; or, the device does not have a Wifi radio installed 
* `1`: DISABLED - The device is installed but not yet enabled
* `2`: DISCONNECTED - The device is installed and enabled, but not yet connected to a network
* `3`: PAIRING - The device is in the process of pairing, but not yet connected to a network
* `4`: CONNECTING - The device is attempting to connect to a network
* `5`: CONNECTED - The device is successfully connected to a network
* `6`: SSID_NOT_FOUND - The requested SSID to connect is not found
* `7`: SSID_CHANGED - The device connected SSID is changed
* `8`: CONNECTION_LOST - The device network connection is lost
* `9`: CONNECTION_FAILED - The device network connection got failed
* `10`: CONNECTION_INTERRUPTED - The device connection is interrupted
* `11`: INVALID_CREDENTIALS - The credentials provided to connect is not valid
* `12`: AUTHENTICATION_FAILED - Authentication process as a whole could not be successfully completed
* `13`: ERROR - The device has encountered an unrecoverable error with the Wifi adapter

### Events

No Events

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
    "id": 42,
    "method": "org.rdk.NetworkManager.GetWifiState"
}
```

#### Response

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "result": {
        "state": 2,
        "success": true
    }
}
```

<a name="Notifications"></a>
# Notifications

Notifications are autonomous events, triggered by the internals of the implementation, and broadcasted via JSON-RPC to all registered observers. Refer to [[Thunder](#Thunder)] for information on how to register for a notification.

The following events are provided by the org.rdk.NetworkManager plugin:

NetworkManager interface events:

| Event | Description |
| :-------- | :-------- |
| [onInterfaceStateChange](#onInterfaceStateChange) | Triggered when an interface state is changed |
| [onAddressChange](#onAddressChange) | Triggered when an IP Address is assigned or lost |
| [onActiveInterfaceChange](#onActiveInterfaceChange) | Triggered when the primary/active interface changes, regardless if it's from a system operation or through the `SetPrimaryInterface` method |
| [onInternetStatusChange](#onInternetStatusChange) | Triggered when internet connection state changed |
| [onAvailableSSIDs](#onAvailableSSIDs) | Triggered when scan completes or when scan cancelled |
| [onWiFiStateChange](#onWiFiStateChange) | Triggered when WIFI connection state get changed |
| [onWiFiSignalStrengthChange](#onWiFiSignalStrengthChange) | Triggered when WIFI connection Signal Strength get changed |


<a name="onInterfaceStateChange"></a>
## *onInterfaceStateChange*

Triggered when an interface state is changed. The possible states are 
* 'INTERFACE_ADDED' 
* 'INTERFACE_LINK_UP' 
* 'INTERFACE_LINK_DOWN' 
* 'INTERFACE_ACQUIRING_IP' 
* 'INTERFACE_REMOVED' 
* 'INTERFACE_DISABLED' 
.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface |
| params.state | string | Current state of the interface |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onInterfaceStateChange",
    "params": {
        "interface": "wlan0",
        "state": "INTERFACE_ADDED"
    }
}
```

<a name="onAddressChange"></a>
## *onAddressChange*

Triggered when an IP Address is assigned or lost.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface |
| params.isIPv6 | boolean | It will be true if the IP address is IPv6 |
| params.ipAddress | string | The IPv6 or IPv4 address for the interface |
| params.status | string | Whether IP address was acquired or lost (must be one of the following: 'ACQUIRED', 'LOST') (must be one of the following: *`ACQUIRED`*, *`LOST`*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onAddressChange",
    "params": {
        "interface": "wlan0",
        "isIPv6": false,
        "ipAddress": "192.168.1.100",
        "status": "ACQUIRED"
    }
}
```

<a name="onActiveInterfaceChange"></a>
## *onActiveInterfaceChange*

Triggered when the primary/active interface changes, regardless if it's from a system operation or through the `SetPrimaryInterface` 

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.oldInterfaceName | string | The previous interface that was changed |
| params.newInterfaceName | string | The current interface |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onActiveInterfaceChange",
    "params": {
        "oldInterfaceName": "wlan0",
        "newInterfaceName": "eth0"
    }
}
```

<a name="onInternetStatusChange"></a>
## *onInternetStatusChange*

Triggered when internet connection state changed.The possible internet connection status are `NO_INTERNET`, `LIMITED_INTERNET`, `CAPTIVE_PORTAL`, `FULLY_CONNECTED`.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.PrevState | integer | The privious internet connection state |
| params.PrevStatus | string | The previous internet connection status |
| params.state | integer | The internet connection state |
| params.status | string | The internet connection status |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onInternetStatusChange",
    "params": {
        "PrevState": 1,
        "PrevStatus": "NO_INTERNET",
        "state": 4,
        "status": "FULLY_CONNECTED"
    }
}
```

<a name="onAvailableSSIDs"></a>
## *onAvailableSSIDs*

Triggered when scan completes or when scan cancelled.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ssids | array | On Available SSID's |
| params.ssids[#] | object |  |
| params.ssids[#].ssid | string | ssid |
| params.ssids[#].security | integer | security |
| params.ssids[#].signalStrength | string | signalStrength |
| params.ssids[#].frequency | string | frequency |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onAvailableSSIDs",
    "params": {
        "ssids": [
            {
                "ssid": "myAP-2.4",
                "security": 6,
                "signalStrength": "-27.000000",
                "frequency": "2.442000"
            }
        ]
    }
}
```

<a name="onWiFiStateChange"></a>
## *onWiFiStateChange*

Triggered when WIFI connection state get changed. The possible states are, 
 * '0' - 'WIFI_STATE_UNINSTALLED' 
 * '1' - 'WIFI_STATE_DISABLED' 
 * '2' - 'WIFI_STATE_DISCONNECTED' 
 * '3' - 'WIFI_STATE_PAIRING' 
 * '4' - 'WIFI_STATE_CONNECTING' 
 * '5' - 'WIFI_STATE_CONNECTED' 
 * '6' - 'WIFI_STATE_SSID_NOT_FOUND' 
 * '7' - 'WIFI_STATE_SSID_CHANGED' 
 * '8' - 'WIFI_STATE_CONNECTION_LOST' 
 * '9' - 'WIFI_STATE_CONNECTION_FAILED' 
 * '10'- 'WIFI_STATE_CONNECTION_INTERRUPTED' 
 * '11' - 'WIFI_STATE_INVALID_CREDENTIALS' 
 * '12' - 'WIFI_STATE_AUTHENTICATION_FAILED' 
 * '13' - 'WIFI_STATE_ERROR' 
.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.state | integer | onWiFiStateChange |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onWiFiStateChange",
    "params": {
        "state": 5
    }
}
```

<a name="onWiFiSignalStrengthChange"></a>
## *onWiFiSignalStrengthChange*

Triggered when WIFI connection Signal Strength get changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ssid | string | Signal Strength changed SSID |
| params.signalLevel | string | Signal Strength |
| params.signalQuality | string | Signal quality |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onWiFiSignalStrengthChange",
    "params": {
        "ssid": "home-new_123",
        "signalLevel": "-27.000000",
        "signalQuality": "Excellent"
    }
}
```

