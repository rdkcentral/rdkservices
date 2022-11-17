-----------------
Build:

bitbake wpeframework-service-plugins

-----------------
Test:

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Telemetry.1.getAvailableReportProfiles"}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Telemetry.1.setReportProfileStatus", "params" : {"reportProfile" : "FTUE", "status" : "STARTED" }}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Telemetry.1.setReportProfileStatus", "params" : {"reportProfile" : "FTUE", "status" : "COMPLETE" }}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Telemetry.1.logApplicationEvent", "params" : {"eventName" : "event", "eventValue" : "value" }}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Telemetry.1.uploadReport"}' http://127.0.0.1:9998/jsonrpc
