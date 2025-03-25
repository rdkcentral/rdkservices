-----------------
# UsbAccess

## Versions
`org.rdk.UsbAccess.1`

## Methods:
```
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.UsbAccess.1.getFileList","params":{"path":"www"}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.UsbAccess.1.createLink","params":{}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.UsbAccess.1.clearLink","params":{}}' http://127.0.0.1:9998/jsonrpc
```

## Responses
```
{"jsonrpc":"2.0","id":3,"result":{"contents":[{"name":"var","t":"f"},{"name":"..","t":"d"},{"name":"pages","t":"d"},{"name":"logs","t":"f"},{"name":".","t":"d"}],"success":true}}
{"jsonrpc":"2.0","id":3,"result":{"baseURL":"http://localhost:50050/usbdrive","success":true}}
{"jsonrpc":"2.0","id":3,"result":{"success":true}}
```

## Events
```
None
```
