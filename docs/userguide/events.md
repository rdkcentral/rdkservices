# Using Events

Many RDK services can emit events. Clients must register for events to receive event notifications. Event registration is performed using the `register` method. Event de-registration is performed using the `unregister` method. Both methods are available on all services that support events.

The following example demonstrates registering for the `activeInputChanged` event that is available on the DisplaySettings service.

```bash
curl -H "Content-Type: application/json" -X POST -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.DisplaySettings.1.register", "params": {"event":"activeInputChanged", "id": "client.events.1"}}' http://127.0.0.1:9998/jsonrpc;
{"jsonrpc":"2.0","id":3,"result":0}
```

Clients receive events over web sockets. The following example demonstrates an event.

```javascript
{
    "jsonrpc":"2.0",
    "method":"client.events.1.activeInputChanged",
    "params":{
        "tvinput":false
    }
}
```
