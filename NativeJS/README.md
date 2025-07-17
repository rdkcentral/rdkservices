-----------------
# NativeJS

## Versions
`org.rdk.jsruntime.1`

## Properties:
```
## Methods:
```
TO laucnch the application
curl --header "Content-Type: application/json"  -H "$token" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.jsruntime.1.launchApplication", "params":{"url":"http://127.0.0.1:50050/demo/helloworld.js"}}' http://127.0.0.1:9998/jsonrpc

Destory the application
curl --header "Content-Type: application/json" -H "$token" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.jsruntime.1.destroyApplication", "params":{"url":"http://127.0.0.1:50050/demo/player.js"}' http://127.0.0.1:9998/jsonrpc

To play video content on display, pass player in options parameter
curl --header "Content-Type: application/json"  -H "$token" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.jsruntime.1.launchApplication", "params":{"url":"http://127.0.0.1:50050/demo/helloworld.js", "options": "player"}}' http://127.0.0.1:9998/jsonrpc
```
## Responses
```
{"jsonrpc":"2.0","id":3,"result":null}

```

## Events
```
none
```

## Full Reference
