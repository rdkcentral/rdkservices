-----------------
Build:

bitbake thunder-plugins

-----------------
Test:

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.UserPreferences.1.setUILanguage", "params": {"language": "UP_LANG1"}}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.UserPreferences.1.getUILanguage", "params": {"language": "UP_LANG1"}}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.UserPreferences.1.setUILanguage", "params": {"language": "UP_LANG2"}}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.UserPreferences.1.getUILanguage", "params": {"language": "UP_LANG2"}}' http://127.0.0.1:9998/jsonrpc