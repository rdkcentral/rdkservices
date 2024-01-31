<!-- Generated automatically, DO NOT EDIT! -->
<a name="head.NetworkManager_Plugin"></a>
# NetworkManager Plugin

**Version: [1.0.0]()**

A NetworkManager plugin for Thunder framework.

### Table of Contents

- [Abbreviation, Acronyms and Terms](#head.Abbreviation,_Acronyms_and_Terms)
- [Description](#head.Description)
- [Configuration](#head.Configuration)
- [Interfaces](#head.Interfaces)
- [Methods](#head.Methods)
- [Notifications](#head.Notifications)

<a name="head.Abbreviation,_Acronyms_and_Terms"></a>
# Abbreviation, Acronyms and Terms

[[Refer to this link](userguide/aat.md)]

<a name="head.Description"></a>
# Description

A Unified `NetworkManager` plugin that allows you to manage Ethernet and Wifi interfaces on the device.
The plugin is designed to be an out-of-process service to be handled by Thunder framework. For more information about the framework refer to [[Thunder](#ref.Thunder)].

<a name="head.Configuration"></a>
# Configuration

The table below lists configuration options of the plugin.

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| callsign | string | Plugin instance name,  *org.rdk.NetworkManager* |
| classname | string | Class name: *NetworkManager* |
| locator | string | Library name: *libWPEFrameworkNetworkManager.so* |
| autostart | boolean | Determines if the plugin shall be started automatically along with the framework |

<a name="head.Interfaces"></a>
# Interfaces

This plugin implements the following interfaces:

- [NetworkManager.json](https://github.com/gururaajar/rdkservices/blob/sprint/2401/NetworkManager/service/NetworkManager.json)

<a name="head.Methods"></a>
# Methods

The following methods are provided by the org.rdk.NetworkManager plugin:

NetworkManager interface methods:

| Method | Description |
| :-------- | :-------- |
| [GetAvailableInterfaces](#method.GetAvailableInterfaces) | Get device supported list of available interface including their state |
| [GetPrimaryInterface](#method.GetPrimaryInterface) | Gets the Primary/default network interface for the device |
| [SetPrimaryInterface](#method.SetPrimaryInterface) | Sets the Primary/default interface for the device |
| [GetIPSettings](#method.GetIPSettings) | Gets the IP setting for the given interface |
| [SetIPSettings](#method.SetIPSettings) | Sets the IP settings for the given interface |
| [GetStunEndpoint](#method.GetStunEndpoint) | Get the STUN Endpoint that is used to identify public IP of the device |
| [SetStunEndPoint](#method.SetStunEndPoint) | Set the STUN Endpoint to be used to identify public IP of the device |
| [GetConnectivityTestEndpoints](#method.GetConnectivityTestEndpoints) | Gets currently used connectivity test endpoints |
| [SetConnectivityTestEndpoints](#method.SetConnectivityTestEndpoints) | Sets connectivity test endpoints |
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
| [SetLogLevel](#method.SetLogLevel) | To set the log level to the plugin for debugging purpose |


<a name="method.GetAvailableInterfaces"></a>
## *GetAvailableInterfaces [<sup>method</sup>](#head.Methods)*

Get Device supported list of available interface including their state.

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
| result?.Interfaces[#].type | string | Interface Type |
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

<a name="method.GetPrimaryInterface"></a>
## *GetPrimaryInterface [<sup>method</sup>](#head.Methods)*

Gets the Primary/default network interface. The active network interface is defined as the one that can make requests to the external network. Returns one of the supported interfaces as per `GetAvailableInterfaces`, or an empty value which indicates that there is no default network interface.

### Events

No Events

### Parameters

This method takes no parameters.

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface in `GetAvailableInterfaces` |

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

<a name="method.SetPrimaryInterface"></a>
## *SetPrimaryInterface [<sup>method</sup>](#head.Methods)*

Sets the primary/default interface to be used for external/internet communication. This call fails if the interface is not enabled.

### Events

| Event | Description |
| :-------- | :-------- |
| [onActiveInterfaceChanged](#event.onActiveInterfaceChanged) | Triggered when device’s default interface changed. |
| [onInterfaceStateChanged](#event.onInterfaceStateChanged) | Triggered when interface’s state changed |
| [onIPAddressChanged](#event.onIPAddressChanged) | Triggered when the device connects to router. |
| [onInternetStatusChanged](#event.onInternetStatusChanged) | Triggered when each IP address is lost or acquired. |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface in `GetAvailableInterfaces` |

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
        "primarydns": "192.168.1.1",
        "secondarydns": "192.168.1.2",
        "success": true
    }
}
```

<a name="method.SetIPSettings"></a>
## *SetIPSettings [<sup>method</sup>](#head.Methods)*

Sets the IP settings.

### Events

| Event | Description |
| :-------- | :-------- |
| [onInternetStatusChanged](#event.onInternetStatusChanged) | Triggered when each IP address is lost or acquired. |
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

<a name="method.GetStunEndpoint"></a>
## *GetStunEndpoint [<sup>method</sup>](#head.Methods)*

Gets the Stun Endpoint used in getPublicIP to indentify the internet/public IP.

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

<a name="method.SetStunEndPoint"></a>
## *SetStunEndPoint [<sup>method</sup>](#head.Methods)*

Sets the Stun Endpoint used for getPublicIP to indentify the internet/public IP.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.endPoint | string | STUN server endPoint |
| params.port | integer | STUN server port |
| params.bindTimeout | integer  | <sup>*(optional)*</sup> STUN server bind timeout |
| params.cacheTimeout | integer | <sup>*(optional)*</sup> STUN server cache timeout |

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
    "method": "org.rdk.NetworkManager.SetStunEndPoint",
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

<a name="method.GetConnectivityTestEndpoints"></a>
## *GetConnectivityTestEndpoints [<sup>method</sup>](#head.Methods)*

Get connectivity test endpoints. on success list out the connectivity test points connections.

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

<a name="method.SetConnectivityTestEndpoints"></a>
## *SetConnectivityTestEndpoints [<sup>method</sup>](#head.Methods)*

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

<a name="method.IsConnectedToInternet"></a>
## *IsConnectedToInternet [<sup>method</sup>](#head.Methods)*

Whether the device has internet connectivity. This API might take up to 3s to validate internet connectivity.

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
        "internetState": 2,
        "success": true
    }
}
```

<a name="method.GetCaptivePortalURI"></a>
## *GetCaptivePortalURI [<sup>method</sup>](#head.Methods)*

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

<a name="method.StartConnectivityMonitoring"></a>
## *StartConnectivityMonitoring [<sup>method</sup>](#head.Methods)*

This method enables continuous monitoring of internet connectivity with heart beat interval thats given. If the monitoring is already happening, it will be restarted with new given interval.

### Events

| Event | Description |
| :-------- | :-------- |
| [onInternetStatusChanged](#event.onInternetStatusChanged) | Triggered when internet connection state changed. |
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

<a name="method.StopConnectivityMonitoring"></a>
## *StopConnectivityMonitoring [<sup>method</sup>](#head.Methods)*

This method stops the continuous internet connectivity monitoring

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

<a name="method.GetPublicIP"></a>
## *GetPublicIP [<sup>method</sup>](#head.Methods)*

It allows either zero parameter or with only interface and ipversion parameter to determine public ip address.

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

<a name="method.Ping"></a>
## *Ping [<sup>method</sup>](#head.Methods)*

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
| params.timeout | integer | timeout |
| params.guid | string | <sup>*(optional)*</sup> The globally unique identifier |


### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.target | string | The target IP address |
| result.success | boolean | Whether the request succeeded |
| result.packetsTransmitted | integer | The number of packets sent |
| result.packetsReceived | integer | The number of packets received |
| result.packetLoss | string | The number of packets lost |
| result.tripMin | string | The minimum amount of time to receive the packets |
| result.tripAvg | string | The average time to receive the packets |
| result.tripMax | string | The maximum amount of time to receive the packets |
| result.tripStdDev | string | The standard deviation for the trip |
| result.error | string | An error message |
| result.guid | string | The globally unique identifier |

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
        "success": true,
        "packetsTransmitted": 10,
        "packetsReceived": 10,
        "packetLoss": "0.0",
        "tripMin": "61.264",
        "tripAvg": "130.397",
        "tripMax": "230.832",
        "tripStdDev": "80.919",
        "error": "...",
        "guid": "..."
    }
}
```

<a name="method.Trace"></a>
## *Trace [<sup>method</sup>](#head.Methods)*

Traces the specified endpoint with the specified number of packets using `traceroute`.

### Events

No Events

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.endpoint | string | The host name or IP address |
| params.ipversion | string | either IPv4 or IPv6 |
| params.noOfRequest | integer | The number of packets to send. Default is 15 |
| params.guid | string | The globally unique identifier |

### Result

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| result | object |  |
| result.target | string | The target IP address |
| result.success | boolean | Whether the request succeeded |
| result.packetsTransmitted | integer | The number of packets sent |
| result.packetsReceived | integer | The number of packets received |
| result.packetLoss | string | The number of packets lost |
| result.tripMin | string | The minimum amount of time to receive the packets |
| result.tripAvg | string | The average time to receive the packets |
| result.tripMax | string | The maximum amount of time to receive the packets |
| result.tripStdDev | string | The standard deviation for the trip |
| result.error | string | An error message |
| result.guid | string | The globally unique identifier |

### Example

#### Request

```json
{
    "jsonrpc": "2.0",
    "id": 42,
    "method": "org.rdk.NetworkManager.Trace",
    "params": {
        "endpoint": "45.57.221.20",
        "ipversion": "IPv4",
        "noOfRequest": 10,
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
        "success": true,
        "error": "...",
        "results": "<<<traceroute command results>>>"
    }
}
```

<a name="method.StartWiFiScan"></a>
## *StartWiFiScan [<sup>method</sup>](#head.Methods)*

This  asynchronous method is to start wifi scaning for available SSIDs. This method supports scanning for specific range of frequency like 2.4GHz only or 5GHz only or 6GHz only or ALL. When no input passed about the frequency to be scanned, it scans for all. It publishes `onAvailableSSIDs` event upon completion.

### Events

| Event | Description |
| :-------- | :-------- |
| [onAvailableSSIDs](#event.onAvailableSSIDs) | Triggered when list of SSIDs is available after the scan completes. |
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

<a name="method.StopWiFiScan"></a>
## *StopWiFiScan [<sup>method</sup>](#head.Methods)*

Stops scanning for SSIDs. Any discovered SSIDs from the call to the `StartWiFiScan` method up to the point where this method is called are still returned as event.

### Events

| Event | Description |
| :-------- | :-------- |
| [onAvailableSSIDs](#event.onAvailableSSIDs) | Triggered with list of SSIDs is available so far when the scan stopped. |

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

<a name="method.GetKnownSSIDs"></a>
## *GetKnownSSIDs [<sup>method</sup>](#head.Methods)*

When the device successfully connected to SSID, it persisted in the device memory to get the device connected across reboots. This method returns all the SSIDs that are saved as array.

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
            "Xfinity_Guest",
            "MyHome-2.4"
        ],
        "success": true
    }
}
```

<a name="method.AddToKnownSSIDs"></a>
## *AddToKnownSSIDs [<sup>method</sup>](#head.Methods)*

This method adds given SSID, passphrase, and security mode to the Known SSIDs list and persists for future sessions. This method only adds to the persistent memory; does not disconnect from currently connected SSID.

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
        "ssid": "Testing-5GHz",
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

<a name="method.RemoveKnownSSID"></a>
## *RemoveKnownSSID [<sup>method</sup>](#head.Methods)*

Removes given ssid from saved ssids list. This method just removes from the list and of the list is having only one entry thats being removed, it will initiate a disconnect.

### Events


| Event | Description |
| :-------- | :-------- |
| [onWiFiStateChanged](#event.onWiFiStateChanged) | Triggered when Wifi state changes to DISCONNECTED |
| [onIPAddressChanged](#event.onIPAddressChanged) | Triggered when an IP Address is assigned or lost |
| [onInternetStatusChange](#event.onInternetStatusChange) | Triggered when internet connection state changed |

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

<a name="method.WiFiConnect"></a>
## *WiFiConnect [<sup>method</sup>](#head.Methods)*

Attempts to connect to the specified SSID with the given passphrase. Passphrase can be `null` when the network security is `NONE`.
When called with no arguments, this method attempts to connect to the saved SSID and password. See `AddToKnownSSIDs`.

### Events

| Event | Description |
| :-------- | :-------- |
| [onWiFiStateChanged](#event.onWiFiStateChanged) | Triggered when Wifi state changes to CONNECTING, CONNECTED . |
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
    "method": "org.rdk.NetworkManager.WiFiConnect",
    "params": {
        "ssid": "Testing-5GHz",
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

<a name="method.WiFiDisconnect"></a>
## *WiFiDisconnect [<sup>method</sup>](#head.Methods)*

This asynchronous method initiates disconnects to the currently connected SSID. A event will be posted upon completion.

### Events

| Event | Description |
| :-------- | :-------- |
| [onWIFIStateChanged](#event.onWIFIStateChanged) | Triggered when Wifi state changes to DISCONNECTED (only if currently connected). |
| [onIPAddressChanged](#event.onIPAddressChanged) | Triggered when an IP Address is assigned or lost |
| [onInternetStatusChange](#event.onInternetStatusChange) | Triggered when internet connection state changed |

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

<a name="method.GetConnectedSSID"></a>
## *GetConnectedSSID [<sup>method</sup>](#head.Methods)*

This method returns information regarding currently connected SSID.

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

<a name="method.StartWPS"></a>
## *StartWPS [<sup>method</sup>](#head.Methods)*

Initiates a connection using Wifi Protected Setup (WPS). An existing connection will be disconnected before attempting to initiate a new connection. Failure in WPS pairing will trigger an error event.

If the `method` parameter is set to `SERIALIZED_PIN`, then RDK retrieves the serialized pin using the Manufacturer (MFR) API. If the `method` parameter is set to `PIN`, then RDK use the pin supplied as part of the request. If the `method` parameter is set to `PBC`, then RDK uses Push Button Configuration (PBC) to obtain the pin.

### Events

| Event | Description |
| :-------- | :-------- |
| [onWIFIStateChanged](#event.onWIFIStateChanged) | Triggered when Wifi state changes to DISCONNECTED (only if currently connected), CONNECTING, CONNECTED. |
| [onIPAddressChanged](#event.onIPAddressChanged) | Triggered when an IP Address is assigned or lost |
| [onInternetStatusChange](#event.onInternetStatusChange) | Triggered when internet connection state changed |
### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.method | string | The method used to obtain the pin (must be one of the following: *PBC*, *PIN*, *SERIALIZED_PIN*) |
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
        "method": "PIN",
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

<a name="method.StopWPS"></a>
## *StopWPS [<sup>method</sup>](#head.Methods)*

Cancels the in-progress WPS pairing operation. The operation forcefully stops the in-progress pairing attempt and aborts the current scan. WPS pairing must be in-progress for the operation to succeed.

### Events

| Event | Description |
| :-------- | :-------- |
| [onWIFIStateChanged](#event.onWIFIStateChanged) | Triggered when Wifi state changes to DISCONNECTED. |
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

<a name="method.GetWiFiSignalStrength"></a>
## *GetWiFiSignalStrength [<sup>method</sup>](#head.Methods)*

Get WiFiSignalStrength of connected SSID.

### Events

| Event | Description |
| :-------- | :-------- |
| [onWiFiSignalStrengthChanged](#event.onWiFiSignalStrengthChanged) | Triggered when Wifi signal strength switches between Excellent, Good, Fair, Weak. |
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

<a name="method.GetSupportedSecurityModes"></a>
## *GetSupportedSecurityModes [<sup>method</sup>](#head.Methods)*

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

<a name="method.SetLogLevel"></a>
## *SetLogLevel [<sup>method</sup>](#head.Methods)*

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
| params.loglevel | integer | Set Log level to get more information |

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
        "loglevel": 1
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

The following events are provided by the org.rdk.NetworkManager plugin:

NetworkManager interface events:

| Event | Description |
| :-------- | :-------- |
| [onInterfaceStateChanged](#event.onInterfaceStateChanged) | Triggered when an interface becomes enabled or disabled |
| [onIPAddressChanged](#event.onIPAddressChanged) | Triggered when an IP Address is assigned or lost |
| [onActiveInterfaceChanged](#event.onActiveInterfaceChanged) | Triggered when the ac interface changes, regardless if it's from a system operation or through the `setDefaultInterface` method |
| [onInternetStatusChange](#event.onInternetStatusChange) | Triggered when internet connection state changed |
| [onAvailableSSIDs](#event.onAvailableSSIDs) | Triggered when got for scan |
| [onWiFiStateChanged](#event.onWiFiStateChanged) | Triggered when WIFI connection state get changed |
| [onWiFiSignalStrengthChanged](#event.onWiFiSignalStrengthChanged) | Triggered when WIFI connection Signal Strength get changed |


<a name="event.onInterfaceStateChanged"></a>
## *onInterfaceStateChanged [<sup>event</sup>](#head.Notifications)*

Triggered when an interface becomes enabled or disabled.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface |
| params.state | string | Whether the interface is enabled (`true`) or disabled (`false`) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onInterfaceStateChanged",
    "params": {
        "interface": "wlan0",
        "state": "Interface_ADDED"
    }
}
```

<a name="event.onIPAddressChanged"></a>
## *onIPAddressChanged [<sup>event</sup>](#head.Notifications)*

Triggered when an IP Address is assigned or lost.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.interface | string | An interface, such as `eth0` or `wlan0`, depending upon availability of the given interface |
| params.ipv6 | string | The IPv6 address for the interface |
| params?.ipv4 | string | <sup>*(optional)*</sup> The IPv4 address for the interface |
| params.status | string | Whether IP address was acquired or lost (must be one of the following: *`ACQUIRED`*, *`LOST`*) |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onIPAddressChanged",
    "params": {
        "interface": "wlan0",
        "ipv6": "2001:0xx8:85a3:0000:0000:8x2x:0370:7334",
        "ipv4": "192.168.1.2",
        "status": "ACQUIRED"
    }
}
```

<a name="event.onActiveInterfaceChanged"></a>
## *onActiveInterfaceChanged [<sup>event</sup>](#head.Notifications)*

Triggered when the ac interface changes, regardless if it's from a system operation or through the `setDefaultInterface` method.

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
    "method": "client.events.onActiveInterfaceChanged",
    "params": {
        "oldInterfaceName": "ETHERNET",
        "newInterfaceName": "WIFI"
    }
}
```

<a name="event.onInternetStatusChange"></a>
## *onInternetStatusChange [<sup>event</sup>](#head.Notifications)*

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
        "PrevState": 0,
        "PrevStatus": "NO_INTERNET",
        "state": 0,
        "status": "NO_INTERNET"
    }
}
```

<a name="event.onAvailableSSIDs"></a>
## *onAvailableSSIDs [<sup>event</sup>](#head.Notifications)*

Triggered when got for scan.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ssids | string | On Available SSID's |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onAvailableSSIDs",
    "params": {
        "ssids": "..."
    }
}
```

<a name="event.onWiFiStateChanged"></a>
## *onWiFiStateChanged [<sup>event</sup>](#head.Notifications)*

Triggered when WIFI connection state get changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.state | integer | onWiFiStateChanged |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onWiFiStateChanged",
    "params": {
        "state": 3
    }
}
```

<a name="event.onWiFiSignalStrengthChanged"></a>
## *onWiFiSignalStrengthChanged [<sup>event</sup>](#head.Notifications)*

Triggered when WIFI connection Signal Strength get changed.

### Parameters

| Name | Type | Description |
| :-------- | :-------- | :-------- |
| params | object |  |
| params.ssid | string | Signal Strength changed SSID |
| params.signalStrength | string | Signal Strength |
| params.quality | string | Signal quality |

### Example

```json
{
    "jsonrpc": "2.0",
    "method": "client.events.onWiFiSignalStrengthChanged",
    "params": {
        "ssid": "comcast123",
        "signalStrength": "...",
        "quality": "..."
    }
}
```

