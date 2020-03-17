-----------------
Build:

bitbake thunder-plugins

-----------------
Test:

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.LoggingPreferences.1.setKeystrokeMaskEnabled", "params":{"keystrokeMaskEnabled":false}}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.LoggingPreferences.1.isKeystrokeMaskEnabled"}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.LoggingPreferences.1.setKeystrokeMaskEnabled", "params":{"keystrokeMaskEnabled":true}}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.LoggingPreferences.1.isKeystrokeMaskEnabled"}' http://127.0.0.1:9998/jsonrpc