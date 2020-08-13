-----------------
# AVInput

## Versions
`org.rdk.AVInput.1`

## Methods:
```
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.AVInput.1.getApiVersionNumber"}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.AVInput.1.numberOfInputs"}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.AVInput.1.currentVideoMode"}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.AVInput.1.contentProtected"}' http://127.0.0.1:9998/jsonrpc
```
## Responses
```
{"jsonrpc":"2.0","id":3,"result":{"version":1,"success":true}}
{"jsonrpc":"2.0","id":3,"result":{"numberOfInputs":1,"success":true}]
{"jsonrpc":"2.0","id":3,"result":{"currentVideoMode":"unknownp","success":true}}
{"jsonrpc":"2.0","id":3,"result":{"isContentProtected":true,"success":true}}
```

## Events
```
onAVInputActive
onAVInputInactive
```
## Events logged
```
onAVInputActive: Notify onAVInputActive {"url":"avin://input0"}
onAVInputInactive: Notify onAVInputInactive {"url":"avin://input0"}
```

## Full Reference
https://wiki.rdkcentral.com/display/RDK/AV+Input

