-----------------
Build:

bitbake wpeframework-service-plugins

-----------------
Test:

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Account.1.getLastCheckoutResetTime"}' http://127.0.0.1:9998/jsonrpc

curl -d '{"jsonrpc":"2.0","id":"3","method": "org.rdk.Account.1.setLastCheckoutResetTime" , "params" : {"resetTime" : 100 }  }' http://127.0.0.1:9998/jsonrpc
