StateObserver
===============
***@mainpage StateObserver***

This plugin provides the interface to get the value of various device properties and set up event listeners 
to be notified when the state of the device changes.


**API's:**
- getValues
- registerListeners
- unregisterListeners
- getApiVersionNumber
- setApiVersionNumber
- getName

## Build:
```
bitbake thunder-plugins
```

>Note: Below listed curl requests contains sample values for testing purposes only.


### getValues :
This API takes a property or an array of properties as input and returns the state and error values of the same.Returns the property values and a success true or false.
```
Request:(example)
curl -d '{"jsonrpc":"2.0","id":"3","method": "com.comcast.StateObserver.1.getValues" ,"params":{"PropertyNames":["com.comcast.channel_map","com.comcast.tune_ready"]}}' http://127.0.0.1:9998/jsonrpc
Response:
{"jsonrpc":"2.0","id":3,"result":{"properties":[{"PropertyName":"com.comcast.channel_map","state":2},{"PropertyName":"com.comcast.tune_ready","state":1}],"success":true}}
```



### registerListeners :
This API is used to register listeners to a properties so that any state change occurring in these registered properties will be notified.These properties will be added to a registered properties list.
It internally calls the getValues API and hence it returns current value of those properties.
```
Request:(example)
curl -d '{"jsonrpc":"2.0","id":"3","method": "com.comcast.StateObserver.1.registerListeners" ,"params":{"PropertyNames":["com.comcast.channel_map"]}}' http://127.0.0.1:9998/jsonrpc
Response:
{"jsonrpc":"2.0","id":3,"result":{"properties":[{"PropertyName":"com.comcast.channel_map","state":2}],"success":true}}
```

### unregisterListeners :
Unregisters the listeners allocated to the properties.The properties will be removed from the registered properties list.
```
Request:(example)
curl -d '{"jsonrpc":"2.0","id":"3","method": "com.comcast.StateObserver.1.unregisterListeners" ,"params":{"PropertyNames":["com.comcast.channel_map"]}}' http://127.0.0.1:9998/jsonrpc
Response:
 {"jsonrpc":"2.0","id":3,"result":{"success":true}}
```

### getApiVersionNumber :
This API is used to get the api verison number.
```
Request:
curl -d '{"jsonrpc":"2.0","id":"3","method": "com.comcast.StateObserver.1.getApiVersionNumber"}' http://127.0.0.1:9998/jsonrpc
Response:
{"jsonrpc":"2.0","id":3,"result":{"version":1,"success":true}}
```

### setApiVersionNumber :
This API is used to set the api verison number.
```
Request:
curl -d '{"jsonrpc":"2.0","id":"3","method": "com.comcast.StateObserver.1.setApiVersionNumber","params":{"version":"2"}}' http://127.0.0.1:9998/jsonrpc
Response:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}
```

### getName :
This API is used to get the plugin name.
```
Request:
curl -d '{"jsonrpc":"2.0","id":"3","method": "com.comcast.StateObserver.1.getName"}' http://127.0.0.1:9998/jsonrpc
Response:
{"jsonrpc":"2.0","id":3,"result":{"Name":"com.comcast.stateObserver","success":true}}
```


## Return Values:
Property state values are returned in the json response.
If any error is present in the property that also is passed in the response:

PROPERTY                       | ERROR CODES |
-------------------------------|------------ |
com.comcast.channel_map        | RDK-03005   |    
com.comcast.card.disconnected  | RDK-03007   |    
com.comcast.cmac               | RDK-03002   |
com.comcast.time_source        | RDK-03006   |
com.comcast.estb_ip            | RDK-03009   |
com.comcast.ecm_ip             | RDK-03004   |
com.comcast.dsg_ca_tunnel      | RDK-03003   |
com.comcast.cable_card         | RDK-03001   |


Along with property values ,success value is also passed in the response .If its  true it indicates API executed correctly and if its false it indicates 
API failure.



