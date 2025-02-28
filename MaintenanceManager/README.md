# MaintenanceManager

## Versions
`org.rdk.MaintenanceManager.1`

## Methods:
```
curl --req POST --data '{"jsonrpc":"2.0","id":"3","method": "Controller.1.activate", "params": {"callsign":"org.rdk.MaintenanceManager"}}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.getMaintenanceStartTime","params":{}}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.setMaintenanceMode","params":{"maintenanceMode":BACKGROUND}}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.getMaintenanceActivityStatus","params":{}}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.startMaintenance","params":{}}' http://127.0.0.1:9998/jsonrpc

```

## Responses:
```
getMaintenanceStartTime
{"jsonrpc":"2.0","id":3,"result":{"maintenanceStartTime":12345678,"success":true}}

setMaintenanceMode
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

getMaintenanceActivityStatus
{"jsonrpc":"2.0","id":3,"result":{"maintenanceStatus":"MAINTENANCE_IDLE or MAINTENANCE_STARTED or MAINTENANCE_ERROR or MAINTENANCE_COMPLETE or MAINTENANCE_INCOMPLETE","lastSuccessfulCompletionTime": 12345678, "isCriticalMaintenanc: true/false, "isRebootPending": true/false, "success":true}}"}}

startMaintenance
{"jsonrpc":"2.0","id":3,"result":{"success":true}}
```

## Events
```
onMaintenanceStatusChange

```
