# DataCapture

## Versions
`org.rdk.dataCapture.1`

## Methods:
```
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.dataCapture.1.enableAudioCapture", "params":{"bufferMaxDuration":6}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc": "2.0",  "id": "3", "method": "org.rdk.dataCapture.1.getAudioClip", "params": {"clipRequest": {"stream": "primary", "duration": 6, "captureMode": "preCapture", "url": "http://musicid.comcast.net/media-service-backend/analyze?trx=83cf6049-b722-4c44-b92e-79a504ae8f85:1458580048400&codec=PCM_16_16K&deviceId=5082732351093257712"}}}' http://127.0.0.1:9998/jsonrpc
```
## Events
```
onAudioClipReady
```
## Responses
```
enableAudioCapture:
{"jsonrpc":"2.0","id":3,"result":{"error":0,"success":true}

getAudioClip:
{"jsonrpc":"2.0","id":3,"result":{"error":0,"success":true}

onAudioClipReady:
{"fileName":"acm-songid0","status":false,"message":"Unable to read data from  /tmp/acm-songid0"}
```
## Full Reference
https://etwiki.sys.comcast.net/display/RDK/DataCapture
