# 10. New Plugin test and validation for In-process Plugin

Each RDK Service can be validated through JSON RPC Commands through HTTP. It has a request and response in JSON format.

"callsign":"org.rdk.PluginName"

| Function | Request | Response |
| :-------- | :-------- | :-------- |
| Activate controller | curl http://localhost:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{ "jsonrpc":"2.0", "id":3, "method":"Controller.1.activate", "params":{"callsign":"org.rdk.PluginName"} }'  | {"jsonrpc":"2.0","id":3,"result":{"success":true}} |
|Deactivate controller|curl http://localhost:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{ "jsonrpc":"2.0", "id":3, "method":"Controller.1.deactivate", "params":{"callsign":"org.rdk.PluginName"} }'|{"jsonrpc":"2.0","id":3,"result":{"success":true}}|
|getPluginStatus|curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getPluginNameStatus"}'|{"jsonrpc":"2.0","id":3,"result":{"connection status from plugin":["CONNECTED"],"success":true}}|
| getPluginNameList |	curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getPluginNameList"}'|{"jsonrpc":"2.0","id":3,"result":{"Supported plugin list":["plug-A","plug-B","plug-C","plug-D","plug-E"],"success":true}}|	
|getPluginNameInfo|	curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getPluginNameInfo", "params":{"plugin_name":"plug-A"}}'| {"jsonrpc":"2.0","id":3,"result":{"supportedTvResolutions":["xyz-plugin","no:430HT5"],"success":true}}root@raspberrypi-rdk-mc:~#|
| event API when hdmi connected|  curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getConnectedVideoDisplays"}'| {"jsonrpc":"2.0","id":3,"result":{"connectedVideoDisplays":["HDMI0"],"success":true}}root@raspberrypi-rdk-mc:~#|
|event API when hdmi not connected| curl http://127.0.0.1:9998/jsonrpc --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method":"org.rdk.PluginName.1.getConnectedVideoDisplays"}'| {"jsonrpc":"2.0","id":3,"result":{"connectedVideoDisplays":[],"success":true}}root@raspberrypi-rdk-mc:~#|