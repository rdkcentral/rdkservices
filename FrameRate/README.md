-----------------
Build:

bitbake wpeframework-service-plugins

or

bitbake thunder-plugins

-----------------
Test:

curl -d '{"jsonrpc":"2.0","id":"3","params": {"frequency":1000},"method": "org.rdk.FrameRate.1.setCollectionFrequency"}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","params": {"newFpsValue":60},"method": "org.rdk.FrameRate.1.updateFps"}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","params": {"newFpsValue":30},"method": "org.rdk.FrameRate.1.updateFps"}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.FrameRate.1.startFpsCollection"}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.FrameRate.1.stopFpsCollection"}' http://127.0.0.1:9998/jsonrpc
