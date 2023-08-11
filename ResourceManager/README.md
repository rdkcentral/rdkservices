-----------------
# ResourceManager

## Versions
`org.rdk.ResourceManager.1`

## Methods:
```
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.ResourceManager.setAVBlocked", "params": {"appid":"Netflix-0", "blocked":true}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.ResourceManager.getBlockedAVApplications"}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.ResourceManager.reserveTTSResource", "params": {"appid":"Netflix-0"}}' http://127.0.0.1:9998/jsonrpc
```

## Responses
```
setAVBlocked:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}
getBlockedAVApplications:
{"jsonrpc":"2.0","id":3,"result":{"clients":["Netflix-0],"success":true}}
reserveTTSResource:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}
```

## Events
```
none
```
## Full Reference
https://etwiki.sys.comcast.net/display/RDKV/RDK+Resource+Manager
