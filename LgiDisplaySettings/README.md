-----------------
Build:

bitbake thunder-plugins

-----------------
Test:

curl -d '{"jsonrpc":"2.0","id":"3","method": "com.lgi.rdk.DisplaySettings.1.getSupportedResolutions"}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "com.lgi.rdk.DisplaySettings.1.getSupportedTvResolutions"}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "com.lgi.rdk.DisplaySettings.1.getConnectedAudioPorts"}' http://127.0.0.1:9998/jsonrpc;

curl -d '{"jsonrpc":"2.0","id":"3","method": "com.lgi.rdk.DisplaySettings.1.getSupportedAudioPorts"}' http://127.0.0.1:9998/jsonrpc;

curl -d '{"jsonrpc":"2.0","id":"3","method": "com.lgi.rdk.DisplaySettings.1.getSupportedAudioModes", "params":{"audioPort":"HDMI0"}}' http://127.0.0.1:9998/jsonrpc;

curl -d '{"jsonrpc":"2.0","id":"3","method": "com.lgi.rdk.DisplaySettings.1.getSoundMode", "params":{"videoDisplay":"HDMI0"}}' http://127.0.0.1:9998/jsonrpc;
