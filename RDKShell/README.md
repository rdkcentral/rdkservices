-----------------
# RDKShell

## Versions
`org.rdk.RDKShell.1`

## Methods:
```
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.moveToFront", "params":{ "client": "org.rdk.Netflix"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.moveToBack", "params":{ "client": "org.rdk.Netflix"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.moveBehind", "params":{ "client": "org.rdk.Netflix", "target": "org.rdk.RDKBrowser2"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.setFocus", "params":{ "client": "org.rdk.Netflix"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.kill", "params":{ "client": "org.rdk.Netflix"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.addKeyIntercept", "params":{"keyCode": 10, "modifiers": ["alt", "shift"], "client": "org.rdk.Netflix"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.removeKeyIntercept", "params":{"keyCode": 10, "modifiers": ["alt", "shift"], "client": "org.rdk.Netflix"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.getScreenResolution", "params":{}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.setScreenResolution", "params":{ "w": 1920, "h": 1080}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.getClients", "params":{}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.getZOrder", "params":{}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.getBounds", "params":{ "client": "org.rdk.RDKBrowser2"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.setBounds", "params":{"client": "org.rdk.RDKBrowser2", "x": 0, "y": 0, "w": 600, "h": 400}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.getVisibility", "params":{ "client": "org.rdk.Netflix"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.setVisibility", "params":{ "client": "org.rdk.Netflix", "visible": true}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.getOpacity", "params":{ "client": "org.rdk.Netflix"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.RDKShell.1.setOpacity", "params":{ "client": "org.rdk.Netflix", "opacity": 100}}' http://127.0.0.1:9998/jsonrpc
```

## Responses
```
moveToFront:
{"jsonrpc":"2.0", "id":3, "result": {} }

moveToBack:
{"jsonrpc":"2.0", "id":3, "result": {} }

moveBehind:
{"jsonrpc":"2.0", "id":3, "result": {} }

setFocus:
{"jsonrpc":"2.0", "id":3, "result": {} }

kill:
{"jsonrpc":"2.0", "id":3, "result": {} }

addKeyIntercept:
{"jsonrpc":"2.0", "id":3, "result": {} }

removeKeyIntercept:
{"jsonrpc":"2.0", "id":3, "result": {} }

getScreenResolution:
{"jsonrpc":"2.0", "id":3, "result": {"w": 1920, "h": 1080} }

setScreenResolution:
{"jsonrpc":"2.0", "id":3, "result": {} }

getClients:
{"jsonrpc":"2.0", "id":3, "result": { "clients": ["org.rdk.Netflix", "org.rdk.RDKBrowser2"]} }

getZOrder:
{"jsonrpc":"2.0", "id":3, "result": { "clients": ["org.rdk.Netflix", "org.rdk.RDKBrowser2"]} }

getBounds:
{"jsonrpc":"2.0", "id":3, "result": {
             "x": 0,
             "y": 0,
             "w": 600,
             "h": 400} }
setBounds:
{"jsonrpc":"2.0", "id":3, "result": {} }

getVisibility:
{"jsonrpc":"2.0", "id":3, "result": {"visible": true} }

setVisibility:
{"jsonrpc":"2.0", "id":3, "result": {} }

getOpacity:
{"jsonrpc":"2.0", "id":3, "result": { "opacity" : 100} }

setOpacity:
{"jsonrpc":"2.0", "id":3, "result": {} }
```

## Events
```
none
```

## Full Reference
https://wiki.rdkcentral.com/display/RDK/RDK+Shell
