-----------------
Build:

bitbake wpeframework-service-plugins

-----------------
Test:

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "HdmiCec.1."}' http://127.0.0.1:9998/jsonrpc
