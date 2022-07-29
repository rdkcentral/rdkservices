# BluetoothSettings

## Versions
`org.rdk.Bluetooth.1`

## Methods:
```
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.getApiVersionNumber"}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.enable"}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.disable"}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.getName"}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.setName", "params":{"name": "Xfinity Bluetooth Audio"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.isDiscoverable"}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.setDiscoverable", "params":{"discoverable":true, "timeout":10}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.startScan", "params": {"timeout": "5", "profile": "SMARTPHONE"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.stopScan"}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.getDiscoveredDevices"}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.getPairedDevices"}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.getConnectedDevices"}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.pair", "params": {"deviceID": "256168644324480"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.unpair", "params": {"deviceID": "256168644324480"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.connect", "params": {"deviceID": "256168644324480", "deviceType": "SMARTPHONE", "profile": "SMARTPHONE"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.disconnect", "params": {"deviceID": "256168644324480"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.setAudioStream", "params": {"deviceID": "256168644324480", "audioStreamName": "PRIMARY"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.getDeviceInfo", "params":{"deviceID":"256168644324480"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.getAudioInfo", "params": {"deviceID": "256168644324480"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.sendAudioPlaybackCommand", "params": {"deviceID": "256168644324480", "command": "PLAY"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id": "3", "method":"org.rdk.Bluetooth.1.respondToEvent", "params": {"deviceID": "256168644324480", "eventType": "onPairingRequest", "responseValue": "ACCEPTED"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.setDeviceVolumeMuteInfo", "params": {"deviceID": "256168644324480", "profile": "WEARABLE HEADSET", "volume": "255", "mute": "1"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.Bluetooth.1.getDeviceVolumeMuteInfo", "params": {"deviceID": "256168644324480", "profile": "WEARABLE HEADSET"}}' http://127.0.0.1:9998/jsonrpc
```

## Responses:
```
getApiVersionNumber:
{"jsonrpc":"2.0","id":3,"result":{"version":1,"success":true}}

enable:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

disable:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

getName:
{"jsonrpc":"2.0","id":3,"result":{"name":"Xfinity Bluetooth Audio","success":true}}

setName:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

isDiscoverable:
{"jsonrpc":"2.0","id":3,"result":{"discoverable":false,"success":true}}

setDiscoverable:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

startScan:
{"jsonrpc":"2.0","id":3,"result":{"status":"AVAILABLE","success":true}}

stopScan:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

getDiscoveredDevices:
{"jsonrpc":"2.0","id":3,"result":{"discoveredDevices":[{"deviceID":"61579454946360","name":"[TV] UE32J5530","deviceType":"TV","connected":false,"paired":false}],"success":true}}

getPairedDevices:
{"jsonrpc":"2.0","id":3,"result":{"pairedDevices":[{"deviceID":"256168644324480","name":"Eleven","deviceType":"SMARTPHONE","connected":true},{"deviceID":"26499258260618","name":"Little Big","deviceType":"SMARTPHONE","connected":false}],"success":true}}

getConnectedDevices:
{"jsonrpc":"2.0","id":3,"result":{"connectedDevices":[{"deviceID":"256168644324480","name":"Eleven","deviceType":"SMARTPHONE","activeState":"0"}],"success":true}}

pair:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

unpair:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

connect:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

disconnect:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

setAudioStream:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

getDeviceInfo:
{"jsonrpc":"2.0","id":3,"result":{"deviceInfo":{"deviceID":"256168644324480","name":"Eleven","deviceType":"SMARTPHONE","manufacturer":"640","MAC":"E8:FB:E9:0C:2C:80","signalStrength":"0","rssi":"0","supportedProfile":"Not Identified;Not Identified;Audio Source;AV Remote Target;AV Remote;Not Identified;Handsfree - Audio Gateway;Not Identified;Not Identified;PnP Information;Generic Attribute;Not Identified"},"success":true}}

getAudioInfo:
{"jsonrpc":"2.0","id":3,"result":{"trackInfo":{"album":"Spacebound Apes","genre":"Jazz","title":"Grace","artist":"Neil Cowley Trio","ui32Duration":"217292","ui32TrackNumber":"1","ui32NumberOfTracks":"73"},"success":true}}

sendAudioPlaybackCommand:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

respondToEvent:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

setDeviceVolumeMuteInfo:
{"jsonrpc":"2.0","id":3,"result":{"success":true}}

getDeviceVolumeMuteInfo:
{"jsonrpc":"2.0","id":3,"result":{"volumeInfo":{"volume":"255","mute":false},"success":true}}
```

## Events
```
onStatusChanged
onPairingRequest
onRequestFailed
onConnectionRequest
onPlaybackRequest
onPlaybackChange
onPlaybackProgress
onPlaybackNewTrack
onDeviceFound
onDeviceLost
onDiscoveredDevice
onDeviceMediaStatus
```

## Full Reference
https://etwiki.sys.comcast.net/display/RDKV/Bluetooth

