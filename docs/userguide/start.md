# Starting Services

Some RDK services are configured to start automatically, while other services are deactivated and must be explicitly started. The Controller service can be used to activate (as well as deactivate) services. The following example starts the DisplaySettings service using the `activate` method. The service callsign is included as a parameter:

**Request**

```bash
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "Controller.1.activate", "params": {"callsign":"org.rdk.DisplaySettings"}}' http://host_address:9998/jsonrpc
```

**Response**

```javascript
{
   "jsonrpc":"2.0",
   "id":3,"result":null
}
```

You can now invoke methods on the DisplaySettings service. The following example demonstrates using the `getConnectedVideoDisplays` method to check if a display is connected to a video port on the device.

**Request**

```bash
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.DisplaySettings.1.getConnectedVideoDisplays"}' http://host_address:9998/jsonrpc
```

**Response**

```javascript
{
   "jsonrpc":"2.0",
   "id":3,
   "result":{
      "connectedVideoDisplays":["HDMI0"],
      "success":true
   }
}
```
