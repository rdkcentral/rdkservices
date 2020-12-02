-----------------
# UsbAccess

## Versions
`org.rdk.UsbAccess.1`

## Methods:
```
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.UsbAccess.1.getFileList","params":{}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.UsbAccess.1.createLink","params":{}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.UsbAccess.1.clearLink","params":{}}' http://127.0.0.1:9998/jsonrpc
```

## Responses
```
{"jsonrpc":"2.0","id":3,"result":{"success":true}}
{"jsonrpc":"2.0","id":3,"result":{"success":true}}
{"jsonrpc":"2.0","id":3,"result":{"success":true}}
```

## Events
```
onUSBMountChanged
```

## Full Reference
https://etwiki.sys.comcast.net/pages/viewpage.action?spaceKey=RDKV&title=UsbAccess