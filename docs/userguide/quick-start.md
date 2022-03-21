# Quick Start

Most RDK devices include several RDK service that are activated by default. For this quick start, we will use the `DeviceInfo` service to demonstrate calling an RDK service.

## Prerequisites

To complete this quick start, you must have the following:

* An RDK-based device running RDK version 4 or higher
* The cURL tool

If you do not have access to an RDK-based device, you can use the RDK reference implementation for the Raspberry Pi. Download the reference implementation using the [Startup Guide](https://developer.rdkcentral.com/firebolt/documentation/build-apps/web-apps/startup-guide/?v=0.6) instructions, which also includes setup instructions.

## Calling an RDK Service

The following example uses the cURL command-line tool to send a JSON request to the Thunder HTTP server. The following request invokes the `DeviceInfo` service and uses the `systeminfo` method which returns basic device information.

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
