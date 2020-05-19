-----------------
Build:

bitbake thunder-plugins

-----------------
Test:


Methods common to typical plugins - getQuirks() and getApiVersionNumber() (not documented in official ControlService API)
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "org.rdk.ControlService.1.getQuirks"}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"4","method":"org.rdk.ControlService.1.getApiVersionNumber"}' http://127.0.0.1:9998/jsonrpc


A couple of example setValue() method calls 
curl -d '{"jsonrpc":"2.0","id":"5","method":"org.rdk.ControlService.1.setValues","params": {"enableASB":"true","conversationalMode" : 3}}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"6","method":"org.rdk.ControlService.1.setValues","params": {"enableOpenChime": true,"chimeVolume" : 2}}' http://127.0.0.1:9998/jsonrpc


An example getValues() method call (takes no parameters)
curl -d '{"jsonrpc":"2.0","id":"7","method":"org.rdk.ControlService.1.getValues"}' http://127.0.0.1:9998/jsonrpc


An example getLastKeypressSource() method call (takes no parameters)
curl -d '{"jsonrpc":"2.0","id":"8","method":"org.rdk.ControlService.1.getLastKeypressSource"}' http://127.0.0.1:9998/jsonrpc


An example getSingleRemoteData method call, for remoteId 2
curl -d '{"jsonrpc":"2.0","id":"9","method":"org.rdk.ControlService.1.getSingleRemoteData","params": {"remoteId" : 2}}' http://127.0.0.1:9998/jsonrpc


An example getAllRemoteData method call (takes no parameters).  If you have many paired XR remotes,the output can be HUGE!
curl -d '{"jsonrpc":"2.0","id":"10","method":"org.rdk.ControlService.1.getAllRemoteData"}' http://127.0.0.1:9998/jsonrpc


An example getLastPairedRemoteData() method call (takes no parameters)
curl -d '{"jsonrpc":"2.0","id":"11","method":"org.rdk.ControlService.1.getLastPairedRemoteData"}' http://127.0.0.1:9998/jsonrpc



