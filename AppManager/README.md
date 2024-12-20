curl -H "$token" -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.AppManager.1.launchApp", "params": {"appId":"WebKitBrowser", "intent":"http://www.google.com", "launchArgs":{}}}' http://127.0.0.1:9998/jsonrpc
curl -H "$token" -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.AppManager.1.getLoadedApps", "params": {}}' http://127.0.0.1:9998/jsonrpc
curl -H "$token" -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.AppManager.1.closeApp", "params": {"appId":"WebKitBrowser"}}' http://127.0.0.1:9998/jsonrpc
curl -H "$token" -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.AppManager.1.terminateApp", "params": {"appId":"WebKitBrowser"}}' http://127.0.0.1:9998/jsonrpc
curl -H "$token" -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.AppManager.1.preloadApp", "params": {"appId":"WebKitBrowser", "intent":"http://www.google.com", "launchArgs":{}}}' http://127.0.0.1:9998/jsonrpc
curl -H "$token" -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.AppManager.1.sendIntent", "params": {"appId":"WebKitBrowser", "intent":"{\"action\":\"launch\",\"context\":{\"source\":\"user\"}}"}' http://127.0.0.1:9998/jsonrpc
