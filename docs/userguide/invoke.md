# Invoking Services

RDK service methods are invoked using the service name/callsign, the service version, and the method name together with any required parameters.

The following example uses the cURL command-line tool to demonstrate making a call to the DeviceInfo service and uses the `systeminfo` method which returns basic device information. The call is made to the default HTTP server running on the device.

**Request**

```bash
curl -H "Content-Type: application/json" -X POST -d '{"jsonrpc":"2.0","id":"3","method": "DeviceInfo.1.systeminfo"}' http://host_address:9998/jsonrpc
```

Replace *host_address* with the IP address of your device.

**Response**

```javascript
{"jsonrpc":"2.0","id":3,"result": {
    "version":"1.0.#0105e8f8ed2716fa2bff6f897e3538d922455ed0",
    "uptime":80454,
    "totalram":639639552,
    "freeram":434946048,
    "devicename":"raspberrypi-rdk-mc",
    "cpuload":"2",
    "serialnumber":"OEuCfXXXXX",
    "time":"Thu, 08 Oct 2020 16:55:58"
    }
}
```
