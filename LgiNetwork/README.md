-----------------
Build:

bitbake thunder-plugins
-----------------

Test:

Commands to use
----------------
curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.getQuirks"}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.setApiVersionNumber", "params":{"version":5}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.getApiVersionNumber"}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.getInterfaces"}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.getDefaultInterface"}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.setDefaultInterface", "params":{"interface":"WIFI", "persist":false}}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.isInterfaceEnabled", "params":{"interface":"WIFI"}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.setInterfaceEnabled", "params":{"interface":"WIFI", "enabled":true, "persist":true}}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.getStbIp"}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.getNamedEndpoints"}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.trace", "params":{"endpoint":"45.57.221.20", "packets": 3}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.traceNamedEndpoint", "params":{"endpointName":"CMTS", "packets": 3}}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.ping", "params":{"endpoint":"45.57.221.20", "packets": 3}}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Network.1.pingNamedEndpoint", "params":{"endpointName":"CMTS", "packets": 3}}' http://127.0.0.1:9998/jsonrpc



