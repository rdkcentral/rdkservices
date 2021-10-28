-----------------
Build:

bitbake thunder-plugins

-----------------
Test:
Enable PIR
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.MotionDetection.1.arm", "params":{"index":"FP_MD", "mode": 2}}' http://127.0.0.1:9998/jsonrpc
Set sensitivity to max
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.MotionDetection.1.setSensitivity", "params":{"index":"FP_MD","name":"high"}}' http://127.0.0.1:9998/jsonrpc
check if armed:
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.MotionDetection.1.isarmed", "params":{"index":"FP_MD"}}' http://127.0.0.1:9998/jsonrpc
set no motion period: Run and see no event is triggered for 10 seconds
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.MotionDetection.1.setNoMotionPeriod", "params":{"index":"FP_MD","period":10}}' http://127.0.0.1:9998/jsonrpc

get all values:
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.MotionDetection.1.getNoMotionPeriod", "params":{"index":"FP_MD"}}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.MotionDetection.1.getSensitivity", "params":{"index":"FP_MD"}}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.MotionDetection.1.getLastMotionEventElapsedTime", "params":{"index":"FP_MD"}}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.MotionDetection.1.getMotionDetectors", "params":{"index":"FP_MD"}}' http://127.0.0.1:9998/jsonrpc

Set value sensitivity
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.MotionDetection.1.setSensitivity", "params":{"index":"FP_MD","value":"20"}}' http://127.0.0.1:9998/jsonrpc

curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.MotionDetection.1.getSensitivity", "params":{"index":"FP_MD"}}' http://127.0.0.1:9998/jsonrpc

Switch off
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":"3", "method":"org.rdk.MotionDetection.1.disarm", "params":{"index":"FP_MD"}}' http://127.0.0.1:9998/jsonrpc
