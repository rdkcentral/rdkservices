-----------------
# MigrationPreparer

## Versions
`org.rdk.MigrationPreparer`

## Methods:
```
curl -H "Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '"' -f 4`" --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"1","method": "org.rdk.MigrationPreparer.write", "params": {"name":"picture/viewing-mode","value":"\"vivid\""}}' http://127.0.0.1:9998/jsonrpc
curl -H "Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '"' -f 4`" --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"1","method": "org.rdk.MigrationPreparer.read", "params": {"name":"picture/viewing-mode"}}' http://127.0.0.1:9998/jsonrpc
curl -H "Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '"' -f 4`" --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"1","method": "org.rdk.MigrationPreparer.delete", "params": {"name":"picture/viewing-mode"}}' http://127.0.0.1:9998/jsonrpc
curl -H "Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '"' -f 4`" --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.MigrationPreparer.setComponentReadiness", "params": {"componentName":RA01}}' http://127.0.0.1:9998/jsonrpc
curl -H "Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '"' -f 4`" --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.MigrationPreparer.getComponentReadiness"}' http://127.0.0.1:9998/jsonrpc
curl -H "Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '"' -f 4`" --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.MigrationPreparer.reset", "params": {"resetType":"RESET_ALL"}}' http://127.0.0.1:9998/jsonrpc

```
## Responses
```
{"jsonrpc":"2.0","id":1,"result":{"success":true}}
{"jsonrpc":"2.0","id":1,"result":{"value":"\"vivid\"","success":true}}
{"jsonrpc":"2.0","id":1,"result":{"success":true}}
{"jsonrpc":"2.0","id":3,"result":{"success":true}}
{"jsonrpc":"2.0","id":3,"result":{"componentList":["RA01"],"success":true}}
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

```
