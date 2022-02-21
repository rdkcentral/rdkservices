-----------------
Build:

bitbake thunder-plugins

-----------------
Test:


Methods common to typical plugins - getQuirks() and getApiVersionNumber() (not specific to the VoiceControl API)

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.VoiceControl.1.getQuirks"}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"4","method":"org.rdk.VoiceControl.1.getApiVersionNumber"}' http://127.0.0.1:9998/jsonrpc




