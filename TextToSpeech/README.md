-----------------
Build:

bitbake thunder-plugins

-----------------
Test:

---acquire Resource--
curl --header "Content-Type: application/json" -X POST http://localhost:9998/jsonrpc -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.TTSResource.1.acquireResource", "params":{"appId":32}}'/jsonrpc

---releaseResource--
curl --header "Content-Type: application/json" -X POST http://localhost:9998/jsonrpc -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.TTSResource.1.releaseResource", "params":{"appId":32}}'/jsonrpc

---claimResource--
curl --header "Content-Type: application/json" -X POST http://localhost:9998/jsonrpc -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.TTSResource.1.claimResource", "params":{"appId":32}}'/jsonrpc

---claimResource--
curl --header "Content-Type: application/json" -X POST http://localhost:9998/jsonrpc -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.TTSResource.1.isSessionActiveForApp", "params":{"appId":32}}'/jsonrpc

