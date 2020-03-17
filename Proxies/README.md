### Proxies thunder plugin

---
Test commands:

curl -d '{"jsonrpc":"2.0","id":"1","method": "org.rdk.Proxies.1.setProxy", "params":{"name":"uri4", "uri":"https://abc.com:123/def/ghi?a=b&c=d#jkl"}}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"1","method": "org.rdk.Proxies.1.getProxy", "params":{"name":"uri4"}}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"1","method": "org.rdk.Proxies.1.setProxy", "params":{"name":"uri4", "uri":"https://xyz.com:123/def/ghi?a=b&c=d#jkl"}}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"1","method": "org.rdk.Proxies.1.getProxy", "params":{"name":"uri4"}}' http://127.0.0.1:9998/jsonrpc
