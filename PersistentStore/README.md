-----------------
# PersistentStore

## Versions
`org.rdk.PersistentStore.1`

## Methods:
```
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.PersistentStore.1.setValue","params":{"namespace":"foo","key":"key1","value":"value1"}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.PersistentStore.1.getValue","params":{"namespace":"foo","key":"key1"}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.PersistentStore.1.deleteKey","params":{"namespace":"foo","key":"key1"}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.PersistentStore.1.deleteNamespace","params":{"namespace":"foo"}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.PersistentStore.1.getKeys","params":{"namespace":"foo"}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.PersistentStore.1.getNamespaces","params":{}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.PersistentStore.1.getStorageSize","params":{}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method":"org.rdk.PersistentStore.1.flushCache"}' http://127.0.0.1:9998/jsonrpc
```

## Responses
```
{"jsonrpc":"2.0","id":3,"result":{"success":true}}
{"jsonrpc":"2.0","id":3,"result":{"value":"value1","success":true}}
{"jsonrpc":"2.0","id":3,"result":{"success":true}}
{"jsonrpc":"2.0","id":3,"result":{"success":true}}
{"jsonrpc":"2.0","id":3,"result":{"keys":["key1","key2","keyN"],"success":true}}
{"jsonrpc":"2.0","id":3,"result":{"namespaces":["ns1","ns2","nsN"],"success":true}}
{"jsonrpc":"2.0","id":3,"result":{"namespaceSizes":{"ns1":534,"ns2":234,"nsN":298},"success":true}}
```

## Events
```
none
```

## Full Reference
https://etwiki.sys.comcast.net/display/RDK/PersistentStore
