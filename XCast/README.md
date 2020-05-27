-----------------
Build:

bitbake rdkservices

-----------------
Test:

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Xcast.1.getApiVersionNumber"}' http://127.0.0.1:9998/jsonrpc

{"jsonrpc":"2.0","id":3,"result":{"version":1,"success":true}}

 curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Xcast.1.getQuirks"}' http://127.0.0.1:9998/jsonrpc

{"jsonrpc":"2.0","id":3,"result":{"quirks":[],"success":true}}

