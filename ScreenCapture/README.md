-----------------
Build:

bitbake wpeframework-service-plugins

or

bitbake thunder-plugins

-----------------
Test:

curl -d '{"jsonrpc":"2.0","id":"3","params": {"url":"http://10.0.0.233/upload.php"},"method": "org.rdk.ScreenCapture.1.uploadScreenCapture"}' http://127.0.0.1:9998/jsonrpc
curl -d '{"jsonrpc":"2.0","id":"3","params": {"url":"http://10.0.0.233/cgi-bin/upload.cgi", "callGUID": "test_guid"},"method": "org.rdk.ScreenCapture.1.uploadScreenCapture"}' http://127.0.0.1:9998/jsonrpc

