-----------------
# UnifiedPlayer

## Versions
`org.rdk.UnifiedPlayer.1`

## Methods:
```
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.UnifiedPlayer.1.getApiVersionNumber"}' http://10.0.0.28:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.UnifiedPlayer.1.open", "params":{ "mediaurl": "http://10.0.0.10:8080/RDK-V/Streams/prog_5001.ts", "mode": "MODE_PLAYBACK", "manage": "MANAGE_NONE", "casinitdata": ""}}' http://10.0.0.28:9998/jsonrpc



curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.UnifiedPlayer.1.getsubtitlesenabled"}' http://10.0.0.28:9998/jsonrpc


curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.UnifiedPlayer.1.destroy"}' http://10.0.0.28:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.UnifiedPlayer.1.pause"}' http://10.0.0.28:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.UnifiedPlayer.1.stop"}' http://10.0.0.28:9998/jsonrpc


curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.UnifiedPlayer.1.play"}' http://10.0.0.28:9998/jsonrpc


curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.UnifiedPlayer.1.getposition"}' http://10.0.0.28:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.UnifiedPlayer.1.getspeed"}' http://10.0.0.28:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.UnifiedPlayer.1.getduration"}' http://10.0.0.28:9998/jsonrpc


curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.UnifiedPlayer.1.getavailableaudiolanguages"}' http://10.0.0.28:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.UnifiedPlayer.1.setposition", "params": {"position": 1000.0}}' http://10.0.0.28:9998/jsonrpc
```
## Responses
```
{"jsonrpc":"2.0","id":3,"result":{"success":true}}
```

## Events
```
onplayerinitialized
onmediaopened
onplaying
onpaused
onclosed
oncomplete
onstatus
onprogress
onwarning
onerror
```