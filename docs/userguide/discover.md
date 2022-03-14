# Discovering Services

RDK services are uniquely identified using a callsign, which takes the form *org.rdk.service_name*. There is also a core service (the Controller service) that manages services in the Thunder framework. The following example sends a request to the Controller service and uses the status method to return information about all registered services including their service callsign. Replace *host_address* with the IP address of your device.

```bash
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "Controller.1.status"}' http://host_address:9998/jsonrpc
```

**Note**: You can access the Controller service directly from a browser using the same URL with the /`Service/Controller/` context (`http://host_address:9998/Service/Controller`). There is also a Controller UI available at `http://host_address:9998/`.

Once you discover the service callsigns, you can request the status of individual services. The following example uses the Controller service to requests the status of the DisplaySettings service using the `org.rdk.DisplaySettings` callsign:

**Request**

```bash
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method":"Controller.1.status@org.rdk.DisplaySettings"}' http://host_address:9998/jsonrpc
```

**Response**

```javascript
{
   "jsonrpc":"2.0",
   "id":3,
   "result":[
       {
        "callsign":"org.rdk.DisplaySettings",
        "locator":"libWPEFrameworkDisplaySettings.so",
        "classname":"DisplaySettings",
        "autostart":false,
        "precondition":["Platform"],
        "state":"deactivated",
        "processedrequests":0,
        "processedobjects":0,
        "observers":0,
        "module":"DisplaySettings",
        "hash":"engineering_build_for_debug_purpose_only"
        }
    ]
}
```
