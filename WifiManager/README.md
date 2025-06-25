-----------------
Build:

bitbake thunder-plugins

-----------------
Test:

Scan:
this requires event handling and can't be done in curl, see "test/thunder-wifimanager-test.js"

Connect/State:
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.Wifi.1.getCurrentState"}'
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.Wifi.1.getConnectedSSID"}'
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.Wifi.1.setEnabled", "params": {"enable": false} }'
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.Wifi.1.connect", "params": {"ssid": "REDGuest", "passphrase": "R3Dguest", "securityMode": 5 }}'
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.Wifi.1.disconnect"}'
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.Wifi.1.isSignalThresholdChangeEnabled"}'
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": 3, "method": "org.rdk.Wifi.1.setSignalThresholdChangeEnabled", "params": {"enabled": true, "interval": 1000}}'

SSID Information : curl -H "Authorization: Bearer `WPEFrameworkSecurityUtility | cut -d '"' -f 4`" -d '{"jsonrpc":"2.0","id":"42","method": "org.rdk.Wifi.retrieveSSID"}' http://127.0.0.1:9998/jsonrpc

onWifiSignalTresholdChanged:
this requires event handling and can't be done in curl, see "test/thunder-wifimanager-test.js"

WPS:
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": "3", "method": "org.rdk.Wifi.1.initiateWPSPairing"}'
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": "3", "method": "org.rdk.Wifi.1.cancelWPSPairing"}'
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": "3", "method": "org.rdk.Wifi.1.saveSSID", "params":{"ssid": "123412341234", "passphrase": "foobar", "securityMode": 2}}'
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": "3", "method": "org.rdk.Wifi.1.clearSSID"}'
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": "3", "method": "org.rdk.Wifi.1.getPairedSSID"}'
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": "3", "method": "org.rdk.Wifi.1.getPairedSSIDInfo"}'
curl -X POST http://127.0.0.1:9998/Service/ -d '{"jsonrpc": "2.0", "id": "3", "method": "org.rdk.Wifi.1.isPaired"}'
